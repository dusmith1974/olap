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

// Implements the sector message.

#include "messages/sector.h"

namespace olap {

Sector::Sector()
  : num_(0),
    competitor_num_(0),
    lap_num_(0),
    time_(Interval()) {
}

Sector::Sector(int num, int competitor_num, int lap_num, long time)
  : num_(num),
    competitor_num_(competitor_num),
    lap_num_(lap_num),
    time_(time) {
}

Sector::~Sector() {
}

Message* Sector::Clone() const { return new Sector(*this); }

Sector::operator std::string() const {
  std::stringstream ss;
  ss << *this;

  return ss.str();
}

int Sector::num() const { return num_; }
void Sector::set_num(int val) { num_ = val; }

int Sector::competitor_num() const { return competitor_num_; }

Interval Sector::time() const { return time_; }

Sector::operator Interval() const { return time_; };

void Sector::Print(std::ostream& os) const {
  os << "sec," << static_cast<LongInterval>(race_time_)
    << "," << time_of_day_ << "," << competitor_num_ << "," << lap_num_
    << "," << num_ << "," << time_ << std::endl;
}

inline bool operator<(const Sector& lhs, const Sector& rhs) {
  return lhs.num() < rhs.num();
}

inline bool operator>(const Sector& lhs, const Sector& rhs) {
  return rhs < lhs;
}

inline bool operator<=(const Sector& lhs, const Sector& rhs) {
  return !(lhs > rhs);
}

inline bool operator>=(const Sector& lhs, const Sector& rhs) {
  return !(lhs < rhs);
}

inline bool operator==(const Sector& lhs, const Sector& rhs) {
  return lhs.num() == rhs.num();
}

inline bool operator!=(const Sector& lhs, const Sector& rhs) {
  return !(lhs == rhs);
}

}  // namespcae olap
