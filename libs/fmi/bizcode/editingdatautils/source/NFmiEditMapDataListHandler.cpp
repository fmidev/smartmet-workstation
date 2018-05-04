// NFmiEditMapDataListHandler.cpp: implementation of the NFmiEditMapDataListHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "NFmiEditMapDataListHandler.h"
#include "nindmesl.h"
#include "NFmiStatusString.h"
#include "NFmiStringList.h"
#include "NFmiDictionaryFunction.h"
#include <fstream>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

NFmiEditMapDataListHandler::NFmiEditMapDataListHandler()
:itsHSade1List(0)
,itsCloudSymbolList(0)
,itsTotalCloudinessSymbolList(0)
,itsPrecipitationFormSymbolList(0)
,itsPastWeatherSymbolList(0)
,itsFogIntensityValueList(0)
,itsPrecipitationTypeValueList(0)
,itsPrecipitationFormValueList(0)
,itsHessaaValueList(0)
{
}

NFmiEditMapDataListHandler::~NFmiEditMapDataListHandler()
{
	delete itsHSade1List;
	delete itsCloudSymbolList;
	delete itsTotalCloudinessSymbolList;
	delete itsPrecipitationFormSymbolList;
	delete itsPastWeatherSymbolList;
	delete itsFogIntensityValueList;
	delete itsPrecipitationTypeValueList;
	delete itsPrecipitationFormValueList;
	delete itsHessaaValueList;
}

void NFmiEditMapDataListHandler::Init(const std::string &theHelpDataPath)
{
    itsHelpDataPath = theHelpDataPath;

	InitFogValueList();
	InitPrecipitationFormValueList();
	InitPrecipitationTypeValueList();
	InitHessaaValueList();
	InitCloudSymbolList();
	InitTotalCloudinessSymbolList();
	InitPrecipitationFormSymbolList();
	InitPastWeatherSymbolList();
	InitHSade1List();
}

bool NFmiEditMapDataListHandler::InitFogValueList(void)
{
	if(itsFogIntensityValueList)
	{
		itsFogIntensityValueList->Clear(true);
		delete itsFogIntensityValueList;
	}

	itsFogIntensityValueList = new NFmiStringList();
	NFmiStatusString* temp = 0;
	temp = new NFmiStatusString(::GetDictionaryString("WeatherAndCloudinessNoFog"), 0);
	itsFogIntensityValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("WeatherAndCloudinessModerateFog"), 1);
	itsFogIntensityValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("WeatherAndCloudinessDenseFog"), 2);
	itsFogIntensityValueList->Add(temp);
	return true;
}

bool NFmiEditMapDataListHandler::InitPrecipitationFormValueList(void)
{
	if(itsPrecipitationFormValueList)
	{
		itsPrecipitationFormValueList->Clear(true);
		delete itsPrecipitationFormValueList;
	}
	
	itsPrecipitationFormValueList = new NFmiStringList();

	NFmiStatusString* temp = 0;

	temp = new NFmiStatusString(::GetDictionaryString("WeatherAndCloudinessDrizzle"), 0);
	itsPrecipitationFormValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("WeatherAndCloudinessRain"), 1);
	itsPrecipitationFormValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("WeatherAndCloudinessSleet"), 2);
	itsPrecipitationFormValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("WeatherAndCloudinessSnow"), 3);
	itsPrecipitationFormValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("WeatherAndCloudinessFreezingDrizzle"), 4);
	itsPrecipitationFormValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("WeatherAndCloudinessFreezingRain"), 5);
	itsPrecipitationFormValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("WeatherAndCloudinessHail"), 6);
	itsPrecipitationFormValueList->Add(temp);
    temp = new NFmiStatusString(::GetDictionaryString("Snow grain"), 7);
    itsPrecipitationFormValueList->Add(temp);
    temp = new NFmiStatusString(::GetDictionaryString("Ice pellet"), 8);
    itsPrecipitationFormValueList->Add(temp);
    return true;
}

bool NFmiEditMapDataListHandler::InitPrecipitationTypeValueList(void)
{
	if(itsPrecipitationTypeValueList)
	{
		itsPrecipitationTypeValueList->Clear(true);
		delete itsPrecipitationTypeValueList;
	}
	itsPrecipitationTypeValueList = new NFmiStringList();
	
	NFmiStatusString* temp = 0;

	temp = new NFmiStatusString(::GetDictionaryString("WeatherAndCloudinessLargeScale"), 1);
	itsPrecipitationTypeValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("WeatherAndCloudinessShower"), 2);
	itsPrecipitationTypeValueList->Add(temp);
	return true;
}

