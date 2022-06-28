#include "PRINCE/Tensors/CountingTensor.hxx"

int main() {
  spdlog::set_pattern("[%^%l%$]: %v");

  spdlog::set_level(spdlog::level::debug);

  std::vector<double> bin_edges = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  {
    spdlog::info("------- Start: prince::CountingTensor --------");

    prince::CountingTensor<double, 1> h1(
        std::array<std::shared_ptr<prince::Axes>, 1>{
            std::make_shared<prince::Axis>(bin_edges)});

    auto bin = h1.FindBin({
        2.5,
    });
    assert(bin[0] == 2);

    h1.Fill({
        2.5,
    });
    assert(h1.Storage(bin) == 1);

    h1.Fill(
        {
            2.5,
        },
        0.5);
    assert(h1.Storage(bin) == 1.5);

    spdlog::info("------- End: prince::CountingTensor --------");
  }

  {
    spdlog::info("------- Start: prince::AdvanceIndex --------");

    prince::CountingTensorErrors<double, 1> h1(
        std::array<std::shared_ptr<prince::Axes>, 1>{
            std::make_shared<prince::Axis>(bin_edges)});

    spdlog::info("------- End: prince::AdvanceIndex --------");
  }
}