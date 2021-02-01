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

#include <stdexcept>
#include "boost\math\special_functions\round.hpp"

#include <thread>

using namespace std;
using namespace Gdiplus;

#ifdef max
#undef max
#endif

static NFmiString GetStrValue(float value, int decimalCount, int totalChars)
{
	NFmiString str;
	if(value != kFloatMissing)
	{
		if(decimalCount == 0)
			str += NFmiStringTools::Convert(::round(value));
		else 
		{
			NFmiString formatStr;
			if(decimalCount == 1)
				formatStr = "%0.1f";
			else if(decimalCount == 2)
				formatStr = "%0.2f";
			else
				formatStr = "%0.3f";
			NFmiValueString valStr(value, formatStr);
			str += valStr;
		}
	}
	else
		str += NFmiString("-");
	if(static_cast<int>(str.GetLen()) < totalChars)
	{
		for(int i=str.GetLen(); i<totalChars; i++)
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
	NFmiString name;
    if(fIsGrid == false && (NFmiInfoOrganizer::IsTempData(theProducer.GetIdent(), true)))
		name += theLocation.GetName() + NFmiString(" ");
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
	NFmiString name(theProducer.GetName());
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
,itsFirstSoundingData()
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
	double x = x0 + itsToolBox->SX(static_cast<long>(xpix1 - xpix0));
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
    double verticalOffset = itsToolBox->SY(boost::math::iround(2 * itsDrawSizeFactor.Y()));
	// Labeleita pit‰‰ siirt‰‰ hieman vertikaali suunnassa, alareunassa pikkuisen ylˆsp‰in (negatiivinen offset)
    NFmiPoint labelOffset(0, -verticalOffset); 
    DrawSecondaryDataHorizontalAxel(labelInfo, lineInfo, kTopCenter, secondaryDataFrame.Bottom(), labelOffset);

    // 2. Piirret‰‰n asteikko yl‰reunaan
	// Labeleita pit‰‰ siirt‰‰ hieman vertikaali suunnassa, yl‰reunassa pikkuisen alasp‰in (positiivinen offset)
    labelOffset.Y(verticalOffset); 
	// Yl‰reunan tektit menev‰t hieman vasemmalle tuntemattomasta syyst‰, ja siksi niit‰ siirret‰‰n hieman oikealla t‰ss‰
	double topHorizontalLabelOffset = itsToolBox->SX(boost::math::iround(5 * itsDrawSizeFactor.X()));
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

void NFmiTempView::DrawSecondaryData(NFmiSoundingDataOpt1 &theData, FmiParameterName theParId, const NFmiTempLineInfo &theLineInfo)
{
    if(theLineInfo.DrawLine() == false)
        return;
    // GDI+ piirto koodia
	const auto& secondaryDataFrame = itsTempViewDataRects.getSecondaryDataFrame();
	itsGdiPlusGraphics->SetClip(CtrlView::Relative2GdiplusRect(itsToolBox, secondaryDataFrame));

    std::deque<float> &drawedParam = theData.GetParamData(theParId);
    std::deque<float> &pressures = theData.GetParamData(kFmiPressure);
    std::deque<float> &heights = theData.GetParamData(kFmiGeomHeight);
    bool useHeight = (theData.PressureDataAvailable() == false && theData.HeightDataAvailable() == true);
    if(drawedParam.size() > 1 && drawedParam.size() == pressures.size() && drawedParam.size() == heights.size())
    {
        int maxMissingValues = theData.ObservationData() ? 15 : 1; // jos per‰kk‰in puuttuu enemm‰n kuin n‰in monta arvoa, ei yhdistet‰ viivoja, vaan katkaistaan
        int consecutiveMissingValues = 0;
        int ssize = static_cast<int>(drawedParam.size());
        std::vector<PointF> points;
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
                    points.push_back(PointF(static_cast<REAL>(x*itsGdiplusScale.X()), static_cast<REAL>(y*itsGdiplusScale.Y())));
                    doingFirstTimeChecks = false;
                }
                else // piirret‰‰n p‰tk‰ mik‰ on vektorissa tallessa
                {
                    ::DrawGdiplusCurve(*itsGdiPlusGraphics, points, theLineInfo, GetUsedCurveDrawSmoothingMode(), IsPrinting(), false);
                    points.clear();
                }
            }

            if(value != kFloatMissing && (useHeight ? h != kFloatMissing : p != kFloatMissing))
                consecutiveMissingValues = 0;
            else
                consecutiveMissingValues++;
        }
        ::DrawGdiplusCurve(*itsGdiPlusGraphics, points, theLineInfo, GetUsedCurveDrawSmoothingMode(), IsPrinting(), false); // lopuksi viel‰ piirret‰‰n loputkin mit‰ on piirrett‰v‰‰
    }
    itsGdiPlusGraphics->ResetClip();
}

