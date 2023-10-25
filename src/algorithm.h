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

#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "instance.h"
#include "cover_provider.h"

namespace cover {

    /**
     * @brief Abstract class representing an algorithm which returns an initial cover for a given polygon
     *
     * An abstract class representing an algorithm which covers an object of class Problem_instance
     * with rectangles. The rectangles may overlap, their union must be exactly equal to the underlying
     * MultiPolygon of the problem instance.
     */
    class Algorithm : public Cover_provider {
    protected:
        /**
         * Virtual function which takes in a polygon and costs and returns a valid rectangle covering of the
         * given instance in the form of a vector of rectangles.
         *
         * @param polygon The polygon to cover
         * @param costs The costs associated with the problem instance
         * @return A vector containing rectangles which cover the passed polygon
         */
      [[nodiscard]] virtual Cover
      calculate_cover(const Polygon_with_holes &polygon,
                      const Problem_instance::Costs &costs,
                      Runtime_environment *env) = 0;

    public:
        /**
         * Wrapper function which simply calls the internal function calculating a cover for the passed polygon
         * with the passed costs. A vector of rectangles which covers the polygon is returned.
         *
         * @param polygon The polygon to cover
         * @param costs The costs associated with the problem instance
         * @return A vector containing rectangles which cover the passed polygon
         */
      [[nodiscard]] Cover
      get_cover_for(const cover::Polygon_with_holes &polygon,
                    const Problem_instance::Costs &costs,
                    Runtime_environment *env) override {
        return calculate_cover(polygon, costs, env);
      }
    };

} // cover

#endif //ALGORITHM_H
