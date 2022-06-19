#!/usr/bin/env python
# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
import argparse
import sys
import os
import subprocess
import glob
import shutil


FILES_TO_SYNC = [
    'README.md',
    'check_protocol_compatibility.py',
    'code_generator.py',
    'concatenate_protocols.py',
    'convert_protocol_to_json.py',
    'crdtp/README.md',
    'crdtp/cbor.cc',
    'crdtp/cbor.h',
    'crdtp/cbor_test.cc',
    'crdtp/dispatch.h',
    'crdtp/dispatch.cc',
    'crdtp/dispatch_test.cc',
    'crdtp/error_support.h',
    'crdtp/error_support.cc',
    'crdtp/error_support_test.cc',
    'crdtp/export.h',
    'crdtp/find_by_first.h',
    'crdtp/find_by_first_test.cc',
    'crdtp/frontend_channel.h',
    'crdtp/maybe_test.cc',
    'crdtp/json.cc',
    'crdtp/json.h',
    'crdtp/json_platform.h',
    'crdtp/json_test.cc',
    'crdtp/maybe.h',
    'crdtp/parser_handler.h',
    'crdtp/protocol_core.cc',
    'crdtp/protocol_core.h',
    'crdtp/protocol_core_test.cc',
    'crdtp/serializable.h',
    'crdtp/serializable.cc',
    'crdtp/serializable_test.cc',
    'crdtp/serializer_traits.h',
    'crdtp/serializer_traits_test.cc',
    'crdtp/span.h',
    'crdtp/span.cc',
    'crdtp/span_test.cc',
    'crdtp/status.cc',
    'crdtp/status.h',
    'crdtp/status_test_support.cc',
    'crdtp/status_test_support.h',
    'crdtp/status_test.cc',
    'crdtp/test_platform.cc',
    'crdtp/test_platform.h',
    'crdtp/transcode.cc',
    'inspector_protocol.gni',
    'inspector_protocol.gypi',
    'lib/*',
    'pdl.py',
    'templates/*',
]


def RunCmd(cmd):
  p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
  (stdoutdata, stderrdata) = p.communicate()
  if p.returncode != 0:
    raise Exception('%s: exit status %d', str(cmd), p.returncode)
  return stdoutdata


def CheckRepoIsClean(path):
  os.chdir(path)  # As a side effect this also checks for existence of the dir.
  # If path isn't a git repo, this will throw and exception.
  # And if it is a git repo and 'git status' has anything interesting to say,
  # then it's not clean (uncommitted files etc.)
  if len(RunCmd(['git', 'status', '--porcelain'])) != 0:
    raise Exception('%s is not a clean git repo (run git status)' % path)
  if not path.endswith('/src'):
    raise Exception('%s does not end with /src' % path)


def CheckRepoIsNotAtMasterBranch(path):
  os.chdir(path)
  stdout = RunCmd(['git', 'rev-parse', '--abbrev-ref', 'HEAD']).strip()
  if stdout == 'master':
    raise Exception('%s is at master branch - refusing to copy there.' % path)


def CheckRepoIsChromiumCheckout(path):
  os.chdir(path)
  if (RunCmd(['git', 'config', '--get', 'remote.origin.url']).strip() !=
      'https://chromium.googlesource.com/chromium/src.git'):
    raise Exception('%s is not a proper Chromium checkout.' % path)


def CheckRepoIsInspectorProtocolCheckout(path):
  os.chdir(path)
  if (RunCmd(['git', 'config', '--get', 'remote.origin.url']).strip() !=
      'https://chromium.googlesource.com/deps/inspector_protocol.git'):
    raise Exception('%s is not a proper inspector_protocol checkout.' % path)


def FindFilesToSyncIn(path):
  files = []
  for f in FILES_TO_SYNC:
    files += glob.glob(os.path.join(path, f))
  files = [os.path.relpath(f, path) for f in files]
  return files


def FilesAreEqual(path1, path2):
  # We check for permissions (useful for executable scripts) and contents.
  return (os.stat(path1).st_mode == os.stat(path2).st_mode and
          open(path1).read() == open(path2).read())


def GetHeadRevision(path):
  os.chdir(path)
  return RunCmd(['git', 'rev-parse', 'HEAD'])


def main(argv):
  parser = argparse.ArgumentParser(description=(
      "Rolls the inspector_protocol project (upstream) into Chromium's "
      "third_party (downstream)."))
  parser.add_argument("--ip_src_upstream",
                      help="The inspector_protocol (upstream) tree.",
                      default="~/ip/src")
  parser.add_argument("--chromium_src_downstream",
                      help="The Chromium src tree.",
                      default="~/chromium/src")
  parser.add_argument('--reverse', dest='reverse', action='store_true',
                      help=("Whether to roll the opposite direction, from "
                            "Chromium (downstream) to inspector_protocol "
                            "(upstream)."))
  parser.set_defaults(reverse=False)
  parser.add_argument('--force', dest='force', action='store_true',
                      help=("Whether to carry out the modifications "
                            "in the destination tree."))
  parser.set_defaults(force=False)

  args = parser.parse_args(argv)
  upstream = os.path.normpath(os.path.expanduser(args.ip_src_upstream))
  downstream = os.path.normpath(os.path.expanduser(
      args.chromium_src_downstream))
  CheckRepoIsClean(upstream)
  CheckRepoIsClean(downstream)
  CheckRepoIsInspectorProtocolCheckout(upstream)
  CheckRepoIsChromiumCheckout(downstream)
  # Check that the destination Git repo isn't at the master branch - it's
  # generally a bad idea to check into the master branch, so we catch this
  # common pilot error here early.
  if args.reverse:
    CheckRepoIsNotAtMasterBranch(upstream)
    src_dir = os.path.join(downstream, 'third_party/inspector_protocol')
    dest_dir = upstream
  else:
    CheckRepoIsNotAtMasterBranch(downstream)
    src_dir = upstream
    dest_dir = os.path.join(downstream, 'third_party/inspector_protocol')
  print('Rolling %s into %s ...' % (src_dir, dest_dir))
  src_files = set(FindFilesToSyncIn(src_dir))
  dest_files = set(FindFilesToSyncIn(dest_dir))
  to_add = [f for f in src_files if f not in dest_files]
  to_delete = [f for f in dest_files if f not in src_files]
  to_copy = [f for f in src_files
             if (f in dest_files and not FilesAreEqual(
                 os.path.join(src_dir, f), os.path.join(dest_dir, f)))]
  print('To add: %s' % to_add)
  print('To delete: %s' % to_delete)
  print('To copy: %s' % to_copy)
  if not to_add and not to_delete and not to_copy:
    print('Nothing to do. You\'re good.')
    sys.exit(0)
  if not args.force:
    print('Rerun with --force if you wish the modifications to be done.')
    sys.exit(1)
  print('You said --force ... as you wish, modifying the destination.')
  for f in to_add + to_copy:
    shutil.copyfile(os.path.join(src_dir, f), os.path.join(dest_dir, f))
    shutil.copymode(os.path.join(src_dir, f), os.path.join(dest_dir, f))
  for f in to_delete:
    os.unlink(os.path.join(dest_dir, f))
  if not args.reverse:
    head_revision = GetHeadRevision(upstream)
    lines = open(os.path.join(dest_dir, 'README.chromium')).readlines()
    f = open(os.path.join(dest_dir, 'README.chromium'), 'w')
    for line in lines:
      if line.startswith('Revision: '):
        f.write('Revision: %s' % head_revision)
      else:
        f.write(line)
    f.close()


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
