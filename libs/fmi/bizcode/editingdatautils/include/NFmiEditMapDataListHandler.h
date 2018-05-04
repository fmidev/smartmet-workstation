// NFmiEditMapDataListHandler.h: interface for the NFmiEditMapDataListHandler class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NFmiParameterName.h"

class NFmiIndexMessageList;
class NFmiStringList;

class NFmiEditMapDataListHandler  
{
public:
	NFmiEditMapDataListHandler();
	virtual ~NFmiEditMapDataListHandler();
	void Init(const std::string &theHelpDataPath);

	NFmiIndexMessageList* TotalCloudinessSymbolList(void){return itsTotalCloudinessSymbolList;};
	NFmiIndexMessageList* PrecipitationFormSymbolList(void){return itsPrecipitationFormSymbolList;};
	NFmiIndexMessageList* PastWeatherSymbolList(void){return itsPastWeatherSymbolList;};

	NFmiIndexMessageList* ParamPictureList(FmiParameterName paramName);
	NFmiStringList* ParamValueList(FmiParameterName paramName);


private:
    bool InitFogValueList(void);
    bool InitPrecipitationFormValueList(void);
    bool InitPrecipitationTypeValueList(void);
    bool InitHessaaValueList(void);
    bool InitCloudSymbolList(void);
    bool InitTotalCloudinessSymbolList(void);
    bool InitPrecipitationFormSymbolList(void);
    bool InitPastWeatherSymbolList(void);
    bool InitHSade1List(void);
    
    // Jotkut listat alustetaan help_data hakemiston tiedostoista
    std::string itsHelpDataPath;

    NFmiIndexMessageList* itsHSade1List;
	NFmiIndexMessageList* itsCloudSymbolList;
	NFmiIndexMessageList* itsTotalCloudinessSymbolList;
	NFmiIndexMessageList* itsPrecipitationFormSymbolList;
	NFmiIndexMessageList* itsPastWeatherSymbolList;
	NFmiStringList* itsFogIntensityValueList;
	NFmiStringList* itsPrecipitationTypeValueList;
	NFmiStringList* itsPrecipitationFormValueList;
	NFmiStringList* itsHessaaValueList;
};
