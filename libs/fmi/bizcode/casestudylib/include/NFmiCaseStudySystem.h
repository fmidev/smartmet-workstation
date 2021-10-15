#pragma once

#include "NFmiProducer.h"
#include "NFmiInfoData.h"
#include "NFmiMetTime.h"
#include "NFmiParam.h"
#include "NFmiCachedRegistryValue.h"

#include <vector>

#include "json_spirit_value.h"
#include "boost/shared_ptr.hpp"

class NFmiHelpDataInfo;
class NFmiInfoOrganizer;
class NFmiQueryInfo;
class NFmiHelpDataInfoSystem;
class CWnd;
class NFmiCaseStudySystem;
class NFmiProducerSystem;
class NFmiDataLoadingInfo;
class NFmiCaseStudySettingsWinRegistry;

class CaseStudyOperationCanceledException
{
public:
	CaseStudyOperationCanceledException(){}
	~CaseStudyOperationCanceledException(){}

};


class NFmiProducerSystemsHolder
{
public:
	NFmiProducerSystemsHolder(void)
	:itsModelProducerSystem(0)
	,itsObsProducerSystem(0)
	,itsSatelImageProducerSystem(0)
	{
	}

	void Reset(void)
	{
		*this = NFmiProducerSystemsHolder();
	}

	NFmiProducerSystem *itsModelProducerSystem; // ei omista, ei tuhoa
	NFmiProducerSystem *itsObsProducerSystem; // ei omista, ei tuhoa
	NFmiProducerSystem *itsSatelImageProducerSystem; // ei omista, ei tuhoa
};

// N‰m‰ CaseStudyDataFile:a koskevat dataosat ovat tallessa Windows rekisteriss‰,
// paitsi jos kyse on CaseStudy dialogin Category tai Producer riveist‰, jolloin niiden arvot
// otetaan lokaali CaseStudyMemory.csmeta tiedostosta.
class NFmiCsDataFileWinReg
{
	int itsCaseStudyDataCount = 0;
	bool fFixedValueForCaseStudyDataCount = false;
	int itsLocalCacheDataCount = 0;
	bool fFixedValueForLocalCacheDataCount = false;
	bool fStore = false;
	bool fProperlyInitialized = false;
public:

	NFmiCsDataFileWinReg();
	NFmiCsDataFileWinReg(int caseStudyDataCount, bool fixedValueForCaseStudyDataCount, int localCacheDataCount, bool fixedValueForLocalCacheDataCount, bool store);

	int CaseStudyDataCount() const { return itsCaseStudyDataCount; }
	bool FixedValueForCaseStudyDataCount() const { return fFixedValueForCaseStudyDataCount; }
	int LocalCacheDataCount() const { return itsLocalCacheDataCount; }
	bool FixedValueForLocalCacheDataCount() const { return fFixedValueForLocalCacheDataCount; }
	bool Store() const { return fStore; }
	bool ProperlyInitialized() const { return fProperlyInitialized; }

	void CaseStudyDataCount(int newValue);
	void FixedValueForCaseStudyDataCount(bool newValue);
	void LocalCacheDataCount(int newValue);
	void FixedValueForLocalCacheDataCount(bool newValue);
	void Store(bool newValue);
	void ProperlyInitialized(bool newValue);

	bool AreDataCountsFixed() const;
	void FixToLastDataOnlyType();
	void DoCheckedAssignment(const NFmiCsDataFileWinReg& other);
	void AdaptFixedSettings(const NFmiCsDataFileWinReg& other);
	bool operator!=(const NFmiCsDataFileWinReg& other) const;
};

class NFmiCaseStudyDataFile
{
public:
	enum DataCategory
	{
		kFmiCategoryError = 0,
		kFmiCategoryModel,
		kFmiCategoryObservation,
		kFmiCategoryAnalyze,
		kFmiCategoryEdited,
		kFmiCategorySatelImage
	};

	NFmiCaseStudyDataFile(void);
	~NFmiCaseStudyDataFile(void);

	void Reset(void);
	bool Init(NFmiHelpDataInfoSystem &theDataInfoSystem, const NFmiHelpDataInfo &theDataInfo, NFmiInfoOrganizer &theInfoOrganizer, const NFmiCsDataFileWinReg& theDataFileWinRegValues, const NFmiCaseStudySystem &theCaseStudySystem);
    void UpdateWithInfo(NFmiInfoOrganizer & theInfoOrganizer, NFmiHelpDataInfoSystem &theDataInfoSystem);
    void Update(const NFmiCaseStudySystem &theCaseStudySystem);
	static json_spirit::Object MakeJsonObject(const NFmiCaseStudyDataFile &theData);

