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

#include "greedy_set_cover_algorithm.h"
#include "datastructures.h"
#include <sstream>

namespace cover {

    /**
     * A struct which maintains information about a rectangle in the queue of the algorithm.
     *
     * Maintains the rectangle itself, its area, the area it covers which is not yet covered by any other rectangle,
     * its cost and its cost per unit which it uniquely covers.
     */
    struct Greedy_set_cover_algorithm::QueueEntry {
        Rectangle rectangle;
        size_t area;
        size_t effective_area;
        size_t cost;
        double cost_per_unit;

        explicit QueueEntry(const Rectangle &rectangle, const Problem_instance::Costs &costs)
            : rectangle(rectangle), area(rectangle.area()), effective_area(area),
              cost(Problem_instance::calculate_total_cost_of_rectangle(rectangle, costs)),
              cost_per_unit(static_cast<double>(cost) / static_cast<double>(effective_area)) {}

        bool 
        update(const Rectangle &picked_rectangle,
               const std::vector<Rectangle> &newly_covered_base_rectangles) {
          LOG(trace) << "Updating " << this->print().rdbuf() <<  " for picked rectangle " << picked_rectangle;

          if (!picked_rectangle.intersects(rectangle)) {
            LOG(trace) << "  Picked rectangle does not intersect, "
                          "skipping";
            // picked rectangle covers none of our area
            return false;
          }

          if (picked_rectangle.fully_contains(rectangle)) {
            LOG(trace) << "Picked rectangle fully contains this entry, making it completely "
                          "redundant, setting"
                          " effective area to 0 and returning from update...";
            // picked rectangle covers us completely
            effective_area = 0;
            return true;
          }

          LOG(trace) << "Picked rectangle partially intersects this entry, checking base "
                        "rectangle intersections...";
          for (const auto &base_rectangle : newly_covered_base_rectangles) {
            LOG(trace) << "Checking intersection with "
                       << base_rectangle;
            if (rectangle.fully_contains(base_rectangle)) {
              LOG(trace) << "Entry rectangle fully contains base rectangle: "
                         << base_rectangle
                         << ", reducing effective area of entry rectangle ("
                         << effective_area
                         << ") by area of base rectangle ("
                         << base_rectangle.area() << ")...";
              assert(effective_area >= base_rectangle.area());
              effective_area -= base_rectangle.area();
            }
          }

          if (effective_area == 0) {
            LOG(trace) << "Entry rectangle has no effective area left, "
                          "returning from update...";
            return true;
          }

          cost_per_unit = static_cast<double>(cost) /
                                static_cast<double>(effective_area);
          LOG(trace) << "  Updated: " << this->print().rdbuf();
          return true;
        }

        std::stringstream print() const {
          std::stringstream ss;
          ss << "QE: " << rectangle << ", cost per unit: " << cost_per_unit
             << ", area: " << area << ", effective area: " << effective_area;
          return ss;
        }
    };

    std::vector<Rectangle> Greedy_set_cover_algorithm::calculate_cover(
        const Polygon_with_holes &polygon, const Problem_instance::Costs &costs,
        Runtime_environment *env) {
      using RectangleQueue = std::vector<QueueEntry>;
      LOG(info) << "Running Eager Greedy Set Cover algorithm (using base rectangle graph)";

      std::vector<Rectangle> cover{};

      LOG(debug) << "Getting base rectangles of polygon";
      if (env->base_rectangles.empty()) {
        env->base_rectangles =
            Rectangle_enumerator::get_base_rectangles(polygon);
      }
      if (env->graph.empty()) {
        env->graph.build(env->base_rectangles);
      }
      const auto &nodes{env->graph.getNodes()};

      auto all_rectangles {env->graph.get_all_rectangles()};
      RectangleQueue rectangle_queue;
      for (const auto &rectangle : all_rectangles) {
        QueueEntry entry{rectangle, costs};
        rectangle_queue.push_back(entry);
      }

      auto current_best_entry{
          std::max_element(rectangle_queue.begin(), rectangle_queue.end(),
                           [](const QueueEntry &lhs, const QueueEntry &rhs) {
                             return lhs.area < rhs.area;
                           })};

      Set<Rectangle> covered_base_rectangles;
      while (true) {
        LOG(debug) << rectangle_queue.size() << " rectangle(s) left in queue";

        const auto best_rectangle{current_best_entry->rectangle};

        LOG(trace) << "Adding rectangle to cover: "
                   << current_best_entry->print().rdbuf();

        std::vector<Rectangle> base_of_best;
        for (auto it = env->graph.begin(best_rectangle.get_top_right(),
                                        best_rectangle.get_bottom_left());
             it != env->graph.end(); ++it) {
          if (covered_base_rectangles.find(nodes[*it].base_rectangle) ==
              covered_base_rectangles.cend()) {
            covered_base_rectangles.insert(nodes[*it].base_rectangle);
            base_of_best.push_back(nodes[*it].base_rectangle);
          }
        }
        LOG(trace) << "Base of added rectangle has size "
                   << base_of_best.size();
        assert(!base_of_best.empty());

        cover.push_back(best_rectangle);
        std::swap(*current_best_entry, rectangle_queue.back());
        rectangle_queue.pop_back();

        LOG(debug) << covered_base_rectangles.size() << " / "
                   << nodes.size() << " covered.";

        if (covered_base_rectangles.size() == nodes.size()) {
          LOG(debug) << "No uncovered base rectangles left, exiting loop";
          break;
        }

        LOG(trace) << "Updating remaining queue entries";

        double current_best_cost{std::numeric_limits<double>::infinity()};
        for (auto entry_it{rectangle_queue.begin()}; entry_it != rectangle_queue.end();) {
          entry_it->update(best_rectangle, base_of_best);

          if (entry_it->effective_area == 0) {
            LOG(trace) << "Entry has no effective area left, pruning it";
            std::swap(*entry_it, rectangle_queue.back());
            rectangle_queue.pop_back();
          } else {
            LOG(trace) << "Entry has cost per unit: " << entry_it->cost_per_unit
                       << ", current best: " << current_best_cost;
            if (entry_it->cost_per_unit < current_best_cost //) {
                || entry_it->cost_per_unit == current_best_cost
                    && entry_it->effective_area > current_best_entry->effective_area) {
              LOG(trace)
                  << "Entry has improved cost, marking it as current best";
              current_best_entry = entry_it;
              current_best_cost = entry_it->cost_per_unit;
            }
            ++entry_it;
          }
        }
      }

      LOG(info) << "Greedy_set_cover_algorithm finished";
      return cover;
    }

} // cover