bool NFmiEditMapDataListHandler::InitHessaaValueList(void)
{
	if(itsHessaaValueList)
	{
		itsHessaaValueList->Clear(true);
		delete itsHessaaValueList;
	}
	itsHessaaValueList = new NFmiStringList();
	NFmiStatusString* temp = 0;

	temp = new NFmiStatusString(::GetDictionaryString("HessaaSunny"), 1);
	itsHessaaValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("HessaaHalfCloudy"), 2);
	itsHessaaValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("HessaaCloudy"), 3);
	itsHessaaValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("HessaaLightRainShowers"), 21);
	itsHessaaValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("HessaaRainShowers"), 22);
	itsHessaaValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("HessaaHeavyRainShowers"), 23);
	itsHessaaValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("HessaaLightRain"), 31);
	itsHessaaValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("HessaaRain"), 32);
	itsHessaaValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("HessaaHeavyRain"), 33);
	itsHessaaValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("HessaaLightSnowShowers"), 41);
	itsHessaaValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("HessaaSnowShowers"), 42);
	itsHessaaValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("HessaaHeavySnowShowers"), 43);
	itsHessaaValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("HessaaLightSnow"), 51);
	itsHessaaValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("HessaaSnow"), 52);
	itsHessaaValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("HessaaHeavySnow"), 53);
	itsHessaaValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("HessaaThunder"), 61);
	itsHessaaValueList->Add(temp);
	temp = new NFmiStatusString(::GetDictionaryString("HessaaHeavyThunder"), 62);
	itsHessaaValueList->Add(temp);
    temp = new NFmiStatusString(::GetDictionaryString("Light sleet showers"), 71);
    itsHessaaValueList->Add(temp);
    temp = new NFmiStatusString(::GetDictionaryString("Sleet showers"), 72);
    itsHessaaValueList->Add(temp);
    temp = new NFmiStatusString(::GetDictionaryString("Heavy sleet showers"), 73);
    itsHessaaValueList->Add(temp);
    temp = new NFmiStatusString(::GetDictionaryString("Light sleet"), 81);
    itsHessaaValueList->Add(temp);
    temp = new NFmiStatusString(::GetDictionaryString("Sleet"), 82);
    itsHessaaValueList->Add(temp);
    temp = new NFmiStatusString(::GetDictionaryString("Heavy sleet"), 83);
    itsHessaaValueList->Add(temp);

	return true;
}

bool NFmiEditMapDataListHandler::InitHSade1List(void)
{
	if(itsHSade1List)
		delete itsHSade1List;
	itsHSade1List = new NFmiIndexMessageList;

	std::ifstream in(itsHelpDataPath + "//HSade1.txt", std::ios::binary);
	if(in)
	{
		in >> *itsHSade1List;
		in.close();
	}
	else
		return false;
	return true;
}

//--------------------------------------------------------
// InitCloudSymbolList 
//--------------------------------------------------------
bool NFmiEditMapDataListHandler::InitCloudSymbolList(void)
{
	if(itsCloudSymbolList)
		delete itsCloudSymbolList;
	itsCloudSymbolList = new NFmiIndexMessageList;
	itsCloudSymbolList->Add(NFmiStatusString("A",0));
	itsCloudSymbolList->Add(NFmiStatusString("B",1));
	itsCloudSymbolList->Add(NFmiStatusString("C",2));
	itsCloudSymbolList->Add(NFmiStatusString("D",3));
	itsCloudSymbolList->Add(NFmiStatusString("E",4));
	itsCloudSymbolList->Add(NFmiStatusString("F",5));
	itsCloudSymbolList->Add(NFmiStatusString("G",6));
	itsCloudSymbolList->Add(NFmiStatusString("H",7));
	itsCloudSymbolList->Add(NFmiStatusString("I",8));
	itsCloudSymbolList->Add(NFmiStatusString("J",9));
	itsCloudSymbolList->Add(NFmiStatusString("K",10));
	itsCloudSymbolList->Add(NFmiStatusString("L",11));
	itsCloudSymbolList->Add(NFmiStatusString("M",12));
	itsCloudSymbolList->Add(NFmiStatusString("N",13));
	itsCloudSymbolList->Add(NFmiStatusString("O",14));
	itsCloudSymbolList->Add(NFmiStatusString("P",15));
	itsCloudSymbolList->Add(NFmiStatusString("Q",16));
	itsCloudSymbolList->Add(NFmiStatusString("R",17));
	itsCloudSymbolList->Add(NFmiStatusString("S",18));
	itsCloudSymbolList->Add(NFmiStatusString("T",19));
	itsCloudSymbolList->Add(NFmiStatusString("U",20));
	itsCloudSymbolList->Add(NFmiStatusString("V",21));
	itsCloudSymbolList->Add(NFmiStatusString("X",22));
	itsCloudSymbolList->Add(NFmiStatusString("Y",23));
	itsCloudSymbolList->Add(NFmiStatusString("Z",24));
	return true;
}

