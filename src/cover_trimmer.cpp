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

#include "cover_trimmer.h"

namespace cover {
    void Cover_trimmer::postprocess_cover(
            Cover &cover, const Polygon_with_holes &polygon,
            const Problem_instance::Costs &costs, Runtime_environment *env,
            std::optional<Map<Point, size_t>> &covered_points) const {

        env->pixel_coverage_invalidated = true;

        auto& br_coverage{ get_or_calculate_br_coverage(polygon, cover, env) };
        const auto& top_right_map{ env->graph.getTopRightMap() };
        const auto& bottom_left_map{ env->graph.getBottomLeftMap() };
        const auto& nodes{ env->graph.getNodes() };

        for (auto& rectangle : cover) {
            trim_top(rectangle, nodes, top_right_map, bottom_left_map, br_coverage);
            trim_bottom(rectangle, nodes, top_right_map, bottom_left_map, br_coverage);
            trim_right(rectangle, nodes, top_right_map, bottom_left_map, br_coverage);
            trim_left(rectangle, nodes, top_right_map, bottom_left_map, br_coverage);
        }
    }

    void Cover_trimmer::trim_top(cover::Rectangle &rectangle_to_trim,
                                   const std::vector<BaseRectNode>& nodes,
                                   const BaseRectGraph::PointBaseRectMap& top_right_map,
                                   const BaseRectGraph::PointBaseRectMap& bottom_left_map,
                                   std::vector<BaseRectNode::PtrType>& br_coverage) {

        auto top_right{ rectangle_to_trim.get_top_right() };
        auto curr_br_idx{ top_right_map.at(top_right) };
        while (true) {
            const auto& curr_right_br{ nodes[curr_br_idx] };
            const auto top_left{ rectangle_to_trim.get_top_left() };
            std::vector<BaseRectNode::PtrType> seen_brs{};
            bool row_redundant{ true };
            while (true) {
                if (br_coverage[curr_br_idx] == 1) {
                    // current br is only covered by this rectangle, making this row not redundant!
                    row_redundant = false;
                    break;
                }

                seen_brs.push_back(curr_br_idx);
                const auto& curr_br{ nodes[curr_br_idx] };

                if (curr_br.base_rectangle.get_top_left() == top_left) {
                    // reached left edge of rectangle, row is truly redundant
                    break;
                } else {
                    // keep going left
                    curr_br_idx = curr_br.left;
                }
            }
            if (!row_redundant) {
                // hit first non-redundant row from top, cannot trim further
                return;
            } else {
                const auto height{ curr_right_br.base_rectangle.height() };
                rectangle_to_trim.shrink_down(height);
                curr_br_idx = curr_right_br.bottom;
                for (const auto idx : seen_brs) {
                    --br_coverage[idx];
                }
            }
        }
    }

    void Cover_trimmer::trim_left(cover::Rectangle &rectangle_to_trim,
                                    const std::vector<BaseRectNode>& nodes,
                                    const BaseRectGraph::PointBaseRectMap& top_right_map,
                                    const BaseRectGraph::PointBaseRectMap& bottom_left_map,
                                    std::vector<BaseRectNode::PtrType>& br_coverage) {

        auto bottom_left{ rectangle_to_trim.get_bottom_left() };
        auto curr_br_idx{ bottom_left_map.at(bottom_left) };
        while (true) {
            const auto& curr_bottom_br{ nodes[curr_br_idx] };
            const auto top_left{ rectangle_to_trim.get_top_left() };
            std::vector<BaseRectNode::PtrType> seen_brs{};
            bool column_is_redundant{ true };
            while (true) {
                if (br_coverage[curr_br_idx] == 1) {
                    // current br is only covered by this rectangle, making this column not redundant!
                    column_is_redundant = false;
                    break;
                }

                seen_brs.push_back(curr_br_idx);
                const auto& curr_br{ nodes[curr_br_idx] };

                if (curr_br.base_rectangle.get_top_left() == top_left) {
                    // reached top edge of rectangle, column is truly redundant
                    break;
                } else {
                    // keep going up
                    curr_br_idx = curr_br.top;
                }
            }
            if (!column_is_redundant) {
                // hit first non-redundant column from left, cannot trim further
                return;
            } else {
                const auto width{ curr_bottom_br.base_rectangle.width() };
                rectangle_to_trim.shrink_left(width);
                curr_br_idx = curr_bottom_br.right;
                for (const auto idx : seen_brs) {
                    --br_coverage[idx];
                }
            }
        }
    }

