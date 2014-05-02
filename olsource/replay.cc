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
#include "boost/bind.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/multiprecision/cpp_dec_float.hpp"
#include "boost/optional.hpp"
#include "boost/ptr_container/ptr_map.hpp"
#include "boost/ptr_container/ptr_vector.hpp"
#include "boost/range/adaptor/map.hpp"
#include "boost/range/algorithm/copy.hpp"
#include "boost/regex.hpp"

namespace adaptors = boost::adaptors;
namespace mp = boost::multiprecision;

using boost::optional;

class Lap;
class Sector;
class Competitor;
class Interval;
class Message;
class Pit;
class Out;

typedef std::vector<std::string> MsgVec;
typedef std::map<int, Competitor> CompetitorMap;
typedef std::vector<Lap> LapVec;
typedef std::vector<Sector> SectorVec;
typedef std::vector<Pit> PitVec;
typedef std::vector<Out> OutVec;

typedef mp::number<mp::cpp_dec_float<3> > cpp_dec_float_3;

// TODO(ds) rm or mv to utils.
template <typename charT, typename traits>
inline std::basic_istream<charT,traits>& ignore_line(std::basic_istream<
    charT,traits>& strm) {
  strm.ignore(std::numeric_limits<std::streamsize>::max(),
              strm.widen('\n'));

   return strm;
}

void PublishMessage(const boost::system::error_code&,
                    const std::string& message) {
  std::cout << message;
}

class Interval {
 public:
  Interval() : milliseconds_(0) {
  };

  explicit Interval(long val) : milliseconds_(val) {
  };

  explicit Interval(const std::chrono::milliseconds& val) : milliseconds_(val) {
  }

  const std::string str() const { return str_; }

  operator long() const { return milliseconds_.count(); }
  operator std::chrono::milliseconds() const { return milliseconds_; }

