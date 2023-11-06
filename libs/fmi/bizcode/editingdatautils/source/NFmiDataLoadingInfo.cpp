//**********************************************************
// C++ Class Name : NFmiDataLoadingInfo 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiDataLoadingInfo.cpp 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 2.1.4 ) 
//  - GD System Name    : Met-editor Plan 3 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : Tiedostojen lataus luokat 
// ---------------------------------------------------  
//  Author         : laura 
//  Creation Date  : Thur - Apr 1, 1999 
// 
// 
//  Description: 
//   Luokkaa k‰ytet‰‰n meteorologin editorissa 
//   datan lukemiseen ja talletukseen. Tiet‰‰ 
//   mm. 
//   haettavat tuottajat, niiden tiedostojen nimet, 
//   polut ja datan kopioiden versio-numeron. 
//   
//   T‰m‰ luokka hallitsee aina yhden tyyppisen 
//   datan tiedot (joita voi olla esim. 'Eurooppa 
//   Data', 'Suomi Data', 'Meri Data' jne.).
// 
//  Change Log: 
// 
//**********************************************************
#include "NFmiDataLoadingInfo.h"
#include "NFmiSettings.h"
#include "NFmiFileSystem.h"
#include "NFmiHPlaceDescriptor.h"
#include "NFmiMetEditorModeDataWCTR.h"
#include "NFmiTimeDescriptor.h"
#include "NFmiStringTools.h"
#include "NFmiFileString.h"
#include "NFmiPathUtils.h"

float NFmiDataLoadingInfo::itsFileVersionNumber = 2.0;

//--------------------------------------------------------
// Constructor/Destructor 
//--------------------------------------------------------
NFmiDataLoadingInfo::NFmiDataLoadingInfo()
:itsAreaIdentifier("Suomi")
,itsProducer() //esim Kepa 
,itsWorkingNameInDialog("Omat")
,itsDataBaseNameInDialog1("Vir. lyhyt")
,itsWorkingFileName("")
,itsDataBaseFileNameIn("")
,itsDataBaseFileNameOut("")
,itsWorkingPath("")
,itsDataBaseInPath("")
,itsDataBaseOutPath("")
,fUseDataBaseOutName(false)
,itsHirlamFileNameList(kDescending) //laura 270419999
,itsEcmwfFileNameList(kDescending)
,itsWorkingFileNameList(kDescending)
,itsDataBaseInFileNameList1(kDescending)
,itsTimeInterpolationSetting(1)
,itsParamCombinationSetting(1)
,itsPrimaryProducerSetting(1)
,itsSecondaryProducerSetting(2)
,itsThirdProducerSetting(2)
,itsHirlamFileNameListIndex(0)
,itsEcmwfFileNameListIndex(0)
,itsWorkingFileNameListIndex(0)
,itsDataBaseFileNameListIndex1(0)
,itsMetEditorModeDataWCTR(0)
,itsHPlaceDescriptor(0)
,itsSelectedFileNamesVector()
,itsDataLengthInHours(0)
,fUseDataCache(false)
,itsCacheDir("")
,itsModel1CacheFilePattern("")
,itsModel2CacheFilePattern("")
,itsCaseStudyTime(NFmiMetTime::gMissingTime)
,itsFileNameTimeStampTemplate("DDHH")
,fUseWallClockTimeInTimeStamp(false)
{
	fUseDataBaseOutName = true;
	UpdatedTimeDescriptor(false);
}

