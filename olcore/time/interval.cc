// Copyright 2014 Duncan Smith
// https://github.com/dusmith1974/olap
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Implements the Interval class.

#include "olcore_pch.h"  // NOLINT

#include "time/interval.h"

#include <iomanip>  // NOLINT
#include <iostream>  // NOLINT

#include "boost/lexical_cast.hpp"
#include "boost/regex.hpp"

namespace olap {
Interval::Interval()
  : milliseconds_(0),
    str_{} {
}

Interval::Interval(int64_t val)
  : milliseconds_(val),
    str_{} {
}

Interval::Interval(const boost::chrono::milliseconds& val)
  : milliseconds_(val),
    str_{} {
}

Interval::~Interval() {
}

const std::string& Interval::str() const { return str_; }

Interval::operator int64_t() const {
  return static_cast<int64_t>(milliseconds_.count());
}

Interval::operator boost::chrono::milliseconds() const {
  return milliseconds_;
}

const Interval& Interval::operator+=(const Interval& interval) {
  milliseconds_ += interval.milliseconds_;
  return *this;
}

// TODO(ds) a + b requires cast to Interval() ?
inline Interval operator+(Interval lhs, const Interval& rhs) {
  lhs += rhs;
  return lhs;
}

// TODO(ds) break up into HMS MSm Sm functions?
std::istream& operator>>(std::istream& is, Interval& interval) {
  std::string str;
  std::getline(is, str);

  interval.str_ = "";
  interval.milliseconds_ = boost::chrono::milliseconds();

  boost::smatch m;
  if (boost::regex_search(str, m, boost::regex(R"((\d+):(\d+)[\.:](\d+))"))) {
    if (std::count(str.begin(), str.end(), ':') > 1) {
      interval.milliseconds_ += boost::chrono::hours(
                                  boost::lexical_cast<int>(m[1].str()));

      interval.milliseconds_ += boost::chrono::minutes(
                                  boost::lexical_cast<int>(m[2].str()));

      interval.milliseconds_ += boost::chrono::seconds(
                                  boost::lexical_cast<int>(m[3].str()));
    } else {
      interval.milliseconds_ += boost::chrono::minutes(
                                  boost::lexical_cast<int>(m[1].str()));

      interval.milliseconds_ += boost::chrono::seconds(
                                  boost::lexical_cast<int>(m[2].str()));

      interval.milliseconds_ += boost::chrono::milliseconds(
                                  boost::lexical_cast<int>(m[3].str()));
    }
  } else if (boost::regex_search(str, m, boost::regex(R"((\d+)\.(\d+))"))) {
    interval.milliseconds_ += boost::chrono::seconds(
                                boost::lexical_cast<int>(m[1].str()));

    interval.milliseconds_ += boost::chrono::milliseconds(
                                boost::lexical_cast<int>(m[2].str()));
  } else {
    interval.str_ = str;  // 1 LAP, 2 LAPS etc.
  }

  return is;
}

std::ostream& operator<<(std::ostream& os,
                         const Interval& interval) {  // NOLINT
  if (!interval.str_.empty()) {
    os << interval.str_;
    return os;
  }

  os << boost::chrono::duration_cast<
     boost::chrono::seconds>(interval.milliseconds_).count();

  os << "." << std::setw(3) << std::setfill('0')
     << boost::chrono::duration_cast<boost::chrono::milliseconds>(
       interval.milliseconds_ % boost::chrono::seconds(1)).count();

  return os;
}
}  // namespace olap