void NFmiTempView::DrawSecondaryData(NFmiSoundingDataOpt1 & theData, int theIndex, const NFmiColor &theUsedSoundingColor)
{
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    if(mtaTempSystem.DrawSecondaryData())
    {
        NFmiTempLineInfo lineInfo = mtaTempSystem.WSLineInfo();
        lineInfo.Color(theUsedSoundingColor);
        lineInfo.Thickness(boost::math::iround(lineInfo.Thickness() * itsDrawSizeFactor.X() * ExtraPrintLineThicknesFactor(true)));
        DrawSecondaryData(theData, kFmiWindSpeedMS, lineInfo); // Piirret‰‰n l‰mpˆtila
        lineInfo = mtaTempSystem.NLineInfo();
        lineInfo.Color(theUsedSoundingColor);
        lineInfo.Thickness(boost::math::iround(lineInfo.Thickness() * itsDrawSizeFactor.X() * ExtraPrintLineThicknesFactor(true)));
        DrawSecondaryData(theData, kFmiTotalCloudCover, lineInfo); // Piirret‰‰n kokonaispilvisyys
        lineInfo = mtaTempSystem.RHLineInfo();
        lineInfo.Color(theUsedSoundingColor);
        lineInfo.Thickness(boost::math::iround(lineInfo.Thickness() * itsDrawSizeFactor.X() * ExtraPrintLineThicknesFactor(true)));
        DrawSecondaryData(theData, kFmiHumidity, lineInfo); // Piirret‰‰n suhteellinen kosteus
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

    if(theToolBox == 0)
		return ;
	itsToolBox = theToolBox;

    try
    {
        InitializeGdiplus(itsToolBox, 0);
        CalcDrawSizeFactors();

        // tehd‰‰n GDI+ maailmaan tarvittavat skaala kertoimet
        itsGdiplusScale.X(itsToolBox->GetClientRect().Width());
        itsGdiplusScale.Y(itsToolBox->GetClientRect().Height());

        NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
        tmax = mtaTempSystem.TAxisEnd();
        tmin = mtaTempSystem.TAxisStart();
        dt = tmax - tmin;

        pmin = mtaTempSystem.PAxisEnd();
        pmax = mtaTempSystem.PAxisStart();

        tdegree = mtaTempSystem.SkewTDegree();
		InitializeHodografRect();
        DrawBackground();

        _1PixelInRel.X(itsToolBox->SX(1));
		_1PixelInRel.Y(itsToolBox->SY(1));
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
			// Resetoidaan itsFirstSoundingData, muuten tulee ongelmia viimeisen 'hyv‰n' 1. datan piirrossa tekstimuotoisiin ikkunoihin, jos t‰ss‰ piirrossa ei lˆydy dataa ollenkaan (esim. ollaan 1. datan aikaikkunan ulkopuolella)
			itsFirstSoundingData = NFmiSoundingDataOpt1(); 
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
	long blPixelOffsetX = boost::math::iround(5 * itsDrawSizeFactor.X());
	long blPixelOffsetY = boost::math::iround(5 * itsDrawSizeFactor.Y());
	double blRelativeOffsetX = itsToolBox->SX(blPixelOffsetX);
	double blRelativeOffsetY = itsToolBox->SY(blPixelOffsetY);
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
	if(IsAnyTextualSideViewVisible())
	{
		DrawStabilityIndexData();
		DrawTextualSoundingData();
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

static NFmiString GetNameText(NFmiSoundingDataOpt1 &theData)
{
	NFmiString str;

    if(theData.MovingSounding())
    {
        if(theData.Location().GetLocation() == NFmiPoint::gMissingLatlon)
            str += "-,-";
        else
        {
            str += CtrlViewUtils::GetLatitudeMinuteStr(theData.Location().GetLatitude(), 1);
            str += ",";
            str += CtrlViewUtils::GetLongitudeMinuteStr(theData.Location().GetLongitude(), 1);
        }
    }
    else
	{
        if(theData.Location().GetIdent() != 0)
        {
            if(theData.Location().GetIdent() < 10000)
                str += "0"; // pit‰‰ tarvittaessa laittaa etu nolla aseman identin eteen
            str += NFmiStringTools::Convert(theData.Location().GetIdent());
            str += " ";
        }
        str += theData.Location().GetName();
    }

	return str;
}
static NFmiString GetLatText(NFmiSoundingDataOpt1 &theData)
{
	NFmiString str("SLAT=");
	if(theData.Location().GetLatitude() != kFloatMissing)
		str += CtrlViewUtils::GetLatitudeMinuteStr(theData.Location().GetLatitude(), 1);
	else
		str += " -";
	return str;
}
static NFmiString GetLonText(NFmiSoundingDataOpt1 &theData)
{
	NFmiString str("SLON=");
	if(theData.Location().GetLongitude() != kFloatMissing)
		str += CtrlViewUtils::GetLongitudeMinuteStr(theData.Location().GetLongitude(), 1);
	else
		str += " -";
	return str;
}

static NFmiString GetElevationText(NFmiSoundingDataOpt1 &theData, boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
	NFmiString str("SELEV=");
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

NFmiString NFmiTempView::GetIndexText(double theValue, const NFmiString &theText, int theDecimalCount)
{
	NFmiString str(theText);
	str += "=";
	if(theValue != kFloatMissing)
		str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(theValue, theDecimalCount);
	else
		str += " -";
	return str;
}

void NFmiTempView::MoveToNextLine(NFmiPoint &theTextPoint, double factor)
{ // siirret‰‰n annettua pistett‰ alasp‰in sen verran mit‰ fontti koko m‰‰r‰‰
	theTextPoint.Y(theTextPoint.Y() + itsToolBox->SY(static_cast<long>(itsDrawingEnvironment->GetFontHeight()*factor)));
}

void NFmiTempView::DrawNextLineToIndexView(NFmiText &theText, const NFmiString &theStr, NFmiPoint &theTextPoint, double factor, bool moveFirst, bool addToString)
{
	if(addToString)
		itsSoundingIndexStr += theStr + "\n";
	if(moveFirst)
		MoveToNextLine(theTextPoint, factor);
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

void NFmiTempView::DrawAllLiftedAirParcels(NFmiSoundingDataOpt1 &theData)
{
	DrawLiftedAirParcel(theData, kLCLCalcSurface);
	DrawLiftedAirParcel(theData, kLCLCalc500m2);
	DrawLiftedAirParcel(theData, kLCLCalcMostUnstable);
}

void NFmiTempView::DrawLiftedAirParcel(NFmiSoundingDataOpt1 &theData, FmiLCLCalcType theLCLCalcType)
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
	p.X(p.X() + itsToolBox->SX(2)); // siirret‰‰n teksti‰ pikkusen oikealle p‰in
	return p;
}

void NFmiTempView::DrawTextualSoundingData(void)
{
	auto& mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
	auto settingsWinReg = mtaTempSystem.GetSoundingViewSettingsFromWindowsRegisty();
	if(!settingsWinReg.ShowTextualSoundingDataSideView())
		return;

	itsSoundingIndexStr = ""; // t‰ytyy nollata t‰m‰ teksti, sit‰ p‰ivitet‰‰n aina sitten DrawNextLineToIndexView-metodissa
	DrawStabilityIndexBackground(itsTempViewDataRects.getTextualSoundingDataSideViewRect());

	// piirret‰‰n vain 1. luotaukseen liittyv‰t indeksit ja ne piirret‰‰n samalla v‰rilla kuin 1. luotaus
	itsDrawingEnvironment->SetFrameColor(mtaTempSystem.SoundingColor(0));
	int fontSize = itsTempViewDataRects.getTextualSoundingDataFontSize();
	if(itsToolBox->GetDC()->IsPrinting())
		fontSize = boost::math::iround(fontSize * 0.9); // printatessa pit‰‰ fonttia hieman jostain syyst‰ pienent‰‰, koska muuten tekstit eiv‰t mahdu kokonaisuudessaan ruutuun
	itsDrawingEnvironment->SetFontSize(NFmiPoint(fontSize * itsDrawSizeFactor.X(), fontSize * itsDrawSizeFactor.Y()));
	itsToolBox->SetTextAlignment(kLeft);

	DrawSoundingInTextFormat(itsFirstSoundingData); // printataan 1. piirretyn luotauksen data tekstimuodossa sivu n‰yttˆˆn
}

void NFmiTempView::DrawStabilityIndexData(void)
{
	auto& mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
	auto settingsWinReg = mtaTempSystem.GetSoundingViewSettingsFromWindowsRegisty();
	if(!settingsWinReg.ShowStabilityIndexSideView())
		return;

	itsSoundingIndexStr = ""; // t‰ytyy nollata t‰m‰ teksti, sit‰ p‰ivitet‰‰n aina sitten DrawNextLineToIndexView-metodissa
	DrawStabilityIndexBackground(itsTempViewDataRects.getStabilityIndexSideViewRect());

	// piirret‰‰n vain 1. luotaukseen liittyv‰t indeksit ja ne piirret‰‰n samalla v‰rilla kuin 1. luotaus
	itsDrawingEnvironment->SetFrameColor(mtaTempSystem.SoundingColor(0));
	int fontSize = itsTempViewDataRects.getStabilityIndexFontSize();
	if(itsToolBox->GetDC()->IsPrinting())
		fontSize = boost::math::iround(fontSize * 0.9); // printatessa pit‰‰ fonttia hieman jostain syyst‰ pienent‰‰, koska muuten tekstit eiv‰t mahdu kokonaisuudessaan ruutuun
	itsDrawingEnvironment->SetFontSize(NFmiPoint(fontSize * itsDrawSizeFactor.X(), fontSize * itsDrawSizeFactor.Y()));
	itsToolBox->SetTextAlignment(kLeft);

	NFmiPoint p(CalcStabilityIndexStartPoint());

	NFmiText text(p, NFmiString(""), 0, itsDrawingEnvironment);

	DrawNextLineToIndexView(text, ::GetNameText(itsFirstSoundingData), p, 1, false);
	DrawNextLineToIndexView(text, ::GetLatText(itsFirstSoundingData), p, 1);
	DrawNextLineToIndexView(text, ::GetLonText(itsFirstSoundingData), p, 1);
	DrawNextLineToIndexView(text, ::GetElevationText(itsFirstSoundingData, itsCtrlViewDocumentInterface->InfoOrganizer()->FindInfo(NFmiInfoData::kStationary)), p, 1);

	// pit‰‰ piirt‰‰ tyhj‰‰ alkuun ett‰ saadaan "kursori-arvoille" tilaa, ne piirret‰‰n DrawOverBitmap-kohdassa
	DrawNextLineToIndexView(text, "Cursor values:", p, 1);
	DrawNextLineToIndexView(text, "", p, 1); // T ja P tulee t‰h‰n DrawOverBitmap-kohdassa
	DrawNextLineToIndexView(text, "", p, 1); // Dry ja Moist tulee t‰h‰n DrawOverBitmap-kohdassa
	DrawNextLineToIndexView(text, "", p, 1); // Mix tulee t‰h‰n DrawOverBitmap-kohdassa
	DrawNextLineToIndexView(text, "", p, 1);
	DrawNextLineToIndexView(text, "", p, 1);
	DrawNextLineToIndexView(text, "-------------", p, 1);

	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcSHOWIndex(), "SHOW", 1), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcLIFTIndex(), "?LIFT", 1), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcKINXIndex(), "KINX", 1), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcCTOTIndex(), "CTOT", 1), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcVTOTIndex(), "VTOT", 1), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcTOTLIndex(), "TOTL", 1), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcGDI(), "GDI", 1), p, 1);
	DrawNextLineToIndexView(text, " ", p, 1);

	DrawNextLineToIndexView(text, "-- Surface --", p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcLCLIndex(kLCLCalcSurface), "LCL", 0), p, 1);
	double ELsur = kFloatMissing;
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcLFCIndex(kLCLCalcSurface, ELsur), "LFC", 0), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(ELsur, "EL", 0), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcCAPE500Index(kLCLCalcSurface), "CAPE", 0), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcCAPE500Index(kLCLCalcSurface, 3000), "0-3kmCAPE", 0), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcCAPE_TT_Index(kLCLCalcSurface, -10, -40), "-10-40CAPE", 0), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcCINIndex(kLCLCalcSurface), "CIN", 0), p, 1);
	DrawNextLineToIndexView(text, " ", p, 1);


	DrawNextLineToIndexView(text, "-- 500m mix --", p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcLCLIndex(kLCLCalc500m2), "LCL", 0), p, 1);
	double EL500m2 = kFloatMissing;
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcLFCIndex(kLCLCalc500m2, EL500m2), "LFC", 0), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(EL500m2, "EL", 0), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcCAPE500Index(kLCLCalc500m2), "CAPE", 0), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcCAPE500Index(kLCLCalc500m2, 3000), "0-3kmCAPE", 0), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcCAPE_TT_Index(kLCLCalc500m2, -10, -40), "-10-40CAPE", 0), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcCINIndex(kLCLCalc500m2), "CIN", 0), p, 1);
	DrawNextLineToIndexView(text, " ", p, 1);


	DrawNextLineToIndexView(text, "-- Most unstable --", p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcLCLIndex(kLCLCalcMostUnstable), "LCL", 0), p, 1);
	double ELunst = kFloatMissing;
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcLFCIndex(kLCLCalcMostUnstable, ELunst), "LFC", 0), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(ELunst, "EL", 0), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcCAPE500Index(kLCLCalcMostUnstable), "CAPE", 0), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcCAPE_TT_Index(kLCLCalcMostUnstable, -10, -40), "-10-40CAPE", 0), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcCINIndex(kLCLCalcMostUnstable), "CIN", 0), p, 1);
	DrawNextLineToIndexView(text, " ", p, 1);

	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcBulkShearIndex(0, 6), "0-6km BS", 1), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcBulkShearIndex(0, 1), "0-1km BS", 1), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcSRHIndex(0, 3), "0-3km SRH", 1), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcSRHIndex(0, 1), "0-1km SRH", 1), p, 1);
	DrawNextLineToIndexView(text, itsFirstSoundingData.Get_U_V_ID_IndexText("L-motion", kLeft), p, 1);
	DrawNextLineToIndexView(text, itsFirstSoundingData.Get_U_V_ID_IndexText("MeanWind", kCenter), p, 1);
	DrawNextLineToIndexView(text, itsFirstSoundingData.Get_U_V_ID_IndexText("R-motion", kRight), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcWSatHeightIndex(1500), "WS1500m", 1), p, 1);
	DrawNextLineToIndexView(text, GetIndexText(itsFirstSoundingData.CalcThetaEDiffIndex(0, 3), "0-3kmThetaE", 1), p, 1);
}


