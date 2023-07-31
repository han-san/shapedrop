#include "tests.hpp"

#include "board.hpp"

#include <cassert>

namespace tests {
auto remove_full_rows() -> void {
  /* Block const y {Color::invalid, true}; */
  /* Block const n {Color::invalid, false}; */
  /* Board boardStart; */
  /* boardStart.data = { */
  /*     n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n,
   * n, */
  /*     n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n,
   * n, */
  /*     n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n,
   * n, */
  /*     n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n,
   * n, */
  /*     n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n,
   * n, */
  /*     n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, y, y, y, n, n,
   * n, */
  /*     n, n, n, y, y, n, y, n, n, n, n, y, y, y, y, n, n, n, n, n, n, y, n, n,
   * y, */
  /*     n, n, n, n, n, y, y, y, y, y, y, y, y, y, y, y, n, y, n, y, n, y, n, y,
   * n, */
  /*     n, y, n, y, n, y, n, y, n, y, y, y, y, y, y, y, y, y, y, y, */
  /* }; */

  /* Board boardEnd; */
  /* boardEnd.data = { */
  /*     n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n,
   * n, */
  /*     n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n,
   * n, */
  /*     n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n,
   * n, */
  /*     n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n,
   * n, */
  /*     n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n,
   * n, */
  /*     n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n,
   * n, */
  /*     n, n, n, n, n, n, n, n, n, n, n, n, n, n, y, y, y, n, n, n, n, n, n, y,
   * y, */
  /*     n, y, n, n, n, n, y, y, y, y, n, n, n, n, n, n, y, n, n, y, n, n, n, n,
   * n, */
  /*     y, n, y, n, y, n, y, n, y, n, n, y, n, y, n, y, n, y, n, y, */
  /* }; */

  /* boardStart.remove_full_rows(); */

  /* for (std::size_t i {0}; i < boardStart.data.size(); ++i) { */
  /*   assert(boardStart.data[i].isActive == boardEnd.data[i].isActive); */
  /* } */
}

auto run() -> void { remove_full_rows(); }
} // namespace tests
