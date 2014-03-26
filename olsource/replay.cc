// Copyright 2013 Duncan Smith
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

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <sstream>
#include <string>

#include "boost/algorithm/string.hpp"
#include "boost/asio.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/ptr_container/ptr_map.hpp"
#include "boost/range/adaptor/map.hpp"
#include "boost/range/algorithm/copy.hpp"
#include "boost/regex.hpp"

namespace adaptors = boost::adaptors;

class Lap;
class Competitor;
class Interval;
class Message;

typedef std::vector<std::string> MsgVec;
typedef std::map<int, Competitor> CompetitorMap;
typedef std::vector<Lap> LapVec;


template <typename charT, typename traits>
inline std::basic_istream<charT,traits>&ignore_line(std::basic_istream<
    charT,traits>& strm) {
  strm.ignore(std::numeric_limits<std::streamsize>::max(),
              strm.widen('\n'));

   return strm;
}

void MessageHandler(const boost::system::error_code&) {
  std::cout << "MSG: " << std::endl;
}

void MessageHandler2(const boost::system::error_code&) {
  std::cout << "MSG2: " << std::endl;
}
class Interval {
 public:
  Interval() : milliseconds_(0) {};
  explicit Interval(long val) : milliseconds_(val) {};
  explicit Interval(const std::chrono::milliseconds& val) : milliseconds_(val) {
  }

  operator long() const { return milliseconds_.count(); }
  operator std::chrono::milliseconds() const { return milliseconds_; }

  const Interval& operator +=(const Interval& interval) {
    milliseconds_ += interval.milliseconds_;
    return *this;
  }

 protected:
  std::chrono::milliseconds milliseconds_;
  std::string str_;

 private:
  friend std::istream& operator>>(std::istream& is, Interval& interval);
  friend std::ostream& operator<<(std::ostream& os, const Interval& interval);

};

std::istream& operator>>(std::istream& is, Interval& interval) {
  std::string str;
  std::getline(is, str);

  interval.str_ = "";
  interval.milliseconds_ = std::chrono::milliseconds();

  boost::smatch m;
  // TODO(ds) handle hours on input too.
  if (boost::regex_search(str, m, boost::regex(R"((\d+):(\d+)\.(\d+))"))) {
    interval.milliseconds_ += std::chrono::minutes(
        boost::lexical_cast<int>(m[1].str()));

    interval.milliseconds_ += std::chrono::seconds(
        boost::lexical_cast<int>(m[2].str()));

    interval.milliseconds_ += std::chrono::milliseconds(
        boost::lexical_cast<int>(m[3].str()));
  } else if (boost::regex_search(str, m, boost::regex(R"((\d+)\.(\d+))"))) {
    interval.milliseconds_ += std::chrono::seconds(
        boost::lexical_cast<int>(m[1].str()));

    interval.milliseconds_ += std::chrono::milliseconds(
        boost::lexical_cast<int>(m[2].str()));
  } else {
    interval.str_ = str;  // 1 LAP, 2 LAPS etc.
  }

  return is;
}

std::ostream& operator<<(std::ostream& os, const Interval& interval) {
  if (!interval.str_.empty()) {
    os << interval.str_;
    return os;
  }

  os << std::chrono::duration_cast<
    std::chrono::seconds>(interval.milliseconds_).count();

  os << "." << std::setw(3) << std::setfill('0')
     << std::chrono::duration_cast<std::chrono::milliseconds>(
      interval.milliseconds_ % std::chrono::seconds(1)).count();

  return os;
}

class LongInterval : public Interval {
 public:
  LongInterval() : Interval(0) {};
  explicit LongInterval(long val) : Interval(val) {};

 private:
  friend std::ostream& operator<<(std::ostream& os,
                                  const LongInterval& long_interval);
};

std::ostream& operator<<(std::ostream& os, const LongInterval& long_interval) {
  std::chrono::hours hours = std::chrono::duration_cast<std::chrono::hours>(
      long_interval.milliseconds_);

  if (hours.count()) {
    os << hours.count() << ":";
    os << std::setw(2) << std::setfill('0');
  }

  os << std::chrono::duration_cast<
    std::chrono::minutes>((hours.count())
        ? long_interval.milliseconds_ % std::chrono::hours(1)
        : long_interval.milliseconds_).count();

  os << ":" << std::setw(2) << std::setfill('0')
    << std::chrono::duration_cast<std::chrono::seconds>(
      long_interval.milliseconds_ % std::chrono::minutes(1)).count();

  os << "." << std::setw(3)
    << std::chrono::duration_cast<std::chrono::milliseconds>(
      long_interval.milliseconds_ % std::chrono::seconds(1)).count();

  return os;
}

