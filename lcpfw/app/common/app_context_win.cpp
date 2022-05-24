#include "common/app_context.h"

#include <Windows.h>

#include <memory>

#include "base/file_version_info.h"
#include "base/version.h"
#include "base/files/file_util.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_handle.h"

//#include "config/app_config.h"

#include "common/app_constants.h"

namespace{

// Indicates whether a file can be opened using the same flags that LoadLibrary() uses to open modules.
bool ModuleCanBeLoaded(const base::FilePath& module_path)
{
    base::win::ScopedHandle module(CreateFileW(module_path.value().c_str(),
                                               GENERIC_READ,
                                               FILE_SHARE_READ,
                                               nullptr,
                                               OPEN_EXISTING,
                                               0,
                                               nullptr));
    return module.IsValid();
}

}


base::FilePath AppContext::GuessMainDLLPath(const base::FilePath& exe_dir, const std::string& exe_ver)
{
    auto main_dll_path = exe_dir.Append(lcpfw::kAppMainDll);
    if (ModuleCanBeLoaded(main_dll_path)) {
        return main_dll_path;
    }

    main_dll_path = exe_dir.AppendASCII(exe_ver).Append(lcpfw::kAppMainDll);
    DCHECK(ModuleCanBeLoaded(main_dll_path));

    return main_dll_path;
}