	static int GetModelRunTimeGapInMinutes(NFmiQueryInfo *theInfo, NFmiInfoData::Type theType, NFmiHelpDataInfo *theHelpDataInfo);

	const std::string& Name(void) const {return itsName;}
	void Name(const std::string &newValue) {itsName = newValue;}
	const std::string& HelpDataInfoName(void) const { return itsHelpDataInfoName; }
	void HelpDataInfoName(const std::string& newValue) { itsHelpDataInfoName = newValue; }
	const NFmiProducer& Producer(void) const {return itsProducer;}
	void Producer(const NFmiProducer &newValue) {itsProducer = newValue;}
	const std::string& FileFilter(void) const {return itsFileFilter;}
	std::string& FileFilter(void) {return itsFileFilter;}
	void FileFilter(const std::string &newValue) {itsFileFilter = newValue;}
	const std::string& RelativeStoredFileFilter(void) const {return itsRelativeStoredFileFilter;}
	void RelativeStoredFileFilter(const std::string &newValue) {itsRelativeStoredFileFilter = newValue;}
	const NFmiCsDataFileWinReg& DataFileWinRegValues() const { return itsDataFileWinRegValues; }
	NFmiCsDataFileWinReg& DataFileWinRegValues() { return itsDataFileWinRegValues; }
	void DataFileWinRegValues(const NFmiCsDataFileWinReg& newValues) { itsDataFileWinRegValues = newValues; }
	int DataIntervalInMinutes(void) const {return itsDataIntervalInMinutes;}
	void DataIntervalInMinutes(int newValue) {itsDataIntervalInMinutes = newValue;}
	double SingleFileSize(void) const {return itsSingleFileSize;}
	void SingleFileSize(double newValue) {itsSingleFileSize = newValue;}
	double TotalFileSize(void) const {return itsTotalFileSize;}
	void TotalFileSize(double newValue) {itsTotalFileSize = newValue;}
	double MaxFileSize(void) const {return itsMaxFileSize;}
	void MaxFileSize(double newValue) {itsMaxFileSize = newValue;}
	DataCategory Category(void) const {return itsCategory;}
	void Category(DataCategory newValue) {itsCategory = newValue;}
	bool ImageFile(void) const {return fImageFile;}
	void ImageFile(bool newValue) {fImageFile = newValue;}
	bool StoreLastDataOnly(void) const;
	bool CategoryHeader(void) const {return fCategoryHeader;}
	void CategoryHeader(bool newValue) {fCategoryHeader = newValue;}
	bool ProducerHeader(void) const {return fProducerHeader;}
	void ProducerHeader(bool newValue) {fProducerHeader = newValue;}
	bool OnlyOneData(void) const {return fOnlyOneData;}
	void OnlyOneData(bool newValue) {fOnlyOneData = newValue;}
	bool IsReadOnlyData() const;

	const std::string& ImageAreaStr(void) const {return itsImageAreaStr;}
	void ImageAreaStr(const std::string &newValue) {itsImageAreaStr = newValue;}
	const NFmiParam& ImageParam(void) const {return itsImageParam;}
	void ImageParam(const NFmiParam &newValue) {itsImageParam = newValue;}
	void ParseJsonValue(json_spirit::Value &theValue);
	void AddDataToHelpDataInfoSystem(boost::shared_ptr<NFmiHelpDataInfoSystem> &theHelpDataInfoSystem, const std::string &theBasePath, NFmiHelpDataInfoSystem& theOriginalHelpDataInfoSystem);

	bool NotifyOnLoad(void) const {return fNotifyOnLoad;}
	void NotifyOnLoad(bool newValue) {fNotifyOnLoad = newValue;}
	const std::string& NotificationLabel(void) const {return itsNotificationLabel;}
	void NotificationLabel(const std::string &newValue) {itsNotificationLabel = newValue;}
	const std::string& CustomMenuFolder(void) const {return itsCustomMenuFolder;}
	void CustomMenuFolder(const std::string &newValue) {itsCustomMenuFolder = newValue;}
	int AdditionalArchiveFileCount(void) const {return itsAdditionalArchiveFileCount;}
	void AdditionalArchiveFileCount(int newValue) {itsAdditionalArchiveFileCount = newValue;}
	void InitDataWithStoredSettings(NFmiCaseStudyDataFile &theOriginalDataFile);
	bool DataEnabled(void) const {return fDataEnabled;}
	void DataEnabled(NFmiHelpDataInfoSystem &theDataInfoSystem, bool newValue);
	NFmiInfoData::Type DataType() const { return itsDataType; }
	void DataType(NFmiInfoData::Type newValue) { itsDataType = newValue; }

