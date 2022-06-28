#include "nuisance/hist/make_axis.hxx"

#include "spdlog/spdlog.h"

#define assert_result(x)                                                       \
  if (!x) {                                                                    \
    spdlog::critical("Test Fail");                                             \
    return 1;                                                                  \
  };

int main() {

  spdlog::set_pattern("[%^%l%$]: %v");

  {
    spdlog::info("Axis test1: uniform_bins axis --- Test");

    YAML::Node node = YAML::Load(R"(
    uniform_bins: [ 100, -1, 1 ]
    name: "mytitle"
    )");

    auto ax = nuis::hist::make_axis(node);

    spdlog::info("Axis test1: uniform_bins axis --- Pass");
  }

  {
    spdlog::info("Axis test2: Variable axis --- Test");

    YAML::Node node = YAML::Load(R"(
    bin_edges: [ -0.5, 1.5, 6, 10 ]
    name: "myothertitle"
    )");

    auto ax = nuis::hist::make_axis(node);

    spdlog::info("Axis test2: Variable axis --- Pass");
  }

  {
    spdlog::info("Axis test3: No name --- Test");

    YAML::Node node = YAML::Load(R"(
    bin_edges: [ -0.5, 1.5, 6, 10 ]
    )");

    auto ax = nuis::hist::make_axis(node);

    spdlog::info("Axis test3: No name --- Pass");
  }

  try {
    spdlog::info("Axis test4: Both descriptors --- Test");

    YAML::Node node = YAML::Load(R"(
    uniform_bins: [ 100, -1, 1 ]
    bin_edges: [ -0.5, 1.5, 6, 10 ]
    )");

    auto ax = nuis::hist::make_axis(node);
    return 1;
  } catch (...) {
    spdlog::info("Axis test3: Both descriptors --- Pass");
  }

  try {
    spdlog::info("Axis test5: Bad uniform_bins descriptor --- Test");

    YAML::Node node = YAML::Load(R"(
    uniform_bins: [ 100, -1, 1, 1 ]
    )");

    auto ax = nuis::hist::make_axis(node);
    return 2;
  } catch (...) {
    spdlog::info("Axis test5: Bad uniform_bins descriptor --- Pass");
  }

  {
    spdlog::info("Axis test6: uniform_bins axis bins --- Test");

    YAML::Node node = YAML::Load(R"(
    uniform_bins: [ 100, 0, 1 ]
    name: "mytitle"
    )");

    auto ax = nuis::hist::make_axis(node);

    bool pass = true;
#define axTST(x, y)                                                            \
  {                                                                            \
    bool tst_pass = (ax.index(x) == y);                                        \
    if (tst_pass) {                                                            \
      spdlog::info("\tax({}) = {}, expected {}", x, ax.index(x), y);           \
    } else {                                                                   \
      spdlog::warn("\tax({}) = {}, expected {}", x, ax.index(x), y);           \
    }                                                                          \
    pass = pass && tst_pass;                                                   \
  }

    axTST(0, 0);
    axTST(0.1, 10);
    axTST(0.11, 11);
    axTST(0.12, 12);
    axTST(0.2, 20);
    axTST(0.5, 50);
    axTST(-1.1, -1);
    axTST(1, ax.size());

#undef axTST

    assert_result(pass);

    spdlog::info("Axis test6: uniform_bins axis bins --- Pass");
  }

  {
    spdlog::info("Axis test7: Variable axis bins --- Test");

    YAML::Node node = YAML::Load(R"(
    bin_edges: [ -0.5, 1.5, 6, 10 ]
    name: "myothertitle"
    )");

    auto ax = nuis::hist::make_axis(node);

    bool pass = true;
#define axTST(x, y)                                                            \
  {                                                                            \
    bool tst_pass = (ax.index(x) == y);                                        \
    if (tst_pass) {                                                            \
      spdlog::info("\tax({}) = {}, expected {}", x, ax.index(x), y);           \
    } else {                                                                   \
      spdlog::warn("\tax({}) = {}, expected {}", x, ax.index(x), y);           \
    }                                                                          \
    pass = pass && tst_pass;                                                   \
  }

    axTST(-1, -1);
    axTST(-0.5, 0);
    axTST(1, 0);
    axTST(1.5, 1);
    axTST(6, 2);
    axTST(6.5, 2);
    axTST(10, ax.size());

#undef axTST

    assert_result(pass);
    spdlog::info("Axis test7: Variable axis bins --- Pass");
  }

  {
    spdlog::info("Axis test8: Step axis bins --- Test");

    YAML::Node node = YAML::Load(R"(
    bin_steps: { low_edge: 0.5, steps: [ [10, 0.1], [10, 0.5] ] }
    name: "myothertitle"
    )");

    auto ax = nuis::hist::make_axis(node);

    bool pass = true;
#define axTST(x, y)                                                            \
  {                                                                            \
    bool tst_pass = (ax.index(x) == y);                                        \
    if (tst_pass) {                                                            \
      spdlog::info("\tax({}) = {}, expected {}", x, ax.index(x), y);           \
    } else {                                                                   \
      spdlog::warn("\tax({}) = {}, expected {}", x, ax.index(x), y);           \
    }                                                                          \
    pass = pass && tst_pass;                                                   \
  }

    assert_result(bool(ax.size() == 20));

    axTST(-1, -1);
    axTST(0.5, 0);
    axTST(0.6, 1);
    axTST(0.725, 2);
    axTST(1.5, 9);
    axTST(1.6, 10);
    axTST(2, 11);
    axTST(6.4, 19);
    axTST(6.5, 20);

#undef axTST
    assert_result(pass);
    spdlog::info("Axis test8: Step axis bins --- Pass");
  }
}