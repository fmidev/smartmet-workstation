#include "NFmiTempView.h"
#include "NFmiToolBox.h"
#include "NFmiDrawingEnvironment.h"
#include "NFmiLine.h"
#include "NFmiRectangle.h"
#include "NFmiStringTools.h"
#include "NFmiText.h"
#include "NFmiMTATempSystem.h"
#include "NFmiValueString.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiWindBarb.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiTotalWind.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiAngle.h"
#include "NFmiPolyline.h"
#include "NFmiDrawParam.h"
#include "NFmiSoundingFunctions.h"
#include "NFmiSoundingIndexCalculator.h"
#include "NFmiProducerSystem.h"
#include "NFmiMetMath.h"
#include "NFmiColorSpaces.h"
#include "NFmiTempLineInfo.h"
#include "NFmiTempLabelInfo.h"
#include "CtrlViewFunctions.h"
#include "CtrlViewDocumentInterface.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "NFmiFastInfoUtils.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "ToolBoxStateRestorer.h"
#include "SoundingDataServerConfigurations.h"
#include "catlog/catlog.h"
#include "ModelDataServerConfiguration.h"
#include "SoundingViewSettingsFromWindowsRegisty.h"
#include "NFmiHelpDataInfo.h"
#include "ColorStringFunctions.h"
#include "NFmiDataModifierAvg.h"
#include "CtrlViewFunctions.h"
#include "NFmiDataModifierMinMax.h"

#include <stdexcept>
#include "boost\math\special_functions\round.hpp"

#include <thread>

using namespace std;
using namespace Gdiplus;

#ifdef max
#undef max
#endif

static std::string GetStrValue(float value, int decimalCount, int totalChars)
{
	std::string str;
	if(value != kFloatMissing)
	{
		if(decimalCount == 0)
			str += NFmiStringTools::Convert(::round(value));
		else 
		{
			std::string formatStr;
			if(decimalCount == 1)
				formatStr = "%0.1f";
			else if(decimalCount == 2)
				formatStr = "%0.2f";
			else
				formatStr = "%0.3f";
			NFmiValueString valStr(value, formatStr.c_str());
			str += valStr;
		}
	}
	else
		str += "-";
	if(str.size() < (size_t)totalChars)
	{
		for(auto i = str.size(); i < (size_t)totalChars; i++)
			str += " ";
	}
	return str;
}

static bool SetHelpLineDrawingAttributes(NFmiToolBox *theTB, NFmiDrawingEnvironment *theEnvi, const NFmiTempLabelInfo &theLabelInfo, const NFmiTempLineInfo &theLineInfo, int &theTrueLineWidth, bool isHelpLine)
{
	bool drawSpecialLines = false;
	theTrueLineWidth = theLineInfo.Thickness();
	if(!(theLineInfo.LineType() == FMI_SOLID || theLineInfo.LineType() == FMI_RELATIVE_FILL))
		if(theTrueLineWidth > 1)
			drawSpecialLines = true;
	if(drawSpecialLines)
		theEnvi->SetPenSize(NFmiPoint(1,1));
	else
		theEnvi->SetPenSize(NFmiPoint(theLineInfo.Thickness(), theLineInfo.Thickness()));
	theEnvi->SetFillPattern(theLineInfo.LineType());
	int fontSize = theLabelInfo.FontSize();
	theEnvi->SetFontSize(NFmiPoint(fontSize, fontSize));
	theTB->SetTextAlignment(theLabelInfo.TextAlignment());
	if(isHelpLine) // apuviivojen kanssa perusteellisemmat asetukset kuin l‰mpp‰ri ja kastepiste luotaus viivojen kanssa
	{
		theEnvi->SetFrameColor(theLineInfo.Color());
	}
	return drawSpecialLines;
}

static NFmiPoint CalcReltiveMoveFromPixels(NFmiToolBox *theTB, const NFmiPoint &thePixelMove)
{
	return NFmiPoint(theTB->SX(static_cast<long>(thePixelMove.X())), theTB->SY(static_cast<long>(thePixelMove.Y())));
}

static void SetLocationNameByItsLatlon(NFmiProducerSystem &theProdSystem, NFmiLocation &theLocation, const NFmiProducer &theProducer, const NFmiMetTime &theOriginTime, bool fIsGrid = false)
{
	std::string name;
    if(fIsGrid == false && (NFmiInfoOrganizer::IsTempData(theProducer.GetIdent(), true)))
		name += std::string(theLocation.GetName()) + " ";
	else
	{ // etsi mallin nimi
		unsigned int modelIndex = theProdSystem.FindProducerInfo(theProducer);
		if(modelIndex > 0)
			name += theProdSystem.Producer(modelIndex).UltraShortName();
		else
			name += "X?";
		name += theOriginTime.ToStr(::GetDictionaryString("MapViewToolTipOrigTimeNormal"));
		name += " ";
	}
	std::string latlonStr;
	latlonStr += CtrlViewUtils::GetLatitudeMinuteStr(theLocation.GetLatitude(), 0);
	latlonStr += ",";
	latlonStr += CtrlViewUtils::GetLongitudeMinuteStr(theLocation.GetLongitude(), 0);
	name += latlonStr.c_str();

	theLocation.SetName(name);
}

static void SetMovingSoundingLocationName(NFmiLocation &theLocation, const NFmiProducer &theProducer)
{
	std::string name(theProducer.GetName());
	name += " ";
	std::string latlonStr;
	latlonStr += CtrlViewUtils::GetLatitudeMinuteStr(theLocation.GetLatitude(), 0);
	latlonStr += ",";
	latlonStr += CtrlViewUtils::GetLongitudeMinuteStr(theLocation.GetLongitude(), 0);
	name += latlonStr.c_str();

	theLocation.SetName(name);
}

static Gdiplus::Color NFmiColor2GdiplusColor(const NFmiColor &theColor)
{
    return Gdiplus::Color(static_cast<BYTE>(theColor.GetRed() * 255), static_cast<BYTE>(theColor.GetGreen() * 255), static_cast<BYTE>(theColor.GetBlue() * 255));
}

class GdiplusGraphicsSmoothModeRestorer
{
    Gdiplus::Graphics& graphics_;
    Gdiplus::SmoothingMode oldMode_;
public:
    GdiplusGraphicsSmoothModeRestorer(Gdiplus::Graphics& graphics, Gdiplus::SmoothingMode wantedSmoothMode, bool printing, bool rectangularLine)
        :graphics_(graphics)
        ,oldMode_(graphics.GetSmoothingMode())
    {
        if(wantedSmoothMode == Gdiplus::SmoothingMode::SmoothingModeAntiAlias && !rectangularLine && !printing)
            graphics_.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
        else
            graphics_.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighSpeed);
    }

    ~GdiplusGraphicsSmoothModeRestorer()
    {
        graphics_.SetSmoothingMode(oldMode_);
    }


};

static void DrawGdiplusCurve(Gdiplus::Graphics &theGraphics, std::vector<PointF> &thePoints, const NFmiTempLineInfo &theLineInfo, Gdiplus::SmoothingMode theWantedSmoothMode, bool fPrinting, bool fRectangularLine)
{
    if(thePoints.size() > 0)
    {
        double lineThicknessFactor = 1.;
        if(thePoints.size() == 1)
            lineThicknessFactor = 3.; // jos vain yksi piste k‰yr‰ss‰ piirr‰ se paksummalla
        Pen pen(::NFmiColor2GdiplusColor(theLineInfo.Color()), static_cast<REAL>(theLineInfo.Thickness()*lineThicknessFactor));

        // piirret‰‰n normaali viiva-tyyppi ja yli yhden pikselin muutkin viivat normaalist1
        if(fPrinting == false && (theLineInfo.LineType() == FMI_SOLID || theLineInfo.Thickness() > 1))
            pen.SetDashStyle(static_cast<Gdiplus::DashStyle>(theLineInfo.LineType()));
        else
        { // mutta yhden pikselin paksuiset muissa perus dasheiss‰ on liian lyhyet ja ep‰selv‰t kuviot, joten teen omat customit
            if(theLineInfo.LineType() == FMI_DASH)
            {
                pen.SetDashStyle(DashStyleCustom);
                REAL dashArray[2] = { 5, 4 };
                pen.SetDashPattern(dashArray, 2);
            }
            else if(theLineInfo.LineType() == FMI_DOT)
            {
                pen.SetDashStyle(DashStyleCustom);
                REAL dashArray[2] = { 2, 3 };
                pen.SetDashPattern(dashArray, 2);
            }
            else if(theLineInfo.LineType() == FMI_DASHDOT)
            {
                pen.SetDashStyle(DashStyleCustom);
                REAL dashArray[4] = { 6, 3, 3, 3 };
                pen.SetDashPattern(dashArray, 4);
            }
            else if(theLineInfo.LineType() == FMI_DASHDOTDOT)
            {
                pen.SetDashStyle(DashStyleCustom);
                REAL dashArray[6] = { 6, 3, 3, 3, 3, 3 };
                pen.SetDashPattern(dashArray, 6);
            }
        }

        GdiplusGraphicsSmoothModeRestorer smoothModeHandler(theGraphics, theWantedSmoothMode, fPrinting, fRectangularLine);
        theGraphics.DrawLines(&pen, &thePoints[0], static_cast<INT>(thePoints.size()));
    }
}

// Tehd‰‰n vaalean harmaa maanalaisten juttujen miksaus v‰riksi
static const NFmiColor gUndergroundMixColor(0.8f, 0.8f, 0.8f);

static NFmiColor MakeUndergroundColor(const NFmiColor& color)
{
	NFmiColor mixedColor(color);
	mixedColor.Mix(gUndergroundMixColor, 0.8f);
	return mixedColor;
}

static NFmiColor MakeUndergroundColorSetup(const NFmiColor& baseColor, const NFmiGroundLevelValue& groundLevelValue, float P)
{
	if(groundLevelValue.IsBelowGroundLevelCase(P))
		return MakeUndergroundColor(baseColor);
	else
		return baseColor;
}

static void MakeUndergroundColorSetup(const NFmiColor& baseColor, const NFmiPoint &basePenSize, const NFmiGroundLevelValue& groundLevelValue, float P, NFmiDrawingEnvironment* envi, bool doFrameColor, bool doFillColor, bool doPenSize)
{
	if(envi)
	{
		auto usedColor = ::MakeUndergroundColorSetup(baseColor, groundLevelValue, P);
		if(doFrameColor)
			envi->SetFrameColor(usedColor);
		if(doFillColor)
			envi->SetFillColor(usedColor);
		if(doPenSize)
		{
			if(groundLevelValue.IsBelowGroundLevelCase(P))
			{
				envi->SetPenSize(NFmiPoint(1, 1));
			}
			else
			{
				envi->SetPenSize(basePenSize);
			}
		}
	}
}

static bool gUseThinLine = true;

static void DrawUndergroundGdiplusCurve(Gdiplus::Graphics& theGraphics, std::vector<PointF>& thePoints, const NFmiTempLineInfo& theLineInfo, Gdiplus::SmoothingMode theWantedSmoothMode, bool fPrinting, bool fRectangularLine)
{
	NFmiTempLineInfo undergroundLineInfo(theLineInfo);
	undergroundLineInfo.Color(::MakeUndergroundColor(undergroundLineInfo.Color()));
	if(gUseThinLine)
	{
		undergroundLineInfo.Thickness(1); // Laitetaan viiva myˆs ohueksi
	}
	::DrawGdiplusCurve(theGraphics, thePoints, undergroundLineInfo, theWantedSmoothMode, fPrinting, fRectangularLine);
}

static void DrawCurveWithPossibleUndergroundSection(Gdiplus::Graphics& theGraphics, std::vector<PointF>& thePointsInOut, std::vector<PointF>& thePossibleUndergroundPointsInOut, const NFmiTempLineInfo& theLineInfo, Gdiplus::SmoothingMode theWantedSmoothMode, bool fPrinting, bool fRectangularLine)
{
	if(!thePossibleUndergroundPointsInOut.empty())
	{
		auto drawedUndergroundPoints = thePossibleUndergroundPointsInOut;
		if(!thePointsInOut.empty())
			drawedUndergroundPoints.push_back(thePointsInOut.front());
		::DrawUndergroundGdiplusCurve(theGraphics, drawedUndergroundPoints, theLineInfo, theWantedSmoothMode, fPrinting, fRectangularLine);
		thePossibleUndergroundPointsInOut.clear();
	}
	::DrawGdiplusCurve(theGraphics, thePointsInOut, theLineInfo, theWantedSmoothMode, fPrinting, fRectangularLine);
	thePointsInOut.clear();
}

TotalSoundingData::TotalSoundingData() = default;

TotalSoundingData::TotalSoundingData(NFmiMTATempSystem& mtaTempSystem)
{
	itsIntegrationRangeInKm = mtaTempSystem.IntegrationRangeInKm();
	itsIntegrationTimeOffset1InHours = mtaTempSystem.IntegrationTimeOffset1InHours();
	itsIntegrationTimeOffset2InHours = mtaTempSystem.IntegrationTimeOffset2InHours();
}

bool TotalSoundingData::IsSameSounding(TotalSoundingData& other)
{
	if(!itsSoundingData.IsSameSounding(other.itsSoundingData))
		return false;
	if(itsIntegrationRangeInKm != other.itsIntegrationRangeInKm)
		return false;
	if(itsIntegrationTimeOffset1InHours != other.itsIntegrationTimeOffset1InHours)
		return false;
	if(itsIntegrationTimeOffset2InHours != other.itsIntegrationTimeOffset2InHours)
		return false;

	return true;
}

bool TotalSoundingData::HasAvgIntegrationData() const
{
	return itsIntegrationPointsCount != 0 || itsIntegrationTimesCount != 0;
}

Gdiplus::SmoothingMode NFmiTempView::GetUsedCurveDrawSmoothingMode() const
{
    // Jos MustDrawTempView on true, on p‰ivitys l‰htenyt karttan‰ytˆn mouse-move drag operaatiosta, ja se  halutaan piirt‰‰ vain nopeasti
    if(itsCtrlViewDocumentInterface->MustDrawTempView())
    {
//        CatLog::logMessage("Mouse-move update, using high-speed curve mode", CatLog::Severity::Trace, CatLog::Category::Visualization);
        return Gdiplus::SmoothingModeHighSpeed;
    }
    else
    {
//        CatLog::logMessage("Non-mouse-move update, using high-quality curve mode", CatLog::Severity::Trace, CatLog::Category::Visualization);
        return itsCurveDrawSmoothingMode;
    }
}

bool NFmiTempView::IsRectangularTemperatureHelperLines() const
{
    if(tdegree == 0 || tdegree == 90)
        return true;
    else
        return false;
}


NFmiTempView::NFmiTempView(const NFmiRect& theRect
						,NFmiToolBox* theToolBox)
:NFmiCtrlView(0, theRect
			 ,theToolBox
			 ,new NFmiDrawingEnvironment()) // t‰m‰ on h‰m‰r‰‰ koodia, mutta hommat on tehty jo aiemmin (luokka suunnittelussa) p‰in peet‰
			 					   // t‰ss‰ luodaan drawingenvi ja annetaan emolle ja otetaan talteen t‰m‰n luokan dataosaan
,itsTempDrawingEnvi(itsDrawingEnvironment) // joka sitten tuhotaan destruktorissa (don't try this at home)
,tmax(50)
,tmin(-80)
,dt(tmax - tmin)
,pmin(100)
,pmax(1050)
,tdegree(45)
,dtperpix(1)
,dlogpperpix(1)
,xpix(1)
,ypix(1)
,itsSelectedProducerSoundingData()
,fHodografInitialized(false)
,fMustResetFirstSoundingData(false)
,itsGdiplusScale(1,1)
,itsDrawSizeFactor(1,1)
,itsLastScreenDrawPixelSizeInMM(0.1, 0.1)
,itsLastScreenDataRectPressureScaleRatio(0)
{
}

NFmiTempView::~NFmiTempView(void)
{
	delete itsTempDrawingEnvi; // koska emo ei tuhoa drawingParamia, se pit‰‰ t‰ss‰ tapuksessa tuhota t‰ss‰, paskaa mutta enjaksa tehd‰ mega muutosta koko Ctrlview systeemiin
}

double NFmiTempView::y2p(double y)
{
	double p = kFloatMissing;
	const auto &dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
	if(y <= dataRect.Bottom() && y >= dataRect.Top())
	{
		double boxh = dataRect.Height();
		double by = dataRect.Bottom();
		double dp = ::log(pmax) - ::log(pmin);
		p = pmax * ::exp((y-by)/(boxh/dp));
	}
	return p;
}

double NFmiTempView::p2y(double p)
{
	const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
	double boxh = dataRect.Height();
	double by = dataRect.Bottom();
	double dp = ::log(pmax) - ::log(pmin);
	return by - (::log(pmax) - ::log(p)) * (boxh / dp);
}

// sijoitetaan data vertikaalisesti standardi ilmakeh‰n mukaan
// h on korkeus metrein‰
double NFmiTempView::h2y(double h)
{
	double p = ::CalcPressureAtHeight(h/1000.);
	return p2y(p);
}

static double deg2rad(double alfa)
{
	double pi = 3.1415926535;
	return alfa * 2 * pi / 360.;
}

double NFmiTempView::pt2x(double p, double t)
{
	const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
	double boxw = dataRect.Width();
	double lx = dataRect.Left();
	double by = dataRect.Bottom();
	double y = p2y(p);

	// 1. mik‰ on dataruudun alaosan pikseli sijainti (ypix0)
	double ypix0 = 0;
	// 2. laske paineen pikseli sijainti (ypix1)
	double ypix1 = itsToolBox->HY(by - y);
	// 3. mik‰ on annetun l‰mpˆtilan pikseli sijainti ruudun alaosassa (xpix0)
	double x0 = lx + (t - tmin) / dt * boxw;
	double xpix0 = itsToolBox->HY(x0);
	// 4. xpix1 = xpix0 + deltax = xpix0 + deltay/tan(alfa)
	double denom = tan(deg2rad(tdegree));
	double xpix1 = denom ? xpix0 + (ypix1 - ypix0)/denom : xpix0;
	// 5. laske x xpix1:n avulla toolboxilla
	double x = x0 + itsToolBox->SX(boost::math::iround(xpix1 - xpix0));
	return x;
}

// laskee relatiivisen pisteen avulla l‰mpˆtilan ruudulla
double NFmiTempView::xy2t(double x, double y)
{
	const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
	double boxw = dataRect.Width();
	double lx = dataRect.Left();
	double by = dataRect.Bottom();

	double deltay = by - y;
	double ypix = itsToolBox->HY(deltay);
	double denom = tan(deg2rad(tdegree));
	double xpix = denom ? ypix/tan(deg2rad(tdegree)) : 0;
	double deltax = itsToolBox->SX(boost::math::iround(xpix));
	double x0 = x - deltax;

	double t = tmin + (x0-lx)*dt/boxw;
	return t;
}

double NFmiTempView::Tpot2x(double tpot, double p)
{
	double t = NFmiSoundingFunctions::Tpot2t(tpot, p);
	double tmp2 = pt2x(p, t);
	return tmp2;
}

// laskee mixing ration (r) ja paineen avulla l‰mpˆtilan
static double rp2t(double r, double p)
{
// johdettu seuraavista kaavoista
// 1. r = 0.622*vp/(p-vp), miss‰ e on vapor pressure
// 2. vp = 6.1121 * exp((17.502*t)/(240.97+t)))   // Buck (1981)

//	t=l‰mpˆtila
//	r=mixing ratio
//	p=paine

// vp = r*p/(a+r)
// X = ln(vp/b)
// t = d*X/(c-X)

	double a=0.622;
	double b=6.1121;
	double c=17.502;
	double d=240.97;

	double vp = (r*p)/(a+r);
	double X = ::log(vp/b);  // log on logaritmi luonnollisesta luvusta
	double t = (d*X)/(c-X);
	return t;
}

// T‰m‰ piirt‰‰ vain apuasteikon vaaka labelit, ei asteikko viivaa.
// theLabelTextAlignment kertoo muiden labeleiden sijoituksen, paitsi 100:n, jolle pit‰‰ antaa oma, jotta se ei menisi yli reunojen.
void NFmiTempView::DrawSecondaryDataHorizontalAxel(NFmiTempLabelInfo &theLabelInfo, NFmiTempLineInfo &theLineInfo, FmiDirection theLabelTextAlignment, double theYPosition, const NFmiPoint &theLabelOffset)
{
    theLabelInfo.TextAlignment(theLabelTextAlignment);
    int trueLineWidth = 0;
    SetHelpLineDrawingAttributes(itsToolBox, itsDrawingEnvironment, theLabelInfo, theLineInfo, trueLineWidth, true);

    NFmiPoint p1(SecondaryDataFrameXoffset(0), theYPosition); // 0-arvon sijainti
    NFmiPoint p2(SecondaryDataFrameXoffset(25), theYPosition); // 25-arvo
    NFmiPoint p3(SecondaryDataFrameXoffset(50), theYPosition); // 50-arvo
    NFmiPoint p4(SecondaryDataFrameXoffset(75), theYPosition); // 75-arvo
    NFmiPoint p5(SecondaryDataFrameXoffset(100), theYPosition); // 100-arvo
    DrawHelpLineLabel(p1, theLabelOffset, 0, theLabelInfo, itsDrawingEnvironment);
	DrawHelpLineLabel(p2, theLabelOffset, 25, theLabelInfo, itsDrawingEnvironment);
	DrawHelpLineLabel(p3, theLabelOffset, 50, theLabelInfo, itsDrawingEnvironment);
	DrawHelpLineLabel(p4, theLabelOffset, 75, theLabelInfo, itsDrawingEnvironment);
	DrawHelpLineLabel(p5, theLabelOffset, 100, theLabelInfo, itsDrawingEnvironment);
}

// Anna apudatalaatikon x-koordinaatti halutulle arvolle v‰lill‰ 0-100
double NFmiTempView::SecondaryDataFrameXoffset(double theValue)
{
	const auto& secondaryDataFrame = itsTempViewDataRects.getSecondaryDataFrame();
	return secondaryDataFrame.Left() + (theValue / 100.) * secondaryDataFrame.Width();
}

// Piirret‰‰n sekund‰‰ridatalaatikon ala- ja yl‰reunoihin pieni asteikko 0 - 100.
// Piirret‰‰n myˆs heiverˆiset pystyapuviivat 0, 50 ja 100 kohtiin.
void NFmiTempView::DrawSecondaryDataRect()
{
    if(!itsCtrlViewDocumentInterface->GetMTATempSystem().DrawSecondaryData())
        return ;

    itsDrawingEnvironment->SetFrameColor(NFmiColor(0, 0, 0));
    itsDrawingEnvironment->SetPenSize(NFmiPoint(1 * itsDrawSizeFactor.X() * ExtraPrintLineThicknesFactor(true), 1 * itsDrawSizeFactor.Y() * ExtraPrintLineThicknesFactor(true)));
	const auto& secondaryDataFrame = itsTempViewDataRects.getSecondaryDataFrame();
	DrawFrame(itsDrawingEnvironment, secondaryDataFrame);

    NFmiTempLabelInfo labelInfo;
    labelInfo.DrawLabelText(true);
    labelInfo.FontSize(boost::math::iround(14 * itsDrawSizeFactor.Y() * ExtraPrintLineThicknesFactor(true)));
    labelInfo.ClipWithDataRect(false);
    NFmiTempLineInfo lineInfo;
    lineInfo.Thickness(boost::math::iround(1 * itsDrawSizeFactor.X() * ExtraPrintLineThicknesFactor(true)));
    lineInfo.DrawLine(false);
    lineInfo.Color(NFmiColor(0.6f, 0.6f, 0.6f)); // t‰m‰ on myˆs label v‰ri

    int trueLineWidth = 0;

    // 1. Piirret‰‰n asteikko alareunaan
    double verticalOffset = ConvertFixedPixelSizeToRelativeHeight(2);
	// Labeleita pit‰‰ siirt‰‰ hieman vertikaali suunnassa, alareunassa pikkuisen ylˆsp‰in (negatiivinen offset)
    NFmiPoint labelOffset(0, -verticalOffset); 
    DrawSecondaryDataHorizontalAxel(labelInfo, lineInfo, kTopCenter, secondaryDataFrame.Bottom(), labelOffset);

    // 2. Piirret‰‰n asteikko yl‰reunaan
	// Labeleita pit‰‰ siirt‰‰ hieman vertikaali suunnassa, yl‰reunassa pikkuisen alasp‰in (positiivinen offset)
    labelOffset.Y(verticalOffset); 
	// Yl‰reunan tektit menev‰t hieman vasemmalle tuntemattomasta syyst‰, ja siksi niit‰ siirret‰‰n hieman oikealla t‰ss‰
	double topHorizontalLabelOffset = ConvertFixedPixelSizeToRelativeWidth(5);
	labelOffset.X(topHorizontalLabelOffset);
	DrawSecondaryDataHorizontalAxel(labelInfo, lineInfo, kBottomCenter, secondaryDataFrame.Top(), labelOffset);

    // 3. Piirret‰‰n vertikaali apuviivat
    lineInfo.Thickness(boost::math::iround(1 * itsDrawSizeFactor.X() * ExtraPrintLineThicknesFactor(true)));
//    lineInfo.LineType(FMI_DASHDOT);
    SetHelpLineDrawingAttributes(itsToolBox, itsDrawingEnvironment, labelInfo, lineInfo, trueLineWidth, true);
    // Piirret‰‰n eri vertikaali apuviivat (0, 25, 50, 75 ja 100)
    DrawSecondaryVerticalHelpLine(secondaryDataFrame.Bottom(), secondaryDataFrame.Top(), 0);
    DrawSecondaryVerticalHelpLine(secondaryDataFrame.Bottom(), secondaryDataFrame.Top(), 25);
    DrawSecondaryVerticalHelpLine(secondaryDataFrame.Bottom(), secondaryDataFrame.Top(), 50);
    DrawSecondaryVerticalHelpLine(secondaryDataFrame.Bottom(), secondaryDataFrame.Top(), 75);
    DrawSecondaryVerticalHelpLine(secondaryDataFrame.Bottom(), secondaryDataFrame.Top(), 100);
}

void NFmiTempView::DrawSecondaryVerticalHelpLine(double theBottom, double theTop, double theValue)
{
    NFmiPoint p1(SecondaryDataFrameXoffset(theValue), theTop);
    NFmiPoint p2(SecondaryDataFrameXoffset(theValue), theBottom);
    NFmiLine l1(p1, p2, 0, itsDrawingEnvironment);
    itsToolBox->Convert(&l1);
}

void NFmiTempView::DrawSecondaryData(NFmiSoundingData &theUsedData, FmiParameterName theParId, const NFmiTempLineInfo &theLineInfo, const NFmiGroundLevelValue& groundLevelValue)
{
    if(theLineInfo.DrawLine() == false)
        return;
    // GDI+ piirto koodia
	const auto& secondaryDataFrame = itsTempViewDataRects.getSecondaryDataFrame();
	itsGdiPlusGraphics->SetClip(CtrlView::Relative2GdiplusRect(itsToolBox, secondaryDataFrame));

    auto &drawedParam = theUsedData.GetParamData(theParId);
	auto &pressures = theUsedData.GetParamData(kFmiPressure);
	auto &heights = theUsedData.GetParamData(kFmiGeomHeight);
    bool useHeight = (theUsedData.PressureDataAvailable() == false && theUsedData.HeightDataAvailable() == true);
    if(drawedParam.size() > 1 && drawedParam.size() == pressures.size() && drawedParam.size() == heights.size())
    {
        int maxMissingValues = theUsedData.ObservationData() ? 15 : 1; // jos per‰kk‰in puuttuu enemm‰n kuin n‰in monta arvoa, ei yhdistet‰ viivoja, vaan katkaistaan
        int consecutiveMissingValues = 0;
        int ssize = static_cast<int>(drawedParam.size());
        std::vector<PointF> points;
		std::vector<PointF> undergroundPoints;
		// Havaittujen luotausten piirrossa halutaan piirt‰‰ yhten‰ist‰ viivaa, vaikka datassa olisikin pieni‰ aukkoja, 
        // sit‰ varten k‰ytet‰‰n (consecutiveMissingValues < maxMissingValues) -tarkastelua.
        // temp2qd-filtteri tuottaa ilmeisesti luotauksia k‰‰nteisess‰ j‰rjestyksess‰ kuin meid‰n kotoisten luotauksien levelit.
        // Nyt k‰vi niin ett‰ Latvialaisten luotauksissa oli jossain tapauksissa ensin n. 100 kpl puuttuvia, sitten vasta tuli
        // leveleit‰ joille lˆytyi arvoja. T‰llˆin ohitettiin aina 1. leveli ja jatkettiin vasta toisesta ei-missing arvoja lˆytyneest‰.
        // Tein doingFirstTimeChecks -muuttujan t‰t‰ tapausta varten.
        bool doingFirstTimeChecks = true;
        for(int i = 0; i<ssize; i++)
        {
            float value = drawedParam[i];
            float p = pressures[i];
            float h = heights[i];
            if(value != kFloatMissing && (useHeight ? (h != kFloatMissing) : (p != kFloatMissing)))
            {
                if(consecutiveMissingValues < maxMissingValues || doingFirstTimeChecks)
                {
                    if(useHeight) // pit‰‰ muuttaa korkeudet paineiksi
                        p = static_cast<float>(::CalcPressureAtHeight(h / 1000.));
                    double x = secondaryDataFrame.Left() + secondaryDataFrame.Width() * value / 100.;
                    double y = p2y(p);
					if(groundLevelValue.IsBelowGroundLevelCase(p))
						undergroundPoints.push_back(PointF(static_cast<REAL>(x * itsGdiplusScale.X()), static_cast<REAL>(y * itsGdiplusScale.Y())));
					else
						points.push_back(PointF(static_cast<REAL>(x*itsGdiplusScale.X()), static_cast<REAL>(y*itsGdiplusScale.Y())));
                    doingFirstTimeChecks = false;
                }
                else // piirret‰‰n p‰tk‰ mik‰ on vektorissa tallessa
                {
					::DrawCurveWithPossibleUndergroundSection(*itsGdiPlusGraphics, points, undergroundPoints, theLineInfo, GetUsedCurveDrawSmoothingMode(), IsPrinting(), false);
                }
            }

            if(value != kFloatMissing && (useHeight ? h != kFloatMissing : p != kFloatMissing))
                consecutiveMissingValues = 0;
            else
                consecutiveMissingValues++;
        }
		// lopuksi viel‰ piirret‰‰n loputkin mit‰ on piirrett‰v‰‰
		::DrawCurveWithPossibleUndergroundSection(*itsGdiPlusGraphics, points, undergroundPoints, theLineInfo, GetUsedCurveDrawSmoothingMode(), IsPrinting(), false);
    }
    itsGdiPlusGraphics->ResetClip();
}

