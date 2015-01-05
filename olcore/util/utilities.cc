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

// General utility functions.

#include "olcore_pch.h"  // NOLINT

#include "util/utilities.h"

#include <utility>  // NOLINT

#include "messages/competitor.h"

namespace olap {
  const optional<CompetitorMap::mapped_type&>
  FindPole(CompetitorMap* competitors) {
    if (!competitors) return optional<CompetitorMap::mapped_type&>();

    auto pole = std::find_if(competitors->begin(), competitors->end(),
                             [](const std::pair<int, Competitor>& elem) {
      return elem.second.grid_pos() == 1;
    });

    return (pole == competitors->end())
      ? optional<CompetitorMap::mapped_type&>()
      : optional<CompetitorMap::mapped_type&>(pole->second);
  }
}  // namespace olap
