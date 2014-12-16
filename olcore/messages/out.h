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

// Contains a class representing a Out.

#ifndef MESSAGES__OUT_H_
#define MESSAGES__OUT_H_

#include "messages/message.h"

namespace olap {

// The Out class.
class Out : public Message {
 public:
  Out();
  virtual ~Out();

  Message* Clone() const;

  operator std::string() const;

  Interval time() const;
  LongInterval time_of_day() const;

  int lap_num() const;
  void set_lap_num(int val);

 protected:
  int competitor_num_;
  int lap_num_;
  LongInterval time_of_day_;
  int num_;
  Interval time_;
  Interval total_time_;

 private:
  friend std::istream& operator>>(std::istream& is, Out& out);

  void Print(std::ostream& os) const override;
};

}  // namespace olap

#endif  // MESSAGES__OUT_H_