NFmiDataLoadingInfo& NFmiDataLoadingInfo::operator=(NFmiDataLoadingInfo& theInfo)
{
	if(this != &theInfo)
	{
		itsAreaIdentifier = theInfo.itsAreaIdentifier;
		itsProducer = theInfo.itsProducer;
		itsModel1NameInDialog = theInfo.itsModel1NameInDialog;
		itsModel2NameInDialog = theInfo.itsModel2NameInDialog;
		itsWorkingNameInDialog = theInfo.itsWorkingNameInDialog;
		itsDataBaseNameInDialog1 = theInfo.itsDataBaseNameInDialog1;
		itsModel1FilePattern = theInfo.itsModel1FilePattern;
		itsModel2FilePattern = theInfo.itsModel2FilePattern;
		itsWorkingFileName = theInfo.itsWorkingFileName;
		itsDataBaseFileNameIn = theInfo.itsDataBaseFileNameIn;
		itsDataBaseFileNameOut = theInfo.itsDataBaseFileNameOut;
		itsWorkingPath = theInfo.itsWorkingPath;
		itsDataBaseInPath = theInfo.itsDataBaseInPath;
		itsDataBaseOutPath = theInfo.itsDataBaseOutPath;
		fUseDataBaseOutName = theInfo.fUseDataBaseOutName;
		itsComboHirlamFileName = theInfo.itsComboHirlamFileName;
		itsComboEcmwfFileName = theInfo.itsComboEcmwfFileName;
		itsComboWorkingFileName = theInfo.itsComboWorkingFileName;
		itsComboDataBase1FileName = theInfo.itsComboDataBase1FileName;
		itsComboDataBase2FileName = theInfo.itsComboDataBase2FileName;
		itsHirlamFileNameList = theInfo.itsHirlamFileNameList;
		itsEcmwfFileNameList = theInfo.itsEcmwfFileNameList;
		itsWorkingFileNameList = theInfo.itsWorkingFileNameList;
		itsDataBaseInFileNameList1 = theInfo.itsDataBaseInFileNameList1;
		itsTimeInterpolationSetting = theInfo.itsTimeInterpolationSetting;
		itsParamCombinationSetting = theInfo.itsParamCombinationSetting;
		itsPrimaryProducerSetting = theInfo.itsPrimaryProducerSetting;
		itsSecondaryProducerSetting = theInfo.itsSecondaryProducerSetting;
		itsThirdProducerSetting = theInfo.itsThirdProducerSetting;
		itsHirlamFileNameListIndex = theInfo.itsHirlamFileNameListIndex;
		itsEcmwfFileNameListIndex = theInfo.itsEcmwfFileNameListIndex;
		itsWorkingFileNameListIndex = theInfo.itsWorkingFileNameListIndex;
		itsDataBaseFileNameListIndex1 = theInfo.itsDataBaseFileNameListIndex1;

		if(itsMetEditorModeDataWCTR)
		{
			delete itsMetEditorModeDataWCTR;
			itsMetEditorModeDataWCTR = 0;
		}
		if(theInfo.itsMetEditorModeDataWCTR)
		{
			itsMetEditorModeDataWCTR = new NFmiMetEditorModeDataWCTR(*theInfo.itsMetEditorModeDataWCTR);
		}

		if(itsHPlaceDescriptor)
		{
			delete itsHPlaceDescriptor;
			itsHPlaceDescriptor = 0;
		}
		if(theInfo.itsHPlaceDescriptor)
		{
			itsHPlaceDescriptor = new NFmiHPlaceDescriptor(*theInfo.itsHPlaceDescriptor);
		}

		itsSelectedFileNamesVector = theInfo.itsSelectedFileNamesVector;
		itsDataLengthInHours = theInfo.itsDataLengthInHours;
		fUseDataCache = theInfo.fUseDataCache;
		itsCacheDir = theInfo.itsCacheDir;
		itsModel1CacheFilePattern = theInfo.itsModel1CacheFilePattern;
		itsModel2CacheFilePattern = theInfo.itsModel2CacheFilePattern;
		itsCaseStudyTime = theInfo.itsCaseStudyTime;
        itsFileNameTimeStampTemplate = theInfo.itsFileNameTimeStampTemplate;
        fUseWallClockTimeInTimeStamp = theInfo.fUseWallClockTimeInTimeStamp;
    }
	return *this;
}


NFmiDataLoadingInfo::~NFmiDataLoadingInfo()
{
	itsHirlamFileNameList.Clear(true);
	itsEcmwfFileNameList.Clear(true);
	itsWorkingFileNameList.Clear(true);
	itsDataBaseInFileNameList1.Clear(true);
	delete itsMetEditorModeDataWCTR;
	delete itsHPlaceDescriptor;
}

void NFmiDataLoadingInfo::InitFileNameLists()
{
	//jos joku seuraavista palauttaa false, voisi tehd‰ jotain!!
	InitHirlamFileNameList();
	InitEcmwfFileNameList();
	InitWorkingFileNameList();
	InitDataBaseInFileNameList1();
}

// Configuration keys
#define CONFIG_DATALOADINGINFO_PRIMARYPRODUCERSETTING "MetEditor::DataLoadingInfo::PrimaryProducerSetting"
#define CONFIG_DATALOADINGINFO_SECONDARYPRODUCERSETTING "MetEditor::DataLoadingInfo::SecondaryProducerSetting"
#define CONFIG_DATALOADINGINFO_THIRDPRODUCERSETTING "MetEditor::DataLoadingInfo::ThirdProducerSetting"
#define CONFIG_DATALOADINGINFO_TIMEINTERPOLATIONSETTING "MetEditor::DataLoadingInfo::TimeInterpolationSetting"
#define CONFIG_DATALOADINGINFO_PARAMCOMBINATIONSETTING "MetEditor::DataLoadingInfo::ParamCombinationSetting"
#define CONFIG_DATALOADINGINFO_AREAIDENTIFIER "MetEditor::DataLoadingInfo::AreaIdentifier"

