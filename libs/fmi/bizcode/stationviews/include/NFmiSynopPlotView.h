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
	NFmiSynopPlotView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
					,NFmiToolBox * theToolBox
					,boost::shared_ptr<NFmiDrawParam> &theDrawParam
					,FmiParameterName theParamId
					,int theRowIndex
                    ,int theColumnIndex);
    ~NFmiSynopPlotView(void);
	void Draw(NFmiToolBox * theGTB) override;
	std::string ComposeToolTipText(const NFmiPoint& theRelativePoint) override;
	void DrawSynopPlot(NFmiToolBox * theGTB, const NFmiLocation &theLocation, const NFmiRect &theRect, const NFmiMetTime &theTime, double theFontSize, bool fDrawSoundingPlot, bool fDrawMinMaxPlot, bool fDrawMetarPlot);

protected:
    void SetMapViewSettings(boost::shared_ptr<NFmiFastQueryInfo> &theUsedInfo) override;
private:
	float GetSynopPlotValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theParam);
	bool DrawWindVector(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiRect &theRect, bool &fWindDrawed, bool metarCase);
	bool DrawNormalFontValues(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiRect &theRect);
	bool PrintParameterValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiRect &theSynopRect, const NFmiPoint &theRelLocation, FmiParameterName theWantedParameter, bool fDoMetarPlotString = false);
	bool DrawSynopFontValues(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiRect &theRect, bool fWindDrawed);
	void DrawSynopPlot(double plotSpacing, NFmiRect &theSynopRect, NFmiDrawingEnvironment &theStationPointEnvi, std::vector<NFmiRect> &theSynopRects, NFmiRect &theEmptySoundingMarkerRect, bool drawStationMarker);
	NFmiRect CalcBaseEmptySoundingMarker(void);
	void DrawEmptySoundingMarker(const NFmiRect &theEmptySoundingMarkerRect);
	bool DrawMinMaxPlot(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiRect &theDrawRect);
	bool DrawMetarPlot(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiRect &theDrawRect);
    bool GetDataFromLocalInfo() const;

	NFmiString GetSnowDepthStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter);
	NFmiString GetVisibilityStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetCloudHeightStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetLowCloudCoverStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	void GetWindValues(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, float &theWindSpeed, float &theWindDir);
	NFmiString GetPresentWeatherStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetAviVisStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetWindGustStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetPressureTendencyStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetTotalCloudinessStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetClStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetCmStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetStateOfGroundStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter);
	NFmiString GetChStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetPastWeatherStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter);
	NFmiString GetPressureStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter, bool fDoMetarPlotString);
	float GetPressureChangeValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetPressureChangeStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter);
	void SetPressureChangeColor(NFmiDrawingEnvironment & theDrawingEnvi, boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetTemperatureStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter, bool fDoMetarPlotString);
	NFmiString GetHeightStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString GetMinMaxTStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theParam);
	NFmiString GetPrecipitationAmountStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
	NFmiString Get12or24HourRainAmountStr(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theWantedParameter);

	std::vector<NFmiRect> itsExistingSynopPlots; // laitetaan kaikki laatikot, joihin on synop havainto plotattu talteen t‰nne, ett‰ ei tule piirretty‰ niit‰ p‰‰llekk‰in
	int itsFontSizeX;
	int itsFontSizeY;
	bool fSoundingPlotDraw; // onko kyseess‰ luotau-synop-plot menossa?
	bool fMinMaxPlotDraw; // onko kyseess‰ luotau-synop-plot menossa?
	bool fMetarPlotDraw; // onko kyseess‰ metar-plot menossa?

	std::vector<FmiParameterName> itsQ2WantedParamVector; // jos dataa haetaan q2-serverilt‰, haetaan n‰m‰ parametrit
};

