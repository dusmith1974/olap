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

// Contains a base class for all timing messages.

#include "boost/asio.hpp"

// The message class.
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
