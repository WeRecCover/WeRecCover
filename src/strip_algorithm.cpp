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

#include "strip_algorithm.h"
#include "baserect_graph.h"
#include "datastructures.h"
#include "rectangle_enumerator.h"
#include "logging.h"

namespace cover {

std::vector<Rectangle> Strip_algorithm::calculate_cover(
    const Polygon_with_holes &polygon,
    const Problem_instance::Costs &costs,
    Runtime_environment *env) {

        if (env->graph.empty()) {
            if (env->base_rectangles.empty()) {
                env->base_rectangles = Rectangle_enumerator::get_base_rectangles(polygon);
            }
            env->graph.build(env->base_rectangles);
        }
        const auto &nodes{env->graph.getNodes()};

        assert(nodes.size() > 1);
        //if (nodes.size() == 1) {
        //    return std::vector<Rectangle>{nodes.front().base_rectangle};
        //}

        // compute height of each node as the length of a longest bottom-going path
        std::vector<size_t> heights(nodes.size(), 0);
        for (size_t i = 0; i < nodes.size(); i++) {
            if (nodes[i].bottom != BaseRectNode::NO_NEIGHBOR) {
                continue;
            }
            LOG(debug) << "Node " << i << " with height " << heights[i] << " has no bottom neighbor.\n";
            // height of this node is set to 0 already
            assert(heights[i] == 0);
            auto top = nodes[i].top;
            size_t h{0};
            while (top != BaseRectNode::NO_NEIGHBOR) {
                h++;
                heights[top] = h;
                LOG(trace) << "Setting height of node " << top << " to " << h << ".\n";
                top = nodes[top].top;
            }
        }

        // now compute strips
        Set<Rectangle> cover;
        //std::vector<Rectangle> cover;
        for (size_t i = 0; i < nodes.size(); i++) {
            if (nodes[i].top != BaseRectNode::NO_NEIGHBOR) {
                continue;
            }
            LOG(trace) << "Node " << i << " with height " << heights[i] << " has no top neighbor.\n";
            const auto h{heights[i]};
            auto left = i;
            while (nodes[left].left != BaseRectNode::NO_NEIGHBOR
                && heights[nodes[left].left] >= h) {
                    left = nodes[left].left;
            }
            LOG(trace) << "Leftmost neighbor is " << left << " with height " << heights[left] << "\n";
            auto right = i;
            while (nodes[right].right != BaseRectNode::NO_NEIGHBOR
                && heights[nodes[right].right] >= h) {
                    right = nodes[right].right;
            }
            LOG(trace) << "Top right node is " << right << " with height " << heights[right] << "\n";
            auto bottomLeft = left;
            for (size_t j = 0; j < h; j++) {
                assert(bottomLeft != BaseRectNode::NO_NEIGHBOR);
                bottomLeft = nodes[bottomLeft].bottom;
            }
            LOG(trace) << "Bottom left node is " << bottomLeft << " with height " << heights[bottomLeft] << "\n";

            LOG(debug) << "Found rectangle ("
                    << nodes[bottomLeft].base_rectangle.get_bottom_left()
                    << "), ("
                    << nodes[right].base_rectangle.get_top_right()
                    << ")\n";
            cover.insert(
                Rectangle{
                    nodes[bottomLeft].base_rectangle.get_bottom_left(),
                    nodes[right].base_rectangle.get_top_right()});
        }
        return { cover.begin(), cover.end() };
    }
}