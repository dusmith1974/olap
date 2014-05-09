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

#include "messages/competitor.h"

namespace olap {

Competitor::Competitor()
  : num_(0),
    grid_pos_(0),
    short_name_(""),
    name_(""),
    team_("") {
}

Message* Competitor::Clone() const { return new Competitor(*this); }

Competitor::operator std::string() const {
  std::stringstream ss;
  ss << *this;

  return ss.str();
}

int Competitor::grid_pos() const { return grid_pos_; }
void Competitor::set_grid_pos(int val) { grid_pos_ = val; }

std::string Competitor::name() const { return name_; }
int Competitor::num() const { return num_; }

cpp_dec_float_3 Competitor::sector_1_percent() const { return sector_1_percent_; }
void Competitor::set_sector_1_percent(cpp_dec_float_3 val) { sector_1_percent_ = val; }

cpp_dec_float_3 Competitor::sector_2_percent() const { return sector_2_percent_; }
void Competitor::set_sector_2_percent(cpp_dec_float_3 val) { sector_2_percent_ = val; }

cpp_dec_float_3 Competitor::sector_3_percent() const { return sector_3_percent_; }
void Competitor::set_sector_3_percent(cpp_dec_float_3 val) { sector_3_percent_ = val; }

void Competitor::Print(std::ostream& os) const {
  os << "com," << "0,0," << num_ << "," << grid_pos_ << "," << short_name_
    << "," << name_ << "," << team_ << std::endl;
}

std::istream& operator>>(std::istream& is, Competitor& competitor) {
  is >> competitor.num_;
  is.ignore();

  std::getline(is, competitor.short_name_, ',');
  std::getline(is, competitor.name_, ',');
  std::getline(is, competitor.team_, ',');

  return is;
}
}  // namespace olap
