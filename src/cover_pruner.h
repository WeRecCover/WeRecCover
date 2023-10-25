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

#ifndef COVER_PRUNER_H
#define COVER_PRUNER_H

#include "cover_postprocessor.h"

namespace cover {

    /**
     * @brief A postprocessor which removes fully redundant rectangles from a given cover
     */
    class Cover_pruner : public Cover_postprocessor {
    protected:

        /**
         * Postprocessing function which removes fully redundant rectangles from the cover.
         *
         * @param cover The cover to remove fully redundant rectangles from
         * @param polygon The polygon associated with the problem instance
         * @param costs The costs associated with the problem instance
         * @param covered_points Map of how many rectangles in the cover cover each point of the polygon, used to
         * calculate the redundant rectangles
         */
      void postprocess_cover(
          Cover &cover, const Polygon_with_holes &polygon,
          const Problem_instance::Costs &costs, Runtime_environment *env,
          std::optional<Map<Point, size_t>> &covered_points) const override;

    public:
        using Cover_postprocessor::Cover_postprocessor;
    };
}

#endif //COVER_PRUNER_H