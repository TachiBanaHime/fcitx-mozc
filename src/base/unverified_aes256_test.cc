// Copyright 2010-2014, Google Inc.
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

#include "base/unverified_aes256.h"
#include "testing/base/public/googletest.h"
#include "testing/base/public/gunit.h"

namespace mozc {
namespace internal {
namespace {

using ::testing::AssertionFailure;
using ::testing::AssertionResult;
using ::testing::AssertionSuccess;

class TestableUnverifiedAES256 : public UnverifiedAES256 {
 public:
  // Change access rights:
  using UnverifiedAES256::MakeKeySchedule;
  using UnverifiedAES256::SubBytes;
  using UnverifiedAES256::InvSubBytes;
  using UnverifiedAES256::MixColumns;
  using UnverifiedAES256::InvMixColumns;
  using UnverifiedAES256::ShiftRows;
  using UnverifiedAES256::InvShiftRows;
  using UnverifiedAES256::TransformECB;
  using UnverifiedAES256::InverseTransformECB;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(TestableUnverifiedAES256);
};

template<size_t kArraySize>
AssertionResult AssertEqualDataWithFormat(
    const char* expected_expression,
    const char* actual_expression,
    const uint8 (&expected)[kArraySize],
    const uint8 (&actual)[kArraySize]) {
  for (size_t i = 0; i < kArraySize; ++i) {
    if (expected[i] != actual[i]) {
      return AssertionFailure()
          << "Data mismatsh at " << i << " byte."
          << " expected: " << static_cast<uint32>(expected[i])
          << ", actual: " << static_cast<uint32>(actual[i]);
    }
  }
  return AssertionSuccess();
}

#define EXPECT_EQ_ARRAY(expected, actual)  \
    EXPECT_PRED_FORMAT2(AssertEqualDataWithFormat,  expected, actual)

TEST(UnverifiedAES256Test, MakeKeySchedule) {
  // A.3 Expansion of a 256-bit Cipher Key
  // http://csrc.nist.gov/publications/fips/fips197/fips-197.pdf
  const uint8 kKey[UnverifiedAES256::kKeyBytes] = {
    0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe,
    0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
    0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7,
    0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4,
  };
  const uint8 kExpected[UnverifiedAES256::kKeyScheduleBytes] = {
    0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe,
    0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
    0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7,
    0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4,
    0x9b, 0xa3, 0x54, 0x11, 0x8e, 0x69, 0x25, 0xaf,
    0xa5, 0x1a, 0x8b, 0x5f, 0x20, 0x67, 0xfc, 0xde,
    0xa8, 0xb0, 0x9c, 0x1a, 0x93, 0xd1, 0x94, 0xcd,
    0xbe, 0x49, 0x84, 0x6e, 0xb7, 0x5d, 0x5b, 0x9a,
    0xd5, 0x9a, 0xec, 0xb8, 0x5b, 0xf3, 0xc9, 0x17,
    0xfe, 0xe9, 0x42, 0x48, 0xde, 0x8e, 0xbe, 0x96,
    0xb5, 0xa9, 0x32, 0x8a, 0x26, 0x78, 0xa6, 0x47,
    0x98, 0x31, 0x22, 0x29, 0x2f, 0x6c, 0x79, 0xb3,
    0x81, 0x2c, 0x81, 0xad, 0xda, 0xdf, 0x48, 0xba,
    0x24, 0x36, 0x0a, 0xf2, 0xfa, 0xb8, 0xb4, 0x64,
    0x98, 0xc5, 0xbf, 0xc9, 0xbe, 0xbd, 0x19, 0x8e,
    0x26, 0x8c, 0x3b, 0xa7, 0x09, 0xe0, 0x42, 0x14,
    0x68, 0x00, 0x7b, 0xac, 0xb2, 0xdf, 0x33, 0x16,
    0x96, 0xe9, 0x39, 0xe4, 0x6c, 0x51, 0x8d, 0x80,
    0xc8, 0x14, 0xe2, 0x04, 0x76, 0xa9, 0xfb, 0x8a,
    0x50, 0x25, 0xc0, 0x2d, 0x59, 0xc5, 0x82, 0x39,
    0xde, 0x13, 0x69, 0x67, 0x6c, 0xcc, 0x5a, 0x71,
    0xfa, 0x25, 0x63, 0x95, 0x96, 0x74, 0xee, 0x15,
    0x58, 0x86, 0xca, 0x5d, 0x2e, 0x2f, 0x31, 0xd7,
    0x7e, 0x0a, 0xf1, 0xfa, 0x27, 0xcf, 0x73, 0xc3,
    0x74, 0x9c, 0x47, 0xab, 0x18, 0x50, 0x1d, 0xda,
    0xe2, 0x75, 0x7e, 0x4f, 0x74, 0x01, 0x90, 0x5a,
    0xca, 0xfa, 0xaa, 0xe3, 0xe4, 0xd5, 0x9b, 0x34,
    0x9a, 0xdf, 0x6a, 0xce, 0xbd, 0x10, 0x19, 0x0d,
    0xfe, 0x48, 0x90, 0xd1, 0xe6, 0x18, 0x8d, 0x0b,
    0x04, 0x6d, 0xf3, 0x44, 0x70, 0x6c, 0x63, 0x1e,
  };
  uint8 key_schedule[UnverifiedAES256::kKeyScheduleBytes] = {};

  TestableUnverifiedAES256::MakeKeySchedule(kKey, key_schedule);
  EXPECT_EQ_ARRAY(kExpected, key_schedule);
}

TEST(UnverifiedAES256Test, SubBytes) {
  uint8 block[UnverifiedAES256::kBlockBytes] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  };
  const uint8 kExpected[UnverifiedAES256::kBlockBytes] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5,
    0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
  };
  TestableUnverifiedAES256::SubBytes(block);
  EXPECT_EQ_ARRAY(kExpected, block);
}

TEST(UnverifiedAES256Test, InvSubBytes) {
  uint8 block[UnverifiedAES256::kBlockBytes] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  };
  const uint8 kExpected[UnverifiedAES256::kBlockBytes] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38,
    0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
  };
  TestableUnverifiedAES256::InvSubBytes(block);
  EXPECT_EQ_ARRAY(kExpected, block);
}

