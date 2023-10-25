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

#ifndef COVERING_COVER_SPLITTER_H
#define COVERING_COVER_SPLITTER_H

#include <CGAL/Boolean_set_operations_2.h>

#include "logging.h"

#include "CGAL_classes.h"
#include "cover_postprocessor.h"
#include "rectangle.h"
#include "instance.h"
#include "datastructures.h"

namespace cover {

    /**
     * @brief Abstract class representing a postprocessor which splits rectangles into smaller rectangles
     */
    class Cover_splitter : public Cover_postprocessor {
    protected:
        /**
         * Iterates the rectangles in the cover and considers splitting them. Calls split_rectangle on each rectangle
         * in the cover and calculates whether the resulting split reduces the overall cost of the cover. If this is
         * the case, the rectangle is replaced by its split, otherwise we continue with the next rectangle.
         *
         * @param cover The cover to consider splits for
         * @param polygon The polygon associated with the problem instance
         * @param costs The costs associated with the problem instance
         * @param covered_points Map of how many rectangles in the cover cover each point of the polygon, used to
         * calculate the uniquely covered points of each rectangle
         */
      void postprocess_cover(
          Cover &cover, const Polygon_with_holes &polygon,
          const Problem_instance::Costs &costs, Runtime_environment *env,
          std::optional<Map<Point, size_t>> &covered_points) const override;

      /**
       * Pure virtual function which splits a rectangle into one or more other
       * rectangles, which are then considered by postprocess_cover.
       *
       * @param rectangle The rectangle to split
       * @param costs The costs associated with the problem instance
       * @param covered_points Map of how many rectangles in the cover cover
       * each point of the polygon
       * @return The proposed split for the rectangle
       */
      virtual std::vector<Rectangle>
      split_rectangle(const Rectangle &rectangle,
                      const Problem_instance::Costs &costs,
                      Runtime_environment *env) const = 0;

        static void
        reduce_covered_amount(const Polygon_with_holes &polygon,
                              const Rectangle &original_rectangle,
                              const std::vector<Rectangle> &split_rectangle,
                              Runtime_environment *env);

        static std::vector<Polygon_with_holes> 
        split_into_polygons(const Rectangle &rectangle,
                            Runtime_environment *env);

        static std::vector<BaseRectNode::PtrType>
        get_uniquely_covered_brs(const Rectangle &rectangle,
                                 Runtime_environment *env);

    public:
        using Cover_postprocessor::Cover_postprocessor;
    };

} // cover

#endif //COVERING_COVER_SPLITTER_H
