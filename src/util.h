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

#ifndef COVERING_UTIL_H
#define COVERING_UTIL_H

#include "logging.h"

#include "CGAL_classes.h"
#include "rectangle.h"
#include "datastructures.h"

namespace cover {

    class Util {
    public:
        using ConcaveMap = Map<Point, std::array<Direction, 2>>;
        using ConcaveMapEntry = ConcaveMap::value_type;

        /**
         * @param direction The direction object to normalize
         * @return The normalized direction object
         */
        static Direction normalize(const Direction &direction);

        /**
         * @param point The point to check for
         * @param polygon The polygon to check for
         * @return Whether the given pointl ies on any edge of the polygon
         */
        static bool has_on_any_edge(const Point &point, const Polygon_with_holes &polygon);

        /**
         * @param point The point to check for
         * @param polygon The polygon to check for
         * @return Whether the given pointl ies on any edge of the polygon
         */
        static bool has_on_any_edge(const Point &point, const Polygon &polygon);

        /**
         * @param ray The ray to intersect
         * @param segment The segment to intersect
         * @return The point the two intersect at if they intersect at a single point or nullopt otherwise
         */
        static std::optional<Point> get_point_intersection(const Ray &ray, const Segment &segment);

        /**
         * Returns the concave vertices in the polygon and the directions opposite of the two edges that the vertex is
         * an endpoint of.
         *
         * @param polygon The polygon containing the concave vertices
         * @return A map mapping each concave vertex to the two directions opposite of its two edges
         */
        static ConcaveMap find_concave_vertices(const Polygon_with_holes &polygon);

        /**
         * Returns the concave vertices in the polygon and the directions opposite of the two edges that the vertex is
         * an endpoint of.
         *
         * @param polygon The polygon containing the concave vertices
         * @return A map mapping each concave vertex to the two directions opposite of its two edges
         */
        static ConcaveMap find_concave_vertices(const Polygon &polygon);

        /**
         * Returns a pair of the concave vertex and the two opposite directions of its edges if the two segments
         * form a concave vertex or nullopt if they do not.
         *
         * @param edge The first edge
         * @param next_edge The next edge
         * @return The pair of the concave vertex and the opposite directions of its two edges if the edges form a
         * concave vertex
         */
        static std::optional<ConcaveMapEntry> determine_vertex_entry(const Segment &edge, const Segment &next_edge);

        /**
         * @param direction The direction object to rotate
         * @return The direction object rotated by 90 degrees
         */
        static inline Direction rotate_90_degrees(const Direction &direction) {
            return {-direction.dy(), direction.dx()};
        };

        /**
         * @param direction The direction object to rotate
         * @return The direction object rotated by 180 degrees
         */
        static inline Direction rotate_180_degrees(const Direction &direction) {
            return {-direction.dx(), -direction.dy()};
        };

        /**
         * @param direction The direction object to rotate
         * @return The direction object rotated by 270 degrees
         */
        static inline Direction rotate_270_degrees(const Direction &direction) {
            return {direction.dy(), -direction.dx()};
        };

        /**
         * Returns the closest intersection of the ray and any edge of the polygon in the direction of the ray if they
         * do intersect, or nullopt otherwise.
         *
         * @param ray The ray to intersect
         * @param polygon The polygon to intersect
         * @return The closest intersection of the ray and the polygon if they do intersect, nullopt otherwise
         */
        static std::optional<Point> get_closest_intersection(const Ray &ray, const Polygon_with_holes &polygon);

        /**
         * Returns the closest intersection of the ray and any edge of the polygon in the direction of the ray if they
         * do intersect, or nullopt otherwise.
         *
         * @param ray The ray to intersect
         * @param polygon The polygon to intersect
         * @return The closest intersection of the ray and the polygon if they do intersect, nullopt otherwise
         */
        static std::optional<Point> get_closest_intersection(const Ray &ray, const Polygon &polygon);

        /**
         * Creates a CGAL arrangement of the polygon's edges and the segments in the vector of cuts.
         *
         * @param polygon The polygon to create the arrangement from
         * @param cuts The cuts to create the arrangement from
         * @return The constructed arrangement
         */
        static Arrangement create_arrangement(const Polygon_with_holes &polygon, const std::vector<Segment> &cuts);

        /**
         * Parses the rectangles contained in the arrangement and returns them as a vector of rectangles.
         *
         * Since CGAl arrangements seemingly have no notion of holes, we need the original polygon to check whether some
         * parts of the arrangement may be holes rather than rectangles we should be parsing out.
         *
         * @param arrangement The arrangement containing the rectangles
         * @param polygon The polygon the arrangement was created from
         * @return The rectangles contained in the arrangement which are not holes
         */
        static std::vector<Rectangle> parse_rectangles(const Arrangement &arrangement,
                                                       const Polygon_with_holes &polygon);
    };

} // cover

#endif //COVERING_UTIL_H