void NFmiTempView::DrawSecondaryData(NFmiSoundingData &theUsedData, const NFmiColor &theUsedSoundingColor, const NFmiGroundLevelValue& groundLevelValue)
{
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    if(mtaTempSystem.DrawSecondaryData())
    {
        NFmiTempLineInfo lineInfo = mtaTempSystem.WSLineInfo();
        lineInfo.Color(theUsedSoundingColor);
        lineInfo.Thickness(boost::math::iround(lineInfo.Thickness() * itsDrawSizeFactor.X() * ExtraPrintLineThicknesFactor(true)));
        DrawSecondaryData(theUsedData, kFmiWindSpeedMS, lineInfo, groundLevelValue); // Piirret‰‰n tuulen nopeus
        lineInfo = mtaTempSystem.NLineInfo();
        lineInfo.Color(theUsedSoundingColor);
        lineInfo.Thickness(boost::math::iround(lineInfo.Thickness() * itsDrawSizeFactor.X() * ExtraPrintLineThicknesFactor(true)));
        DrawSecondaryData(theUsedData, kFmiTotalCloudCover, lineInfo, groundLevelValue); // Piirret‰‰n kokonaispilvisyys
        lineInfo = mtaTempSystem.RHLineInfo();
        lineInfo.Color(theUsedSoundingColor);
        lineInfo.Thickness(boost::math::iround(lineInfo.Thickness() * itsDrawSizeFactor.X() * ExtraPrintLineThicknesFactor(true)));
        DrawSecondaryData(theUsedData, kFmiHumidity, lineInfo, groundLevelValue); // Piirret‰‰n suhteellinen kosteus
    }
}

struct LineLabelDrawData
{
	PointF itsPlace;
	string itsText;
};

static void DrawGdiplusStringVector(Gdiplus::Graphics &theGraphics, std::vector<LineLabelDrawData> lineLabels, const NFmiTempLabelInfo &theLabelInfo, const Gdiplus::Rect &theClippingRect, const NFmiColor &theColor)
{
	if(theLabelInfo.DrawLabelText() == false)
		return ;
	if(theLabelInfo.ClipWithDataRect())
		theGraphics.SetClip(theClippingRect);
	else
		theGraphics.ResetClip();

    Gdiplus::StringFormat stringFormat;
    CtrlView::SetGdiplusAlignment(theLabelInfo.TextAlignment(), stringFormat);

	Gdiplus::Color usedColor(::NFmiColor2GdiplusColor(theColor));
	Gdiplus::SolidBrush aBrush(usedColor);
	size_t ssize = lineLabels.size();
	for(size_t i=0; i<ssize; i++)
	{
		LineLabelDrawData &lineLabelData = lineLabels[i];

		std::wstring fontNameStr(L"Arial");
		Gdiplus::Font aFont(fontNameStr.c_str(), static_cast<REAL>(theLabelInfo.FontSize()), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		std::wstring wString = CtrlView::StringToWString(lineLabelData.itsText);
		theGraphics.DrawString(wString.c_str(), static_cast<INT>(wString.size()), &aFont, lineLabelData.itsPlace, &stringFormat, &aBrush);
	}

	theGraphics.ResetClip(); // lopuksi viela varmuuden vuoksi clippi alueen poisto
}

static void AddLineLabelData(const PointF &thePoint, const PointF &theOffsetPoint, double theValue, vector<LineLabelDrawData> &theLabels)
{
	LineLabelDrawData tmp;
	tmp.itsPlace = thePoint;
	tmp.itsPlace = tmp.itsPlace + theOffsetPoint;
	tmp.itsText = NFmiStringTools::Convert<double>(theValue);
	theLabels.push_back(tmp);
}

double NFmiTempView::CalcDataRectPressureScaleRatio(void)
{
	// T‰m‰ CalculateAllDataViewRelatedRects metodia pit‰‰ kutsua jo t‰‰ll‰, koska
	// t‰‰ll‰ k‰ytet‰‰n itsTempViewDataRects.soundingCurveDataRect_:in muuttujan arvoa.
	CalculateAllDataViewRelatedRects();
	const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
	double pScaleWidth = itsTempViewDataRects.getPressureScaleWidth();
	if(pScaleWidth == 0)
		return 0;
	else
		return dataRect.Width() / pScaleWidth;
}

void NFmiTempView::CalcDrawSizeFactors(void)
{
	CtrlViewUtils::GraphicalInfo &gInfo = itsCtrlViewDocumentInterface->GetMTATempSystem().GetGraphicalInfo();
	if(itsToolBox->GetDC()->IsPrinting())
	{
		itsDrawSizeFactor.X(itsLastScreenDrawPixelSizeInMM.X() / (1./gInfo.itsPixelsPerMM_x));
		itsDrawSizeFactor.Y(itsLastScreenDrawPixelSizeInMM.Y() / (1./gInfo.itsPixelsPerMM_y));

		// muuten ehk‰ pikselien suhteet voisi laskea n‰in, mutta kun n‰ytˆnohjaimiin ei voi luottaa ett‰ ne antaisivat 
		// n‰ytˆn koon oikein millimetreiss‰. T‰st‰ syyst‰ joudun tekem‰‰n virityksen, jolla yritet‰‰n saada
		// viel‰ yksi skaala kerroin peliin. Eli lasketaan CurveDataRect:in ja itsIndexRectin suhteet ja korjataan skaala kertoimia
		// iteratiivisesti (jos index-rect on siis yleens‰ olemassa).
		if(itsLastScreenDataRectPressureScaleRatio)
		{
			for(int i=0; i<3; i++)
			{ // iteroidaan kohti sopivia kertoimia
				double currentDataRectPressureScaleRatio = CalcDataRectPressureScaleRatio();
				itsDrawSizeFactor.X(itsDrawSizeFactor.X() * currentDataRectPressureScaleRatio/itsLastScreenDataRectPressureScaleRatio);
				itsDrawSizeFactor.Y(itsDrawSizeFactor.Y() * currentDataRectPressureScaleRatio/itsLastScreenDataRectPressureScaleRatio);
			}
			// Lasketaan viel‰ kerran printteri‰ varten kaikki piirtoalueet (joka tehd‰‰n myˆs 
			// CalcDataRectPressureScaleRatio metodissa), koska itsDrawSizeFactor:in arvoja on viel‰ p‰ivitelty.
			CalculateAllDataViewRelatedRects();
		}
	}
	else
	{
		itsDrawSizeFactor.X(1);
		itsDrawSizeFactor.Y(1);
		itsLastScreenDrawPixelSizeInMM.X(1./gInfo.itsPixelsPerMM_x);
		itsLastScreenDrawPixelSizeInMM.Y(1./gInfo.itsPixelsPerMM_y);
		itsLastScreenDataRectPressureScaleRatio = CalcDataRectPressureScaleRatio();
	}
}

void NFmiTempView::Draw(NFmiToolBox *theToolBox)
{
    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, __FUNCTION__);

//    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	itsTempViewScrollingData.resetLastTextualSideViewValues();
    if(theToolBox == 0)
		return ;
	itsToolBox = theToolBox;
	itsOperationalMode = SoundingViewOperationMode::NormalDrawMode;

    try
    {
        InitializeGdiplus(itsToolBox, 0);
        CalcDrawSizeFactors();

        // tehd‰‰n GDI+ maailmaan tarvittavat skaala kertoimet
        itsGdiplusScale.X(itsToolBox->GetClientRect().Width());
        itsGdiplusScale.Y(itsToolBox->GetClientRect().Height());

        NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
		SetupTAxisValues(mtaTempSystem.TAxisStart(), mtaTempSystem.TAxisEnd());

        pmin = mtaTempSystem.PAxisEnd();
        pmax = mtaTempSystem.PAxisStart();

        tdegree = mtaTempSystem.SkewTDegree();
		InitializeHodografRect();
        DrawBackground();

        _1PixelInRel.X(ConvertFixedPixelSizeToRelativeWidth(1));
		_1PixelInRel.Y(ConvertFixedPixelSizeToRelativeHeight(1));
		const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
		xpix = itsToolBox->HX(dataRect.Width());
        ypix = itsToolBox->HY(dataRect.Height());
        dtperpix = xpix / dt;
        dlogpperpix = ypix / (::log(pmax) - ::log(pmin));
        DrawSecondaryDataRect();

        {
            ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &dataRect);
            DrawDryAdiapaticks();
            DrawMoistAdiapaticks();
            DrawYAxel();
            DrawXAxel();
            DrawWindModificationArea();
            DrawMixingRatio();
            DrawCondensationTrailProbabilityLines();
            DrawFlightLevelScale();
            DrawHeightScale();

            NFmiPoint oldFontSize(itsDrawingEnvironment->GetFontSize());
            NFmiColor oldFillColor(itsDrawingEnvironment->GetFillColor());
            itsDrawingEnvironment->SetFillColor(NFmiColor(0.9f, 0.9f, 0.9f)); // laitetaan harmaa tausta teksteille, ett‰ ne erottuu
            itsDrawingEnvironment->SetFontSize(NFmiPoint(18 * itsDrawSizeFactor.X(), 18 * itsDrawSizeFactor.Y()));
            itsDrawingEnvironment->SetPenSize(NFmiPoint(1 * itsDrawSizeFactor.X(), 1 * itsDrawSizeFactor.Y()));
			DrawSoundingsInMTAMode();

            // siivotaan piirto ominaisuudet takaisin
            itsDrawingEnvironment->SetFillColor(oldFillColor);
            itsDrawingEnvironment->SetFontSize(oldFontSize);

            // piirr‰ lopuksi vain data alueen frame, koska diagrammin piirros on sotkenut sit‰
            itsDrawingEnvironment->DisableFill();
            itsDrawingEnvironment->SetFrameColor(NFmiColor(0.f, 0.f, 0.f));
            itsDrawingEnvironment->SetPenSize(NFmiPoint(1 * itsDrawSizeFactor.X(), 1 * itsDrawSizeFactor.Y()));
            DrawFrame(itsDrawingEnvironment, dataRect);
        }
		DrawTextualSideViewRelatedStuff();
	} // end of try
	catch(...)
	{
	}
	CleanGdiplus(); // t‰t‰ pit‰‰ kutsua piirron lopuksi InitializeGdiplus -metodin vastin parina.
}

// Eri data alueet pit‰‰ laskea oikeassa j‰rjestyksess‰.
// Niiden laskut aloitetaan oikeasta reunasta ja edet‰‰n vasemmalle.
// Eri n‰ytˆnosiot voivat olla n‰kyviss‰ tai piilossa, joka vaikuttaa aina seuraavien alueiden laskuihin.
// Sijoitetaan kaikki laatikot jopa tyhjin‰ niiden oikeaan paikkaan, jotta niiden reunoja voidaan aina 
// k‰ytt‰‰ loppujen piirtoalueiden laskuissa, t‰m‰ helpottaa logiikkaa laskuissa.
void NFmiTempView::CalculateAllDataViewRelatedRects()
{
	itsTempViewDataRects.calculateAllDataViewRelatedRects(itsRect, itsDrawSizeFactor, itsToolBox);
}

void NFmiTempView::InitializeHodografRect(void)
{
	if(!itsCtrlViewDocumentInterface->GetMTATempSystem().ShowHodograf())
		return;
	auto& hodografViewData = itsCtrlViewDocumentInterface->GetMTATempSystem().GetHodografViewData();
	const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
	double relativeHeight = dataRect.Height() * hodografViewData.RelativiHeightFactor();
	long pixelHeight = itsToolBox->HY(relativeHeight);
	long pixelWidth = pixelHeight;
	double relativeWidth = itsToolBox->SX(pixelWidth);
	double blRelativeOffsetX = ConvertFixedPixelSizeToRelativeWidth(5);
	double blRelativeOffsetY = ConvertFixedPixelSizeToRelativeHeight(5);
	NFmiRect hodografRect = hodografViewData.Rect();
	NFmiPoint centePoint(hodografRect.Center());
	if(fHodografInitialized == false) // 1, kerralla pit‰‰ laskea laatikon koko ja sijainti
	{
		NFmiPoint blOffsetPoint(blRelativeOffsetX, -blRelativeOffsetY);
		NFmiPoint blPoint(dataRect.BottomLeft());
		blPoint += blOffsetPoint;
		hodografRect = NFmiRect(blPoint.X(), blPoint.Y() - relativeHeight, blPoint.X() + relativeWidth, blPoint.Y());
		fHodografInitialized = true;
	}
	else
	{ // lopuksi lasketaan vain laatikon kokoa, suhteellinen keskipisteen sijainti otetaan vanhasta laatikosta
		hodografRect.Width(relativeWidth);
		hodografRect.Height(relativeHeight);
		hodografRect.Center(centePoint);
	}
	hodografViewData.Rect(hodografRect);
}

bool NFmiTempView::IsAnyTextualSideViewVisible() const
{
	auto& soundingSettings = itsCtrlViewDocumentInterface->GetMTATempSystem().GetSoundingViewSettingsFromWindowsRegisty();
	return soundingSettings.ShowStabilityIndexSideView() || soundingSettings.ShowTextualSoundingDataSideView();
}

void NFmiTempView::DrawTextualSideViewRelatedStuff()
{
	// T‰ytyy nollata t‰m‰ teksti, sit‰ p‰ivitet‰‰n aina sitten DrawNextLineToIndexView-metodissa
	itsSoundingIndexStr = ""; 
	if(IsAnyTextualSideViewVisible())
	{
		DrawStabilityIndexData(itsSelectedProducerSoundingData.itsSoundingData);
		DrawTextualSoundingData(itsSelectedProducerSoundingData);
		// Lis‰t‰‰n varmuuden vuoksi rivinvaihto, niin n‰kee paremmin, miss‰ eri tekstiosuudet vaihtuvat
		itsSoundingIndexStr += "\n"; 
		DrawAnimationControls();
	}
}

void NFmiTempView::DrawStabilityIndexBackground(const NFmiRect& sideViewRect)
{
	itsDrawingEnvironment->SetFrameColor(NFmiColor(0,0,0));
	itsDrawingEnvironment->SetFillColor(NFmiColor(1.0f, 1.0f, 1.0f));
	itsDrawingEnvironment->EnableFill();
	DrawFrame(itsDrawingEnvironment, sideViewRect);
}

static std::string GetNameText(const NFmiLocation &location, bool movingSounding)
{
	std::string str;

    if(movingSounding)
    {
        if(location.GetLocation() == NFmiPoint::gMissingLatlon)
            str += "-,-";
        else
        {
            str += CtrlViewUtils::GetLatitudeMinuteStr(location.GetLatitude(), 1);
            str += ",";
            str += CtrlViewUtils::GetLongitudeMinuteStr(location.GetLongitude(), 1);
        }
    }
    else
	{
        if(location.GetIdent() != 0)
        {
            if(location.GetIdent() < 10000)
                str += "0"; // pit‰‰ tarvittaessa laittaa etu nolla aseman identin eteen
            str += NFmiStringTools::Convert(location.GetIdent());
            str += " ";
        }
        str += location.GetName();
    }

	return str;
}
static std::string GetLatText(NFmiSoundingData &theData)
{
	std::string str("SLAT=");
	if(theData.Location().GetLatitude() != kFloatMissing)
		str += CtrlViewUtils::GetLatitudeMinuteStr(theData.Location().GetLatitude(), 1);
	else
		str += " -";
	return str;
}
static std::string GetLonText(NFmiSoundingData &theData)
{
	std::string str("SLON=");
	if(theData.Location().GetLongitude() != kFloatMissing)
		str += CtrlViewUtils::GetLongitudeMinuteStr(theData.Location().GetLongitude(), 1);
	else
		str += " -";
	return str;
}

static std::string GetElevationText(NFmiSoundingData &theData, boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	std::string str("SELEV=");
	if(theInfo && theInfo->Param(kFmiTopoGraf))
	{
		float elevValue = theInfo->InterpolatedValue(theData.Location().GetLocation());
		if(elevValue != kFloatMissing)
			str += NFmiStringTools::Convert<float>(elevValue);
		else
			str += " -";
	}
	else
		str += " -";
	return str;
}

std::string NFmiTempView::GetIndexText(double theValue, const std::string &theText, int theDecimalCount)
{
	std::string str(theText);
	str += "=";
	if(theValue != kFloatMissing)
		str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(theValue, theDecimalCount);
	else
		str += " -";
	return str;
}

double NFmiTempView::CalcRelativeTextLineHeight(int fontSizeInPixels, double heightFactor)
{
	int usedFontSizeInPixels = std::max(1, fontSizeInPixels);
	return itsToolBox->SY(boost::math::iround(usedFontSizeInPixels * heightFactor));
}

void NFmiTempView::MoveToNextLine(double relativeLineHeight, NFmiPoint& theTextPoint)
{ 
	theTextPoint.Y(theTextPoint.Y() + relativeLineHeight);
}

void NFmiTempView::DrawNextLineToIndexView(double relativeLineHeight, NFmiText &theText, const std::string &theStr, NFmiPoint &theTextPoint, bool moveFirst, bool addToString)
{
	if(addToString)
		itsSoundingIndexStr += theStr + "\n";
	if(moveFirst)
		MoveToNextLine(relativeLineHeight, theTextPoint);
	theText.SetRelativeRect(NFmiRect(theTextPoint, theTextPoint));
	theText.SetText(theStr);
	itsToolBox->Convert(&theText);
}

static NFmiTempLineInfo& GetAirParcelLineInfo(NFmiMTATempSystem &theMTATempSystem, FmiLCLCalcType theLCLCalcType)
{
	if(theLCLCalcType == kLCLCalcSurface)
		return theMTATempSystem.AirParcel1LineInfo();
	else if(theLCLCalcType == kLCLCalc500m2)
		return theMTATempSystem.AirParcel2LineInfo();
	else if(theLCLCalcType == kLCLCalcMostUnstable)
		return theMTATempSystem.AirParcel3LineInfo();

	throw std::runtime_error("GetAirParcelLineInfo - wrong calculation type given.");
}

static NFmiTempLabelInfo& GetAirParcelLabelInfo(NFmiMTATempSystem &theMTATempSystem, FmiLCLCalcType theLCLCalcType)
{
	if(theLCLCalcType == kLCLCalcSurface)
		return theMTATempSystem.AirParcel1LabelInfo();
	else if(theLCLCalcType == kLCLCalc500m2)
		return theMTATempSystem.AirParcel2LabelInfo();
	else if(theLCLCalcType == kLCLCalcMostUnstable)
		return theMTATempSystem.AirParcel3LabelInfo();

	throw std::runtime_error("GetAirParcelLabelInfo - wrong calculation type given.");
}

void NFmiTempView::DrawAllLiftedAirParcels(NFmiSoundingData &theData)
{
	DrawLiftedAirParcel(theData, kLCLCalcSurface);
	DrawLiftedAirParcel(theData, kLCLCalc500m2);
	DrawLiftedAirParcel(theData, kLCLCalcMostUnstable);
}

void NFmiTempView::DrawLiftedAirParcel(NFmiSoundingData &theData, FmiLCLCalcType theLCLCalcType)
{
	// 1. calc T,Td,P values from 500 m layer avg or surface values
	double T=kFloatMissing,
		   Td=kFloatMissing,
		   P=kFloatMissing;
	if(!theData.GetValuesNeededInLCLCalculations(theLCLCalcType, T, Td, P))
		return ;

	double pLCL = theData.CalcLCLPressureLevel(theLCLCalcType);

	// 2. lift parcel until its warmer than environment
	// 2.1 first adiabatically till LCL and than moist adiabatically
	// iterate with CalcTOfLiftedAirParcel from 500 m avg P to next sounding pressure level
	// until T-parcel is warmer than T at that pressure level in sounding
	std::deque<float> &pValues = theData.GetParamData(kFmiPressure);
	std::deque<float> &tValues = theData.GetParamData(kFmiTemperature);
	int ssize = static_cast<int>(pValues.size());
	double TofLiftedParcer_previous = kFloatMissing;
	double P_previous = kFloatMissing;

    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    NFmiTempLineInfo lineInfo = ::GetAirParcelLineInfo(mtaTempSystem, theLCLCalcType);
	lineInfo.Thickness(boost::math::iround(lineInfo.Thickness() * itsDrawSizeFactor.X()));
    NFmiTempLabelInfo labelInfo = ::GetAirParcelLabelInfo(mtaTempSystem, theLCLCalcType);
	labelInfo.FontSize(boost::math::iround(labelInfo.FontSize() * itsDrawSizeFactor.Y()));
	labelInfo.StartPointPixelOffSet(ScaleOffsetPoint(labelInfo.StartPointPixelOffSet()));
	int trueLineWidth = boost::math::iround(1 * itsDrawSizeFactor.X());
	bool drawSpecialLines = SetHelpLineDrawingAttributes(itsToolBox, itsDrawingEnvironment, labelInfo, lineInfo, trueLineWidth, true);
	const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
    ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &dataRect);
	for(int i = 0; i < ssize; i++)
	{
		if(pValues[i] != kFloatMissing && pValues[i] <= P) // aloitetaan LFC etsint‰ vasta 'aloitus' korkeuden j‰lkeen
		{
			if(pValues[i] >= 100) // ei piirret‰ pakettien nostoa yli halutun rajan
			{
				if(tValues[i] != kFloatMissing) // kaikilla painepinnoilla ei ole l‰mpˆtilaa
				{
					double TofLiftedParcer = theData.CalcTOfLiftedAirParcel(T, Td, P, pValues[i]);

					if(lineInfo.DrawLine())
					{
						if(P_previous != kFloatMissing && TofLiftedParcer_previous != kFloatMissing)
						{
							if(pValues[i] < pLCL && P_previous > pLCL) // lcl-pisteeseen pit‰‰ saada yksi piirto piste, muuten tulee joskus ruman n‰kˆinen nosto viiva
							{
								double TofLiftedParcerInLCL = theData.CalcTOfLiftedAirParcel(T, Td, P, pLCL);
								double x1 = pt2x(pLCL, TofLiftedParcerInLCL);
								double x2 = pt2x(P_previous, TofLiftedParcer_previous);
								double y1 = p2y(pLCL);
								double y2 = p2y(P_previous);
								DrawLine(NFmiPoint(x1, y1), NFmiPoint(x2, y2), drawSpecialLines, trueLineWidth, true, 0, itsDrawingEnvironment);
								TofLiftedParcer_previous = TofLiftedParcerInLCL;
								P_previous = pLCL;
							}

							double x1 = pt2x(pValues[i], TofLiftedParcer);
							double x2 = pt2x(P_previous, TofLiftedParcer_previous);
							double y1 = p2y(pValues[i]);
							double y2 = p2y(P_previous);
							DrawLine(NFmiPoint(x1, y1), NFmiPoint(x2, y2), drawSpecialLines, trueLineWidth, true, 0, itsDrawingEnvironment);
						}
					}
					TofLiftedParcer_previous = TofLiftedParcer;
					P_previous = pValues[i];
				}
			}
		}
	}
}

NFmiPoint NFmiTempView::CalcStabilityIndexStartPoint(void)
{
	NFmiPoint p(itsTempViewDataRects.getStabilityIndexSideViewRect().TopLeft());
	p.X(p.X() + ConvertFixedPixelSizeToRelativeWidth(2)); // siirret‰‰n teksti‰ pikkusen oikealle p‰in
	return p;
}

void NFmiTempView::DrawTextualSoundingData(TotalSoundingData& usedTotalData)
{
	auto doContinue = DoTextualSideViewSetup(
		itsCtrlViewDocumentInterface->GetMTATempSystem().GetSoundingViewSettingsFromWindowsRegisty().ShowTextualSoundingDataSideView(),
		itsTempViewDataRects.getTextualSoundingDataSideViewRect(),
		itsTempViewDataRects.getTextualSoundingDataFontSize(),
		itsTextualSoundingDataNextLineFontHeightFactor,
		itsTextualSoundingDataRelativeLineHeight);
	if(doContinue)
	{
		// Printataan 1. piirretyn luotauksen data tekstimuodossa sivu n‰yttˆˆn.
		DrawSoundingInTextFormat(usedTotalData);
	}
}

const NFmiColor& NFmiTempView::GetSelectedProducersColor() const
{
	auto& mtaSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
	return mtaSystem.SoundingColor(mtaSystem.GetSelectedProducerIndex(true));
}

bool NFmiTempView::DoTextualSideViewSetup(bool showSideView, const NFmiRect &sideViewRect, int fontSize, double fontHeightFactor, double& relativeLineHeightOut)
{
	if(!showSideView)
		return false;

	DrawStabilityIndexBackground(sideViewRect);

	// Laitetaan toolboxin piirtoasetukset kuntoon ennen lopullisen piirto funktion kutsua.
	// Piirret‰‰n vain 1. luotaukseen liittyv‰t indeksit ja ne piirret‰‰n samalla v‰rilla kuin 1. luotaus
	itsDrawingEnvironment->SetFrameColor(GetSelectedProducersColor());
	int usedFontSize = fontSize;
	if(itsToolBox->GetDC()->IsPrinting())
	{
		// Printatessa pit‰‰ fonttia hieman jostain syyst‰ pienent‰‰, koska muuten tekstit eiv‰t mahdu kokonaisuudessaan ruutuun
		usedFontSize = boost::math::iround(usedFontSize * 0.9);
	}
	usedFontSize = boost::math::iround(usedFontSize * itsDrawSizeFactor.Y());
	itsDrawingEnvironment->SetFontSize(NFmiPoint(usedFontSize, usedFontSize));
	relativeLineHeightOut = CalcRelativeTextLineHeight(usedFontSize, fontHeightFactor);
	itsToolBox->SetTextAlignment(kLeft);
	return true;
}


void NFmiTempView::DrawStabilityIndexData(NFmiSoundingData& usedData)
{
	auto doContinue = DoTextualSideViewSetup(
		itsCtrlViewDocumentInterface->GetMTATempSystem().GetSoundingViewSettingsFromWindowsRegisty().ShowStabilityIndexSideView(),
		itsTempViewDataRects.getStabilityIndexSideViewRect(),
		itsTempViewDataRects.getStabilityIndexFontSize(),
		itsStabilityIndexNextLineFontHeightFactor,
		itsStabilityIndexRelativeLineHeight);
	if(!doContinue)
		return;

	NFmiPoint p(CalcStabilityIndexStartPoint());

	NFmiText text(p, NFmiString(""), false, 0, itsDrawingEnvironment);
	auto lineH = itsStabilityIndexRelativeLineHeight;
	DrawNextLineToIndexView(lineH, text, ::GetNameText(usedData.Location(), usedData.MovingSounding()), p, false);
	DrawNextLineToIndexView(lineH, text, ::GetLatText(usedData), p);
	DrawNextLineToIndexView(lineH, text, ::GetLonText(usedData), p);
	DrawNextLineToIndexView(lineH, text, ::GetElevationText(usedData, itsCtrlViewDocumentInterface->InfoOrganizer()->FindInfo(NFmiInfoData::kStationary)), p);

	// pit‰‰ piirt‰‰ tyhj‰‰ alkuun ett‰ saadaan "kursori-arvoille" tilaa, ne piirret‰‰n DrawOverBitmap-kohdassa
	DrawNextLineToIndexView(lineH, text, "Cursor values:", p);
	DrawNextLineToIndexView(lineH, text, "", p); // T ja P tulee t‰h‰n DrawOverBitmap-kohdassa
	DrawNextLineToIndexView(lineH, text, "", p); // Dry ja Moist tulee t‰h‰n DrawOverBitmap-kohdassa
	DrawNextLineToIndexView(lineH, text, "", p); // Mix tulee t‰h‰n DrawOverBitmap-kohdassa
	DrawNextLineToIndexView(lineH, text, "", p);
	DrawNextLineToIndexView(lineH, text, "", p);
	DrawNextLineToIndexView(lineH, text, "-------------", p);

	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcSHOWIndex(), "SHOW", 1), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcLIFTIndex(), "?LIFT", 1), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcKINXIndex(), "KINX", 1), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcCTOTIndex(), "CTOT", 1), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcVTOTIndex(), "VTOT", 1), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcTOTLIndex(), "TOTL", 1), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcGDI(), "GDI", 1), p);
	DrawNextLineToIndexView(lineH, text, " ", p);

	DrawNextLineToIndexView(lineH, text, "-- Surface --", p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcLCLIndex(kLCLCalcSurface), "LCL", 0), p);
	double ELsur = kFloatMissing;
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcLFCIndex(kLCLCalcSurface, ELsur), "LFC", 0), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(ELsur, "EL", 0), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcCAPE500Index(kLCLCalcSurface), "CAPE", 0), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcCAPE500Index(kLCLCalcSurface, 3000), "0-3kmCAPE", 0), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcCAPE_TT_Index(kLCLCalcSurface, -10, -40), "-10-40CAPE", 0), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcCINIndex(kLCLCalcSurface), "CIN", 0), p);
	DrawNextLineToIndexView(lineH, text, " ", p);


	DrawNextLineToIndexView(lineH, text, "-- 500m mix --", p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcLCLIndex(kLCLCalc500m2), "LCL", 0), p);
	double EL500m2 = kFloatMissing;
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcLFCIndex(kLCLCalc500m2, EL500m2), "LFC", 0), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(EL500m2, "EL", 0), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcCAPE500Index(kLCLCalc500m2), "CAPE", 0), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcCAPE500Index(kLCLCalc500m2, 3000), "0-3kmCAPE", 0), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcCAPE_TT_Index(kLCLCalc500m2, -10, -40), "-10-40CAPE", 0), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcCINIndex(kLCLCalc500m2), "CIN", 0), p);
	DrawNextLineToIndexView(lineH, text, " ", p);


	DrawNextLineToIndexView(lineH, text, "-- Most unstable --", p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcLCLIndex(kLCLCalcMostUnstable), "LCL", 0), p);
	double ELunst = kFloatMissing;
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcLFCIndex(kLCLCalcMostUnstable, ELunst), "LFC", 0), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(ELunst, "EL", 0), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcCAPE500Index(kLCLCalcMostUnstable), "CAPE", 0), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcCAPE_TT_Index(kLCLCalcMostUnstable, -10, -40), "-10-40CAPE", 0), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcCINIndex(kLCLCalcMostUnstable), "CIN", 0), p);
	DrawNextLineToIndexView(lineH, text, " ", p);

	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcBulkShearIndex(0, 6), "0-6km BS", 1), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcBulkShearIndex(0, 1), "0-1km BS", 1), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcSRHIndex(0, 3), "0-3km SRH", 1), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcSRHIndex(0, 1), "0-1km SRH", 1), p);
	DrawNextLineToIndexView(lineH, text, std::string(usedData.Get_U_V_ID_IndexText("L-motion", kLeft)), p);
	DrawNextLineToIndexView(lineH, text, std::string(usedData.Get_U_V_ID_IndexText("MeanWind", kCenter)), p);
	DrawNextLineToIndexView(lineH, text, std::string(usedData.Get_U_V_ID_IndexText("R-motion", kRight)), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcWSatHeightIndex(1500), "WS1500m", 1), p);
	DrawNextLineToIndexView(lineH, text, GetIndexText(usedData.CalcThetaEDiffIndex(0, 3), "0-3kmThetaE", 1), p);
}


std::pair<float, std::string> NFmiTempView::MakeTextualSoundingLevelString(int levelIndex, std::deque<float>& pVec, std::deque<float>& tVec, std::deque<float>& tdVec, std::deque<float>& zVec, std::deque<float>& wsVec, std::deque<float>& wdVec)
{
	float P = pVec[levelIndex];
	// Jos luotauksesta lˆytyy mik‰ tahansa arvo jostain levelilt‰, piirret‰‰n sen rivin tiedot
	if(P == kFloatMissing && tVec[levelIndex] == kFloatMissing && zVec[levelIndex] == kFloatMissing && wsVec[levelIndex] == kFloatMissing && wdVec[levelIndex] == kFloatMissing)
		return std::make_pair(P, ""); // Luotausdataan voi tulla t‰ysin puuttuvia leveleit‰, skipataan ne!!!

	std::string str = "";
	str += GetStrValue(P, 0, 4);
	str += " ";
	str += GetStrValue(tVec[levelIndex], 1, 5);
	str += " ";
	float TdValue = tdVec[levelIndex];
	if(TdValue <= -100.f)
		str += GetStrValue(tdVec[levelIndex], 0, 5); // jos kastepiste on alle -100 tiputetaan desimaalit pois, jotta tila riitt‰‰
	else
		str += GetStrValue(tdVec[levelIndex], 1, 5);
	str += " ";
	str += GetStrValue(zVec[levelIndex], 0, 5);
	str += " ";
	str += GetStrValue(wsVec[levelIndex], 0, 2);
	str += " ";
	str += GetStrValue(wdVec[levelIndex], 0, 3);
	return std::make_pair(P, str);
}

