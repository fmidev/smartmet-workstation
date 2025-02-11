//**********************************************************
// C++ Class Name : NFmiIsoLineView
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/ver104/ProjApp/zeditmap2/nfmiviews/NFmiIsoLineView.cpp
//
//
// GDPro Properties
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class
//  - GD Method         : UML ( 2.1.4 )
//  - GD System Name    : metedit nfmiviews
//  - GD View Type      : Class Diagram
//  - GD View Name      : Class Diagram
// ---------------------------------------------------
//  Author         : pietarin
//  Creation Date  : Mon - Nov 16, 1998
//
//
//  Description:
//   Draws the given grid-data in isoline form.
//   Only lines are drawn, not iso values.
//
//
//  Change Log:
// Changed 1999.08.31/Marko	Muutin isoviivan piirrossa arvojen piirto systeemin niin
//							, ett‰ vain yksi arvo piirret‰‰n.
//
//**********************************************************
#include "NFmiIsoLineView.h"
#include "NFmiToolBox.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiQueryData.h"
#include "NFmiDrawParam.h"
#include "NFmiAreaMaskList.h"
#include "NFmiValueLineList.h"
#include "NFmiDataModifierClasses.h"

#include "NFmiIsoLineData.h"
#include "ToolMasterDrawingFunctions.h"
#include "ToolMasterHelperFunctions.h"
#include "NFmiArea.h"
#include "NFmiValueString.h"
#include "NFmiValueLine.h"
#include "NFmiGrid.h"
#include "NFmiText.h"
#include "NFmiRectangle.h"
#include "NFmiDataHints.h"
#include "NFmiContourTree.h"
#include "NFmiPolyline.h"
#include "NFmiStereographicArea.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiGridPointCache.h"
#include "NFmiMilliSecondTimer.h"
#include "CtrlViewDocumentInterface.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "GraphicalInfo.h"
#include "NFmiQueryDataUtil.h"
#include "CtrlViewWin32Functions.h"
#include "NFmiFastInfoUtils.h"
#include "ToolMasterColorCube.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "catlog/catlog.h"
#include "EditedInfoMaskHandler.h"
#include "ToolBoxStateRestorer.h"
#include "SpecialDesctopIndex.h"
#include "CtrlViewFunctions.h"
#include "ForcedIsolineLogging.h"

#include "datautilities\DataUtilitiesAdapter.h"
#include "NFmiApplicationWinRegistry.h"
#include "NFmiVisualizationSpaceoutSettings.h"
#include "NFmiSmartToolModifier.h"

#include <limits>

#include "boost\math\special_functions\round.hpp"

// eli n. 30 x 30 (=900) hilasta ylˆsp‰in ei tehd‰ alikolmioita
static const int gMaxGridPointsThatImagineWillSubTriangulate = 900;

using namespace std;
using namespace Imagine;

static bool CoordComparison(const NFmiPathElement &first, const NFmiPathElement &second);
static void MakePossiblePathCombinations(list<NFmiPathData*> &thePathDataList);
static void CombinePaths(NFmiPathData &first, NFmiPathData &second);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// LabelBox -----------------------------------------

double LabelBox::itsPixelsPerMM_x = 1;
double LabelBox::itsPixelsPerMM_y = 1;

LabelBox::LabelBox(void)
{
}

LabelBox::LabelBox(float theFontHeight, float theIsoLineValue /*int theLabelLetterCount*/, boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiToolBox *theToolBox, NFmiDrawingEnvironment &theEnviroment)
{
    Init(theFontHeight, theIsoLineValue, theDrawParam, theToolBox, theEnviroment);
}


void LabelBox::Init(void)
{
    itsStrategy = 2;
    itsFontSize = 4; // [mm]. (T‰ll‰ hetkell‰ 4 pix on n. 1 mm);
    itsIsoLineValue = kFloatMissing;

    itsFontColor = NFmiColor(0, 0, 0);
    itsBoxFillColor = NFmiColor(1, 1, 0.75);
    itsBoxStrokeColor = NFmiColor(0, 0, 0);

    itsBoxStrokeWidth = 1;
    itsIsoLineLabelHeight = 5.f; //EL

    itsFontOpacity = 1;
    itsBoxFillOpacity = 1;
    itsBoxStrokeOpacity = 1;
}

void LabelBox::Init(float theFontHeight, float theIsoLineValue, boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiToolBox *theToolBox, NFmiDrawingEnvironment &theEnviroment)
{
    itsStrategy = 2; //EL: mist‰ t‰‰ revitt‰isiin?????????????????????
    itsFontSize = theFontHeight;  // [mm]

    itsIsoLineValue = theIsoLineValue;

    itsBoxFillColor = theDrawParam->IsolineLabelBoxFillColor();
    itsFontColor = theDrawParam->IsolineTextColor();

    if(theDrawParam->UseSingleColorsWithSimpleIsoLines())
    {
        itsBoxStrokeColor = theDrawParam->IsolineColor();
    }
    else
    {
        // FrameColor asetettu aikaisemmin, ks. Set*Envi()
        itsBoxStrokeColor = theEnviroment.GetFrameColor();
    }

    itsBoxStrokeWidth = theDrawParam->SimpleIsoLineWidth();
    itsIsoLineLabelHeight = theDrawParam->SimpleIsoLineLabelHeight();

    itsFontOpacity = 1;
    itsBoxFillOpacity = 1;
    itsBoxStrokeOpacity = 1;

    CalcLabelString();
    CalcLabelBoxRect(theToolBox);
}

void LabelBox::InitPixelPerMMRatio(NFmiToolBox * theGTB)
{
    static bool fInitialized = false;

    if(!fInitialized)
    {
        fInitialized = true;

        int screenWidthInMM = GetDeviceCaps(theGTB->GetDC()->GetSafeHdc(), HORZSIZE);
        int screenHeightInMM = GetDeviceCaps(theGTB->GetDC()->GetSafeHdc(), VERTSIZE);
        int screenWidthInPixels = GetDeviceCaps(theGTB->GetDC()->GetSafeHdc(), HORZRES);
        int screenHeightInPixels = GetDeviceCaps(theGTB->GetDC()->GetSafeHdc(), VERTRES);
        itsPixelsPerMM_x = static_cast<double>(screenWidthInPixels) / static_cast<double>(screenWidthInMM);
        itsPixelsPerMM_y = static_cast<double>(screenHeightInPixels) / static_cast<double>(screenHeightInMM);
    }
}

void LabelBox::CalcLabelString(void)
{
    string labelStr(static_cast<char*>(NFmiValueString::GetStringWithMaxDecimalsSmartWay(itsIsoLineValue, 2)));
    itsLabelString = labelStr;
}

const std::string& LabelBox::LabelString(void)
{
    return itsLabelString;
}

// Millimetri fonttipikseleiksi
double LabelBox::CalcFontWidthInPixels(void)
{
    return itsFontSize * itsPixelsPerMM_x * 1.9;
}

double LabelBox::CalcFontHeightInPixels(void)
{
    return itsFontSize * itsPixelsPerMM_y * 1.6;
}


double LabelBox::CalcWidthInPixels(void)
{
    return CalcFontWidthInPixels()*LabelString().size();
}

double LabelBox::CalcHeightInPixels(void)
{
    return CalcFontHeightInPixels();
}


// Fonttikoko suhteellisiksi koordinaateiksi
double LabelBox::CalcRelativeFontWidth(NFmiToolBox *theToolBox)
{
    return CalcFontWidthInPixels()*static_cast<double>(theToolBox->SX((long)1));
}

double LabelBox::CalcRelativeFontHeight(NFmiToolBox *theToolBox)
{
    return CalcFontHeightInPixels()*static_cast<double>(theToolBox->SY((long)1));
}

void LabelBox::CalcLabelBoxRect(NFmiToolBox *theToolBox)
{
    int labelLetterCount = static_cast<int>(LabelString().size());
    double wFactor = labelLetterCount == 1 ? 0.7 : 0.45;
    double labelRelativeBoxWidth = labelLetterCount*CalcRelativeFontWidth(theToolBox) * wFactor;
    double labelRelativeBoxHeight = CalcRelativeFontHeight(theToolBox) * 0.7;

    itsLabelBoxRect = NFmiRect(NFmiPoint(0., 0.), NFmiPoint(labelRelativeBoxWidth, labelRelativeBoxHeight));
}

const NFmiRect& LabelBox::LabelBoxRect(void) const
{
    return itsLabelBoxRect;
}

void LabelBox::Center(const NFmiPoint &theLabelCenterPoint)
{
    itsLabelBoxRect.Center(theLabelCenterPoint);
}

NFmiPoint LabelBox::Center(void)
{
    return itsLabelBoxRect.Center();
}

// LabelBox -----------------------------------------




std::mutex NFmiIsoLineView::sToolMasterOperationMutex;


//--------------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------------

NFmiIsoLineView::NFmiIsoLineView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
    , NFmiToolBox * theToolBox
    , boost::shared_ptr<NFmiDrawParam> &theDrawParam
    , FmiParameterName theParamId
    , NFmiPoint theOffSet
    , NFmiPoint theSize
    , int viewGridRowNumber
    , int viewGridColumnNumber)
:NFmiStationView(theMapViewDescTopIndex, theArea
        , theToolBox
        , theDrawParam
        , theParamId
        , theOffSet
        , theSize
        , viewGridRowNumber
        , viewGridColumnNumber)
, fDataOk(false)
    , itsIsolineValues()
    , itsTransparencyDrawBitmap(0)
    , itsTransparencyDrawOldBitmap(0)
    , itsTransparencyDrawDC(0)
    , itsTransparencyDrawBackupDC(0)
    , itsLastBitmapSize()
    , fDrawUsingTransparency(false)
{
    dataUtilitiesAdapter = make_unique<SmartMetDataUtilities::DataUtilitiesAdapter<>>();
}

NFmiIsoLineView::~NFmiIsoLineView(void)
{
    CtrlView::DestroyBitmap(&itsTransparencyDrawBitmap, DeleteTransparencyBitmap());
}

bool NFmiIsoLineView::IsToolMasterAvailable(void)
{
    return itsCtrlViewDocumentInterface->IsToolMasterAvailable();
}

bool NFmiIsoLineView::DeleteTransparencyBitmap()
{
    return true;
}

bool NFmiIsoLineView::IsMapViewCase()
{
    return true;
}

// valitsin l‰pin‰kyv‰ksi v‰riksi mahd. l‰helt‰ valkoista, mutta siiit‰ v‰h‰n poikkeavan, koska
// t‰m‰ l‰pin‰kyvyys asetus ei toimi. Eli t‰st‰ v‰rist‰ ei tule l‰pin‰kyv‰‰ (en osaa k‰ytt‰‰ Gdiplus-systeemej‰
// funktiossa CFmiGdiPlusHelpers::DrawBitmapToDC???).
// Valitsin tuon v‰rin siksi ett‰ se blendattuna melkein h‰vi‰‰ kokonaan n‰kyvist‰ eik‰ tummennat alle j‰‰vi‰ v‰rej‰,
// mutta ei toivottavasti sekoitu mihink‰‰ k‰ytettyyn valkoiseen v‰riin.
static const COLORREF gFullyTransparentColor = 0x00fefdfe; // joku random v‰ri joka saa edustaa t‰ysin l‰pin‰kyv‰‰ pohjav‰ri‰, joka annetaan l‰pin‰kyvyys kikkailussa pohjav‰riksi

void NFmiIsoLineView::PrepareForTransparentDraw(void)
{
    // ****** l‰pin‰kyv‰ kentt‰ ***********************************
    // t‰h‰n tulisi mahdollinen l‰pin‰kyvyyys jutun alku asettelut:
    // luo Cbitmap tarvittaessa, tee CDC joka k‰ytt‰‰ bitmapia
    // aseta CDC toolmasterille ja toolboxille ja jatka kuvan piirtoa.
    // ****** l‰pin‰kyv‰ kentt‰ ***********************************

    fDrawUsingTransparency = false;
    if(itsDrawParam->Alpha() < 100.f && itsDrawParam->IsParamHidden() == false)
        fDrawUsingTransparency = true;
    if(fDrawUsingTransparency)
    {
        CRect clientRect = GetTrueClientRect();
        CSize clientAreaSize = clientRect.Size();
        itsTransparencyDrawDC = new CDC;
        itsTransparencyDrawDC->CreateCompatibleDC(itsToolBox->GetDC());
        if(itsTransparencyDrawBitmap == 0 || itsLastBitmapSize != clientAreaSize || itsCtrlViewDocumentInterface->Printing())
        {
            itsLastBitmapSize = clientAreaSize;
            CtrlView::MakeCombatibleBitmap(itsCtrlViewDocumentInterface->TransparencyContourDrawView(), &itsTransparencyDrawBitmap, itsLastBitmapSize.cx, itsLastBitmapSize.cy);
        }
        itsTransparencyDrawBackupDC = itsToolBox->GetDC();
        itsTransparencyDrawOldBitmap = itsTransparencyDrawDC->SelectObject(itsTransparencyDrawBitmap);
        // t‰ytet‰‰n tyhj‰ kuva jollain ihmev‰rill‰, joka on sitten loppu peliss‰ t‰ysin l‰pin‰kyv‰
        CRect fillArea(0, 0, itsLastBitmapSize.cx, itsLastBitmapSize.cy);
        CBrush aBrush(gFullyTransparentColor);
        itsTransparencyDrawDC->FillRect(fillArea, &aBrush);
        CtrlView::SetToolsDCs(itsTransparencyDrawDC, itsToolBox, clientRect, itsCtrlViewDocumentInterface->IsToolMasterAvailable());
    }
}

