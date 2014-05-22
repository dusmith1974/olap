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

// Contains a class representing a competitor.

#ifndef MESSAGES_COMPETITOR_H_
#define MESSAGES_COMPETITOR_H_

#include "messages/message.h"
#include "messages/competitor_fwd.h"

#include "boost/multiprecision/cpp_dec_float.hpp"

namespace olap {
namespace mp = boost::multiprecision;

typedef mp::number<mp::cpp_dec_float<3> > cpp_dec_float_3;

// The competitor message.
class Competitor final : public Message {
 public:
  Competitor();

  Message* Clone() const;

  operator std::string() const;

  int grid_pos() const;
  void set_grid_pos(int val);

  std::string name() const;
  int num() const;

  cpp_dec_float_3 sector_1_percent() const;
  void set_sector_1_percent(cpp_dec_float_3 val);

  cpp_dec_float_3 sector_2_percent() const;
  void set_sector_2_percent(cpp_dec_float_3 val);

  cpp_dec_float_3 sector_3_percent() const;
  void set_sector_3_percent(cpp_dec_float_3 val);

 private:
  friend std::istream& operator>>(std::istream& is, Competitor& competitor);

  void Print(std::ostream& os) const override;

  int num_;
  int grid_pos_;
  std::string short_name_;
  std::string name_;
  std::string team_;
  cpp_dec_float_3 sector_1_percent_;
  cpp_dec_float_3 sector_2_percent_;
  cpp_dec_float_3 sector_3_percent_;
};

}  // namespace

#endif // MESSAGES_COMPETITOR_H_
