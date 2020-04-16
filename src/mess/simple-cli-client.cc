#include "odb/mess/simple-cli-client.hh"

#include <cstddef>
#include <cstdint>
#include <sstream>

#include "odb/mess/db-client.hh"
#include "odb/server/vm-api.hh"

namespace odb {

namespace {

enum class TypeDesc { u8, u16, u32, u64, i8, i16, i32, i64, f32, f64 };

const char *type_strs[] = {"u8",  "u16", "u32", "u64", "i8",
                           "i16", "i32", "i64", "f32", "f64"};
const TypeDesc type_vals[] = {
    TypeDesc::u8,  TypeDesc::u16, TypeDesc::u32, TypeDesc::u64, TypeDesc::i8,
    TypeDesc::i16, TypeDesc::i32, TypeDesc::i64, TypeDesc::f32, TypeDesc::f64,
};

struct RegVariant {
  bool is_resolved;
  union {
    vm_reg_t reg_id;
    const char *reg_name;
  };
};

constexpr int VALUE_IVAL = 1;
constexpr int VALUE_DVAL = 2;
constexpr int VALUE_SYM_NAME = 3;
constexpr int VALUE_SYM_ID = 4;

struct ValueVariant {

  int type;

  union {
    long long ival;
    double dval;
    const char *sym_name;
    vm_sym_t sym_id;
  };
};

size_t type_desc_size(TypeDesc t) {
  switch (t) {
  case TypeDesc::u8:
  case TypeDesc::i8:
    return 1;
  case TypeDesc::u16:
  case TypeDesc::i16:
    return 2;
  case TypeDesc::i32:
  case TypeDesc::u32:
  case TypeDesc::f32:
    return 4;
  case TypeDesc::i64:
  case TypeDesc::u64:
  case TypeDesc::f64:
    return 8;
  }

  return 0;
}

long long parse_int(const std::string &str, bool is_signed) {
  std::size_t len;
  long long res;
  bool valid = true;
  try {
    res = std::stoll(str, &len);
  } catch (std::exception &) {
    valid = false;
  }

  if (len != str.size())
    valid = false;
  else if (res < 0 && !is_signed)
    valid = false;

  if (!valid)
    throw VMApi::Error("Cannot parse `" + str + "' as an integer");

  return res;
}

double parse_double(const std::string &str) {
  std::size_t len;
  double res;
  bool valid = true;
  try {
    res = std::stod(str, &len);
  } catch (std::exception &) {
    valid = false;
  }

  if (len != str.size())
    valid = false;

  if (!valid)
    throw VMApi::Error("Cannot parse `" + str + "' as a floating number");

  return res;
}

void print_val(std::ostream &os, const char *buff, TypeDesc ty) {
  switch (ty) {
  case TypeDesc::u8:
    os << *reinterpret_cast<const std::uint8_t *>(buff);
    break;
  case TypeDesc::u16:
    os << *reinterpret_cast<const std::uint16_t *>(buff);
    break;
  case TypeDesc::u32:
    os << *reinterpret_cast<const std::uint32_t *>(buff);
    break;
  case TypeDesc::u64:
    os << *reinterpret_cast<const std::uint64_t *>(buff);
    break;
  case TypeDesc::i8:
    os << *reinterpret_cast<const std::int8_t *>(buff);
    break;
  case TypeDesc::i16:
    os << *reinterpret_cast<const std::int16_t *>(buff);
    break;
  case TypeDesc::i32:
    os << *reinterpret_cast<const std::int32_t *>(buff);
    break;
  case TypeDesc::i64:
    os << *reinterpret_cast<const std::int64_t *>(buff);
    break;
  case TypeDesc::f32:
    os << *reinterpret_cast<const float *>(buff);
    break;
  case TypeDesc::f64:
    os << *reinterpret_cast<const double *>(buff);
    break;
  };
}

void load_val(char *buf, ValueVariant &val, TypeDesc ty) {
  switch (ty) {
  case TypeDesc::u8:
    *reinterpret_cast<std::uint8_t *>(buf) =
        val.type == VALUE_IVAL ? val.ival : val.dval;
    break;
  case TypeDesc::u16:
    *reinterpret_cast<std::uint16_t *>(buf) =
        val.type == VALUE_IVAL ? val.ival : val.dval;
    break;
  case TypeDesc::u32:
    *reinterpret_cast<std::uint32_t *>(buf) =
        val.type == VALUE_IVAL ? val.ival : val.dval;
    break;
  case TypeDesc::u64:
    *reinterpret_cast<std::uint64_t *>(buf) =
        val.type == VALUE_IVAL ? val.ival : val.dval;
    break;
  case TypeDesc::i8:
    *reinterpret_cast<std::int8_t *>(buf) =
        val.type == VALUE_IVAL ? val.ival : val.dval;
    break;
  case TypeDesc::i16:
    *reinterpret_cast<std::int16_t *>(buf) =
        val.type == VALUE_IVAL ? val.ival : val.dval;
    break;
  case TypeDesc::i32:
    *reinterpret_cast<std::int32_t *>(buf) =
        val.type == VALUE_IVAL ? val.ival : val.dval;
    break;
  case TypeDesc::i64:
    *reinterpret_cast<std::int64_t *>(buf) =
        val.type == VALUE_IVAL ? val.ival : val.dval;
    break;
  case TypeDesc::f32:
    *reinterpret_cast<float *>(buf) =
        val.type == VALUE_IVAL ? val.ival : val.dval;
    break;
  case TypeDesc::f64:
    *reinterpret_cast<double *>(buf) =
        val.type == VALUE_IVAL ? val.ival : val.dval;
    break;
  };
}

TypeDesc type_desc_parse(const std::string &s) {
  for (std::size_t i = 0; i < sizeof(type_strs) / sizeof(type_strs[0]); ++i)
    if (s == type_strs[i])
      return type_vals[i];
  throw VMApi::Error("Invalid command syntax: `" + s + "' is not a type");
}

RegVariant r_reg(const std::string &s) {
  RegVariant res;
  if (s.empty() || s.front() != '%') {
    throw VMApi::Error("Syntax error: `" + s + "' is not a register.");
  }

  auto reg = s.substr(1);
  if (reg.size() > 0 && reg[0] >= '0' && reg[0] <= '9') {
    res.reg_id = parse_int(reg, false);
    res.is_resolved = true;
  }

  else {
    res.reg_name = &s[1];
    res.is_resolved = false;
  }

  return res;
}

ValueVariant r_value(const std::string &s) {
  ValueVariant res;

  if (s.size() > 1 && s[0] == '@') {
    if (s[1] >= '0' && s[1] <= '9') {
      res.sym_id = parse_int(s.substr(1), false);
      res.type = VALUE_SYM_ID;
    } else {
      res.sym_name = &s[1];
      res.type = VALUE_SYM_NAME;
    }
    return res;
  }

  try {
    res.dval = parse_double(s);
    res.type = VALUE_DVAL;
    return res;
  } catch (...) {
    res.ival = parse_int(s, true);
    res.type = VALUE_IVAL;
    return res;
  }
}

void resolve_regs(DBClient &db, std::vector<RegVariant> &regs) {
  std::vector<const char *> names;

  for (const auto &r : regs)
    if (!r.is_resolved)
      names.push_back(r.reg_name);

  if (names.empty())
    return;

  std::vector<vm_reg_t> ids(names.size());
  db.find_regs_ids(&names[0], &ids[0], names.size());

  std::size_t i = 0;
  for (auto &r : regs)
    if (!r.is_resolved) {
      r.is_resolved = true;
      r.reg_id = ids[i++];
    }
}

void resolve_vals(DBClient &db, std::vector<ValueVariant> &vv) {

  // Resolve symbol ids
  std::vector<vm_sym_t> ids;
  for (const auto &v : vv)
    if (v.type == VALUE_SYM_ID)
      ids.push_back(v.sym_id);

  if (ids.size() != 0) {
    std::vector<SymbolInfos> syms(ids.size());
    db.get_symbols_by_ids(&ids[0], &syms[0], ids.size());

    std::size_t i = 0;
    for (auto &v : vv)
      if (v.type == VALUE_SYM_ID) {
        v.type = VALUE_IVAL;
        v.ival = syms[i++].addr;
      }
  }

  // Resolve symbol names
  std::vector<const char *> names;
  for (const auto &v : vv)
    if (v.type == VALUE_SYM_NAME)
      names.push_back(v.sym_name);

  if (names.size() != 0) {
    std::vector<SymbolInfos> syms(names.size());
    db.get_symbols_by_names(&names[0], &syms[0], ids.size());

    std::size_t i = 0;
    for (auto &v : vv)
      if (v.type == VALUE_SYM_NAME) {
        v.type = VALUE_IVAL;
        v.ival = syms[i++].addr;
      }
  }
}

} // namespace

SimpleCLIClient::SimpleCLIClient(DBClient &env) : _env(env) {}

std::string SimpleCLIClient::exec(const std::string &cmd) {
  // Split into args
  std::istringstream is(cmd);
  _cmd.clear();
  std::string arg;
  while (is >> arg)
    _cmd.push_back(arg);
  if (_cmd.empty())
    return "";

  auto &name = _cmd.front();

  try {

    if (name == "preg")
      return _cmd_preg();
    else if (name == "sreg")
      return _cmd_sreg();
    else if (name == "pregi")
      return _cmd_pregi();
    else if (name == "pmem")
      return _cmd_pmem();
    else if (name == "smem")
      return _cmd_smem();
    else if (name == "psym")
      return _cmd_psym();
    else if (name == "code")
      return _cmd_code();
    else if (name == "b")
      return _cmd_b();
    else if (name == "delb")
      return _cmd_delb();
    else if (name == "run")
      return _cmd_run();
    else if (name == "step")
      return _cmd_step();
    else if (name == "next")
      return _cmd_next();
    else if (name == "fin")
      return _cmd_fin();
    else if (name == "state")
      return _cmd_state();
    else if (name == "bt")
      return _cmd_bt();
    else if (name == "vm")
      return _cmd_vm();
    else
      throw VMApi::Error("Unknown command `" + name + "'");

  } catch (VMApi::Error &e) {
    return "Error: " + std::string(e.what());
  }
}

std::string SimpleCLIClient::_cmd_preg() {
  if (_cmd.size() < 3)
    throw VMApi::Error("preg: missing arguments");

  auto ty = type_desc_parse(_cmd[1]);
  auto len = type_desc_size(ty);

  std::vector<RegVariant> regs;
  for (std::size_t i = 2; i < _cmd.size(); ++i)
    regs.push_back(r_reg(_cmd[i]));

  resolve_regs(_env, regs);

  std::vector<vm_reg_t> ids;
  for (const auto &r : regs)
    ids.push_back(r.reg_id);

  vm_size_t regs_size[] = {len, 0};
  std::vector<char> buffer(len * ids.size());
  std::vector<char *> buf_ptrs(ids.size());
  for (std::size_t i = 0; i < ids.size(); ++i)
    buf_ptrs[i] = &buffer[len * i];

  _env.get_regs(&ids[0], &buf_ptrs[0], regs_size, ids.size());

  std::ostringstream os;
  for (std::size_t i = 0; i < ids.size(); ++i) {
    os << _cmd[i + 2] << ": ";
    print_val(os, &buffer[i * len], ty);
    os << "\n";
  }

  return os.str();
}

std::string SimpleCLIClient::_cmd_sreg() {
  if (_cmd.size() < 4 || _cmd.size() % 2)
    throw VMApi::Error("sreg: missing arguments");

  auto ty = type_desc_parse(_cmd[1]);
  auto len = type_desc_size(ty);

  std::vector<RegVariant> regs;
  std::vector<ValueVariant> vals;
  for (std::size_t i = 2; i < _cmd.size(); ++i) {
    regs.push_back(r_reg(_cmd[i]));
    vals.push_back(r_value(_cmd[i]));
  }

  resolve_regs(_env, regs);
  resolve_vals(_env, vals);

  std::vector<vm_reg_t> ids;
  for (const auto &r : regs)
    ids.push_back(r.reg_id);

  vm_size_t regs_size[] = {len, 0};
  std::vector<char> buffer(len * ids.size());
  std::vector<char *> buf_ptrs(ids.size());
  for (std::size_t i = 0; i < ids.size(); ++i) {
    buf_ptrs[i] = &buffer[len * i];
    load_val(buf_ptrs[i], vals[i], ty);
  }

  _env.set_regs(&ids[0], (const char **)&buf_ptrs[0], regs_size, ids.size());
  return "";
}

std::string SimpleCLIClient::_cmd_pregi() {
  if (_cmd.size() < 2)
    throw VMApi::Error("pregi: missing arguments");

  std::vector<RegVariant> regs;
  for (std::size_t i = 1; i < _cmd.size(); ++i)
    regs.push_back(r_reg(_cmd[i]));
  resolve_regs(_env, regs);

  std::vector<vm_reg_t> ids;
  for (const auto &r : regs)
    ids.push_back(r.reg_id);
  std::vector<RegInfos> infos(ids.size());
  _env.get_regs_infos(&ids[0], &infos[0], ids.size());

  std::ostringstream os;
  for (std::size_t i = 0; i < ids.size(); ++i) {
    const auto &inf = infos[1];
    os << _cmd[i + 1] << ": ";
    os << "{"
       << "id: " << inf.idx << ", name: " << inf.name << ", size: " << inf.size
       << "}\n";
  }

  return os.str();
}

std::string SimpleCLIClient::_cmd_pmem() {
  if (_cmd.size() != 4)
    throw VMApi::Error("pmem: missing arguments");

  auto ty = type_desc_parse(_cmd[1]);
  auto ty_len = type_desc_size(ty);
  std::size_t nb_items = parse_int(_cmd[3], false);
  vm_size_t buff_size = nb_items * ty_len;
  std::vector<char> buff(buff_size);
  char *buff_ptr = &buff[0];

  // Read address
  std::vector<ValueVariant> vals = {r_value(_cmd[2])};
  resolve_vals(_env, vals);
  if (vals[0].type != VALUE_IVAL || vals[0].ival < 0)
    throw VMApi::Error("pmem: invalid address `" + _cmd[2] + "'");
  vm_ptr_t addr = vals[0].ival;

  _env.read_mem(&addr, &buff_size, &buff_ptr, 1);

  std::ostringstream os;
  for (std::size_t i = 0; i < nb_items; ++i) {
    print_val(os, &buff[i * ty_len], ty);
    os << " ";
  }

  return os.str();
}

std::string SimpleCLIClient::_cmd_smem() {
  if (_cmd.size() < 4)
    throw VMApi::Error("smem: missing arguments");

  auto ty = type_desc_parse(_cmd[1]);
  auto ty_len = type_desc_size(ty);
  std::size_t nb_items = _cmd.size() - 2;
  vm_size_t buff_size = nb_items * ty_len;
  std::vector<char> buff(buff_size);
  const char *buff_ptr = &buff[0];

  // Read address and values
  std::vector<ValueVariant> vals;
  for (std::size_t i = 2; i < _cmd.size(); ++i)
    vals.push_back(r_value(_cmd[i]));
  resolve_vals(_env, vals);

  // Find address
  if (vals[0].type != VALUE_IVAL || vals[0].ival < 0)
    throw VMApi::Error("smem: invalid address `" + _cmd[2] + "'");
  vm_ptr_t addr = vals[0].ival;

  // Load values to buffer
  for (std::size_t i = 0; i < nb_items; ++i)
    load_val(&buff[i * ty_len], vals[i + 1], ty);

  _env.write_mem(&addr, &buff_size, &buff_ptr, 1);
  return "";
}

std::string SimpleCLIClient::_cmd_psym() {
  if (_cmd.size() != 2)
    throw VMApi::Error("psym: missing arguments");

  // Read symbol infos
  auto val = r_value(_cmd[1]);
  SymbolInfos infos;
  if (val.type == VALUE_SYM_ID)
    _env.get_symbols_by_ids(&val.sym_id, &infos, 1);
  else if (val.type == VALUE_SYM_NAME)
    _env.get_symbols_by_names(&val.sym_name, &infos, 1);
  else
    throw VMApi::Error("psym: argument '" + _cmd[1] + "' not a symbol");

  std::ostringstream os;
  os << "{"
     << "id: " << infos.idx << ", name: " << infos.name
     << ", addr: " << infos.addr << "}\n";
  return os.str();
}

std::string SimpleCLIClient::_cmd_code() {
  // Get raw code
  vm_ptr_t addr = _env.get_execution_point();
  std::size_t n = _cmd.size() < 2 ? 3 : parse_int(_cmd[1], false);
  addr = addr < n ? 0 : addr - n;
  std::size_t nins = 2 * n + 1;
  vm_size_t code_size;
  std::vector<std::string> raw_code;
  _env.get_code_text(addr, nins, code_size, raw_code);

  // Find and load references
  std::vector<vm_sym_t> refs;
  std::vector<std::array<std::size_t, 3>> ref_pos;
  for (std::size_t i = 0; i < raw_code.size(); ++i) {
    const auto &l = raw_code[i];
    std::size_t pos = 0;
    while (pos < l.size()) {

      pos = l.find(pos, '{');
      if (pos == std::string::npos)
        break;
      std::size_t beg_pos = pos;
      ++pos;
      if (l[pos] < '0' || l[pos] > '9')
        continue;
      vm_sym_t id = 0;
      while (l[pos] >= '0' && l[pos] <= '9')
        id = 10 * id + (l[pos++] - '0');
      if (l[pos] != '}')
        continue;

      ++pos;
      refs.push_back(id);
      ref_pos.push_back({i, beg_pos, pos});
    }
  }

  std::vector<SymbolInfos> syms_infos(refs.size());
  _env.get_symbols_by_ids(&refs[0], &syms_infos[0], refs.size());

  // Create string code
  std::ostringstream os;
  std::size_t ref_i = 0;
  for (std::size_t i = 0; i < raw_code.size(); ++i) {
    const auto &l = raw_code[i];
    std::size_t off = 0;

    while (ref_i < refs.size() && ref_pos[ref_i][0] == i) {
      os << l.substr(off, ref_pos[ref_i][1] - off);
      os << syms_infos[ref_i].name;
      off = ref_pos[ref_i][2];
      ++ref_i;
    }

    if (off < l.size())
      os << l.substr(off);
    os << "\n";
  }
  return os.str();
}

} // namespace odb