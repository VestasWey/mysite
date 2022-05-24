#pragma once

#include "base/files/file_path.h"
#include "base/strings/string16.h"

enum class ApplicationMode {
    DefaultMode
};

class AppContext {
public:
    static AppContext* Current();

    void Init();

    const base::FilePath& GetExecutablePath() const;

    const base::FilePath& GetExecutableDirectory() const;

    // Main directory is the one that contains main dll.
    const base::FilePath& GetMainDirectory() const;

    const base::FilePath& GetMainDLLPath() const;

    const std::string& GetExecutableVersion() const;

    unsigned short GetExecutableBuildNumber() const;

    ApplicationMode GetApplicationMode() const;

    bool InApplicationMode(ApplicationMode mode) const;

private:
    AppContext();

    ~AppContext();

    static base::FilePath GuessMainDLLPath(const base::FilePath& exe_dir, const std::string& exe_ver);

private:
    bool inited_ = false;
    ApplicationMode app_mode_ = ApplicationMode::DefaultMode;
    base::FilePath exe_path_;
    base::FilePath exe_dir_;
    std::string exe_ver_;
    base::FilePath main_dir_;
    base::FilePath main_dll_path_;

    DISALLOW_COPY_AND_ASSIGN(AppContext);
};
