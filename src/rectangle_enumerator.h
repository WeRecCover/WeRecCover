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

#ifndef COVERING_RECTANGLE_ENUMERATOR_H
#define COVERING_RECTANGLE_ENUMERATOR_H

#include <CGAL/Boolean_set_operations_2.h>

#include "CGAL_classes.h"
#include "rectangle.h"
#include "util.h"
#include "datastructures.h"

namespace cover {

    /**
     * @brief Class to enumerate all potentially relevant rectangles that fit inside a given polygon
     *
     * First finds the "base rectangles" of the polygon, which are the rectangles we get when we draw two rays
     * from each concave vertex of the polygon into its interior until it hits one of the polygon's edges which the
     * concave vertex is not an endpoint of. Then, all possible rectangular combinations of these base rectangles are
     * determined, these are then all potentially relevant rectangles which fit inside the polygon.
     */
    class Rectangle_enumerator {
    protected:
        enum class Side {
            TOP,
            RIGHT,
            BOTTOM,
            LEFT
        };

        using NeighborMap = Map<Segment, Rectangle>;
        using NeighborSideMap = Map<Side, NeighborMap>;

        /**
         * Picks cuts from the concave vertex into both of its directions facing into the interior of the polygon until
         * the cut intersects an edge of the polygon which the concave vertex is not an endpoint of.
         *
         * @param polygon The polygon to pick the cuts in
         * @param concave_entry The concave to create the cuts from
         * @param cuts The vector to add the cuts to
         */
        static void pick_cuts(const Polygon_with_holes &polygon, const Util::ConcaveMapEntry &concave_entry,
                              std::vector<Segment> &cuts);

        /**
         * Creates a map of maps which maps either Side::TOP or Side::LEFT to a map which maps a segment to a
         * rectangle. If the top-level key is Side::TOP, the segment is the top edge of the corresponding rectangle,
         * otherwise, it is the left edge of the corresponding rectangle.
         *
         * This map can be used to find the neighbor of a rectangle on its bottom or right side quickly, if one exists.
         *
         * Note that for this to work, the rectangles must have exactly one bottom or right neighbor, meaning both
         * rectangles must share the corresponding edge, which is the case for base rectangles.
         *
         * @param rectangles The rectangles to populate the maps with
         * @return The map of maps which can be used to determine a rectangle's neighbors to the right and the bottom
         */
        static NeighborSideMap build_neighbor_side_map(const std::vector<Rectangle> &rectangles);

        /**
         * Visits a base rectangle, adding all rectangles which have this base rectangle in their top-left corner and
         * are entirely made up of base rectangles to the vector of seen rectangles.
         *
         * @param base_rectangle The base rectangle to visit
         * @param neighbors The map of neighbors of the base rectangles
         * @param seen The vector of seen rectangles
         */
        static void visit(const Rectangle &base_rectangle, const NeighborSideMap &neighbors,
                          std::vector<Rectangle> &seen);

        /**
         * Returns the right neighbor of a rectangle or nullopt if it has no right neighbor.
         *
         * @param rectangle The rectangle to retrieve the right neighbor of
         * @param neighbors The map of neighbors of rectangles
         * @return The right neighbor or nullopt
         */
        static std::optional<Rectangle> get_right_neighbor(const Rectangle &rectangle,
                                                           const NeighborSideMap &neighbors);

        /**
         * Returns the bottom neighbor of a rectangle or nullopt if it has no bottom neighbor.
         *
         * @param rectangle The rectangle to retrieve the bottom neighbor of
         * @param neighbors The map of neighbors of rectangles
         * @return The bottom neighbor or nullopt
         */
        static std::optional<Rectangle> get_bottom_neighbor(const Rectangle &rectangle,
                                                            const NeighborSideMap &neighbors);

    public:
        /**
         * Returns a vector containing the base rectangles of the polygon.
         *
         * @param polygon The polygon to calculate the base rectangles of
         * @return The vector of base rectangles contained in the polygon
         */
        static std::vector<Rectangle> get_base_rectangles(const Polygon_with_holes &polygon);

        /**
         * Returns a vector of all possible unions of base rectangles which are themselves rectangles.
         *
         * @param base_rectangles The base rectangles to calculate the unions of
         * @return A vector of all rectangles which are unions of base rectangles
         */
        static std::vector<Rectangle> enumerate_rectangles(const std::vector<Rectangle> &base_rectangles);

        /**
         * Returns a vector of all possible unions of base rectangles contained in the polygon which are
         * themselves rectangles.
         *
         * @param polygon The polygon to use the base rectangles of
         * @return A vector of all rectangles wich are unions of base rectangles of the given polygon
         */
        static std::vector<Rectangle> enumerate_rectangles(const Polygon_with_holes &polygon);
    };

} // cover

#endif //COVERING_RECTANGLE_ENUMERATOR_H
