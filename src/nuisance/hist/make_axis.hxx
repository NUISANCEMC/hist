#pragma once

#include "boost/histogram.hpp"

#include "yaml-cpp/yaml.h"

namespace nuis {

namespace hist {

struct metadata {
  std::string name;
  std::string units;
};

using axis_regular = boost::histogram::axis::regular<>;
using axis_variable = boost::histogram::axis::variable<>;
using axis = boost::histogram::axis::variant<axis_regular, axis_variable>;

/*
 * Constructs a nuis::hist::axis from a YAML descriptor of the format
 *
 * ---
 * uniform_bins: [ <nbins>, <axis_low_edge>, <axis_high_edge> ]
 * bin_edges: [ 0, 1, 2.3, 5.6, ... ]
 * bin_steps: { low_edge: 0, steps: [ [10, 0.1], [20, 0.5], ... ] }
 * name: "some name"
 * units: "some units"
 * ...
 *
 * If more than one of uniform_bins, bin_edges, or bin_steps are found, an
 * exception will be raised.
 */
inline axis make_axis(YAML::Node const &axis_descriptor) {

  bool have_uniform_bins = bool(axis_descriptor["uniform_bins"]);
  bool have_bin_edges = bool(axis_descriptor["bin_edges"]);
  bool have_bin_steps = bool(axis_descriptor["bin_steps"]);

  if ((have_uniform_bins + have_bin_edges + have_bin_steps) != 1) {
    throw 1;
  }

  std::string name =
      axis_descriptor["name"] ? axis_descriptor["name"].as<std::string>() : "";
  std::string units = axis_descriptor["units"]
                          ? axis_descriptor["units"].as<std::string>()
                          : "";

  if (have_uniform_bins) {
    auto uniform_bins_node = axis_descriptor["uniform_bins"];
    if (uniform_bins_node.size() != 3) {
      throw 1;
    }

    int n = uniform_bins_node[0].as<int>();
    double low = uniform_bins_node[1].as<double>();
    double high = uniform_bins_node[2].as<double>();
    return axis_regular(n, low, high, name + "%20" + units);
  } else if (have_bin_edges) {
    std::vector<double> bin_edges;
    auto bin_edges_node = axis_descriptor["bin_edges"];
    std::for_each(
        bin_edges_node.begin(), bin_edges_node.end(),
        [&](auto const &v) { bin_edges.push_back(v.template as<double>()); });
    return axis_variable(bin_edges.begin(), bin_edges.end(),
                         name + "%20" + units);
  } else if (have_bin_steps) {
    std::vector<double> bin_edges;

    auto bin_steps_node = axis_descriptor["bin_steps"];
    bin_edges.push_back(bin_steps_node["low_edge"].as<double>());

    for (auto const &steps : bin_steps_node["steps"]) {
      for (int i = 0; i < steps[0].as<int>(); ++i) {
        bin_edges.push_back(bin_edges.back() + steps[1].as<double>());
      }
    }

    return axis_variable(bin_edges.begin(), bin_edges.end(),
                         name + "%20" + units);
  }

  throw 1;
}

} // namespace hist

} // namespace nuis