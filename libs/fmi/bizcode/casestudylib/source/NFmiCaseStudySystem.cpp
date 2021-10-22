#include "stdafx.h"
#include "NFmiCaseStudySystem.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiDrawParam.h"
#include "NFmiFileSystem.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiProducerSystem.h"
#include "NFmiFileString.h"
#include "CShellFileOp.h"
#include "NFmiAreaFactory.h"
#include "NFmiDataLoadingInfo.h"
#include "CtrlViewFunctions.h"
#include "NFmiMacroParamfunctions.h"
#include "NFmiPathUtils.h"
#include "NFmiApplicationWinRegistry.h"
#include "NFmiQueryData.h"
#include "NFmiQueryDataUtil.h"

#include "boost/shared_ptr.hpp"
#include <boost/filesystem/operations.hpp>
#include "json_spirit_writer.h"
#include "json_spirit_reader.h"
#include "json_spirit_writer_options.h"

#ifdef _MSC_VER
#pragma warning( disable : 4503 ) // t‰m‰ est‰‰ varoituksen joka tulee VC++ 2012 k‰‰nt‰j‰ll‰, kun jonkun boost-luokan nimi merkkein‰ ylitt‰‰ jonkun rajan
#endif

static const std::string gJsonName_Name = "Name";
static const std::string gJsonName_Info = "Info";
static const std::string gJsonName_Path = "Path";
static const std::string gJsonName_Time = "Time";
static const std::string gJsonName_ZipFiles = "ZipFiles";
static const std::string gJsonName_DeleteTmpFiles = "DeleteTmpFiles";
static const std::string gJsonName_DataArray = "DataArray";
static const std::string gJsonName_CategoryHeader = "CategoryHeader";
static const std::string gJsonName_Category = "Category";
static const std::string gJsonName_IsCategoryHeader = "IsCategoryHeader";
static const std::string gJsonName_CategoryDataArray = "CategoryDataArray";

static const std::string gJsonName_ProducerName = "ProducerName";
static const std::string gJsonName_ProducerId = "ProducerId";
static const std::string gJsonName_FileFilter = "FileFilter";
static const std::string gJsonName_RelativeStoredFileFilter = "RelativeStoredFileFilter";
static const std::string gJsonName_StartOffsetInMinutes_legacy = "StartOffsetInMinutes";
static const std::string gJsonName_EndOffsetInMinutes_legacy = "EndOffsetInMinutes";
static const std::string gJsonName_CaseStudyDataCount = "CaseStudyDataCount";
static const std::string gJsonName_LocalCacheDataCount = "LocalCacheDataCount";
static const std::string gJsonName_DataType = "DataType";
static const std::string gJsonName_Store = "Store";
static const std::string gJsonName_ImageFile = "ImageFile";
static const std::string gJsonName_StoreLastDataOnly = "StoreLastDataOnly";
static const std::string gJsonName_ProducerHeader = "ProducerHeader";
static const std::string gJsonName_IsProducerHeader = "IsProducerHeader";
static const std::string gJsonName_OnlyOneData = "OnlyOneData";
static const std::string gJsonName_ImageAreaStr = "ImageAreaStr";
static const std::string gJsonName_ImageParamName = "ImageParamName";
static const std::string gJsonName_ImageParamId = "ImageParamId";
static const std::string gJsonName_ProducerDataArray = "ProducerDataArray";

static const std::string gJsonName_NotifyOnLoad = "NotifyOnLoad";
static const std::string gJsonName_NotificationLabel = "NotificationLabel";
static const std::string gJsonName_CustomMenuFolder = "CustomMenuFolder";
static const std::string gJsonName_AdditionalArchiveFileCount = "AdditionalArchiveFileCount";

static std::string NormalizePathDelimiters(const std::string& thePath)
{
	std::string fixedPath = thePath;
	return NFmiStringTools::ReplaceChars(fixedPath, '/', '\\');
}

static void DoCsDataFileWinRegValuesInitializingChecks(NFmiCsDataFileWinReg& csDataValues)
{
	// Jos saatu ainakin yksi dataCount arvo joka ei ole puuttuva (-1), laitetaan alustus 'kunnolliseksi'.
	if(csDataValues.CaseStudyDataCount() >= 0 || csDataValues.LocalCacheDataCount() >= 0)
		csDataValues.ProperlyInitialized(true);
}

// ************************************************************
// *****   NFmiCsDataFileWinReg alkaa  ************************
// ************************************************************

NFmiCsDataFileWinReg::NFmiCsDataFileWinReg() = default;

NFmiCsDataFileWinReg::NFmiCsDataFileWinReg(int caseStudyDataCount, bool fixedValueForCaseStudyDataCount, int localCacheDataCount, bool fixedValueForLocalCacheDataCount, bool store)
:itsCaseStudyDataCount(caseStudyDataCount)
,fFixedValueForCaseStudyDataCount(fixedValueForCaseStudyDataCount)
,itsLocalCacheDataCount(localCacheDataCount)
,fFixedValueForLocalCacheDataCount(fixedValueForLocalCacheDataCount)
,fStore(store)
,fProperlyInitialized(false)
{
}

void NFmiCsDataFileWinReg::CaseStudyDataCount(int newValue) 
{ 
	if(!fFixedValueForCaseStudyDataCount)
		itsCaseStudyDataCount = newValue; 
}

void NFmiCsDataFileWinReg::FixedValueForCaseStudyDataCount(bool newValue) 
{ 
	fFixedValueForCaseStudyDataCount = newValue; 
}

void NFmiCsDataFileWinReg::LocalCacheDataCount(int newValue) 
{ 
	if(!fFixedValueForLocalCacheDataCount)
		itsLocalCacheDataCount = newValue; 
}

void NFmiCsDataFileWinReg::FixedValueForLocalCacheDataCount(bool newValue) 
{ 
	fFixedValueForLocalCacheDataCount = newValue; 
}

void NFmiCsDataFileWinReg::Store(bool newValue) 
{ 
	fStore = newValue; 
}

void NFmiCsDataFileWinReg::ProperlyInitialized(bool newValue) 
{ 
	fProperlyInitialized = newValue; 
}

bool NFmiCsDataFileWinReg::AreDataCountsFixed() const
{
	return fFixedValueForCaseStudyDataCount && fFixedValueForLocalCacheDataCount;
}

void NFmiCsDataFileWinReg::FixToLastDataOnlyType()
{
	itsCaseStudyDataCount = 1;
	fFixedValueForCaseStudyDataCount = true;
	itsLocalCacheDataCount = 1;
	fFixedValueForLocalCacheDataCount = true;
	fProperlyInitialized = true;
}

// Otetaan other:ista asetuksia, mutta vain ei-fiksatuissa tapauksissa.
void NFmiCsDataFileWinReg::DoCheckedAssignment(const NFmiCsDataFileWinReg& other)
{
	if(!fFixedValueForCaseStudyDataCount)
		itsCaseStudyDataCount = other.itsCaseStudyDataCount;
	if(!fFixedValueForLocalCacheDataCount)
		itsLocalCacheDataCount = other.itsLocalCacheDataCount;
	fStore = other.fStore;
	fProperlyInitialized = true;
}

// Otetaan other:ista mahdolliset fiksatut asetukset
void NFmiCsDataFileWinReg::AdaptFixedSettings(const NFmiCsDataFileWinReg& other)
{
	if(other.fFixedValueForCaseStudyDataCount)
	{
		itsCaseStudyDataCount = other.itsCaseStudyDataCount;
		fFixedValueForCaseStudyDataCount = other.fFixedValueForCaseStudyDataCount;
	}
	if(other.fFixedValueForLocalCacheDataCount)
	{
		itsLocalCacheDataCount = other.itsLocalCacheDataCount;
		fFixedValueForLocalCacheDataCount = other.fFixedValueForLocalCacheDataCount;
	}
}

bool NFmiCsDataFileWinReg::operator != (const NFmiCsDataFileWinReg & other) const
{
	if(itsCaseStudyDataCount != other.itsCaseStudyDataCount)
		return true;
	if(itsLocalCacheDataCount != other.itsLocalCacheDataCount)
		return true;
	if(fStore != other.fStore)
		return true;

	// fProperlyInitialized data osio ei kiinnosta t‰ss‰ vertailussa

	return false;
}


// ************************************************************
// *****   NFmiCaseStudyDataFile alkaa  ***********************
// ************************************************************

NFmiCaseStudyDataFile::NFmiCaseStudyDataFile(void)
:itsName()
,itsProducer()
,itsFileFilter()
,itsRelativeStoredFileFilter()
,itsDataFileWinRegValues()
,itsDataIntervalInMinutes(0)
,itsSingleFileSize(0)
,itsTotalFileSize(0)
,itsMaxFileSize(0)
,itsCategory(kFmiCategoryError)
,itsDataType(NFmiInfoData::kNoDataType)
,itsLevelCount(0)
,itsImageAreaStr()
,itsImageParam()
,fImageFile(false)
,fCategoryHeader(false)
,fProducerHeader(false)
,fOnlyOneData(false)
,fNotifyOnLoad(false)
,itsNotificationLabel()
,itsCustomMenuFolder()
,itsAdditionalArchiveFileCount(0)
,fDataEnabled(true)
{
	// HUOM! ei saa kutsua Reset-metodia, koska siell‰ kutsutaan taas oletus konstruktoria!!!
}

NFmiCaseStudyDataFile::~NFmiCaseStudyDataFile(void)
{
}

void NFmiCaseStudyDataFile::Reset(void)
{
	*this = NFmiCaseStudyDataFile();
}

static boost::shared_ptr<NFmiFastQueryInfo> GetInfo(NFmiInfoOrganizer &theInfoOrganizer, const std::string &theFileNameFilter)
{
	std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector = theInfoOrganizer.GetInfos(theFileNameFilter);
	if(infoVector.size())
		return infoVector[0];
	else
		return boost::shared_ptr<NFmiFastQueryInfo>();
}

int NFmiCaseStudyDataFile::GetModelRunTimeGapInMinutes(NFmiQueryInfo *theInfo, NFmiInfoData::Type theType, NFmiHelpDataInfo *theHelpDataInfo)
{
    // Pakko laittaa osoittamaan 1. parametriiin, koska t‰nne tulee aito NFmiQueryInfo olio, ja se ei osaa hakea 1. Producer:ia automaattisesti.
    theInfo->FirstParam();

	if(theHelpDataInfo && theHelpDataInfo->NonFixedTimeGab())
		return -1; // Jos datan konffissa on erikseen m‰‰r‰tty m‰‰rittelem‰tˆn aikav‰li, niin k‰ytet‰‰n sit‰.
	if(theType == NFmiInfoData::kKepaData)
		return -1; // editoidulla datalla ei ole vakio ajov‰li‰, niit‰ editoidaan ja l‰hetet‰‰n milloin vain
	
	if(theHelpDataInfo)
	{
		auto modelRunTimeGapInHours = theHelpDataInfo->ModelRunTimeGapInHours();
		if(modelRunTimeGapInHours < 0)
			return -1;
		if(modelRunTimeGapInHours > 0)
			return static_cast<int>(std::round(theHelpDataInfo->ModelRunTimeGapInHours() * 60)); // Jos datan konffissa on erikseen m‰‰r‰tty malliajov‰li, niin k‰ytet‰‰n sit‰.
	}

	int modelRunTimeGap = 0; // defaultti on 0, joka p‰tee kaikkiin datoihin, mist‰ ei ole arkistodataa k‰ytˆss‰, kuten analyysi, havainnot jne.
	if(NFmiDrawParam::IsModelRunDataType(theType))
	{
		if(theType == NFmiInfoData::kViewable && theInfo->Producer()->GetIdent() == kFmiRADARNRD)
		{
			modelRunTimeGap = 1*60; // tutka-tuliset on poikkeustapaus, sill‰ malliajov‰li on 1 h
		}
		else
		{
			modelRunTimeGap = 6*60; // mallidatojen defaultti on 6h
			if(theInfo)
			{
				unsigned long prodId = theInfo->Producer()->GetIdent();
				if(prodId == kFmiMTAECMWF || prodId == kFmiECMWF)
					modelRunTimeGap = 12*60; // Ecmwf:lle aikav‰li on 12h
				else if(prodId == 118) // 118 on EC Kalman-Laps tuottaja, se on myˆs 12 tunnin v‰lein kun se lasketaan EC:n pohjalta
					modelRunTimeGap = 12*60; // Ecmwf:lle aikav‰li on 12h
				else if(prodId == 199) // 199 on Harmonie ja sit‰ ajetaan ainakin p‰iv‰s aikaan 3h v‰lein
					modelRunTimeGap = 3*60; // Ecmwf:lle aikav‰li on 12h
			}
		}
	}
	return modelRunTimeGap;
}

static int GetImageGapInMinutes(const NFmiHelpDataInfo &theDataInfo)
{
	int gapInMinutes = 60; // default on 60 min, esim. noaa kuvat tulevat ep‰m‰‰r‰isesti, eik‰ ole oikeaa arvoa sille, t‰m‰n avulla lasketaan vain arvio data m‰‰rist‰
	FmiProducerName prodId = static_cast<FmiProducerName>(theDataInfo.ImageDataIdent().GetProducer()->GetIdent());
	if(prodId == 3050) // cinesat/meteosat9
		gapInMinutes = 15;
	else if(prodId == 3042) // meteosat8
		gapInMinutes = 15;
	return gapInMinutes;
}

static int GetModelRunTimeGapInMinutes(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiHelpDataInfo *theHelpDataInfo)
{
	if(theInfo)
	{
		return NFmiCaseStudyDataFile::GetModelRunTimeGapInMinutes(theInfo.get(), theInfo->DataType(), theHelpDataInfo);
	}
	return 0;
}

static double BoostFileSize(const std::string &theFileName)
{
	try
	{
		boost::intmax_t fileSizeBoost = boost::filesystem::file_size(theFileName);
		return static_cast<double>(fileSizeBoost);
	}
	catch(...)
	{
	}
	return 0;
}

static double LatestFileByteCount(const std::string & theFilePattern)
{
	return ::BoostFileSize(NFmiFileSystem::NewestPatternFileName(theFilePattern));
}

// mm. satel kuvien koon tarkistus on muuten liian hidasta (luetaan verkosta viimeisimm‰n tiedoston koko), 
// joten etsit‰‰n vain ensimm‰inen patterniin sopiva tiedosto ja otetaan sen koko.
static double FastFileByteCount(const std::string & theFilePattern, const NFmiCaseStudySystem &theCaseStudySystem)
{
	if(theCaseStudySystem.DoApproximateDataSize(theFilePattern))
		return ::BoostFileSize(NFmiFileSystem::FirstPatternFileName(theFilePattern));
	else
		return 0;
}

double NFmiCaseStudyDataFile::EvaluateTotalDataSize(void)
{
	if(StoreLastDataOnly())
		return itsSingleFileSize;
	else
	{
		return DataFileWinRegValues().CaseStudyDataCount() * itsSingleFileSize;
	}
}

void NFmiCaseStudyDataFile::Update(const NFmiCaseStudySystem &theCaseStudySystem)
{
	itsSingleFileSize = static_cast<double>(::FastFileByteCount(itsFileFilter, theCaseStudySystem));
	itsTotalFileSize = EvaluateTotalDataSize();
}

