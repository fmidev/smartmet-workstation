//© Ilmatieteenlaitos/software by Marko
//  Original 31.08.2004
//
//
//-------------------------------------------------------------------- NFmiSynopPlotView.h

#pragma once

#include "NFmiStationView.h"
#include "NFmiFastInfoUtils.h"

class NFmiArea;
class NFmiToolBox;
class NFmiDrawingEnvironment;
class NFmiDrawParam;
class NFmiSynopPlotSettings;

//_________________________________________________________ NFmiSynopPlotView

class NFmiSynopPlotView : public NFmiStationView
{
public:
	NFmiSynopPlotView(int theMapViewDescTopIndex, std::shared_ptr<NFmiArea> &theArea
					,NFmiToolBox * theToolBox
					,std::shared_ptr<NFmiDrawParam> &theDrawParam
					,FmiParameterName theParamId
					,int theRowIndex
                    ,int theColumnIndex);
    ~NFmiSynopPlotView(void);
	void Draw(NFmiToolBox * theGTB) override;
	std::string ComposeToolTipText(const NFmiPoint& theRelativePoint) override;
	void DrawSynopPlot(NFmiToolBox * theGTB, const NFmiLocation &theLocation, const NFmiRect &theRect, const NFmiMetTime &theTime, double theFontSize, bool fDrawSoundingPlot, bool fDrawMinMaxPlot, bool fDrawMetarPlot);

protected:
    void SetMapViewSettings(std::shared_ptr<NFmiFastQueryInfo> &theUsedInfo) override;
private:
	float GetSynopPlotValue(std::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theParam);
	bool DrawWindVector(std::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiRect &theRect, bool &fWindDrawed, bool metarCase);
	bool DrawNormalFontValues(std::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiRect &theRect);
	bool PrintParameterValue(std::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiRect &theSynopRect, const NFmiPoint &theRelLocation, FmiParameterName theWantedParameter, bool fDoMetarPlotString = false);
	bool DrawSynopFontValues(std::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiRect &theRect, bool fWindDrawed);
	void DrawSynopPlot(double plotSpacing, NFmiRect &theSynopRect, NFmiDrawingEnvironment &theStationPointEnvi, std::vector<NFmiRect> &theSynopRects, NFmiRect &theEmptySoundingMarkerRect, bool drawStationMarker);
	NFmiRect CalcBaseEmptySoundingMarker(void);
	void DrawEmptySoundingMarker(const NFmiRect &theEmptySoundingMarkerRect);
	bool DrawMinMaxPlot(std::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiRect &theDrawRect);
	bool DrawMetarPlot(std::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiRect &theDrawRect);
    bool GetDataFromLocalInfo() const;

	NFmiString GetSnowDepthStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter);
	NFmiString GetVisibilityStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetCloudHeightStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetLowCloudCoverStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo);
	void GetWindValues(std::shared_ptr<NFmiFastQueryInfo> &theInfo, float &theWindSpeed, float &theWindDir);
	NFmiString GetPresentWeatherStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetAviVisStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetWindGustStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetPressureTendencyStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetTotalCloudinessStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetClStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetCmStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetStateOfGroundStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter);
	NFmiString GetChStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetPastWeatherStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter);
	NFmiString GetPressureStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter, bool fDoMetarPlotString);
	float GetPressureChangeValue(std::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetPressureChangeStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter);
	void SetPressureChangeColor(NFmiDrawingEnvironment & theDrawingEnvi, std::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetTemperatureStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter, bool fDoMetarPlotString);
	NFmiString GetHeightStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetMinMaxTStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theParam);
	NFmiString GetPrecipitationAmountStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString Get12or24HourRainAmountStr(std::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter);

	std::vector<NFmiRect> itsExistingSynopPlots; // laitetaan kaikki laatikot, joihin on synop havainto plotattu talteen t‰nne, ett‰ ei tule piirretty‰ niit‰ p‰‰llekk‰in
	int itsFontSizeX;
	int itsFontSizeY;
	bool fSoundingPlotDraw; // onko kyseess‰ luotau-synop-plot menossa?
	bool fMinMaxPlotDraw; // onko kyseess‰ luotau-synop-plot menossa?
	bool fMetarPlotDraw; // onko kyseess‰ metar-plot menossa?

	std::vector<FmiParameterName> itsQ2WantedParamVector; // jos dataa haetaan q2-serverilt‰, haetaan n‰m‰ parametrit
};

