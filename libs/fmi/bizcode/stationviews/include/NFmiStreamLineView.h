//© Ilmatieteenlaitos/software by Marko Pietarinen
//  Original 3.3.2014
//
//
//-------------------------------------------------------------------- NFmiStreamLineView.h

#pragma once

#include "NFmiStationView.h"
#include "NFmiParameterName.h"
#include "NFmiSimplifier.h"
#include "NFmiMilliSecondTimer.h"

class NFmiToolBox;
class NFmiArea;
class GdiPlusLineInfo;
namespace Gdiplus
{
	class PointF;
    class Bitmap;
}

inline double Dot(const NFmiPoint& a, const NFmiPoint& b) 
{ 
    return (a.X()*b.X()) + (a.Y()*b.Y()); 
}

inline double PerpDot(const NFmiPoint& a, const NFmiPoint& b)
{ 
    return (a.Y()*b.X()) - (a.X()*b.Y()); 
}

inline double Norm(const NFmiPoint& c1)
{
    return ::sqrt(::Dot(c1, c1));
}

// L‰hde: http://www.cplusplus.com/forum/beginner/49408/
// koodi muutettu newbase maailmaan
inline bool LineCollision(const NFmiPoint& A1, const NFmiPoint& A2, const NFmiPoint& B1, const NFmiPoint& B2)
{
    NFmiPoint a(A2-A1);
    NFmiPoint b(B2-B1);

    double f = ::PerpDot(a,b);
    if(!f)      // lines are parallel
        return false;
    
    NFmiPoint c(B2-A2);
    double aa = ::PerpDot(a,c);
    double bb = ::PerpDot(b,c);

    if(f < 0)
    {
        if(aa > 0)     return false;
        if(bb > 0)     return false;
        if(aa < f)     return false;
        if(bb < f)     return false;
    }
    else
    {
        if(aa < 0)     return false;
        if(bb < 0)     return false;
        if(aa > f)     return false;
        if(bb > f)     return false;
    }

    return true;
}

static bool LineIntersectsRect(const NFmiPoint &p1, const NFmiPoint &p2, const NFmiRect &r)
{
    return LineCollision(p1, p2, r.BottomLeft(), r.BottomRight()) ||
        LineCollision(p1, p2, r.BottomRight(), r.TopRight()) ||
            LineCollision(p1, p2, r.TopRight(), r.TopLeft()) ||
            LineCollision(p1, p2, r.TopLeft(), r.BottomLeft()) ||
            (r.IsInside(p1) && r.IsInside(p2));
}

class NFmiStreamLineView; // pit‰‰ esitell‰, koska k‰ytet‰‰n luokan metodeissa

class NFmiStreamlineData
{
public:
    NFmiStreamlineData(void);
    NFmiStreamlineData(const std::vector<NFmiPoint> &forwardPath, const std::vector<NFmiPoint> &backwardPath, NFmiStreamLineView *theView, const NFmiPoint &theStartingLatlon, const NFmiPoint &theStartingPointIndex, bool edgeOfDataStartPoint);
    ~NFmiStreamlineData(void){}

    bool HasRealPath(void) const;
    bool IsShorty(double theLengthLimitInKM) const;
    void SimplifyData(NFmiStreamLineView *theView, double theAreaLimit);
    void CropCrossingPaths(const NFmiStreamlineData &theOtherPath, NFmiStreamLineView *theView);
    void CropClosingPaths(const NFmiStreamlineData &theOtherPath, double theLimit, NFmiStreamLineView *theView);

    const std::vector<NFmiPoint>& ForwardPathLatlon(void) const {return itsForwardPathLatlon;}
    const std::vector<NFmiPoint>& BackwardPathLatlon(void) const {return itsBackwardPathLatlon;}
    const std::vector<NFmiPoint>& ForwardPathRelative(void) const {return itsForwardPathRelative;}
    const std::vector<NFmiPoint>& BackwardPathRelative(void) const {return itsBackwardPathRelative;}
    const NFmiPoint& StartingLatlon(void) const {return itsStartingLatlon;}
    const NFmiPoint& StartingPointIndex(void) const {return itsStartingPointIndex;}
    static std::vector<NFmiPoint> SimplifyPath(const std::vector<NFmiPoint> &thePath, double theAreaLimit, size_t theSimplificationStartIndex);
private:
    void CropCrossingPaths(std::vector<NFmiPoint> &theCheckedPath, const std::vector<NFmiPoint> &theOtherPath, bool fromSameData);
    void CropClosingPaths(std::vector<NFmiPoint> &theCheckedPath, const std::vector<NFmiPoint> &theOtherPath, double theLimit);
    void UpdatePaths(NFmiStreamLineView *theView);