TEST(UnverifiedAES256Test, MixColumns) {
  uint8 block[UnverifiedAES256::kBlockBytes] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  };
  const uint8 kExpected[UnverifiedAES256::kBlockBytes] = {
    0x02, 0x07, 0x00, 0x05, 0x06, 0x03, 0x04, 0x01,
    0x0a, 0x0f, 0x08, 0x0d, 0x0e, 0x0b, 0x0c, 0x09,
  };
  TestableUnverifiedAES256::MixColumns(block);
  EXPECT_EQ_ARRAY(kExpected, block);
}

TEST(UnverifiedAES256Test, InvMixColumns) {
  uint8 block[UnverifiedAES256::kBlockBytes] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  };
  const uint8 kExpected[UnverifiedAES256::kBlockBytes] = {
    0x0a, 0x0f, 0x08, 0x0d, 0x0e, 0x0b, 0x0c, 0x09,
    0x02, 0x07, 0x00, 0x05, 0x06, 0x03, 0x04, 0x01,
  };
  TestableUnverifiedAES256::InvMixColumns(block);
  EXPECT_EQ_ARRAY(kExpected, block);
}

TEST(UnverifiedAES256Test, ShiftRows) {
  uint8 block[UnverifiedAES256::kBlockBytes] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  };
  const uint8 kExpected[UnverifiedAES256::kBlockBytes] = {
    0x00, 0x05, 0x0a, 0x0f, 0x04, 0x09, 0x0e, 0x03,
    0x08, 0x0d, 0x02, 0x07, 0x0c, 0x01, 0x06, 0x0b,
  };
  TestableUnverifiedAES256::ShiftRows(block);
  EXPECT_EQ_ARRAY(kExpected, block);
}

