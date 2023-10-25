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

#ifndef COVERING_LOGGING_H
#define COVERING_LOGGING_H

#ifdef COVER_MAX_LOG_LEVEL
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/attributes.hpp>

namespace logging = boost::log;
#endif

// SOURCE START: https://stackoverflow.com/a/69109527/6875882
// Based on this StackOverflow answer
#ifdef COVER_MAX_LOG_LEVEL
#define COVER_LOGGING_ENABLED
#define IF_LOG_LEVEL(lvl) \
    if constexpr(logging::trivial::lvl >= logging::trivial::COVER_MAX_LOG_LEVEL)
#define LOG(lvl) \
    IF_LOG_LEVEL(lvl) \
        BOOST_LOG_TRIVIAL(lvl)
#else
#define IF_LOG_LEVEL(lvl) \
    if constexpr(false)
#define LOG(lvl) \
    if constexpr(false) \
        std::cout
#endif
// SOURCE END

#endif //COVERING_LOGGING_H
