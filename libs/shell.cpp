/*                                                                  -*- c++ -*-
 * Copyright © 2018 Ron R Wills <ron@digitalcombine.ca>
 *
 * This file is part of Cutlet.
 *
 * Cutlet is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cutlet is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cutlet.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cutlet>
#include <vector>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <sys/wait.h>
#include <fcntl.h>
#include <regex>
#include <iostream>

#define DEBUG_SHELL 1

#if DEBUG_SHELL
#pragma message ("Shell library debugging enabled")
#endif

/**
 */
class _io {
public:
  _io() {}
  _io(const _io &other) = delete;

  virtual ~_io() {}

  /** Duplicate io file descriptors for the child process.
   */
  virtual void dup(int redirect) = 0;

  virtual void close(int redirect) = 0;
};

/**
 */
class _pipe : public _io {
public:
  _pipe() {
    if (pipe(_fds) == -1) {
      //throw strerror(errno);
    }
  }
  _pipe(const _pipe &other) = delete;

  virtual ~_pipe() noexcept {
    if (_fds[0] != -1) ::close(_fds[0]);
    if (_fds[1] != -1) ::close(_fds[1]);
  }

  virtual void dup(int redirect) {
    switch (redirect) {
    case STDIN_FILENO: // 0
#if DEBUG_SHELL
      std::clog << "  redirecting pipe " << _fds[0] << " -> " << redirect
                << std::endl;
#endif
      if (dup2(_fds[0], STDIN_FILENO) == -1) { // Reassign stdin.
        throw cutlet::exception(strerror(errno));
      }
      break;

    case -1: // Redirect stdout and stderr
#if DEBUG_SHELL
      std::clog << "  redirecting pipe " << _fds[1] << " -> "
                <<  STDOUT_FILENO << ", " << STDERR_FILENO
                << std::endl;
#endif
      if (dup2(_fds[1], STDOUT_FILENO) == -1) { // Reassign stdin.
        throw cutlet::exception(strerror(errno));
      }
      if (dup2(_fds[1], STDERR_FILENO) == -1) { // Reassign stderr.
        throw cutlet::exception(strerror(errno));
      }
      break;

    default:
#if DEBUG_SHELL
      std::clog << "  redirecting pipe " << _fds[1] << " -> " << redirect
                << std::endl;
#endif
      if (dup2(_fds[1], redirect) == -1) { // Reassign the descriptor.
        throw cutlet::exception(strerror(errno));
      }
      break;
    }

    /* One side of the pipe has been duplicated to stdin or stdout and the
     * other side is the other side so we can close them both.
     */
    ::close(_fds[0]);
    ::close(_fds[1]);
  }

  virtual void close(int redirect) {
    switch (redirect) {
    case STDIN_FILENO: // 0
#if DEBUG_SHELL
      std::clog << "  closing pipe " << _fds[0] << std::endl;
#endif
      if (_fds[0] != -1) {
        ::close(_fds[0]);
        _fds[0] = -1;
      }
      break;

    default:
#if DEBUG_SHELL
      std::clog << "  closing pipe " << _fds[1] << std::endl;
#endif
      if (_fds[1] != -1) {
        ::close(_fds[1]);
        _fds[1] = -1;
      }
      break;
    }
  }

private:
  int _fds[2];
};

/**
 */
class _redirect : public _io {
public:
  _redirect(const std::string &filename, int flags) : _close(true) {
    if ((_fd = open(filename.c_str(), flags,
                    S_IRUSR | S_IWUSR |
                    S_IRGRP | S_IWGRP |
                    S_IROTH | S_IWOTH)) == -1) {
      // throw
    }
  }
  _redirect(int fd) : _fd(fd), _close(false) {}
  _redirect(const _redirect &other) = delete;

  virtual ~_redirect() noexcept {
    if (_fd != -1) ::close(_fd);
  }

  virtual void dup(int redirect) {
    switch (redirect) {
    case STDIN_FILENO: // 0
#if DEBUG_SHELL
      std::clog << "  redirecting in " << _fd << " -> " << redirect
                << std::endl;
#endif
      if (dup2(_fd, STDIN_FILENO) == -1) { // Reassign stdin.
        throw cutlet::exception(strerror(errno));
      }
      break;
    case -1:
      if (dup2(_fd, STDOUT_FILENO) == -1) { // Reassign stdin.
        //throw strerror(errno)
      }
      if (dup2(_fd, STDERR_FILENO) == -1) { // Reassign stderr.
        //throw strerror(errno)
      }
      break;
    default:
#if DEBUG_SHELL
      std::clog << "  redirecting fd " << redirect << " -> " << _fd
                << std::endl;
#endif
      if (dup2(_fd, redirect) == -1) { // Reassign the descriptor.
        throw cutlet::exception(strerror(errno));
      }
      break;
    }

    /* We can close our file descriptor because it has been duplicated.
     */
    if (_close) ::close(_fd);
    _fd = -1;
  }