#define CONFIG_DATALOADINGINFO_PRODUCERIDENT "MetEditor::DataLoadingInfo::ProducerIdent"
#define CONFIG_DATALOADINGINFO_PRODUCERNAME "MetEditor::DataLoadingInfo::ProducerName"

#define CONFIG_DATALOADINGINFO_MODEL1NAMEINDIALOG "MetEditor::DataLoadingInfo::Model1NameInDialog"
#define CONFIG_DATALOADINGINFO_MODEL2NAMEINDIALOG "MetEditor::DataLoadingInfo::Model2NameInDialog"

#define CONFIG_DATALOADINGINFO_WORKINGNAMEINDIALOG "MetEditor::DataLoadingInfo::WorkingNameInDialog"
#define CONFIG_DATALOADINGINFO_DATABASENAMEINDIALOG1 "MetEditor::DataLoadingInfo::DataBaseNameInDialog1"

#define CONFIG_DATALOADINGINFO_MODEL1FILENAMEPATTERN "MetEditor::DataLoadingInfo::Model1FileNamePattern"
#define CONFIG_DATALOADINGINFO_MODEL2FILENAMEPATTERN "MetEditor::DataLoadingInfo::Model2FileNamePattern"

#define CONFIG_DATALOADINGINFO_WORKINGFILENAME "MetEditor::DataLoadingInfo::WorkingFileName"
#define CONFIG_DATALOADINGINFO_DATABASEFILENAMEIN "MetEditor::DataLoadingInfo::DataBaseFileNameIn"

#define CONFIG_DATALOADINGINFO_DATABASEFILENAMEOUT "MetEditor::DataLoadingInfo::DataBaseFileNameOut"
#define CONFIG_DATALOADINGINFO_DATABASEINPATH "MetEditor::DataLoadingInfo::DataBaseInPath"
#define CONFIG_DATALOADINGINFO_DATABASEOUTPATH "MetEditor::DataLoadingInfo::DataBaseOutPath"

#define CONFIG_DATALOADINGINFO_DATALENGTHINHOURS "MetEditor::DataLoadingInfo::DataLengthInHours"

static void FixPathEndWithSeparator(std::string &theFixedPathStr)
{
	if(theFixedPathStr.empty() == false)
	{
		std::string::value_type lastLetter = theFixedPathStr[theFixedPathStr.size()-1];
		if((lastLetter == '/' || lastLetter == '\\') == false)
			theFixedPathStr.push_back(kFmiDirectorySeparator);
	}
}

static bool IsWallClockUsedWithTimeStamp(const std::string & theUsedTimeStampTemplate)
{
    std::string::size_type pos = theUsedTimeStampTemplate.find("mm");
    return pos != std::string::npos;
}

