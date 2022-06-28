#pragma once

#include "nuisance/hist/make_axis.hxx"

#include "spdlog/spdlog.h"

namespace nuis {

namespace hist {

using hist = boost::histogram::histogram<
    std::vector<nuis::hist::axis>,
    boost::histogram::storage_adaptor<
        std::vector<boost::histogram::accumulators::weighted_sum<>>>>;

/*
 * Constructs a nuis::hist::histogram from a YAML descriptor of the format
 *
 * ---
 * axes:
 * -
 *   uniform_bins: [ <nbins>, <axis_low_edge>, <axis_high_edge> ]
 *   name: "some name"
 * -
 *   uniform_bins: [ <nbins>, <axis_low_edge>, <axis_high_edge> ]
 *   name: "some name"
 * content:
 *   has_flow_bins: true
 *   values: [ ... ]
 *   error: [ ... ]
 * ...
 *
 */
inline hist make_hist(YAML::Node const &hist_descriptor) {
  std::vector<nuis::hist::axis> axes;
  for (auto const &ax : hist_descriptor["axes"]) {
    axes.emplace_back(make_axis(ax));
  }

  hist rtn = boost::histogram::make_weighted_histogram(std::move(axes));

  if (!hist_descriptor["content"] || !hist_descriptor["content"]["values"]) {
    return rtn;
  }

  std::vector<double> values =
      hist_descriptor["content"]["values"].as<std::vector<double>>();

  std::vector<double> errors =
      hist_descriptor["content"]["errors"].as<std::vector<double>>(
          std::vector<double>{});

  auto has_flow_bins =
      bool(hist_descriptor["content"]["has_flow_bins"])
          ? hist_descriptor["content"]["has_flow_bins"].as<std::string>()
          : "";

  spdlog::critical("has_flow_bins: {}", has_flow_bins);

  size_t ctr = 0;
  for (auto &&x :
       indexed(rtn, (has_flow_bins=="true") ? boost::histogram::coverage::all
                                  : boost::histogram::coverage::inner)) {
    if (ctr >= values.size()) {
      std::stringstream ss("");
      ss << "{";
      for (size_t i = 0; i < hist_descriptor["axes"].size(); ++i) {
        ss << x.index(i) << ", ";
      }
      ss << "}";

      spdlog::critical("When parsing histogram, tried to read values entry "
                       "{}, which does not exist, into bin: {}",
                       ctr, ss.str());
      throw;
    }

    *x = boost::histogram::accumulators::weighted_sum<>{
        values[ctr], std::pow((errors.size() > ctr) ? errors[ctr] : 0, 2)};
    ctr++;
  }

  return rtn;
}

} // namespace hist

} // namespace nuis