  const Interval& operator+=(const Interval& interval) {
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

inline Interval operator+(Interval lhs, const Interval& rhs) {
  lhs += rhs;
  return lhs;
}

std::istream& operator>>(std::istream& is, Interval& interval) {
  std::string str;
  std::getline(is, str);

  interval.str_ = "";
  interval.milliseconds_ = std::chrono::milliseconds();

  boost::smatch m;
  if (boost::regex_search(str, m, boost::regex(R"((\d+):(\d+)[\.:](\d+))"))) {
    if (std::count(str.begin(), str.end(), ':') > 1) {
      interval.milliseconds_ += std::chrono::hours(
          boost::lexical_cast<int>(m[1].str()));

      interval.milliseconds_ += std::chrono::minutes(
          boost::lexical_cast<int>(m[2].str()));

      interval.milliseconds_ += std::chrono::seconds(
          boost::lexical_cast<int>(m[3].str()));
    } else {
      interval.milliseconds_ += std::chrono::minutes(
          boost::lexical_cast<int>(m[1].str()));

      interval.milliseconds_ += std::chrono::seconds(
          boost::lexical_cast<int>(m[2].str()));

      interval.milliseconds_ += std::chrono::milliseconds(
          boost::lexical_cast<int>(m[3].str()));
    }
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
  LongInterval() : Interval(0) {
  };

  explicit LongInterval(long val) : Interval(val) {
  };

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

    timer_ = std::shared_ptr<boost::asio::deadline_timer>(
      new boost::asio::deadline_timer(*service,
                                 boost::posix_time::milliseconds(race_time())));
  }

  void start_timer() {
    timer_->async_wait(boost::bind(PublishMessage,
                       boost::asio::placeholders::error,
                       static_cast<std::string>(*this)));
  }

  operator std::string() const {
    std::stringstream ss;
    ss << *this;

    return ss.str();
  }

  static LongInterval race_start_time() { return race_start_time_; }

  static void set_race_start_time(const LongInterval& val) {
    race_start_time_ = val;
  }

  LongInterval time_of_day() const { return time_of_day_; }
  void set_time_of_day(const LongInterval& val) { time_of_day_ = val; }

  Interval race_time() const { return race_time_; }

  void set_race_time(const Interval& val) {
    race_time_ = val;
    time_of_day_ = LongInterval(Message::race_start_time() + race_time());
  }

 protected:
  Interval race_time_;
  LongInterval time_of_day_;

 private:
  friend std::ostream& operator<<(std::ostream& os, const Message& message);

  static LongInterval race_start_time_;

  virtual void Print(std::ostream& os) const = 0;

  std::shared_ptr<boost::asio::deadline_timer> timer_;
};

LongInterval Message::race_start_time_;

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

  int grid_pos() const { return grid_pos_; }
  void set_grid_pos(int val) { grid_pos_ = val; }

  std::string name() const { return name_; }
  int num() const { return num_; }

  cpp_dec_float_3 sector_1_percent() const { return sector_1_percent_; }
  void set_sector_1_percent(cpp_dec_float_3 val) { sector_1_percent_ = val; }

  cpp_dec_float_3 sector_2_percent() const { return sector_2_percent_; }
  void set_sector_2_percent(cpp_dec_float_3 val) { sector_2_percent_ = val; }

  cpp_dec_float_3 sector_3_percent() const { return sector_3_percent_; }
  void set_sector_3_percent(cpp_dec_float_3 val) { sector_3_percent_ = val; }

 private:
  friend std::istream& operator>>(std::istream& is, Competitor& competitor);

  void Print(std::ostream& os) const override {
    os << "com," << "0,0," << num_ << "," << grid_pos_ << "," << short_name_
      << "," << name_ << "," << team_ << std::endl;
  }

  int num_;
  int grid_pos_;
  std::string short_name_;
  std::string name_;
  std::string team_;
  cpp_dec_float_3 sector_1_percent_;
  cpp_dec_float_3 sector_2_percent_;
  cpp_dec_float_3 sector_3_percent_;
};

std::istream& operator>>(std::istream& is, Competitor& competitor) {
  is >> competitor.num_;
  is.ignore();

  std::getline(is, competitor.short_name_, ',');
  std::getline(is, competitor.name_, ',');
  std::getline(is, competitor.team_, ',');

  return is;
}

void ReadBestSectors(CompetitorMap* competitors) {
  if (!competitors) return;

  std::ifstream file;
  file.open("BestSectors.txt");

  int num = 0;
  cpp_dec_float_3 lap_time;
  cpp_dec_float_3 sector_1, sector_2, sector_3;
  cpp_dec_float_3 sector_1_pc, sector_2_pc, sector_3_pc;

  std::string str;
  while (std::getline(file, str)) {
    std::istringstream iss(str);
    iss >> num;
    iss >> sector_1; iss >> sector_2; iss >> sector_3;

    sector_1_pc = sector_1 / (sector_1 + sector_2 + sector_3);
    sector_2_pc = sector_2 / (sector_1 + sector_2 + sector_3);
    sector_3_pc = sector_3 / (sector_1 + sector_2 + sector_3);
    sector_3_pc += 1 - (sector_1_pc + sector_2_pc + sector_3_pc);
    (*competitors)[num].set_sector_1_percent(sector_1_pc);
    (*competitors)[num].set_sector_2_percent(sector_2_pc);
    (*competitors)[num].set_sector_3_percent(sector_3_pc);

    // TODO(ds) NDEBUG release
    assert(1 == sector_1_pc + sector_2_pc + sector_3_pc);
  }
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

  ReadBestSectors(competitors);
}

const optional<CompetitorMap::mapped_type&> FindPole(CompetitorMap* competitors) {
  if (!competitors) return optional<CompetitorMap::mapped_type&>();

  auto pole = std::find_if(competitors->begin(), competitors->end(),
                           [] (const std::pair<int, Competitor>& elem) {
                             return elem.second.grid_pos() == 1;
                           });

  return (pole == competitors->end()) ? optional<CompetitorMap::mapped_type&>()
    : optional<CompetitorMap::mapped_type&>(pole->second);
}

class Lap final : public Message {
 public:
  Lap()
    : num_(0),
      competitor_num_(0),
      gap_(Interval()),
      time_(LongInterval()) {
  }

