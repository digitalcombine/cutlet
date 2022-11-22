/*                                                                  -*- c++ -*-
 * Copyright © 2020 Ron R Wills <ron@digitalcombine.ca>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <cutlet>

// We need to declare init_cutlet as a C function.
extern "C" {
  DECLSPEC void init_cutlet(cutlet::interpreter *interp);
}

class _frame_var : public cutlet::variable {
public:
  _frame_var(cutlet::frame::pointer frame);
  virtual ~_frame_var() noexcept;

  virtual cutlet::variable::pointer
  operator ()(cutlet::variable::pointer self,
              cutlet::interpreter &interp,
              const cutlet::list &arguments);

  virtual operator std::string() const;

private:
  cutlet::frame::pointer _frame;
};

_frame_var::_frame_var(cutlet::frame::pointer frame) : _frame(frame) {}

_frame_var::~_frame_var() noexcept {}

/****************************
 * _frame_var::operator () *
 ****************************/

cutlet::variable::pointer
_frame_var::operator ()(cutlet::variable::pointer self,
                        cutlet::interpreter &interp,
                        const cutlet::list &arguments) {
  (void)self;
  (void)interp;

  std::string op = *(arguments[0]);

  // XXX local name ??=? value?
  if (op == "label") {
    // $frame label ??=? value?
    return new cutlet::string(_frame->label());

  } else if (op == "state") {
    // $frame state ??=? value?
    switch (_frame->state()) {
    case cutlet::frame::FS_DONE:
      return new cutlet::string("done");
    case cutlet::frame::FS_RUNNING:
      return new cutlet::string("running");
    case cutlet::frame::FS_BREAK:
      return new cutlet::string("break");
    case cutlet::frame::FS_CONTINUE:
      return new cutlet::string("continue");
    }

  } else if (op == "variables") {
    return new cutlet::list(_frame->variables());
  }

  throw std::runtime_error(std::string("Unknown operator ") +
                           op + " for frame variable.");
}

_frame_var::operator std::string() const {
  return "frame(" + _frame->label() + ")";
}

/***************
 * debug.stack *
 ***************/

static cutlet::variable::pointer _stack(cutlet::interpreter &interp,
                                        const cutlet::list &arguments) {
  cutlet::list *frms = new cutlet::list();

  int count = interp.frames();
  for (int c = 1; c < count; c++) {
    frms->push_back(new _frame_var(interp.frame(c)));
  }

  return frms;
}

/***************
 * init_cutlet *
 ***************/

void init_cutlet(cutlet::interpreter *interp) {
  interp->add("debug.stack", _stack);
}