static std::string GetStationsShortName(NFmiSoundingData &theData)
{
	const auto& location = theData.Location();
    if(location.GetIdent() == 0)
        return ::GetNameText(location, theData.MovingSounding());
    else
        return std::string(location.GetName());
}

void NFmiTempView::DrawSoundingInTextFormat(TotalSoundingData & usedTotalData)
{
	// Muista: jos t‰m‰ piirret‰‰n muuten kuin kaiken muun j‰lkeen Draw-metodissa,
	// ei clippausta saa muuttaa kesken kaiken.
	const auto& textualDataRect = itsTempViewDataRects.getTextualSoundingDataSideViewRect();
    ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &textualDataRect);

	NFmiPoint p(textualDataRect.TopLeft());
	p.X(p.X() + ConvertFixedPixelSizeToRelativeWidth(2)); // siirret‰‰n teksti‰ pikkusen oikealle p‰in
	auto str(::GetStationsShortName(usedTotalData.itsSoundingData));
	auto envi = *itsDrawingEnvironment;
	NFmiText text(p, NFmiString(""), true, 0, &envi);
	auto lineH = itsTextualSoundingDataRelativeLineHeight;
	DrawNextLineToIndexView(lineH, text, str, p, false);

	str = "P    T     Td    Z     WS WD";
	DrawNextLineToIndexView(lineH, text, str, p);

	auto soundingDataLevelStrings = MakeSoundingDataLevelStrings(usedTotalData.itsSoundingData);
	DrawWantedTextualSoundingDataLevels(text, p, soundingDataLevelStrings, lineH, usedTotalData.itsGroundLevelValue);
}

void NFmiTempView::DrawSimpleLineWithGdiplus(const NFmiTempLineInfo& lineInfo, const NFmiPoint& relativeP1, const NFmiPoint& relativeP2, bool fixEndPixelX, bool fixEndPixelY)
{
	auto gdiplusP1 = CtrlView::Relative2GdiplusPoint(itsToolBox, relativeP1);
	auto gdiplusP2 = CtrlView::Relative2GdiplusPoint(itsToolBox, relativeP2);
	CtrlView::DrawLine(*itsGdiPlusGraphics,
		boost::math::iround(gdiplusP1.X),
		boost::math::iround(gdiplusP1.Y),
		boost::math::iround(gdiplusP2.X - (fixEndPixelX ? 1. : 0.)),
		boost::math::iround(gdiplusP2.Y - (fixEndPixelY ? 1. : 0.)),
		lineInfo.Color(),
		float(lineInfo.Thickness()));
}

void NFmiTempView::DrawTextualSideViewScrollingVisuals(NFmiPoint& p, double relativeLineHeight, int totalSoundingRows, int fullVisibleRows, int startingRowIndex, bool drawUpwardSounding)
{
	// Piirret‰‰n vaakaviiva joka erottaa otsikko osion ja itse tekstidatarivit.
	// Huom! Tekstirivin piirto systeemin hankaluuden takia, t‰ss‰ pit‰‰ l‰htˆkorkeuteen lis‰t‰ reilu 1 rivi,
	// koska seuraavaa tekstirivi‰ piirrett‰ess‰ rivin korkeus lis‰t‰‰n ennen piirtoa.
	auto dividerLineRelativeHeight = p.Y() + (relativeLineHeight * 1.1);

	NFmiTempLineInfo basicLineInfo; // oletusarvot ok: musta, 1 [pix], solid
	const auto& textualRect = itsTempViewDataRects.getTextualSoundingDataSideViewRect();
	// Piirr‰ vaaka legendan jakoviiva
	NFmiPoint horP1(textualRect.Left(), dividerLineRelativeHeight);
	NFmiPoint horP2(textualRect.Right(), dividerLineRelativeHeight);
	DrawSimpleLineWithGdiplus(basicLineInfo, horP1, horP2, true, false);
	// Piirr‰ scrolli hissin pystyjakoviiva
	auto posX = textualRect.Right() - ConvertFixedPixelSizeToRelativeWidth(5);
	NFmiPoint vertP1(posX, dividerLineRelativeHeight);
	NFmiPoint vertP2(posX, textualRect.Bottom());
	DrawSimpleLineWithGdiplus(basicLineInfo, vertP1, vertP2, false, true);
	// Piirr‰ vaaleanharmaa scrollialue laatikko
	auto scrollAreaRelativeWidth = ConvertFixedPixelSizeToRelativeWidth(4);
	auto scrollAreaRect = itsTempViewScrollingData.calcScrollAreaRect(dividerLineRelativeHeight, scrollAreaRelativeWidth, textualRect, totalSoundingRows, fullVisibleRows, startingRowIndex);
	NFmiDrawingEnvironment filledRectEnvi;
	filledRectEnvi.SetFillColor(NFmiColor(0.7f, 0.7f, 0.7f));
	filledRectEnvi.EnableFill();
	filledRectEnvi.DisableFrame();
	NFmiRectangle filledRect(scrollAreaRect, nullptr, &filledRectEnvi);
	itsToolBox->Convert(&filledRect);
}

void NFmiTempView::DrawWantedTextualSoundingDataLevels(NFmiText& text, NFmiPoint& p, const std::vector<std::pair<float, std::string>>& levelStrings, double relativeLineHeight, const NFmiGroundLevelValue& groundLevelValue)
{
	auto totalTextRowCount = CalcSideViewTextRowCount(itsTempViewDataRects.getTextualSoundingDataSideViewRect(), p, relativeLineHeight, true);
	if(totalTextRowCount > 0)
	{
		auto totalSoundingRows = static_cast<int>(levelStrings.size());
		auto fullVisibleRows = static_cast<int>(totalTextRowCount);
		auto drawUpwardSounding = itsCtrlViewDocumentInterface->GetMTATempSystem().GetSoundingViewSettingsFromWindowsRegisty().SoundingTextUpward();
		auto startingRowIndex = itsTempViewScrollingData.calcActualScrollingIndex(totalSoundingRows, fullVisibleRows, drawUpwardSounding);
		if(totalSoundingRows > fullVisibleRows)
		{  // piirret‰‰n scrollaus-hissi visuaalit vain jos niille on tarvetta
			DrawTextualSideViewScrollingVisuals(p, relativeLineHeight, totalSoundingRows, fullVisibleRows, startingRowIndex, drawUpwardSounding);
		}
		auto* usedEnvi = text.GetEnvironment();
		auto baseColor = usedEnvi->GetFrameColor();
		auto dummyPenSize = usedEnvi->GetPenSize();
		int counter = 0;
		for(auto rowIndex = startingRowIndex; rowIndex < totalSoundingRows; rowIndex++)
		{
			::MakeUndergroundColorSetup(baseColor, dummyPenSize, groundLevelValue, levelStrings[rowIndex].first, usedEnvi, true, false, false);
			DrawNextLineToIndexView(relativeLineHeight, text, levelStrings[rowIndex].second, p);
			counter++;
			if(counter > fullVisibleRows)
				break;
		}
	}
}

double NFmiTempView::CalcSideViewTextRowCount(const NFmiRect& viewRect, const NFmiPoint& currentRowCursor, double relativeTextRowHeight, bool advanceBeforeDraw)
{
	double usableHeight = viewRect.Bottom() - currentRowCursor.Y();
	auto rowCount = usableHeight / relativeTextRowHeight;
	if(advanceBeforeDraw)
		rowCount--;
	return rowCount;
}

std::vector<std::pair<float, std::string>> NFmiTempView::MakeSoundingDataLevelStrings(NFmiSoundingData& theData)
{
	std::deque<float>& pVec = theData.GetParamData(kFmiPressure);
	std::deque<float>& tVec = theData.GetParamData(kFmiTemperature);
	std::deque<float>& tdVec = theData.GetParamData(kFmiDewPoint);
	std::deque<float>& zVec = theData.GetParamData(kFmiGeopHeight);
	std::deque<float>& wsVec = theData.GetParamData(kFmiWindSpeedMS);
	std::deque<float>& wdVec = theData.GetParamData(kFmiWindDirection);
	int ssize = static_cast<int>(pVec.size());
	std::vector<std::pair<float, std::string>> levels;
	auto drawUpwardSounding = itsCtrlViewDocumentInterface->GetMTATempSystem().GetSoundingViewSettingsFromWindowsRegisty().SoundingTextUpward();
	if(drawUpwardSounding)
	{
		for(int i = ssize - 1; i >= 0; i--)
			levels.push_back(MakeTextualSoundingLevelString(i, pVec, tVec, tdVec, zVec, wsVec, wdVec));
	}
	else
	{
		for(int i = 0; i < ssize; i++)
			levels.push_back(MakeTextualSoundingLevelString(i, pVec, tVec, tdVec, zVec, wsVec, wdVec));
	}

	// Tyhj‰ level stringit pit‰‰ viel‰ lopuksi poistaa
	levels.erase(std::remove_if(levels.begin(), levels.end(), [](const auto& levelPair) {return levelPair.second.empty(); }), levels.end());

	return levels;
}

// Haluan ett‰ vaaka-asteikon arvot eiv‰t mene varsinkaan oikealla olevien sivuosioiden alueelle,
// siksi teen esi-clippaus alueen, joka rajoittuu data-alueen ja siit‰ total-alueen bottom osioon.

static NFmiRect MakeXAxelPreClipArea(const NFmiTempViewDataRects &tempViewDataRects)
{
	NFmiRect preClipArea = tempViewDataRects.getSoundingCurveDataRect();
	preClipArea.Bottom(tempViewDataRects.getTotalSoundingViewRect().Bottom());
	return preClipArea;
}

// eli piirret‰‰n l‰mpp‰ri apuviivat
void NFmiTempView::DrawXAxel(void)
{
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    NFmiTempLineInfo lineInfo = mtaTempSystem.TemperatureHelpLineInfo();
	if(mtaTempSystem.TemperatureHelpLineInfo().DrawLine() == false)
		return ;
	lineInfo.Thickness(boost::math::iround(lineInfo.Thickness() * itsDrawSizeFactor.X()));
	const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
 // GDI+ piirto koodia
	itsGdiPlusGraphics->SetClip(CtrlView::Relative2GdiplusRect(itsToolBox, dataRect));
    NFmiTempLabelInfo labelInfo = mtaTempSystem.TemperatureHelpLabelInfo();
	labelInfo.FontSize(boost::math::iround(labelInfo.FontSize() * itsDrawSizeFactor.Y()));
	labelInfo.StartPointPixelOffSet(ScaleOffsetPoint(labelInfo.StartPointPixelOffSet()));
	NFmiPoint offsetPoint = labelInfo.StartPointPixelOffSet();
	// muutos piirto systeeimeiss‰ (toolbox -> Gdi+) liikutti vakio l‰mpˆtila label paikkaa hieman ylˆsp‰in, t‰ss‰ teen yleis fixin, ett‰ se putoaa dataruudun alle
	offsetPoint.Set(labelInfo.StartPointPixelOffSet().X(), labelInfo.StartPointPixelOffSet().Y() + labelInfo.FontSize()/2.);
	labelInfo.StartPointPixelOffSet(offsetPoint);

	Gdiplus::PointF moveLabelInPixels(static_cast<REAL>(labelInfo.StartPointPixelOffSet().X()), static_cast<REAL>(labelInfo.StartPointPixelOffSet().Y()));
	double TStart = mtaTempSystem.TemperatureHelpLineStart();
	double TEnd = mtaTempSystem.TemperatureHelpLineEnd();
	double TStep = mtaTempSystem.TemperatureHelpLineStep();
	auto preClipArea = ::MakeXAxelPreClipArea(itsTempViewDataRects);
	std::vector<LineLabelDrawData> lineLabels;
    for(double t = TStart; t <= TEnd; t += TStep)
    {
        std::vector<PointF> points;
        double x1 = pt2x(pmax, t);
        double y1 = p2y(pmax);
        double x2 = pt2x(pmin, t);
        double y2 = p2y(pmin);

        points.push_back(PointF(static_cast<REAL>(x1 * itsGdiplusScale.X()), static_cast<REAL>(y1 * itsGdiplusScale.Y())));
        points.push_back(PointF(static_cast<REAL>(x2 * itsGdiplusScale.X()), static_cast<REAL>(y2 * itsGdiplusScale.Y())));
		if(preClipArea.IsInside(NFmiPoint(x1, y1)))
		{
	        ::AddLineLabelData(PointF(static_cast<REAL>(x1 * itsGdiplusScale.X()), static_cast<REAL>(y1 * itsGdiplusScale.Y())), moveLabelInPixels, t, lineLabels);
		}
        ::DrawGdiplusCurve(*itsGdiPlusGraphics, points, lineInfo, GetUsedCurveDrawSmoothingMode(), IsPrinting(), IsRectangularTemperatureHelperLines());
    }
	itsGdiPlusGraphics->ResetClip();
	::DrawGdiplusStringVector(*itsGdiPlusGraphics, lineLabels, labelInfo, CtrlView::Relative2GdiplusRect(itsToolBox, dataRect), lineInfo.Color());
}

// t‰m‰ piirt‰‰ Paineen apuviivat
void NFmiTempView::DrawYAxel(void)
{
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    NFmiTempLineInfo lineInfo = mtaTempSystem.PressureLineInfo();
	if(lineInfo.DrawLine() == false)
		return ;
	lineInfo.Thickness(boost::math::iround(lineInfo.Thickness() * itsDrawSizeFactor.X()));

    // Piirret‰‰n paineen apuviivat myˆs apudata alueelle, jos se on k‰ytˆss‰, lasketaan k‰ytetty dataRect
    NFmiRect usedDataRect = itsTempViewDataRects.getSoundingCurveDataRect();
    if(mtaTempSystem.DrawSecondaryData())
        usedDataRect = usedDataRect.SmallestEnclosing( itsTempViewDataRects.getSecondaryDataFrame());
 // GDI+ piirto koodia
    itsGdiPlusGraphics->SetClip(CtrlView::Relative2GdiplusRect(itsToolBox, usedDataRect));
    NFmiTempLabelInfo labelInfo = mtaTempSystem.PressureLabelInfo();
	labelInfo.FontSize(boost::math::iround(labelInfo.FontSize() * itsDrawSizeFactor.Y()));
	labelInfo.StartPointPixelOffSet(ScaleOffsetPoint(labelInfo.StartPointPixelOffSet()));
	Gdiplus::PointF moveLabelInPixels(static_cast<REAL>(labelInfo.StartPointPixelOffSet().X()), static_cast<REAL>(labelInfo.StartPointPixelOffSet().Y()));

	const auto &values = mtaTempSystem.PressureValues();
	auto endIt = values.cend();
    double x1 = usedDataRect.Left();
    double x2 = usedDataRect.Right();
	// # Y-axel
	std::vector<LineLabelDrawData> lineLabels;
	for(auto it = values.cbegin(); it != endIt;  ++it)
	{
		std::vector<PointF> points;
		double y = p2y(*it);

		points.push_back(PointF(static_cast<REAL>(x1 * itsGdiplusScale.X()), static_cast<REAL>(y * itsGdiplusScale.Y())));
		points.push_back(PointF(static_cast<REAL>(x2 * itsGdiplusScale.X()), static_cast<REAL>(y * itsGdiplusScale.Y())));
		::AddLineLabelData(PointF(static_cast<REAL>(x1 * itsGdiplusScale.X()), static_cast<REAL>(y * itsGdiplusScale.Y())), moveLabelInPixels, *it, lineLabels);
		::DrawGdiplusCurve(*itsGdiPlusGraphics, points, lineInfo, GetUsedCurveDrawSmoothingMode(), IsPrinting(), true);
	}
	itsGdiPlusGraphics->ResetClip();
    ::DrawGdiplusStringVector(*itsGdiPlusGraphics, lineLabels, labelInfo, CtrlView::Relative2GdiplusRect(itsToolBox, usedDataRect), lineInfo.Color());
}

NFmiPoint NFmiTempView::ScaleOffsetPoint(const NFmiPoint &thePoint)
{
	NFmiPoint offsetPoint = thePoint;
	offsetPoint.X(offsetPoint.X() * itsDrawSizeFactor.X());
	offsetPoint.Y(offsetPoint.Y() * itsDrawSizeFactor.Y());
	return offsetPoint;
}

void NFmiTempView::DrawMixingRatio(void)
{
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    NFmiTempLabelInfo labelInfo = mtaTempSystem.MixingRatioLabelInfo();
	labelInfo.FontSize(boost::math::iround(labelInfo.FontSize()* itsDrawSizeFactor.Y()));
	labelInfo.StartPointPixelOffSet(ScaleOffsetPoint(labelInfo.StartPointPixelOffSet()));
    NFmiTempLineInfo lineInfo = mtaTempSystem.MixingRatioLineInfo();
	lineInfo.Thickness(boost::math::iround(lineInfo.Thickness() * itsDrawSizeFactor.X() * ExtraPrintLineThicknesFactor(false)));
	DrawMixingRatio(labelInfo, lineInfo, mtaTempSystem.MixingRatioValues(), pmax, 150, 0, itsDrawingEnvironment);
}

// Kaksi paikkaa mihin label voidaan sijoittaa:
// 1. Heti viimeisen labelin yl‰ puolelle jos on tilaa.
// 2. Heti viimeisen labeli ala puolelle, jos ylh‰‰ll‰ ei ole tilaa.
static double CalcHelpScaleUnitStringYPos(const NFmiRect &theRect, double lastLabelHeight, double relFontHeight, double relFontYmove)
{
	double unitStringY = 0;
	if(lastLabelHeight - relFontHeight + relFontYmove > theRect.Top())
		unitStringY = lastLabelHeight - relFontHeight + relFontYmove;
	else
		unitStringY = lastLabelHeight + relFontHeight + relFontYmove;
	return unitStringY;
}

// Konvertoi halutun vakio pikseli m‰‰r‰n joko x- tai y-suuntaiseen suhteelliseen pituuteen.
// Laskuissa otetaan huomioon printtauksen koukerot, eli vakio pikseli pituus 
// kerrotaan itsDrawSizeFactor:illa.
double NFmiTempView::ConvertFixedPixelSizeToRelativeWidth(long fixedPixelSize)
{
	return itsToolBox->SX(boost::math::iround(fixedPixelSize * itsDrawSizeFactor.X()));
}

double NFmiTempView::ConvertFixedPixelSizeToRelativeHeight(long fixedPixelSize)
{
	return itsToolBox->SY(boost::math::iround(fixedPixelSize * itsDrawSizeFactor.Y()));
}

void NFmiTempView::DrawFlightLevelScale(void)
{
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    if(mtaTempSystem.ShowFlightLevelScale())
	{
		NFmiDrawingEnvironment envi;
        NFmiTempLabelInfo labelInfo;
		labelInfo.DrawLabelText(true);
		labelInfo.FontSize(boost::math::iround(18 * itsDrawSizeFactor.Y()));
		long extraOffsetInPixels = 0;
		if(mtaTempSystem.ShowKilometerScale())
			extraOffsetInPixels = static_cast<long>(labelInfo.FontSize()*1.6); // siirret‰‰n asteikkoa, jos myˆs kilometri asteikko on n‰kyviss‰
		labelInfo.StartPointPixelOffSet(NFmiPoint(-6 * itsDrawSizeFactor.X(), -labelInfo.FontSize()/2));
		double extraOffset = ConvertFixedPixelSizeToRelativeWidth(extraOffsetInPixels);
		labelInfo.ClipWithDataRect(true);
		labelInfo.TextAlignment(kRight);
        NFmiTempLineInfo lineInfo;
		lineInfo.DrawLine(false);
		lineInfo.Color(NFmiColor(0.f, 0.f, 0.f)); // t‰m‰ on myˆs label v‰ri
		lineInfo.Thickness(boost::math::iround(2 * itsDrawSizeFactor.X()));

		double tickMarkWidth = ConvertFixedPixelSizeToRelativeWidth(6);
		double unitStringYoffset = itsToolBox->SY(labelInfo.FontSize());
		int trueLineWidth = boost::math::iround(1 * itsDrawSizeFactor.X());
		SetHelpLineDrawingAttributes(itsToolBox, &envi, labelInfo, lineInfo, trueLineWidth, true);
		NFmiPoint moveLabelRelatively(CalcReltiveMoveFromPixels(itsToolBox, labelInfo.StartPointPixelOffSet()));

		const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
		double lastHeightInDataBox = 0;
		for (double flLevel = 0; flLevel <= 500;  flLevel += 10)
		{
			bool onlySmallTick = true;
			double shortTickChange = tickMarkWidth/3.;
			if(static_cast<int>(flLevel) % 50 == 0 || flLevel < 50)
			{
				onlySmallTick = false;
				shortTickChange = 0;
			}
			double P = ::CalcFlightLevelPressure(flLevel*100);
			double x = dataRect.Right() - extraOffset;
			double y = p2y(P);
			NFmiPoint p1(x, y);
			NFmiPoint p2(x-tickMarkWidth + shortTickChange, y);
			NFmiLine l1(p1, p2, 0, &envi);
			itsToolBox->Convert(&l1);
			if(onlySmallTick == false)
			{
				DrawHelpLineLabel(p1, moveLabelRelatively, flLevel, labelInfo, &envi);
				if(dataRect.IsInside(p2))
					lastHeightInDataBox = y;
			}
		}
		if(lastHeightInDataBox != 0)
		{
			double unitStringY = CalcHelpScaleUnitStringYPos(dataRect, lastHeightInDataBox, unitStringYoffset, moveLabelRelatively.Y());
			double unitStringX = dataRect.Right();
			NFmiText txt1(NFmiPoint(unitStringX + moveLabelRelatively.X() - extraOffset, unitStringY), "FL", false, 0, &envi);
			itsToolBox->Convert(&txt1);
		}
	}
}

// piirt‰‰ standardi ilmakeh‰n mukaiset kilometri palkit ja tekstit data ruudun oikeaan laitaan
void NFmiTempView::DrawHeightScale(void)
{
	if(itsCtrlViewDocumentInterface->GetMTATempSystem().ShowKilometerScale())
	{
        NFmiTempLabelInfo labelInfo;
		labelInfo.DrawLabelText(true);
		labelInfo.FontSize(boost::math::iround(18 * itsDrawSizeFactor.Y()));
		labelInfo.StartPointPixelOffSet(NFmiPoint(-6 * itsDrawSizeFactor.X(), -labelInfo.FontSize()/2));
		labelInfo.ClipWithDataRect(true);
		labelInfo.TextAlignment(kRight);
        NFmiTempLineInfo lineInfo;
		lineInfo.Thickness(boost::math::iround(1 * itsDrawSizeFactor.X()));
		lineInfo.DrawLine(false);
		lineInfo.Color(NFmiColor(0.f, 0.f, 0.f)); // t‰m‰ on myˆs label v‰ri

		double tickMarkWidth = ConvertFixedPixelSizeToRelativeWidth(6);
		double unitStringYoffset = itsToolBox->SY(labelInfo.FontSize());
		int trueLineWidth = boost::math::iround(1 * itsDrawSizeFactor.X());
		SetHelpLineDrawingAttributes(itsToolBox, itsDrawingEnvironment, labelInfo, lineInfo, trueLineWidth, true);
		NFmiPoint moveLabelRelatively(CalcReltiveMoveFromPixels(itsToolBox, labelInfo.StartPointPixelOffSet()));

		const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
		double lastHeightInDataBox = 0;
		for (double heightKM = 0; heightKM <= 15;  heightKM++)
		{
			double P = CalcPressureAtHeight(heightKM);
			double x = dataRect.Right();
			double y = p2y(P);
			NFmiPoint p1(x, y);
			NFmiPoint p2(x-tickMarkWidth, y);
			NFmiLine l1(p1, p2, 0, itsDrawingEnvironment);
			itsToolBox->Convert(&l1);
			DrawHelpLineLabel(p1, moveLabelRelatively, heightKM, labelInfo, itsDrawingEnvironment);
			if(dataRect.IsInside(p2))
				lastHeightInDataBox = y;
		}
		if(lastHeightInDataBox != 0)
		{
			double unitStringY = CalcHelpScaleUnitStringYPos(dataRect, lastHeightInDataBox, unitStringYoffset, moveLabelRelatively.Y());
			double unitStringX = dataRect.Right();
			NFmiText txt1(NFmiPoint(unitStringX + moveLabelRelatively.X(), unitStringY), "KM", false, 0, itsDrawingEnvironment);
			itsToolBox->Convert(&txt1);
		}
	}
}

// Piirt‰‰ tiivistys juova sapluunaan luotauksen kosteus arvoja.
// piirret‰‰n annetulla paine v‰lill‰. Arvot piirret‰‰n annetun
// mixingratio viivan oikealle puolelle ja paineen avulla lasketaan korkeus
// mihin 'label' laitetaan.
void NFmiTempView::DrawCondensationTrailRHValues(NFmiSoundingData &theData, double startP, double endP, double theMixRatio)
{
	std::deque<float>&tV = theData.GetParamData(kFmiTemperature);
	std::deque<float>&tdV = theData.GetParamData(kFmiDewPoint);
	std::deque<float>&pV = theData.GetParamData(kFmiPressure);
	if(pV.size() > 0 && pV.size() == tV.size() && pV.size() == tdV.size())
	{
		NFmiDrawingEnvironment envi;
		envi.SetFrameColor(NFmiColor(0.99f, 0.5f, 0.f));
		envi.BoldFont(true);
		long fontSize = boost::math::iround(20 * itsDrawSizeFactor.Y());
		envi.SetFontSize(NFmiPoint(fontSize, fontSize));
        NFmiTempLabelInfo labelInfo;
		labelInfo.DrawLabelText(true);
		double xShift = ConvertFixedPixelSizeToRelativeWidth(8);
		double yShift = ConvertFixedPixelSizeToRelativeHeight(boost::math::iround(fontSize/2.));
		NFmiPoint moveLabelRelatively(xShift, -yShift);
		std::string unitStr("%");
		double lastY = -99;

		for(int i=0; i<static_cast<int>(pV.size()); i++)
		{
			float P = pV[i];
			if(P != kFloatMissing && tV[i] != kFloatMissing && tdV[i] != kFloatMissing )
			{
				if(P <= startP && P >= endP)
				{ // jos ollaan halutulla valill‰
					int RH = static_cast<int>(NFmiSoundingFunctions::CalcRH(tV[i], tdV[i]));
					double y = p2y(P);
					double T = NFmiSoundingFunctions::TMR(theMixRatio, P);
					double x = pt2x(P, T);

					if(::fabs(lastY - y) > yShift * 1.1) // piirret‰‰n kosteus arvoja vain jos ei tule liian tihe‰sti
					{
						NFmiPoint p1(x, y);
						DrawHelpLineLabel(p1, moveLabelRelatively, RH, labelInfo, &envi, unitStr);
						lastY = y;
					}
				}
			}
		}
	}
}

void NFmiTempView::DrawCondensationTrailProbabilityLines(void)
{
	if(itsCtrlViewDocumentInterface->GetMTATempSystem().ShowCondensationTrailProbabilityLines())
	{
		NFmiDrawingEnvironment envi;
        NFmiTempLabelInfo labelInfo;
		labelInfo.FontSize(boost::math::iround(labelInfo.FontSize() * itsDrawSizeFactor.Y()));
		labelInfo.DrawLabelText(false);
        NFmiTempLineInfo lineInfo;
		lineInfo.LineType(FMI_SOLID);
		lineInfo.Color(NFmiColor(0.99f, 0.5f, 0.f));
		lineInfo.Thickness(boost::math::iround(2 * itsDrawSizeFactor.X()));
		std::vector<double> values{ 0.11, 0.135, 0.17, 0.32 };
		double startP = CalcPressureAtHeight(6); // aloitetaan viivan piirto 6 km:sta

		double deltaP = -15;
		DrawMixingRatio(labelInfo, lineInfo, values, startP, 100, deltaP, &envi);

		// piirret‰‰n sitten tod. n‰k. labelit viivoihin (jotka ovat eri juttu kuin piirretyt mixing ratio arvot)
		labelInfo.DrawLabelText(true);
		labelInfo.FontSize(boost::math::iround(16 * itsDrawSizeFactor.Y()));
		envi.SetFontSize(NFmiPoint(labelInfo.FontSize(), labelInfo.FontSize()));
		envi.BoldFont(true);
		labelInfo.StartPointPixelOffSet(NFmiPoint(-5* itsDrawSizeFactor.X(), 0 * itsDrawSizeFactor.Y()));
		std::vector<double> probValues{ 0, 40, 70, 100 };

		NFmiPoint moveLabelRelatively(CalcReltiveMoveFromPixels(itsToolBox, labelInfo.StartPointPixelOffSet()));
		auto itProb = probValues.cbegin();
		auto endIt = values.cend();
		double P = startP;
		for (auto it = values.cbegin(); it != endIt;  ++it, ++itProb)
		{
			double T = NFmiSoundingFunctions::TMR(*it, P);
			double X = pt2x(P, T);
			double Y = p2y(P);
			NFmiPoint p1(X, Y);
			DrawHelpLineLabel(p1, moveLabelRelatively, *itProb, labelInfo, &envi);
			P += deltaP;
		}
	}
}

// T‰nne annetut labelInfo ja lineInfo on jo skaalattu niin ett‰ printtaus kertoimia ei tarvise k‰ytt‰‰ t‰‰ll‰.
void NFmiTempView::DrawMixingRatio(const NFmiTempLabelInfo &theLabelInfo, const NFmiTempLineInfo &theLineInfo,
								   const std::vector<double> &theValues, double startP, double endP, double deltaStartLevelP,
								   NFmiDrawingEnvironment * /* theEnvi */ )
{
	if(theLineInfo.DrawLine() == false)
		return ;
 // GDI+ piirto koodia
	const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
	itsGdiPlusGraphics->SetClip(CtrlView::Relative2GdiplusRect(itsToolBox, dataRect));
	Gdiplus::PointF moveLabelInPixels(static_cast<REAL>(theLabelInfo.StartPointPixelOffSet().X()), static_cast<REAL>(theLabelInfo.StartPointPixelOffSet().Y()));

	auto endIt = theValues.cend();
	double deltap = 50;
	std::vector<LineLabelDrawData> lineLabels;
	for (auto it = theValues.cbegin(); it != endIt;  ++it)
	{
		std::vector<PointF> points;
		for (double p = startP; p >= endP; p-=deltap)
		{
			double t = NFmiSoundingFunctions::TMR(*it, p);
			double x = pt2x(p, t);
			double y = p2y(p);
			points.push_back(PointF(static_cast<REAL>(x * itsGdiplusScale.X()), static_cast<REAL>(y * itsGdiplusScale.Y())));
			if(p == pmax) // piirret‰‰n label vain alku pisteeseen
				::AddLineLabelData(PointF(static_cast<REAL>(x * itsGdiplusScale.X()), static_cast<REAL>(y * itsGdiplusScale.Y())), moveLabelInPixels, *it, lineLabels);
		}
		startP += deltaStartLevelP; // t‰m‰ on mielest‰ni turha
		::DrawGdiplusCurve(*itsGdiPlusGraphics, points, theLineInfo, GetUsedCurveDrawSmoothingMode(), IsPrinting(), false);
	}
	itsGdiPlusGraphics->ResetClip();
	::DrawGdiplusStringVector(*itsGdiPlusGraphics, lineLabels, theLabelInfo, CtrlView::Relative2GdiplusRect(itsToolBox, dataRect), theLineInfo.Color());
}

