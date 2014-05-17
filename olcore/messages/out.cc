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

// Implements the Out class.

#include "messages/out.h"

namespace olap {

Out::Out()
  : competitor_num_(0),
    lap_num_(0),
    time_of_day_(LongInterval()),
    num_(0),
    time_(Interval()),
    total_time_(Interval()) {
}

Out::~Out() {
}

Message* Out::Clone() const { return new Out(*this); }

Out::operator std::string() const {
  std::stringstream ss;
  ss << *this;

  return ss.str();
}

Interval Out::time() const { return time_; }
LongInterval Out::time_of_day() const { return time_of_day_; }

int Out::lap_num() const { return lap_num_; }
void Out::set_lap_num(int val) { lap_num_ = val; }

void Out::Print(std::ostream& os) const {
  os << "out," << static_cast<LongInterval>(race_time_) << ","
    << time_of_day_ << "," << competitor_num_ << "," << lap_num_ << ","
    << num_ << "," << time_ << "," << total_time_ << std::endl;
}

std::istream& operator>>(std::istream& is, Out& out) {
  std::string str;

  is >> out.competitor_num_, is.ignore();
  is >> out.lap_num_, is.ignore();

  std::getline(is, str, ',');
  out.time_of_day_ = boost::lexical_cast<LongInterval>(str);

  is >> out.num_, is.ignore();

  std::getline(is, str, ',');
  out.time_ = boost::lexical_cast<LongInterval>(str);
  // TODO(ds) op+=
  out.time_of_day_ = LongInterval(out.time_of_day_ + out.time_);

  std::getline(is, str, ',');
  out.total_time_ = boost::lexical_cast<LongInterval>(str);

  //out.race_time_ = Interval(1);

  return is;
}
}  // namespcae olap
