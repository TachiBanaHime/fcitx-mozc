# -*- coding: utf-8 -*-
# Copyright 2010-2021, Google Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""Generate a C++ include file for testing data from TSV files."""

import argparse
import logging
import sys


def escape(string):
  return ', '.join('0x%02x' % char for char in string.encode('utf-8'))


def convert_tsv(tsv, outfile):
  """Loads a TSV file and returns an entry of TestCase."""
  for line in tsv:
    line = line.rstrip()
    if not line or line.startswith('#'):
      continue

    fields = line.split('\t')
    if len(fields) == 3:
      label = fields[0]
      expected = fields[1]
      query = fields[2]
    elif len(fields) < 6:
      logging.warning('invalid row format: %s', line)
      continue
    else:
      label = fields[0]
      expected = fields[4]
      query = fields[5]

    outfile.write('  // {{%s}, {%s}, {%s}},\n' % (label, expected, query))
    outfile.write(
        '  TestCase{"%s", "%s", "%s"},\n'
        % (escape(label), escape(expected), escape(query))
    )
  tsv.close()


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument(
      'inputs', nargs='+', type=argparse.FileType(mode='r', encoding='utf-8')
  )
  parser.add_argument(
      '--output',
      type=argparse.FileType(mode='w', encoding='utf-8'),
      default=sys.stdout,
  )
  args = parser.parse_args()
  logging.basicConfig(level=logging.INFO, encoding='utf-8')

  args.output.write('''// Automatically generated by mozc
constexpr TestCase kTestCases[] = {
''')

  for tsv in args.inputs:
    convert_tsv(tsv, args.output)

  args.output.write('};\n')


if __name__ == '__main__':
  main()