double NFmiTempView::ExtraPrintLineThicknesFactor(bool fMainCurve)
{
	double extraThicknessFactor = 1.0;
	if(itsToolBox->GetDC()->IsPrinting())
	{
		if(fMainCurve)	
			extraThicknessFactor = 1.1; // huomasin ett‰ itse luotaus k‰yr‰t eiv‰t erotu paksuuden puolesta niin hyvin printill‰ kuin ruudulla, joten ohennan apuviivoja v‰h‰n
		else
			extraThicknessFactor = 0.7; // huomasin ett‰ itse luotaus k‰yr‰t eiv‰t erotu paksuuden puolesta niin hyvin printill‰ kuin ruudulla, joten ohennan apuviivoja v‰h‰n
	}
	return extraThicknessFactor;
}

void NFmiTempView::DrawDryAdiapaticks(void)
{
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    NFmiTempLineInfo lineInfo = mtaTempSystem.DryAdiabaticLineInfo();
	if(lineInfo.DrawLine() == false)
		return ;
	lineInfo.Thickness(boost::math::iround(lineInfo.Thickness()* itsDrawSizeFactor.X() * ExtraPrintLineThicknesFactor(false)));
	const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
 // GDI+ piirto koodia
	itsGdiPlusGraphics->SetClip(CtrlView::Relative2GdiplusRect(itsToolBox, dataRect));

    NFmiTempLabelInfo labelInfo = mtaTempSystem.DryAdiabaticLabelInfo();
	labelInfo.FontSize(boost::math::iround(labelInfo.FontSize() * itsDrawSizeFactor.Y()));
	Gdiplus::PointF moveLabelInPixels(static_cast<REAL>(labelInfo.StartPointPixelOffSet().X() * itsDrawSizeFactor.X()), static_cast<REAL>(labelInfo.StartPointPixelOffSet().Y() * itsDrawSizeFactor.Y()));
	const auto &values = mtaTempSystem.DryAdiabaticValues();
	auto endIt = values.cend();
	double deltap = 50;
	std::vector<LineLabelDrawData> lineLabels;
	for (auto it = values.cbegin(); it != endIt;  ++it)
	{
		std::vector<PointF> points;
		for (double p = pmax; p > 100; p-=deltap)
		{
			double x = Tpot2x(*it, p);
			double y = p2y(p);
			points.push_back(PointF(static_cast<REAL>(x * itsGdiplusScale.X()), static_cast<REAL>(y * itsGdiplusScale.Y())));
			if(p == pmax) // piirret‰‰n label vain alku pisteeseen
				::AddLineLabelData(PointF(static_cast<REAL>(x * itsGdiplusScale.X()), static_cast<REAL>(y * itsGdiplusScale.Y())), moveLabelInPixels, *it, lineLabels);
		}
		::DrawGdiplusCurve(*itsGdiPlusGraphics, points, lineInfo, GetUsedCurveDrawSmoothingMode(), IsPrinting(), false);
	}
	itsGdiPlusGraphics->ResetClip();
	::DrawGdiplusStringVector(*itsGdiPlusGraphics, lineLabels, labelInfo, CtrlView::Relative2GdiplusRect(itsToolBox, dataRect), lineInfo.Color());
}

void NFmiTempView::DrawMoistAdiapaticks(void)
{
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    NFmiTempLineInfo lineInfo = mtaTempSystem.MoistAdiabaticLineInfo();
	if(lineInfo.DrawLine() == false)
		return ;
	lineInfo.Thickness(boost::math::iround(lineInfo.Thickness() * itsDrawSizeFactor.X() * ExtraPrintLineThicknesFactor(false)));

 // GDI+ piirto koodia
	const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
	itsGdiPlusGraphics->SetClip(CtrlView::Relative2GdiplusRect(itsToolBox, dataRect));
    NFmiTempLabelInfo labelInfo = mtaTempSystem.MoistAdiabaticLabelInfo();
	labelInfo.FontSize(boost::math::iround(labelInfo.FontSize() * itsDrawSizeFactor.Y()));
	Gdiplus::PointF moveLabelInPixels(static_cast<REAL>(labelInfo.StartPointPixelOffSet().X() * itsDrawSizeFactor.X()), static_cast<REAL>(labelInfo.StartPointPixelOffSet().Y() * itsDrawSizeFactor.Y()));

	std::vector<LineLabelDrawData> lineLabels;
	const auto &values = mtaTempSystem.MoistAdiabaticValues();
	auto endIt = values.cend();
	double deltap = 50;
	for (auto it = values.cbegin(); it != endIt;  ++it)
	{
		std::vector<PointF> points;
		double P   = pmax;
		double TK  = *it;
		double AOS = NFmiSoundingFunctions::OS(TK, 1000.);

		double ATSA  = NFmiSoundingFunctions::TSA(AOS, P);
		for(int J = 0; J < 125; J++)
		{
			double P0 = P;
			if(P <= 100)
				break;
			double T0 = ATSA;

			P = P - deltap;
			ATSA = NFmiSoundingFunctions::TSA(AOS, P);
			double x = pt2x(P0, T0);  //; Find rotated temperature position
			double y = p2y(P0);
			points.push_back(PointF(static_cast<REAL>(x * itsGdiplusScale.X()), static_cast<REAL>(y * itsGdiplusScale.Y())));
			if(P0 == pmax) // piirret‰‰n label vain alku pisteeseen
				::AddLineLabelData(PointF(static_cast<REAL>(x * itsGdiplusScale.X()), static_cast<REAL>(y * itsGdiplusScale.Y())), moveLabelInPixels, *it, lineLabels);
		}
		::DrawGdiplusCurve(*itsGdiPlusGraphics, points, lineInfo, GetUsedCurveDrawSmoothingMode(), IsPrinting(), false);
	}
	itsGdiPlusGraphics->ResetClip();
	::DrawGdiplusStringVector(*itsGdiPlusGraphics, lineLabels, labelInfo, CtrlView::Relative2GdiplusRect(itsToolBox, dataRect), lineInfo.Color());
}

void NFmiTempView::DrawHelpLineLabel(const NFmiPoint &p1, const NFmiPoint &theMoveLabelRelatively, double theValue, const NFmiTempLabelInfo &theLabelInfo, NFmiDrawingEnvironment * theEnvi, const std::string &thePostStr)
{
	if(theLabelInfo.DrawLabelText())
	{
        ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), theLabelInfo.ClipWithDataRect());
		std::string str(NFmiStringTools::Convert<double>(theValue));
		str += thePostStr; // jos joku loppu liite on haluttu laittaa labeliin, se tulee t‰ss‰
		NFmiText txt(p1 + theMoveLabelRelatively, str, false, 0, theEnvi);
		itsToolBox->Convert(&txt);
	}
}

void NFmiTempView::DrawLine(const NFmiPoint &p1, const NFmiPoint &p2, bool drawSpecialLines, int theTrueLineWidth, bool startWithXShift, int theHelpDotPixelSize, NFmiDrawingEnvironment * theEnvi)
{
	// yksi viiva piirret‰‰n aina
	NFmiLine line1(p1, p2, 0, theEnvi);
	itsToolBox->Convert(&line1);
	if(drawSpecialLines)
	{ // gdi (CDC-luokka) kirjasto ei tarjoa erikois viivoille muuta kuin yhden pikselin paksuista versiota, joten
	  // se pit‰‰ koodata t‰h‰n itse hieman kˆpˆsti tosin, teen max 3 pikseli‰ leve‰‰ viivaa
		// piirret‰‰n ensin yksi viiva yhden pikselin viereen
		NFmiPoint pExtra11(p1);
		if(startWithXShift)
			pExtra11.X(pExtra11.X() + _1PixelInRel.X());
		else
			pExtra11.Y(pExtra11.Y() + _1PixelInRel.Y());
		NFmiPoint pExtra12(p2);
		if(startWithXShift)
			pExtra12.X(pExtra12.X() + _1PixelInRel.X());
		else
			pExtra12.Y(pExtra12.Y() + _1PixelInRel.Y());
		NFmiLine line2(pExtra11, pExtra12, 0, theEnvi);
		itsToolBox->Convert(&line2);

		if(theTrueLineWidth > 2)
		{
			// piirret‰‰n viel‰  yksi viiva yhden pikselin viereen toiseen suuntaan
			NFmiPoint pExtra21(p1);
			if(startWithXShift)
				pExtra21.X(pExtra21.X() - _1PixelInRel.X());
			else
				pExtra21.Y(pExtra21.Y() - _1PixelInRel.Y());
			NFmiPoint pExtra22(p2);
			if(startWithXShift)
				pExtra22.X(pExtra22.X() - _1PixelInRel.X());
			else
				pExtra22.Y(pExtra22.Y() - _1PixelInRel.Y());
			NFmiLine line3(pExtra21, pExtra22, 0, theEnvi);
			itsToolBox->Convert(&line3);
		}
	}
	if(theHelpDotPixelSize) // jos apu laatikko koko on suurempi kuin 0, piirret‰‰n aloituspisteeseen halutun kokoinen laatikko
	{
		double width = ConvertFixedPixelSizeToRelativeWidth(theHelpDotPixelSize);
		double height = ConvertFixedPixelSizeToRelativeHeight(theHelpDotPixelSize);
		NFmiRect rec(0,0,width, height);
		rec.Center(p2);
		NFmiRectangle rec2(rec, 0, theEnvi);
		itsToolBox->Convert(&rec2);
	}
}

void NFmiTempView::DrawBackground(void)
{
	itsDrawingEnvironment->SetPenSize(NFmiPoint(1,1));
	itsDrawingEnvironment->EnableFrame();
	itsDrawingEnvironment->EnableFill();
	itsDrawingEnvironment->SetFillColor(NFmiColor(1.f, 1.f, 1.f));
	itsDrawingEnvironment->SetFrameColor(NFmiColor(0.f,0.f,0.f));
	DrawFrame(itsDrawingEnvironment);

	itsDrawingEnvironment->SetFillColor(NFmiColor(1.f, 1.f, 1.f));
	itsDrawingEnvironment->SetFrameColor(NFmiColor(0.f,0.f,0.f));
	DrawFrame(itsDrawingEnvironment, itsTempViewDataRects.getSoundingCurveDataRect());
}

static NFmiLocation GetSoundingLocation(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiMTATempSystem::TempInfo &theTempInfo, NFmiProducerSystem &theProdSystem)
{
	bool movingSounding = NFmiFastInfoUtils::IsMovingSoundingData(theInfo);
	NFmiLocation location(theTempInfo.Latlon());
    if(theInfo)
    {
        if(theInfo->IsGrid() == false)// && ((*it).Producer().GetIdent() == kFmiTEMP || (*it).Producer().GetIdent() == kFmiRAWTEMP))
        {
            if(!movingSounding)
            {
                if(theInfo->NearestLocation(location, 1000 * 1000))  // 1000km max et‰isyys
                    location = *theInfo->Location();
            }
            else
                ::SetMovingSoundingLocationName(location, *theInfo->Producer());
        }
        else
            ::SetLocationNameByItsLatlon(theProdSystem, location, theInfo->IsGrid() ? *theInfo->Producer() : theTempInfo.Producer(), theInfo->OriginTime(), theInfo->IsGrid());
    }
	return location;
}

static bool IsSurfaceDataCombiningAllowed(CtrlViewDocumentInterface* ctrlViewDocumentInterface, boost::shared_ptr<NFmiFastQueryInfo>& theInfo)
{
	if(ctrlViewDocumentInterface && theInfo)
	{
		auto* helpInfoData = ctrlViewDocumentInterface->HelpDataInfoSystem()->FindHelpDataInfo(theInfo->DataFilePattern());
		if(helpInfoData)
		{
			return helpInfoData->AllowCombiningToSurfaceDataInSoundingView();
		}
	}
	return false;
}

static boost::shared_ptr<NFmiFastQueryInfo> GetPossibleGroundData(const NFmiProducer& theProducer, NFmiInfoOrganizer& theInfoOrganizer)
{
	boost::shared_ptr<NFmiFastQueryInfo> groundDataInfo;
	// Jos tuottajalta lˆytyy pintadataa, miss‰ parametri kFmiPressureAtStationLevel, palautetaan se.
	auto infoVec = theInfoOrganizer.GetInfos(theProducer.GetIdent());
	if(infoVec.size())
	{
		for(size_t i = 0; i < infoVec.size(); i++)
		{
			boost::shared_ptr<NFmiFastQueryInfo> tmpInfo = infoVec[i];
			if(tmpInfo && tmpInfo->Param(kFmiPressureAtStationLevel))
			{
				groundDataInfo = tmpInfo; // lˆytyi data ja siit‰ tarvittava parametri, otetaan se k‰yttˆˆn
				break;
			}
		}
	}
	return groundDataInfo;
}


// Haetaan painepinta datalle pinta-dataa, ett‰ luotauksia voidaan leikata maanpinnalle.
static boost::shared_ptr<NFmiFastQueryInfo> GetPossibleGroundData(CtrlViewDocumentInterface* ctrlViewDocumentInterface, boost::shared_ptr<NFmiFastQueryInfo>& theInfo, const NFmiProducer& theProducer)
{
	if(ctrlViewDocumentInterface && theInfo)
	{
		auto& infoOrganizer = *ctrlViewDocumentInterface->InfoOrganizer();
		theInfo->FirstLevel();
		if(theInfo->Level()->LevelType() == kFmiPressureLevel || theInfo->Level()->LevelType() == kFmiHybridLevel)
		{
			if(::IsSurfaceDataCombiningAllowed(ctrlViewDocumentInterface, theInfo))
			{
				// jos kyse on painepinta datasta ja lˆytyy vastaavan datan pinta data, josta lˆytyy paine aseman korkeudelta, fixataan luotaus dataa pintadatan avulla
				auto possibleGroundDataInfo = ::GetPossibleGroundData(theProducer, infoOrganizer);
				if(possibleGroundDataInfo)
				{
					return possibleGroundDataInfo;
				}
			}
		}
	}
	return nullptr;
}

NFmiGroundLevelValue NFmiTempView::GetPossibleGroundLevelValue(boost::shared_ptr<NFmiFastQueryInfo>& soundingInfo, const NFmiPoint& latlon, const NFmiMetTime& atime)
{
	NFmiGroundLevelValue groundLevelValue;
	if(soundingInfo)
	{
		// Laitetaan maanpinnalla leikattaviin datoihin vain painepintadatat.
		// En tied‰ pit‰isikˆ height-level-datat myˆs laittaa t‰h‰n (ei ole testidataa, niin turha viel‰ tehd‰).
		if(soundingInfo->LevelType() == kFmiPressureLevel)
		{
			auto& infoOrganizer = *itsCtrlViewDocumentInterface->InfoOrganizer();
			auto possibleGroundDataInfo = ::GetPossibleGroundData(*soundingInfo->Producer(), infoOrganizer);
			if(possibleGroundDataInfo)
			{
				// T‰ll‰ datalla on par 472 eli stationPressure
				groundLevelValue.itsStationPressureInMilliBars = possibleGroundDataInfo->InterpolatedValue(latlon, atime);
			}
			auto topoData = infoOrganizer.FindInfo(NFmiInfoData::kStationary);
			if(topoData && topoData->Param(kFmiTopoGraf))
			{
				// Otetaan topo datasta korkeus metreiss‰ ja muunnetaan se standardi-ilmakeh‰n paineeksi
				groundLevelValue.itsTopographyHeightInMillibars = static_cast<float>(CalcPressureAtHeight(topoData->InterpolatedValue(latlon) / 1000.f));
			}
		}
	}
	return groundLevelValue;
}

void NFmiTempView::DrawSoundingsInMTAMode(void)
{
    itsSoundingDataCacheForTooltips.clear();
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    int modelRunCount = mtaTempSystem.ModelRunCount();
	int producerIndex = 0;
	itsLegendDrawingLineData.clear();
	for(const auto &tempInfo : mtaTempSystem.GetTemps())
	{
		for(const auto &selectedProducer : mtaTempSystem.SoundingComparisonProducers())
		{
			if(modelRunCount > 0)
			{
				int startIndex = -modelRunCount;
				for(int modelRunIndex = startIndex; modelRunIndex <= 0; modelRunIndex++)
				{
					double brightningFactor = CtrlView::CalcBrightningFactor(0, modelRunCount, modelRunIndex); // mit‰ isompi luku, sit‰ enemm‰n vaalenee (0-100), vanhemmat malliajot vaaleammalla
					DrawOneSounding(selectedProducer, tempInfo, producerIndex, brightningFactor, modelRunIndex);
				}
			}
			else
			{
				DrawOneSounding(selectedProducer, tempInfo, producerIndex, 0, 0);
			}
			producerIndex++;
		}
	}

	if(!IsInScanMode())
	{
		DrawLegendLineData();
	}
}

static NFmiMetTime GetUsedSoundingDataTime(CtrlViewDocumentInterface *documentInterface, const NFmiMTATempSystem::TempInfo &tempInfo)
{
    NFmiMetTime usedSoundingTime(tempInfo.Time());
    NFmiMTATempSystem &mtaTempSystem = documentInterface->GetMTATempSystem();
    if(mtaTempSystem.GetSoundingViewSettingsFromWindowsRegisty().SoundingTimeLockWithMapView())
        usedSoundingTime = documentInterface->ActiveMapTime();
    return usedSoundingTime;
}

bool NFmiTempView::IsSelectedProducerIndex(int theProducerIndex) const
{
	return theProducerIndex == itsCtrlViewDocumentInterface->GetMTATempSystem().GetSelectedProducerIndex(true);
}

void NFmiTempView::ResetTextualScrollingIfSoundingDataChanged(const NFmiMTATempSystem::SoundingProducer& theProducer, const NFmiMTATempSystem::TempInfo& theTempInfo, boost::shared_ptr<NFmiFastQueryInfo>& theInfo, int theProducerIndex)
{
	// Tarkastelut tehd‰‰n vain valitulle sounding tuottaja datalle.
	if(IsSelectedProducerIndex(theProducerIndex))
	{
		NFmiMetTime dataOriginTime = NFmiMetTime::gMissingTime;
		if(theInfo)
			dataOriginTime = theInfo->OriginTime();

		SoundingInformation soundingInformation(theTempInfo.Time(), dataOriginTime, theTempInfo.Latlon(), theProducer);
		if(itsTempViewScrollingData.isSoundingChanged(soundingInformation))
			itsTempViewScrollingData.resetRelativeScrollingIndex();
	}
}

static bool IsNewData(boost::shared_ptr<NFmiFastQueryInfo> &info)
{
	return CtrlViewUtils::IsConsideredAsNewData(info, 0, false);
}

bool NFmiTempView::IsInScanMode() const
{
	return (itsOperationalMode == SoundingViewOperationMode::FitScalesScanMode);
}

void NFmiTempView::DrawOneSounding(const NFmiMTATempSystem::SoundingProducer &theProducer, const NFmiMTATempSystem::TempInfo &theTempInfo, int theProducerIndex, double theBrightningFactor, int theModelRunIndex)
{
    auto usedTempInfo(theTempInfo);
    usedTempInfo.Time(::GetUsedSoundingDataTime(itsCtrlViewDocumentInterface, theTempInfo));
	// Amdar datoilla (tuottaja id 1015) on erikois aikaikkuna, mist‰ datoja etsit‰‰n, 
	// sen alkuhaarukka pit‰‰ antaa FindSoundingInfo, kaikille muille datoille arvo on 0.
	int amdarDataStartOffsetInMinutes = (theProducer.GetIdent() == 1015) ? 30 : 0;
	bool mainCurve = (theModelRunIndex == 0);

	boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->InfoOrganizer()->FindSoundingInfo(theProducer, usedTempInfo.Time(), usedTempInfo.Latlon(), theModelRunIndex, NFmiInfoOrganizer::ParamCheckFlags(true), amdarDataStartOffsetInMinutes);
	if(theProducer.useServer() || info)
	{
		auto sounding = GetTotalsoundingData(info, usedTempInfo, theProducer, theProducerIndex);
		if(IsInScanMode())
		{
			itsScanData.push_back(sounding);
			return;
		}

        NFmiColor usedColor(itsCtrlViewDocumentInterface->GetMTATempSystem().SoundingColor(theProducerIndex));
		if(theBrightningFactor != 0)
			usedColor = NFmiColorSpaces::GetBrighterColor(usedColor, theBrightningFactor);
		itsDrawingEnvironment->SetFrameColor(usedColor);
        bool onSouthernHemiSphere = usedTempInfo.Latlon().Y() < 0;
		sounding.itsGroundLevelValue = GetPossibleGroundLevelValue(info, usedTempInfo.Latlon(), usedTempInfo.Time());
		DrawSounding(sounding, theProducerIndex, usedColor, mainCurve, onSouthernHemiSphere, ::IsNewData(info));
        itsSoundingDataCacheForTooltips.insert(std::make_pair(NFmiMTATempSystem::SoundingDataCacheMapKey(usedTempInfo, theProducer, theModelRunIndex), sounding));
	}
	else
	{
		if(!IsInScanMode())
		{
			ResetSelectedDataInEmptyCase(theProducerIndex, theModelRunIndex);
			DrawMainDataLegendInEmptyCase(mainCurve, usedTempInfo, theProducer, theModelRunIndex, theProducerIndex);
		}
	}
}

TotalSoundingData NFmiTempView::GetTotalsoundingData(boost::shared_ptr<NFmiFastQueryInfo>& info, NFmiMTATempSystem::TempInfo& usedTempInfo, const NFmiMTATempSystem::SoundingProducer& theProducer, int theProducerIndex)
{
	auto usedLocationWithName = ::GetSoundingLocation(info, usedTempInfo, itsCtrlViewDocumentInterface->ProducerSystem());
	usedTempInfo.Latlon(usedLocationWithName.GetLocation());
	auto groundDataInfo = ::GetPossibleGroundData(itsCtrlViewDocumentInterface, info, theProducer);
	ResetTextualScrollingIfSoundingDataChanged(theProducer, usedTempInfo, info, theProducerIndex);
	NFmiMTATempSystem& mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
	TotalSoundingData sounding(mtaTempSystem);
	FillSoundingData(info, sounding, usedTempInfo.Time(), usedLocationWithName, groundDataInfo, theProducer);
	return sounding;
}

void NFmiTempView::DrawMainDataLegendInEmptyCase(bool mainCurve, const NFmiMTATempSystem::TempInfo& usedTempInfo, const NFmiMTATempSystem::SoundingProducer& theProducer, int theModelRunIndex, int theProducerIndex)
{
	if(mainCurve)
	{
		// Vaikka dataa ei lˆytynyt, piirret‰‰n kuitenkin luotauksen legenda tiedot n‰kyviin
		NFmiSoundingData emptySoundingData;
		emptySoundingData.Location(NFmiLocation(usedTempInfo.Latlon()));
		emptySoundingData.Time(usedTempInfo.Time());
		// Haetaan luotausdata ilman aika hakuehtoa, jottaa saataisiin mahdollinen originTime datasta
		boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->InfoOrganizer()->FindSoundingInfo(theProducer, theModelRunIndex, NFmiInfoOrganizer::ParamCheckFlags(true));
		if(info)
		{
			emptySoundingData.OriginTime(info->OriginTime());
			if(info->IsGrid())
			{
				auto usedLocationWithName = ::GetSoundingLocation(info, usedTempInfo, itsCtrlViewDocumentInterface->ProducerSystem());
				emptySoundingData.Location(usedLocationWithName);
			}
		}
		TotalSoundingData totalData;
		totalData.itsSoundingData = emptySoundingData;
		DrawStationInfo(totalData, theProducerIndex, ::IsNewData(info));
	}
}

// Jos ei lˆytynyt mit‰‰n dataa ja kyse oli 1. piirrett‰v‰st‰ luotausdatasta, pit‰‰ itsSelectedProducerSoundingData 
// dataosa nollata, jotta tekstimuotoisiin sivun‰yttˆihin ei j‰isi vanha data 'kummittelemaan'
void NFmiTempView::ResetSelectedDataInEmptyCase(int theProducerIndex, int theModelRunIndex)
{
	if(IsSelectedProducerIndex(theProducerIndex) && theModelRunIndex == 0)
	{
		itsSelectedProducerSoundingData = TotalSoundingData();
	}
}

bool NFmiTempView::FillSoundingData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, TotalSoundingData &theSoundingData, const NFmiMetTime &theTime, const NFmiLocation &theLocation, boost::shared_ptr<NFmiFastQueryInfo> &theGroundDataInfo, const NFmiMTATempSystem::SoundingProducer &theProducer)
{
	bool status = false;
	if(theProducer.useServer())
	{
		status = FillSoundingDataFromServer(theProducer, theSoundingData.itsSoundingData, theTime, theLocation);
	}
	else
	{
		if(DoIntegrationSounding(theInfo, theSoundingData))
		{
			status = FillIntegrationSounding(theInfo, theSoundingData, theTime, theLocation, theGroundDataInfo);
		}
		else
		{
			status = NFmiSoundingIndexCalculator::FillSoundingData(theInfo, theSoundingData.itsSoundingData, theTime, theLocation, theGroundDataInfo);
		}
	}

	// T‰m‰ tehd‰‰n vasta lopulliselle, siis mahdollisesti yhdistelm‰luotausdatalle, kerralla
	FillInPossibleMissingPressureData(theSoundingData.itsSoundingData, theProducer, theTime, theLocation);
	return status;
}

bool NFmiTempView::DoIntegrationSounding(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, TotalSoundingData& theSoundingData)
{
	auto isModelData = theInfo->IsGrid();
	auto doAreaIntegration = theSoundingData.itsIntegrationRangeInKm > 0;
	auto doTimeIntegration = theSoundingData.itsIntegrationTimeOffset1InHours != 0 || theSoundingData.itsIntegrationTimeOffset2InHours != 0;
	return isModelData && (doAreaIntegration || doTimeIntegration);
}

bool NFmiTempView::FillIntegrationSounding(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, TotalSoundingData& theSoundingData, const NFmiMetTime& theTime, const NFmiLocation& theLocation, boost::shared_ptr<NFmiFastQueryInfo>& theGroundDataInfo)
{
	auto rangeInMeters = theSoundingData.itsIntegrationRangeInKm * 1000.;
	bool singleLocation = (rangeInMeters == 0);
	auto timeOffset1InHours = theSoundingData.itsIntegrationTimeOffset1InHours;
	auto timeOffset2InHours = theSoundingData.itsIntegrationTimeOffset2InHours;
	if(timeOffset1InHours > timeOffset2InHours)
	{
		// Varmistetaan ett‰ laskuihin offsetit menev‰t oikeassa j‰rjestyksess‰
		std::swap(timeOffset1InHours, timeOffset2InHours);
	}
	auto startTime = theTime;
	startTime.ChangeByMinutes(boost::math::iround(timeOffset1InHours * 60.));
	long minuteRange = boost::math::iround((timeOffset2InHours - timeOffset1InHours) * 60.);
	unsigned long timeIndex1 = gMissingIndex;
	unsigned long timeIndex2 = gMissingIndex;
	bool singleTime = (minuteRange == 0);
	if(singleTime || NFmiFastInfoUtils::FindTimeIndicesForGivenTimeRange(theInfo, startTime, minuteRange, timeIndex1, timeIndex2))
	{
		std::vector<unsigned long> locationIndexes;
		if(!singleLocation)
		{
			locationIndexes = CalcAreaIntegrationLocationIndexes(theInfo, theLocation, rangeInMeters);
		}
		if(singleLocation || !locationIndexes.empty())
		{
			return FillIntegrationSounding(theInfo, theSoundingData, startTime, theLocation, theGroundDataInfo, timeIndex1, timeIndex2, locationIndexes);
		}
	}

	return false;
}

std::vector<FmiParameterName> gSoundingParametersWithNormalAvg{ kFmiTemperature,kFmiDewPoint,kFmiHumidity,kFmiPressure,kFmiGeopHeight,kFmiWindUMS,kFmiWindVMS,kFmiTotalCloudCover };

static bool CalcAvgSoundingData(TotalSoundingData& theSoundingDataOut, std::vector<NFmiSoundingData>& soundingDataList, boost::shared_ptr<NFmiFastQueryInfo>& theInfo, boost::shared_ptr<NFmiFastQueryInfo>& theGroundDataInfo)
{
	if(!soundingDataList.empty())
	{
		NFmiDataModifierAvg avg;
		auto soundingDataSize = soundingDataList.size();
		auto parameterSize = gSoundingParametersWithNormalAvg.size();
		auto levelSize = soundingDataList.front().GetParamData(gSoundingParametersWithNormalAvg.front()).size();
		// Varmistetaan ett‰ tulosdatassa on tilaa l‰htˆdatan levelien verran
		for(size_t parameterIndex = 0; parameterIndex < parameterSize; parameterIndex++)
		{
			theSoundingDataOut.itsSoundingData.GetParamData(gSoundingParametersWithNormalAvg[parameterIndex]).resize(levelSize);
		}

		// Lasketaan jokaisen l‰htˆdatan parametetrien keskiarvo kaikille leveleille erikseen ja 
		// sijoitetaan saatu keskiarvo tulosdatan kyseisen parametrin ja kyseisen levelin kohdalle.
		for(size_t levelIndex = 0; levelIndex < levelSize; levelIndex++)
		{
			for(size_t parameterIndex = 0; parameterIndex < parameterSize; parameterIndex++)
			{
				avg.Clear();
				for(size_t soundingDataIndex = 0; soundingDataIndex < soundingDataSize; soundingDataIndex++)
				{
					avg.Calculate(soundingDataList[soundingDataIndex].GetParamData(gSoundingParametersWithNormalAvg[parameterIndex])[levelIndex]);
				}
				theSoundingDataOut.itsSoundingData.GetParamData(gSoundingParametersWithNormalAvg[parameterIndex])[levelIndex] = avg.CalculationResult();
			}
		}

		// Tietyt tuuliparametrit (WS, WD, WVec) pit‰‰ laskea u- ja v-komponenttien Avg arvojen avulla
		const auto& u = theSoundingDataOut.itsSoundingData.GetParamData(kFmiWindUMS);
		const auto& v = theSoundingDataOut.itsSoundingData.GetParamData(kFmiWindVMS);
		auto& WS = theSoundingDataOut.itsSoundingData.GetParamData(kFmiWindSpeedMS);
		auto& WD = theSoundingDataOut.itsSoundingData.GetParamData(kFmiWindDirection);
		auto& WVec = theSoundingDataOut.itsSoundingData.GetParamData(kFmiWindVectorMS);
		for(size_t levelIndex = 0; levelIndex < levelSize; levelIndex++)
		{
			NFmiFastInfoUtils::CalcDequeWindSpeedAndDirectionFromComponents(u, v, WS, WD);
			NFmiFastInfoUtils::CalcDequeWindVectorFromSpeedAndDirection(WS, WD, WVec);
		}

		theSoundingDataOut.itsSoundingData.MakeFillDataPostChecks(theInfo, theGroundDataInfo);
		return true;
	}
	return false;
}

