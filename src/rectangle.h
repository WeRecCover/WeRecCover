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

#ifndef COVERING_RECTANGLE_H
#define COVERING_RECTANGLE_H

#include <boost/functional/hash.hpp>

#include "CGAL_classes.h"
#include "datastructures.h"

namespace cover {
    /**
     * @brief A simple rectangle class
     *
     * A simple rectangle wrapper around CGAL's Polygon_2 class. May look into wrapping/using CGAL::Iso_rectangle
     * instead later.
     */
    class Rectangle {

    protected:
        Point bottom_left, top_right;

    public:
        /**
         * Constructor taking smallest and largest X coordinate, as well as smallest and largest Y coordinate.
         *
         * @param min_x X coordinate of left side of rectangle
         * @param min_y Y coordinate of bottom of rectangle
         * @param max_x X coordinate of right side of rectangle
         * @param max_y Y coordinate of top of rectangle
         */
        Rectangle(const NumType &min_x, const NumType &min_y, const NumType &max_x, const NumType &max_y);

        /**
         * Copy constructor.
         *
         * @param other The rectangle to copy
         */
        Rectangle(const Rectangle &other);

        /**
         * Create a unit rectangle with the passed point as its top left corner.
         *
         * @param top_left The top left corner of the rectangle
         */
        explicit Rectangle(const Point &top_left);

        /**
         * Create a rectangle with the passed points as its bottom left 
         * and top right corner.
         *
         * @param bottom_left The bottom left corner of the rectangle
         * @param top_right The top right corner of the rectangle
         */
        Rectangle(const Point &bottom_left, const Point &top_right)
            : bottom_left(bottom_left), top_right(top_right) { }

        [[nodiscard]]
        NumType width() const;

        [[nodiscard]]
        NumType height() const;

        /**
         * Moves the bottom edge of the rectangle down by amount.
         *
         * @param amount Amount to move the bottom edge down by
         */
        void extend_down(const NumType &amount);

        /**
         * Moves the left edge of the rectangle left by amount.
         *
         * @param amount Amount to move the left edge left by
         */
        void extend_left(const NumType &amount);

        /**
         * Moves the right edge of the rectangle right by amount.
         *
         * @param amount Amount to move the right edge right by
         */
        void extend_right(const NumType &amount);

        /**
         * Moves the top edge of the rectangle up by amount.
         *
         * @param amount Amount to move the top edge up by
         */
        void shrink_up(const NumType &amount);

        /**
         * Moves the top edge of the rectangle down by amount.
         *
         * @param amount Amount to move the top edge down by
         */
        void shrink_down(const NumType &amount);

        /**
         * Moves the right edge of the rectangle left by amount.
         *
         * @param amount Amount to move the right edge left by
         */
        void shrink_left(const NumType &amount);

        /**
         * Moves the left edge of the rectangle right by amount.
         *
         * @param amount Amount to move the left edge right by
         */
        void shrink_right(const NumType &amount);

        /**
         * @return The bottom left vertex of the rectangle
         */
        [[nodiscard]] Point get_bottom_left() const { return bottom_left; }

        /**
         * @return The top right vertex of the rectangle
         */
        [[nodiscard]] Point get_top_right() const { return top_right; }

        /**
         * @return The bottom right vertex of the rectangle
         */
        [[nodiscard]] Point get_bottom_right() const;

        /**
         * @return The top left vertex of the rectangle
         */
        [[nodiscard]] Point get_top_left() const;

        /**
         * @return The bottom edge of the rectangle
         */
        [[nodiscard]] Segment get_bottom_edge() const;

        /**
         * @return The left edge of the rectangle
         */
        [[nodiscard]] Segment get_left_edge() const;

        /**
         * @return The right edge of the rectangle
         */
        [[nodiscard]] Segment get_right_edge() const;

        /**
         * @return The top edge of the rectangle
         */
        [[nodiscard]] Segment get_top_edge() const;

        /**
         * @return The y coordinate of the bottom edge of the rectangle
         */
        [[nodiscard]] NumType get_min_y() const;

        /**
         * @return The x coordinate of the left edge of the rectangle
         */
        [[nodiscard]] NumType get_min_x() const;

        /**
         * @return The y coordinate of the top edge of the rectangle
         */
        [[nodiscard]] NumType get_max_y() const;

        /**
         * @return The x coordinate of the right edge of the rectangle
         */
        [[nodiscard]] NumType get_max_x() const;

        /**
         * @return The area of the rectangle
         */
        [[nodiscard]] inline size_t area() const {
            return (top_right.x() - bottom_left.x()) * (top_right.y() - bottom_left.y());
        }

        /**
         * @return The integer coordinate points lying inside the rectangle
         */
        [[nodiscard]] Set<Point> get_covered_points() const;

        /**
         * @return The rectangle as a CGAL polygon
         */
        [[nodiscard]] Polygon as_polygon() const;

        /**
         * Returns whether this rectangle fully contains the other rectangle.
         *
         * @param other The rectangle which this rectangle may fully contain
         * @return Whether this rectangle fully contains the other rectangle
         */
        [[nodiscard]] inline bool fully_contains(const Rectangle &other) const {
            return bottom_left.x() <= other.bottom_left.x() && bottom_left.y() <= other.bottom_left.y() &&
                   top_right.x() >= other.top_right.x() && top_right.y() >= other.top_right.y();
        }

        /**
         * Returns whether this rectangle and the other rectangle intersect in any way.
         *
         * @param other The rectangle to check intersection with
         * @return Whether the this rectangle intersects the other rectangle in any way
         */
        [[nodiscard]] bool intersects(const Rectangle &other) const;

        /**
         * Returns whether the passed segment intersects any part of the rectangle which is part of its boundary.
         *
         * @param segment The segment to check for intersection
         * @return Whether the segment intersects any part of this rectangle which is not part of its boundary
         */
        [[nodiscard]] bool fully_intersects(const Segment &segment) const;

        /**
         * Combines this rectangle and the other rectangle into a new rectangle which contains both of them and is
         * as small as possible.
         *
         * @param other The rectangle to combine with
         * @return The combined rectangle
         */
        [[nodiscard]] Rectangle join(const Rectangle &other) const {
          const auto min_x{std::min(get_min_x(), other.get_min_x())};
          const auto min_y{std::min(get_min_y(), other.get_min_y())};
          const auto max_x{std::max(get_max_x(), other.get_max_x())};
          const auto max_y{std::max(get_max_y(), other.get_max_y())};

          return {min_x, min_y, max_x, max_y};
        }

        bool operator==(const Rectangle &other) const {
            return top_right == other.top_right && bottom_left == other.bottom_left;
        }

        bool operator<(const Rectangle &other) const {
          return get_bottom_left() < other.get_bottom_left() ||
                 get_bottom_left() == other.get_bottom_left() &&
                     get_top_right() < other.get_top_right();
        }
    };

} // cover

namespace std {
    template<>
    struct hash<cover::Rectangle> {
        size_t operator()(const cover::Rectangle &rectangle) const {
            size_t seed{0};
            boost::hash_combine(seed, rectangle.get_bottom_right());
            boost::hash_combine(seed, rectangle.get_top_left());
            return seed;
        }
    };

    std::ostream &operator<<(std::ostream &, const cover::Rectangle &);
}  // std

#endif //COVERING_RECTANGLE_H
