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

#include "replay/replay.h"

#include "osoa/service/logging.h"
#include "osoa/service/comms.h"

using osoa::Error;
using osoa::Logging;

namespace olap {

Replay::Replay() {
}

Replay::~Replay() {
}

// Add customizations specific to this particular service.
Error Replay::Initialize(int argc, const char *argv[]) {
  /*po::options_description& config = args()->config();

  // Add a command line option (for the number of times to log the test msg).
  auto msg_count_option =
    new po::typed_value<decltype(msg_count_)>(&msg_count_);
  msg_count_option->value_name("number");
  config.add_options()
    ("msg-count,o", msg_count_option, "number of msgs");

  // Set the callback handler for the listening port when connections are made.
  comms()->set_on_connect_callback(std::bind(&Test::OnConnect, this));*/

  return super::Initialize(argc, argv);
}

// Starts the base class service, logs messages and connects to other services.
Error Replay::Start() {
  Error code = super::Start();
  if (Error::kSuccess != code) return code;

  BOOST_LOG_SEV(*Logging::logger(), blt::debug) << "Replaying race.";

  for (const auto& subscription : comms()->subscriptions()) {
    code = comms()->Subscribe(subscription.second);
    if (Error::kSuccess == code) BOOST_LOG_SEV(*Logging::logger(), blt::debug)
      << "Subscribed to TODO(ds) format subscription name.";
  }

  return Error::kSuccess;
}

// No tidy up is required except to stop the base class service.
Error Replay::Stop() {
  return super::Stop();
}

void Replay::AddTopicMessage(const std::string& topic, const std::string& message, int num) {
  comms()->AddTopicMessage(topic, message, num);
}

}  // namespcae olap
