#pragma once


//#include "content/public/browser/notification_service.h"

#include "main/app_thread_impl.h"


class AppSubThread
    : public AppThreadImpl
{
public:
    explicit AppSubThread(AppThread::ID identifier);
    virtual ~AppSubThread();

protected:
    virtual void Init() override;
    virtual void CleanUp() override;

private:
    void IOThreadPreCleanUp();

    //std::unique_ptr<content::NotificationService> notification_service_;

    DISALLOW_COPY_AND_ASSIGN(AppSubThread);
};

