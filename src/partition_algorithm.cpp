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

#include "partition_algorithm.h"

namespace cover {
    const Direction Partition_algorithm::UP_DIRECTION = {0, 1};
    const Direction Partition_algorithm::RIGHT_DIRECTION = {1, 0};
    const Direction Partition_algorithm::DOWN_DIRECTION = {0, -1};
    const Direction Partition_algorithm::LEFT_DIRECTION = {-1, 0};

    std::vector<Segment> Partition_algorithm::find_good_diagonals(
            const Polygon_with_holes &polygon,
            const Util::ConcaveMap &concave_vertices) {

        std::vector<Segment> good_diagonals{};

        Map<NumType, std::vector<NumType>> x_aligned{};
        Map<NumType, std::vector<NumType>> y_aligned{};

        LOG(debug) << "Determining horizontally/vertically aligned concave vertices";

        for (const auto &[vertex, _]: concave_vertices) {
            x_aligned[vertex.y()].push_back(vertex.x());
            y_aligned[vertex.x()].push_back(vertex.y());
        }

        LOG(trace) << "Sorting horizontally aligned concave vertices";
        for (auto &[_, vec]: x_aligned) {
            std::sort(vec.begin(), vec.end());
        }

        LOG(trace) << "Sorting vertically aligned concave vertices";
        for (auto &[_, vec]: y_aligned) {
            std::sort(vec.begin(), vec.end());
        }

        LOG(debug) << "Finding good vertical diagonals";
        find_good_diagonals(polygon, UP_DIRECTION, y_aligned, concave_vertices, good_diagonals);

        LOG(debug) << "Finding good horizontal diagonals";
        find_good_diagonals(polygon, RIGHT_DIRECTION, x_aligned, concave_vertices, good_diagonals);

        LOG(debug) << "Found good diagonals";
        return good_diagonals;
    }

    void Partition_algorithm::find_good_diagonals(const Polygon_with_holes &polygon,
                                                  const Direction &positive_direction,
                                                  const Map<NumType,
                                                          std::vector<NumType>> &aligned,
                                                  const Util::ConcaveMap &concave_vertices,
                                                  std::vector<Segment> &good_diagonals) {
        const auto negative_direction{Util::rotate_180_degrees(positive_direction)};
        bool is_horizontally_aligned{positive_direction.dy() == 0};

        for (const auto &[fixed_coordinate, variable_coordinates]: aligned) {
            if (variable_coordinates.size() <= 1) {
                // too few aligned vertices to form a diagonal
                continue;
            }

            auto variable_coordinate_it{variable_coordinates.begin()};
            while (variable_coordinate_it != (variable_coordinates.end() - 1)) {
                auto x{fixed_coordinate};
                auto y{*variable_coordinate_it};

                if (is_horizontally_aligned) {
                    std::swap(x, y);
                }

                const Point point{x, y};
                LOG(trace) << "Processing candidate good diagonal vertex: " << point;
                const auto &open_directions{concave_vertices.find(point)->second};
                if (std::find(open_directions.begin(), open_directions.end(),
                              positive_direction) == open_directions.end()) {
                    // our current concave vertex is not open in the positive direction, if this were a good diagonal,
                    // we would have already found it, since we check whether the current concave is open positively
                    // and the next concave is open negatively, next concave could be open positively or negatively,
                    // so increment once
                    LOG(trace) << "Candidate is not open in positive direction, continuing...";
                    ++variable_coordinate_it;
                    continue;
                }

                const auto next_variable_coordinate{*(variable_coordinate_it + 1)};

                if (is_horizontally_aligned) {
                    x = next_variable_coordinate;
                } else {
                    y = next_variable_coordinate;
                }

                const Point other_point{x, y};
                LOG(trace) << "Potential good diagonal between source: " << point << " and target: "
                           << other_point;
                const auto &other_open_directions{concave_vertices.find(other_point)->second};
                if (std::find(other_open_directions.begin(), other_open_directions.end(),
                              negative_direction) == other_open_directions.end()) {
                    // next concave is not open in the negative direction, meaning it cannot form a good diagonal
                    // with our current concave, which is open positively, but it could form a good diagonal with
                    // the concave after it, increment once
                    LOG(trace) << "Target concave is not open in the negative direction, continuing...";
                    ++variable_coordinate_it;
                    continue;
                }

                const Segment segment{point, other_point};
                LOG(trace) << "Candidate good diagonal found: " << segment;
                if (is_valid_good_diagonal(segment, polygon)) {
                    LOG(trace) << "Candidate " << segment << " is valid, added to good diagonals";
                    good_diagonals.push_back(segment);
                }

                if (++variable_coordinate_it != variable_coordinates.end() - 1) {
                    // double incrementing here on purpose, since our current concave is positively open and the next
                    // one is negatively open, we already know that the next concave cannot form another good
                    // diagonal across this axis, so we can skip it here
                    ++variable_coordinate_it;
                }
            }
        }
    }