CRect NFmiIsoLineView::GetTrueClientRect()
{
    if(itsMapViewDescTopIndex > CtrlViewUtils::kFmiMaxMapDescTopIndex)
        return itsToolBox->GetClientRect();
    else
    {
        auto viewSizeInPixels = itsCtrlViewDocumentInterface->MapViewSizeInPixels(itsMapViewDescTopIndex);
        return CRect(0, 0, static_cast<int>(viewSizeInPixels.X()), static_cast<int>(viewSizeInPixels.Y()));
    }
}

void NFmiIsoLineView::EndTransparentDraw(void)
{
    // ****** l‰pin‰kyv‰ kentt‰ lopetus jutut ******************************
    // Kun CDC:ll‰ olevaan bitmapiin on piirretty, ota oikea CDC k‰yttˆˆn
    // toolboxiin ja toolmasteriin. Piirr‰ v‰liaikaisesta CDC:sta
    // kuva halutulla l‰pin‰kyvyydell‰ koko kuvan p‰‰lle (tee GdiplusHelperiin
    // funktio, joka ottaa CBitmapin ja convertoi sen Gdiplus::Bitmap:iksi
    // ja piirt‰‰ sen CDC:hen halutulla l‰pin‰kyvyydell‰).
    // Ota bitmap pois v‰liaika CDC:st‰ ja tuhoa CDC.
    // ****** l‰pin‰kyv‰ kentt‰ lopetus jutut ******************************

    if(fDrawUsingTransparency)
    {
        // juuri piirretty kuva on nyt itsTransparencyDrawDC:ssa ja siin‰ olevassa itsTransparencyDrawBitmap:issa
        CRect clientRect = itsToolBox->GetClientRect(); // oletus, t‰h‰n on jo laskettu oikea alue
        CtrlView::SetToolsDCs(itsTransparencyDrawBackupDC, itsToolBox, clientRect, itsCtrlViewDocumentInterface->IsToolMasterAvailable());
        itsTransparencyDrawBitmap = itsTransparencyDrawDC->SelectObject(itsTransparencyDrawOldBitmap);

        NFmiPoint clientAreaSize = itsCtrlViewDocumentInterface->MapViewSizeInPixels(itsMapViewDescTopIndex);
        Gdiplus::Bitmap gdiBitmap(*itsTransparencyDrawBitmap, 0);

        Gdiplus::Color aColor;
        gdiBitmap.GetPixel(10, 10, &aColor);
        COLORREF aColRef = aColor.ToCOLORREF();

        // HUOM! kuvan saa piirt‰‰ vain oman n‰ytˆn p‰‰lle, muuten jos ollaan moni ruutuisessa kokonaisn‰ytˆss‰,
        // tulee alpha piirtoa eri ruutujen p‰‰lle useina kerroksina ja t‰llˆin ensin piirrettyjen ruutujen
        // p‰‰lle tulee useita kerroksia 'lakkaa', joka tummentaa sen ruuden enemm‰n kuin pit‰isi.
        CRect mfcRect;
        itsToolBox->ConvertRect(GetFrame(), mfcRect);

        NFmiRect sourceRect(mfcRect.left, mfcRect.top, mfcRect.right, mfcRect.bottom);
        Gdiplus::RectF destRect(static_cast<Gdiplus::REAL>(mfcRect.left), static_cast<Gdiplus::REAL>(mfcRect.top), static_cast<Gdiplus::REAL>(mfcRect.Width()), static_cast<Gdiplus::REAL>(mfcRect.Height()));

        // alpha: 0 on t‰ysin l‰pin‰kyv‰, 0.5 = semi transparent ja 1.0 = opaque
        Gdiplus::REAL alpha = itsDrawParam->Alpha() / 100.f;
        CtrlView::DrawBitmapToDC_4(itsToolBox->GetDC(), gdiBitmap, sourceRect, destRect, true, NFmiImageAttributes(Gdiplus::Color(gFullyTransparentColor), alpha), itsGdiPlusGraphics);

        // lopuksi tuhotaan piirto otukset
        itsTransparencyDrawDC->DeleteDC();
        delete itsTransparencyDrawDC;
        itsTransparencyDrawDC = 0;
    }
    fDrawUsingTransparency = false;
}

//--------------------------------------------------------
// Draw
//--------------------------------------------------------
void NFmiIsoLineView::Draw(NFmiToolBox *theGTB)
{
    if(!theGTB)
        return;

    if(!IsParamDrawn())
    {
        CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging("NFmiIsoLineView doesn't draw anything, param was hidden", this);
        return;
    }

    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, __FUNCTION__);

    itsIsolineValues = kFloatMissing; // varmistetaan ett‰ t‰m‰ on tyhj‰‰ t‰ynn‰
    fGetSynopDataFromQ2 = false; // aluksi laitetaan falseksi, haku tehd‰‰n kerran PrepareForStationDraw-metodissa jossa onnistumisen kanssa lippu laitetaan p‰‰lle
    itsOptimizedGridPtr.reset();
    CalculateGeneralStationRect();
    MakeDrawedInfoVector();
    if(itsInfoVector.empty())
        return;
    SetMapViewSettings(itsInfoVector.front());
    UpdateCachedParameterName();
    SetupPossibleWindMetaParamData();

    if(itsDrawParam->Alpha() <= 0)
        return;

    bool tryStationGriddingDataDrawWithSeveralSourceDatas = (itsInfo->IsGrid() == false) && (itsInfoVector.size() > 1);
    if(fGetCurrentDataFromQ2Server == false)
    {
        if(!DataIsDrawable(itsInfo,itsTime) && tryStationGriddingDataDrawWithSeveralSourceDatas == false && IsSpecialMatrixDataDraw() == false)
        {
            if(itsInfo->DataType() != NFmiInfoData::kStationary && itsInfo->DataType() != NFmiInfoData::kMacroParam && itsInfo->DataType() != NFmiInfoData::kQ3MacroParam)
            {
                if(!NFmiFastInfoUtils::IsModelClimatologyData(itsInfo))
                    return;
            }
        }
    }
    if(itsInfo->DataType() == NFmiInfoData::kStationary)
        itsInfo->FirstTime(); // staattiselle datalle laitetaan ainoa aika paalle
    else
        itsInfo->Time(itsTime); // asetetaan aika kohdalleen, ett‰ hatchays toimii

    PrepareForTransparentDraw(); // jos piirto-ominaisuudessa on transparenssia, pit‰‰ tehd‰ kikka vitonen

    try
    {
        if(itsDrawParam)
        {
            DrawIsoLines();

            if(IsAccessoryStationDataDrawn())
            {
                DrawAllAccessoryStationData(); // piirret‰‰n aktiivisen parametrin hilapisteet
            }
        }
    }
    catch(...)
    {
    }

    EndTransparentDraw(); // jos piirrossa oli l‰pin‰kyvyytt‰, pit‰‰ viel‰ tehd‰ pari kikkaa ja siivota j‰ljet

    DrawObsComparison(); // vertailut havaintoihin piirret‰‰n vaikka data on piilossa
}

static double GetDifferenceDrawIsoLineGab(double theCurrentIsolineGap)
{
    double isolineGap = theCurrentIsolineGap;
    isolineGap /= 4.;
    if(isolineGap >= 1.)
        isolineGap = int(isolineGap);
    else if(isolineGap >= 0.1)
    {
        isolineGap *= 10.;
        isolineGap = int(isolineGap);
        isolineGap /= 10.;
    }
    else // if(isolineGap < 0.1)
    {
        isolineGap *= 100.;
        isolineGap = int(isolineGap);
        isolineGap /= 100.;
    }
    return isolineGap;
}

// n‰it‰ on kutsuttava pareittain RestoreUpDifferenceDrawing-metodin kanssa
void NFmiIsoLineView::SetUpDifferenceDrawing(boost::shared_ptr<NFmiDrawParam> &theUsedDrawParam)
{
    fDoDifferenceDrawSwitch = false;
    // piirret‰‰n toistaiseksi erotus vanhalla tavalla
    if(theUsedDrawParam->ShowDifferenceToOriginalData())
    {
        auto differenceDrawParam = itsCtrlViewDocumentInterface->GetDrawDifferenceDrawParam();
        if(differenceDrawParam)
        {
            double usedIsoLineStep = ::GetDifferenceDrawIsoLineGab(theUsedDrawParam->IsoLineGab());
            itsBackupDrawParamForDifferenceDrawing = theUsedDrawParam;
            theUsedDrawParam = differenceDrawParam;
            theUsedDrawParam->StationDataViewType(itsBackupDrawParamForDifferenceDrawing->StationDataViewType());
            theUsedDrawParam->IsoLineGab(usedIsoLineStep);
            int numOfShownDigits = 0;
            if(usedIsoLineStep >= 1.)
                numOfShownDigits = 0;
            else if(usedIsoLineStep >= 0.1)
                numOfShownDigits = 1;
            else
                numOfShownDigits = 2;
            theUsedDrawParam->IsoLineLabelDigitCount(numOfShownDigits);

            fDoDifferenceDrawSwitch = true;
        }
    }
}

// n‰it‰ on kutsuttava pareittain SetUpDifferenceDrawing-metodin kanssa
void NFmiIsoLineView::RestoreUpDifferenceDrawing(boost::shared_ptr<NFmiDrawParam> &theUsedDrawParam) // n‰it‰ on kutsuttava pareittain
{
    if(fDoDifferenceDrawSwitch)
    {
        theUsedDrawParam = itsBackupDrawParamForDifferenceDrawing;
        itsBackupDrawParamForDifferenceDrawing.reset();
    }
    fDoDifferenceDrawSwitch = false;
}

bool NFmiIsoLineView::LeftButtonUp(const NFmiPoint& thePlace, unsigned long theKey)
{
    return NFmiStationView::LeftButtonUp(thePlace, theKey);
}

bool NFmiIsoLineView::RightButtonUp(const NFmiPoint& thePlace, unsigned long theKey)
{
    return NFmiStationView::RightButtonUp(thePlace, theKey);
}

static void AdjustZoomedAreaRect(NFmiRect &theZoomedAreaRect)
{
    // HUOM!!! viilaan zoom laatikon hienoista ep‰tarkkuutta, pit‰isi korjata area metodeihin!!!
    static const double errorLimit = 0.000001;
    if(fabs(theZoomedAreaRect.Left()) < errorLimit)
        theZoomedAreaRect.Left(0);
    if(fabs(theZoomedAreaRect.Top()) < errorLimit)
        theZoomedAreaRect.Top(0);
    if(fabs(1. - theZoomedAreaRect.Right()) < errorLimit)
        theZoomedAreaRect.Right(1);
    if(fabs(1. - theZoomedAreaRect.Bottom()) < errorLimit)
        theZoomedAreaRect.Bottom(1);
}

//************************************************************************
//*********** toolmaster juttuja, siirr‰ pois ****************************
//*********** t‰ytet‰‰n toolmaster piirto functionlle tietoja ************
//************************************************************************


// T‰m‰ funktio palauttaa true, jos pelk‰n zoomatun alueen datan k‰yttˆ ja piirto on mahdollista.
// Jos mahdollista palauttaa myˆs uuden zoomed-area-rectin ja zoomatun alueen hilan 'boundingbox' indeksit.
// Zoomatun datan k‰yttˆ mahdollista jos:
// 1. teht‰v‰ s‰‰stˆ hilapisteiss‰ on riitt‰v‰.
// 2. zoomattu alue on kokonaan datan sis‰ll‰ (t‰m‰ on helppo ohjelmoida, tulevaisuudessa ehdot voivat muuttua joustavimmiksi)
// 3. datatype ei saa olla macroParam (t‰m‰n toteutus vaatii erillist‰ viilausta, joten t‰m‰ ehto poistuu tulevaisuudessa)
// Funktio olettaa seuraavia asioita:
// 1. Annettu data on hilamuotoista.
// 2. Annettu area ja datan area ovat samaa tyyppi‰ (ja ne eiv‰t ole 0-pointtereita!).
// 3. Annettu data (theInfo) on olemassa.
bool NFmiIsoLineView::IsZoomingPossible(boost::shared_ptr<NFmiFastQueryInfo>& theInfo, boost::shared_ptr<NFmiArea>& theMapArea, NFmiRect& theCroppedXyRectOut, int& x1, int& y1, int& x2, int& y2)
{
    if(IsQ2ServerUsed() && fGetCurrentDataFromQ2Server) // q2serverilt‰ haetaan aina vain karttan‰ytˆn t‰ytt‰v‰‰n hilaan, eli zooming alueen rajoittamista ei saa k‰ytt‰‰
        return false;
    if(IsSpecialMatrixDataDraw())
        return false;
    if(!NFmiDrawParam::IsMacroParamCase(theInfo->DataType()))
    {
        return NFmiSmartToolModifier::GetPossibleCropGridPoints(theInfo, theMapArea, theCroppedXyRectOut, x1, y1, x2, y2, 0.9);
    }
    return false;
}

