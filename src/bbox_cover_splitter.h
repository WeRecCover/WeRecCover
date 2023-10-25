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

#ifndef COVERING_BOUNDING_BOX_COVER_SPLITTER_H
#define COVERING_BOUNDING_BOX_COVER_SPLITTER_H

#include "logging.h"

#include "CGAL_classes.h"
#include "cover_splitter.h"
#include "instance.h"
#include "datastructures.h"

namespace cover {

    /**
     * @brief Bounding box splitter which splits a rectangle into bounding boxes of its produced gaps
     *
     * This class is part of a postprocessor for a cover with overlapping rectangles, which considers removing
     * a rectangle from the cover and covering the gaps left by it with their bounding boxes.
     */
    class Bounding_box_cover_splitter : public Cover_splitter {
    public:
        /**
         * Splits the rectangle into bounding boxes of the gaps produced by removing the rectangle from the cover.
         *
         * @param rectangle The rectangle to split
         * @param costs The costs of the problem instance
         * @return The bounding boxes of the gaps in the form of a vector of rectangles
         */
      std::vector<Rectangle>
      split_rectangle(const cover::Rectangle &rectangle,
                      const Problem_instance::Costs &costs,
                      Runtime_environment *env) const override;

    public:
        using Cover_splitter::Cover_splitter;
    };

} // cover

#endif //COVERING_BOUNDING_BOX_COVER_SPLITTER_H