    bool Partition_algorithm::is_valid_good_diagonal(const Segment &segment, const Polygon_with_holes &polygon) {
        LOG(trace) << "Checking validity of good diagonal" << segment;

        for (const auto &edge: polygon.outer_boundary().edges()) {
            if (intersects_interior(edge, segment)) {
                return false;
            }
        }

        for (const auto &hole: polygon.holes()) {
            for (const auto &edge: hole.edges()) {
                if (intersects_interior(edge, segment)) {
                    return false;
                }
            }
        }

        return true;
    }

    // SOURCE BEGIN
    // Code was based on https://stackoverflow.com/q/23293992/6875882
    // Retrieved on 27.11.2022
    std::vector<Segment>
    Partition_algorithm::determine_ideal_good_diagonal_set(
            const std::vector<std::pair<Segment, Segment>> &intersecting_good_diagonals,
            Set<Point> &handled_concave_vertices) {
        LOG(debug) << "Determining ideal good diagonal set via bipartite intersection flow graph";

        if (intersecting_good_diagonals.empty()) {
            LOG(debug) << "There are no intersecting good diagonals, all good diagonals can be used";
            return {};
        }

        LOG(debug) << "Constructing flow graph";

        FlowGraph flow_graph{};

        const auto source{add_vertex(flow_graph)};
        const auto sink{add_vertex(flow_graph)};

        LOG(trace) << "Source node: " << source << ", sink node: " << sink;

        Map<Segment, GraphVertex> segment_vertex_map{};
        Map<GraphVertex, Segment> vertex_segment_map{};
        std::vector<GraphEdge> reverse_edges{};
        std::vector<float> capacities{};

        for (const auto &[vertical, horizontal]: intersecting_good_diagonals) {
            const auto vertical_vertex{get_or_create_vertex(flow_graph, vertical,
                                                            segment_vertex_map, vertex_segment_map)};
            const auto horizontal_vertex{get_or_create_vertex(flow_graph, horizontal,
                                                              segment_vertex_map, vertex_segment_map)};

            if (vertical_vertex.second) {
                add_edge(flow_graph, source, vertical_vertex.first, 1, reverse_edges, capacities);
            }

            add_edge(flow_graph, vertical_vertex.first, horizontal_vertex.first, std::numeric_limits<float>::infinity(),
                     reverse_edges, capacities);

            if (horizontal_vertex.second) {
                add_edge(flow_graph, horizontal_vertex.first, sink, 1, reverse_edges, capacities);
            }
        }

        std::vector<float> residual_capacities(num_edges(flow_graph), 0);

        LOG(debug) << "Calculating maximum flow";
        boykov_kolmogorov_max_flow(flow_graph,
                                   make_iterator_property_map(&capacities[0], get(edge_index, flow_graph)),
                                   make_iterator_property_map(&residual_capacities[0], get(edge_index, flow_graph)),
                                   make_iterator_property_map(&reverse_edges[0], get(edge_index, flow_graph)),
                                   get(boost::vertex_color, flow_graph),
                                   get(boost::vertex_index, flow_graph),
                                   source,
                                   sink
        );

        LOG(debug) << "Determining ideal cuts";
        std::vector<Segment> ideal_cuts{};

        auto source_out_edges{out_edges(source, flow_graph)};

        for (auto it{source_out_edges.first}; it != source_out_edges.second; ++it) {
            const auto target{boost::target(*it, flow_graph)};
            const auto color{get(vertex_color, flow_graph, target)};

            // color indicates source/sink partition, see boost docs for details
            if (color == boost::black_color) {
                const auto cut{vertex_segment_map[target]};
                LOG(trace) << "Cut " << cut << " is ideal vertical cut, adding to cut set";
                ideal_cuts.push_back(cut);

                LOG(trace) << "Adding " << cut.source() << " and " << cut.target()
                           << " to set of handled concave vertices";
                handled_concave_vertices.insert({cut.source(), cut.target()});
            }
        }

        auto sink_in_edges{out_edges(sink, flow_graph)};

        for (auto it{sink_in_edges.first}; it != sink_in_edges.second; ++it) {
            const auto target{boost::target(*it, flow_graph)};
            const auto color{get(vertex_color, flow_graph, target)};

            // color indicates source/sink partition, see boost docs for details
            if (color != boost::black_color) {
                const auto cut{vertex_segment_map[target]};
                LOG(trace) << "Cut " << cut << " is ideal horizontal cut, adding to cut set";
                ideal_cuts.push_back(cut);

                LOG(trace) << "Adding " << cut.source() << " and " << cut.target()
                           << " to set of handled concave vertices";
                handled_concave_vertices.insert({cut.source(), cut.target()});
            }
        }

        LOG(debug) << "Picked " << ideal_cuts.size() << " ideal cuts";

        return ideal_cuts;
    }

