#include "common/app_context.h"

#import <Foundation/Foundation.h>
#include <sys/syslimits.h>
#include <mach-o/dyld.h>

#include <memory>

#include "base/files/file_util.h"
#include "base/file_version_info.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/version.h"

#include "config/app_config.h"

#include "public/common/app_constants.h"


namespace
{
    // Implementation adapted from Chromium's base/mac/foundation_util.mm
    bool UncachedAmIBundled() {
      return [[[NSBundle mainBundle] bundlePath] hasSuffix:@".app"];
    }

    bool AmIBundled() {
      static bool am_i_bundled = UncachedAmIBundled();
      return am_i_bundled;
    }

    bool ModuleCanBeLoaded(const base::FilePath& module_path)
    {
        return base::PathExists(module_path);
    }
}


base::FilePath AppContext::GuessMainDLLPath(const base::FilePath& exe_dir, const std::string& exe_ver)
{
    // 先exe当前目录找，找不到就从Bundle的“Content/Frameworks/Versions/版本号目录”找

    auto main_dll_path = exe_dir.Append(lcpfw::kAppMainDll);
    if (ModuleCanBeLoaded(main_dll_path))
    {
        return main_dll_path;
    }

    if (AmIBundled())
    {
        main_dll_path = exe_dir.DirName().Append("Frameworks/Versions").Append(exe_ver).Append(lcpfw::kAppMainDll);
        if (ModuleCanBeLoaded(main_dll_path))
        {
            return main_dll_path;
        }
    }
    
    NOTREACHED() << "main dll not found.";
    return {};
}
