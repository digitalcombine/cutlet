/*                                                                 -*- c++ -*-
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

#include <libcutlet/parser>
#include <iostream>

//#define DEBUG_PARSER 1

#if defined(DEBUG_PARSER)
#pragma message ("PARSER debugging enabled")
#endif

/******************************************************************************
 */

std::ostream &operator <<(std::ostream &os, const parser::token &token) {
  os << token.position() << ": " << (unsigned int)token
     << " " << (const std::string &)token;
  return os;
}

std::ostream &operator <<(std::ostream &os,
                          const parser::tokenizer &tokenizer) {
  for (auto &token: tokenizer.tokens) {
    os << token << "\n";
  }
  return os;
}

/******************************************************************************
 * class parser::token
 */

/************************
 * parser::token::token *
 ************************/

parser::token::token(unsigned int id, const std::string &value)
  : _id(id), _value(value), _position(0), _offset(0) {
#if defined(DEBUG_PARSER)
  if (_id != 7) {
    std::clog << "TOKEN:";
    switch (_id) {
    case 0: std::clog << "EOF"; break;
    case 1: std::clog << "WORD"; break;
    case 2: std::clog << "VAR"; break;
    case 3: std::clog << "STRING"; break;
    case 4: std::clog << "SUBCMD"; break;
    case 5: std::clog << "BLOCK"; break;
    case 6: std::clog << "COMMENT"; break;
    case 7: std::clog << "EOL"; break;
    }
    std::clog << ":" << _position << ":" << _offset
              << ": " <<_value << std::endl;
  }
#endif
}

parser::token::token(const token &other)
  : _id(other._id), _value(other._value), _file(other._file),
    _position(other._position), _offset(other._offset) {
#if defined(DEBUG_PARSER)
  /*if (_id != 7) {
    std::clog << "COPY:" << _id << ":" << _position << ":" << _offset
              << ": " << _value << std::endl;
              }*/
#endif
}

parser::token::token(unsigned int id, const std::string &value,
                     std::streampos position)
  : _id(id), _value(value), _position(position), _offset(0) {
#if defined(DEBUG_PARSER)
  if (_id != 7) {
    std::clog << "TOKEN:";
    switch (_id) {
    case 0: std::clog << "EOF"; break;
    case 1: std::clog << "WORD"; break;
    case 2: std::clog << "VAR"; break;
    case 3: std::clog << "STRING"; break;
    case 4: std::clog << "SUBCMD"; break;
    case 5: std::clog << "BLOCK"; break;
    case 6: std::clog << "COMMENT"; break;
    case 7: std::clog << "EOL"; break;
    }
    std::clog << ":" << _position << ":" << _offset
              << ": " <<_value << std::endl;
  }
#endif
}

parser::token::token(unsigned int id, const std::string &value,
                     std::streampos position, std::streamoff offset)
  : _id(id), _value(value), _position(position), _offset(offset) {
#if defined(DEBUG_PARSER)
  if (_id != 7) {
    std::clog << "TOKEN:";
    switch (_id) {
    case 0: std::clog << "EOF"; break;
    case 1: std::clog << "WORD"; break;
    case 2: std::clog << "VAR"; break;
    case 3: std::clog << "STRING"; break;
    case 4: std::clog << "SUBCMD"; break;
    case 5: std::clog << "BLOCK"; break;
    case 6: std::clog << "COMMENT"; break;
    case 7: std::clog << "EOL"; break;
    }
    std::clog << ":" << _position << ":" << _offset
              << ": " <<_value << std::endl;
  }
#endif
}

/*************************
 * parser::token::~token *
 *************************/

parser::token::~token() noexcept {
}

/*****************************
 * parser::token::operator = *
 *****************************/

parser::token &parser::token::operator =(const token &other) {
  if (this != &other) {
    _id = other._id;
    _value = other._value;
    _position = other._position;
    _offset = other._offset;
  }
  return *this;
}

/******************************
 * parser::token::operator == *
 ******************************/

bool parser::token::operator ==(const token &other) const {
  if (_id == other._id and _value == other._value)
    return true;
  return false;
}

bool parser::token::operator ==(unsigned int id) const {
  return _id == id;
}

/******************************************************************************
 * class parser::syntax_error
 */

/**************************************
 * parser::syntax_error::syntax_error *
 **************************************/

parser::syntax_error::syntax_error() noexcept
  : _message("syntax error"), _token(0, "") {
}