class Message {
 public:
  Message() : race_time_(Interval()) {
  }

  virtual ~Message() {
  }

  virtual Message* Clone() const = 0;

  void set_timer(boost::asio::io_service* service) {
    if (!service) return;
    timer_ = std::shared_ptr<boost::asio::deadline_timer>(new boost::asio::deadline_timer(*service, boost::posix_time::milliseconds(race_time())));
  }

  void start_timer() {
    timer_->async_wait(&MessageHandler2);
  }

  operator std::string() const {
    std::stringstream ss;
    ss << *this;

    return ss.str();
  }

  Interval race_time() const { return race_time_; }
  void set_race_time(const Interval& val) { race_time_ = val; }

 protected:
  Interval race_time_;

 private:
  friend std::ostream& operator<<(std::ostream& os, const Message& message);

  virtual void Print(std::ostream& os) const = 0;

  std::shared_ptr<boost::asio::deadline_timer> timer_;
};

std::ostream& operator<<(std::ostream& os, const Message& message) {
  message.Print(os);
  return os;
}

class Competitor final : public Message {
 public:
  Competitor()
    : num_(0),
      grid_pos_(0),
      short_name_(""),
      name_(""),
      team_("") {
  }

  Message* Clone() const { return new Competitor(*this); }

  operator std::string() const {
    std::stringstream ss;
    ss << *this;

    return ss.str();
  }

  void set_grid_pos(int val) { grid_pos_ = val; }
  int num() const { return num_; }

 private:
  friend std::istream& operator>>(std::istream& is, Competitor& competitor);

  void Print(std::ostream& os) const {
    os << "comp," << "0," << num_ << "," << grid_pos_ << "," << short_name_
      << "," << name_ << "," << team_ << std::endl;
  }

  int num_;
  int grid_pos_;
  std::string short_name_;
  std::string name_;
  std::string team_;
};

std::istream& operator>>(std::istream& is, Competitor& competitor) {
  is >> competitor.num_;
  is.ignore();

  std::getline(is, competitor.short_name_, ',');
  std::getline(is, competitor.name_, ',');
  std::getline(is, competitor.team_, ',');

  return is;
}

void ReadCompetitors(CompetitorMap* competitors) {
  if (!competitors) return;

  std::ifstream file;
  file.open("competitors.txt");

  int grid_pos = 0;
  std::string line;
  while (std::getline(file, line)) {
    auto competitor = boost::lexical_cast<Competitor>(line);
    competitor.set_grid_pos(++grid_pos);
    (*competitors)[competitor.num()] = competitor;
  }
}

// TODO(ds) Message base class with race_time?
class Lap final : public Message {
 public:
  Lap()
    : num_(0),
      competitor_num_(0),
      gap_(Interval()),
      time_(LongInterval()) {
  }

  Message* Clone() const { return new Lap(*this); }

  operator std::string() const {
    std::stringstream ss;
    ss << *this;

    return ss.str();
  }

 public:
  void set_num(int val) { num_ = val; }

  int competitor_num() const { return competitor_num_; }

  operator Interval() const { return gap_; }
  operator LongInterval() const { return time_; };

 private:
  friend std::istream& operator>>(std::istream& is, Lap& lap);

  void Print(std::ostream& os) const {
    os << "lap," << static_cast<long>(race_time_) << "," << competitor_num_
      << "," << num_ << "," << gap_ << "," << time_ << std::endl;
  }

  int num_;
  int competitor_num_;
  Interval gap_;
  LongInterval time_;
};

