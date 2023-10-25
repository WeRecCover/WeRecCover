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

#include "result_writer.h"
#include "algorithm_runner.h"
#include <sstream>

namespace cover {
    std::string Result_writer::multi_polygon_to_wkt_string(const MultiPolygon &multi_polygon) {
        CGAL::internal::Geometry_container<const MultiPolygon, boost::geometry::multi_polygon_tag> gc(multi_polygon);

        std::ostringstream wkt_stream{};

        wkt_stream << boost::geometry::wkt(gc);

        return wkt_stream.str();
    }

    json Result_writer::result_to_json(const Problem_instance &instance,
                                       const std::string &algorithm_full_name,
                                       const std::vector<Algorithm_runner::Result> &results,
                                       const std::string &startTime,
                                       const std::string &endTime) {

        MultiPolygon cover_multi_polygon{};

        for (const auto &result : results) {
            for (const auto &rectangle: result.cover) {
                cover_multi_polygon.push_back(Polygon_with_holes(rectangle.as_polygon()));
            }
        }

        json output{
                {"time_start",                  startTime},
                {"time_end",                    endTime},
                {"algorithm",                   algorithm_full_name},
                {"instance_name",               instance.get_name()},
                {"input_polygon",               multi_polygon_to_wkt_string(instance.get_multi_polygon())},
                {"creation_cost",               instance.get_rectangle_creation_cost()},
                {"area_cost",                   instance.get_rectangle_area_cost()},
                {"cover",                       multi_polygon_to_wkt_string(cover_multi_polygon)},
                {"cover_size",                  results[0].cover_size},
                {"total_cost",                  results[0].cost.area_cost + results[0].cost.creation_cost},
                {"total_creation_cost",         results[0].cost.creation_cost},
                {"total_area_cost",             results[0].cost.area_cost},
                {"execution_time_seconds",      std::chrono::duration_cast<std::chrono::seconds>(
                        results[0].execution_time).count()},
                {"execution_time_milliseconds", std::chrono::duration_cast<std::chrono::milliseconds>(
                        results[0].execution_time).count()},
                {"execution_time_nanoseconds",  results[0].execution_time.count()},
        };

        switch (results[0].is_valid) {
            case Algorithm_runner::Result::Validity::VALID:
                output["is_valid"] = true;
                break;
            case Algorithm_runner::Result::Validity::INVALID:
                output["is_valid"] = false;
                break;
            case Algorithm_runner::Result::Validity::TIMEOUT:
                output["is_valid"] = "timeout";
                break;
            default:
                output["is_valid"] = json::value_t::null;
        }
        output["polygon"] = json::array();
        for (size_t i = 1; i < results.size(); i++) {
            output["polygon"][i-1] = {
                {"polygon",                     i},
                {"cover_size",                  results[i].cover_size},
                {"total_cost",                  results[i].cost.area_cost + results[i].cost.creation_cost},
                {"total_creation_cost",         results[i].cost.creation_cost},
                {"total_area_cost",             results[i].cost.area_cost},
                {"execution_time_seconds",      std::chrono::duration_cast<std::chrono::seconds>(
                        results[i].execution_time).count()},
                {"execution_time_milliseconds", std::chrono::duration_cast<std::chrono::milliseconds>(
                        results[i].execution_time).count()},
                {"execution_time_nanoseconds",  results[i].execution_time.count()},
            };
            switch (results[i].is_valid) {
                case Algorithm_runner::Result::Validity::VALID:
                    output["polygon"][i-1]["is_valid"] = true;
                    break;
                case Algorithm_runner::Result::Validity::INVALID:
                    output["polygon"][i-1]["is_valid"] = false;
                    break;
                case Algorithm_runner::Result::Validity::TIMEOUT:
                    output["polygon"][i-1]["is_valid"] = "timeout";
                    break;
                default:
                    output["polygon"][i-1]["is_valid"] = json::value_t::null;
            }
        }

        return output;
    }

    std::stringstream Result_writer::result_to_csv(const Problem_instance &instance,
                                    const std::string &algorithm_full_name,
                                    const std::vector<Algorithm_runner::Result> &results,
                                    const std::string &startTime,
                                    const std::string &endTime) {

        std::stringstream str;
        for (size_t i = 0; i < results.size(); i++) {
            const auto &result = results[i];
            str
                << startTime << ","
                << endTime << ","
                << instance.get_name() << ","
                << (results.size() - 1) << ","
                << i << ","
                << algorithm_full_name << ","
                << instance.get_rectangle_creation_cost() << ","
                << instance.get_rectangle_area_cost() << ","
                << result.cover_size << ","
                << result.cost.creation_cost << ","
                << result.cost.area_cost << ","
                << result.cost.area_cost + result.cost.creation_cost << ","
                << std::chrono::duration_cast<std::chrono::seconds>(result.execution_time).count() << ","
                << std::chrono::duration_cast<std::chrono::milliseconds>(result.execution_time).count() << ","
                << result.execution_time.count() << ",";


            switch (result.is_valid) {
                case Algorithm_runner::Result::Validity::VALID:
                    str << "true";
                    break;
                case Algorithm_runner::Result::Validity::INVALID:
                    str << "false";
                    break;
                case Algorithm_runner::Result::Validity::TIMEOUT:
                    str << "timeout";
                    break;
                default:
                    str << "null";
            }
            str << "\n";
        }
        return str;
    }

    std::stringstream get_csv_header() {
        std::stringstream str;
        str << "time_start,"
            << "time_end,"
            << "instance_name,"
            << "num_polygons,"
            << "polygon_id,"
            << "algorithm,"
            << "creation_cost,"
            << "area_cost,"
            << "cover_size,"
            << "total_creation_cost,"
            << "total_area_cost,"
            << "total_cost,"
            << "execution_time_seconds,"
            << "execution_time_milliseconds,"
            << "execution_time_nanoseconds,"
            << "valid\n";
        return str;
    }

    void Result_writer::write_result(const Problem_instance &instance,
                                     const std::vector<Algorithm_runner::Result> &results,
                                     const std::string &algorithm_full_name,
                                     const fs::path &output_path,
                                    const std::string &startTime,
                                    const std::string &endTime) {

        auto parent_path{output_path.parent_path()};
        if (!parent_path.empty() && !fs::exists(parent_path)) {
            fs::create_directories(parent_path);
        }
        if (output_path.extension() == ".csv") {
            if (!fs::exists(output_path)) {
                std::ofstream out_file{output_path};
                out_file << get_csv_header().rdbuf();
                out_file << result_to_csv(instance, algorithm_full_name,
                                        results,startTime, endTime).rdbuf();
            } else {
                std::ofstream out_file{output_path, std::ios_base::app};
                out_file << result_to_csv(instance, algorithm_full_name,
                                        results, startTime, endTime).rdbuf();

            }
        } else {
            std::ofstream out_file{output_path};
            out_file << result_to_json(instance, algorithm_full_name,
                                        results, startTime, endTime);
        }
    }

} // cover
