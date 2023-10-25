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

#include "baserect_graph.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include "rectangle_enumerator.h"

namespace cover {

void BaseRectGraph::build(const Polygon_with_holes &polygon) {
    build(Rectangle_enumerator::get_base_rectangles(polygon));
}

void BaseRectGraph::build(std::vector<Rectangle> base_rectangles) {

    LOG(info) << "Building base rect graph with " << base_rectangles.size() << " node(s)...";

    clear();
    nodes.reserve(base_rectangles.size());


    assert(base_rectangles.size() > 1);

    std::sort(base_rectangles.begin(), base_rectangles.end(),
            [](const auto &first, const auto &second) {
        auto tl1 = first.get_top_left();
        auto tl2 = second.get_top_left();
        return (tl1.x() < tl2.x()
            || tl1.x() == tl2.x() && tl1.y() > tl2.y());
            });

    for (const auto &rectangle : base_rectangles) {
        const auto id = nodes.size();
        nodes.emplace_back(rectangle);
        auto &node = nodes.back();
        auto tl = rectangle.get_top_left();
        auto left = topRight.find(tl);
        if (left != topRight.end()) {
            node.left = left->second;
            nodes[node.left].right = id;
        }
        auto top = bottomLeft.find(tl);
        if (top != bottomLeft.end()) {
            node.top = top->second;
            nodes[node.top].bottom = id;
        }
        bottomLeft[rectangle.get_bottom_left()] = id;
        topRight[rectangle.get_top_right()] = id;
    }
    LOG(info) << "Base rect graph has been built.";
}

BaseRectGraph::BaseRectGraph(const Polygon_with_holes &polygon) {
  build(polygon);
}

std::vector<size_t> BaseRectGraph::get_node_heights() const {
  std::vector<size_t> heights(nodes.size(), 0);
  for (size_t i = 0; i < nodes.size(); i++) {
    if (nodes[i].bottom != BaseRectNode::NO_NEIGHBOR) {
      continue;
    }
    // height of this node is set to 0 already
    assert(heights[i] == 0);
    auto top = nodes[i].top;
    size_t h{0};
    while (top != BaseRectNode::NO_NEIGHBOR) {
      h++;
      heights[top] = h;
      top = nodes[top].top;
    }
  }
  return heights;
}

std::vector<cover::Rectangle> BaseRectGraph::get_all_rectangles() const {
    std::vector<Rectangle> rectangles;
    const auto &heights {get_node_heights()};
    for (size_t i = 0; i < nodes.size(); i++) {
        const auto &node = nodes[i];
        LOG(trace) << "Enumerating rectangles with top right base rectangle " << node.base_rectangle;
        const auto tr {node.base_rectangle.get_top_right()};
        auto max_height = heights[i];
        auto left {i};
        while (left != BaseRectNode::NO_NEIGHBOR)  {
            max_height = std::min(heights[left], max_height);
            LOG(trace) << "  Going left. Max height is now " << max_height;
            auto down {left};
            for (size_t h = 0; h <= max_height; h++) {
                rectangles.push_back({nodes[down].base_rectangle.get_bottom_left(), tr});
                LOG(trace) << "  Added rectangle " << rectangles.back();
                down = nodes[down].bottom;
            }
            left = nodes[left].left;
       }
    }
    return rectangles;
}

size_t BaseRectGraph::count_all_rectangles() const {
    size_t rectangle_count {0};
    const auto &heights {get_node_heights()};
    for (size_t i = 0; i < nodes.size(); i++) {
        const auto &node = nodes[i];
        const auto tr {node.base_rectangle.get_top_right()};
        auto max_height = heights[i];
        auto left {i};
        while (left != BaseRectNode::NO_NEIGHBOR)  {
            max_height = std::min(heights[left], max_height);
            rectangle_count += max_height + 1;
            left = nodes[left].left;
       }
    }
    return rectangle_count;
}


std::vector<cover::Rectangle>
BaseRectGraph::get_all_rectangles_within(const Point &top_right,
                                         const Point &bottom_left) {

  LOG(debug) << "Computing rectangles within " << bottom_left << " / " << top_right;
  std::vector<Rectangle> rectangles;
  for (auto it = begin(top_right, bottom_left); it != end(); ++it) {
    const auto tr {nodes[*it].base_rectangle.get_top_right()};
    for (auto jt = it; jt != end(); ++jt) {
        const auto bl {nodes[*jt]};
        if (bl.base_rectangle.get_bottom_left().y() < tr.y() ) {
            rectangles.push_back({bl.base_rectangle.get_bottom_left(), tr});
            LOG(debug) << "Found contained rectangle " << rectangles.back();
        }
    }
  }
  return rectangles;
}

std::vector<cover::Rectangle> BaseRectGraph::get_maximal_rectangles() const {
  Set<Rectangle> rectangles;
  const auto &heights{get_node_heights()};
  for (size_t i = 0; i < nodes.size(); i++) {
    if (nodes[i].top != BaseRectNode::NO_NEIGHBOR) {
      continue;
    }

    LOG(trace) << "Node " << i << " with height " << heights[i]
               << " has no top neighbor.\n";
    for (size_t h = 0; h <= heights[i]; h++) {
      auto left{i};
      auto right{i};
      auto min_height {heights[i]};
      while (nodes[left].left != BaseRectNode::NO_NEIGHBOR &&
             heights[nodes[left].left] >= h) {
        min_height = std::min(min_height, heights[nodes[left].left]);
        left = nodes[left].left;
      }
      while (nodes[right].right != BaseRectNode::NO_NEIGHBOR &&
             heights[nodes[right].right] >= h) {
        min_height = std::min(min_height, heights[nodes[right].right]);
        //if (heights[nodes[right].right] == h) {
        //  heightLimited = true;
        //}
        right = nodes[right].right;
      }
        if (min_height == h) {
        // cannot expand in either direction
        auto bottomLeft{left};
        for (size_t j = 0; j < h; j++) {
          assert(bottomLeft != BaseRectNode::NO_NEIGHBOR);
          bottomLeft = nodes[bottomLeft].bottom;
        }
        const Rectangle rect{nodes[bottomLeft].base_rectangle.get_bottom_left(),
                             nodes[right].base_rectangle.get_top_right()};

        LOG(debug) << "Found maximal rectangle " << rect;
        rectangles.insert(rect);
      }
    }
  }
  return { rectangles.begin(), rectangles.end() };
}

} // cover