    bool operator==(const NFmiCaseStudyDataFile &other) const;
    bool operator!=(const NFmiCaseStudyDataFile &other) const;
private:
	double EvaluateTotalDataSize(void);
	void ParseJsonPair(json_spirit::Pair &thePair);

	// Datan nimi, satel-imagen tapauksessa channel, muuten datatyyppi (pinta, painepinta, mallipinta jne.) / filefilter-siivottuna
	std::string itsName;
	// NFmiHelpDataInfo oliolta saatu datan uniikki nimi
	std::string itsHelpDataInfoName;
	NFmiProducer itsProducer;
	std::string itsFileFilter;
	NFmiCsDataFileWinReg itsDataFileWinRegValues;
	std::string itsRelativeStoredFileFilter; // t‰h‰n talletetaan suhteellinen polku, mihin data lopulta talletetaan CaseStudy-rakenteessa, t‰t‰ k‰ytet‰‰n sitten kun dataa k‰ytet‰‰n tulevaisuudessa
	int itsDataIntervalInMinutes; // kuinka usein dataa tuotetaan (malliajo v‰li tai kuvan tuotanto v‰li), t‰m‰n avulla lasketaan arvio itsTotalFileSize-arvoksi
	double itsSingleFileSize; // viimeisimm‰n datatiedoston koko
	double itsTotalFileSize; // lasketaan arvio yhden datan perusteella ja alku/loppu offsettien ja intervallin avulla
	double itsMaxFileSize; // t‰t‰ k‰ytet‰‰n producer- ja category- header infojen yhteydess‰ kertomaan arvio kaikkien mahdollisten datojen yhteis koosta vaikka niit‰ ei olisi valittu talletettavaksi
	DataCategory itsCategory;
	NFmiInfoData::Type itsDataType; // k‰ytet‰‰n priorisoinnissa
	int itsLevelCount; // k‰ytet‰‰n priorisoinnissa
	std::string itsImageAreaStr; // jos kyse on kuva-datasta, pit‰‰ tallettaa myˆs kuvan alue
	NFmiParam itsImageParam; // jos kyseess‰ kuva-data, pit‰‰ tallettaa myˆs parametrin id ja nimi
	bool fImageFile; // jos kyseess‰ on kuva tyyppist‰ dataa, t‰h‰n true, jos queryDataa, laitetaan false
	bool fCategoryHeader;
	bool fProducerHeader;
	bool fOnlyOneData;

	bool fNotifyOnLoad; // Jos datan latauksen yhteydess‰ halutaan tehd‰ ilmoitus, t‰m‰ on true. Oletus arvo on false
	std::string itsNotificationLabel; // Jos notifikaatioon halutaan tietty sanoma, se voidaan antaa t‰h‰n. Defaulttina annetaan tiedoston nimi
	std::string itsCustomMenuFolder; // Jos data halutaan laittaa haluttuun hakemistoon param-popupeissa, tehd‰‰n sellainen asetus helpdatassa t‰ss‰ 
	int itsAdditionalArchiveFileCount;	// defaultti on 0, joitakin datoja (esim. kepa-datoja, joita tuotetaan n. 15-20 per p‰iv‰) , ei normaali asetus riit‰, joten t‰h‰n lis‰t‰‰n v‰h‰n extraa arkistoa varten
	bool fDataEnabled; // T‰m‰ tieto tulee NFmiHelpDataInfo:n IsEnabled-metodista. Eli k‰ytt‰‰kˆ SmartMet kyseist‰ dataa alkuunkaan. Jos t‰m‰ on false, ei sit‰ talletetan dataan vaikka store-asetus olisi true.

};

class NFmiCaseStudyProducerData
{
public:
	NFmiCaseStudyProducerData(void);
	NFmiCaseStudyProducerData(const NFmiProducer &theProducer);
	~NFmiCaseStudyProducerData(void);