    void Partition_algorithm::add_edge(FlowGraph &flow_graph, GraphVertex source, GraphVertex target, float capacity,
                                       std::vector<GraphEdge> &reverse_edges,
                                       std::vector<float> &capacities) {
        const auto next_edge_id{num_edges(flow_graph)};

        const auto edge{boost::add_edge(source, target, next_edge_id, flow_graph).first};
        const auto reverse_edge{boost::add_edge(target, source, next_edge_id + 1, flow_graph).first};

        LOG(trace) << "Adding edge " << edge << " from " << source << " to " << target
                   << " with capacity " << capacity;

        reverse_edges.push_back(reverse_edge);
        reverse_edges.push_back(edge);
        capacities.push_back(capacity);
        capacities.push_back(0);
    }
    // SOURCE END

    std::pair<Partition_algorithm::GraphVertex, bool> Partition_algorithm::get_or_create_vertex(
            FlowGraph &flow_graph, const Segment &segment,
            Map<Segment, GraphVertex> &segment_vertex_map,
            Map<GraphVertex, Segment> &vertex_segment_map) {
        const auto potential_existing{segment_vertex_map.find(segment)};

        if (potential_existing != segment_vertex_map.end()) {
            return {potential_existing->second, false};
        }

        const auto newly_created{boost::add_vertex(flow_graph)};
        segment_vertex_map.insert({segment, newly_created});
        vertex_segment_map.insert({newly_created, segment});

        LOG(trace) << "Added new node: " << newly_created;

        put(vertex_index, flow_graph, newly_created, num_vertices(flow_graph) - 1);
        put(vertex_color, flow_graph, newly_created, boost::white_color);

        return {newly_created, true};
    }