static NFmiCaseStudyDataFile::DataCategory GetDataCategory(const NFmiHelpDataInfo &theDataInfo)
{
	NFmiInfoData::Type type = theDataInfo.DataType();
	if(type == NFmiInfoData::kViewable || type == NFmiInfoData::kHybridData|| type == NFmiInfoData::kModelHelpData || type == NFmiInfoData::kTrajectoryHistoryData)
		return NFmiCaseStudyDataFile::kFmiCategoryModel;
    else if(type == NFmiInfoData::kClimatologyData)
        return NFmiCaseStudyDataFile::kFmiCategoryModel; // Kaikki fraktiili mallidatat on laitettu t‰h‰n kategoriaan. Miten erotella havainto fraktiilit?
    else if(type == NFmiInfoData::kAnalyzeData)
		return NFmiCaseStudyDataFile::kFmiCategoryAnalyze;
	else if(type == NFmiInfoData::kObservations || type == NFmiInfoData::kFlashData || type == NFmiInfoData::kSingleStationRadarData)
		return NFmiCaseStudyDataFile::kFmiCategoryObservation;
	else if(type == NFmiInfoData::kKepaData || type == NFmiInfoData::kEditingHelpData)
		return NFmiCaseStudyDataFile::kFmiCategoryEdited;
	else if(type == NFmiInfoData::kSatelData)
		return NFmiCaseStudyDataFile::kFmiCategorySatelImage;
	else
		return NFmiCaseStudyDataFile::kFmiCategoryError;
}

bool NFmiCaseStudyDataFile::Init(NFmiHelpDataInfoSystem &theDataInfoSystem, const NFmiHelpDataInfo &theDataInfo, NFmiInfoOrganizer &theInfoOrganizer, const NFmiCsDataFileWinReg& theDataFileWinRegValues, const NFmiCaseStudySystem &theCaseStudySystem)
{
	Reset();

	itsHelpDataInfoName = theDataInfo.Name();
	itsDataFileWinRegValues = theDataFileWinRegValues;
	itsCategory = GetDataCategory(theDataInfo);
	itsDataType = theDataInfo.DataType();
	fNotifyOnLoad = theDataInfo.NotifyOnLoad();
	itsNotificationLabel = theDataInfo.NotificationLabel();
	itsCustomMenuFolder = theDataInfo.CustomMenuFolder();
	itsAdditionalArchiveFileCount = theDataInfo.AdditionalArchiveFileCount();
	fDataEnabled = theDataInfo.IsEnabled();


	if(theDataInfo.DataType() == NFmiInfoData::kSatelData)
	{ 
		// Kyse on kuva datasta.
		// Kuvia ei cacheteta, joten pit‰‰ pyyt‰‰ 'originaali' polkua, ei UsedFileNameFilter -polkua
		itsFileFilter = ::NormalizePathDelimiters(theDataInfo.FileNameFilter()); 
		itsName = theDataInfo.ImageDataIdent().GetParamName();
		itsProducer = *(theDataInfo.ImageDataIdent().GetProducer());
		fImageFile = true;
		itsDataIntervalInMinutes = ::GetImageGapInMinutes(theDataInfo);
		itsSingleFileSize = static_cast<double>(::FastFileByteCount(itsFileFilter, theCaseStudySystem));
		itsTotalFileSize = EvaluateTotalDataSize();
		itsLevelCount = 1;
		if(theDataInfo.ImageArea())
			itsImageAreaStr = theDataInfo.LegacyAreaString();
		itsImageParam = NFmiParam(*(theDataInfo.ImageDataIdent().GetParam()));

		return true;
	}
	else
	{ 
		// Kyse on queryDatasta.
		if(theDataInfo.IsCombineData())
		{
			// Yhdistelm‰datoille otetaan yhdistelm‰n fileFilter
			itsFileFilter = ::NormalizePathDelimiters(theDataInfo.CombineDataPathAndFileName());
		}
		else
			itsFileFilter = ::NormalizePathDelimiters(theDataInfo.UsedFileNameFilter(theDataInfoSystem));

		itsName = theDataInfo.GetCleanedName();
		fImageFile = false;
		if(theDataInfo.IsCombineData())
			itsDataFileWinRegValues.FixToLastDataOnlyType();
		itsSingleFileSize = static_cast<double>(::FastFileByteCount(itsFileFilter, theCaseStudySystem));

        UpdateWithInfo(theInfoOrganizer, theDataInfoSystem);
        itsTotalFileSize = EvaluateTotalDataSize();
		return true;
	}
}

void NFmiCaseStudyDataFile::UpdateWithInfo(NFmiInfoOrganizer & theInfoOrganizer, NFmiHelpDataInfoSystem &theDataInfoSystem)
{
    boost::shared_ptr<NFmiFastQueryInfo> info = ::GetInfo(theInfoOrganizer, itsFileFilter);
    if(info)
    {
        itsProducer = *(info->Producer());
        itsDataIntervalInMinutes = ::GetModelRunTimeGapInMinutes(info, theDataInfoSystem.FindHelpDataInfo(itsFileFilter));
        itsLevelCount = static_cast<int>(info->SizeLevels());
    }
}

void NFmiCaseStudyDataFile::DataEnabled(NFmiHelpDataInfoSystem &theDataInfoSystem, bool newValue)
{
    NFmiHelpDataInfo *info = theDataInfoSystem.FindHelpDataInfo(FileFilter());
    if(info) // category ja producer tasoilla ei ole info:a
        info->Enable(newValue);
    fDataEnabled = newValue;
}

json_spirit::Object NFmiCaseStudyDataFile::MakeJsonObject(const NFmiCaseStudyDataFile &theData)
{
	json_spirit::Object jsonObject;
	jsonObject.push_back(json_spirit::Pair(gJsonName_Name, theData.Name()));
	jsonObject.push_back(json_spirit::Pair(gJsonName_ProducerName, std::string(theData.Producer().GetName())));
	jsonObject.push_back(json_spirit::Pair(gJsonName_ProducerId, static_cast<int>(theData.Producer().GetIdent())));
	jsonObject.push_back(json_spirit::Pair(gJsonName_FileFilter, theData.FileFilter()));
	jsonObject.push_back(json_spirit::Pair(gJsonName_RelativeStoredFileFilter, theData.RelativeStoredFileFilter()));
	// start ja end offsetit laitetaan vain, jotta systeemi olisi taaksep‰in vanhoihin versioihin n‰hden mahdollisimman yhteensopiva
	jsonObject.push_back(json_spirit::Pair(gJsonName_StartOffsetInMinutes_legacy, 10 * 60));
	jsonObject.push_back(json_spirit::Pair(gJsonName_EndOffsetInMinutes_legacy, 0));
	jsonObject.push_back(json_spirit::Pair(gJsonName_CaseStudyDataCount, theData.DataFileWinRegValues().CaseStudyDataCount()));
	jsonObject.push_back(json_spirit::Pair(gJsonName_LocalCacheDataCount, theData.DataFileWinRegValues().LocalCacheDataCount()));
//	int itsDataIntervalInMinutes;	// ei ole hyˆty‰ tallettaa case-study metadataan? johdettavissa?
//	double itsSingleFileSize;		// ei ole hyˆty‰ tallettaa case-study metadataan? johdettavissa?
//	double itsTotalFileSize;		// ei ole hyˆty‰ tallettaa case-study metadataan? johdettavissa?
//	double itsMaxFileSize;			// ei ole hyˆty‰ tallettaa case-study metadataan? johdettavissa?
//	DataCategory itsCategory;		// ei ole hyˆty‰ tallettaa case-study metadataan? johdettavissa?
	jsonObject.push_back(json_spirit::Pair(gJsonName_DataType, static_cast<int>(theData.itsDataType)));
//	int itsLevelCount;				// ei ole hyˆty‰ tallettaa case-study metadataan? johdettavissa?
	jsonObject.push_back(json_spirit::Pair(gJsonName_Store, theData.DataFileWinRegValues().Store()));
	jsonObject.push_back(json_spirit::Pair(gJsonName_ImageFile, theData.ImageFile()));
	jsonObject.push_back(json_spirit::Pair(gJsonName_StoreLastDataOnly, theData.StoreLastDataOnly()));
	jsonObject.push_back(json_spirit::Pair(gJsonName_IsCategoryHeader, theData.CategoryHeader()));
	jsonObject.push_back(json_spirit::Pair(gJsonName_IsProducerHeader, theData.ProducerHeader()));
	jsonObject.push_back(json_spirit::Pair(gJsonName_OnlyOneData, theData.OnlyOneData()));

	jsonObject.push_back(json_spirit::Pair(gJsonName_NotifyOnLoad, theData.NotifyOnLoad()));
	jsonObject.push_back(json_spirit::Pair(gJsonName_NotificationLabel, theData.NotificationLabel()));
	jsonObject.push_back(json_spirit::Pair(gJsonName_CustomMenuFolder, theData.CustomMenuFolder()));
	jsonObject.push_back(json_spirit::Pair(gJsonName_AdditionalArchiveFileCount, theData.AdditionalArchiveFileCount()));

	if(theData.ImageFile())
	{
		jsonObject.push_back(json_spirit::Pair(gJsonName_ImageAreaStr, theData.ImageAreaStr()));
		jsonObject.push_back(json_spirit::Pair(gJsonName_ImageParamName, std::string(theData.ImageParam().GetName())));
		jsonObject.push_back(json_spirit::Pair(gJsonName_ImageParamId, static_cast<int>(theData.ImageParam().GetIdent())));
	}

	return jsonObject;
}

void NFmiCaseStudyDataFile::ParseJsonValue(json_spirit::Value &theValue)
{
	if(theValue.type() == json_spirit::obj_type)
	{
		json_spirit::Object tmpObj = theValue.get_obj();
		for(json_spirit::Object::iterator it = tmpObj.begin(); it != tmpObj.end(); ++it)
		{
			ParseJsonPair(*it);
		}
		::DoCsDataFileWinRegValuesInitializingChecks(itsDataFileWinRegValues);
	}
}

void NFmiCaseStudyDataFile::ParseJsonPair(json_spirit::Pair &thePair)
{
	// T‰ss‰ puret‰‰n CaseStudySystem luokan p‰‰tason pareja.
	if(thePair.name_ == gJsonName_Name)
		itsName = thePair.value_.get_str();
	else if(thePair.name_ == gJsonName_ProducerName)
		itsProducer.SetName(thePair.value_.get_str());
	else if(thePair.name_ == gJsonName_ProducerId)
		itsProducer.SetIdent(thePair.value_.get_int());
	else if(thePair.name_ == gJsonName_FileFilter)
	{
		// Muutetaan viel‰ luetut polut niin ett‰ SHFileOperation-funktio ymm‰rt‰‰ ne varmasti
		itsFileFilter = ::NormalizePathDelimiters(thePair.value_.get_str());
	}
	else if(thePair.name_ == gJsonName_RelativeStoredFileFilter)
	{
		// Muutetaan viel‰ luetut polut niin ett‰ SHFileOperation-funktio ymm‰rt‰‰ ne varmasti
		itsRelativeStoredFileFilter = ::NormalizePathDelimiters(thePair.value_.get_str());
	}
	else if(thePair.name_ == gJsonName_CaseStudyDataCount)
		DataFileWinRegValues().CaseStudyDataCount(thePair.value_.get_int());
	else if(thePair.name_ == gJsonName_LocalCacheDataCount)
		DataFileWinRegValues().LocalCacheDataCount(thePair.value_.get_int());
	else if(thePair.name_ == gJsonName_DataType)
		itsDataType = static_cast<NFmiInfoData::Type>(thePair.value_.get_int());
	else if(thePair.name_ == gJsonName_Store)
		DataFileWinRegValues().Store(thePair.value_.get_bool());
	else if(thePair.name_ == gJsonName_ImageFile)
		fImageFile = thePair.value_.get_bool();
	else if(thePair.name_ == gJsonName_StoreLastDataOnly)
		; // tyhj‰‰ legacy koodia, ei tehd‰ mit‰‰n, voisi varmaan poistaa
	else if(thePair.name_ == gJsonName_IsCategoryHeader)
		fCategoryHeader = thePair.value_.get_bool();
	else if(thePair.name_ == gJsonName_IsProducerHeader)
		fProducerHeader = thePair.value_.get_bool();
	else if(thePair.name_ == gJsonName_OnlyOneData)
		fOnlyOneData = thePair.value_.get_bool();
	else if(thePair.name_ == gJsonName_ImageAreaStr)
		itsImageAreaStr = thePair.value_.get_str();
	else if(thePair.name_ == gJsonName_ImageParamName)
		itsImageParam.SetName(thePair.value_.get_str());
	else if(thePair.name_ == gJsonName_ImageParamId)
		itsImageParam.SetIdent(thePair.value_.get_int());
	else if(thePair.name_ == gJsonName_NotifyOnLoad)
		fNotifyOnLoad = thePair.value_.get_bool();
	else if(thePair.name_ == gJsonName_NotificationLabel)
		itsNotificationLabel = thePair.value_.get_str();
	else if(thePair.name_ == gJsonName_CustomMenuFolder)
		itsCustomMenuFolder = thePair.value_.get_str();
	else if(thePair.name_ == gJsonName_AdditionalArchiveFileCount)
		itsAdditionalArchiveFileCount = thePair.value_.get_int();
}

bool NFmiCaseStudyDataFile::StoreLastDataOnly(void) const 
{ 
	return itsDataFileWinRegValues.CaseStudyDataCount() == 1 && itsDataFileWinRegValues.FixedValueForCaseStudyDataCount();
}

bool NFmiCaseStudyDataFile::IsReadOnlyData() const
{
	if(StoreLastDataOnly())
		return true;
	if(itsDataType == NFmiInfoData::kSatelData)
		return true;

	return false;
}

static void DoFinalSetupsFromOriginalHelpDataInfo(NFmiHelpDataInfo& currentHelpDataInfo, const std::string &currentFileFilter, NFmiHelpDataInfoSystem& theOriginalHelpDataInfoSystem)
{
	auto helpDataInfo = theOriginalHelpDataInfoSystem.FindHelpDataInfo(currentFileFilter);
	if(helpDataInfo)
	{
		currentHelpDataInfo.NonFixedTimeGab(helpDataInfo->NonFixedTimeGab());
		currentHelpDataInfo.ModelRunTimeGapInHours(helpDataInfo->ModelRunTimeGapInHours());
		currentHelpDataInfo.TimeInterpolationRangeInMinutes(helpDataInfo->TimeInterpolationRangeInMinutes());
		currentHelpDataInfo.AllowCombiningToSurfaceDataInSoundingView(helpDataInfo->AllowCombiningToSurfaceDataInSoundingView());
		if(currentHelpDataInfo.Name().empty())
			currentHelpDataInfo.Name(helpDataInfo->Name());
	}
}

