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

#include "rectangle.h"

namespace cover {
    Rectangle::Rectangle(const NumType &min_x, const NumType &min_y, const NumType &max_x, const NumType &max_y) {
        if (min_x >= max_x || min_y >= max_y) {
            throw std::runtime_error("Rectangle has invalid min/max coordinates: " + std::to_string(min_x) + ' '
                                     + std::to_string(min_y) + ' ' + std::to_string(max_x) + ' ' +
                                     std::to_string(max_y));
        }

        bottom_left = Point(min_x, min_y);
        top_right = Point(max_x, max_y);

        assert(bottom_left != top_right);
    }

    Rectangle::Rectangle(const cover::Rectangle &other)
            : Rectangle(other.bottom_left.x(), other.bottom_left.y(), other.top_right.x(), other.top_right.y()) {};

    Point Rectangle::get_bottom_right() const {
        return {top_right.x(), bottom_left.y()};
    }

    NumType Rectangle::width() const {
        return top_right.x() - bottom_left.x();
    }

    NumType Rectangle::height() const {
        return top_right.y() - bottom_left.y();
    }

    Point Rectangle::get_top_left() const {
        return {bottom_left.x(), top_right.y()};
    }

    void Rectangle::extend_down(const NumType &amount) {
        bottom_left -= {0, amount};
    }

    void Rectangle::extend_left(const NumType &amount) {
        bottom_left -= {amount, 0};
    }

    void Rectangle::extend_right(const NumType &amount) {
        top_right += {amount, 0};
    }

    void Rectangle::shrink_up(const NumType &amount) {
        bottom_left += {0, amount};
    }

    void Rectangle::shrink_down(const NumType &amount) {
        top_right -= {0, amount};
    }

    void Rectangle::shrink_left(const NumType &amount) {
        bottom_left += {amount, 0};
    }

    void Rectangle::shrink_right(const NumType &amount) {
        top_right -= {amount, 0};
    }

    Segment Rectangle::get_left_edge() const {
        return {get_top_left(), bottom_left};
    }

    Segment Rectangle::get_bottom_edge() const {
        return {bottom_left, get_bottom_right()};
    }

    Segment Rectangle::get_right_edge() const {
        return {get_bottom_right(), top_right};
    }

    Segment Rectangle::get_top_edge() const {
        return {top_right, get_top_left()};
    }

    NumType Rectangle::get_min_x() const {
        return bottom_left.x();
    }

    NumType Rectangle::get_min_y() const {
        return bottom_left.y();
    }

    NumType Rectangle::get_max_x() const {
        return top_right.x();
    }

    NumType Rectangle::get_max_y() const {
        return top_right.y();
    }

    Rectangle::Rectangle(const Point &top_left) {
        NumType left_x{top_left.x()}, right_x{left_x + 1};
        NumType top_y{top_left.y()}, bottom_y{top_y - 1};

        top_right = {right_x, top_y};
        bottom_left = {left_x, bottom_y};

        assert(top_right != bottom_left);
    }

    Set<Point> Rectangle::get_covered_points() const {
        Set<Point> covered_points{};

        for (auto row{get_min_y()}; row != get_max_y(); ++row) {
            for (auto column{get_min_x()}; column != get_max_x(); ++column) {
                covered_points.insert(Point(column, row));
            }
        }

        return covered_points;
    }

    Polygon Rectangle::as_polygon() const {
        Polygon polygon{};

        assert(bottom_left != top_right);

        polygon.push_back(bottom_left);
        polygon.push_back(get_bottom_right());
        polygon.push_back(top_right);
        polygon.push_back(get_top_left());

        return polygon;
    }

    bool Rectangle::intersects(const Rectangle &other) const {
        if (other.top_right.x() <= bottom_left.x() || top_right.x() <= other.bottom_left.x()) {
            // rectangles are horizontally disjoint
            return false;
        }

        if (other.top_right.y() <= bottom_left.y() || top_right.y() <= other.bottom_left.y()) {
            // rectangles are vertically disjoint
            return false;
        }

        return true;
    }

    bool Rectangle::fully_intersects(const Segment &segment) const {
        bool vertical{segment.is_vertical()};

        if (vertical) {
            const auto segment_x{segment.target().x()};
            if (segment_x >= get_max_x() || segment_x <= get_min_x()) {
                return false;
            }
            const auto segment_y_1{segment.target().y()};
            const auto segment_y_2{segment.source().y()};
            return !((segment_y_1 >= get_max_y() && segment_y_2 >= get_max_y()) ||
                     (segment_y_1 <= get_min_y() && segment_y_2 <= get_min_y()));
        } else {
            const auto segment_y{segment.target().y()};
            if (segment_y >= get_max_y() || segment_y <= get_min_y()) {
                return false;
            }
            const auto segment_x_1{segment.target().x()};
            const auto segment_x_2{segment.source().x()};
            return !((segment_x_1 >= get_max_x() && segment_x_2 >= get_max_x()) ||
                     (segment_x_1 <= get_min_x() && segment_x_2 <= get_min_x()));
        }
    }
} // cover

namespace std {
    std::ostream &operator<<(std::ostream &out, const cover::Rectangle &r) {
        out << "[ " << r.get_bottom_left() << " / " << r.get_top_right() << " ]";
        return out;
    }
}