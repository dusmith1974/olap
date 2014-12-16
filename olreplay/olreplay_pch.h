#ifndef OLREPLAY_PCH_H_
#define OLREPLAY_PCH_H_

#ifdef USE_PCH
#if defined(_MSC_VER) && _MSC_VER >= 1400 

#pragma warning(disable:4512)
#pragma warning(disable:4996)
#pragma warning(disable:4510)
#pragma warning(disable:4610)
#pragma warning(disable:4100)

#include <thread>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <set>
#include <utility>
#include <vector>
#include <memory>
#include <deque>
#include <sstream>
#include <cmath>
#include <numeric>

#include "boost/lexical_cast.hpp"
#include "boost/range/adaptor/map.hpp"
#include "boost/range/algorithm/copy.hpp"
#include "boost/regex.hpp"
#include "boost/noncopyable.hpp"
#include "boost/asio.hpp"
#include "boost/optional.hpp"
#include "boost/bind.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/enable_shared_from_this.hpp"

#pragma warning(default:4512)
#pragma warning(default:4996)
#pragma warning(default:4510)
#pragma warning(default:4610)
#pragma warning(default:4100)
#endif
#endif  // USE_PCH
#endif  // OLREPLAY_PCH_H_
