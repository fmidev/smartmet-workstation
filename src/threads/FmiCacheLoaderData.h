#pragma once

/*
* T�h�n tietorakenteeseen laitetaan kaikki tieto, mit� esim. kolme eri
* CFmiQueryDataCacheLoaderThread:ia tarvitsee.
* Mm. threadi-prioriteetti, ja tiedostokoko rajat ja ajo-semaphore.
*/

#include <string>

#ifndef UNIX
#include "stdafx.h"

class CSemaphore;
#endif // UNIX

class CFmiCacheLoaderData
{
public:
	CFmiCacheLoaderData(void);
	~CFmiCacheLoaderData(void);

	int itsThreadPriority; // prioriteetit 1, 2 ja 3. 1. on pienille (ja t�rkeille) datoille, 2. on keskisuurille
							// ja 3. on isoille (ja v�hemm�n t�rkeille)
	double itsMinDataSizeInMB;
	double itsMaxDataSizeInMB;
#ifndef UNIX
	CSemaphore *itsThreadRunning; // t�m�n avulla yritet��n lopettaan jatkuvasti py�riv� working thread 'siististi'
#endif // UNIX
	std::string itsThreadName; // t�ss� on threadin nimi esim. "thread #1"
	int itsStartUpWaitTimeInMS;
};

