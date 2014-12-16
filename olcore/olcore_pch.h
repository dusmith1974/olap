#ifndef OLCORE_PCH_H_
#define OLCORE_PCH_H_

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
#endif  // OLCORE_PCH_H_
