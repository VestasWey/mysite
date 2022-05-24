#include "secret/app_secret.h"


APP_LIB_EXPORT AppSecret* AppSecretEntry()
{
    return GetAppSecret();
}
