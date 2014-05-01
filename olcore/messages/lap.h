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

// Contains a class representing a competitors lap.

#include <iostream>

// The lap message.
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
