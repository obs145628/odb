#pragma once

#include <iostream>

#include "rom.hh"

namespace mvm0 {

ROM parse_is(std::istream &is);
ROM parse_file(const std::string &path);

} // namespace mvm0
