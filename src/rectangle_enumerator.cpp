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

#include "rectangle_enumerator.h"
#include <algorithm>

namespace cover {
    void Rectangle_enumerator::pick_cuts(const Polygon_with_holes &polygon, const Util::ConcaveMapEntry &concave_entry,
                                         std::vector<Segment> &cuts) {
        LOG(trace) << "Picking cuts for concave vertex " << concave_entry.first;

        // picking both possible cuts for the concave vertex
        for (const auto &direction: concave_entry.second) {
            LOG(trace) << "Picking cut in direction " << direction;
            const Ray ray{concave_entry.first, direction};
            const auto closest_intersection{Util::get_closest_intersection(ray, polygon)};

            assert(closest_intersection.has_value());

            LOG(trace) << "Picked cut: " << Segment(concave_entry.first, closest_intersection.value());

            cuts.emplace_back(concave_entry.first, closest_intersection.value());
        }
    }

    std::vector<Rectangle> Rectangle_enumerator::get_base_rectangles(const Polygon_with_holes &polygon) {
        LOG(debug) << "Generating base rectangles";

        assert(polygon.outer_boundary().size() > 4 || polygon.has_holes());

        //if (polygon.outer_boundary().size() == 4 && !polygon.has_holes()) {
        //    const auto &boundary = polygon.outer_boundary();
        //    auto bl {*std::min_element(boundary.vertices_begin(), boundary.vertices_end())};
        //    auto tr {*std::max_element(boundary.vertices_begin(), boundary.vertices_end())};
        //    LOG(debug) << "Returning base rectangle " << bl << ", " << tr
        //        << " for polygon " << polygon << "\n";
        //    return { Rectangle(bl, tr) };
        //}

        // making two cuts from every concave vertex in the polygon
        const auto concave_vertices{Util::find_concave_vertices(polygon)};

        LOG(trace) << "Picking cuts";
        std::vector<Segment> cuts{};
        for (const auto &entry: concave_vertices) {
            pick_cuts(polygon, entry, cuts);
            IF_LOG_LEVEL(debug) {
                if (cuts.size() % 10000 == 0) {
                    LOG(debug) << "Picked " << cuts.size() << " cuts";
                }
            }
        }
        return Util::parse_rectangles(Util::create_arrangement(polygon, cuts), polygon);
    }

    Rectangle_enumerator::NeighborSideMap
    Rectangle_enumerator::build_neighbor_side_map(const std::vector<Rectangle> &rectangles) {
        LOG(debug) << "Building neighbor side map";
        NeighborSideMap neighbor_side_map{};

        for (const auto &rectangle: rectangles) {
            neighbor_side_map[Side::TOP].insert({rectangle.get_top_edge(), rectangle});
            neighbor_side_map[Side::LEFT].insert({rectangle.get_left_edge(), rectangle});
        }

        return neighbor_side_map;
    }

    void Rectangle_enumerator::visit(const Rectangle &base_rectangle, const NeighborSideMap &neighbors,
                                     std::vector<Rectangle> &seen) {
        LOG(trace) << "Visiting rectangle " << base_rectangle.as_polygon();

        auto min_x{base_rectangle.get_min_x()}, max_y{base_rectangle.get_max_y()};

        LOG(trace) << "Starting with: min_x = " << min_x << ", min_y = ";

        Rectangle current_top_rectangle{base_rectangle};
        std::optional<size_t> maximum_down_extension{};

        // going right
        while (true) {
            auto max_x{current_top_rectangle.get_max_x()};
            Rectangle current_inner_rectangle{current_top_rectangle};

            size_t down_extensions{0};
            // going down
            while (true) {
                auto min_y{current_inner_rectangle.get_min_y()};
                seen.emplace_back(min_x, min_y, max_x, max_y);

                if (down_extensions == maximum_down_extension) {
                    break;
                }

                const auto inner_bottom_neighbor{get_bottom_neighbor(current_inner_rectangle, neighbors)};
                if (inner_bottom_neighbor.has_value()) {
                    current_inner_rectangle = inner_bottom_neighbor.value();
                    ++down_extensions;
                } else {
                    maximum_down_extension = down_extensions;
                    break;
                }
            }

            const auto top_right_neighbor{get_right_neighbor(current_top_rectangle, neighbors)};
            if (top_right_neighbor.has_value()) {
                current_top_rectangle = top_right_neighbor.value();
            } else {
                break;
            }
        }
    }

    std::vector<Rectangle> Rectangle_enumerator::enumerate_rectangles(const Polygon_with_holes &polygon) {
        return enumerate_rectangles(get_base_rectangles(polygon));
    }

    std::optional<Rectangle> Rectangle_enumerator::get_right_neighbor(
            const Rectangle &rectangle, const Rectangle_enumerator::NeighborSideMap &neighbors) {
        LOG(trace) << "Getting right neighbor of rectangle " << rectangle.as_polygon();
        if (neighbors.count(Side::LEFT) == 0) {
            LOG(trace) << "Rectangle has no right neighbor";
            return {};
        }

        const auto &right_neighbors{neighbors.at(Side::LEFT)};
        const auto &entry{right_neighbors.find(rectangle.get_right_edge().opposite())};
        auto right_neighbor{entry == right_neighbors.end() ? std::nullopt : std::make_optional(entry->second)};

        IF_LOG_LEVEL(trace) {
            if (right_neighbor.has_value()) {
                LOG(trace) << "Rectangle's right neighbor is " << right_neighbor->as_polygon();
            }
        }

        return right_neighbor;
    }

    std::optional<Rectangle> Rectangle_enumerator::get_bottom_neighbor(
            const Rectangle &rectangle, const Rectangle_enumerator::NeighborSideMap &neighbors) {
        LOG(trace) << "Getting bottom neighbor of rectangle " << rectangle.as_polygon();
        if (neighbors.count(Side::TOP) == 0) {
            LOG(trace) << "Rectangle has no bottom neighbor";
            return {};
        }

        const auto &bottom_neighbors{neighbors.at(Side::TOP)};
        const auto &entry{bottom_neighbors.find(rectangle.get_bottom_edge().opposite())};

        auto bottom_neighbor{entry == bottom_neighbors.end() ? std::nullopt : std::make_optional(entry->second)};

        IF_LOG_LEVEL(trace) {
            if (bottom_neighbor.has_value()) {
                LOG(trace) << "Rectangle's bottom neighbor is " << bottom_neighbor->as_polygon();
            }
        }

        return bottom_neighbor;
    }

    std::vector<Rectangle> Rectangle_enumerator::enumerate_rectangles(const std::vector<Rectangle> &base_rectangles) {
        LOG(debug) << "Enumerating rectangles, base rectangle amount = " << base_rectangles.size();
        const auto neighbor_map{build_neighbor_side_map(base_rectangles)};

        LOG(debug) << "Visiting base rectangles";
        std::vector<Rectangle> seen_rectangles{};
        for (const auto &base_rectangle: base_rectangles) {
            visit({{base_rectangle}}, neighbor_map, seen_rectangles);
        }

        LOG(debug) << "Enumerated rectangle amount = " << seen_rectangles.size();
        return seen_rectangles;
    }
} // cover
