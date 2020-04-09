#include <catch2/catch.hpp>

#include "odb/utils/range-map.hh"

#include <algorithm>
#include <cstdint>
#include <vector>

using map_t = odb::RangeMap<int>;

namespace {

void check_map(const map_t &map, const std::vector<std::size_t> &ranges,
               const std::vector<int> &vals) {
  REQUIRE(map.size() == vals.size());
  for (std::size_t i = 0; i < vals.size(); ++i) {
    auto r = map[i];
    REQUIRE(r.low == ranges[i]);
    if (i + 1 == vals.size())
      REQUIRE(r.high == ranges[i + 1]);
    else
      REQUIRE(r.high == ranges[i + 1] - 1);
    REQUIRE(r.val == vals[i]);
  }
}

std::uint32_t xs32_next(std::uint32_t x) {
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return x;
}

} // namespace

TEST_CASE("range_map_empty", "") {
  map_t m(0, 150, 78);
  REQUIRE(m.get(0) == 78);
  REQUIRE(m.get(1) == 78);
  REQUIRE(m.get(56) == 78);
  REQUIRE(m.get(128) == 78);
  REQUIRE(m.get(149) == 78);
  REQUIRE(m.get(150) == 78);

  REQUIRE(m.range_of(0).low == 0);
  REQUIRE(m.range_of(0).high == 150);
  REQUIRE(m.range_of(0).val == 78);
  REQUIRE(m.range_of(1).low == 0);
  REQUIRE(m.range_of(1).high == 150);
  REQUIRE(m.range_of(1).val == 78);
  REQUIRE(m.range_of(56).low == 0);
  REQUIRE(m.range_of(56).high == 150);
  REQUIRE(m.range_of(56).val == 78);
  REQUIRE(m.range_of(128).low == 0);
  REQUIRE(m.range_of(128).high == 150);
  REQUIRE(m.range_of(128).val == 78);
  REQUIRE(m.range_of(149).low == 0);
  REQUIRE(m.range_of(149).high == 150);
  REQUIRE(m.range_of(149).val == 78);
  REQUIRE(m.range_of(150).low == 0);
  REQUIRE(m.range_of(150).high == 150);
  REQUIRE(m.range_of(150).val == 78);

  check_map(m, {0, 150}, {78});
}

TEST_CASE("range_map_change_one", "") {
  map_t m(0, 150, 78);
  m.set(45, 92, 23);
  REQUIRE(m.get(0) == 78);
  REQUIRE(m.get(1) == 78);
  REQUIRE(m.get(44) == 78);
  REQUIRE(m.get(45) == 23);
  REQUIRE(m.get(46) == 23);
  REQUIRE(m.get(91) == 23);
  REQUIRE(m.get(92) == 23);
  REQUIRE(m.get(93) == 78);
  REQUIRE(m.get(149) == 78);
  REQUIRE(m.get(150) == 78);

  check_map(m, {0, 45, 93, 150}, {78, 23, 78});
}

TEST_CASE("range_map_change_one_no_effect", "") {
  map_t m(0, 150, 78);
  m.set(45, 92, 78);
  REQUIRE(m.get(0) == 78);
  REQUIRE(m.get(1) == 78);
  REQUIRE(m.get(44) == 78);
  REQUIRE(m.get(45) == 78);
  REQUIRE(m.get(46) == 78);
  REQUIRE(m.get(91) == 78);
  REQUIRE(m.get(92) == 78);
  REQUIRE(m.get(93) == 78);
  REQUIRE(m.get(149) == 78);
  REQUIRE(m.get(150) == 78);

  check_map(m, {0, 150}, {78});
}

TEST_CASE("range_map_many", "") {
  map_t m(0, 150, 78);
  m.set(3, 8, 1);
  m.set(9, 15, 2);
  m.set(23, 37, 6);
  m.set(100, 150, 5);
  REQUIRE(m.get(0) == 78);
  REQUIRE(m.get(1) == 78);
  REQUIRE(m.get(2) == 78);
  REQUIRE(m.get(3) == 1);
  REQUIRE(m.get(4) == 1);
  REQUIRE(m.get(7) == 1);
  REQUIRE(m.get(8) == 1);
  REQUIRE(m.get(9) == 2);
  REQUIRE(m.get(10) == 2);
  REQUIRE(m.get(14) == 2);
  REQUIRE(m.get(15) == 2);
  REQUIRE(m.get(16) == 78);
  REQUIRE(m.get(22) == 78);
  REQUIRE(m.get(23) == 6);
  REQUIRE(m.get(24) == 6);
  REQUIRE(m.get(36) == 6);
  REQUIRE(m.get(37) == 6);
  REQUIRE(m.get(38) == 78);
  REQUIRE(m.get(99) == 78);
  REQUIRE(m.get(100) == 5);
  REQUIRE(m.get(101) == 5);
  REQUIRE(m.get(149) == 5);
  REQUIRE(m.get(150) == 5);

  check_map(m, {0, 3, 9, 16, 23, 38, 100, 150}, {78, 1, 2, 78, 6, 78, 5});
}

TEST_CASE("range_map_many_fusion_all", "") {
  map_t m(0, 150, 78);
  m.set(3, 8, 1);
  m.set(9, 15, 2);
  m.set(23, 37, 6);
  m.set(100, 137, 5);
  m.set(2, 137, 78);
  check_map(m, {0, 150}, {78});
}