// Osuvatko data j‰ n‰ytˆn alue ollenkaan p‰‰llekk‰in.
// EI t‰ydellinen testi:
// Tutkit‰‰n onko alue 1:n mik‰‰n kulma piste alue 2:n sis‰ll‰.
// Tarvittaessa tutkitaan viel‰ toisinp‰in, eli onko alue 2:n
// mik‰‰n kulma piste alue 1:en sis‰ll‰.
static bool IsDataInView(const boost::shared_ptr<NFmiArea> &theDataArea, const boost::shared_ptr<NFmiArea> &theViewArea)
{
    if(theDataArea && theViewArea)
    {
        if(theDataArea->IsInside(theViewArea->BottomLeftLatLon()))
            return true;
        if(theDataArea->IsInside(theViewArea->BottomRightLatLon()))
            return true;
        if(theDataArea->IsInside(theViewArea->TopLeftLatLon()))
            return true;
        if(theDataArea->IsInside(theViewArea->TopRightLatLon()))
            return true;

        if(theViewArea->IsInside(theDataArea->BottomLeftLatLon()))
            return true;
        if(theViewArea->IsInside(theDataArea->BottomRightLatLon()))
            return true;
        if(theViewArea->IsInside(theDataArea->TopLeftLatLon()))
            return true;
        if(theViewArea->IsInside(theDataArea->TopRightLatLon()))
            return true;

        // koska edell‰ tehty tarkastelu ei ole t‰ydellinen, paikataan sit‰ hieman kokeilemalla onko
        // eri alueiden keskipisteet toisen alueen sis‰ll‰. Nurkkapisteit‰ tarkasteltaessa
        // voi tulla virheit‰, jotka on helposti tarkastettavissa k‰ytt‰m‰ll‰ keskipistett‰.
        if(theDataArea->IsInside(theViewArea->ToLatLon(theViewArea->XYArea().Center())))
            return true;
        if(theViewArea->IsInside(theDataArea->ToLatLon(theDataArea->XYArea().Center())))
            return true;
    }
    return false;
}

bool NFmiIsoLineView::DifferentWorldViews(const NFmiArea *area1, const NFmiArea * area2)
{
    if(area1 && area2)
        return area1->PacificView() ^ area2->PacificView();
    else
        return false;
}

// Tuulen suunta on kulma joka alkaa pystysuunnasta (0/360 astetta) ja menee myˆt‰p‰iv‰‰n ymp‰ri. 
// ToolMaster haluaa kulmat niin ett‰ ne alkavat 90 asteen kohdalta (it‰ suunta) ja menev‰t
// vastap‰iv‰‰n ymp‰ri. Joten joudutaan rukkaamaan asteita origon ja suunnan suhteen.
// Huom! Lis‰ksi kulmaa s‰‰det‰‰n viel‰ 180 astetta, koska nuolet piirret‰‰n 180 astetta toiseen suuntaan.
static float WindAngleToToolMasterAngle(float windAngle)
{
    if(windAngle == kFloatMissing)
        return kFloatMissing;
    float toolmasterAngle = -windAngle + 90 + 180; // k‰‰nnet‰‰n suunta ja siirret‰‰n origoa
    if(toolmasterAngle < 0)
        toolmasterAngle += 360;
    return toolmasterAngle;
}

bool NFmiIsoLineView::FillIsoLineVisualizationInfo(boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiIsoLineData* theIsoLineData, bool fToolMasterUsed, bool fStationData)
{
    if(!(theDrawParam && theIsoLineData))
        return false;

    if(theIsoLineData->itsDataMinValue == kFloatMissing || theIsoLineData->itsDataMaxValue == kFloatMissing)
        return false;

    auto viewType = theDrawParam->GetViewType(fStationData);
    bool initializationStatus = false;
    if(NFmiDrawParam::IsColorContourType(viewType))
    {
        if(theDrawParam->UseSimpleIsoLineDefinitions())
            FillSimpleColorContourInfo(theDrawParam, theIsoLineData, fStationData, fToolMasterUsed);
        else
            FillCustomColorContourInfo(theDrawParam, theIsoLineData, fStationData, fToolMasterUsed);
        initializationStatus = theIsoLineData->itsColorContouringData.isCorrectlyInitialized();
    }
    if(NFmiDrawParam::IsIsolineType(viewType))
    {
        if(theDrawParam->UseSimpleIsoLineDefinitions()) // 2=isoline
            FillIsoLineInfoSimple(theDrawParam, theIsoLineData, fToolMasterUsed, fStationData);
        else // custom isoviivat t‰h‰n
            FillIsoLineInfoCustom(theDrawParam, theIsoLineData, fToolMasterUsed, fStationData);
        initializationStatus = theIsoLineData->itsIsolineVizualizationData.isCorrectlyInitialized();
    }

    FillHatchInfo(theDrawParam, theIsoLineData);
    return initializationStatus;
}

// Presumption: theDrawParam and theIsoLineData parameters are not nullptr's 
void NFmiIsoLineView::FillHatchInfo(boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiIsoLineData* theIsoLineData)
{
    // Yhden karttaruudun korkeutta tarvitaan hatching laskuissa
    theIsoLineData->itsSingleSubMapViewHeightInMillimeters = itsCtrlViewDocumentInterface->SingleMapViewHeightInMilliMeters(itsMapViewDescTopIndex);
    if(theDrawParam->UseWithIsoLineHatch1())
    {
        theIsoLineData->itsHatch1.fUseHatch = true;
        theIsoLineData->itsHatch1.fDrawHatchBorders = theDrawParam->DrawIsoLineHatchWithBorders1();

        NFmiColor col(theDrawParam->IsoLineHatchColor1());
        int colorIndex1 = ToolMasterColorCube::RgbToColorIndex(col);
        theIsoLineData->itsHatch1.itsHatchColorIndex = colorIndex1;

        theIsoLineData->itsHatch1.itsHatchColorRef = RGB(col.Red() * 255, col.Green() * 255, col.Blue() * 255);

        theIsoLineData->itsHatch1.itsHatchPattern = theDrawParam->IsoLineHatchType1();
        theIsoLineData->itsHatch1.itsHatchLowerLimit = ColorContouringData::GetToolMasterContourLimitChangeValue(theDrawParam->IsoLineHatchLowValue1());
        theIsoLineData->itsHatch1.itsHatchUpperLimit = ColorContouringData::GetToolMasterContourLimitChangeValue(theDrawParam->IsoLineHatchHighValue1());
    }

    if(theDrawParam->UseWithIsoLineHatch2())
    {
        theIsoLineData->itsHatch2.fUseHatch = true;
        theIsoLineData->itsHatch2.fDrawHatchBorders = theDrawParam->DrawIsoLineHatchWithBorders2();

        NFmiColor col(theDrawParam->IsoLineHatchColor2());
        int colorIndex1 = ToolMasterColorCube::RgbToColorIndex(col);
        theIsoLineData->itsHatch2.itsHatchColorIndex = colorIndex1;

        theIsoLineData->itsHatch2.itsHatchColorRef = RGB(col.Red() * 255, col.Green() * 255, col.Blue() * 255);

        theIsoLineData->itsHatch2.itsHatchPattern = theDrawParam->IsoLineHatchType2();
        theIsoLineData->itsHatch2.itsHatchLowerLimit = ColorContouringData::GetToolMasterContourLimitChangeValue(theDrawParam->IsoLineHatchLowValue2());
        theIsoLineData->itsHatch2.itsHatchUpperLimit = ColorContouringData::GetToolMasterContourLimitChangeValue(theDrawParam->IsoLineHatchHighValue2());
    }
}

void NFmiIsoLineView::FillBaseColorContourInfo_new(boost::shared_ptr<NFmiDrawParam>& theDrawParam, NFmiIsoLineData* theIsoLineData, bool fStationData, bool fToolMasterUsed)
{
    theIsoLineData->fUseIsoLines = 0; // toistaiseksi viel‰ ilman isoviivoja!!!
    theIsoLineData->fUseColorContours = 1;
    theIsoLineData->fUseIsoLineGabWithCustomContours = fToolMasterUsed ? theDrawParam->UseIsoLineGabWithCustomContours() : false; // jos imagine piirto, t‰m‰ pit‰‰ laittaa falseksi, muuten tulee sotkua

    auto viewType = theDrawParam->GetViewType(fStationData);
    if(viewType == NFmiMetEditorTypes::View::kFmiColorContourView && theDrawParam->UseSeparatorLinesBetweenColorContourClasses())
        theIsoLineData->fUseSeparatorLinesBetweenColorContourClasses = true;
    if(viewType == NFmiMetEditorTypes::View::kFmiQuickColorContourView)
        theIsoLineData->fUseColorContours = 2; // 2 asettaa quick contourin p‰‰lle
    if(viewType == NFmiMetEditorTypes::View::kFmiColorContourIsoLineView)
        theIsoLineData->fDrawLabelsOverContours = true;
}

ContouringJobData NFmiIsoLineView::MakeContouringJobData(boost::shared_ptr<NFmiDrawParam>& theDrawParam)
{
    ContouringJobData contouringJobData;
    contouringJobData.dataIdent_ = theDrawParam->Param();
    contouringJobData.level_ = theDrawParam->Level();
    contouringJobData.nameAbbreviation_ = theDrawParam->ParameterAbbreviation();
    contouringJobData.dataType_ = theDrawParam->DataType();
    contouringJobData.viewIndex_ = itsMapViewDescTopIndex;
    contouringJobData.rowIndex_ = CalcRealRowIndex();
    contouringJobData.layerIndex_ = itsViewRowLayerNumber;
    return contouringJobData;
}

void NFmiIsoLineView::FillCustomColorContourInfo_new(boost::shared_ptr<NFmiDrawParam>& theDrawParam, NFmiIsoLineData* theIsoLineData)
{
    auto contouringJobData = MakeContouringJobData(theDrawParam);
    theIsoLineData->itsColorContouringData.initialize(contouringJobData, theDrawParam);
}

// Presumption: theDrawParam and theIsoLineData parameters are not nullptr's 
void NFmiIsoLineView::FillCustomColorContourInfo(boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiIsoLineData* theIsoLineData, bool fStationData, bool fToolMasterUsed)
{
    FillBaseColorContourInfo_new(theDrawParam, theIsoLineData, fStationData, fToolMasterUsed);
    theIsoLineData->fUseCustomColorContoursClasses = true;
    FillCustomColorContourInfo_new(theDrawParam, theIsoLineData);
}

void NFmiIsoLineView::FillSimpleColorContourInfo_new(boost::shared_ptr<NFmiDrawParam>& theDrawParam, NFmiIsoLineData* theIsoLineData)
{
    auto contouringJobData = MakeContouringJobData(theDrawParam);
    theIsoLineData->itsColorContouringData.initialize(contouringJobData, theDrawParam);
}

// Presumption: theDrawParam and theIsoLineData parameters are not nullptr's 
void NFmiIsoLineView::FillSimpleColorContourInfo(boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiIsoLineData* theIsoLineData, bool fStationData, bool fToolMasterUsed)
{
    FillBaseColorContourInfo_new(theDrawParam, theIsoLineData, fStationData, fToolMasterUsed);
    theIsoLineData->fUseCustomColorContoursClasses = false;
    FillSimpleColorContourInfo_new(theDrawParam, theIsoLineData);
}

void NFmiIsoLineView::FillIsoLineInfoSimple_new(boost::shared_ptr<NFmiDrawParam>& theDrawParam, NFmiIsoLineData* theIsoLineData, bool fStationData)
{
    auto contouringJobData = MakeContouringJobData(theDrawParam);
    theIsoLineData->itsIsolineVizualizationData.initialize(contouringJobData, theDrawParam, theIsoLineData->itsDataMinValue, theIsoLineData->itsDataMaxValue, fStationData);
}

// Presumption: theDrawParam and theIsoLineData parameters are not nullptr's 
void NFmiIsoLineView::FillIsoLineInfoSimple(boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiIsoLineData* theIsoLineData, bool fToolMasterUsed, bool fStationData)
{
    ForcedLogging::IsolineDrawingInfo(theDrawParam, theIsoLineData, fStationData, fToolMasterUsed, IsMapViewCase());

    theIsoLineData->fUseIsoLines = 1;
    theIsoLineData->fUseCustomIsoLineClasses = false;
    FillIsoLineInfoSimple_new(theDrawParam, theIsoLineData, fStationData);
}

// Presumption: theDrawParam and theIsoLineData parameters are not nullptr's 
void NFmiIsoLineView::FillIsoLineInfoCustom(boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiIsoLineData* theIsoLineData, bool fToolMasterUsed, bool fStationData)
{
    ForcedLogging::IsolineDrawingInfo(theDrawParam, theIsoLineData, fStationData, fToolMasterUsed, IsMapViewCase());

    theIsoLineData->fUseIsoLines = 1;
    theIsoLineData->fUseCustomIsoLineClasses = true;
    FillIsoLineInfoSimple_new(theDrawParam, theIsoLineData, fStationData);
}

NFmiPolyline* NFmiIsoLineView::CreateEmptyPolyLine(const NFmiRect &theRect, NFmiDrawingEnvironment *theEnvi)
{
    return new NFmiPolyline(theRect, 0, theEnvi
        , 1 // 1 = opaque
        , -1);  // -1 tarkoittaa, ettei k‰ytet‰ hatchia
}

