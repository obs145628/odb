#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "cli.hh"
#include "clicodes.hh"
#include "view-command.hh"

namespace {

std::unique_ptr<CLI> cli;
std::unique_ptr<ViewCommand> vcode;
std::unique_ptr<ViewCommand> vcmd;

void draw_hline(std::size_t row0, std::size_t col0, std::size_t ncols) {
  clicodes::move_cur(row0, col0);
  std::cout << '+';
  for (std::size_t i = 2; i < ncols; ++i)
    std::cout << '-';
  std::cout << '+';
}

void draw_vline(std::size_t row0, std::size_t col0, std::size_t nrows) {
  for (std::size_t i = 0; i < nrows; ++i) {
    clicodes::move_cur(row0 + i, col0);
    std::cout << (i > 0 && i + 1 < nrows ? '|' : '+');
  }
}

void draw_box(std::size_t row0, std::size_t col0, std::size_t nrows,
              std::size_t ncols) {
  draw_hline(row0, col0, ncols);
  draw_hline(row0 + nrows - 1, col0, ncols);
  draw_vline(row0, col0, nrows);
  draw_vline(row0, col0 + ncols - 1, nrows);
}

void render_code() {
  // @TODO add safeguard to only call code when in right state
  vcode->clear();
  auto code_nb = vcode->height() + 1;
  vcode->write(cli->exec("code " + std::to_string(code_nb)));

  draw_box(vcode->row0() - 1, vcode->col0() - 2, vcode->height() + 2,
           vcode->width() + 4);
  vcode->render();
}

void render_vcmd() {
  draw_box(vcmd->row0() - 1, vcmd->col0() - 2, vcmd->height() + 2,
           vcmd->width() + 4);
  vcmd->render();
}

void render_all() {
  clicodes::clear();
  render_code();
  render_vcmd();
}

// get next command to run
// returns empty if received EOF
std::string get_input() {
  while (true) {
    vcmd->write("> ");
    render_vcmd();

    // check if stdin EOF
    std::cin.peek();
    if (!std::cin.good())
      return {};

    std::string cmd;
    std::getline(std::cin, cmd);
    vcmd->write(cmd);
    vcmd->write("\n");
    if (!cmd.empty())
      return cmd;
  }
}

} // namespace

int main(int argc, char **argv) {
  if (argc > 3) {
    std::cerr << "Usage: odb-cli [<hostname>] [<port>]\n";
    return 1;
  }

  cli = std::make_unique<CLI>(argc, argv);

  auto term_size = clicodes::term_size();
  auto term_rows = term_size.first;
  auto term_cols = term_size.second;
  vcmd = std::make_unique<ViewCommand>(term_rows - term_rows / 3 - 1, 4,
                                       term_rows / 3, term_cols - 2 * 3,
                                       /*sift_lines=*/true);
  vcode = std::make_unique<ViewCommand>(2, vcmd->col0(), vcmd->row0() - 2 - 2,
                                        vcmd->width(), /*sift_lines=*/false);

  render_all();

  while (1) {
    // Check if still connected
    if (!cli->next())
      break;

    if (cli->state_switched())
      vcmd->write(cli->exec("state"));
    render_all();

    auto cmd = get_input();
    if (cmd.empty())
      break;
    std::string out = cli->exec(cmd);

    vcmd->write(out);
    if (!out.empty() && out.back() != '\n')
      vcmd->write("\n");
  }

  clicodes::clear();
  return 0;
}
