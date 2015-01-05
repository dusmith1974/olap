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

// Implements the lap message.

#include "olcore_pch.h"  // NOLINT

#include "messages/lap.h"

#include "boost/lexical_cast.hpp"
#include "boost/regex.hpp"

namespace olap {
  Lap::Lap()
    : num_(0),
    competitor_num_(0),
    gap_(Interval()),
    time_(LongInterval()) {
  }

  Lap::~Lap() {
  }

  Message* Lap::Clone() const { return new Lap(*this); }

  Lap::operator std::string() const {
    std::stringstream ss;
    ss << *this;

    return ss.str();
  }

  int Lap::num() const { return num_; }
  void Lap::set_num(int val) { num_ = val; }

  Interval Lap::gap() const { return gap_; }
  void Lap::set_gap(Interval val) { gap_ = val; }

  int Lap::competitor_num() const { return competitor_num_; }

  LongInterval Lap::time() const { return time_; }

  Lap::operator Interval() const { return gap_; }
  Lap::operator LongInterval() const { return time_; }

  void Lap::Print(std::ostream& os) const {  // NOLINT
    os << "lap," << static_cast<LongInterval>(race_time_)
      << "," << time_of_day_ << "," << competitor_num_
      << "," << num_ << "," << gap_ << "," << time_ << std::endl;
  }

  std::istream& operator>>(std::istream& is, Lap& lap) {
    std::string str;
    std::getline(is, str);

    boost::smatch m;
    using boost::regex;
    using boost::regex_search;
    using boost::lexical_cast;

    if (regex_search(str, m, regex(R"(^\d+)")))
      lap.competitor_num_ = lexical_cast<int>(m.str());

    if (regex_search(str, m, regex(R"(^\d+\s(\d+)\s(?!LAP))")))
      lap.num_ = lexical_cast<int>(m[1].str());

    if (regex_search(str, m, regex(R"(((?<=\s)\d+\.\d+)|(\d+ LAPS?)|(PIT))")))
      lap.gap_ = lexical_cast<Interval>(m.str());
    else
      lap.gap_ = Interval();

    if (regex_search(str, m, regex(R"((\d+:)\d+[\.:]\d+)")))
      lap.time_ = lexical_cast<LongInterval>(m.str());

    return is;
  }
}  // namespace olap
