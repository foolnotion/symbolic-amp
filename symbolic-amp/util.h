#pragma once

#include "random.h"
#include "dataset.h"
#include <algorithm>

namespace util
{
  static dataset random_dataset(random* rnd, int nvariables, int nrows)
  {
    dataset ds;
    for (int i = 0; i < nvariables; ++i)
    {
      std::stringstream ss;
      ss << "x" << (i + 1);
      auto values = std::vector<double>(nrows);
      std::generate(begin(values), end(values), [&rnd]() { return rnd->next_double(); });
      ds.add(ss.str(), values);
    }
    return ds;
  }
}