void NFmiCaseStudyDataFile::AddDataToHelpDataInfoSystem(boost::shared_ptr<NFmiHelpDataInfoSystem> &theHelpDataInfoSystem, const std::string &theBasePath, NFmiHelpDataInfoSystem& theOriginalHelpDataInfoSystem)
{
	NFmiHelpDataInfo helpDataInfo;
	helpDataInfo.FileNameFilter(theBasePath + RelativeStoredFileFilter(), true); // true = fiksataan filefilter--polku
	helpDataInfo.DataType(itsDataType);
	helpDataInfo.Name(HelpDataInfoName());
	helpDataInfo.NotifyOnLoad(NotifyOnLoad());
	helpDataInfo.NotificationLabel(NotificationLabel());
	helpDataInfo.CustomMenuFolder(CustomMenuFolder());
	helpDataInfo.AdditionalArchiveFileCount(AdditionalArchiveFileCount());
	if(ImageFile())
	{
		helpDataInfo.ImageDataIdent(NFmiDataIdent(ImageParam(), Producer()));
		helpDataInfo.FakeProducerId(static_cast<int>(Producer().GetIdent()));
		try
		{
            NFmiAreaFactory::return_type area = NFmiAreaFactory::Create(ImageAreaStr());
			helpDataInfo.ImageArea(area);
		}
		catch(...)
		{
		}
	}
	if(itsDataType == NFmiInfoData::kKepaData || itsDataType == NFmiInfoData::kEditingHelpData || itsDataType == NFmiInfoData::kSingleStationRadarData)
		helpDataInfo.FakeProducerId(static_cast<int>(Producer().GetIdent())); // n‰ill‰ datatyypeill‰ on poikkeus tuottajat, siis eri tuottaja smartmetissa kuin mit‰ lˆytyy itse datasta

	::DoFinalSetupsFromOriginalHelpDataInfo(helpDataInfo, FileFilter(), theOriginalHelpDataInfoSystem);
	theHelpDataInfoSystem->AddDynamic(helpDataInfo);
}

// otetaan talteen vain offsetit ja store
void NFmiCaseStudyDataFile::InitDataWithStoredSettings(NFmiCaseStudyDataFile &theOriginalDataFile)
{
	if(!itsDataFileWinRegValues.ProperlyInitialized())
	{
		itsDataFileWinRegValues.DoCheckedAssignment(theOriginalDataFile.DataFileWinRegValues());
	}
}

bool NFmiCaseStudyDataFile::operator==(const NFmiCaseStudyDataFile &other) const
{
    if(itsName != other.itsName)
        return false;
    if(itsProducer != other.itsProducer)
        return false;
    if(itsFileFilter != other.itsFileFilter)
        return false;
    if(itsRelativeStoredFileFilter != other.itsRelativeStoredFileFilter)
        return false;
    if(itsDataFileWinRegValues != other.itsDataFileWinRegValues)
        return false;
	if(itsCategory != other.itsCategory)
        return false;
    if(itsDataType != other.itsDataType)
        return false;
    if(itsImageAreaStr != other.itsImageAreaStr)
        return false;
    if(itsImageParam != other.itsImageParam)
        return false;
    if(fImageFile != other.fImageFile)
        return false;
    if(fCategoryHeader != other.fCategoryHeader)
        return false;
    if(fProducerHeader != other.fProducerHeader)
        return false;
    if(fOnlyOneData != other.fOnlyOneData)
        return false;
    if(fNotifyOnLoad != other.fNotifyOnLoad)
        return false;
    if(itsNotificationLabel != other.itsNotificationLabel)
        return false;
    if(itsCustomMenuFolder != other.itsCustomMenuFolder)
        return false;
    if(itsAdditionalArchiveFileCount != other.itsAdditionalArchiveFileCount)
        return false;
    if(fDataEnabled != other.fDataEnabled)
        return false;

    return true;
}

bool NFmiCaseStudyDataFile::operator!=(const NFmiCaseStudyDataFile &other) const
{
    return !(*this == other);
}

// ************************************************************
// *****   NFmiCaseStudyDataFile loppuu  **********************
// ************************************************************


// ************************************************************
// *****   NFmiCaseStudyProducerData alkaa  *******************
// ************************************************************

NFmiCaseStudyProducerData::NFmiCaseStudyProducerData(void)
:itsProducerHeaderInfo()
,itsFilesData()
{
	ProducerHeaderInfo().ProducerHeader(true);
}

NFmiCaseStudyProducerData::NFmiCaseStudyProducerData(const NFmiProducer &theProducer)
:itsProducerHeaderInfo()
,itsFilesData()
{
	ProducerHeaderInfo().Producer(theProducer);
	ProducerHeaderInfo().ProducerHeader(true);
	ProducerHeaderInfo().Name(std::string(theProducer.GetName()));
}

NFmiCaseStudyProducerData::~NFmiCaseStudyProducerData(void)
{
}

void NFmiCaseStudyProducerData::AddData(const NFmiCaseStudyDataFile &theData)
{
	itsFilesData.push_back(theData);
}

// updeittaa kaikki tuottajan data koot ja paivitt‰‰ totalSize ja maxSize-koot
void NFmiCaseStudyProducerData::Update(const NFmiCaseStudySystem &theCaseStudySystem)
{
	double totalFileSize = 0;
	double maxFileSize = 0;
	for(size_t i = 0; i < itsFilesData.size(); i++)
	{
		NFmiCaseStudyDataFile &fileData = itsFilesData[i];
		fileData.Update(theCaseStudySystem);
		maxFileSize += fileData.TotalFileSize();
		if(fileData.DataFileWinRegValues().Store())
			totalFileSize += fileData.TotalFileSize();
	}
	ProducerHeaderInfo().TotalFileSize(totalFileSize);
	ProducerHeaderInfo().MaxFileSize(maxFileSize);
}

void NFmiCaseStudyProducerData::ProducerStore(bool newValue)
{
	ProducerHeaderInfo().DataFileWinRegValues().Store(newValue);
	for(size_t i = 0; i < itsFilesData.size(); i++)
	{
		itsFilesData[i].DataFileWinRegValues().Store(newValue);
	}
}

void NFmiCaseStudyProducerData::ProducerEnable(NFmiHelpDataInfoSystem &theDataInfoSystem, bool newValue)
{
    ProducerHeaderInfo().DataEnabled(theDataInfoSystem, newValue);
	for(size_t i = 0; i < itsFilesData.size(); i++)
	{
        itsFilesData[i].DataEnabled(theDataInfoSystem, newValue);
	}
}

void NFmiCaseStudyProducerData::ProducerDataCount(int theDataCount, bool theCaseStudyCase)
{
	if(!ProducerHeaderInfo().IsReadOnlyData())
	{
		// Ei sallita alle 1:n lukuja. Jos ei halua tallettaa dataa CaseStudy pakettiin, laitetaan Store -optio falseksi.
		// Ja jos ei halua s‰ilytt‰‰ data tiedostoja lokaali cachessa ollenkaan, laitetaan Enable data - optio falseksi.
		if(theDataCount < 1)
			theDataCount = 1;

		if(theCaseStudyCase)
			ProducerHeaderInfo().DataFileWinRegValues().CaseStudyDataCount(theDataCount);
		else
			ProducerHeaderInfo().DataFileWinRegValues().LocalCacheDataCount(theDataCount);

		for(size_t i = 0; i < itsFilesData.size(); i++)
		{
			if(theCaseStudyCase)
				itsFilesData[i].DataFileWinRegValues().CaseStudyDataCount(theDataCount);
			else
				itsFilesData[i].DataFileWinRegValues().LocalCacheDataCount(theDataCount);
		}
	}
}

json_spirit::Object NFmiCaseStudyProducerData::MakeJsonObject(const NFmiCaseStudyProducerData &theData, bool fMakeFullStore)
{
	json_spirit::Array dataArray;
	const std::vector<NFmiCaseStudyDataFile> &dataVector = theData.FilesData();
	for(size_t i = 0; i < dataVector.size(); i++)
	{
		if(fMakeFullStore || dataVector[i].DataFileWinRegValues().Store())
		{ // vain talletettavat datat laitetaan listaan
			json_spirit::Object tmpObject = NFmiCaseStudyDataFile::MakeJsonObject(dataVector[i]);
			if(tmpObject.size())
			{
				json_spirit::Value tmpVal(tmpObject);
				dataArray.push_back(tmpVal);
			}
		}
	}

	json_spirit::Object jsonObject; // luodaan ns. null-objekti
	if(dataArray.size())
	{ // t‰ytet‰‰n objekti vain jos lˆytyi yht‰‰n talletettavaa dataa
		json_spirit::Object jsonHeaderObject = NFmiCaseStudyDataFile::MakeJsonObject(theData.ProducerHeaderInfo());
		jsonObject.push_back(json_spirit::Pair(gJsonName_ProducerHeader, jsonHeaderObject));
		jsonObject.push_back(json_spirit::Pair(gJsonName_ProducerDataArray, dataArray));
	}
	return jsonObject;
}

void NFmiCaseStudyProducerData::ParseJsonValue(json_spirit::Value &theValue)
{
	if(theValue.type() == json_spirit::obj_type)
	{
		json_spirit::Object tmpObj = theValue.get_obj();
		for(json_spirit::Object::iterator it = tmpObj.begin(); it != tmpObj.end(); ++it)
		{
			ParseJsonPair(*it);
		}
	}
}

void NFmiCaseStudyProducerData::ParseJsonPair(json_spirit::Pair &thePair)
{
	// T‰ss‰ puret‰‰n CaseStudySystem luokan p‰‰tason pareja.
	if(thePair.name_ == gJsonName_ProducerHeader)
	{
		itsProducerHeaderInfo.ParseJsonValue(thePair.value_);
	}
	else if(thePair.name_ == gJsonName_ProducerDataArray)
	{
		json_spirit::Array dataFileArray = thePair.value_.get_array();
		if(dataFileArray.size())
		{
			for(json_spirit::Array::iterator it = dataFileArray.begin(); it != dataFileArray.end(); ++it)
			{
				NFmiCaseStudyDataFile fileData;
				fileData.ParseJsonValue(*it);
				itsFilesData.push_back(fileData);
			}
		}
	}
}

void NFmiCaseStudyProducerData::SetCategory(NFmiCaseStudyDataFile::DataCategory theCategory)
{
	itsProducerHeaderInfo.Category(theCategory);
	for(size_t i = 0; i < itsFilesData.size(); i++)
		itsFilesData[i].Category(theCategory);
}

void NFmiCaseStudyProducerData::AddDataToHelpDataInfoSystem(boost::shared_ptr<NFmiHelpDataInfoSystem> &theHelpDataInfoSystem, const std::string &theBasePath, NFmiHelpDataInfoSystem& theOriginalHelpDataInfoSystem)
{
	for(size_t i = 0; i < itsFilesData.size(); i++)
		itsFilesData[i].AddDataToHelpDataInfoSystem(theHelpDataInfoSystem, theBasePath, theOriginalHelpDataInfoSystem);
}

// Joskus kun tehd‰‰n UpdateNoProducerData -p‰ˆivityksi‰, tulee samalle tuottajelle useampi kuin yksi data. T‰llˆin 
// NFmiCaseStudyDataFile -luokan fOnlyOneData -attribuutti pit‰‰ k‰yd‰ nollaamassa sen tuottajan datoille. 
void NFmiCaseStudyProducerData::UpdateOnlyOneDataStates()
{
    if(itsFilesData.size() >= 2)
    {
        for(auto &fileData : itsFilesData)
        {
            fileData.OnlyOneData(false);
        }
    }
}

void NFmiCaseStudyProducerData::InitDataWithStoredSettings(NFmiCaseStudyProducerData &theOriginalProducerData)
{
	// otetaan talteen header-osiosta offsetit ja store -tieto
	ProducerHeaderInfo().DataFileWinRegValues().DoCheckedAssignment(theOriginalProducerData.ProducerHeaderInfo().DataFileWinRegValues());

	// Lis‰ksi jokaisen lˆytyneen vastin parin tiedot p‰ivitet‰‰n
	std::vector<NFmiCaseStudyDataFile> &origDataFiles = theOriginalProducerData.FilesData();
	if(origDataFiles.size() > 0)
	{
		for(size_t i = 0; i < origDataFiles.size(); i++)
		{
			NFmiCaseStudyDataFile *dataFile = GetDataFile(origDataFiles[i].FileFilter());
			if(dataFile)
			{
				dataFile->InitDataWithStoredSettings(origDataFiles[i]);
			}
		}
	}
}

long NFmiCaseStudyProducerData::GetProducerIdent() const
{
	return ProducerHeaderInfo().Producer().GetIdent();
}

NFmiCaseStudyDataFile* NFmiCaseStudyProducerData::GetDataFile(const std::string &theFileFilter)
{
	for(size_t i = 0; i < itsFilesData.size(); i++)
	{
		if(itsFilesData[i].FileFilter() == theFileFilter)
			return &itsFilesData[i];
	}
	return 0;
}

bool NFmiCaseStudyProducerData::operator==(const NFmiCaseStudyProducerData &other) const
{
    if(itsProducerHeaderInfo != other.itsProducerHeaderInfo)
        return false;
    if(itsFilesData != other.itsFilesData)
        return false;

    return true;
}

bool NFmiCaseStudyProducerData::operator!=(const NFmiCaseStudyProducerData &other) const
{
    return !(*this == other);
}

// ************************************************************
// *****   NFmiCaseStudyProducerData loppuu  ******************
// ************************************************************


// ************************************************************
// *****   NFmiCaseStudyCategoryData alkaa  *******************
// ************************************************************

NFmiCaseStudyCategoryData::NFmiCaseStudyCategoryData(void)
:itsCategoryHeaderInfo()
,itsProducersData()
,itsParsingCategory(NFmiCaseStudyDataFile::kFmiCategoryError)
{
	CategoryHeaderInfo().CategoryHeader(true);
}

NFmiCaseStudyCategoryData::NFmiCaseStudyCategoryData(const std::string &theName, NFmiCaseStudyDataFile::DataCategory theCategory)
:itsCategoryHeaderInfo()
,itsProducersData()
{
	CategoryHeaderInfo().Name(theName);
	CategoryHeaderInfo().Category(theCategory);
	CategoryHeaderInfo().CategoryHeader(true);
}

NFmiCaseStudyCategoryData::~NFmiCaseStudyCategoryData(void)
{
}

static void SetProducerHeaderInfoValues(NFmiCaseStudyProducerData &theProducerData, const NFmiCaseStudyDataFile &theFileData)
{
	theProducerData.ProducerHeaderInfo().Category(theFileData.Category());
	if(theProducerData.ProducerHeaderInfo().Category() == NFmiCaseStudyDataFile::kFmiCategorySatelImage)
	{
		theProducerData.ProducerHeaderInfo().DataType(NFmiInfoData::kSatelData);
	}
	theProducerData.ProducerHeaderInfo().DataFileWinRegValues(theFileData.DataFileWinRegValues());
}

void NFmiCaseStudyCategoryData::AddData(NFmiCaseStudyDataFile &theData)
{
	theData.DataFileWinRegValues().AdaptFixedSettings(CategoryHeaderInfo().DataFileWinRegValues());
	// onko tuotttajalle jo omaa datasetti, jos ei ole, luo ensin se ja lis‰‰ data sitten tuottajelle
	for(auto& producerData : itsProducersData)
	{
		if(producerData.GetProducerIdent() == theData.Producer().GetIdent())
		{
			producerData.AddData(theData);
			return;
		}
	}
	// jos siis ei lˆytynyt tuottajaa, lis‰t‰‰n sellainen t‰ss‰
	NFmiCaseStudyProducerData newProdData(theData.Producer());
	::SetProducerHeaderInfoValues(newProdData, theData);
	newProdData.AddData(theData);
	itsProducersData.push_back(newProdData);
}