bool NFmiTempView::FillIntegrationSounding(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, TotalSoundingData& theSoundingData, const NFmiMetTime& theTime, const NFmiLocation& theLocation, boost::shared_ptr<NFmiFastQueryInfo>& theGroundDataInfo, unsigned long timeIndex1, unsigned long timeIndex2, const std::vector<unsigned long>& locationIndexes)
{
	std::vector<NFmiSoundingData> soundingDataList;
	bool singleLocation = locationIndexes.empty();
	bool singleTime = (timeIndex1 == timeIndex2);
	theSoundingData.itsIntegrationPointsCount = singleLocation ? 1 : int(locationIndexes.size());
	theSoundingData.itsIntegrationTimesCount = singleTime ? 1 : int(timeIndex2 - timeIndex1 + 1);
	NFmiSoundingData data;
	for(auto timeIndex = timeIndex1; timeIndex <= timeIndex2; timeIndex++)
	{
		theInfo->TimeIndex(timeIndex);
		auto currentTime = singleTime ? theTime : theInfo->Time();
		if(singleLocation)
		{
			if(NFmiSoundingIndexCalculator::FillSoundingData(theInfo, data, currentTime, theLocation, theGroundDataInfo))
			{
				soundingDataList.push_back(data);
			}
		}
		else
		{
			for(auto locationIndex : locationIndexes)
			{
				theInfo->LocationIndex(locationIndex);
				NFmiLocation usedLocation(theInfo->LatLon());
				if(NFmiSoundingIndexCalculator::FillSoundingData(theInfo, data, currentTime, usedLocation, theGroundDataInfo))
				{
					soundingDataList.push_back(data);
				}
			}
		}
		if(singleTime)
			break; // tullaan loopista ulos jos vain yhden ajan tarkastelu, en saanut rakennettua sopivaa ehtoa for-looppiin ja j‰‰ ikilooppiin muuten
	}

	if(!soundingDataList.empty())
	{
		theSoundingData.itsSoundingData.Time(theTime);
		theSoundingData.itsSoundingData.Location(theLocation);
		theSoundingData.itsSoundingData.OriginTime(theInfo->OriginTime());
		return ::CalcAvgSoundingData(theSoundingData, soundingDataList, theInfo, theGroundDataInfo);
	}
	return false;
}

// Oletus: theGridPoint sis‰lt‰‰ kokonaisluvut X ja Y arvoina
static unsigned long CalcGridPointLocationIndex(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, const NFmiPoint& theGridPoint)
{
	if(theGridPoint.X() < 0 || theGridPoint.Y() < 0)
		return gMissingIndex;
	if(theGridPoint.X() >= theInfo->GridXNumber() || theGridPoint.Y() >= theInfo->GridYNumber())
		return gMissingIndex;

	// grid-point x/y arvot floor:ataan indeksi laskuja varten varmuuden vuoksi
	return ((int)theGridPoint.Y() * theInfo->GridXNumber()) + (int)theGridPoint.X();
}

struct InsideGridRangeData
{
	unsigned long locationIndex = gMissingIndex;
	bool insideGrid = false;
	bool insideRange = false;
};

static InsideGridRangeData IsInsideGridAndRange(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, const NFmiPoint& theCheckedGridPoint, const NFmiLocation& theLocationOrig, double theRangeInMeters)
{
	InsideGridRangeData resultData;
	resultData.locationIndex = ::CalcGridPointLocationIndex(theInfo, theCheckedGridPoint);
	resultData.insideGrid = resultData.locationIndex != gMissingIndex;
	auto gridPointLatlon = theInfo->Grid()->GridToLatLon(theCheckedGridPoint);
	resultData.insideRange = theLocationOrig.Distance(gridPointLatlon) <= theRangeInMeters;
	return resultData;
}

static void AddPossibleMatches(bool &isAnyPointInsideRange, std::set<unsigned long>& matchingGridPointRingLocationIndexesSet, const InsideGridRangeData& insideGridRangeData)
{
	if(insideGridRangeData.insideRange)
	{
		isAnyPointInsideRange = true;
		if(insideGridRangeData.locationIndex != gMissingIndex)
		{
			matchingGridPointRingLocationIndexesSet.insert(insideGridRangeData.locationIndex);
		}
	}
}

// Palauttaa parin jossa:
// first:issa on tieto oliko yksik‰‰n testattu hilapiste tarpeeksi l‰hell‰ theLocation:ia (voi olla datan oman hilan ulkonakin)
// second:issa on lista niist‰ datan hilapisteist‰, jotka olivat s‰teen sis‰ll‰ originaalipisteest‰
static std::pair<bool, std::vector<unsigned long>> CalcMatchingGridPointRingLocationIndexes(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, const NFmiPoint& theGridPointOrig, const NFmiLocation& theLocationOrig, double theRangeInMeters, int ringIndex)
{
	if(ringIndex == 0)
	{
		auto isInsideResult = ::IsInsideGridAndRange(theInfo, theGridPointOrig, theLocationOrig, theRangeInMeters);
		auto locationIndexes = (isInsideResult.insideGrid) ? std::vector<unsigned long>{isInsideResult.locationIndex} : std::vector<unsigned long>();
		return std::make_pair(isInsideResult.insideRange, locationIndexes);
	}

	// Left eli pysty sarake, kun x offset on -ringIndex
	int leftIndexX = (int)(theGridPointOrig.X() - ringIndex);
	int rightIndexX = (int)(theGridPointOrig.X() + ringIndex);
	// Down eli vaaka sarake, kun y offset on -ringIndex
	int downIndexY = (int)(theGridPointOrig.Y() - ringIndex);
	int upIndexY = (int)(theGridPointOrig.Y() + ringIndex);

	bool isAnyPointInsideRange = false;
	// std::set:in avulla eliminoidaan keh‰nkulmien duplikaatit
	auto resultSet = std::set<unsigned long>();
	for(int index = -ringIndex; index <= ringIndex; index++)
	{
		// K‰yd‰‰n l‰pi 4 annetun keh‰n reunustaa (left, right, up, down)
		NFmiPoint leftColumnGridPoint(leftIndexX, theGridPointOrig.Y() + index);
		::AddPossibleMatches(isAnyPointInsideRange, resultSet, ::IsInsideGridAndRange(theInfo, leftColumnGridPoint, theLocationOrig, theRangeInMeters));
		NFmiPoint rightColumnGridPoint(rightIndexX, theGridPointOrig.Y() + index);
		::AddPossibleMatches(isAnyPointInsideRange, resultSet, ::IsInsideGridAndRange(theInfo, rightColumnGridPoint, theLocationOrig, theRangeInMeters));
		NFmiPoint upColumnGridPoint(theGridPointOrig.X() + index, upIndexY);
		::AddPossibleMatches(isAnyPointInsideRange, resultSet, ::IsInsideGridAndRange(theInfo, upColumnGridPoint, theLocationOrig, theRangeInMeters));
		NFmiPoint downColumnGridPoint(theGridPointOrig.X() + index, downIndexY);
		::AddPossibleMatches(isAnyPointInsideRange, resultSet, ::IsInsideGridAndRange(theInfo, downColumnGridPoint, theLocationOrig, theRangeInMeters));
	}

	std::vector<unsigned long> resultVector(resultSet.begin(), resultSet.end());
	return std::make_pair(isAnyPointInsideRange, resultVector);
}

// Tee funktio joka laskee annetun s‰teen sis‰lt‰ lˆytyv‰t hilapisteiden indeksit.
// Katso mallia NFmiInfoAreaMaskOccurrance::InitializeLocationIndexCaches metodista,
// mutta optimoi koodia niin ett‰ lasket ensin kilometreihin sopivan laatikon sijainnin 
// datan maailmassa ja muuta ne x/y suuntaisiksi hilapiste lokaatioiksi 0-n, 0-m
// Ja vasta t‰lle pikkulaatikon alueella oleville pisteille tee lopullinen rangetarkistus.
std::vector<unsigned long> NFmiTempView::CalcAreaIntegrationLocationIndexes(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, const NFmiLocation& theLocation, double theRangeInMeters)
{
	std::vector<unsigned long> locationIndexes;
	if(theInfo->IsGrid())
	{
		auto gridPointOrigPrecise = theInfo->Grid()->LatLonToGrid(theLocation.GetLocation());
		// Otetaan l‰himp‰‰n tasahilaan pyˆristetty aloituspisteeksi
		auto gridPointOrig = NFmiPoint(std::round(gridPointOrigPrecise.X()), std::round(gridPointOrigPrecise.Y()));
		// Ruvetaan tutkimaan tuon originaali gridPoint:in ymp‰rille keh‰ss‰ ett‰ lˆytyykˆ
		// pisteit‰ riitt‰v‰n l‰helt‰, kunnes joltain keh‰lt‰ ei lˆydy en‰‰ yht‰‰n osumaa.
		for(int ringIndex = 0; ringIndex < (int)theInfo->GridXNumber(); ringIndex++)
		{
			auto result = ::CalcMatchingGridPointRingLocationIndexes(theInfo, gridPointOrig, theLocation, theRangeInMeters, ringIndex);
			if(result.first == false)
			{
				// Tarkastetulta keh‰lt‰ ei lˆytynyt yht‰‰n hilapistett‰ (hilan sis‰lt‰ tai ulkoa), voidaan lopettaa etsiminen
				break; 
			}

			auto& ringLocationIndexes = result.second;
			if(!ringLocationIndexes.empty())
			{
				locationIndexes.insert(locationIndexes.end(), ringLocationIndexes.begin(), ringLocationIndexes.end());
			}
		}
	}
	return locationIndexes;
}

void NFmiTempView::FillInPossibleMissingPressureData(NFmiSoundingData& theSoundingData, const NFmiProducer &dataProducer, const NFmiMetTime& theTime, const NFmiLocation& theLocation)
{
	if(theSoundingData.HeightDataAvailable() && !theSoundingData.PressureDataAvailable())
	{
		for(const auto& selectedProducer : itsCtrlViewDocumentInterface->GetMTATempSystem().SoundingComparisonProducers())
		{
			if(selectedProducer.GetIdent() != dataProducer.GetIdent())
			{
				boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->InfoOrganizer()->FindSoundingInfo(selectedProducer, theTime, theLocation.GetLocation(), 0, NFmiInfoOrganizer::ParamCheckFlags(true));
				if(info && info->IsGrid() && info->PressureDataAvailable() && info->TimeDescriptor().IsInside(theTime))
				{
					auto& pVector = theSoundingData.GetParamData(kFmiPressure);
					auto& hVector = theSoundingData.GetParamData(kFmiGeomHeight);
					if(pVector.size() == hVector.size())
					{
						info->Param(kFmiPressure);
						const auto& latlon = theLocation.GetLocation();
						for(size_t index = 0; index < hVector.size(); index++)
						{
							auto height = hVector[index];
							if(height != kFloatMissing)
							{
								auto pressure = info->HeightValue(height, latlon, theTime);
								pVector[index] = pressure;
							}
						}
						theSoundingData.SetVerticalParamStatus();
						break;
					}
				}
			}
		}
	}
}

// palauttaa annetun laatikon sis‰lt‰ pisteen, johon relatiivisessa 0,0 - 1,1 maailmassa
// oleva piste osoittaa. T‰m‰ relatiivinen maailma on positiivinen oikealle ja ylˆs.
static NFmiPoint GetRelativeLocationFromRect(const NFmiRect &theRect, const NFmiPoint &thePoint)
{
	double x = theRect.Left() + thePoint.X() * theRect.Width();
	double y = theRect.Bottom() - thePoint.Y() * theRect.Height();
	return NFmiPoint(x, y);
}

static NFmiPoint GetRelativePointFromWindSpeedSpace(double u, double v, double minU, double maxU, double minV, double maxV, const NFmiRect &hodoRect)
{
	// sovitetaan tuulikomponentit ensin 0,0 - 1,1 maailmaan
	double uRel = (u-minU)/(maxU-minU);
	double vRel = (v-minV)/(maxV-minV);
	return GetRelativeLocationFromRect(hodoRect, NFmiPoint(uRel, vRel));
}

NFmiPoint NFmiTempView::GetRelativePointFromHodograf(double u, double v)
{
	auto& hodografViewData = itsCtrlViewDocumentInterface->GetMTATempSystem().GetHodografViewData();
	auto hodografScaleMaxValue = hodografViewData.ScaleMaxValue();
	return ::GetRelativePointFromWindSpeedSpace(u, v, -hodografScaleMaxValue, hodografScaleMaxValue, -hodografScaleMaxValue, hodografScaleMaxValue, hodografViewData.Rect());
}

bool NFmiTempView::MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta)
{
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
	auto& hodografViewData = mtaTempSystem.GetHodografViewData();
	const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
	auto isInsideTextualSoundingRect = itsTempViewDataRects.getTextualSoundingDataSideViewRect().IsInside(thePlace);
	if(!isInsideTextualSoundingRect && theKey & kCtrlKey && theKey & kShiftKey)
	{
		// CTRL + SHIFT + rullalla s‰‰det‰‰n kaikkialta ikkunasta valitun tuottaja indeksi‰
		auto direction = (theDelta > 0) ? kDown : kUp;
		mtaTempSystem.ToggleSelectedProducerIndex(direction);
		return true;
	}

	if(mtaTempSystem.ShowHodograf() && hodografViewData.Rect().IsInside(thePlace))
	{
		// Jos hodografi n‰kyy ja hiiren kursori on sen sis‰ll‰
		if(theKey & kCtrlKey)
		{
			// s‰‰det‰‰n suhteellista kokoa
			hodografViewData.AdjustRelativiHeightFactor(theDelta);
		}
		else
		{
			// s‰‰det‰‰n arvoalueen kokoa
			hodografViewData.AdjustScaleMaxValue(theDelta);
		}
		return true;
	}
	else if(itsTempViewDataRects.getAnimationButtonRect().IsInside(thePlace))
	{
		if(theDelta > 0)
            mtaTempSystem.ChangeSoundingsInTime(kForward);
		else
            mtaTempSystem.ChangeSoundingsInTime(kBackward);
		return true;
	}
	else if(itsTempViewDataRects.getAnimationStepButtonRect().IsInside(thePlace))
	{
		if(theDelta > 0)
            mtaTempSystem.NextAnimationStep();
		else
            mtaTempSystem.PreviousAnimationStep();
		return true;
	}
	else if(isInsideTextualSoundingRect)
	{
		auto drawUpward = mtaTempSystem.GetSoundingViewSettingsFromWindowsRegisty().SoundingTextUpward();
		int scrollValue = 3; // pelkk‰ rulla
		if(theKey & kCtrlKey)
		{
			scrollValue = 20; // rulla + ctrl
			if(theKey & kShiftKey)
				scrollValue = 100; // rulla + ctrl + shift
		}

		if(theDelta > 0)
			return itsTempViewScrollingData.doScrolling(-scrollValue, drawUpward);
		else
			return itsTempViewScrollingData.doScrolling(scrollValue, drawUpward);
	}
	else if(dataRect.Bottom() < thePlace.Y() && dataRect.Left() < thePlace.X())
	{ // nyt kursori on l‰mpp‰ri asteikolla
		double change = theDelta < 0 ? 2 : -2;
		if(dataRect.Center().X() > thePlace.X())
            mtaTempSystem.TAxisStart(mtaTempSystem.TAxisStart() + change);
		else
            mtaTempSystem.TAxisEnd(mtaTempSystem.TAxisEnd() + change);
		return true;
	}
	else if(dataRect.Left() > thePlace.X() && dataRect.Bottom() > thePlace.Y())
	{ // nyt kursori on paineasteikolla
		double change = theDelta < 0 ? -10 : 10;
		if(dataRect.Center().Y() > thePlace.Y())
		{
			change = GetPAxisChangeValue(::fabs(change)); // yl‰p‰‰t‰ s‰‰dett‰ess‰ pit‰‰ laskeskella hieman
			double finalValue = round((mtaTempSystem.PAxisEnd() + ((theDelta < 0) ? -change : change)) / change) * change;
            mtaTempSystem.PAxisEnd(finalValue);
		}
		else
            mtaTempSystem.PAxisStart(mtaTempSystem.PAxisStart() + change);
		return true;
	}

	// lopuksi katsotaan ollaanko tarpeeksi l‰hell‰ tuuliviirej‰ ja muokataanko niit‰
	return ModifySoundingWinds(thePlace, theKey, theDelta);
}

static const NFmiRect CalcGeneralWindBarbRect(const NFmiToolBox *theToolBox, const NFmiPoint &theWindvectorSizeInPixels)
{
	return NFmiRect(0,0, theToolBox->SX(static_cast<long>(theWindvectorSizeInPixels.X()*2)), theToolBox->SY(static_cast<long>(theWindvectorSizeInPixels.Y()*2)));
}

static double CalcWindBarbXPos(const NFmiRect &theDataViewRect, const NFmiRect &theWindBarbSizeRect, int theProducerIndex)
{ 
	// theProducerIndex pit‰‰ lis‰t‰ yksi
	return theDataViewRect.Left() + theWindBarbSizeRect.Width() * 0.5 + theWindBarbSizeRect.Width() * 0.5 * (theProducerIndex + 1);
}

static const int gWindModificationAreaWidthInPixels = 11;

void NFmiTempView::DrawWindModificationArea(void)
{
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    if(mtaTempSystem.WindModificationAreaLineInfo().DrawLine() == false)
		return ;

	NFmiDrawingEnvironment envi; // tehd‰‰n vaalean harmaat viivat

	const NFmiTempLineInfo &lineInfo = mtaTempSystem.WindModificationAreaLineInfo();
	const NFmiTempLabelInfo labelInfo; // t‰m‰ on feikki, ei v‰li‰
	int trueLineWidth = boost::math::iround(1 * itsDrawSizeFactor.X());
	bool drawSpecialLines = SetHelpLineDrawingAttributes(itsToolBox, &envi, labelInfo, lineInfo, trueLineWidth, true);

	NFmiPoint windVecSizeInPixels = mtaTempSystem.WindvectorSizeInPixels();
	windVecSizeInPixels = ScaleOffsetPoint(windVecSizeInPixels);
	NFmiRect windBarbRect(::CalcGeneralWindBarbRect(itsToolBox, windVecSizeInPixels));
	const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
	double xPos = ::CalcWindBarbXPos(dataRect, windBarbRect, 0); // vain 1. (0:s indeksi) luotausta voi muokata, joten piirret‰‰n siihen liittyv‰ muokkaus alue
	double width = ConvertFixedPixelSizeToRelativeWidth(gWindModificationAreaWidthInPixels);

	DrawLine(NFmiPoint(xPos-width/2., dataRect.Top()), NFmiPoint(xPos-width/2., dataRect.Bottom()), drawSpecialLines, trueLineWidth, true, 0, &envi);
	DrawLine(NFmiPoint(xPos+width/2., dataRect.Top()), NFmiPoint(xPos+width/2., dataRect.Bottom()), drawSpecialLines, trueLineWidth, true, 0, &envi);
}

bool NFmiTempView::ModifySoundingWinds(const NFmiPoint &thePlace, unsigned long theKey, short theDelta)
{
	if(itsFirstSoundinWindBarbXPos != kFloatMissing)
	{
		double width = ConvertFixedPixelSizeToRelativeWidth(gWindModificationAreaWidthInPixels);
		const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
		NFmiRect windModRect(itsFirstSoundinWindBarbXPos - width/2., dataRect.Top(), itsFirstSoundinWindBarbXPos + width/2., dataRect.Bottom());
		if(windModRect.IsInside(thePlace))
		{ 
			// Jos oltiin tarpeeksi l‰hell‰ 1. soundingdatan tuuliviiri rivistˆ‰, muokataan l‰hinn‰ olevaa viiri‰
			float P = static_cast<float>(y2p(thePlace.Y()));
			bool windDirModified = theKey & kCtrlKey;
			FmiParameterName parId = windDirModified ? kFmiWindDirection : kFmiWindSpeedMS;
			float addValue = windDirModified ? 10.f : 1.f;
			if(theDelta < 0)
				addValue = -addValue;
			float minValue = 0;
			float maxValue = windDirModified ? 360 : kFloatMissing;
			bool fCircularValue = windDirModified ? true : false;
			bool status = itsSelectedProducerSoundingData.itsSoundingData.Add2ParamAtNearestP(P, parId, addValue, minValue, maxValue, fCircularValue);
			itsSelectedProducerSoundingData.itsSoundingData.UpdateUandVParams();
			return status;
		}
	}
	return false;
}

void NFmiTempView::DrawHodograf(NFmiSoundingData & theData, int theProducerIndex)
{
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    if(!mtaTempSystem.ShowHodograf())
		return ;
	if(mtaTempSystem.ShowOnlyFirstSoundingInHodograf() && !IsSelectedProducerIndex(theProducerIndex))
		return ;

	NFmiRect oldRect(itsToolBox->RelativeClipRect());
	itsToolBox->RelativeClipRect(mtaTempSystem.GetHodografViewData().Rect(), true);
	NFmiColor oldFillColor(itsDrawingEnvironment->GetFillColor());

	DrawHodografBase(theProducerIndex);
	DrawHodografUpAndDownWinds(theData, theProducerIndex);
	DrawHodografCurve(theData, theProducerIndex);
	DrawHodografWindVectorMarkers(theData, theProducerIndex);
	DrawHodografHeightMarkers(theData, theProducerIndex);

	// lopuksi palautetaan vanhat v‰rit
	itsDrawingEnvironment->SetFillColor(oldFillColor);
	itsToolBox->RelativeClipRect(oldRect, true);
}

// Pieter Groenemeijer wrote:
// The Corfidi-vector technique, which can help forecasters estimate the
// motion of mesoscale convective systems. There are two vectors that
// resemble two extremes modes of MCS propagation: upwind and downwind.
// It would be helpful to plot the following on the hodograph (and label
// the correpsonding direction and speed in m/s in the bar on the right)
//
// 1. an arrow from the origin to (u_upwind,v_upwind) labelled 'upwind'
//
// u_upwind = u_cloud_layer - u_llj
// u_cloud_layer = average u between 300 and 850 hPa (non-pressure
// weighted, i.e. take average of wind at equal height intervals).
//
// u_llj = u(p_max_wind)
// p_max_wind = level below the 850 hPa level at which sqrt(u^2+ v^2) is maximal
//
// v_upwind = v_cloud_layer - v_llj
// v_llj = v(p_max_wind)
// v_cloud_layer = average v between 300 and 850 hPa (non-pressure weighted)
//
// 2. an arrow from the origin to (u_downwind,v_downwind) labelled
// 'downwind' or 'downw.'
//
// u_downwind = 2 * u_cloud_layer - u_llj
// v_downwind = 2 * v_cloud_layer - v_llj
// 
// HUOM! Paavo Korpelalta tuli viesti‰, ett‰ down_wind lasketaankin seuraavalla tavalla:
// Paavo: u_downwind = u_cloud_layer + u_upwind (Poikkeaa huomattavasti originaalista)
// Lis‰ksi pari pienemp‰‰, virityst‰:
// Paavo: u_cloud_layer = vertz_avg(u, LCL, 0.6 * EL)  // T‰st‰ seuraa ongelma, koska EL:n arvot ovat puuttuvaa monissa tilanteissa, lis‰ksi n‰it‰ LCL+EL arvoja ei ole k‰ytˆss‰ t‰‰ll‰, ellei niit‰ lasketa erikseen
// Paavo: u_llj = u-komponentti(p_max_wind), miss‰ p_max_wind on on maksimi tuulennopeus 0 ja 1500 metrin v‰liss‰

class UpDownWindCalculationBaseData
{
public:
	double lclPressure = kFloatMissing;
	double elPressure = kFloatMissing;
	double h1 = kFloatMissing;
	double h2 = kFloatMissing;
};

static UpDownWindCalculationBaseData CalcUpDownWindBaseData(NFmiSoundingData& theData)
{
	UpDownWindCalculationBaseData baseData;
	// Yritet‰‰n laskea EL:lle ei-puuttuvaa arvoa eri laskutyypeill‰ tietyss‰ priorisointij‰rjestyksess‰
	FmiLCLCalcType usedLclCalcType = kLCLCalcMostUnstable;
	theData.CalcLFCIndex(usedLclCalcType, baseData.elPressure);
	if(baseData.elPressure == kFloatMissing)
	{
		usedLclCalcType = kLCLCalc500m2;
		theData.CalcLFCIndex(usedLclCalcType, baseData.elPressure);
		if(baseData.elPressure == kFloatMissing)
		{
			usedLclCalcType = kLCLCalcSurface;
			theData.CalcLFCIndex(usedLclCalcType, baseData.elPressure);
			if(baseData.elPressure == kFloatMissing)
			{
				// Jos surface tyyppikin palautti puuttuvaa, lasketaan jatkossa LCL kuitenkin mostUnstable tyyliin
				usedLclCalcType = kLCLCalcMostUnstable;
			}
		}
	}

	baseData.lclPressure = theData.CalcLCLIndex(usedLclCalcType);
	bool elPressureMissing = (baseData.elPressure == kFloatMissing);
	baseData.h1 = theData.GetValueAtPressure(kFmiGeopHeight, static_cast<float>(baseData.lclPressure));
	double usedEndPressure = elPressureMissing ? 300 : baseData.elPressure;
	baseData.h2 = theData.GetValueAtPressure(kFmiGeopHeight, static_cast<float>(usedEndPressure));
	return baseData;
}

void NFmiTempView::DrawHodografUpAndDownWinds(NFmiSoundingData & theData, int theProducerIndex)
{
	auto upDownWindBaseData = ::CalcUpDownWindBaseData(theData);
	bool elPressureMissing = (upDownWindBaseData.elPressure == kFloatMissing);
	if(upDownWindBaseData.h1 != kFloatMissing && upDownWindBaseData.h2 != kFloatMissing)
	{
		double usedUpperLimitHeight = upDownWindBaseData.h2 * 0.6;
		double u_cloud_layer = kFloatMissing;
		double v_cloud_layer = kFloatMissing;
		theData.CalcAvgWindComponentValues(upDownWindBaseData.h1, usedUpperLimitHeight, u_cloud_layer, v_cloud_layer);
		if(u_cloud_layer != kFloatMissing && v_cloud_layer != kFloatMissing)
		{
			float lowPressureLimitForMaxWind = theData.GetValueAtHeight(kFmiPressure, 0);
			float highPressureLimitForMaxWind = theData.GetValueAtHeight(kFmiPressure, 1500);
			float maxWSPressure = theData.FindPressureWhereHighestValue(kFmiWindSpeedMS, lowPressureLimitForMaxWind, highPressureLimitForMaxWind);
			if(maxWSPressure != kFloatMissing)
			{
				double u_llj = kFloatMissing;
				u_llj = theData.GetValueAtPressure(kFmiWindUMS, maxWSPressure);
				double v_llj = kFloatMissing;
				v_llj = theData.GetValueAtPressure(kFmiWindVMS, maxWSPressure);
				if(u_llj != kFloatMissing && v_llj != kFloatMissing)
				{
					// s‰‰det‰‰n piirrett‰vien nuolien piirto ominaisuudet
					NFmiColor markerFillColor(0.8f, 0.8f, 0.8f);
					NFmiColor markerFrameColor(markerFillColor);
					NFmiColor textColor(itsCtrlViewDocumentInterface->GetMTATempSystem().SoundingColor(theProducerIndex));
					NFmiDrawingEnvironment markEnvi; // laitetaan oma envi ympyr‰lle
					markEnvi.SetFrameColor(markerFrameColor);
					markEnvi.SetFillColor(markerFillColor);
					markEnvi.EnableFill();
					long arrowHeadPixelSize = 3;
					double markWidth = ConvertFixedPixelSizeToRelativeWidth(arrowHeadPixelSize);
					double markHeight = ConvertFixedPixelSizeToRelativeHeight(arrowHeadPixelSize);
					NFmiPoint scale(markWidth, markHeight);

					// piirr‰ nuoli (0, 0) -> (u_upwind, v_upwind) ja laita label "upwind"
					double u_upwind = u_cloud_layer - u_llj;
					double v_upwind = v_cloud_layer - v_llj;
					{
						NFmiPoint relP1(GetRelativePointFromHodograf(0, 0));
						NFmiPoint relP2(GetRelativePointFromHodograf(u_upwind, v_upwind));

						// lasketaan upwind-vektorin suunta asteina
						NFmiWindDirection theDirection(u_upwind, v_upwind);
						double vdir1 = theDirection.Value();


						// piirr‰ upwind-nuolen viiva osio
						NFmiLine line1(relP1, relP2, 0, &markEnvi);
						itsToolBox->Convert(&line1);

						// tehd‰‰n piirrett‰v‰ upwind-nuolen k‰rki kolmio ja piirret‰‰n se
						NFmiPolyline markerPolyLine1(itsRect, 0, &markEnvi);
						markerPolyLine1.AddPoint(::RotatePoint(NFmiPoint(-1, 1), vdir1));
						markerPolyLine1.AddPoint(::RotatePoint(NFmiPoint(0, -1), vdir1));
						markerPolyLine1.AddPoint(::RotatePoint(NFmiPoint(1, 1), vdir1));
						itsToolBox->DrawPolyline(&markerPolyLine1, relP2, scale);
						std::string upWindMarker = elPressureMissing ? "U*" : "U";
						DrawHodografTextWithMarker(upWindMarker, static_cast<float>(u_upwind), static_cast<float>(v_upwind), textColor, markerFrameColor, markerFillColor, 5, 18, kLeft, kNone);
					}

					// sitten downwind
					{
						double u_downwind = 2 * u_cloud_layer - u_llj;
						double v_downwind = 2 * v_cloud_layer - v_llj;

						// piirr‰ nuoli (0, 0) -> (u_downwind, v_downwind) ja laita label "downwind"
						NFmiPoint relP1(GetRelativePointFromHodograf(0, 0));
						NFmiPoint relP2(GetRelativePointFromHodograf(u_downwind, v_downwind));

						// lasketaan upwind-vektorin suunta asteina
						NFmiWindDirection v1(u_downwind, v_downwind);
						double vdir1 = v1.Value();


						// piirr‰ upwind-nuolen viiva osio
						NFmiLine line1(relP1, relP2, 0, &markEnvi);
						itsToolBox->Convert(&line1);

						// tehd‰‰n piirrett‰v‰ upwind-nuolen k‰rki kolmio ja piirret‰‰n se
						NFmiPolyline markerPolyLine1(itsRect, 0, &markEnvi);
						markerPolyLine1.AddPoint(::RotatePoint(NFmiPoint(-1, 1), vdir1));
						markerPolyLine1.AddPoint(::RotatePoint(NFmiPoint(0, -1), vdir1));
						markerPolyLine1.AddPoint(::RotatePoint(NFmiPoint(1, 1), vdir1));
						itsToolBox->DrawPolyline(&markerPolyLine1, relP2, scale);
						std::string downWindMarker = elPressureMissing ? "D*" : "D";
						DrawHodografTextWithMarker(downWindMarker, static_cast<float>(u_downwind), static_cast<float>(v_downwind), textColor, markerFrameColor, markerFillColor, 5, 18, kLeft, kNone);
					}
				}
			}
		}
	}
}

void NFmiTempView::DrawHodografWindVectorMarkers(NFmiSoundingData & theData, int theProducerIndex)
{
	NFmiColor markFillColor(0,0,0);
	NFmiColor markFrameColor(0,0,0);
	NFmiColor textColor(itsCtrlViewDocumentInterface->GetMTATempSystem().SoundingColor(theProducerIndex));

	{
		double u0_6 = kFloatMissing;
		double v0_6 = kFloatMissing;
		theData.Calc_U_and_V_mean_0_6km(u0_6, v0_6);
		if(u0_6 != kFloatMissing && v0_6 != kFloatMissing)
		{
			DrawHodografTextWithMarker("M", static_cast<float>(u0_6), static_cast<float>(v0_6), textColor, markFrameColor, markFillColor, 6, 18, kLeft, kCross);
		}
	}

	{
		double u_ID_right = kFloatMissing;
		double v_ID_right = kFloatMissing;
		theData.Calc_U_and_V_IDs_right(u_ID_right, v_ID_right);
		if(u_ID_right != kFloatMissing && v_ID_right != kFloatMissing)
		{
			DrawHodografTextWithMarker("R", static_cast<float>(u_ID_right), static_cast<float>(v_ID_right), textColor, markFrameColor, markFillColor, 3, 18, kLeft, kTriangle);
		}
	}

	{
		double u_ID_left = kFloatMissing;
		double v_ID_left = kFloatMissing;
		theData.Calc_U_and_V_IDs_left(u_ID_left, v_ID_left);
		if(u_ID_left != kFloatMissing && v_ID_left != kFloatMissing)
		{
			DrawHodografTextWithMarker("L", static_cast<float>(u_ID_left), static_cast<float>(v_ID_left), textColor, markFrameColor, markFillColor, 3, 18, kLeft, kTriangle);
		}
	}
}

