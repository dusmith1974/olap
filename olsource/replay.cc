#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/copy.hpp>

namespace adaptors = boost::adaptors;

class Competitor;

typedef std::vector<std::string> MsgVec;
typedef std::map<int, Competitor> CompetitorMap;

typedef std::pair<int, std::string> Event;

struct EventPQueueSortCriterion {
  bool operator()(const Event& a, const Event& b) const {
    return a.first > b.first;
  }
};

typedef std::priority_queue<Event, std::vector<Event>,
                            EventPQueueSortCriterion> EventPQueue;

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

int main() {
  MsgVec msgs;
  EventPQueue events;
  CompetitorMap competitors;

  ReadCompetitors(&competitors);
  /*boost::copy(competitors | adaptors::map_values,
              std::ostream_iterator<Competitor>(std::cout));*/

  boost::copy(competitors | adaptors::map_values,
              std::back_inserter(msgs));

  std::copy(msgs.begin(), msgs.end(),
      std::ostream_iterator<std::string>(std::cout));

  while (!events.empty()) {
    std::cout << events.top().first << " " << events.top().second;
    events.pop();
  }
}
