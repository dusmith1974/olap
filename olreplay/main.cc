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

// Recreates an events timing messages from the results.

#include "olreplay_pch.h"

#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

#include "boost/asio.hpp"
#include "boost/optional.hpp"
#include "boost/range/adaptor/map.hpp"
#include "boost/range/algorithm/copy.hpp"

#include "osoa/service/comms/comms.h"

#include "olcore/messages/messages.h"
#include "olcore/util/utilities.h"

#include "readers/readers.h"
#include "replay/replay.h"
#include "util/utilities.h"

namespace adaptors = boost::adaptors;
namespace asio = boost::asio;
using boost::optional;

olap::Replay replay;

namespace olap {

void PublishMessage(const boost::system::error_code&,
                    const std::string& message) {
  replay.AddTopicMessage("data", message, 1);
}

int run() {
  MsgVec msgs;
  CompetitorMap competitors;
  CompetitorLapMap lap_history, lap_analysis, all_laps;
  CompetitorSectorMap sectors;

  ReadCompetitors(&competitors);
  boost::copy(competitors | adaptors::map_values,
              std::back_inserter(msgs));

  MessageMap message_map;
  AddMessages(competitors | adaptors::map_values, &message_map);

  Lap leaders_lap;
  ReadLapHistory(&lap_history, &leaders_lap);
  for (const auto& laps : lap_history | adaptors::map_values)
    all_laps[laps.begin()->competitor_num()].push_back(*laps.begin());

  Message::set_race_start_time(ReadLapAnalysis(leaders_lap, &lap_analysis));

  for (auto& laps : lap_analysis) {
    const auto other = lap_history.find(laps.first);
    if (other != lap_history.end() && other->second.size()) {
      std::transform(laps.second.begin(), std::next(laps.second.begin(),
                                                    other->second.size() - 1),
                     std::next(other->second.begin()),
                     laps.second.begin(),
                     [] (Lap& a, const Lap& b) {
                       a.set_gap(b.gap());
                       return a;
                     });
    }
  }

  for (const auto& laps : lap_analysis | adaptors::map_values)
    std::copy(laps.begin(), laps.end(), std::back_inserter(
          all_laps[laps.begin()->competitor_num()]));

  for (auto& laps : all_laps)
    std::partial_sum(laps.second.begin(), laps.second.end(),
                     laps.second.begin(),
                     [] (Lap&a, Lap&b) {
                       b.set_race_time(Interval(a.race_time() + b.time()));
                       return b;
                     });

  // Set the time of day on the first laps.
  for (auto& laps : all_laps) {
    decltype(laps.second)::iterator iter = std::next(laps.second.begin());
    iter = (iter > laps.second.end()) ? laps.second.end() : iter;

    std::for_each(laps.second.begin(), iter,
                  [] (Lap& lap) {
                    lap.set_time_of_day(LongInterval(
                        lap.time_of_day() + Message::race_start_time()));
                  });
  }

  for (const auto& laps : all_laps | adaptors::map_values) {
    std::copy(laps.begin(), laps.end(), std::back_inserter(msgs));
    AddMessages(laps, &message_map);
  }

  // Doesn't look as though the sector analysis pdf is published (to the public)
  // so we'll have to guess the sector times based on the current lap time and
  // the fastest sectors.
  long sector_1, sector_2, sector_3;
  for (const auto& laps : all_laps | adaptors::map_values) {
    for (const auto& lap : laps) {
      sector_1 = cpp_dec_float_3(static_cast<long>(lap.time()) * competitors[lap.competitor_num()].sector_1_percent()).convert_to<long>();
      sector_2 = cpp_dec_float_3(static_cast<long>(lap.time()) * competitors[lap.competitor_num()].sector_2_percent()).convert_to<long>();
      sector_3 = cpp_dec_float_3(static_cast<long>(lap.time()) * competitors[lap.competitor_num()].sector_3_percent()).convert_to<long>();
      sector_3 += static_cast<long>(lap.time()) - (sector_1 + sector_2 + sector_3);

      assert(static_cast<long>(lap.time()) == (sector_1 + sector_2 + sector_3));

      Sector s1(1, lap.competitor_num(), lap.num(), sector_1);
      Sector s2(2, lap.competitor_num(), lap.num(), sector_2);
      Sector s3(3, lap.competitor_num(), lap.num(), sector_3);

      s1.set_race_time(Interval(lap.race_time() - lap.time() + s1.time()));
      s2.set_race_time(Interval(s1.race_time() + s2.time()));
      s3.set_race_time(Interval(s2.race_time() + s3.time()));

      sectors[lap.competitor_num()].push_back(s1);
      sectors[lap.competitor_num()].push_back(s2);
      sectors[lap.competitor_num()].push_back(s3);
    }
  }

  for (const auto& competitor_sectors : sectors | adaptors::map_values) {
    std::copy(competitor_sectors.begin(), competitor_sectors.end(), std::back_inserter(msgs));
    AddMessages(competitor_sectors, &message_map);
  }

  PitVec pits;
  OutVec outs;
  pits.reserve(50);
  outs.reserve(50);
  ReadPits(Message::race_start_time(), &pits, &outs);

  AddMessages(pits, &message_map);
  AddMessages(outs, &message_map);

  boost::asio::io_service io_service;

  for (const auto& message : message_map)
    message.second->set_timer(&io_service);

  for (const auto& message : message_map)
    message.second->start_timer(PublishMessage);

  io_service.run();

  return 0;
}
}  // namespace olap

int main(int argc, const char* argv[]) {
  osoa::Error code = replay.Initialize(argc, argv);
  if (osoa::Error::kSuccess != code)
    return static_cast<int>(code);

  if (osoa::Error::kSuccess == replay.Start()) {
    if (replay.publishing())
      olap::run();

    replay.Stop();
  }

  return static_cast<int>(osoa::Error::kSuccess);
}
