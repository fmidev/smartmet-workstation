#pragma once

/*
* Tähän tietorakenteeseen laitetaan kaikki tieto, mitä esim. kolme eri 
* CFmiQueryDataCacheLoaderThread:ia tarvitsee.
* Mm. threadi-prioriteetti, ja tiedostokoko rajat ja ajo-semaphore.
*/

#include "stdafx.h"
#include <string>

class CSemaphore;

class CFmiCacheLoaderData
{
public:
	CFmiCacheLoaderData(void);
	~CFmiCacheLoaderData(void);

	int itsThreadPriority; // prioriteetit 1, 2 ja 3. 1. on pienille (ja tärkeille) datoille, 2. on keskisuurille 
							// ja 3. on isoille (ja vähemmän tärkeille)
	double itsMinDataSizeInMB;
	double itsMaxDataSizeInMB;
	CSemaphore *itsThreadRunning; // tämän avulla yritetään lopettaan jatkuvasti pyörivä working thread 'siististi'
	std::string itsThreadName; // tässä on threadin nimi esim. "thread #1"
	int itsStartUpWaitTimeInMS;
};

