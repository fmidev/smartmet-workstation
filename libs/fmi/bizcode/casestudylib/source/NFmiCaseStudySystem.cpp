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
static const std::string gJsonName_StartOffsetInMinutes = "StartOffsetInMinutes";
static const std::string gJsonName_EndOffsetInMinutes = "EndOffsetInMinutes";
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

// ************************************************************
// *****   NFmiCaseStudyDataFile alkaa  ***********************
// ************************************************************

NFmiCaseStudyDataFile::NFmiCaseStudyDataFile(void)
:itsName()
,itsProducer()
,itsFileFilter()
,itsRelativeStoredFileFilter()
,itsStartOffsetInMinutes(0)
,itsEndOffsetInMinutes(0)
,itsDataIntervalInMinutes(0)
,itsSingleFileSize(0)
,itsTotalFileSize(0)
,itsMaxFileSize(0)
,itsCategory(kFmiCategoryError)
,itsDataType(NFmiInfoData::kNoDataType)
,itsLevelCount(0)
,itsImageAreaStr()
,itsImageParam()
,fStore(false)
,fImageFile(false)
,fStoreLastDataOnly(false)
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
	auto infoVector = theInfoOrganizer.GetInfos(theFileNameFilter);
	if(infoVector.size())
		return infoVector[0];
	else
		return boost::shared_ptr<NFmiFastQueryInfo>();
}

static bool StoreLastDataOnly(const NFmiHelpDataInfo &theDataInfo)
{
	if(theDataInfo.IsCombineData())
		return true;
	NFmiInfoData::Type dataType = theDataInfo.DataType();
	if(dataType == NFmiInfoData::kAnalyzeData || dataType == NFmiInfoData::kObservations || dataType == NFmiInfoData::kFlashData || dataType == NFmiInfoData::kSingleStationRadarData || dataType == NFmiInfoData::kTrajectoryHistoryData)
		return true;
	else
		return false;
}

