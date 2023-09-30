/*                                                                 -*- c++ -*-
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

#include "utilities.h"
#include <unistd.h>
#include <stdlib.h>
#include <libcutlet/utilities>

/***********
 * fexists *
 ***********/

bool fexists(const std::string &filename) {
  return (access(filename.c_str(), F_OK) == 0);
}

/************
 * is_space *
 ************/

bool is_space(const std::string &value) {
  if (value == "\u0020") return true;  // SPACE
  if (value == "\u0009") return true;  // CHARACTER TABULATION (tab)

  if (value == "\u00a0") return true; // NO-BREAK SPACE
  if (value == "\u2000") return true; // EN QUAD
  if (value == "\u2001") return true; // EM QUAD
  if (value == "\u2002") return true; // EN SPACE (nut)
  if (value == "\u2003") return true; // EM SPACE (mutton)
  if (value == "\u2004") return true; // THREE-PER-EM SPACE (thick space)
  if (value == "\u2005") return true; // FOUR-PER-EM SPACE (mid space)
  if (value == "\u2006") return true; // SIX-PER-EM SPACE
  if (value == "\u2007") return true; // FIGURE SPACE
  if (value == "\u2008") return true; // PUNCTUATION SPACE
  if (value == "\u2009") return true; // THIN SPACE
  if (value == "\u200a") return true; // HAIR SPACE

  if (value == "\u202f") return true; // NARROW NO-BREAK SPACE
  if (value == "\u205f") return true; // MEDIUM MATHEMATICAL SPACE

  if (value == "\u3000") return true; // IDEOGRAPHIC SPACE
  return false;
}

/**********
 * is_eol *
 **********/

bool is_eol(const std::string &value) {
  // Check for all of the unicode end of line characters.
  if (value == "\u000a")       return true; // LF \n
  if (value == "\u000b")       return true; // VT \v
  if (value == "\u000c")       return true; // FF \f
  if (value == "\u000d\u000a") return true; // CR LF \r\n
  if (value == "\u000d")       return true; // CR \r
  if (value == "\u0085")       return true; // NEL
  if (value == "\u2028")       return true; // LS
  if (value == "\u2029")       return true; // PS
  return false;
}

std::string env(const std::string &name) {
  const char *value = getenv(name.c_str());
  if (value == nullptr) return "";
  return value;
}

