// Copyright 2015 Duncan Smith
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

// Pre-compiled header.

#ifndef OLCORE_OLCORE_PCH_H_
#define OLCORE_OLCORE_PCH_H_

#ifdef USE_PCH
#if defined(_MSC_VER) && _MSC_VER >= 1400

#include <fstream>
#include <functional>
#include <iomanip>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "boost/chrono.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/multiprecision/cpp_dec_float.hpp"
#include "boost/optional.hpp"
#include "boost/ptr_container/ptr_map.hpp"
#include "boost/regex.hpp"

#endif
#endif  // USE_PCH
#endif  // OLCORE_OLCORE_PCH_H_
