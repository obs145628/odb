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
  /// Set the Range in interval [min_key...max_key], filled with `val`
  /// min_key and max_key cannot be changed afterwards
  RangeMap(std::size_t min_key, std::size_t max_key, const T &val);

  std::size_t min_key() const { return _min_key; }
  std::size_t max_key() const { return _max_key; }

  /// Returns the value associated with `key`
  const T &get(std::size_t key) const;

  /// Map[low...high] = val
  void set(std::size_t low, std::size_t high, const T &val);

  /// @returns the range of keys `key` belong to
  std::pair<std::size_t, std::size_t> range(std::size_t key) const;

private:
  struct Node {
    std::size_t beg_key;
    T val;
    Node(std::size_t beg_key, const T &val) : beg_key(beg_key), val(val) {}
  };

  const std::size_t _min_key;
  const std::size_t _max_key;
  std::vector<Node> _ranges;

  using _range_iter_t = typename std::vector<Node>::const_iterator;

  _range_iter_t _find_node(std::size_t key) const;
};

template <class T>
RangeMap<T>::RangeMap(std::size_t min_key, std::size_t max_key, const T &val)
    : _min_key(min_key), _max_key(max_key) {
  _ranges.emplace_back(min_key, val);
}

template <class T> const T &RangeMap<T>::get(std::size_t key) const {
  return _find_node(key)->val;
}

template <class T>
typename RangeMap<T>::_range_iter_t
RangeMap<T>::_find_node(std::size_t key) const {
  auto it = _ranges.cbegin();
  auto next = it + 1;
  auto end = _ranges.cend();
  while (next != end && key < next->beg_key) {
    it = next;
    ++next;
  }

  return it;
}

template <class T>
std::pair<std::size_t, std::size_t> RangeMap<T>::range(std::size_t key) const {
  auto it = _find_node(key);
  auto next = it + 1;
  auto end_key = next == _ranges.end() ? _max_key : next->_min_key - 1;
  return std::make_pair(it->beg_key, end_key);
}

} // namespace odb
