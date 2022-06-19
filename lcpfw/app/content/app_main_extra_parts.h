#pragma once

// Interface class for Parts owned by ChromeBrowserMainParts.
// The default implementation for all methods is empty.

// Most of these map to content::BrowserMainParts methods. This interface is
// separate to allow stages to be further subdivided for Chrome specific
// initialization stages (e.g. browser process init, profile init).

// While ChromeBrowserMainParts are platform-specific,
// ChromeBrowserMainExtraParts are used to do further initialization for various
// Chrome toolkits (e.g., GTK, VIEWS, ASH, AURA, etc.; see
// ChromeContentBrowserClient::CreateBrowserMainParts()).

class AppMainExtraParts
{
public:
    virtual ~AppMainExtraParts() {}

    // EarlyInitialization methods.
    virtual void PreEarlyInitialization() {}
    virtual void PostEarlyInitialization() {}

    // ToolkitInitialized methods.
    virtual void ToolkitInitialized() {}

    // MainMessageLoopStart methods.
    virtual void PreMainMessageLoopStart() {}
    virtual void PostMainMessageLoopStart() {}

    // MainMessageLoopRun methods.
    virtual void PreCreateThreads() {}
    virtual void PostCreateThreads() {}
    virtual void PreProfileInit() {}
    virtual void PostProfileInit() {}
    virtual void PreAppStart() {}
    virtual void PostAppStart() {}
    virtual void PreMainMessageLoopRun() {}
    virtual void PostMainMessageLoopRun() {}
};

