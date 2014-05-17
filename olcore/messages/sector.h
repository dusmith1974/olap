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

#ifndef MESSAGES_SECTOR_H_
#define MESSAGES_SECTOR_H_

#include <iostream>

#include "messages/message.h"

namespace olap {

class Sector;
typedef std::vector<Sector> SectorVec;
typedef std::map<int, SectorVec> CompetitorSectorMap;

class Sector final : public Message {
 public:
  Sector();
  Sector(int num_val, int competitor_num_val, int lap_num_val, long time_val);

  virtual ~Sector();

  Message* Clone() const;

  operator std::string() const;

  int num() const;
  void set_num(int val);

  int competitor_num() const;

  Interval time() const;

  operator Interval() const;

 private:
  void Print(std::ostream& os) const override;

  int num_;
  int competitor_num_;
  int lap_num_;
  Interval time_;
};


}  // namespace olap

#endif  // MESSAGES_SECTOR_H_
