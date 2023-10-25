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

#ifndef COVERING_PARTITION_ALGORITHM_H
#define COVERING_PARTITION_ALGORITHM_H

#include <limits>

#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/boykov_kolmogorov_max_flow.hpp>

#include "logging.h"

#include "CGAL_classes.h"
#include "algorithm.h"
#include "util.h"
#include "datastructures.h"

using namespace boost;

namespace cover {

    /**
     * @brief Algorithm which finds a partition of a polygon into an optimal number of rectangles in polynomial time
     */
    class Partition_algorithm : public Algorithm {
    protected:
        using FlowGraph = adjacency_list <vecS, hash_setS, directedS,
        property<vertex_index_t, size_t, property<vertex_color_t, boost::default_color_type>>,
        property <edge_index_t, size_t>>;

        using GraphVertex = FlowGraph::vertex_descriptor;
        using GraphEdge = FlowGraph::edge_descriptor;

        static const Direction UP_DIRECTION, RIGHT_DIRECTION, DOWN_DIRECTION, LEFT_DIRECTION;

        /**
         * Checks if a good diagonal, i.e. a fictional segment which connects two concave vertices is "valid", meaning
         * that it does not leave the interior of the polygon and does not intersect any edge of the polygon at any
         * point(s) except its endpoints.
         *
         * @param segment The diagonal to check for validity
         * @param polygon The polygon the segment should stay inside of
         * @return Whether the diagonal is valid or not
         */
        static bool is_valid_good_diagonal(const Segment &segment, const Polygon_with_holes &polygon);

        /**
         * Checks whether segment1 and segment2 intersect at any point(s) except their endpoints. This is used by
         * is_valid_good_diagonal to determine whether a good diagonal intersects an edge of the polygon in a way
         * which makes it invalid.
         *
         * Returns:
         * - false if the segments do not intersect
         * - false if the segments intersect only at their endpoints
         * - true otherwise
         *
         * @param segment1 The first segment
         * @param segment2 The second segment
         * @return Whether the two segments intersect at any point(s) except their endpoints
         */
        static bool intersects_interior(const Segment &segment1, const Segment &segment2);

        /**
         * Finds a vector of valid, good diagonals, which are fictional segments connecting concave vertices of the
         * polygon which never leave the polygon's interior.
         *
         * This is accomplished by first finding concave vertices which are aligned vertically or horizontally,
         * then sorting them according to the axis that they are NOT aligned on. Afterwards, we iterate the aligned
         * vertices and check whether a valid, good diagonal can be formed between them.
         *
         * @param polygon The polygon to search for good diagonals
         * @param concave_vertices Map of concave vertices of the polygon and the two directions opposite of the
         * two edges the vertex is a part of
         * @return A vector of valid, good diagonals contained within the polygon
         */
        static std::vector<Segment> find_good_diagonals(const Polygon_with_holes &polygon,
                                                        const Util::ConcaveMap &concave_vertices);

        /**
         * Finds good diagonals amongst a set of aligned, sorted, concave vertices in a given positive direction.
         *
         * @param polygon The polygon to search for good diagonals
         * @param positive_direction Either the UP direction if the vertices are vertically aligned or the RIGHT
         * direction if they are horizontally aligned
         * @param aligned A map mapping a coordinate on one axis to multiple coordinates on the other axis, indicating
         * the aligned vertices
         * @param concave_vertices Map of concave vertices of the polygon and the two directions opposite of the
         * two edges the vertex is a part of
         * @param good_diagonals The vector to add the good diagonals to when they are found
         */
        static void find_good_diagonals(const Polygon_with_holes &polygon,
                const Direction &positive_direction, const Map<NumType,
                std::vector<NumType>> &aligned, const Util::ConcaveMap &concave_vertices,
                std::vector<Segment> &good_diagonals);

        /**
         * Finds good diagonals in the input vector which intersect at any point. Returns a vector containing pairs of
         * intersecting diagonals where the first element is a vertical diagonal and the second is horizontal.
         *
         * @param good_diagonals The vector of good diagonals in the polygon
         * @return The vector of intersecting diagonals
         */
        static std::vector<std::pair<Segment, Segment>> find_intersecting_good_diagonals(
                const std::vector<Segment> &good_diagonals);

