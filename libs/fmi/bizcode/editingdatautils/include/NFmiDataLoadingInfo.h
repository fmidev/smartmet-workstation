//**********************************************************
// C++ Class Name : NFmiDataLoadingInfo 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiDataLoadingInfo.h 
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
//  Author         : pietarin 
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

#pragma once

#include "NFmiString.h"
#include "NFmiProducer.h"
#include "nsorstrl.h"
#include "NFmiDataMatrix.h"
#include "NFmiMetTime.h"

class NFmiTimeDescriptor;
class NFmiHPlaceDescriptor;
class NFmiMetEditorModeDataWCTR;
class NFmiTimeBag;

class NFmiDataLoadingInfo
{

 public:
	
	NFmiDataLoadingInfo(void);
	virtual  ~NFmiDataLoadingInfo(void);

	void InitFileNameLists(void);
		
	NFmiString CreateWorkingFileName (int theVersion);
	NFmiString CreateDataBaseOutFileName (int theVersion);

	int LatestWorkingVersion (void);
	int LatestDataBaseInVersion (int theIndex);

	bool InitHirlamFileNameList (void);
	bool InitEcmwfFileNameList (void);
	bool InitWorkingFileNameList (void);
	bool InitDataBaseInFileNameList1 (void); //listoja tarvitaan kahdet, sill‰ dialogissa on samaan aikaan molemmat listat

	NFmiString Model1Path(void);
	NFmiString Model2Path(void);
	const NFmiString& AreaIdentifier(void) {return itsAreaIdentifier;}

	const NFmiProducer& Producer(void) const {return itsProducer;} 
	void Producer(const NFmiProducer& theProducer){itsProducer = theProducer;}
   
	NFmiString Model1NameInDialog(void) {return itsModel1NameInDialog;}
	NFmiString Model2NameInDialog(void) {return itsModel2NameInDialog;}
	const NFmiString& WorkingNameInDialog(void) const {return itsWorkingNameInDialog;}
	void WorkingNameInDialog(const NFmiString& theWorkingNameInDialog){itsWorkingNameInDialog = theWorkingNameInDialog;}
	const NFmiString& DataBaseNameInDialog1(void) const {return itsDataBaseNameInDialog1;}
	void DataBaseNameInDialog1(const NFmiString& theDataBaseNameInDialog){itsDataBaseNameInDialog1 = theDataBaseNameInDialog;}

	const NFmiString& Model1FilePattern(void);
	void Model1FilePattern(const NFmiString &newValue);
	const NFmiString& Model2FilePattern(void);
	void Model2FilePattern(const NFmiString &newValue);

	const NFmiString& WorkingFileName(void){return itsWorkingFileName;}
	void WorkingFileName(const NFmiString& theWorkingFileName){itsWorkingFileName = theWorkingFileName;}
	const NFmiString& DataBaseFileNameIn(void){return itsDataBaseFileNameIn;}
	void DataBaseFileNameIn(const NFmiString& theDataBaseFileNameIn){itsDataBaseFileNameIn = theDataBaseFileNameIn;}
	const NFmiString& DataBaseFileNameOut(void){return fUseDataBaseOutName ? itsDataBaseFileNameOut : itsDataBaseFileNameIn;}
	void DataBaseFileNameOut(const NFmiString& theDataBaseFileNameOut){itsDataBaseFileNameOut = theDataBaseFileNameOut; 
																		if(!fUseDataBaseOutName)
																			itsDataBaseFileNameIn = theDataBaseFileNameOut;}
	
	const NFmiString& WorkingPath(void){return itsWorkingPath;}
	void WorkingPath(const NFmiString& theWorkingPath){itsWorkingPath = theWorkingPath;}
	const NFmiString& DataBaseInPath(void);
	void DataBaseInPath(const NFmiString &newValue);
	const NFmiString& DataBaseOutPath(void){return itsDataBaseOutPath;}
	void DataBaseOutPath(const NFmiString& theDataBaseOutPath){itsDataBaseOutPath = theDataBaseOutPath;}
	
	NFmiSortedStringList& HirlamFileNameList(void){return itsHirlamFileNameList;}
	void HirlamFileNameList(NFmiSortedStringList& theHirlamFileNameList){itsHirlamFileNameList = theHirlamFileNameList;}
	NFmiSortedStringList& EcmwfFileNameList(void){return itsEcmwfFileNameList;}
	void EcmwfFileNameList(NFmiSortedStringList& theEcmwfFileNameList){itsEcmwfFileNameList = theEcmwfFileNameList;}
	NFmiSortedStringList& WorkingFileNameList(void){return itsWorkingFileNameList;}
	void WorkingFileNameList(NFmiSortedStringList& theWorkingFileNameList){itsWorkingFileNameList = theWorkingFileNameList;}
	NFmiSortedStringList& DataBaseInFileNameList1(void){return itsDataBaseInFileNameList1;}
	void DataBaseInFileNameList1(NFmiSortedStringList& theDataBaseInFileNameList){itsDataBaseInFileNameList1 = theDataBaseInFileNameList;}

