/*                                                                  -*- c++ -*-
 * Copyright © 2018 Ron R Wills <ron@digitalcombine.ca>
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

/******************************************************************************
 * class cutlet::frame
 */

/************************
 * cutlet::frame::frame *
 ************************/

cutlet::frame::frame(const std::string &label)
  : _state(FS_RUNNING), _label(label) {}

cutlet::frame::frame(pointer uplevel, const std::string &label)
  : _state(FS_RUNNING), _uplevel(uplevel), _label(label) {}

/*************************
 * cutlet::frame::~frame *
 *************************/

cutlet::frame::~frame() noexcept {
  memory::gc::collect();
}

/***************************
 * cutlet::frame::variable *
 ***************************/

cutlet::variable::pointer
cutlet::frame::variable(const std::string &name) const {
  // Search for the variable and return it's value if it is found.
  auto item = _variables.find(name);
  if (item != _variables.end())
    return item->second;

  // Return a null value if we don't have it.
  return nullptr;
}

void cutlet::frame::variable(const std::string &name,
                             variable::pointer value) {
  if (value.is_null()) {
    // If the value in null erase the variable from the frame.
    _variables.erase(name);
  } else {
    // Set the variable's new value.
    _variables[name] = value;
  }
}

cutlet::list cutlet::frame::variables() const {
  cutlet::list names;
  for (auto &it: _variables) {
    names.push_back(new cutlet::string(it.first));
  }
  return names;
}

/***********************
 * cutlet::frame::done *
 ***********************/

void cutlet::frame::done(variable::pointer result) {
  _return = result;
  _state = FS_DONE;
}

bool cutlet::frame::done() const {
  switch (_state) {
  case FS_DONE:
  case FS_BREAK:
  case FS_CONTINUE:
    return true;
  case FS_RUNNING:
    return false;
  }
}

/************************
 * cutlet::frame::state *
 ************************/

cutlet::frame::state_t cutlet::frame::state() const {
  return _state;
}

void cutlet::frame::state(cutlet::frame::state_t new_state) {
  _state = new_state;
}

/************************
 * cutlet::frame::label *
 ************************/

void cutlet::frame::label(const std::string &value) {
  _label = value;
}

std::string cutlet::frame::label() const {
  return _label;
}

/**************************
 * cutlet::frame::uplevel *
 **************************/

cutlet::frame::pointer
cutlet::frame::uplevel(unsigned int levels) const {
  cutlet::frame::pointer result;
  bool end = false;

  // Returning a reference to ourselves just breaks things.
  if (levels == 0)
    throw std::runtime_error("Internal error returning a frame level 0.");

  result = _uplevel;
  --levels;

  while (result and levels > 0 and not end) {
    /* We don't allow this to go pass a frame if the global enviroment was
     * changed. We don't want to break the sandboxing.
     */
    if (not result->_sandbox_orig.is_null())
      end = true;

    result = result->_uplevel;
    --levels;
  }

  if (not result or (levels and end))
    throw std::runtime_error("Frame level out of range");

  return result;
}

/*************************
 * cutlet::frame::parent *
 *************************/

cutlet::frame::pointer cutlet::frame::parent() const {
  return _uplevel;
}

void cutlet::frame::parent(pointer frame) {
  _uplevel = frame;
}

/******************************************************************************
 * class cutlet::block_frame
 */

/************************************
 * cutlet::block_frame::block_frame *
 ************************************/

cutlet::block_frame::block_frame(cutlet::frame::pointer uplevel)
  : frame(uplevel) {
}

cutlet::block_frame::block_frame(const std::string &label,
                                 cutlet::frame::pointer uplevel)
  : frame(uplevel, label) {}

/*************************************
 * cutlet::block_frame::~block_frame *
 *************************************/

cutlet::block_frame::~block_frame() noexcept {}

/*********************************
 * cutlet::block_frame::variable *
 *********************************/

cutlet::variable::pointer
cutlet::block_frame::variable(const std::string &name) const {
  // First see if we have the variable.
  cutlet::variable::pointer result = cutlet::frame::variable(name);

  // If we don't have the variable then check up a level.
  if (not result)
    result = _uplevel->variable(name);

  // Return what we find.
  return result;
}

void cutlet::block_frame::variable(const std::string &name,
                                   variable::pointer value) {
  // First we see if this frame has the variable.
  cutlet::variable::pointer var = cutlet::frame::variable(name);
  if (not var) {

    // If we don't have the variable we look up levels.
    var = _uplevel->variable(name);
    if (var) {
      // Upper levels have the variable so set their value.
      _uplevel->variable(name, value);
      return;
    }
  }

  // Either we already have the variable or it wasn't found so set the value in
  // this frame.
  cutlet::frame::variable(name, value);
}

/*****************************
 * cutlet::block_frame::done *
 *****************************/

void cutlet::block_frame::done(variable::pointer result) {
  uplevel(1)->done(result);
}

bool cutlet::block_frame::done() const {
  return uplevel(1)->done();
}

/******************************
 * cutlet::block_frame::state *
 ******************************/

cutlet::frame::state_t cutlet::block_frame::state() const {
  return _uplevel->state();
}

void cutlet::block_frame::state(cutlet::frame::state_t new_state) {
  _uplevel->state(new_state);
}

/******************************
 * cutlet::block_frame::label *
 ******************************/

std::string cutlet::block_frame::label() const {
  return std::string("^ ") + frame::label();
}

/*******************************
 * cutlet::block_frame::parent *
 *******************************/

cutlet::frame::pointer cutlet::block_frame::parent() const {
  return _parent;
}

void cutlet::block_frame::parent(pointer frame) {
  _parent = frame;
}

/******************************************************************************
 * class cutlet::loop_frame
 */

/************************************
 * cutlet::loop_frame::loop_frame *
 ************************************/

cutlet::loop_frame::loop_frame(frame::pointer uplevel)
  : block_frame(uplevel) {
}

cutlet::loop_frame::loop_frame(const std::string &label,
                               frame::pointer uplevel)
  : block_frame(label, uplevel) {
}

/*************************************
 * cutlet::loop_frame::~loop_frame *
 *************************************/

cutlet::loop_frame::~loop_frame() noexcept {}

/*****************************
 * cutlet::loop_frame::done *
 *****************************/

bool cutlet::loop_frame::done() const {
  return (parent()->done() or _uplevel->done());
}

/******************************
 * cutlet::loop_frame::state *
 ******************************/

cutlet::frame::state_t cutlet::loop_frame::state() const {
  if (parent()->state() == FS_RUNNING) {
    return _uplevel->state();
  }
  return parent()->state();
}

void cutlet::loop_frame::state(cutlet::frame::state_t new_state) {
  switch (new_state) {
  case FS_RUNNING:
  case FS_DONE:
    _uplevel->state(new_state);
  default:
    parent()->state(new_state);
    break;
  }
}

/******************************
 * cutlet::loop_frame::label *
 ******************************/

std::string cutlet::loop_frame::label() const {
  return std::string("@ ") + frame::label();
}
