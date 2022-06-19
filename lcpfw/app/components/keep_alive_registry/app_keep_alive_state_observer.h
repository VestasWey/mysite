// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

class KeepAliveStateObserver {
 public:
  virtual void OnKeepAliveStateChanged(bool is_keeping_alive) = 0;
  virtual void OnKeepAliveRestartStateChanged(bool can_restart) = 0;

 protected:
  virtual ~KeepAliveStateObserver() {}
};