    std::vector<std::pair<Segment, Segment>>
    Partition_algorithm::find_intersecting_good_diagonals(const std::vector<Segment> &good_diagonals) {
        LOG(debug) << "Determining intersecting good diagonals";

        std::vector<Segment> verticals{};
        std::vector<Segment> horizontals{};

        LOG(trace) << "Sorting good diagonals into horizontals and verticals";

        for (const auto &diagonal: good_diagonals) {
            if (diagonal.is_horizontal()) {
                horizontals.push_back(diagonal);
            } else {
                verticals.push_back(diagonal);
            }
        }

        LOG(trace) << "Intersecting horizontals and verticals";
        std::vector<std::pair<Segment, Segment>> intersections{};

        for (const auto &vertical: verticals) {
            for (const auto &horizontal: horizontals) {
                if (CGAL::do_intersect(vertical, horizontal)) {
                    LOG(trace) << "Vertical " << vertical << " and horizontal " << horizontal
                               << "intersect, adding to intersection list";
                    intersections.emplace_back(vertical, horizontal);
                }
            }
        }

        LOG(debug) << "Determined intersecting good diagonals";

        return intersections;
    }

    Segment Partition_algorithm::pick_cut(const Polygon_with_holes &polygon,
                                          const Util::ConcaveMapEntry &concave_entry,
                                          const std::vector<Segment> &previous_cuts,
                                          Set<Point> &handled_concave_vertices) {
        LOG(trace) << "Picking a cut for concave vertex: " << concave_entry.first;

        handled_concave_vertices.insert(concave_entry.first);

        // picking essentially whatever open direction is available here, doesn't really matter
        // which one, since we're just picking ANY cut
        const auto direction{concave_entry.second.at(0).vector()};

        const Ray ray{concave_entry.first, direction};

				OrderedSet<Point> point_intersections{};

        // intersecting ray with polygon's boundary
        for (const auto &edge: polygon.outer_boundary().edges()) {
            const auto result{Util::get_point_intersection(ray, edge)};
            if (result.has_value()) {
                point_intersections.insert(result.value());
            }
        }

        // intersecting ray with holes
        for (const auto &hole: polygon.holes()) {
            for (const auto &edge: hole.edges()) {
                const auto result{Util::get_point_intersection(ray, edge)};
                if (result.has_value()) {
                    point_intersections.insert(result.value());
                }
            }
        }

        // intersecting ray with previous cuts
        for (const auto &cut: previous_cuts) {
            const auto result{Util::get_point_intersection(ray, cut)};
            if (result.has_value()) {
                point_intersections.insert(result.value());
            }
        }

        // we will intersect the concave itself at some point, so we're taking it out here
        point_intersections.erase(concave_entry.first);

        // choosing "closest" intersection point in the chosen open direction from the position of our current concave
        const auto closest_intersection_point{
                (direction.x() + direction.y() > 0 ? *point_intersections.begin() : *point_intersections.rbegin())};

        Segment picked_cut{concave_entry.first, closest_intersection_point};

        LOG(trace) << "Picked cut: " << picked_cut;

        return picked_cut;
    }

    std::vector<Rectangle> Partition_algorithm::determine_resulting_rectangles(const Polygon_with_holes &polygon,
                                                                               const std::vector<Segment> &cuts) {
        return Util::parse_rectangles(Util::create_arrangement(polygon, cuts), polygon);
    }