parser::syntax_error::syntax_error(const std::string &message) noexcept
  : _message(message), _token(0, "") {
}

parser::syntax_error::syntax_error(const std::string &message,
                                   token token) noexcept
  : _message(message), _token(token) {
}

parser::syntax_error::syntax_error(const exception& other) noexcept
  : _message(other.what()), _token(0, "") {
}

parser::syntax_error::syntax_error(const syntax_error& other) noexcept
  : _message(other._message), _token(other._token) {
}

/***************************************
 * parser::syntax_error::~syntax_error *
 ***************************************/

parser::syntax_error::~syntax_error() noexcept {
}

/************************************
 * parser::syntax_error::operator = *
 ************************************/

parser::syntax_error &
parser::syntax_error::operator =(const syntax_error &other) noexcept {
  if (this != &other)
    _message = other._message;
  return *this;
}

/******************************
 * parser::syntax_error::what *
 ******************************/

const char* parser::syntax_error::what() const noexcept {
  return _message.c_str();
}

/***********************************
 * parser::syntax_error::get_token *
 ***********************************/

const parser::token &parser::syntax_error::get_token() const noexcept {
  return _token;
}

/******************************************************************************
 * class parser::tokenizer
 */

/********************************
 * parser::tokenizer::tokenizer *
 ********************************/

parser::tokenizer::tokenizer() : stream(NULL), position(0) {
  tokens.push_back(token(T_EOF, "", position));
}

/*********************************
 * parser::tokenizer::~tokenizer *
 *********************************/

parser::tokenizer::~tokenizer() noexcept {
}

/****************************
 * parser::tokenizer::parse *
 ****************************/

void parser::tokenizer::parse(const std::string &source) {
  push(source);
}

void parser::tokenizer::parse(std::istream &source) {
  push(source);
}

/********************************
 * parser::tokenizer::get_token *
 ********************************/

parser::token parser::tokenizer::get_token() {
  if (is_more()) {
    auto result = tokens.front();
    tokens.pop_front();
    return result;
  }
  throw syntax_error("Incomplete syntax", token(T_INVALID, "", position));
}

/****************************
 * parser::tokenizer::front *
 ****************************/

parser::token parser::tokenizer::front() {
  if (is_more()) return tokens.front();
  throw syntax_error("Incomplete syntax", token(T_INVALID, "", position));
}

/*****************************
 * parser::tokenizer::expect *
 *****************************/

bool parser::tokenizer::expect(unsigned int id) noexcept {
  if (is_more()) {
    return (tokens.front() == id);
  }
  return false;
}

bool parser::tokenizer::expect(unsigned int id,
                               const std::string &value) noexcept {
  if (is_more()) {
    token tk(id, value);
    return (tokens.front() == tk);
  }
  return false;
}

/*****************************
 * parser::tokenizer::permit *
 *****************************/

void parser::tokenizer::permit(unsigned int id) {
  if (expect(id)) {
    tokens.pop_front();
    return;
  }
  throw syntax_error("Got unexpected value of " +
                     (const std::string &)tokens.front(),
                     tokens.front());
}

void parser::tokenizer::permit(unsigned int id, const std::string &value) {

  if (expect(id, value)) {
    tokens.pop_front();
    return;
  }
  throw syntax_error("Got unexpected value of " +
                     (const std::string &)tokens.front(),
                     tokens.front());
}

/******************************
 * parser::tokenizer::is_more *
 ******************************/

bool parser::tokenizer::is_more() {
  if (not tokens.empty()) return true;

  parse_tokens(); // No tokens found so attempt to get more.
  if (not tokens.empty()) return true;
  return false;
}

/***************************
 * parser::tokenizer::next *
 ***************************/

void parser::tokenizer::next() {
  if (is_more()) tokens.pop_front();
}

/****************************
 * parser::tokenizer::reset *
 ****************************/

void parser::tokenizer::reset() noexcept {
  tokens.clear();
  code.clear();
  stream = NULL;
}

/***************************
 * parser::tokenizer::push *
 ***************************/

void parser::tokenizer::push() {
#if defined(DEBUG_PARSER)
  std::clog << "TOKENIZER: pushing empty" << std::endl;
#endif
  auto token = get_token();
  _states.push({tokens, code, file, stream, position});
  reset();
  position = token._position + token._offset;
  code = token._value;
  parse_tokens();
}