// Etsii lˆytyykˆ itsProducersData -vektorista 0-tuottajaa. 
// Jos lˆytyy, poista se vektorista ja palauta. Muuten palauta tyhj‰ otus.
NFmiCaseStudyProducerData NFmiCaseStudyCategoryData::RemoveNoProducerData()
{
    auto iter = std::find_if(itsProducersData.begin(), itsProducersData.end(), [](const auto &producerData) {return producerData.GetProducerIdent() == 0; });
    if(iter != itsProducersData.end())
    {
        NFmiCaseStudyProducerData tmp = *iter;
        itsProducersData.erase(iter);
        return tmp;
    }
    else
        return NFmiCaseStudyProducerData();
}

void NFmiCaseStudyCategoryData::UpdateOnlyOneDataStates()
{
    for(auto &producerData : itsProducersData)
    {
        producerData.UpdateOnlyOneDataStates();
    }
}

void NFmiCaseStudyCategoryData::UpdateNoProducerData(NFmiHelpDataInfoSystem &theDataInfoSystem, NFmiInfoOrganizer &theInfoOrganizer)
{
    NFmiCaseStudyProducerData noProducerData = RemoveNoProducerData();
    std::vector<NFmiCaseStudyDataFile> &fileDataVector = noProducerData.FilesData();
    for(auto &noProducerDataFile : fileDataVector)
    {
        noProducerDataFile.UpdateWithInfo(theInfoOrganizer, theDataInfoSystem);
        AddData(noProducerDataFile);
    }
    UpdateOnlyOneDataStates(); // Pit‰‰ p‰ivitt‰‰ OnlyOneData statukset, jos lˆytyi uusia datoja jollekin tuottajalle.
}

// updeittaa kaikki kategorian tuottajien data koot ja paivitt‰‰ omat totalSize ja maxSize-koot
void NFmiCaseStudyCategoryData::Update(const NFmiCaseStudySystem &theCaseStudySystem)
{
	double totalFileSize = 0;
	double maxFileSize = 0;
	for(auto& producerData : itsProducersData)
	{
		producerData.Update(theCaseStudySystem);
		totalFileSize += producerData.ProducerHeaderInfo().TotalFileSize();
		maxFileSize += producerData.ProducerHeaderInfo().MaxFileSize();
	}
	itsCategoryHeaderInfo.TotalFileSize(totalFileSize);
	itsCategoryHeaderInfo.MaxFileSize(maxFileSize);
}

// updeittaa kategorian halutun tuottajan data koot ja paivitt‰‰ omat totalSize ja maxSize-koot
void NFmiCaseStudyCategoryData::Update(unsigned long theProdId, const NFmiCaseStudySystem &theCaseStudySystem)
{
	double totalFileSize = 0;
	double maxFileSize = 0;
	for(auto& producerData : itsProducersData)
	{
		if(producerData.GetProducerIdent() == static_cast<long>(theProdId))
			producerData.Update(theCaseStudySystem);
		totalFileSize += producerData.ProducerHeaderInfo().TotalFileSize();
		maxFileSize += producerData.ProducerHeaderInfo().MaxFileSize();
	}
	itsCategoryHeaderInfo.TotalFileSize(totalFileSize);
	itsCategoryHeaderInfo.MaxFileSize(maxFileSize);
}

void NFmiCaseStudyCategoryData::ProducerStore(unsigned long theProdId, bool newValue, const NFmiCaseStudySystem &theCaseStudySystem)
{
	for(auto& producerData : itsProducersData)
	{
		if(producerData.GetProducerIdent() == static_cast<long>(theProdId))
			producerData.ProducerStore(newValue);
	}
	Update(theProdId, theCaseStudySystem);
}

void NFmiCaseStudyCategoryData::CategoryStore(bool newValue, const NFmiCaseStudySystem &theCaseStudySystem)
{
	itsCategoryHeaderInfo.DataFileWinRegValues().Store(newValue);
	for(auto& producerData : itsProducersData)
		producerData.ProducerStore(newValue);
	Update(theCaseStudySystem);
}

void NFmiCaseStudyCategoryData::ProducerEnable(NFmiHelpDataInfoSystem &theDataInfoSystem, unsigned long theProdId, bool newValue, const NFmiCaseStudySystem &theCaseStudySystem)
{
	for(auto& producerData : itsProducersData)
	{
		if(producerData.GetProducerIdent() == static_cast<long>(theProdId))
			producerData.ProducerEnable(theDataInfoSystem, newValue);
	}
	Update(theProdId, theCaseStudySystem);
}

void NFmiCaseStudyCategoryData::CategoryEnable(NFmiHelpDataInfoSystem &theDataInfoSystem, bool newValue, const NFmiCaseStudySystem &theCaseStudySystem)
{
    itsCategoryHeaderInfo.DataEnabled(theDataInfoSystem, newValue);
	for(auto& producerData : itsProducersData)
		producerData.ProducerEnable(theDataInfoSystem, newValue);
	Update(theCaseStudySystem);
}

void NFmiCaseStudyCategoryData::ProducerDataCount(unsigned long theProdId, int theDataCount, const NFmiCaseStudySystem& theCaseStudySystem, bool theCaseStudyCase)
{
	for(auto& producerData : itsProducersData)
	{
		if(producerData.GetProducerIdent() == static_cast<long>(theProdId))
		{
			producerData.ProducerDataCount(theDataCount, theCaseStudyCase);
		}
	}
	Update(theProdId, theCaseStudySystem);
}

void NFmiCaseStudyCategoryData::CategoryDataCount(int theDataCount, const NFmiCaseStudySystem& theCaseStudySystem, bool theCaseStudyCase)
{
	if(!itsCategoryHeaderInfo.IsReadOnlyData())
	{
		if(theCaseStudyCase)
			itsCategoryHeaderInfo.DataFileWinRegValues().CaseStudyDataCount(theDataCount);
		else
			itsCategoryHeaderInfo.DataFileWinRegValues().LocalCacheDataCount(theDataCount);

		for(auto& producerData : itsProducersData)
		{
			producerData.ProducerDataCount(theDataCount, theCaseStudyCase);
		}
		Update(theCaseStudySystem);
	}
}

json_spirit::Object NFmiCaseStudyCategoryData::MakeJsonObject(const NFmiCaseStudyCategoryData &theData, bool fMakeFullStore)
{
	json_spirit::Array dataArray;
	const auto &dataList = theData.ProducersData();
	for(const auto &data : dataList)
	{
		json_spirit::Object tmpObject = NFmiCaseStudyProducerData::MakeJsonObject(data, fMakeFullStore);
		if(tmpObject.size())
		{
			json_spirit::Value tmpVal(tmpObject);
			dataArray.push_back(tmpVal); // vain ei null-otukset laitetaan listaan
		}
	}

	json_spirit::Object jsonObject; // luodaan aluksi ns. null-objekti
	if(dataArray.size())
	{ // t‰ytet‰‰n objekti vain jos lˆytyi yht‰‰n talletettua tuottajaa
		json_spirit::Object jsonHeaderObject = NFmiCaseStudyDataFile::MakeJsonObject(theData.CategoryHeaderInfo());
		jsonObject.push_back(json_spirit::Pair(gJsonName_Category, theData.CategoryHeaderInfo().Category()));
		jsonObject.push_back(json_spirit::Pair(gJsonName_CategoryHeader, jsonHeaderObject));
		jsonObject.push_back(json_spirit::Pair(gJsonName_CategoryDataArray, dataArray));
	}
	return jsonObject;
}

void NFmiCaseStudyCategoryData::ParseJsonValue(json_spirit::Value &theValue)
{
	if(theValue.type() == json_spirit::obj_type)
	{
		json_spirit::Object tmpObj = theValue.get_obj();
		for(json_spirit::Object::iterator it = tmpObj.begin(); it != tmpObj.end(); ++it)
		{
			ParseJsonPair(*it);
		}
		CategoryHeaderInfo().Category(itsParsingCategory); // categoria pit‰‰ asettaa lopuksi viel‰ headeriinkin
	}
}

void NFmiCaseStudyCategoryData::ParseJsonPair(json_spirit::Pair &thePair)
{
	// T‰ss‰ puret‰‰n CaseStudySystem luokan p‰‰tason pareja.
	if(thePair.name_ == gJsonName_Category)
	{
		itsParsingCategory = static_cast<NFmiCaseStudyDataFile::DataCategory>(thePair.value_.get_int());
	}
	else if(thePair.name_ == gJsonName_CategoryHeader)
	{
		itsCategoryHeaderInfo.ParseJsonValue(thePair.value_);
	}
	else if(thePair.name_ == gJsonName_CategoryDataArray)
	{
		json_spirit::Array producerArray = thePair.value_.get_array();
		if(producerArray.size())
		{
			for(json_spirit::Array::iterator it = producerArray.begin(); it != producerArray.end(); ++it)
			{
				NFmiCaseStudyProducerData producerData;
				producerData.ParseJsonValue(*it);
				producerData.SetCategory(itsParsingCategory);
				itsProducersData.push_back(producerData);
			}
		}
	}
}

void NFmiCaseStudyCategoryData::AddDataToHelpDataInfoSystem(boost::shared_ptr<NFmiHelpDataInfoSystem> &theHelpDataInfoSystem, const std::string &theBasePath, NFmiHelpDataInfoSystem& theOriginalHelpDataInfoSystem)
{
	for(auto& producerData : itsProducersData)
		producerData.AddDataToHelpDataInfoSystem(theHelpDataInfoSystem, theBasePath, theOriginalHelpDataInfoSystem);
}

void NFmiCaseStudyCategoryData::InitDataWithStoredSettings(NFmiCaseStudyCategoryData& theOriginalCategoryData)
{
	// otetaan talteen header-osiosta DataFileWinRegValues -tieto
	CategoryHeaderInfo().DataFileWinRegValues().DoCheckedAssignment(theOriginalCategoryData.CategoryHeaderInfo().DataFileWinRegValues());

	// Lis‰ksi jokaisen lˆytyneen producerDatan vastin parin tiedot p‰ivitet‰‰n
	auto& originalProducersData = theOriginalCategoryData.ProducersData();
	for(auto& originalProducerData : originalProducersData)
	{
		NFmiCaseStudyProducerData* producerData = GetProducerData(originalProducerData.GetProducerIdent());
		if(producerData)
		{
			producerData->InitDataWithStoredSettings(originalProducerData);
		}
	}
}

void NFmiCaseStudyCategoryData::PutNoneProducerDataToEndFix()
{
	auto iter = std::find_if(itsProducersData.begin(), itsProducersData.end(),
		[](const auto& producerData) {return producerData.GetProducerIdent() == 0; }
	);
	if(iter != itsProducersData.end())
	{
		// Jos lˆytyi ns. None producer, siirret‰‰n se listan loppuun
		itsProducersData.splice(itsProducersData.end(), itsProducersData, iter);
	}
}

NFmiCaseStudyProducerData* NFmiCaseStudyCategoryData::GetProducerData(unsigned long theProdId)
{
	for(auto &producerData : itsProducersData)
	{
		if(producerData.GetProducerIdent() == static_cast<long>(theProdId))
			return &producerData;
	}
	return 0;
}

bool NFmiCaseStudyCategoryData::operator==(const NFmiCaseStudyCategoryData &other) const
{
    if(itsCategoryHeaderInfo != other.itsCategoryHeaderInfo)
        return false;
    if(itsProducersData != other.itsProducersData)
        return false;

    return true;
}

bool NFmiCaseStudyCategoryData::operator!=(const NFmiCaseStudyCategoryData &other) const
{
    return !(*this == other);
}

// ************************************************************
// *****   NFmiCaseStudyCategoryData loppuu  ******************
// ************************************************************


// ************************************************************
// *****   NFmiCaseStudySystem alkaa  *************************
// ************************************************************

NFmiCaseStudySystem::NFmiCaseStudySystem(void)
:itsName("Case1")
,itsInfo("Your Case Study info here")
,itsCaseStudyPath()
,itsSmartMetLocalCachePath()
,itsTime()
,fZipFiles()
,fStoreWarningMessages()
,fCropDataToZoomedMapArea()
,fDeleteTmpFiles(true)
,fApproximateOnlyLocalDataSize(true)
,itsCategoriesData()
,itsCaseStudyDialogData()
,itsTreePatternArray()
{
    HKEY usedKey = HKEY_CURRENT_USER;   
    std::string sectionName = "\\General";
    std::string baseRegistryPath = "Software\\Fmi\\SmartMet";
    itsCaseStudyPath = ::CreateRegValue<CachedRegString>(baseRegistryPath, sectionName, "\\CaseStudyPath", usedKey, "D:\\data\\case");
    fZipFiles = ::CreateRegValue<CachedRegBool>(baseRegistryPath, sectionName, "\\ZipFiles", usedKey, true);
	fStoreWarningMessages = ::CreateRegValue<CachedRegBool>(baseRegistryPath, sectionName, "\\StoreWarningMessages", usedKey, true);
	fCropDataToZoomedMapArea = ::CreateRegValue<CachedRegBool>(baseRegistryPath, sectionName, "\\CropDataToZoomedMapArea", usedKey, false);
}

NFmiCaseStudySystem::~NFmiCaseStudySystem(void)
{
}

void NFmiCaseStudySystem::Reset(void)
{
	*this = NFmiCaseStudySystem();
}

static void SetCategoryHeaderInfoValues(NFmiCaseStudyCategoryData &theCategory, const NFmiCsDataFileWinReg &categoryDefaultValues)
{
	if(theCategory.CategoryHeaderInfo().Category() == NFmiCaseStudyDataFile::kFmiCategorySatelImage)
	{
		theCategory.CategoryHeaderInfo().DataType(NFmiInfoData::kSatelData);
	}
	theCategory.CategoryHeaderInfo().DataFileWinRegValues(categoryDefaultValues);
}

static NFmiCsDataFileWinReg MakeCsDataFileWinRegValues(const NFmiHelpDataInfo& info, NFmiCaseStudySettingsWinRegistry& theCaseStudySettingsWinRegistry)
{
	const int defaultModelDataCaseStudyCount = NFmiCaseStudySettingsWinRegistry::GetDefaultCaseStudyCountValue(info.DataType());
	const int defaultModelLocalCacheCount = NFmiCaseStudySettingsWinRegistry::GetDefaultLocalCacheCountValue(info.DataType());
	bool fixedValueForCaseStudyCount = defaultModelDataCaseStudyCount <= 1;
	bool fixedValueForLocalCacheCount = defaultModelLocalCacheCount <= 1;
	const auto& uniqueDataName = info.Name();
	int caseStudyDataCount = theCaseStudySettingsWinRegistry.GetHelpDataCaseStudyCount(uniqueDataName);
	int localCacheDataCount = theCaseStudySettingsWinRegistry.GetHelpDataLocalCacheCount(uniqueDataName);
	bool store = theCaseStudySettingsWinRegistry.GetStoreDataState(uniqueDataName);
	return NFmiCsDataFileWinReg(caseStudyDataCount, fixedValueForCaseStudyCount, localCacheDataCount, fixedValueForLocalCacheCount, store);
}