void NFmiTempView::DrawHodografBase(int theProducerIndex)
{
	// Piirret‰‰n pohjat vain indeksille 0, koska se on 1. piirtokierroksessa
	if(theProducerIndex == 0)
	{
		// piirr‰ pohja laatikko fillill‰
		int fontSize = boost::math::iround(16 * itsDrawSizeFactor.Y());
		itsDrawingEnvironment->SetFrameColor(NFmiColor(0, 0, 0));
		itsDrawingEnvironment->SetFillColor(NFmiColor(0.99f, 0.98f, 0.92f));
		itsDrawingEnvironment->SetFontSize(NFmiPoint(fontSize, fontSize));
		auto& hodografViewData = itsCtrlViewDocumentInterface->GetMTATempSystem().GetHodografViewData();
		const auto& hodografRect = hodografViewData.Rect();
		NFmiRectangle rec(hodografRect, 0, itsDrawingEnvironment);
		itsToolBox->Convert(&rec);

		// kirjoita hodografi teksti yl‰ nurkkaan
		itsToolBox->SetTextAlignment(kLeft);
		std::string titleStr(::GetDictionaryString("TempViewHodographTitle"));
		NFmiText titleTxt(hodografRect.TopLeft(), titleStr, false, 0, itsDrawingEnvironment);
		itsToolBox->Convert(&titleTxt);

		// piirr‰ apu ympyr‰t haalealla v‰rill‰
		itsDrawingEnvironment->SetFrameColor(NFmiColor(0.85f, 0.85f, 0.85f));
		itsDrawingEnvironment->DisableFill();
		auto hodografScaleMaxValue = hodografViewData.ScaleMaxValue();
		for(double x = 10; x <= hodografScaleMaxValue; x += 10)
		{
			NFmiPoint relP1(GetRelativePointFromHodograf(-x, -x));
			NFmiPoint relP2(GetRelativePointFromHodograf(x, x));
			itsToolBox->DrawEllipse(NFmiRect(relP1, relP2), itsDrawingEnvironment);
		}

		itsDrawingEnvironment->SetFrameColor(NFmiColor(0, 0, 0));
		// piirr‰ asteikot
		NFmiPoint leftCenter(hodografRect.Left(), hodografRect.Center().Y());
		NFmiPoint rightCenter(hodografRect.Right(), hodografRect.Center().Y());
		NFmiPoint centerTop(hodografRect.Center().X(), hodografRect.Top());
		NFmiPoint centerBottom(hodografRect.Center().X(), hodografRect.Bottom());

		NFmiLine line1(leftCenter, rightCenter, 0, itsDrawingEnvironment);
		itsToolBox->Convert(&line1);
		NFmiLine line2(centerTop, centerBottom, 0, itsDrawingEnvironment);
		itsToolBox->Convert(&line2);

		double tickHeightRel = ConvertFixedPixelSizeToRelativeHeight(3);
		double tickWidthRel = ConvertFixedPixelSizeToRelativeWidth(3);

		itsToolBox->SetTextAlignment(kCenter);
		for(double x = -hodografScaleMaxValue; x <= hodografScaleMaxValue; x += 10)
		{ // piirret‰‰n u-akselin sakarat
			NFmiPoint relP(GetRelativePointFromHodograf(x, 0));
			NFmiPoint uP1(relP.X(), relP.Y() - tickHeightRel);
			NFmiPoint uP2(relP.X(), relP.Y() + tickHeightRel);
			NFmiLine lineU(uP1, uP2, 0, itsDrawingEnvironment);
			itsToolBox->Convert(&lineU);
			std::string str(NFmiStringTools::Convert<double>(x));
			NFmiText txt(uP2, str.c_str(), false, 0, itsDrawingEnvironment);
			itsToolBox->Convert(&txt);
		}
		itsToolBox->SetTextAlignment(kLeft);
		double yShift = itsToolBox->SY(fontSize / 2);
		for(double y = -hodografScaleMaxValue; y <= hodografScaleMaxValue; y += 10)
		{ // piirret‰‰n v-akselin sakarat
			NFmiPoint relP(GetRelativePointFromHodograf(0, y));
			NFmiPoint vP1(relP.X() - tickWidthRel, relP.Y());
			NFmiPoint vP2(relP.X() + tickWidthRel, relP.Y());
			NFmiLine lineV(vP1, vP2, 0, itsDrawingEnvironment);
			itsToolBox->Convert(&lineV);
			if(y != 0)
			{
				std::string str(NFmiStringTools::Convert<double>(y));
				NFmiPoint txtP(vP2.X(), vP2.Y() - yShift);
				NFmiText txt(txtP, str.c_str(), false, 0, itsDrawingEnvironment);
				itsToolBox->Convert(&txt);
			}
		}

		// pit‰‰ viel‰ piirt‰‰ laatikon reunat, koska ne ovat saattaneet sotkeentua
		NFmiRectangle rec2(hodografRect, 0, itsDrawingEnvironment);
		itsToolBox->Convert(&rec2);
	}
}

void NFmiTempView::DrawHodografCurve(NFmiSoundingData &theData, int theProducerIndex)
{
	// piirr‰ itse hodografi k‰yr‰
	NFmiColor soundingColor(itsCtrlViewDocumentInterface->GetMTATempSystem().SoundingColor(theProducerIndex));
	itsDrawingEnvironment->SetFrameColor(soundingColor); // itse tuuli k‰ppyr‰ piirret‰‰n luotauksen omalla v‰rill‰
	std::deque<float>&uV = theData.GetParamData(kFmiWindUMS);
	std::deque<float>&vV = theData.GetParamData(kFmiWindVMS);
	std::deque<float>&pV = theData.GetParamData(kFmiPressure);
	std::deque<float>&zV = theData.GetParamData(kFmiGeomHeight);

	// piirret‰‰n ensin k‰yr‰t
	// k‰yr‰t piirret‰‰n siten ett‰ ne feidataan originaali v‰rist‰ kohti vaalean harmaata, mit‰ korkeammalla k‰yr‰ on
	float fadeAwayFactor = 0.9f;
	float minPressureLevel = 200;
	float maxHeightLevel = 12000;
	NFmiColor fadeAwayColor(soundingColor.Red() + ((1-soundingColor.Red())*fadeAwayFactor), soundingColor.Green() + ((1-soundingColor.Green())*fadeAwayFactor), soundingColor.Blue() + ((1-soundingColor.Blue())*fadeAwayFactor));
	if(uV.size() > 0 && uV.size() == vV.size())
	{
		double currentU = kFloatMissing;
		double lastU = kFloatMissing;
		double currentV = kFloatMissing;
		double lastV = kFloatMissing;
		int ssize = static_cast<int>(uV.size());
		itsDrawingEnvironment->SetPenSize(NFmiPoint(2 * itsDrawSizeFactor.X(), 2 * itsDrawSizeFactor.Y()));
		for(int i=0; i<ssize;i++)
		{
			if(pV[i] != kFloatMissing && pV[i] < minPressureLevel) // ei piirret‰ hodografi k‰yr‰‰ ihan ylˆs asti
			{
				if(zV[i] != kFloatMissing && zV[i] > maxHeightLevel) // jos kus ei ole paine dataa, silloin tarkastellaan korkeus datasta, piirret‰‰nkˆ
					break;
			}
			if(uV[i] != kFloatMissing && vV[i] != kFloatMissing)
			{
				currentU = uV[i];
				currentV = vV[i];

				if(lastU != kFloatMissing && lastV != kFloatMissing)
				{
					NFmiPoint relP1(GetRelativePointFromHodograf(lastU, lastV));
					NFmiPoint relP2(GetRelativePointFromHodograf(currentU, currentV));

					NFmiColor tmpColor(soundingColor);
					tmpColor.Mix(fadeAwayColor, 1 - (pV[i] - minPressureLevel)/1000);
					itsDrawingEnvironment->SetFrameColor(tmpColor);
					NFmiLine lineUV(relP1, relP2, 0, itsDrawingEnvironment);
					itsToolBox->Convert(&lineUV);
				}
				lastU = currentU;
				lastV = currentV;
			}
		}
	}
	itsDrawingEnvironment->SetFrameColor(soundingColor);
}

void NFmiTempView::DrawHodografTextWithMarker(const std::string &theText, float u, float v, const NFmiColor &theTextColor, const NFmiColor &theMarkerColor, const NFmiColor &theMarkerFillColor, int theMarkerSizeInPixel, int theFontSize, FmiDirection theTextAlignment, MarkerShape theMarkerShape)
{
	NFmiDrawingEnvironment markEnvi; // laitetaan oma envi ympyr‰lle
	markEnvi.SetFrameColor(theMarkerColor);
	markEnvi.SetFillColor(theMarkerFillColor);
	markEnvi.EnableFill();
	NFmiDrawingEnvironment fontEnvi; // laitetaan oma envi tekstille
	fontEnvi.SetFrameColor(theTextColor);
	fontEnvi.SetFontSize(NFmiPoint(theFontSize * itsDrawSizeFactor.X(), theFontSize * itsDrawSizeFactor.Y()));
	itsToolBox->SetTextAlignment(theTextAlignment);

	double markWidth = ConvertFixedPixelSizeToRelativeWidth(theMarkerSizeInPixel);
	double markHeight = ConvertFixedPixelSizeToRelativeHeight(theMarkerSizeInPixel);
	NFmiRect markRect(0,0,markWidth,markHeight);
	if(u != kFloatMissing && v != kFloatMissing)
	{ // piirret‰‰n merkki
		NFmiPoint relP(GetRelativePointFromHodograf(u, v));
		if(theMarkerShape == kCircle) // 1=circle
		{
			markRect.Center(relP);
			itsToolBox->DrawEllipse(markRect, &markEnvi);
		}
		else if(theMarkerShape == kTriangle) // 2=triangle
		{
			NFmiPolyline markerPolyLine(itsRect, 0, &markEnvi);
			markerPolyLine.AddPoint(NFmiPoint(-1, 1));
			markerPolyLine.AddPoint(NFmiPoint(0, -1));
			markerPolyLine.AddPoint(NFmiPoint(1, 1));
			NFmiPoint scale(markWidth, markHeight);
			itsToolBox->DrawPolyline(&markerPolyLine, relP, scale);
		}
		else if(theMarkerShape == kCross) // 1=circle
		{
			markEnvi.SetPenSize(NFmiPoint(2,2));
			markRect.Center(relP);
			NFmiLine line1(markRect.BottomLeft(), markRect.TopRight(), 0, &markEnvi);
			itsToolBox->Convert(&line1);
			NFmiLine line2(markRect.TopLeft(), markRect.BottomRight(), 0, &markEnvi);
			itsToolBox->Convert(&line2);
		}
		NFmiText txtObject(relP, theText.c_str(), false, 0, &fontEnvi);
		itsToolBox->Convert(&txtObject);
	}
}

void NFmiTempView::DrawHodografHeightMarkers(NFmiSoundingData &theData, int theProducerIndex)
{
	// piirret‰‰n sitten halutut korkeudet hodografi k‰yr‰‰n
	// piirret‰‰n merkkipiste k‰yr‰‰n aina tiettyihin korkeuksiin (interpoloituna)
	std::deque<std::pair<float, int> > markValues; // integeri kertoo, miten label printataan, true=sellaisenaan ja false=1000 -> 1k
	markValues.push_back(std::make_pair(0.f, true));
	markValues.push_back(std::make_pair(1000.f, false));
	markValues.push_back(std::make_pair(3000.f, false));
	markValues.push_back(std::make_pair(6000.f, false));

	NFmiColor markFillColor(0,0,0);
	NFmiColor markFrameColor(0,0,0);
	NFmiColor textColor(itsCtrlViewDocumentInterface->GetMTATempSystem().SoundingColor(theProducerIndex));
	for(std::deque<std::pair<float, int> >::iterator markValuesIt = markValues.begin(); markValuesIt != markValues.end(); ++markValuesIt)
	{
		float h = (*markValuesIt).first;
		if((*markValuesIt).second == false)
			h += theData.ZeroHeight(); // nolla korkeus on lis‰tt‰v‰ haluttuun korkeuteen
		float u = kFloatMissing;
		float v = kFloatMissing;
		if(h == 0) // pinta arvo on erikois tapaus, sit‰ ei voi interpoloida, haetaan alin
		{
			if(!theData.GetLowestNonMissingValues(h, u, v))
				continue ;
		}
		else
		{
			u = theData.GetValueAtHeight(kFmiWindUMS, h);
			v = theData.GetValueAtHeight(kFmiWindVMS, h);
		}

		if(h != kFloatMissing)
		{ // piirret‰‰n korkeus merkki ja siirryt‰‰n odottamaan seuraavaa rajaa
			std::string heightStr((*markValuesIt).second ?
                NFmiStringTools::Convert<int>(boost::math::iround(h)) :
                NFmiStringTools::Convert<int>(boost::math::iround((*markValuesIt).first / 1000.f)) + "k");
			DrawHodografTextWithMarker(heightStr, u, v, textColor, markFrameColor, markFillColor, 5, 18, kLeft, kCircle);
		}
	}
}

bool NFmiTempView::CheckIsSoundingDataChanged(TotalSoundingData& theUsedData)
{
	return !itsSelectedProducerSoundingData.IsSameSounding(theUsedData);
}

NFmiTempView::SoundingDataEqual NFmiTempView::MakeSoundingDataEqual()
{
	auto& mtaSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
	return SoundingDataEqual{mtaSystem.IntegrationRangeInKm(), mtaSystem.IntegrationTimeOffset1InHours(), mtaSystem.IntegrationTimeOffset2InHours() };
}


bool NFmiTempView::SoundingDataEqual::operator==(const SoundingDataEqual& other) const
{
	std::string debugStr = "SoundingDataEqual::oper==: this ";
	debugStr += std::to_string(itsAvgRangeInKm);
	debugStr += ", ";
	debugStr += std::to_string(itsAvgTimeRange1Inhours);
	debugStr += ", ";
	debugStr += std::to_string(itsAvgTimeRange2Inhours);
	debugStr += " and other ";
	debugStr += std::to_string(other.itsAvgRangeInKm);
	debugStr += ", ";
	debugStr += std::to_string(other.itsAvgTimeRange1Inhours);
	debugStr += ", ";
	debugStr += std::to_string(other.itsAvgTimeRange2Inhours);
	CatLog::logMessage(debugStr, CatLog::Severity::Debug, CatLog::Category::Visualization);

	if(itsAvgRangeInKm != other.itsAvgRangeInKm)
		return false;
	if(itsAvgTimeRange1Inhours != other.itsAvgTimeRange1Inhours)
		return false;
	if(itsAvgTimeRange2Inhours != other.itsAvgTimeRange2Inhours)
		return false;
	return true;
}

void NFmiTempView::SetupUsedSoundingData(TotalSoundingData& theUsedDataInOut, int theProducerIndex, bool fMainCurve)
{
	// Valitun luotaus tuottajan data laitetaan talteen indeksi laskuja varten
	if(IsSelectedProducerIndex(theProducerIndex) && fMainCurve) 
	{
		// Laitetaan haettu luotausdata valituksi vain jos se on muuttunut
		if(fMustResetFirstSoundingData || CheckIsSoundingDataChanged(theUsedDataInOut))
		{
			fMustResetFirstSoundingData = false;
			// Jos ei ole muuttunut (aika/origtime/tuottaja/paikka), sit‰ on saatettu modifioida ja muutoksia ei heitet‰ pois
			itsSelectedProducerSoundingData = theUsedDataInOut; 
		}
		else
		{
			// Muuten pit‰‰ sijoittaa t‰llessa ollut valittutuottajadata piirrett‰v‰‰n dataan
			theUsedDataInOut = itsSelectedProducerSoundingData; 
		}
	}
}

void NFmiTempView::DrawSounding(TotalSoundingData &theUsedDataInOut, int theProducerIndex, const NFmiColor &theUsedSoundingColor, bool fMainCurve, bool onSouthernHemiSphere, bool isNewData)
{
	SetupUsedSoundingData(theUsedDataInOut, theProducerIndex, fMainCurve);
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    if(theProducerIndex >= mtaTempSystem.MaxTempsShowed())
		return ; // lopetetaan kun on piirretty maksi m‰‰r‰ luotauksia yhteen kuvaan

	NFmiDrawingEnvironment* envi = itsDrawingEnvironment;
	envi->SetPenSize(NFmiPoint(1 * itsDrawSizeFactor.X(), 1 * itsDrawSizeFactor.Y()));
	envi->SetFrameColor(theUsedSoundingColor);
	itsToolBox->UseClipping(true); // laitetaan clippaus taas p‰‰lle (huonoa koodia, mutta voi voi)

    DrawSecondaryData(theUsedDataInOut.itsSoundingData, theUsedSoundingColor, theUsedDataInOut.itsGroundLevelValue);

    if(fMainCurve)
        DrawHodograf(theUsedDataInOut.itsSoundingData, theProducerIndex);

	if(IsSelectedProducerIndex(theProducerIndex) && fMainCurve)
		DrawAllLiftedAirParcels(itsSelectedProducerSoundingData.itsSoundingData); // piirret‰‰n ilmapakettien nostot vain valitun tuottajan luotaukseen

	// HUOM! ensin piirret‰‰n kastepiste ja sitten l‰mpˆtila, koska j‰lkimm‰isen‰ piirretty
	// voi peitt‰‰ allleen toisen piirrot ja koska l‰mpˆtila on t‰rke‰mpi, sen pit‰‰ tulla pintaan.
	// P‰‰lle piirto tarkoittaa mm. ett‰ katkoviivat peitt‰v‰t alleen yhten‰isen viivan!!

	envi->SetFrameColor(theUsedSoundingColor);
	itsToolBox->UseClipping(true); // laitetaan clippaus taas p‰‰lle (huonoa koodia, mutta voi voi)

	{
        NFmiTempLineInfo lineInfo = mtaTempSystem.DewPointLineInfo();
		lineInfo.Color(theUsedSoundingColor);
		lineInfo.Thickness(boost::math::iround(lineInfo.Thickness() * itsDrawSizeFactor.X() * ExtraPrintLineThicknesFactor(true)));
		DrawTemperatures(theUsedDataInOut.itsSoundingData, kFmiDewPoint, lineInfo, theUsedDataInOut.itsGroundLevelValue);
	}

	{
        NFmiTempLineInfo lineInfo = mtaTempSystem.TemperatureLineInfo();
		lineInfo.Color(theUsedSoundingColor);
		lineInfo.Thickness(boost::math::iround(lineInfo.Thickness() * itsDrawSizeFactor.X() * ExtraPrintLineThicknesFactor(true)));
		DrawTemperatures(theUsedDataInOut.itsSoundingData, kFmiTemperature, lineInfo, theUsedDataInOut.itsGroundLevelValue);
	}

	// Draw height values
	if(fMainCurve)
		DrawHeightValues(theUsedDataInOut.itsSoundingData, theProducerIndex, theUsedDataInOut.itsGroundLevelValue);

	// laitetaan takaisin 'solid' kyn‰
	envi->SetFillPattern(FMI_SOLID);
	envi->SetPenSize(NFmiPoint(1, 1));
	DrawWind(theUsedDataInOut.itsSoundingData, theProducerIndex, onSouthernHemiSphere, theUsedDataInOut.itsGroundLevelValue);

	if(fMainCurve)
	{
		DrawLCL(theUsedDataInOut.itsSoundingData, theProducerIndex, kLCLCalcSurface);
		DrawLCL(theUsedDataInOut.itsSoundingData, theProducerIndex, kLCLCalc500m2);
		DrawLCL(theUsedDataInOut.itsSoundingData, theProducerIndex, kLCLCalcMostUnstable);
		DrawTrMw(theUsedDataInOut.itsSoundingData, theProducerIndex);
	}
	if(IsSelectedProducerIndex(theProducerIndex) && fMainCurve && mtaTempSystem.ShowCondensationTrailProbabilityLines())
		DrawCondensationTrailRHValues(theUsedDataInOut.itsSoundingData, 400, 200, 0.32); // 0.32 on viimeinen apuviiva mik‰ piirret‰‰n, arvot laitetaan sen oikealle puolelle

	if(fMainCurve)
		DrawStationInfo(theUsedDataInOut, theProducerIndex, isNewData);
}

static void AddStringLabelData(const PointF &thePoint, const PointF &theOffsetPoint, const std::string &theStr, vector<LineLabelDrawData> &theLabels)
{
	LineLabelDrawData tmp;
	tmp.itsPlace = thePoint;
	tmp.itsPlace = tmp.itsPlace + theOffsetPoint;
	tmp.itsText = theStr;
	theLabels.push_back(tmp);
}

// Piirret‰‰n luotauksen Tropopaussi merkki ja maksimi tuuli
void NFmiTempView::DrawTrMw(NFmiSoundingData &theData, int theProducerIndex)
{
	std::vector<LineLabelDrawData> lineLabels;
    NFmiTempLabelInfo labelInfo(NFmiPoint(4, 0), kLeft, 12, true, true);
	labelInfo.FontSize(boost::math::iround(labelInfo.FontSize() * itsDrawSizeFactor.Y()));
	Gdiplus::PointF moveLabelInPixels(static_cast<REAL>(labelInfo.StartPointPixelOffSet().X() * itsDrawSizeFactor.X()), static_cast<REAL>(labelInfo.StartPointPixelOffSet().Y() * itsDrawSizeFactor.Y()));

	// 1. Etsi minimi l‰mpˆtila ja sen paine
	double minT = kFloatMissing;
	double minTpressure = kFloatMissing;
	if(theData.GetTrValues(minT, minTpressure))
	{
		// 2. Laita TR merkki piirto listaan
		double x = pt2x(minTpressure, minT);
		double y = p2y(minTpressure);
		::AddStringLabelData(PointF(static_cast<REAL>(x * itsGdiplusScale.X()), static_cast<REAL>(y * itsGdiplusScale.Y())), moveLabelInPixels, "TR", lineLabels);
	}
	// 3. Etsi maksimi tuuli ja sen l‰mpˆtila ja paine (l‰mpˆtilasta saadaan WS -merkin X-sijainti)
	double maxWS = kFloatMissing;
	double maxWSPressure = kFloatMissing;
	if(theData.GetMwValues(maxWS, maxWSPressure))
	{
		double temperatureAtMaxWs = theData.GetValueAtPressure(kFmiTemperature, static_cast<float>(maxWSPressure));
		if(temperatureAtMaxWs == kFloatMissing)
			temperatureAtMaxWs = minT; // kokeillaan piirt‰‰ edes minT kohtaan merkki
		// 4. Laita MW merkki piirto listaan
		double x = pt2x(maxWSPressure, temperatureAtMaxWs);
		double y = p2y(maxWSPressure);
		::AddStringLabelData(PointF(static_cast<REAL>(x * itsGdiplusScale.X()), static_cast<REAL>(y * itsGdiplusScale.Y())), moveLabelInPixels, "MW", lineLabels);
	}

	// 4. Piirr‰ TR ja MW merkki
	::DrawGdiplusStringVector(*itsGdiPlusGraphics, lineLabels, labelInfo, CtrlView::Relative2GdiplusRect(itsToolBox, itsTempViewDataRects.getSoundingCurveDataRect()), itsCtrlViewDocumentInterface->GetMTATempSystem().SoundingColor(theProducerIndex));
}

void NFmiTempView::DrawLCL(NFmiSoundingData &theData, int theProducerIndex, FmiLCLCalcType theLCLCalcType)
{
	// piirret‰‰n vain valitun tuottajan luotaukseen
	if(IsSelectedProducerIndex(theProducerIndex))
	{
		NFmiMTATempSystem& mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
		const NFmiTempLineInfo& lineInfo = ::GetAirParcelLineInfo(mtaTempSystem, theLCLCalcType);
		const NFmiTempLabelInfo& labelInfo = ::GetAirParcelLabelInfo(mtaTempSystem, theLCLCalcType);
		if(labelInfo.DrawLabelText())
		{
			int trueLineWidth = 1;
			bool drawSpecialLines = SetHelpLineDrawingAttributes(itsToolBox, itsDrawingEnvironment, labelInfo, lineInfo, trueLineWidth, true);

			double pLCL = theData.CalcLCLPressureLevel(theLCLCalcType);
			if(pLCL != kFloatMissing)
			{
				float T = theData.GetValueAtPressure(kFmiTemperature, static_cast<float>(pLCL));
				if(T != kFloatMissing)
				{
					double x = pt2x(pLCL, T);
					double y = p2y(pLCL);
					double lclLineLength = ConvertFixedPixelSizeToRelativeWidth(30);
					NFmiPoint p1(x - lclLineLength / 2., y);
					NFmiPoint p2(x + lclLineLength / 2., y);
					NFmiPoint p3(x + lclLineLength / 1.8, y);
					p3.Y(p3.Y() - itsToolBox->SY(static_cast<long>(itsDrawingEnvironment->GetFontHeight() / 2.)));
					DrawLine(p1, p2, drawSpecialLines, trueLineWidth, false, 0, itsDrawingEnvironment);
					std::string str("LCL (");
					str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(pLCL, 0);
					str += ", ";
					str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(T, 1);
					str += ")";
					NFmiText txt(p3, str, false, 0, itsDrawingEnvironment);
					itsToolBox->Convert(&txt);
				}
			}
		}
	}
}

// piirret‰‰n paine asteikon viereen luotauksesta korkeus arvoja
void NFmiTempView::DrawHeightValues(NFmiSoundingData &theData, int theProducerIndex, const NFmiGroundLevelValue& groundLevelValue)
{
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    if(mtaTempSystem.HeightValueLabelInfo().DrawLabelText() == false)
		return ;
    NFmiTempLabelInfo labelInfo = mtaTempSystem.HeightValueLabelInfo();
	labelInfo.FontSize(boost::math::iround(labelInfo.FontSize() * itsDrawSizeFactor.Y()));
	labelInfo.StartPointPixelOffSet(ScaleOffsetPoint(labelInfo.StartPointPixelOffSet()));
	int trueLineWidth = boost::math::iround(1 * itsDrawSizeFactor.X()); // t‰m‰ on sin‰ns‰ turha, koodi kopioitu toisaalta, miss‰ piirret‰‰n myˆs apuviivoja
	// t‰ss‰kin on turhaa juttua, koska mit‰‰n viivoje ei piirret‰, mutta t‰m‰ asettaa tietyt piirto-ominaisuudet kohdalleen
	// Huom! k‰ytet‰‰n tahallaan MoistAdiabaticLineInfo:a, koska korkeus jutulla ei ole omia viiva asetuksia
	SetHelpLineDrawingAttributes(itsToolBox, itsDrawingEnvironment, labelInfo, mtaTempSystem.MoistAdiabaticLineInfo(), trueLineWidth, false);
	auto baseColor = itsDrawingEnvironment->GetFrameColor();
	NFmiPoint moveLabelRelatively(CalcReltiveMoveFromPixels(itsToolBox, labelInfo.StartPointPixelOffSet()));

	if(mtaTempSystem.DrawOnlyHeightValuesOfFirstDrawedSounding() && IsSelectedProducerIndex(theProducerIndex) || mtaTempSystem.DrawOnlyHeightValuesOfFirstDrawedSounding() == false)
	{
		std::deque<float> &geoms = theData.GetParamData(kFmiGeomHeight);
		std::deque<float> &pressures = theData.GetParamData(kFmiPressure);
		bool useHeight = (theData.PressureDataAvailable() == false && theData.HeightDataAvailable() == true);
		if(geoms.size() > 1 && geoms.size() == pressures.size())
		{
			itsToolBox->UseClipping(labelInfo.ClipWithDataRect());
			double neededPrintedTextYDiff = itsToolBox->SY(static_cast<long>(labelInfo.FontSize())) * 1.5; // ei haluta prittailla korkeus tekstej‰ p‰‰llekk‰in, joten pit‰‰ odottaa
																								// joskus ett‰ tulee tarpeeksi v‰li‰ ennen kuin uusi korkeus teksti printataan
			double lastPrintedTextY = 25; // 25 on vain iso luku, joka korvataan 1. kierroksella, jolloin teksti‰ piirret‰‰n ruutuun
			int ssize = static_cast<int>(geoms.size());
			for(int i=0;i<ssize; i++)
			{
				float gValue = geoms[i];
				float pValue = pressures[i];
				if(gValue != kFloatMissing && (useHeight || pValue != kFloatMissing))
				{
					double currentTextY = (useHeight ? h2y(gValue) : p2y(pValue)) + moveLabelRelatively.Y();
					double currentTextX = itsTempViewDataRects.getSoundingCurveDataRect().Left() + moveLabelRelatively.X();
					if(::fabs(currentTextY-lastPrintedTextY) > neededPrintedTextYDiff)
					{
						NFmiPoint p(currentTextX, currentTextY);
						std::string str(NFmiStringTools::Convert<int>(static_cast<int>(gValue)));
						str += "m";
						auto usedColor = ::MakeUndergroundColorSetup(baseColor, groundLevelValue, pValue);
						itsDrawingEnvironment->SetFrameColor(usedColor);
						NFmiText txt(p, str, false, 0, itsDrawingEnvironment);
						itsToolBox->Convert(&txt);
						lastPrintedTextY = currentTextY;
					}
				}
			}
			itsToolBox->UseClipping(true); // clippaus laitettava uudestaan p‰‰lle
		}
	}
	itsDrawingEnvironment->SetFrameColor(baseColor);
}

NFmiPoint NFmiTempView::CalcStringRelativeSize(const std::string &str, double fontSize, const std::string& fontName)
{
	auto fontNameWide = CtrlView::StringToWString(fontName);
	auto usedFont = CtrlView::CreateFontPtr(static_cast<float>(fontSize), fontNameWide, Gdiplus::FontStyleRegular);
	Gdiplus::PointF oringinInPixels(0, 0);
	auto stringBoundingRectInPixels = CtrlView::GetStringBoundingBox(*itsGdiPlusGraphics, str, oringinInPixels, *usedFont);
	return CtrlView::GdiplusRect2Relative(itsToolBox, stringBoundingRectInPixels).Size();
}

const std::string gMaxLegendLocationNameText("1: E00/05.12 62∞23'N, 25∞41'E    ");

static std::string MakeLegendStringCorrectLength(std::string legendLineStr)
{
	for(auto i = legendLineStr.size(); i < gMaxLegendLocationNameText.size(); i++)
		legendLineStr += " "; // t‰ytet‰‰n nime‰ spaceilla, ett‰ pohjav‰ritys peitt‰‰ saman alan jokaiselle legendan riville
	return legendLineStr;
}

static std::string MakeLegendLocationNameStr(NFmiSoundingData& theData, int theProducerIndex, bool isNewData, bool allowHighlights)
{
	std::string locationNameStr = std::to_string(theProducerIndex + 1);
	locationNameStr += ":";
	if(allowHighlights && isNewData)
		locationNameStr += " *";
	else
		locationNameStr += " ";
	locationNameStr += ::GetNameText(theData.Location(), theData.MovingSounding());
	return ::MakeLegendStringCorrectLength(locationNameStr);
}

