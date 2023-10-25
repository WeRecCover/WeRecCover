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

#include "cover_joiner.h"

namespace cover {
    Cover_joiner::AlignmentMap Cover_joiner::calculate_x_alignments(const std::vector<Rectangle> &cover) {
        // places index of each rectangle in the cover into a map of containers, if two rectangles have the same
        // max and minimum y coordinates, they will be placed in the same container, i.e. they are horizontally aligned
        // each container is then sorted according to the rectangle's x coordinate

        LOG(debug) << "Determining horizontally aligned rectangles";
        AlignmentMap x_aligned{};

        size_t i{0};
        for (const auto &rectangle: cover) {
            x_aligned[{rectangle.get_min_y(), rectangle.get_max_y()}].emplace_back(i++);
        }

        for (auto &[_, aligned_indices]: x_aligned) {
            std::sort(aligned_indices.begin(), aligned_indices.end(), [&](const size_t a, const size_t b) {
                const auto &rectangle_a{cover.at(a)};
                const auto &rectangle_b{cover.at(b)};

                return rectangle_a.get_min_x() < rectangle_b.get_min_x();
            });
        }

        return x_aligned;
    }

    Cover_joiner::AlignmentMap Cover_joiner::calculate_y_alignments(const std::vector<Rectangle> &cover) {
        // places index of each rectangle in the cover into a map of containers, if two rectangles have the same
        // max and minimum x coordinates, they will be placed in the same container, i.e. they are vertically aligned
        // each container is then sorted according to the rectangle's y coordinate
        LOG(debug) << "Determining vertically aligned rectangles";

        AlignmentMap y_aligned{};

        //Map<size_t, int_fast64_t> min_ys{};

        size_t i{0};
        for (const auto &rectangle: cover) {
            y_aligned[{rectangle.get_min_x(), rectangle.get_max_x()}].emplace_back(i++);
        }

        for (auto &[_, aligned_indices]: y_aligned) {
            std::sort(aligned_indices.begin(), aligned_indices.end(), [&](const size_t a, const size_t b) {
                const auto &rectangle_a{cover.at(a)};
                const auto &rectangle_b{cover.at(b)};

                return rectangle_a.get_min_y() < rectangle_b.get_min_y();
            });
        }

        return y_aligned;
    }

    OrderedSet<size_t> Cover_joiner::join_aligned_entries(const Polygon_with_holes &polygon,
                                                        std::vector<Rectangle> &cover,
                                                        const std::vector<size_t> &aligned_indices,
                                                        const Problem_instance::Costs &costs,
                                                        bool vertically_aligned) {
        // joins vertically/horizontally aligned rectangles if their combined cost is lower than the total of their
        // individual costs, if multiple rectangles are aligned in a line, an attempt to join is only made between
        // neighboring rectangles, i.e. in a chain like a --> b --> c, first an attempt is made to join a and b, if
        // it succeeds, we get ab --> c and an attempt will be made to join ab and c, otherwise, an attempt will be
        // made to join b and c

        if (aligned_indices.size() == 1) {
            return {};
        }

        LOG(debug) << "Joining aligned rectangles";

        size_t prev_index{aligned_indices.at(0)};
        std::vector<Rectangle>::iterator curr_rectangle_it{};
        auto prev_rectangle_it{cover.begin() + prev_index};

        OrderedSet<size_t> to_be_deleted{};

        for (auto index{aligned_indices.begin() + 1}; index != aligned_indices.end(); ++index) {
            curr_rectangle_it = cover.begin() + *index;

            const auto current_cost{
                    Problem_instance::calculate_total_cost_of_cover({*curr_rectangle_it, *prev_rectangle_it}, costs)};
            LOG(debug) << "Current cost is " << std::to_string(current_cost);
            const auto proposed_join{curr_rectangle_it->join(*prev_rectangle_it)};

            const auto proposed_cost{Problem_instance::calculate_total_cost_of_rectangle(proposed_join, costs)};
            LOG(debug) << "Proposed new cost is " << std::to_string(proposed_cost);

            LOG(trace) << "Checking whether rectangle: " << proposed_join.as_polygon() << " is valid";
            if (proposed_cost < current_cost && is_valid(polygon, proposed_join, vertically_aligned)) {
                LOG(debug) << "Proposed join valid and cheaper than current cost, added to cover";
                to_be_deleted.insert(prev_index);
                to_be_deleted.insert(*index);

                cover.push_back(proposed_join);

                prev_index = cover.size() - 1;
                prev_rectangle_it = cover.end() - 1;
            } else {
                LOG(debug) << "Proposed join is invalid or too costly";
                prev_index = *index;
                prev_rectangle_it = curr_rectangle_it;
            }
        }

        return to_be_deleted;
    }

    bool Cover_joiner::is_valid(const Polygon_with_holes &polygon, const Rectangle &rectangle, bool is_vertical_join) {
        if (!is_valid(polygon.outer_boundary(), rectangle, is_vertical_join)) {
            return false;
        }

        return std::all_of(polygon.holes().begin(), polygon.holes().end(), [&](const auto &hole) {
            return is_valid(hole, rectangle, is_vertical_join);
        });
    }

    bool Cover_joiner::is_valid(const Polygon &polygon, const Rectangle &rectangle, bool is_vertical_join) {
        auto edge_it{polygon.edges_begin()};
        if ((is_vertical_join && edge_it->is_vertical()) || (!is_vertical_join && edge_it->is_horizontal())) {
            // skip first edge if it isn't the opposite of our join type
            // if we're joining vertically, we can ignore vertical edges, same for horizontal joins and vertical edges
            ++edge_it;
        }

        while (edge_it != polygon.edges_end()) {
            if (rectangle.fully_intersects(*edge_it)) {
                return false;
            }

            if (++edge_it == polygon.edges_end()) {
                break;
            } else {
                ++edge_it;
            }
        }

        return true;
    }

    void Cover_joiner::postprocess_cover(
        cover::Cover_provider::Cover &cover,
        const cover::Polygon_with_holes &polygon,
        const Problem_instance::Costs &costs, Runtime_environment *env,
        std::optional<Map<Point, size_t>> &covered_points) const {
      LOG(info) << "Running Cover_joiner on returned cover";

      auto x_aligned{calculate_x_alignments(cover)};

      OrderedSet<size_t> joined_indices{};

      LOG(debug) << "Joining horizontally aligned rectangles";
      // join horizontally aligned rectangles
      for (auto &[_, aligned_indices] : x_aligned) {
        const auto newly_joined{join_aligned_entries(
            polygon, cover, aligned_indices, costs, false)};
        joined_indices.insert(newly_joined.begin(), newly_joined.end());
      }

      for (auto index{joined_indices.rbegin()}; index != joined_indices.rend();
           ++index) {
        cover.erase(cover.begin() + *index);
      }

      joined_indices.clear();

      auto y_aligned{calculate_y_alignments(cover)};

      LOG(debug) << "Joining vertically aligned rectangles";
      // join vertically aligned rectangles
      for (auto &[_, aligned_indices] : y_aligned) {
        const auto newly_joined{
            join_aligned_entries(polygon, cover, aligned_indices, costs, true)};
        joined_indices.insert(newly_joined.begin(), newly_joined.end());
      }

      for (auto index{joined_indices.rbegin()}; index != joined_indices.rend();
           ++index) {
        cover.erase(cover.begin() + *index);
      }

      LOG(info) << "Cover joiner finished";
    }
} // cover
