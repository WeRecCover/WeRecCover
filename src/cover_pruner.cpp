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

#include "cover_pruner.h"
#include "rectangle_enumerator.h"

namespace cover {

void Cover_pruner::postprocess_cover(
    Cover_provider::Cover &cover, const Polygon_with_holes &polygon,
    const Problem_instance::Costs &costs, Runtime_environment *env,
    std::optional<Map<Point, size_t>> &covered_points) const {

  LOG(info) << "Running Cover_pruner on cover";

  env->pixel_coverage_invalidated = true;

  if (env->base_rectangles.empty()) {
    env->base_rectangles = Rectangle_enumerator::get_base_rectangles(polygon);
  }

  const auto &nodes = env->graph.getNodes();
  const auto &topRight = env->graph.getTopRightMap();

  size_t num_pruned {0};

  auto& covered{ get_or_calculate_br_coverage(polygon, cover, env) };

  for (size_t i = 0; i < cover.size();) {
    const auto &rectangle{cover[i]};
    LOG(debug) << "Checking cover rectangle " << rectangle.as_polygon() << "...\n";
    bool redundant = true;

    const auto tr{rectangle.get_top_right()};
    const auto bl{rectangle.get_bottom_left()};

    for (auto it = env->graph.begin(tr, bl); it != env->graph.end(); ++it) {
      assert(covered[*it] > 0);
      if (covered[*it] == 1) {
        redundant = false;
      }
    }

    if (redundant) {
      LOG(debug) << "Rectangle " << rectangle.as_polygon()
                 << " is fully redundant, pruning it\n";

      // decrement counts
      for (auto it = env->graph.begin(tr, bl); it != env->graph.end(); ++it) {
        LOG(trace) << "Base rectangle " << *it << " was covered "
                   << covered[*it] << " times.\n";
        assert(covered[*it] > 1);
        covered[*it]--;
      }

      num_pruned++;
      if (i < cover.size()) {
        std::swap(cover.back(), cover[i]);
      }
      cover.pop_back();
    } else {
      i++;
    }
  }
  LOG(info) << "Pruned " << num_pruned << " rectangles.";
}

} // cover