void NFmiDataLoadingInfo::Configure(const std::string& theAbsoluteWorkingDirectory, const std::string& theCacheBaseDir, const std::string& theCacheLocalDir, bool useDataCache)
{
	std::string temp;
	NFmiString producerName;

	fUseDataBaseOutName = true;

	itsPrimaryProducerSetting = NFmiSettings::Require<int>(CONFIG_DATALOADINGINFO_PRIMARYPRODUCERSETTING);
	itsSecondaryProducerSetting = NFmiSettings::Require<int>(CONFIG_DATALOADINGINFO_SECONDARYPRODUCERSETTING);
	itsThirdProducerSetting = NFmiSettings::Require<int>(CONFIG_DATALOADINGINFO_THIRDPRODUCERSETTING);

	itsAreaIdentifier = NFmiSettings::Require<std::string>(CONFIG_DATALOADINGINFO_AREAIDENTIFIER);

	temp = NFmiSettings::Require<std::string>(CONFIG_DATALOADINGINFO_PRODUCERNAME);
	producerName = NFmiString(temp);

	itsProducer.SetContents(NFmiSettings::Require<int>(CONFIG_DATALOADINGINFO_PRODUCERIDENT), producerName);

	itsModel1NameInDialog = NFmiSettings::Require<std::string>(CONFIG_DATALOADINGINFO_MODEL1NAMEINDIALOG);
	itsModel2NameInDialog = NFmiSettings::Require<std::string>(CONFIG_DATALOADINGINFO_MODEL2NAMEINDIALOG);

	temp = NFmiSettings::Require<std::string>(CONFIG_DATALOADINGINFO_WORKINGNAMEINDIALOG);
	itsWorkingNameInDialog = NFmiString(temp);

	temp = NFmiSettings::Require<std::string>(CONFIG_DATALOADINGINFO_DATABASENAMEINDIALOG1);
	itsDataBaseNameInDialog1 = NFmiString(temp);

	itsModel1FilePattern = NFmiSettings::Require<std::string>(CONFIG_DATALOADINGINFO_MODEL1FILENAMEPATTERN);
	itsModel2FilePattern = NFmiSettings::Require<std::string>(CONFIG_DATALOADINGINFO_MODEL2FILENAMEPATTERN);

	temp = NFmiSettings::Require<std::string>(CONFIG_DATALOADINGINFO_WORKINGFILENAME);
	itsWorkingFileName = NFmiString(temp);

	temp = NFmiSettings::Require<std::string>(CONFIG_DATALOADINGINFO_DATABASEFILENAMEIN);
	itsDataBaseFileNameIn = NFmiString(temp);

	temp = NFmiSettings::Require<std::string>(CONFIG_DATALOADINGINFO_DATABASEFILENAMEOUT);
	itsDataBaseFileNameOut = NFmiString(temp);

	temp = NFmiSettings::Require<std::string>(CONFIG_DATALOADINGINFO_DATABASEINPATH);
	itsDataBaseInPath = NFmiString(temp);

	temp = NFmiSettings::Require<std::string>(CONFIG_DATALOADINGINFO_DATABASEOUTPATH);
	itsDataBaseOutPath = NFmiString(temp);

	itsDataLengthInHours = NFmiSettings::Require<int>(CONFIG_DATALOADINGINFO_DATALENGTHINHOURS);

    itsFileNameTimeStampTemplate = NFmiSettings::Optional<std::string>("MetEditor::DataLoadingInfo::FileNameTimeStampTemplate", "DDHH");
    fUseWallClockTimeInTimeStamp = ::IsWallClockUsedWithTimeStamp(itsFileNameTimeStampTemplate);

	if(!itsMetEditorModeDataWCTR)
			itsMetEditorModeDataWCTR = new NFmiMetEditorModeDataWCTR;

	// NFmiMetEditorModeDataWCTR knows how to initialize itself from the global settings.
	itsMetEditorModeDataWCTR->Configure();

	itsCacheDir = theCacheLocalDir;
	itsWorkingPath = theCacheBaseDir + "edited\\";
	fUseDataCache = useDataCache;
	NormalizeAllPathDelimiters(theAbsoluteWorkingDirectory); // t‰m‰ pit‰‰ tehd‰ ensin, ett‰ kenoviivat on oikein
	itsModel1CacheFilePattern = MakeCacheFilePattern(itsModel1FilePattern);
	itsModel2CacheFilePattern = MakeCacheFilePattern(itsModel2FilePattern);

	NormalizeAllPathDelimiters(theAbsoluteWorkingDirectory); // tehd‰‰n t‰m‰ viel‰ toistamiseen ett‰ varmasti kaikki kenot on oikein p‰in
	InitFileNameLists(); //luodaan tiedostonnimilistat valmiiksi, koska t‰m‰ on yleisin tapa luoda info
	UpdatedTimeDescriptor(false);

}

NFmiString NFmiDataLoadingInfo::MakeCacheFilePattern(const NFmiString &theNormalFilePattern)
{
	NFmiFileString fileStr(theNormalFilePattern);
	NFmiString cacheFileStr = itsCacheDir;
	cacheFileStr += fileStr.FileName();
	return cacheFileStr;
}

static NFmiString DoTotalPathFix(const NFmiString &thePath, const std::string& theAbsoluteWorkingDirectory)
{
	std::string tmpPath = thePath;
	tmpPath = PathUtils::makeFixedAbsolutePath(tmpPath, theAbsoluteWorkingDirectory, false);
	return NFmiString(tmpPath);
}

