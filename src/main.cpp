/**
 * MIT License
 *
 * Copyright (c) 2023 Julian Unterweger, Kathrin Hanauer, Martin Seybold
 *                    Faculty of Computer Science, University of Vienna, Austria
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <chrono>
#include <iostream>
#include <iomanip>
#include <ctime>

#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

#include "instance.h"
#include "strip_algorithm.h"
#include "partition_algorithm.h"
#include "result_writer.h"
#include "algorithm_runner.h"
#include "bbox_cover_splitter.h"
#include "partition_cover_splitter.h"
#include "cover_joiner.h"
#include "greedy_set_cover_algorithm.h"
#include "ILP_algorithm.h"
#include "cover_trimmer.h"
#include "cover_pruner.h"
#include "cover_joiner_full.h"
#include "bbox_cover_splitter.h"
#include "partition_cover_splitter.h"
#include "cover_trimmer.h"

namespace keywords = boost::log::keywords;
using namespace cover;

std::ostream &operator<<(std::ostream &s, const std::chrono::time_point<std::chrono::system_clock> &tp) {
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    s  << std::put_time(std::localtime(&time_t), "%Y-%m-%d %X");
    return s;
}

std::vector<std::string> split(const std::string &str) {
    std::size_t pos, startPos = 0;
    std::vector<std::string> tokens;
    while ((pos = str.find("+", startPos, 1)) != std::string::npos) {
        tokens.emplace_back(str.substr(startPos, pos - startPos));
        startPos = pos + 1;
    }
    tokens.emplace_back(str.substr(startPos));
    return tokens;
}

std::unique_ptr<Algorithm> string_to_algorithm(const std::string &str, double timeout) {
    if (str == "greedy") {
        return std::make_unique<Greedy_set_cover_algorithm>();
    } else if (str == "strip") {
        return std::make_unique<Strip_algorithm>();
    } else if (str == "partition") {
        return std::make_unique<Partition_algorithm>();
    }
#ifdef GUROBI_AVAILABLE
        else if (str == "ilp") {
            return std::make_unique<ILP_algorithm>(false, timeout);
        } else if (str == "ilp-pixel") {
            return std::make_unique<ILP_algorithm>(true, timeout);
        }
#else
    else if (str == "ilp" || str == "ilp-pixel") {
        throw std::runtime_error("Cannot use ilp formulation as Gurobi was unavailable at compile time");
    }
#endif
    else {
        throw std::runtime_error("Unknown algorithm name specified");
    }
}

template<class T>
std::unique_ptr<Cover_postprocessor> string_to_postprocessor(const std::string &str,
                                                             std::unique_ptr<T> previous_provider) {
    if (str == "prune") {
        return std::make_unique<Cover_pruner>(std::move(previous_provider));
    } else if (str == "trim") {
        return std::make_unique<Cover_trimmer>(std::move(previous_provider));
    } else if (str == "join") {
        return std::make_unique<Cover_joiner>(std::move(previous_provider));
    } else if (str == "join-full") {
        return std::make_unique<Cover_joiner_full>(std::move(previous_provider));
    } else if (str == "bbox-split") {
        return std::make_unique<Bounding_box_cover_splitter>(std::move(previous_provider));
    } else if (str == "partition-split") {
        return std::make_unique<Partition_cover_splitter>(std::move(previous_provider));
    } else {
        throw std::runtime_error("Unknown postprocessor name specified: " + str);
    }
}



int main(int argc, char **argv) {

    CLI::App app{"App description"};

    std::string polygon_wkt_path{};
    app.add_option("-i,--input,input", polygon_wkt_path, "path to this problem instance's polygon's WKT file")
            ->required()
            ->check(CLI::ExistingFile);

    std::pair<CostType, CostType> costs{};
    app.add_option("-c,--costs,costs", costs, "(creation cost, area cost) pair for this problem instance")
            ->required()
            ->check(CLI::Range(0, std::numeric_limits<int>::max()));

    std::string algorithm_name{};
    app.add_option("-a,--algorithm,algorithm", algorithm_name, "name of the algorithm to use to solve the passed "
                                                               "problem instance.")
            ->required()
            ->ignore_case();

    std::vector<std::string> postprocessor_names{};
    app.add_option("-p,--postprocessors,postprocessors", postprocessor_names, "names of the postprocessors to run on "
                                                                              "the cover returned by the algorithm, "
                                                                              "executed in order from left to right")
            ->ignore_case()
            ->check(CLI::IsMember({"prune", "trim", "trim-reverse", "join", "join-full",
                                   "bbox-split", "partition-split",
                                   "brprune", "brbbox-split", "brpartition-split", "brtrim"}));

    std::string output_path{};
    app.add_option("-o,--output,output", output_path, "path where the JSON or CSV file containing the results of "
                                                           "running the algorithm on the input should be created, "
                                                           "non-existing folders will be created, pre-existing JSON files "
                                                           "will be overwritten")
            ->required();

    bool verify_cover{true};
    app.add_option("-v,--verify", verify_cover, "whether to verify that the algorithm's result is actually a valid "
                                                "cover, default is true, the time spent verifying is not counted "
                                                "towards the algorithm's execution time");

    double timeout {0.0};
    app.add_option("-t,--timeout", timeout, "set a timeout in seconds per polygon");

    std::string log_file{};
#ifdef COVER_MAX_LOG_LEVEL
    app.add_option("-l,--log-file", log_file, "path to write logs to");
#else
    app.add_option("-l,--log-file", log_file, "path to write logs to (note: logging was turned off during "
                                              "compile time, so this option cannot be used, to enable it "
                                              "provide COVER_MAX_LOG_LEVEL in src/CMakeLists.txt or when "
                                              "running cmake");
#endif

    CLI11_PARSE(app, argc, argv);

#ifdef COVER_MAX_LOG_LEVEL
    logging::add_common_attributes();
    if (!log_file.empty()) {
        logging::add_file_log(
                keywords::file_name = log_file,
                keywords::target_file_name = log_file,
                keywords::auto_flush = true,
                keywords::format = "[%TimeStamp%] - [%Severity%]: %Message%"
        );
        std::cout << "\nLog file: " << log_file << std::endl;
    } else {
        std::cout << "\nLog file: -\n";
    }
#else
    if (!log_file.empty()) {
        std::cerr << "\nWARNING: log file path specified via -l or --log-file, but logging was disabled at "
                     "compile time, no logging will take place\n";
    }
#endif

    std::cout << "Problem instance:\n\tInput WKT: " << polygon_wkt_path << "\n\tCreation cost: "
              << costs.first << "\n\tArea cost: " << costs.second << std::endl;

    Problem_instance instance{polygon_wkt_path, costs.first, costs.second};

    auto  algorithm_tokens = split(algorithm_name);
    const auto &base_algorithm_name = algorithm_tokens[0];

    std::unique_ptr<Algorithm> algorithm{string_to_algorithm(base_algorithm_name, timeout)};
    postprocessor_names.insert(postprocessor_names.begin(), algorithm_tokens.begin()+1, algorithm_tokens.end());

    std::unique_ptr<Cover_postprocessor> current_postprocessor{nullptr};
    bool prune_used{false};
    for (const auto &postprocessor_name: postprocessor_names) {
        if (postprocessor_name == "trim"  && !prune_used) {
            std::cerr << "WARNING: 'trim' assumes there are no fully redundant rectangles in the cover, "
                         "if you are certain that there are no redundant rectangles, ignore this warning, "
                         "otherwise you may want to prune first";
        } else if (postprocessor_name == "prune") {
            prune_used = true;
        }
        if (current_postprocessor == nullptr) {
            current_postprocessor = string_to_postprocessor(postprocessor_name, std::move(algorithm));
        } else {
            current_postprocessor = string_to_postprocessor(postprocessor_name, std::move(current_postprocessor));
        }
    }

    std::cout << "\nUsing:\n\tAlgorithm: " << base_algorithm_name
        << "\n\tPostprocessors: ";

    std::stringstream ss;
    ss << base_algorithm_name;
    if (postprocessor_names.empty()) {
        std::cout << '-' << std::endl;
    } else {
        for (const auto &postprocessor: postprocessor_names) {
            std::cout << postprocessor << " ";
            ss << "+" << postprocessor;
        }
        std::cout << std::endl;
    }
    auto algorithm_full_name {ss.str()};
    std::cout
        << "\n\tFull algorithm name: " << algorithm_full_name;

    std::unique_ptr<Cover_provider> cover_provider{};

    if (current_postprocessor != nullptr) {
        cover_provider = std::move(current_postprocessor);
    } else {
        cover_provider = std::move(algorithm);
    }

    std::cout << "\nOutput path: " << output_path;
    std::cout << "\nCover verification: " << (verify_cover ? "on" : "off");

    const auto &exp_start = std::chrono::system_clock::now();
    std::cout << "\n\nStart creating cover at " << exp_start
        << "..." << std::endl;
    const auto results{Algorithm_runner::run_algorithm(*cover_provider, instance, verify_cover)};
    const auto &exp_end = std::chrono::system_clock::now();
    std::cout << "Finished at " << exp_end << ".\n\nResults:" ;

    auto printResult = [](const auto &result) {
        std::cout
            << "\n\tTotal cost: " << result.cost.creation_cost + result.cost.area_cost
            << "\n\tCreation cost: " << result.cost.creation_cost
            << "\n\tArea cost: " << result.cost.area_cost
            << "\n\tCover size: " << result.cover_size
            << "\n\tExecution time: "
                  << std::chrono::duration<double>(result.execution_time).count() << "s"
            << "\n\tValid: "
            << (result.is_valid == Algorithm_runner::Result::Validity::VALID
                ? "yes" : (result.is_valid == Algorithm_runner::Result::Validity::INVALID
                ? "NO" : (result.is_valid == Algorithm_runner::Result::Validity::TIMEOUT
                ? "TIMEOUT" : "not checked (specify -v to enable verification)")));
    };
    int retval = 0;
    std::ostringstream warning_string{};
    for (size_t i = 1; i < results.size(); i++) {
        const auto &result = results[i];
        std::cout << "\nPolygon " << i << "/" << (results.size()-1) << ":";
        printResult(result);
        if (result.is_valid == Algorithm_runner::Result::Validity::INVALID) {
                retval |= 1;

                warning_string << "\nAlgorithm '" << algorithm_full_name
                    << "' failed to cover polygon "
                    << i << "/" << (results.size()-1)
                    << " in instance '"
                    << instance.get_name()
                    << "' with creation cost "
                    << instance.get_costs().creation_cost
                    << " and area cost " << instance.get_costs().area_cost << "\n";
        }
        if (result.is_valid == Algorithm_runner::Result::Validity::TIMEOUT) {
                retval |= 2;
                warning_string << "\nAlgorithm '" << algorithm_full_name
                    << "' reached timeout on polygon "
                    << i << "/" << (results.size()-1)
                    << " in instance '"
                    << instance.get_name()
                    << "' with creation cost "
                    << instance.get_costs().creation_cost
                    << " and area cost " << instance.get_costs().area_cost << "\n";
        }
    }
    if (retval > 0) {
        std::cerr << warning_string.str() << std::endl;
        LOG(warning) << warning_string.str();
    }

    std::cout << "\n\nTotal for all polygons in this instance:";
    printResult(results[0]);

    std::cout << "\n\nWriting result to: " << output_path << std::endl;
    std::stringstream start, end;
    start << exp_start;
    end << exp_end;
    Result_writer::write_result(instance, results, algorithm_full_name, output_path, start.str(), end.str());

    return retval;
}
