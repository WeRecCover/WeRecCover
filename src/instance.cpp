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

#include "instance.h"

namespace cover {

    Problem_instance::Problem_instance(const fs::path &wkt_path,
                                       size_t rectangle_creation_cost, size_t rectangle_area_cost) :
            multi_polygon(convert_wkt_to_multi_polygon(wkt_path)),
            costs({rectangle_creation_cost, rectangle_area_cost}), wkt_path(wkt_path), name(convert_to_name(wkt_path)) {
    }

    MultiPolygon Problem_instance::convert_wkt_to_multi_polygon(const fs::path &wkt_path) {
        if (!fs::exists(wkt_path)) {
            throw std::runtime_error("Input WKT file '" + wkt_path.string() + "' not found");
        }

        if (wkt_path.extension() != ".wkt") {
            throw std::runtime_error("File '" + wkt_path.string() + "' is not a .wkt file");
        }

        std::ifstream wkt_stream{wkt_path.string()};
        MultiPolygon multi_polygon{};
        CGAL::IO::read_multi_polygon_WKT(wkt_stream, multi_polygon);
        return multi_polygon;
    }

    std::string Problem_instance::convert_to_name(const fs::path &wkt_path) {
        auto name{wkt_path.parent_path().filename().string() + '_' + wkt_path.filename().stem().string()};
        std::replace(name.begin(), name.end(), '\\', '_');
        std::replace(name.begin(), name.end(), '/', '_');
        std::replace(name.begin(), name.end(), ' ', '_');

        return name;
    }

    Problem_instance::Costs Problem_instance::calculate_cost_of_cover(const std::vector<Rectangle> &cover,
                                                                      const Problem_instance::Costs &costs) {
        Costs total_cost;

        for (const auto &rectangle: cover) {
            total_cost += calculate_cost_of_rectangle(rectangle, costs);
        }

        return total_cost;
    }
} // cover
