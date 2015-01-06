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

// Implements the Replay class.

#include "olreplay_pch.h"  // NOLINT
#include "replay/replay.h"

#include <vector>  // NOLINT

#include "boost/algorithm/string.hpp"

#include "osoa/service/args.h"
#include "osoa/service/comms/comms.h"
#include "osoa/service/logging.h"

using osoa::Error;
using osoa::Logging;

namespace olap {
  Replay::Replay() : publishing_(false) {
  }

  Replay::~Replay() {
  }

  // Add customizations specific to this particular service.
  Error Replay::Initialize(int argc, const char *argv[]) {
    Error result = super::Initialize(argc, argv);

    if (!args()->listening_port().empty())
      set_publishing(true);

    return result;
  }

  // Starts the base class service, logs messages and connects to other
  // services.
  Error Replay::Start() {
    Error code = super::Start();
    if (Error::kSuccess != code) return code;

    if (publishing()) {
      BOOST_LOG_SEV(*Logging::logger(), blt::debug) << "Replaying race.";
    } else {
      if (args()->services().size()) {
        // Split "127.0.0.1:8000" into "127.0.0.1" and "8000"
        std::vector<std::string> tokens;
        std::string service = args()->services()[0];
        boost::split(tokens, service, boost::is_any_of(":"));

        if (tokens.size() > 1) {
          code = comms()->Subscribe(tokens[0], tokens[1]);
        }
      }
    }
    return Error::kSuccess;
  }

  // No tidy up is required except to stop the base class service.
  Error Replay::Stop() {
    comms()->Shutdown();  // mv into super::Stop() ?
    return super::Stop();
  }

  // TODO(ds) ren PublishMessage, drop num.
  void Replay::AddTopicMessage(const std::string& topic,
                               const std::string& message, int num) {
    (void)num;
    (void)topic;
    comms()->PublishMessage(message);
  }

  bool Replay::publishing() const {
    return publishing_;
  }

  void Replay::set_publishing(bool val) {
    publishing_ = val;
  }
}  // namespace olap
