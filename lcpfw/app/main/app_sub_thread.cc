#include "app_sub_thread.h"

#ifdef OS_WIN
#include "base/win/scoped_com_initializer.h"
#endif

#include "base/threading/thread_restrictions.h"
#include "build/build_config.h"

//#include "content/browser/notification_service_impl.h"


AppSubThread::AppSubThread(AppThread::ID identifier)
    : AppThreadImpl(identifier)
{
#ifdef OS_WIN
    init_com_with_mta(true);
#endif
}

AppSubThread::~AppSubThread()
{
    Stop();
}

void AppSubThread::Init()
{
    //notification_service_.reset(new content::NotificationServiceImpl());

    AppThreadImpl::Init();

    if (AppThread::CurrentlyOn(AppThread::IO))
    {
        // Though this thread is called the "IO" thread, it actually just routes
        // messages around; it shouldn't be allowed to perform any blocking disk
        // I/O.
        base::ThreadRestrictions::SetIOAllowed(false);
        base::ThreadRestrictions::DisallowWaiting();
    }
}

void AppSubThread::CleanUp()
{
    if (app_thread_id() == AppThread::IO)
    {
        IOThreadPreCleanUp();
    }

    AppThreadImpl::CleanUp();

    //notification_service_.reset();
}

void AppSubThread::IOThreadPreCleanUp()
{
}

