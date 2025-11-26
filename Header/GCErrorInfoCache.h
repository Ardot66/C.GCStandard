#ifndef __GC_ERROR_INFO_CACHE__
#define __GC_ERROR_INFO_CACHE__

#include "GCList.h"
#include <pthread.h>

ListDefine(ErrorInfo, GCInternalListErrorInfo);
typedef struct
{
    GCInternalListErrorInfo ErrorInfos;
    pthread_mutex_t Mutex;
} ErrorInfoCache;

ErrorInfoCache *ErrorInfoCacheCreate();
void ErrorInfoCacheFree(ErrorInfoCache *cache);
void ErrorInfoCachePrint(ErrorInfoCache *cache);

#endif