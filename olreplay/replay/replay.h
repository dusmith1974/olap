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

// Contains a class representing a Replay.

#ifndef OLREPLAY_REPLAY_REPLAY_H_
#define OLREPLAY_REPLAY_REPLAY_H_

#include <string>

#include "boost/noncopyable.hpp"

#include "osoa/service/service.h"

namespace olap {
// The Replay class.
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif
class Replay final : public osoa::Service, private boost::noncopyable {
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

 public:
  Replay();
  virtual ~Replay();

  // Initializes the service ready for use and adds command line options
  // specific to this service.
  osoa::Error Initialize(int argc, const char* argv[]) override;

  // Starts the service, logs messages and connects to other services.
  osoa::Error Start() override;

  // Stops the service.
  osoa::Error Stop() override;

  void AddTopicMessage(const std::string& topic,
                       const std::string& message, int num);

  // TODO(ds) mv to base?
  bool publishing() const;
  void set_publishing(bool val);

 private:
  typedef Service super;

  bool publishing_;
};
}  // namespace olap

#endif  // OLREPLAY_REPLAY_REPLAY_H_
