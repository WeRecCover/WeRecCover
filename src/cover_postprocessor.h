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

#ifndef COVER_POSTPROCESSOR_H
#define COVER_POSTPROCESSOR_H

#include "cover_provider.h"
#include "algorithm.h"
#include "datastructures.h"
#include "rectangle_enumerator.h"

namespace cover {

    /**
     * @brief Abstract class representing a cover postprocessor
     *
     * A postprocessor can be wrapped around either an Algorithm or another Cover_postprocessor, this way multiple
     * postprocessors can be chained after each other to achieve more complex behavior.
     */
    class Cover_postprocessor : public Cover_provider {
    private:
        std::unique_ptr<Cover_postprocessor> previous_postprocessor;
        std::unique_ptr<Algorithm> underlying_algorithm;

    protected:
        /**
         * Pure virtual function for postprocessing a cover for a given polygon with given costs
         *
         * The covered_points parameter can be passed along between postprocessors to avoid redundant computations,
         * since multiple postprocessors use this parameter.
         *
         * @param cover The cover to postprocess
         * @param polygon The polygon the cover was calculated for
         * @param costs The costs associated with the problem instance
         * @param covered_points Optional map of how many rectangles in the cover cover each point of the polygon
         */
      virtual void postprocess_cover(
          Cover &cover, const Polygon_with_holes &polygon,
          const Problem_instance::Costs &costs, Runtime_environment *env,
          std::optional<Map<Point, size_t>> &covered_points) const = 0;

      /**
       * Wrapper function which retrieves the cover from the previous
       * postprocessor/the underlying algorithm and then postprocesses it.
       *
       * @param polygon The polygon associated with the problem instance
       * @param costs The costs associated with the problem instance
       * @param covered_points Optional map of how many rectangles in the cover
       * cover each point of the polygon
       * @return The postprocessed cover
       */
      virtual Cover get_post_processed_cover_for(
          const Polygon_with_holes &polygon,
          const Problem_instance::Costs &costs, Runtime_environment *env,
          std::optional<Map<Point, size_t>> &covered_points) const;

      /**
       * Function which retrieves the cover calculated by the previous
       * postprocessor/the underlying algorithm.
       *
       * @param polygon The polygon associated with the problem instance
       * @param costs The costs associated with the problem instance
       * @param covered_points Optional map of how many rectangles in the cover
       * cover each point of the polygon
       * @return The cover calculated by the previous postprocessor/underlying
       * algorithm
       */
      Cover get_previous_cover_for(
          const Polygon_with_holes &polygon,
          const Problem_instance::Costs &costs, Runtime_environment *env,
          std::optional<Map<Point, size_t>> &covered_points) const;

      static std::vector<Rectangle>& get_or_calculate_brs(
              const Polygon_with_holes &polygon,
              Runtime_environment *env);

      static std::vector<size_t>& get_or_calculate_br_coverage(
              const Polygon_with_holes &polygon,
              const Cover& cover,
              Runtime_environment *env);

      static BaseRectGraph& get_or_calculate_br_graph(
              const Polygon_with_holes &polygon,
              Runtime_environment *env);

    public:
      /**
       * Returns postprocessed cover for the given polygon with the given costs.
       *
       * @param polygon The polygon associated with the problem instance
       * @param costs The costs associated with the problem instance
       * @return The postprocessed cover for the polygon and costs
       */
      [[nodiscard]] Cover get_cover_for(const Polygon_with_holes &polygon,
                                        const Problem_instance::Costs &costs,
                                        Runtime_environment *env) override;

      /**
       * Constructor taking a previous postprocessor which will be called before
       * this one.
       *
       * @param previous_postprocessor Postprocessor to retrieve cover for this
       * postprocessor from
       */
      explicit Cover_postprocessor(
          std::unique_ptr<Cover_postprocessor> previous_postprocessor)
          : previous_postprocessor(std::move(previous_postprocessor)),
            underlying_algorithm(nullptr){};

      /**
       * Constructor taking the underlying algorithm to retrieve the cover from.
       *
       * @param underlying_algorithm Algorithm to retrieve initial cover from
       */
      explicit Cover_postprocessor(
          std::unique_ptr<Algorithm> underlying_algorithm)
          : previous_postprocessor(nullptr),
            underlying_algorithm(std::move(underlying_algorithm)){};
    };

} // cover

#endif //COVER_POSTPROCESSOR_H