TEST(UnverifiedAES256Test, TransformECB_Zero) {
  const uint8 kKey[UnverifiedAES256::kKeyBytes] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };
  uint8 key_schedule[UnverifiedAES256::kKeyScheduleBytes] = {};
  TestableUnverifiedAES256::MakeKeySchedule(kKey, key_schedule);

  uint8 block[UnverifiedAES256::kBlockBytes] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };

  // To reproduce this with OpenSSL:
  // $ openssl enc -e -nopad -aes-256-ecb -K 0000000000000000000000000000000000000000000000000000000000000000 -in <(echo "0000: 00000000000000000000000000000000" | xxd -r) -out >(xxd -g 1 -i -c 8)
  const uint8 kExpected[UnverifiedAES256::kBlockBytes] = {
    0xdc, 0x95, 0xc0, 0x78, 0xa2, 0x40, 0x89, 0x89,
    0xad, 0x48, 0xa2, 0x14, 0x92, 0x84, 0x20, 0x87,
  };

  TestableUnverifiedAES256::TransformECB(key_schedule, block);
  EXPECT_EQ_ARRAY(kExpected, block);
}

TEST(UnverifiedAES256Test, InverseTransformECB_Zero) {
  const uint8 kKey[UnverifiedAES256::kKeyBytes] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };
  uint8 key_schedule[UnverifiedAES256::kKeyScheduleBytes] = {};
  TestableUnverifiedAES256::MakeKeySchedule(kKey, key_schedule);

  uint8 block[UnverifiedAES256::kBlockBytes] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };

  // To reproduce this with OpenSSL:
  // $ openssl enc -d -nopad -aes-256-ecb -K 0000000000000000000000000000000000000000000000000000000000000000 -in <(echo "0000: 00000000000000000000000000000000" | xxd -r) -out >(xxd -g 1 -i -c 8)
  const uint8 kExpected[UnverifiedAES256::kBlockBytes] = {
    0x67, 0x67, 0x1c, 0xe1, 0xfa, 0x91, 0xdd, 0xeb,
    0x0f, 0x8f, 0xbb, 0xb3, 0x66, 0xb5, 0x31, 0xb4,
  };
  TestableUnverifiedAES256::InverseTransformECB(key_schedule, block);
  EXPECT_EQ_ARRAY(kExpected, block);
}

TEST(UnverifiedAES256Test, TransformECB) {
  // http://csrc.nist.gov/publications/fips/fips197/fips-197.pdf
  // C.3 AES-256 (Nk=8, Nr=14)
  const uint8 kKey[UnverifiedAES256::kKeyBytes] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  };
  uint8 key_schedule[UnverifiedAES256::kKeyScheduleBytes] = {};
  TestableUnverifiedAES256::MakeKeySchedule(kKey, key_schedule);
  uint8 block[UnverifiedAES256::kBlockBytes] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
  };
  const uint8 kExpected[UnverifiedAES256::kBlockBytes] = {
    0x8e, 0xa2, 0xb7, 0xca, 0x51, 0x67, 0x45, 0xbf,
    0xea, 0xfc, 0x49, 0x90, 0x4b, 0x49, 0x60, 0x89,
  };
  TestableUnverifiedAES256::TransformECB(key_schedule, block);
  EXPECT_EQ_ARRAY(kExpected, block);
}

TEST(UnverifiedAES256Test, InverseTransformECB) {
  // http://csrc.nist.gov/publications/fips/fips197/fips-197.pdf
  // C.3 AES-256 (Nk=8, Nr=14)
  const uint8 kKey[UnverifiedAES256::kKeyBytes] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  };
  uint8 key_schedule[UnverifiedAES256::kKeyScheduleBytes] = {};
  TestableUnverifiedAES256::MakeKeySchedule(kKey, key_schedule);
  uint8 block[UnverifiedAES256::kBlockBytes] = {
    0x8e, 0xa2, 0xb7, 0xca, 0x51, 0x67, 0x45, 0xbf,
    0xea, 0xfc, 0x49, 0x90, 0x4b, 0x49, 0x60, 0x89,
  };
  const uint8 kExpected[UnverifiedAES256::kBlockBytes] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
  };
  TestableUnverifiedAES256::InverseTransformECB(key_schedule, block);
  EXPECT_EQ_ARRAY(kExpected, block);
}

