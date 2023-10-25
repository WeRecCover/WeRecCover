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

#include "cover_joiner_full.h"

namespace cover {
    bool Cover_joiner_full::is_valid(const Polygon_with_holes &polygon, const Rectangle &rectangle) {
        if (!is_valid(polygon.outer_boundary(), rectangle)) {
            return false;
        }

        return std::all_of(polygon.holes().begin(), polygon.holes().end(), [&](const auto &hole) {
            return is_valid(hole, rectangle);
        });
    }

    bool Cover_joiner_full::is_valid(const Polygon &polygon, const Rectangle &rectangle) {
        return std::all_of(polygon.edges().begin(), polygon.edges().end(), [&](const Segment &edge) {
            return !rectangle.fully_intersects(edge);
        });
    }

    void Cover_joiner_full::postprocess_cover(
        Cover_provider::Cover &cover, const Polygon_with_holes &polygon,
        const Problem_instance::Costs &costs, Runtime_environment *env,
        std::optional<Map<Point, size_t>> &covered_points) const {
      auto it{cover.begin()};
      while (it != cover.end()) {
        auto inner_it{it};
        std::optional<CostType> current_best_cost_reduction{};
        auto current_best_other_rectangle{inner_it};
        auto current_best_joined_rectangle{*inner_it};
        while (++inner_it != cover.end()) {
          const auto potentially_joined{try_join_rectangles(
              *it, *inner_it, polygon, costs, current_best_cost_reduction)};

          if (potentially_joined.has_value()) {
            current_best_cost_reduction = potentially_joined.value().second;
            current_best_other_rectangle = inner_it;
            current_best_joined_rectangle = potentially_joined.value().first;
          }
        }
        if (current_best_cost_reduction.has_value()) {
          bool best_is_last{current_best_other_rectangle == cover.end() - 1};

          *it = *(cover.end() - 1);
          cover.pop_back();

          if (!best_is_last) {
            *current_best_other_rectangle = *(cover.end() - 1);
          } else {
            *it = *(cover.end() - 1);
          }

          cover.pop_back();
          cover.push_back(current_best_joined_rectangle);
        } else {
          ++it;
        }
      }
    }

    std::optional<std::pair<Rectangle, CostType>> Cover_joiner_full::try_join_rectangles(
            const Rectangle &first, const Rectangle &second,
            const Polygon_with_holes &polygon,
            const Problem_instance::Costs &costs,
            std::optional<CostType> current_best_cost_reduction) {
        const auto max_x{std::max({first.get_max_x(), second.get_max_x()})};
        const auto min_x{std::min({first.get_min_x(), second.get_min_x()})};
        const auto max_y{std::max({first.get_max_y(), second.get_max_y()})};
        const auto min_y{std::min({first.get_min_y(), second.get_min_y()})};

        Rectangle joined{min_x, min_y, max_x, max_y};

        const auto original_cost{
                Problem_instance::calculate_total_cost_of_rectangle(first, costs) +
                Problem_instance::calculate_total_cost_of_rectangle(second, costs)
        };

        const auto joined_cost{Problem_instance::calculate_total_cost_of_rectangle(joined, costs)};
        const auto cost_reduction{original_cost - joined_cost};

        if (
                joined_cost >= original_cost ||
                (current_best_cost_reduction.has_value() && current_best_cost_reduction.value() <= cost_reduction) ||
                !is_valid(polygon, joined)) {
            return {};
        }

        return {{joined, cost_reduction}};
    }

} // cover
