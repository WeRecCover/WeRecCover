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

#ifndef COVERING_ILP_ALGORITHM_H
#define COVERING_ILP_ALGORITHM_H

#ifdef GUROBI_AVAILABLE  // do not compile this if gurobi is not available

#include <boost/chrono.hpp>

#include <gurobi_c++.h>

#include "logging.h"

#include "CGAL_classes.h"
#include "rectangle_enumerator.h"
#include "rectangle.h"
#include "algorithm.h"

namespace cover {

    /**
     * @brief Algorithm which provides exact solutions for problem instances via an ILP formulation using Gurobi
     */
    class ILP_algorithm : public Algorithm {
    private:
        bool use_pixels;
        bool timeout_reached {false};
        GRBEnv env{true};
    protected:
        /**
         * Constructs the model to be solved by Gurobi from the provided base rectangles of the polygon,
         * the set of possible rectangles made up of base rectangles that can fit inside the polygon and the
         * costs associated with the problem instance.
         *
         * The model has one variable per possible rectangle and one constraint per base rectangle.
         *
         * @param base_rectangles The base rectangles of the polygon
         * @param cover_rectangles The possible rectangles made up of base rectangles that can fit inside the polygon
         * @param costs The costs associated with the problem instance
         * @return The ILP model
         */
        void construct_model(const std::vector<Rectangle> &base_rectangles,
                                               const std::vector<Rectangle> &cover_rectangles,
                                               const Problem_instance::Costs &costs,
                                               GRBModel &model,
                                               std::vector<GRBVar> &variables) const;

        /**
         * Calculates an exact solution for the polygon and costs using an ILP formulation.
         *
         * @param polygon The polygon to cover
         * @param costs The costs associated with the problem instance
         * @return An optimal cover for the polygon and costs
         */
        [[nodiscard]] std::vector<Rectangle>
        calculate_cover(const Polygon_with_holes &polygon,
                        const Problem_instance::Costs &costs,
                        Runtime_environment *rtenv) override;

      public:
        /**
         * Constructor which allows specifying that pixels should be used instead of base rectangles. Essentially,
         * this will cause the ILP to treat every unit square which fits in the polygon as a base rectangle, meaning
         * all possible rectangles with integer coordinates which can fit inside the polygon will be considered. This
         * can severely degrade performance and is only supported for experimental purposes.
         *
         * @param use_pixels Whether to use pixels as base rectangles or not, default is no
         */
        explicit ILP_algorithm(bool use_pixels = false, double timeout = 0)
                : use_pixels(use_pixels) {
            env.set(GRB_IntParam_LogToConsole, 0);
            env.set(GRB_DoubleParam_MIPGap, 0.0);
            if (timeout > 0) {
                env.set(GRB_DoubleParam_TimeLimit, timeout);
            }
            env.start();
        }

        [[nodiscard]] virtual bool timeouted() const override { return timeout_reached; }
    };

} // cover

#endif //GUROBI_AVAILABLE
#endif //COVERING_ILP_ALGORITHM_H