// Joskus konffeissa voi menn‰ eri polkujen kanssa hakemisto erottimet eri suuntiin
// ja sen j‰lkeen ei ehk‰ tietyt metodit mm. NFmiFileString-luokassa ehk‰ en‰‰ toimikaan toivotulla tavalla.
// Siksi kun asetukset on luettu konfiguraatioista, laitetaan kaikki polku erottimet
// samanlaisiksi varmuuden vuoksi.
void NFmiDataLoadingInfo::NormalizeAllPathDelimiters(const std::string& theAbsoluteWorkingDirectory)
{
	itsModel1FilePattern = ::DoTotalPathFix(itsModel1FilePattern, theAbsoluteWorkingDirectory);
	itsModel2FilePattern = ::DoTotalPathFix(itsModel2FilePattern, theAbsoluteWorkingDirectory);
	itsWorkingPath = ::DoTotalPathFix(itsWorkingPath, theAbsoluteWorkingDirectory);
	itsDataBaseInPath = ::DoTotalPathFix(itsDataBaseInPath, theAbsoluteWorkingDirectory);
	itsDataBaseOutPath = ::DoTotalPathFix(itsDataBaseOutPath, theAbsoluteWorkingDirectory);
	itsCacheDir = ::DoTotalPathFix(itsCacheDir, theAbsoluteWorkingDirectory);
}

NFmiString NFmiDataLoadingInfo::CreateWorkingFileNameFilter()
{
	NFmiString workingFileName("");
	workingFileName += itsWorkingPath;
	workingFileName += itsWorkingFileName;
	workingFileName += "_";
	workingFileName += itsAreaIdentifier;
	workingFileName += "_*.sqd";
	return workingFileName;
}

NFmiString NFmiDataLoadingInfo::CreateDataBaseInFileNameFilter(int /* index */ )
{
	NFmiString dataBaseFileName("");
	dataBaseFileName += DataBaseInPath();
	if(DataBaseFileNameIn().Search(reinterpret_cast<const unsigned char *>("*"))) // 11.6.2001/Marko Jos filtteri on jo annettu kokonaisuudessaan ohjaustiedostossa, k‰ytet‰‰n suoraan sit‰ eik‰ rakenneta
		dataBaseFileName += DataBaseFileNameIn();
	else
	{
		dataBaseFileName += DataBaseFileNameIn();
		dataBaseFileName += "_";
		dataBaseFileName += itsAreaIdentifier;
		dataBaseFileName += "_";
		int length = DataLengthInHours();
		if(length > 0)
			dataBaseFileName += NFmiStringTools::Convert<int>(length);
		dataBaseFileName +=	"*.sqd";
	}
	return dataBaseFileName;
}

NFmiString NFmiDataLoadingInfo::CreateWorkingFileNameTimeFilter()
{
	NFmiString workingFileName("");
	workingFileName += itsWorkingPath;
	workingFileName += itsWorkingFileName;
	workingFileName += "_";
	workingFileName += itsAreaIdentifier;
	workingFileName += "_";
	workingFileName += CreateTimeString();
	return workingFileName;
}

NFmiString NFmiDataLoadingInfo::CreateDataBaseInFileNameTimeFilter(int /* theIndex */ )
{ // HUOM!!! onko t‰m‰ sama koodi kahdesti!!!!!
	NFmiString dataBaseFileName("");
	dataBaseFileName += DataBaseInPath();
	// Jos filtteri on jo annettu kokonaisuudessaan ohjaustiedostossa,
	// k‰ytet‰‰n suoraan sit‰ eik‰ rakenneta
	if(DataBaseFileNameIn().Search(reinterpret_cast<const unsigned char *>("*")))
		dataBaseFileName += DataBaseFileNameIn();
	else
	{
		dataBaseFileName += DataBaseFileNameIn();
		dataBaseFileName += "_";
		dataBaseFileName += itsAreaIdentifier;
		dataBaseFileName += "_";
		int length = DataLengthInHours();
		if(length > 0)
			dataBaseFileName += NFmiStringTools::Convert<int>(length);
		dataBaseFileName += CreateTimeString(false);
	}
	return dataBaseFileName;
}

static NFmiString MakeTimeStampForFileName(const std::string &theTimeStringTemplate, const NFmiMetTime &theTime, bool fUseWallClockTimeInTimeStamp)
{
    if(fUseWallClockTimeInTimeStamp)
    {
        NFmiTime aTime;
        NFmiTime utcTime = aTime.UTCTime();
        utcTime.SetSec(aTime.GetSec());
        return utcTime.ToStr(NFmiString(theTimeStringTemplate), kEnglish);
    }
    else
        return theTime.ToStr(NFmiString(theTimeStringTemplate), kEnglish);
}