bool NFmiCaseStudySystem::Init(NFmiHelpDataInfoSystem &theDataInfoSystem, NFmiInfoOrganizer &theInfoOrganizer, NFmiCaseStudySettingsWinRegistry& theCaseStudySettingsWinRegistry)
{
	std::vector<NFmiCaseStudyCategoryData> originalCategoriesData;
	originalCategoriesData.swap(itsCategoriesData); // otetaan talteen valmiiksi konffitiedostosta ladatut asetukset, ett‰ voidaan lopuksi s‰‰t‰‰ asetuksia niiden mukaan

	const int defaultModelDataCaseStudyCount = NFmiCaseStudySettingsWinRegistry::GetDefaultCaseStudyCountValue(NFmiInfoData::kViewable);
	const int defaultModelLocalCacheCount = NFmiCaseStudySettingsWinRegistry::GetDefaultLocalCacheCountValue(NFmiInfoData::kViewable);
	const bool defaultStoreDataSetting = true;
	NFmiCsDataFileWinReg category1DefaultValues(defaultModelDataCaseStudyCount, false, defaultModelLocalCacheCount, false, defaultStoreDataSetting);
	NFmiCaseStudyCategoryData category1(::GetDictionaryString("Model data"), NFmiCaseStudyDataFile::kFmiCategoryModel);
	::SetCategoryHeaderInfoValues(category1, category1DefaultValues);
	itsCategoriesData.push_back(category1);

	const int defaultObservationCaseStudyCount = 1;
	const int defaultObservationLocalCacheCount = 1;
	NFmiCsDataFileWinReg category2DefaultValues(defaultObservationCaseStudyCount, true, defaultObservationLocalCacheCount, true, defaultStoreDataSetting);
	NFmiCaseStudyCategoryData category2(::GetDictionaryString("Observation data"), NFmiCaseStudyDataFile::kFmiCategoryObservation);
	::SetCategoryHeaderInfoValues(category2, category2DefaultValues);
	itsCategoriesData.push_back(category2);

	const int defaultAnalyzeCaseStudyCount = 1;
	const int defaultAnalyzeLocalCacheCount = 1;
	NFmiCsDataFileWinReg category3DefaultValues(defaultAnalyzeCaseStudyCount, true, defaultAnalyzeLocalCacheCount, true, defaultStoreDataSetting);
	NFmiCaseStudyCategoryData category3(::GetDictionaryString("Analyze data"), NFmiCaseStudyDataFile::kFmiCategoryAnalyze);
	::SetCategoryHeaderInfoValues(category3, category3DefaultValues);
	itsCategoriesData.push_back(category3);

	NFmiCaseStudyCategoryData category4(::GetDictionaryString("Edited data"), NFmiCaseStudyDataFile::kFmiCategoryEdited);
	// K‰ytet‰‰n myˆs t‰ss‰ mallidata (category 1) defaultValues:eja
	::SetCategoryHeaderInfoValues(category4, category1DefaultValues);
	itsCategoriesData.push_back(category4);

	const int defaultSatelDataCaseStudyCount = NFmiCaseStudySettingsWinRegistry::GetDefaultCaseStudyCountValue(NFmiInfoData::kSatelData);
	const int defaultSatelDataLocalCacheCount = 0;
	NFmiCsDataFileWinReg category5DefaultValues(defaultSatelDataCaseStudyCount, false, defaultSatelDataLocalCacheCount, true, defaultStoreDataSetting);
	NFmiCaseStudyCategoryData category5(::GetDictionaryString("Satellite image data"), NFmiCaseStudyDataFile::kFmiCategorySatelImage);
	::SetCategoryHeaderInfoValues(category5, category5DefaultValues);
	itsCategoriesData.push_back(category5);

	for(const auto& info : theDataInfoSystem.DynamicHelpDataInfos())
	{
		NFmiCaseStudyDataFile data;
		data.Init(theDataInfoSystem, info, theInfoOrganizer, ::MakeCsDataFileWinRegValues(info, theCaseStudySettingsWinRegistry), *this);
		AddData(data);
	}
	InitDataWithStoredSettings(originalCategoriesData);
	PutNoneProducerDataToEndFix();
	Update();
	return true;
}

// Kun data on alustettu 1. kerran, laitetaan tuntemattomien datojen (None) producer osio aina viimeiseksi kategorian listoissa.
void NFmiCaseStudySystem::PutNoneProducerDataToEndFix()
{
	for(auto& categoryData : itsCategoriesData)
	{
		categoryData.PutNoneProducerDataToEndFix();
	}
}

void NFmiCaseStudySystem::UpdateValuesBackToWinRegistry(NFmiCaseStudySettingsWinRegistry& theCaseStudySettingsWinRegistry)
{
	// Laitetaan CaseStudy dialogissa tehdyt muutokset talteen Windows rekisteriin

	// 1. CaseStudy data count
	auto &caseStudyDataCountMap = theCaseStudySettingsWinRegistry.GetHelpDataCaseStudyCountMap();
	// 2. Local cache data count
	auto& localCacheDataCountMap = theCaseStudySettingsWinRegistry.GetHelpDataLocalCacheCountMap();
	// 3. CaseStudy store
	auto& caseStudyStoreMap = theCaseStudySettingsWinRegistry.GetCaseStudyStoreDataMap();
	for(auto& categoryData : itsCategoriesData)
	{
		for(auto& producerData : categoryData.ProducersData())
		{
			for(auto& dataFile : producerData.FilesData())
			{
				const auto& helpDataInfoName = dataFile.HelpDataInfoName();
				auto caseStudyDataCountIter = caseStudyDataCountMap.find(helpDataInfoName);
				if(caseStudyDataCountIter != caseStudyDataCountMap.end())
					*caseStudyDataCountIter->second.second = dataFile.DataFileWinRegValues().CaseStudyDataCount();

				auto localCacheDataCountIter = localCacheDataCountMap.find(helpDataInfoName);
				if(localCacheDataCountIter != localCacheDataCountMap.end())
					*localCacheDataCountIter->second.second = dataFile.DataFileWinRegValues().LocalCacheDataCount();

				auto caseStudyStoreIter = caseStudyStoreMap.find(helpDataInfoName);
				if(caseStudyStoreIter != caseStudyStoreMap.end())
					*caseStudyStoreIter->second = dataFile.DataFileWinRegValues().Store();
			}
		}
	}
}

void NFmiCaseStudySystem::InitDataWithStoredSettings(std::vector<NFmiCaseStudyCategoryData>& theOriginalCategoriesData)
{
	for(auto &originalCategoryData : theOriginalCategoriesData)
	{
		auto categoryData = GetCategoryData(originalCategoryData.CategoryHeaderInfo().Category());
		if(categoryData)
		{
			categoryData->InitDataWithStoredSettings(originalCategoryData);
		}
	}
}

NFmiCaseStudyCategoryData* NFmiCaseStudySystem::GetCategoryData(NFmiCaseStudyDataFile::DataCategory theCategory)
{
	for(auto &categoryData : itsCategoriesData)
	{
		if(categoryData.CategoryHeaderInfo().Category() == theCategory)
			return &categoryData;
	}
	return nullptr;
}

void NFmiCaseStudySystem::AddData(NFmiCaseStudyDataFile &theData)
{
	auto categoryData = GetCategoryData(theData.Category());
	if(categoryData)
		categoryData->AddData(theData);
}

// updeittaa kaikki kategorian tuottajien data koot ja paivitt‰‰ omat totalSize ja maxSize-koot
void NFmiCaseStudySystem::Update(void)
{
	for(auto& categoryData : itsCategoriesData)
	{
		categoryData.Update(*this);
	}
}

// Jos CaseStudy dialogi on avattu enenn kuin kaikkia datoja on ehditty lukea SmartMetiin, j‰‰v‰t
// puuttuvat datat 0 producer Id tuottajan alle. Nyt pit‰‰ k‰yd‰ kaikki ne datat uudestaan l‰pi
// ja katsoa onko kyseinen data luettu k‰yttˆˆn. Sitten data pit‰‰ lis‰t‰ systeemiin uudestaan, ett‰ 
// se menee oikeaan lokeroon.
void NFmiCaseStudySystem::UpdateNoProducerData(NFmiHelpDataInfoSystem &theDataInfoSystem, NFmiInfoOrganizer &theInfoOrganizer)
{
	for(auto& categoryData : itsCategoriesData)
	{
        categoryData.UpdateNoProducerData(theDataInfoSystem, theInfoOrganizer);
    }
	Update();
	PutNoneProducerDataToEndFix();
}

// updeittaa halutun kategorian halutun tuottajien data koot ja paivitt‰‰ omat totalSize ja maxSize-koot
void NFmiCaseStudySystem::Update(NFmiCaseStudyDataFile::DataCategory theCategory, unsigned long theProdId)
{
	auto categoryData = GetCategoryData(theCategory);
	if(categoryData)
		categoryData->Update(theProdId, *this);
}

static std::string NormalizeWindowsPathString(const std::string &thePath)
{
	std::string pathStr(thePath);
	NFmiStringTools::LowerCase(pathStr);
	pathStr = ::NormalizePathDelimiters(pathStr);
	return pathStr;
}

bool NFmiCaseStudySystem::IsPathInLocalCache(const std::string &thePath) const
{
	std::string pathStr = ::NormalizeWindowsPathString(thePath);
	if(itsSmartMetLocalCachePath.empty() == false)
	{
		std::string::size_type pos = pathStr.find(itsSmartMetLocalCachePath);
		if(pos != std::string::npos)
			return true;
	}

	std::string::size_type pos = pathStr.find("c:\\"); // jos polku on c-asemalla, sekin on lokaali
	if(pos != std::string::npos)
		return true;

	return false;
}

void NFmiCaseStudySystem::SmartMetLocalCachePath(const std::string &newValue) 
{
	itsSmartMetLocalCachePath = ::NormalizeWindowsPathString(newValue);
}

bool NFmiCaseStudySystem::DoApproximateDataSize(const std::string &thePath) const
{
	if(fApproximateOnlyLocalDataSize)
		return IsPathInLocalCache(thePath);
	else
		return true;
}

void NFmiCaseStudySystem::ProducerStore(NFmiCaseStudyDataFile::DataCategory theCategory, unsigned long theProdId, bool newValue)
{
	auto categoryData = GetCategoryData(theCategory);
	if(categoryData)
		categoryData->ProducerStore(theProdId, newValue, *this);
}

void NFmiCaseStudySystem::CategoryStore(NFmiCaseStudyDataFile::DataCategory theCategory, bool newValue)
{
	auto categoryData = GetCategoryData(theCategory);
	if(categoryData)
		categoryData->CategoryStore(newValue, *this);
}

void NFmiCaseStudySystem::ProducerEnable(NFmiHelpDataInfoSystem &theDataInfoSystem, NFmiCaseStudyDataFile::DataCategory theCategory, unsigned long theProdId, bool newValue)
{
	auto categoryData = GetCategoryData(theCategory);
	if(categoryData)
		categoryData->ProducerEnable(theDataInfoSystem, theProdId, newValue, *this);
}

void NFmiCaseStudySystem::CategoryEnable(NFmiHelpDataInfoSystem &theDataInfoSystem, NFmiCaseStudyDataFile::DataCategory theCategory, bool newValue)
{
	auto categoryData = GetCategoryData(theCategory);
	if(categoryData)
		categoryData->CategoryEnable(theDataInfoSystem, newValue, *this);
}

void NFmiCaseStudySystem::ProducerDataCount(NFmiCaseStudyDataFile::DataCategory theCategory, unsigned long theProdId, int theDataCount, bool theCaseStudyCase)
{
	auto categoryData = GetCategoryData(theCategory);
	if(categoryData)
		categoryData->ProducerDataCount(theProdId, theDataCount, *this, theCaseStudyCase);
}

void NFmiCaseStudySystem::CategoryDataCount(NFmiCaseStudyDataFile::DataCategory theCategory, int theDataCount, bool theCaseStudyCase)
{
	auto categoryData = GetCategoryData(theCategory);
	if(categoryData)
		categoryData->CategoryDataCount(theDataCount, *this, theCaseStudyCase);
}

static NFmiProducerSystem* GetProducerSystem(NFmiProducerSystemsHolder &theProducerSystemsHolder, NFmiCaseStudyDataFile::DataCategory theCategory)
{
	if(theCategory == NFmiCaseStudyDataFile::kFmiCategoryModel)
		return theProducerSystemsHolder.itsModelProducerSystem;
	else if(theCategory == NFmiCaseStudyDataFile::kFmiCategoryObservation)
		return theProducerSystemsHolder.itsObsProducerSystem;
	else if(theCategory == NFmiCaseStudyDataFile::kFmiCategoryAnalyze)
		return theProducerSystemsHolder.itsObsProducerSystem; // analyysi tuottajat on sijoitettu myˆs obs-tuottaja listaan
	else if(theCategory == NFmiCaseStudyDataFile::kFmiCategorySatelImage)
		return theProducerSystemsHolder.itsSatelImageProducerSystem;

	return 0;
}

void NFmiCaseStudySystem::SetProducerName(NFmiProducerSystemsHolder &theProducerSystemsHolder, NFmiCaseStudyProducerData &theProducerData)
{
	NFmiCaseStudyDataFile &prodHeaderInfo = theProducerData.ProducerHeaderInfo();
	NFmiProducer tmpProd(prodHeaderInfo.Producer());
	// 1. tuottajanimi arvaus on jo jostain producerin datasta otettu tuottaja nimi
	std::string prodName = tmpProd.GetName();

	NFmiProducerSystem *producerSystem = ::GetProducerSystem(theProducerSystemsHolder, prodHeaderInfo.Category());
	if(producerSystem)
	{
		unsigned long index = producerSystem->FindProducerInfo(tmpProd); // palauttaa 0-indeksin, jos ei lˆytynyt
		if(index)
		{
			NFmiProducerInfo &prodInfo = producerSystem->Producer(index);
			prodName = prodInfo.Name();
		}
	}
	
	// asetetaan lopuksi saatu nimi sek‰ nimeksi ett‰ tuottaja-olion nimeksi
	prodHeaderInfo.Name(prodName);
	tmpProd.SetName(prodName);
	prodHeaderInfo.Producer(tmpProd);
}

void NFmiCaseStudySystem::FillCaseStudyDialogData(NFmiProducerSystemsHolder &theProducerSystemsHolder)
{
	itsCaseStudyDialogData.clear();
	itsTreePatternArray.clear();

	std::vector<NFmiCaseStudyCategoryData> &categoryDataVec = CategoriesData();
	for(size_t i = 0; i < categoryDataVec.size(); i++)
	{
		NFmiCaseStudyCategoryData &categoryData = categoryDataVec[i];
		itsCaseStudyDialogData.push_back(&categoryData.CategoryHeaderInfo());
		itsTreePatternArray.push_back(1); // p‰‰taso puussa

		auto &producersData = categoryData.ProducersData();
		for(auto & producerData : producersData)
		{
			std::vector<NFmiCaseStudyDataFile> &filesDataVec = producerData.FilesData();
			bool hasOnlyOneData = filesDataVec.size() <= 1;
			if(hasOnlyOneData == false)
			{ // producer-taso tehd‰‰n, vain jos tuotttajalla on useita datoja
				SetProducerName(theProducerSystemsHolder, producerData);
				itsCaseStudyDialogData.push_back(&producerData.ProducerHeaderInfo());
				itsTreePatternArray.push_back(2); // toinen taso puussa
			}

			for(size_t k = 0; k < filesDataVec.size(); k++)
			{
				NFmiCaseStudyDataFile &dataFile = filesDataVec[k];
				if(hasOnlyOneData)
					dataFile.OnlyOneData(true);
				itsCaseStudyDialogData.push_back(&dataFile);
				itsTreePatternArray.push_back(hasOnlyOneData ? 2 : 3); // alin (file kohtainen) taso puussa (tai jos vain yksi data tuottajalla, on taso 2)
			}
		}
	}
}