bool NFmiEditMapDataListHandler::InitTotalCloudinessSymbolList(void)
{
	if(itsTotalCloudinessSymbolList)
		delete itsTotalCloudinessSymbolList;
	itsTotalCloudinessSymbolList = new NFmiIndexMessageList;
	itsTotalCloudinessSymbolList->Add(NFmiStatusString("`",0));
	itsTotalCloudinessSymbolList->Add(NFmiStatusString("a",1));
	itsTotalCloudinessSymbolList->Add(NFmiStatusString("b",2));
	itsTotalCloudinessSymbolList->Add(NFmiStatusString("c",3));
	itsTotalCloudinessSymbolList->Add(NFmiStatusString("d",4));
	itsTotalCloudinessSymbolList->Add(NFmiStatusString("e",5));
	itsTotalCloudinessSymbolList->Add(NFmiStatusString("f",6));
	itsTotalCloudinessSymbolList->Add(NFmiStatusString("g",7));
	itsTotalCloudinessSymbolList->Add(NFmiStatusString("h",8));
	itsTotalCloudinessSymbolList->Add(NFmiStatusString("i",9));
	return true;
}

// kTDrizzle = 0;
// kTRain = 1;
// kTSleet = 2;
// kTSnow = 3;
// kTFreezingDrizzle = 4;
// kTFreezingRain = 5;
// kTHail = 6;
// 7 = Snow grain
// 8 = Ice pellet
bool NFmiEditMapDataListHandler::InitPrecipitationFormSymbolList(void)
{
	if(itsPrecipitationFormSymbolList)
		delete itsPrecipitationFormSymbolList;
	itsPrecipitationFormSymbolList = new NFmiIndexMessageList;
	itsPrecipitationFormSymbolList->Add(NFmiStatusString("Ï",0));
	itsPrecipitationFormSymbolList->Add(NFmiStatusString("Ù",1));
	itsPrecipitationFormSymbolList->Add(NFmiStatusString("á",2));
	itsPrecipitationFormSymbolList->Add(NFmiStatusString("ã",3));
	itsPrecipitationFormSymbolList->Add(NFmiStatusString("Õ",4));
	itsPrecipitationFormSymbolList->Add(NFmiStatusString("ß",5));
    itsPrecipitationFormSymbolList->Add(NFmiStatusString("6", 6));
    itsPrecipitationFormSymbolList->Add(NFmiStatusString("ê", 7));
    itsPrecipitationFormSymbolList->Add(NFmiStatusString("ì", 8));
	return true;
}

bool NFmiEditMapDataListHandler::InitPastWeatherSymbolList(void)
{
	if(itsPastWeatherSymbolList)
		delete itsPastWeatherSymbolList;
	itsPastWeatherSymbolList = new NFmiIndexMessageList;
	itsPastWeatherSymbolList->Add(NFmiStatusString("1",3));
	itsPastWeatherSymbolList->Add(NFmiStatusString("2",4));
	itsPastWeatherSymbolList->Add(NFmiStatusString("3",5));
	itsPastWeatherSymbolList->Add(NFmiStatusString("4",6));
	itsPastWeatherSymbolList->Add(NFmiStatusString("5",7));
	itsPastWeatherSymbolList->Add(NFmiStatusString("6",8));
	itsPastWeatherSymbolList->Add(NFmiStatusString("7",9));
	return true;
}

NFmiIndexMessageList* NFmiEditMapDataListHandler::ParamPictureList(FmiParameterName paramName)
{
	NFmiIndexMessageList* list = 0;
	switch (paramName)
	{
	case kFmiWeatherSymbol1:
	case kFmiWeatherSymbol2:
	case kFmiPresentWeather: // 25.1.2000/Marko
	case kFmiAviationWeather1:
	case kFmiAviationWeather2:
	case kFmiAviationWeather3:
		list = itsHSade1List;
		break;
	case kFmiCloudSymbol:
		list = itsCloudSymbolList;
		break;
	default:
		break;
	}
	return list;
}

NFmiStringList* NFmiEditMapDataListHandler::ParamValueList(FmiParameterName paramName)
{
	NFmiStringList* list = 0;
	switch (paramName)
	{
	case kFmiPrecipitationForm:
    case kFmiPotentialPrecipitationForm:
		return itsPrecipitationFormValueList;
	case kFmiPrecipitationType:
    case kFmiPotentialPrecipitationType:
		return itsPrecipitationTypeValueList;  
    case kFmiFogIntensity:
		return itsFogIntensityValueList;
	case kFmiWeatherSymbol3:
		return itsHessaaValueList;
	default:
		break;
	}
	return list;
}
