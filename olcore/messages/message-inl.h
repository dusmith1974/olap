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

// Contains inline templates for the message class.

#ifndef MESSAGES_MESSAGE_INL_H_
#define MESSAGES_MESSAGE_INL_H_

#include "boost/ptr_container/ptr_map.hpp"

#include "messages/message.h"
#include "time/interval.h"

namespace olap {

typedef boost::ptr_multimap<Interval, Message> MessageMap;

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

}  // namespace olap
#endif  // MESSAGES_MESSAGE_INL_H_
