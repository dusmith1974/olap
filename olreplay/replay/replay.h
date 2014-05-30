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

#ifndef REPLAY__REPLAY_H_
#define REPLAY__REPLAY_H_

#include "boost/noncopyable.hpp"

#include "osoa/service/service.h"

namespace olap {

// The Replay class.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
class Replay final : public osoa::Service, private boost::noncopyable {
#pragma GCC diagnostic pop
 public:
  Replay();
  virtual ~Replay();

  // Initializes the service ready for use and adds command line options
  // specific to this service.
  osoa::Error Initialize(int argc, const char *argv[]) override;

  // Starts the service, logs messages and connects to other services.
  osoa::Error Start() override;

  // Stops the service.
  osoa::Error Stop() override;

 private:
  typedef Service super;
};

}  // namespace olap

#endif  // REPLAY__REPLAY_H_
