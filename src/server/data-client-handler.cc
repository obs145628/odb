#include "odb/server/data-client-handler.hh"

#include <atomic>
#include <cassert>
#include <chrono>
#include <thread>

#include "odb/mess/request-handler.hh"
#include "odb/mess/request.hh"
#include "odb/mess/serial.hh"
#include "odb/server/db-client-impl-vmside.hh"
#include "odb/server/server-app.hh"
#include "odb/server/tcp-data-server.hh"

namespace odb {

class DataClientServerRunner {

public:
  enum class State {
    CONNECTING,
    NO_REQ,
    SENDING_RES,
    HAS_REQ,
    ERROR,
  };

  DataClientServerRunner(std::unique_ptr<AbstractDataServer> &&serv)
      : _serv(std::move(serv)), _rh(true) {}

  ~DataClientServerRunner() {
    if (_th.joinable())
      _th.join();
  }

  void start() {
    _stop = false;
    _state = State::CONNECTING;
    _th = std::thread(&DataClientServerRunner::loop, this);
  }

  void stop() {
    _stop = true;
    _state = State::ERROR;
  }

  State state() const { return _state; }

  // Called by main thread to read received request
  SerialInBuff &get_req() {
    assert(_state == State::HAS_REQ);
    return _in;
  }

  // Called by main thread to write response
  SerialOutBuff &get_res() {
    assert(_state == State::HAS_REQ);
    return _out;
  }

  RequestHandler &request_handler() { return _rh; }

  // Called by main thread to signal thread that res is ready to be sent
  void signal_res() {
    assert(_state == State::HAS_REQ);
    _state = State::SENDING_RES;
  }

  void loop() {
    // Connect
    if (!_serv->connect()) {
      stop();
      return;
    }
    _state = State::NO_REQ;

    while (!_stop) {
      // Receiving request
      if (!_serv->recv_data(_in)) {
        stop();
        break;
      }
      _state = State::HAS_REQ;

      // Waiting until response written by main thread
      while (!_stop && _state == State::HAS_REQ) {
        std::this_thread::yield();
      }
      if (_stop)
        break;

      // Sending response
      assert(_state == State::SENDING_RES);
      if (!_serv->send_data(_out)) {
        stop();
        break;
      }
      _state = State::NO_REQ;
    }
  }

private:
  std::unique_ptr<AbstractDataServer> _serv;

