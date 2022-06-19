#!/usr/bin/env python
# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Generate a C++ file containing information on all accepted CT logs."""

import base64
import datetime
import hashlib
import json
import math
import sys


def _write_cpp_header(f):
  f.write("// This file is auto-generated, DO NOT EDIT.\n\n")


def _write_disqualified_log_info_struct_definition(f):
  f.write(
      "// Information related to previously-qualified, but now disqualified,"
      "\n"
      "// CT logs.\n"
      "struct DisqualifiedCTLogInfo {\n"
      "  // The ID of the log (the SHA-256 hash of |log_info.log_key|.\n"
      "  const char log_id[33];\n"
      "  const CTLogInfo log_info;\n"
      "  // The offset from the Unix Epoch of when the log was disqualified."
      "\n"
      "  // SCTs embedded in pre-certificates after this date should not"
      " count\n"
      "  // towards any uniqueness/freshness requirements.\n"
      "  const base::TimeDelta disqualification_date;\n"
      "};\n\n")


def _split_and_hexify_binary_data(bin_data):
  """Pretty-prints, in hex, the given bin_data."""
  if sys.version_info.major == 2:
    hex_data = "".join("\\x%.2x" % ord(c) for c in bin_data)
  else:
    hex_data = "".join("\\x%.2x" % c for c in bin_data)
  # line_width % 4 must be 0 to avoid splitting the hex-encoded data
  # across '\' which will escape the quotation marks.
  line_width = 68
  assert line_width % 4 == 0
  num_splits = int(math.ceil(len(hex_data) / float(line_width)))
  return [
      '"%s"' % hex_data[i * line_width:(i + 1) * line_width]
      for i in range(num_splits)
  ]


def _get_log_ids_array(log_ids, array_name):
  num_logs = len(log_ids)
  log_ids.sort()
  log_id_length = len(log_ids[0]) + 1
  log_id_code = [
      "// The list is sorted.\n",
      "const char %s[][%d] = {\n" % (array_name, log_id_length)
  ]
  for i in range(num_logs):
    split_hex_id = _split_and_hexify_binary_data(log_ids[i])
    s = "    %s" % ("\n    ".join(split_hex_id))
    if (i < num_logs - 1):
      s += ',\n'
    log_id_code.append(s)
  log_id_code.append('};\n\n')
  return log_id_code


def _get_log_ids_for_operator(logs_by_operator, operator_name):
  """Returns a list of Log IDs of logs operated by operator_name."""
  for operator in logs_by_operator:
    if operator["name"] == operator_name:
      return [
          base64.b64decode(log["log_id"])
          for log in operator["logs"]
          if _is_log_once_or_currently_qualified(log)
      ]
  return []


def _is_log_disqualified(log):
  # Disqualified logs are denoted with state="retired"
  assert (len(log.get("state")) == 1)
  log_state = list(log.get("state"))[0]
  return log_state == "retired"


def _escape_c_string(s):
  def _escape_char(c):
    if 32 <= ord(c) <= 126 and c not in '\\"':
      return c
    else:
      return "\\%03o" % ord(c)

  return "".join(_escape_char(c) for c in s)


def _to_loginfo_struct(log):
  """Converts the given log to a CTLogInfo initialization code."""
  log_key = base64.b64decode(log["key"])
  split_hex_key = _split_and_hexify_binary_data(log_key)
  s = "    {"
  s += "\n     ".join(split_hex_key)
  s += ',\n     %d' % (len(log_key))
  s += ',\n     "%s"' % (_escape_c_string(log["description"]))
  s += '}'
  return s


def _get_log_definitions(logs):
  """Returns a list of strings, each is a CTLogInfo definition."""
  return [_to_loginfo_struct(log) for log in logs]


def _timestamp_to_timedelta_since_unixepoch(timestamp):
  dt = datetime.datetime.strptime(timestamp, "%Y-%m-%dT%H:%M:%SZ")
  unix_epoch = datetime.datetime(1970, 1, 1, 0, 0)
  # Both objects in this operation are intended to represent UTC
  # time, although they have a tzinfo of None associated with them.
  return (dt - unix_epoch).total_seconds()


def _to_disqualified_loginfo_struct(log):
  log_id = base64.b64decode(log["log_id"])
  s = "    {"
  s += "\n     ".join(_split_and_hexify_binary_data(log_id))
  s += ",\n"
  s += _to_loginfo_struct(log)
  s += ",\n"
  s += '     base::TimeDelta::FromSeconds(%d)' % (
      _timestamp_to_timedelta_since_unixepoch(
          log["state"]["retired"]["timestamp"]))
  s += '}'
  return s


def _get_disqualified_log_definitions(logs):
  """Returns a list of DisqualifiedCTLogInfo definitions."""
  return [_to_disqualified_loginfo_struct(log) for log in logs]


def _sorted_disqualified_logs(all_logs):
  return sorted([l for l in all_logs if _is_log_disqualified(l)],
                key = lambda l: base64.b64decode(l["log_id"]))


def _write_qualifying_logs_loginfo(f, qualifying_logs):
  f.write("// The set of all presently-qualifying CT logs.\n")
  f.write("const CTLogInfo kCTLogList[] = {\n")
  f.write(",\n".join(_get_log_definitions(qualifying_logs)))
  f.write("\n};\n\n")


def _is_log_once_or_currently_qualified(log):
  assert (len(log.get("state")) == 1)
  return list(log.get("state"))[0] not in ("pending", "rejected")


def generate_cpp_file(input_file, f):
  """Generate a header file of known logs to be included by Chromium."""
  json_log_list = json.load(input_file)
  _write_cpp_header(f)

  # Logs with pending/rejected should not be considered by Chromium
  logs_by_operator = json_log_list["operators"]
  logs = []
  for operator in logs_by_operator:
    for log in operator["logs"]:
      if _is_log_once_or_currently_qualified(log):
        logs.append(log)

  # Write the list of currently-qualifying logs.
  qualifying_logs = [log for log in logs if not _is_log_disqualified(log)]
  _write_qualifying_logs_loginfo(f, qualifying_logs)

  # Write the IDs of all CT Logs operated by Google
  google_log_ids = _get_log_ids_for_operator(logs_by_operator, "Google")
  f.writelines(_get_log_ids_array(google_log_ids, "kGoogleLogIDs"))

  # Write the list of all disqualified logs.
  _write_disqualified_log_info_struct_definition(f)
  f.write("// The set of all disqualified logs, sorted by |log_id|.\n")
  f.write("constexpr DisqualifiedCTLogInfo kDisqualifiedCTLogList[] = {\n")
  f.write(",\n".join(
      _get_disqualified_log_definitions(_sorted_disqualified_logs(logs))))
  f.write("\n};\n")


def main():
  if len(sys.argv) != 3:
    print(("usage: %s in_loglist_json out_header" % sys.argv[0]))
    return 1
  with open(sys.argv[1], "r") as infile, open(sys.argv[2], "w") as outfile:
    generate_cpp_file(infile, outfile)
  return 0


if __name__ == "__main__":
  sys.exit(main())