    void Cover_trimmer::trim_bottom(cover::Rectangle &rectangle_to_trim,
                                      const std::vector<BaseRectNode>& nodes,
                                      const BaseRectGraph::PointBaseRectMap& top_right_map,
                                      const BaseRectGraph::PointBaseRectMap& bottom_left_map,
                                      std::vector<BaseRectNode::PtrType>& br_coverage) {

        auto bottom_left{ rectangle_to_trim.get_bottom_left() };
        auto curr_br_idx{ bottom_left_map.at(bottom_left) };
        while (true) {
            const auto& curr_left_br{ nodes[curr_br_idx] };
            const auto bottom_right{ rectangle_to_trim.get_bottom_right() };
            std::vector<BaseRectNode::PtrType> seen_brs{};
            bool row_is_redundant{ true };
            while (true) {
                if (br_coverage[curr_br_idx] == 1) {
                    // current br is only covered by this rectangle, making this row not redundant!
                    row_is_redundant = false;
                    break;
                }

                seen_brs.push_back(curr_br_idx);
                const auto& curr_br{ nodes[curr_br_idx] };

                if (curr_br.base_rectangle.get_bottom_right() == bottom_right) {
                    // reached right edge of rectangle, row is truly redundant
                    break;
                } else {
                    // keep going right
                    curr_br_idx = curr_br.right;
                }
            }
            if (!row_is_redundant) {
                // hit first non-redundant row from bottom, cannot trim further
                return;
            } else {
                const auto height{ curr_left_br.base_rectangle.height() };
                rectangle_to_trim.shrink_up(height);
                curr_br_idx = curr_left_br.top;
                for (const auto idx : seen_brs) {
                    --br_coverage[idx];
                }
            }
        }
    }

    void Cover_trimmer::trim_right(cover::Rectangle &rectangle_to_trim,
                                     const std::vector<BaseRectNode>& nodes,
                                     const BaseRectGraph::PointBaseRectMap& top_right_map,
                                     const BaseRectGraph::PointBaseRectMap& bottom_left_map,
                                     std::vector<BaseRectNode::PtrType>& br_coverage) {
        auto top_right{ rectangle_to_trim.get_top_right() };
        auto curr_br_idx{ top_right_map.at(top_right) };
        while (true) {
            const auto& curr_top_br{ nodes[curr_br_idx] };
            const auto bottom_right{ rectangle_to_trim.get_bottom_right() };
            std::vector<BaseRectNode::PtrType> seen_brs{};
            bool column_is_redundant{ true };
            while (true) {
                if (br_coverage[curr_br_idx] == 1) {
                    // current br is only covered by this rectangle, making this column not redundant!
                    column_is_redundant = false;
                    break;
                }

                seen_brs.push_back(curr_br_idx);
                const auto& curr_br{ nodes[curr_br_idx] };

                if (curr_br.base_rectangle.get_bottom_right() == bottom_right) {
                    // reached bottom edge of rectangle, column is truly redundant
                    break;
                } else {
                    // keep going down
                    curr_br_idx = curr_br.bottom;
                }
            }
            if (!column_is_redundant) {
                // hit first non-redundant column from right, cannot trim further
                return;
            } else {
                const auto width{ curr_top_br.base_rectangle.width() };
                rectangle_to_trim.shrink_right(width);
                curr_br_idx = curr_top_br.left;
                for (const auto idx : seen_brs) {
                    --br_coverage[idx];
                }
            }
        }
    }
}
