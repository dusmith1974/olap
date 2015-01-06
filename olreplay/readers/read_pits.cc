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

// Implements ReadPits.

#include "olreplay_pch.h"  // NOLINT

#include "readers/read_pits.h"

#include <fstream>  // NOLINT
#include <string>  // NOLINT

#include "boost/lexical_cast.hpp"

#include "olcore/messages/out.h"
#include "olcore/messages/pit.h"
#include "olcore/time/interval.h"

namespace olap {
  void ReadPits(const Interval& race_start_time, PitVec* pits, OutVec *outs) {
    if (!pits || !outs) return;

    std::ifstream file;
    std::string filename("PitStopSummary.txt");

    file.open(filename);
    if (!file.is_open())
      throw std::runtime_error("Could not open " + filename);

    std::string str;
    while (std::getline(file, str)) {
      Pit pit = boost::lexical_cast<Pit>(str);
      Out out = boost::lexical_cast<Out>(str);

      pit.set_race_time(LongInterval(pit.time_of_day() - race_start_time));
      out.set_race_time(Interval(pit.race_time() + out.time()));

      pits->push_back(pit);
      outs->push_back(out);
    }
  }
}  // namespace olap
