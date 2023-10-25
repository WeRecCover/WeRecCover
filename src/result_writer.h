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

#ifndef COVERING_RESULT_WRITER_H
#define COVERING_RESULT_WRITER_H

#include <string>
#include <experimental/filesystem>
#include <sstream>

#include <CGAL/IO/WKT.h>

#include <boost/geometry.hpp>

#include <nlohmann/json.hpp>

#include "logging.h"

#include "algorithm_runner.h"

namespace fs = std::experimental::filesystem;

using json = nlohmann::json;

namespace cover {

    /**
     * @brief Class which writes results of the algorithms to the filesystem in a JSON format
     */
    class Result_writer {
    protected:
        /**
         * Converts the provided deque of polygons into a WKT string.
         *
         * @param multi_polygon The deque of polygons to convert
         * @return A WKT string representing the deque of polygons
         */
        static std::string multi_polygon_to_wkt_string(const MultiPolygon &multi_polygon);

        /**
         * Combines and converts the problem instance, algorithm name, postprocessor names and algorithm result into
         * a JSON object.
         *
         * @param instance The problem instance the algorithm was run on
         * @param algorithm_name The name of the used algorithm
         * @param postprocessor_names The names of the postprocessors in order of their application
         * @param results The results of running the algorithm on the problem instance, one result per polygon
         * @return The combined JSON object
         */
        static json result_to_json(const Problem_instance &instance,
                                   const std::string &algorithm_full_name,
                                   const std::vector<Algorithm_runner::Result> &results,
                                   const std::string &startTime,
                                   const std::string &endTime);

        /**
         * Combines and converts the problem instance, algorithm name, postprocessor names and algorithm result into
         * a CSV line.
         *
         * @param instance The problem instance the algorithm was run on
         * @param algorithm_name The name of the used algorithm
         * @param postprocessor_names The names of the postprocessors in order of their application
         * @param results The results of running the algorithm on the problem instance, one result per polygon
         * @return One line per polygon in CSV format
         */
        static std::stringstream result_to_csv(const Problem_instance &instance,
                                   const std::string &algorithm_full_name,
                                   const std::vector<Algorithm_runner::Result> &results,
                                   const std::string &startTime,
                                   const std::string &endTime);

    public:
        /**
         * Converts the problem instance, algorithm result, algorithm names and postprocessor names into a JSON object
         * which is then written to the filesystem at the output_path.
         *
         * @param instance The problem instance the algorithm was run on
         * @param result The result of running the algorithm on the problem instance
         * @param algorithm_name The name of the used algorithm
         * @param postprocessor_names The names of the postprocessors in order of their application
         * @param output_path The path to output the JSON file at
         */
        static void write_result(const Problem_instance &instance,
                                 const std::vector<Algorithm_runner::Result> &results,
                                 const std::string &algorithm_full_name,
                                 const fs::path &output_path,
                                 const std::string &startTime,
                                 const std::string &endTime);
    };

} // cover

#endif //COVERING_RESULT_WRITER_H
