// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/common/multi_process_lock.h"

#include "base/logging.h"
#include "base/mac/scoped_cftyperef.h"
#include "base/sys_string_conversions.h"

class MultiProcessLockMac : public MultiProcessLock {
 public:
  explicit MultiProcessLockMac(const std::string& name) : name_(name) { }

  virtual ~MultiProcessLockMac() {
    if (port_ != NULL) {
      Unlock();
    }
  }

  virtual bool TryLock() {
    if (port_ != NULL) {
      DLOG(ERROR) << "MultiProcessLock is already locked - " << name_;
      return true;
    }

    if (name_.length() > MULTI_PROCESS_LOCK_NAME_MAX_LEN) {
      LOG(ERROR) << "Socket name too long (" << name_.length()
                 << " > " << MULTI_PROCESS_LOCK_NAME_MAX_LEN << ") - " << name_;
      return false;
    }

    CFStringRef cf_name(base::SysUTF8ToCFStringRef(name_));
    base::mac::ScopedCFTypeRef<CFStringRef> scoped_cf_name(cf_name);
    port_.reset(CFMessagePortCreateLocal(NULL, cf_name, NULL, NULL, NULL));
    return port_ != NULL;
  }

  virtual void Unlock() {
    if (port_ == NULL) {
      DLOG(ERROR) << "Over-unlocked MultiProcessLock - " << name_;
      return;
    }
    port_.reset();
  }

 private:
  std::string name_;
  base::mac::ScopedCFTypeRef<CFMessagePortRef> port_;
  DISALLOW_COPY_AND_ASSIGN(MultiProcessLockMac);
};

MultiProcessLock* MultiProcessLock::Create(const std::string &name) {
  return new MultiProcessLockMac(name);
}