void NFmiIsoLineView::ConvertPath2PolyLineList(Imagine::NFmiPath& thePath, std::list<NFmiPolyline*> &thePolyLineList, bool relative_moves, bool removeghostlines, const NFmiRect &theRect, NFmiDrawingEnvironment *theEnvi)
{
    using namespace Imagine;

    NFmiPolyline* currentPolyLine = 0;
    float last_x, last_y;
    float last_out_x, last_out_y;
    NFmiPathOperation last_op = kFmiMoveTo;
    bool firstTime = true;

    last_x = last_y = kFloatMissing;
    last_out_x = last_out_y = kFloatMissing;

    NFmiPathData::const_iterator iter = thePath.Elements().begin();
    for(; iter != thePath.Elements().end(); ++iter)
    {
        float x = static_cast<float>((*iter).X());
        float y = static_cast<float>((*iter).Y());
        // Special code for first move

        if((*iter).Oper() == kFmiConicTo || (*iter).Oper() == kFmiCubicTo)
            throw std::runtime_error("Conic and Cubic control points not supported in NFmiPath::SVG()");
        bool out_ok = true;
        if(removeghostlines && (*iter).Oper() == kFmiGhostLineTo)
        {
            out_ok = false;
        }
        else
        {
            // If ghostlines are being ignored, we must output a moveto
            // when the ghostlines end and next operation is lineto.
            if(removeghostlines &&
                (last_op == kFmiGhostLineTo) &&
                ((*iter).Oper() == kFmiLineTo))
            {
                if(relative_moves)
                {
                    if(last_out_x == kFloatMissing && last_out_y == kFloatMissing)
                    {
                        thePolyLineList.push_back(currentPolyLine); // laitetaan edellinen polyline talteen listaa
                        currentPolyLine = CreateEmptyPolyLine(theRect, theEnvi); // luodaan uusi polyline
                        currentPolyLine->AddPoint(NFmiPoint(last_x, last_y)); // laitetaan uuden polylinen 1. piste
                    }
                    else
                    {
                        thePolyLineList.push_back(currentPolyLine); // laitetaan edellinen polyline talteen listaa
                        currentPolyLine = CreateEmptyPolyLine(theRect, theEnvi); // luodaan uusi polyline
                        currentPolyLine->AddPoint(NFmiPoint(last_x - last_out_x, last_y - last_out_y)); // laitetaan uuden polylinen 1. piste
                    }
                }
                else
                {
                    thePolyLineList.push_back(currentPolyLine); // laitetaan edellinen polyline talteen listaa
                    currentPolyLine = CreateEmptyPolyLine(theRect, theEnvi); // luodaan uusi polyline
                    currentPolyLine->AddPoint(NFmiPoint(last_x, last_y)); // laitetaan uuden polylinen 1. piste
                }
                last_op = kFmiMoveTo;
                last_out_x = last_x;
                last_out_y = last_y;
            }

            if(firstTime)
            {
                firstTime = false;
                currentPolyLine = CreateEmptyPolyLine(theRect, theEnvi); // luodaan uusi polyline
                currentPolyLine->AddPoint(NFmiPoint(x, y)); // laitetaan uuden polylinen 1. piste
            }
            // Relative moves are "m dx dy" and "l dx dy"
            else if(relative_moves)
            {
                if((*iter).Oper() == kFmiMoveTo)
                {
                    thePolyLineList.push_back(currentPolyLine); // laitetaan edellinen polyline talteen listaa
                    currentPolyLine = CreateEmptyPolyLine(theRect, theEnvi); // luodaan uusi polyline
                    currentPolyLine->AddPoint(NFmiPoint(x - last_out_x, y - last_out_y)); // laitetaan uuden polylinen 1. piste
                }
                else if((*iter).Oper() == kFmiLineTo)
                {
                    currentPolyLine->AddPoint(NFmiPoint(x - last_out_x, y - last_out_y)); // laitetaan uuden polylinen 1. piste
                }
                else if(!removeghostlines && (*iter).Oper() == kFmiGhostLineTo)
                {
                    currentPolyLine->AddPoint(NFmiPoint(x - last_out_x, y - last_out_y)); // laitetaan uuden polylinen 1. piste
                }
                else
                {
                    currentPolyLine->AddPoint(NFmiPoint(x - last_out_x, y - last_out_y)); // laitetaan uuden polylinen 1. piste
                }
            }
            // Absolute moves are "M x y" and "L x y"
            else
            {
                if((*iter).Oper() == kFmiMoveTo)
                {
                    thePolyLineList.push_back(currentPolyLine); // laitetaan edellinen polyline talteen listaa
                    currentPolyLine = CreateEmptyPolyLine(theRect, theEnvi); // luodaan uusi polyline
                    currentPolyLine->AddPoint(NFmiPoint(x, y)); // laitetaan uuden polylinen 1. piste
                }
                else if((*iter).Oper() == kFmiLineTo)
                {
                    currentPolyLine->AddPoint(NFmiPoint(x, y));
                }
                else if(!removeghostlines && (*iter).Oper() == kFmiGhostLineTo)
                {
                    currentPolyLine->AddPoint(NFmiPoint(x, y));
                }
                else
                {
                    currentPolyLine->AddPoint(NFmiPoint(x, y));
                }
            }
        }
        last_op = (*iter).Oper();

        last_x = x;
        last_y = y;
        if(out_ok)
        {
            last_out_x = x;
            last_out_y = y;
        }
    }
    if(currentPolyLine)
        thePolyLineList.push_back(currentPolyLine); // laitetaan viel‰ viimeinen polyline listaan
}

static int CalcIsolineWidthInPixels(float wantedLineWidthInMM)
{
    float mm_per_pixel = 1000.f / 260.f; // arvio omasta n‰ytˆst‰ni
    int wantedPenWidthInPixels = boost::math::iround(wantedLineWidthInMM * mm_per_pixel);
    if(wantedPenWidthInPixels < 1)
        wantedPenWidthInPixels = 1;
    return wantedPenWidthInPixels;
}

static void SetPenSize(IsolineVizualizationData& isolineVizData, NFmiDrawingEnvironment& theEnvi)
{
    const auto& lineWidths = isolineVizData.lineWidths();
    int wantedPenWidthInPixels = lineWidths.empty() ? 0 : CalcIsolineWidthInPixels(lineWidths[0]);
    theEnvi.SetPenSize(NFmiPoint(wantedPenWidthInPixels, wantedPenWidthInPixels));
}

static void SetSimpleIsoLineEnvi(IsolineVizualizationData& isolineVizData, NFmiDrawingEnvironment &theEnvi, float theValue)
{
    ::SetPenSize(isolineVizData, theEnvi);

    if(isolineVizData.useSingleColor())
    {
        theEnvi.SetFrameColor(isolineVizData.finalColors()[0]);
    }
    else
    {
        auto multiColorValues = isolineVizData.getMultiColorValues(theValue);
        theEnvi.SetFrameColor(multiColorValues.second);
    }
}

static void SetCustomIsoLineEnvi(IsolineVizualizationData& isolineVizData, NFmiDrawingEnvironment &theEnvi, int theIsoLineIndex)
{
    ::SetPenSize(isolineVizData, theEnvi);

    const auto& colors = isolineVizData.finalColors();
    if(theIsoLineIndex < colors.size())
    {
        theEnvi.SetFrameColor(colors[theIsoLineIndex]);
    }
    theEnvi.DisableFill();
}

static void SetSimpleColorContourEnvi(NFmiIsoLineData& theIsoLineData, NFmiDrawingEnvironment &theEnvi, float theLowerLimit, float theUpperLimit)
{
    ::SetPenSize(theIsoLineData.itsIsolineVizualizationData, theEnvi);
    auto& colorContouringData = theIsoLineData.itsColorContouringData;
    const auto& fillColor = colorContouringData.getValueRangeColor(theLowerLimit, theUpperLimit);
    theEnvi.SetFillColor(fillColor);
    auto colorIsColorFullyOpaque = ToolMasterColorCube::IsColorFullyOpaque(fillColor);
    if(colorIsColorFullyOpaque)
        theEnvi.EnableFill();
    else
        theEnvi.DisableFill();

    if(colorIsColorFullyOpaque && theIsoLineData.fUseSeparatorLinesBetweenColorContourClasses)
    {
        theEnvi.SetFrameColor(ToolMasterColorCube::ColorIndexToRgb(3)); // jos v‰liviivat piirret‰‰n, piirret‰‰n ne toistaiseksi mustalla (=3)
        theEnvi.EnableFrame();
    }
    else
    {
        theEnvi.DisableFrame(); // TƒMƒ RAJAVIIVAN PIIRRON ESTO PITƒƒ KATSOA jotenkin toolboxista, kun ei toimi
    }

}

static void DrawPolyLineList(NFmiToolBox *theGTB, list<NFmiPolyline*> &thePolyLineList, const NFmiPoint &theOffSet)
{
    NFmiPoint scale; // ei tehd‰ skaalausta ollenkaan, kun skaalat on 0:aa
    list<NFmiPolyline*>::iterator it = thePolyLineList.begin();
    list<NFmiPolyline*>::iterator endIter = thePolyLineList.end();
    for(; it != endIter; ++it)
        theGTB->DrawPolyline(*it, theOffSet, scale);

}

//template<typename T>
struct PointerDestroyer
{
    template<typename T>
    void operator()(T* thePtr)
    {
        delete thePtr;
    }
};

static void CalcLocationsXYMatrix(NFmiDataMatrix<NFmiPoint> & theMatrix, const NFmiArea & theArea, int nx, int ny)
{
    theMatrix.Resize(nx, ny, NFmiPoint(kFloatMissing, kFloatMissing));
    NFmiGrid aGrid(&theArea, nx, ny);

    for(int j = 0; j < ny; j++)
        for(int i = 0; i < nx; i++)
            theMatrix[i][j] = theArea.ToXY(aGrid.LatLon(j*nx + i));
}

static void LocationsXYForArchiveData(NFmiDataMatrix<NFmiPoint> & theMatrix, const NFmiArea & theArea, size_t xnum, size_t ynum)
{
    NFmiGrid grid(&theArea, static_cast<unsigned long>(xnum), static_cast<unsigned long>(ynum));

    theMatrix.Resize(xnum, ynum, NFmiPoint(kFloatMissing, kFloatMissing));

    for(size_t j = 0; j < ynum; j++)
        for(size_t i = 0; i < xnum; i++)
            theMatrix[i][j] = theArea.ToXY(grid.LatLon(static_cast<unsigned long>(j*xnum + i)));
}


void NFmiIsoLineView::DrawIsoLinesWithImagine(void)
{
    LabelBox::InitPixelPerMMRatio(itsToolBox);

    NFmiIsoLineData isoLineData;

    if(!itsInfo)
        return;

    if(itsInfo->IsGrid())
    {
        // huom. q2serverilta data voi olla minne tahansa, joten sen k‰yttˆ on poikkeus
        boost::shared_ptr<NFmiArea> infoArea(itsInfo->Area()->Clone());
        if(IsQ2ServerUsed() == false && IsDataInView(infoArea, GetArea()) == false)
            return; // ei tarvitse piirt‰‰ ollenkaan, koska data ei osu n‰ytˆn alueelle ollenkaan.
    }

    // Tyhjennet‰‰n aina piirron aluksi
    itsExistingLabels.clear();

    int xCount = 0;
    int yCount = 0;
    if(itsInfo->IsGrid() == false)
    {
        NFmiPoint usedGridSize = itsCtrlViewDocumentInterface->StationDataGridSize();
        xCount = static_cast<int>(usedGridSize.X());
        yCount = static_cast<int>(usedGridSize.Y());
    }
    else
    {
        xCount = itsInfo->Grid()->XNumber();
        yCount = itsInfo->Grid()->YNumber();
    }

    isoLineData.itsInfo = itsInfo;
    isoLineData.itsParam = itsInfo->Param();
    isoLineData.itsTime = itsInfo->Time();

    {
        // Laitetaan t‰m‰ erilliseen blokkiin, jotta vanha maski arvo saadaan takaisin p‰‰lle niin kuin vanhallakin koodilla
        EditedInfoMaskHandler editedInfoMaskHandler(itsInfo, NFmiMetEditorTypes::kFmiNoMask); // k‰yd‰‰n kaikki pisteet l‰pi
        if(!FillIsoLineDataWithGridData(isoLineData, 0, 0, 0, 0))
            return;
    }

    SetUpDifferenceDrawing(itsDrawParam);

    /* bool status = */ FillIsoLineVisualizationInfo(itsDrawParam, &isoLineData, false, itsInfo->IsGrid() == false);

    Imagine::NFmiDataHints helper(itsIsolineValues);
    NFmiGridPointCache::Data coordData;

    coordData.itsOffSet = itsArea->TopLeft();
    NFmiDataMatrix<NFmiPoint> *usedCoordinatesPtr = 0;
    NFmiPoint usedOffset; // pit‰‰ laskea mik‰ on k‰ytetty offsetti, kun contoureja aletaan piirt‰m‰‰n
    string gridCacheStr = itsInfo->Grid() ? NFmiGridPointCache::MakeGridCacheStr(*itsInfo->Grid()) : "";
    if(fGetCurrentDataFromQ2Server)
        gridCacheStr = NFmiGridPointCache::MakeGridCacheStr(*itsArea, itsIsolineValues.NX(), itsIsolineValues.NY());
    auto &gridPointCache = itsCtrlViewDocumentInterface->GridPointCache(itsMapViewDescTopIndex);
    NFmiGridPointCache::pointMap::iterator it = gridPointCache.Find(gridCacheStr);
    if(gridCacheStr.empty() == false && it != gridPointCache.End())
    { // jos lˆytyi cachesta, laitetaan ne koordinaateiksi
        usedCoordinatesPtr = &(*it).second.itsPoints;
        usedOffset = itsArea->TopLeft() - (*it).second.itsOffSet;
    }
    else if(itsInfo->Grid())
    { // jos ei lˆytynyt, lasketaan koordinaatit ja laitetaan ne cacheen talteen
        if(fGetCurrentDataFromQ2Server == false)
            itsInfo->LocationsXY(coordData.itsPoints, *itsArea); // otetaan koordinaatit t‰m‰n ruudun arealla, jossa on XYRect kohdallaan
        else
            ::LocationsXYForArchiveData(coordData.itsPoints, *itsArea, itsIsolineValues.NX(), itsIsolineValues.NY());

        gridPointCache.Add(gridCacheStr, coordData);
        usedCoordinatesPtr = &coordData.itsPoints;
        usedOffset = NFmiPoint(0, 0); // kun kyse on 'originaali' hila pisteist‰, offset on (0, 0)
    }
    else // asema datasta lasketaan hilattua dataa
    { // pit‰‰ laskea hila viel‰ t‰ss‰
        ::CalcLocationsXYMatrix(coordData.itsPoints, *itsArea, static_cast<int>(itsIsolineValues.NX()), static_cast<int>(itsIsolineValues.NY())); // otetaan koordinaatit t‰m‰n ruudun arealla, jossa on XYRect kohdallaan
        usedCoordinatesPtr = &coordData.itsPoints;
        usedOffset = NFmiPoint(0, 0); // kun kyse on 'originaali' hila pisteist‰, offset on (0, 0)
    }

    if(isoLineData.itsHatch1.fUseHatch)
        DrawHatchesWithImagine(isoLineData, isoLineData.itsHatch1, itsIsolineValues, *usedCoordinatesPtr, helper, usedOffset);
    if(isoLineData.itsHatch2.fUseHatch)
        DrawHatchesWithImagine(isoLineData, isoLineData.itsHatch2, itsIsolineValues, *usedCoordinatesPtr, helper, usedOffset);

    if(isoLineData.fUseIsoLines)
    {
        if(!isoLineData.fUseCustomIsoLineClasses) // piirret‰‰n tasa v‰liset isoviivat
            DrawSimpleIsoLinesWithImagine(isoLineData, itsIsolineValues, *usedCoordinatesPtr, helper, usedOffset);
        else
            DrawCustomIsoLinesWithImagine(isoLineData, itsIsolineValues, *usedCoordinatesPtr, helper, usedOffset);
    }
    else
    {
        if(isoLineData.fUseCustomColorContoursClasses) // piirret‰‰n tasa v‰liset isoviivat
            DrawCustomColorContourWithImagine(isoLineData, itsIsolineValues, *usedCoordinatesPtr, helper, usedOffset);
        else
            DrawSimpleColorContourWithImagine(isoLineData, itsIsolineValues, *usedCoordinatesPtr, helper, usedOffset);
    }

    RestoreUpDifferenceDrawing(itsDrawParam);
}

