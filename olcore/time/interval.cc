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

#include "interval.h"

namespace olap {

Interval::Interval() : milliseconds_(0) {
}

Interval::Interval(long val) : milliseconds_(val) {
}

Interval::Interval(const std::chrono::milliseconds& val) : milliseconds_(val) {
}

Interval::~Interval() {
}

const std::string Interval::str() const { return str_; }

Interval::operator long() const { return milliseconds_.count(); }
Interval::operator std::chrono::milliseconds() const { return milliseconds_; }

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
  interval.milliseconds_ = std::chrono::milliseconds();

  boost::smatch m;
  if (boost::regex_search(str, m, boost::regex(R"((\d+):(\d+)[\.:](\d+))"))) {
    if (std::count(str.begin(), str.end(), ':') > 1) {
      interval.milliseconds_ += std::chrono::hours(
          boost::lexical_cast<int>(m[1].str()));

      interval.milliseconds_ += std::chrono::minutes(
          boost::lexical_cast<int>(m[2].str()));

      interval.milliseconds_ += std::chrono::seconds(
          boost::lexical_cast<int>(m[3].str()));
    } else {
      interval.milliseconds_ += std::chrono::minutes(
          boost::lexical_cast<int>(m[1].str()));

      interval.milliseconds_ += std::chrono::seconds(
          boost::lexical_cast<int>(m[2].str()));

      interval.milliseconds_ += std::chrono::milliseconds(
          boost::lexical_cast<int>(m[3].str()));
    }
  } else if (boost::regex_search(str, m, boost::regex(R"((\d+)\.(\d+))"))) {
    interval.milliseconds_ += std::chrono::seconds(
        boost::lexical_cast<int>(m[1].str()));

    interval.milliseconds_ += std::chrono::milliseconds(
        boost::lexical_cast<int>(m[2].str()));
  } else {
    interval.str_ = str;  // 1 LAP, 2 LAPS etc.
  }

  return is;
}

std::ostream& operator<<(std::ostream& os, const Interval& interval) {
  if (!interval.str_.empty()) {
    os << interval.str_;
    return os;
  }

  os << std::chrono::duration_cast<
    std::chrono::seconds>(interval.milliseconds_).count();

  os << "." << std::setw(3) << std::setfill('0')
     << std::chrono::duration_cast<std::chrono::milliseconds>(
      interval.milliseconds_ % std::chrono::seconds(1)).count();

  return os;
}
}  // namespace
