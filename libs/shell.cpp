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

#include <cutlet.h>
#include <vector>
#include <unistd.h>

#include <iostream>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <sys/wait.h>

/**************
 * _exec_pipe *
 **************/

static int _exec_pipe(const cutlet::list &cmd,
                      int read = -1, int write = -1) {
  auto pid = fork();
  if (pid >= 0) {
    if (pid == 0) {
      std::vector<const char *> args;

      for (auto &it: cmd) {
        args.push_back(new char[((std::string)*it).size() + 1]);
        strcpy((char *)args.back(), ((std::string)*it).c_str());
      }
      args.push_back(nullptr);

      if (read != -1) {
        if (dup2(read, STDIN_FILENO) == -1) {
        }
        close(read);
      }
      if (write != -1) {
        if (dup2(write, STDOUT_FILENO) == -1) {
        }
        close(write);
      }

      if (execvp(args[0], (char * const*)&args[0]) == -1) {
        std::cerr << strerror(errno) << std::endl;
        exit(errno);
      }
    } else {
      int status;
      while (waitpid(pid, &status, 0) != pid);
      return status;
    }
  }

  throw std::runtime_error(strerror(errno));
}

// def exec command
static cutlet::variable::pointer _exec(cutlet::interpreter &interp,
                                       const cutlet::list &parameters) {
  (void)interp;

  return new cutlet::string(_exec_pipe(parameters));
}

// def environ variable ¿=? ¿value?
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
  interp->add("exec", _exec);
  interp->add("environ", _environ);
}