static void SetHatchEnvi(NFmiIsoLineData & /* theIsoLineData */, NFmiDrawingEnvironment &theEnvi, const NFmiHatchingSettings & theHatchSettings)
{
    theEnvi.EnableFill();
    if(theHatchSettings.fDrawHatchBorders)
        theEnvi.EnableFrame();
    else
        theEnvi.DisableFrame();
    NFmiColor color(ToolMasterColorCube::ColorIndexToRgb(theHatchSettings.itsHatchColorIndex));
    theEnvi.SetFillColor(color);
    theEnvi.SetHatchPattern(theHatchSettings.itsHatchPattern);
}

static std::unordered_map<std::string, NFmiDataMatrix<NFmiPoint> > g_LatlonMatrixCache;

static Imagine::NFmiPath CalcContourPath(float theValue1, float theValue2, NFmiIsoLineData& theIsoLineData, NFmiDataMatrix<float> &theValues, NFmiDataMatrix<NFmiPoint> &theCoordinates, Imagine::NFmiDataHints &theHelper, const boost::shared_ptr<NFmiArea> &theMapArea, bool fLinesOnly)
{
    Imagine::NFmiContourTree tree(theValue1, theValue2);
    tree.LinesOnly(fLinesOnly);
    if(theIsoLineData.itsXNumber * theIsoLineData.itsYNumber > gMaxGridPointsThatImagineWillSubTriangulate) // ei tehd‰ subtriangle jakoa jos hilapisteit‰ tarpeeksi paljon muutenkin
        tree.SubTriangleMode(false);								// eli n. 30 x 30 (=900) hilasta ylˆsp‰in ei tehd‰ alikolmioita

    // HUOM! t‰ss‰ pit‰‰ tarkastella nimenomaan datan mahdollista gridiss‰ olevaa areaa, ei queryInfon Area-metodin palauttavaa areaa.
    // T‰m‰ siksi, ett‰ jos ollaan piirt‰m‰ss‰ havainto dataa isoviiva moodissa, pit‰‰ menn‰ else-haaraan, ja sinne p‰‰see kun gridArea on 0-pointteri
    NFmiArea *dataGridArea = theIsoLineData.itsInfo->Grid() ? theIsoLineData.itsInfo->Grid()->Area() : 0;
    if(NFmiIsoLineView::DifferentWorldViews(dataGridArea, theMapArea.get()))
    {
        NFmiDataMatrix<NFmiPoint> *usedLatlonMatrix = nullptr;
        size_t xSize = theIsoLineData.itsInfo->GridXNumber();
        size_t ySize = theIsoLineData.itsInfo->GridYNumber();
        std::string latlonMatrixKey = theIsoLineData.itsInfo->Area()->AreaStr() + "-" + NFmiStringTools::Convert(xSize) + "x" + NFmiStringTools::Convert(ySize);
        auto latlonCacheIter = g_LatlonMatrixCache.find(latlonMatrixKey);
        if(latlonCacheIter == g_LatlonMatrixCache.end())
        {
            NFmiDataMatrix<NFmiPoint> latlonMatrix(xSize, ySize);
            auto latlonCacheVector = theIsoLineData.itsInfo->RefQueryData()->LatLonCache();
            for(size_t i = 0; i < latlonCacheVector->size(); i++)
                latlonMatrix[i % xSize][i / xSize] = latlonCacheVector->at(i);
            auto insertIter = g_LatlonMatrixCache.insert(std::make_pair(latlonMatrixKey, latlonMatrix));
            if(insertIter.second)
                usedLatlonMatrix = &insertIter.first->second;
        }
        else
            usedLatlonMatrix = &latlonCacheIter->second;
        // datan laittaminen mihin tahansa projektioon ** anna t‰h‰n todelliset lat-lon koordinaatit pisteille **
        if(usedLatlonMatrix)
        {
            tree.Contour(*usedLatlonMatrix, theValues, theHelper, Imagine::NFmiContourTree::kFmiContourLinear);
            Imagine::NFmiPath path = tree.Path();
            Imagine::NFmiPath path2 = path.PacificView(theMapArea->PacificView());
            path2.Project(theMapArea.get());
            return path2;
        }
    }
    else
    {
        tree.Contour(theCoordinates, theValues, theHelper, Imagine::NFmiContourTree::kFmiContourLinear);
        return tree.Path();
    }

    return Imagine::NFmiPath();
}

void NFmiIsoLineView::DrawHatchesWithImagine(NFmiIsoLineData& theIsoLineData, const NFmiHatchingSettings& theHatchSettings, NFmiDataMatrix<float> &theValues, NFmiDataMatrix<NFmiPoint> &theCoordinates, Imagine::NFmiDataHints &theHelper, const NFmiPoint &theOffSet)
{
    NFmiDrawingEnvironment envi;
    list<NFmiPolyline*> polyLineList;
    float lowerLimit = 0.f;
    float upperLimit = 0.f;
    if(theHatchSettings.fUseHatch)
    {
        lowerLimit = theHatchSettings.itsHatchLowerLimit;
        upperLimit = theHatchSettings.itsHatchUpperLimit;
        Imagine::NFmiPath path = ::CalcContourPath(lowerLimit, upperLimit, theIsoLineData, theValues, theCoordinates, theHelper, itsArea, false);

        SetHatchEnvi(theIsoLineData, envi, theHatchSettings);

        ConvertPath2PolyLineList(path, polyLineList, false, false, itsArea->XYArea(), &envi);

        ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsArea->XYArea());
        itsToolBox->DrawMultiPolygon(polyLineList, &envi, theOffSet);

        std::for_each(polyLineList.begin(), polyLineList.end(), PointerDestroyer());
        polyLineList.clear();
    }
}

// ei fillattuja tasa v‰lein olevia contoureja
void NFmiIsoLineView::DrawSimpleIsoLinesWithImagine(NFmiIsoLineData& theIsoLineData, NFmiDataMatrix<float> &theValues, NFmiDataMatrix<NFmiPoint> &theCoordinates, Imagine::NFmiDataHints &theHelper, const NFmiPoint &theOffSet)
{
    NFmiDrawingEnvironment envi;
    list<NFmiPolyline*> polyLineList;

    LabelBox labelBox;
    float fontSize = itsDrawParam->SimpleIsoLineLabelHeight() * ::CalcMMSizeFactor(static_cast<float>(itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsViewHeightInMM), 1.1f); // [mm]

    auto& isolineVizData = theIsoLineData.itsIsolineVizualizationData;
    float currentIsoLineValue = isolineVizData.startLimitValue();
    for(int i = 0; i < isolineVizData.usedIsolineCount(); i++, currentIsoLineValue += isolineVizData.usedStep())
    {
        Imagine::NFmiPath path = ::CalcContourPath(currentIsoLineValue, kFloatMissing, theIsoLineData, theValues, theCoordinates, theHelper, itsArea, true);

        SetSimpleIsoLineEnvi(isolineVizData, envi, currentIsoLineValue);

        labelBox.Init(fontSize, currentIsoLineValue, itsDrawParam, itsToolBox, envi);

        ConvertPath2PolyLineList(path, polyLineList, false, true, itsArea->XYArea(), &envi);
        ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsArea->XYArea());

        DrawPolyLineList(itsToolBox, polyLineList, theOffSet);
        if(itsDrawParam->SimpleIsoLineLabelHeight() > 0) // ShowSimpleIsoLineLabelBox())
        {
            // EL -->
            if(!itsDrawParam->UseSingleColorsWithSimpleIsoLines())
            {
                // Kyseess‰ EI ole "single-color" -tyyppinen isoviiva.
                // Labeltekstin v‰riksi voidaan siten ottaa jo aikaisemmin SetSimpleIsoLineEnvi():ssa
                // laskettu envi.FrameColor():n arvo
                itsDrawParam->FrameColor(envi.GetFrameColor());
            }
            // <-- EL

            AddLabelsToDrawList(path, currentIsoLineValue, itsArea->XYArea(), labelBox, theOffSet);
        }

        std::for_each(polyLineList.begin(), polyLineList.end(), PointerDestroyer());
        polyLineList.clear();
    }

    if(itsDrawParam->SimpleIsoLineLabelHeight() > 0) // ShowSimpleIsoLineLabelBox())
        DrawAllLabels(theOffSet);
}

// ei fillattuja speciaali contoureja/isoviivoja
void NFmiIsoLineView::DrawCustomIsoLinesWithImagine(NFmiIsoLineData& theIsoLineData, NFmiDataMatrix<float> &theValues, NFmiDataMatrix<NFmiPoint> &theCoordinates, Imagine::NFmiDataHints &theHelper, const NFmiPoint &theOffSet)
{
    NFmiDrawingEnvironment envi;
    list<NFmiPolyline*> polyLineList;

    auto& isolineVizData = theIsoLineData.itsIsolineVizualizationData;
    LabelBox labelBox;
    float fontSize = 0; // itsDrawParam->SimpleIsoLineLabelHeight(); // [mm]
    float fontSizeFactor = ::CalcMMSizeFactor(static_cast<float>(itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsViewHeightInMM), 1.1f);
    float currentIsoLineValue = isolineVizData.startLimitValue();
    for(int i = 0; i < isolineVizData.usedIsolineCount(); i++)
    {
        fontSize = isolineVizData.labelHeights()[i] * fontSizeFactor;
        currentIsoLineValue = isolineVizData.finalClassLimits()[i];
        Imagine::NFmiPath path = ::CalcContourPath(currentIsoLineValue, kFloatMissing, theIsoLineData, theValues, theCoordinates, theHelper, itsArea, true);

        SetCustomIsoLineEnvi(isolineVizData, envi, i);

        labelBox.Init(fontSize, currentIsoLineValue, itsDrawParam, itsToolBox, envi);

        ConvertPath2PolyLineList(path, polyLineList, false, true, itsArea->XYArea(), &envi);

        ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsArea->XYArea());

        DrawPolyLineList(itsToolBox, polyLineList, theOffSet);

        if(fontSize > 0)
        {
            // EL -->
            if(!itsDrawParam->UseSingleColorsWithSimpleIsoLines())
            {
                // Kyseess‰ EI ole "single-color" -tyyppinen isoviiva.
                // Labeltekstin v‰riksi voidaan siten ottaa jo aikaisemmin SetSimpleIsoLineEnvi():ssa
                // laskettu envi.FrameColor():n arvo
                itsDrawParam->FrameColor(envi.GetFrameColor());
            }
            // <-- EL

            AddLabelsToDrawList(path, currentIsoLineValue, itsArea->XYArea(), labelBox, theOffSet);
        }

        std::for_each(polyLineList.begin(), polyLineList.end(), PointerDestroyer());
        polyLineList.clear();
    }

    DrawAllLabels(theOffSet);
}