TEST(UnverifiedAES256Test, TransformCBC_Zero) {
  const uint8 kKey[UnverifiedAES256::kKeyBytes] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };
  const uint8 kIV[UnverifiedAES256::kBlockBytes] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };
  const size_t kNumBlocks = 4;

  uint8 block[UnverifiedAES256::kBlockBytes * kNumBlocks] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };

  // To reproduce this with OpenSSL:
  // $ openssl enc -e -nopad -aes-256-cbc -K 0000000000000000000000000000000000000000000000000000000000000000 -iv 00000000000000000000000000000000 -in <(echo -e "0000: 00000000000000000000000000000000 \n0010: 00000000000000000000000000000000 \n0020: 00000000000000000000000000000000 \n0030: 00000000000000000000000000000000" | xxd -r) -out >(xxd -g 1 -i -c 8)
  const uint8 kExpected[UnverifiedAES256::kBlockBytes * kNumBlocks] = {
    0xdc, 0x95, 0xc0, 0x78, 0xa2, 0x40, 0x89, 0x89,
    0xad, 0x48, 0xa2, 0x14, 0x92, 0x84, 0x20, 0x87,
    0x08, 0xc3, 0x74, 0x84, 0x8c, 0x22, 0x82, 0x33,
    0xc2, 0xb3, 0x4f, 0x33, 0x2b, 0xd2, 0xe9, 0xd3,
    0x8b, 0x70, 0xc5, 0x15, 0xa6, 0x66, 0x3d, 0x38,
    0xcd, 0xb8, 0xe6, 0x53, 0x2b, 0x26, 0x64, 0x91,
    0x5d, 0x0d, 0xcc, 0x19, 0x25, 0x80, 0xae, 0xe9,
    0xef, 0x8a, 0x85, 0x68, 0x19, 0x3f, 0x1b, 0x44,
  };

  TestableUnverifiedAES256::TransformCBC(kKey, kIV, block, kNumBlocks);
  EXPECT_EQ_ARRAY(kExpected, block);
}

TEST(UnverifiedAES256Test, InverseTransformCBC_Zero) {
  const uint8 kKey[UnverifiedAES256::kKeyBytes] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };
  const uint8 kIV[UnverifiedAES256::kBlockBytes] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };
  const size_t kNumBlocks = 4;
  uint8 block[UnverifiedAES256::kBlockBytes * kNumBlocks] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };

  // To reproduce this with OpenSSL:
  // $ openssl enc -d -nopad -aes-256-cbc -K 0000000000000000000000000000000000000000000000000000000000000000 -iv 00000000000000000000000000000000 -in <(echo -e "0000: 00000000000000000000000000000000 \n0010: 00000000000000000000000000000000 \n0020: 00000000000000000000000000000000 \n0030: 00000000000000000000000000000000" | xxd -r) -out >(xxd -g 1 -i -c 8)
  const uint8 kExpected[UnverifiedAES256::kBlockBytes * kNumBlocks] = {
    0x67, 0x67, 0x1c, 0xe1, 0xfa, 0x91, 0xdd, 0xeb,
    0x0f, 0x8f, 0xbb, 0xb3, 0x66, 0xb5, 0x31, 0xb4,
    0x67, 0x67, 0x1c, 0xe1, 0xfa, 0x91, 0xdd, 0xeb,
    0x0f, 0x8f, 0xbb, 0xb3, 0x66, 0xb5, 0x31, 0xb4,
    0x67, 0x67, 0x1c, 0xe1, 0xfa, 0x91, 0xdd, 0xeb,
    0x0f, 0x8f, 0xbb, 0xb3, 0x66, 0xb5, 0x31, 0xb4,
    0x67, 0x67, 0x1c, 0xe1, 0xfa, 0x91, 0xdd, 0xeb,
    0x0f, 0x8f, 0xbb, 0xb3, 0x66, 0xb5, 0x31, 0xb4,
  };
  TestableUnverifiedAES256::InverseTransformCBC(kKey, kIV, block, kNumBlocks);
  EXPECT_EQ_ARRAY(kExpected, block);
}

