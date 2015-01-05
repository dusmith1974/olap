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

// Contains the sector message class.

#ifndef OLCORE_MESSAGES_SECTOR_H_
#define OLCORE_MESSAGES_SECTOR_H_

#include <string>

#include "messages/message.h"
#include "messages/sector_fwd.h"

namespace olap {
  class Sector final : public Message {
  public:
    Sector();
    Sector(int num_val,
           int competitor_num_val,
           int lap_num_val,
           int64_t time_val);

    virtual ~Sector();

    Message* Clone() const;

    operator std::string() const;

    int num() const;
    void set_num(int val);

    int competitor_num() const;

    Interval time() const;

    operator Interval() const;

  private:
    void Print(std::ostream& os) const override;  // NOLINT

    int num_;
    int competitor_num_;
    int lap_num_;
    Interval time_;
  };
}  // namespace olap

#endif  // OLCORE_MESSAGES_SECTOR_H_
