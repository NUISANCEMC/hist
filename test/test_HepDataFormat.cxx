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
    spdlog::info("HepDatFormat test1: 1D axis make_hist --- Test");

    YAML::Node node = YAML::Load(R"(
      independent_variables:
      - header: {name: Leading dilepton PT, units: GEV}
        values:
        - {low: 0, high: 60}
        - {low: 60, high: 100}
        - {low: 100, high: 200}
        - {low: 200, high: 600}
      dependent_variables:
      - header: {name: 10**6 * 1/SIG(fiducial) * D(SIG(fiducial))/DPT, 
                 units: GEV**-1
                }
        qualifiers:
        - {name: RE, 
           value: "P P --> Z0 < LEPTON+ LEPTON- > Z0 < LEPTON+ LEPTON-> X"}
        - {name: SQRT(S), units: GEV, value: 7000}
        values:
        - value: 7000
          errors:
          - {symerror: 1100, label: stat}
          - {symerror: 79, label: 'sys,detector'}
          - {symerror: 15, label: 'sys,background'}
        - value: 9800
          errors:
          - {symerror: 1600, label: stat}
          - {symerror: 75, label: 'sys,detector'}
          - {symerror: 15, label: 'sys,background'}
        - value: 1600
          errors:
          - {symerror: 490, label: stat}
          - {symerror: 41, label: 'sys,detector'}
          - {symerror: 2, label: 'sys,background'}
        - value: 80
          errors:
          - {symerror: 60, label: stat}
          - {symerror: 2, label: 'sys,detector'}
          - {symerror: 0, label: 'sys,background'}
    )");

    auto hist = nuis::hist::make_hist(node);
    bool pass = true;

#define fillTST(x, y)                                                          \
  {                                                                            \
    bool tst_pass = (hist.at(x).value() == y);                                 \
    if (tst_pass) {                                                            \
      spdlog::info("\tax({}) = {}, expected {}", x, hist.at(x).value(), y);    \
    } else {                                                                   \
      spdlog::warn("\tax({}) = {}, expected {}", x, hist.at(x).value(), y);    \
    }                                                                          \
    pass = pass && tst_pass;                                                   \
  }

    fillTST(0, 7000);
    fillTST(1, 9800);
    fillTST(2, 1600);
    fillTST(3, 80);

#undef fillTST

    assert_result(pass);

    spdlog::info("HepDatFormat test1: 1D axis make_hist --- Pass");
  }

  {
    spdlog::info("HepDatFormat test2: 2D axis make_hist --- Test");

    YAML::Node node = YAML::Load(R"(
      independent_variables:
      - header: {name: ind_var_1}
        values:
        - {low: 0, high: 60}
        - {low: 60, high: 100}
        - {low: 0, high: 60}
        - {low: 60, high: 100}
      - header: {name: ind_var_2}
        values:
        - {low: 100, high: 200}
        - {low: 100, high: 200}
        - {low: 200, high: 600}
        - {low: 200, high: 600}
      dependent_variables:
      - header: {name: dep_var}
        values:
        - value: 1 
          errors:
          - {symerror: 4, label: stat}
        - value: 2
          errors:
          - {symerror: 3, label: stat}
        - value: 3 
          errors:
          - {symerror: 2, label: stat}
        - value: 4 
          errors:
          - {symerror: 1, label: stat}
    )");

    auto hist = nuis::hist::make_hist(node);
    bool pass = true;

#define fillTST(x1, x2, y)                                                     \
  {                                                                            \
    bool tst_pass = (hist.at(std::vector<int>{x1, x2}).value() == y);          \
    if (tst_pass) {                                                            \
      spdlog::info("\tax({},{}) = {}, expected {}", x1, x2,                    \
                   hist.at(std::vector<int>{x1, x2}).value(), y);              \
    } else {                                                                   \
      spdlog::warn("\tax({},{}) = {}, expected {}", x1, x2,                    \
                   hist.at(std::vector<int>{x1, x2}).value(), y);              \
    }                                                                          \
    pass = pass && tst_pass;                                                   \
  }

#define fillTSTerr(x1, x2, y)                                                     \
  {                                                                            \
    bool tst_pass = (hist.at(std::vector<int>{x1, x2}).variance() == y*y);          \
    if (tst_pass) {                                                            \
      spdlog::info("\tax({},{}) = {}, expected var {}", x1, x2,                    \
                   hist.at(std::vector<int>{x1, x2}).variance(), y*y);              \
    } else {                                                                   \
      spdlog::warn("\tax({},{}) = {}, expected var {}", x1, x2,                    \
                   hist.at(std::vector<int>{x1, x2}).variance(), y*y);              \
    }                                                                          \
    pass = pass && tst_pass;                                                   \
  }

    fillTST(0, 0, 1);
    fillTST(1, 0, 2);
    fillTST(0, 1, 3);
    fillTST(1, 1, 4);

    fillTSTerr(0, 0, 4);
    fillTSTerr(1, 0, 3);
    fillTSTerr(0, 1, 2);
    fillTSTerr(1, 1, 1);

#undef fillTST
#undef fillTSTerr

    assert_result(pass);

    spdlog::info("HepDatFormat test2: 2D axis make_hist --- Pass");
  }
}