/*static bool is_numeric(const std::string &value) {
  switch (value[0]) {
  case '\xe0':
    // Devavagari Digits U+0966 - U+096F
    if (value >= "\u0966" and value <= "\u096f") return true;

    // Bengali Digits U+09E6 - U+09EF
    if (value >= "\u09e6" and value <= "\u09ef") return true;

    // Gurmukhi Digits U+0A66 - U+0A6F
    if (value >= "\u0a66" and value <= "\u0a6f") return true;

    // Gujarati Digits U+0AE6 - U+0AEF
    if (value >= "\u0ae6" and value <= "\u0aef") return true;

    // Oriya Digits U+0B66 - U+0B6F
    if (value >= "\u0b66" and value <= "\u0b6f") return true;

    // Tamil Digits U+0BE6 - U+0BEF
    if (value >= "\u0be6" and value <= "\u0bef") return true;

    // Telugu Digits U+0C66 - U+0C6F
    if (value >= "\u0c66" and value <= "\u0c6f") return true;

    // Kannada Digits U+0CE6 - U+0CEF
    if (value >= "\u0ce6" and value <= "\u0cef") return true;

    // Malayalam Digits U+0D66 - U+0D6F
    if (value >= "\u0d66" and value <= "\u0d6f") return true;

    // Sinhala Digits U+0DE6 - U+0DEF
    if (value >= "\u0de6" and value <= "\u0def") return true;

    // Thai Digits U+0E50 - U+0E59
    if (value >= "\u0e50" and value <= "\u0e59") return true;

    // Lao Digits U+0ED0 - U+0ED9
    if (value >= "\u0ed0" and value <= "\u0ed9") return true;

    // Tibetan Digits U+0F20 - U+0F29
    if (value >= "\u0f20" and value <= "\u0f29") return true;

    break;

  case '\xe1':
    // Myanmar Digits U+1040 - U+1049
    if (std::strcmp(value, "\xe1\x81\x80") >= 0 and
        std::strcmp(value, "\xe1\x81\x89") <= 0)
      return 3;

    // Myanmar Shan Digits U+1090 - U+1099
    if (std::strcmp(value, "\xe1\x82\x90") >= 0 and
        std::strcmp(value, "\xe1\x82\x99") <= 0)
      return 3;

    // Khmer Digits U+17E0 - U+17E9
    if (std::strcmp(value, "\xe1\x9f\xa0") >= 0 and
        std::strcmp(value, "\xe1\x9f\xa9") <= 0)
      return 3;

    // Mongolian Digits U+1810 - U+1819
    if (std::strcmp(value, "\xe1\xa0\x90") >= 0 and
        std::strcmp(value, "\xe1\xa0\x99") <= 0)
      return 3;

    // Limbu Digits U+1946 - U+194F
    if (std::strcmp(value, "\xe1\xa5\x86") >= 0 and
        std::strcmp(value, "\xe1\xa5\x8f") <= 0)
      return 3;

    // New Tai Lue Digits U+19D0 - U+19D9
    if (std::strcmp(value, "\xe1\xa7\x90") >= 0 and
        std::strcmp(value, "\xe1\xa7\x99") <= 0)
      return 3;

    // Tai Tham Hora Digits U+1A80 - U+1A89
    if (std::strcmp(value, "\xe1\xaa\x80") >= 0 and
        std::strcmp(value, "\xe1\xaa\x89") <= 0)
      return 3;

    // Tai Tham Tham Digits U+1A90 - U+1A99
    if (std::strcmp(value, "\xe1\xaa\x90") >= 0 and
        std::strcmp(value, "\xe1\xaa\x99") <= 0)
      return 3;

    // Balinese Digits U+1B50 - U+1B59
    if (std::strcmp(value, "\xe1\xad\x90") >= 0 and
        std::strcmp(value, "\xe1\xad\x99") <= 0)
      return 3;

    // Sundanese Digits U+1BB0 - U+1BB9
    if (std::strcmp(value, "\xe1\xae\xb0") >= 0 and
        std::strcmp(value, "\xe1\xae\xb9") <= 0)
      return 3;

    // Lepcha Digits U+1C40 - U+1C49
    if (std::strcmp(value, "\xe1\xb1\x80") >= 0 and
        std::strcmp(value, "\xe1\xb1\x89") <= 0)
      return 3;

    // Ol Chiki Digits U+1C50 - U+1C59
    if (std::strcmp(value, "\xe1\xb1\x90") >= 0 and
        std::strcmp(value, "\xe1\xb1\x99") <= 0)
      return 3;

    break;

  case '\xea':
    // Vai Digits U+A620 - U+A629
    if (std::strcmp(value, "\xea\x98\xa0") >= 0 and
        std::strcmp(value, "\xea\x98\xa9") <= 0)
      return 3;

    // Saurashtra Digits U+A8D0 - U+A8D9
    if (std::strcmp(value, "\xea\xa3\x90") >= 0 and
        std::strcmp(value, "\xea\xa3\x99") <= 0)
      return 3;

    // Kayah Li Digits U+A900 - U+A909
    if (std::strcmp(value, "\xea\xa4\x80") >= 0 and
        std::strcmp(value, "\xea\xa4\x89") <= 0)
      return 3;

    // Javanese Digits U+A9D0 - U+A9D9
    if (std::strcmp(value, "\xea\xa7\x90") >= 0 and
        std::strcmp(value, "\xea\xa7\x99") <= 0)
      return 3;

    // Myanmar Tai Laing Digits U+A9F0 - U+A9F9
    if (std::strcmp(value, "\xea\xa7\xb0") >= 0 and
        std::strcmp(value, "\xea\xa7\xb9") <= 0)
      return 3;

    // Cham Digits U+AA50 - U+AA59
    if (std::strcmp(value, "\xea\xa9\x90") >= 0 and
        std::strcmp(value, "\xea\xa9\x99") <= 0)
      return 3;

    // Meetei Mayek Digits U+ABF0 - U+ABF9
    if (std::strcmp(value, "\xea\xaf\xb0") >= 0 and
        std::strcmp(value, "\xea\xaf\xb9") <= 0)
      return 3;

    break;

  case '\xf0':
    // Osmanya Digits U+104A0 - U+104A9
    if (std::strcmp(value, "\xf0\x90\x92\xa0") >= 0 and
        std::strcmp(value, "\xf0\x90\x92\xa9") <= 0)
      return 4;

    // Brahmi Digits U+11066 - U+1106F
    if (std::strcmp(value, "\xf0\x91\x81\xa6") >= 0 and
        std::strcmp(value, "\xf0\x91\x81\xaf") <= 0)
      return 4;

    // Sora Sompeng Digits U+110F0 - U+110F9
    if (std::strcmp(value, "\xf0\x91\x83\xb0") >= 0 and
        std::strcmp(value, "\xf0\x91\x83\xb9") <= 0)
      return 4;

    // Chakma Digits U+11136 - U+1113F
    if (std::strcmp(value, "\xf0\x91\x84\xb6") >= 0 and
        std::strcmp(value, "\xf0\x91\x84\xbf") <= 0)
      return 4;

    // Sharada Digits U+111D0 - U+111D9
    if (std::strcmp(value, "\xf0\x91\x87\x90") >= 0 and
        std::strcmp(value, "\xf0\x91\x87\x99") <= 0)
      return 4;

    // Khudawadi Digits U+112F0 - U+112F9
    if (std::strcmp(value, "\xf0\x91\x8b\xb0") >= 0 and
        std::strcmp(value, "\xf0\x91\x8b\xb9") <= 0)
      return 4;

    // Newa Digits U+11450 - U+11459
    if (std::strcmp(value, "\xf0\x91\x91\x90") >= 0 and
        std::strcmp(value, "\xf0\x91\x91\x99") <= 0)
      return 4;

    // Tirhuta Digits U+114D0 - U+114D9
    if (std::strcmp(value, "\xf0\x91\x93\x90") >= 0 and
        std::strcmp(value, "\xf0\x91\x93\x99") <= 0)
      return 4;

    // Modi Digits U+11650 - U+11659
    if (std::strcmp(value, "\xf0\x91\x99\x90") >= 0 and
        std::strcmp(value, "\xf0\x91\x99\x99") <= 0)
      return 4;

    // Takri Digits U+116C0 - U+116C9
    if (std::strcmp(value, "\xf0\x91\x9b\x80") >= 0 and
        std::strcmp(value, "\xf0\x91\x9b\x89") <= 0)
      return 4;

    // Ahom Digits U+11730 - U+11739
    if (std::strcmp(value, "\xf0\x91\x9c\xb0") >= 0 and
        std::strcmp(value, "\xf0\x91\x9c\xb9") <= 0)
      return 4;

    // Warang Citi Digits U+118E0 - U+118E9
    if (std::strcmp(value, "\xf0\x91\xa3\xa0") >= 0 and
        std::strcmp(value, "\xf0\x91\xa3\xa9") <= 0)
      return 4;

    // Bhaiksuki Digits U+11C50 - U+11C59
    if (std::strcmp(value, "\xf0\x91\xb1\x90") >= 0 and
        std::strcmp(value, "\xf0\x91\xb1\x99") <= 0)
      return 4;

    // Masaram Digits U+11D50 - U+11D59
    if (std::strcmp(value, "\xf0\x91\xb5\x90") >= 0 and
        std::strcmp(value, "\xf0\x91\xb5\x99") <= 0)
      return 4;

    // Mro Digits U+16A60 - U+16A69
    if (std::strcmp(value, "\xf0\x96\xa9\xa0") >= 0 and
        std::strcmp(value, "\xf0\x96\xa9\xa9") <= 0)
      return 4;

    // Pahawh Digits U+16B50 - U+16B59
    if (std::strcmp(value, "\xf0\x96\xad\x90") >= 0 and
        std::strcmp(value, "\xf0\x96\xad\x99") <= 0)
      return 4;

    // Mathematical Bold Digits U+1D7CE - U+1D7D7
    // Mathematical Double-Struck Digits U+1D7D8 - U+1D7E1
    // Mathematical Sans-Serif Digits U+1D7E2 - U+1D7EB
    // Mathematical Sans-Serif Bold Digits U+1D7EC - U+1D7F5
    // Mathematical Monospace Digits U+1D7F6 - U+1D7FF
    if (std::strcmp(value, "\xf0\x9d\x9f\x8e") >= 0 and
        std::strcmp(value, "\xf0\x9d\x9f\xbf") <= 0)
      return 4;

    // Adlam Digits U+1E950 - U+1E959
    if (std::strcmp(value, "\xf0\x9e\xa5\x90") >= 0 and
        std::strcmp(value, "\xf0\x9e\xa5\x99") <= 0)
      return 4;

    break;

  default:
    // Digits U+0030 - U+0039
    if (*value >= '\x30' and *value <= '\x39')
      return 1;

    // Arabic-Indic Digits U+0660 - U+0669
    if (std::strcmp(value, "\xd9\xa0") >= 0 and
        std::strcmp(value, "\xd9\xa9") <= 0)
      return 2;

    // Extended Arabic-Indic Digits U+06F0-U+06F9
    if (std::strcmp(value, "\xdb\xb0") >= 0 and
        std::strcmp(value, "\xdb\xb9") <= 0)
      return 2;

    // NKo Digits U+07C0 - U+07C9
    if (std::strcmp(value, "\xdf\x80") >= 0 and
        std::strcmp(value, "\xdf\x89") <= 0)
      return 2;

    // Fullwidth Digits U+FF10 - U+FF19
    if (std::strcmp(value, "\xef\xbc\x90") >= 0 and
        std::strcmp(value, "\xef\xbc\x99") <= 0)
      return 3;

    break;
  }

  return 0;
}

static unsigned int to_numeric(const std::string &value) {
  switch (value[0]) {
  case '\xe0':
    // Devavagari Digits U+0966 - U+096F
    if (value >= "\u0966" and value <= "\u096f") return value[2] - '\xa6';

    // Bengali Digits U+09E6 - U+09EF
    if (value >= "\u09e6" and value <= "\u09ef") return value[2] - '\xa6';

    // Gurmukhi Digits U+0A66 - U+0A6F
    if (value >= "\u0a66" and value <= "\u0a6f") return value[2] - '\xa6';

    // Gujarati Digits U+0AE6 - U+0AEF
    if (value >= "\u0ae6" and value <= "\u0aef") return value[2] - '\xa6';

    // Oriya Digits U+0B66 - U+0B6F
    if (value >= "\u0b66" and value <= "\u0b6f") return value[2] - '\xa6';

    // Tamil Digits U+0BE6 - U+0BEF
    if (value >= "\u0be6" and value <= "\u0bef") return value[2] - '\xa6';

    // Telugu Digits U+0C66 - U+0C6F
    if (value >= "\u0c66" and value <= "\u0c6f") return value[2] - '\xa6';

    // Kannada Digits U+0CE6 - U+0CEF
    if (value >= "\u0ce6" and value <= "\u0cef") return value[2] - '\xa6';

    // Malayalam Digits U+0D66 - U+0D6F
    if (value >= "\u0d66" and value <= "\u0d6f") return value[2] - '\xa6';

    // Sinhala Digits U+0DE6 - U+0DEF
    if (value >= "\u0de6" and value <= "\u0def") return value[2] - '\xa6';

    // Thai Digits U+0E50 - U+0E59
    if (value >= "\u0e50" and value <= "\u0e59") return value[2] - '\x90';

    // Lao Digits U+0ED0 - U+0ED9
    if (value >= "\u0ed0" and value <= "\u0ed9") return value[2] - '\x90';

    // Tibetan Digits U+0F20 - U+0F29
    if (value >= "\u0f20" and value <= "\u0f29") return value[2] - '\xa0';

    break;

  case '\xe1':
    // Myanmar Digits U+1040 - U+1049
    if (value >= "\u1040" and value <= "\u1049") return value[2] - '\x80';

    // Myanmar Shan Digits U+1090 - U+1099
    if (value >= "\u1090" and value <= "\u1099") return value[2] - '\x90';

    // Khmer Digits U+17E0 - U+17E9
    if (std::strcmp(value, "\xe1\x9f\xa0") >= 0 and
        std::strcmp(value, "\xe1\x9f\xa9") <= 0)
      return value[2] - '\xa0';

    // Mongolian Digits U+1810 - U+1819
    if (value >= "\u1810" and value <= "\u1819") return value[2] - '\x90';

    // Limbu Digits U+1946 - U+194F
    if (std::strcmp(value, "\xe1\xa5\x86") >= 0 and
        std::strcmp(value, "\xe1\xa5\x8f") <= 0)
      return value[2] - '\x86';

    // New Tai Lue Digits U+19D0 - U+19D9
    if (value >= "\u19d0" and value <= "\u19d9") return value[2] - '\x90';

    // Tai Tham Hora Digits U+1A80 - U+1A89
    if (std::strcmp(value, "\xe1\xaa\x80") >= 0 and
        std::strcmp(value, "\xe1\xaa\x89") <= 0)
      return value[2] - '\x80';

    // Tai Tham Tham Digits U+1A90 - U+1A99
    if (value >= "\u1a90" and value <= "\u1a99") return value[2] - '\x90';

    // Balinese Digits U+1B50 - U+1B59
    if (value >= "\u1b50" and value <= "\u1b59") return value[2] - '\x90';

    // Sundanese Digits U+1BB0 - U+1BB9
    if (std::strcmp(value, "\xe1\xae\xb0") >= 0 and
        std::strcmp(value, "\xe1\xae\xb9") <= 0)
      return value[2] - '\xb0';

    // Lepcha Digits U+1C40 - U+1C49
    if (std::strcmp(value, "\xe1\xb1\x80") >= 0 and
        std::strcmp(value, "\xe1\xb1\x89") <= 0)
      return value[2] - '\x80';

    // Ol Chiki Digits U+1C50 - U+1C59
    if (value >= "\u1c50" and value <= "\u1c59") return value[2] - '\x90';

    break;

  case '\xea':
    // Vai Digits U+A620 - U+A629
    if (std::strcmp(value, "\xea\x98\xa0") >= 0 and
        std::strcmp(value, "\xea\x98\xa9") <= 0)
      return value[2] - '\xa0';

    // Saurashtra Digits U+A8D0 - U+A8D9
    if (std::strcmp(value, "\xea\xa3\x90") >= 0 and
        std::strcmp(value, "\xea\xa3\x99") <= 0)
      return value[2] - '\x90';

    // Kayah Li Digits U+A900 - U+A909
    if (std::strcmp(value, "\xea\xa4\x80") >= 0 and
        std::strcmp(value, "\xea\xa4\x89") <= 0)
      return value[2] - '\x80';

    // Javanese Digits U+A9D0 - U+A9D9
    if (std::strcmp(value, "\xea\xa7\x90") >= 0 and
        std::strcmp(value, "\xea\xa7\x99") <= 0)
      return value[2] - '\x90';

    // Myanmar Tai Laing Digits U+A9F0 - U+A9F9
    if (std::strcmp(value, "\xea\xa7\xb0") >= 0 and
        std::strcmp(value, "\xea\xa7\xb9") <= 0)
      return value[2] - '\xb0';

    // Cham Digits U+AA50 - U+AA59
    if (std::strcmp(value, "\xea\xa9\x90") >= 0 and
        std::strcmp(value, "\xea\xa9\x99") <= 0)
      return value[2] - '\x90';

    // Meetei Mayek Digits U+ABF0 - U+ABF9
    if (std::strcmp(value, "\xea\xaf\xb0") >= 0 and
        std::strcmp(value, "\xea\xaf\xb9") <= 0)
      return value[2] - '\xb0';

    break;

  case '\xf0':
    // Osmanya Digits U+104A0 - U+104A9
    if (std::strcmp(value, "\xf0\x90\x92\xa0") >= 0 and
        std::strcmp(value, "\xf0\x90\x92\xa9") <= 0)
      return value[3] - '\xa0';

    // Brahmi Digits U+11066 - U+1106F
    if (std::strcmp(value, "\xf0\x91\x81\xa6") >= 0 and
        std::strcmp(value, "\xf0\x91\x81\xaf") <= 0)
      return value[3] - '\xa6';

    // Sora Sompeng Digits U+110F0 - U+110F9
    if (std::strcmp(value, "\xf0\x91\x83\xb0") >= 0 and
        std::strcmp(value, "\xf0\x91\x83\xb9") <= 0)
      return value[3] - '\xb0';

    // Chakma Digits U+11136 - U+1113F
    if (std::strcmp(value, "\xf0\x91\x84\xb6") >= 0 and
        std::strcmp(value, "\xf0\x91\x84\xbf") <= 0)
      return value[3] - '\xb6';

    // Sharada Digits U+111D0 - U+111D9
    if (std::strcmp(value, "\xf0\x91\x87\x90") >= 0 and
        std::strcmp(value, "\xf0\x91\x87\x99") <= 0)
      return value[3] - '\x90';

    // Khudawadi Digits U+112F0 - U+112F9
    if (std::strcmp(value, "\xf0\x91\x8b\xb0") >= 0 and
        std::strcmp(value, "\xf0\x91\x8b\xb9") <= 0)
      return value[3] - '\xb0';

    // Newa Digits U+11450 - U+11459
    if (std::strcmp(value, "\xf0\x91\x91\x90") >= 0 and
        std::strcmp(value, "\xf0\x91\x91\x99") <= 0)
      return value[3] - '\x90';

    // Tirhuta Digits U+114D0 - U+114D9
    if (std::strcmp(value, "\xf0\x91\x93\x90") >= 0 and
        std::strcmp(value, "\xf0\x91\x93\x99") <= 0)
      return value[3] - '\x90';

    // Modi Digits U+11650 - U+11659
    if (std::strcmp(value, "\xf0\x91\x99\x90") >= 0 and
        std::strcmp(value, "\xf0\x91\x99\x99") <= 0)
      return value[3] - '\x90';

    // Takri Digits U+116C0 - U+116C9
    if (std::strcmp(value, "\xf0\x91\x9b\x80") >= 0 and
        std::strcmp(value, "\xf0\x91\x9b\x89") <= 0)
      return value[3] - '\x80';

    // Ahom Digits U+11730 - U+11739
    if (std::strcmp(value, "\xf0\x91\x9c\xb0") >= 0 and
        std::strcmp(value, "\xf0\x91\x9c\xb9") <= 0)
      return value[3] - '\xb0';

    // Warang Citi Digits U+118E0 - U+118E9
    if (std::strcmp(value, "\xf0\x91\xa3\xa0") >= 0 and
        std::strcmp(value, "\xf0\x91\xa3\xa9") <= 0)
      return value[3] - '\xa0';

    // Bhaiksuki Digits U+11C50 - U+11C59
    if (std::strcmp(value, "\xf0\x91\xb1\x90") >= 0 and
        std::strcmp(value, "\xf0\x91\xb1\x99") <= 0)
      return value[3] - '\x90';

    // Masaram Digits U+11D50 - U+11D59
    if (std::strcmp(value, "\xf0\x91\xb5\x90") >= 0 and
        std::strcmp(value, "\xf0\x91\xb5\x99") <= 0)
      return value[3] - '\x90';

    // Mro Digits U+16A60 - U+16A69
    if (std::strcmp(value, "\xf0\x96\xa9\xa0") >= 0 and
        std::strcmp(value, "\xf0\x96\xa9\xa9") <= 0)
      return value[3] - '\xa0';

    // Pahawh Digits U+16B50 - U+16B59
    if (std::strcmp(value, "\xf0\x96\xad\x90") >= 0 and
        std::strcmp(value, "\xf0\x96\xad\x99") <= 0)
      return value[3] - '\x90';

    // Mathematical Bold Digits U+1D7CE - U+1D7D7
    // Mathematical Double-Struck Digits U+1D7D8 - U+1D7E1
    // Mathematical Sans-Serif Digits U+1D7E2 - U+1D7EB
    // Mathematical Sans-Serif Bold Digits U+1D7EC - U+1D7F5
    // Mathematical Monospace Digits U+1D7F6 - U+1D7FF
    if (std::strcmp(value, "\xf0\x9d\x9f\x8e") >= 0 and
        std::strcmp(value, "\xf0\x9d\x9f\xbf") <= 0)
      return (value[3] - '\x8e') % 10;

    // Adlam Digits U+1E950 - U+1E959
    if (std::strcmp(value, "\xf0\x9e\xa5\x90") >= 0 and
        std::strcmp(value, "\xf0\x9e\xa5\x99") <= 0)
      return value[3] - '\x90';

    break;

  default:
    // Digits U+0030 - U+0039
    if (value[0] >= '\x30' and value[0] <= '\x39')
      return value[0] - '\x30';

    // Arabic-Indic Digits U+0660 - U+0669
    if (std::strcmp(value, "\xd9\xa0") >= 0 and
        std::strcmp(value, "\xd9\xa9") <= 0)
      return value[1] - '\xa0';

    // Extended Arabic-Indic Digits U+06F0-U+06F9
    if (std::strcmp(value, "\xdb\xb0") >= 0 and
        std::strcmp(value, "\xdb\xb9") <= 0)
      return value[1] - '\xb0';

    // NKo Digits U+07C0 - U+07C9
    if (std::strcmp(value, "\xdf\x80") >= 0 and
        std::strcmp(value, "\xdf\x89") <= 0)
      return value[1] - '\x80';

    // Fullwidth Digits U+FF10 - U+FF19
    if (std::strcmp(value, "\xef\xbc\x90") >= 0 and
        std::strcmp(value, "\xef\xbc\x99") <= 0)
      return value[2] - '\x90';

    break;
  }

  return 0;
}*/

