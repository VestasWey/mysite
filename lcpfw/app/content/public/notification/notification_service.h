// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file describes a central switchboard for notifications that might
// happen in various parts of the application, and allows users to register
// observers for various classes of events that they're interested in.

#ifndef CONTENT_PUBLIC_BROWSER_NOTIFICATION_SERVICE_H_
#define CONTENT_PUBLIC_BROWSER_NOTIFICATION_SERVICE_H_

#include "content/common/content_export.h"
#include "content/public/notification/notification_details.h"
#include "content/public/notification/notification_source.h"

namespace content {

class CONTENT_EXPORT NotificationService {
 public:
  // Returns the NotificationService object for the current thread, or nullptr
  // if none.
  static NotificationService* current();

  static NotificationService* Create();

  virtual ~NotificationService() {}

  // Synchronously posts a notification to all interested observers.
  // Source is a reference to a NotificationSource object representing
  // the object originating the notification (can be
  // NotificationService::AllSources(), in which case
  // only observers interested in all sources will be notified).
  // Details is a reference to an object containing additional data about
  // the notification.  If no additional data is needed, NoDetails() is used.
  // There is no particular order in which the observers will be notified.
  virtual void Notify(int type,
                      const NotificationSource& source,
                      const NotificationDetails& details) = 0;

  // Returns a NotificationSource that represents all notification sources
  // (for the purpose of registering an observer for events from all sources).
  static Source<void> AllSources() { return Source<void>(nullptr); }

  // Returns a NotificationDetails object that represents a lack of details
  // associated with a notification.  (This is effectively a null pointer.)
  static Details<void> NoDetails() { return Details<void>(nullptr); }
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_NOTIFICATION_SERVICE_H_