	int TimeInterpolationSetting(void){return itsTimeInterpolationSetting;}
	void TimeInterpolationSetting(int theTimeInterpolationSetting){itsTimeInterpolationSetting = theTimeInterpolationSetting;}
	int ParamCombinationSetting(void){return itsParamCombinationSetting;}
	void ParamCombinationSetting(int theParamCombinationSetting){itsParamCombinationSetting = theParamCombinationSetting;}
	int PrimaryProducerSetting(void){return itsPrimaryProducerSetting;}
	void PrimaryProducerSetting(int thePrimaryProducerSetting){itsPrimaryProducerSetting = thePrimaryProducerSetting;}
	int SecondaryProducerSetting(void){return itsSecondaryProducerSetting;}
	void SecondaryProducerSetting(int theSecondaryProducerSetting){itsSecondaryProducerSetting = theSecondaryProducerSetting;}
	int ThirdProducerSetting(void){return itsThirdProducerSetting;}
	void ThirdProducerSetting(int theThirdProducerSetting){itsThirdProducerSetting = theThirdProducerSetting;}

	int HirlamFileNameListIndex(void){return itsHirlamFileNameListIndex;}
	void HirlamFileNameListIndex(int theHirlamFileNameListIndex){itsHirlamFileNameListIndex = theHirlamFileNameListIndex;}
	int EcmwfFileNameListIndex(void){return itsEcmwfFileNameListIndex;}
	void EcmwfFileNameListIndex(int theEcmwfFileNameListIndex){itsEcmwfFileNameListIndex = theEcmwfFileNameListIndex;}
	int WorkingFileNameListIndex(void){return itsWorkingFileNameListIndex;}
	void WorkingFileNameListIndex(int theWorkingFileNameListIndex){itsWorkingFileNameListIndex = theWorkingFileNameListIndex;}
	int DataBaseFileNameListIndex1(void){return itsDataBaseFileNameListIndex1;}
	void DataBaseFileNameListIndex1(int theDataBaseFileNameListIndex){itsDataBaseFileNameListIndex1 = theDataBaseFileNameListIndex;}

	void HPlaceDescriptor(NFmiHPlaceDescriptor* theHPlaceDescriptor){itsHPlaceDescriptor = theHPlaceDescriptor;}
	NFmiHPlaceDescriptor* HPlaceDescriptor(void){return itsHPlaceDescriptor;}

	NFmiString NewFileName(void);
	const NFmiString& ComboHirlamFileName(void){return itsComboHirlamFileName;};
	void ComboHirlamFileName(const NFmiString& newName){itsComboHirlamFileName = newName;};
	const NFmiString& ComboEcmwfFileName(void){return itsComboEcmwfFileName;};
	void ComboEcmwfFileName(const NFmiString& newName){itsComboEcmwfFileName = newName;};
	const NFmiString& ComboWorkingFileName(void){return itsComboWorkingFileName;};
	void ComboWorkingFileName(const NFmiString& newName){itsComboWorkingFileName = newName;};
	const NFmiString& ComboDataBase1FileName(void){return itsComboDataBase1FileName;};
	void ComboDataBase1FileName(const NFmiString& newName){itsComboDataBase1FileName = newName;};
	const NFmiString& ComboDataBase2FileName(void){return itsComboDataBase2FileName;};
	void ComboDataBase2FileName(const NFmiString& newName){itsComboDataBase2FileName = newName;};

	NFmiString CreateWorkingFileNameFilter (void);
	NFmiString CreateDataBaseInFileNameFilter (int theIndex);

	void UpdateSelectedFileNamesVector(void);
	const std::vector<NFmiString>& SelectedFileNamesVector(void) const{return itsSelectedFileNamesVector;};

	NFmiMetEditorModeDataWCTR* MetEditorModeDataWCTR(void) {return itsMetEditorModeDataWCTR;}
	int ForecastingLength(void) const;
	const NFmiTimeBag& MaximalCoverageTimeBag(void) const;
	const NFmiTimeDescriptor& LoadedTimeDescriptor(void) const;
	const NFmiTimeDescriptor& UpdatedTimeDescriptor(void) const;
	int DataLengthInHours(void) {return itsDataLengthInHours;}
	const NFmiMetTime& CaseStudyTime(void) const {return itsCaseStudyTime;}
	void CaseStudyTime(const NFmiMetTime &newValue) {itsCaseStudyTime = newValue;}
	bool UseDataCache(void) const {return fUseDataCache;}
	void UseDataCache(bool newValue) {fUseDataCache = newValue;}