int NFmiCaseStudyDataFile::GetModelRunTimeGapInMinutes(NFmiQueryInfo *theInfo, NFmiInfoData::Type theType, NFmiHelpDataInfo *theHelpDataInfo)
{
    // Pakko laittaa osoittamaan 1. parametriiin, koska t‰nne tulee aito NFmiQueryInfo olio, ja se ei osaa hakea 1. Producer:ia automaattisesti.
    theInfo->FirstParam();

	if(theHelpDataInfo && theHelpDataInfo->NonFixedTimeGab())
		return -1; // Jos datan konffissa on erikseen m‰‰r‰tty m‰‰rittelem‰tˆn aikav‰li, niin k‰ytet‰‰n sit‰.
	if(theType == NFmiInfoData::kKepaData)
		return -1; // editoidulla datalla ei ole vakio ajov‰li‰, niit‰ editoidaan ja l‰hetet‰‰n milloin vain
	if(theHelpDataInfo && theHelpDataInfo->ModelRunTimeGapInHours())
		return static_cast<int>(std::round(theHelpDataInfo->ModelRunTimeGapInHours() * 60)); // Jos datan konffissa on erikseen m‰‰r‰tty malliajov‰li, niin k‰ytet‰‰n sit‰.

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
	if(fStoreLastDataOnly)
		return itsSingleFileSize;
	else
	{
		if(itsDataIntervalInMinutes == -1)
			itsDataIntervalInMinutes = 180; // tehd‰‰n sellainen olettamus ett‰ editoiduille datoille eli ep‰m‰'‰r‰inen ilmestymis aika (= -1) dataa oletetaan tulevan kerran 3 tunnissa
		if((itsEndOffsetInMinutes < itsStartOffsetInMinutes) || itsDataIntervalInMinutes <= 0)
			return 0; // t‰m‰ on virhetilanne
		else
		{
			int timeDiffInMinutes = itsEndOffsetInMinutes - itsStartOffsetInMinutes;
			int dataCount = static_cast<int>(::round(timeDiffInMinutes / static_cast<double>(itsDataIntervalInMinutes)));
			if(dataCount == 0)
				dataCount = 1;
			return dataCount * itsSingleFileSize;
		}
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

bool NFmiCaseStudyDataFile::Init(NFmiHelpDataInfoSystem &theDataInfoSystem, const NFmiHelpDataInfo &theDataInfo, NFmiInfoOrganizer &theInfoOrganizer, int theStartOffsetInMinutes, int theEndOffsetInMinutes, bool isStored, const NFmiCaseStudySystem &theCaseStudySystem)
{
	Reset();

	fStore = isStored;
	itsStartOffsetInMinutes = theStartOffsetInMinutes;
	itsEndOffsetInMinutes = theEndOffsetInMinutes;
	itsCategory = GetDataCategory(theDataInfo);
	itsDataType = theDataInfo.DataType();
	fNotifyOnLoad = theDataInfo.NotifyOnLoad();
	itsNotificationLabel = theDataInfo.NotificationLabel();
	itsCustomMenuFolder = theDataInfo.CustomMenuFolder();
	itsAdditionalArchiveFileCount = theDataInfo.AdditionalArchiveFileCount();
	fDataEnabled = theDataInfo.IsEnabled();


	if(theDataInfo.DataType() == NFmiInfoData::kSatelData)
	{ // kyse on kuva datasta
		itsFileFilter = theDataInfo.FileNameFilter(); // kuvia ei viel‰ cacheteta, joten pit‰‰ pyyt‰‰ 'originaali' polkua, ei UsedFileNameFilter -polkua
		itsName = theDataInfo.ImageDataIdent().GetParamName();
		itsProducer = *(theDataInfo.ImageDataIdent().GetProducer());
		fImageFile = true;
		fStoreLastDataOnly = false;
		itsDataIntervalInMinutes = ::GetImageGapInMinutes(theDataInfo);
		itsSingleFileSize = static_cast<double>(::FastFileByteCount(itsFileFilter, theCaseStudySystem));
		itsTotalFileSize = EvaluateTotalDataSize();
		itsLevelCount = 1;
		if(theDataInfo.ImageArea())
			itsImageAreaStr = theDataInfo.ImageArea()->AreaStr();
		itsImageParam = NFmiParam(*(theDataInfo.ImageDataIdent().GetParam()));

		return true;
	}
	else
	{ // kyse on queryDatasta
		if(theDataInfo.IsCombineData())
			itsFileFilter = theDataInfo.CombineDataPathAndFileName(); // yhdistelm‰ datoille otetaan yhdistelm‰n fileFilter
		else
			itsFileFilter = theDataInfo.UsedFileNameFilter(theDataInfoSystem);

		itsName = theDataInfo.GetCleanedName();
		fImageFile = false;
		fStoreLastDataOnly = ::StoreLastDataOnly(theDataInfo);
//		itsSingleFileSize = static_cast<double>(::LatestFileByteCount(itsFileFilter));
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
	jsonObject.push_back(json_spirit::Pair(gJsonName_StartOffsetInMinutes, theData.StartOffsetInMinutes()));
	jsonObject.push_back(json_spirit::Pair(gJsonName_EndOffsetInMinutes, theData.EndOffsetInMinutes()));
//	int itsDataIntervalInMinutes;	// ei ole hyˆty‰ tallettaa case-study metadataan? johdettavissa?
//	double itsSingleFileSize;		// ei ole hyˆty‰ tallettaa case-study metadataan? johdettavissa?
//	double itsTotalFileSize;		// ei ole hyˆty‰ tallettaa case-study metadataan? johdettavissa?
//	double itsMaxFileSize;			// ei ole hyˆty‰ tallettaa case-study metadataan? johdettavissa?
//	DataCategory itsCategory;		// ei ole hyˆty‰ tallettaa case-study metadataan? johdettavissa?
	jsonObject.push_back(json_spirit::Pair(gJsonName_DataType, static_cast<int>(theData.itsDataType)));
//	int itsLevelCount;				// ei ole hyˆty‰ tallettaa case-study metadataan? johdettavissa?
	jsonObject.push_back(json_spirit::Pair(gJsonName_Store, theData.Store()));
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
		itsFileFilter = thePair.value_.get_str();
		NFmiStringTools::ReplaceChars(itsFileFilter, '/', '\\'); // muutetaan luetut polut niin ett‰ SHFileOperation-funktio ymm‰rt‰‰ ne varmasti
	}
	else if(thePair.name_ == gJsonName_RelativeStoredFileFilter)
	{
		itsRelativeStoredFileFilter = thePair.value_.get_str();
		NFmiStringTools::ReplaceChars(itsFileFilter, '/', '\\'); // muutetaan luetut polut niin ett‰ SHFileOperation-funktio ymm‰rt‰‰ ne varmasti
	}
	else if(thePair.name_ == gJsonName_StartOffsetInMinutes)
		itsStartOffsetInMinutes = thePair.value_.get_int();
	else if(thePair.name_ == gJsonName_EndOffsetInMinutes)
		itsEndOffsetInMinutes = thePair.value_.get_int();
	else if(thePair.name_ == gJsonName_DataType)
		itsDataType = static_cast<NFmiInfoData::Type>(thePair.value_.get_int());
	else if(thePair.name_ == gJsonName_Store)
		fStore = thePair.value_.get_bool();
	else if(thePair.name_ == gJsonName_ImageFile)
		fImageFile = thePair.value_.get_bool();
	else if(thePair.name_ == gJsonName_StoreLastDataOnly)
		fStoreLastDataOnly = thePair.value_.get_bool();
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

void NFmiCaseStudyDataFile::AddDataToHelpDataInfoSystem(boost::shared_ptr<NFmiHelpDataInfoSystem> &theHelpDataInfoSystem, const std::string &theBasePath)
{
	NFmiHelpDataInfo helpDataInfo;
	helpDataInfo.FileNameFilter(theBasePath + RelativeStoredFileFilter(), true); // true = fiksataan filefilter--polku
	helpDataInfo.DataType(itsDataType);
	helpDataInfo.Name(Name());
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

	theHelpDataInfoSystem->AddDynamic(helpDataInfo);
}

bool NFmiCaseStudyDataFile::FindSuitableCaseStudyTime(NFmiMetTime &theTimeOut)
{
	if(Store() && OnlyOneData() == false) // jos data on tarkoitus tallettaa ja se ei ole "vain viimeinen  data talletetaan" -tyyppi‰
	{
		if(StartOffsetInMinutes() < 0) // jos alkuajan offset on pienempi kuin 0
		{
			NFmiMetTime aTime(1);
			aTime.ChangeByMinutes(StartOffsetInMinutes());
			aTime.SetTimeStep(60);
			theTimeOut = aTime;
			return true;
		}
	}
	return false;
}

// otetaan talteen vain offsetit ja store
void NFmiCaseStudyDataFile::InitDataWithStoredSettings(NFmiCaseStudyDataFile &theOriginalDataFile)
{
	StartOffsetInMinutes(theOriginalDataFile.StartOffsetInMinutes());
	EndOffsetInMinutes(theOriginalDataFile.EndOffsetInMinutes());
	Store(theOriginalDataFile.Store());
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
    if(itsStartOffsetInMinutes != other.itsStartOffsetInMinutes)
        return false;
    if(itsEndOffsetInMinutes != other.itsEndOffsetInMinutes)
        return false;
    if(itsCategory != other.itsCategory)
        return false;
    if(itsDataType != other.itsDataType)
        return false;
    if(itsImageAreaStr != other.itsImageAreaStr)
        return false;
    if(itsImageParam != other.itsImageParam)
        return false;
    if(fStore != other.fStore)
        return false;
    if(fImageFile != other.fImageFile)
        return false;
    if(fStoreLastDataOnly != other.fStoreLastDataOnly)
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

void NFmiCaseStudyProducerData::OrganizeDatas(void)
{
	SortDatas();
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
		if(fileData.Store())
			totalFileSize += fileData.TotalFileSize();
	}
	ProducerHeaderInfo().TotalFileSize(totalFileSize);
	ProducerHeaderInfo().MaxFileSize(maxFileSize);
}

void NFmiCaseStudyProducerData::ProducerStore(bool newValue)
{
	ProducerHeaderInfo().Store(newValue);
	for(size_t i = 0; i < itsFilesData.size(); i++)
	{
		itsFilesData[i].Store(newValue);
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

void NFmiCaseStudyProducerData::ProducerOffset(bool startOffset, int theOffsetInMinutes)
{
	if(startOffset)
		ProducerHeaderInfo().StartOffsetInMinutes(theOffsetInMinutes);
	else
		ProducerHeaderInfo().EndOffsetInMinutes(theOffsetInMinutes);
	for(size_t i = 0; i < itsFilesData.size(); i++)
	{
		if(startOffset)
			itsFilesData[i].StartOffsetInMinutes(theOffsetInMinutes);
		else
			itsFilesData[i].EndOffsetInMinutes(theOffsetInMinutes);
	}
}

void NFmiCaseStudyProducerData::SortDatas(void)
{
	// prioriteetti j‰rjestys?
}

json_spirit::Object NFmiCaseStudyProducerData::MakeJsonObject(const NFmiCaseStudyProducerData &theData, bool fMakeFullStore)
{
	json_spirit::Array dataArray;
	const std::vector<NFmiCaseStudyDataFile> &dataVector = theData.FilesData();
	for(size_t i = 0; i < dataVector.size(); i++)
	{
		if(fMakeFullStore || dataVector[i].Store())
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

void NFmiCaseStudyProducerData::AddDataToHelpDataInfoSystem(boost::shared_ptr<NFmiHelpDataInfoSystem> &theHelpDataInfoSystem, const std::string &theBasePath)
{
	for(size_t i = 0; i < itsFilesData.size(); i++)
		itsFilesData[i].AddDataToHelpDataInfoSystem(theHelpDataInfoSystem, theBasePath);
}

bool NFmiCaseStudyProducerData::FindSuitableCaseStudyTime(NFmiMetTime &theTimeOut)
{
	for(size_t i = 0; i < itsFilesData.size(); i++)
	{
		if(itsFilesData[i].FindSuitableCaseStudyTime(theTimeOut))
			return true;
	}
	return false;
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
	ProducerHeaderInfo().StartOffsetInMinutes(theOriginalProducerData.ProducerHeaderInfo().StartOffsetInMinutes());
	ProducerHeaderInfo().EndOffsetInMinutes(theOriginalProducerData.ProducerHeaderInfo().EndOffsetInMinutes());
	ProducerHeaderInfo().Store(theOriginalProducerData.ProducerHeaderInfo().Store());

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

NFmiCaseStudyCategoryData::NFmiCaseStudyCategoryData(const std::string &theName, NFmiCaseStudyDataFile::DataCategory theCategory, bool theLatestDataOnly)
:itsCategoryHeaderInfo()
,itsProducersData()
{
	CategoryHeaderInfo().Name(theName);
	CategoryHeaderInfo().Category(theCategory);
	CategoryHeaderInfo().StoreLastDataOnly(theLatestDataOnly);
	CategoryHeaderInfo().CategoryHeader(true);
}

NFmiCaseStudyCategoryData::~NFmiCaseStudyCategoryData(void)
{
}

static void SetProducerHeaderInfoValues(NFmiCaseStudyProducerData &theProducerData, const NFmiCaseStudyDataFile &theFileData)
{
	theProducerData.ProducerHeaderInfo().Category(theFileData.Category());
	theProducerData.ProducerHeaderInfo().StartOffsetInMinutes(theFileData.StartOffsetInMinutes());
	theProducerData.ProducerHeaderInfo().EndOffsetInMinutes(theFileData.EndOffsetInMinutes());
	theProducerData.ProducerHeaderInfo().Store(theFileData.Store());
	theProducerData.ProducerHeaderInfo().StoreLastDataOnly(theFileData.StoreLastDataOnly());
}

void NFmiCaseStudyCategoryData::AddData(const NFmiCaseStudyDataFile &theData)
{
	// onko tuotttajalle jo omaa datasetti, jos ei ole, luo ensin se ja lis‰‰ data sitten tuottajelle
	for(size_t i = 0; i < itsProducersData.size(); i++)
	{
		NFmiCaseStudyProducerData &prodData = itsProducersData[i];
		if(prodData.ProducerHeaderInfo().Producer().GetIdent() == theData.Producer().GetIdent())
		{
			prodData.AddData(theData);
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
    auto iter = std::find_if(itsProducersData.begin(), itsProducersData.end(), [](const auto &producerData) {return producerData.ProducerHeaderInfo().Producer().GetIdent() == 0; });
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
	for(size_t i = 0; i < itsProducersData.size(); i++)
	{
		NFmiCaseStudyProducerData &producerData = itsProducersData[i];
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
	for(size_t i = 0; i < itsProducersData.size(); i++)
	{
		NFmiCaseStudyProducerData &producerData = itsProducersData[i];
		if(producerData.ProducerHeaderInfo().Producer().GetIdent() == static_cast<long>(theProdId))
			producerData.Update(theCaseStudySystem);
		totalFileSize += producerData.ProducerHeaderInfo().TotalFileSize();
		maxFileSize += producerData.ProducerHeaderInfo().MaxFileSize();
	}
	itsCategoryHeaderInfo.TotalFileSize(totalFileSize);
	itsCategoryHeaderInfo.MaxFileSize(maxFileSize);
}

void NFmiCaseStudyCategoryData::ProducerStore(unsigned long theProdId, bool newValue, const NFmiCaseStudySystem &theCaseStudySystem)
{
	for(size_t i = 0; i < itsProducersData.size(); i++)
	{
		if(itsProducersData[i].ProducerHeaderInfo().Producer().GetIdent() == static_cast<long>(theProdId))
			itsProducersData[i].ProducerStore(newValue);
	}
	Update(theProdId, theCaseStudySystem);
}

void NFmiCaseStudyCategoryData::CategoryStore(bool newValue, const NFmiCaseStudySystem &theCaseStudySystem)
{
	itsCategoryHeaderInfo.Store(newValue);
	for(size_t i = 0; i < itsProducersData.size(); i++)
		itsProducersData[i].ProducerStore(newValue);
	Update(theCaseStudySystem);
}

void NFmiCaseStudyCategoryData::ProducerEnable(NFmiHelpDataInfoSystem &theDataInfoSystem, unsigned long theProdId, bool newValue, const NFmiCaseStudySystem &theCaseStudySystem)
{
	for(size_t i = 0; i < itsProducersData.size(); i++)
	{
		if(itsProducersData[i].ProducerHeaderInfo().Producer().GetIdent() == static_cast<long>(theProdId))
			itsProducersData[i].ProducerEnable(theDataInfoSystem, newValue);
	}
	Update(theProdId, theCaseStudySystem);
}

void NFmiCaseStudyCategoryData::CategoryEnable(NFmiHelpDataInfoSystem &theDataInfoSystem, bool newValue, const NFmiCaseStudySystem &theCaseStudySystem)
{
    itsCategoryHeaderInfo.DataEnabled(theDataInfoSystem, newValue);
	for(size_t i = 0; i < itsProducersData.size(); i++)
		itsProducersData[i].ProducerEnable(theDataInfoSystem, newValue);
	Update(theCaseStudySystem);
}

void NFmiCaseStudyCategoryData::ProducerOffset(unsigned long theProdId, bool startOffset, int theOffsetInMinutes, const NFmiCaseStudySystem &theCaseStudySystem)
{
	for(size_t i = 0; i < itsProducersData.size(); i++)
	{
		if(itsProducersData[i].ProducerHeaderInfo().Producer().GetIdent() == static_cast<long>(theProdId))
			itsProducersData[i].ProducerOffset(startOffset, theOffsetInMinutes);
	}
	Update(theProdId, theCaseStudySystem);
}

void NFmiCaseStudyCategoryData::CategoryOffset(bool startOffset, int theOffsetInMinutes, const NFmiCaseStudySystem &theCaseStudySystem)
{
	if(startOffset)
		itsCategoryHeaderInfo.StartOffsetInMinutes(theOffsetInMinutes);
	else
		itsCategoryHeaderInfo.EndOffsetInMinutes(theOffsetInMinutes);

	for(size_t i = 0; i < itsProducersData.size(); i++)
		itsProducersData[i].ProducerOffset(startOffset, theOffsetInMinutes);
	Update(theCaseStudySystem);
}

void NFmiCaseStudyCategoryData::OrganizeDatas(void)
{
	SortDatas();
	for(size_t i = 0; i < itsProducersData.size(); i++)
	{
		itsProducersData[i].OrganizeDatas();
	}
}

void NFmiCaseStudyCategoryData::SortDatas(void)
{
	// miten producer datat sortataan? mik‰ on prioriteetti j‰rjestys?
	for(size_t i = 0; i < itsProducersData.size(); i++)
	{
	}
}

json_spirit::Object NFmiCaseStudyCategoryData::MakeJsonObject(const NFmiCaseStudyCategoryData &theData, bool fMakeFullStore)
{
	json_spirit::Array dataArray;
	const std::vector<NFmiCaseStudyProducerData> &dataVector = theData.ProducersData();
	for(size_t i = 0; i < dataVector.size(); i++)
	{
		json_spirit::Object tmpObject = NFmiCaseStudyProducerData::MakeJsonObject(dataVector[i], fMakeFullStore);
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

void NFmiCaseStudyCategoryData::AddDataToHelpDataInfoSystem(boost::shared_ptr<NFmiHelpDataInfoSystem> &theHelpDataInfoSystem, const std::string &theBasePath)
{
	for(size_t i = 0; i < itsProducersData.size(); i++)
		itsProducersData[i].AddDataToHelpDataInfoSystem(theHelpDataInfoSystem, theBasePath);
}

bool NFmiCaseStudyCategoryData::FindSuitableCaseStudyTime(NFmiMetTime &theTimeOut)
{
	for(size_t i = 0; i < itsProducersData.size(); i++)
	{
		if(itsProducersData[i].FindSuitableCaseStudyTime(theTimeOut))
			return true;
	}
	return false;
}

void NFmiCaseStudyCategoryData::InitDataWithStoredSettings(NFmiCaseStudyCategoryData &theOriginalCategoryData)
{
	// otetaan talteen header-osiosta offsetit ja store -tieto
	CategoryHeaderInfo().StartOffsetInMinutes(theOriginalCategoryData.CategoryHeaderInfo().StartOffsetInMinutes());
	CategoryHeaderInfo().EndOffsetInMinutes(theOriginalCategoryData.CategoryHeaderInfo().EndOffsetInMinutes());
	CategoryHeaderInfo().Store(theOriginalCategoryData.CategoryHeaderInfo().Store());

	// Lis‰ksi jokaisen lˆytyneen producerDatan vastin parin tiedot p‰ivitet‰‰n
	std::vector<NFmiCaseStudyProducerData> &origProducerData = theOriginalCategoryData.ProducersData();
	if(origProducerData.size() > 0)
	{
		for(size_t i = 0; i < origProducerData.size(); i++)
		{
			NFmiCaseStudyProducerData *producerData = GetProducerData(origProducerData[i].ProducerHeaderInfo().Producer().GetIdent());
			if(producerData)
			{
				producerData->InitDataWithStoredSettings(origProducerData[i]);
			}
		}
	}
}

NFmiCaseStudyProducerData* NFmiCaseStudyCategoryData::GetProducerData(unsigned long theProdId)
{
	for(size_t i = 0; i < itsProducersData.size(); i++)
	{
		if(itsProducersData[i].ProducerHeaderInfo().Producer().GetIdent() == static_cast<long>(theProdId))
			return &itsProducersData[i];
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
}

NFmiCaseStudySystem::~NFmiCaseStudySystem(void)
{
}

void NFmiCaseStudySystem::Reset(void)
{
	*this = NFmiCaseStudySystem();
}

static void SetCategoryHeaderInfoValues(NFmiCaseStudyCategoryData &theCategory, int theStartOffsetInMinutes, int theEndOffsetInMinutes, bool isStored)
{
	theCategory.CategoryHeaderInfo().StartOffsetInMinutes(theStartOffsetInMinutes);
	theCategory.CategoryHeaderInfo().EndOffsetInMinutes(theEndOffsetInMinutes);
	theCategory.CategoryHeaderInfo().Store(isStored);
}

bool NFmiCaseStudySystem::Init(NFmiHelpDataInfoSystem &theDataInfoSystem, NFmiInfoOrganizer &theInfoOrganizer)
{
	const int defaultStartOffsetInMinutes = -12*60;
	const int defaultEndOffsetInMinutes = 0;
	const bool defaultStoreDataSetting = true;

	std::vector<NFmiCaseStudyCategoryData> originalCategoriesData;
	originalCategoriesData.swap(itsCategoriesData); // otetaan talteen valmiiksi konffitiedostosta ladatut asetukset, ett‰ voidaan lopuksi s‰‰t‰‰ asetuksia niiden mukaan

	NFmiCaseStudyCategoryData category1(::GetDictionaryString("Model data"), NFmiCaseStudyDataFile::kFmiCategoryModel, false);
	::SetCategoryHeaderInfoValues(category1, defaultStartOffsetInMinutes, defaultEndOffsetInMinutes, defaultStoreDataSetting);
	itsCategoriesData.push_back(category1);
	NFmiCaseStudyCategoryData category2(::GetDictionaryString("Observation data"), NFmiCaseStudyDataFile::kFmiCategoryObservation, true);
	::SetCategoryHeaderInfoValues(category2, defaultStartOffsetInMinutes, defaultEndOffsetInMinutes, defaultStoreDataSetting);
	itsCategoriesData.push_back(category2);
	NFmiCaseStudyCategoryData category3(::GetDictionaryString("Analyze data"), NFmiCaseStudyDataFile::kFmiCategoryAnalyze, true);
	::SetCategoryHeaderInfoValues(category3, defaultStartOffsetInMinutes, defaultEndOffsetInMinutes, defaultStoreDataSetting);
	itsCategoriesData.push_back(category3);
	NFmiCaseStudyCategoryData category4(::GetDictionaryString("Edited data"), NFmiCaseStudyDataFile::kFmiCategoryEdited, false);
	::SetCategoryHeaderInfoValues(category4, defaultStartOffsetInMinutes, defaultEndOffsetInMinutes, defaultStoreDataSetting);
	itsCategoriesData.push_back(category4);
	NFmiCaseStudyCategoryData category5(::GetDictionaryString("Satellite image data"), NFmiCaseStudyDataFile::kFmiCategorySatelImage, false);
	::SetCategoryHeaderInfoValues(category5, defaultStartOffsetInMinutes, defaultEndOffsetInMinutes, defaultStoreDataSetting);
	itsCategoriesData.push_back(category5);

	const auto &infos = theDataInfoSystem.DynamicHelpDataInfos();
	for(size_t i = 0; i < infos.size(); i++)
	{
		NFmiCaseStudyDataFile data;
		data.Init(theDataInfoSystem, infos[i], theInfoOrganizer, defaultStartOffsetInMinutes, defaultEndOffsetInMinutes, defaultStoreDataSetting, *this);
		NFmiStringTools::ReplaceChars(data.FileFilter(), '/', '\\');
		AddData(data);
	}
	OrganizeDatas();
	InitDataWithStoredSettings(originalCategoriesData);
	Update();
	return true;
}

void NFmiCaseStudySystem::InitDataWithStoredSettings(std::vector<NFmiCaseStudyCategoryData> &theOriginalCategoriesData)
{
	if(theOriginalCategoriesData.size() > 0)
	{
		for(size_t i = 0; i < theOriginalCategoriesData.size(); i++)
		{
			NFmiCaseStudyCategoryData *categoryData = GetCategoryData(theOriginalCategoriesData[i].CategoryHeaderInfo().Category());
			if(categoryData)
			{
				categoryData->InitDataWithStoredSettings(theOriginalCategoriesData[i]);
			}
		}
	}
}

NFmiCaseStudyCategoryData* NFmiCaseStudySystem::GetCategoryData(NFmiCaseStudyDataFile::DataCategory theCategory)
{
	for(size_t i = 0; i < itsCategoriesData.size(); i++)
	{
		if(itsCategoriesData[i].CategoryHeaderInfo().Category() == theCategory)
			return &itsCategoriesData[i];
	}
	return 0;
}

void NFmiCaseStudySystem::AddData(const NFmiCaseStudyDataFile &theData)
{
	// miten jaetaan eri kategorioihin? onko jokaiselle kategorialle oma lista? lis‰ksi pit‰‰ jakaa tuottaja kohtaisesti
	for(size_t i = 0; i < itsCategoriesData.size(); i++)
	{
		if(itsCategoriesData[i].CategoryHeaderInfo().Category() == theData.Category())
		{
			itsCategoriesData[i].AddData(theData);
			break;
		}
	}
}

// updeittaa kaikki kategorian tuottajien data koot ja paivitt‰‰ omat totalSize ja maxSize-koot
void NFmiCaseStudySystem::Update(void)
{
	for(size_t i = 0; i < itsCategoriesData.size(); i++)
	{
		NFmiCaseStudyCategoryData &categoryData = itsCategoriesData[i];
		categoryData.Update(*this);
	}
}

// Jos CaseStudy dialogi on avattu enenn kuin kaikkia datoja on ehditty lukea SmartMetiin, j‰‰v‰t
// puuttuvat datat 0 producer Id tuottajan alle. Nyt pit‰‰ k‰yd‰ kaikki ne datat uudestaan l‰pi
// ja katsoa onko kyseinen data luettu k‰yttˆˆn. Sitten data pit‰‰ lis‰t‰ systeemiin uudestaan, ett‰ 
// se menee oikeaan lokeroon.
void NFmiCaseStudySystem::UpdateNoProducerData(NFmiHelpDataInfoSystem &theDataInfoSystem, NFmiInfoOrganizer &theInfoOrganizer)
{
    for(size_t i = 0; i < itsCategoriesData.size(); i++)
    {
        NFmiCaseStudyCategoryData &categoryData = itsCategoriesData[i];
        categoryData.UpdateNoProducerData(theDataInfoSystem, theInfoOrganizer);
    }
}

// updeittaa halutun kategorian halutun tuottajien data koot ja paivitt‰‰ omat totalSize ja maxSize-koot
void NFmiCaseStudySystem::Update(NFmiCaseStudyDataFile::DataCategory theCategory, unsigned long theProdId)
{
	for(size_t i = 0; i < itsCategoriesData.size(); i++)
	{
		NFmiCaseStudyCategoryData &categoryData = itsCategoriesData[i];
		if(categoryData.CategoryHeaderInfo().Category() == theCategory)
		{
			categoryData.Update(theProdId, *this);
			break;
		}
	}
}

static std::string NormalizeWindowsPathString(const std::string &thePath)
{
	std::string pathStr(thePath);
	NFmiStringTools::LowerCase(pathStr);
	NFmiStringTools::ReplaceChars(pathStr, '/', '\\');
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
	for(size_t i = 0; i < itsCategoriesData.size(); i++)
	{
		NFmiCaseStudyCategoryData &categoryData = itsCategoriesData[i];
		if(categoryData.CategoryHeaderInfo().Category() == theCategory)
		{
			categoryData.ProducerStore(theProdId, newValue, *this);
			break;
		}
	}
}

void NFmiCaseStudySystem::CategoryStore(NFmiCaseStudyDataFile::DataCategory theCategory, bool newValue)
{
	for(size_t i = 0; i < itsCategoriesData.size(); i++)
	{
		NFmiCaseStudyCategoryData &categoryData = itsCategoriesData[i];
		if(categoryData.CategoryHeaderInfo().Category() == theCategory)
		{
			categoryData.CategoryStore(newValue, *this);
			break;
		}
	}
}

void NFmiCaseStudySystem::ProducerEnable(NFmiHelpDataInfoSystem &theDataInfoSystem, NFmiCaseStudyDataFile::DataCategory theCategory, unsigned long theProdId, bool newValue)
{
	for(size_t i = 0; i < itsCategoriesData.size(); i++)
	{
		NFmiCaseStudyCategoryData &categoryData = itsCategoriesData[i];
		if(categoryData.CategoryHeaderInfo().Category() == theCategory)
		{
			categoryData.ProducerEnable(theDataInfoSystem, theProdId, newValue, *this);
			break;
		}
	}
}

void NFmiCaseStudySystem::CategoryEnable(NFmiHelpDataInfoSystem &theDataInfoSystem, NFmiCaseStudyDataFile::DataCategory theCategory, bool newValue)
{
	for(size_t i = 0; i < itsCategoriesData.size(); i++)
	{
		NFmiCaseStudyCategoryData &categoryData = itsCategoriesData[i];
		if(categoryData.CategoryHeaderInfo().Category() == theCategory)
		{
			categoryData.CategoryEnable(theDataInfoSystem, newValue, *this);
			break;
		}
	}
}

void NFmiCaseStudySystem::ProducerOffset(NFmiCaseStudyDataFile::DataCategory theCategory, unsigned long theProdId, bool startOffset, int theOffsetInMinutes)
{
	for(size_t i = 0; i < itsCategoriesData.size(); i++)
	{
		NFmiCaseStudyCategoryData &categoryData = itsCategoriesData[i];
		if(categoryData.CategoryHeaderInfo().Category() == theCategory)
		{
			categoryData.ProducerOffset(theProdId, startOffset, theOffsetInMinutes, *this);
			break;
		}
	}
}

void NFmiCaseStudySystem::CategoryOffset(NFmiCaseStudyDataFile::DataCategory theCategory, bool startOffset, int theOffsetInMinutes)
{
	for(size_t i = 0; i < itsCategoriesData.size(); i++)
	{
		NFmiCaseStudyCategoryData &categoryData = itsCategoriesData[i];
		if(categoryData.CategoryHeaderInfo().Category() == theCategory)
		{
			categoryData.CategoryOffset(startOffset, theOffsetInMinutes, *this);
			break;
		}
	}
}

void NFmiCaseStudySystem::OrganizeDatas(void)
{
	for(size_t i = 0; i < itsCategoriesData.size(); i++)
	{
		itsCategoriesData[i].OrganizeDatas();
	}
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

		std::vector<NFmiCaseStudyProducerData> &producersDataVec = categoryData.ProducersData();
		for(size_t j = 0; j < producersDataVec.size(); j++)
		{
			NFmiCaseStudyProducerData &producerData = producersDataVec[j];
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

// Etsii 1. datan, mill‰ on startTimeOffset jotain muuta kuin 0:aa ja kyse ei ole obs/analyysi tms datoista, joilla otetaan vain viimeisin data.
NFmiMetTime NFmiCaseStudySystem::FindSuitableCaseStudyTime(void)
{
	NFmiMetTime aTime;
	std::vector<NFmiCaseStudyCategoryData> &dataVector = CategoriesData();
	for(size_t i = 0; i < dataVector.size(); i++)
	{
		if(dataVector[i].FindSuitableCaseStudyTime(aTime))
			break; // jos t‰st‰ categoriasta lˆytyi sopiva aika kandidaatti, lopetetaan
	}
	return aTime;
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

	// otetaan currentti aika CaseStudy-ajaksi, ei kannatakaan etsi‰ CaseStudyn alku aikaa, koska viimeiset mallit tulev‰t n‰kyviin vain 
	// loppu ajalle eli kannattaa tallettaa t‰h‰n caseStudy-aikahaarukan loppuaika.
	itsTime = NFmiMetTime(); // FindSuitableCaseStudyTime();
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
	std::string basePath = ::GetDirectory(theMetaDataFilePath);
	std::string dataDir = basePath;
	NFmiStringTools::ReplaceChars(dataDir, '/', '\\');
	if(dataDir.size() && dataDir[dataDir.size()-1] != '\\')
		dataDir += "\\";
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

// 1. Tekee listan kopioitavista tiedostoista.
// 2. Ottaa huomioon aikarajoitteet.
// 3. Jos endOffset on 0 tai positiivinen, ei ole rajoitusta tiedoston aikaleiman 'uutuuteen' n‰hden, 
// eli silloin kopsataan uusimmat tiedostot, vaikka niiss‰ olisi sein‰kelloa uudempi aika.
// 4. Mukana on koko polku ja tiedoston nimi.
static std::list<std::string> GetWantedFileList(NFmiCaseStudyDataFile &theDataFile)
{
	std::list<std::string> copyedFiles;
	if(theDataFile.StoreLastDataOnly())
	{
		std::string fileName = NFmiFileSystem::NewestPatternFileName(theDataFile.FileFilter());
		copyedFiles.push_back(fileName);
	}
	else
	{
		NFmiMetTime startTime;
		startTime.ChangeByMinutes(theDataFile.StartOffsetInMinutes());
		time_t timeLimit = startTime.EpochTime();
		std::list<std::pair<std::string, time_t> > filesWithTimes = NFmiFileSystem::PatternFiles(theDataFile.FileFilter(), timeLimit);

		NFmiMetTime endTime;
		endTime.ChangeByMinutes(theDataFile.EndOffsetInMinutes());
		time_t endTimeLimit = endTime.EpochTime();
		// viel‰ pit‰‰ erottaa mahdolliset liian tuoreet tiedostot pois ja tehd‰ lopullinen lista
		bool keepAllFiles = theDataFile.EndOffsetInMinutes() >= 0; // jos end-offset on positiivinen/0, pidet‰‰n kaikki tiedostot
		for(std::list<std::pair<std::string, time_t> >::iterator it = filesWithTimes.begin(); it != filesWithTimes.end(); ++it)
		{
			if(keepAllFiles)
				copyedFiles.push_back((*it).first);
			else if(endTimeLimit > (*it).second)
				copyedFiles.push_back((*it).first);
		}
	}

	return copyedFiles;
}

// t‰m‰ ottaa viimeisen polun osion annetusta polusta. Esim.
// "C:\\data\case1_data" -> "case1_data"
static std::string GetRelativeDataDirectory(const std::string &theDataDir)
{
	std::string tmpStr(theDataDir);
	NFmiStringTools::ReplaceChars(tmpStr, '/', '\\');
	NFmiStringTools::TrimR(tmpStr, '\\'); // poistetaan varmuuden vuoksi per‰ss‰ olevat kenoviivat
	std::string::size_type pos = tmpStr.find_last_of("\\");
	if(pos != std::string::npos)
	{
		std::string relPath(tmpStr.begin()+pos+1, tmpStr.end());
		return relPath;
	}
	else
		return "";
}


#ifdef CopyFile // winkkari makro pit‰‰ disabloida ensin ennen kuin voi k‰ytt‰‰ NFmiFileSystem:in FileCopy-funktiota
#undef CopyFile
#endif 

// 1. kopioi annetut tiedostot theDestDir-hakemistoon.
// 2. Pit‰‰kˆ muokata NFmiCaseStudyDataFile:en uusi suhteellinen polku, ett‰ myˆhemmin sit‰ voidaan k‰ytt‰‰ kun dataa ladataan katsottavaksi?
static void CopyFilesToDestination(const std::list<std::string> &theCopyedFiles, const std::string &theDestDir, int theProgressDialogMaxCount, int &theProgressCounter, CWnd *theCopyWindowPos)
{
	if(theCopyedFiles.size() == 0)
		return ;

	CShellFileOp sfo;
	BOOL bAPICalled = FALSE;
	int nAPIReturnVal = 0;
	for(std::list<std::string>::const_iterator it = theCopyedFiles.begin(); it != theCopyedFiles.end(); ++it)
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

static void StoreFileData(const std::string &theProducerDir, const std::string &theRelativeProducerDir, NFmiCaseStudyDataFile &theDataFile, int theProgressDialogMaxCount, int &theProgressCounter, CWnd *theCopyWindowPos)
{
	std::string usedDestDir = ::GetDataFileDir(theProducerDir, theDataFile);
	std::string usedRelativeDestDir = ::GetDataFileDir(theRelativeProducerDir, theDataFile);
	usedRelativeDestDir += "\\";
	usedRelativeDestDir += ::GetRelativeDataDirectory(theDataFile.FileFilter()); // k‰ytet‰‰n taas GetRelativeDataDirectory funktiota ett‰ saadaan k‰ytetty tiedosto-filter -loppuosio talteen
	theDataFile.RelativeStoredFileFilter(usedRelativeDestDir);

	if(NFmiFileSystem::CreateDirectory(usedDestDir))
	{
		std::list<std::string> copyedFiles = ::GetWantedFileList(theDataFile);
		::CopyFilesToDestination(copyedFiles, usedDestDir, theProgressDialogMaxCount, theProgressCounter, theCopyWindowPos);
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

static void StoreProducerData(const std::string &theCategoryDir, const std::string &theRelativeCategoryDir, NFmiCaseStudyProducerData &theProducerData, int theProgressDialogMaxCount, int &theProgressCounter, CWnd *theCopyWindowPos)
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
					::StoreFileData(producerDir, relativeProducerDir, dataFile, theProgressDialogMaxCount, theProgressCounter, theCopyWindowPos);
					imageFiles = true;
				}
				else
					::AddDataFilesToList(producerDir, relativeProducerDir, dataFile, copyedFiles);
			}
		}
		if(imageFiles == false)
			::CopyFilesToDestination(copyedFiles, producerDir, theProgressDialogMaxCount, theProgressCounter, theCopyWindowPos);
	}
}

static std::string GetCategoryDir(const std::string &theBaseDir, NFmiCaseStudyCategoryData &theCategory)
{
	std::string categoryDir = theBaseDir;
	categoryDir += "\\";
	categoryDir += theCategory.CategoryHeaderInfo().Name();
	return categoryDir;
}

static void StoreCategoryData(const std::string &theDataDir, const std::string &theRelativeDataDir, NFmiCaseStudyCategoryData &theCategory, int theProgressDialogMaxCount, int &theProgressCounter, CWnd *theCopyWindowPos)
{
	std::string categoryDir = ::GetCategoryDir(theDataDir, theCategory);
	std::string relativeCategoryDir = ::GetCategoryDir(theRelativeDataDir, theCategory);
	if(NFmiFileSystem::CreateDirectory(categoryDir))
	{
		bool storeLastDataOnlyCategory = theCategory.CategoryHeaderInfo().StoreLastDataOnly();
		std::list<std::string> copyedFiles;
		std::vector<NFmiCaseStudyProducerData> &producerDataVec = theCategory.ProducersData();
		for(size_t i=0; i < producerDataVec.size(); i++)
		{
			if(storeLastDataOnlyCategory)
				::AddProducerDataFilesToList(categoryDir, relativeCategoryDir, producerDataVec[i], copyedFiles);
			else
				::StoreProducerData(categoryDir, relativeCategoryDir, producerDataVec[i], theProgressDialogMaxCount, theProgressCounter, theCopyWindowPos);
		}
		if(storeLastDataOnlyCategory)
			::CopyFilesToDestination(copyedFiles, categoryDir, theProgressDialogMaxCount, theProgressCounter, theCopyWindowPos);
	}
}

int NFmiCaseStudySystem::CalculateProgressDialogCount(void) const
{
	int counter = 0;
	for(size_t i=0; i < itsCategoriesData.size(); i++)
	{
		const NFmiCaseStudyCategoryData &categoryData = itsCategoriesData[i];
		const std::vector<NFmiCaseStudyProducerData> &producerDataVec = categoryData.ProducersData();
		for(size_t j=0; j < producerDataVec.size(); j++)
		{
			bool storeLastDataOnlyCategory = categoryData.CategoryHeaderInfo().StoreLastDataOnly();
			const NFmiCaseStudyProducerData &producerData = producerDataVec[j];
			if(producerData.FilesData().size())
			{
				const std::vector<NFmiCaseStudyDataFile> &fileDataVec = producerData.FilesData();
				bool imageProducer = fileDataVec[0].ImageFile();
				for(size_t k=0; k < fileDataVec.size(); k++)
				{
					const NFmiCaseStudyDataFile &fileData = fileDataVec[k];
					if(fileData.Store())
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
bool NFmiCaseStudySystem::MakeCaseStudyData(const std::string &theFullPathMetaDataFileName, CWnd *theParentWindow, CWnd *theCopyWindowPos)
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
				::StoreCategoryData(dataDir, relativeDataDir, itsCategoriesData[i], progressDialogMaxCount, progressCounter, theCopyWindowPos);
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
boost::shared_ptr<NFmiHelpDataInfoSystem> NFmiCaseStudySystem::MakeHelpDataInfoSystem(const NFmiHelpDataInfoSystem &theOriginalHelpDataInfoSystem, const std::string &theBasePath)
{
	boost::shared_ptr<NFmiHelpDataInfoSystem> helpDataInfoSystem;
	if(itsCategoriesData.size())
	{
		const auto &staticHelpDataInfos =  theOriginalHelpDataInfoSystem.StaticHelpDataInfos();
		helpDataInfoSystem = boost::shared_ptr<NFmiHelpDataInfoSystem>(new NFmiHelpDataInfoSystem());
		for(size_t i = 0; i < staticHelpDataInfos.size(); i++)
			helpDataInfoSystem->AddStatic(staticHelpDataInfos[i]);
		helpDataInfoSystem->CacheMaxFilesPerPattern(theOriginalHelpDataInfoSystem.CacheMaxFilesPerPattern()); // t‰m‰ on otettava talteen, muuten ei lokaali CaseStudyn arkisto toimi

		for(size_t i = 0; i < itsCategoriesData.size(); i++)
			itsCategoriesData[i].AddDataToHelpDataInfoSystem(helpDataInfoSystem, theBasePath);
	}
	return helpDataInfoSystem;
}

std::string NFmiCaseStudySystem::MakeCaseStudyFilePattern(const std::string &theFilePattern, const std::string &theBasePath, bool fMakeOnlyPath)
{
	std::string patternWithoutPath = NFmiFileSystem::FileNameFromPath(theFilePattern);
	if(patternWithoutPath.empty() == false)
	{
		for(size_t i = 0; i < itsCategoriesData.size(); i++)
		{
			NFmiCaseStudyCategoryData &categoryData = itsCategoriesData[i];
			std::vector<NFmiCaseStudyProducerData> &producerDataVector = categoryData.ProducersData();
			for(size_t j = 0; j < producerDataVector.size(); j++)
			{
				NFmiCaseStudyProducerData &producerData = producerDataVector[j];
				std::vector<NFmiCaseStudyDataFile> &dataFileVector = producerData.FilesData();
				for(size_t k = 0; k < dataFileVector.size(); k++)
				{
					NFmiCaseStudyDataFile &dataFile = dataFileVector[k];
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

// ************************************************************
// *****   NFmiCaseStudySystem loppuu  ************************
// ************************************************************