void NFmiTempView::DrawOneLevelStringData(NFmiText &text, NFmiPoint &p, int levelIndex, std::deque<float> &pVec, std::deque<float> &tVec, std::deque<float> &tdVec, std::deque<float> &zVec, std::deque<float> &wsVec, std::deque<float> &wdVec)
{
	// Jos luotauksesta lˆytyy mik‰ tahansa arvo jostain levelilt‰, piirret‰‰n sen rivin tiedot
	if(pVec[levelIndex] == kFloatMissing && tVec[levelIndex] == kFloatMissing && zVec[levelIndex] == kFloatMissing && wsVec[levelIndex] == kFloatMissing && wdVec[levelIndex] == kFloatMissing)
		return; // Luotausdataan voi tulla t‰ysin puuttuvia leveleit‰, skipataan ne!!!
	std::string str = "";
	str += GetStrValue(pVec[levelIndex], 0, 4);
	str += " ";
	str += GetStrValue(tVec[levelIndex], 1, 5);
	str += " ";
	float TdValue = tdVec[levelIndex];
	if(TdValue <= -100)
		str += GetStrValue(tdVec[levelIndex], 0, 5); // jos kastepiste on
	else
		str += GetStrValue(tdVec[levelIndex], 1, 5);
	str += " ";
	str += GetStrValue(zVec[levelIndex], 0, 5);
	str += " ";
	str += GetStrValue(wsVec[levelIndex], 0, 2);
	str += " ";
	str += GetStrValue(wdVec[levelIndex], 0, 3);
	DrawNextLineToIndexView(text, str, p, 0.8);
}

static NFmiString GetStationsShortName(NFmiSoundingDataOpt1 &theData)
{
    if(theData.Location().GetIdent() == 0)
        return ::GetNameText(theData);
    else
        return theData.Location().GetName();
}

void NFmiTempView::DrawSoundingInTextFormat(NFmiSoundingDataOpt1 &theData)
{
	// Muista: jos t‰m‰ piirret‰‰n muuten kuin kaiken muun j‰lkeen Draw-metodissa,
	// ei clippausta saa muuttaa kesken kaiken.
	const auto& textualDataRect = itsTempViewDataRects.getTextualSoundingDataSideViewRect();
    ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &textualDataRect);

    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    // piirret‰‰n vain 1. luotaukseen liittyv‰t luotaus arvot ja ne piirret‰‰n samalla v‰rilla kuin 1. luotaus
	itsDrawingEnvironment->SetFrameColor(mtaTempSystem.SoundingColor(0));

	NFmiPoint p(textualDataRect.TopLeft());
	p.X(p.X() + itsToolBox->SX(2)); // siirret‰‰n teksti‰ pikkusen oikealle p‰in
	std::deque<float> &pVec = theData.GetParamData(kFmiPressure);
	std::deque<float> &tVec = theData.GetParamData(kFmiTemperature);
	std::deque<float> &tdVec = theData.GetParamData(kFmiDewPoint);
	std::deque<float> &zVec = theData.GetParamData(kFmiGeopHeight);
	std::deque<float> &wsVec = theData.GetParamData(kFmiWindSpeedMS);
	std::deque<float> &wdVec = theData.GetParamData(kFmiWindDirection);
	NFmiString str(::GetStationsShortName(theData));
	int fontSize = boost::math::iround(itsTempViewDataRects.getTextualSoundingDataFontSize() * itsDrawSizeFactor.Y());
	if(itsToolBox->GetDC()->IsPrinting())
		fontSize = boost::math::iround(fontSize * 0.9); // printatessa pit‰‰ fonttia hieman jostain syyst‰ pienent‰‰, koska muuten tekstit eiv‰t mahdu kokonaisuudessaan ruutuun
	itsDrawingEnvironment->SetFontSize(NFmiPoint(fontSize, fontSize));
	NFmiText text(p, NFmiString(""), 0, itsDrawingEnvironment);
	DrawNextLineToIndexView(text, str, p, 0.0); // Ei vied‰ 1. rivi‰ ollenkaan alasp‰in, koska muuten j‰‰ tyhj‰‰ yl‰osaan (en ymm‰rr‰ t‰t‰ kohtaa, jos laitan kommenttiin, rivi siirtyy alas kuitenkin)

	str = "P    T     Td    Z     WS WD";
	DrawNextLineToIndexView(text, str, p, 0.8);

	int ssize = static_cast<int>(pVec.size());
    if(mtaTempSystem.GetSoundingViewSettingsFromWindowsRegisty().SoundingTextUpward())
    {
        for(int i = ssize - 1; i >= 0; i--)
            DrawOneLevelStringData(text, p, i, pVec, tVec, tdVec, zVec, wsVec, wdVec);
    }
    else
    {
	    for(int i=0; i < ssize; i++)
            DrawOneLevelStringData(text, p, i, pVec, tVec, tdVec, zVec, wsVec, wdVec);
    }
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

	checkedVector<double> &values = mtaTempSystem.PressureValues();
	checkedVector<double>::const_iterator endIt = values.end();
    double x1 = usedDataRect.Left();
    double x2 = usedDataRect.Right();
	// # Y-axel
	std::vector<LineLabelDrawData> lineLabels;
	for(checkedVector<double>::const_iterator it = values.begin(); it != endIt;  ++it)
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
		double extraOffset = itsToolBox->SX(extraOffsetInPixels);
		labelInfo.ClipWithDataRect(true);
		labelInfo.TextAlignment(kRight);
        NFmiTempLineInfo lineInfo;
		lineInfo.DrawLine(false);
		lineInfo.Color(NFmiColor(0.f, 0.f, 0.f)); // t‰m‰ on myˆs label v‰ri
		lineInfo.Thickness(boost::math::iround(2 * itsDrawSizeFactor.X()));

		double tickMarkWidth = itsToolBox->SX(boost::math::iround(6 * itsDrawSizeFactor.X()));
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
			NFmiText txt1(NFmiPoint(unitStringX + moveLabelRelatively.X() - extraOffset, unitStringY), "FL", 0, &envi);
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

		double tickMarkWidth = itsToolBox->SX(boost::math::iround(6 * itsDrawSizeFactor.X()));
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
			NFmiText txt1(NFmiPoint(unitStringX + moveLabelRelatively.X(), unitStringY), "KM", 0, itsDrawingEnvironment);
			itsToolBox->Convert(&txt1);
		}
	}
}

