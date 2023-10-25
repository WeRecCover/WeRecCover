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

#ifdef GUROBI_AVAILABLE  // do not compile this if gurobi is not available

#include "ILP_algorithm.h"

namespace cover {
        using clock = std::chrono::high_resolution_clock;
        using nanos = std::chrono::nanoseconds;
        using millis = std::chrono::milliseconds;

    void ILP_algorithm::construct_model(const std::vector<Rectangle> &base_rectangles,
                                            const std::vector<Rectangle> &cover_rectangles,
                                            const Problem_instance::Costs &costs,
                                            GRBModel &model,
                                            std::vector<GRBVar> &variables) const {
        LOG(debug) << "Constructing primal ILP model";
        model.setObjective(GRBLinExpr{}, GRB_MINIMIZE);

        LOG(trace) << "Constructing model variables";
        variables.reserve(cover_rectangles.size());
        size_t cover_rectangle_index{0};

        for (const auto &cover_rectangle: cover_rectangles) {
            const auto cost{Problem_instance::calculate_total_cost_of_rectangle(cover_rectangle, costs)};
            const auto var{model.addVar(0.0, 1.0, cost, GRB_BINARY)};

            LOG(trace) << "Constructed model variable " << cost << " * c" << cover_rectangle_index++ << " for rectangle "
                       << cover_rectangle.as_polygon();

            variables.push_back(var);
        }

        LOG(trace) << "Constructing model constraints";
        std::vector<GRBVar> covering_rectangles{};
        covering_rectangles.reserve(cover_rectangles.size());
        for (const auto &base_rectangle: base_rectangles) {
            LOG(trace) << "Constructing constraint for base rectangle " << base_rectangle.as_polygon();

            std::string constraint_as_string{};

            covering_rectangles.clear();
            for (size_t i = 0; i < cover_rectangles.size(); i++) {
                const auto &cover_rectangle = cover_rectangles[i];
                if (cover_rectangle.fully_contains(base_rectangle)) {
                    const auto &rectangle_variable{variables.at(i)};
                    covering_rectangles.push_back(rectangle_variable);

                    IF_LOG_LEVEL(trace) {
                        if (!constraint_as_string.empty()) {
                            constraint_as_string.append(" + ");
                        }
                        constraint_as_string.append('c' + std::to_string(i));
                    }
                }
            }

            std::vector<double> coefficients(covering_rectangles.size(), 1.0);

            GRBLinExpr constraint{};
            constraint.addTerms(&coefficients[0], &covering_rectangles[0], coefficients.size());

            model.addConstr(constraint >= 1);

            IF_LOG_LEVEL(trace) {
                LOG(trace) << "Constructed constraint: " << constraint_as_string << " >= 1";
            }
        }
    }

    std::vector<Rectangle>
    ILP_algorithm::calculate_cover(const Polygon_with_holes &polygon,
                                   const Problem_instance::Costs &costs,
                                   Runtime_environment *rtenv) {
      timeout_reached = false;
      LOG(info) << "Running ILP_algorithm";

      if (rtenv->base_rectangles.empty()) {
        rtenv->base_rectangles = Rectangle_enumerator::get_base_rectangles(polygon);
      }
      auto base_rectangles{rtenv->base_rectangles};

      if (use_pixels) {
        LOG(warning) << "Using ILP in Pixels mode, do not use this outside of "
                        "test cases as it may be "
                        "extremely slow on large instances and is only "
                        "available for testing purposes";
        // use pixels as basis rectangles instead, used solely to show
        // qualitative equivalence of basis rectangle and pixel solution in a
        // test case, not used in actual experiments
        std::vector<Rectangle> pixel_rectangles{};
        for (const auto &base_rectangle : base_rectangles) {
          for (const auto &pixel : base_rectangle.get_covered_points()) {
            pixel_rectangles.emplace_back(Point(pixel.x(), pixel.y() + 1));
          }
        }
        base_rectangles = pixel_rectangles;
      }

      const auto cover_rectangles{
          Rectangle_enumerator::enumerate_rectangles(base_rectangles)};

      GRBModel model{env};
      std::vector<GRBVar> variables;
      construct_model(base_rectangles, cover_rectangles, costs, model,
                      variables);

      LOG(debug) << "Optimizing ILP model with Gurobi";
      model.optimize();

      auto status = model.get(GRB_IntAttr_Status);
      LOG(info) << "ILP finished with status code " << status;

      std::vector<Rectangle> cover{};
      if (status == GRB_OPTIMAL) {
        for (size_t i = 0; i < variables.size(); i++) {
          if (round(variables[i].get(GRB_DoubleAttr_X)) == 1) {
            LOG(trace) << "Rectangle " << cover_rectangles[i].as_polygon()
                       << " was picked by ILP, adding to cover";
            cover.push_back(cover_rectangles[i]);
          }
        }
      } else if (status == GRB_TIME_LIMIT) {
        timeout_reached = true;
      } else {
        std::cerr << "ILP finished with status code " << status << std::endl;
      }
      return cover;
    }
} // cover

#endif //GUROBI_AVAILABLE