	/**
	* Initializes instance from global settings.
	*/
	void Configure(const std::string& theAbsoluteWorkingDirectory, const std::string& theCacheBaseDir, const std::string &theCacheLocalDir, bool useDataCache);
	NFmiDataLoadingInfo& operator=(NFmiDataLoadingInfo& theInfo);

private:	
	NFmiString MakeCacheFilePattern(const NFmiString &theNormalFilePattern);
	void NormalizeAllPathDelimiters(const std::string& theAbsoluteWorkingDirectory);
	NFmiDataLoadingInfo(NFmiDataLoadingInfo& theInfo); // esto, ei toteutusta
	NFmiString CreateWorkingFileNameTimeFilter (void);

	NFmiString CreateDataBaseInFileNameTimeFilter (int theIndex);

	int CalculateVersion(unsigned long index, const NFmiString& name);
	NFmiString CreateTimeString(bool fileLength = true, float theTimeResolutionInMinutes = kFloatMissing);

	bool ReadList(NFmiSortedStringList& fileNameList, const NFmiString& filter);
	bool CheckAndCreateDirectory(const NFmiString& thePath);

	static float itsFileVersionNumber;

	NFmiString itsAreaIdentifier;
	NFmiProducer itsProducer;

	NFmiString itsModel1NameInDialog;
	NFmiString itsModel2NameInDialog;
	//   T‰m‰ nimi on dialogissa talletettujen tiedostojen kohdalla ('omat')
	NFmiString itsWorkingNameInDialog;
	//   T‰m‰ nimi on dialogissa kantaan vietyjen tiedostojen kohdalla ('viralliset')
	NFmiString itsDataBaseNameInDialog1;

	NFmiString itsModel1FilePattern;
	NFmiString itsModel2FilePattern;
	NFmiString itsWorkingFileName;
	NFmiString itsDataBaseFileNameIn; // 11.6.2001/Marko Muutin DB nimirakenteen in+out:ksi. Koska Aluetoimistot alkavat lukemaan Kepa:a ja l‰hett‰v‰t ShortRangea
									  // lis‰ksi In nimeen voidaan laittaa tiedoston filtteri nimi kokonaisuudessaa, koska aluetoimiston tietojen avulla ei voi rakentaa nimi filtteri‰ oikein
	NFmiString itsDataBaseFileNameOut;
		
	NFmiString itsWorkingPath;
	NFmiString itsDataBaseInPath;
	NFmiString itsDataBaseOutPath;
	bool fUseDataBaseOutName; // versio 2. k‰ytt‰‰ molemmiss‰ In versiota, mutta 3:sta eteenp‰in in ja out:ia

	NFmiString itsComboHirlamFileName;
	NFmiString itsComboEcmwfFileName;
	NFmiString itsComboWorkingFileName;
	NFmiString itsComboDataBase1FileName;
	NFmiString itsComboDataBase2FileName;

	//oikeat tiedostonnimilistat
	NFmiSortedStringList itsHirlamFileNameList;
	NFmiSortedStringList itsEcmwfFileNameList;
	NFmiSortedStringList itsWorkingFileNameList;
	NFmiSortedStringList itsDataBaseInFileNameList1; //listoja pit‰‰ olla kahdet

	//   Dialogin muisti (radio-buttonin asento)
	int itsTimeInterpolationSetting;
	int itsParamCombinationSetting;
	int itsPrimaryProducerSetting;
	int itsSecondaryProducerSetting;
	int itsThirdProducerSetting;

	int itsHirlamFileNameListIndex;
	int itsEcmwfFileNameListIndex;
	int itsWorkingFileNameListIndex;
	int itsDataBaseFileNameListIndex1;
	NFmiMetEditorModeDataWCTR *itsMetEditorModeDataWCTR;

	NFmiHPlaceDescriptor* itsHPlaceDescriptor;

	std::vector<NFmiString> itsSelectedFileNamesVector; // dialogissa valitut tiedostonimet talletetaan t‰h‰n
	int itsDataLengthInHours;

	// SmartMetin cache-moodissa data kopioidaan kovalevylle ja pit‰‰ olla polut ja file-patternit osoittamaan myˆs niihin.
	bool fUseDataCache;
	NFmiString itsCacheDir;
	NFmiString itsModel1CacheFilePattern;
	NFmiString itsModel2CacheFilePattern;
	NFmiMetTime itsCaseStudyTime; // jos t‰ss‰ on joku muu arvo kuin NFmiMetTime::gMissingTime, k‰ytet‰‰n t‰t‰ kun rakenntetaan editoitavan datan timeDescriptoria
    std::string itsFileNameTimeStampTemplate; // T‰m‰n avulla tehd‰‰n working ja tietokantaan l‰hetett‰v‰n tiedoston aikaleima. Oletusarvona "DDHH", voidaan halutessa asettaa konfiguraatioissa MetEditor::DataLoadingInfo::FileNameTimeStampTemplate -asetuksella.
    bool fUseWallClockTimeInTimeStamp; // Jos TimeStamp templaatissa on mukana mm osio (eli minuutit), otetaan timeStamp sein‰kelloajasta
};

