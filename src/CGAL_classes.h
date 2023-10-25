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

#ifndef COVERING_CGAL_CLASSES_H
#define COVERING_CGAL_CLASSES_H

#include <CGAL/Cartesian.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Ray_2.h>
#include <CGAL/Point_2.h>
#include <CGAL/Segment_2.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Direction_2.h>

namespace cover {
    using NumType = double;
    using Kernel = CGAL::Exact_predicates_inexact_constructions_kernel;

    using Point = CGAL::Point_2<Kernel>;
    using Polygon = CGAL::Polygon_2<Kernel>;
    using Polygon_with_holes = CGAL::Polygon_with_holes_2<Kernel>;
    using MultiPolygon = std::deque<Polygon_with_holes>;

    using Ray = CGAL::Ray_2<Kernel>;
    using Segment = CGAL::Segment_2<Kernel>;
    using Direction = CGAL::Direction_2<Kernel>;

    using ArrangementTraits = CGAL::Arr_segment_traits_2<Kernel>;
    using Arrangement = CGAL::Arrangement_2<ArrangementTraits>;
    using ArrangementSegment = ArrangementTraits::X_monotone_curve_2;
}

#endif //COVERING_CGAL_CLASSES_H