/******************************************************************************
 * class cutlet::arg_tokens
 */

/**********************************
 * cutlet::arg_tokens::arg_tokens *
 **********************************/

cutlet::arg_tokens::arg_tokens(const arg_tokens &other)
  : _list(other._list), _it(other._it), _errmsg(other._errmsg) {
}

cutlet::arg_tokens::arg_tokens(const cutlet::list &list,
                               const std::string &errmsg)
  : _list(&list), _it(list.begin()), _errmsg(errmsg) {
}

/*******************************
 * cutlet::arg_tokens::is_more *
 *******************************/

bool cutlet::arg_tokens::is_more() const {
  return (_it != _list->end());
}

/****************************
 * cutlet::arg_tokens::next *
 ****************************/

cutlet::variable::pointer cutlet::arg_tokens::next() {
  ++_it;
  if (_it == _list->end())
    throw std::runtime_error("Expected more arguments.");
  return *_it;
}

/******************************
 * cutlet::arg_tokens::expect *
 ******************************/

bool cutlet::arg_tokens::expect(const std::string &value) const {
  return (cutlet::primative<std::string>(*_it) == value);
}

/******************************
 * cutlet::arg_tokens::permit *
 ******************************/

void cutlet::arg_tokens::permit(const std::string &value) const {
  if (cutlet::primative<std::string>(*_it) != value)
    throw std::runtime_error(std::string("Expected ") + value +
                             " but got " +
                             cutlet::primative<std::string>(*_it)
                             + " instead.");
}

/******************************
 * cutlet::arg_tokens::permit *
 ******************************/

cutlet::variable::pointer cutlet::arg_tokens::get() const {
  return *_it;
}

/***********************************
 * cutlet::arg_tokens::operator ++ *
 ***********************************/

cutlet::arg_tokens &cutlet::arg_tokens::operator ++() {
  ++_it;
  return *this;
}

cutlet::arg_tokens cutlet::arg_tokens::operator ++(int) {
  arg_tokens res(*this);
  ++_it;
  return res;
}