    std::vector<NFmiPoint> itsForwardPathLatlon;
    std::vector<NFmiPoint> itsBackwardPathLatlon;
    std::vector<NFmiPoint> itsForwardPathRelative;
    std::vector<NFmiPoint> itsBackwardPathRelative;
    bool fPathsUpdated; // ollaanko kertaalleen laskettu latlon-poluista relatiiviset polut
    NFmiPoint itsStartingLatlon;
    NFmiPoint itsStartingPointIndex;
    bool fEdgeOfDataStartPoint; // jos laskenta alkoi datan reunalta, joskus halutaan poistaa tynk‰ viivoja
};

// Simppeli luokka, johon talletetaan suhteellinen alue ja onko alue jo eliminoitu vai ei.
// Eliminointi tarkoittaa sit‰ ett‰ kun tehd‰‰n streamline polkuja ja polku etenee ja menee yli 
// toisen aloituspisteen alueen, ei kyseisest‰ aloituspisteest‰ kannata alkaa laskemaan en‰‰ streamlineja.
class NFmiStartPointEliminationData
{
public:
    NFmiStartPointEliminationData(void)
    :itsStartPointArea()
    ,itsStartingPointLatlon(NFmiPoint::gMissingLatlon)
    ,itsStartingPointIndex(-1, -1)
    ,fWasUsedAsStartingPoint(false)
    ,fEliminated(false)
    ,fWasOutOfZoomedAreaStartingPoint(false)
    ,itsStartingPointIndexEliminated(-1, -1)
    ,fForwardPathEliminate(false)
    ,itsEliminationPointIndex(-1)
    ,fMarkedShortly(false)
    ,itsPriority(999999)
    {
    }

    NFmiStartPointEliminationData(const NFmiRect &theStartPointArea, const NFmiPoint &theStartingPointLatlon, const NFmiPoint &theStartingPointIndex)
    :itsStartPointArea(theStartPointArea)
    ,itsStartingPointLatlon(theStartingPointLatlon)
    ,itsStartingPointIndex(theStartingPointIndex)
    ,fWasUsedAsStartingPoint(false)
    ,fEliminated(false)
    ,fWasOutOfZoomedAreaStartingPoint(false)
    ,itsStartingPointIndexEliminated(-1, -1)
    ,fForwardPathEliminate(false)
    ,itsEliminationPointIndex(-1)
    ,fMarkedShortly(false)
    ,itsPriority(999999)
    {
    }

    NFmiRect itsStartPointArea; // jonkin suuruin suhteellinen alue aloitupisteen ymp‰rilt‰
    NFmiPoint itsStartingPointLatlon;
    NFmiPoint itsStartingPointIndex;
    bool fWasUsedAsStartingPoint;
    bool fEliminated; // onko t‰m‰ aloituspiste merkitty eliminoiduksi
    bool fWasOutOfZoomedAreaStartingPoint;

    // T‰ss‰ on debuggaus infoa, mik‰ polku ja mik‰ piste eliminoi t‰m‰n aloituspisteen
    NFmiPoint itsStartingPointIndexEliminated;
    bool fForwardPathEliminate;
    size_t itsEliminationPointIndex;
    bool fMarkedShortly; // onko t‰m‰ poistettu siksi ett‰ se on todettu lyhyeksi
    int itsPriority; // Priorisointi luku 0-n, mit‰ pienempi luku, sit‰ korkeampi prioriteetti. Streamline:jen keskialoituspiste on 0-prioriteetti, sitten pysty- ja vaakakeskirivit keskelt‰ reunoja kohden ovat seuraavina jne.
};

class StreamlineCalculationParameters
{
public:
    StreamlineCalculationParameters(const boost::shared_ptr<NFmiDrawParam> &theDrawParam);

