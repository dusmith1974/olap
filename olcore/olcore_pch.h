#ifndef OLCORE_PCH_H_
#define OLCORE_PCH_H_

#ifdef USE_PCH
#if defined(_MSC_VER) && _MSC_VER >= 1400 

#pragma warning(disable:4512)
#pragma warning(disable:4996)
#pragma warning(disable:4510)
#pragma warning(disable:4610)
#pragma warning(disable:4100)

#include <map>
#include <vector>
#include <functional>
#include <memory>
#include <string>
#include <iomanip>
#include <fstream>
#include <iosfwd>

#include "boost/multiprecision/cpp_dec_float.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/regex.hpp"
#include "boost/ptr_container/ptr_map.hpp"
#include "boost/optional.hpp"
#include "boost/chrono.hpp"

#pragma warning(default:4512)
#pragma warning(default:4996)
#pragma warning(default:4510)
#pragma warning(default:4610)
#pragma warning(default:4100)
#endif

#endif  // USE_PCH
#endif  // OLCORE_PCH_H_
