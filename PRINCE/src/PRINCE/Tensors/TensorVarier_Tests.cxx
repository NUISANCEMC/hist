#include "PRINCE/Tensors/TensorVarier.hxx"

int main() {
  spdlog::set_pattern("[%^%l%$]: %v");

  spdlog::set_level(spdlog::level::debug);

  std::vector<double> bin_edges = {0, 1, 2, 3};

  {
    spdlog::info(
        "------- Start: prince::TensorVarier1DPiecewiseLinear --------");

    prince::CountingTensor<double, 1> h1(
        std::array<std::shared_ptr<prince::Axes>, 1>{
            std::make_shared<prince::Axis>(bin_edges)});

    prince::TensorVarier1DPiecewiseLinear<double, 1, 1> v1(
        std::array<std::shared_ptr<prince::Axes>, 1>{
            std::make_shared<prince::Axis>(bin_edges)},
        std::array<prince::PiecewiseParam, 1>{prince::PiecewiseParam{-1, 0, 1}});

    h1.Fill({0.5}, 2);
    v1.Fill(
        {
            0.5,
        },
        0, -1, 4);

    v1.Fill(
        {
            0.5,
        },
        0, 1, 1);

    h1.Fill({1.5}, 1);
    v1.Fill(
        {
            1.5,
        },
        0, -1, 0);

    v1.Fill(
        {
            1.5,
        },
        0, 1, 2);

    h1.Fill({2.5}, 1);
    v1.Fill(
        {
            2.5,
        },
        0, -1, 1.2);

    v1.Fill(
        {
            2.5,
        },
        0, 1, 0.8);

    v1.Finalize(h1);

    decltype(h1) h2_CV = v1.Vary(h1, {0});
    decltype(h1) h2_p1 = v1.Vary(h1, {1});
    decltype(h1) h2_half = v1.Vary(h1, {0.5});
    decltype(h1) h2_mhalf = v1.Vary(h1, {-0.5});

    std::stringstream ss("");
    ss << "Tensor: \n" << h1.Storage;
    spdlog::debug("{}", ss.str());

    ss.str("");
    ss << "Tensor Varier: \n" << v1.Storage;
    spdlog::debug("{}", ss.str());

    ss.str("");
    ss << "TensorVarier(p = 0): \n" << h2_CV.Storage;
    spdlog::debug("{}", ss.str());

        ss.str("");
    ss << "TensorVarier(p = 1): \n" << h2_p1.Storage;
    spdlog::debug("{}", ss.str());

        ss.str("");
    ss << "TensorVarier(p = 0.5): \n" << h2_half.Storage;
    spdlog::debug("{}", ss.str());

        ss.str("");
    ss << "TensorVarier(p = -0.5): \n" << h2_mhalf.Storage;
    spdlog::debug("{}", ss.str());

    spdlog::info("------- End: prince::TensorVarier1DPiecewiseLinear --------");
  }
}