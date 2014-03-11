#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <sstream>
#include <string>

#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/range/adaptor/map.hpp"
#include "boost/range/algorithm/copy.hpp"
#include "boost/regex.hpp"

namespace adaptors = boost::adaptors;

class Competitor;
class Lap;

typedef std::vector<std::string> MsgVec;
typedef std::map<int, Competitor> CompetitorMap;
typedef std::vector<Lap> LapVec;

typedef std::pair<int, std::string> Event;

struct EventPQueueSortCriterion {
  bool operator()(const Event& a, const Event& b) const {
    return a.first > b.first;
  }
};

typedef std::priority_queue<Event, std::vector<Event>,
                            EventPQueueSortCriterion> EventPQueue;

template <typename charT, typename traits>
inline std::basic_istream<charT,traits>&ignore_line (std::basic_istream<charT,traits>& strm)
{
   strm.ignore(std::numeric_limits<std::streamsize>::max(),
               strm.widen('\n'));

   return strm;
}

class Competitor {
 public:
  Competitor()
    : num_(0),
      grid_pos_(0),
      short_name_(""),
      name_(""),
      team_("") {
 }

 public:
  void set_grid_pos(int val) { grid_pos_ = val; }
  int num() const { return num_; }

  operator std::string() const {
    std::stringstream ss;
    ss << *this;

    return ss.str();
  }

 private:
  friend std::istream& operator>>(std::istream& is, Competitor& competitor);
  friend std::ostream& operator<<(std::ostream& os,
                                  const Competitor& competitor);
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

std::ostream& operator<<(std::ostream& os, const Competitor& competitor) {
  os << "comp," << "0," << competitor.num_ << "," << competitor.grid_pos_ << ","
     << competitor.short_name_ << "," << competitor.name_ << ","
     << competitor.team_ << std::endl;

  return os;
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

class Lap {
 public:
  Lap()
    : num_(0),
      competitor_num_(0),
      gap_(""),
      time_("") {
 }

 public:
  void set_num(int val) { num_ = val; }
  int competitor_num() const { return competitor_num_; }

  operator std::string() const {
    std::stringstream ss;
    ss << *this;

    return ss.str();
  }

 private:
  friend std::istream& operator>>(std::istream& is, Lap& lap);
  friend std::ostream& operator<<(std::ostream& os, const Lap& lap);

  int num_;
  int competitor_num_;
  std::string gap_;
  std::string time_;
};

std::istream& operator>>(std::istream& is, Lap& lap) {
  std::string str;
  std::getline(is, str);

  boost::smatch m;
  if (boost::regex_search(str, m, boost::regex(R"(^\d+)")))
    lap.competitor_num_ = boost::lexical_cast<int>(m.str());

  if (boost::regex_search(str, m, boost::regex(R"(((?<= )\d+\.\d+)|(\d+ LAPS?)|(PIT))")))
    lap.gap_ = m.str();
  else
    lap.gap_ = "0.000";

  if (boost::regex_search(str, m, boost::regex(R"(\d+:\d+\.\d+)")))
    lap.time_ = m.str();

  return is;
}

std::ostream& operator<<(std::ostream& os, const Lap& lap) {
  os << "lap," << "RT," << lap.competitor_num_ << "," << lap.gap_ << "," << lap.time_ << std::endl;
  return os;
}

void ReadRaceHist(LapVec* laps) {
  if (!laps) return;

  std::ifstream file;
  file.open("race_hist.txt");

  // Competitor num followed by any other data (upto the next competitor num).
  boost::regex rgx(R"(((?<=\s)[0-9]+\s)(.(?!(?1)))*)");

  std::string line;
  std::map<int,int> competitor_lap_count;
  boost::sregex_token_iterator end;
  while (std::getline(file, line)) {
    boost::sregex_token_iterator iter(line.cbegin(), line.cend(), rgx, 0);
    for (; iter != end; ++iter) {
      auto lap = boost::lexical_cast<Lap>(boost::trim_copy(iter->str()));
      lap.set_num(++competitor_lap_count[lap.competitor_num()]);
      laps->push_back(lap);
    }
  }
}

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

  ReadRaceHist(&laps);
  std::copy(laps.begin(), laps.end(), std::back_inserter(msgs));

  std::copy(msgs.begin(), msgs.end(),
      std::ostream_iterator<std::string>(std::cout));

  while (!events.empty()) {
    std::cout << events.top().first << " " << events.top().second;
    events.pop();
  }
}
