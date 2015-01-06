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

// Contains a class representing a Pit.

#ifndef OLCORE_MESSAGES_PIT_H_
#define OLCORE_MESSAGES_PIT_H_

#include <string>

#include "messages/message.h"
#include "messages/pit_fwd.h"

namespace olap {
// The Pit class.
class Pit : public Message {
 public:
  Pit();

  virtual ~Pit();

  Message* Clone() const;

  operator std::string() const;

  int lap_num() const;
  void set_lap_num(int val);

 protected:
  int competitor_num_;
  int lap_num_;
  int num_;

 private:
  friend std::istream& operator>>(std::istream& is, Pit& pit);

  void Print(std::ostream& os) const override;  // NOLINT
};
}  // namespace olap

#endif  // OLCORE_MESSAGES_PIT_H_
