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

#include "cover_provider.h"
#include "cover_postprocessor.h"

namespace cover {
    Cover_provider::Cover
    Cover_postprocessor::get_cover_for(const Polygon_with_holes &polygon, const Problem_instance::Costs &costs, Runtime_environment *env) {
        std::optional<Map<Point, size_t>> covered_points{};
        return get_post_processed_cover_for(polygon, costs, env, covered_points);
    }

    Cover_provider::Cover Cover_postprocessor::get_post_processed_cover_for(
        const Polygon_with_holes &polygon, const Problem_instance::Costs &costs,
        Runtime_environment *env,
        std::optional<Map<Point, size_t>> &covered_points) const {
      auto cover{get_previous_cover_for(polygon, costs, env, covered_points)};

      postprocess_cover(cover, polygon, costs, env, covered_points);

      return cover;
    }

    Cover_provider::Cover Cover_postprocessor::get_previous_cover_for(
        const Polygon_with_holes &polygon, const Problem_instance::Costs &costs,
        Runtime_environment *env,
        std::optional<Map<Point, size_t>> &covered_points) const {

      if (previous_postprocessor != nullptr) {
        return previous_postprocessor->get_post_processed_cover_for(
            polygon, costs, env, covered_points);
      } else {
        return underlying_algorithm->get_cover_for(polygon, costs, env);
      }
    }

    BaseRectGraph& Cover_postprocessor::get_or_calculate_br_graph(
            const Polygon_with_holes &polygon,
            Runtime_environment *env) {
        if (env->graph.empty()) {
            env->graph.build(get_or_calculate_brs(polygon, env));
        }

        return env->graph;
    }

    std::vector<Rectangle>& Cover_postprocessor::get_or_calculate_brs(const Polygon_with_holes &polygon,
                                                                      Runtime_environment *env) {
        if (env->base_rectangles.empty()) {
            env->base_rectangles = Rectangle_enumerator::get_base_rectangles(polygon);
        }

        return env->base_rectangles;
    }

    std::vector<size_t>& Cover_postprocessor::get_or_calculate_br_coverage(const Polygon_with_holes &polygon,
                                                                           const Cover_provider::Cover &cover,
                                                                           Runtime_environment *env) {
        if (env->base_rectangle_cover_counts.empty()) {
            const auto &nodes = get_or_calculate_br_graph(polygon, env).getNodes();
            std::vector<size_t> covered(nodes.size(), 0);
            for (const auto& rectangle : cover) {
                LOG(debug) << "Computing base rectangles covered by rectangle " << rectangle.as_polygon() << "...\n";

                const auto tr{rectangle.get_top_right()};
                const auto bl{rectangle.get_bottom_left()};

                for (auto it = env->graph.begin(tr, bl); it != env->graph.end(); ++it) {
                    LOG(trace) << "Base rectangle " << *it << " is covered.\n";
                    ++covered[*it];
                    assert(rectangle.fully_contains(nodes[*it].base_rectangle));
                }
            }
            env->base_rectangle_cover_counts = std::move(covered);
        }

        return env->base_rectangle_cover_counts;
    }
}