NFmiString NFmiDataLoadingInfo::CreateTimeString(bool fileLength, float theTimeResolutionInMinutes)
{
	NFmiString fileName;//("T‰t‰_ei_ole_viel‰_toteutettu");
	if(itsMetEditorModeDataWCTR)
	{
		const NFmiTimeBag& tmpTimes = itsMetEditorModeDataWCTR->MaximalCoverageTimeBag();
		int resolution = tmpTimes.Resolution()/60;
		if(fileLength)
		{
			fileName += NFmiString(NFmiStringTools::Convert<int>(itsMetEditorModeDataWCTR->WantedDataLengthInHours()));
			fileName += "_";
			if(itsMetEditorModeDataWCTR->EditorMode() == NFmiMetEditorModeDataWCTR::kOperativeWCTR)
				fileName += "x";
			else
				fileName += NFmiString(NFmiStringTools::Convert<int>(resolution));
			fileName += "_";
            fileName += ::MakeTimeStampForFileName(itsFileNameTimeStampTemplate, tmpTimes.FirstTime(), fUseWallClockTimeInTimeStamp);
		}
		else
		{
			if(fileLength)
			{
				fileName += NFmiString(NFmiStringTools::Convert<int>(itsMetEditorModeDataWCTR->WantedDataLengthInHours()));
				fileName += "_";
			}
			if(theTimeResolutionInMinutes != kFloatMissing)
				resolution = static_cast<int>(theTimeResolutionInMinutes/60);
			fileName += NFmiString(NFmiStringTools::Convert<int>(resolution));
			fileName += "_";
            fileName += ::MakeTimeStampForFileName(itsFileNameTimeStampTemplate, tmpTimes.FirstTime(), fUseWallClockTimeInTimeStamp);
		}
	}
	return fileName;
}

//--------------------------------------------------------
// CreateWorkingFileName 
//--------------------------------------------------------
//   Luo tiedoston nimi talletuksen yhteydess‰, 
//   mukaan liitet‰‰n tyˆversion indeksi. "KEPA_SUOMI_48_1_3112_V1.sqd"
//   Versio saadaan katsomalla jo olemassa olevia tyˆtiedostoja.
//
NFmiString NFmiDataLoadingInfo::CreateWorkingFileName(int theVersion)
{
	NFmiString workingFileName("");
	workingFileName += itsWorkingPath;
	workingFileName += itsWorkingFileName;
	workingFileName += "_";
	workingFileName += itsAreaIdentifier;
	workingFileName += "_";
	workingFileName += CreateTimeString();
	workingFileName += "_V";

	// tein 0 ja 00 lis‰y systeemin versionumeron eteen, ett‰ lataus dialogissa versiot n‰kyv‰t j‰rjestyksess‰ (aakkosj‰rjestyksess‰)
	if(theVersion < 10)
		workingFileName += "00";
	else if(theVersion < 100)
		workingFileName += "0";
	workingFileName += NFmiString(NFmiStringTools::Convert<int>(theVersion));
	workingFileName += ".sqd";
	return workingFileName;
}

//--------------------------------------------------------
// CreateDataBaseFileName 
//--------------------------------------------------------
//   Luo tiedoston nimi tietokantaan viennin yhteydess‰, 
//   mukaan liitet‰‰n DB-version indeksi. "KEPA_SUOMI_48_1_3112_DB1.sqd"
//   Versio saadaan katsomalla jo olemassa olevia tiedostoja.
//
NFmiString NFmiDataLoadingInfo::CreateDataBaseOutFileName(int theVersion)
{
	NFmiString dataBaseFileName("");
	dataBaseFileName += itsDataBaseOutPath;
	dataBaseFileName += DataBaseFileNameOut();
	dataBaseFileName += "_";
	dataBaseFileName += itsAreaIdentifier;
	dataBaseFileName += "_";
	dataBaseFileName += CreateTimeString();
	dataBaseFileName += "_DB";
	dataBaseFileName += NFmiString(NFmiStringTools::Convert<int>(theVersion));
	dataBaseFileName += ".sqd";
	return dataBaseFileName;
}

int NFmiDataLoadingInfo::CalculateVersion(unsigned long index, const NFmiString& name)
{
	int version = 0;
	for( ; isdigit(name[index]); index++)
	{
		version += (name[index]-'0');
		version *= 10;
	}
	if(version != 0)
		version /= 10;
	return version;
}

//--------------------------------------------------------
// LatestWorkingVersion 
//--------------------------------------------------------
//   K‰y tyˆtiedostot l‰pi ja etsii viimeisen version.
//
int NFmiDataLoadingInfo::LatestWorkingVersion()
{
	NFmiString filter = CreateWorkingFileNameTimeFilter();
	int version = 0;
	if(itsWorkingFileNameList.Reset())
	{
		do
		{
			NFmiString name = itsWorkingPath;
			name += *itsWorkingFileNameList.Current();
			if(name.Search(filter))
			{
				unsigned long index = name.SearchLast(NFmiString("_"));
				if(name[++index] == 'V')
				{
					int temp = CalculateVersion(index+1, name);
					if(version < temp)
						version = temp;
				}
			}
		} while(itsWorkingFileNameList.Next());
	}
	return version;
}

