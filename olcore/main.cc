// simple test app for dev
// TODO(ds) replace with google-test
#include <iostream>

#include "time/interval.h"

int main() {
  using olap::Interval;

  Interval a(5);
  std::cout << a << std::endl;

  Interval b(2);
  std::cout << b << std::endl;

  Interval c = Interval(a + b);
  std::cout << c << std::endl;
}