json_spirit::Object NFmiCaseStudySystem::MakeJsonObject(NFmiCaseStudySystem &theData, bool fMakeFullStore)
{
	json_spirit::Array dataArray;
	std::vector<NFmiCaseStudyCategoryData> &dataVector = theData.CategoriesData();
	for(size_t i = 0; i < dataVector.size(); i++)
	{
		json_spirit::Object tmpObject = NFmiCaseStudyCategoryData::MakeJsonObject(dataVector[i], fMakeFullStore);
		if(tmpObject.size())
		{
			json_spirit::Value tmpVal(tmpObject);
			dataArray.push_back(tmpVal); // talletetaan categoria data vain jos siell‰ oli yht‰‰n dataa talletettavaksi
		}
	}

	json_spirit::Object jsonObject; // luodaan aluksi ns. null-objekti
	if(dataArray.size())
	{  // talletetaan CaseStudy-data vain jos oli yht‰‰n categoriaa, jolla talletettiin yht‰‰n dataa
		jsonObject.push_back(json_spirit::Pair(gJsonName_Name, theData.Name()));
		jsonObject.push_back(json_spirit::Pair(gJsonName_Info, theData.Info()));
        jsonObject.push_back(json_spirit::Pair(gJsonName_Path, theData.CaseStudyPath()));
		jsonObject.push_back(json_spirit::Pair(gJsonName_Time, std::string(theData.Time().ToStr("YYYY.MM.DD HH:mm", kEnglish))));
		jsonObject.push_back(json_spirit::Pair(gJsonName_ZipFiles, theData.ZipFiles()));
		jsonObject.push_back(json_spirit::Pair(gJsonName_DeleteTmpFiles, theData.DeleteTmpFiles()));
		jsonObject.push_back(json_spirit::Pair(gJsonName_DataArray, dataArray));
	}
	return jsonObject;
}

bool NFmiCaseStudySystem::AreStoredMetaDataChanged(const NFmiCaseStudySystem &other)
{
	if(itsName != other.itsName)
        return true;
    if(itsInfo != other.itsInfo)
        return true;
    if(itsCaseStudyPath != other.itsCaseStudyPath)
        return true;
    // Huom! aikaa ei t‰ss‰ tarkastella, koska siihen otetaan aina vain nykyinen ajan hetki ja se muuttuisi aina
    if(*fZipFiles != *other.fZipFiles)
        return true;
	if(*fStoreWarningMessages != *other.fStoreWarningMessages)
		return true;
	if(fDeleteTmpFiles != other.fDeleteTmpFiles)
        return true;
    if(itsCategoriesData != other.itsCategoriesData)
        return true;

    return false;
}

#ifdef CreateDirectory // win32/MFC makro pit‰‰ undef:ata, muuten voi tulla ongelmia
#undef CreateDirectory
#endif

static bool DoErrorActions(CWnd *theParentWindow, const std::string &theErrorStr, const std::string &theCaptionStr)
{
    ::MessageBox(theParentWindow ? theParentWindow->GetSafeHwnd() : AfxGetMainWnd()->GetSafeHwnd(), CA2T(theErrorStr.c_str()), CA2T(theCaptionStr.c_str()), MB_OK | MB_ICONWARNING);
	return false;
}

// fMakeFullStore -parametri tarkoittaa ett‰ talletetaanko tiedot myˆs ei talletettavista datoista. Kun tehd‰‰n CaseStudy-dataa, t‰t‰ ei haluta tehd‰,
// mutta kun talletetaan CaseStudy-muistia halutaan myˆs ei talletettujen datojen tiedot talteen.
// Lis‰ksi jos fMakeFullStore on true, k‰ytet‰‰n suoraan theMetaDataTotalFileNameInOut -parametria tallennustiedoston polku+nimen‰,
// muuten talletus polku otetaan Path-metodista.
bool NFmiCaseStudySystem::StoreMetaData(CWnd *theParentWindow, std::string &theMetaDataTotalFileNameInOut, bool fMakeFullStore)
{
	const std::string metaDataFileExtension = "csmeta";
	const std::string metaDataFileExtensionWithDot = "." + metaDataFileExtension;

	std::string pathStr = fMakeFullStore ? NFmiFileSystem::PathFromPattern(theMetaDataTotalFileNameInOut) : CaseStudyPath();
	if(pathStr.empty())
	{
		std::string errStr(::GetDictionaryString("Given path was empty, you must provide absolute path for Case Study data.\nE.g. C:\\data or D:\\data"));
		std::string captionStr(::GetDictionaryString("Case-Study data path was empty"));
		return ::DoErrorActions(theParentWindow, errStr, captionStr);
	}

	NFmiFileString fileStr(pathStr);
	if(fileStr.IsAbsolutePath() == false)
	{
		std::string errStr(::GetDictionaryString("Given path"));
		errStr += "\n";
		errStr += pathStr;
		errStr += "\n";
		errStr += "was not absolute, you must provide absolute path for Case Study data.\nE.g. C:\\data or D:\\data";
		std::string captionStr(::GetDictionaryString("Case-Study data path was not absolute"));
		return ::DoErrorActions(theParentWindow, errStr, captionStr);
	}

	if(fileStr.Extension() == metaDataFileExtension)
	{
		std::string errStr(::GetDictionaryString("Given path "));
		errStr += "\n";
		errStr += pathStr;
		errStr += "\n";
		errStr += "had case-study metadata file extension '";
		errStr += metaDataFileExtension;
		errStr += "', can't allow it,\n";
		errStr += "'because propably last loaded case-study data was just left there";
		std::string captionStr(::GetDictionaryString("Case-Study data path had case-study file extension"));
		return ::DoErrorActions(theParentWindow, errStr, captionStr);
	}

	// Otetaan currentti aika CaseStudy-ajaksi.
	itsTime = NFmiMetTime();
	json_spirit::Object jsonObject = NFmiCaseStudySystem::MakeJsonObject(*this, fMakeFullStore);
	if(jsonObject.size() == 0)
	{
		std::string errStr(::GetDictionaryString("There was nothing to store in selected Case-Study data set."));
		std::string captionStr(::GetDictionaryString("Nothing to store when storing Case-Study data"));
		return ::DoErrorActions(theParentWindow, errStr, captionStr);
	}

	if(NFmiFileSystem::DirectoryExists(pathStr) == false)
	{ // yritet‰‰n luoda polkua
		if(NFmiFileSystem::CreateDirectory(pathStr) == false)
		{
			std::string errStr(::GetDictionaryString("Unable to create directory"));
			errStr += ":\n";
			errStr += pathStr;
			std::string captionStr(::GetDictionaryString("Error when storing Case-Study data"));
			return ::DoErrorActions(theParentWindow, errStr, captionStr);
		}
	}

	std::string totalFileName = (fMakeFullStore ? theMetaDataTotalFileNameInOut : (pathStr + '\\' + Name() + metaDataFileExtensionWithDot));
	std::ofstream out(totalFileName.c_str(), std::ios::binary);
	if(!out)
	{
		std::string errStr(::GetDictionaryString("Unable to create file"));
		errStr += ":\n";
		errStr += totalFileName;
		std::string captionStr(::GetDictionaryString("Error when storing Case-Study data"));
		return ::DoErrorActions(theParentWindow, errStr, captionStr);
	}
	if(fMakeFullStore == false)
		theMetaDataTotalFileNameInOut = totalFileName;

	json_spirit::write(jsonObject, out, json_spirit::pretty_print);

	return true;
}

bool NFmiCaseStudySystem::ReadMetaData(const std::string &theFullPathFileName, CWnd *theParentWindow)
{
    // CaseStudySystem pit‰‰ resetoida ennen kuin aletaan lukemaan uutta tietoa tiedostosta.
    Reset();

	if(theFullPathFileName.empty())
	{
		std::string errStr(::GetDictionaryString("Given Case Study file name was empty, you must provide absolute path and filename for Case Study data.\nE.g. C:\\data\\case1.csmeta or D:\\data\\case1.csmeta"));
		std::string captionStr(::GetDictionaryString("Case-Study data file name was empty"));
		return ::DoErrorActions(theParentWindow, errStr, captionStr);
	}

	if(NFmiFileSystem::FileExists(theFullPathFileName) == false)
	{
		std::string errStr(::GetDictionaryString("Given Case Study file doesn't exist"));
		errStr += ":\n";
		errStr += theFullPathFileName;
		std::string captionStr(::GetDictionaryString("Error when trying to open Case-Study data"));
		return ::DoErrorActions(theParentWindow, errStr, captionStr);
	}

	std::ifstream in(theFullPathFileName.c_str(), std::ios_base::in | std::ios_base::binary);
	if(!in)
	{
		std::string errStr(::GetDictionaryString("Unable to open given Case Study file"));
		errStr += ":\n";
		errStr += theFullPathFileName;
		std::string captionStr(::GetDictionaryString("Error when trying to open Case-Study data"));
		return ::DoErrorActions(theParentWindow, errStr, captionStr);
	}

	json_spirit::Value metaDataValue;
	if(json_spirit::read(in, metaDataValue))
	{
		try
		{
			ParseJsonValue(metaDataValue);
			return true;
		}
		catch(std::exception & /* e */ )
		{
		}
		catch(...)
		{
		}
	}
	return false;
}

void NFmiCaseStudySystem::ParseJsonValue(json_spirit::Value &theValue)
{
	if(theValue.type() == json_spirit::obj_type)
	{
		json_spirit::Object caseStudyObject = theValue.get_obj();
		if(caseStudyObject.size() == 0)
			throw std::runtime_error("Case Study metadata was empty.");

		for(json_spirit::Object::iterator it = caseStudyObject.begin(); it != caseStudyObject.end(); ++it)
		{
			ParseJsonPair(*it);
		}
	}
}

void NFmiCaseStudySystem::ParseJsonPair(json_spirit::Pair &thePair)
{
	// T‰ss‰ puret‰‰n CaseStudySystem luokan p‰‰tason pareja.
	if(thePair.name_ == gJsonName_Name)
		itsName = thePair.value_.get_str();
	else if(thePair.name_ == gJsonName_Info)
		itsInfo = thePair.value_.get_str();
	else if(thePair.name_ == gJsonName_Path)
		; // itsPath = thePair.value_.get_str();
	else if(thePair.name_ == gJsonName_Time)
	{
		std::string tmpTimeStr = thePair.value_.get_str();
		NFmiStringTools::ReplaceAll(tmpTimeStr, " ", ""); // space:t pois
		NFmiStringTools::ReplaceAll(tmpTimeStr, ":", ""); // ':' -merkit pois
		NFmiStringTools::ReplaceAll(tmpTimeStr, ".", ""); // '.' -merkit pois
		NFmiMetTime tmpTime;
		tmpTime.FromStr(tmpTimeStr, kYYYYMMDDHHMM);
		itsTime = tmpTime;
	}
	else if(thePair.name_ == gJsonName_ZipFiles)
		*fZipFiles = thePair.value_.get_bool();
	else if(thePair.name_ == gJsonName_DeleteTmpFiles)
		fDeleteTmpFiles = thePair.value_.get_bool();
	else if(thePair.name_ == gJsonName_DataArray)
		ParseJsonCategoryArray(thePair.value_.get_array());
}

void NFmiCaseStudySystem::ParseJsonCategoryArray(json_spirit::Array &theCategories)
{
	for(json_spirit::Array::iterator it = theCategories.begin(); it != theCategories.end(); ++it)
	{
		NFmiCaseStudyCategoryData categoryData;
		categoryData.ParseJsonValue(*it);
		itsCategoriesData.push_back(categoryData);
	}
}

static std::string GetDirectory(const std::string &theFileFilter)
{
	NFmiFileString fileStr(theFileFilter);
	NFmiString str;
	str += fileStr.Device();
	str += fileStr.Path();

	return std::string(str);
}

std::string NFmiCaseStudySystem::MakeBaseDataDirectory(const std::string& theMetaDataFilePath, const std::string& theCaseStudyName)
{
	std::string basePath = ::NormalizePathDelimiters(::GetDirectory(theMetaDataFilePath));
	std::string dataDir = basePath;
	PathUtils::addDirectorySeparatorAtEnd(dataDir);
	dataDir += theCaseStudyName;
	dataDir += "_data";
	return dataDir;
}

std::string NFmiCaseStudySystem::MakeCaseStudyDataHakeDirectory(const std::string& theBaseCaseStudyDataDirectory)
{
	std::string caseStudyDataHakeDirectory = theBaseCaseStudyDataDirectory;
	PathUtils::addDirectorySeparatorAtEnd(caseStudyDataHakeDirectory);
	caseStudyDataHakeDirectory += "Messages\\HAKE\\";
	return caseStudyDataHakeDirectory;
}

static std::list<std::pair<std::string, std::time_t>> TimeSortFilesDescending(std::list<std::pair<std::string, std::time_t>> filesWithTimes)
{
	filesWithTimes.sort(
		[](const auto &pair1, const auto &pair2) 
		{
			return pair1.second > pair2.second;
		}
	);
	return filesWithTimes;
}

static std::list<std::string> GetFinalFileList(const std::list<std::pair<std::string, std::time_t>>& filesWithTimes)
{
	std::list<std::string> files;
	for(const auto& fileTimePair : filesWithTimes)
		files.push_back(fileTimePair.first);
	return files;
}

// 1. Tekee listan kopioitavista tiedostoista.
// 2. Ottaa huomioon aikarajoitteet.
// 3. Jos endOffset on 0 tai positiivinen, ei ole rajoitusta tiedoston aikaleiman 'uutuuteen' n‰hden, 
// eli silloin kopsataan uusimmat tiedostot, vaikka niiss‰ olisi sein‰kelloa uudempi aika.
// 4. Mukana on koko polku ja tiedoston nimi.
static std::list<std::string> GetWantedFileList(NFmiCaseStudyDataFile& theDataFile)
{
	const auto& dataFileWinRegValues = theDataFile.DataFileWinRegValues();
	if(theDataFile.StoreLastDataOnly() || dataFileWinRegValues.CaseStudyDataCount() == 1)
	{
		std::list<std::string> copyedFileList;
		std::string fileName = NFmiFileSystem::NewestPatternFileName(theDataFile.FileFilter());
		copyedFileList.push_back(fileName);
		return copyedFileList;
	}
	else if(dataFileWinRegValues.CaseStudyDataCount() > 0)
	{
		// Haetaan kaikki tiedostot mit‰ FileFilter:ill‰ lˆytyy (annettu aikaraja 1 on 1970.01.01 00:00:01) ja katsotaan sitten listasta n kpl uusinta
		time_t earliestTimeLimit = 1;
		auto filesWithTimes = NFmiFileSystem::PatternFiles(theDataFile.FileFilter(), earliestTimeLimit);
		if(filesWithTimes.size() <= dataFileWinRegValues.CaseStudyDataCount())
			return ::GetFinalFileList(filesWithTimes);

		// J‰rjestet‰‰n tiedostot ajan suhteen laskevassa j‰rjestyksess‰, jolloin uusimmat ovat listan k‰rjess‰
		auto timeSortedFiles = ::TimeSortFilesDescending(filesWithTimes);
		timeSortedFiles.resize(dataFileWinRegValues.CaseStudyDataCount());
		return ::GetFinalFileList(timeSortedFiles);
	}

	return std::list<std::string>();
}

