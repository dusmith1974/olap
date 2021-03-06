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

// Implements the base class for all timing messages.

#include "olcore_pch.h"  // NOLINT

#include "messages/message.h"

#include "boost/bind.hpp"

namespace asio = boost::asio;

namespace olap {
LongInterval Message::race_start_time_;
std::atomic<int> Message::quick_time_{500};

Message::Message()
  : race_time_(Interval()),
    time_of_day_{},
    timer_{} {
}

Message::~Message() {
}

void Message::set_timer(boost::asio::io_service* service) {
  if (!service) return;

  Interval time = race_time();
  // #define OLAP_QUICK_RACE
#ifdef OLAP_QUICK_RACE
  quick_time_ += 100;
  time = Interval(quick_time_);
#endif  // OLAP_QUICK_RACE

  timer_ = std::shared_ptr<boost::asio::deadline_timer>(
             new asio::deadline_timer(*service,
                                      boost::posix_time::milliseconds(time)));
}

void Message::start_timer(std::function<void(const boost::system::error_code&,
                          const std::string&)> fn) {
  using boost::bind;
  timer_->async_wait(bind(fn,
                          asio::placeholders::error,
                          static_cast<std::string>(*this)));
}

Message::operator std::string() const {
  std::stringstream ss;
  ss << *this;

  return ss.str();
}

LongInterval Message::race_start_time() { return race_start_time_; }

void Message::set_race_start_time(const LongInterval& val) {
  race_start_time_ = val;
}

LongInterval Message::time_of_day() const { return time_of_day_; }
void Message::set_time_of_day(const LongInterval& val) { time_of_day_ = val; }

Interval Message::race_time() const { return race_time_; }

void Message::set_race_time(const Interval& val) {
  race_time_ = val;
  time_of_day_ = LongInterval(Message::race_start_time() + race_time());
}

void Message::reset_quick_time() {
  quick_time_ = 500;
}

std::ostream& operator<<(std::ostream& os, const Message& message) {  // NOLINT
  message.Print(os);
  return os;
}
}  // namespace olap
