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

#include "util.h"

namespace cover {
    Direction Util::normalize(const Direction &direction) {
        auto x = direction.dx() == 0 ? 0 : (direction.dx() < 0 ? -1 : 1);
        auto y = direction.dy() == 0 ? 0 : (direction.dy() < 0 ? -1 : 1);

        return {x, y};
    }

    bool Util::has_on_any_edge(const Point &point, const Polygon_with_holes &polygon) {
        const auto &boundary{polygon.outer_boundary()};

        if (has_on_any_edge(point, boundary)) {
            return true;
        }

        for (const auto &hole: polygon.holes()) {
            if (has_on_any_edge(point, hole)) {
                return true;
            }
        }
        return std::any_of(polygon.holes().begin(), polygon.holes().end(),
                           [=](const auto &hole) { return has_on_any_edge(point, hole); });
    }

    bool Util::has_on_any_edge(const Point &point, const Polygon &polygon) {
        if (polygon.is_simple()) {
            return polygon.has_on_boundary(point);
        }

        return std::any_of(polygon.edges().begin(), polygon.edges().end(),
                           [=](const auto &edge) { return CGAL::do_intersect(edge, point); });
    }

    std::optional<Point> Util::get_point_intersection(const Ray &ray, const Segment &segment) {
        const auto result{CGAL::intersection(segment, ray)};

        if (result != boost::none) {
            const auto point{boost::get<Point>(&*result)};
            if (point) {
                return *point;
            }
        }

        return {};
    }

    Util::ConcaveMap Util::find_concave_vertices(const Polygon_with_holes &polygon) {
        LOG(trace) << "Finding concave vertices of polygon with holes";

        auto concave_vertices{find_concave_vertices(polygon.outer_boundary())};

        for (const auto &hole: polygon.holes()) {
            for (const auto &concave_entry: find_concave_vertices(hole)) {
                if (concave_vertices.find(concave_entry.first) == concave_vertices.end()) {
                    concave_vertices.insert(concave_entry);
                } else {
                    concave_vertices.erase(concave_entry.first);
                }
            }
        }

        return concave_vertices;
    }

    Util::ConcaveMap Util::find_concave_vertices(const Polygon &polygon) {
        LOG(trace) << "Finding concave vertices of polygon";

        // expect polygon to have at least 4 edges
        assert(polygon.edges().size() >= 4);

        auto concave_vertices{ConcaveMap()};
        auto next_edge{polygon.edges_circulator()};

        for (const auto &edge: polygon.edges()) {
            LOG(trace) << "Trying to find concave of edge: " << edge;
            ++next_edge;

            auto entry{determine_vertex_entry(edge, *next_edge)};

            if (!entry.has_value()) {
                LOG(trace) << "Edge has no concave vertex";
                continue;
            }

            LOG(trace) << "Edge has concave vertex: " << entry.value().first << " with open directions: "
                       << entry.value().second.at(0) << ", " << entry.value().second.at(1);

            concave_vertices.insert(entry.value());
        }

        LOG(trace) << concave_vertices.size() << " concave vertices found";

        return concave_vertices;
    }

    std::optional<Util::ConcaveMapEntry> Util::determine_vertex_entry(const Segment &edge, const Segment &next_edge) {

        const auto direction{Util::normalize(edge.direction())};

        if (rotate_270_degrees(direction) != next_edge.direction()) {
            return {};
        }

        return {{edge.target(), {direction, rotate_90_degrees(direction)}}};
    }

    std::optional<Point> Util::get_closest_intersection(const Ray &ray, const Polygon_with_holes &polygon) {
				OrderedSet<Point> intersections{};

        const auto closest_outer_intersection{get_closest_intersection(ray, polygon.outer_boundary())};

        if (closest_outer_intersection.has_value()) {
            intersections.insert(closest_outer_intersection.value());
        }

        for (const auto &hole: polygon.holes()) {
            const auto closest_hole_intersection{get_closest_intersection(ray, hole)};

            if (closest_hole_intersection.has_value()) {
                intersections.insert(closest_hole_intersection.value());
            }
        }

        if (intersections.empty()) {
            return {};
        }

        const auto direction{Util::normalize(ray.direction())};

        if (direction.dy() > 0 || direction.dx() > 0) {
            return {*intersections.begin()};
        } else {
            return {*intersections.rbegin()};
        }
    }

