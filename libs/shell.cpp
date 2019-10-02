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

static int _exec_pipe(const cutlet::list &cmd, int read = 0, int write = 1) {
  auto pid = fork();
  if (pid >= 0) {
    if (pid == 0) {
      std::vector<const char *> args;

      for (auto &it: cmd) {
        args.push_back(new char[((std::string)*it).size() + 1]);
        strcpy((char *)args.back(), ((std::string)*it).c_str());
      }
      args.push_back(nullptr);

      /*if (read != 0) {
        dup2(read, 0);
        close(read);
      }
      if (write != 1) {
        dup2(write, 1);
        close(write);
      }*/
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
  return new cutlet::string(_exec_pipe(parameters));
}

// def env variable ¿=? ¿value?
static cutlet::variable::pointer _env(cutlet::interpreter &interp,
                                 const cutlet::list &parameters) {
  return new cutlet::string(_exec_pipe(parameters));
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
  interp->add("env", _env);
  //interp->add("glob")
}