// Piirt‰‰ tiivistys juova sapluunaan luotauksen kosteus arvoja.
// piirret‰‰n annetulla paine v‰lill‰. Arvot piirret‰‰n annetun
// mixingratio viivan oikealle puolelle ja paineen avulla lasketaan korkeus
// mihin 'label' laitetaan.
void NFmiTempView::DrawCondensationTrailRHValues(NFmiSoundingDataOpt1 &theData, double startP, double endP, double theMixRatio)
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
		double xShift = itsToolBox->SX(boost::math::iround(8 * itsDrawSizeFactor.Y()));
		double yShift = itsToolBox->SX(boost::math::iround(fontSize/2.));
		NFmiPoint moveLabelRelatively(xShift, -yShift);
		NFmiString unitStr("%");
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
		checkedVector<double> values;
		values.push_back(0.11);
		values.push_back(0.135);
		values.push_back(0.17);
		values.push_back(0.32);
		double startP = CalcPressureAtHeight(6); // aloitetaan viivan piirto 6 km:sta

		double deltaP = -15;
		DrawMixingRatio(labelInfo, lineInfo, values, startP, 100, deltaP, &envi);

		// piirret‰‰n sitten tod. n‰k. labelit viivoihin (jotka ovat eri juttu kuin piirretyt mixing ratio arvot)
		labelInfo.DrawLabelText(true);
		labelInfo.FontSize(boost::math::iround(16 * itsDrawSizeFactor.Y()));
		envi.SetFontSize(NFmiPoint(labelInfo.FontSize(), labelInfo.FontSize()));
		envi.BoldFont(true);
		labelInfo.StartPointPixelOffSet(NFmiPoint(-5* itsDrawSizeFactor.X(), 0 * itsDrawSizeFactor.Y()));
		checkedVector<double> probValues;
		probValues.push_back(0);
		probValues.push_back(40);
		probValues.push_back(70);
		probValues.push_back(100);

		NFmiPoint moveLabelRelatively(CalcReltiveMoveFromPixels(itsToolBox, labelInfo.StartPointPixelOffSet()));
		checkedVector<double>::const_iterator itProb = probValues.begin();
		checkedVector<double>::const_iterator endIt = values.end();
		double P = startP;
		for (checkedVector<double>::const_iterator it = values.begin(); it != endIt;  ++it, ++itProb)
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
								   const checkedVector<double> &theValues, double startP, double endP, double deltaStartLevelP,
								   NFmiDrawingEnvironment * /* theEnvi */ )
{
	if(theLineInfo.DrawLine() == false)
		return ;
 // GDI+ piirto koodia
	const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
	itsGdiPlusGraphics->SetClip(CtrlView::Relative2GdiplusRect(itsToolBox, dataRect));
	Gdiplus::PointF moveLabelInPixels(static_cast<REAL>(theLabelInfo.StartPointPixelOffSet().X()), static_cast<REAL>(theLabelInfo.StartPointPixelOffSet().Y()));

	checkedVector<double>::const_iterator endIt = theValues.end();
	double deltap = 50;
	std::vector<LineLabelDrawData> lineLabels;
	for (checkedVector<double>::const_iterator it = theValues.begin(); it != endIt;  ++it)
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
	checkedVector<double> &values = mtaTempSystem.DryAdiabaticValues();
	checkedVector<double>::const_iterator endIt = values.end();
	double deltap = 50;
	std::vector<LineLabelDrawData> lineLabels;
	for (checkedVector<double>::const_iterator it = values.begin(); it != endIt;  ++it)
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
	checkedVector<double> &values = mtaTempSystem.MoistAdiabaticValues();
	checkedVector<double>::const_iterator endIt = values.end();
	double deltap = 50;
	for (checkedVector<double>::const_iterator it = values.begin(); it != endIt;  ++it)
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

void NFmiTempView::DrawHelpLineLabel(const NFmiPoint &p1, const NFmiPoint &theMoveLabelRelatively, double theValue, const NFmiTempLabelInfo &theLabelInfo, NFmiDrawingEnvironment * theEnvi, const NFmiString &thePostStr)
{
	if(theLabelInfo.DrawLabelText())
	{
        ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), theLabelInfo.ClipWithDataRect());
		NFmiString str(NFmiStringTools::Convert<double>(theValue));
		str += thePostStr; // jos joku loppu liite on haluttu laittaa labeliin, se tulee t‰ss‰
		NFmiText txt(p1 + theMoveLabelRelatively, str, 0, theEnvi);
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
		double width = itsToolBox->SX(theHelpDotPixelSize);
		double height = itsToolBox->SY(theHelpDotPixelSize);
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

// Haetaan painepinta datalle pinta-dataa, ett‰ luotauksia voidaan leikata maanpinnalle.
static boost::shared_ptr<NFmiFastQueryInfo> GetPossibleGroundData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiProducer &theProducer, NFmiInfoOrganizer &theInfoOrganizer, NFmiInfoData::Type theDataType)
{
	boost::shared_ptr<NFmiFastQueryInfo> groundDataInfo;
    if(theInfo)
    {
        theInfo->FirstLevel();
        if(theInfo->Level()->LevelType() == kFmiPressureLevel || theInfo->Level()->LevelType() == kFmiHybridLevel)
        { // jos kyse on painepinta datasta ja lˆytyy vastaavan datan pinta data, josta lˆytyy paine aseman korkeudelta, fixataan luotaus dataa pintadatan avulla
            checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > infoVec = theInfoOrganizer.GetInfos(theDataType, true, theProducer.GetIdent());
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
        }
    }
	return groundDataInfo;
}