	void AddData(const NFmiCaseStudyDataFile &theData);
	void Update(const NFmiCaseStudySystem &theCaseStudySystem);
    void UpdateOnlyOneDataStates();
    void ProducerStore(bool newValue);
	void ProducerEnable(NFmiHelpDataInfoSystem &theDataInfoSystem, bool newValue);
	void ProducerDataCount(int theDataCount, bool theCaseStudyCase);
	std::vector<NFmiCaseStudyDataFile>& FilesData(void) {return itsFilesData;}
	const std::vector<NFmiCaseStudyDataFile>& FilesData(void) const {return itsFilesData;}
	NFmiCaseStudyDataFile& ProducerHeaderInfo(void) {return itsProducerHeaderInfo;}
	const NFmiCaseStudyDataFile& ProducerHeaderInfo(void) const {return itsProducerHeaderInfo;}
	static json_spirit::Object MakeJsonObject(const NFmiCaseStudyProducerData &theData, bool fMakeFullStore);
	void ParseJsonValue(json_spirit::Value &theValue);
	void SetCategory(NFmiCaseStudyDataFile::DataCategory theCategory);
	void AddDataToHelpDataInfoSystem(boost::shared_ptr<NFmiHelpDataInfoSystem> &theHelpDataInfoSystem, const std::string &theBasePath, NFmiHelpDataInfoSystem& theOriginalHelpDataInfoSystem);
	void InitDataWithStoredSettings(NFmiCaseStudyProducerData &theOriginalProducerData);
	long GetProducerIdent() const;

    bool operator==(const NFmiCaseStudyProducerData &other) const;
    bool operator!=(const NFmiCaseStudyProducerData &other) const;
private:
	void ParseJsonPair(json_spirit::Pair &thePair);
	NFmiCaseStudyDataFile* GetDataFile(const std::string &theFileFilter);

	NFmiCaseStudyDataFile itsProducerHeaderInfo;
	std::vector<NFmiCaseStudyDataFile> itsFilesData;
};

class NFmiCaseStudyCategoryData
{
public:
	NFmiCaseStudyCategoryData(void);
	NFmiCaseStudyCategoryData(const std::string &theName, NFmiCaseStudyDataFile::DataCategory theCategory);
	~NFmiCaseStudyCategoryData(void);

	void AddData(NFmiCaseStudyDataFile &theData);
    void UpdateNoProducerData(NFmiHelpDataInfoSystem &theDataInfoSystem, NFmiInfoOrganizer &theInfoOrganizer);
	void Update(const NFmiCaseStudySystem &theCaseStudySystem);
	void Update(unsigned long theProdId, const NFmiCaseStudySystem &theCaseStudySystem);
	void ProducerStore(unsigned long theProdId, bool newValue, const NFmiCaseStudySystem &theCaseStudySystem);
	void CategoryStore(bool newValue, const NFmiCaseStudySystem &theCaseStudySystem);
	void ProducerEnable(NFmiHelpDataInfoSystem &theDataInfoSystem, unsigned long theProdId, bool newValue, const NFmiCaseStudySystem &theCaseStudySystem);
	void CategoryEnable(NFmiHelpDataInfoSystem &theDataInfoSystem, bool newValue, const NFmiCaseStudySystem &theCaseStudySystem);
	void ProducerDataCount(unsigned long theProdId, int theDataCount, const NFmiCaseStudySystem& theCaseStudySystem, bool theCaseStudyCase);
	void CategoryDataCount(int theDataCount, const NFmiCaseStudySystem& theCaseStudySystem, bool theCaseStudyCase);
	std::list<NFmiCaseStudyProducerData>& ProducersData(void) {return itsProducersData;}
	const std::list<NFmiCaseStudyProducerData>& ProducersData(void) const {return itsProducersData;}
	static json_spirit::Object MakeJsonObject(const NFmiCaseStudyCategoryData &theData, bool fMakeFullStore);
	void ParseJsonValue(json_spirit::Value &theValue);

	NFmiCaseStudyDataFile& CategoryHeaderInfo(void) {return itsCategoryHeaderInfo;}
	const NFmiCaseStudyDataFile& CategoryHeaderInfo(void) const {return itsCategoryHeaderInfo;}
	void AddDataToHelpDataInfoSystem(boost::shared_ptr<NFmiHelpDataInfoSystem> &theHelpDataInfoSystem, const std::string &theBasePath, NFmiHelpDataInfoSystem& theOriginalHelpDataInfoSystem);
	void InitDataWithStoredSettings(NFmiCaseStudyCategoryData &theOriginalCategoryData);
	NFmiCaseStudyProducerData* GetProducerData(unsigned long theProdId);
	void PutNoneProducerDataToEndFix();

