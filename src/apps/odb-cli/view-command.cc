#include "view-command.hh"

#include <cctype>

#include "clicodes.hh"

ViewCommand::ViewCommand(std::size_t row0, std::size_t col0, std::size_t height,
                         std::size_t width, bool shift_lines)
    : _row0(row0), _col0(col0), _nrows(height), _ncols(width),
      _shift_lines(shift_lines) {
  clear();
}

void ViewCommand::clear() {
  _lines.clear();
  _lines.push_back({});
}

void ViewCommand::write(const std::string &str) {
  // First handle all special chars (keep only visible ones)
  std::string rstr;
  for (std::size_t i = 0; i < str.size(); ++i) {
    char c = str[i];
    if (isprint(c) || c == '\n' || c == ' ')
      rstr.push_back(c);
    else if (c == '\t')
      rstr += "    ";
  }

  std::string *last = &_lines.back();
  for (std::size_t i = 0; i < rstr.size(); ++i) {
    char c = rstr[i];
    if (last->size() == _ncols || c == '\n') {
      if (_lines.size() == _nrows && !_shift_lines)
        return;
      _lines.push_back({});
      if (_lines.size() > _nrows)
        _lines.erase(_lines.begin());
      last = &_lines.back();
    }

    if (c != '\n')
      last->push_back(c);
  }
}

void ViewCommand::render() {
  for (std::size_t i = 0; i < _lines.size(); ++i) {
    clicodes::move_cur(_row0 + i, _col0);
    std::cout << _lines[i];
  }
  std::cout.flush();
}