void NFmiTempView::DrawSoundingsInMTAMode(void)
{
    itsSoundingDataCacheForTooltips.clear();
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    int modelRunCount = mtaTempSystem.ModelRunCount();
	int index = 0;
	const NFmiMTATempSystem::Container &temps = mtaTempSystem.GetTemps();
	for(NFmiMTATempSystem::Container::const_iterator it = temps.begin() ; it != temps.end(); ++it)
	{
		for(const auto &selectedProducer : mtaTempSystem.SoundingComparisonProducers())
		{
			if(modelRunCount > 0)
			{
				int startIndex = -modelRunCount;
				for(int modelRunIndex = startIndex; modelRunIndex <= 0; modelRunIndex++)
				{
					double brightningFactor = CtrlView::CalcBrightningFactor(0, modelRunCount, modelRunIndex); // mit‰ isompi luku, sit‰ enemm‰n vaalenee (0-100), vanhemmat malliajot vaaleammalla
					DrawOneSounding(selectedProducer, *it, index, brightningFactor, modelRunIndex);
				}
			}
			else
			{
				DrawOneSounding(selectedProducer, *it, index, 0, 0);
			}
			index++;
		}
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

void NFmiTempView::DrawOneSounding(const NFmiMTATempSystem::SoundingProducer &theProducer, const NFmiMTATempSystem::TempInfo &theTempInfo, int theIndex, double theBrightningFactor, int theModelRunIndex)
{
    auto usedTempInfo(theTempInfo);
    auto useServerData = theProducer.useServer();
    usedTempInfo.Time(::GetUsedSoundingDataTime(itsCtrlViewDocumentInterface, theTempInfo));
    
	boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->InfoOrganizer()->FindSoundingInfo(theProducer, usedTempInfo.Time(), theModelRunIndex, NFmiInfoOrganizer::ParamCheckFlags(true));
	if(useServerData || info)
	{
        auto usedLocationWithName = ::GetSoundingLocation(info, theTempInfo, itsCtrlViewDocumentInterface->ProducerSystem());
        usedTempInfo.Latlon(usedLocationWithName.GetLocation());
		boost::shared_ptr<NFmiFastQueryInfo> groundDataInfo = ::GetPossibleGroundData(info, theProducer, *itsCtrlViewDocumentInterface->InfoOrganizer(), NFmiInfoData::kViewable);
		if(groundDataInfo == 0)
			groundDataInfo = ::GetPossibleGroundData(info, theProducer, *itsCtrlViewDocumentInterface->InfoOrganizer(), NFmiInfoData::kAnalyzeData);  // t‰m‰ on siksi ett‰ LAPS data on nyky‰‰n analyysi dataa, pit‰‰ korjata t‰m‰ infoorganizer sotku joskus kunnolla!!!

		NFmiSoundingDataOpt1 sounding;
		FillSoundingData(info, sounding, usedTempInfo.Time(), usedLocationWithName, groundDataInfo, theProducer);
        NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
        NFmiColor usedColor(mtaTempSystem.SoundingColor(theIndex));
		if(theBrightningFactor != 0)
			usedColor = NFmiColorSpaces::GetBrighterColor(usedColor, theBrightningFactor);
		itsDrawingEnvironment->SetFrameColor(usedColor);
		bool mainCurve = theModelRunIndex == 0;
        bool onSouthernHemiSphere = usedTempInfo.Latlon().Y() < 0;
		DrawSounding(sounding, theIndex, usedColor, mainCurve, onSouthernHemiSphere);
        itsSoundingDataCacheForTooltips.insert(std::make_pair(NFmiMTATempSystem::SoundingDataCacheMapKey(usedTempInfo, theProducer, theModelRunIndex), sounding));
	}
}

bool NFmiTempView::FillSoundingData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiSoundingDataOpt1 &theSoundingData, const NFmiMetTime &theTime, const NFmiLocation &theLocation, boost::shared_ptr<NFmiFastQueryInfo> &theGroundDataInfo, const NFmiMTATempSystem::SoundingProducer &theProducer)
{
    if(theProducer.useServer())
        return FillSoundingDataFromServer(theProducer, theSoundingData, theTime, theLocation);
    else
        return NFmiSoundingIndexCalculator::FillSoundingDataOpt1(theInfo, theSoundingData, theTime, theLocation, theGroundDataInfo);
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

static double CalcWindBarbXPos(const NFmiRect &theDataViewRect, const NFmiRect &theWindBarbSizeRect, int theIndex)
{ // indeksiin pit‰‰ lis‰t‰ yksi
	return theDataViewRect.Left() + theWindBarbSizeRect.Width() * 0.5 + theWindBarbSizeRect.Width() * 0.5 * (theIndex + 1);
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
	double width = itsToolBox->SX(boost::math::iround(gWindModificationAreaWidthInPixels * itsDrawSizeFactor.X()));

	DrawLine(NFmiPoint(xPos-width/2., dataRect.Top()), NFmiPoint(xPos-width/2., dataRect.Bottom()), drawSpecialLines, trueLineWidth, true, 0, &envi);
	DrawLine(NFmiPoint(xPos+width/2., dataRect.Top()), NFmiPoint(xPos+width/2., dataRect.Bottom()), drawSpecialLines, trueLineWidth, true, 0, &envi);
}

bool NFmiTempView::ModifySoundingWinds(const NFmiPoint &thePlace, unsigned long theKey, short theDelta)
{
	if(itsFirstSoundinWindBarbXPos != kFloatMissing)
	{
		double width = itsToolBox->SX(gWindModificationAreaWidthInPixels);
		const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
		NFmiRect windModRect(itsFirstSoundinWindBarbXPos - width/2., dataRect.Top(), itsFirstSoundinWindBarbXPos + width/2., dataRect.Bottom());
		if(windModRect.IsInside(thePlace))
		{ // jos oltiin tarpeeksi l‰hell‰ 1. soundingdatan tuuliviiri rivistˆ‰, muokataan l‰hinn‰ olevaa viiri‰
			float P = static_cast<float>(y2p(thePlace.Y()));
			bool windDirModified = theKey & kCtrlKey;
			FmiParameterName parId = windDirModified ? kFmiWindDirection : kFmiWindSpeedMS;
			float addValue = windDirModified ? 10.f : 1.f;
			if(theDelta < 0)
				addValue = -addValue;
			float minValue = 0;
			float maxValue = windDirModified ? 360 : kFloatMissing;
			bool fCircularValue = windDirModified ? true : false;
			bool status = itsFirstSoundingData.Add2ParamAtNearestP(P, parId, addValue, minValue, maxValue, fCircularValue);
			itsFirstSoundingData.UpdateUandVParams();
			return status;
		}
	}
	return false;
}

void NFmiTempView::DrawHodograf(NFmiSoundingDataOpt1 & theData, int theIndex)
{
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    if(!mtaTempSystem.ShowHodograf())
		return ;
	if(mtaTempSystem.ShowOnlyFirstSoundingInHodograf() && theIndex > 0)
		return ;

	NFmiRect oldRect(itsToolBox->RelativeClipRect());
	itsToolBox->RelativeClipRect(mtaTempSystem.GetHodografViewData().Rect(), true);
	NFmiColor oldFillColor(itsDrawingEnvironment->GetFillColor());

	DrawHodografBase(theIndex);
	DrawHodografUpAndDownWinds(theData, theIndex);
	DrawHodografCurve(theData, theIndex);
	DrawHodografWindVectorMarkers(theData, theIndex);
	DrawHodografHeightMarkers(theData, theIndex);

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
void NFmiTempView::DrawHodografUpAndDownWinds(NFmiSoundingDataOpt1 & theData, int theIndex)
{
	double h1 = theData.GetValueAtPressure(kFmiGeopHeight, 850);
	double h2 = theData.GetValueAtPressure(kFmiGeopHeight, 300);
	if(h1 != kFloatMissing && h2 != kFloatMissing)
	{
		double u_cloud_layer = kFloatMissing;
		double v_cloud_layer = kFloatMissing;
		theData.CalcAvgWindComponentValues(h1, h2, u_cloud_layer, v_cloud_layer);
		if(u_cloud_layer != kFloatMissing && v_cloud_layer != kFloatMissing)
		{
			float maxWSPressure = theData.FindPressureWhereHighestValue(kFmiWindSpeedMS, 1100, 850);
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
					NFmiColor textColor(itsCtrlViewDocumentInterface->GetMTATempSystem().SoundingColor(theIndex));
					NFmiDrawingEnvironment markEnvi; // laitetaan oma envi ympyr‰lle
					markEnvi.SetFrameColor(markerFrameColor);
					markEnvi.SetFillColor(markerFillColor);
					markEnvi.EnableFill();
					long arrowHeadSize = boost::math::iround(3 * itsDrawSizeFactor.X());
					double markWidth = itsToolBox->SX(arrowHeadSize);
					double markHeight = itsToolBox->SY(arrowHeadSize);
					NFmiPoint scale(markWidth, markHeight);

					// piirr‰ nuoli (0, 0) -> (u_upwind, v_upwind) ja laita label "upwind"
					{
						double u_upwind = u_cloud_layer - u_llj;
						double v_upwind = v_cloud_layer - v_llj;

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
						DrawHodografTextWithMarker("U", static_cast<float>(u_upwind), static_cast<float>(v_upwind), textColor, markerFrameColor, markerFillColor, 5, 18, kLeft, kNone);
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
						DrawHodografTextWithMarker("D", static_cast<float>(u_downwind), static_cast<float>(v_downwind), textColor, markerFrameColor, markerFillColor, 5, 18, kLeft, kNone);
					}
				}
			}
		}
	}
}

void NFmiTempView::DrawHodografWindVectorMarkers(NFmiSoundingDataOpt1 & theData, int theIndex)
{
	NFmiColor markFillColor(0,0,0);
	NFmiColor markFrameColor(0,0,0);
	NFmiColor textColor(itsCtrlViewDocumentInterface->GetMTATempSystem().SoundingColor(theIndex));

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

void NFmiTempView::DrawHodografBase(int theIndex)
{
	// piirr‰ pohja laatikko fillill‰
	int fontSize = boost::math::iround(16 * itsDrawSizeFactor.Y());
	itsDrawingEnvironment->SetFrameColor(NFmiColor(0,0,0));
	itsDrawingEnvironment->SetFillColor(NFmiColor(0.99f,0.98f,0.92f));
	itsDrawingEnvironment->SetFontSize(NFmiPoint(fontSize, fontSize));

	if(theIndex == 0)
	{
		auto& hodografViewData = itsCtrlViewDocumentInterface->GetMTATempSystem().GetHodografViewData();
		const auto& hodografRect = hodografViewData.Rect();
		NFmiRectangle rec(hodografRect, 0, itsDrawingEnvironment);
		itsToolBox->Convert(&rec);

		// kirjoita hodografi teksti yl‰ nurkkaan
		itsToolBox->SetTextAlignment(kLeft);
		NFmiString titleStr(::GetDictionaryString("TempViewHodographTitle"));
		NFmiText titleTxt(hodografRect.TopLeft(), titleStr, 0, itsDrawingEnvironment);
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

		itsDrawingEnvironment->SetFrameColor(NFmiColor(0,0,0));
		// piirr‰ asteikot
		NFmiPoint leftCenter(hodografRect.Left(), hodografRect.Center().Y());
		NFmiPoint rightCenter(hodografRect.Right(), hodografRect.Center().Y());
		NFmiPoint centerTop(hodografRect.Center().X(), hodografRect.Top());
		NFmiPoint centerBottom(hodografRect.Center().X(), hodografRect.Bottom());

		NFmiLine line1(leftCenter, rightCenter, 0, itsDrawingEnvironment);
		itsToolBox->Convert(&line1);
		NFmiLine line2(centerTop, centerBottom, 0, itsDrawingEnvironment);
		itsToolBox->Convert(&line2);

		double tickHeightRel = itsToolBox->SY(boost::math::iround(3 * itsDrawSizeFactor.Y()));
		double tickWidthRel = itsToolBox->SX(boost::math::iround(3 * itsDrawSizeFactor.X()));

		itsToolBox->SetTextAlignment(kCenter);
		for(double x = -hodografScaleMaxValue; x <= hodografScaleMaxValue; x += 10)
		{ // piirret‰‰n u-akselin sakarat
			NFmiPoint relP(GetRelativePointFromHodograf(x, 0));
			NFmiPoint uP1(relP.X(), relP.Y()-tickHeightRel);
			NFmiPoint uP2(relP.X(), relP.Y()+tickHeightRel);
			NFmiLine lineU(uP1, uP2, 0, itsDrawingEnvironment);
			itsToolBox->Convert(&lineU);
			std::string str(NFmiStringTools::Convert<double>(x));
			NFmiText txt(uP2, str.c_str(), 0, itsDrawingEnvironment);
			itsToolBox->Convert(&txt);
		}
		itsToolBox->SetTextAlignment(kLeft);
		double yShift = itsToolBox->SY(fontSize/2);
		for(double y = -hodografScaleMaxValue; y <= hodografScaleMaxValue; y += 10)
		{ // piirret‰‰n v-akselin sakarat
			NFmiPoint relP(GetRelativePointFromHodograf(0, y));
			NFmiPoint vP1(relP.X()-tickWidthRel, relP.Y());
			NFmiPoint vP2(relP.X()+tickWidthRel, relP.Y());
			NFmiLine lineV(vP1, vP2, 0, itsDrawingEnvironment);
			itsToolBox->Convert(&lineV);
			if(y != 0)
			{
				std::string str(NFmiStringTools::Convert<double>(y));
				NFmiPoint txtP(vP2.X(), vP2.Y() - yShift);
				NFmiText txt(txtP, str.c_str(), 0, itsDrawingEnvironment);
				itsToolBox->Convert(&txt);
			}
		}

		// pit‰‰ viel‰ piirt‰‰ laatikon reunat, koska ne ovat saattaneet sotkeentua
		NFmiRectangle rec2(hodografRect, 0, itsDrawingEnvironment);
		itsToolBox->Convert(&rec2);
	}
}

void NFmiTempView::DrawHodografCurve(NFmiSoundingDataOpt1 &theData, int theIndex)
{
	// piirr‰ itse hodografi k‰yr‰
	NFmiColor soundingColor(itsCtrlViewDocumentInterface->GetMTATempSystem().SoundingColor(theIndex));
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

	double markWidth = itsToolBox->SX(boost::math::iround(theMarkerSizeInPixel * itsDrawSizeFactor.X()));
	double markHeight = itsToolBox->SY(boost::math::iround(theMarkerSizeInPixel * itsDrawSizeFactor.Y()));
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
		NFmiText txtObject(relP, theText.c_str(), 0, &fontEnvi);
		itsToolBox->Convert(&txtObject);
	}
}

void NFmiTempView::DrawHodografHeightMarkers(NFmiSoundingDataOpt1 &theData, int theIndex)
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
	NFmiColor textColor(itsCtrlViewDocumentInterface->GetMTATempSystem().SoundingColor(theIndex));
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

void NFmiTempView::DrawSounding(NFmiSoundingDataOpt1 &theData, int theIndex, const NFmiColor &theUsedSoundingColor, bool fMainCurve, bool onSouthernHemiSphere)
{
	if(theIndex == 0 && fMainCurve) // eka luotaus laitetaan talteen indeksi laskuja varten
	{
		if(fMustResetFirstSoundingData || !itsFirstSoundingData.IsSameSounding(theData)) // laitetaan haettu luotausdata 'firstiksi' vain jos se on muuttunut
		{
			fMustResetFirstSoundingData = false;
			itsFirstSoundingData = theData; // jos ei ole muuttunut (aika/origtime/tuottaja/paikka), sit‰ on saatettu modifioida ja muutoksia ei heitet‰ pois
		}
		else
			theData = itsFirstSoundingData; // pit‰‰ sijoittaa t‰llessa ollut firsti piirrett‰v‰‰n theData:an
	}
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    if(theIndex >= mtaTempSystem.MaxTempsShowed())
		return ; // lopetetaan kun on piirretty maksi m‰‰r‰ luotauksia yhteen kuvaan
	NFmiDrawingEnvironment* envi = itsDrawingEnvironment;
	envi->SetPenSize(NFmiPoint(1 * itsDrawSizeFactor.X(), 1 * itsDrawSizeFactor.Y()));
	envi->SetFrameColor(theUsedSoundingColor);
	itsToolBox->UseClipping(true); // laitetaan clippaus taas p‰‰lle (huonoa koodia, mutta voi voi)

    DrawSecondaryData(theData, theIndex, theUsedSoundingColor);

    if(fMainCurve)
        DrawHodograf(theData, theIndex);

	if(theIndex == 0 && fMainCurve)
		DrawAllLiftedAirParcels(itsFirstSoundingData); // piirret‰‰n ilmapakettien nostot vain 1. luotaukseen

	// HUOM! ensin piirret‰‰n kastepiste ja sitten l‰mpˆtila, koska j‰lkimm‰isen‰ piirretty
	// voi peitt‰‰ allleen toisen piirrot ja koska l‰mpˆtila on t‰rke‰mpi, sen pit‰‰ tulla pintaan.
	// P‰‰lle piirto tarkoittaa mm. ett‰ katkoviivat peitt‰v‰t alleen yhten‰isen viivan!!

	envi->SetFrameColor(theUsedSoundingColor);
	itsToolBox->UseClipping(true); // laitetaan clippaus taas p‰‰lle (huonoa koodia, mutta voi voi)

	{
        NFmiTempLineInfo lineInfo = mtaTempSystem.DewPointLineInfo();
		lineInfo.Color(theUsedSoundingColor);
		lineInfo.Thickness(boost::math::iround(lineInfo.Thickness() * itsDrawSizeFactor.X() * ExtraPrintLineThicknesFactor(true)));
		DrawTemperatures(theData, kFmiDewPoint, lineInfo);
	}

	{
        NFmiTempLineInfo lineInfo = mtaTempSystem.TemperatureLineInfo();
		lineInfo.Color(theUsedSoundingColor);
		lineInfo.Thickness(boost::math::iround(lineInfo.Thickness() * itsDrawSizeFactor.X() * ExtraPrintLineThicknesFactor(true)));
		DrawTemperatures(theData, kFmiTemperature, lineInfo);
	}

	// Draw height values
	if(fMainCurve)
		DrawHeightValues(theData, theIndex);

	// laitetaan takaisin 'solid' kyn‰
	envi->SetFillPattern(FMI_SOLID);
	envi->SetPenSize(NFmiPoint(1, 1));
	DrawWind(theData, theIndex, onSouthernHemiSphere);

	if(fMainCurve)
	{
		DrawLCL(theData, theIndex, kLCLCalcSurface);
		DrawLCL(theData, theIndex, kLCLCalc500m2);
		DrawLCL(theData, theIndex, kLCLCalcMostUnstable);
		DrawTrMw(theData, theIndex);
	}
	if(theIndex == 0 && fMainCurve && mtaTempSystem.ShowCondensationTrailProbabilityLines())
		DrawCondensationTrailRHValues(theData, 400, 200, 0.32); // 0.32 on viimeinen apuviiva mik‰ piirret‰‰n, arvot laitetaan sen oikealle puolelle

	if(fMainCurve)
		DrawStationInfo(theData, theIndex);
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
void NFmiTempView::DrawTrMw(NFmiSoundingDataOpt1 &theData, int theIndex)
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
	::DrawGdiplusStringVector(*itsGdiPlusGraphics, lineLabels, labelInfo, CtrlView::Relative2GdiplusRect(itsToolBox, itsTempViewDataRects.getSoundingCurveDataRect()), itsCtrlViewDocumentInterface->GetMTATempSystem().SoundingColor(theIndex));
}

void NFmiTempView::DrawLCL(NFmiSoundingDataOpt1 &theData, int theIndex, FmiLCLCalcType theLCLCalcType)
{
	if(theIndex != 0) // piirret‰‰n vain 1. luotaukseen
		return ;

    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    const NFmiTempLineInfo &lineInfo = ::GetAirParcelLineInfo(mtaTempSystem, theLCLCalcType);
	const NFmiTempLabelInfo &labelInfo = ::GetAirParcelLabelInfo(mtaTempSystem, theLCLCalcType);
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
				double lclLineLength = itsToolBox->SX(30);
				NFmiPoint p1(x - lclLineLength/2., y);
				NFmiPoint p2(x + lclLineLength/2., y);
				NFmiPoint p3(x + lclLineLength/1.8, y);
				p3.Y(p3.Y() - itsToolBox->SY(static_cast<long>(itsDrawingEnvironment->GetFontHeight()/2.)));
				DrawLine(p1, p2, drawSpecialLines, trueLineWidth, false, 0, itsDrawingEnvironment);
				NFmiString str("LCL (");
				str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(pLCL, 0);
				str += ", ";
				str += NFmiValueString::GetStringWithMaxDecimalsSmartWay(T, 1);
				str += ")";
				NFmiText txt(p3, str, 0, itsDrawingEnvironment);
				itsToolBox->Convert(&txt);
			}
		}
	}
}

