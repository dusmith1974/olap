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

// Implements the ReadLapAnalysis class.

#include "olreplay_pch.h"  // NOLINT

#include "readers/read_lap_analysis.h"

#include <fstream>  // NOLINT

#include "boost/lexical_cast.hpp"

#include "olcore/messages/lap.h"
#include "olcore/time/long_interval.h"

namespace olap {
  LongInterval ReadLapAnalysis(const Lap& leaders_lap, CompetitorLapMap* lap_analysis) {
    if (!lap_analysis) return LongInterval(0);

    std::ifstream file;
    std::string filename("RaceLapAnalysis.txt");

    file.open(filename);
    if (!file.is_open())
      throw std::runtime_error("Could not open " + filename);

    LongInterval race_start_time;
    std::string line;
    while (std::getline(file, line)) {
      auto lap = boost::lexical_cast<Lap>(line);
      if (lap.num() > 1)
        (*lap_analysis)[lap.competitor_num()].push_back(lap);
      else if (lap.num() == 1
               && lap.competitor_num() == leaders_lap.competitor_num())
               race_start_time = LongInterval(lap.time() - leaders_lap.time());
    }

    return race_start_time;
  }
}  // namespace olap
