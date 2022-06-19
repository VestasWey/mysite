#pragma once

#include "content/public/notification/notification_types.h"

namespace lcpfw
{
    enum NotificationType
    {
        NOTIFICATION_APP_START = content::NOTIFICATION_CONTENT_END + 1,

        NOTIFICATION_APP_ACTIVE,
        NOTIFICATION_APP_EXIT,
        
        NOTIFICATION_APP_END,
    };
}
