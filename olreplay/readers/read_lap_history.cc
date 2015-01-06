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

// Implements the ReadLapHistory class.

#include "olreplay_pch.h"  // NOLINT

#include "readers/read_lap_history.h"

#include <algorithm>  // NOLINT
#include <fstream>  // NOLINT
#include <string>  // NOLINT

#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/range/adaptor/map.hpp"
#include "boost/regex.hpp"

#include "olcore/messages/lap.h"

namespace adaptors = boost::adaptors;

namespace olap {
  void ReadLapHistory(CompetitorLapMap* lap_history, Lap* leaders_lap) {
    if (!lap_history || !leaders_lap) return;

    std::ifstream file;
    std::string filename("RaceHist.txt");

    file.open(filename);
    if (!file.is_open())
      throw std::runtime_error("Could not open " + filename);

    // Competitor num followed by any other data (upto the next competitor num).
    boost::regex rgx(R"(((?<=\s)[0-9]+\s)(.(?!(?1)))*)");

    std::string line;
    boost::sregex_token_iterator end;
    boost::smatch m;

    int page = 1;
    int row = 1;
    while (std::getline(file, line)) {
      boost::sregex_token_iterator iter(line.cbegin(), line.cend(), rgx, 0);
      if (iter == end) {
        ++page;
        row = 1;
      }

      int leaders_lap_no = 1 + ((page - 1) * 5);
      for (; iter != end; ++iter) {
        auto lap = boost::lexical_cast<Lap>(boost::trim_copy(iter->str()));

        if (lap.competitor_num()) {
          int lap_no = leaders_lap_no++;

          // Deduct from leaders lap if we've been lapped.
          std::string str = lap.gap().str();
          if (boost::regex_search(str, m, boost::regex(R"((\d+) LAP)"))) {
            lap_no -= boost::lexical_cast<int>(m[1].str());
          }

          lap.set_num(lap_no);

          if (lap.num() == 1)
            lap.set_race_time(lap.time());

          (*lap_history)[lap.competitor_num()].push_back(lap);

          if (page == 1 && row == 1 && lap_no == 1)
            *leaders_lap = lap;
        }
      }

      ++row;
    }

    // Sort the laps by lap number for each competitor.
    for (auto& laps : (*lap_history | adaptors::map_values))
      std::sort(laps.begin(), laps.end());
  }
}  // namespace olap
