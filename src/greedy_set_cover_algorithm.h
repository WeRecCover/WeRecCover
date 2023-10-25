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

#ifndef COVERING_GREEDY_SET_COVER_ALGORITHM_H
#define COVERING_GREEDY_SET_COVER_ALGORITHM_H

#include <CGAL/Polygon_set_2.h>

#include "algorithm.h"
#include "rectangle_enumerator.h"
#include "algorithm_runner.h"

namespace cover {
    /**
     * @brief Algorithm which calculates a cover via the greedy weighted set cover algorithm
     */
    class Greedy_set_cover_algorithm : public Algorithm {
    protected:
        struct QueueEntry;

        /**
         * Calculates a cover for the provided polygon and costs using the greedy set cover algorithm.
         *
         * First creates a queue of all possible rectangles, then repeatedly picks the rectangle which
         * minimizes cost_per_unit until all base rectangles have been covered.
         *
         * @param polygon The polygon to cover
         * @param costs The costs associated with the problem instance
         * @return A cover of the polygon
         */
        [[nodiscard]] std::vector<Rectangle>
        calculate_cover(const Polygon_with_holes &polygon,
                        const Problem_instance::Costs &costs,
                        Runtime_environment *env) override;

    };

} // cover


#endif //COVERING_GREEDY_SET_COVER_ALGORITHM_H
