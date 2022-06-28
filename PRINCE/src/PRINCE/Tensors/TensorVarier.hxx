#pragma once

#include "PRINCE/Tensors/CountingTensor.hxx"

#include <iterator>

namespace prince {

namespace detail {

template <typename StorageType>
Eigen::Tensor<StorageType, 3>
TensorVarierInitHelper(std::array<std::shared_ptr<Axes>, 1> axes, long NParams,
                       long NKnots) {
  return Eigen::Tensor<StorageType, 3>(NKnots, NParams, axes[0]->NBins())
      .setZero();
}

template <typename StorageType>
Eigen::Tensor<StorageType, 4>
TensorVarierInitHelper(std::array<std::shared_ptr<Axes>, 2> axes, long NParams,
                       long NKnots) {
  return Eigen::Tensor<StorageType, 4>(NKnots, NParams, axes[0]->NBins(),
                                       axes[1]->NBins())
      .setZero();
}

template <typename StorageType>
Eigen::Tensor<StorageType, 5>
TensorVarierInitHelper(std::array<std::shared_ptr<Axes>, 3> axes, long NParams,
                       long NKnots) {
  return Eigen::Tensor<StorageType, 5>(NKnots, NParams, axes[0]->NBins(),
                                       axes[1]->NBins(), axes[2]->NBins())
      .setZero();
}

template <std::size_t Rank>
using ParamVarBinIndex = typename std::array<long, Rank>;

} // namespace detail

template <typename StorageType, std::size_t Rank, long NParams = 1>
class TensorVarier {
public:
  using TensorType = CountingTensor<StorageType, Rank>;

  virtual TensorType Vary_nocheck(TensorType const &ct, double const *x) = 0;
  TensorType Vary(TensorType const &ct, std::array<double, NParams> const &x) {
    return Vary_nocheck(ct, x.data());
  }
};

struct PiecewiseParam {
  double Low;
  double CV;
  double High;
};

template <typename StorageType, std::size_t Rank, long NParams = 1>
class TensorVarier1DPiecewiseLinear
    : public TensorVarier<StorageType, Rank, NParams> {

public:
  using TensorType = CountingTensor<StorageType, Rank>;
  using Index_t = detail::ParamVarBinIndex<Rank + 2>;
  using BinIndex_t = detail::BinIndex<Rank>;

  TensorVarier1DPiecewiseLinear(
      std::array<std::shared_ptr<Axes>, Rank> axes,
      std::array<PiecewiseParam, NParams> params_layout)
      : Storage(detail::TensorVarierInitHelper<StorageType>(axes, NParams, 2)),
        fAxes(axes), fParamKnots(params_layout), fFinalized(false) {
    NDims = detail::GetNDims<Rank>(axes);
  }

  void Fill(std::vector<double> const &x, long param, long knot,
            StorageType weight = 1) {
    assert(!fFinalized);
    assert(x.size() == NDims);
    assert(param < NParams);
    assert((knot == -1) || (knot == 1));

    Index_t index;

    BinIndex_t bin_index = detail::FindBin_nocheck<Rank>(fAxes, x.data());

    index[0] = (knot + 1) / 2;
    index[1] = param;
    std::copy_n(bin_index.begin(), NDims, &index[2]);

    Storage(index) += weight;
  }

  void Finalize(TensorType const &ct) {
    Index_t index;
    BinIndex_t bin_index;
    std::fill_n(bin_index.begin(), NDims, 0);

    std::array<long, Rank> binshape = detail::NBinsShape<Rank>(fAxes);

    do {
      for (long p = 0; p < NParams; ++p) {
        index[1] = p;

        for (long k = -1; k < 2; k += 2) {
          index[0] = (k + 1) / 2;
          std::copy_n(bin_index.begin(), NDims, &index[2]);

          spdlog::debug("Finalize: K = {}({}), CV = {}, V = {} -> {}", k,
                        index[0], ct.Storage(bin_index), Storage(index),
                        Storage(index) / ct.Storage(bin_index));

          Storage(index) /= ct.Storage(bin_index);

          if (!std::isnormal(Storage(index))) {
            Storage(index) = 0;
          }
        }
      }
    } while (detail::AdvanceIndex(bin_index, binshape));
    fFinalized = true;
  }

  TensorType Vary_nocheck(TensorType const &ct, double const *x) override {
    assert(fFinalized);

    TensorType out = ct;

    std::vector<std::tuple<long, double>> evaluator;

    for (int p = 0; p < NParams; ++p) {
      evaluator.emplace_back(0, 0);

      if (x[p] > fParamKnots[p].CV) {
        std::get<0>(evaluator.back()) = 1;
        std::get<1>(evaluator.back()) =
            (x[p] - fParamKnots[p].CV) /
            (fParamKnots[p].High - fParamKnots[p].CV);
      } else {
        std::get<0>(evaluator.back()) = 0;
        std::get<1>(evaluator.back()) =
            (fParamKnots[p].CV - x[p]) /
            (fParamKnots[p].CV - fParamKnots[p].Low);
      }

      spdlog::debug("eval(p[{}], v = {} -- [{},{},{}]): other = {}, dist = {}",
                    p, x[p], fParamKnots[p].Low, fParamKnots[p].CV,
                    fParamKnots[p].High, std::get<0>(evaluator.back()),
                    std::get<1>(evaluator.back()));
    }

    Index_t index;

    BinIndex_t bin_index;
    std::fill_n(bin_index.begin(), NDims, 0);
    // - This is really not using Eigen
    // - We know the binning is rectangular, probably don't need the 'advanceindex' call and a while loop

    std::array<long, Rank> binshape = detail::NBinsShape<Rank>(fAxes);

    do {
      std::copy_n(bin_index.begin(), NDims, &index[2]);
      double w = 1;
      for (long p = 0; p < NParams; ++p) {
        index[0] = std::get<0>(evaluator[p]);
        index[1] = p;

        w *= ((1.0 - std::get<1>(evaluator[p])) +
              std::get<1>(evaluator[p]) * Storage(index));

        spdlog::debug("CV = {}, Oth_frac = {}, dist = {} | W = {}, result = {}",
                      ct.Storage(bin_index), Storage(index),
                      std::get<1>(evaluator[p]), w, w * ct.Storage(bin_index));
      }
      out.Storage(bin_index) = ct.Storage(bin_index) * w;
    } while (detail::AdvanceIndex(bin_index, binshape));

    return out;
  }

  Eigen::Tensor<StorageType, Rank + 2> Storage;

  long NDims;

private:
  std::array<std::shared_ptr<Axes>, Rank> fAxes;
  std::array<PiecewiseParam, NParams> fParamKnots;

  bool fFinalized;
};

} // namespace prince