  virtual ~Lap() {
  }

  Message* Clone() const { return new Lap(*this); }

  operator std::string() const {
    std::stringstream ss;
    ss << *this;

    return ss.str();
  }

  int num() const { return num_; }
  void set_num(int val) { num_ = val; }

  Interval gap() const { return gap_; }
  void set_gap(Interval val) { gap_ = val; }

  int competitor_num() const { return competitor_num_; }

  LongInterval time() const { return time_; }

  operator Interval() const { return gap_; }
  operator LongInterval() const { return time_; };

 private:
  friend std::istream& operator>>(std::istream& is, Lap& lap);

  void Print(std::ostream& os) const override {
    os << "lap," << static_cast<LongInterval>(race_time_)
      << "," << time_of_day_ << "," << competitor_num_
      << "," << num_ << "," << gap_ << "," << time_ << std::endl;
  }

  int num_;
  int competitor_num_;
  Interval gap_;
  LongInterval time_;
};

inline bool operator<(const Lap& lhs, const Lap& rhs) {
  return lhs.num() < rhs.num();
}

inline bool operator>(const Lap& lhs, const Lap& rhs) {
  return rhs < lhs;
}

inline bool operator<=(const Lap& lhs, const Lap& rhs) {
  return !(lhs > rhs);
}

inline bool operator>=(const Lap& lhs, const Lap& rhs) {
  return !(lhs < rhs);
}

inline bool operator==(const Lap& lhs, const Lap& rhs) {
  return lhs.num() == rhs.num();
}

inline bool operator!=(const Lap& lhs, const Lap& rhs) {
  return !(lhs == rhs);
}

std::istream& operator>>(std::istream& is, Lap& lap) {
  std::string str;
  std::getline(is, str);

  boost::smatch m;
  if (boost::regex_search(str, m, boost::regex(R"(^\d+)")))
    lap.competitor_num_ = boost::lexical_cast<int>(m.str());

  if (boost::regex_search(str, m, boost::regex(R"(^\d+\s(\d+)\s(?!LAP))")))
    lap.num_ = boost::lexical_cast<int>(m[1].str());

  if (boost::regex_search(str, m,
                        boost::regex(R"(((?<=\s)\d+\.\d+)|(\d+ LAPS?)|(PIT))")))
    lap.gap_ = boost::lexical_cast<Interval>(m.str());
  else
    lap.gap_ = Interval();

  if (boost::regex_search(str, m, boost::regex(R"((\d+:)\d+[\.:]\d+)")))
    lap.time_ = boost::lexical_cast<LongInterval>(m.str());

  return is;
}

class Sector final : public Message {
 public:
  Sector()
    : num_(0),
      competitor_num_(0),
      lap_num_(0),
      time_(Interval()) {
  }

  Sector(int num, int competitor_num, int lap_num, long time)
    : num_(num),
      competitor_num_(competitor_num),
      lap_num_(lap_num),
      time_(time) {
  }

  virtual ~Sector() {
  }

  Message* Clone() const { return new Sector(*this); }

  operator std::string() const {
    std::stringstream ss;
    ss << *this;

    return ss.str();
  }

  int num() const { return num_; }
  void set_num(int val) { num_ = val; }

  int competitor_num() const { return competitor_num_; }

  Interval time() const { return time_; }

  operator Interval() const { return time_; };

 private:
  void Print(std::ostream& os) const override {
    os << "sec," << static_cast<LongInterval>(race_time_)
      << "," << time_of_day_ << "," << competitor_num_ << "," << lap_num_
      << "," << num_ << "," << time_ << std::endl;
  }

