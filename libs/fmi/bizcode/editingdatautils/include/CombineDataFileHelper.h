#pragma once
#include <string>
#include <functional>
#include <ctime>

class NFmiQueryData;

using LoggingFunction = std::function<void(const std::string&)>;

class CombineDataFileHelper
{
	const std::string temporaryDataFileNameExtension_ = "_TMP_FILE_DELETE_THIS_";
	// Esim. D:\SmartMet\wrk\data\partial_data\tutka_suomi\*_tutka_suomi.sqd
	std::string partialDataFileFilter_;
	// Esim. D:\SmartMet\wrk\data\cache\tutka_suomi\*_tutka_suomi.sqd
	std::string combinedDataFileFilter_;
	// Esim. D:\SmartMet\wrk\data\cache\tutka_suomi\*_tutka_suomi.sqd_TMP_FILE_DELETE_THIS_
	std::string temporaryDataFileFilter_;
	int maxTimeSteps_ = 0;
	bool doRebuildCheck_ = false;
	// Esim. 202102121114 (YYYYMMDDHHmm lokaaliajasta)
	std::string fileNameTimeStamp_;
	LoggingFunction& loggingFunction_;
	// Jos löytyy tmp-tiedosto, joka blokkaa uuden datan luonnin, käsitellään raportointia sen mukaan.
	mutable bool temporaryFileIsBlocking_ = false;
public:
	CombineDataFileHelper(const std::string& partialDataFileFilter, const std::string& combinedDataFileFilter, int maxTimeSteps, bool doRebuildCheck, LoggingFunction& loggingFunction);

	void doDataCombinationWork();
	static bool checkIfDataCombinationIsNeeded(const std::string& partialDataFileFilter, const std::string& combinedDataFileFilter, std::time_t* lastCheckedPartialDataFileTimeInOut);
private:
	void tryCleaningUpTemporaryFiles() const;
	void doFinalTemporaryFileCleaning(const std::string& temporaryFileFilter, const std::string& basePath) const;
	bool isDataCombinationNeeded(std::time_t *lastCheckedPartialDataFileTimeInOut = nullptr) const;
	bool tryToStoreCombinedDataToFile(NFmiQueryData& queryData);
	void initTemporaryDataFileFilter();
	std::string makeFileTimeStamp();
	void logMessage(const std::string& message) const;
};

