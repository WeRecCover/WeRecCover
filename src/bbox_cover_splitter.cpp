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

#include "bbox_cover_splitter.h"

namespace cover {

std::vector<Rectangle> Bounding_box_cover_splitter::split_rectangle(
    const cover::Rectangle &rectangle, const Problem_instance::Costs &costs,
    Runtime_environment *env) const {
  LOG(trace) << "Attempting bounding box split on rectangle: "
             << rectangle.as_polygon();
  const auto polygon_split{split_into_polygons(rectangle, env)};
  std::vector<Rectangle> bounding_box_rectangles{};

  for (const auto &polygon : polygon_split) {
    const auto bbox{polygon.bbox()};
    bounding_box_rectangles.emplace_back(bbox.xmin(), bbox.ymin(), bbox.xmax(),
                                         bbox.ymax());
  }

  LOG(trace) << "Split into " << bounding_box_rectangles.size()
             << " rectangles";

  return bounding_box_rectangles;
}
} // cover
