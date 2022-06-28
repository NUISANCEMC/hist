#pragma once

#include <memory>

#include "Eigen/CXX11/Tensor"
#include "PRINCE/Tensors/Axis.hxx"
#include "spdlog/spdlog.h"

namespace prince {

namespace detail {

template <typename StorageType>
Eigen::Tensor<StorageType, 1>
TensorInitHelper(std::array<std::shared_ptr<Axes>, 1> axes) {
  return Eigen::Tensor<StorageType, 1>(axes[0]->NBins()).setZero();
}

template <typename StorageType>
Eigen::Tensor<StorageType, 2>
TensorInitHelper(std::array<std::shared_ptr<Axes>, 2> axes) {
  return Eigen::Tensor<StorageType, 2>(axes[0]->NBins(), axes[1]->NBins())
      .setZero();
}

template <typename StorageType>
Eigen::Tensor<StorageType, 3>
TensorInitHelper(std::array<std::shared_ptr<Axes>, 3> axes) {
  return Eigen::Tensor<StorageType, 3>(axes[0]->NBins(), axes[1]->NBins(),
                                       axes[2]->NBins())
      .setZero();
}

template <std::size_t Rank> using BinIndex = typename std::array<long, Rank>;

template <std::size_t Rank>
std::array<long, Rank>
NBinsShape(std::array<std::shared_ptr<Axes>, Rank> const &axes) {
  std::array<long, Rank> bin_ranges;
  for (long i = 0; i < Rank; ++i) {
    bin_ranges[i] = axes[i]->NBins();
  }
  return bin_ranges;
}

template <std::size_t Rank>
bool AdvanceIndex(BinIndex<Rank> &cbin, std::array<long, Rank> bins_shape) {
  for (long i = (Rank - 1); i >= 0; i--) {
    cbin[i]++;

    if (cbin[i] >= bins_shape[i]) {
      // Hit the top of the last binning
      if (!i) {
        return false;
      }

      cbin[i] = 0;
    } else {
      break;
    }
  }
  return true;
}

template <std::size_t Rank>
BinIndex<Rank>
FindBin_nocheck(std::array<std::shared_ptr<Axes>, Rank> const &axes,
                double const *x) {
  BinIndex<Rank> binning;

  long first_el = 0;
  for (long i = 0; i < Rank; ++i) {
    binning[i] = axes[i]->FindBin_nocheck(&x[first_el]);
    first_el += axes[i]->NDims;
  }

  return binning;
}

template <std::size_t Rank>
long GetNDims(std::array<std::shared_ptr<Axes>, Rank> const &axes) {
  long NDims = 0;
  for (long i = 0; i < Rank; ++i) {
    NDims += axes[i]->NDims;
    spdlog::debug("CountingTensor axis[{}]: NDims: {}, running total: {}", i,
                  axes[i]->NDims, NDims);
  }
  return NDims;
}

} // namespace detail

/// A histogram-like tensor object
///
/// Can be 'filled' with values and weights and tracks bin contents like a
/// histogram.
///
/// \note It is named CountingTensor to try and highlight the disparity between
/// the rank of the tensor and the dimensionality of the equivalent histogram.
/// Each 'dimension' of the tensor can contain a possibly multi-dimensional
/// prince::Axes which has been flattened to a single dimension in the tensor
/// implementation.
template <typename StorageType, std::size_t Rank> class CountingTensor {
public:
  Eigen::Tensor<StorageType, Rank> Storage;
  using Index_t = detail::BinIndex<Rank>;

  CountingTensor(std::array<std::shared_ptr<Axes>, Rank> axes)
      : Storage(detail::TensorInitHelper<StorageType>(axes)), fAxes(axes) {
    NDims = detail::GetNDims<Rank>(axes);
  }

  Index_t FindBin(std::vector<double> const &x) const {
    assert(x.size() == NDims);
    return detail::FindBin_nocheck<Rank>(fAxes, x.data());
  }

  virtual void Fill(std::vector<double> const &x, StorageType weight = 1) {
    Index_t index = FindBin(x);
    Storage(index) += weight;
  }

  long NDims;

private:
  std::array<std::shared_ptr<Axes>, Rank> fAxes;
};

template <typename StorageType, std::size_t Rank>
class CountingTensorErrors : public CountingTensor<StorageType, Rank> {
public:
  Eigen::Tensor<StorageType, Rank> ErrorStorage;
  using Index_t = typename CountingTensor<StorageType, Rank>::Index_t;

  CountingTensorErrors(std::array<std::shared_ptr<Axes>, Rank> axes)
      : CountingTensor<StorageType, Rank>(axes),
        ErrorStorage(detail::TensorInitHelper<StorageType>(axes)) {}

  void Fill(std::vector<double> const &x, StorageType weight = 1) override {
    Index_t index = CountingTensor<StorageType, Rank>::FindBin(x);
    this->Storage(index) += weight;
    ErrorStorage(index) += weight * weight;
  }
};
} // namespace prince