    bool operator==(const NFmiCaseStudyCategoryData &other) const;
    bool operator!=(const NFmiCaseStudyCategoryData &other) const;
private:
	void ParseJsonPair(json_spirit::Pair &thePair);
    NFmiCaseStudyProducerData RemoveNoProducerData();
    void UpdateOnlyOneDataStates();

	NFmiCaseStudyDataFile itsCategoryHeaderInfo;
	std::list<NFmiCaseStudyProducerData> itsProducersData;
	NFmiCaseStudyDataFile::DataCategory itsParsingCategory; // parsittaessa categori-dataa, otetaan t‰h‰n talteen yleis-kategoria, joka annetaan kaikille datoille 
															// En talleta sit‰ NFmiCaseStudyDataFile:n json-dataan, koska halusin karsia datam‰‰r‰‰, mielest‰ni t‰m‰ on muutenkin turha, kun luetaan datoja
};


class NFmiCaseStudySystem
{
public:
	NFmiCaseStudySystem(void);
	~NFmiCaseStudySystem(void);

	bool Init(NFmiHelpDataInfoSystem &theDataInfoSystem, NFmiInfoOrganizer &theInfoOrganizer, NFmiCaseStudySettingsWinRegistry & theCaseStudySettingsWinRegistry);
	void UpdateValuesBackToWinRegistry(NFmiCaseStudySettingsWinRegistry& theCaseStudySettingsWinRegistry);
    void UpdateNoProducerData(NFmiHelpDataInfoSystem &theDataInfoSystem, NFmiInfoOrganizer &theInfoOrganizer);
    void Update(void);
	void Reset(void); // HUOM! ei saa kutsua konstruktorissa, koska t‰m‰ kutsuu konstruktoria
	void Update(NFmiCaseStudyDataFile::DataCategory theCategory, unsigned long theProdId);
	void ProducerStore(NFmiCaseStudyDataFile::DataCategory theCategory, unsigned long theProdId, bool newValue);
	void CategoryStore(NFmiCaseStudyDataFile::DataCategory theCategory, bool newValue);
	void ProducerEnable(NFmiHelpDataInfoSystem &theDataInfoSystem, NFmiCaseStudyDataFile::DataCategory theCategory, unsigned long theProdId, bool newValue);
	void CategoryEnable(NFmiHelpDataInfoSystem &theDataInfoSystem, NFmiCaseStudyDataFile::DataCategory theCategory, bool newValue);
	void ProducerDataCount(NFmiCaseStudyDataFile::DataCategory theCategory, unsigned long theProdId, int theDataCount, bool theCaseStudyCase);
	void CategoryDataCount(NFmiCaseStudyDataFile::DataCategory theCategory, int theDataCount, bool theCaseStudyCase);
	std::vector<NFmiCaseStudyCategoryData>& CategoriesData(void) {return itsCategoriesData;}
	void FillCaseStudyDialogData(NFmiProducerSystemsHolder &theProducerSystemsHolder);
	std::vector<NFmiCaseStudyDataFile*>& CaseStudyDialogData(void) {return itsCaseStudyDialogData;}
	std::vector<unsigned char>& TreePatternArray(void) {return itsTreePatternArray;}

	const std::string& Name(void) const {return itsName;}
	void Name(const std::string &newValue) {itsName = newValue;}
	const std::string& Info(void) const {return itsInfo;}
	void Info(const std::string &newValue) {itsInfo = newValue;}
	std::string CaseStudyPath(void) const;
	void CaseStudyPath(const std::string &newValue);
	const NFmiMetTime& Time(void) const {return itsTime;}
	void Time(const NFmiMetTime &newValue) {itsTime = newValue;}
    bool ZipFiles(void) const;
    void ZipFiles(bool newValue);
	bool StoreWarningMessages(void) const;
	void StoreWarningMessages(bool newValue);
	bool DeleteTmpFiles(void) const {return fDeleteTmpFiles;}
	void DeleteTmpFiles(bool newValue) {fDeleteTmpFiles = newValue;}
	bool ApproximateOnlyLocalDataSize(void) const {return fApproximateOnlyLocalDataSize;}
	void ApproximateOnlyLocalDataSize(bool newValue) {fApproximateOnlyLocalDataSize = newValue;}
	const std::string& SmartMetLocalCachePath(void) const {return itsSmartMetLocalCachePath;}
	void SmartMetLocalCachePath(const std::string &newValue);
	bool IsPathInLocalCache(const std::string &thePath) const;
	bool DoApproximateDataSize(const std::string &thePath) const;

