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

#ifndef MOZC_CONVERTER_SEGMENTS_H_
#define MOZC_CONVERTER_SEGMENTS_H_

#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <vector>

#include "base/freelist.h"
#include "base/number_util.h"
#include "base/port.h"
#include "converter/lattice.h"
#include "absl/strings/string_view.h"

namespace mozc {

class Segment final {
 public:
  enum SegmentType {
    FREE,            // FULL automatic conversion.
    FIXED_BOUNDARY,  // cannot consist of multiple segments.
    FIXED_VALUE,     // cannot consist of multiple segments.
                     // and result is also fixed
    SUBMITTED,       // submitted node
    HISTORY          // history node. It is hidden from user.
  };

  struct Candidate {
    enum Attribute {
      DEFAULT_ATTRIBUTE = 0,
      // this was the best candidate before learning
      BEST_CANDIDATE = 1 << 0,
      // this candidate was reranked by user
      RERANKED = 1 << 1,
      // don't save it in history
      NO_HISTORY_LEARNING = 1 << 2,
      // don't save it in suggestion
      NO_SUGGEST_LEARNING = 1 << 3,
      // NO_HISTORY_LEARNING | NO_SUGGEST_LEARNING
      NO_LEARNING = (1 << 2 | 1 << 3),
      // learn it with left/right context
      CONTEXT_SENSITIVE = 1 << 4,
      // has "did you mean"
      SPELLING_CORRECTION = 1 << 5,
      // No need to have full/half width expansion
      NO_VARIANTS_EXPANSION = 1 << 6,
      // No need to have extra descriptions
      NO_EXTRA_DESCRIPTION = 1 << 7,
      // was generated by real-time conversion
      REALTIME_CONVERSION = 1 << 8,
      // contains tokens in user dictionary.
      USER_DICTIONARY = 1 << 9,
      // command candidate. e.g., incognito mode.
      COMMAND_CANDIDATE = 1 << 10,
      // key characters are consumed partially.
      // Consumed size is |consumed_key_size|.
      // If not set, all the key characters are consumed.
      PARTIALLY_KEY_CONSUMED = 1 << 11,
      // Typing correction candidate.
      // - Special description should be shown when the candidate is created
      //   by a dictionary predictor.
      // - No description should be shown when the candidate is loaded from
      //   history.
      // - Otherwise following unexpected behavior can be observed.
      //   1. Type "やんしょん" and submit "マンション" (annotated with "補正").
      //   2. Type "まんしょん".
      //   3. "マンション" (annotated with "補正") is shown as a candidate
      //      regardless of a user's correct typing.
      TYPING_CORRECTION = 1 << 12,
      // Auto partial suggestion candidate.
      // - Special description should be shown when the candidate is created
      //   by a dictionary predictor.
      // - No description should be shown when the candidate is loaded from
      //   history.
      AUTO_PARTIAL_SUGGESTION = 1 << 13,
      // Predicted from user prediction history.
      USER_HISTORY_PREDICTION = 1 << 14,
      // Contains suffix dictionary.
      SUFFIX_DICTIONARY = 1 << 15,
    };

    enum Command {
      DEFAULT_COMMAND = 0,
      ENABLE_INCOGNITO_MODE,      // enables "incognito mode".
      DISABLE_INCOGNITO_MODE,     // disables "incognito mode".
      ENABLE_PRESENTATION_MODE,   // enables "presentation mode".
      DISABLE_PRESENTATION_MODE,  // disables "presentation mode".
    };

    // Bit field indicating candidate source info.
    // This should be used for usage stats.
    // TODO(mozc-team): Move Attribute fields for source info
    // to SourceInfo.
    enum SourceInfo {
      SOURCE_INFO_NONE = 0,
      // Attributes for zero query suggestion.
      // These are used for usage stats.
      // For DICTIONARY_PREDICTOR_ZERO_QUERY_XX, XX stands for the
      // types defined at zero_query_list.h.
      DICTIONARY_PREDICTOR_ZERO_QUERY_NONE = 1 << 0,
      DICTIONARY_PREDICTOR_ZERO_QUERY_NUMBER_SUFFIX = 1 << 1,
      DICTIONARY_PREDICTOR_ZERO_QUERY_EMOTICON = 1 << 2,
      DICTIONARY_PREDICTOR_ZERO_QUERY_EMOJI = 1 << 3,
      DICTIONARY_PREDICTOR_ZERO_QUERY_BIGRAM = 1 << 4,
      DICTIONARY_PREDICTOR_ZERO_QUERY_SUFFIX = 1 << 5,

      USER_HISTORY_PREDICTOR = 1 << 6,
    };

    std::string key;    // reading
    std::string value;  // surface form
    std::string content_key;
    std::string content_value;

    size_t consumed_key_size = 0;

    // Meta information
    std::string prefix;
    std::string suffix;
    // Description including description type and message
    std::string description;