// fillattuja tasav‰lisi‰ contoureja
void NFmiIsoLineView::DrawSimpleColorContourWithImagine(NFmiIsoLineData& theIsoLineData, NFmiDataMatrix<float> &theValues, NFmiDataMatrix<NFmiPoint> &theCoordinates, Imagine::NFmiDataHints &theHelper, const NFmiPoint &theOffSet)
{
    static int bsCounter = 0;
    bsCounter++;
    NFmiDrawingEnvironment envi;
    list<NFmiPolyline*> polyLineList;
    bool drawLabels = theIsoLineData.fDrawLabelsOverContours;
    auto& colorContouringData = theIsoLineData.itsColorContouringData;
    auto &contourLimits = colorContouringData.finalClassLimits();

    LabelBox labelBox;
    float fontSize = itsDrawParam->SimpleIsoLineLabelHeight() * ::CalcMMSizeFactor(static_cast<float>(itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsViewHeightInMM), 1.1f); // [mm]
    float upperLimit = contourLimits.front();
    float lowerLimit = upperLimit - colorContouringData.finalBlendingStep(); // HUOM! alarasta pit‰‰ v‰hent‰‰ viel‰ yksi steppi, koska contourit piirret‰‰n eri tekniikalla kuin toolmasterissa
    float currentIsoLineValue = lowerLimit;

    for(int i = 0; i <= contourLimits.size(); i++)
    {
        if(i < contourLimits.size())
            upperLimit = contourLimits[i];
        else
            upperLimit += colorContouringData.finalBlendingStep();
        Imagine::NFmiPath path = ::CalcContourPath(lowerLimit, upperLimit, theIsoLineData, theValues, theCoordinates, theHelper, itsArea, false);

        SetSimpleColorContourEnvi(theIsoLineData, envi, lowerLimit, upperLimit);

        if(drawLabels)
            labelBox.Init(fontSize, currentIsoLineValue, itsDrawParam, itsToolBox, envi);

        ConvertPath2PolyLineList(path, polyLineList, false, false, itsArea->XYArea(), &envi);

        ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsArea->XYArea());

        if(theIsoLineData.fUseSeparatorLinesBetweenColorContourClasses) // 22.9.04/EL
            DrawPolyLineList(itsToolBox, polyLineList, theOffSet);

        itsToolBox->DrawMultiPolygon(polyLineList, &envi, theOffSet);

        // 22.9.04/EL ---->
        if(drawLabels)
        {
            if(fontSize > 0)
            {
                if(!itsDrawParam->UseSingleColorsWithSimpleIsoLines())
                {
                    // Kyseess‰ EI ole "single-color" -tyyppinen isoviiva.
                    // Labeltekstin v‰riksi voidaan siten ottaa jo aikaisemmin SetSimpleColorContourEnvi():ssa
                    // laskettu envi.FrameColor():n arvo
                    itsDrawParam->FrameColor(envi.GetFrameColor());
                }

                AddLabelsToDrawList(path, currentIsoLineValue, itsArea->XYArea(), labelBox, theOffSet);
            }
        }
        // 22.9.04/EL <----

        std::for_each(polyLineList.begin(), polyLineList.end(), PointerDestroyer());
        polyLineList.clear();

        lowerLimit = currentIsoLineValue = upperLimit;
    }

    if(drawLabels)
        DrawAllLabels(theOffSet);
}

// fillattuja spesiaali rajoilla olevia contoureja
void NFmiIsoLineView::DrawCustomColorContourWithImagine(NFmiIsoLineData& theIsoLineData, NFmiDataMatrix<float> &theValues, NFmiDataMatrix<NFmiPoint> &theCoordinates, Imagine::NFmiDataHints &theHelper, const NFmiPoint &theOffSet)
{
    static const int invisibleColorIndex = 2;
    static const float minValue = -99999999999.f;
    static const float maxValue = 99999999999.f;
    NFmiDrawingEnvironment envi;
    list<NFmiPolyline*> polyLineList;
    bool drawLabels = theIsoLineData.fDrawLabelsOverContours;
    auto& colorContouringData = theIsoLineData.itsColorContouringData;
    auto& isolineVizData = theIsoLineData.itsIsolineVizualizationData;
    const auto& customContourLimits = colorContouringData.finalClassLimits();
    const auto& customContourColorIndexies = colorContouringData.finalColorIndexies();
    auto usedStep = colorContouringData.finalBlendingStep();
    const auto& labelHeights = isolineVizData.labelHeights();

    LabelBox labelBox;
    float fontSize = 0; //itsDrawParam->SimpleIsoLineLabelHeight(); // [mm]
    float fontSizeFactor = ::CalcMMSizeFactor(static_cast<float>(itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsViewHeightInMM), 1.1f);
    float upperLimit = customContourLimits.front();
    float lowerLimit = upperLimit - colorContouringData.finalBlendingStep(); // HUOM! alarasta pit‰‰ v‰hent‰‰ viel‰ yksi steppi, koska contourit piirret‰‰n eri tekniikalla kuin toolmasterissa
    float currentIsoLineValue = lowerLimit;
    for(int i = 0; i <= customContourLimits.size(); i++)
    {
        if(i < customContourLimits.size())
            upperLimit = customContourLimits[i];
        else
            upperLimit += colorContouringData.finalBlendingStep();

        if(customContourColorIndexies[i] == invisibleColorIndex)
            continue;
        if(labelHeights.empty())
            fontSize = 0;
        else
            fontSize = labelHeights[i] * fontSizeFactor;
        Imagine::NFmiPath path = ::CalcContourPath(lowerLimit, kFloatMissing, theIsoLineData, theValues, theCoordinates, theHelper, itsArea, false);
        SetSimpleColorContourEnvi(theIsoLineData, envi, lowerLimit, upperLimit);

        if(drawLabels)
            labelBox.Init(fontSize, lowerLimit, itsDrawParam, itsToolBox, envi);

        ConvertPath2PolyLineList(path, polyLineList, false, false, itsArea->XYArea(), &envi);

        ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &itsArea->XYArea());

        if(theIsoLineData.fUseSeparatorLinesBetweenColorContourClasses) // 22.9.04/EL
            DrawPolyLineList(itsToolBox, polyLineList, theOffSet);

        itsToolBox->DrawMultiPolygon(polyLineList, &envi, theOffSet);

        // 22.9.04/EL ---->
        if(drawLabels)
        {
            if(fontSize > 0)
            {
                if(!itsDrawParam->UseSingleColorsWithSimpleIsoLines())
                {
                    // Kyseess‰ EI ole "single-color" -tyyppinen isoviiva.
                    // Labeltekstin v‰riksi voidaan siten ottaa jo aikaisemmin SetCustomColorContourEnvi():ssa
                    // laskettu envi.FrameColor():n arvo
                    itsDrawParam->FrameColor(envi.GetFrameColor());
                }

                AddLabelsToDrawList(path, lowerLimit, itsArea->XYArea(), labelBox, theOffSet);
            }
        }
        // 22.9.04/EL <----

        std::for_each(polyLineList.begin(), polyLineList.end(), PointerDestroyer());
        polyLineList.clear();
        lowerLimit = currentIsoLineValue = upperLimit;
    }

    if(drawLabels)
        DrawAllLabels(theOffSet);
}

void NFmiIsoLineView::AddLabelsToDrawList(Imagine::NFmiPath &thePath, float theIsoLineValue, const NFmiRect &theRelativeRect, LabelBox &theLabelBox, const NFmiPoint &theOffSet)
{
    // irroita erilliset isoviivat (moveto kohdalla katkeaa)
    // irroita myˆs ghostlinet (skippaa ne)
    // tee NFmiPathData otuksia isoviivoista
    // laske yhteen stringi‰

    list<NFmiPathData*> pathDataList;
    NFmiPathData *partialPathData = new NFmiPathData;
    NFmiPathData::const_iterator iter = thePath.Elements().begin();
    NFmiPathData::const_iterator endIter = thePath.Elements().end();
    NFmiPathElement currentElement(kFmiMoveTo, 0, 0);
    NFmiPathElement lastElement(kFmiMoveTo, 0, 0);

    for(; iter != endIter; ++iter)
    {
        currentElement = *iter;
        if(currentElement.Oper() == kFmiGhostLineTo && lastElement.Oper() == kFmiGhostLineTo)
            ;
        else if(currentElement.Oper() == kFmiGhostLineTo)
        {
            if(partialPathData->size() > 0)
                pathDataList.push_back(partialPathData);
            else
                delete partialPathData;
            partialPathData = new NFmiPathData;
        }
        else if(currentElement.Oper() == kFmiMoveTo)
        {
            if(partialPathData->size() > 0)
                pathDataList.push_back(partialPathData);
            else
                delete partialPathData;
            partialPathData = new NFmiPathData;
            partialPathData->push_back(currentElement);
        }
        else if(currentElement.Oper() == kFmiLineTo && lastElement.Oper() == kFmiGhostLineTo)
        {
            NFmiPathElement tmpElement(lastElement);
            tmpElement.Oper(kFmiMoveTo);
            partialPathData->push_back(tmpElement); // viimeisin ghostline pit‰‰ muuttaa movetoksi
            partialPathData->push_back(currentElement);
        }
        else if(currentElement.Oper() == kFmiLineTo)
            partialPathData->push_back(currentElement);
        lastElement = currentElement;
    }
    // lopuksi pit‰‰ viel‰ laittaa loput
    if(partialPathData->size() > 0)
        pathDataList.push_back(partialPathData);
    else
        delete partialPathData;
    MakePossiblePathCombinations(pathDataList);
    AddLabelsToDrawList(pathDataList, theIsoLineValue, theRelativeRect, theLabelBox, theOffSet);

    // pit‰‰ tuhota dynaaminen data
    std::for_each(pathDataList.begin(), pathDataList.end(), PointerDestroyer());
    pathDataList.clear();
}

void NFmiIsoLineView::AddLabelsToDrawList(list<Imagine::NFmiPathData*> &thePathDataList, float theIsoLineValue, const NFmiRect &theRelativeRect, LabelBox &theLabelBox, const NFmiPoint &theOffSet)
{
    NFmiRect usedRelativeRect(theRelativeRect); // t‰ytyy feikata eli tehd‰ offset muutos t‰h‰n relative rectiin, muuten ei saada labeleita laatikon sis‰‰n
    NFmiPoint usedCenter = usedRelativeRect.Center() - theOffSet;
    usedRelativeRect.Center(usedCenter);
    list<NFmiPathData*>::iterator iter = thePathDataList.begin();
    list<NFmiPathData*>::iterator endIter = thePathDataList.end();
    for(; iter != endIter; ++iter)
    {
        AddLabelToSingleIsoLine(*(*iter), theIsoLineValue, usedRelativeRect, theLabelBox);
    }
}

void NFmiIsoLineView::AddLabelToSingleIsoLine(const NFmiPathData &thePath, float /* theIsoLineValue */, const NFmiRect &theRelativeRect, LabelBox &theLabelBox)
{
    if(MoveLabelBoxToBestPossiblePlace(thePath, theRelativeRect, theLabelBox))
    {
        itsExistingLabels.push_back(theLabelBox);
    }
}

//EL: ----->
bool NFmiIsoLineView::MoveLabelBoxToBestPossiblePlace(const NFmiPathData &thePath, const NFmiRect &theRelativeRect, LabelBox &theLabelBox)
{
    // M‰‰ritt‰‰ label-laatikolle "parhaan" sijainnin, sijainti annetaan Center()-metodilla.

    if(thePath.size() >= 2)
    {
        float x = static_cast<float>(thePath[1].X());
        float y = static_cast<float>(thePath[1].Y());

        NFmiPoint centerPoint(x, y);
        NFmiPoint sparePoint(centerPoint); // t‰m‰ otetaan k‰yttˆˆn jos muuta paikkaa ei lˆydy
        bool sparePointUpdated = false;
        int sizeElements = static_cast<int>(thePath.size());

        // Aloitetaan indeksien l‰pik‰ynti "keskelt‰" path dataa kohti 'traverseDirection' m‰‰r‰‰m‰‰ suuntaa -JA- sen j‰lkeen vastakkaiseen suuntaan
        // ==> labelien sijainti jakaantuu tasaisemmin ==> v‰hemm‰n "labelruuhkaa" ==> labelien paikan etsint‰ nopeutuu kolme nanosekuntia ...
        //
        // traverseDirection -1 ==> keskelt‰ kohti alkup‰‰t‰ eli kokonaislukuv‰li  [0, sizeElements/2],
        // traverseDirection  1 ==> keskelt‰ kohti loppup‰‰t‰ eli kokonaislukuv‰li [sizeElements/2 + 1, sizeElements[

        int beginIndex = 0;
        int endIndex = 0;
        int traverseDirection = 1;

        for(int i = 0; i < 2; i++)
        {
            if(i == 0)
            {
                beginIndex = sizeElements / 2;
                endIndex = -1;
                traverseDirection = -1;
            }
            else
            {
                beginIndex = (sizeElements / 2) + 1;
                endIndex = sizeElements;
                traverseDirection = 1;
            }

            for(int j = beginIndex; j != endIndex; j += traverseDirection)
            {
                if(thePath[j].Oper() == kFmiGhostLineTo)
                    continue;

                NFmiPoint centerPoint(thePath[j].X(), thePath[j].Y());
                theLabelBox.Center(centerPoint);

                if(theRelativeRect.IsInside(theLabelBox.LabelBoxRect()))
                {
                    if(LabelDontOvelapWithOthers(theLabelBox))
                    { // lˆytyi hyv‰ paikka, otetaan se talteen ja lopetetaan
                        StoreLabel(theLabelBox);
                        return true;
                    }
                    else if(!sparePointUpdated)
                    { // otetaan talteen yksi paikka, miss‰ label olisi ainakin relatiivisen alueen sis‰ll‰, vaikka muuten olisi overlappia
                        sparePointUpdated = true;
                        sparePoint = theLabelBox.Center();
                    }
                }
            }
        }

        if(!(theLabelBox.Strategy() == 2))
        {
            theLabelBox.Center(sparePoint); // jos ei lˆytynyt 't‰ydellist‰' paikaa, k‰ytet‰‰n se varapaikka
            StoreLabel(theLabelBox);
            return true;
        }
    }
    return false;
}
// <--- /EL


bool NFmiIsoLineView::AreRectsOverLapping(const NFmiRect &first, const NFmiRect &second)
{
    return first.IsInside(second.BottomLeft()) || first.IsInside(second.TopRight())
        || first.IsInside(second.BottomRight()) || first.IsInside(second.TopLeft())
        || second.IsInside(first.BottomLeft()) || second.IsInside(first.TopRight())
        || second.IsInside(first.BottomRight()) || second.IsInside(first.TopLeft());
}

bool NFmiIsoLineView::LabelDontOvelapWithOthers(LabelBox &theLabelBoxObj)
{
    int numberOfLabels = static_cast<int>(itsExistingLabels.size());
    if(numberOfLabels > 0)
    {
        for(int i = 0; i < numberOfLabels; i++)
        {
            if(AreRectsOverLapping((itsExistingLabels[i].LabelBoxRect()), (theLabelBoxObj.LabelBoxRect())))
                return false;
        }
    }
    return true;
}


