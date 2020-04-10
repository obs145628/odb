//===-- utils/range-map.hh - RangeMap class definition ---------*- C++//-*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Map with whole range of keys that have the same associated value
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cassert>
#include <utility>
#include <vector>

namespace odb {

/// Map (association key => value)
/// There is no insertion / deletion, initially alls keys have the same val over
/// the full range of keys.
/// It's then possible to change the value of any range of keys
/// Impl only store ranges, and not the vals for every element
template <class T> class RangeMap {

public:
  class Range {
  public:
    Range(std::size_t low, std::size_t high, const T &val)
        : low(low), high(high), val(val) {}

    const std::size_t low;
    const std::size_t high;
    const T &val;
  };

  /// Set the Range in interval [min_key...max_key], filled with `val`
  /// min_key and max_key cannot be changed afterwards
  RangeMap(std::size_t min_key, std::size_t max_key, const T &val);

  std::size_t min_key() const { return _min_key; }
  std::size_t max_key() const { return _max_key; }

  // @returns the number of ranges
  std::size_t size() const { return _ranges.size(); }

  /// Returns the value associated with `key`
  const T &get(std::size_t key) const;

  /// Map[low...high] = val
  void set(std::size_t low, std::size_t high, const T &val);

  /// @returns the range of keys `key` belong to
  /// May be invalidated when `set` method is called
  Range range_of(std::size_t key) const;

  /// @returns the `i`-th range in key order
  Range operator[](std::size_t i) const;

private:
  struct Node {
    std::size_t beg_key;
    T val;
    Node(std::size_t beg_key, const T &val) : beg_key(beg_key), val(val) {}
  };

  const std::size_t _min_key;
  const std::size_t _max_key;
  std::vector<Node> _ranges;

  using _range_iter_t = typename std::vector<Node>::iterator;
  using _const_range_iter_t = typename std::vector<Node>::const_iterator;

  _range_iter_t _find_node(std::size_t key);
  _const_range_iter_t _find_node(std::size_t key) const;
  void _fix();
};

template <class T>
RangeMap<T>::RangeMap(std::size_t min_key, std::size_t max_key, const T &val)
    : _min_key(min_key), _max_key(max_key) {
  _ranges.emplace_back(min_key, val);
}

template <class T> const T &RangeMap<T>::get(std::size_t key) const {
  assert(key >= _min_key);
  assert(key <= _max_key);
  return _find_node(key)->val;
}

template <class T>
void RangeMap<T>::set(std::size_t low, std::size_t high, const T &val) {
  assert(low >= _min_key);
  assert(high <= _max_key);
  assert(low <= high);

  auto low_it = _find_node(low);
  if (low_it->beg_key != low) {
    // If low in the middle of a node, we cut in in half
    low_it = _ranges.emplace(low_it + 1, low, low_it->val);
  }

  auto high_it = _ranges.end();
  if (high != _max_key) { // special case for max value
    high_it = _find_node(high + 1);
    if (high_it->beg_key != high + 1) {
      // If high in the middle of a node, we cut in in half
      high_it = _ranges.emplace(high_it + 1, high + 1, high_it->val);
    }
  }

  // Need to find it again if array was reallocated
  low_it = _find_node(low);
  for (auto it = low_it; it != high_it; ++it)
    it->val = val;

  _fix();
}

template <class T>
typename RangeMap<T>::Range RangeMap<T>::range_of(std::size_t key) const {
  assert(key >= _min_key);
  assert(key <= _max_key);
  auto it = _find_node(key);
  auto next = it + 1;
  auto end_key = next == _ranges.cend() ? _max_key : next->beg_key - 1;
  return Range(it->beg_key, end_key, it->val);
}

template <class T>
typename RangeMap<T>::Range RangeMap<T>::operator[](std::size_t i) const {
  assert(i < _ranges.size());
  auto it = _ranges.cbegin() + i;
  auto next = it + 1;
  auto end_key = next == _ranges.cend() ? _max_key : next->beg_key - 1;
  return Range(it->beg_key, end_key, it->val);
}

template <class T>
typename RangeMap<T>::_const_range_iter_t
RangeMap<T>::_find_node(std::size_t key) const {
  auto it = _ranges.cbegin();
  auto next = it + 1;
  auto end = _ranges.cend();
  while (next != end && key >= next->beg_key) {
    it = next;
    ++next;
  }

  return it;
}

template <class T>
typename RangeMap<T>::_range_iter_t RangeMap<T>::_find_node(std::size_t key) {
  auto it = _ranges.begin();
  auto next = it + 1;
  auto end = _ranges.end();
  while (next != end && key >= next->beg_key) {
    it = next;
    ++next;
  }

  return it;
}

template <class T> void RangeMap<T>::_fix() {
  std::size_t idx = 0;
  while (idx + 1 < _ranges.size()) {
    if (_ranges[idx].val == _ranges[idx + 1].val)
      _ranges.erase(_ranges.begin() + idx + 1);
    else
      ++idx;
  }
}

} // namespace odb
