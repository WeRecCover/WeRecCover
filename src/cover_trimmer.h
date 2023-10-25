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

#ifndef COVERING_COVER_TRIMMER_H
#define COVERING_COVER_TRIMMER_H

#include "logging.h"

#include "cover_postprocessor.h"
#include "baserect_graph.h"

namespace cover {
    class Cover_trimmer : public Cover_postprocessor {
    protected:
        void postprocess_cover(
                Cover &cover, const Polygon_with_holes &polygon,
                const Problem_instance::Costs &costs, Runtime_environment *env,
                std::optional<Map<Point, size_t>> &covered_points) const override;

        static void trim_top(
                Rectangle& rectangle_to_trim,
                const std::vector<BaseRectNode>& nodes,
                const BaseRectGraph::PointBaseRectMap& top_right_map,
                const BaseRectGraph::PointBaseRectMap& bottom_left_map,
                std::vector<BaseRectNode::PtrType>& br_coverage);

        static void trim_left(
                Rectangle& rectangle_to_trim,
                const std::vector<BaseRectNode>& nodes,
                const BaseRectGraph::PointBaseRectMap& top_right_map,
                const BaseRectGraph::PointBaseRectMap& bottom_left_map,
                std::vector<BaseRectNode::PtrType>& br_coverage);

        static void trim_bottom(
                Rectangle& rectangle_to_trim,
                const std::vector<BaseRectNode>& nodes,
                const BaseRectGraph::PointBaseRectMap& top_right_map,
                const BaseRectGraph::PointBaseRectMap& bottom_left_map,
                std::vector<BaseRectNode::PtrType>& br_coverage);

        static void trim_right(
                Rectangle& rectangle_to_trim,
                const std::vector<BaseRectNode>& nodes,
                const BaseRectGraph::PointBaseRectMap& top_right_map,
                const BaseRectGraph::PointBaseRectMap& bottom_left_map,
                std::vector<BaseRectNode::PtrType>& br_coverage);

    public:
        using Cover_postprocessor::Cover_postprocessor;
    };
}

#endif //COVERING_COVER_TRIMMER_H