//--------------------------------------------------------
// LatestDataBaseVersion 
//--------------------------------------------------------
//   Etsii viimeisen tietokantaan viedyn tiedoston version.
//

int NFmiDataLoadingInfo::LatestDataBaseInVersion(int theIndex)
{
	NFmiString filterTmp = CreateDataBaseInFileNameTimeFilter(theIndex);
	int version = 0;
	// jotain h‰mminki‰ vanhassa koodissa, ei v‰ltt‰m‰tt‰ ole hakenut viimeisimm‰n tietokanta datan DB-numeroa
	// korjasin koodin uuteen uskoon etsim‰ll‰ viimeisimm‰n tiedoston aikaleiman avulla.
	std::string filter(filterTmp);
	std::string foundFileName;
	NFmiFileSystem::FindFile(filter, true, &foundFileName);
	NFmiString foundFileName2(foundFileName.c_str());
	NFmiString newDateString(CreateTimeString());
	if(foundFileName2.Search(newDateString)) // katsotaan lˆytyykˆ samalla aikaleimalla se viimeisin tietokanta tiedosto
	{
		unsigned long index = foundFileName2.SearchLast(NFmiString("_"));
		if(foundFileName2[++index] == 'D')
		{
			int temp = CalculateVersion(index+2, foundFileName2);
			if(version < temp)
				version = temp;
		}
	}
	return version;
}

// korjaa path-parametri referenssiksi ja metodi niin, ettei se aina yrit‰ luoda kaikkia hakemistoja
bool NFmiDataLoadingInfo::CheckAndCreateDirectory(const NFmiString& thePath)
{
    if(NFmiFileSystem::DirectoryExists(thePath.CharPtr()))
		return true; // jos hakemisto on jo olemassa, turha tehd‰ muuta kuin palauttaa true
	NFmiString path(thePath);
	path += "/";
	unsigned long index = 1;
	for(; index < path.GetLen(); )
	{
		index = path.Search(NFmiString("/"),index+1);
		std::string name(path.GetChars(1,index-1));
		NFmiFileSystem::CreateDirectory(name);
	}
	return true;
}

bool NFmiDataLoadingInfo::ReadList(NFmiSortedStringList& fileNameList, const NFmiString& filter)
{
	if(filter.GetLen() <= 0)
		return false;

	std::string filePattern = filter;
	std::string directory = NFmiFileSystem::PathFromPattern(filePattern);
	if(NFmiFileSystem::CreateDirectory(directory) == false)
		return false;
	std::list<std::string> fileList = NFmiFileSystem::PatternFiles(filePattern);
	fileNameList.Clear(true);
	for(std::list<std::string>::iterator it = fileList.begin(); it != fileList.end(); ++it)
	{
		fileNameList.Add(new NFmiString(*it));
	}
	return true;

/*
	NFmiFileString usedFileFilterStr(filter);
	usedFileFilterStr.NormalizeDelimiter();

	fileNameList.Clear(true);
	struct _finddata_t fileinfo;
	long handle;

	unsigned long index = usedFileFilterStr.SearchLast(NFmiString("\\"));
	if(index == 0)
		return false;
	NFmiString directory("");
	directory += usedFileFilterStr.GetChars(1,index-1);

	if(!CheckAndCreateDirectory(directory))
		return false;
	NFmiString* fileName = 0;
	if((handle = static_cast<long>(_findfirst(usedFileFilterStr, &fileinfo ))) != -1)
	{
		if(fileinfo.attrib != 16) // ei ole hakemisto
		{
			fileName = new NFmiString(fileinfo.name);
			fileNameList.Add(fileName);
		}
	}
	else
		return false;

	while(!_findnext( handle, &fileinfo ))
	{
		if(fileinfo.attrib != 16)
		{
			fileName = new NFmiString(fileinfo.name);
			fileNameList.Add(fileName);
		}
	}
	fileNameList.Reset();
	_findclose(handle);

	return true;
*/
}

bool NFmiDataLoadingInfo::InitHirlamFileNameList()
{
	return ReadList(itsHirlamFileNameList, Model1FilePattern());
}

bool NFmiDataLoadingInfo::InitEcmwfFileNameList()
{
	return ReadList(itsEcmwfFileNameList, Model2FilePattern());
}

bool NFmiDataLoadingInfo::InitWorkingFileNameList()
{
	NFmiString filter = CreateWorkingFileNameFilter();
	return ReadList(itsWorkingFileNameList,filter);
}