//EL --->
void NFmiIsoLineView::DrawAllLabels(const NFmiPoint &theOffSet)
{
    //	static int dummy = kArcl;

    int numberOfIsoLines = static_cast<int>(itsExistingLabels.size());
    if(numberOfIsoLines < 1)
        return;

    NFmiDrawingEnvironment env;
    env.EnableFill();
    env.DisableFrame();
    env.BoldFont(true);

    env.SetFontType(kNoneFont); // t‰m‰ asettaa default fontin p‰‰lle, joka mielest‰ni n‰ytt‰‰ hyv‰lt‰
    NFmiPoint oldFontSize = env.GetFontSize();

    FmiDirection oldTextAlignment = itsToolBox->GetTextAlignment();
    itsToolBox->SetTextAlignment(kLeft);


    if(itsDrawParam->UseSingleColorsWithSimpleIsoLines())
    {
        // Kaikilla labeleilla on samat color-attribuutit
        // ==> voidaan k‰ytt‰‰ esim. ensimm‰isen labelin (i=0) attribuutteja

        env.SetFrameColor(itsExistingLabels[0].BoxStrokeColor());
        env.SetFillColor(itsExistingLabels[0].BoxFillColor());

        for(int i = 0; i < numberOfIsoLines; i++)
        {
            if(itsDrawParam->ShowSimpleIsoLineLabelBox())
            {
                // "Pohjalaatikko" labeltekstille
                NFmiRectangle aRect(itsExistingLabels[i].LabelBoxRect().TopLeft() + theOffSet
                    , itsExistingLabels[i].LabelBoxRect().BottomRight() + theOffSet
                    , 0
                    , &env);
                itsToolBox->Convert(&aRect);
            }

            // Labeltekstin v‰ri
            env.SetFrameColor(itsExistingLabels[0].FontColor());

            int textHeight = static_cast<int>(itsExistingLabels[i].CalcHeightInPixels());
            // Labeltekstin pituus pikseleiss‰
            int textWidth = static_cast<int>(itsExistingLabels[i].CalcWidthInPixels());

            NFmiPoint newFontSize(textWidth, textHeight);
            env.SetFontSize(newFontSize);

            NFmiPoint textLocation(itsExistingLabels[i].LabelBoxRect().Left() + itsExistingLabels[i].LabelBoxRect().Width() / 30.,
                itsExistingLabels[i].LabelBoxRect().Top() - itsExistingLabels[i].LabelBoxRect().Height() / 7.5);
            textLocation += theOffSet;
            if(itsExistingLabels[i].LabelString().size() == 1) // yhden kokoinen teksti pit‰‰ kohdistaa erikseen laatikon keskelle
                textLocation.X(textLocation.X() + itsExistingLabels[i].LabelBoxRect().Width() / 5.);

            NFmiText aText(textLocation, itsExistingLabels[i].LabelString(), false, 0, &env);
            itsToolBox->Convert(&aText);
        }
    }
    else
    {
        for(int i = 0; i < numberOfIsoLines; i++)
        {
            if(itsDrawParam->ShowSimpleIsoLineLabelBox())
            {
                // "Pohjalaatikko" labeltekstille
                env.SetFillColor(itsExistingLabels[i].BoxFillColor());
                env.SetFrameColor(itsExistingLabels[i].BoxStrokeColor());

                NFmiRectangle aRect(itsExistingLabels[i].LabelBoxRect().TopLeft() + theOffSet
                    , itsExistingLabels[i].LabelBoxRect().BottomRight() + theOffSet
                    , 0, &env);
                itsToolBox->Convert(&aRect);
            }


            // Labeltekstin v‰ri
            env.SetFrameColor(itsExistingLabels[i].FontColor());

            int textHeight = static_cast<int>(itsExistingLabels[i].CalcHeightInPixels());
            // Labeltekstin pituus pikseleiss‰
            int textWidth = static_cast<int>(itsExistingLabels[i].CalcWidthInPixels());

            NFmiPoint newFontSize(textWidth, textHeight);
            env.SetFontSize(newFontSize);

            NFmiPoint textLocation(itsExistingLabels[i].LabelBoxRect().Left() + itsExistingLabels[i].LabelBoxRect().Width() / 30.,
                itsExistingLabels[i].LabelBoxRect().Top() - itsExistingLabels[i].LabelBoxRect().Height() / 7.5);
            textLocation += theOffSet;
            if(itsExistingLabels[i].LabelString().size() == 1) // yhden kokoinen teksti pit‰‰ kohdistaa erikseen laatikon keskelle
                textLocation.X(textLocation.X() + itsExistingLabels[i].LabelBoxRect().Width() / 5.);

            NFmiText aText(textLocation, itsExistingLabels[i].LabelString(), false, 0, &env);
            itsToolBox->Convert(&aText);
        }
    }

    env.SetFontSize(oldFontSize);
    itsToolBox->SetTextAlignment(oldTextAlignment);
}
// EL <---


static bool CoordComparison(const NFmiPathElement &first, const NFmiPathElement &second)
{
    //EL -->
        // Epsilonvertailu tarkan arvon vertailun sijaan.
    const double eps = 0.0001; // Arvo hatusta

    return ((abs(first.X() - second.X()) < eps) && (abs(first.Y() - second.Y()) < eps));

    //EL <--

}

static void MakePossiblePathCombinations(list<NFmiPathData*> &thePathDataList)
{
    list<NFmiPathData*>::iterator iter = thePathDataList.begin();
    list<NFmiPathData*>::iterator endIter = thePathDataList.end();

    //EL: i ja j -luupperit ovat sin‰ns‰ turhia mutta kun en osaa vertailla iteraattoreita ...
    for(long i = 0; iter != endIter; ++iter, i++)
    {
        NFmiPathData &p1 = *(*iter);
        list<NFmiPathData*>::iterator iter2 = iter;
        ++iter2;
        for(long j = 0; iter2 != endIter; j++)
        {
            NFmiPathData &p2 = *(*iter2);

            if(CoordComparison(p1[0], p2[p2.size() - 1]))
            {
                CombinePaths(p2, p1);
                p1.swap(p2);
                delete *iter2;
                iter2 = thePathDataList.erase(iter2);
            }
            else if(CoordComparison(p1[p1.size() - 1], p2[0]))
            {
                CombinePaths(p1, p2);
                delete *iter2;
                iter2 = thePathDataList.erase(iter2);
            }
            else
                ++iter2;
            //else ei osaa viel‰ verrata pelkk‰st‰‰n alkuja tai loppuja  koska se vaatii viel‰ polun k‰‰ntˆ‰
        }
    }
}

static void CombinePaths(NFmiPathData &first, NFmiPathData &second)
{ // yhdist‰‰ siten, ett‰ 1. per‰‰n laitetaan 2. datat
    int size2 = static_cast<int>(second.size());
    for(int i = 1; i < size2; i++) // j‰tet‰‰n 1. elementi pois (moveto)
        first.push_back(second[i]);
}


void NFmiIsoLineView::StoreLabel(LabelBox &theLabelBox)
{
    itsExistingLabels.push_back(theLabelBox);
}

void NFmiIsoLineView::DrawIsoLines(void)
{
    string filefilter = "default";
    if(itsInfo)
    {
        filefilter = itsInfo->DataFilePattern();
    }
    SmartMetDataUtilities::Toolmaster toolmasterStatus = itsCtrlViewDocumentInterface->IsToolMasterAvailable()
        ? SmartMetDataUtilities::Toolmaster::Available
        : SmartMetDataUtilities::Toolmaster::NotAvailable;
    dataUtilitiesAdapter->setToolmasterAvailability(toolmasterStatus);
    dataUtilitiesAdapter->setMapViewIndex(itsMapViewDescTopIndex);
    dataUtilitiesAdapter->setFileFilter(filefilter);
    dataUtilitiesAdapter->initializeDrawingData(*itsInfo, *itsArea);
    if(IsIsoLinesDrawnWithImagine())
    {
        //#ifdef NDEBUG // ei piirret‰ isoviivoja kuin release moodissa, koska degug-moodissa piirto on niin pirun hidasta
        DrawIsoLinesWithImagine();
        //#endif // NDEBUG // ei piirret‰ isoviivoja kuin release moodissa, koska degug-moodissa piirto on niin pirun hidasta
    }
    else
        DrawIsoLinesWithToolMaster();
}

// piirret‰‰nkˆ toolmasterilla vai imaginella. Piirret‰‰n toolmasterilla toistaiseksi
// nopeuden ja n‰ˆn takia jos voi.
// Eli palauta false = piirr‰ toolmasterilla jos se on k‰ytˆss‰ ja
// jos projektio luokat ovat samoja ja jos kyseess‰ oli stereographic areat, niiden
// orientaatiot pit‰‰ olla samat.
bool NFmiIsoLineView::IsIsoLinesDrawnWithImagine(void)
{
    if(!IsToolMasterAvailable())
        return true;

    if(itsInfo->IsGrid() == false)
        return false; // jos asema-data piirret‰‰n hilamoodissa, ei k‰ytet‰ imaginea, koska silloin piirret‰‰n aina 'oikeaan' hilaan.

    if(IsQ2ServerUsed() && fGetCurrentDataFromQ2Server)
        return false;  // q2-data haetaan aina suoraan ruudulle, joten toolmasteria k‰ytet‰‰n

    if(dataUtilitiesAdapter->isModifiedDataDrawingPossible())
        return false;

    boost::shared_ptr<NFmiArea> infoArea(itsInfo->Area()->Clone());
    return !NFmiQueryDataUtil::AreAreasSameKind(itsArea.get(), infoArea.get());
}

bool NFmiIsoLineView::FillGridRelatedData(NFmiIsoLineData &isoLineData, NFmiRect &zoomedAreaRect)
{
    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, __FUNCTION__);

    isoLineData.itsIsolineMinLengthFactor = itsCtrlViewDocumentInterface->ApplicationWinRegistry().IsolineMinLengthFactor();

    if(!FillGridRelatedData_IsDataVisible())
        return false;

    bool fillGridDataStatus = false;
    if(FillGridRelatedData_VisualizationOptimizationChecks(isoLineData, zoomedAreaRect, fillGridDataStatus))
    {
        return fillGridDataStatus;
    }
    if(FillGridRelatedData_BetterVisualizationChecks(isoLineData, zoomedAreaRect, fillGridDataStatus))
    {
        return fillGridDataStatus;
    }
    if(FillGridRelatedData_ZoomingChecks(isoLineData, zoomedAreaRect, fillGridDataStatus))
    {
        return fillGridDataStatus;
    }
    FillGridRelatedData_NormalDataCase(isoLineData, zoomedAreaRect, fillGridDataStatus);
    return fillGridDataStatus;
}

// FillGridRelatedData metodin paloittelu osiin, tapaus jossa k‰sitell‰‰n loput normi tapaukset.
// T‰m‰n ei tarvitse palauttaa tietoa ett‰ k‰sittelikˆ t‰m‰ asiat, koska t‰m‰ aina k‰sittelee loput tapaukset, mit‰ ei edell‰ saatu k‰sitelty‰.
void NFmiIsoLineView::FillGridRelatedData_NormalDataCase(NFmiIsoLineData& isoLineData, NFmiRect& zoomedAreaRect, bool& fillGridDataStatus)
{
    isoLineData.itsInfo = itsInfo;
    isoLineData.itsParam = itsInfo->Param();
    isoLineData.itsTime = this->itsTime;

    EditedInfoMaskHandler editedInfoMaskHandler(itsInfo, NFmiMetEditorTypes::kFmiNoMask); // k‰yd‰‰n kaikki pisteet l‰pi
    fillGridDataStatus = FillIsoLineDataWithGridData(isoLineData, 0, 0, 0, 0);
    if(IsQ2ServerUsed() && fGetCurrentDataFromQ2Server) // q2server tapauksessa haetaan vain ruudun alueelle dataa, joten poikkeaa normaali piirrosta
        zoomedAreaRect = itsInfo->Area()->XYArea();
    else if(itsInfo->IsGrid())
    {
        auto mapArea = GetArea();
        auto origDataArea = itsInfo->Area();
        if(DifferentWorldViews(origDataArea, mapArea.get()))
        { // tehd‰‰n dataArea, joka on karttapohjan maailmassa
            boost::shared_ptr<NFmiArea> origDataAreaClone(origDataArea->Clone());
            NFmiPoint blLatlon = origDataAreaClone->BottomLeftLatLon();
            NFmiPoint trLatlon = origDataAreaClone->TopRightLatLon();
            double origLongitudeDifference = trLatlon.X() - blLatlon.X();
            NFmiLongitude lonFixer(blLatlon.X(), mapArea->PacificView());
            blLatlon.X(lonFixer.Value());
            lonFixer.SetValue(trLatlon.X());
            trLatlon.X(lonFixer.Value());
            double newLongitudeDifference = trLatlon.X() - blLatlon.X();
            if(newLongitudeDifference < 0 || ::fabs(origLongitudeDifference - newLongitudeDifference) > 0.1)
                trLatlon.X(blLatlon.X() + origLongitudeDifference);
            origDataAreaClone->PacificView(mapArea->PacificView());
            boost::shared_ptr<NFmiArea> newArea(origDataAreaClone->NewArea(blLatlon, trLatlon));
            zoomedAreaRect = newArea->XYArea(mapArea.get());
        }
        else
            zoomedAreaRect = itsInfo->Area()->XYArea(mapArea.get());
    }
    else
    {
        zoomedAreaRect = NFmiRect(0, 0, 1, 1); // gridattu station data on aina 0,0 - 1,1 maailmassa
    }
}

