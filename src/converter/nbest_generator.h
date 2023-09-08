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

#ifndef MOZC_CONVERTER_NBEST_GENERATOR_H_
#define MOZC_CONVERTER_NBEST_GENERATOR_H_

#include <cstdint>
#include <string>
#include <vector>

#include "base/container/freelist.h"
#include "converter/candidate_filter.h"
#include "converter/connector.h"
#include "converter/lattice.h"
#include "converter/node.h"
#include "converter/segmenter.h"
#include "converter/segments.h"
#include "dictionary/pos_matcher.h"
#include "dictionary/suppression_dictionary.h"
#include "prediction/suggestion_filter.h"
#include "request/conversion_request.h"

#undef STRICT  // minwindef.h has the definition.

namespace mozc {

class NBestGenerator {
 public:
  enum BoundaryCheckMode {
    // Boundary check mode.
    // For the case like;
    //   Candidate edge:      |  candidate  |
    //   Nodes:        |Node A|Node B|Node C|Node D|

    // For normal conversion.
    //  Candidate boundary is strictly same as inner boundary.
    // A-B: Should be the boundary
    // B-C: Should not be the boundary
    // C-D: Should be the boundary
    STRICT = 0,

    // For resegmented segment.
    //  Check mid point only.
    // A-B: Don't care
    // B-C: Should not be the boundary
    // C-D: Don't care
    ONLY_MID,

    // For Realtime conversion ("私の名前は中野です").
    //  Check only for candidate edge.
    // A-B: Should be the boundary
    // B-C: Don't care
    // C-D: Should be the boundary
    ONLY_EDGE,
  };

  // Try to enumerate N-best results between begin_node and end_node.
  NBestGenerator(
      const dictionary::SuppressionDictionary *suppression_dictionary,
      const Segmenter *segmenter, const Connector &connector,
      const dictionary::PosMatcher *pos_matcher, const Lattice *lattice,
      const SuggestionFilter &suggestion_filter);
  NBestGenerator(const NBestGenerator &) = delete;
  NBestGenerator &operator=(const NBestGenerator &) = delete;
  ~NBestGenerator() = default;

  // Reset the iterator status.
  void Reset(const Node *begin_node, const Node *end_node,
             BoundaryCheckMode mode);

  // Set candidates.
  void SetCandidates(const ConversionRequest &request,
                     const std::string &original_key, size_t expand_size,
                     Segment *segment);

 private:
  enum BoundaryCheckResult {
    VALID = 0,
    VALID_WEAK_CONNECTED,  // Valid but should get penalty.
    INVALID,
  };

  struct QueueElement {
    const Node *node;
    const QueueElement *next;
    // f(x) = h(x) + g(x): cost function for A* search
    int32_t fx;
    // g(x): current cost
    // After the search, |gx| should contain the candidates' cost.
    // Please refer to the comment in NBestGenerator::Next() of .cc file
    // for more detail on the candidates' cost.
    int32_t gx;
    // transition cost part of g(x).
    // Do not take the transition costs to edge nodes.
    int32_t structure_gx;
    int32_t w_gx;

    static bool Comparator(const QueueElement *q1, const QueueElement *q2);
  };

  // This is just a priority_queue of const QueueElement*, but supports
  // more operations in addition to std::priority_queue.
  class Agenda {
   public:
    Agenda() = default;
    Agenda(const Agenda &) = delete;
    Agenda &operator=(const Agenda &) = delete;
    ~Agenda() = default;

    const QueueElement *Top() const { return priority_queue_.front(); }
    bool IsEmpty() const { return priority_queue_.empty(); }
    void Clear() { priority_queue_.clear(); }
    void Reserve(int size) { priority_queue_.reserve(size); }

    void Push(const QueueElement *element);
    void Pop();

   private:
    std::vector<const QueueElement *> priority_queue_;
  };

  // Iterator:
  // Can obtain N-best results by calling Next() in sequence.
  bool Next(const ConversionRequest &request, const std::string &original_key,
            Segment::Candidate *candidate);

  int InsertTopResult(const ConversionRequest &request,
                      const std::string &original_key,
                      Segment::Candidate *candidate);

  void MakeCandidate(Segment::Candidate *candidate, int32_t cost,
                     int32_t structure_cost, int32_t wcost,
                     const std::vector<const Node *> &nodes) const;

  // Helper function for Next(). Checks node boundary conditions.
  BoundaryCheckResult BoundaryCheck(const Node *lnode, const Node *rnode,
                                    bool is_edge) const;
  // Helper functions for BoundaryCheck.
  BoundaryCheckResult CheckStrict(const Node *lnode, const Node *rnode,
                                  bool is_edge) const;
  BoundaryCheckResult CheckOnlyMid(const Node *lnode, const Node *rnode,
                                   bool is_edge) const;
  BoundaryCheckResult CheckOnlyEdge(const Node *lnode, const Node *rnode,
                                    bool is_edge) const;

  int GetTransitionCost(const Node *lnode, const Node *rnode) const;

  // Create queue element from freelist
  const QueueElement *CreateNewElement(const Node *node,
                                       const QueueElement *next, int32_t fx,
                                       int32_t gx, int32_t structure_gx,
                                       int32_t w_gx);

  // References to relevant modules.
  const dictionary::SuppressionDictionary *suppression_dictionary_;
  const Segmenter *segmenter_;
  const Connector &connector_;
  const dictionary::PosMatcher *pos_matcher_;
  const Lattice *lattice_;

  const Node *begin_node_ = nullptr;
  const Node *end_node_ = nullptr;

  Agenda agenda_;
  FreeList<QueueElement> freelist_;
  std::vector<const Node *> top_nodes_;
  converter::CandidateFilter filter_;
  bool viterbi_result_checked_ = false;
  BoundaryCheckMode check_mode_ = STRICT;

#ifdef MOZC_CANDIDATE_DEBUG
  std::vector<Segment::Candidate> bad_candidates_;
#endif  // MOZC_CANDIDATE_DEBUG
};

}  // namespace mozc

#endif  // MOZC_CONVERTER_NBEST_GENERATOR_H_
