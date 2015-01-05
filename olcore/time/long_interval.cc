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

// Implements the LongInterval class.

#include "olcore_pch.h"  // NOLINT

#include "time/long_interval.h"

#include <iomanip>  // NOLINT
#include <iostream>  // NOLINT

namespace olap {
  LongInterval::LongInterval() : Interval(0) {
  }

  LongInterval::LongInterval(int64_t val) : Interval(val) {
  }

  std::ostream& operator<<(std::ostream& os,
                           const LongInterval& long_interval) {  // NOLINT
    using boost::chrono::hours;
    hours the_hours = boost::chrono::duration_cast<boost::chrono::hours>(
      long_interval.milliseconds_);

    if (the_hours.count()) {
      os << the_hours.count() << ":";
      os << std::setw(2) << std::setfill('0');
    }

    os << boost::chrono::duration_cast<
      boost::chrono::minutes>((the_hours.count())
      ? long_interval.milliseconds_ % boost::chrono::hours(1)
      : long_interval.milliseconds_).count();

    os << ":" << std::setw(2) << std::setfill('0')
      << boost::chrono::duration_cast<boost::chrono::seconds>(
      long_interval.milliseconds_ % boost::chrono::minutes(1)).count();

    os << "." << std::setw(3)
      << boost::chrono::duration_cast<boost::chrono::milliseconds>(
      long_interval.milliseconds_ % boost::chrono::seconds(1)).count();

    return os;
  }
}  // namespace olap
