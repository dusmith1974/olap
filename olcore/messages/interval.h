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

// Contains a class representing an Xyz.

#ifndef MESSAGES_INTERVAL_H_
#define MESSAGES_INTERVAL_H_

#include <iostream>

namespace olap {

// The lap message.
class Xyz /*final : public Message*/ {
 public:
  Xyz();
  /*virtual*/ ~Xyz();
};

}  // namespace olap

#endif  // MESSAGES_INTERVAL_H_
