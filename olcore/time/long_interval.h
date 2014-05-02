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

// Contains the LongInterval class.

#ifndef TIME_LONG_INTERVAL_H_
#define TIME_LONG_INTERVAL_H_

#include "time/interval.h"

namespace olap {

class LongInterval : public Interval {
 public:
  LongInterval();
  explicit LongInterval(long val);

 private:
  friend std::ostream& operator<<(std::ostream& os,
                                  const LongInterval& long_interval);
};
}  // namespace olap

#endif  // TIME_LONG_INTERVAL_H_
