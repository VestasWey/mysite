// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_COMMAND_UPDATER_DELEGATE_H_
#define CHROME_BROWSER_COMMAND_UPDATER_DELEGATE_H_


// like content\public\notification\notification_source.h:NotificationSource
// Do not declare a CommandParamsDetails directly, use either CommandParams or
// NoCommandParams().
class CommandParamsDetails {
public:
    CommandParamsDetails()
    {
    }

    CommandParamsDetails(const CommandParamsDetails& other)
        : payload_ptr_(other.payload_ptr_)
    {
    }

    virtual ~CommandParamsDetails()
    {
    }

    uintptr_t map_key() const
    {
        return reinterpret_cast<uintptr_t>(payload_ptr_);
    }

    bool operator!=(const CommandParamsDetails& other) const
    {
        return payload_ptr_ != other.payload_ptr_;
    }

    bool operator==(const CommandParamsDetails& other) const
    {
        return payload_ptr_ == other.payload_ptr_;
    }

protected:
    explicit CommandParamsDetails(const void* ptr)
        : payload_ptr_(ptr)
    {
    }

    const void* payload_ptr_ = nullptr;
};

template <class T>
class CommandParams : public CommandParamsDetails
{
public:
    explicit CommandParams(T* ptr)
        : CommandParamsDetails(ptr)
    {
    }

    CommandParams(const CommandParamsDetails& other)
        : CommandParamsDetails(other)
    {
    }

    T* operator->() const
    {
        return ptr();
    }

    T* ptr() const
    {
        // The casts here allow this to compile with both T = Foo and T = const Foo.
        return static_cast<T*>(const_cast<void*>(payload_ptr_));
    }
};

inline CommandParams<void> EmptyCommandParams()
{
    return CommandParams<void>(nullptr);
}

inline CommandParams<void> DummyCommandParams()
{
    return CommandParams<void>(reinterpret_cast<void*>(1));
}


// Implement this interface so that your object can execute commands when
// needed.
class CommandUpdaterDelegate {
 public:
  // Performs the action associated with the command with the specified ID and
  // using the given disposition.
  virtual void ExecuteCommandWithParams(
      int id,
      const CommandParamsDetails& params) = 0;

 protected:
  virtual ~CommandUpdaterDelegate() {}
};

#endif  // CHROME_BROWSER_COMMAND_UPDATER_DELEGATE_H_