  virtual void close(int redirect) {
  }

private:
  int _fd;
  bool _close;
};

class _command {
private:
  typedef struct {
    int redirect;
    _io *io;
  } _io_specs_t;

public:
  _command(cutlet::list::const_iterator start,
           cutlet::list::const_iterator end) : _pid(-1), _cmd(start, end) {
#if DEBUG_SHELL
    std::clog << "SHELL: building " << (std::string)_cmd << std::endl;
#endif
  }
  _command(const _command &other) = delete;
  virtual ~_command() noexcept {
    /* We don't worry about waitpid results here. We're just trying to prevent
     * zombie processes.
     */
    if (_pid != -1) waitpid(_pid, nullptr, 0);
  }

  void io(int redirect, _io *io) {
    _io_dirs.push_back({redirect, io});
  }

  /**
   */
  void operator()() {
    _pid = fork();
    if (_pid >= 0) {

      if (_pid == 0) { // Child process
        std::vector<const char *> args;

#if DEBUG_SHELL
        std::clog << "SHELL: " << (std::string)_cmd << std::endl;
#endif

        // Build the command to pass to execvp.
        for (auto &it: _cmd) {
          args.push_back(new char[((std::string)*it).size() + 1]);
          strcpy((char *)args.back(), ((std::string)*it).c_str());
        }
        args.push_back(nullptr);

        // Do any IO redirections required.
        for (auto &io: _io_dirs) {
          io.io->dup(io.redirect);
        }

        // Execute the command.
        if (execvp(args[0], (char * const*)&args[0]) == -1) {
          std::cerr << strerror(errno) << std::endl;
          exit(errno);
        }
      } else { // This is us.
        usleep(150);
#if DEBUG_SHELL
        std::clog << "SHELL: " << (std::string)_cmd
                  << " pid = " << _pid << std::endl;
#endif
        // Do any IO redirections required.
        for (auto &io: _io_dirs) {
          io.io->close(io.redirect);
        }
      }
    } else { // We were unable to fork a new process.
      throw cutlet::exception(strerror(errno));
    }
  }

  int wait() {
    int status;
#if DEBUG_SHELL
    std::clog << "SHELL: Waiting for " << _pid << std::endl;
#endif
    while (waitpid(_pid, &status, 0) != _pid);
    _pid = -1;
    return WEXITSTATUS(status);
  }

private:
  int _pid;
  cutlet::list _cmd;

  std::list<_io_specs_t> _io_dirs;
};

/******************************************************************************
 */

static const std::regex re_pipe("(([0-9]+|&)>)?\\|");
static const std::regex re_redir_out("([0-9]+|&)?>([0-9]+|>)?");
static const std::regex re_redir_in("([0-9]+)?<");

