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

#ifndef COVERING_PROBLEM_INSTANCE_H
#define COVERING_PROBLEM_INSTANCE_H

#include <cstdint>
#include <experimental/filesystem>
#include <fstream>

#include <CGAL/IO/WKT.h>

#include "CGAL_classes.h"
#include "rectangle.h"

namespace fs = std::experimental::filesystem;

namespace cover {
    /**
     * @brief Represents a single problem instance
     *
     * Represents a single instance of the rectilinear polygon covering problem with costs for rectangle creation and
     * rectangle area.
     */
    class Problem_instance {
    public:
        struct Costs {
            CostType creation_cost {0};
            CostType area_cost {0};
            void operator+=(const Costs &other) {
                creation_cost += other.creation_cost;
                area_cost += other.area_cost;
            }
            CostType sum() const {
                return creation_cost + area_cost;
            }
        };

    protected:
        const fs::path wkt_path;
        const std::string name;
        const MultiPolygon multi_polygon;
        const Costs costs;
        //uint_fast64_t timeout {0};

        /**
         * Converts the path to the WKT file of the problem instance into a compact name.
         *
         * @param wkt_path The path to the WKT file
         * @return A compact name for the WKT file
         */
        static std::string convert_to_name(const fs::path &wkt_path);

    public:
        /**
         * Constructor that takes the path to a WKT file and converts it into a MultiPolygon.
         *
         * @param wkt_path Path to the WKT file to use for this problem instance
         * @param rectangle_creation_cost Costs for creating a single rectangle when covering part of the
         *                                    problem's MultiPolygon
         * @param rectangle_area_cost Costs per area unit of a single rectangle when covering part of the
         *                            problem's MultiPolygon
         */
        Problem_instance(const fs::path &wkt_path, size_t rectangle_creation_cost,
                         size_t rectangle_area_cost);

        /**
         * Returns the compact name of the problem instance's WKT file.
         *
         * @return The compact name of the problem instance's WKT file
         */
        [[nodiscard]] const std::string &get_name() const {
            return name;
        }

        /**
         * Returns the problem instance's underlying MultiPolygon which needs to be covered.
         *
         * @return Problem instance's underlying MultiPolygon
         */
        [[nodiscard]] const MultiPolygon &get_multi_polygon() const {
            return multi_polygon;
        }

        /**
         * Returns the cost struct associated with the problem instance.
         *
         * @return The cost struct associated with the problem instance
         */
        [[nodiscard]] Costs get_costs() const {
            return costs;
        }

        /**
         * Returns the cost incurred when creating a single rectangle when covering the problem instance's
         * underlying MultiPolygon.
         *
         * @return Costs incurred when creating a rectangle
         */
        [[nodiscard]] CostType get_rectangle_creation_cost() const {
            return costs.creation_cost;
        }

        /**
         * Returns the cost incurred for each area unit of a created rectangle when covering part of the problem
         * instance's underlying MultiPolygon.
         *
         * @return Costs incurred per
         */
        [[nodiscard]] CostType get_rectangle_area_cost() const {
            return costs.area_cost;
        }

        /**
         * Returns the total, combined cost incurred by a rectangle when used in this problem instance.
         *
         * @param rectangle The rectangle to calculate the cost of
         * @return Total cost of creation and area for the given rectangle
         */
        [[nodiscard]] Costs calculate_cost_of_rectangle(const Rectangle &rectangle) const {
            return calculate_cost_of_rectangle(rectangle, costs);
        }

        /**
         * Returns the total, combined cost incurred by a whole covering on this problem instance.
         *
         * @param cover The rectangle cover to calculate the cost of
         * @return The cost of the rectangle cover
         */
        [[nodiscard]] Costs calculate_cost_of_cover(const std::vector<Rectangle> &cover) const {
            return calculate_cost_of_cover(cover, costs);
        }

        /**
         * Returns a struct containing the creation cost and area cost of the rectangles in a cover.
         *
         * @param cover The cover to calculate the cost of
         * @param costs The costs associated with the problem instance
         * @return Struct containing creation cost and area cost of the cover
         */
        static Costs calculate_cost_of_cover(const std::vector<Rectangle> &cover, const Costs &costs);

        /**
         * Returns a struct containing the creation cost and area cost of a rectangle.
         *
         * @param rectangle The rectangle to calculate the cost of
         * @param costs The costs associated with the problem instance
         * @return Struct containing creation cost and area cost of the cover
         */
        static Costs calculate_cost_of_rectangle(const Rectangle &rectangle, const Costs &costs) {
            return {costs.creation_cost, costs.area_cost * rectangle.area()};
        }

        /**
         * Returns the total cost of the passed cover.
         *
         * @param cover The cover to calculate the total cost of
         * @param costs The costs associated with the problem instance
         * @return The total cost of the cover
         */
        static CostType calculate_total_cost_of_cover(const std::vector<Rectangle> &cover, const Costs &costs) {
            return calculate_cost_of_cover(cover, costs).sum();
        }

        /**
         * Returns the total cost of the passed rectangle.
         *
         * @param rectangle The rectangle to calculate the total cost of
         * @param costs The costs associated with the problem instance
         * @return The total cost of the rectangle
         */
        static CostType calculate_total_cost_of_rectangle(const Rectangle &rectangle, const Costs &costs) {
            return calculate_cost_of_rectangle(rectangle, costs).sum();
        }

    protected:
        /**
         * Converts the WKT file at the passed path into a deque of polygons.
         *
         * @param wkt_path The path to the WKT file
         * @return The deque of polygons contained in the WKT file
         */
        static MultiPolygon convert_wkt_to_multi_polygon(const fs::path &wkt_path);
    };

} // cover

#endif //COVERING_PROBLEM_INSTANCE_H
