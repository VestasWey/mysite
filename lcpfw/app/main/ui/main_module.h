#pragma once


class MainWindow;
class ScopedKeepAlive;
class StartupMainCreator;

class MainModule
    : public base::RefCounted<MainModule>
{
public:
    MainWindow* main_window() const;

private:
    MainModule();
    ~MainModule();

    void Init();
    void Uninit();

    void ActuallyShutdown();
    void InitMainWindow();
    void OnMainWindowDestroyed();

private:
    friend class StartupMainCreator;
    friend class base::RefCounted<MainModule>;

    MainWindow* main_window_ = nullptr;
    std::unique_ptr<ScopedKeepAlive> keep_alive_;

    DISALLOW_COPY_AND_ASSIGN(MainModule);
};
