#pragma once

#include <map>
#include <string>
#include <vector>

namespace mvm0 {

constexpr std::size_t SYM_NONE = static_cast<std::size_t>(-1);

struct Ins {
  std::size_t def_sym; // symbol id If ins has a symbol, -1 otherwhise
  std::size_t use_sym; // symbol id if one operand is a symbol, 0 otherwhise
  std::string name;
  std::vector<int> args;
};

struct ROM {
  std::vector<Ins> ins;
  std::vector<std::size_t> sym_defs;       // mapping symbol idx => ins idx
  std::vector<std::string> syms;           // mapping symbol idx => name
  std::map<std::string, std::size_t> smap; // mapping symbol name => idx
};

} // namespace mvm0