  int num_;
  int competitor_num_;
  int lap_num_;
  Interval time_;
};

inline bool operator<(const Sector& lhs, const Sector& rhs) {
  return lhs.num() < rhs.num();
}

inline bool operator>(const Sector& lhs, const Sector& rhs) {
  return rhs < lhs;
}

inline bool operator<=(const Sector& lhs, const Sector& rhs) {
  return !(lhs > rhs);
}

inline bool operator>=(const Sector& lhs, const Sector& rhs) {
  return !(lhs < rhs);
}

inline bool operator==(const Sector& lhs, const Sector& rhs) {
  return lhs.num() == rhs.num();
}

inline bool operator!=(const Sector& lhs, const Sector& rhs) {
  return !(lhs == rhs);
}

class Pit : public Message {
 public:
  Pit()
    : competitor_num_(0),
      lap_num_(0),
      num_(0) {
  }

  virtual ~Pit() {
  }

  Message* Clone() const { return new Pit(*this); }

  operator std::string() const {
    std::stringstream ss;
    ss << *this;

    return ss.str();
  }

  int lap_num() const { return lap_num_; }
  void set_lap_num(int val) { lap_num_ = val; }

 protected:
  int competitor_num_;
  int lap_num_;
  int num_;

 private:
  friend std::istream& operator>>(std::istream& is, Pit& pit);

  void Print(std::ostream& os) const override {
    os << "pit," << static_cast<LongInterval>(race_time_) << ","
      << time_of_day_ << "," << competitor_num_<< "," << lap_num_ << "," << num_
      << std::endl;
  }
};

std::istream& operator>>(std::istream& is, Pit& pit) {
  std::string str;

  is >> pit.competitor_num_, is.ignore();
  is >> pit.lap_num_, is.ignore();

  std::getline(is, str, ',');
  pit.time_of_day_ = boost::lexical_cast<LongInterval>(str);

  is >> pit.num_, is.ignore();

  std::getline(is, str, ',');
  std::getline(is, str, ',');

  return is;
}

class Out : public Message {
 public:
  Out()
    : competitor_num_(0),
      lap_num_(0),
      time_of_day_(LongInterval()),
      num_(0),
      time_(Interval()),
      total_time_(Interval()) {
  }

  virtual ~Out() {
  }

  Message* Clone() const { return new Out(*this); }

  operator std::string() const {
    std::stringstream ss;
    ss << *this;

    return ss.str();
  }

  Interval time() const { return time_; }
  LongInterval time_of_day() const { return time_of_day_; }

  int lap_num() const { return lap_num_; }
  void set_lap_num(int val) { lap_num_ = val; }

 protected:
  int competitor_num_;
  int lap_num_;
  LongInterval time_of_day_;
  int num_;
  Interval time_;
  Interval total_time_;

 private:
  friend std::istream& operator>>(std::istream& is, Out& out);

