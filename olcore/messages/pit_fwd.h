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

// Contains the forward declarations for a Pit message.

#ifndef MESSAGES__PIT_FWD_H_
#define MESSAGES__PIT_FWD_H_

#include <vector>

namespace olap {

class Pit;
typedef std::vector<Pit> PitVec;

}  // namespace olap

#endif  // MESSAGES__PIT_FWD_H_