    int itsTimeStepInMinutes; // Kun streamlineja lasketaan, ilmapartikkelia liikuttaan tuulen mukana n‰in pitk‰‰n yhteen suuntaan, ennen kuin katsotaan uudessa kohtaa, mihin ja miten lujaa tuuli puhaltaa
    double itsMaxAreaLimit; // streamline polkujen simplifikaatio kerrointa m‰‰ritett‰ess‰ k‰ytet‰‰n t‰t‰ rajana (~0.002)
    double itsMaxLengthInKMLimit; // streamline polkujen simplifikaatio kerrointa m‰‰ritett‰ess‰ k‰ytet‰‰n t‰t‰ rajana (~4200 km)
    float itsLineWidthInMM; // Streamline polun viivan paksuus millimetreiss‰
    NFmiColor itsLineColor; // Streamline viivan v‰ri
    NFmiColor itsArrowHeadColor; // Streamline viivassa olevien nuolien v‰ri
    float itsNoScaleLimit; // Jos t‰m‰ on 0, nuolen p‰‰t ovat samankokoisia. Jos ei, nuolien kokoa skaalataan riippuen paikallisesta tuulen nopeudesta ja t‰st‰ arvosta, jos nopeus alle t‰m‰n luvun, nuolenp‰‰ on pienempi ja jos nopeus on suurempi, nuolenp‰‰ on isompi
    double itsStartPointDistInMM; // Streamline laskuille lasketaan aloituspiste ruudukko, joka perustuu datan hilaan. Aloituspiste ruudukkoon pyrit‰‰n saamaan t‰ll‰inen fyysinen v‰li x- ja y-suunnassa karttan‰ytˆll‰.
    bool fUseRelativeParticleJumps; // Streamline laskut tehd‰‰n joko viem‰ll‰ ilmapartikkelia tuulen mukana (jos false), tai sit‰ hyppyytet‰‰n tuulen suuntaan suhteessa hilapisteiden v‰liin (jos true)
    double itsRelativeParticleJumpLength; // Hilapisteiden v‰li kerrotaan t‰ll‰, jolloin saadaan haluttu partikkelin suhteellinen hyppy pituus
    int itsDoSimplificationLimit; // Kun lasketaan streamline, aina n‰in monen uuden pisteen j‰lkeen lasketaan polun yksinkertaistuslaskut
    float itsRelativeCalcWindSpeedLimit; // jos tuulen nopeus alle t‰m‰n rajan, laskut menev‰t suhteellisiin siirtoihin
    int itsClosingToItselfCheck; // tehd‰‰n itseens‰ l‰hestymis tarkastelut vain joka 3:lla kerralla
    //bool fDoIntersectingItselfCheck; // HUOM! t‰t‰ tarkastelua ei kannata j‰tt‰‰ pois miss‰‰n tapauksessa!!!  // Tehd‰‰nkˆ itsens‰ kanssa leikkaamis testi‰ vai ei
    int itsTooFarFromZoomedAreaCheck; // Kuinka usein tehd‰‰n tarkastelu, onko uusin piste liian kaukana zoomatun alueen ulkopuolella
    double itsTooFarFromZoomedAreaCheckFactor; // Suhteessa zoomatun alueen metriseen maailman (leveys+korkeus)/2, kuinka kaukana piste maksimissaan saa olla
};

class NFmiStreamLineView : public NFmiStationView
{
public:
	NFmiStreamLineView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
							,NFmiToolBox * theToolBox
							,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							,FmiParameterName theParamId
							,int theRowIndex
                            ,int theColumnIndex);
    ~NFmiStreamLineView(void);
	void Draw (NFmiToolBox * theGTB) override;
	std::string ComposeToolTipText(const NFmiPoint& theRelativePoint) override;

