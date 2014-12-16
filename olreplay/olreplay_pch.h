#ifndef OLREPLAY_PCH_H_
#define OLREPLAY_PCH_H_

#ifdef USE_PCH
#if defined(_MSC_VER) && _MSC_VER >= 1400 

#include <algorithm>
#include <cmath>
#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "boost/enable_shared_from_this.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/noncopyable.hpp"
#include "boost/optional.hpp"
#include "boost/range/adaptor/map.hpp"
#include "boost/range/algorithm/copy.hpp"
#include "boost/regex.hpp"
#include "boost/shared_ptr.hpp"

#endif
#endif  // USE_PCH
#endif  // OLREPLAY_PCH_H_
