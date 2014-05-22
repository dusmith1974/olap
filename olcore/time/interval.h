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

// Contains the Interval class.

#ifndef TIME_INTERVAL_H_
#define TIME_INTERVAL_H_

#include <chrono>
#include <iosfwd>
#include <string>

namespace olap {

// Represents a timespan.
class Interval {
 public:
  Interval();
  virtual ~Interval();

  explicit Interval(long val);
  explicit Interval(const std::chrono::milliseconds& val);

  const std::string str() const;

  operator long() const;
  operator std::chrono::milliseconds() const;

  const Interval& operator+=(const Interval& interval);

 protected:
  std::chrono::milliseconds milliseconds_;
  std::string str_;

 private:
  friend std::istream& operator>>(std::istream& is, Interval& interval);
  friend std::ostream& operator<<(std::ostream& os, const Interval& interval);
};

}  // namespace olap

#endif  // TIME_INTERVAL_H_
