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

// Implements the Replay class.

#include "olreplay_pch.h"  // NOLINT
#include "replay/replay.h"

#include <vector>  // NOLINT

#include "boost/algorithm/string.hpp"
#include "boost/asio.hpp"
#include "boost/optional.hpp"
#include "boost/range/adaptor/map.hpp"
#include "boost/range/algorithm/copy.hpp"

#include "osoa/service/args.h"
#include "osoa/service/comms/comms.h"
#include "osoa/service/logging.h"
#include "olcore/util/utilities.h"

#include "olcore/messages/messages.h"

#include "readers/readers.h"
#include "replay/replay.h"
#include "util/utilities.h"

namespace adaptors = boost::adaptors;
namespace asio = boost::asio;

using boost::optional;
using osoa::Error;
using osoa::Logging;

namespace olap {
namespace {
void AddMessage(const Message& msg, MessageMap* message_map) {
  Interval race_time = msg.race_time();
  message_map->insert(race_time, msg.Clone());
}

template<typename T>
void AddMessages(T coll, MessageMap* message_map) {
  if (!message_map) return;

  for (const auto& msg : coll)
    AddMessage(msg, message_map);
}
}  // namespace

Replay::Replay()
  : continue_replay_(false),
    msgs_{},
    message_map_{},
    competitors_{},
    lap_history_{},
    lap_analysis_{},
    all_laps_{},
    sectors_{},
    io_service_{},
    replay_thread_{} {
}

Replay::~Replay() {
}

// Add customizations specific to this particular service.
Error Replay::Initialize(int argc, const char* argv[]) {
  Error result = super::Initialize(argc, argv);

  return result;
}

// Starts the base class service, logs messages and connects to other
// services.
Error Replay::DoStart() {
  Error code = Error::kSuccess;
  if (publishing()) {
    Run();
  } else {
    if (args()->services().size()) {
      // Split "127.0.0.1:8000" into "127.0.0.1" and "8000"
      std::vector<std::string> tokens;
      std::string service = args()->services()[0];
      boost::split(tokens, service, boost::is_any_of(":"));

      if (tokens.size() > 1)
        code = comms()->Subscribe(tokens[0], tokens[1]);
    }
  }

  return code;
}

int Replay::Run() {
  BOOST_LOG_SEV(*Logging::logger(), blt::debug)
      << "Preparing messages.";

  ReadCompetitors(&competitors_);
  boost::copy(competitors_ | adaptors::map_values,
              std::back_inserter(msgs_));

  AddMessages(competitors_ | adaptors::map_values, &message_map_);

  Lap leaders_lap;
  ReadLapHistory(&lap_history_, &leaders_lap);
  for (const auto& laps : lap_history_ | adaptors::map_values)
    all_laps_[laps.begin()->competitor_num()].push_back(*laps.begin());

  Message::set_race_start_time(ReadLapAnalysis(leaders_lap, &lap_analysis_));

  for (auto& laps : lap_analysis_) {
    const auto other = lap_history_.find(laps.first);
    if (other != lap_history_.end() && other->second.size()) {
      std::transform(laps.second.begin(), std::next(laps.second.begin(),
                     other->second.size() - 1),
                     std::next(other->second.begin()),
                     laps.second.begin(),
      [](Lap& a, const Lap& b) {
        a.set_gap(b.gap());
        return a;
      });
    }
  }

  for (const auto& laps : lap_analysis_ | adaptors::map_values)
    std::copy(laps.begin(), laps.end(), std::back_inserter(
                all_laps_[laps.begin()->competitor_num()]));

  for (auto& laps : all_laps_)
    std::partial_sum(laps.second.begin(), laps.second.end(),
                     laps.second.begin(),
    [](Lap&a, Lap&b) {
    b.set_race_time(Interval(a.race_time() + b.time()));
    return b;
  });

  // Set the time of day on the first laps.
  for (auto& laps : all_laps_) {
    decltype(laps.second)::iterator iter = std::next(laps.second.begin());
    iter = (iter > laps.second.end()) ? laps.second.end() : iter;

    std::for_each(laps.second.begin(), iter,
    [](Lap& lap) {
      lap.set_time_of_day(LongInterval(
                            lap.time_of_day() + Message::race_start_time()));
    });
  }

  for (const auto& laps : all_laps_ | adaptors::map_values) {
    std::copy(laps.begin(), laps.end(), std::back_inserter(msgs_));
    AddMessages(laps, &message_map_);
  }

  // Doesn't look as though the sector analysis pdf is published (to the
  // public) so we'll have to guess the sector times based on the current lap
  // time and the fastest sectors.
  int64_t sector_1, sector_2, sector_3;
  for (const auto& laps : all_laps_ | adaptors::map_values) {
    for (const auto& lap : laps) {
      sector_1 = cpp_dec_float_3(
                   static_cast<int64_t>(lap.time())
                   * competitors_[lap.competitor_num()].sector_1_percent())
                 .convert_to<int64_t>();

      sector_2 = cpp_dec_float_3(
                   static_cast<int64_t>(lap.time())
                   * competitors_[lap.competitor_num()].sector_2_percent())
                 .convert_to<int64_t>();

      sector_3 = cpp_dec_float_3(
                   static_cast<int64_t>(lap.time())
                   * competitors_[lap.competitor_num()].sector_3_percent())
                 .convert_to<int64_t>();

      sector_3 += static_cast<int64_t>(lap.time())
                  - (sector_1 + sector_2 + sector_3);

      assert(static_cast<int64_t>(lap.time())
             == (sector_1 + sector_2 + sector_3));

      Sector s1(1, lap.competitor_num(), lap.num(), sector_1);
      Sector s2(2, lap.competitor_num(), lap.num(), sector_2);
      Sector s3(3, lap.competitor_num(), lap.num(), sector_3);

      s1.set_race_time(Interval(lap.race_time() - lap.time() + s1.time()));
      s2.set_race_time(Interval(s1.race_time() + s2.time()));
      s3.set_race_time(Interval(s2.race_time() + s3.time()));

      sectors_[lap.competitor_num()].push_back(s1);
      sectors_[lap.competitor_num()].push_back(s2);
      sectors_[lap.competitor_num()].push_back(s3);
    }
  }

  for (const auto& competitor_sectors : sectors_ | adaptors::map_values) {
    std::copy(
      competitor_sectors.begin(),
      competitor_sectors.end(),
      std::back_inserter(msgs_));

    AddMessages(competitor_sectors, &message_map_);
  }

  PitVec pits;
  OutVec outs;
  pits.reserve(50);
  outs.reserve(50);
  ReadPits(Message::race_start_time(), &pits, &outs);

  AddMessages(pits, &message_map_);
  AddMessages(outs, &message_map_);

  // Cut race time for quick debug/test.
  // #define OLAP_SHORT_RACE
#ifdef OLAP_SHORT_RACE
  auto pos = message_map_.begin();
  std::advance(pos, 50);
  message_map_.erase(pos, message_map_.end());
#endif  // OLAP_SHORT_RACE

#define OLAP_CONTINUOUS_REPLAY
#ifdef OLAP_CONTINUOUS_REPLAY
  continue_replay_ = true;
#endif

  BOOST_LOG_SEV(*Logging::logger(), blt::debug)
      << "Starting replay thread.";

  replay_thread_ = std::thread([&]() { Play(); });

  return 0;
}

void Replay::Play() {
  do {
    BOOST_LOG_SEV(*Logging::logger(), blt::debug)
        << "Starting timers.";

    for (const auto& message : message_map_)
      message.second->set_timer(&io_service_);

    for (const auto& message : message_map_)
      message.second->start_timer(boost::bind(&Replay::PublishMessage,
                                              this, _1, _2));
    BOOST_LOG_SEV(*Logging::logger(), blt::debug)
        << "Race started.";
    io_service_.run();

    io_service_.reset();
    Message::reset_quick_time();
  } while (continue_replay_);

  msgs_.clear();
  competitors_.clear();
  lap_history_.clear();
  lap_analysis_.clear();
  all_laps_.clear();
  sectors_.clear();
  message_map_.clear();
}

// No tidy up is required except to stop the base class service.
Error Replay::DoStop() {
  continue_replay_ = false;
  io_service_.stop();

  BOOST_LOG_SEV(*Logging::logger(), blt::debug)
      << "Joining replay thread.";

  if (replay_thread_.joinable())
    replay_thread_.join();

  return Error::kSuccess;
}

// TODO(ds) ren PublishMessage, drop num.
void Replay::AddTopicMessage(const std::string& topic,
                             const std::string& message, int num) {
  (void)num;
  (void)topic;
  comms()->PublishMessage(message);
}

void Replay::PublishMessage(const boost::system::error_code&,
                            const std::string& message) {
  AddTopicMessage("data", message, 1);
}
}  // namespace olap
