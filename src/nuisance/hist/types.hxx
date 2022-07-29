#pragma once

#include "boost/histogram.hpp"

namespace nuis {

namespace hist {

struct metadata {
  std::string name;
  std::string units;
};

using axis_regular = boost::histogram::axis::regular<>;
using axis_variable = boost::histogram::axis::variable<>;
using axis = boost::histogram::axis::variant<axis_regular, axis_variable>;


using hist = boost::histogram::histogram<
    std::vector<nuis::hist::axis>,
    boost::histogram::storage_adaptor<
        std::vector<boost::histogram::accumulators::weighted_sum<>>>>;

}

}