void parser::tokenizer::push(token value) {
#if defined(DEBUG_PARSER)
  std::clog << "TOKENIZER: pushing token" << std::endl;
#endif
  _states.push({tokens, code, file, stream, position});
  reset();
  position = value._position + value._offset;
  code = value._value;
  file = value._file; // XXX
  parse_tokens();
}

void parser::tokenizer::push(const std::string &value) {
#if defined(DEBUG_PARSER)
  std::clog << "TOKENIZER: pushing string" << std::endl;
#endif
  std::clog << "pushing " << value << std::endl;
  _states.push({tokens, code, file, stream, position});
  reset();
  position = 0;
  code = value;
  file = "<string>";
  parse_tokens();
}

void parser::tokenizer::push(std::istream &value, const std::string &source) {
#if defined(DEBUG_PARSER)
  std::clog << "TOKENIZER: pushing stream" << std::endl;
#endif
  _states.push({tokens, code, file, stream, position});
  reset();
  position = value.tellg();
  stream = &value;
  code.clear();
  file = source;
  parse_tokens();
}

/**************************
 * parser::tokenizer::pop *
 **************************/

void parser::tokenizer::pop() {
#if defined(DEBUG_PARSER)
  std::clog << "TOKENIZER: popping" << std::endl;
#endif
  auto &top = _states.top();
  tokens = top.tokens;
  code = top.code;
  stream = top.stream;
  position = top.position;
  _states.pop();
}

/****************************************
 * parser::tokenizer::add_token_pattern *
 ****************************************/

void parser::tokenizer::add_token_pattern(unsigned int token_id,
                                          const std::string &pattern) {
  _patterns.push_back({token_id, std::regex(pattern, std::regex::ECMAScript)});
}

/*******************************************
 * parser::tokenizer::clear_token_patterns *
 *******************************************/

void parser::tokenizer::clear_token_patterns() {
  _patterns.clear();
}

void parser::tokenizer::add_token(unsigned int id, const std::string &value) {
  token result(id, value);
  result._file = file;
  tokens.push_back(result);
}

void parser::tokenizer::add_token(unsigned int id, const std::string &value,
                                  std::streampos spos) {
  token result(id, value, spos);
  result._file = file;
  tokens.push_back(result);
}

void parser::tokenizer::add_token(unsigned int id, const std::string &value,
                                  std::streampos spos,
                                  std::streamoff soff) {
  token result(id, value, spos, soff);
  result._file = file;
  tokens.push_back(result);
}

/***************************************
 * parser::tokenizer::parse_next_token *
 ***************************************/

void parser::tokenizer::parse_next_token() {
  if (not code.empty()) {
    std::smatch match;

    for (auto &pattern: _patterns) {
      if (regex_search(code, match, pattern.pattern,
                       std::regex_constants::match_continuous)) {
        tokens.push_back(token(pattern.token_id, match.str(), position));
        code = code.substr(match.length());
        position += match.length();
        return;
      }
    }

    throw syntax_error("Syntax error", token(T_INVALID, code, position));
  }
}

/***********************************
 * parser::tokenizer::parse_tokens *
 ***********************************/

void parser::tokenizer::parse_tokens() {
  // If we don't have any code then see if we can get more from the stream.
  if (code.empty()) {
    if (stream) {
      position = stream->tellg();
      if (not getline(*stream, code, '\0')) {
        add_token(T_EOF, "", position);
        stream = NULL;
        return;
      }
    } else
      return;
  }

  // If we have some code then tokenize it.
  if (not code.empty()) {
    while (not code.empty())
      parse_next_token();
    if (not stream)
      add_token(T_EOF, "", position);
  }
}

/*****************************************************************************
 * class parser::grammer
 */

/*****************************
 * parser::grammer::~grammer *
 *****************************/

parser::grammer::~grammer() noexcept {}

/*************************
 * parser::grammer::eval *
 *************************/

void parser::grammer::eval(const token &code) {
  if (tokens) {
    tokens->push(code);
    entry();
    tokens->pop();
  } else
    throw std::runtime_error("parser::grammer tokenizer not set");
}

void parser::grammer::eval(const std::string &code) {
  if (tokens) {
    tokens->push(code);
    entry();
    tokens->pop();
  } else
    throw std::runtime_error("parser::grammer tokenizer not set");
}

void parser::grammer::eval(std::istream &code, const std::string &source) {
  if (tokens) {
    tokens->push(code, source);
    entry();
    tokens->pop();
  } else
    throw std::runtime_error("parser::grammer tokenizer not set");
}
