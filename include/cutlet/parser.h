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

#include <exception>
#include <list>
#include <stack>
#include <string>
#include <istream>
#include <regex>

#ifndef _PARSER_H
#define _PARSER_H

namespace parser {
  class token;
  class tokenizer;
}

std::ostream &operator <<(std::ostream &os, const parser::token &token);

std::ostream &operator <<(std::ostream &os,
                          const parser::tokenizer &tokenizer);

/** @ns */
namespace parser {

  /**
   */
  class token {
  public:
    token(unsigned int id, const std::string &value);
    token(unsigned int id, const std::string &value, std::streampos position);
    token(unsigned int id, const std::string &value, std::streampos position,
          std::streamoff offset);
    token(const token &other);

    virtual ~token() noexcept;

    /** Position in the code stream where the token was found.
     * @return Stream start position of the token.
     */
    std::streampos position() const { return _position; }

    token &operator =(const token &other);

    bool operator ==(const token &other) const;
    bool operator ==(unsigned int id) const;

    operator unsigned int() const { return _id; }
    operator const std::string &() const { return _value; }

    friend class tokenizer;

  private:
    /** Token type ID. */
    unsigned int _id;
    std::string _value;
    std::streampos _position;

    /** If the content of the token isn't at the stream position we specify the
     * offset to the content here. This is necessary if the token is to be
     * parsed further.
     */
    std::streamoff _offset;
  };

  class syntax_error : public std::exception {
  public:
    syntax_error() noexcept;
    syntax_error(const std::string &message) noexcept;
    syntax_error(const std::string &message, token token) noexcept;
    syntax_error(const exception& other) noexcept;
    syntax_error(const syntax_error& other) noexcept;
    virtual ~syntax_error() noexcept;

    syntax_error& operator=(const syntax_error &other) noexcept;

    virtual const char* what() const noexcept;
    const token &get_token() const noexcept;

  protected:
    std::string _message;
    token _token;
  };

  /** Tokenizer for a recursive decent parser.
   */
  class tokenizer {
  public:
    static const unsigned int T_EOF = 0;
    static const unsigned int T_INVALID = -1;

    tokenizer();
    virtual ~tokenizer() noexcept;

    void parse(const std::string &code);
    void parse(std::istream &code);

    /** Pop the token from the front of the list and return it. If there aren't
     * any tokens available a parser::syntax_error is thrown.
     */
    token get_token();
    token front();

    bool expect(unsigned int id) noexcept;
    bool expect(unsigned int id, const std::string &value) noexcept;

    void permit(unsigned int id);
    void permit(unsigned int id, const std::string &value);

    bool is_more();
    operator bool() { return is_more(); }

    void next();

    virtual void reset() noexcept;

    /** Removes the token from the from the front of the list and pushes it
     * into the tokenizer as code to be parsed further.
     */
    void push();
    void push(token value);
    void push(const std::string &code);
    void push(std::istream &code);

    /** Pop any current code from the tokenizer and restore any code that was
     * push earlier.
     */
    void pop();

    friend std::ostream &::operator <<(std::ostream &os,
                                       const parser::tokenizer &tokenizer);

  protected:
    /** The list of parsed tokens. */
    std::list<token> tokens;
    std::string code;
    std::istream *stream;

    std::streampos position;

    void add_token_pattern(unsigned int token_id, const std::string &pattern);
    void clear_token_patterns();

    /**
     */
    virtual void parse_tokens();

  private:
    struct token_pat_s {
      unsigned int token_id;
      std::regex pattern;
    };
    std::list<token_pat_s> _patterns;

    struct stack_s {
      std::list<token> tokens;
      std::string code;
      std::istream *stream;
      std::streampos position;
    };
    std::stack<stack_s> _states;

    void parse_next_token();
  };

  class grammer {
  public:
    grammer() : tokens(nullptr) {}

    void eval(const token &code);
    void eval(const std::string &code);
    void eval(std::istream &code);

  protected:
    tokenizer *tokens;

    virtual void entry() = 0;
  };
}

#endif /* _PARSER_H */