// def exec command
static cutlet::variable::pointer _eval(cutlet::interpreter &interp,
                                       const cutlet::list &parameters) {
  (void)interp;

  std::list<_io *> ios;
  std::list<_command *> commands;
  bool use_last = false;

  auto start = parameters.begin();
  auto end = start;
  end++;

  // Build the commands and the pipes.
  for (;end != parameters.end(); end++) {
    std::smatch iomtch;
    std::string bit((std::string)(**end));

    if (std::regex_match(bit, iomtch, re_pipe)) {
      // Pipe specifier found. Set up the pipe and commands.
      _command *new_cmd = new _command(start, end);

      // Pipe to stdin for new command if needed.
      if (use_last)
        new_cmd->io(STDIN_FILENO, ios.back());

      int redirect = STDOUT_FILENO;

      // Determine the file descriptor for the pipe.
      if (iomtch[2] == "&") {
        redirect = -1;
      } else if (iomtch[2] != "") {
        redirect = atoi(iomtch[2].str().c_str());
      }

      // Fail an attempt to pipe the output of stdin.
      if (redirect == STDIN_FILENO) {
        throw cutlet::exception("Unable to pipe stdin output to pipe");
      }

      // Create our new pipe and redirect the new commands stdout to it.
      _pipe *new_pipe = new _pipe;
      new_cmd->io(redirect, new_pipe);
      ios.push_back(new_pipe);

      commands.push_back(new_cmd);
      start = ++end;
      if (end == parameters.end()) break;

      use_last = true;

    } else if (std::regex_match(bit, iomtch, re_redir_out)) {
      // Redirect output.
      _command *new_cmd = new _command(start, end);

      // Pipe to stdin for new command if needed.
      if (use_last)
        new_cmd->io(STDIN_FILENO, ios.back());

      int redirect = STDOUT_FILENO;
      // Determine the file descriptor for the pipe.
      if (iomtch[1] == "&") {
        redirect = -1;
      } else if (iomtch[1] != "") {
        redirect = atoi(iomtch[1].str().c_str());
      }

      _redirect *new_redir = nullptr;
      if (iomtch[2] == "" or iomtch[2] == ">") {
        int flags = 0;
        if (iomtch[2] == "") flags = O_CREAT|O_WRONLY|O_TRUNC;
        else if (iomtch[2] == ">") flags = O_APPEND|O_WRONLY;

        if (++end != parameters.end()) {
          new_redir = new _redirect((std::string)(**end), flags);
        } else {
          throw cutlet::exception("filename expected");
        }
      } else {
        new_redir = new _redirect(atoi(iomtch[2].str().c_str()));
      }
      new_cmd->io(redirect, new_redir);
      ios.push_back(new_redir);

      commands.push_back(new_cmd);
      start = ++end;
      if (end == parameters.end()) break;

      use_last = false;

    } else if (std::regex_match(bit, iomtch, re_redir_in)) {
      // Redirect input.
      _command *new_cmd = new _command(start, end);

      // Pipe to stdin for new command if needed.
      if (use_last)
        new_cmd->io(STDIN_FILENO, ios.back());

      std::clog << "DEBUG: redirect input" << std::endl;

      int redirect = STDIN_FILENO;
      // Determine the file descriptor for the pipe.
      if (iomtch[1] != "") {
        redirect = atoi(iomtch[1].str().c_str());
      }

      _redirect *new_redir = nullptr;
      if (++end != parameters.end()) {
        new_redir = new _redirect((std::string)(**end), O_RDONLY);
      } else {
        throw cutlet::exception("filename expected");
      }

      new_cmd->io(redirect, new_redir);
      ios.push_back(new_redir);

      commands.push_back(new_cmd);
      start = ++end;
      if (end == parameters.end()) break;

      use_last = false;
    }
  }

  if (start != parameters.end()) {
    _command *new_cmd = new _command(start, end);
    // Pipe to stdin for new command if needed.
    if (use_last)
      new_cmd->io(STDIN_FILENO, ios.back());
    commands.push_back(new_cmd);
  }

  // Execute the commands.
  for (auto &cmd: commands) (*cmd)();

  int status;
  for (auto &cmd: commands) status = cmd->wait();

  // Cleanup all the io redirects and commands.
  for (auto &io: ios) delete io;
  for (auto &cmd: commands) delete cmd;

  return new cutlet::string(status);
}

// def env variable ¿=? ¿value?
static cutlet::variable::pointer _environ(cutlet::interpreter &interp,
                                          const cutlet::list &parameters) {
  (void)interp;

  // Make sure we have to right number of parameters.
  size_t p_count = parameters.size();
  if (p_count < 2 or p_count > 3) {
   std::stringstream mesg;
   mesg << "Invalid number of parameters for environ "
        << (p_count >= 1 ? cutlet::convert<std::string>(parameters[0]) : "")
        << " (1 <= " << p_count
        << " <= 3).\n environ variable ¿=? ¿value?";
   throw std::runtime_error(mesg.str());
  }

  if (p_count == 1) {
#ifdef HAVE_SECURE_GETENV
    char *res =
      secure_getenv(cutlet::convert<std::string>(parameters[0]).c_str());
#else
    char *res = getenv(cutlet::convert<std::string>(parameters[0]).c_str());
#endif
    if (res == nullptr) return nullptr;
    return new cutlet::string(res);

  } else {
    cutlet::variable::pointer value = parameters[1];

    if (p_count == 3) {
      if (cutlet::convert<std::string>(value) != "=") {
        std::stringstream mesg;
        mesg << "Invalid token " << cutlet::convert<std::string>(value)
             << " for environ\n environ variable ¿=? ¿value?";
        throw std::runtime_error(mesg.str());
      }

      value = parameters[2];

      if (cutlet::convert<std::string>(value) == "nil") {
        unsetenv(cutlet::convert<std::string>(parameters[0]).c_str());
      } else {
        setenv(cutlet::convert<std::string>(parameters[0]).c_str(),
               cutlet::convert<std::string>(value).c_str(), 1);
      }
    }

  }

  return nullptr;
}

/***************
 * init_cutlet *
 ***************/

// We need to declare init_cutlet as a C function.
extern "C" {
  DECLSPEC void init_cutlet(cutlet::interpreter *interp);
}

void init_cutlet(cutlet::interpreter *interp) {
  interp->add("eval", _eval);
  interp->add("env", _environ,
              "env variable ¿=? ¿value?\n\n"
              "");
}