TEST(UnverifiedAES256Test, TransformCBC_NonZero) {
  const uint8 kKey[UnverifiedAES256::kKeyBytes] = {
    0x00, 0xfe, 0x02, 0xfc, 0x04, 0xfa, 0x06, 0xf8,
    0xf7, 0x09, 0xf5, 0x0b, 0xf3, 0x0d, 0xf1, 0x0f,
    0xff, 0x01, 0xfd, 0x03, 0xfb, 0x05, 0xf9, 0x07,
    0x08, 0xf6, 0x0a, 0xf4, 0x0c, 0xf2, 0x0e, 0xf0,
  };
  const uint8 kIV[UnverifiedAES256::kBlockBytes] = {
    0xf0, 0xe1, 0xd2, 0xc3, 0xb4, 0xa5, 0x96, 0x87,
    0x78, 0x69, 0x5a, 0x4b, 0x3c, 0x2d, 0x1e, 0x0f,
  };
  const size_t kNumBlocks = 5;
  uint8 block[UnverifiedAES256::kBlockBytes * kNumBlocks] = {
    0x6f, 0x72, 0x67, 0x61, 0x6e, 0x69, 0x7a, 0x65,
    0x20, 0x74, 0x68, 0x65, 0x20, 0x77, 0x6f, 0x72,
    0x6c, 0x64, 0x27, 0x73, 0x20, 0x69, 0x6e, 0x66,
    0x6f, 0x72, 0x6d, 0x61, 0x74, 0x69, 0x6f, 0x6e,
    0x20, 0x61, 0x6e, 0x64, 0x20, 0x6d, 0x61, 0x6b,
    0x65, 0x20, 0x69, 0x74, 0x20, 0x75, 0x6e, 0x69,
    0x76, 0x65, 0x72, 0x73, 0x61, 0x6c, 0x6c, 0x79,
    0x20, 0x61, 0x63, 0x63, 0x65, 0x73, 0x73, 0x69,
    0x62, 0x6c, 0x65, 0x20, 0x61, 0x6e, 0x64, 0x20,
    0x75, 0x73, 0x65, 0x66, 0x75, 0x6c, 0x00, 0x00,
  };

  // To reproduce this with OpenSSL:
  // $ openssl enc -e -nopad -aes-256-cbc -K 00fe02fc04fa06f8f709f50bf30df10fff01fd03fb05f90708f60af40cf20ef0 -iv f0e1d2c3b4a5968778695a4b3c2d1e0f -in <(echo -e "0000: 6f7267616e697a652074686520776f72 \n0010: 6c64277320696e666f726d6174696f6e \n0020: 20616e64206d616b6520697420756e69 \n0030: 76657273616c6c792061636365737369 \n0040: 626c6520616e642075736566756c0000" | xxd -r) -out >(xxd -g 1 -i -c 8)
  const uint8 kExpected[UnverifiedAES256::kBlockBytes * kNumBlocks] = {
    0xcd, 0xba, 0x6b, 0xc8, 0xec, 0x35, 0x36, 0x8c,
    0x2f, 0x1a, 0x99, 0x57, 0x44, 0xdd, 0x3a, 0x51,
    0x63, 0xd0, 0x02, 0xd8, 0xab, 0x72, 0x5f, 0xb7,
    0x5a, 0x83, 0x0c, 0xf9, 0x37, 0x9d, 0x80, 0xeb,
    0x6d, 0x1a, 0xb2, 0xdd, 0xca, 0x9a, 0xee, 0x32,
    0xea, 0x91, 0xc5, 0xab, 0xc2, 0x85, 0xb2, 0xa5,
    0x30, 0x4e, 0xe5, 0x1a, 0xc9, 0xa2, 0x36, 0xa0,
    0x64, 0x42, 0x21, 0x7e, 0xf6, 0xae, 0x5c, 0x7e,
    0x84, 0xff, 0x63, 0x7f, 0x78, 0x3a, 0x4e, 0xab,
    0x0a, 0x5e, 0xb2, 0x49, 0x74, 0xcd, 0x0f, 0x85,
  };
  TestableUnverifiedAES256::TransformCBC(kKey, kIV, block, kNumBlocks);
  EXPECT_EQ_ARRAY(kExpected, block);
}

