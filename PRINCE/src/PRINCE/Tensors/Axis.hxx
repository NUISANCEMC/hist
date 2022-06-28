#pragma once

#include "Eigen/CXX11/Tensor"

#include "spdlog/spdlog.h"

namespace prince {

namespace detail {

constexpr static long kFailSearch = std::numeric_limits<long>::max();
constexpr static long kOOR_High = std::numeric_limits<long>::max() - 1;
constexpr static long kOOR_Low = std::numeric_limits<long>::max() - 2;

long BinarySearchBin(double const *bin_edges, long const NBins,
                     double const v) {
  // -- Can do both of these checks once when axes are built.
  // TODO: Check NBins >= 1
  // TODO: Check bin ordering and no duplicates

  spdlog::debug("BinarySearchBin ------ Start -------");
  spdlog::debug("NBins: {} v: {}", NBins, v);

  if (v < bin_edges[0]) {
    spdlog::debug("Out of range High: {} < {}", v, bin_edges[0]);
    spdlog::debug("BinarySearchBin ------ End -------");
    return kOOR_Low;
  } else if (v >= bin_edges[NBins]) {
    spdlog::debug("Out of range High: {} >= {}", v, bin_edges[NBins]);
    spdlog::debug("BinarySearchBin ------ End -------");
    return kOOR_High;
  }

  // Do the binary search

  long L = 0;
  long R = NBins;

  while (L < R) {
    long i = (L + R) / 2;

    spdlog::debug("L: {}, R: {}, bin[{}] = [{}, {}]", L, R, i, bin_edges[i],
                  bin_edges[i + 1]);

    if (v < bin_edges[i]) {
      spdlog::debug("{} < {}", v, bin_edges[i], bin_edges[i + 1]);
      R = i;
    } else if (v > bin_edges[i + 1]) {
      spdlog::debug("{} >= {}", v, bin_edges[i + 1], bin_edges[i + 1]);
      L = i;
    } else {
      spdlog::debug("Found bin: {}, {} <= {} < {}", i, bin_edges[i], v,
                    bin_edges[i + 1]);
      spdlog::debug("BinarySearchBin ------ End -------");
      return i;
    }
  }

  spdlog::debug("BinarySearchBin ------ End -------");
  return kFailSearch;
}
} // namespace detail

/// Interface for Axes
///
/// Axes are functions that map a vector of input values onto one unsigned
/// integer
///
/// They must also being able to pre-compute the maximum integer that they will
/// return given a specified, possibly multi-dimensional, binning scheme
class Axes {
public:
  long NDims;

  virtual long FindBin_nocheck(double const *x) const = 0;

  long FindBin(std::vector<double> const &x) {
    assert(x.size() == NDims);
    return FindBin_nocheck(x.data());
  }

  virtual long NBins() const = 0;

  virtual ~Axes() {}

protected:
  Axes(long D) : NDims(D) {}
};

/// Simple 1D axis object
class Axis : public Axes {
public:
  Axis(std::vector<double> bin_edges) : Axes(1), fbin_edges(bin_edges){};

  long FindBin(double const &x) const {
    return detail::BinarySearchBin(fbin_edges.data(), fbin_edges.size() - 1, x);
  }

  long FindBin_nocheck(double const *x) const override { return FindBin(x[0]); }

  long NBins() const override { return (fbin_edges.size() - 1); }

private:
  std::vector<double> fbin_edges;
};

/// Rectangular multi-dimensional, independent list of axes
class AxesRectangular : public Axes {
public:
  AxesRectangular(std::initializer_list<Axis> axes)
      : Axes(axes.size()), fSubAxes(axes),
        fDimensionBinMultipliers(axes.size()) {
    fDimensionBinMultipliers[0] = 1;
    for (long i = 1; i < NDims; ++i) {
      fDimensionBinMultipliers[i] =
          fDimensionBinMultipliers[i - 1] * fSubAxes[i].NBins();
    }

    fNBins =
        std::accumulate(fSubAxes.begin(), fSubAxes.end(), long(1),
                        [](long v, Axis const &a) { return v * a.NBins(); });
  }

  long FindBin_nocheck(double const *x) const override {
    spdlog::debug("AxesRectangular<{}>::FindBin ------ Start -------", NDims);
    long global_bin = 0;
    for (long i = 0; i < NDims; ++i) {
      long ax_bin = fSubAxes[i].FindBin(x[i]);
      spdlog::debug("Axis {} bin = {}", i, ax_bin);
      global_bin += ax_bin * fDimensionBinMultipliers[i];
      spdlog::debug("Dimensional[{}] bin multiplier: {}", i, fDimensionBinMultipliers[i]);
      spdlog::debug("Running global bin {}", global_bin);
    }
    spdlog::debug("Found global bin: {}", global_bin);
    spdlog::debug("AxesRectangular<{}>::FindBin ------ End -------", NDims);
    return global_bin;
  }

  std::vector<long> DecomposeGlobalBin(long gbin) {
    spdlog::debug(
        "AxesRectangular<{}>::DecomposeGlobalBin ------ Start -------", NDims);

    spdlog::debug("Global bin = {}", gbin);
    std::vector<long> sub_axis_bins(NDims);

    for (long i = (NDims - 1); i > 0; --i) {
      sub_axis_bins[i] = gbin / fDimensionBinMultipliers[i];
      spdlog::debug("Axis {} bin = {}", i, sub_axis_bins[i]);
      gbin -= sub_axis_bins[i] * fDimensionBinMultipliers[i];
      spdlog::debug("global residual = {}", gbin);
    }
    sub_axis_bins[0] = gbin;
    spdlog::debug("Axis 0 bin = {}", sub_axis_bins[0]);

    spdlog::debug("AxesRectangular<{}>::DecomposeGlobalBin ------ End -------",
                  NDims);
    return sub_axis_bins;
  }

  long NBins() const override { return fNBins; }

private:
  std::vector<Axis> fSubAxes;
  std::vector<long> fDimensionBinMultipliers;
  long fNBins;
};

/// Enables non-uniform, rectangular binning, where the binning of each 'level'
/// of axis is dependent on the previous 'level'.
///
/// Defining these can be painful, we do not recommend going more than two
/// levels deep.
// class AxesHierarchial : public Axes {
//  public:
// template<long D>
// virtual long FindBin(std::array<double,D> const &x) const = 0;

// virtual long NBins() const = 0;

//  private:
//   Eigen::Tensor<double, NDims> Binning;
//   Eigen::Tensor<int, NDims> Binning_NBins;
// };

}; // namespace prince