//===-- apps/odb-cli/clicodes.hh - Terminatol UI tools ----------*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Misc functions to write control codes to stdout to control terminal
///
//===----------------------------------------------------------------------===//

#pragma once

#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>
#include <utility>

namespace clicodes {

inline void clear() {
  std::cout << "\033[H\033[J";
  std::cout.flush();
}

inline void move_cur(std::size_t row, std::size_t col) {
  std::cout << "\033[" << row << ";" << col << "H";
  std::cout.flush();
}

inline std::pair<std::size_t, std::size_t> term_size() {
  struct winsize size;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
  return std::make_pair(size.ws_row, size.ws_col);
}

} // namespace clicodes