void NFmiTempView::SetupLegendDrawingEnvironment()
{
	itsLegendDrawingSetup = LegendDrawingSetup();
	NFmiMTATempSystem& mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
	// laitetaan piirto ominaisuudet p‰‰lle
	itsLegendDrawingSetup.oldTextAlignment = itsToolBox->GetTextAlignment();
	// kLeft + 1000 on ik‰v‰ toolbox pikaviritys, joka laittaa tekstin alle 
	// pohjav‰ri‰, jota tarvitaan luotaus legendan kirjoittamisessa.
	itsToolBox->SetTextAlignment(static_cast<FmiDirection>(kLeft + 1000));
	NFmiPoint fontSize = mtaTempSystem.LegendTextSize();
	fontSize = ScaleOffsetPoint(fontSize);
	itsDrawingEnvironment->SetFontSize(fontSize);
	itsToolBox->ConvertEnvironment(itsDrawingEnvironment);
	NFmiText measuredText(NFmiPoint(-9999, -9999), gMaxLegendLocationNameText, true, 0, itsDrawingEnvironment);
	auto relativeTextSize = itsToolBox->MeasureTextCorrect(measuredText);
	itsLegendDrawingSetup.maxNameLengthRelative = relativeTextSize.X();
	itsLegendDrawingSetup.textLineHeightRelative = relativeTextSize.Y() * 0.99;
}

NFmiPoint NFmiTempView::CalcLegendTextStartPoint()
{
	const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
	NFmiPoint point = dataRect.TopRight();
	point.X(point.X() - itsLegendDrawingSetup.maxNameLengthRelative * 1.35);
	point.Y(point.Y() + itsLegendDrawingSetup.textLineHeightRelative * 0.);
	point.X(std::max(point.X(), dataRect.Left()));
	return point;
}

const NFmiColor gNormalBackGroundTextColor(0.86f, 0.86f, 0.86f);
const NFmiColor gSelectedBackGroundTextColor(0.92f, 0.92f, 0.92f);

void NFmiTempView::DrawSelectedProducerIndexText(const NFmiPoint& textPoint)
{
	std::string selectedProducerIndexText = ::GetDictionaryString("Producer ind:");
	selectedProducerIndexText += std::to_string(itsCtrlViewDocumentInterface->GetMTATempSystem().GetSelectedProducerIndex(true) + 1);
	selectedProducerIndexText += " (CTRL+SHIFT+wheel)";
	AddLegendLineData(selectedProducerIndexText, GetSelectedProducersColor(), gNormalBackGroundTextColor, true);
}

void NFmiTempView::AddLegendLineData(const std::string& text, const NFmiColor& textColor, const NFmiColor& backgroundColor, bool doHorizontalLineSeparator)
{
	itsLegendDrawingLineData.push_back(LegendDrawingLineData{ text, textColor, backgroundColor, doHorizontalLineSeparator });
}

void NFmiTempView::DrawStationInfo(TotalSoundingData& theData, int theProducerIndex, bool isNewData)
{
	NFmiMTATempSystem& mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
	if(!mtaTempSystem.DrawLegendText())
		return;

	SetupLegendDrawingEnvironment();
	bool allowHighlights = !itsCtrlViewDocumentInterface->BetaProductGenerationRunning();
	auto locationNameStr = ::MakeLegendLocationNameStr(theData.itsSoundingData, theProducerIndex, isNewData, allowHighlights);

	NFmiPoint point = CalcLegendTextStartPoint();
	point.Y(point.Y() + (2.0 * theProducerIndex * itsLegendDrawingSetup.textLineHeightRelative));
	if(theProducerIndex == 0)
	{
		DrawSelectedProducerIndexText(point);
	}

	// Aseman aika tiedot piirret‰‰n p‰‰lle
	NFmiMetTime time(theData.itsSoundingData.Time());
	std::string timestr(time.ToStr(::GetDictionaryString("TempViewLegendTimeFormat")));
	timestr = ::MakeLegendStringCorrectLength(timestr);
	auto textColor = mtaTempSystem.SoundingColor(theProducerIndex);
	auto backgroundColor = IsSelectedProducerIndex(theProducerIndex) ? gSelectedBackGroundTextColor : gNormalBackGroundTextColor;
	AddLegendLineData(timestr, textColor, backgroundColor, false);
	// Aseman nimi ja paikka tiedot laitetaan sen alle
	AddLegendLineData(locationNameStr, textColor, backgroundColor, false);
	AddPossibleAvgIntegrationInfo(theData, textColor, backgroundColor);
}

static std::string MakePossibleAvgIntegrationInfo(TotalSoundingData& theData)
{
	if(theData.HasAvgIntegrationData())
	{
		std::string integrationStr = "[Avg from ";
		integrationStr += std::to_string(theData.itsIntegrationPointsCount);
		integrationStr += " pts and ";
		integrationStr += std::to_string(theData.itsIntegrationTimesCount);
		integrationStr += " times]";
		return integrationStr;
	}
	else
		return "";
}

void NFmiTempView::AddPossibleAvgIntegrationInfo(TotalSoundingData& theData, const NFmiColor& textColor, const NFmiColor& backgroundColor)
{
	auto possibleAvgIntegrationInfoStr = ::MakePossibleAvgIntegrationInfo(theData);
	if(!possibleAvgIntegrationInfoStr.empty())
	{
		possibleAvgIntegrationInfoStr = ::MakeLegendStringCorrectLength(possibleAvgIntegrationInfoStr);
		AddLegendLineData(possibleAvgIntegrationInfoStr, textColor, backgroundColor, false);
	}
}

void NFmiTempView::DrawLegendLineData()
{
	std::string newDataSearchText = ": ";
	newDataSearchText += CtrlViewUtils::ParameterStringHighlightCharacter;

	SetupLegendDrawingEnvironment();
	NFmiPoint point = CalcLegendTextStartPoint();
	for(const auto& legendLineData : itsLegendDrawingLineData)
	{
		itsDrawingEnvironment->SetFrameColor(legendLineData.itsTextColor);
		itsDrawingEnvironment->SetFillColor(legendLineData.itsBackgroundColor);
		if(legendLineData.itsText.find(newDataSearchText) != std::string::npos)
		{
			itsDrawingEnvironment->BoldFont(true);
		}
		NFmiText text1(point, legendLineData.itsText, true, 0, itsDrawingEnvironment);
		itsToolBox->Convert(&text1);
		if(legendLineData.fDoHorizontalLineSeparator)
		{
			DrawLegendLineDataSeparator(point);
		}
		point.Y(point.Y() + itsLegendDrawingSetup.textLineHeightRelative);
		itsDrawingEnvironment->BoldFont(false);
	}
}

void NFmiTempView::DrawLegendLineDataSeparator(const NFmiPoint &textPoint)
{
	auto relativeLeftEdge = textPoint.X() - itsToolBox->SX(1);
	auto relativeRightEdge = relativeLeftEdge + itsLegendDrawingSetup.maxNameLengthRelative + itsToolBox->SX(2);
	NFmiDrawingEnvironment separatorLineEnvi;
	NFmiPoint point1(relativeLeftEdge, textPoint.Y() + itsLegendDrawingSetup.textLineHeightRelative - itsToolBox->SY(1));
	NFmiPoint point2(relativeRightEdge, point1.Y());
	NFmiLine separatorLine(point1, point2, 0, &separatorLineEnvi);
	itsToolBox->Convert(&separatorLine);
}

static NFmiRect gMissingRect(0,0,0,0);

void NFmiTempView::DrawWind(NFmiSoundingData& theData, int theProducerIndex, bool onSouthernHemiSphere, const NFmiGroundLevelValue& groundLevelValue)
{
	itsFirstSoundinWindBarbXPos = kFloatMissing;
	NFmiMTATempSystem& mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
	if(!mtaTempSystem.DrawWinds())
		return;
	NFmiDrawingEnvironment envi(*itsDrawingEnvironment);
	auto baseColor = envi.GetFrameColor();
	double usedPenSize = 2.0;
	if(itsToolBox->GetDC()->IsPrinting())
		usedPenSize = 1.3; // tehd‰‰n printtausta varten v‰h‰n ohuempaa viivaa
	envi.SetPenSize(NFmiPoint(usedPenSize * itsDrawSizeFactor.X(), usedPenSize * itsDrawSizeFactor.Y())); // tehd‰‰n v‰h‰n paksumpaa viivaa, ett‰ n‰kyy paremmin
	auto basePenSize = envi.GetPenSize();
	std::deque<float>& wsValues = theData.GetParamData(kFmiWindSpeedMS);
	std::deque<float>& wdValues = theData.GetParamData(kFmiWindDirection);
	std::deque<float>& pressures = theData.GetParamData(kFmiPressure);
	std::deque<float>& heights = theData.GetParamData(kFmiGeomHeight);
	bool useHeight = (theData.PressureDataAvailable() == false && theData.HeightDataAvailable() == true);
	if(wsValues.size() > 1 && wsValues.size() == wdValues.size() && wsValues.size() == pressures.size() && wsValues.size() == heights.size())
	{
		int spaceOutFactor = itsCtrlViewDocumentInterface->SoundingViewWindBarbSpaceOutFactor();
		NFmiRect lastDrawnRect(gMissingRect);
		NFmiPoint point;
		NFmiPoint windVecSizeInPixels = mtaTempSystem.WindvectorSizeInPixels();
		windVecSizeInPixels = ScaleOffsetPoint(windVecSizeInPixels);
		NFmiRect windBarbRect(::CalcGeneralWindBarbRect(itsToolBox, windVecSizeInPixels));
		double xPos = ::CalcWindBarbXPos(itsTempViewDataRects.getSoundingCurveDataRect(), windBarbRect, theProducerIndex);
		if(IsSelectedProducerIndex(theProducerIndex))
			itsFirstSoundinWindBarbXPos = xPos;
		int ssize = static_cast<int>(wsValues.size());
		for(int i = 0; i < ssize; i++)
		{
			float ws = wsValues[i];
			float wd = wdValues[i];
			float p = pressures[i];
			float h = heights[i];
			if((useHeight ? h != kFloatMissing : p != kFloatMissing) && ws != kFloatMissing && wd != kFloatMissing)
			{
				// x suunnassa pit‰‰ siirt‰‰ tuulivektoreita aina oikealle p‰in, ett‰ ne eiv‰t ole toistensa p‰‰ll‰
				point = NFmiPoint(xPos, useHeight ? h2y(h) : p2y(p));
				windBarbRect.Center(point);
				if(spaceOutFactor == 0 || lastDrawnRect == gMissingRect || lastDrawnRect.IsInside(windBarbRect.Center()) == false)
				{
					::MakeUndergroundColorSetup(baseColor, basePenSize, groundLevelValue, p, &envi, true, true, true);
					NFmiWindBarb(ws
						, wd
						, windBarbRect
						, itsToolBox
						, onSouthernHemiSphere
						, 0.3f // ???
						, 0.3f // ???
						, 0
						, &envi).Build();
					lastDrawnRect = windBarbRect;
					if(spaceOutFactor == 1)
					{ // 1:ll‰ ei harvenneta niin paljoa, pienenet‰‰n testi recti‰
						NFmiPoint aSize(lastDrawnRect.Size());
						aSize *= NFmiPoint(0.5, 0.5);
						lastDrawnRect.Size(aSize);
						lastDrawnRect.Center(windBarbRect.Center());

					}
				}
			}
		}
	}
}

// piirt‰‰ sek‰ l‰mpp‰ri ett‰ kastepiste viivat
// HUOM! t‰nne tullessa theLineInfo on jo skaalattu printtauksen suhteen jos tarpeen
void NFmiTempView::DrawTemperatures(NFmiSoundingData &theData, FmiParameterName theParId, const NFmiTempLineInfo &theLineInfo, const NFmiGroundLevelValue& groundLevelValue)
{
	if(theLineInfo.DrawLine() == false)
		return ;
 // GDI+ piirto koodia
	itsGdiPlusGraphics->SetClip(CtrlView::Relative2GdiplusRect(itsToolBox, itsTempViewDataRects.getSoundingCurveDataRect()));

	std::deque<float> &temperatures = theData.GetParamData(theParId);
	std::deque<float> &pressures = theData.GetParamData(kFmiPressure);
	std::deque<float> &heights = theData.GetParamData(kFmiGeomHeight);
	bool useHeight = (theData.PressureDataAvailable() == false && theData.HeightDataAvailable() == true);
	if(temperatures.size() > 1 && temperatures.size() == pressures.size() && temperatures.size() == heights.size())
	{
		int maxMissingValues = theData.ObservationData() ? 15 : 1; // jos per‰kk‰in puuttuu enemm‰n kuin n‰in monta arvoa, ei yhdistet‰ viivoja, vaan katkaistaan
		int consecutiveMissingValues = 0;
		int ssize = static_cast<int>(temperatures.size());
		std::vector<PointF> points;
		std::vector<PointF> undergroundPoints;
		// Havaittujen luotausten piirrossa halutaan piirt‰‰ yhten‰ist‰ viivaa, vaikka datassa olisikin pieni‰ aukkoja, 
		// sit‰ varten k‰ytet‰‰n (consecutiveMissingValues < maxMissingValues) -tarkastelua.
		// temp2qd-filtteri tuottaa ilmeisesti luotauksia k‰‰nteisess‰ j‰rjestyksess‰ kuin meid‰n kotoisten luotauksien levelit.
		// Nyt k‰vi niin ett‰ Latvialaisten luotauksissa oli jossain tapauksissa ensin n. 100 kpl puuttuvia, sitten vasta tuli
		// leveleit‰ joille lˆytyi arvoja. T‰llˆin ohitettiin aina 1. leveli ja jatkettiin vasta toisesta ei-missing arvoja lˆytyneest‰.
		// Tein doingFirstTimeChecks -muuttujan t‰t‰ tapausta varten.
		bool doingFirstTimeChecks = true; 
		for(int i=0; i<ssize; i++)
		{
			float t = temperatures[i];
			float p = pressures[i];
			float h = heights[i];
			if(t != kFloatMissing && (useHeight ? (h != kFloatMissing) : (p != kFloatMissing)))
			{
				if(consecutiveMissingValues < maxMissingValues || doingFirstTimeChecks)
				{
					if(useHeight) // pit‰‰ muuttaa korkeudet paineiksi
						p = static_cast<float>(::CalcPressureAtHeight(h/1000.));
                    if(p <= 1)
                        break; // Jos ollaan tultu ilmakeh‰n yl‰rajoille, lopetetaan loopitus
					double x = pt2x(p, t);
					double y = p2y(p);
					if(groundLevelValue.IsBelowGroundLevelCase(p))
						undergroundPoints.push_back(PointF(static_cast<REAL>(x * itsGdiplusScale.X()), static_cast<REAL>(y * itsGdiplusScale.Y())));
					else
						points.push_back(PointF(static_cast<REAL>(x * itsGdiplusScale.X()), static_cast<REAL>(y * itsGdiplusScale.Y())));
					doingFirstTimeChecks = false;
				}
				else // piirret‰‰n p‰tk‰ mik‰ on vektorissa tallessa
				{
					::DrawCurveWithPossibleUndergroundSection(*itsGdiPlusGraphics, points, undergroundPoints, theLineInfo, GetUsedCurveDrawSmoothingMode(), IsPrinting(), false);
				}
			}

			if(t != kFloatMissing && (useHeight ? h != kFloatMissing : p != kFloatMissing))
				consecutiveMissingValues = 0;
			else
				consecutiveMissingValues++;
		}
		// lopuksi viel‰ piirret‰‰n loputkin mit‰ on piirrett‰v‰‰
		::DrawCurveWithPossibleUndergroundSection(*itsGdiPlusGraphics, points, undergroundPoints, theLineInfo, GetUsedCurveDrawSmoothingMode(), IsPrinting(), false);
	}
	itsGdiPlusGraphics->ResetClip();
}

bool NFmiTempView::LeftButtonUp(const NFmiPoint &thePlace, unsigned long theKey)
{
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    mtaTempSystem.LeftMouseDown(false);
	const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
	if(dataRect.Bottom() < thePlace.Y() && dataRect.Left() < thePlace.X())
	{ 
		// Nyt on klikattu l‰mpp‰ri asteikkoa
		double change = 5;
		if(dataRect.Center().X() > thePlace.X())
            mtaTempSystem.TAxisStart(mtaTempSystem.TAxisStart() - change);
		else
            mtaTempSystem.TAxisEnd(mtaTempSystem.TAxisEnd() - change);
		return true;
	}
	else if(dataRect.Left() > thePlace.X() && dataRect.Bottom() > thePlace.Y())
	{ 
		// Nyt on klikattu paine asteikkoa
		double change = 50.;
		if(dataRect.Center().Y() > thePlace.Y())
		{
			change = GetPAxisChangeValue(change); // yl‰p‰‰t‰ s‰‰dett‰ess‰ pit‰‰ laskeskella hieman
			double finalValue = round((mtaTempSystem.PAxisEnd() + change) / change) * change;
            mtaTempSystem.PAxisEnd(finalValue);
		}
		else
            mtaTempSystem.PAxisStart(mtaTempSystem.PAxisStart() + change);
		return true;
	}
	else if(dataRect.IsInside(thePlace))
	{
		if(mtaTempSystem.ShowHodograf())
		{ 
			// s‰‰det‰‰n hodografin center pistett‰
			mtaTempSystem.GetHodografViewData().SetCenter(thePlace);
			return true;
		}
		else
			return ModifySounding(itsSelectedProducerSoundingData.itsSoundingData, thePlace, theKey, kFmiDewPoint, 3);
	}
	else if(itsTempViewDataRects.getAnimationButtonRect().IsInside(thePlace))
	{
        mtaTempSystem.ChangeSoundingsInTime(kBackward);
		return true;
	}
	else if(itsTempViewDataRects.getAnimationStepButtonRect().IsInside(thePlace))
	{
        mtaTempSystem.PreviousAnimationStep();
		return true;
	}

	return false;
}

bool NFmiTempView::RightButtonUp(const NFmiPoint &thePlace, unsigned long theKey)
{
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    mtaTempSystem.RightMouseDown(false);
	const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
	if(dataRect.Bottom() < thePlace.Y() && dataRect.Left() < thePlace.X())
	{ 
		// Nyt on klikattu l‰mpp‰ri asteikkoa
		double change = 5;
		if(dataRect.Center().X() > thePlace.X())
            mtaTempSystem.TAxisStart(mtaTempSystem.TAxisStart() + change);
		else
            mtaTempSystem.TAxisEnd(mtaTempSystem.TAxisEnd() + change);
		return true;
	}
	else if(dataRect.Left() > thePlace.X() && dataRect.Bottom() > thePlace.Y())
	{ 
		// Nyt on klikattu paine asteikkoa
		double change = 50.;
		if(dataRect.Center().Y() > thePlace.Y())
		{
			change = GetPAxisChangeValue(change); // yl‰p‰‰t‰ s‰‰dett‰ess‰ pit‰‰ laskeskella hieman
			double finalValue = round((mtaTempSystem.PAxisEnd() - change) / change) * change;
            mtaTempSystem.PAxisEnd(finalValue);
		}
		else
            mtaTempSystem.PAxisStart(mtaTempSystem.PAxisStart() - change);
		return true;
	}
	else if(dataRect.IsInside(thePlace))
		return ModifySounding(itsSelectedProducerSoundingData.itsSoundingData, thePlace, theKey, kFmiTemperature, 3);
	else if(itsTempViewDataRects.getAnimationButtonRect().IsInside(thePlace))
	{
        mtaTempSystem.ChangeSoundingsInTime(kForward);
		return true;
	}
	else if(itsTempViewDataRects.getAnimationStepButtonRect().IsInside(thePlace))
	{
        mtaTempSystem.NextAnimationStep();
		return true;
	}
	return false;
}

bool NFmiTempView::ModifySounding(NFmiSoundingData &theSoundingData, const NFmiPoint &thePlace, unsigned long theKey, FmiParameterName theParam, int theDistToleranceInPixels)
{
	if(!itsCtrlViewDocumentInterface->GetMTATempSystem().ShowHodograf()) // ei muokkausta jos hodografi on n‰kyviss‰
	{
		double p = y2p(thePlace.Y());
		if(p != kFloatMissing)
		{
			double T = kFloatMissing;
			double Td = kFloatMissing;
			double pValue = kFloatMissing;
			if(theSoundingData.GetTandTdValuesFromNearestPressureLevel(p, pValue, T, Td))
			{
				if(theKey & kCtrlKey)
					return QuickModifySounding(theSoundingData, theParam, pValue, T, Td);
				else
				{
					double maxTolerance = itsToolBox->SY(theDistToleranceInPixels);
					double currentY = p2y(pValue);
					if(::fabs(thePlace.Y() - currentY) > maxTolerance)
						return false; // ei muokata, jos lˆydetty painepinta ei ollut tarpeeksi l‰hell‰

					double newT = xy2t(thePlace.X(), thePlace.Y());
					if(newT != kFloatMissing)
					{
						if(theParam == kFmiTemperature)
						{
							theSoundingData.SetValueToPressureLevel(static_cast<float>(pValue), static_cast<float>(newT), kFmiTemperature);
							if(newT < Td && Td != kFloatMissing) // jos l‰mpˆtila menee alle kastepisteen, pit‰‰ kastepiste s‰‰t‰‰ samaan arvoon
								theSoundingData.SetValueToPressureLevel(static_cast<float>(pValue), static_cast<float>(newT), kFmiDewPoint);
						}
						else if(theParam == kFmiDewPoint)
						{ // kastepiste ei saa menn‰ yli l‰mpˆtilan
							if(newT > T)
								newT = T;
							theSoundingData.SetValueToPressureLevel(static_cast<float>(pValue), static_cast<float>(newT), kFmiDewPoint);
						}
						return true;
					}
				}
			}
		}
	}
	return false;
}

// annetusta paineesta alasp‰in laskee kuiva-adiapaattisesti lasketun l‰mpˆtilan luotaukselle ja asettaa sen.
// Jos kyseess‰ kastepiste, muuttaa kastepisteen mixing ration mukaisesta annetusta paineesta alas.
bool NFmiTempView::QuickModifySounding(NFmiSoundingData &theSoundingData, FmiParameterName theParam, double P, double T, double Td)
{
	if(theParam == kFmiTemperature)
		return theSoundingData.ModifyT2DryAdiapaticBelowGivenP(P, T);
	else if(theParam == kFmiDewPoint)
		return theSoundingData.ModifyTd2MixingRatioBelowGivenP(P, T, Td);
	return false;
}

double NFmiTempView::GetPAxisChangeValue(double change)
{
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    if(mtaTempSystem.PAxisEnd() <= 100)
		change = 10;
	if(mtaTempSystem.PAxisEnd() <= 10)
		change = 1;
	return change;
}

void NFmiTempView::ResetScales(void)
{
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    mtaTempSystem.PAxisEnd(mtaTempSystem.ResetScalesEndP());
    mtaTempSystem.PAxisStart(mtaTempSystem.ResetScalesStartP());
	if(mtaTempSystem.SkewTDegree() == 45)
	{
        mtaTempSystem.TAxisStart(mtaTempSystem.ResetScalesSkewTStartT());
        mtaTempSystem.TAxisEnd(mtaTempSystem.ResetScalesSkewTEndT());
	}
	else
	{
        mtaTempSystem.TAxisStart(mtaTempSystem.ResetScalesStartT());
        mtaTempSystem.TAxisEnd(mtaTempSystem.ResetScalesEndT());
	}
}

void NFmiTempView::DrawAnimationControls(void)
{
	double moveX = ConvertFixedPixelSizeToRelativeWidth(2);
	NFmiDrawingEnvironment envi;
	envi.SetFontSize(NFmiPoint(18 * itsDrawSizeFactor.Y(), 18 * itsDrawSizeFactor.Y()));
	envi.EnableFill();
	envi.SetFrameColor(NFmiColor(0,0,0));
	envi.SetFillColor(NFmiColor(0.8f,0.8f,0.8f));

	const auto& animButtomRect = itsTempViewDataRects.getAnimationButtonRect();
	NFmiRectangle rec1(animButtomRect, 0, &envi);
	itsToolBox->Convert(&rec1);
	std::string str1("Anim.");
	NFmiText txt1(NFmiPoint(animButtomRect.TopLeft().X() + moveX, animButtomRect.TopLeft().Y()) , str1, false, 0, &envi);
	itsToolBox->Convert(&txt1);

	const auto& animStepButtomRect = itsTempViewDataRects.getAnimationStepButtonRect();
	NFmiRectangle rec2(animStepButtomRect, 0, &envi);
	itsToolBox->Convert(&rec2);
	std::string str2;
	int step = itsCtrlViewDocumentInterface->GetMTATempSystem().AnimationTimeStepInMinutes();
	if(step == 30)
		str2 += 'Ω';
	else if(step >= 60)
		str2 += NFmiStringTools::Convert<int>(step/60);
	str2 += "h";
	NFmiText txt2(NFmiPoint(animStepButtomRect.TopLeft().X() + moveX, animStepButtomRect.TopLeft().Y()), str2, false, 0, &envi);
	itsToolBox->Convert(&txt2);
}

#define KEYDOWN(vk_code) ((::GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)

void NFmiTempView::DrawOverBitmapThings(NFmiToolBox *theGTB, const NFmiPoint &thePlace)
{
	itsToolBox = theGTB;

	if(itsToolBox->GetDC()->IsPrinting())
		return ; // toistaiseksi mit‰‰n n‰ist‰ ei printata, koska ne liittyv‰t kursorin liikkumiseen luotausn‰ytˆn p‰‰ll‰

	itsDrawSizeFactor.Y(1.);
	itsDrawSizeFactor.X(1.);
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    itsDrawingEnvironment->SetFrameColor(GetSelectedProducersColor());
	int fontSize = itsTempViewDataRects.getStabilityIndexFontSize();
	itsDrawingEnvironment->SetFontSize(NFmiPoint(fontSize * itsDrawSizeFactor.X(), fontSize * itsDrawSizeFactor.Y()));
	itsToolBox->SetTextAlignment(kLeft);

	// 1. mik‰ on kursori kohdan P, T, Tpot (=dry), moistT ja W (=mixing ratio)
	double pressure = kFloatMissing;
	double temperature = kFloatMissing;
	double Tpot = kFloatMissing;
	double moistT = kFloatMissing;
	double W = kFloatMissing;
	const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
	if(dataRect.IsInside(thePlace))
	{
		pressure = y2p(thePlace.Y());
		temperature = xy2t(thePlace.X(), thePlace.Y());
		if(pressure != kFloatMissing && temperature != kFloatMissing)
		{
			Tpot = NFmiSoundingFunctions::T2tpot(temperature, pressure);
			moistT = NFmiSoundingFunctions::CalcMoistT(temperature, pressure);
			W = NFmiSoundingFunctions::FindNearestW(temperature, pressure);
		}
	}
	if(mtaTempSystem.GetSoundingViewSettingsFromWindowsRegisty().ShowStabilityIndexSideView())
	{ 
		// Jos ollaan indeksin‰ytto tilassa, lasketaan kursorin kohdalle arvoja ja laitetaan ne indeksi ikkunaan
		NFmiPoint p(CalcStabilityIndexStartPoint());

		NFmiText text(p, NFmiString(""), false, 0, itsDrawingEnvironment);
		auto lineH = itsStabilityIndexRelativeLineHeight;
		for(int i=0; i<4; i++)
			DrawNextLineToIndexView(lineH, text, "", p, true, false); // n‰it‰ ei lis‰t‰ soundingIndex-stringiin

		DrawNextLineToIndexView(lineH, text, GetIndexText(temperature, "T", 1), p, true, false);
		DrawNextLineToIndexView(lineH, text, GetIndexText(pressure, "P", 1), p, true, false);
		DrawNextLineToIndexView(lineH, text, GetIndexText(Tpot, "dry", 1), p, true, false);
		DrawNextLineToIndexView(lineH, text, GetIndexText(moistT, "moist", 1), p, true, false);
		DrawNextLineToIndexView(lineH, text, GetIndexText(W, "mix", 1), p, true, false);
	}

	if(dataRect.IsInside(thePlace))
	{
		// Jos pelkk‰ SHIFT pohjassa ja liikutellaan hiirt‰ luotausk‰yr‰ ikkunassa, piirret‰‰n apu viivoja (dry + moist adiapaatit ja mixing ratio)
		if(KEYDOWN(VK_SHIFT) && !KEYDOWN(VK_CONTROL))
		{
			// PIIRRETƒƒN LUOTAUS IKKUNAAN APUVIIVOJA KURSORIN KOHDALLE, JOS shift-NƒPPƒIN ON POHJASSA (apuviivat=kuiva, kostea ja mix)
            ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &dataRect);
			NFmiDrawingEnvironment envi;
			envi.SetFrameColor(NFmiColor(0,0.7f,0));
			envi.SetPenSize(NFmiPoint(2,2));

			if(pressure != kFloatMissing && temperature != kFloatMissing)
			{
				// 2. piirr‰ halutunlainen p‰tk‰ kuiva adiapaattia
				double deltap = 50;
				for (double p = pmax; p > 100; p-=deltap)
				{
					double x1 = Tpot2x(Tpot, p);
					double x2 = Tpot2x(Tpot, p - deltap);
					double y1 = p2y(p);
					double y2 = p2y(p-deltap);
					NFmiPoint p1(x1, y1);
					DrawLine(p1, NFmiPoint(x2, y2), false, 1, true, 0, &envi);
				}

				// 3. piirr‰ halutunlainen p‰tk‰ kostea adiapaattia
				if(moistT != kFloatMissing && moistT < 68) // moistT < ? on kokeellinen raja, koska laskut rajaht‰v‰t hieman kun menn‰‰n yli tietyn rajan
				{
					double P   = pmax;
					double TK  = moistT;
					double AOS = NFmiSoundingFunctions::OS(TK, 1000.);

					double ATSA  = NFmiSoundingFunctions::TSA(AOS, P);
					for(int J = 0; J < 125; J++)
					{
						double P0 = P;
						if(P <= 100)
							break;
						double T0 = ATSA;

						P = P - deltap;
						ATSA = NFmiSoundingFunctions::TSA(AOS, P);
			//			if(J > 0)
						{
							double newx0 = pt2x(P0, T0);  //; Find rotated temperature position
							double newx1 = pt2x(P, ATSA); //; Find rotated temperature position
							double y0 = p2y(P0);
							double y = p2y(P);
							NFmiPoint p1(newx0, y0);
							DrawLine(p1, NFmiPoint(newx1, y), false, 1, true, 0, &envi);
						}
					}
				}


				// 4. piirr‰ halutunlainen p‰tk‰ mix ratiota
				if(W != kFloatMissing)
				{
					for (double p = pmax; p > 100; p-=deltap)
					{
						double p2 = p - deltap;
						if(p-deltap < 100)
							p2 = 100;
						double t1 = NFmiSoundingFunctions::TMR(W, p);
						double t2 = NFmiSoundingFunctions::TMR(W, p2);
						double x1 = pt2x(p, t1);
						double x2 = pt2x(p2, t2);
						double y1 = p2y(p);
						double y2 = p2y(p2);
						NFmiPoint p1(x1, y1);
						DrawLine(p1, NFmiPoint(x2, y2), false, 1, true, 0, &envi);
					}
				}
			}
		}
	}
}