// piirret‰‰n paine asteikon viereen luotauksesta korkeus arvoja
void NFmiTempView::DrawHeightValues(NFmiSoundingDataOpt1 &theData, int theIndex)
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
	NFmiPoint moveLabelRelatively(CalcReltiveMoveFromPixels(itsToolBox, labelInfo.StartPointPixelOffSet()));

	if(mtaTempSystem.DrawOnlyHeightValuesOfFirstDrawedSounding() && theIndex == 0 || mtaTempSystem.DrawOnlyHeightValuesOfFirstDrawedSounding() == false)
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
						NFmiString str(NFmiStringTools::Convert<int>(static_cast<int>(gValue)));
						str += "m";
						NFmiText txt(p, str, 0, itsDrawingEnvironment);
						itsToolBox->Convert(&txt);
						lastPrintedTextY = currentTextY;
					}
				}
			}
			itsToolBox->UseClipping(true); // clippaus laitettava uudestaan p‰‰lle
		}
	}
}

void NFmiTempView::DrawStationInfo(NFmiSoundingDataOpt1 &theData, int theIndex)
{
	static const NFmiString maxNameText("1: E00/05.12 62∞23'N, 25∞41'E    ");

    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    if(!mtaTempSystem.DrawLegendText())
		return ;

	// laitetaan piirto ominaisuudet p‰‰lle
	FmiDirection oldAligment = itsToolBox->GetTextAlignment();
	itsToolBox->SetTextAlignment(static_cast<FmiDirection>(kLeft + 1000)); // + 1000 SUPERPIKAVIRITYS!!!! (laittaa kirjaimen alle pohjav‰ri‰)
																		// tarvitaan luotaus legendan kirjoittamisessa
	NFmiPoint fontSize = mtaTempSystem.LegendTextSize();
	fontSize = ScaleOffsetPoint(fontSize);
	itsDrawingEnvironment->SetFontSize(fontSize);
	double maxNameLength = maxNameText.GetLen();
	double maxNameLengthRel = itsToolBox->MeasureText(maxNameText);

	double mtaModeIndexFactor = 2;

	NFmiString locationNameStr;
	locationNameStr += NFmiValueString::GetStringWithMaxDecimalsSmartWay(theIndex+1, 0);
	locationNameStr += ": ";
	locationNameStr += ::GetNameText(theData);
	for(int i = locationNameStr.GetLen(); i < maxNameLength; i++)
		locationNameStr += " "; // t‰ytet‰‰n nime‰ spaceilla, ett‰ pohjav‰ritys peitt‰‰ saman alan jokaiselle 'nimelle'

	// LAITA MY÷S TƒMƒ INDEKSI JUTTU KUNTOON!!!!!!!
	long index = theIndex;//GetData()->GetSoundings()->GetColourIndex();
	itsDrawingEnvironment->SetFrameColor(mtaTempSystem.SoundingColor(index));
	NFmiDrawingEnvironment* envi = itsDrawingEnvironment;
	double fontY = itsToolBox->SY(envi->GetFontHeight());

	const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
	NFmiPoint point = dataRect.TopRight();
	point.X(point.X() - maxNameLengthRel * 1.35);
	point.Y(point.Y() + fontY * 0.95 + 1.0 * index * mtaModeIndexFactor * fontY);
    point.X(std::max(point.X(), dataRect.Left()));

	{ // vain 1. aseman aika kirjataan (paitsi MTA moodissa), loput asemat tulevat sitten allekkain
		NFmiMetTime time(theData.Time());
		NFmiString timestr(time.ToStr(::GetDictionaryString("TempViewLegendTimeFormat")));
		for(int i = timestr.GetLen(); i < maxNameLength; i++)
			timestr += " "; // t‰ytet‰‰n nime‰ spaceilla, ett‰ pohjav‰ritys peitt‰‰ saman alan jokaiselle 'nimelle'
		NFmiPoint timePoint(point);
		timePoint.Y(point.Y() - fontY);
		NFmiText text2(timePoint, timestr, 0, envi);
		itsToolBox->Convert(&text2);
	}
	NFmiText text1(point, locationNameStr, 0, envi);
	itsToolBox->Convert(&text1);

	itsToolBox->SetTextAlignment(oldAligment);
}

