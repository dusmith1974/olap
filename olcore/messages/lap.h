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

// Contains a class representing a competitors lap.

#ifndef MESSAGES_LAP_H_
#define MESSAGES_LAP_H_

#include "lap_fwd.h"
#include "messages/message.h"

namespace olap {

// The lap message.
class Lap final : public Message {
 public:
  Lap();
  virtual ~Lap();

  Message* Clone() const;

  operator std::string() const;

  int num() const;
  void set_num(int val);

  Interval gap() const;
  void set_gap(Interval val);

  int competitor_num() const;

  LongInterval time() const;

  operator Interval() const;
  operator LongInterval() const;

 private:
  friend std::istream& operator>>(std::istream& is, Lap& lap);

  void Print(std::ostream& os) const override;

  int num_;
  int competitor_num_;
  Interval gap_;
  LongInterval time_;
};

inline bool operator<(const Lap& lhs, const Lap& rhs) {
  return lhs.num() < rhs.num();
}

inline bool operator>(const Lap& lhs, const Lap& rhs) {
  return rhs < lhs;
}

inline bool operator<=(const Lap& lhs, const Lap& rhs) {
  return !(lhs > rhs);
}

inline bool operator>=(const Lap& lhs, const Lap& rhs) {
  return !(lhs < rhs);
}

inline bool operator==(const Lap& lhs, const Lap& rhs) {
  return lhs.num() == rhs.num();
}

inline bool operator!=(const Lap& lhs, const Lap& rhs) {
  return !(lhs == rhs);
}
}  // namespace olap

#endif  // MESSAGES_LAP_H_
