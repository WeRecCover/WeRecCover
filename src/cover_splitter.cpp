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

#include "cover_splitter.h"

namespace cover {
    void Cover_splitter::postprocess_cover(Cover &cover, const Polygon_with_holes &polygon,
                                           const Problem_instance::Costs &costs, Runtime_environment *env,
                                           std::optional<Map<Point, size_t>> &covered_points) const {
        LOG(info) << "Subclass of Cover_splitter postprocessing cover";

        get_or_calculate_br_coverage(polygon, cover, env);
        env->pixel_coverage_invalidated = true;

        std::vector<Rectangle> newly_added_rectangles{};

        auto rectangle_it{cover.begin()};
        while (rectangle_it != cover.end()) {
            const auto current_costs{Problem_instance::calculate_cost_of_rectangle(*rectangle_it, costs)};
            const auto current_total_cost{current_costs.area_cost + current_costs.creation_cost};

            const auto split{split_rectangle(*rectangle_it, costs, env)};
            const auto new_costs{Problem_instance::calculate_cost_of_cover(split, costs)};
            const auto new_total_cost{new_costs.creation_cost + new_costs.area_cost};

            LOG(debug) << "Cost of current rectangle: " << current_total_cost
                       << ", cost of proposed split: " << new_total_cost;

            if (new_total_cost < current_total_cost) {
                LOG(debug) << "Split improves solution, adding to cover, removing old rectangle";
                newly_added_rectangles.insert(newly_added_rectangles.end(), split.begin(), split.end());
                reduce_covered_amount(polygon, *rectangle_it, split, env);

                auto last_rectangle{cover.end() - 1};
                *rectangle_it = *last_rectangle;
                cover.pop_back();
            } else {
                LOG(debug) << "Split does not improve solution, continuing";
                ++rectangle_it;
            }
        }

        cover.insert(cover.end(), newly_added_rectangles.begin(), newly_added_rectangles.end());
    }

    void Cover_splitter::reduce_covered_amount(const Polygon_with_holes &polygon,
                                                const Rectangle &original_rectangle,
                                               const std::vector<Rectangle> &split_rectangles,
                                               Runtime_environment *env) {
        LOG(debug) << "Updating covered brs for new split";
        assert(!env->base_rectangles.empty() && !env->graph.empty());

        auto& covered{env->base_rectangle_cover_counts};
        const auto& nodes{ env->graph.getNodes() };

        const auto tr{ original_rectangle.get_top_right() };
        const auto bl{ original_rectangle.get_bottom_left() };

        for (auto it{ env->graph.begin(tr, bl) }; it != env->graph.end(); ++it) {
            const auto& base_rect{ nodes[*it].base_rectangle };

            const auto still_covered{
                std::any_of(split_rectangles.begin(), split_rectangles.end(), [&](const Rectangle& new_rectangle){
                    return new_rectangle.fully_contains(base_rect);
                })
            };

            if (!still_covered) {
                --covered[*it];
                assert(covered[*it] >= 1);
            }
        }
    }

    std::vector<Polygon_with_holes> Cover_splitter::split_into_polygons(const Rectangle &rectangle, Runtime_environment *env) {
        assert(!env->graph.empty() && !env->base_rectangle_cover_counts.empty() && !env->base_rectangles.empty());

        const auto& nodes{ env->graph.getNodes() };

        auto uniquely_covered{ get_uniquely_covered_brs(rectangle, env) };
        std::vector<Polygon> as_polygons{};
        std::sort(uniquely_covered.begin(), uniquely_covered.end(),
            [&nodes](const auto &first, const auto &second) {
                return nodes[first].base_rectangle < nodes[second].base_rectangle;
            });
        for (auto& unique_br : uniquely_covered) {
            const auto& base_rect{ nodes[unique_br].base_rectangle };
            LOG(trace) << base_rect.as_polygon();
            as_polygons.push_back(base_rect.as_polygon());
        }

        std::vector<Polygon_with_holes> split_polygons{};
        CGAL::join(as_polygons.begin(), as_polygons.end(),
                   std::back_inserter(split_polygons), CGAL::Tag_false());

        return split_polygons;
    }

    std::vector<BaseRectNode::PtrType>
    Cover_splitter::get_uniquely_covered_brs(const Rectangle &rectangle, Runtime_environment *env) {
        assert(!env->graph.empty() && !env->base_rectangle_cover_counts.empty() && !env->base_rectangles.empty());

        auto& graph{ env->graph };
        const auto& covered{ env->base_rectangle_cover_counts };
        std::vector<BaseRectNode::PtrType> uniquely_covered{};

        const auto tr{ rectangle.get_top_right() };
        const auto bl{ rectangle.get_bottom_left() };

        for (auto it{ graph.begin(tr, bl) }; it != graph.end(); ++it) {
            if (covered[*it] == 1) {
                uniquely_covered.push_back(*it);
            }
        }

        return uniquely_covered;
    }
}