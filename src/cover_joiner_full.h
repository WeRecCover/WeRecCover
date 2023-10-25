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

#ifndef COVERING_COVER_JOINER_FULL_H
#define COVERING_COVER_JOINER_FULL_H

#include "logging.h"

#include "cover_postprocessor.h"

namespace cover {

    /**
     * @brief Cover postprocessor which attempts to join rectangles in a cover
     *
     * In contrast to the regular Cover_joiner class, this class considers joining any two rectangles in the cover
     * which can be joined. It does this by checking for every rectangle what the cost reduction of joining them
     * would be, if it is larger than the current cost reduction, a check is performed to see if the joined rectangle
     * is valid, if it is, the best cost reduction is updated. At the end of such an iteration, we join the two
     * rectangles giving the best cost reduction, then carry on trying to join the next rectangle until there are no
     * more joins to be considered.
     */
    class Cover_joiner_full : public Cover_postprocessor {
    protected:
        /**
         * Function which calculates whether a given rectangle lies fully within the passed polygon.
         *
         * @param polygon The polygon the rectangle should be contained in
         * @param rectangle The rectangle that should be checked for validity
         * @return Whether the rectangle is fully contained in the polygon
         */
        static bool is_valid(const Polygon_with_holes &polygon, const Rectangle &rectangle);

        /**
         * Same as the other is_valid, but for a polygon without holes. This is just called by the other is_valid
         * as a subroutine.
         *
         * @param polygon The polygon the rectangle should be contained in
         * @param rectangle The rectangle that should be checked for validity
         * @return Whether the rectangle is fully contained in the polygon
         */
        static bool is_valid(const Polygon &polygon, const Rectangle &rectangle);

        /**
         * Checks whether joining first and second gives a better cost reduction than the current one, if this is the
         * case and the joined rectangle is valid, a pair of the joined rectangle and the new cost reduction is
         * returned.
         *
         * @param first First rectangle to be joined
         * @param second Second rectangle to be joined
         * @param polygon The polygon the join is taking place in
         * @param costs The costs associated with the problem instance
         * @param current_best_cost_reduction The cost reduction to compare ours to
         * @return A pair of joined rectangle and cost reduction if the cost reduction is better and the rectangle
         * is valid, nullopt otherwise
         */
        static std::optional<std::pair<Rectangle, CostType>> try_join_rectangles(
                const Rectangle &first, const Rectangle &second,
                const Polygon_with_holes &polygon,
                const Problem_instance::Costs &costs,
                std::optional<CostType> current_best_cost_reduction);

        /**
         * Function to postprocess the given cover.
         *
         * @param cover The cover to postprocess
         * @param polygon The polygon the cover was calculated for
         * @param costs The costs associated with the problem instance
         * @param covered_points Optional map of how many rectangles in the cover cover each point of the polygon
         */
        void postprocess_cover(
            Cover &cover, const Polygon_with_holes &polygon,
            const Problem_instance::Costs &costs, Runtime_environment *env,
            std::optional<Map<Point, size_t>> &covered_points) const override;

      public:
        using Cover_postprocessor::Cover_postprocessor;
    };

} // cover

#endif //COVERING_COVER_JOINER_FULL_H
