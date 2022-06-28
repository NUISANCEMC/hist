#include "nuisance/hist/make_hist.hxx"

#include "spdlog/spdlog.h"

#define assert_result(x)                                                       \
  if (!x) {                                                                    \
    spdlog::critical("Test Fail");                                             \
    return 1;                                                                  \
  };

int main() {

  spdlog::set_pattern("[%^%l%$]: %v");

  {
    spdlog::info("Hist test1: 1D axis make_hist --- Test");

    YAML::Node node = YAML::Load(R"(
    axes:
    -
      uniform_bins: [ 100, -1, 1 ]
      name: "mytitle"
    )");

    auto hist = nuis::hist::make_hist(node);

    assert_result(bool(hist.rank() == 1));
    assert_result(bool(hist.size() == 102));

    spdlog::info("Hist test1: 1D axis make_hist --- Pass");
  }

  {
    spdlog::info("Hist test2: 2D axis make_hist --- Test");

    YAML::Node node = YAML::Load(R"(
    axes:
    -
      uniform_bins: [ 100, -1, 1 ]
      name: "mytitle"

    -
      uniform_bins: [ 200, -1, 1 ]
      name: "myothertitle"
    )");

    auto hist = nuis::hist::make_hist(node);

    assert_result(bool(hist.rank() == 2));
    assert_result(bool(hist.size() == (102 * 202)));

    spdlog::info("Hist test2: 2D axis make_hist --- Pass");
  }

  {
    spdlog::info("Hist test3: 1D hist, test fill scalar --- Test");

    YAML::Node node = YAML::Load(R"(
    axes:
    -
      uniform_bins: [ 100, -1, 1 ]
      name: "mytitle"
    )");

    auto hist = nuis::hist::make_hist(node);
    hist(-2);
    hist(0);
    hist(0);
    hist(0.5);
    hist(1);
    hist(1);

    bool pass = true;

#define fillTST(x, y)                                                          \
  {                                                                            \
    bool tst_pass = (hist.at(x).value() == y);                                         \
    if (tst_pass) {                                                            \
      spdlog::info("\tax({}) = {}, expected {}", #x, hist.at(x).value(), y);   \
    } else {                                                                   \
      spdlog::warn("\tax({}) = {}, expected {}", #x, hist.at(x).value(), y);   \
    }                                                                          \
    pass = pass && tst_pass;                                                   \
  }
    fillTST(-1, 1);
    fillTST(0, 0);
    fillTST(49, 0);
    fillTST(50, 2);
    fillTST(75, 1);
    fillTST(100, 2);

#undef fillTST

    assert_result(pass);

    spdlog::info("Hist test3: 1D hist, test fill scalar --- Pass");
  }

  {
    spdlog::info("Hist test4: 1D hist, test fill iterable --- Test");

    YAML::Node node = YAML::Load(R"(
    axes:
    -
      uniform_bins: [ 100, -1, 1 ]
      name: "mytitle"
    )");

    auto hist = nuis::hist::make_hist(node);
    hist.fill(std::vector<double>{-2, 0, 0, 0.5, 1, 1});

    bool pass = true;

#define fillTST(x, y)                                                          \
  {                                                                            \
    bool tst_pass = (hist.at(x).value() == y);                                         \
    if (tst_pass) {                                                            \
      spdlog::info("\tax({}) = {}, expected {}", #x, hist.at(x).value(), y);   \
    } else {                                                                   \
      spdlog::warn("\tax({}) = {}, expected {}", #x, hist.at(x).value(), y);   \
    }                                                                          \
    pass = pass && tst_pass;                                                   \
  }
    fillTST(-1, 1);
    fillTST(0, 0);
    fillTST(49, 0);
    fillTST(50, 2);
    fillTST(75, 1);
    fillTST(100, 2);

#undef fillTST

    assert_result(pass);

    spdlog::info("Hist test4: 1D hist, test fill iterable --- Pass");
  }

  {
    spdlog::info("Hist test5: 2D hist, test fill scalars --- Test");

    YAML::Node node = YAML::Load(R"(
    axes:
    -
      uniform_bins: [ 100, -1, 1 ]
      name: "mytitle"
    -
      uniform_bins: [ 100, -1, 1 ]
      name: "mytitle"
    )");

    auto hist = nuis::hist::make_hist(node);
    hist(0, 0);

    bool pass = true;

#define fillTST(x, y)                                                          \
  {                                                                            \
    bool tst_pass = (hist.at(x).value() == y);                                         \
    if (tst_pass) {                                                            \
      spdlog::info("\tax({}) = {}, expected {}", #x, hist.at(x).value(), y);   \
    } else {                                                                   \
      spdlog::warn("\tax({}) = {}, expected {}", #x, hist.at(x).value(), y);   \
    }                                                                          \
    pass = pass && tst_pass;                                                   \
  }

    std::tuple<int, int> indx{50, 50};
    fillTST(indx, 1);

#undef fillTST

    assert_result(pass);

    spdlog::info("Hist test5: 2D hist, test fill scalars --- Pass");
  }

  {
    spdlog::info(
        "Hist test6: 2D hist, test fill iterable of iterables --- Test");

    YAML::Node node = YAML::Load(R"(
    axes:
    -
      uniform_bins: [ 100, -1, 1 ]
      name: "mytitle"
    -
      uniform_bins: [ 100, -1, 1 ]
      name: "mytitle"
    )");

    auto hist = nuis::hist::make_hist(node);
    hist.fill(std::vector<std::vector<double>>{std::vector<double>{0, 0},
                                               std::vector<double>{0, 0}});

    bool pass = true;

#define fillTST(x, y)                                                          \
  {                                                                            \
    bool tst_pass = (hist.at(x).value() == y);                                         \
    if (tst_pass) {                                                            \
      spdlog::info("\tax({}) = {}, expected {}", #x, hist.at(x).value(), y);   \
    } else {                                                                   \
      spdlog::warn("\tax({}) = {}, expected {}", #x, hist.at(x).value(), y);   \
    }                                                                          \
    pass = pass && tst_pass;                                                   \
  }

    std::tuple<int, int> indx{50, 50};
    fillTST(indx, 2);

#undef fillTST

    assert_result(pass);

    spdlog::info(
        "Hist test6: 2D hist, test fill iterable of iterables --- Pass");
  }

  {
    spdlog::info("Hist test7: 1D hist, test set value --- Test");

    YAML::Node node = YAML::Load(R"(
    axes:
    -
      uniform_bins: [ 100, -1, 1 ]
      name: "mytitle"
    )");

    auto hist = nuis::hist::make_hist(node);
    hist.at(50) = 5;

    bool pass = true;

#define fillTST(x, y)                                                          \
  {                                                                            \
    bool tst_pass = (hist.at(x).value() == y);                                         \
    if (tst_pass) {                                                            \
      spdlog::info("\tax({}) = {}, expected {}", #x, hist.at(x).value(), y);   \
    } else {                                                                   \
      spdlog::warn("\tax({}) = {}, expected {}", #x, hist.at(x).value(), y);   \
    }                                                                          \
    pass = pass && tst_pass;                                                   \
  }

    fillTST(50, 5);

#undef fillTST

    assert_result(pass);

    spdlog::info("Hist test7: 1D hist, test set value --- Pass");
  }

  {
    spdlog::info("Hist test8: 1D hist, test read value --- Test");

    YAML::Node node = YAML::Load(R"(
    axes:
    -
      uniform_bins: [ 10, 0, 1 ]
      name: "mytitle"
    content:
      values: [ 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 ]
    )");

    auto hist = nuis::hist::make_hist(node);

    bool pass = true;

#define fillTST(x, y)                                                          \
  {                                                                            \
    bool tst_pass = (hist.at(x).value() == y);                                         \
    if (tst_pass) {                                                            \
      spdlog::info("\tax({}) = {}, expected {}", #x, hist.at(x).value(), y);   \
    } else {                                                                   \
      spdlog::warn("\tax({}) = {}, expected {}", #x, hist.at(x).value(), y);   \
    }                                                                          \
    pass = pass && tst_pass;                                                   \
  }

    for (int i = 0; i < 10; ++i) {
      fillTST(i, 10 - i);
    }

#undef fillTST

    assert_result(pass);

    spdlog::info("Hist test8: 1D hist, test read value --- Pass");
  }

    {
    spdlog::info("Hist test9: 1D hist, test read value/flow --- Test");

    YAML::Node node = YAML::Load(R"(
    axes:
    -
      uniform_bins: [ 10, 0, 1 ]
      name: "mytitle"
    content:
      has_flow_bins: true,
      values: [ -1, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, -1 ] 
    
    )");

    auto hist = nuis::hist::make_hist(node);

    bool pass = true;

#define fillTST(x, y)                                                          \
  {                                                                            \
    bool tst_pass = (hist.at(x).value() == y);                                         \
    if (tst_pass) {                                                            \
      spdlog::info("\tax({}) = {}, expected {}", #x, hist.at(x).value(), y);   \
    } else {                                                                   \
      spdlog::warn("\tax({}) = {}, expected {}", #x, hist.at(x).value(), y);   \
    }                                                                          \
    pass = pass && tst_pass;                                                   \
  }

    for (int i = 0; i < 10; ++i) {
      fillTST(i, 10 - i);
    }

    fillTST(-1, -1);
    fillTST(10, -1);

#undef fillTST

    assert_result(pass);

    spdlog::info("Hist test9: 1D hist, test read value/flow --- Pass");
  }

  {
    spdlog::info("Hist test10: 2D hist, test read value --- Test");

    YAML::Node node = YAML::Load(R"(
    axes:
    -
      uniform_bins: [ 3, 0, 3 ]
      name: "mytitle"
    -
      uniform_bins: [ 3, 0, 3 ]
      name: "mytitle"
    content:
      values: [ 1 ,2 ,3
                4 ,5 ,6
                7 ,8 ,9 ]
    )");

    auto hist = nuis::hist::make_hist(node);

    bool pass = true;

#define fillTST(x, y)                                                          \
  {                                                                            \
    bool tst_pass = (hist.at(x).value() == y);                                         \
    if (tst_pass) {                                                            \
      spdlog::info("\tax({}) = {}, expected {}", #x, hist.at(x).value(), y);   \
    } else {                                                                   \
      spdlog::warn("\tax({}) = {}, expected {}", #x, hist.at(x).value(), y);   \
    }                                                                          \
    pass = pass && tst_pass;                                                   \
  }

    int ctr = 1;
    for (int j = 0; j < 5; ++j) {
      for (int i = 0; i < 5; ++i) {

        std::tuple<int, int> indx{i - 1, j - 1};
        fillTST(indx, ctr++);
      }
    }

#undef fillTST

    assert_result(pass);

    spdlog::info("Hist test10: 2D hist, test read value --- Pass");
  }

  {
    spdlog::info("Hist test11: 2D hist, test read value/flow --- Test");

    YAML::Node node = YAML::Load(R"(
    axes:
    -
      uniform_bins: [ 3, 0, 3 ]
      name: "mytitle"
    -
      uniform_bins: [ 3, 0, 3 ]
      name: "mytitle"
    content: { 
      has_flow_bins: true,
      values: -1, -1,-1,-1, 99, 
              -1, 1 ,2 ,3 , 99,
              -1, 4 ,5 ,6 , 99,
              -1, 7 ,8 ,9 , 99,
              99, 99,99,99, 99 ]
            }
    )");

    auto hist = nuis::hist::make_hist(node);

    bool pass = true;

#define fillTST(x, y)                                                          \
  {                                                                            \
    bool tst_pass = (hist.at(x).value() == y);                                         \
    if (tst_pass) {                                                            \
      spdlog::info("\tax({}) = {}, expected {}", #x, hist.at(x).value(), y);   \
    } else {                                                                   \
      spdlog::warn("\tax({}) = {}, expected {}", #x, hist.at(x).value(), y);   \
    }                                                                          \
    pass = pass && tst_pass;                                                   \
  }

    int ctr = 1;
    for (int j = 0; j < 5; ++j) {
      for (int i = 0; i < 5; ++i) {

        double expected = 0;
        if ((i == 4) || (j == 4)) {
          expected = 99;
        } else if ((i == 0) || (j == 0)) {
          expected = -1;
        } else {
          expected = ctr++;
        }

        std::tuple<int, int> indx{i - 1, j - 1};
        fillTST(indx, expected);
      }
    }

#undef fillTST

    assert_result(pass);

    spdlog::info("Hist test11: 2D hist, test read value/flow --- Pass");
  }
}