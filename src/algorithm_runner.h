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

#ifndef COVERING_ALGORITHM_RUNNER_H
#define COVERING_ALGORITHM_RUNNER_H

#include <map>
#include <chrono>

#include <boost/thread/thread.hpp>
#include <boost/thread/future.hpp>

#include <CGAL/Boolean_set_operations_2.h>

#include "CGAL_classes.h"
#include "algorithm.h"
#include "instance.h"
#include "rectangle.h"
#include "cover_provider.h"
#include "runtime_environment.h"

namespace cover {
    /**
     * @brief A class which can be used to run an Algorithm on a problem instance
     *
     * A class which can be used to run an Algorithm on a provided problem instance and returns the results,
     * including the cover calculated by the algorithm, as well as its cover's cost and execution time.
     */
    class Algorithm_runner {
        using clock = std::chrono::high_resolution_clock;
        using nanos = std::chrono::nanoseconds;

    public:
        using Cover = std::vector<Rectangle>;

        /**
         * @brief Struct representing the result of running an algorithm on a problem instance
         *
         * Contains the calculated cover, total creation cost, total area cost, execution time and whether
         * the calculated polygon is valid, if verification was turned on.
         */
        struct Result {
            size_t cover_size;
            Problem_instance::Costs cost;
            nanos execution_time;
            enum Validity { INVALID = 0, VALID = 1, UNCHECKED = 2, TIMEOUT = 3 }
                is_valid { UNCHECKED };
            Cover cover;
        };

        /**
         * Runs the given Algorithm on a single provided Problem_instance. The results are returned in a
         * tuple containing:
         *
         * - a vector of the Rectangle objects returned by the algorithm as a cover for the corresponding instance
         * - a value representing the cost of the cover returned by the algorithm
         * - the execution time of the algorithm on the instance in milliseconds
         *
         * An optional boolean can be passed to specify whether the cover returned by the algorithm should be checked
         * for correctness. Meaning that the union of the rectangles returned by the algorithm is exactly equal to the
         * problem instance's underlying MultiPolygon. By default, verification will be performed.
         *
         * @param algorithm The algorithm to evaluate on the problem instance
         * @param instance The problem instance to run the algorithm on
         * @param verify Whether to check the correctness of the cover returned by the algorithm, default is true
         * @return The result of running the algorithm on the provided problem instance
         */
        static std::vector<Result>
        run_algorithm(Cover_provider &algorithm,
                      const Problem_instance &instance,
                      bool verify = true);

        /**
         * Returns whether the provided vector of Rectangle objects is a valid cover of the provided MultiPolygon.
         * To be a valid cover the union of the provided rectangles must be *exactly* equal to the
         * provided polygon.
         *
         * This is verified by joining all touching rectangles into individual, disjoint polygons. Then the symmetric
         * difference of the problem instance's MultiPolygon and these rectangle polygons is computed. If they are
         * identical, the result will be empty, and true will be returned. Otherwise, false is returned, since there
         * are either areas of the problem instance's polygon that aren't covered or there are rectangles that cross
         * the boundary of the problem instance's polygon, which is also not allowed.
         *
         * @param rectangles The set of rectangles constituting a potential cover
         * @param polygon The polygon to cover
         * @return Whether the rectangles are a valid cover of the MultiPolygon or not
         */
        static bool is_valid_cover(const Cover &rectangles, const Polygon_with_holes &polygon);

        static bool verify_cover(const Cover &rectangles, const Polygon_with_holes &polygon);
    };
}

namespace std {
    std::ostream &operator<<(std::ostream &out, const cover::Algorithm_runner::Result::Validity &status);
}

#endif //COVERING_ALGORITHM_RUNNER_H
