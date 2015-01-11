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

// Recreates an events timing messages from the results.

#include "olreplay_pch.h"  // NOLINT

#include <iostream>  // NOLINT
#include <map>  // NOLINT
#include <numeric>  // NOLINT
#include <string>  // NOLINT
#include <thread>  // NOLINT
#include <vector>  // NOLINT

#include "replay/replay.h"
#include "osoa/service/logging.h"

namespace olap {
}  // namespace olap

int main(int argc, const char* argv[]) {
  olap::Replay replay;
  osoa::Error code = replay.Initialize(argc, argv);
  if (osoa::Error::kSuccess != code)
    return static_cast<int>(code);

  BOOST_LOG_SEV(*osoa::Logging::logger(), blt::info)
      << "olap run";

  if (osoa::Error::kSuccess == replay.Start())
    replay.Stop();

  return static_cast<int>(osoa::Error::kSuccess);
}
