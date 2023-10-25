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

#ifndef COVERING_COVER_JOINER_H
#define COVERING_COVER_JOINER_H

#include "logging.h"

#include "cover_postprocessor.h"

namespace cover {

    /**
     * @brief Cover postprocessor which attempts to join vertically and horizontally aligned rectangles in a cover
     *
     * A postprocessor which considers combining vertically and horizontally aligned rectangles into one larger one.
     * Rectangles in the cover are assumed to be disjoint, which is the case for a partition.
     *
     * Proceeds as follows:
     * - Finds aligned rectangles
     * - Calculates joined rectangle
     * - If it is more expensive than the uncombined rectangles -> continue
     * - Otherwise, if the combined rectangle is valid -> replace the original rectangles with the new one and continue
     *
     * This process is performed first for the horizontally aligned rectangles, then for the vertically aligned
     * rectangles, since alignments can change after combining.
     */
    class Cover_joiner : public Cover_postprocessor {
    protected:
        using AlignmentMap = Map<Point, std::vector<size_t>>;

        /**
         * Calculates rectangles in the cover whose maximum and minimum y coordinates are the same, meaning the
         * top and bottom edges of the rectangles all lie on the same line.
         *
         * The returned map maps (min_y, max_y) "points" to a vector containing the indices of the rectangles
         * with the corresponding minimum and maximum y coordinates in the cover vector. The indices are sorted
         * according to the minimum x coordinate of the rectangles.
         *
         * @param cover The cover containing the rectangles to calculate the alignment of
         * @return Map of the aligned rectangles
         */
        static AlignmentMap calculate_x_alignments(const std::vector<Rectangle> &cover);

        /**
         * Calculates rectangles in the cover whose maximum and minimum x coordinates are the same, meaning the
         * left and right edges of the rectangles all lie on the same line.
         *
         * The returned map maps (min_x, max_x) "points" to a vector containing the indices of the rectangles
         * with the corresponding minimum and maximum x coordinates in the cover vector. The indices are sorted
         * according to the minimum y coordinate of the rectangles.
         *
         * @param cover The cover containing the rectangles to calculate the alignment of
         * @return Map of the aligned rectangles
         */
        static AlignmentMap calculate_y_alignments(const std::vector<Rectangle> &cover);

        /**
         * Function which calculates whether a given rectangle lies fully within the passed polygon.
         * The is_vertical_join parameter is used to speed up computation by checking only the relevant
         * half of edges of the polygon instead of all edges.
         *
         * @param polygon The polygon the rectangle should be contained in
         * @param rectangle The rectangle that should be checked for validity
         * @param is_vertical_join Whether the rectangle is the result of a vertical join or not
         * @return Whether the rectangle is fully contained in the polygon
         */
        static bool is_valid(const Polygon_with_holes &polygon, const Rectangle &rectangle,
                             bool is_vertical_join);

        /**
         * Same as the other is_valid function, but for a polygon without holes. Just used as a subroutine of
         * the other is_valid function.
         *
         * @param polygon The polygon the rectangle should be contained in
         * @param rectangle The rectangle that should be checked for validity
         * @param is_vertical_join Whether the rectangle is the result of a vertical join or not
         * @return Whether the rectangle is fully contained in the polygon
         */
        static bool is_valid(const Polygon &polygon, const Rectangle &rectangle, bool is_vertical_join);

        /**
         * Function which attempts to join a given vector of aligned rectangles.
         *
         * Considers joining each rectangle in the vector with the next one. If this reduces costs, the joined rectangle
         * is added to the cover and joining it with the next rectangle is considered, otherwise we move on to the next
         * rectangle and consider joining it with the rectangle after it and so on.
         *
         * @param polygon The polygon the cover was calculated for
         * @param cover The calculated cover for the polygon
         * @param aligned_indices The indices of the vertically or horizontally aligned rectangles
         * @param costs The costs associated with the problem instance
         * @param vertically_aligned Whether the rectangles are vertically or horizontally aligned
         * @return The set of indices of rectangles which were joined
         */
        static OrderedSet<size_t> join_aligned_entries(const Polygon_with_holes &polygon,
                                                     std::vector<Rectangle> &cover,
                                                     const std::vector<size_t> &aligned_indices,
                                                     const Problem_instance::Costs &costs,
                                                     bool vertically_aligned);

        /**
         * Function to postprocess the given cover.
         *
         * @param cover The cover to postprocess
         * @param polygon The polygon the cover was calculated for
         * @param costs The costs associated with the problem instance
         * @param covered_points Optional map of how many rectangles in the cover cover each point of the polygon
         */
        void postprocess_cover(Cover &cover, const Polygon_with_holes &polygon,
                               const Problem_instance::Costs &costs, Runtime_environment *env,
                               std::optional<Map<Point, size_t>> &covered_points) const override;

    public:
        using Cover_postprocessor::Cover_postprocessor;
    };

} // cover

#endif //COVERING_COVER_JOINER_H