TEST(UnverifiedAES256Test, InverseTransformCBC_NonZero) {
  const uint8 kKey[UnverifiedAES256::kKeyBytes] = {
    0x00, 0xfe, 0x02, 0xfc, 0x04, 0xfa, 0x06, 0xf8,
    0xf7, 0x09, 0xf5, 0x0b, 0xf3, 0x0d, 0xf1, 0x0f,
    0xff, 0x01, 0xfd, 0x03, 0xfb, 0x05, 0xf9, 0x07,
    0x08, 0xf6, 0x0a, 0xf4, 0x0c, 0xf2, 0x0e, 0xf0,
  };
  const uint8 kIV[UnverifiedAES256::kBlockBytes] = {
    0xf0, 0xe1, 0xd2, 0xc3, 0xb4, 0xa5, 0x96, 0x87,
    0x78, 0x69, 0x5a, 0x4b, 0x3c, 0x2d, 0x1e, 0x0f,
  };
  const size_t kNumBlocks = 5;

  uint8 block[UnverifiedAES256::kBlockBytes * kNumBlocks] = {
    0xcd, 0xba, 0x6b, 0xc8, 0xec, 0x35, 0x36, 0x8c,
    0x2f, 0x1a, 0x99, 0x57, 0x44, 0xdd, 0x3a, 0x51,
    0x63, 0xd0, 0x02, 0xd8, 0xab, 0x72, 0x5f, 0xb7,
    0x5a, 0x83, 0x0c, 0xf9, 0x37, 0x9d, 0x80, 0xeb,
    0x6d, 0x1a, 0xb2, 0xdd, 0xca, 0x9a, 0xee, 0x32,
    0xea, 0x91, 0xc5, 0xab, 0xc2, 0x85, 0xb2, 0xa5,
    0x30, 0x4e, 0xe5, 0x1a, 0xc9, 0xa2, 0x36, 0xa0,
    0x64, 0x42, 0x21, 0x7e, 0xf6, 0xae, 0x5c, 0x7e,
    0x84, 0xff, 0x63, 0x7f, 0x78, 0x3a, 0x4e, 0xab,
    0x0a, 0x5e, 0xb2, 0x49, 0x74, 0xcd, 0x0f, 0x85,
  };

  // To reproduce this with OpenSSL:
  // $ openssl enc -d -nopad -aes-256-cbc -K 00fe02fc04fa06f8f709f50bf30df10fff01fd03fb05f90708f60af40cf20ef0 -iv f0e1d2c3b4a5968778695a4b3c2d1e0f -in <(echo -e "0000: cdba6bc8ec35368c2f1a995744dd3a51 \n0010: 63d002d8ab725fb75a830cf9379d80eb \n0020: 6d1ab2ddca9aee32ea91c5abc285b2a5 \n0030: 304ee51ac9a236a06442217ef6ae5c7e \n0040: 84ff637f783a4eab0a5eb24974cd0f85" | xxd -r) -out >(xxd -g 1 -i -c 8)
  const uint8 kExpected[UnverifiedAES256::kBlockBytes * kNumBlocks] = {
    0x6f, 0x72, 0x67, 0x61, 0x6e, 0x69, 0x7a, 0x65,
    0x20, 0x74, 0x68, 0x65, 0x20, 0x77, 0x6f, 0x72,
    0x6c, 0x64, 0x27, 0x73, 0x20, 0x69, 0x6e, 0x66,
    0x6f, 0x72, 0x6d, 0x61, 0x74, 0x69, 0x6f, 0x6e,
    0x20, 0x61, 0x6e, 0x64, 0x20, 0x6d, 0x61, 0x6b,
    0x65, 0x20, 0x69, 0x74, 0x20, 0x75, 0x6e, 0x69,
    0x76, 0x65, 0x72, 0x73, 0x61, 0x6c, 0x6c, 0x79,
    0x20, 0x61, 0x63, 0x63, 0x65, 0x73, 0x73, 0x69,
    0x62, 0x6c, 0x65, 0x20, 0x61, 0x6e, 0x64, 0x20,
    0x75, 0x73, 0x65, 0x66, 0x75, 0x6c, 0x00, 0x00,
  };
  TestableUnverifiedAES256::InverseTransformCBC(kKey, kIV, block, kNumBlocks);
  EXPECT_EQ_ARRAY(kExpected, block);
}

// TODO(yukawa): Add more tests based on well-known test vectors.

}  // namespace
}  // namespace internal
}  // namespace mozc
