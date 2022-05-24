#pragma once

#include "base/callback.h"
#include "base/command_line.h"
#include "build/build_config.h"

#if defined(OS_MAC)
namespace base {
namespace mac {
class ScopedNSAutoreleasePool;
}
}
#endif

class AppMainParts;
struct MainFunctionParams;

using CreatedMainPartsClosure = base::Callback<std::unique_ptr<AppMainParts>(const MainFunctionParams& params)>;

struct MainFunctionParams {
    explicit MainFunctionParams(const base::CommandLine& cl, const CreatedMainPartsClosure& pc)
        : command_line(cl),
        created_main_parts_closure(pc) {}

    const base::CommandLine& command_line;

    // base::OnceClosure* ui_task = nullptr;
    CreatedMainPartsClosure created_main_parts_closure;
};
