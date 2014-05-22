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

#ifndef MESSAGES_MESSAGE_H_
#define MESSAGES_MESSAGE_H_

#include <functional>
#include <memory>

#include "boost/asio.hpp"

#include "messages/message_fwd.h"
#include "time/interval.h"
#include "time/long_interval.h"

namespace olap {

// The message class.
class Message {
 public:
  Message();
  virtual ~Message();

  virtual Message* Clone() const = 0;

  void set_timer(boost::asio::io_service* service);

  void start_timer(std::function<void(const boost::system::error_code&, const std::string&)> fn);

  operator std::string() const;

  static LongInterval race_start_time();

  static void set_race_start_time(const LongInterval& val);

  LongInterval time_of_day() const;
  void set_time_of_day(const LongInterval& val);

  Interval race_time() const;

  void set_race_time(const Interval& val);

 protected:
  Interval race_time_;
  LongInterval time_of_day_;

 private:
  friend std::ostream& operator<<(std::ostream& os, const Message& message);

  static LongInterval race_start_time_;

  virtual void Print(std::ostream& os) const = 0;

  std::shared_ptr<boost::asio::deadline_timer> timer_;
};

}  // namespace olap
#endif  // MESSAGES_MESSAGE_H_
