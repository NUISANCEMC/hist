#pragma once

#include "nuisance/hist/make_axis.hxx"

#include "spdlog/spdlog.h"

namespace nuis {

namespace hist {

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
inline hist make_hist_nuisance(YAML::Node const &hist_descriptor) {
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

  bool has_flow_bins =
      hist_descriptor["content"]["has_flow_bins"].as<bool>(false);

  size_t ctr = 0;
  for (auto &&x :
       indexed(rtn, has_flow_bins ? boost::histogram::coverage::all
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

/*
 * Constructs a nuis::hist::histogram from a YAML descriptor of the format
 *
 * ---
 * independent_variables:
 * - header: {name: Leading dilepton PT, units: GEV}
 *   values:
 *   - {low: 0, high: 60}
 *   - {low: 60, high: 100}
 *   - {low: 100, high: 200}
 *   - {low: 200, high: 600}
 * dependent_variables:
 * - header: {name: 10**6 * 1/SIG(fiducial) * D(SIG(fiducial))/DPT,
 *            units: GEV**-1
 *           }
 *   qualifiers:
 *   - {name: RE,
 *      value: "P P --> Z0 < LEPTON+ LEPTON- > Z0 < LEPTON+ LEPTON-> X"}
 *   - {name: SQRT(S), units: GEV, value: 7000}
 *   values:
 *   - value: 7000
 *     errors:
 *     - {symerror: 1100, label: stat}
 *     - {symerror: 79, label: 'sys,detector'}
 *     - {symerror: 15, label: 'sys,background'}
 *   - value: 9800
 *     errors:
 *     - {symerror: 1600, label: stat}
 *     - {symerror: 75, label: 'sys,detector'}
 *     - {symerror: 15, label: 'sys,background'}
 *   - value: 1600
 *     errors:
 *     - {symerror: 490, label: stat}
 *     - {symerror: 41, label: 'sys,detector'}
 *     - {symerror: 2, label: 'sys,background'}
 *   - value: 80
 *     errors:
 *     - {symerror: 60, label: stat}
 *     - {symerror: 2, label: 'sys,detector'}
 *     - {symerror: 0, label: 'sys,background'}
 * ...
 *
 */

inline hist make_hist_HepData(YAML::Node const &hist_descriptor,
                              std::string const &error_label) {

  std::vector<nuis::hist::axis> axes;
  std::vector<std::vector<int>> axis_bin_indices;

  for (auto &indep_var : hist_descriptor["independent_variables"]) {
    axis_bin_indices.emplace_back();
    std::string name = indep_var["header"]["name"].as<std::string>();
    std::string units = indep_var["header"]["units"].as<std::string>("");

    spdlog::info("Indep var: [ name: {}, units: {} ]: ", name, units);

    std::vector<std::array<double, 2>> bins;

    bool isContiguous = true;
    bool repeatedBins = false;

    int bin_index = 0;
    for (auto &bin : indep_var["values"]) {
      std::array<double, 2> bin_edges{bin["low"].as<double>(),
                                      bin["high"].as<double>()};
      if (!bins.size()) {
        bins.push_back(std::move(bin_edges));
        spdlog::info("\tAdded bin [{}, {}]", bins.back()[0], bins.back()[1]);
        axis_bin_indices.back().push_back(bin_index++);
        continue;
      }

      if (bin_edges[0] < bins.back()[1]) {

        int bin_it = 0;
        for (; bin_it < bins.size(); ++bin_it) {
          if ((bin_edges[0] == bins[bin_it][0]) &&
              (bin_edges[1] == bins[bin_it][1])) {
            repeatedBins = true;
            break;
          }
        }
        if (bin_it != bins.size()) {
          axis_bin_indices.back().push_back(bin_it);
          continue;
        }

        spdlog::critical(
            "Found bin: [{}, {}], which is in already added range: "
            "[{}, {}], but doesn't match an existing bin:",
            bin_edges[0], bin_edges[1], bins.front()[0], bins.back()[1]);
        for (auto &bin_def : bins) {
          spdlog::critical("\t[{}, {}]", bin_def[0], bin_def[1]);
        }

        throw;

      } else if (bin_edges[0] == bins.back()[1]) {
        bins.push_back(std::move(bin_edges));
        spdlog::info("\tAdded bin [{}, {}]", bins.back()[0], bins.back()[1]);
        axis_bin_indices.back().push_back(bin_index++);
      } else if (bin_edges[0] > bins.back()[1]) {
        isContiguous = false;
        spdlog::critical("New bin low edge {} is greater than last bin up edge "
                         "{}, this axis will be non-contiguous.",
                         bin_edges[0], bins.back()[1]);
        axis_bin_indices.back().push_back(bin_index++);
      }
    }

    spdlog::info("--Built HepData Independent Axis:");
    for (auto &bin_def : bins) {
      spdlog::info("\t[{}, {}]", bin_def[0], bin_def[1]);
    }

    if (!isContiguous) {
      spdlog::critical("Cannot currently instantiate non-contiguous axis.");
      throw;
    }

    axes.emplace_back(make_axis(name, units, bins));
  }

  for (int val_it = 0; val_it < axis_bin_indices.front().size(); ++val_it) {
    std::stringstream ss("");
    ss << "{";
    for (int ax_it = 0; ax_it < axis_bin_indices.size(); ++ax_it) {
      ss << axis_bin_indices[ax_it][val_it]
         << ((ax_it + 1 == axis_bin_indices.size()) ? " }" : ", ");
    }
    spdlog::info("--index: {}", ss.str());
  }

  hist rtn = boost::histogram::make_weighted_histogram(std::move(axes));

  if (!hist_descriptor["dependent_variables"]) {
    return rtn;
  }

  std::vector<double> values;
  std::vector<double> errors;

  for (auto var : hist_descriptor["dependent_variables"]) {
    for (auto val : var["values"]) {
      values.push_back(val["value"].as<double>());

      for (auto errsrc : val["errors"]) {
        if (errsrc["label"].as<std::string>("") == error_label) {
          errors.push_back(errsrc["symerror"].as<double>());
          break;
        }
      }
    }
  }

  if (values.size() != axis_bin_indices.front().size()) {
    spdlog::critical("When reading HepData histogram, the number of values "
                     "({}) didn't correspond to the number of bins ({}).",
                     values.size(), axis_bin_indices.front().size());
    throw;
  }

  size_t ctr = 0;

  for (int val_it = 0; val_it < axis_bin_indices.front().size(); ++val_it) {

    std::vector<double> index;
    for (int ax_it = 0; ax_it < axis_bin_indices.size(); ++ax_it) {
      index.push_back(axis_bin_indices[ax_it][val_it]);
    }

    rtn.at(index) = boost::histogram::accumulators::weighted_sum<>{
        values[ctr], std::pow((errors.size() > ctr) ? errors[ctr] : 0, 2)};
    ctr++;
  }

  return rtn;
}

inline hist make_hist(YAML::Node const &hist_descriptor,
                      std::string const &error_label = "stat") {
  if (bool(hist_descriptor["axes"]) && bool(hist_descriptor["content"])) {
    return make_hist_nuisance(hist_descriptor);
  } else if (bool(hist_descriptor["independent_variables"]) &&
             bool(hist_descriptor["dependent_variables"])) {
    return make_hist_HepData(hist_descriptor, error_label);
  }

  throw;
}

} // namespace hist

} // namespace nuis