	static json_spirit::Object MakeJsonObject(NFmiCaseStudySystem &theData, bool fMakeFullStore);
	bool StoreMetaData(CWnd *theParentWindow, std::string &theMetaDataTotalFileNameInOut, bool fMakeFullStore);
    bool AreStoredMetaDataChanged(const NFmiCaseStudySystem &other);
	bool ReadMetaData(const std::string &theFullPathFileName, CWnd *theParentWindow);
	bool MakeCaseStudyData(const std::string &theFullPathMetaDataFileName, CWnd *theParentWindow, CWnd *theCopyWindowPos); // voi heitt‰‰ CaseStudyOperationCanceledException -poikkeuksen!!!
	boost::shared_ptr<NFmiHelpDataInfoSystem> MakeHelpDataInfoSystem(NFmiHelpDataInfoSystem &theOriginalHelpDataInfoSystem, const std::string &theBasePath);
	NFmiCaseStudyDataFile* FindCaseStudyDataFile(const std::string& theUniqueHelpDataInfoName);
	void PutNoneProducerDataToEndFix();

	void SetUpDataLoadinInfoForCaseStudy(NFmiDataLoadingInfo &theDataLoadingInfo, const std::string &theBasePath);
	static std::string MakeBaseDataDirectory(const std::string& theMetaDataFilePath, const std::string& theCaseStudyName);
	static std::string MakeCaseStudyDataHakeDirectory(const std::string& theBaseCaseStudyDataDirectory);

private:
	void AddData(NFmiCaseStudyDataFile &theData);
	void SetProducerName(NFmiProducerSystemsHolder &theProducerSystemsHolder, NFmiCaseStudyProducerData &theProducerData);
	void ParseJsonValue(json_spirit::Value &theValue);
	void ParseJsonPair(json_spirit::Pair &thePair);
	void ParseJsonCategoryArray(json_spirit::Array &theCategories);
	int CalculateProgressDialogCount(void) const;
	void InitDataWithStoredSettings(std::vector<NFmiCaseStudyCategoryData> &theOriginalCategoriesData);
	NFmiCaseStudyCategoryData* GetCategoryData(NFmiCaseStudyDataFile::DataCategory theCategory);
	std::string MakeCaseStudyFilePattern(const std::string &theFilePattern, const std::string &theBasePath, bool fMakeOnlyPath);

	std::string itsName; // talletettavan case-studyn nimi
	std::string itsInfo; // talletettavan case-studyn info
    boost::shared_ptr<CachedRegString> itsCaseStudyPath; // t‰ss‰ on abs polku hakemistoon johon on tarkoitus tehd‰ case study talletuksia
	boost::shared_ptr<CachedRegBool> fZipFiles; // pakataanko tehty case-study data lopuksi vai ei
	boost::shared_ptr<CachedRegBool> fStoreWarningMessages; // Talletetaanko mit‰‰n sanomia (l‰hinn‰ HAKE) CaseStudy dataan
    std::string itsSmartMetLocalCachePath; // polku johon smartmet tallettaa queryDatat, jos data lˆytyy t‰m‰n polun alta, sen koko voidaan aina arvioida
	NFmiMetTime itsTime; // talletetttavan case-studyn ns. sein‰kello aika
	bool fDeleteTmpFiles; // deletoidaanko case-study datat zippauksen j‰lkeen
	bool fApproximateOnlyLocalDataSize; // arvioidaan vain lokaali cachell‰ olevien datojen koko, kaikki muualla olevat tavarat merkit‰‰n 0:ksi
	std::vector<NFmiCaseStudyCategoryData> itsCategoriesData;
	std::vector<NFmiCaseStudyDataFile*> itsCaseStudyDialogData; // t‰h‰n talletetaan lista CSDataFile-olioita, joiden avulla on tarkoitus t‰ytt‰‰ CaseStudy-dialogin Grid Control
																// vektoriin laitetaan vain pointterit, jotta muokkaukset menev‰t perille molempiin datarakenteisiin.
																// HUOM! Ei omista eik‰ tuhoa olioita.
	std::vector<unsigned char> itsTreePatternArray; // t‰t‰ on grid-controlliin laitettavan puurakenteen syvyys rakenne p‰‰taso eli category on 1, producer taso on 2 ja fileData taso on 3
};