        /**
         * Finds the set of good diagonals among a vector of intersecting good diagonals which contains the largest
         * number of good diagonals which do not intersect.
         *
         * Uses a max-flow graph algorithm to find a maximum independent set in a bipartite graph formed by the
         * intersecting good diagonals.
         *
         * @param intersecting_good_diagonals The vector of intersecting good diagonals
         * @param handled_concave_vertices The set of concave vertices which are the endpoint of a cut made by the
         * partition algorithm, when this function picks diagonals, it adds their endpoints to this set to indicate
         * that they have been handled and do not need to be processed later
         * @return A maximum set of non-intersecting good diagonals from the intersecting_good_diagonals vector
         */
        static std::vector<Segment>
        determine_ideal_good_diagonal_set(const std::vector<std::pair<Segment, Segment>> &intersecting_good_diagonals,
                                          Set<Point> &handled_concave_vertices);

        /**
         * Adds an edge to the flow graph used by determine_ideal_good_diagonal_set.
         *
         * @param flow_graph The flow graph to add the edge to
         * @param source The source vertex of the edge
         * @param target The target vertex of the edge
         * @param capacity The capacity of the edge
         * @param reverse_edges The vector of reverse edges of the flow graph (needed by the underlying graph algorithm)
         * @param capacities The vector of capacities of the edges of the flow graph
         */
        static void add_edge(FlowGraph &flow_graph, GraphVertex source, GraphVertex target, float capacity,
                             std::vector<FlowGraph::edge_descriptor> &reverse_edges, std::vector<float> &capacities);

        /**
         * First creates a vertex in the flow graph, if it does not exist yet, then returns it and indicates whether
         * it was newly created or not.
         *
         * @param flow_graph The flow graph to add the vertex to
         * @param segment The good diagonal the vertex represents
         * @param segment_vertex_map A map mapping good diagonals in the polygon to vertices in the flow graph
         * @param vertex_segment_map A map mapping vertices in the flow graph to good diagonals in the polygon
         * @return A pair containing the vertex and a bool indicating whether it was newly created or not
         */
        static std::pair<GraphVertex, bool> get_or_create_vertex(FlowGraph &flow_graph, const Segment &segment,
                                                Map<Segment, GraphVertex> &segment_vertex_map,
                                                Map<GraphVertex, Segment> &vertex_segment_map);

        /**
         * Creates a segment with the concave vertex passed as one endpoint and the closest intersection with
         * a segment from the previous_cuts vector or any of the polygon's edges. Used to pick cuts for concave
         * vertices which are not yet handled by any good diagonal.
         *
         * @param polygon The polygon associated with the problem instance
         * @param concave_entry The concave vertex and the two directions opposite of the two edges it is a part of
         * @param previous_cuts The segments to check for intersection in addition to the polygon's edges
         * @param handled_concave_vertices The set of concave vertices which are the endpoint of some cut made by
         * the partition algorithm, this function adds the vertex it processed to this set
         * @return The picked segment
         */
        static Segment pick_cut(const Polygon_with_holes &polygon, const Util::ConcaveMapEntry &concave_entry,
                                const std::vector<Segment> &previous_cuts,
                                Set<Point> &handled_concave_vertices);

        /**
         * Returns the rectangles in the interior of the polygon which are implied by the vector of cuts.
         *
         * @param polygon The polygon containing the rectangles/cuts
         * @param cuts The cuts which form the rectangles
         * @return The set of rectangles implied by the cuts
         */
        static std::vector<Rectangle> determine_resulting_rectangles(const Polygon_with_holes &polygon,
                                                                     const std::vector<Segment> &cuts);

        /**
         * Calculates an optimal (meaning it contains the fewest rectangles possible) partition of the polygon into
         * rectangles, which is also a cover, but without any overlap.
         *
         * @param polygon The polygon associated with the problem instance
         * @param costs The costs associated with the problem instance
         * @return An optimal partition of the polygon into rectangles
         */
        [[nodiscard]] std::vector<Rectangle> calculate_cover(const Polygon_with_holes &polygon,
                                               const Problem_instance::Costs &costs, Runtime_environment *env) override;

    };

} // cover

#endif //COVERING_PARTITION_ALGORITHM_H