  void Print(std::ostream& os) const override {
    os << "out," << static_cast<LongInterval>(race_time_) << ","
      << time_of_day_ << "," << competitor_num_ << "," << lap_num_ << ","
      << num_ << "," << time_ << "," << total_time_ << std::endl;
  }
};

std::istream& operator>>(std::istream& is, Out& out) {
  std::string str;

  is >> out.competitor_num_, is.ignore();
  is >> out.lap_num_, is.ignore();

  std::getline(is, str, ',');
  out.time_of_day_ = boost::lexical_cast<LongInterval>(str);

  is >> out.num_, is.ignore();

  std::getline(is, str, ',');
  out.time_ = boost::lexical_cast<LongInterval>(str);
  // TODO(ds) op+=
  out.time_of_day_ = LongInterval(out.time_of_day_ + out.time_);

  std::getline(is, str, ',');
  out.total_time_ = boost::lexical_cast<LongInterval>(str);

  //out.race_time_ = Interval(1);

  return is;
}

typedef std::map<int, LapVec> CompetitorLapMap;
typedef std::map<int, SectorVec> CompetitorSectorMap;

LongInterval ReadLapAnalysis(const Lap& leaders_lap, CompetitorLapMap* lap_analysis) {
  if (!lap_analysis) return LongInterval(0);

  std::ifstream file;
  file.open("RaceLapAnalysis.txt");

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

void ReadLapHistory(CompetitorLapMap* lap_history, Lap* leaders_lap) {
  if (!lap_history || !leaders_lap) return;

  std::ifstream file;
  file.open("RaceHist.txt");

  // Competitor num followed by any other data (upto the next competitor num).
  boost::regex rgx(R"(((?<=\s)[0-9]+\s)(.(?!(?1)))*)");

  std::string line;
  //std::map<int,int> competitor_lap_count;
  //std::map<int,Interval> competitor_race_time;
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
        if (boost::regex_search(boost::lexical_cast<std::string>(lap.gap().str()),
                                m, boost::regex(R"((\d+) LAP)")))
          lap_no -= boost::lexical_cast<int>(m[1].str());

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

void ReadPits(const Interval& race_start_time, PitVec* pits, OutVec *outs) {
  if (!pits || !outs) return;

  std::ifstream file;
  file.open("PitStopSummary.txt");

  std::string str;
  while (std::getline(file, str)) {
    Pit pit = boost::lexical_cast<Pit>(str);
    Out out = boost::lexical_cast<Out>(str);

    pit.set_race_time(LongInterval(pit.time_of_day() - race_start_time));
    out.set_race_time(pit.race_time() + out.time());

    pits->push_back(pit);
    outs->push_back(out);
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

int main() {
  MsgVec msgs;
  //EventPQueue events;
  CompetitorMap competitors;
  CompetitorLapMap lap_history;
  CompetitorLapMap lap_analysis;
  CompetitorLapMap all_laps;
  CompetitorSectorMap sectors;

  ReadCompetitors(&competitors);
  /*boost::copy(competitors | adaptors::map_values,
              std::ostream_iterator<Competitor>(std::cout));*/
  boost::copy(competitors | adaptors::map_values,
              std::back_inserter(msgs));

  MessageMap message_map;
  AddMessages(competitors | adaptors::map_values, &message_map);

  optional<CompetitorMap::mapped_type&> pole = FindPole(&competitors);
  std::cout << "pole: " << (*pole).num() << " " << (*pole).name() << std::endl;

  Lap leaders_lap;
  ReadLapHistory(&lap_history, &leaders_lap);
  for (const auto& laps : lap_history | adaptors::map_values)
    all_laps[laps.begin()->competitor_num()].push_back(*laps.begin());

  Message::set_race_start_time(ReadLapAnalysis(leaders_lap, &lap_analysis));

  std::cout << "RST: " << Message::race_start_time() << std::endl;

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
                       b.set_race_time(a.race_time() + b.time());
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
  // so we'll have to guess them based on the lap time and the fastest sectors.
  long sector_1, sector_2, sector_3, lap_time;
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

  //Event ev = std::make_pair(laps[0].race_time(), std::unique_ptr<Message>(new Lap(laps[0])));
  //events.push(std::move(ev));

  //std::copy(msgs.begin(), msgs.end(),
    //        std::ostream_iterator<std::string>(std::cout));

  /*std::cout << events.size() << std::endl;
  while (!events.empty()) {
    std::cout << events.top().first << " " << *events.top().second);
    events.pop();
  }*/

  boost::asio::io_service service;

  for (const auto& message : message_map) {
    std::cout << "msg " << *message.second;
    message.second->set_timer(&service);
  }

  for (const auto& message : message_map)
    message.second->start_timer();

  service.run();

  // TODO(ds)
  // team names from pits
  // race start time = lowest fastlap on lap - laps to 0
  // pit in = tod - race start time
  // pit out = pit in + duration
  // pit message
  // add all messages to events order by race time
  // re-write as service
  // fire callback to publish
  // write some clients, ncurses, web, native?
}
