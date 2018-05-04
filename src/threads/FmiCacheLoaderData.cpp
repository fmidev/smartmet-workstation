
#include "FmiCacheLoaderData.h"
#include "afxmt.h"

CFmiCacheLoaderData::CFmiCacheLoaderData(void)
:itsThreadPriority(0)
,itsMinDataSizeInMB(0)
,itsMaxDataSizeInMB(0)
,itsThreadRunning(new CSemaphore())
,itsThreadName()
{
}

CFmiCacheLoaderData::~CFmiCacheLoaderData(void)
{
	delete itsThreadRunning;
}
