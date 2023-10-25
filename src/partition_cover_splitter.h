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

#ifndef COVERING_PARTITION_COVER_SPLITTER_H
#define COVERING_PARTITION_COVER_SPLITTER_H

#include "logging.h"

#include "CGAL_classes.h"
#include "cover_splitter.h"
#include "partition_algorithm.h"

namespace cover {
    /**
     * @brief Partition cover splitter which splits a rectangle into its polygonal gaps then partitions them
     *
     * This class is part of a postprocessor for a cover with overlapping rectangles, which considers removing
     * a rectangle from the cover and covering the gaps left by it by partitioning them into smaller rectangles.
     *
     * This uses the Partition_algorithm class under the hood for the partitioning step.
     */
    class Partition_cover_splitter : public Cover_splitter {
    protected:
        /**
         * Splits the rectangle into polygonal gaps produced by removing the rectangle from the cover, then covers
         * them by partitioning them into rectangles.
         *
         * @param rectangle The rectangle to split
         * @param costs The costs of the problem instance
         * @return The rectangle partition of the gaps produced by removing the rectangle
         */
      std::vector<Rectangle>
      split_rectangle(const cover::Rectangle &rectangle,
                      const Problem_instance::Costs &costs,
                      Runtime_environment *env) const override;

    public:
        using Cover_splitter::Cover_splitter;
    };

} // cover

#endif //COVERING_PARTITION_COVER_SPLITTER_H