  std::thread _th;
  std::atomic<bool> _stop;
  std::atomic<State> _state;
  SerialInBuff _in;
  SerialOutBuff _out;
  RequestHandler _rh;
};

DataClientHandler::DataClientHandler(Debugger &db, const ServerConfig &conf,
                                     Kind kind)
    : ClientHandler(db, conf), _kind(kind), _runner(nullptr) {}

DataClientHandler::~DataClientHandler() {}

void DataClientHandler::setup_connection() {
  if (!_runner.get())
    _init();

  using State = DataClientServerRunner::State;

  if (_runner->state() == State::CONNECTING) {
    return;
  } else if (_runner->state() == State::ERROR) {
    _client_disconnected();
  } else {
    _client_connected();
  }
}

void DataClientHandler::run_command() {
  // Blocking until next command or disconnected
  using State = DataClientServerRunner::State;
  while (_runner->state() != State::HAS_REQ &&
         _runner->state() != State::ERROR) {

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    // std::this_thread::yield();
  }
  if (_runner->state() == State::ERROR) {
    _client_disconnected();
    return;
  }

  DBClientImplVMSide dc(get_debugger());
  // @tip ok to create one at every call, just an interface without state
  auto &rh = _runner->request_handler();
  auto &is = _runner->get_req();
  auto &os = _runner->get_res();
  os.reset();

  ReqType is_ty;
  is >> is_ty;

  try {
    // @tip code duplication, could factorize with macros, but code wouldn't
    // look much like valid C++.
    switch (is_ty) {
    case ReqType::CONNECT: {
      ReqConnect req;
      rh.server_read_request(is, req);
      dc.connect(req.out_infos, req.out_udp);
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    }

    case ReqType::STOP: {
      throw VMApi::Error("Cannot stop already stopped program\n");
      /*
    ReqStop req;
    rh.server_read_request(is, req);
    dc.stop();
    os << is_ty;
    rh.server_write_response(os, req);
      */
      break;
    }

    case ReqType::CHECK_STOPPED: {
      ReqCheckStopped req;
      rh.server_read_request(is, req);
      dc.check_stopped(req.out_udp);
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    };

    case ReqType::GET_REGS: {
      ReqGetRegs req;
      rh.server_read_request(is, req);
      vm_size_t regs_sizes[2] = {req.reg_size, 0};
      dc.get_regs(req.ids, req.out_bufs, regs_sizes, req.nregs);
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    };

    case ReqType::GET_REGS_VAR: {
      ReqGetRegsVar req;
      rh.server_read_request(is, req);
      dc.get_regs(req.in_ids, req.out_bufs, req.in_regs_size, req.nregs);
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    };

    case ReqType::SET_REGS: {
      ReqSetRegs req;
      rh.server_read_request(is, req);
      vm_size_t regs_sizes[2] = {req.reg_size, 0};
      dc.set_regs(req.in_ids, (const char **)req.in_bufs, regs_sizes,
                  req.nregs);
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    };

    case ReqType::SET_REGS_VAR: {
      ReqSetRegsVar req;
      rh.server_read_request(is, req);
      dc.set_regs(req.in_ids, (const char **)req.in_bufs, req.in_regs_size,
                  req.nregs);
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    };

    case ReqType::GET_REGS_INFOS: {
      ReqGetRegsInfos req;
      rh.server_read_request(is, req);
      dc.get_regs_infos(req.ids, req.out_infos, req.nregs);
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    };

    case ReqType::FIND_REGS_IDS: {
      ReqFindRegsIds req;
      rh.server_read_request(is, req);
      dc.find_regs_ids((const char **)req.in_bufs, req.out_ids, req.nregs);
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    };

    case ReqType::READ_MEM: {
      ReqReadMem req;
      rh.server_read_request(is, req);
      vm_size_t bufs_size[2] = {req.buf_size, 0};
      dc.read_mem(req.in_addrs, bufs_size, req.out_bufs, req.nbufs);
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    };

    case ReqType::READ_MEM_VAR: {
      ReqReadMemVar req;
      rh.server_read_request(is, req);
      dc.read_mem(req.in_addrs, req.in_bufs_size, req.out_bufs, req.nbufs);
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    };

    case ReqType::WRITE_MEM: {
      ReqWriteMem req;
      rh.server_read_request(is, req);
      vm_size_t bufs_size[2] = {req.buf_size, 0};
      dc.write_mem(req.in_addrs, bufs_size, (const char **)req.in_bufs,
                   req.nbufs);
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    };

    case ReqType::WRITE_MEM_VAR: {
      ReqWriteMemVar req;
      rh.server_read_request(is, req);
      dc.write_mem(req.in_addrs, req.in_bufs_size, (const char **)req.in_bufs,
                   req.nbufs);
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    };

    case ReqType::GET_SYMS_BY_IDS: {
      ReqGetSymsByIds req;
      rh.server_read_request(is, req);
      dc.get_symbols_by_ids(req.in_ids, req.out_infos, req.nsyms);
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    };

    case ReqType::GET_SYMS_BY_ADDR: {
      ReqGetSymsByAddr req;
      rh.server_read_request(is, req);
      dc.get_symbols_by_addr(req.addr, req.size, req.out_infos);
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    };

    case ReqType::GET_SYMS_BY_NAMES: {
      ReqGetSymsByNames req;
      rh.server_read_request(is, req);
      dc.get_symbols_by_names((const char **)req.in_names, req.out_infos,
                              req.nsyms);
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    };

    case ReqType::GET_CODE_TEXT: {
      ReqGetCodeText req;
      rh.server_read_request(is, req);
      dc.get_code_text(req.addr, req.nins, req.out_text, req.out_sizes);
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    };

    case ReqType::ADD_BKPS: {
      ReqAddBkps req;
      rh.server_read_request(is, req);
      dc.add_breakpoints(req.in_addrs, req.size);
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    };

    case ReqType::DEL_BKPS: {
      ReqDelBkps req;
      rh.server_read_request(is, req);
      dc.del_breakpoints(req.in_addrs, req.size);
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    };

    case ReqType::RESUME: {
      ReqResume req;
      rh.server_read_request(is, req);
      dc.resume(req.type);
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    };

    default:
      throw VMApi::Error("Bad API request");
    }

  } catch (VMApi::Error &e) {
    ReqErr err;
    err.msg = e.what();
    os << ReqType::ERR;
    rh.server_write_response(os, err);
  }

  _runner->signal_res();
}

void DataClientHandler::check_stopped() {

  // Doesn't block, just check if has request or get deconneted
  using State = DataClientServerRunner::State;
  if (_runner->state() != State::HAS_REQ)
    return;
  if (_runner->state() == State::ERROR) {
    _client_disconnected();
    return;
  }

  DBClientImplVMSide dc(get_debugger());
  // @tip ok to create one at every call, just an interface without state
  auto &rh = _runner->request_handler();
  auto &is = _runner->get_req();
  auto &os = _runner->get_res();
  os.reset();

  ReqType is_ty;
  is >> is_ty;

  try {
    // Only stop command is valid
    switch (is_ty) {

    case ReqType::STOP: {
      ReqStop req;
      rh.server_read_request(is, req);
      dc.stop();
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    }

    case ReqType::CHECK_STOPPED: {
      ReqCheckStopped req;
      rh.server_read_request(is, req);
      dc.check_stopped(req.out_udp);
      os << is_ty;
      rh.server_write_response(os, req);
      break;
    };

    default:
      throw VMApi::Error(
          "Only stop and check stopped request can be sent while VM running");
    }

  } catch (VMApi::Error &e) {
    ReqErr err;
    err.msg = e.what();
    os << ReqType::ERR;
    rh.server_write_response(os, err);
  }

  _runner->signal_res();
}

void DataClientHandler::_init() {
  auto &conf = get_conf();
  std::unique_ptr<AbstractDataServer> serv;

  if (_kind == Kind::TCP_SERVER)
    serv = std::make_unique<TCPDataServer>(conf.tcp_port);

  assert(serv);
  _runner = std::make_unique<DataClientServerRunner>(std::move(serv));
  _runner->start();
}

} // namespace odb
