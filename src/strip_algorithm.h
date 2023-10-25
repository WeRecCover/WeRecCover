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

#ifndef BASE_RECT_STRIP_ALGORITHM_H
#define BASE_RECT_STRIP_ALGORITHM_H

#include "CGAL_classes.h"
#include "algorithm.h"

namespace cover {

    /**
     * @brief Computes a cover for a polygon based on extending maximal rectangles
     *
     * The algorithm is an adaptation of the algorithm by Kumar and Ramesh to the
     * base rectangle graph.
     */
    class Strip_algorithm : public Algorithm {

        /**
         * Calculates a cover for the give polygon by calling process_edge for every edge of the polygon which
         * has either a hole or the polygon's exterior above it.
         *
         * @param polygon The polygon to calculate the cover for
         * @param costs The costs associated with the problem instance
         * @return The cover calculated by the algorithm
         */
        [[nodiscard]] std::vector<Rectangle>
        calculate_cover(const Polygon_with_holes &polygon,
                        const Problem_instance::Costs &costs,
                        Runtime_environment *env) override;
    };

}

#endif //BASE_RECT_STRIP_ALGORITHM_H