protected:
    void DrawStreamLineData(void);
    void DrawStreamLineData2(const StreamlineCalculationParameters &theCalcParams);
    std::vector<NFmiStreamlineData> CalcStreamLinePaths(const StreamlineCalculationParameters &theCalcParams);
    void DoStartingPointCalcualtions(const StreamlineCalculationParameters &theCalcParams, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, std::vector<NFmiStreamlineData> &theLatlonPaths, int timeStepInMinutes, double theProximityLimit, NFmiDataMatrix<NFmiStartPointEliminationData> &theStartingPointEliminatioMatrix, size_t xIndex, size_t yIndex, double theLengthLimitInKM);
    std::vector<NFmiPoint> SearchPathOneDirection(const StreamlineCalculationParameters &theCalcParams, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiPoint &theStartLatlonPoint, int timeStepInMinutes, double theProximityLimit, bool goForwardDir, double theAreaLimit);
    void CropOvelappings(NFmiStreamlineData &theCurrentPath, std::vector<NFmiStreamlineData> &theLatlonPaths, double theProximityLimit);
    void DrawStreamLinePaths(const StreamlineCalculationParameters &theCalcParams, const std::vector<NFmiStreamlineData> &theLatlonPaths);
    void DrawArroyHeads(const StreamlineCalculationParameters &theCalcParams, std::vector<Gdiplus::PointF> &theStreamLineGdiPoints, const GdiPlusLineInfo &theLineInfo, bool forwardDirection, float theLineWidthInMM);
    NFmiDataMatrix<NFmiPoint> CalcPotencialStartingPoints(const StreamlineCalculationParameters &theCalcParams);
    NFmiDataMatrix<NFmiStartPointEliminationData> CalcStartingPointEliminationMatrix(const NFmiDataMatrix<NFmiPoint> &theStartingPointMatrix, boost::shared_ptr<NFmiArea> &theZoomedArea);
    NFmiStreamlineData CalcSingleStreamLinePath(const StreamlineCalculationParameters &theCalcParams, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiPoint &theStartLatlonPoint, const NFmiPoint &theStartPointIndex, int timeStepInMinutes, double theProximityLimit, bool edgeOfDataPoint, double theAreaLimit);
    void CalcWindValues(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiPoint &theLatlon, float &WS, float &WD);
    void DrawOneWayPath(const StreamlineCalculationParameters &theCalcParams, const std::vector<NFmiPoint> &theOneWayPath, const GdiPlusLineInfo &theLineInfo, bool forwardDirection, float theLineWidthInMM);
    void DrawOneWayPath(const StreamlineCalculationParameters &theCalcParams, std::vector<Gdiplus::PointF> &gdiPoints, const GdiPlusLineInfo &lineInfo, bool forwardDirection, float theLineWidthInMM);
    void DrawDebugRect(int sizeInPixels, const NFmiPoint &theLatlon, const NFmiColor &theColor);
    void DrawDebugPath(const NFmiStreamlineData &thePath, int rectSizeInPixels, const NFmiColor &theRectColor, const NFmiColor &theFontColor);
    void DrawDebugPath(const std::vector<NFmiPoint>& theVectorPath, int rectSizeInPixels, const NFmiColor &theRectColor, const NFmiColor &theFontColor);
    void DrawDebugString(double fontSizeInMM, const NFmiPoint &thePoint, const NFmiColor &theColor, const std::string &theText, bool pointInLatlon);
    void DrawDebugEliminationMatrixInfo(const NFmiDataMatrix<NFmiStartPointEliminationData> &theStartingPointEliminatioMatrix);
    void DrawDebugEliminationInfo(const NFmiStartPointEliminationData &theEliminatioInfo);
    void DoSingleThreadCalculations(const StreamlineCalculationParameters &theCalcParams, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiDataMatrix<NFmiStartPointEliminationData> &theStartingPointEliminatioMatrix, int timeStepInMinutes, std::vector<NFmiStreamlineData> &theLatlonPathsOut, double theProximityLimit, double theLengthLimitInKM);
    void DoMultiThreadCalculations(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiDataMatrix<NFmiStartPointEliminationData> &theStartingPointEliminatioMatrix, int timeStepInMinutes, std::vector<NFmiStreamlineData> &theLatlonPathsOut, double theProximityLimit, double theLengthLimitInKM);
    void LogStreamlineDebugInfo(const StreamlineCalculationParameters &theCalcParams);

	double itsScreenPixelSizeInMM; // t‰h‰n pit‰‰ saada talteen pikselin koko mm:eiss‰ silloin kun ei olla printtaamassa, 
								// koska viivan paksuus on annettu ruudulla n‰kyviss‰ pikselikoossa.

    // Lasketaan tuulen u- ja v- komponentit matriisiin, t‰llˆin niit‰ voidaan tasoittaa ja laskuoperaatiot ovat nopeampia.
    // N‰iden avulla lasketaan sitten haluttuihin pisteisiin WS ja WD arvot.
    NFmiDataMatrix<float> itsWindUComponent;
    NFmiDataMatrix<float> itsWindVComponent;

    // streamline laskuihin liittyv‰‰ debug infoa, jotka voidaan printata ruudulle halutessa
    void ResetDebugData(void);
    void DrawDebugDataOnMap(void);
    size_t itsDebugTotalStartingPoints;
    size_t itsDebugOutOfAreaStartingPoints;
    size_t itsDebugCalculatedStartingPoints;
    size_t itsDebugEliminatedStartingPoints;
    size_t itsDebugTotalPointsCalculated;
    size_t itsDebugTotalSimplifiedPoints;
    size_t itsDebugTotalStreamLineCount;
    size_t itsDebugTotalRelativeJumpPointsCalculated;
    NFmiMilliSecondTimer itsDebugTimer1;
    size_t itsDebugCombinedTimerMS1;
    NFmiMilliSecondTimer itsDebugTimer2;
    size_t itsDebugCombinedTimerMS2;
    NFmiMilliSecondTimer itsDebugTimer3;
    size_t itsDebugCombinedTimerMS3;
};

