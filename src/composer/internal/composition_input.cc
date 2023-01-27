// Copyright 2010-2021, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "composer/internal/composition_input.h"

#include <string>

#include "base/logging.h"
#include "base/util.h"

namespace mozc {
namespace composer {

bool CompositionInput::Init(const commands::KeyEvent &key_event,
                            bool is_new_input) {
  if (key_event.has_key_code()) {
    Util::Ucs4ToUtf8(key_event.key_code(), &raw_);
  } else if (key_event.has_key_string()) {
    raw_ = key_event.key_string();
  } else {
    LOG(WARNING) << "input is empty";
    return false;
  }

  if (key_event.has_key_string()) {
    set_conversion(key_event.key_string());
  }
  if (!key_event.probable_key_event().empty()) {
    set_probable_key_events(key_event.probable_key_event());
  }
  set_is_new_input(is_new_input);
  return true;
}

void CompositionInput::InitFromRaw(const std::string &raw, bool is_new_input) {
  set_raw(raw);
  set_is_new_input(is_new_input);
}

void CompositionInput::InitFromRawAndConv(const std::string &raw,
                                          const std::string &conversion,
                                          bool is_new_input) {
  set_raw(raw);
  set_conversion(conversion);
  set_is_new_input(is_new_input);
}

void CompositionInput::Clear() {
  raw_.clear();
  conversion_.clear();
  probable_key_events_.Clear();
  is_new_input_ = false;
}

bool CompositionInput::Empty() const {
  return raw().empty() && conversion().empty();
}

const std::string &CompositionInput::conversion() const {
  return conversion_;
}

void CompositionInput::clear_conversion() {
  conversion_.clear();
}

void CompositionInput::set_conversion(const std::string &conversion) {
  conversion_ = conversion;
}

}  // namespace composer
}  // namespace mozc