std::istream& operator>>(std::istream& is, Lap& lap) {
  std::string str;
  std::getline(is, str);

  boost::smatch m;
  if (boost::regex_search(str, m, boost::regex(R"(^\d+)")))
    lap.competitor_num_ = boost::lexical_cast<int>(m.str());

  if (boost::regex_search(str, m,
                        boost::regex(R"(((?<=\s)\d+\.\d+)|(\d+ LAPS?)|(PIT))")))
    lap.gap_ = boost::lexical_cast<Interval>(m.str());
  else
    lap.gap_ = Interval();

  if (boost::regex_search(str, m, boost::regex(R"(\d+:\d+\.\d+)")))
    lap.time_ = boost::lexical_cast<LongInterval>(m.str());

  return is;
}

void ReadRaceHistory(LapVec* laps) {
  if (!laps) return;

  std::ifstream file;
  file.open("race_hist.txt");

  // Competitor num followed by any other data (upto the next competitor num).
  boost::regex rgx(R"(((?<=\s)[0-9]+\s)(.(?!(?1)))*)");

  std::string line;
  std::map<int,int> competitor_lap_count;
  std::map<int,Interval> competitor_race_time;
  boost::sregex_token_iterator end;
  while (std::getline(file, line)) {
    boost::sregex_token_iterator iter(line.cbegin(), line.cend(), rgx, 0);
    for (; iter != end; ++iter) {
      auto lap = boost::lexical_cast<Lap>(boost::trim_copy(iter->str()));

      lap.set_num(++competitor_lap_count[lap.competitor_num()]);
      lap.set_race_time(competitor_race_time[lap.competitor_num()]
          += static_cast<LongInterval>(lap));

      laps->push_back(lap);
    }
  }
}


typedef std::pair<long, std::unique_ptr<Message>> Event;

struct EventPQueueSortCriterion {
  bool operator()(const Event& a, const Event& b) const {
    return a.first > b.first;
  }
};

typedef boost::ptr_multimap<Interval, Message> MessageMap;

typedef std::priority_queue<Event, std::vector<Event>,
                            EventPQueueSortCriterion> EventPQueue;
namespace {
template<typename T>
void AddMessages(T coll, MessageMap* message_map) {
  if (!message_map) return;

  for (const auto& msg : coll) {
    Interval race_time = msg.race_time();
    message_map->insert(race_time, msg.Clone());
  }
}
}  // namespace

int main() {
  MsgVec msgs;
  EventPQueue events;
  CompetitorMap competitors;
  LapVec laps;

  ReadCompetitors(&competitors);
  /*boost::copy(competitors | adaptors::map_values,
              std::ostream_iterator<Competitor>(std::cout));*/
  boost::copy(competitors | adaptors::map_values,
              std::back_inserter(msgs));

  MessageMap message_map;
  AddMessages(competitors | adaptors::map_values, &message_map);

  ReadRaceHistory(&laps);
  std::copy(laps.begin(), laps.end(), std::back_inserter(msgs));

  //Event ev = std::make_pair(laps[0].race_time(), std::unique_ptr<Message>(new Lap(laps[0])));
  //events.push(std::move(ev));

  AddMessages(laps, &message_map);

  std::copy(msgs.begin(), msgs.end(),
            std::ostream_iterator<std::string>(std::cout));

  /*std::cout << events.size() << std::endl;
  while (!events.empty()) {
    std::cout << events.top().first << " " << *events.top().second);
    events.pop();
  }*/

  boost::asio::io_service service;

  std::cout << message_map.size() << std::endl;
  for (const auto& message : message_map) {
    std::cout << "msg " << *message.second;
    message.second->set_timer(&service);
  }

  // repeat every second to show time, see timer in osoa comms
  boost::asio::deadline_timer t(service, boost::posix_time::milliseconds(1000));
  t.async_wait(&MessageHandler);

  for (const auto& message : message_map) {
    message.second->start_timer();
  }

  // base class msg, msgvec on event not string (eg with racetime), deadline timer in event or shared ptr here.
  // show racetime every second
  // bind msg string to handler
  /*for (const auto& lap : laps) {
    long l = lap.race_time();
    boost::asio::deadline_timer* tl = new boost::asio::deadline_timer(service, boost::posix_time::milliseconds(l));
    tl->async_wait(&MessageHandler2);
  }*/

  service.run();

  // TODO(ds)
  // race start time = lowest fastlap on lap - laps to 0
  // pit in = tod - race start time
  // pit out = pit in + duration
  // pit message
  // add all messages to events order by race time
  // re-write as service
  // fire callback to publish
}