static NFmiRect gMissingRect(0,0,0,0);

void NFmiTempView::DrawWind(NFmiSoundingDataOpt1 &theData, int theIndex, bool onSouthernHemiSphere)
{
	itsFirstSoundinWindBarbXPos = kFloatMissing;
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    if(!mtaTempSystem.DrawWinds())
		return ;

	itsDrawingEnvironment->EnableFill();
	NFmiColor oldFillColor(itsDrawingEnvironment->GetFillColor());
	try
	{
		itsDrawingEnvironment->SetFillColor(itsDrawingEnvironment->GetFrameColor());
		double usedPenSize = 2.0;
		if(itsToolBox->GetDC()->IsPrinting())
			usedPenSize = 1.3; // tehd‰‰n printtausta varten v‰h‰n ohuempaa viivaa
		itsDrawingEnvironment->SetPenSize(NFmiPoint(usedPenSize * itsDrawSizeFactor.X(), usedPenSize * itsDrawSizeFactor.Y())); // tehd‰‰n v‰h‰n paksumpaa viivaa, ett‰ n‰kyy paremmin
		std::deque<float> &wsValues = theData.GetParamData(kFmiWindSpeedMS);
		std::deque<float> &wdValues = theData.GetParamData(kFmiWindDirection);
		std::deque<float> &pressures = theData.GetParamData(kFmiPressure);
		std::deque<float> &heights = theData.GetParamData(kFmiGeomHeight);
		bool useHeight = (theData.PressureDataAvailable() == false && theData.HeightDataAvailable() == true);
		if(wsValues.size() > 1 && wsValues.size() == wdValues.size() && wsValues.size() == pressures.size() && wsValues.size() == heights.size())
		{
			int spaceOutFactor = itsCtrlViewDocumentInterface->SoundingViewWindBarbSpaceOutFactor();
			NFmiRect lastDrawnRect(gMissingRect);
			NFmiPoint point;
			NFmiPoint windVecSizeInPixels = mtaTempSystem.WindvectorSizeInPixels();
			windVecSizeInPixels = ScaleOffsetPoint(windVecSizeInPixels);
			NFmiRect windBarbRect(::CalcGeneralWindBarbRect(itsToolBox, windVecSizeInPixels));
			double xPos = ::CalcWindBarbXPos(itsTempViewDataRects.getSoundingCurveDataRect(), windBarbRect, theIndex);
			if(theIndex == 0)
				itsFirstSoundinWindBarbXPos = xPos;
			int ssize = static_cast<int>(wsValues.size());
			for(int i=0;i<ssize; i++)
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
						NFmiWindBarb(ws
									,wd
									,windBarbRect
									,itsToolBox
                                    ,onSouthernHemiSphere
									,0.3f // ???
									,0.3f // ???
									,0
									,itsDrawingEnvironment).Build();
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
	catch(...)
	{
		// varmistetaan ett‰ saadaan palautettua vanha filli v‰ri vaikka poikkeus lent‰isi
	}
	itsDrawingEnvironment->SetFillColor(oldFillColor);
}

// piirt‰‰ sek‰ l‰mpp‰ri ett‰ kastepiste viivat
// HUOM! t‰nne tullessa theLineInfo on jo skaalattu printtauksen suhteen jos tarpeen
void NFmiTempView::DrawTemperatures(NFmiSoundingDataOpt1 &theData, FmiParameterName theParId, const NFmiTempLineInfo &theLineInfo)
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
					points.push_back(PointF(static_cast<REAL>(x * itsGdiplusScale.X()), static_cast<REAL>(y * itsGdiplusScale.Y())));
					doingFirstTimeChecks = false;
				}
				else // piirret‰‰n p‰tk‰ mik‰ on vektorissa tallessa
				{
					::DrawGdiplusCurve(*itsGdiPlusGraphics, points, theLineInfo, GetUsedCurveDrawSmoothingMode(), IsPrinting(), false);
					points.clear();
				}
			}

			if(t != kFloatMissing && (useHeight ? h != kFloatMissing : p != kFloatMissing))
				consecutiveMissingValues = 0;
			else
				consecutiveMissingValues++;
		}
		::DrawGdiplusCurve(*itsGdiPlusGraphics, points, theLineInfo, GetUsedCurveDrawSmoothingMode(), IsPrinting(), false); // lopuksi viel‰ piirret‰‰n loputkin mit‰ on piirrett‰v‰‰
	}
	itsGdiPlusGraphics->ResetClip();
}

