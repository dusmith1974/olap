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

// Implements the Pit class.

#include "messages/pit.h"

#include "boost/lexical_cast.hpp"

namespace olap {

Pit::Pit()
  : competitor_num_(0),
    lap_num_(0),
    num_(0) {
}

Pit::~Pit() {
}

Message* Pit::Clone() const { return new Pit(*this); }

Pit::operator std::string() const {
  std::stringstream ss;
  ss << *this;

  return ss.str();
}

int Pit::lap_num() const { return lap_num_; }
void Pit::set_lap_num(int val) { lap_num_ = val; }


void Pit::Print(std::ostream& os) const {
  os << "pit," << static_cast<LongInterval>(race_time_) << ","
    << time_of_day_ << "," << competitor_num_<< "," << lap_num_ << "," << num_
    << std::endl;
}

std::istream& operator>>(std::istream& is, Pit& pit) {
  std::string str;

  is >> pit.competitor_num_, is.ignore();
  is >> pit.lap_num_, is.ignore();

  std::getline(is, str, ',');
  pit.time_of_day_ = boost::lexical_cast<LongInterval>(str);

  is >> pit.num_, is.ignore();

  std::getline(is, str, ',');
  std::getline(is, str, ',');

  return is;
}

}  // namespcae olap
