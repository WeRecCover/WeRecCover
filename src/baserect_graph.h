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

#ifndef BASERECT_GRAPH_H
#define BASERECT_GRAPH_H

#include "CGAL_classes.h"
#include "rectangle.h"
#include <limits>
#include <iterator>

namespace cover {

struct BaseRectNode {
    using PtrType = size_t;
    constexpr static PtrType NO_NEIGHBOR = std::numeric_limits<PtrType>::max();
    PtrType left {NO_NEIGHBOR};
    PtrType right {NO_NEIGHBOR};
    PtrType top {NO_NEIGHBOR};
    PtrType bottom {NO_NEIGHBOR};
    Rectangle base_rectangle;

    BaseRectNode(const Rectangle &rect) : base_rectangle(rect) {}
};

class BaseRectGraph {
public:
    using PointBaseRectMap = Map<Point, BaseRectNode::PtrType>;

  struct SuperRectangleIterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = BaseRectNode::PtrType;
    using value_type = BaseRectNode::PtrType;
    using pointer = const value_type*;
    using reference = const value_type&;

    SuperRectangleIterator(const std::vector<BaseRectNode> &nodes) : nodes(nodes) { }
    SuperRectangleIterator(const value_type &start, const Point &bottom_left, const std::vector<BaseRectNode> &nodes)
        : going_left(start), going_down(start), bottom_left(bottom_left), nodes(nodes) {}

    reference operator*() const { return going_down; }
    pointer operator->() { return &going_down; }

    // Prefix increment
    SuperRectangleIterator& operator++() {
      if (nodes[going_down].base_rectangle.get_bottom_left().y() >
              bottom_left.y()
              && nodes[going_down].bottom != BaseRectNode::NO_NEIGHBOR) {
        going_down = nodes[going_down].bottom;
      } else if (
        nodes[going_left].base_rectangle.get_bottom_left().x() > bottom_left.x()
          && nodes[going_left].left != BaseRectNode::NO_NEIGHBOR) {
        going_left = nodes[going_left].left;
        going_down = going_left;
      } else {
        going_left = BaseRectNode::NO_NEIGHBOR;
        going_down = BaseRectNode::NO_NEIGHBOR;
      }
      return *this;
    }

    // Postfix increment
    SuperRectangleIterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }

    friend bool operator== (const SuperRectangleIterator& a, const SuperRectangleIterator& b) { return a.going_down == b.going_down; };
    friend bool operator!= (const SuperRectangleIterator& a, const SuperRectangleIterator& b) { return a.going_down != b.going_down; };

    private:
        value_type going_left {BaseRectNode::NO_NEIGHBOR};
        value_type going_down {BaseRectNode::NO_NEIGHBOR};
        Point bottom_left;
        const std::vector<BaseRectNode> &nodes;
  };

    explicit BaseRectGraph() {}
    explicit BaseRectGraph(const Polygon_with_holes &polygon);
    ~BaseRectGraph() = default;

    void build(const Polygon_with_holes &polygon);
    void build(std::vector<Rectangle> base_rectangles);

    const std::vector<BaseRectNode> &getNodes() const { return nodes; }
    const PointBaseRectMap &getBottomLeftMap() const { return bottomLeft; }
    const PointBaseRectMap &getTopRightMap() const { return topRight; }

    void clear() { nodes.clear(); bottomLeft.clear(); topRight.clear(); }
    [[nodiscard]] bool empty() const { return nodes.empty(); }

    /**
     * Get an iterator for all base rectangles within the rectangle specified
     * by top_right and bottom_left.
     *
     * @param top_right The top right corner of the rectangle to iterate over
     * @param bottom_left The bottom left corner of the rectangle to iterate over
     * @return an iterator over all base rectangles within the larger rectangle
     */
    SuperRectangleIterator begin(const Point &top_right,
                                 const Point &bottom_left) {
      return SuperRectangleIterator(topRight[top_right], bottom_left, nodes);
    }

    /**
     * End iterator for all base rectangles.
     *
     * @return a past-the-end iterator
     */
    SuperRectangleIterator end() {
        return SuperRectangleIterator(nodes);
    }

    /**
     * Returns a vector containing at position i the height of node i.
     *
     * @return A vector containing the height of each node i
     */
    std::vector<size_t> get_node_heights() const;

    /**
     * Returns a vector of all possible unions of base rectangles contained in
     * the polygon which are themselves rectangles.
     *
     * @return A vector of all rectangles which are unions of base rectangles of
     * the polygon
     */
    std::vector<cover::Rectangle> get_all_rectangles() const;

    /**
     * Returns the number of all possible unions of base rectangles contained in
     * the polygon which are themselves rectangles.
     *
     * @return The number of all rectangles which are unions of base rectangles of
     * the polygon
     */
    size_t count_all_rectangles() const;

    /**
     * Returns a vector of all possible unions of base rectangles contained in
     * the rectangle specified by top right and bottom left corner.
     *
     * @return A vector of all rectangles contained in the specified rectangle
     */
    std::vector<cover::Rectangle>
    get_all_rectangles_within(const Point &top_right, const Point &bottom_left);

    /**
     * Returns a vector of all maximal unions of base rectangles contained in
     * the polygon which are themselves rectangles.
     *
     * @return A vector of all maximal rectangles within the polygon
     */
    std::vector<cover::Rectangle> get_maximal_rectangles() const;

private:
    std::vector<BaseRectNode> nodes;
    Map<Point, BaseRectNode::PtrType> bottomLeft;
    Map<Point, BaseRectNode::PtrType> topRight;
};
}

#endif // BASERECT_GRAPH_H