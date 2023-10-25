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

#include "algorithm_runner.h"

#include <utility>
#include <iostream>

#include "logging.h"

namespace cover {
    bool Algorithm_runner::is_valid_cover(const Cover &rectangles, const Polygon_with_holes &polygon) {
        LOG(debug) << "Verifying cover...";

        assert(polygon.outer_boundary().size() > 4 || polygon.has_holes());

        std::vector<Polygon_with_holes> joined_rectangles{};
        std::vector<Polygon> rectangle_polygons{};
        rectangle_polygons.reserve(rectangles.size());

        auto rects{rectangles};
        std::sort(rects.begin(), rects.end());

          for (const auto &rectangle: rects) {
            if (rectangle.get_max_y() <= rectangle.get_min_y() || rectangle.get_max_x() <= rectangle.get_min_x()) {
                return false;
            }
            rectangle_polygons.push_back(rectangle.as_polygon());
        }

        LOG(trace) << "Joining " << rectangle_polygons.size() << " rectangles into polygon(s)...";
        CGAL::join(rectangle_polygons.begin(), rectangle_polygons.end(),
                   std::back_inserter(joined_rectangles), CGAL::Tag_false());

        LOG(trace) << "Done, result is " << joined_rectangles.size() << " polygon(s).";
        if (joined_rectangles.empty() || joined_rectangles.size() > 1) {
            return false;
        }

        LOG(trace) << "Computing symmetric difference of input polygon and joined rectangles polygon...";
        std::vector<Polygon_with_holes> differences{};
        CGAL::symmetric_difference(polygon, joined_rectangles.at(0), std::back_inserter(differences), CGAL::Tag_false());
        LOG(trace) << "Done, result is " << differences.size();

        return differences.empty();

    }

    bool Algorithm_runner::verify_cover(const Cover &rectangles, const Polygon_with_holes &polygon) {
        LOG(debug) << "Verifying cover II...";

        assert(polygon.outer_boundary().size() > 4 || polygon.has_holes());

        auto rects{rectangles};
        std::sort(rects.begin(), rects.end());

        std::vector<Polygon_with_holes> uncovered_polygons{};
        uncovered_polygons.push_back(polygon);
        std::vector<Polygon_with_holes> uncovered_polygons2{};
        LOG(trace) << "Subtracting " << rects.size() << " rectangles from input polygon...";
        std::vector<Polygon_with_holes> diff;
        for (const auto &rectangle: rects) {
            if (rectangle.get_max_y() <= rectangle.get_min_y() || rectangle.get_max_x() <= rectangle.get_min_x()) {
                return false;
            }
            CGAL::difference(rectangle.as_polygon(), polygon, std::back_inserter(diff));
            if (!diff.empty()) {
              return false;
            }
            for (const auto &ucpoly : uncovered_polygons) {
              CGAL::difference(ucpoly, rectangle.as_polygon(), std::back_inserter(uncovered_polygons2), CGAL::Tag_false());
            }
            std::swap(uncovered_polygons, uncovered_polygons2);
            uncovered_polygons2.clear();
        }

        LOG(trace) << "Done, " << uncovered_polygons.size() << " polygon(s) left.";

        return uncovered_polygons.empty();
    }

    std::vector<Algorithm_runner::Result>
    Algorithm_runner::run_algorithm(Cover_provider &algorithm,
                                    const Problem_instance &instance,
                                    bool verify) {
      Result::Validity valid{Result::Validity::UNCHECKED};
      std::vector<Algorithm_runner::Result> results;
      results.reserve(instance.get_multi_polygon().size() + 1);
      results.emplace_back(Result()); // use for total
      if (verify) {
        results[0].is_valid = Result::Validity::VALID;
      }

      const auto &polygons {instance.get_multi_polygon()};
      Runtime_environment env;
      for (const auto &polygon : polygons) {
        if (polygon.outer_boundary().size() == 4 && !polygon.has_holes()) {
          LOG(info) << "Polygon is hole-free rectangle, skipping...";
          continue;
        }

        env.clear();
        LOG(info) << "Computing cover for polygon " << results.size() << " / " << polygons.size();
        const auto start_time{clock::now()};

        const auto partial_cover{
            algorithm.get_cover_for(polygon, instance.get_costs(), &env)};
        const auto end_time{clock::now()};

        if (algorithm.timeouted()) {
          valid = Result::Validity::TIMEOUT;
        } else if (verify) {
          valid = is_valid_cover(partial_cover, polygon)
                      ? Result::Validity::VALID
                      : Result::Validity::INVALID;
        }

        const auto duration{
            std::chrono::duration_cast<nanos>(end_time - start_time)};
        const auto cost = instance.calculate_cost_of_cover(partial_cover);
        const auto size = partial_cover.size();

        LOG(info) << "Finished within " << duration.count() << "ns, validity status: " << valid;

        results.push_back(
            {size, cost, duration, valid, std::move(partial_cover)});

        results[0].cover_size += size;
        results[0].cost += cost;
        results[0].execution_time += duration;
        if (algorithm.timeouted()) {
          results[0].is_valid = Result::Validity::TIMEOUT;
        } else if (valid == Result::Validity::INVALID) {
          results[0].is_valid = Result::Validity::INVALID;
        }
      }
      LOG(info) << (polygons.size() - results.size() + 1) << " trivial polygons skipped.";

      return results;
    }
}

namespace std {

std::ostream &
operator<<(std::ostream &out,
                const cover::Algorithm_runner::Result::Validity &status) {
  switch (status) {
  case cover::Algorithm_runner::Result::INVALID:
    out << "invalid";
    break;
  case cover::Algorithm_runner::Result::VALID:
    out << "valid";
    break;
  case cover::Algorithm_runner::Result::UNCHECKED:
    out << "unchecked";
    break;
  case cover::Algorithm_runner::Result::TIMEOUT:
    out << "timeout";
    break;
  }
  return out;
}

} // namespace std