bool NFmiTempView::LeftButtonDown(const NFmiPoint &thePlace, unsigned long /* theKey */ )
{
	if(itsTempViewDataRects.getSoundingCurveDataRect().IsInside(thePlace)) // ainakin toistaiseksi pit‰‰ olla datarectiss‰, ett‰ capture l‰htee p‰‰lle
        itsCtrlViewDocumentInterface->GetMTATempSystem().LeftMouseDown(true);
	return false;
}

bool NFmiTempView::RightButtonDown(const NFmiPoint &thePlace, unsigned long /* theKey */ )
{
	if(itsTempViewDataRects.getSoundingCurveDataRect().IsInside(thePlace)) // ainakin toistaiseksi pit‰‰ olla datarectiss‰, ett‰ capture l‰htee p‰‰lle
        itsCtrlViewDocumentInterface->GetMTATempSystem().RightMouseDown(true);
	return false;
}

bool NFmiTempView::MouseMove(const NFmiPoint &thePlace, unsigned long  theKey)
{
	if(itsTempViewDataRects.getSoundingCurveDataRect().IsInside(thePlace))
	{
        NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
        if(mtaTempSystem.RightMouseDown()) // oikea nappi on tarke‰mpi koska sill‰ muokataan l‰mpp‰ri‰ (vasen muokkaa kastepistett‰)
			return ModifySounding(itsSelectedProducerSoundingData.itsSoundingData, thePlace, theKey, kFmiTemperature, 3);
		else if(mtaTempSystem.LeftMouseDown())
			return ModifySounding(itsSelectedProducerSoundingData.itsSoundingData, thePlace, theKey, kFmiDewPoint, 3);
	}
	return false;
}

static float GetFinalTooltipValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiParameterName theParId, float P, const NFmiPoint &theLatlon, const NFmiMetTime &usedTime)
{
    NFmiFastInfoUtils::MetaWindParamUsage metaWindParamUsage = NFmiFastInfoUtils::CheckMetaWindParamUsage(theInfo);
    if(metaWindParamUsage.ParamNeedsMetaCalculations(theParId))
    {
        // Kun tullaan t‰h‰n kohtaan, oletetaan ett‰ halutaan WS/WD parametreja, mutta datasta lˆytyy vain tuulen u ja v komponentit
        NFmiFastInfoUtils::QueryInfoParamStateRestorer restorer(*theInfo);
        theInfo->Param(kFmiWindUMS);
        float u = theInfo->PressureLevelValue(P, theLatlon, usedTime);
        theInfo->Param(kFmiWindVMS);
        float v = theInfo->PressureLevelValue(P, theLatlon, usedTime);
        if(theParId == kFmiWindSpeedMS)
            return NFmiFastInfoUtils::CalcWS(u, v);
        else if(theParId == kFmiWindDirection)
            return NFmiFastInfoUtils::CalcWD(u, v);
        else
            return kFloatMissing;
    }
    else
        return theInfo->PressureLevelValue(P, theLatlon, usedTime);
}

static std::string GetTooltipValueStr(const std::string &theParStr, NFmiSoundingData &soundingData, FmiParameterName theParId, int theMaxDecimalCount, float P, float heigthInMetersInStaAth)
{
	std::string str = theParStr;
	float value = soundingData.GetValueAtPressure(theParId, P);
	if(value == kFloatMissing)
	{
		value = soundingData.GetValueAtHeight(theParId, heigthInMetersInStaAth);
	}
		
	if(value == kFloatMissing)
		str += " - ";
	else
		str += static_cast<char*>(NFmiValueString::GetStringWithMaxDecimalsSmartWay(value, theMaxDecimalCount));
	return str;
}

static std::string GetSoundingToolTipText(NFmiTempView::SoundingDataCacheMap &soundingDataCache, const NFmiMTATempSystem::ServerProducer &producer, const NFmiMTATempSystem::TempInfo &theTempInfo, int modelRunIndex, float P, int theZeroBasedIndex, bool doNormalString, const std::string &locationName, float heigthInMetersInStaAth, boost::shared_ptr<NFmiFastQueryInfo>& theInfo)
{
	std::string str;
	if(doNormalString)
	{
		str += NFmiStringTools::Convert(theZeroBasedIndex+1);
		str += ": ";

		std::string timestr(theTempInfo.Time().ToStr(::GetDictionaryString("TempViewLegendTimeFormat")));
		str += timestr;
		str += "\n";
		if(CtrlViewUtils::IsConsideredAsNewData(theInfo, modelRunIndex, false))
		{
			// Lis‰t‰‰n uuden datan korostus merkki
			str += CtrlViewUtils::ParameterStringHighlightCharacter;
		}

		if(theInfo && !theInfo->IsGrid())
		{
			// Jos kyse oli asemadatasta, lis‰t‰‰n sen id rimpsuun, koska se on mukana legendassakin
			auto stationId = theInfo->Location()->GetIdent();
			if(stationId < 10000)
				str += "0";
			str += std::to_string(stationId);
			str += " ";
		}

		str += locationName;
		str += "\n";
	}

    auto soundingDataIter = soundingDataCache.find(NFmiMTATempSystem::SoundingDataCacheMapKey(theTempInfo, producer, modelRunIndex));
    if(soundingDataIter != soundingDataCache.end())
    {
		auto& subSoundingData = soundingDataIter->second.itsSoundingData;
        // sitten laitetaan interpoloidut arvot annetun korkeuden mukaan eri parametreille
        str += "P: ";
        str += static_cast<char*>(NFmiValueString::GetStringWithMaxDecimalsSmartWay(P, 1));
        str += ::GetTooltipValueStr(" T: ", subSoundingData, kFmiTemperature, 1, P, heigthInMetersInStaAth);
        str += ::GetTooltipValueStr(" Td: ", subSoundingData, kFmiDewPoint, 1, P, heigthInMetersInStaAth);
        str += ::GetTooltipValueStr(" WD: ", subSoundingData, kFmiWindDirection, 0, P, heigthInMetersInStaAth);
		str += ::GetTooltipValueStr(" WS: ", subSoundingData, kFmiWindSpeedMS, 1, P, heigthInMetersInStaAth);
		str += ::GetTooltipValueStr(" z: ", subSoundingData, kFmiGeomHeight, 0, P, heigthInMetersInStaAth);
        str += ::GetTooltipValueStr(" N: ", subSoundingData, kFmiTotalCloudCover, 0, P, heigthInMetersInStaAth);
        str += ::GetTooltipValueStr(" RH: ", subSoundingData, kFmiHumidity, 1, P, heigthInMetersInStaAth);
		auto possibleIntegrationInfoStr = ::MakePossibleAvgIntegrationInfo(soundingDataIter->second);
		if(!possibleIntegrationInfoStr.empty())
		{
			str += "\n";
			str += possibleIntegrationInfoStr;
		}
    }
    else
        str += " P: - T: - Td: - WD: - WS: -";
    return str;
}

static float CalcHeightInMetersAtPressure(float pressure)
{
	if(pressure != kFloatMissing)
	{
		float heigthInKmInStaAth = static_cast<float>(CalcHeightAtPressure(pressure));
		if(heigthInKmInStaAth != kFloatMissing)
		{
			return heigthInKmInStaAth * 1000.f;
		}
	}
	return kFloatMissing;
}

std::string NFmiTempView::ComposeToolTipText(const NFmiPoint & theRelativePoint)
{
	std::string str;
    if(itsTempViewDataRects.getSoundingCurveDataRect().IsInside(theRelativePoint) || itsTempViewDataRects.getSecondaryDataFrame().IsInside(theRelativePoint))
    {
        NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
        int modelRunCount = mtaTempSystem.ModelRunCount();
        float pressure = static_cast<float>(y2p(theRelativePoint.Y()));
		float heigthInMetersInStaAth = ::CalcHeightInMetersAtPressure(pressure);
        int index = 0;
        const NFmiMTATempSystem::Container &tempInfos = mtaTempSystem.GetTemps();
        for(const auto &constantLoopTempInfo : tempInfos)
        {
            for(const auto &selectedProducer : mtaTempSystem.SoundingComparisonProducers())
            {
	            NFmiMTATempSystem::TempInfo usedTempInfo = constantLoopTempInfo;
                usedTempInfo.Time(::GetUsedSoundingDataTime(itsCtrlViewDocumentInterface, usedTempInfo));
                boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->InfoOrganizer()->FindSoundingInfo(selectedProducer, usedTempInfo.Time(), usedTempInfo.Latlon(), 0, NFmiInfoOrganizer::ParamCheckFlags(true));
                if(selectedProducer.useServer() || info)
                {
                    auto usedLocationWithName = ::GetSoundingLocation(info, usedTempInfo, itsCtrlViewDocumentInterface->ProducerSystem());
                    usedTempInfo.Latlon(usedLocationWithName.GetLocation());
                    str += "<b><font color=";
                    str += ColorString::Color2HtmlColorStr(mtaTempSystem.SoundingColor(index));
                    str += ">";
                    str += ::GetSoundingToolTipText(itsSoundingDataCacheForTooltips, selectedProducer, usedTempInfo, 0, pressure, index, true, std::string(usedLocationWithName.GetName()), heigthInMetersInStaAth, info);
                    if(modelRunCount > 0 && NFmiDrawParam::IsModelRunDataType(info->DataType()))
                    { 
                        // lis‰t‰‰n edelliset malliajo -osio tooltippiin 
                        for(int modelRunIndex = -1; modelRunIndex >= -modelRunCount; modelRunIndex--)
                        {
                            str += "\n[";
                            str += NFmiStringTools::Convert(modelRunIndex);
                            str += "]\t";
                            str += ::GetSoundingToolTipText(itsSoundingDataCacheForTooltips, selectedProducer, usedTempInfo, modelRunIndex, pressure, index, false, std::string(usedLocationWithName.GetName()), heigthInMetersInStaAth, info);
                        }
                    }
                    str += "</font></b>";

                    str += "<br><hr color=red><br>"; // v‰liviiva
                }
                index++;
            }
        }
    }
	return str;
}

// Luotausdatan esimerkkihaku: ec-euro-mallipintadata, lonlat-piste,parametrilista, ascii formaatti, haluttu origintime, haluttu starttime, tarkkuus double => 1 desimaali
// http://smartmet.fmi.fi/timeseries?producer=ecmwf_eurooppa_mallipinta&lonlat=23.6,64.3&param=temperature,dewpoint,relativehumidity,pressure,geomheight,totalcloudcover,windspeedms,winddirection,level&timesteps=1&format=ascii&precision=double&origintime=201901170000&starttime=201901191200

// Sen tulos :

std::string soundingFromServerTestString =
"-53.9 nan 2.0 93.4 15872.5 0.0 10.6 307.5 59.0 "
"-54.0 nan 2.1 98.4 15533.9 0.0 11.0 307.7 60.0 "
"-54.1 nan 2.3 103.7 15198.3 0.0 11.5 310.0 61.0 "
"-54.1 nan 2.4 109.2 14865.5 0.0 12.0 307.2 62.0 "
"-54.0 nan 2.5 115.0 14535.4 0.0 12.1 307.0 63.0 "
"-53.9 nan 2.6 121.0 14208.0 0.0 11.9 305.0 64.0 "
"-53.9 nan 2.7 127.3 13883.3 0.0 11.8 307.0 65.0 "
"-53.9 nan 2.9 133.9 13561.2 0.0 12.2 308.6 66.0 "
"-54.1 nan 3.1 140.7 13242.1 0.0 13.1 308.6 67.0 "
"-54.1 nan 3.3 147.8 12925.8 0.0 13.8 303.7 68.0 "
"-53.9 nan 3.4 155.2 12612.1 0.0 14.2 300.2 69.0 "
"-53.6 nan 3.4 163.0 12300.8 0.0 13.9 300.0 70.0 "
"-53.3 nan 3.5 171.0 11991.6 0.0 13.4 300.0 71.0 "
"-53.1 nan 3.6 179.3 11684.8 0.0 12.9 300.0 72.0 "
"-52.9 nan 3.9 188.0 11380.3 0.0 12.5 300.4 73.0 "
"-52.6 nan 4.2 197.0 11078.1 0.0 12.1 301.4 74.0 "
"-52.5 nan 4.7 206.4 10778.2 0.0 11.4 301.1 75.0 "
"-52.4 nan 5.1 216.1 10480.7 0.0 10.8 303.1 76.0 "
"-52.5 nan 5.5 226.2 10185.7 0.0 10.4 301.6 77.0 "
"-52.6 nan 5.9 236.7 9893.4 0.0 9.9 301.6 78.0 "
"-52.9 nan 6.8 247.6 9603.8 0.0 9.6 301.2 79.0 "
"-53.3 nan 8.3 258.9 9317.0 0.0 9.0 295.0 80.0 "
"-53.8 nan 9.9 270.5 9033.2 0.0 8.2 293.0 81.0 "
"-54.3 nan 11.6 282.7 8752.5 0.0 7.3 285.0 82.0 "
"-54.8 nan 14.2 295.2 8474.7 0.0 6.5 272.5 83.0 "
"-55.3 nan 19.0 308.2 8200.0 0.0 5.9 263.1 84.0 "
"-55.6 nan 32.0 321.7 7928.0 0.0 5.7 244.2 85.0 "
"-55.5 nan 54.7 335.5 7658.3 0.0 5.8 227.4 86.0 "
"-54.9 nan 68.0 350.0 7390.5 0.0 6.2 213.7 87.0 "
"-53.9 nan 77.3 364.8 7123.8 0.0 6.6 207.1 88.0 "
"-52.7 nan 77.7 380.3 6858.1 0.0 6.8 199.2 89.0 "
"-51.2 nan 72.9 396.1 6592.9 0.0 6.8 200.1 90.0 "
"-49.8 nan 70.2 412.6 6328.1 0.0 6.4 203.7 91.0 "
"-48.3 nan 71.2 429.6 6063.8 0.0 5.9 210.7 92.0 "
"-46.8 nan 72.9 447.1 5799.9 0.6 5.3 221.0 93.0 "
"-45.4 nan 76.7 465.2 5536.5 2.2 4.8 235.2 94.0 "
"-43.8 nan 81.9 483.8 5273.4 4.8 4.5 256.1 95.0 "
"-41.9 nan 88.6 503.0 5010.9 5.6 4.9 278.8 96.0 "
"-39.8 nan 86.2 522.8 4749.6 3.8 5.7 299.4 97.0 "
"-37.5 nan 80.9 542.9 4490.7 2.0 6.6 309.1 98.0 "
"-35.3 nan 77.7 563.2 4235.6 1.5 7.4 318.8 99.0 "
"-33.3 nan 77.8 583.8 3985.4 2.6 8.2 323.3 100.0 "
"-31.3 nan 80.5 604.4 3741.0 6.3 8.5 326.2 101.0 "
"-29.4 nan 83.3 625.0 3503.2 11.8 8.2 325.1 102.0 "
"-27.7 nan 88.7 645.4 3273.0 19.1 7.8 324.1 103.0 "
"-26.0 nan 93.1 665.6 3050.9 22.6 7.8 323.0 104.0 "
"-24.5 nan 96.8 685.5 2837.3 25.9 8.1 319.5 105.0 "
"-23.1 nan 99.3 705.0 2632.9 31.4 8.3 319.8 106.0 "
"-21.8 nan 99.8 724.0 2437.9 38.6 8.1 326.6 107.0 "
"-20.7 nan 99.8 742.4 2252.4 47.5 7.7 325.2 108.0 "
"-19.5 nan 99.9 760.3 2076.7 52.4 7.5 323.5 109.0 "
"-18.5 nan 99.7 777.4 1910.6 53.4 7.3 319.5 110.0 "
"-17.6 nan 99.5 794.0 1754.0 47.3 7.1 317.1 111.0 "
"-16.7 nan 99.3 809.7 1606.8 43.5 6.8 317.6 112.0 "
"-15.8 nan 98.7 824.7 1468.8 41.5 6.5 312.2 113.0 "
"-15.0 nan 98.0 838.9 1339.6 35.9 6.1 312.2 114.0 "
"-14.4 nan 96.9 852.4 1219.1 27.7 5.5 310.9 115.0 "
"-13.8 nan 95.5 865.1 1106.8 24.5 4.5 307.3 116.0 "
"-13.4 nan 94.4 877.1 1002.4 29.2 3.7 294.9 117.0 "
"-13.0 nan 93.6 888.3 905.6 35.9 3.4 289.5 118.0 "
"-12.7 nan 94.1 898.9 816.0 41.7 3.2 286.9 119.0 "
"-12.4 nan 95.5 908.7 733.0 47.3 3.2 285.6 120.0 "
"-12.1 nan 97.6 917.9 656.5 54.0 3.1 284.3 121.0 "
"-11.7 nan 98.2 926.4 585.8 60.8 3.1 280.8 122.0 "
"-11.2 nan 99.1 934.3 520.6 62.7 3.3 282.3 123.0 "
"-10.6 nan 99.1 941.6 460.6 62.5 3.5 281.0 124.0 "
"-10.1 nan 99.5 948.4 405.3 61.7 3.6 281.9 125.0 "
"-9.6 nan 99.2 954.7 354.5 57.4 3.8 281.4 126.0 "
"-9.2 nan 99.0 960.5 307.8 41.8 3.9 282.9 127.0 "
"-8.8 nan 97.9 965.9 264.9 26.8 4.0 281.0 128.0 "
"-8.4 nan 97.0 970.8 225.5 20.2 4.0 281.0 129.0 "
"-8.1 nan 95.6 975.3 189.4 9.6 4.0 283.8 130.0 "
"-7.8 nan 94.3 979.5 156.4 7.8 4.2 286.0 131.0 "
"-7.6 nan 93.2 983.3 126.1 6.5 4.1 284.8 132.0 "
"-7.4 nan 92.2 986.8 98.3 5.3 4.0 284.2 133.0 "
"-7.3 nan 90.9 990.1 73.0 3.5 3.9 283.0 134.0 "
"-7.0 nan 89.5 993.0 49.8 2.7 3.7 283.0 135.0 "
"-6.8 nan 88.6 995.7 28.6 2.4 3.4 283.0 136.0 "
"-6.5 nan 88.0 998.2 9.3 1.2 2.9 282.4 137.0 ";


static std::string MakeProdTimeLocString(const NFmiProducer &producer, const NFmiMetTime &theTime, const NFmiLocation &theLocation)
{
    std::string str = "producer: ";
    str += producer.GetName();
    str += ", for time: ";
    str += theTime.ToStr("YYYY.MM.DD HH:mm");
    str += ", at location: ";
    str += std::to_string(theLocation.GetLongitude());
    str += ",";
    str += std::to_string(theLocation.GetLatitude());
    return str;
}

static void TraceLogSoundingFromServerRequest(const std::string &requestUriStr, const NFmiProducer &producer, const NFmiMetTime &theTime, const NFmiLocation &theLocation)
{
    if(requestUriStr.empty())
    {
        std::string logMessage = "Unable to make sounding data request URL for ";
        ::MakeProdTimeLocString(producer, theTime, theLocation);
        CatLog::logMessage(requestUriStr, CatLog::Severity::Warning, CatLog::Category::NetRequest);
    }
    else
    {
        if(CatLog::doTraceLevelLogging())
        {
            CatLog::logMessage(requestUriStr, CatLog::Severity::Trace, CatLog::Category::NetRequest);
        }
    }
}

static void ReportFailedSoundingFromServerRequest(const std::string &requestUriStr, const NFmiProducer &producer, const NFmiMetTime &theTime, const NFmiLocation &theLocation)
{
    if(!CatLog::doTraceLevelLogging())
    {
        // Jos ei olla trace loggin tilassa, ei edell‰ lokitettu t‰t‰ requestia, tehd‰‰n se nyt kun tuli ongelmia sen kanssa
        CatLog::logMessage(requestUriStr, CatLog::Severity::Debug, CatLog::Category::NetRequest);
    }

    std::string logMessage = "Sounding data server request failed for ";
    logMessage += ::MakeProdTimeLocString(producer, theTime, theLocation);
    CatLog::logMessage(logMessage, CatLog::Severity::Warning, CatLog::Category::NetRequest);
}

bool NFmiTempView::FillSoundingDataFromServer(const NFmiMTATempSystem::SoundingProducer &theProducer, NFmiSoundingData &theSoundingData, const NFmiMetTime &theTime, const NFmiLocation &theLocation)
{
    auto requestUriStr = itsCtrlViewDocumentInterface->GetMTATempSystem().GetSoundingDataServerConfigurations().makeFinalServerRequestUrl(theProducer.GetIdent(), theTime, theLocation.GetLocation());
    ::TraceLogSoundingFromServerRequest(requestUriStr, theProducer, theTime, theLocation);
    if(requestUriStr.empty())
        return false;
    std::string soundingDataResponseFromServer;

    {
        // Raportoidaan trace tasolla pelk‰n haun kesto
        CtrlViewUtils::CtrlViewTimeConsumptionReporter timeConsumptionReporter(this, "Sounding data from server request");
        itsCtrlViewDocumentInterface->MakeHTTPRequest(requestUriStr, soundingDataResponseFromServer, true);
    }

    if(soundingDataResponseFromServer.empty())
    {
        ::ReportFailedSoundingFromServerRequest(requestUriStr, theProducer, theTime, theLocation);
        NFmiLocation errorLocation = theSoundingData.Location();
        errorLocation.SetName("No data from server");
        theSoundingData.Location(errorLocation);
        return false;
    }
    const auto &paramsInServerData = itsCtrlViewDocumentInterface->GetMTATempSystem().GetSoundingDataServerConfigurations().wantedParameters();
    auto status = theSoundingData.FillSoundingData(paramsInServerData, soundingDataResponseFromServer, theTime, theLocation, nullptr);
    // Laitetaan lopuksi serverilt‰ haetun origintime:n avulla luotauksen paikan nimi lopulliseen kuntoon
    NFmiLocation finalLocation = theSoundingData.Location();
    ::SetLocationNameByItsLatlon(itsCtrlViewDocumentInterface->ProducerSystem(), finalLocation, theProducer, theSoundingData.OriginTime(), true);
	std::string finalNameWithServerMarker = "(S) ";
    finalNameWithServerMarker += finalLocation.GetName();
    finalLocation.SetName(finalNameWithServerMarker);
    theSoundingData.Location(finalLocation);
    return status;
}

// Vaisala diagrammi on helppo tapaus, katso ‰‰ri T ja Td arvot kaikista luotauksista
// ja kerroksista ja lis‰‰ v‰h‰n toppausta reunoille ja siin‰ on T-akselin alku+loppu rajat.
std::pair<double, double> NFmiTempView::GetVisibleTemperatureRangeForVaisalaDiagram(const std::pair<double, double>& originalRange)
{
	NFmiDataModifierMinMax autoAdjustTMinMaxValues;
	for(auto& totalSoundingData : itsScanData)
	{
		ScanSingleDataVaisala(totalSoundingData, autoAdjustTMinMaxValues);
	}

	if(autoAdjustTMinMaxValues.MinValue() != kFloatMissing)
	{
		auto rawStartT = autoAdjustTMinMaxValues.MinValue();
		auto rawEndT = autoAdjustTMinMaxValues.MaxValue();
		// Jos originaali rangen sis‰‰n mahtuu kaikki arvot, k‰ytet‰‰n niit‰,
		// muuten lasketaan uusi range pyˆrist‰en ja lis‰‰m‰ll‰ marginaalia.
		if(rawStartT < originalRange.first || rawEndT > originalRange.second)
		{
			double newStartT = std::floor(autoAdjustTMinMaxValues.MinValue());
			double newEndT = std::ceil(autoAdjustTMinMaxValues.MaxValue());
			auto diff = (newEndT - newStartT);
			auto padding = std::round(std::sqrt(diff) / 2.);
			if(padding < 3.)
			{
				padding = 3.;
			}
			newStartT -= padding;
			newEndT += padding;
			return std::make_pair(newStartT, newEndT);
		}
	}

	return originalRange;
}

// Skannaa annetun T-rangen kaikille datoille ja laskee kuinka monta n‰kyv‰‰ pistett‰
// rangeen osuu. Palauttaa true, jos n‰kyvyys oli t‰ydellinen. Palauttaa parametreina potentiaaliset vs todelliset n‰kyvyydet.
bool NFmiTempView::ScanRangeForAllDataSkewT(double startT, double endT, int& potenciallyVisibleValuesInOut, int& actuallyVisibleValuesInOut)
{
	SetupTAxisValues(startT, endT);
	for(auto& totalSoundingData : itsScanData)
	{
		ScanSingleDataSkewT(totalSoundingData, potenciallyVisibleValuesInOut, actuallyVisibleValuesInOut);
	}
	return (potenciallyVisibleValuesInOut == actuallyVisibleValuesInOut);
}

// Skew-T diagrammi on hankala, pit‰‰ k‰yd‰ l‰pi lista erilaisia vaihtoehtoja T-akselin 
// alku+loppup‰iden et‰isyyksille. Toisessa iteraatiossa k‰yd‰‰n l‰pi sopivista aloituskohdista.
// Sitten k‰yd‰‰n l‰pi joka luotaus kaikilla leveleill‰ ja katsotaan kuinka 
// moni T+Td arvo j‰‰ oikeasti n‰kyviin. N‰in etsit‰‰n jotain miss‰ on eniten n‰kyviss‰ 
// olevia arvoja. En tied‰ kuinka paljon iteraatioita oikeasti pit‰‰ tehd‰...
std::pair<double, double> NFmiTempView::GetVisibleTemperatureRangeForSkewTDiagram(const std::pair<double, double>& originalRange)
{
	vector<double> suitableTRanges{ 35, 40, 45, 50, 55, 60, 65, 70, 75, 80 };
	vector<double> suitableTStartValues{ 0, 5, -5, 10, -10, 15, -15, 20, -20, 25, -25, 30, -30, 35, -35, -40, -45, -50, -55, -60, -65, -70, -75, -80};
	int potenciallyVisibleValues = 0;
	int actuallyVisibleValues = 0;
	if(ScanRangeForAllDataSkewT(originalRange.first, originalRange.second, potenciallyVisibleValues, actuallyVisibleValues))
	{
		// Originaali T-akselilla kaikki pisteet n‰kyv‰t, palautetaan se sellaisenaan heti
		return originalRange;
	}

	// Laitetaan originaali asteikon n‰kyvyydet ja arvot alkuarvauksiksi
	int bestVisibleCount = actuallyVisibleValues;
	std::pair<double, double> bestVisibleRange = originalRange;
	for(auto suitableRange : suitableTRanges)
	{
		for(auto suitableStartValue : suitableTStartValues)
		{
			auto startT = suitableStartValue;
			auto endT = suitableStartValue + suitableRange;
			potenciallyVisibleValues = 0;
			actuallyVisibleValues = 0;
			if(ScanRangeForAllDataSkewT(startT, endT, potenciallyVisibleValues, actuallyVisibleValues))
			{
				// Kaikki pisteet n‰kyv‰t, palautetaan sellainen heti
				return std::make_pair(startT, endT);
			}
			else if(bestVisibleCount < actuallyVisibleValues)
			{
				bestVisibleCount = actuallyVisibleValues;
				bestVisibleRange = std::make_pair(startT, endT);
			}
		}
	}
	return bestVisibleRange;
}

void NFmiTempView::ScanVisualizedData()
{
	itsOperationalMode = SoundingViewOperationMode::FitScalesScanMode;
	itsScanData.clear();
	DrawSoundingsInMTAMode();
	itsOperationalMode = SoundingViewOperationMode::NormalDrawMode;
	NFmiMTATempSystem& mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
	std::pair<double, double> startEndTemperatureScaleValues(mtaTempSystem.TAxisStart(), mtaTempSystem.TAxisEnd());
	if(mtaTempSystem.SkewTDegree() == 0)
	{
		startEndTemperatureScaleValues = GetVisibleTemperatureRangeForVaisalaDiagram(startEndTemperatureScaleValues);
	}
	else
	{
		startEndTemperatureScaleValues = GetVisibleTemperatureRangeForSkewTDiagram(startEndTemperatureScaleValues);
	}

	mtaTempSystem.TAxisStart(startEndTemperatureScaleValues.first);
	mtaTempSystem.TAxisEnd(startEndTemperatureScaleValues.second);
}

void NFmiTempView::ScanSingleDataVaisala(TotalSoundingData& totalSoundingData, NFmiDataModifierMinMax& theAutoAdjustTMinMaxValuesOut)
{
	auto& soundingData = totalSoundingData.itsSoundingData;
	if(soundingData.IsDataGood())
	{
		const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
		const auto& P_container = soundingData.GetParamData(kFmiPressure);
		const auto& T_container = soundingData.GetParamData(kFmiTemperature);
		const auto& Td_container = soundingData.GetParamData(kFmiDewPoint);
		for(size_t index = 0; index < P_container.size(); index++)
		{
			auto P = P_container[index];
			double y = p2y(P);
			if(y <= dataRect.Bottom() && y >= dataRect.Top())
			{
				auto T = T_container[index];
				theAutoAdjustTMinMaxValuesOut.Calculate(T);
				auto Td = Td_container[index];
				theAutoAdjustTMinMaxValuesOut.Calculate(Td);
//				double x1 = pt2x(P, T);
//				double x2 = pt2x(P, Td);
			}
		}
	}
}

void NFmiTempView::CheckIsTVisible(float T, float P, double yPos, int& potenciallyVisibleValuesInOut, int& actuallyVisibleValuesInOut)
{
	if(T != kFloatMissing)
	{
		potenciallyVisibleValuesInOut++;
		double x1 = pt2x(P, T);
		if(itsTempViewDataRects.getSoundingCurveDataRect().IsInside(NFmiPoint(x1, yPos)))
		{
			actuallyVisibleValuesInOut++;
		}
	}
}

void NFmiTempView::ScanSingleDataSkewT(TotalSoundingData& totalSoundingData, int& potenciallyVisibleValuesInOut, int& actuallyVisibleValuesInOut)
{
	auto& soundingData = totalSoundingData.itsSoundingData;
	if(soundingData.IsDataGood())
	{
		bool previousHeightValueWasChecked = false;
		double lastInsideRectY = -999;
		const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
		const auto& P_container = soundingData.GetParamData(kFmiPressure);
		const auto& T_container = soundingData.GetParamData(kFmiTemperature);
		const auto& Td_container = soundingData.GetParamData(kFmiDewPoint);
		for(size_t index = 0; index < P_container.size(); index++)
		{
			auto P = P_container[index];
			double y = p2y(P);
			if(y <= dataRect.Bottom() && y >= dataRect.Top())
			{
				previousHeightValueWasChecked = true;
				lastInsideRectY = y;
				CheckIsTVisible(T_container[index], P, y, potenciallyVisibleValuesInOut, actuallyVisibleValuesInOut);
				CheckIsTVisible(Td_container[index], P, y, potenciallyVisibleValuesInOut, actuallyVisibleValuesInOut);
			}
			else
			{
				if(previousHeightValueWasChecked)
				{
					// Jos edellisen levelin arvo on tarkastettu ja seuraava on ulkona,
					// halutaan juuri se seuraava viel‰ tarkastaa, koska edellisest‰ menev‰ viiva voi muuten menn‰ ruudun ulos.
					CheckIsTVisible(T_container[index], P, lastInsideRectY, potenciallyVisibleValuesInOut, actuallyVisibleValuesInOut);
					CheckIsTVisible(Td_container[index], P, lastInsideRectY, potenciallyVisibleValuesInOut, actuallyVisibleValuesInOut);
				}
				previousHeightValueWasChecked = false;
				lastInsideRectY = -999;
			}
		}
	}
}

void NFmiTempView::AutoAdjustSoundingScales()
{
	ScanVisualizedData();
}

void NFmiTempView::SetupTAxisValues(double startT, double endT)
{
	tmax = endT;
	tmin = startT;
	dt = tmax - tmin;
}
