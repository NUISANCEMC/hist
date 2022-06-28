#include "PRINCE/Tensors/Axis.hxx"

#include "spdlog/spdlog.h"

int main() {
  spdlog::set_pattern("[%^%l%$]: %v");

  spdlog::set_level(spdlog::level::debug);

  std::vector<double> bin_edges = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  {
    spdlog::info("------- Start: BinarySearchBin --------");

    long bin_OOR_Low = prince::detail::BinarySearchBin(
        bin_edges.data(), bin_edges.size() - 1, -1);
    assert(bin_OOR_Low == prince::detail::kOOR_Low);

    long bin_OOR_High = prince::detail::BinarySearchBin(
        bin_edges.data(), bin_edges.size() - 1, 11);
    assert(bin_OOR_High == prince::detail::kOOR_High);

    long bin_Range_Edge_Low = prince::detail::BinarySearchBin(
        bin_edges.data(), bin_edges.size() - 1, 0);
    assert(bin_Range_Edge_Low == 0);

    long bin_Range_Edge_High = prince::detail::BinarySearchBin(
        bin_edges.data(), bin_edges.size() - 1, 10);
    assert(bin_Range_Edge_High == prince::detail::kOOR_High);

    long bin_Bin_Edge_Low = prince::detail::BinarySearchBin(
        bin_edges.data(), bin_edges.size() - 1, 1);
    assert(bin_Bin_Edge_Low == 1);

    long bin_Bin_Edge_High = prince::detail::BinarySearchBin(
        bin_edges.data(), bin_edges.size() - 1, 2);
    assert(bin_Bin_Edge_High == 2);

    long bin_Bin = prince::detail::BinarySearchBin(bin_edges.data(),
                                                     bin_edges.size() - 1, 2.5);
    assert(bin_Bin == 2);

    spdlog::info("------- End: BinarySearchBin --------");
  }

  prince::Axis ax_1d(bin_edges);
  
  {
    spdlog::info("------- Start: prince::Axis --------");

    long bin_OOR_Low = ax_1d.FindBin(-1);
    assert(bin_OOR_Low == prince::detail::kOOR_Low);

    long bin_OOR_High = ax_1d.FindBin(11);
    assert(bin_OOR_High == prince::detail::kOOR_High);

    long bin_Range_Edge_Low = ax_1d.FindBin(0);
    assert(bin_Range_Edge_Low == 0);

    long bin_Range_Edge_High = ax_1d.FindBin(10);
    assert(bin_Range_Edge_High == prince::detail::kOOR_High);

    long bin_Bin_Edge_Low = ax_1d.FindBin(1);
    assert(bin_Bin_Edge_Low == 1);

    long bin_Bin_Edge_High = ax_1d.FindBin(2);
    assert(bin_Bin_Edge_High == 2);

    long bin_Bin = ax_1d.FindBin(2.5);
    assert(bin_Bin == 2);

    spdlog::info("------- End: prince::Axis --------");

  }

  {

    spdlog::info("------- Start: prince::AxesRectangular --------");

    prince::AxesRectangular ax_2d({ax_1d, ax_1d});

    long bin_x = ax_2d.FindBin({2,0});
    assert(bin_x == 2);

    long bin_y = ax_2d.FindBin({0,2});
    assert(bin_y == 20);

    long bin_xy = ax_2d.FindBin({5,2});
    assert(bin_xy == 25);

    auto bins = ax_2d.DecomposeGlobalBin(bin_xy);
    assert(bins[0] == 5);
    assert(bins[1] == 2);

    spdlog::info("------- End: prince::AxesRectangular --------");

  }
}