    std::optional<Point> Util::get_closest_intersection(const Ray &ray, const Polygon &polygon) {
				OrderedSet<Point> intersections{};

        for (const auto &edge: polygon.edges()) {
            if (edge.source() == ray.source() || edge.target() == ray.source()) {
                continue;
            }

            const auto intersection{CGAL::intersection(ray, edge)};
            if (intersection == boost::none) {
                continue;
            }

            const auto point{boost::get<Point>(&*intersection)};

            if (point) {
                intersections.insert(*point);
            } else {
                const auto segment{boost::get<Segment>(&*intersection)};
                intersections.insert(segment->source());
                intersections.insert(segment->target());
            }
        }

        if (intersections.empty()) {
            return {};
        }

        const auto direction{Util::normalize(ray.direction())};

        if (direction.dy() > 0 || direction.dx() > 0) {
            return {*intersections.begin()};
        } else {
            return {*intersections.rbegin()};
        }
    }

    Arrangement
    Util::create_arrangement(const Polygon_with_holes &polygon, const std::vector<Segment> &cuts) {
        LOG(debug) << "Creating arrangement";

        std::vector<ArrangementSegment> combined_cuts{cuts.begin(), cuts.end()};

        for (const auto &edge: polygon.outer_boundary().edges()) {
            combined_cuts.emplace_back(edge);
        }

        for (const auto &hole: polygon.holes()) {
            for (const auto &edge: hole.edges()) {
                combined_cuts.emplace_back(edge);
            }
        }

        LOG(debug) << "Constructing arrangement with " << combined_cuts.size() << " segments";
        Arrangement arrangement{};
        CGAL::insert(arrangement, combined_cuts.begin(), combined_cuts.end());

        return arrangement;
    }

    std::vector<Rectangle>
    Util::parse_rectangles(const Arrangement &arrangement, const Polygon_with_holes &polygon) {
        std::vector<Rectangle> rectangles{};

        for (auto face{arrangement.faces_begin()}; face != arrangement.faces_end(); ++face) {
            if (!face->has_outer_ccb()) {
                continue;
            }

            assert(!face->is_unbounded());
            assert(!face->is_fictitious());
            assert(face->has_outer_ccb());

            auto half_edge_it{face->outer_ccb()};

            const auto first_vertex{half_edge_it->source()->point()};

            auto min_x{first_vertex.x()},
                    max_x{first_vertex.x()},
                    min_y{first_vertex.y()},
                    max_y{first_vertex.y()};

            const auto curve{half_edge_it->curve()};
            Direction current_direction{Util::normalize(Segment(curve).direction())};
            size_t seen_direction_changes{0};
            bool is_rectangle{true};
            ++half_edge_it;

            // recording the maximum and minimum x/y coordinates of the rectangle and also checking whether it
            // actually is a rectangle by observing the number of times the edges change directions as we iterate them
            do {
                const auto vertex{half_edge_it->source()->point()};

                if (vertex.x() > max_x) {
                    max_x = vertex.x();
                } else if (vertex.x() < min_x) {
                    min_x = vertex.x();
                }

                if (vertex.y() > max_y) {
                    max_y = vertex.y();
                } else if (vertex.y() < min_y) {
                    min_y = vertex.y();
                }

                const auto new_curve{half_edge_it->curve()};
                Direction new_direction{Util::normalize(Segment(new_curve).direction())};

                if (new_direction != current_direction && new_direction !=
                                                          Util::rotate_180_degrees(current_direction) &&
                    ++seen_direction_changes == 5) {
                    is_rectangle = false;
                    break;
                }

                current_direction = new_direction;
            } while (++half_edge_it != face->outer_ccb());

            if (is_rectangle) {
                Rectangle rectangle{min_x, min_y, max_x, max_y};
                bool is_hole{false};

                // checking if the rectangle was a hole in the polygon
                for (const auto &hole: polygon.holes()) {
                    if (rectangle.as_polygon().bbox() == hole.bbox()) {
                        is_hole = true;
                        break;
                    }
                }

                if (!is_hole) {
                    rectangles.push_back(rectangle);
                    IF_LOG_LEVEL(debug) {
                        if (rectangles.size() % 1000 == 0) {
                            LOG(debug) << "Seen " << rectangles.size() << " rectangles";
                        }
                    }
                }
            }
        }

        return rectangles;
    }

} // cover