bool NFmiDataLoadingInfo::InitDataBaseInFileNameList1()
{
	NFmiString filter = CreateDataBaseInFileNameFilter(1);
	return ReadList(itsDataBaseInFileNameList1,filter);
}

NFmiString NFmiDataLoadingInfo::NewFileName()
{
	NFmiString fileNameWithPath("");
	int version = LatestWorkingVersion();
	fileNameWithPath = CreateWorkingFileName(version+1);
	return fileNameWithPath;
}

NFmiString NFmiDataLoadingInfo::Model1Path(void)
{
	NFmiFileString fileStr(Model1FilePattern());
	NFmiString filePath = fileStr.Device();
	filePath += fileStr.Path();
	return filePath;
}

NFmiString NFmiDataLoadingInfo::Model2Path(void)
{
	NFmiFileString fileStr(Model2FilePattern());
	NFmiString filePath = fileStr.Device();
	filePath += fileStr.Path();
	return filePath;
}

void NFmiDataLoadingInfo::UpdateSelectedFileNamesVector(void)
{
	itsSelectedFileNamesVector.clear();
	NFmiString fileName(""); 
	NFmiString fileNameWithPath("");

	fileName = itsComboHirlamFileName;
	fileNameWithPath = Model1Path();
	fileNameWithPath += fileName;
	itsSelectedFileNamesVector.push_back(fileNameWithPath);

	fileName = itsComboEcmwfFileName;
	fileNameWithPath = Model2Path();
	fileNameWithPath += fileName;
	itsSelectedFileNamesVector.push_back(fileNameWithPath);

	fileName = itsComboWorkingFileName;
	fileNameWithPath = itsWorkingPath;
	fileNameWithPath += fileName;
	itsSelectedFileNamesVector.push_back(fileNameWithPath);

	fileName = itsComboDataBase1FileName;
	fileNameWithPath = DataBaseInPath();
	fileNameWithPath += fileName;
	itsSelectedFileNamesVector.push_back(fileNameWithPath);
}

int NFmiDataLoadingInfo::ForecastingLength(void) const
{
	if(itsMetEditorModeDataWCTR)
		return itsMetEditorModeDataWCTR->WantedDataLengthInHours();
	return kShortMissing;
}

const NFmiTimeBag& NFmiDataLoadingInfo::MaximalCoverageTimeBag(void) const
{
	static NFmiTimeBag dummy;
	if(itsMetEditorModeDataWCTR)
		return itsMetEditorModeDataWCTR->MaximalCoverageTimeBag();
	return dummy;
}

const NFmiTimeDescriptor& NFmiDataLoadingInfo::LoadedTimeDescriptor(void) const
{
	return itsMetEditorModeDataWCTR->TimeDescriptor();
}

const NFmiTimeDescriptor& NFmiDataLoadingInfo::UpdatedTimeDescriptor(bool useCurrentAsStart) const
{
	NFmiMetTime timeNow;
	if(itsCaseStudyTime != NFmiMetTime::gMissingTime)
		timeNow = itsCaseStudyTime;
	if(itsMetEditorModeDataWCTR)
		return itsMetEditorModeDataWCTR->TimeDescriptor(timeNow, timeNow, true, useCurrentAsStart);
	else
	{
		static NFmiTimeDescriptor dummy;
		return dummy;
	}
}

const NFmiString& NFmiDataLoadingInfo::Model1FilePattern(void) 
{
	return fUseDataCache ? itsModel1CacheFilePattern : itsModel1FilePattern;
}

void NFmiDataLoadingInfo::Model1FilePattern(const NFmiString &newValue)
{
	if(fUseDataCache)
		itsModel1CacheFilePattern = newValue;
	else
		itsModel1FilePattern = newValue;
}

const NFmiString& NFmiDataLoadingInfo::Model2FilePattern(void) 
{
	return fUseDataCache ? itsModel2CacheFilePattern : itsModel2FilePattern;
}

void NFmiDataLoadingInfo::Model2FilePattern(const NFmiString &newValue)
{
	if(fUseDataCache)
		itsModel2CacheFilePattern = newValue;
	else
		itsModel2FilePattern = newValue;
}

const NFmiString& NFmiDataLoadingInfo::DataBaseInPath(void)
{
	return fUseDataCache ? itsCacheDir : itsDataBaseInPath;
}

void NFmiDataLoadingInfo::DataBaseInPath(const NFmiString &newValue)
{
	if(fUseDataCache)
		itsCacheDir = newValue;
	else
		itsDataBaseInPath = newValue;
}
