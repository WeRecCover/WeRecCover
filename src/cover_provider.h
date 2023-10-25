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

#ifndef COVERING_COVER_PROVIDER_H
#define COVERING_COVER_PROVIDER_H

#include "logging.h"

#include "CGAL_classes.h"
#include "rectangle.h"
#include "instance.h"
#include "baserect_graph.h"
#include "runtime_environment.h"

namespace cover {

    /**
     * @brief Abstract class representing a class that can provide a rectangle cover for a polygon
     */
    class Cover_provider {
    public:
        using Cover = std::vector<Rectangle>;

        /**
         * Pure virtual function returning a cover for the given polygon with the given associated costs.
         *
         * @param polygon The polygon to cover
         * @param costs The costs associated with the problem instance
         * @return A vector of rectangles which covers the polygon
         */
        [[nodiscard]] virtual Cover
        get_cover_for(const Polygon_with_holes &polygon,
                      const Problem_instance::Costs &costs,
                      Runtime_environment *env) = 0;

        [[nodiscard]] virtual bool timeouted() const { return false; }

        virtual ~Cover_provider() = default;
    };
}

#endif //COVERING_COVER_PROVIDER_H