    // Usage ID
    int32_t usage_id = 0;
    // Title of the usage containing basic form of this candidate.
    std::string usage_title;
    // Content of the usage.
    std::string usage_description;

    // Context "sensitive" candidate cost.
    // Taking adjacent words/nodes into consideration.
    // Basically, candidate is sorted by this cost.
    int32_t cost = 0;
    // Context "free" candidate cost
    // NOT taking adjacent words/nodes into consideration.
    int32_t wcost = 0;
    // (cost without transition cost between left/right boundaries)
    // Cost of only transitions (cost without word cost adjacent context)
    int32_t structure_cost = 0;

    // lid of left-most node
    uint16_t lid = 0;
    // rid of right-most node
    uint16_t rid = 0;

    // Attributes of this candidate. Can set multiple attributes
    // defined in enum |Attribute|.
    uint32_t attributes = 0;

    // Candidate's source info which will be used for usage stats.
    uint32_t source_info = SOURCE_INFO_NONE;

    // Candidate style. This is not a bit-field.
    // The style is defined in enum |Style|.
    NumberUtil::NumberString::Style style =
        NumberUtil::NumberString::DEFAULT_STYLE;

    // Command of this candidate. This is not a bit-field.
    // The style is defined in enum |Command|.
    Command command = DEFAULT_COMMAND;

    // Boundary information for realtime conversion.  This will be set only for
    // realtime conversion result candidates.  Each element is the encoded
    // lengths of key, value, content key and content value.
    std::vector<uint32_t> inner_segment_boundary;

#ifndef NDEBUG
    std::string log;
#endif  // NDEBUG

    static bool EncodeLengths(size_t key_len, size_t value_len,
                              size_t content_key_len, size_t content_value_len,
                              uint32_t *result);

    // This function ignores error, so be careful when using this.
    static uint32_t EncodeLengths(size_t key_len, size_t value_len,
                                  size_t content_key_len,
                                  size_t content_value_len) {
      uint32_t result;
      EncodeLengths(key_len, value_len, content_key_len, content_value_len,
                    &result);
      return result;
    }

    // Inserts a new element to |inner_segment_boundary|.  If one of four
    // lengths is longer than 255, this method returns false.
    bool PushBackInnerSegmentBoundary(size_t key_len, size_t value_len,
                                      size_t content_key_len,
                                      size_t content_value_len);

    // Iterates inner segments.  Usage example:
    // for (InnerSegmentIterator iter(&cand); !iter.Done(); iter.Next()) {
    //   absl::string_view s = iter.GetContentKey();
    //   ...
    // }
    class InnerSegmentIterator final {
     public:
      explicit InnerSegmentIterator(const Candidate *candidate)
          : candidate_(candidate),
            key_offset_(candidate->key.data()),
            value_offset_(candidate->value.data()),
            index_(0) {}

      bool Done() const {
        return index_ == candidate_->inner_segment_boundary.size();
      }

      void Next();
      absl::string_view GetKey() const;
      absl::string_view GetValue() const;
      absl::string_view GetContentKey() const;
      absl::string_view GetContentValue() const;

     private:
      const Candidate *candidate_;
      const char *key_offset_;
      const char *value_offset_;
      size_t index_;
    };

    void Init();

    // Returns functional key.
    // functional_key =
    // key.substr(content_key.size(), key.size() - content_key.size());
    absl::string_view functional_key() const;

    // Returns functional value.
    // functional_value =
    // value.substr(content_value.size(), value.size() - content_value.size());
    absl::string_view functional_value() const;

    bool IsValid() const;
    std::string DebugString() const;
  };

  Segment();

  Segment(const Segment &x);
  Segment &operator=(const Segment &x);

  ~Segment();

  SegmentType segment_type() const;
  void set_segment_type(const SegmentType &segment_type);

  const std::string &key() const;
  void set_key(absl::string_view key);

  // check if the specified index is valid or not.
  bool is_valid_index(int i) const;

  // Candidate manupluations
  // getter
  const Candidate &candidate(int i) const;

  // setter
  Candidate *mutable_candidate(int i);

  // push and insert candidates
  Candidate *push_front_candidate();
  Candidate *push_back_candidate();
  Candidate *add_candidate();  // alias of push_back_candidate()
  Candidate *insert_candidate(int i);
  void insert_candidate(int i, std::unique_ptr<Candidate> candidate);
  void insert_candidates(int i,
                         std::vector<std::unique_ptr<Candidate>> &&candidates);

  // get size of candidates
  size_t candidates_size() const;

  // erase candidate
  void pop_front_candidate();
  void pop_back_candidate();
  void erase_candidate(int i);
  void erase_candidates(int i, size_t size);

  // erase all candidates
  // do not erase meta candidates
  void clear_candidates();

