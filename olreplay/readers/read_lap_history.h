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

// Contains a class representing a ReadLapHistory.

#ifndef OLREPLAY_READERS_READ_LAP_HISTORY_H_
#define OLREPLAY_READERS_READ_LAP_HISTORY_H_

#include "olcore/messages/lap_fwd.h"

namespace olap {
  void ReadLapHistory(CompetitorLapMap* lap_history, Lap* leaders_lap);
}  // namespace olap

#endif  // OLREPLAY_READERS_READ_LAP_HISTORY_H_