    std::vector<Rectangle>
    Partition_algorithm::calculate_cover(const Polygon_with_holes &polygon,
                                         const Problem_instance::Costs &costs,
                                         Runtime_environment *env) {
      LOG(info) << "Partition_algorithm running";

      LOG(debug) << "Gathering concave vertices";

      const auto concave_map{Util::find_concave_vertices(polygon)};
      Set<Point> handled_concave_vertices{};

      LOG(debug) << "Determining good diagonals";

      const auto good_diagonals{find_good_diagonals(polygon, concave_map)};

      const auto intersecting_good_diagonals{
          find_intersecting_good_diagonals(good_diagonals)};

      const auto ideal_good_intersecting_diagonals{
          determine_ideal_good_diagonal_set(intersecting_good_diagonals,
                                            handled_concave_vertices)};

      LOG(debug) << "Adding ideal diagonal set to set of cuts";

      std::vector<Segment> cuts{ideal_good_intersecting_diagonals.begin(),
                                ideal_good_intersecting_diagonals.end()};

      // flattening the intersection map
      Set<Segment> intersecting_good_diags{};
      for (const auto &[vertical, horizontal] : intersecting_good_diagonals) {
        intersecting_good_diags.insert({vertical, horizontal});
      }

      LOG(debug) << "Picking remaining non-intersecting good diagonals";
      for (const auto &good_diagonal : good_diagonals) {
        if (intersecting_good_diags.count(good_diagonal) == 0) {
          cuts.push_back(good_diagonal);
          handled_concave_vertices.insert(
              {good_diagonal.source(), good_diagonal.target()});
        }
      }

      LOG(debug) << "Picking arbitrary cuts for remaining concave vertices";
      for (const auto &entry : concave_map) {
        const auto is_handled{handled_concave_vertices.count(entry.first) != 0};

        if (!is_handled) {
          const auto chosen_cut{
              pick_cut(polygon, entry, cuts, handled_concave_vertices)};
          cuts.push_back(chosen_cut);
        }
      }

      LOG(debug) << "Used " << cuts.size() << " cuts";

      auto partition{determine_resulting_rectangles(polygon, cuts)};

      LOG(info) << "Partition_algorithm finished";

      return partition;
    }

    bool Partition_algorithm::intersects_interior(const Segment &segment1, const Segment &segment2) {
        const auto segment1_vertical{segment1.is_vertical()};
        const auto segment2_vertical{segment2.is_vertical()};

        if (segment1_vertical && segment2_vertical) {
            if (segment1.target().x() != segment2.target().x()) {
                return false;
            }

            auto segment1_max_y{segment1.target().y()};
            auto segment1_min_y{segment1.source().y()};

            if (segment1_max_y <= segment1_min_y) {
                std::swap(segment1_max_y, segment1_min_y);
            }

            const auto segment2_source_y{segment2.source().y()};
            const auto segment2_target_y{segment2.target().y()};

            return !((segment2_source_y >= segment1_max_y && segment2_target_y >= segment1_max_y) ||
                     segment2_source_y <= segment1_min_y && segment2_target_y <= segment1_min_y);
        } else if (!segment1_vertical && !segment2_vertical) {
            if (segment1.target().y() != segment2.target().y()) {
                return false;
            }

            auto segment1_max_x{segment1.target().x()};
            auto segment1_min_x{segment1.source().x()};

            if (segment1_max_x <= segment1_min_x) {
                std::swap(segment1_max_x, segment1_min_x);
            }

            const auto segment2_source_x{segment2.source().x()};
            const auto segment2_target_x{segment2.target().x()};

            return !((segment2_source_x >= segment1_max_x && segment2_target_x >= segment1_max_x) ||
                     segment2_source_x <= segment1_min_x && segment2_target_x <= segment1_min_x);
        } else {
            auto horizontal{segment1};
            auto vertical{segment2};

            if (horizontal.is_vertical()) {
                std::swap(horizontal, vertical);
            }

            const auto horizontal_y{horizontal.target().y()};
            auto vertical_max_y{vertical.target().y()};
            auto vertical_min_y{vertical.source().y()};

            if (vertical_max_y < vertical_min_y) {
                std::swap(vertical_max_y, vertical_min_y);
            }

            if (!(horizontal_y < vertical_max_y && horizontal_y > vertical_min_y)) {
                return false;
            }

            const auto vertical_x{vertical.target().x()};
            auto horizontal_max_x{horizontal.target().x()};
            auto horizontal_min_x{horizontal.source().x()};

            if (horizontal_max_x < horizontal_min_x) {
                std::swap(horizontal_max_x, horizontal_min_x);
            }

            if (!(vertical_x < horizontal_max_x && vertical_x > horizontal_min_x)) {
                return false;
            }

            return true;
        }
    }

} // cover