TEST_CASE("range_map_many_fusion2", "") {
  map_t m(0, 150, 78);
  m.set(3, 8, 1);
  m.set(9, 15, 2);
  m.set(23, 37, 6);
  m.set(100, 137, 5);
  check_map(m, {0, 3, 9, 16, 23, 38, 100, 138, 150},
            {78, 1, 2, 78, 6, 78, 5, 78});
}

TEST_CASE("range_map_many_fusion_beg", "") {
  map_t m(0, 150, 78);
  m.set(3, 8, 1);
  m.set(9, 15, 2);
  m.set(23, 37, 6);
  m.set(100, 137, 5);
  m.set(0, 12, 2);
  check_map(m, {0, 16, 23, 38, 100, 138, 150}, {2, 78, 6, 78, 5, 78});
}

TEST_CASE("range_map_many_fusion_end", "") {
  map_t m(0, 150, 78);
  m.set(3, 8, 1);
  m.set(9, 15, 2);
  m.set(23, 37, 6);
  m.set(100, 137, 5);
  m.set(97, 150, 5);
  check_map(m, {0, 3, 9, 16, 23, 38, 97, 150}, {78, 1, 2, 78, 6, 78, 5});
}

TEST_CASE("range_map_many_fusion_one", "") {
  map_t m(0, 150, 78);
  m.set(3, 8, 1);
  m.set(9, 15, 2);
  m.set(23, 37, 6);
  m.set(100, 137, 5);
  m.set(3, 137, 78);
  check_map(m, {0, 150}, {78});
}

TEST_CASE("range_map_many_change_one", "") {
  map_t m(0, 150, 78);
  m.set(3, 8, 1);
  m.set(9, 15, 2);
  m.set(23, 37, 6);
  m.set(100, 137, 5);
  m.set(0, 150, 11);
  check_map(m, {0, 150}, {11});
}

TEST_CASE("range_map_one_left", "") {
  map_t m(0, 150, 78);
  m.set(3, 8, 1);
  m.set(9, 15, 2);
  m.set(23, 37, 6);
  m.set(100, 137, 5);
  m.set(1, 149, 11);
  check_map(m, {0, 1, 150, 150}, {78, 11, 78});
}

TEST_CASE("range_map_one_by_one", "") {
  map_t m(0, 10, 78);
  for (size_t i = 0; i <= 10; ++i)
    m.set(i, i, i * i);
  REQUIRE(m.size() == 11);
  for (std::size_t i = 0; i <= 10; ++i) {
    auto r = m[i];
    REQUIRE(r.low == i);
    REQUIRE(r.high == i);
    REQUIRE(r.val == static_cast<int>(i * i));
  }
}

TEST_CASE("range_map_rand_vect", "") {
  std::uint32_t rng = 678;
  constexpr std::size_t len = 1500;
  std::vector<int> vals(len);
  map_t m(0, len - 1, 4);
  for (std::size_t i = 0; i < len; ++i) {
    rng = xs32_next(rng);
    vals[i] = rng % 4;
  }

  for (std::size_t i = 0; i < len; ++i)
    m.set(i, i, vals[i]);

  for (std::size_t i = 0; i < len; ++i)
    REQUIRE(m.get(i) == vals[i]);

  int start_val = vals[0];
  std::size_t start_idx = 0;
  std::size_t nr = 0;
  for (std::size_t i = 0; i < len; ++i) {
    if (start_val == vals[i])
      continue;

    auto r = m[nr++];
    REQUIRE(r.low == start_idx);
    REQUIRE(r.high == i - 1);
    REQUIRE(r.val == start_val);
    start_val = vals[i];
    start_idx = i;
  }

  auto r = m[nr++];
  REQUIRE(r.low == start_idx);
  REQUIRE(r.high == len - 1);
  REQUIRE(r.val == start_val);
  REQUIRE(nr == m.size());
}

TEST_CASE("range_map_rand_vect_sorted", "") {
  std::uint32_t rng = 1678;
  constexpr std::size_t len = 1500;
  std::vector<int> vals(len);
  map_t m(0, len - 1, 36);
  for (std::size_t i = 0; i < len; ++i) {
    rng = xs32_next(rng);
    vals[i] = rng % 36;
  }
  std::sort(vals.begin(), vals.end());

  for (std::size_t i = 0; i < len; ++i)
    m.set(i, i, vals[i]);

  for (std::size_t i = 0; i < len; ++i)
    REQUIRE(m.get(i) == vals[i]);

  int start_val = vals[0];
  std::size_t start_idx = 0;
  std::size_t nr = 0;
  for (std::size_t i = 0; i < len; ++i) {
    if (start_val == vals[i])
      continue;

    auto r = m[nr++];
    REQUIRE(r.low == start_idx);
    REQUIRE(r.high == i - 1);
    REQUIRE(r.val == start_val);
    start_val = vals[i];
    start_idx = i;
  }

  auto r = m[nr++];
  REQUIRE(r.low == start_idx);
  REQUIRE(r.high == len - 1);
  REQUIRE(r.val == start_val);
  REQUIRE(nr == m.size());
}