  // meta candidates
  // TODO(toshiyuki): Integrate meta candidates to candidate and delete these
  size_t meta_candidates_size() const;
  void clear_meta_candidates();
  const std::vector<Candidate> &meta_candidates() const;
  std::vector<Candidate> *mutable_meta_candidates();
  const Candidate &meta_candidate(size_t i) const;
  Candidate *mutable_meta_candidate(size_t i);
  Candidate *add_meta_candidate();

  // move old_idx-th-candidate to new_index
  void move_candidate(int old_idx, int new_idx);

  void Clear();

  // Keep clear() method as other modules are still using the old method
  void clear() { Clear(); }

  std::string DebugString() const;

  // For debug. Candidate words removed through conversion process.
  std::vector<Candidate> removed_candidates_for_debug_;

 private:
  SegmentType segment_type_;
  // Note that |key_| is shorter than usual when partial suggestion is
  // performed.
  // For example if the preedit text is "しれ|ません", there is only a segment
  // whose |key_| is "しれ".
  // There is no way to detect by using only a segment whether this segment is
  // for partial suggestion or not.
  // You should detect that by using both Composer and Segments.
  std::string key_;
  std::deque<Candidate *> candidates_;
  std::vector<Candidate> meta_candidates_;
  std::vector<std::unique_ptr<Candidate>> pool_;
};

// Segments is basically an array of Segment.
// Note that there are two types of Segment
// a) History Segment (SegmentType == HISTORY OR SUBMITTED)
//    Segments user entered just before the transacton
// b) Conversion Segment
//    Current segments user inputs
//
// Array of segment is represented as an array as follows
// segments_array[] = {HS_0,HS_1,...HS_N, CS0, CS1, CS2...}
//
// * segment(i) and mutable_segment(int i)
//  access segment regardless of History/Conversion distinctions
//
// * history_segment(i) and mutable_history_segment(i)
//  access only History Segment
//
// conversion_segment(i) and mutable_conversion_segment(i)
//  access only Conversion Segment
//  segment(i + history_segments_size()) == conversion_segment(i)
class Segments final {
 public:
  enum RequestType {
    CONVERSION,          // normal conversion
    REVERSE_CONVERSION,  // reverse conversion
    PREDICTION,          // show prediction with user tab key
    SUGGESTION,          // show prediction automatically
    PARTIAL_PREDICTION,  // show prediction using the text before cursor
    PARTIAL_SUGGESTION,  // show suggestion using the text before cursor
  };

  // Client of segments can remember any string which can be used
  // to revert the last Finish operation.
  // "id" can be used for identifying the purpose of the key;
  struct RevertEntry {
    enum RevertEntryType {
      CREATE_ENTRY,
      UPDATE_ENTRY,
    };
    uint16_t revert_entry_type = 0;
    // UserHitoryPredictor uses '1' for now.
    // Do not use duplicate keys.
    uint16_t id = 0;
    uint32_t timestamp = 0;
    std::string key;
  };

  Segments();

  Segments(const Segments &x);
  Segments &operator=(const Segments &x);

  ~Segments();

  RequestType request_type() const;
  void set_request_type(RequestType request_type);

  // getter
  const Segment &segment(size_t i) const;
  const Segment &conversion_segment(size_t i) const;
  const Segment &history_segment(size_t i) const;

  // setter
  Segment *mutable_segment(size_t i);
  Segment *mutable_conversion_segment(size_t i);
  Segment *mutable_history_segment(size_t i);

  // push and insert segments
  Segment *push_front_segment();
  Segment *push_back_segment();
  Segment *add_segment();  // alias of push_back_segment()
  Segment *insert_segment(size_t i);

  // get size of segments
  size_t segments_size() const;
  size_t history_segments_size() const;
  size_t conversion_segments_size() const;

  // erase segment
  void pop_front_segment();
  void pop_back_segment();
  void erase_segment(size_t i);
  void erase_segments(size_t i, size_t size);

  // erase all segments
  void clear_history_segments();
  void clear_conversion_segments();
  void clear_segments();

  void set_max_history_segments_size(size_t max_history_segments_size);
  size_t max_history_segments_size() const;

  bool resized() const;
  void set_resized(bool resized);

  // clear segments
  void Clear();

  // Dump Segments structure
  std::string DebugString() const;

  // Revert entries
  void clear_revert_entries();
  size_t revert_entries_size() const;
  RevertEntry *push_back_revert_entry();
  const RevertEntry &revert_entry(size_t i) const;
  RevertEntry *mutable_revert_entry(size_t i);

  // setter
  Lattice *mutable_cached_lattice();

 private:
  size_t max_history_segments_size_;
  bool resized_;

  RequestType request_type_;
  ObjectPool<Segment> pool_;
  std::deque<Segment *> segments_;
  std::vector<RevertEntry> revert_entries_;
  std::unique_ptr<Lattice> cached_lattice_;
};

}  // namespace mozc

#endif  // MOZC_CONVERTER_SEGMENTS_H_