// FillGridRelatedData metodin paloittelu osiin, tapaus jossa on k‰ytˆss‰ Visualization optimization systeemi.
// T‰llˆin lasketaan dataan ja kartta-alueeseen sopiva harvennettu hila ja data t‰ytet‰‰n siihen.
// Palauttaa bool arvon: k‰sittelikˆ t‰m‰ funktio kyseisen datan tapauksen, eik‰ tarvitse tehd‰ en‰‰ muuta.
// Huom! On jo tarkastettu, n‰kyykˆ data kartta-alueella, joten sit‰ ei tavitse tarkastella en‰‰.
bool NFmiIsoLineView::FillGridRelatedData_VisualizationOptimizationChecks(NFmiIsoLineData& isoLineData, NFmiRect& zoomedAreaRect, bool& fillGridDataStatus)
{
    auto& visSettings = GetVisualizationSettings();
    auto mapArea = GetArea();
    NFmiGrid optimizedGrid;
    if(visSettings.checkIsOptimizationsUsed(*itsInfo, *mapArea, optimizedGrid, CalcViewGridSize(), itsCtrlViewDocumentInterface->BetaProductGenerationRunning()))
    {
        fillGridDataStatus = FillIsoLineDataWithGridData(isoLineData, 0, 0, 0, 0, &optimizedGrid);
        zoomedAreaRect = optimizedGrid.Area()->XYArea(mapArea.get());
        itsOptimizedGridPtr.reset(new NFmiGrid(optimizedGrid));
        return true;
    }
    return false;
}

// FillGridRelatedData metodin paloittelu osiin, tapaus jossa datan karttaprojektio sopii yhteen pohjakartan projektioon ja dataa 
// voidaan hakea croppaamalla pienempi pala visualisointeja varten.
// Palauttaa bool arvon: k‰sittelikˆ t‰m‰ funktio kyseisen datan tapauksen, eik‰ tarvitse tehd‰ en‰‰ muuta.
bool NFmiIsoLineView::FillGridRelatedData_ZoomingChecks(NFmiIsoLineData& isoLineData, NFmiRect& zoomedAreaRect, bool& fillGridDataStatus)
{
    fillGridDataStatus = false;
    int x1 = 0;
    int y1 = 0;
    int x2 = 0;
    int y2 = 0;
    if(itsInfo->IsGrid() && IsZoomingPossible(itsInfo, GetArea(), zoomedAreaRect, x1, y1, x2, y2))
    {
        CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(std::string(__FUNCTION__) + ": zoomed grid used (faster)", this);
        isoLineData.itsInfo = itsInfo;
        isoLineData.itsParam = itsInfo->Param();
        isoLineData.itsTime = this->itsTime;
        isoLineData.fUseOriginalDataInPixelToGridRatioCalculations = true;

        EditedInfoMaskHandler editedInfoMaskHandler(itsInfo, NFmiMetEditorTypes::kFmiNoMask); // k‰yd‰‰n kaikki pisteet l‰pi
        fillGridDataStatus = FillIsoLineDataWithGridData(isoLineData, x1, y1, x2, y2);
        return true;
    }
    return false;
}

// FillGridRelatedData metodin paloittelu osiin, tapaus jossa datan karttaporjektio ei sovi pohjakartan projektioon ja dataa 
// pit‰‰ interpoloida suorakulmaiseen hilaan, mutta ilman harvennusoptimointi juttuja.
// Palauttaa bool arvon: k‰sittelikˆ t‰m‰ funktio kyseisen datan tapauksen, eik‰ tarvitse tehd‰ en‰‰ muuta.
bool NFmiIsoLineView::FillGridRelatedData_BetterVisualizationChecks(NFmiIsoLineData& isoLineData, NFmiRect& zoomedAreaRect, bool& fillGridDataStatus)
{
    fillGridDataStatus = false;
    if(!fGetCurrentDataFromQ2Server && dataUtilitiesAdapter->isModifiedDataDrawingPossible())
    {
        if(dataUtilitiesAdapter->isThereAnythingToDraw())
        {
            isoLineData.itsInfo = itsInfo;
            isoLineData.itsParam = itsInfo->Param();
            isoLineData.itsTime = this->itsTime; // T‰h‰n pistet‰‰n kartalla oleva aika
            // Mutta pit‰‰ varmistaa ett‰ data interpoloidaan oikealta ajalta myˆs klimatologisilta datoilta (kuten Era-5, tms.)
            auto usedInterpolationTime = NFmiFastInfoUtils::GetUsedTimeIfModelClimatologyData(itsInfo, itsTime);
            auto paramId = itsDrawParam->Param().GetParamIdent();
            if(metaWindParamUsage.ParamNeedsMetaCalculations(paramId))
            {
                FinalFillWindMetaDataMatrix(itsInfo, isoLineData.itsIsolineData, usedInterpolationTime, false, 0, 0, 0, 0, paramId);
            }
            else
            {
                itsInfo->Values(*dataUtilitiesAdapter->getInterpolatedData(), isoLineData.itsIsolineData, usedInterpolationTime, kFloatMissing, kFloatMissing, itsTimeInterpolationRangeInMinutes, fAllowNearestTimeInterpolation);
            }

            itsIsolineValues = isoLineData.itsIsolineData;
            fillGridDataStatus = initializeIsoLineData(isoLineData);
            zoomedAreaRect = dataUtilitiesAdapter->getCroppedArea()->XYArea(GetArea().get());
        }
        return true;
    }
    return false;
}

bool NFmiIsoLineView::FillGridRelatedData_IsDataVisible()
{
    if(itsInfo->IsGrid())
    {
        // huom. q2serverilta data voi olla minne tahansa, joten sen k‰yttˆ on poikkeus
        boost::shared_ptr<NFmiArea> infoArea(itsInfo->Area()->Clone());
        if(IsQ2ServerUsed() == false && IsDataInView(infoArea, GetArea()) == false)
            return false; // ei tarvitse piirt‰‰ ollenkaan, koska data ei osu n‰ytˆn alueelle ollenkaan.
    }
    return true;
}

static void ReportDataGridSize(NFmiCtrlView *view, NFmiIsoLineData &isoLineData)
{
    if(CatLog::doTraceLevelLogging())
    {
        std::string message = __FUNCTION__;
        message += ": grid size " + std::to_string(isoLineData.itsXNumber) + " X " + std::to_string(isoLineData.itsYNumber);
        CtrlViewUtils::CtrlViewTimeConsumptionReporter::makeSeparateTraceLogging(message, view);
    }
}

void NFmiIsoLineView::DoGridRelatedVisualization(NFmiIsoLineData &isoLineData, NFmiRect &zoomedAreaRect)
{
    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, __FUNCTION__);
    ReportDataGridSize(this, isoLineData);
    SetUpDifferenceDrawing(itsDrawParam);
    {
        // Making ToolMaster setup and drawing in one protected zone
        std::lock_guard<std::mutex> toolMasterLock(NFmiIsoLineView::sToolMasterOperationMutex);
        if(FillIsoLineVisualizationInfo(itsDrawParam, &isoLineData, true, itsInfo->IsGrid() == false))
        {
            NFmiPoint pixelToGridRatio = CalcPixelToGridRatio(isoLineData, zoomedAreaRect);
            NFmiRect relRect(GetFrame());
            AdjustZoomedAreaRect(zoomedAreaRect);
            if(itsDrawParam->IsMacroParamCase(true))
            {
                isoLineData.itsParam.GetParam()->SetName(itsDrawParam->ParameterAbbreviation());
            }
            ::ToolMasterDraw(itsToolBox->GetDC(), &isoLineData, relRect, zoomedAreaRect, pixelToGridRatio, -1, GetVisualizationSettings());
        }
    }
    RestoreUpDifferenceDrawing(itsDrawParam);
}

static void CalcDownSizedMatrix(const NFmiDataMatrix<float>& theOrigData, NFmiDataMatrix<float>& theDownSizedData, const NFmiParam& param)
{
    auto paramId = static_cast<FmiParameterName>(param.GetIdent());
    auto interpolationMethod = param.InterpolationMethod();
    auto dontInvertY = true;
    double xDiff = 1.0 / (theDownSizedData.NX() - 1.0);
    double yDiff = 1.0 / (theDownSizedData.NY() - 1.0);
    for(size_t j = 0; j < theDownSizedData.NY(); j++)
    {
        for(size_t i = 0; i < theDownSizedData.NX(); i++)
        {
            NFmiPoint pt(i * xDiff, j * yDiff);
            if(pt.X() > 1.)
                pt.X(1.); // t‰m‰ on varmistus, jos laskenta tarkkuus ongelmat vie rajan yli
            if(pt.Y() > 1.)
                pt.Y(1.); // t‰m‰ on varmistus, jos laskenta tarkkuus ongelmat vie rajan yli
            theDownSizedData[i][j] = theOrigData.InterpolatedValue(pt, paramId, dontInvertY, interpolationMethod);
        }
    }
}

static void BuildDownSizedData(NFmiIsoLineData& theOrigIsoLineData, NFmiIsoLineData& theDownSizedIsoLineData, const NFmiPoint& theDownSizeFactor)
{
    int newSizeX = boost::math::iround(theOrigIsoLineData.itsXNumber / theDownSizeFactor.X());
    int newSizeY = boost::math::iround(theOrigIsoLineData.itsYNumber / theDownSizeFactor.Y());
    // T‰ytet‰‰n uuden isolineDatan hila-arvot halutuille osaalueilleen.
    NFmiDataMatrix<float> downSizedGridData(newSizeX, newSizeY, kFloatMissing);
    ::CalcDownSizedMatrix(theOrigIsoLineData.itsIsolineData, downSizedGridData, *theOrigIsoLineData.itsParam.GetParam());

    // Alustetaan uusi isolinedata harvennetulla matriisilla ja sen piirtoasetukset otetaan originaalista
    theDownSizedIsoLineData.InitIsoLineData(downSizedGridData, &theOrigIsoLineData);
}

static std::string MakeIsoLineDataGridSizeString(NFmiIsoLineData* theIsoLineData)
{
    std::string str = std::to_string(theIsoLineData->itsXNumber);
    str += " x ";
    str += std::to_string(theIsoLineData->itsYNumber);
    return str;
}

void NFmiIsoLineView::DoPossibleIsolineSafetyFeatureDownSizing(NFmiIsoLineData* theIsoLineDataInOut, const NFmiRect& zoomedAreaRect)
{
    NFmiPoint pixelToGridRatio = CalcPixelToGridRatio(*theIsoLineDataInOut, zoomedAreaRect);
    NFmiPoint downSizeFactor;
    // Toolmaster piirto bugi ilmenee kun hila vs. pikseli suhde on n. 1 tai alle. Kierr‰n siten ongelman niin ett‰ kun t‰m‰
    // ratio on tarpeeksi pieni, lasken uuden hila koon, niin ett‰ sen suhdeluku on minimiss‰‰n 1.3 ja interpoloin t‰ll‰iseen
    // uuteen hilaan datan. T‰m‰n j‰lkeen piirto onnistuu ilman ongelmia, eik‰ asiakas huomaa juuri mit‰‰n.
    bool doDownSize = IsolineDataDownSizingNeeded(*theIsoLineDataInOut, pixelToGridRatio, downSizeFactor, itsDrawParam);
    if(doDownSize)
    {
        NFmiIsoLineData downSizedIsoLineData;
        ::BuildDownSizedData(*theIsoLineDataInOut, downSizedIsoLineData, downSizeFactor);
        std::string logMessage = "Down-sizing isoline data for safety reasons from ";
        logMessage += ::MakeIsoLineDataGridSizeString(theIsoLineDataInOut);
        logMessage += " to ";
        logMessage += ::MakeIsoLineDataGridSizeString(&downSizedIsoLineData);
        itsCtrlViewDocumentInterface->LogAndWarnUser(logMessage, "dummy dialog title", CatLog::Severity::Debug, CatLog::Category::Visualization, true);
        
        // Korvataan havennetulla datalla piirrett‰v‰ data
        *theIsoLineDataInOut = downSizedIsoLineData;
        if(NFmiQueryDataUtil::AreAreasSameKind(itsArea.get(), theIsoLineDataInOut->itsInfo->Area()))
        {
            // Jos datan ja karttapohjan projektiot ovat samantyyppisi‰, pit‰‰ itsOptimizedGridPtr p‰ivitt‰‰
            // mahdollista hilapistepiirtoa varten datan n‰kyv‰ll‰ alueella.
            auto visibleDataArea = itsArea->XYArea().Intersection(itsArea->XYArea(theIsoLineDataInOut->itsInfo->Area()));
            UpdateOptimizedGridValues(visibleDataArea, downSizedIsoLineData.itsXNumber, downSizedIsoLineData.itsYNumber);
        }
    }
}

void NFmiIsoLineView::DrawIsoLinesWithToolMaster(void)
{
    if(itsInfo)
    {
        NFmiIsoLineData isoLineData;
        NFmiRect zoomedAreaRect;
        if(FillGridRelatedData(isoLineData, zoomedAreaRect))
        {
            DoPossibleIsolineSafetyFeatureDownSizing(&isoLineData, zoomedAreaRect);
            DoGridRelatedVisualization(isoLineData, zoomedAreaRect);
        }
    }
}

bool NFmiIsoLineView::FillIsoLineDataWithGridData(NFmiIsoLineData& theIsoLineData, int x1, int y1, int x2, int y2, NFmiGrid* optimizedDataGrid)
{
    if(CalcViewFloatValueMatrix(itsIsolineValues, x1, y1, x2, y2, theIsoLineData.fUseOriginalDataInPixelToGridRatioCalculations, optimizedDataGrid) == false)
        return false;

    return initializeIsoLineData(theIsoLineData);
}

bool NFmiIsoLineView::initializeIsoLineData(NFmiIsoLineData &theIsoLineData)
{
    // itsInfo on saattanut muuttua esim. macroParam tapauksessa, miss‰ k‰ytetty RESOLUTION = xxx asetuksia
    theIsoLineData.itsInfo = itsInfo;

    return theIsoLineData.InitIsoLineData(itsIsolineValues);
}