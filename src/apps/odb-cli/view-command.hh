//===-- apps/odb-cli/view-command.hh - ViewCommand class --------*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// View panel displaying command input / output
///
//===----------------------------------------------------------------------===//

#pragma once

#include <string>
#include <vector>

/// Class to render a sequence of lines in the CLI.
/// Display lines of text like a terminal, but in a defined region of the whole
/// CLI
class ViewCommand {

public:
  /// (`row0`, `col0`) represent where in the console the text must be displayed
  /// `height` is the maximum number of lines that can be displayed
  /// `width` is the maxinum number of characters by line, if more, text goes to
  /// the next line
  /// If `shift_lines` is false, stop writing when limit is reached
  ///  Otherwhise remove first line
  ViewCommand(std::size_t row0, std::size_t col0, std::size_t height,
              std::size_t width, bool shift_lines);

  std::size_t row0() const { return _row0; }
  std::size_t col0() const { return _col0; }
  std::size_t height() const { return _nrows; }
  std::size_t width() const { return _ncols; }

  // Clear all terminal output
  void clear();

  /// Add text to the output. It handles all '\n' in str, and if
  /// line longer than 'width'
  /// Doesn't print anything until render is called
  void write(const std::string &str);

  /// Put CLI cursor at (row0, col0) and render at most `height` lines.
  void render();

private:
  std::vector<std::string> _lines;
  std::size_t _row0;
  std::size_t _col0;
  std::size_t _nrows;
  std::size_t _ncols;
  bool _shift_lines;
};