// t‰m‰ ottaa viimeisen polun osion annetusta polusta. Esim.
// "C:\\data\case1_data" -> "case1_data"
static std::string GetRelativeDataDirectory(const std::string &theDataDir)
{
	std::string tmpStr = ::NormalizePathDelimiters(theDataDir);
	// Pit‰‰ poistaa per‰ss‰ mahdollisesti oleva(t) kenoviiva(t)
	NFmiStringTools::TrimR(tmpStr, '\\'); 
	std::string::size_type pos = tmpStr.find_last_of("\\");
	if(pos != std::string::npos)
	{
		std::string relPath(tmpStr.begin()+pos+1, tmpStr.end());
		return relPath;
	}
	else
		return "";
}

static double DoFixesToGridPointValue(double value, bool horizontalCase, const NFmiGrid& dataGrid)
{
	auto bottomLeftGridPoint = dataGrid.FirstGridPoint();
	auto topRightGridPoint = dataGrid.LastGridPoint();
	if(horizontalCase)
	{
		if(value < bottomLeftGridPoint.X())
			return bottomLeftGridPoint.X();
		if(value > topRightGridPoint.X())
			return topRightGridPoint.X();
	}
	else
	{
		if(value < bottomLeftGridPoint.Y())
			return bottomLeftGridPoint.Y();
		if(value > topRightGridPoint.Y())
			return topRightGridPoint.Y();
	}

	return value;
}

static std::pair<NFmiRect, NFmiGrid> CalcPossibleCropGrid(NFmiAreaFactory::return_type& mapViewArea, const NFmiGrid& dataGrid)
{
	auto gridPointBottomLeft = dataGrid.LatLonToGrid(mapViewArea->BottomLeftLatLon());
	auto gridPointBottomRight = dataGrid.LatLonToGrid(mapViewArea->BottomRightLatLon());
	auto gridPointTopLeft = dataGrid.LatLonToGrid(mapViewArea->TopLeftLatLon());
	auto gridPointTopRight = dataGrid.LatLonToGrid(mapViewArea->TopRightLatLon());
	auto left = std::floor(gridPointBottomLeft.X());
	left = ::DoFixesToGridPointValue(left, true, dataGrid);
	auto bottom = std::floor(gridPointBottomLeft.Y());
	bottom = ::DoFixesToGridPointValue(bottom, false, dataGrid);
	auto right = std::ceil(gridPointTopRight.X());
	right = ::DoFixesToGridPointValue(right, true, dataGrid);
	auto top = std::ceil(gridPointTopRight.Y());
	top = ::DoFixesToGridPointValue(top, false, dataGrid);
	auto width = (right - left) + 1;
	auto height = (top - bottom) + 1;
	auto croppedGridPointArea = width * height;
	auto dataGridArea = static_cast<double>(dataGrid.XNumber() * dataGrid.YNumber());
	auto croppedToOrigGridAreaRatio = croppedGridPointArea / dataGridArea;
	if(croppedToOrigGridAreaRatio > 0 && croppedToOrigGridAreaRatio < 0.8)
	{
		NFmiPoint croppedBottomLeftGridPoint(left, bottom);
		NFmiPoint croppedTopRightGridPoint(right, top);
		auto croppedBottomLeftLatlonPoint = dataGrid.GridToLatLon(croppedBottomLeftGridPoint);
		auto croppedTopRightLatlonPoint = dataGrid.GridToLatLon(croppedTopRightGridPoint);
		auto newCroppedArea = dataGrid.Area()->CreateNewArea(croppedBottomLeftLatlonPoint, croppedTopRightLatlonPoint);
		if(newCroppedArea)
		{
			NFmiRect cropGridPointRect(left, top, right, bottom);
			NFmiGrid croppedGrid(newCroppedArea, static_cast<unsigned long>(width), static_cast<unsigned long>(height));
			return std::make_pair(cropGridPointRect, croppedGrid);
		}
	}

	return std::make_pair(NFmiRect(), NFmiGrid());
}

static std::unique_ptr<NFmiQueryData> CreateCroppedQueryData(NFmiFastQueryInfo &sourceFastInfo, const NFmiRect& croppedGridRect, const NFmiGrid& croppedGrid)
{
	NFmiHPlaceDescriptor hplaceDescriptor(croppedGrid);
	NFmiQueryInfo croppedMetaInfo(sourceFastInfo.ParamDescriptor(), sourceFastInfo.TimeDescriptor(), hplaceDescriptor, sourceFastInfo.VPlaceDescriptor());
	std::unique_ptr<NFmiQueryData> croppedQueryData(NFmiQueryDataUtil::CreateEmptyData(croppedMetaInfo));
	if(croppedQueryData)
	{
		NFmiFastQueryInfo croppedFastInfo(croppedQueryData.get());
		int x1 = static_cast<int>(croppedGridRect.Left());
		int x2 = static_cast<int>(croppedGridRect.Right());
		int y1 = static_cast<int>(croppedGridRect.Top());
		int y2 = static_cast<int>(croppedGridRect.Bottom());
		for(sourceFastInfo.ResetParam(), croppedFastInfo.ResetParam(); sourceFastInfo.NextParam() && croppedFastInfo.NextParam();)
		{
			for(sourceFastInfo.ResetLevel(), croppedFastInfo.ResetLevel(); sourceFastInfo.NextLevel() && croppedFastInfo.NextLevel();)
			{
				for(sourceFastInfo.ResetTime(), croppedFastInfo.ResetTime(); sourceFastInfo.NextTime() && croppedFastInfo.NextTime();)
				{
					NFmiDataMatrix<float> matrix = sourceFastInfo.CroppedValues(x1, y1, x2, y2);
					croppedFastInfo.SetValues(matrix);
				}
			}
		}
	}
	return croppedQueryData;
}

static bool IsAnyAreaCornersInsideOtherArea(const NFmiArea* theArea1, const NFmiArea* theArea2)
{
	if(theArea1 && theArea2)
	{
		if(theArea2->IsInside(theArea1->BottomLeftLatLon()))
			return true;
		if(theArea2->IsInside(theArea1->BottomRightLatLon()))
			return true;
		if(theArea2->IsInside(theArea1->TopLeftLatLon()))
			return true;
		if(theArea2->IsInside(theArea1->TopRightLatLon()))
			return true;
	}
	return false;
}

static bool AreAreasOverlapping(const NFmiArea* theArea1, const NFmiArea* theArea2)
{
	if(theArea1 && theArea2)
	{
		if(::IsAnyAreaCornersInsideOtherArea(theArea1, theArea2))
			return true;
		if(::IsAnyAreaCornersInsideOtherArea(theArea2, theArea1))
			return true;
	}
	return false;
}

static bool CropDataToDestination(const std::string& filePath, const std::string& theDestDir, NFmiAreaFactory::return_type& mapViewArea)
{
	try
	{
		NFmiQueryData sourceData(filePath);
		NFmiFastQueryInfo sourceFastInfo(&sourceData);
		if(sourceFastInfo.IsGrid())
		{
			if(NFmiQueryDataUtil::AreAreasSameKind(sourceFastInfo.Area(), mapViewArea.get()))
			{
				auto cropRectAndGrid = ::CalcPossibleCropGrid(mapViewArea, *sourceFastInfo.Grid());
				if(cropRectAndGrid.first.Width())
				{
					auto croppedQueryData = ::CreateCroppedQueryData(sourceFastInfo, cropRectAndGrid.first, cropRectAndGrid.second);
					if(croppedQueryData)
					{
						NFmiFileString fileString(filePath);
						std::string destFilePath = theDestDir + '\\';
						destFilePath += fileString.FileName();
						croppedQueryData->Write(destFilePath);
						return true;
					}
				}
			}

			if(!::AreAreasOverlapping(sourceFastInfo.Area(), mapViewArea.get()))
			{
				// Jos kartan ja datan alueet eiv‰t ole p‰‰llekk‰in mill‰‰n tavalla, palautetaan t‰ss‰ true, jotta 
				// kyseist‰ dataa ei talleteta 'turhaan' case-studyyn.
				// Esimerkkitapaus: 
				// Maailmadatasta tehd‰‰n Australian alueelle caseStudy-data. Kartalla on latlon-projektio.
				// Halutaan ett‰ euro-alueen stereograafiset (hirlam jne.) datat j‰‰v‰t pois.
				return true;
			}
		}
	}
	catch(...)
	{
	}
	return false;
}

static std::list<std::string> DoCropDataToZoomedAreaOperations(const std::list<std::string>& theCopyedFiles, const std::string& theDestDir, const std::string& theCropDataAreaString)
{
	std::list<std::string> remainingFiles = theCopyedFiles;
	if(!theCropDataAreaString.empty())
	{
		try
		{
			auto mapViewArea = NFmiAreaFactory::Create(theCropDataAreaString);
			if(mapViewArea)
			{
				for(const auto &filePath : theCopyedFiles)
				{
					if(CropDataToDestination(filePath, theDestDir, mapViewArea))
						remainingFiles.remove(filePath);
				}
			}
		}
		catch(...)
		{ }
	}
	return remainingFiles;
}


#ifdef CopyFile // winkkari makro pit‰‰ disabloida ensin ennen kuin voi k‰ytt‰‰ NFmiFileSystem:in FileCopy-funktiota
#undef CopyFile
#endif 

// 1. kopioi annetut tiedostot theDestDir-hakemistoon.
// 2. Pit‰‰kˆ muokata NFmiCaseStudyDataFile:en uusi suhteellinen polku, ett‰ myˆhemmin sit‰ voidaan k‰ytt‰‰ kun dataa ladataan katsottavaksi?
static void CopyFilesToDestination(const std::list<std::string> &theCopyedFiles, const std::string &theDestDir, int theProgressDialogMaxCount, int &theProgressCounter, CWnd *theCopyWindowPos, const std::string& theCropDataAreaString)
{
	auto remainingCopyedFiles = theCopyedFiles;
	if(!theCropDataAreaString.empty())
		remainingCopyedFiles = ::DoCropDataToZoomedAreaOperations(remainingCopyedFiles, theDestDir, theCropDataAreaString);
	if(remainingCopyedFiles.size() == 0)
		return ;

	CShellFileOp sfo;
	BOOL bAPICalled = FALSE;
	int nAPIReturnVal = 0;
	for(std::list<std::string>::const_iterator it = remainingCopyedFiles.begin(); it != remainingCopyedFiles.end(); ++it)
	{
        sfo.AddSourceFile(CA2T((*it).c_str()));
	}

    sfo.AddDestFile(CA2T(theDestDir.c_str()));

	// Set up a few flags that control the operation.
    sfo.SetOperationFlags( 
	  FO_COPY,			// the operation type (copy in this case)
	  theCopyWindowPos ? theCopyWindowPos : AfxGetMainWnd(),	// pointer to parent window
      FALSE,			// flag - silent mode?
      FALSE,			// flag - allow undo?
      FALSE,			// flag - should wild cards affect files only?
      TRUE,				// flag - suppress confirmation messages?
      TRUE,				// flag - suppress confirmation messages when making directories?
      FALSE,			// flag - rename files when name collisions occur?
      FALSE				// flag - simple progress dialog?
	  );

	std::string progresstitleStr(::GetDictionaryString("Case Study copy"));
	progresstitleStr += " (";
	progresstitleStr += NFmiStringTools::Convert(theProgressCounter);
	progresstitleStr += "/";
	progresstitleStr += NFmiStringTools::Convert(theProgressDialogMaxCount);
	progresstitleStr += ")\0\0";
    sfo.SetProgressDlgTitle(CA2T(progresstitleStr.c_str())); // toimii vain simple progress dialog -flagi p‰‰ll‰

	BOOL anyOperationsAborted = FALSE;
    // Start the operation.
	if(sfo.Go(&bAPICalled, &nAPIReturnVal, &anyOperationsAborted))
	{
		// The operation succeeded!
	}
	else
	{
		if ( !bAPICalled )
		{
			// SHFileOperation() wasn't called - check the info you passed
			// in to the CShellFileOp object.  The DEBUG version will
			// throw ASSERTs and/or show TRACE messages to help you out.
		}
		else
		{
			// SHFileOperation() returned nonzero (failure).  That return
			// value is now in nAPIReturnVal.
		}
	}
	if(anyOperationsAborted)
	{
		// copy-dialogille on sanottu Cancel, kysyt‰‰n, halutaanko koko operaatio keskeytt‰‰ kokonaisuudessaa...
		std::string questionStr(::GetDictionaryString("Do you want to cancel the whole CaseStudy operation?"));;
		questionStr += "\n\n";
		questionStr += ::GetDictionaryString("Press Yes if you want to cancel the whole\nCaseStudy-data making operation.");
		questionStr += "\n\n";
		questionStr += ::GetDictionaryString("Press No if you want to continue the operation.");
		std::string titleStr(::GetDictionaryString("Cancel the whole CaseStudy operation?"));
        int result = ::MessageBox(::GetDesktopWindow(), CA2T(questionStr.c_str()), CA2T(titleStr.c_str()), MB_YESNO | MB_ICONWARNING | MB_SETFOREGROUND);
		if(result == IDYES) // operaatio keskeytetaan kokonaan, jos on painettu Yes-nappia
			throw CaseStudyOperationCanceledException();
	}

	theProgressCounter++;
}

static std::string GetDataFileDir(const std::string &theBaseDir, NFmiCaseStudyDataFile &theDataFile)
{
	std::string usedDir = theBaseDir;
	if(theDataFile.ImageFile())
	{ // kuville tehd‰‰n viel‰ omat kanava/data kohtaiset hakemistot
		usedDir += "\\";
		usedDir += theDataFile.ImageParam().GetName();
	}
	return usedDir;
}

static void StoreFileData(const std::string &theProducerDir, const std::string &theRelativeProducerDir, NFmiCaseStudyDataFile &theDataFile, int theProgressDialogMaxCount, int &theProgressCounter, CWnd *theCopyWindowPos, const std::string& theCropDataAreaString)
{
	std::string usedDestDir = ::GetDataFileDir(theProducerDir, theDataFile);
	std::string usedRelativeDestDir = ::GetDataFileDir(theRelativeProducerDir, theDataFile);
	usedRelativeDestDir += "\\";
	usedRelativeDestDir += ::GetRelativeDataDirectory(theDataFile.FileFilter()); // k‰ytet‰‰n taas GetRelativeDataDirectory funktiota ett‰ saadaan k‰ytetty tiedosto-filter -loppuosio talteen
	theDataFile.RelativeStoredFileFilter(usedRelativeDestDir);

	if(NFmiFileSystem::CreateDirectory(usedDestDir))
	{
		std::list<std::string> copyedFiles = ::GetWantedFileList(theDataFile);
		::CopyFilesToDestination(copyedFiles, usedDestDir, theProgressDialogMaxCount, theProgressCounter, theCopyWindowPos, theCropDataAreaString);
	}
}