bool NFmiTempView::LeftButtonUp(const NFmiPoint &thePlace, unsigned long theKey)
{
    NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
    mtaTempSystem.LeftMouseDown(false);
	const auto& dataRect = itsTempViewDataRects.getSoundingCurveDataRect();
	if(dataRect.Bottom() < thePlace.Y() && dataRect.Left() < thePlace.X())
	{ // nyt on klikattu l‰mpp‰ri asteikkoa
		double change = 5;
		if(dataRect.Center().X() > thePlace.X())
            mtaTempSystem.TAxisStart(mtaTempSystem.TAxisStart() - change);
		else
            mtaTempSystem.TAxisEnd(mtaTempSystem.TAxisEnd() - change);
		return true;
	}
	else if(dataRect.Left() > thePlace.X() && dataRect.Bottom() > thePlace.Y())
	{ // nyt on klikattu paine asteikkoa
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
			return ModifySounding(itsFirstSoundingData, thePlace, theKey, kFmiDewPoint, 3);
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
	{ // nyt on klikattu l‰mpp‰ri asteikkoa
		double change = 5;
		if(dataRect.Center().X() > thePlace.X())
            mtaTempSystem.TAxisStart(mtaTempSystem.TAxisStart() + change);
		else
            mtaTempSystem.TAxisEnd(mtaTempSystem.TAxisEnd() + change);
		return true;
	}
	else if(dataRect.Left() > thePlace.X() && dataRect.Bottom() > thePlace.Y())
	{ // nyt on klikattu paine asteikkoa
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
		return ModifySounding(itsFirstSoundingData, thePlace, theKey, kFmiTemperature, 3);
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

bool NFmiTempView::ModifySounding(NFmiSoundingDataOpt1 &theSoundingData, const NFmiPoint &thePlace, unsigned long theKey, FmiParameterName theParam, int theDistToleranceInPixels)
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
bool NFmiTempView::QuickModifySounding(NFmiSoundingDataOpt1 &theSoundingData, FmiParameterName theParam, double P, double T, double Td)
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
	double moveX = itsToolBox->SX(boost::math::iround(2 * itsDrawSizeFactor.X()));
	NFmiDrawingEnvironment envi;
	envi.SetFontSize(NFmiPoint(18 * itsDrawSizeFactor.Y(), 18 * itsDrawSizeFactor.Y()));
	envi.EnableFill();
	envi.SetFrameColor(NFmiColor(0,0,0));
	envi.SetFillColor(NFmiColor(0.8f,0.8f,0.8f));

	const auto& animButtomRect = itsTempViewDataRects.getAnimationButtonRect();
	NFmiRectangle rec1(animButtomRect, 0, &envi);
	itsToolBox->Convert(&rec1);
	NFmiString str1("Anim.");
	NFmiText txt1(NFmiPoint(animButtomRect.TopLeft().X() + moveX, animButtomRect.TopLeft().Y()) , str1, 0, &envi);
	itsToolBox->Convert(&txt1);

	const auto& animStepButtomRect = itsTempViewDataRects.getAnimationStepButtonRect();
	NFmiRectangle rec2(animStepButtomRect, 0, &envi);
	itsToolBox->Convert(&rec2);
	NFmiString str2;
	int step = itsCtrlViewDocumentInterface->GetMTATempSystem().AnimationTimeStepInMinutes();
	if(step == 30)
		str2 += 'Ω';
	else if(step >= 60)
		str2 += NFmiStringTools::Convert<int>(step/60);
	str2 += "h";
	NFmiText txt2(NFmiPoint(animStepButtomRect.TopLeft().X() + moveX, animStepButtomRect.TopLeft().Y()), str2, 0, &envi);
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
    itsDrawingEnvironment->SetFrameColor(mtaTempSystem.SoundingColor(0));
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
	{ // jos ollaan indeksin‰ytto tilassa, lasketaan kursorin kohdalle arvoja ja laitetaan ne indeksi ikkunaan
		NFmiPoint p(CalcStabilityIndexStartPoint());

		NFmiText text(p, NFmiString(""), 0, itsDrawingEnvironment);
		for(int i=0; i<4; i++)
			DrawNextLineToIndexView(text, "", p, 1, true, false); // n‰it‰ ei lis‰t‰ soundingIndex-stringiin

		DrawNextLineToIndexView(text, GetIndexText(temperature, "T", 1), p, 1, true, false);
		DrawNextLineToIndexView(text, GetIndexText(pressure, "P", 1), p, 1, true, false);
		DrawNextLineToIndexView(text, GetIndexText(Tpot, "dry", 1), p, 1, true, false);
		DrawNextLineToIndexView(text, GetIndexText(moistT, "moist", 1), p, 1, true, false);
		DrawNextLineToIndexView(text, GetIndexText(W, "mix", 1), p, 1, true, false);
	}

	if(dataRect.IsInside(thePlace))
	{
		if(KEYDOWN(VK_SHIFT)) // jos shift pohjassa piirret‰‰n apu viivoja
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
			return ModifySounding(itsFirstSoundingData, thePlace, theKey, kFmiTemperature, 3);
		else if(mtaTempSystem.LeftMouseDown())
			return ModifySounding(itsFirstSoundingData, thePlace, theKey, kFmiDewPoint, 3);
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

static std::string GetTooltipValueStr(const std::string &theParStr, NFmiSoundingDataOpt1 &soundingData, FmiParameterName theParId, int theMaxDecimalCount, float P)
{
	std::string str = theParStr;
	float value =  soundingData.GetValueAtPressure(theParId, P);
	if(value == kFloatMissing)
		str += " - ";
	else
		str += static_cast<char*>(NFmiValueString::GetStringWithMaxDecimalsSmartWay(value, theMaxDecimalCount));
	return str;
}

static std::string GetSoundingToolTipText(NFmiTempView::SoundingDataCacheMap &soundingDataCache, const NFmiMTATempSystem::ServerProducer &producer, const NFmiMTATempSystem::TempInfo &theTempInfo, int modelRunIndex, float P, int theZeroBasedIndex, bool doNormalString, const NFmiString &locationName)
{
	std::string str;
	if(doNormalString)
	{
		str += NFmiStringTools::Convert(theZeroBasedIndex+1);
		str += ": ";

		NFmiString timestr(theTempInfo.Time().ToStr(::GetDictionaryString("TempViewLegendTimeFormat")));
		str += static_cast<char*>(timestr);
		str += "\n";
		str += locationName;
		str += "\n";
	}

    auto soundingDataIter = soundingDataCache.find(NFmiMTATempSystem::SoundingDataCacheMapKey(theTempInfo, producer, modelRunIndex));
    if(soundingDataIter != soundingDataCache.end())
    {
        // sitten laitetaan interpoloidut arvot annetun korkeuden mukaan eri parametreille
        str += "P: ";
        str += static_cast<char*>(NFmiValueString::GetStringWithMaxDecimalsSmartWay(P, 1));
        str += ::GetTooltipValueStr(" T: ", soundingDataIter->second, kFmiTemperature, 1, P);
        str += ::GetTooltipValueStr(" Td: ", soundingDataIter->second, kFmiDewPoint, 1, P);
        str += ::GetTooltipValueStr(" WD: ", soundingDataIter->second, kFmiWindDirection, 0, P);
        str += ::GetTooltipValueStr(" WS: ", soundingDataIter->second, kFmiWindSpeedMS, 1, P);
        str += ::GetTooltipValueStr(" N: ", soundingDataIter->second, kFmiTotalCloudCover, 0, P);
        str += ::GetTooltipValueStr(" RH: ", soundingDataIter->second, kFmiHumidity, 1, P);
    }
    else
        str += " P: - T: - Td: - WD: - WS: -";
    return str;
}

std::string NFmiTempView::ComposeToolTipText(const NFmiPoint & theRelativePoint)
{
	std::string str;
    if(itsTempViewDataRects.getSoundingCurveDataRect().IsInside(theRelativePoint) || itsTempViewDataRects.getSecondaryDataFrame().IsInside(theRelativePoint))
    {
        NFmiMTATempSystem &mtaTempSystem = itsCtrlViewDocumentInterface->GetMTATempSystem();
        int modelRunCount = mtaTempSystem.ModelRunCount();
        float pressure = static_cast<float>(y2p(theRelativePoint.Y()));
        int index = 0;
        const NFmiMTATempSystem::Container &tempInfos = mtaTempSystem.GetTemps();
        for(const auto &constantLoopTempInfo : tempInfos)
        {
            NFmiMTATempSystem::TempInfo usedTempInfo = constantLoopTempInfo;
            for(const auto &selectedProducer : mtaTempSystem.SoundingComparisonProducers())
            {
                usedTempInfo.Time(::GetUsedSoundingDataTime(itsCtrlViewDocumentInterface, usedTempInfo));
                boost::shared_ptr<NFmiFastQueryInfo> info = itsCtrlViewDocumentInterface->InfoOrganizer()->FindSoundingInfo(selectedProducer, usedTempInfo.Time(), 0, NFmiInfoOrganizer::ParamCheckFlags(true));
                if(selectedProducer.useServer() || info)
                {
                    auto usedLocationWithName = ::GetSoundingLocation(info, usedTempInfo, itsCtrlViewDocumentInterface->ProducerSystem());
                    usedTempInfo.Latlon(usedLocationWithName.GetLocation());
                    str += "<b><font color=";
                    str += CtrlViewUtils::Color2HtmlColorStr(mtaTempSystem.SoundingColor(index));
                    str += ">";
                    str += ::GetSoundingToolTipText(itsSoundingDataCacheForTooltips, selectedProducer, usedTempInfo, modelRunCount, pressure, index, true, usedLocationWithName.GetName());
                    if(modelRunCount > 0 && NFmiDrawParam::IsModelRunDataType(info->DataType()))
                    { 
                        // lis‰t‰‰n edelliset malliajo -osio tooltippiin 
                        for(int modelRunIndex = -1; modelRunIndex >= -modelRunCount; modelRunIndex--)
                        {
                            str += "\n[";
                            str += NFmiStringTools::Convert(modelRunIndex);
                            str += "]\t";
                            str += ::GetSoundingToolTipText(itsSoundingDataCacheForTooltips, selectedProducer, usedTempInfo, modelRunIndex, pressure, index, false, usedLocationWithName.GetName());
                        }
                    }
                    str += "</font></b>";

                    str += "<br><hr color=red><br>"; // v‰liviiva
                    index++;
                }
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

bool NFmiTempView::FillSoundingDataFromServer(const NFmiMTATempSystem::SoundingProducer &theProducer, NFmiSoundingDataOpt1 &theSoundingData, const NFmiMetTime &theTime, const NFmiLocation &theLocation)
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
    NFmiString finalNameWithServerMarker = "(S) ";
    finalNameWithServerMarker += finalLocation.GetName();
    finalLocation.SetName(finalNameWithServerMarker);
    theSoundingData.Location(finalLocation);
    return status;
}

