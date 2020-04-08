#include "../include/mvm0/parser.hh"
#include "../include/mvm0/defs.hh"

#include <cassert>
#include <fstream>
#include <sstream>

namespace mvm0 {

namespace {

bool is_wspace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

std::string str_trim(const std::string &str) {

  std::size_t bpos = 0;
  while (bpos < str.length() && is_wspace(str[bpos]))
    ++bpos;
  if (bpos == str.length())
    return "";

  std::size_t epos = str.length() - 1;
  while (is_wspace(str[epos]))
    --epos;

  return std::string(str.begin() + bpos, str.begin() + epos + 1);
}

std::vector<std::string> str_split(const std::string &str, char sep) {
  std::vector<std::string> res;
  std::istringstream is(str);
  std::string val;
  while (std::getline(is, val, sep))
    res.push_back(val);
  return res;
}

int parse_arg(const std::string &val, const ROM &code, Ins &ins) {
  assert(!val.empty());
  if (val[0] >= '0' && val[0] <= '9')
    return std::atoi(val.c_str());
  if (val[0] == 'r')
    return std::atoi(val.c_str() + 1);

  assert(val[0] == '@');
  auto it = code.smap.find(val.substr(1));
  assert(it != code.smap.end());
  std::size_t sym_idx = it->second;
  ins.use_sym = sym_idx;
  return static_cast<int>(MEM_CODE_START + code.sym_defs[sym_idx]);
}

} // namespace

ROM parse_is(std::istream &is) {
  ROM res;

  std::size_t ins_idx = 0;
  std::size_t sym_idx = 0;
  std::string line;
  while (std::getline(is, line, '\n')) {
    std::string sym;
    line = str_trim(line);
    if (line.empty())
      continue;

    if (line[0] == '@') {
      std::string sym = line.substr(1);
      res.syms.push_back(sym);
      res.smap.emplace(sym, sym_idx++);
      res.sym_defs.push_back(ins_idx);
    }

    else {
      Ins ins;
      ins.name = line;
      ins.def_sym = sym.empty() ? SYM_NONE : sym_idx;
      ins.use_sym = SYM_NONE;
      ++ins_idx;
    }
  }

  for (auto &ins : res.ins) {
    auto args = str_split(ins.name, ' ');
    ins.name = args[0];
    for (std::size_t i = 1; i < args.size(); ++i)
      ins.args.push_back(parse_arg(args[i], res, ins));
  }

  return res;
}

ROM parse_file(const std::string &path) {
  std::ifstream is(path);
  return parse_is(is);
}

} // namespace mvm0