static void AddDataFilesToList(const std::string &theProducerDirIn, const std::string &theRelativeProducerDirIn, NFmiCaseStudyDataFile &theDataFileInOut, std::list<std::string> &theCopyedFilesInOut)
{
	std::string usedDestDir = ::GetDataFileDir(theProducerDirIn, theDataFileInOut);
	std::string usedRelativeDestDir = ::GetDataFileDir(theRelativeProducerDirIn, theDataFileInOut);
	usedRelativeDestDir += "\\";
	usedRelativeDestDir += ::GetRelativeDataDirectory(theDataFileInOut.FileFilter()); // k‰ytet‰‰n taas GetRelativeDataDirectory funktiota ett‰ saadaan k‰ytetty tiedosto-filter -loppuosio talteen
	theDataFileInOut.RelativeStoredFileFilter(usedRelativeDestDir);

	std::list<std::string> copyedFiles = ::GetWantedFileList(theDataFileInOut);
	theCopyedFilesInOut.insert(theCopyedFilesInOut.end(), copyedFiles.begin(), copyedFiles.end());
}

static void AddProducerDataFilesToList(const std::string &theCategoryDirIn, const std::string &theRelativeCategoryDirIn, NFmiCaseStudyProducerData &theProducerData, std::list<std::string> &theCopyedFilesInOut)
{
	std::vector<NFmiCaseStudyDataFile> &fileDataVec = theProducerData.FilesData();
	for(size_t i=0; i < fileDataVec.size(); i++)
	{
		if(fileDataVec[i].DataEnabled()) // jos data on disabloitu smartmetin konffeissa, sit‰ ei myˆsk‰‰n yritet‰ ottaa mukaan case-study -dataan
			::AddDataFilesToList(theCategoryDirIn, theRelativeCategoryDirIn, fileDataVec[i], theCopyedFilesInOut);
	}
}

static std::string GetProducerDir(const std::string &theBaseDir, NFmiCaseStudyProducerData &theProducerData)
{
	std::string producerDir = theBaseDir;
	producerDir += "\\";
	producerDir += theProducerData.ProducerHeaderInfo().Name();
	return producerDir;
}

static void StoreProducerData(const std::string &theCategoryDir, const std::string &theRelativeCategoryDir, NFmiCaseStudyProducerData &theProducerData, int theProgressDialogMaxCount, int &theProgressCounter, CWnd *theCopyWindowPos, const std::string& theCropDataAreaString)
{
	std::string producerDir = ::GetProducerDir(theCategoryDir, theProducerData);
	std::string relativeProducerDir = ::GetProducerDir(theRelativeCategoryDir, theProducerData);
	if(NFmiFileSystem::CreateDirectory(producerDir))
	{
		bool imageFiles = false;
		std::list<std::string> copyedFiles;
		std::vector<NFmiCaseStudyDataFile> &fileDataVec = theProducerData.FilesData();
		for(size_t i=0; i < fileDataVec.size(); i++)
		{
			NFmiCaseStudyDataFile &dataFile = fileDataVec[i];
			if(dataFile.DataEnabled()) // jos data on disabloitu smartmetin konffeissa, sit‰ ei myˆsk‰‰n yritet‰ ottaa mukaan case-study -dataan
			{
				if(dataFile.ImageFile())
				{
					::StoreFileData(producerDir, relativeProducerDir, dataFile, theProgressDialogMaxCount, theProgressCounter, theCopyWindowPos, theCropDataAreaString);
					imageFiles = true;
				}
				else
					::AddDataFilesToList(producerDir, relativeProducerDir, dataFile, copyedFiles);
			}
		}
		if(imageFiles == false)
			::CopyFilesToDestination(copyedFiles, producerDir, theProgressDialogMaxCount, theProgressCounter, theCopyWindowPos, theCropDataAreaString);
	}
}

static std::string GetCategoryDir(const std::string &theBaseDir, NFmiCaseStudyCategoryData &theCategory)
{
	std::string categoryDir = theBaseDir;
	categoryDir += "\\";
	categoryDir += theCategory.CategoryHeaderInfo().Name();
	return categoryDir;
}

static void StoreCategoryData(const std::string &theDataDir, const std::string &theRelativeDataDir, NFmiCaseStudyCategoryData &theCategory, int theProgressDialogMaxCount, int &theProgressCounter, CWnd *theCopyWindowPos, const std::string& theCropDataAreaString)
{
	std::string categoryDir = ::GetCategoryDir(theDataDir, theCategory);
	std::string relativeCategoryDir = ::GetCategoryDir(theRelativeDataDir, theCategory);
	if(NFmiFileSystem::CreateDirectory(categoryDir))
	{
		bool storeLastDataOnlyCategory = theCategory.CategoryHeaderInfo().StoreLastDataOnly();
		std::list<std::string> copyedFiles;
		auto &producersData = theCategory.ProducersData();
		for(auto & producerData : producersData)
		{
			if(storeLastDataOnlyCategory)
				::AddProducerDataFilesToList(categoryDir, relativeCategoryDir, producerData, copyedFiles);
			else
				::StoreProducerData(categoryDir, relativeCategoryDir, producerData, theProgressDialogMaxCount, theProgressCounter, theCopyWindowPos, theCropDataAreaString);
		}
		if(storeLastDataOnlyCategory)
			::CopyFilesToDestination(copyedFiles, categoryDir, theProgressDialogMaxCount, theProgressCounter, theCopyWindowPos, theCropDataAreaString);
	}
}

int NFmiCaseStudySystem::CalculateProgressDialogCount(void) const
{
	int counter = 0;
	for(const auto &categoryData : itsCategoriesData)
	{
		const auto &producersData = categoryData.ProducersData();
		for(const auto & producerData : producersData)
		{
			bool storeLastDataOnlyCategory = categoryData.CategoryHeaderInfo().StoreLastDataOnly();
			if(producerData.FilesData().size())
			{
				const auto &fileDataVec = producerData.FilesData();
				bool imageProducer = fileDataVec[0].ImageFile();
				for(const auto &fileData : fileDataVec)
				{
					if(fileData.DataFileWinRegValues().Store())
					{
						counter++;
						if(imageProducer == false)
							break; // tullaan ulos jos ei ole imageTuottaja, koska silloin on vain yksi kopiointi
						if(storeLastDataOnlyCategory)
							goto storeLastDataOnlyBailOut; // t‰m‰ koodi oli yksinkertaisin goto-lausekkeen avulla, jos kyse on analyysi/havainto datasta, tehd‰‰n kyseisille datoille kerta kopio koko kategorialle
					}
				} // fileVec -for
			}
		} // producerVec -for
storeLastDataOnlyBailOut: ;
	}
	return counter;
}

// t‰st‰ tulee joskus varmaan erillinen funktio, mutta nyt teen CaseStudy-datan teko funktiota. 
// Sen teht‰v‰t ovat:
// 1. Lue annettu metadata-tiedosto ja tee sen avulla CaseStudy-tietorakenteet.
// 2. Tee annettuun polkuun hakemisto CaseStudyName"_data"
// 3. Tee sen alle hakemisto jokaiselle kategorialle
// 4. Tee kategoria-hakemistojen alle jokaiselle tuottajalle oma hakemisto
// 5. Tee image datoille viel‰ jokaiselle parametrille oma hakemisto
// 6. Kopioi halutut datat omiin hakemistoihinha annetuista poluista annetuilla fileFiltereill‰ ja aikav‰lill‰
// HUOM! Voi heitt‰‰ CaseStudyOperationCanceledException -poikkeuksen!!!
bool NFmiCaseStudySystem::MakeCaseStudyData(const std::string &theFullPathMetaDataFileName, CWnd *theParentWindow, CWnd *theCopyWindowPos, const std::string& theCropDataAreaString)
{
	if(ReadMetaData(theFullPathMetaDataFileName, theParentWindow))
	{
		int progressDialogMaxCount = CalculateProgressDialogCount();
		int progressCounter = 1;
		std::string dataDir = NFmiCaseStudySystem::MakeBaseDataDirectory(theFullPathMetaDataFileName, Name());
		std::string relativeDataDir = ::GetRelativeDataDirectory(dataDir);
		if(NFmiFileSystem::CreateDirectory(dataDir))
		{
			for(size_t i=0; i < itsCategoriesData.size(); i++)
				::StoreCategoryData(dataDir, relativeDataDir, itsCategoriesData[i], progressDialogMaxCount, progressCounter, theCopyWindowPos, theCropDataAreaString);
		}

		// TODO pit‰‰kˆ metadata tallettaa nyt uusilla poluilla?
		std::string dummyMetaFileName; // t‰ll‰ ei tee mit‰‰n t‰ss‰, mutta pit‰‰ antaan funktiolle
		StoreMetaData(theParentWindow, dummyMetaFileName, false); // tehd‰‰n uudelleen talletus p‰ivitetyill‰ relatiivisilla poluilla
		// TODO Ent‰ mist‰ tiet‰‰ ett‰ onko metadataan jo laitettu uudet suhteelliset polut? (tai pit‰‰kˆ edes tiet‰‰)
		// TODO pit‰isikˆ tehd‰ kaksi metadatatiedostoa, joissa toisessa olisi orig-tiedot ja toisessa p‰ivitetyt polut?
	}
	return false;
}

// Tekee nyt ladattuna olevasta caseStudy-datasta NFmiHelpDataInfoSystem -olion. Staattinen data-osio annetaan argumenttina, 
// koska niit‰ ei talleteta CaseStudy-dataan.
boost::shared_ptr<NFmiHelpDataInfoSystem> NFmiCaseStudySystem::MakeHelpDataInfoSystem(NFmiHelpDataInfoSystem &theOriginalHelpDataInfoSystem, const std::string &theBasePath)
{
	boost::shared_ptr<NFmiHelpDataInfoSystem> helpDataInfoSystem;
	if(itsCategoriesData.size())
	{
		const std::vector<NFmiHelpDataInfo> &staticHelpDataInfos =  theOriginalHelpDataInfoSystem.StaticHelpDataInfos();
		helpDataInfoSystem = boost::shared_ptr<NFmiHelpDataInfoSystem>(new NFmiHelpDataInfoSystem());
		for(size_t i = 0; i < staticHelpDataInfos.size(); i++)
			helpDataInfoSystem->AddStatic(staticHelpDataInfos[i]);

		for(size_t i = 0; i < itsCategoriesData.size(); i++)
			itsCategoriesData[i].AddDataToHelpDataInfoSystem(helpDataInfoSystem, theBasePath, theOriginalHelpDataInfoSystem);
	}
	return helpDataInfoSystem;
}

NFmiCaseStudyDataFile* NFmiCaseStudySystem::FindCaseStudyDataFile(const std::string& theUniqueHelpDataInfoName)
{
	if(!theUniqueHelpDataInfoName.empty())
	{
		for(auto& categoryData : itsCategoriesData)
		{
			for(auto& producerData : categoryData.ProducersData())
			{
				for(auto& dataFile : producerData.FilesData())
				{
					if(dataFile.HelpDataInfoName() == theUniqueHelpDataInfoName)
						return &dataFile;
				}
			}
		}
	}
	return nullptr;
}

std::string NFmiCaseStudySystem::MakeCaseStudyFilePattern(const std::string &theFilePattern, const std::string &theBasePath, bool fMakeOnlyPath)
{
	std::string patternWithoutPath = NFmiFileSystem::FileNameFromPath(theFilePattern);
	if(patternWithoutPath.empty() == false)
	{
		for(auto categoryData : itsCategoriesData)
		{
			auto &producersData = categoryData.ProducersData();
			for(auto &producerData : producersData)
			{
				auto &dataFileVector = producerData.FilesData();
				for(auto & dataFile : dataFileVector)
				{
					auto pos = MacroParam::ci_find_substr(dataFile.RelativeStoredFileFilter(), patternWithoutPath); // t‰m‰ etsint‰ tyˆ pit‰‰ tehd‰ case-insensitiivisti, tai muuten kaikkien eri konffeissa olevien juttujen pit‰‰ olla tarkalleen samalla lailla kirjoitettu
					if(pos != MacroParam::ci_string_not_found)
					{ // lˆytyi haluttu data, rakenna nyt siihen sopiva file-filter
						std::string newFilePattern = theBasePath;
						newFilePattern += dataFile.RelativeStoredFileFilter();
						if(fMakeOnlyPath)
							return NFmiFileSystem::PathFromPattern(newFilePattern);
						else
							return newFilePattern;
					}
				}
			}
		}
	}
	return "";
}

// Asettaa annettuun dataloadingInfoon kaikki tavittavat CaseStudy-datoihin viittaavat polut ja muut kuntoon, ett‰
// CaseStudy-moodissa voidaan ladata editoitavaa dataa halutulla tavalla.
void NFmiCaseStudySystem::SetUpDataLoadinInfoForCaseStudy(NFmiDataLoadingInfo &theDataLoadingInfo, const std::string &theBasePath)
{
	theDataLoadingInfo.UseDataCache(false); // case-studyssa dataa ei lueta cachesta, vaan case-studyn omista data hakemistoista

	std::string model1FilePattern = theDataLoadingInfo.Model1FilePattern();
	std::string model1NewFilePattern = MakeCaseStudyFilePattern(model1FilePattern, theBasePath, false);
	theDataLoadingInfo.Model1FilePattern(model1NewFilePattern);

	std::string model2FilePattern = theDataLoadingInfo.Model2FilePattern();
	std::string model2NewFilePattern = MakeCaseStudyFilePattern(model2FilePattern, theBasePath, false);
	theDataLoadingInfo.Model2FilePattern(model2NewFilePattern);

	std::string dataBaseInPattern = theDataLoadingInfo.DataBaseFileNameIn();
	std::string dataBaseInNewPattern = MakeCaseStudyFilePattern(dataBaseInPattern, theBasePath, true);
	theDataLoadingInfo.DataBaseInPath(dataBaseInNewPattern);

	theDataLoadingInfo.InitFileNameLists();
	theDataLoadingInfo.CaseStudyTime(Time());
	theDataLoadingInfo.UpdatedTimeDescriptor();
}

std::string NFmiCaseStudySystem::CaseStudyPath(void) const
{
    return *itsCaseStudyPath;
}

void NFmiCaseStudySystem::CaseStudyPath(const std::string &newValue)
{
    *itsCaseStudyPath = newValue;
}

bool NFmiCaseStudySystem::ZipFiles(void) const 
{ 
    return *fZipFiles; 
}

void NFmiCaseStudySystem::ZipFiles(bool newValue) 
{ 
    *fZipFiles = newValue; 
}

bool NFmiCaseStudySystem::StoreWarningMessages(void) const
{
	return *fStoreWarningMessages;
}

void NFmiCaseStudySystem::StoreWarningMessages(bool newValue)
{
	*fStoreWarningMessages = newValue;
}

bool NFmiCaseStudySystem::CropDataToZoomedMapArea(void) const
{
	return *fCropDataToZoomedMapArea;
}

void NFmiCaseStudySystem::CropDataToZoomedMapArea(bool newValue)
{
	*fCropDataToZoomedMapArea = newValue;
}

// ************************************************************
// *****   NFmiCaseStudySystem loppuu  ************************
// ************************************************************
