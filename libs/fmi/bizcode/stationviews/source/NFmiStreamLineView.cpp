//© Ilmatieteenlaitos/Marko Pietarinen
//  Original 3.3.2014
//
//
//-------------------------------------------------------------------- NFmiStreamLineView.cpp
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ k‰‰nt‰j‰n varoitusta (liian pitk‰ nimi >255 merkki‰ joka johtuu 'puretuista' STL-template nimist‰)
#endif

#include "NFmiStreamLineView.h"
#include "NFmiToolBox.h"
#include "NFmiDrawParam.h"
#include "NFmiArea.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiQueryData.h"
#include "NFmiConceptualDataView.h"
#include "NFmiDictionaryFunction.h"
#include "CtrlViewDocumentInterface.h"
#include "GraphicalInfo.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "GdiPlusLineInfo.h"
#include "CtrlViewKeyboardFunctions.h"
#include "CtrlViewFunctions.h"
#include "catlog/catlog.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "NFmiFastInfoUtils.h"
#include "NFmiQueryDataUtil.h"

#include <gdiplus.h>

#include <boost/thread.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Jos haluat n‰hd‰ debug infoa piirrett‰v‰n ruudulle, laita true t‰m‰n arvoksi.
bool gDrawDebugInfo = false;


class Triangle 
{
public:
    int indices[3];
    double area;
    Triangle * prev;
    Triangle * next;
};

static bool CompareTri(Triangle *i, Triangle *j)
{
    // important note here:
    // http://stackoverflow.com/questions/12290479/stdsort-fails-on-stdvector-of-pointers
    
    if (i->area != j->area) 
        return i->area < j->area;
    else 
        return false;
}

//--------------------------------------------------------------
static double TriArea(const NFmiPoint &d0, const NFmiPoint &d1, const NFmiPoint &d2)
{
    double dArea = ((d1.X() - d0.X())*(d2.Y() - d0.Y()) - (d2.X() - d0.X())*(d1.Y() - d0.Y()))/2.0;
    return (dArea > 0.0) ? dArea : -dArea;
}

//--------------------------------------------------------------
void CalcVisvaSimplification(std::vector<NFmiPoint> &pathToSimplify, std::vector<NFmiPoint> &resultPath)
{
    resultPath = pathToSimplify;
    int total = static_cast<int>(pathToSimplify.size());
    // if we have 100 points, we have 98 triangles to look at
    int nTriangles = total - 2;
    std::vector<Triangle*> triangles(nTriangles);
    for (int i = 1; i < total-1; i++)
    {
        Triangle *tempTri = new Triangle;
        tempTri->indices[0] = i-1;
        tempTri->indices[1] = i;
        tempTri->indices[2] = i+1;
        tempTri->area = TriArea(pathToSimplify[tempTri->indices[0]],
                                pathToSimplify[tempTri->indices[1]],
                                pathToSimplify[tempTri->indices[2]]);
        triangles[i-1] = tempTri;
    }
    
    // set the next and prev triangles, use NULL on either end. this helps us update traingles that might need to be removed
    for (int i = 0; i < nTriangles; i++)
    {
        triangles[i]->prev = (i == 0 ? NULL : triangles[i-1]);
        triangles[i]->next = (i == nTriangles-1 ? NULL : triangles[i+1]);
    }
    
    std::vector<Triangle*> trianglesVec;
 
    for (int i = 0; i < nTriangles; i++)
    {
        trianglesVec.push_back(triangles[i]);
    }
    
    int count = 0;
    while (!trianglesVec.empty())
    {
        std::sort(trianglesVec.begin(), trianglesVec.end(), ::CompareTri);
        Triangle * tri = trianglesVec[0];
//        resultPath[tri->indices[1]].z = total - count; // store the "importance" of this point in numerical order of removal (but inverted, so 0 = most improtant, n = least important. end points are 0.
        count ++;
        
        if (tri->prev != NULL)
        {
            tri->prev->next = tri->next;
            tri->prev->indices[2] = tri->indices[2]; // check!
            
            tri->prev->area = ::TriArea(pathToSimplify[tri->prev->indices[0]],
                                        pathToSimplify[tri->prev->indices[1]],
                                        pathToSimplify[tri->prev->indices[2]]);
        }
        
        if (tri->next != NULL)
        {
            tri->next->prev = tri->prev;
            tri->next->indices[0] = tri->indices[0]; // check!
            tri->next->area = TriArea(pathToSimplify[tri->next->indices[0]],
                                      pathToSimplify[tri->next->indices[1]],
                                      pathToSimplify[tri->next->indices[2]]);
        }
        trianglesVec.erase(trianglesVec.begin());
    }
    
    // free the memory we just allocated above.
    for (int i = 0; i < nTriangles; i++)
    {
        delete triangles[i];
    }
}

// Markon oma Visvalingam-Whyatt polun yksinkertaistus funktio,
// k‰ytetty pohjana pseudo-koodia seuraavalta sivulta:
// http://web.cs.sunyit.edu/~poissad/projects/Curve/about_algorithms/whyatt
static std::vector<NFmiPoint> VisvalingamWhyattSimplification(const std::vector<NFmiPoint> &theOrigPath, double theAreaLimit, size_t theSimplificationStartIndex = 0)
{
    if(theOrigPath.size() < 3)
        return theOrigPath;
    else
    {
        std::list<NFmiPoint> points(theOrigPath.begin()+theSimplificationStartIndex, theOrigPath.end());
        std::list<NFmiPoint>::iterator pointsIter = points.begin();
        NFmiPoint *p1 = &(*pointsIter);
        NFmiPoint *p2 = &(*++pointsIter);
        NFmiPoint *p3 = 0;
        ++pointsIter;
        std::list<double> areas;
        for( ; pointsIter != points.end(); ++pointsIter)
        {
            p3 = &(*pointsIter);
            areas.push_back(::TriArea(*p1, *p2, *p3));
            p1 = p2;
            p2 = p3;
        }

        bool removed = false;
        do
        {
            removed = false;
            std::list<double>::iterator minElement = std::min_element(areas.begin(), areas.end());
            if(*minElement < theAreaLimit)
            {
                removed = true;
                // poistetaan pienint‰ aluetta vastaava area ja point listoista
                size_t dist = std::distance(areas.begin(), minElement);
                std::list<NFmiPoint>::iterator removedPoint = points.begin();
                std::advance(removedPoint, dist+1);
                // std::list erase palauttaa sen iteraattorin, joka on poistettavasta seuraava
                std::list<double>::iterator erasedAreaIter = areas.erase(minElement);
                std::list<NFmiPoint>::iterator erasedPointIter = points.erase(removedPoint);

                if(areas.size())
                { // pit‰‰ olla v‰h. 1 area listassa, ennen kuin niit‰ p‰ivitet‰‰n
                    // sitten pit‰‰ laskea yksi/kaksi areaa uusiksi, koska niiden toinen reunapiste on muuttunut
                    if(dist == 0)
                    { // pit‰‰ laskea vain 1. area uusiksi
                        pointsIter = points.begin();
                        p1 = &(*pointsIter);
                        p2 = &(*++pointsIter);
                        p3 = &(*++pointsIter);
                        std::list<double>::iterator areaIter = areas.begin();
                        *areaIter = ::TriArea(*p1, *p2, *p3);
                    }
                    else if(dist == areas.size())
                    { // pit‰‰ laskea vain viimeinen area uusiksi
                        pointsIter = points.end();
                        p3 = &(*--pointsIter);
                        p2 = &(*--pointsIter);
                        p1 = &(*--pointsIter);
                        std::list<double>::iterator areaIter = areas.end();
                        --areaIter;
                        *areaIter = ::TriArea(*p1, *p2, *p3);
                    }
                    else
                    { // p‰ivitet‰‰n 2 areaa poistetun pisteen molemmilta puolilta
                        pointsIter = erasedPointIter;
                        --pointsIter;
                        --pointsIter;
                        p1 = &(*pointsIter);
                        p2 = &(*++pointsIter);
                        p3 = &(*++pointsIter);
                        --erasedAreaIter;
                        *erasedAreaIter = ::TriArea(*p1, *p2, *p3);
                        p1 = p2;
                        p2 = p3;
                        p3 = &(*++pointsIter);
                        ++erasedAreaIter;
                        *erasedAreaIter = ::TriArea(*p1, *p2, *p3);
                    }
                }
            }
        }while(removed && points.size() > 2);

        if(theSimplificationStartIndex > 0)
            theSimplificationStartIndex--;
        std::vector<NFmiPoint> tmpPath(theOrigPath.begin(), theOrigPath.begin()+theSimplificationStartIndex);
        tmpPath.insert(tmpPath.end(), points.begin(), points.end());
        return tmpPath;
    }
}

static void TestVisvaFunction(void)
{
    std::vector<NFmiPoint> path1; 
    double areaLimit = 0.2;
    std::vector<NFmiPoint> simplifiedPath1 = ::VisvalingamWhyattSimplification(path1, areaLimit);
    std::vector<NFmiPoint> path2; 
    path2.push_back(NFmiPoint(1, 1));
    path2.push_back(NFmiPoint(2, 1));
    path2.push_back(NFmiPoint(3, 1.1));
    std::vector<NFmiPoint> simplifiedPath2 = ::VisvalingamWhyattSimplification(path2, areaLimit);
    std::vector<NFmiPoint> path3(path2); 
    path3.push_back(NFmiPoint(4, 1.8));
    std::vector<NFmiPoint> simplifiedPath3 = ::VisvalingamWhyattSimplification(path3, areaLimit);
    std::vector<NFmiPoint> path4; 
    path4.push_back(NFmiPoint(1, 1));
    path4.push_back(NFmiPoint(2, 1.1));
    path4.push_back(NFmiPoint(3, 1.15));
    path4.push_back(NFmiPoint(6, 5.1));
    path4.push_back(NFmiPoint(4, 6.8));
    std::vector<NFmiPoint> simplifiedPath4 = ::VisvalingamWhyattSimplification(path4, areaLimit);
    std::vector<NFmiPoint> path5; 
    path5.push_back(NFmiPoint(1, 1));
    path5.push_back(NFmiPoint(2, 3.6));
    path5.push_back(NFmiPoint(3, 3.58));
    path5.push_back(NFmiPoint(4, 3.61));
    path5.push_back(NFmiPoint(3, 6.8));
    std::vector<NFmiPoint> simplifiedPath5 = ::VisvalingamWhyattSimplification(path5, areaLimit);
    int x = 0;
}

// L‰hde: http://www.john.geek.nz/2009/03/code-shortest-distance-between-any-two-line-segments/
// Muutin koodin newbase:lle ja vain karsittu 2D ratkaisu
static double GetShortestDistance(const NFmiPoint& A1, const NFmiPoint& A2, const NFmiPoint& B1, const NFmiPoint& B2)
{
    double EPS = 0.00000001;
 
    NFmiPoint delta21 = A2 - A1;
    NFmiPoint delta41 = B2 - A1;
    NFmiPoint delta13 = A1 - B1;
 
    double a = ::Dot(delta21, delta21);
    double b = ::Dot(delta21, delta41);
    double c = ::Dot(delta41, delta41);
    double d = ::Dot(delta21, delta13);
    double e = ::Dot(delta41, delta13);
    double D = a * c - b * b;
 
    double sc, sN, sD = D;
    double tc, tN, tD = D;
 
    if (D < EPS)
    {
        sN = 0.0;
        sD = 1.0;
        tN = e;
        tD = c;
    }
    else
    {
        sN = (b * e - c * d);
        tN = (a * e - b * d);
        if (sN < 0.0)
        {
            sN = 0.0;
            tN = e;
            tD = c;
        }
        else if (sN > sD)
        {
            sN = sD;
            tN = e + b;
            tD = c;
        }
    }
 
    if (tN < 0.0)
    {
        tN = 0.0;
 
        if (-d < 0.0)
            sN = 0.0;
        else if (-d > a)
            sN = sD;
        else
        {
            sN = -d;
            sD = a;
        }
    }
    else if (tN > tD)
    {
        tN = tD;
        if ((-d + b) < 0.0)
            sN = 0;
        else if ((-d + b) > a)
            sN = sD;
        else
        {
            sN = (-d + b);
            sD = a;
        }
    }
 
    if (::fabs(sN) < EPS) sc = 0.0;
    else sc = sN / sD;
    if (::fabs(tN) < EPS) tc = 0.0;
    else tc = tN / tD;
 
    NFmiPoint dP;
    dP.X(delta13.X() + (sc * delta21.X()) - (tc * delta41.X()));
    dP.Y(delta13.Y() + (sc * delta21.Y()) - (tc * delta41.Y()));
 
    return ::Norm(dP);
}
 
// Menev‰tkˆ viivat 1 ja 2 ristiin? 
// Kun katsotaan viivaa 1, joka on path:ss‰ ja joka alkaa index:st‰.
// Eli viiva 1 on: path[index] -> path[index + 1]
// Viiva 2 on: p2 -> p3
// Poikkeus heitet‰‰n, jos joku path ei sis‰ll‰ niin montaa pistett‰ kuin index+1 odottaa.
static bool IsLinesIntersecting(const std::vector<NFmiPoint> &path, size_t index, const NFmiPoint &p2, const NFmiPoint &p3)
{
    if(path.size() <= index + 1)
        throw std::runtime_error("Error in NFmiStreamLineView.cpp IsLinesIntersecting(path, index, p2, p3) -function: path too small to given index");

    return LineCollision(path[index], path[index + 1], p2, p3);
}

// ***************************************************
// ************* NFmiStreamlineData ******************
// ***************************************************

NFmiStreamlineData::NFmiStreamlineData(void)
:itsForwardPathLatlon()
,itsBackwardPathLatlon()
,itsForwardPathRelative()
,itsBackwardPathRelative()
,fPathsUpdated(false)
,itsStartingLatlon(NFmiPoint::gMissingLatlon)
,itsStartingPointIndex(-1,-1)
,fEdgeOfDataStartPoint(false)
{
}

NFmiStreamlineData::NFmiStreamlineData(const std::vector<NFmiPoint> &forwardPath, const std::vector<NFmiPoint> &backwardPath, NFmiStreamLineView *theView, const NFmiPoint &theStartingLatlon, const NFmiPoint &theStartingPointIndex, bool edgeOfDataStartPoint)
:itsForwardPathLatlon(forwardPath)
,itsBackwardPathLatlon(backwardPath)
,itsForwardPathRelative()
,itsBackwardPathRelative()
,fPathsUpdated(false)
,itsStartingLatlon(theStartingLatlon)
,itsStartingPointIndex(theStartingPointIndex)
,fEdgeOfDataStartPoint(edgeOfDataStartPoint)
{
    UpdatePaths(theView);
}

static std::vector<NFmiPoint> CalcRelativePath(const std::vector<NFmiPoint> &theLatlonPath, NFmiStreamLineView *theView)
{
    std::vector<NFmiPoint> relativePath;
    for(size_t i = 0; i < theLatlonPath.size(); i++)
        relativePath.push_back(theView->LatLonToViewPoint(theLatlonPath[i]));
    return relativePath;
}

static std::vector<NFmiPoint> CalcLatlonPath(const std::vector<NFmiPoint> &theRelativePath, NFmiStreamLineView *theView)
{
    std::vector<NFmiPoint> latlonPath;
    for(size_t i = 0; i < theRelativePath.size(); i++)
        latlonPath.push_back(theView->ViewPointToLatLon(theRelativePath[i]));
    return latlonPath;
}

static void DoAutoPathUpdate(std::vector<NFmiPoint> &theLatlonPath, std::vector<NFmiPoint> &theRelativePath, NFmiStreamLineView *theView)
{
    if(theLatlonPath.size() != theRelativePath.size())
    { // tehd‰‰n p‰ivitys vain jos pituuksissa on eroja
        // p‰ivitet‰‰n sen polun avulla joka on lyhyempi, koska sit‰ polkua on luultavasti juuri leikattu jossain tarkastuksissa
        if(theLatlonPath.size() < theRelativePath.size())
            theRelativePath = ::CalcRelativePath(theLatlonPath, theView);
        else if(theRelativePath.size() < theLatlonPath.size())
            theLatlonPath = ::CalcLatlonPath(theRelativePath, theView);
        // else jos polut saman mittaisia, ei tarvitse tehd‰ mit‰‰n
    }
}

void NFmiStreamlineData::UpdatePaths(NFmiStreamLineView *theView)
{
    if(!fPathsUpdated)
    {
        itsForwardPathRelative = ::CalcRelativePath(itsForwardPathLatlon, theView);
        itsBackwardPathRelative = ::CalcRelativePath(itsBackwardPathLatlon, theView);
    }
    else
    { // alustus on tehty ainakin kerran, nyt katsotaan kummassa (latlon/relative) on lyhyempi polku ja p‰ivitet‰‰n sen kautta toinen
        ::DoAutoPathUpdate(itsForwardPathLatlon, itsForwardPathRelative, theView);
        ::DoAutoPathUpdate(itsBackwardPathLatlon, itsBackwardPathRelative, theView);
    }
    fPathsUpdated = true; // nyt ainakin molemmat polut on alustettu
}

bool NFmiStreamlineData::HasRealPath(void) const
{
    if(itsForwardPathLatlon.size() > 1 || itsBackwardPathLatlon.size() > 1)
        return true;
    else
        return false;
}

static double CalcLineLengthInKM(const NFmiPoint &p1, const NFmiPoint &p2)
{
    NFmiLocation loc(p1);
    return loc.Distance(p2) / 1000.;
}

// Jos on vain 2 pisteen p‰tk‰, yleens‰ se on turhake ja n‰ytt‰‰ 
// rumalta kartalla ja se kannattaa poistaa piirrett‰vien joukosta.
// Leikkaus koodit voivat j‰tt‰‰ myˆs toiseen suuntaan menev‰n polun 
// pituudeksi 1:n, joka on siis vain piste, ja se pit‰‰ huomioida 
// t‰ss‰ poistettavaksi.
bool NFmiStreamlineData::IsShorty(double theLengthLimitInKM) const
{
    if(itsForwardPathLatlon.size() <= 2 && itsBackwardPathLatlon.size() <= 1)
        return true;
    else if(itsForwardPathLatlon.size() <= 1 && itsBackwardPathLatlon.size() <= 2)
        return true;
    else
    {
        if(itsForwardPathLatlon.size() == 2 && itsBackwardPathLatlon.size() == 2)
        {   // jos molemmat olivat yhden viivan pituisia
            // Jos kyse oli ns. datan reuna pisteesta, todetaan polku shortyksi
            if(fEdgeOfDataStartPoint)
                return true;
            else
            {
                // lasketaan jos toinen viivoista on metreiss‰ todella lyhyt ja luokitellaan se t‰llˆin lyhyeksi
                if(::CalcLineLengthInKM(itsForwardPathLatlon[0], itsForwardPathLatlon[1]) <= theLengthLimitInKM)
                    return true;
                else if(::CalcLineLengthInKM(itsBackwardPathLatlon[0], itsBackwardPathLatlon[1]) <= theLengthLimitInKM)
                    return true;
            }
        }
        return false;
    }
}

// SimplyfyPath kutsutaan latlon poluille, mutta CropCrossingPaths pit‰‰ 
// kutsua relatiivisille poluille.
void NFmiStreamlineData::SimplifyData(NFmiStreamLineView *theView, double theAreaLimit)
{
    itsForwardPathLatlon = SimplifyPath(itsForwardPathLatlon, theAreaLimit, 0);
    itsBackwardPathLatlon = SimplifyPath(itsBackwardPathLatlon, theAreaLimit, 0);
    UpdatePaths(theView); // pakko p‰ivitt‰‰ relatiivia polkuja, koska latlon polkujen koko sattoi muuttua
    CropCrossingPaths(itsBackwardPathRelative, itsForwardPathRelative, true);
    ::DoAutoPathUpdate(itsBackwardPathLatlon, itsBackwardPathRelative, theView); // pakko p‰ivitt‰‰ backward relatiivista polkua, koska sen latlon vastine saattoi muuttua
}

std::vector<NFmiPoint> NFmiStreamlineData::SimplifyPath(const std::vector<NFmiPoint> &thePath, double theAreaLimit, size_t theSimplificationStartIndex)
{
    return ::VisvalingamWhyattSimplification(thePath, theAreaLimit, theSimplificationStartIndex);
}

void NFmiStreamlineData::CropCrossingPaths(const NFmiStreamlineData &theOtherPath, NFmiStreamLineView *theView)
{
    CropCrossingPaths(itsForwardPathRelative, theOtherPath.itsForwardPathRelative, false);
    CropCrossingPaths(itsForwardPathRelative, theOtherPath.itsBackwardPathRelative, false);
    CropCrossingPaths(itsBackwardPathRelative, theOtherPath.itsForwardPathRelative, false);
    CropCrossingPaths(itsBackwardPathRelative, theOtherPath.itsBackwardPathRelative, false);
    UpdatePaths(theView);
}

// fromSameData tarkoittaa ovatko polut samasta vai eri otuksesta, jos ne ovat samasta, silloin aloitetaan indeksist‰ 1, koska muuten
// samasta datasta tulevat forward ja backward polut voivat menn‰ pikkuisen ristiin, heti alussa, laskenta tarkkuuden takia.
void NFmiStreamlineData::CropCrossingPaths(std::vector<NFmiPoint> &theCheckedPath, const std::vector<NFmiPoint> &theOtherPath, bool fromSameData)
{
    size_t startinIndex = fromSameData ? 1 : 0;
    if(theCheckedPath.size() > startinIndex+1 && theOtherPath.size() > startinIndex+1)
    {
        for(size_t i = startinIndex; i < theCheckedPath.size()-1; i++)
        {
            for(size_t j = startinIndex; j < theOtherPath.size()-1; j++)
            {
                if(::IsLinesIntersecting(theOtherPath, j, theCheckedPath[i], theCheckedPath[i+1]))
                { // joku viiva leikkasi, leikataan theCheckedPath i;st‰ alkean pois ja lopetetaan
                    theCheckedPath.resize(i);
                    return ;
                }
            }
        }
    }
}

// Lasketaan l‰hestyvien polkujen ongelma viel‰ latlon maailmassa, koska limitti on laskettu siin‰
void NFmiStreamlineData::CropClosingPaths(const NFmiStreamlineData &theOtherPath, double theLimit, NFmiStreamLineView *theView)
{
    CropClosingPaths(itsForwardPathLatlon, theOtherPath.itsForwardPathLatlon, theLimit);
    CropClosingPaths(itsForwardPathLatlon, theOtherPath.itsBackwardPathLatlon, theLimit);
    CropClosingPaths(itsBackwardPathLatlon, theOtherPath.itsForwardPathLatlon, theLimit);
    CropClosingPaths(itsBackwardPathLatlon, theOtherPath.itsBackwardPathLatlon, theLimit);
    UpdatePaths(theView);
}

void NFmiStreamlineData::CropClosingPaths(std::vector<NFmiPoint> &theCheckedPath, const std::vector<NFmiPoint> &theOtherPath, double theLimit)
{
    if(theCheckedPath.size() > 1 && theOtherPath.size() > 1)
    {
        for(size_t i = 0; i < theCheckedPath.size()-1; i++)
        {
            for(size_t j = 0; j < theOtherPath.size()-1; j++)
            {
                if(::GetShortestDistance(theCheckedPath[i], theCheckedPath[i+1], theOtherPath[j], theOtherPath[j+1]) < theLimit)
                { // joku viiva liippaa nyt tarpeeksi l‰helt‰, leikataan theCheckedPath i+1:st‰ alkean pois ja lopetetaan
                    theCheckedPath.resize(i+1);
                    return ;
                }
            }
        }
    }
}

StreamlineCalculationParameters::StreamlineCalculationParameters(const boost::shared_ptr<NFmiDrawParam> &theDrawParam)
:itsTimeStepInMinutes(30)
,itsMaxAreaLimit(4./2000.)
,itsMaxLengthInKMLimit(6.*700.)
,itsLineWidthInMM(0.2f)
,itsLineColor(0,0,1)
,itsArrowHeadColor(0,0,0)
,itsNoScaleLimit(0)
,itsStartPointDistInMM(10)
,fUseRelativeParticleJumps(false)
,itsRelativeParticleJumpLength(0.1)
,itsDoSimplificationLimit(10)
,itsRelativeCalcWindSpeedLimit(5)
,itsClosingToItselfCheck(3)
,itsTooFarFromZoomedAreaCheck(5)
,itsTooFarFromZoomedAreaCheckFactor(0.04)
{
    if(theDrawParam)
    {
        itsTimeStepInMinutes = FmiRound(30 * theDrawParam->ContourGab());
        if(itsTimeStepInMinutes < 1)
            itsTimeStepInMinutes = 1;
        itsMaxAreaLimit = theDrawParam->SimpleIsoLineLabelHeight() / 2000; // default on 4, joten jaetaan arvo sopivalla luvulla ett‰ defaultista saadaan sopiva alkuarvaus (~0.001 - 0.003)
        itsMaxLengthInKMLimit = theDrawParam->IsoLineSplineSmoothingFactor() * 700;  // default on 6, joten kerrotaan arvo sopivalla luvulla ett‰ defaultista saadaan sopiva alkuarvaus (3000 - 6000)
        itsLineWidthInMM = theDrawParam->SimpleIsoLineWidth();
        itsLineColor = theDrawParam->IsolineColor();
        itsArrowHeadColor = theDrawParam->IsolineTextColor();
        itsNoScaleLimit = theDrawParam->SimpleIsoLineZeroValue();
        itsStartPointDistInMM = theDrawParam->IsoLineGab() * 10;
        fUseRelativeParticleJumps = theDrawParam->DrawOnlyOverMask();
        itsRelativeParticleJumpLength = theDrawParam->SimpleIsoLineLabelHeight() / 10.;
        if(itsRelativeParticleJumpLength == 0)
            itsRelativeParticleJumpLength = 0.1;
//        if(theDrawParam->UseIsoLineFeathering())
    }
}


// ***************************************************
// ************* NFmiStreamLineView ******************
// ***************************************************

NFmiStreamLineView::NFmiStreamLineView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
											,NFmiToolBox *theToolBox
											,boost::shared_ptr<NFmiDrawParam> &theDrawParam
											,FmiParameterName theParamId
											,int theRowIndex
                                            ,int theColumnIndex)
: NFmiStationView(theMapViewDescTopIndex, theArea
			  ,theToolBox
			  ,theDrawParam
			  ,theParamId
			  ,NFmiPoint(0, 0)
			  ,NFmiPoint(1, 1)
			  ,theRowIndex
              ,theColumnIndex)
,itsWindUComponent()
,itsWindVComponent()
,itsDebugTotalStartingPoints(0)
,itsDebugOutOfAreaStartingPoints(0)
,itsDebugCalculatedStartingPoints(0)
,itsDebugEliminatedStartingPoints(0)
,itsDebugTotalPointsCalculated(0)
,itsDebugTotalSimplifiedPoints(0)
,itsDebugTotalStreamLineCount(0)
,itsDebugTotalRelativeJumpPointsCalculated(0)
{
}

NFmiStreamLineView::~NFmiStreamLineView(void)
{
}

void NFmiStreamLineView::Draw(NFmiToolBox *theGTB)
{
    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, __FUNCTION__);
    //    ::TestVisvaFunction();
    //    return ;

	if(!theGTB)
		return;
	itsToolBox = theGTB;
	if(!IsParamDrawn())
		return;

	if(itsToolBox->GetDC()->IsPrinting() == FALSE)
		itsScreenPixelSizeInMM = 1./itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_x;

	try
	{
		InitializeGdiplus(itsToolBox, &GetFrame());
		itsGdiPlusGraphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias); // Huom. antialiasointi saattaa hidastaa yll‰tt‰v‰n paljon piirtoa (ei kuitenkaan nyt testien mukaan)

		DrawStreamLineData();
	}
    catch(std::exception &e)
	{
        itsCtrlViewDocumentInterface->LogAndWarnUser(e.what(), "", CatLog::Severity::Error, CatLog::Category::Visualization, true);
	}
	catch(...)
	{
	}
	CleanGdiplus(); // t‰t‰ pit‰‰ kutsua piirron lopuksi InitializeGdiplus -metodin vastin parina.
}

void NFmiStreamLineView::LogStreamlineDebugInfo(const StreamlineCalculationParameters &theCalcParams)
{
    bool logStreamlineDebugInfo = false;
    if(!logStreamlineDebugInfo)
        return ;

//    if(itsDoc->MapViewDescTop(itsMapViewDescTopIndex)->AnimationDataRef().AnimationOn())
    {
        std::string logStr("StreamLine calculations and drawing debug information:\n");
        logStr += "Streamline calcualtions: ";
        logStr += itsDebugTimer1.EasyTimeDiffStr();
        logStr += "\nStreamline drawing: ";
        logStr += itsDebugTimer2.EasyTimeDiffStr();
        logStr += "\nStreamline count: ";
        logStr += NFmiStringTools::Convert(itsDebugTotalStreamLineCount);
        logStr += "\nSimplified point count: ";
        logStr += NFmiStringTools::Convert(itsDebugTotalSimplifiedPoints);
        logStr += "\nTotal point count: ";
        logStr += NFmiStringTools::Convert(itsDebugTotalPointsCalculated);
        logStr += "\nTotal relative jump point count: ";
        logStr += NFmiStringTools::Convert(itsDebugTotalRelativeJumpPointsCalculated);

        logStr += "\nsimpl-limit: ";
        logStr += NFmiStringTools::Convert(theCalcParams.itsDoSimplificationLimit);
        logStr += " closing-limit: ";
        logStr += NFmiStringTools::Convert(theCalcParams.itsClosingToItselfCheck);
        logStr += " rel-jump-WS-limit: ";
        logStr += NFmiStringTools::Convert(theCalcParams.itsRelativeCalcWindSpeedLimit);
        logStr += " rel-jump-len: ";
        logStr += NFmiStringTools::Convert(theCalcParams.itsRelativeParticleJumpLength);
        logStr += " too-far-check: ";
        logStr += NFmiStringTools::Convert(theCalcParams.itsTooFarFromZoomedAreaCheck);

        itsCtrlViewDocumentInterface->LogAndWarnUser(logStr, "", CatLog::Severity::Debug, CatLog::Category::Visualization, true);
    }
}

void NFmiStreamLineView::DrawStreamLineData(void)
{ 
    itsInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, false, false);
    if(itsInfo && itsInfo->Grid() && itsInfo->TimeDescriptor().IsInside(itsTime))
    {
        if(itsInfo->Param(kFmiWindUMS) && itsInfo->Param(kFmiWindVMS))
        {
            itsInfo->Param(kFmiWindUMS);
            itsInfo->Values(itsWindUComponent, itsTime);
            itsInfo->Param(kFmiWindVMS);
            itsInfo->Values(itsWindVComponent, itsTime);
        }
        else if(itsInfo->Param(kFmiWindSpeedMS) && itsInfo->Param(kFmiWindDirection))
        {
            itsInfo->Param(kFmiWindSpeedMS);
            NFmiDataMatrix<float> ws;
            itsInfo->Values(ws, itsTime);
            itsInfo->Param(kFmiWindDirection);
            NFmiDataMatrix<float> wd;
            itsInfo->Values(wd, itsTime);
            NFmiFastInfoUtils::CalcMatrixWindComponentsFromSpeedAndDirection(ws, wd, itsWindUComponent, itsWindVComponent);
        }
        else
            return;

        StreamlineCalculationParameters calcParams(itsDrawParam);

//        for(int doSimplificationLimit = 5; doSimplificationLimit < 15; doSimplificationLimit++)
//        {
            DrawStreamLineData2(calcParams);
//        }
    }
}

// T‰lle funktiolle voidaan antaa halutunlainen StreamlineCalculationParameters -olio ja tehd‰ testej‰ sill‰.
void NFmiStreamLineView::DrawStreamLineData2(const StreamlineCalculationParameters &theCalcParams)
{
    itsDebugTimer1.StartTimer();
    std::vector<NFmiStreamlineData> latlonPaths = CalcStreamLinePaths(theCalcParams);
    itsDebugTimer1.StopTimer();

    itsDebugTimer2.StartTimer();
    DrawStreamLinePaths(theCalcParams, latlonPaths);
    itsDebugTimer2.StopTimer();
    LogStreamlineDebugInfo(theCalcParams);
}

static void UVcomponents2SpeedDir(float u, float v, float &WS, float &WD)
{
    WS = kFloatMissing;
    WD = kFloatMissing;
    NFmiWindDirection windDirection(u, v);
    if(windDirection.Value() != kFloatMissing)
    {
        WD = static_cast<float>(windDirection.Value());
        WS = ::sqrt(u*u + v*v);
    }
}

static float InterpolateValueFromMatrix(const NFmiDataMatrix<float> &theMatrix, const NFmiPoint &theLatlonPoint, const NFmiArea *theDataArea)
{
    NFmiPoint p = theDataArea->LatLonToWorldXY(theLatlonPoint);
    return theMatrix.InterpolatedValue(p, theDataArea->WorldRect(), kFmiWindUMS, true);
}

void NFmiStreamLineView::CalcWindValues(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiPoint &theLatlon, float &WS, float &WD)
{
    float u = ::InterpolateValueFromMatrix(itsWindUComponent, theLatlon, theInfo->Area());
    float v = ::InterpolateValueFromMatrix(itsWindVComponent, theLatlon, theInfo->Area());
    ::UVcomponents2SpeedDir(u, v, WS, WD);
}

static void UpdateWindValues(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiPoint &thetLatlonPoint, const NFmiMetTime &theTime, float &WS, float &WD)
{
    theInfo->Param(kFmiWindSpeedMS);
    WS = theInfo->InterpolatedValue(thetLatlonPoint, theTime);
    theInfo->Param(kFmiWindDirection);
    WD = theInfo->InterpolatedValue(thetLatlonPoint, theTime);
}

// Tuulen mukana kulkevan partikkelin uusi sijainti
static NFmiLocation CalcNewLocation(const NFmiLocation &theCurrentLocation, double WS, double WD, int theTimeStepInMinutes, bool isForwardDir, bool pacificView)
{
	double dist = WS * theTimeStepInMinutes * 60; // saadaan kuljettu matka metrein‰
	double dir = ::fmod(isForwardDir ? (WD + 180) : WD, 360); // jos backward trajectory pit‰‰ k‰‰nt‰‰ virtaus suunta 180 asteella
	return theCurrentLocation.GetLocation(dir, dist, pacificView);
}

// HHilaruudukossa suhteellisen matkan kulkeva partikkeli, kuljettava matka annetaan suoraan parametrina.
static NFmiLocation CalcNewLocation2(const NFmiLocation &theCurrentLocation, const NFmiPoint &distInMeters, double WD, bool isForwardDir, bool pacificView)
{
    double usedDistInMeters = (distInMeters.X() + distInMeters.Y()) / 2.; // t‰ss‰ pit‰isi laskea k‰ytetyn kulman mukaan pituudet x- ja y-suunnassa, nyt lasketaan vain keskiarvo
	double dir = ::fmod(isForwardDir ? (WD + 180) : WD, 360); // jos backward trajectory pit‰‰ k‰‰nt‰‰ virtaus suunta 180 asteella
	return theCurrentLocation.GetLocation(dir, usedDistInMeters, pacificView);
}

// path:iin ollaan lis‰‰m‰ss‰ uutta pistett‰ (newPoint), jos se ei leikkaa polun kanssa.
// Eli k‰yd‰‰n l‰pi path alusta melkein loppuun ja verrataan jokaista yksitt‰ist‰ viivaa
// path:in viimeisen pisteen ja newPoint:in muodostaman viivan kanssa.
static bool IsPathIntersectingLastLine(const std::vector<NFmiPoint> &path, const NFmiPoint &newPoint)
{
    if(path.size() > 2) // pit‰‰ olla v‰hint‰‰ 3 pistett‰
    {
        const NFmiPoint &lastPointInPath = path[path.size()-1];
        // Looppi k‰yd‰‰n 1. pisteest‰  3. viimeiseen asti. 3. viimeisest‰ alkava viiva sis‰lt‰‰ siis 2. viimeisen pisteen.
        // Mutta ei ole kannattavaa katsoa meneeko yhten‰isen polun kaksi viimeist‰ viivaa ristiin.
        for(size_t i = 0; i < path.size() - 3; i++)
        {
            if(::IsLinesIntersecting(path, i, lastPointInPath, newPoint))
                return true;
        }
    }
    return false;
}

// T‰ll‰ estet‰‰n matalapaine pyˆrteit‰ tai muuten tulemista itse‰‰n l‰helle.
// Tutkitaan path:in viimeisen pisteen ja newPoint:in v‰lisen viivan l‰heisyytt‰ aiempaan polkuun.
// Tutkiminen aloitetaan lopusta (path[size-1] -> path[size-2]) ja menn‰‰n alkua kohden.
// Aluksi kahden tutkittavan viivan l‰heisyys on selv‰‰, mutta kun kerran tutkittavat viivat ovat 
// eronneet tarpeeksi, laitetaan lippu p‰‰lle. T‰m‰n j‰lkeen jos jos p‰‰st‰‰n taas polun l‰helle,
// todetaan ett‰ viimeisin piste on tullut liian l‰helle polkua (uudestaan) ja polku voidaan lopettaa.
static bool IsPathClosingToItSelf(const std::vector<NFmiPoint> &path, const NFmiPoint &newPoint, double theProximityLimit)
{
    if(path.size() > 3) // pit‰‰ olla v‰hint‰‰ 4 pistett‰, ett‰ t‰ss‰ on j‰rke‰
    {
        const NFmiPoint &lastPointInPath = path[path.size()-1];
        bool lineSeparation = false; // viivat eiv‰t ole viel‰ eronneet toisistaan
        // Looppi k‰yd‰‰n 2. viimeisest‰ pisteest‰ alkua kohden. Siten ett‰ i menee 3. viimeisest‰ alkuun.
        for(int i = static_cast<int>(path.size() - 2); i >= 0; i--)
        {
            double distance = ::GetShortestDistance(newPoint, lastPointInPath, path[i], path[i+1]);
            if(lineSeparation)
            {
                if(distance < theProximityLimit)
                    return true;
            }
            else
            {
                if(distance > theProximityLimit)
                    lineSeparation = true;
            }
        }
    }
    return false;
}

// path:iin ollaan lis‰‰m‰ss‰ uutta pistett‰ (newPoint), jos se ei leikkaa polun kanssa.
// Eli k‰yd‰‰n l‰pi path lopusta melkein alkuun ja verrataan jokaista yksitt‰ist‰ viivaa
// path:in ensimm‰isen pisteen ja newPoint:in muodostaman viivan kanssa.
static bool IsPathIntersectingFirstLine(const std::vector<NFmiPoint> &path, const NFmiPoint &newPoint)
{
    if(path.size() > 2) // pit‰‰ olla v‰hint‰‰ 3 pistett‰
    {
        const NFmiPoint &firstPointInPath = path[0];
        // Looppi k‰yd‰‰n 2. viimeisest‰ pisteest‰ taaksep‰in 2. pisteeseen asti. Muista ett‰ IsLinesIntersecting-funktio
        // tutkii annetun indeksi ja seuraavan pisteen v‰list‰ viivaa. Eli ensin tarkastetaan viiva v‰lill‰ path[size - 2] -> path[size - 1]
        // Viimeisen‰ tutkitaan path[1] -> path[2]
        // Mutta ei ole kannattavaa katsoa meneeko path[0] -> path[1] ja path[0] -> newPoint viivat ristiin.
        for(size_t i = path.size() - 2; i > 0; i--)
        {
            if(::IsLinesIntersecting(path, i, firstPointInPath, newPoint))
                return true;
        }
    }
    return false;
}

struct StartingPointMatrixHelper
{
    StartingPointMatrixHelper(void)
    :itsFoundGoodSize(-1)
    ,itsFoundGoodStep(-1)
    ,itsFoundGoodStartIndex(-1)
    {
    }

    bool CalcSizesImpl(int theOrigGridSize, int theCountInData, int theSizeOffset, int thePlaceOffset1, int thePlaceOffset2)
    {
        int testedGridSize = theOrigGridSize - thePlaceOffset1 - thePlaceOffset2 - 1;
        int testedWantedSize = theCountInData + theSizeOffset - 1;
        int stepSize = testedGridSize / testedWantedSize;
        if(stepSize * testedWantedSize == testedGridSize)
        {
            itsFoundGoodSize = testedWantedSize + 1;
            itsFoundGoodStep = stepSize;
            itsFoundGoodStartIndex = thePlaceOffset1;
            return true;
        }
        else
            return false;
    }

    bool CalcSizes(int theOrigGridSize, int theCountInData, int theSizeOffset, int thePlaceOffset1, int thePlaceOffset2)
    {
        if(!CalcSizesImpl(theOrigGridSize, theCountInData, theSizeOffset, thePlaceOffset1, thePlaceOffset2))
            CalcSizesImpl(theOrigGridSize, theCountInData, -theSizeOffset, thePlaceOffset1, thePlaceOffset2); // jos ei osunut, lasketaan koko offset viel‰ negatiivisena
        return SizeFound();
    }

    void Reset(void)
    {
         *this = StartingPointMatrixHelper();
    }

    bool SizeFound(void)
    {
        return (itsFoundGoodSize > 0) && (itsFoundGoodStep > 0) && (itsFoundGoodStartIndex >= 0);
    }

    // Kun etsit‰‰n optimaalia jakosuhteita, pit‰‰ pysty‰ rankkaamaan suuruusj‰rjestykseen n‰it‰ lukuja.
    // Mit‰ pienempi, sen parempi.
    bool operator<(const StartingPointMatrixHelper &other) const
    {
        if(itsFoundGoodStartIndex != other.itsFoundGoodStartIndex)
            return itsFoundGoodStartIndex < other.itsFoundGoodStartIndex; // aloitus indeksin pienuus on t‰rkein tekij‰, ett‰ aloituspisteit‰ saadaan datan reunoille asti
        if(itsFoundGoodSize != other.itsFoundGoodSize)
            return itsFoundGoodSize > other.itsFoundGoodSize; // jos koko oli suurempi, se oli parempi
        if(itsFoundGoodStep < other.itsFoundGoodStep)
            return itsFoundGoodStep < other.itsFoundGoodStep;
        return false;
    }

    int itsFoundGoodSize; // lasketun sopivan aloitus hilan koko X/Y-suunnassa
    int itsFoundGoodStep; // lasketun sopivan aloitus hilan steppiv‰li originaali hilaan n‰hden X/Y-suunnassa
    int itsFoundGoodStartIndex; // lasketun sopivan aloitus hilan aloitus offset X/Y-suunnassa
};

// Jos helper todetaan kandidaatiksi (=SizeFound), lis‰t‰‰n t‰m‰ helperlistaan.
// Palauttaa true, jos ehdokas oli 't‰ysosuma', eli startIndex oli 0 tai 1. T‰llˆin myˆsk‰‰n ei saa resetoida, 
// koska kandidaatti palautetaan suoraan k‰yttˆˆn.
static bool CheckStartMatrixSizes(StartingPointMatrixHelper &theStartMatrixHelper, std::set<StartingPointMatrixHelper> &theHelperList)
{
    if(theStartMatrixHelper.SizeFound())
    {
        theHelperList.insert(theStartMatrixHelper);
        if((theStartMatrixHelper.itsFoundGoodStartIndex == 0) || (theStartMatrixHelper.itsFoundGoodStartIndex == 1))
            return true;
        theStartMatrixHelper.Reset();
    }
    return false;
}

static StartingPointMatrixHelper CalcSuitableStartMatrixSize(int theOrigSize, int theWantedSize)
{
    if(theOrigSize < theWantedSize)
    { // jos on zoomattu niin syv‰lle dataan, ett‰ haluttu koko ylitt‰‰ originaali koon, ei kannata jatkaa koon kasvattamista, vaan tyydyt‰‰n originaali hilaan
        StartingPointMatrixHelper startMatrixHelper2;
        if(startMatrixHelper2.CalcSizes(theOrigSize, theOrigSize, 0, 0, 0))
            return startMatrixHelper2;
    }

    std::set<StartingPointMatrixHelper> helperList;
    StartingPointMatrixHelper startMatrixHelper;
    int maxOffSetSize = FmiRound(theWantedSize/2.);
    for(int sizeOffset = 0; sizeOffset < maxOffSetSize; sizeOffset++)
    {
        startMatrixHelper.CalcSizes(theOrigSize, theWantedSize, sizeOffset, 0, 0); // 1. kokeillaan t‰t‰ jakoa alkaen vasemmasta reunasta -> loppuun
        if(::CheckStartMatrixSizes(startMatrixHelper, helperList))
            return startMatrixHelper;
        int maxOffSetPlace = FmiRound(theWantedSize/3.);
        for(int placeOffset = 1; placeOffset < maxOffSetPlace; placeOffset++)
        {
            startMatrixHelper.CalcSizes(theOrigSize, theWantedSize, sizeOffset, placeOffset, placeOffset-1); // 2. kokeillaan t‰t‰ jakoa alkaen vasemmasta reunasta +1 -> loppuun
            if(::CheckStartMatrixSizes(startMatrixHelper, helperList))
                return startMatrixHelper;
            startMatrixHelper.CalcSizes(theOrigSize, theWantedSize, sizeOffset, placeOffset, placeOffset); // 3. kokeillaan t‰t‰ jakoa alkaen vasemmasta reunasta +1 -> loppuun -1
            if(::CheckStartMatrixSizes(startMatrixHelper, helperList))
                return startMatrixHelper;
        }
    }
    if(helperList.size())
        return *helperList.begin();
    else
        return StartingPointMatrixHelper();
}

// Funktio saa originaali datan, ja sen hilaan lasketut ensi arvaukset aloitupiste matriisin kooksi.
// Jokainen aloitus piste pit‰‰ kuitenkin laskea sopimaan originaali hilaan, joten pit‰‰ tehd‰ viel‰ temppuja.
// Etsit‰‰n originaali hilasta sellainen alihila, johon annettu toivekoko saadaan mahtumaan tasa stepein.
// Etsit‰‰n sellainen iteroimalla. 
// Lopuksi t‰ytet‰‰n matriisi latlon-pisteill‰, jotka ovat niiss‰ originaalihilapisteiss‰, joihin saadaan laskuissa viittaukset.
static NFmiDataMatrix<NFmiPoint> MakeFinalStartingPointMatrix(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, double startPointCountInDataX, double startPointCountInDataY)
{
    NFmiDataMatrix<NFmiPoint> startingPoints;
    int countInDataX = FmiRound(startPointCountInDataX);
    int countInDataY = FmiRound(startPointCountInDataY);
    if(countInDataX >= 2 && countInDataY >= 2)
    { // pit‰‰ olla v‰hint‰in 3x3 hila, ennen kuin tehd‰‰n t‰t‰ iterointia
        StartingPointMatrixHelper startMatrixHelperX = ::CalcSuitableStartMatrixSize(theInfo->GridXNumber(), countInDataX);
        StartingPointMatrixHelper startMatrixHelperY = ::CalcSuitableStartMatrixSize(theInfo->GridYNumber(), countInDataY);

        if(startMatrixHelperX.SizeFound() && startMatrixHelperY.SizeFound())
        {
            auto origDataGridPoints = theInfo->RefQueryData()->LatLonCache();
            startingPoints.Resize(startMatrixHelperX.itsFoundGoodSize, startMatrixHelperY.itsFoundGoodSize, NFmiPoint::gMissingLatlon);
            for(int y = 0; y < startMatrixHelperY.itsFoundGoodSize; y++)
            {
                for(int x = 0; x < startMatrixHelperX.itsFoundGoodSize; x++)
                {
                    int cacheIndexX = startMatrixHelperX.itsFoundGoodStartIndex + x * startMatrixHelperX.itsFoundGoodStep;
                    int cacheIndexY = startMatrixHelperY.itsFoundGoodStartIndex + y * startMatrixHelperY.itsFoundGoodStep;
                    int cacheIndex = cacheIndexY * theInfo->GridXNumber() + cacheIndexX;
                    startingPoints[x][y] = origDataGridPoints->at(cacheIndex);
                }
            }
        }
    }
    else
    { // ei tehd‰ viel‰ alle originaali hilan menev‰‰ resoluutiota
    }
    return startingPoints;
}

// Tehd‰‰n matriisi, jossa on potentiaaliset streamline aloituspisteet. 
// Teen matriisin pohjautuen k‰ytetyn datan alueeseen ja hilaan, t‰ten tarkoitus olisi tehd‰
// aloituspiste matriisista mahdollisimman stabiili. Stabiili sen suhteen, jos aluetta zoomataan tai pannataan, 
// niin alopituspisteiden paikat eiv‰t hyppisi villisti.
// Pisteiden pit‰‰ olla sopivalla v‰ljyydell‰ toisistaan, ett‰ streamlineista tulee visuallisesti miellytt‰v‰n n‰kˆisi‰.
// Pisteiden v‰ljyys riippuu sek‰ datan hilan tarkkuudesta ja karttan‰ytˆn (mahdollisen ali ruudun) 
// fyysisest‰ koosta ja jostain v‰ljyys parametrista, joka tulee drawParamista.
// Aloituspisteet lasketaan aina vain datan alueelle, vaikka kartan zoomaustaso olisi mik‰.
NFmiDataMatrix<NFmiPoint> NFmiStreamLineView::CalcPotencialStartingPoints(const StreamlineCalculationParameters &theCalcParams)
{
	// yhden aliruudun koko pikseleiss‰ karttan‰ytˆll‰
    NFmiPoint viewSizeInPixels = itsCtrlViewDocumentInterface->ActualMapBitmapSizeInPixels(itsMapViewDescTopIndex);
    auto &graphicalInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
	float widthInMM = static_cast<float>(viewSizeInPixels.X() / graphicalInfo.itsPixelsPerMM_x);
	float heightInMM = static_cast<float>(viewSizeInPixels.Y() / graphicalInfo.itsPixelsPerMM_y);

    NFmiRect dataWorldRect = itsInfo->Area()->WorldRect();
    NFmiRect mapAreaWorldRect = itsArea->WorldRect();
    double map2DataAreaRatioX = mapAreaWorldRect.Width() / dataWorldRect.Width();
    double map2DataAreaRatioY = mapAreaWorldRect.Height() / dataWorldRect.Height();

    double startPointDistInMM = theCalcParams.itsStartPointDistInMM;
    double startPointCountInMapX = widthInMM / startPointDistInMM;
    double startPointCountInMapY = heightInMM / startPointDistInMM;
    double startPointCountInDataX = startPointCountInMapX / map2DataAreaRatioX;
    double startPointCountInDataY = startPointCountInMapY / map2DataAreaRatioY;

    // Nyt pit‰‰ yritt‰‰ sovittaa datan originaali hilaan saadut startPointCountX+Y luvut 
    // ja niihin liittyv‰t aloitus indeksit ja stepit
    return ::MakeFinalStartingPointMatrix(itsInfo, startPointCountInDataX, startPointCountInDataY);
}

// K‰ytet‰‰n ns. virallista aika-steppi‰, paitsi jos tuulen nopeus menee alle limitin.
// T‰llˆin kasvatetaan aika-steppi‰ sit‰ enemm‰n, mit‰ enemm‰n WS l‰hestyy 0:aa.
static int CalcUsedTimeStep(int theOfficialTimeStepInMinutes, float WS, float theBoostLimit)
{
    if(WS >= theBoostLimit)
        return theOfficialTimeStepInMinutes;
    else
    {
        float factor = theBoostLimit - WS;
        if(factor < 1)
            factor = 1;
        return FmiRound(theOfficialTimeStepInMinutes*factor);
    }
}

// laskee infosta sen hetkisen hilapisteen kohdasta factoriin suhteutetun hilakoon metreiss‰.
static NFmiPoint CalcWantedTravelDistanceInMeters(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, double gridSizeFactor)
{
    unsigned long originalLocationIndex = theInfo->LocationIndex();
    if(originalLocationIndex != gMissingIndex)
    {
        double xdist = kFloatMissing;
        double ydist = kFloatMissing;
        NFmiLocation loc1(theInfo->LatLon());
        if(theInfo->MoveLeft())
        {
            NFmiLocation loc2(theInfo->LatLon());
            xdist = loc2.Distance(loc1);
        }
        else if(theInfo->MoveRight())
        {
            NFmiLocation loc2(theInfo->LatLon());
            xdist = loc2.Distance(loc1);
        }

        if(theInfo->MoveUp())
        {
            NFmiLocation loc3(theInfo->LatLon());
            ydist = loc3.Distance(loc1);
        }
        else if(theInfo->MoveDown())
        {
            NFmiLocation loc3(theInfo->LatLon());
            ydist = loc3.Distance(loc1);
        }

        if(xdist != kFloatMissing && ydist != kFloatMissing)
        {
            xdist = std::fabs(xdist) * gridSizeFactor;
            ydist = std::fabs(ydist) * gridSizeFactor;
            return NFmiPoint(xdist, ydist);
        }
    }

    return NFmiPoint::gMissingLatlon;
}

static double PointToRectangleDistance(const NFmiRect &rect, const NFmiPoint &point)
{
#ifdef max
#undef max // VC++ on m‰‰ritellyt max/min makrot, jotka sotkevat C++ standardin funktiot
#endif

    NFmiPoint rectCenter = rect.Center();
    double dx = std::max(std::fabs(point.X() - rectCenter.X()) - rect.Width() / 2, 0.);
    double dy = std::max(std::fabs(point.Y() - rectCenter.Y()) - rect.Height() / 2, 0.);
    return ::sqrt(dx * dx + dy * dy);
}

static bool IsPointTooFarFromZoomedArea(boost::shared_ptr<NFmiArea> &theZoomedArea, const NFmiPoint &theLatlonPoint, double theMetricMarginFactor)
{
    if(theZoomedArea && theMetricMarginFactor)
    {
        if(!theZoomedArea->IsInside(theLatlonPoint))
        {
            NFmiRect metricRect = theZoomedArea->WorldRect();
            NFmiPoint xyLocation = theZoomedArea->LatLonToWorldXY(theLatlonPoint);
            double distanceLimit = ((metricRect.Width() + metricRect.Height()) / 2.) * theMetricMarginFactor;
            if(::PointToRectangleDistance(metricRect, xyLocation) > distanceLimit)
                return true;
        }
    }
    return false;
}

std::vector<NFmiPoint> NFmiStreamLineView::SearchPathOneDirection(const StreamlineCalculationParameters &theCalcParams, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiPoint &theStartLatlonPoint, int timeStepInMinutes, double theProximityLimit, bool goForwardDir, double theAreaLimit)
{
    const double usedAreaLimit = theAreaLimit / 2.; // t‰‰ll‰ k‰ytet‰‰n hienompaa areaa kuin ulkopuolella
    const int maxAllowedPathSize = 500;
    const int maxAllowedIndex = 5000;
    bool pacificView = itsArea->PacificView();
    std::vector<NFmiPoint> path;
    NFmiLocation currentLocation(theStartLatlonPoint);
    path.push_back(currentLocation.GetLocation());
    // joskus pit‰‰ k‰ytt‰‰ pidemp‰‰ aikasteppi‰, koska varsinkin maanpinnalla tuulien nopeudet voivat l‰hesty‰ 0:aa ja polun rakentaminen etenee etanan vauhtia
    int usedTimeStepInThisIteration = timeStepInMinutes; 
    int index = 0;
    float WS = kFloatMissing; 
    float WD = kFloatMissing;
    CalcWindValues(theInfo, currentLocation.GetLocation(), WS, WD);
    NFmiPoint relativeTravelDistanceInMeters = ::CalcWantedTravelDistanceInMeters(theInfo, theCalcParams.itsRelativeParticleJumpLength);
    size_t simplificationStartingIndex = 0;

    for( ; (WS != kFloatMissing && WD != kFloatMissing) ; index++)
    {
        if(theCalcParams.fUseRelativeParticleJumps)
        {
            itsDebugTotalRelativeJumpPointsCalculated++;
            currentLocation = ::CalcNewLocation2(currentLocation, relativeTravelDistanceInMeters, WD, goForwardDir, pacificView);
        }
        else
            currentLocation = ::CalcNewLocation(currentLocation, WS, WD, usedTimeStepInThisIteration, goForwardDir, pacificView);
        if(::IsPathIntersectingLastLine(path, currentLocation.GetLocation()))
            break;
        // tutkitaan vain joka 3:lla kerralla meneeko viiva liian l‰helle itse‰‰n
        if(index && index % theCalcParams.itsClosingToItselfCheck == 0 && ::IsPathClosingToItSelf(path, currentLocation.GetLocation(), theProximityLimit))
            break;
        if(theCalcParams.itsTooFarFromZoomedAreaCheck && index && index % theCalcParams.itsTooFarFromZoomedAreaCheck == 0 && ::IsPointTooFarFromZoomedArea(itsArea, currentLocation.GetLocation(), theCalcParams.itsTooFarFromZoomedAreaCheckFactor))
            break;
        path.push_back(currentLocation.GetLocation()); // eteenp‰in ment‰esss‰ laitetaan loppuun
        if(path.size() > maxAllowedPathSize)
            break; // joskus polku j‰‰ junnaamaan paikalleen ilman ett‰ viivat risti‰v‰t, laitetaan josssain vaiheessa poikki, koska tarkasteulut kasvavat kaiken aikaa
        CalcWindValues(theInfo, currentLocation.GetLocation(), WS, WD);
        usedTimeStepInThisIteration = ::CalcUsedTimeStep(timeStepInMinutes, WS, theCalcParams.itsRelativeCalcWindSpeedLimit);
/////        closingToItselfCheck = FmiRound(FmiMin(3, ::pow(static_cast<double>(path.size()), 0.33)));
        if(index && index % theCalcParams.itsDoSimplificationLimit == 0)
        {
            path = NFmiStreamlineData::SimplifyPath(path, usedAreaLimit, simplificationStartingIndex);
            simplificationStartingIndex = path.size();
        }
//        if((!theCalcParams.fUseRelativeParticleJumps && WS <= 1) || index > maxAllowedIndex)
        if(WS <= 0.1f || index > maxAllowedIndex)
            break; // alle 0.1 m/s tuulella on turha jatkaa tai jos ollaan jo junnattu polun kanssa ‰lyttˆm‰ll‰ m‰‰r‰ll‰ pisteit‰
    }

    itsDebugTotalPointsCalculated += index;
    return path;
}

NFmiStreamlineData NFmiStreamLineView::CalcSingleStreamLinePath(const StreamlineCalculationParameters &theCalcParams, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiPoint &theStartLatlonPoint, const NFmiPoint &theStartPointIndex, int timeStepInMinutes, double theProximityLimit, bool edgeOfDataPoint, double theAreaLimit)
{
    itsDebugCalculatedStartingPoints++;
    // 1. Etsit‰‰n polkua ensin eteenp‰in...
    std::vector<NFmiPoint> forwardPath = SearchPathOneDirection(theCalcParams, theInfo, theStartLatlonPoint, timeStepInMinutes, theProximityLimit, true, theAreaLimit);
    // 2. Etsit‰‰n polkua sitten taaksep‰in...
    std::vector<NFmiPoint> backwardPath = SearchPathOneDirection(theCalcParams, theInfo, theStartLatlonPoint, timeStepInMinutes, theProximityLimit, false, theAreaLimit);
    return NFmiStreamlineData(forwardPath, backwardPath, this, theStartLatlonPoint, theStartPointIndex, edgeOfDataPoint);
}

// Katsotaan ett‰ theCurrentPath ei mene ristiin olemassa olevien polkujen kanssa.
// lis‰ksi katsotaan ett‰ se ei mene liian l‰helle olemassa olevien polkujen kanssa.
void NFmiStreamLineView::CropOvelappings(NFmiStreamlineData &theCurrentPath, std::vector<NFmiStreamlineData> &theLatlonPaths, double theProximityLimit)
{
    for(size_t i = 0; i < theLatlonPaths.size(); ++i)
    {
        NFmiStreamlineData &streamlineData = theLatlonPaths[i];
        theCurrentPath.CropCrossingPaths(streamlineData, this);
        theCurrentPath.CropClosingPaths(streamlineData, theProximityLimit, this);
    }
}

// Lasketaan kolmen matriisin keskell‰ olevan pisteen avulla viivojen et‰isyyksien l‰heisyytt‰
// kuvaava kerroin, jota k‰ytet‰‰n l‰hekk‰isten viivojen poistossa rajana.
static double CalcProximityLimit(const NFmiDataMatrix<NFmiPoint> &theStartingPoints)
{
    if(theStartingPoints.NX() > 2 && theStartingPoints.NY() > 2)
    {
        int xInd = static_cast<int>(theStartingPoints.NX() / 2);
        int yInd = static_cast<int>(theStartingPoints.NY() / 2);
        double diffX = theStartingPoints[xInd+1][yInd].X() - theStartingPoints[xInd][yInd].X();
        double diffY = theStartingPoints[xInd][yInd+1].Y() - theStartingPoints[xInd][yInd].Y();
        return ::sqrt((diffX*diffX) + (diffY*diffY)) / 8.;
    }
    else
        return 1; // en jaksa tehd‰ pienille matriiseille erikoistapausta, menkˆˆt pieleen
}

void NFmiStreamLineView::DrawDebugRect(int sizeInPixels, const NFmiPoint &theLatlon, const NFmiColor &theColor)
{
    // debuggaus koodia
    NFmiRect startPointRect;
    double width = itsToolBox->SX(sizeInPixels);
    startPointRect.Size(NFmiPoint(width, width));
    startPointRect.Center(LatLonToViewPoint(theLatlon));
    Gdiplus::Rect rectInPixels = CtrlView::Relative2GdiplusRect(itsToolBox, startPointRect);    
    CtrlView::DrawRect(*itsGdiPlusGraphics, rectInPixels, theColor, theColor, false, true, 1, Gdiplus::DashStyleSolid);
}

void NFmiStreamLineView::DrawDebugString(double fontSizeInMM, const NFmiPoint &thePoint, const NFmiColor &theColor, const std::string &theText, bool pointInLatlon)
{
    // debuggaus koodia
    std::wstring fontNameStr(L"Arial");
    NFmiPoint relativePoint = pointInLatlon ? (LatLonToViewPoint(thePoint)) : thePoint;
    auto &graphInfo = itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex);
    CtrlView::DrawTextToRelativeLocation(*itsGdiPlusGraphics, theColor, fontSizeInMM, theText, relativePoint, graphInfo.itsPixelsPerMM_x, itsToolBox, fontNameStr, kLeft);
}

static void EliminateOutOfZoomedAreaStartingPoints(NFmiDataMatrix<NFmiStartPointEliminationData> &theStartingPointEliminatioMatrix, boost::shared_ptr<NFmiArea> &theZoomedArea, size_t &theDebugOutOfAreaStartingPoints)
{
    for(size_t j=0; j < theStartingPointEliminatioMatrix.NY(); j++)
    {
        for(size_t i=0; i < theStartingPointEliminatioMatrix.NX(); i++)
        {
            if(!theZoomedArea->IsInside(theStartingPointEliminatioMatrix[i][j].itsStartingPointLatlon))
            {
                theStartingPointEliminatioMatrix[i][j].fWasOutOfZoomedAreaStartingPoint = true;
                theDebugOutOfAreaStartingPoints++;
            }
        }
    }
}

// Hakee sen laatikon rajat (indeksit), mink‰ sis‰‰n zoomatun alueen pisteet mahtuvat.
// Jos zoomattu laatikko on tarpeeksi paljon pienempi kuin koko matriisi, palauta true (t‰llˆin kutsuja laskee uuden pienemm‰n matriisin), muuten false.
static bool GetZoomedAreaIndexies(const NFmiDataMatrix<NFmiStartPointEliminationData> &theEliminationMatrix, size_t &zoomAreaLeftIndexOut, size_t &zoomAreaTopIndexOut, size_t &zoomAreaRightIndexOut, size_t &zoomAreaBottomIndexOut)
{
    const size_t maxIndexLimit = 99999999;
    const size_t minIndexLimit = 0;
    size_t leftIndex = maxIndexLimit;
    size_t rightIndex = minIndexLimit;
    size_t bottomIndex = maxIndexLimit;
    size_t topIndex = minIndexLimit;
    for(size_t j=0; j < theEliminationMatrix.NY(); j++)
    {
        for(size_t i=0; i < theEliminationMatrix.NX(); i++)
        {
            if(!theEliminationMatrix[i][j].fWasOutOfZoomedAreaStartingPoint)
            {
                if(i < leftIndex)
                    leftIndex = i;
                if(i > rightIndex)
                    rightIndex = i;
                if(j < bottomIndex)
                    bottomIndex = j;
                if(j > topIndex)
                    topIndex = j;
            }
        }
    }

    zoomAreaLeftIndexOut = leftIndex;
    zoomAreaTopIndexOut = topIndex;
    zoomAreaRightIndexOut = rightIndex;
    zoomAreaBottomIndexOut = bottomIndex;

    if(leftIndex != maxIndexLimit && rightIndex != minIndexLimit && bottomIndex != maxIndexLimit && topIndex != minIndexLimit)
    {
        double zoomedWidth = static_cast<double>(rightIndex - leftIndex + 1);
        double zoomedHeight = static_cast<double>(topIndex - bottomIndex + 1);
        double zoomedArea = zoomedHeight * zoomedWidth;
        double matrixArea = static_cast<double>(theEliminationMatrix.NX() * theEliminationMatrix.NY());
        double ratio = zoomedArea / matrixArea;
        if(ratio < 0.7)
            return true; // jos zoomatun matriisin ja originaali matriisien koossa on tarpeeksi iso ero, voidaan tehd‰ zoomattu matriisi
    }
    return false;
}

template<typename Matrix>
static size_t GetMatrixMidIndex(const Matrix& matrix, bool xDimension)
{
    size_t actualSize = xDimension ? matrix.NX() : matrix.NY();
    
    if(actualSize <= 1)
        throw std::runtime_error("Streamline calculations had starting point matrix size smaller than 2 which would crash application if we continue calculations here");
    else if(actualSize == 2)
    {
        // Jos matriisin koko on 2, pit‰‰ palauttaa 0:aa,muuten jatko laskut kaatavat ohjelman
        return 0;
    }
    else
        return actualSize / 2;
}

NFmiDataMatrix<NFmiStartPointEliminationData> NFmiStreamLineView::CalcStartingPointEliminationMatrix(const NFmiDataMatrix<NFmiPoint> &theStartingPointMatrix, boost::shared_ptr<NFmiArea> &theZoomedArea)
{
    NFmiDataMatrix<NFmiPoint> relativeStartingPointMatrix = theStartingPointMatrix;
    for(size_t j=0; j <relativeStartingPointMatrix.NY(); j++)
    {
        for(size_t i=0; i <relativeStartingPointMatrix.NX(); i++)
        {
            relativeStartingPointMatrix[i][j] = LatLonToViewPoint(theStartingPointMatrix[i][j]);
        }
    }

    size_t xInd = ::GetMatrixMidIndex(relativeStartingPointMatrix, true);
    size_t yInd = ::GetMatrixMidIndex(relativeStartingPointMatrix, false);
    // eliminointi laatikon leveys on n. keskell‰ olevien aloituspisteiden et‰isyys kerrottuna jolloin keroimella joka on n. v‰lill‰ 0.3 - 1
    double width = (relativeStartingPointMatrix[xInd+1][yInd].X() - relativeStartingPointMatrix[xInd][yInd].X()) * 0.5;
    double height = (relativeStartingPointMatrix[xInd][yInd].Y() - relativeStartingPointMatrix[xInd][yInd+1].Y()) * 0.5;

    NFmiRect baseRect(0, 0, width, height);
    NFmiDataMatrix<NFmiStartPointEliminationData> eliminationMatrix(relativeStartingPointMatrix.NX(), relativeStartingPointMatrix.NY());
    for(size_t j=0; j < eliminationMatrix.NY(); j++)
    {
        for(size_t i=0; i < eliminationMatrix.NX(); i++)
        {
            baseRect.Center(relativeStartingPointMatrix[i][j]);
            eliminationMatrix[i][j] = NFmiStartPointEliminationData(baseRect, theStartingPointMatrix[i][j], NFmiPoint(static_cast<double>(i), static_cast<double>(j)));
        }
    }

    // Katsotaan mitk‰ aloituspisteet osuvat zoomatun alueen sis‰lle
    ::EliminateOutOfZoomedAreaStartingPoints(eliminationMatrix, theZoomedArea, itsDebugOutOfAreaStartingPoints);
    size_t zoomAreaLeftIndex = 0;
    size_t zoomAreaTopIndex = 0;
    size_t zoomAreaRightIndex = 0;
    size_t zoomAreaBottomIndex = 0;
    if(::GetZoomedAreaIndexies(eliminationMatrix, zoomAreaLeftIndex, zoomAreaTopIndex, zoomAreaRightIndex, zoomAreaBottomIndex))
    { // Laske uusi pienempi matriisi, jossa mukana vain zoomatun alueen sis‰osio
        size_t zoomedSizeX = zoomAreaRightIndex - zoomAreaLeftIndex + 1;
        size_t zoomedSizeY = zoomAreaTopIndex - zoomAreaBottomIndex + 1;
        NFmiDataMatrix<NFmiStartPointEliminationData> zoomedEliminationMatrix(zoomedSizeX, zoomedSizeY);
        for(size_t j=0; j < zoomedEliminationMatrix.NY(); j++)
        {
            for(size_t i=0; i < zoomedEliminationMatrix.NX(); i++)
            {
                zoomedEliminationMatrix[i][j] = eliminationMatrix[i + zoomAreaLeftIndex][j + zoomAreaBottomIndex];
                zoomedEliminationMatrix[i][j].itsStartingPointIndex = NFmiPoint(static_cast<double>(i), static_cast<double>(j)); // indeksit pit‰‰ myˆs p‰ivitt‰‰ kohdalleen
            }
        }
        return zoomedEliminationMatrix;
    }

    return eliminationMatrix;
}

static bool StartingPointEliminated(const NFmiDataMatrix<NFmiStartPointEliminationData> &theStartingPointEliminatioMatrix, size_t xIndex, size_t yIndex)
{
    return theStartingPointEliminatioMatrix[xIndex][yIndex].fEliminated;
}

static bool CrossesOverWorldsEdge(double lon1, double lon2)
{
    double longitudeDiff = lon2 - lon1;
    if(::fabs(longitudeDiff) > 300)
        return true;
    else
        return false;
}

static bool CrossesOverRelativeWorldsEdge(double x1, double x2, double relativeLengthLimit)
{
    double longitudeDiff = x2 - x1;
    if(::fabs(longitudeDiff) > relativeLengthLimit)
        return true;
    else
        return false;
}

static void EliminatePointFromMatrix(const NFmiPoint &p1, const NFmiPoint &p2, NFmiDataMatrix<NFmiStartPointEliminationData> &theStartingPointEliminatioMatrix, const NFmiStreamlineData &theCurrentPath, bool forwardPath, size_t pointIndex, double relativeWorldWidth)
{
    double relativeLengthLimit = relativeWorldWidth * 0.8;

//    bool pointFoundInMatrix = false;
    for(size_t j=0; j < theStartingPointEliminatioMatrix.NY(); j++)
    {
        for(size_t i=0; i < theStartingPointEliminatioMatrix.NX(); i++)
        {
            NFmiStartPointEliminationData &spElimData = theStartingPointEliminatioMatrix[i][j];
            if(!spElimData.fEliminated && !spElimData.fWasUsedAsStartingPoint && !spElimData.fWasOutOfZoomedAreaStartingPoint)
            {
                if(::CrossesOverRelativeWorldsEdge(p1.X(), p2.X(), relativeLengthLimit) == false && ::LineIntersectsRect(p1, p2, spElimData.itsStartPointArea))
                {
                    spElimData.fEliminated = true;
                    spElimData.itsStartingPointIndexEliminated = theCurrentPath.StartingPointIndex();
                    spElimData.fForwardPathEliminate = forwardPath;
                    spElimData.itsEliminationPointIndex = pointIndex;
//                    pointFoundInMatrix = true;
//                    break;
                }
            }
        }
//        if(pointFoundInMatrix)
//            break;
    }
}

static void EliminateSinglePathFromStartingPoints(const std::vector<NFmiPoint> &relativePath, const NFmiStreamlineData &theCurrentPath, NFmiDataMatrix<NFmiStartPointEliminationData> &theStartingPointEliminatioMatrix, bool forwardPath, double relativeWorldWidth)
{
    if(relativePath.size() > 1)
    {
        for(size_t i=0; i < relativePath.size()-1; i++)
            ::EliminatePointFromMatrix(relativePath[i], relativePath[i+1], theStartingPointEliminatioMatrix, theCurrentPath, forwardPath, i, relativeWorldWidth);
    }
}

static void EliminateStartingPoints(const NFmiStreamlineData &theCurrentPath, NFmiDataMatrix<NFmiStartPointEliminationData> &theStartingPointEliminatioMatrix, double relativeWorldWidth)
{
    // merkit‰‰n ensin ett‰ t‰m‰n polun aloituspiste aloittajaksi
    const NFmiPoint &startIndex = theCurrentPath.StartingPointIndex();
    theStartingPointEliminatioMatrix[static_cast<size_t>(startIndex.X())][static_cast<size_t>(startIndex.Y())].fWasUsedAsStartingPoint = true;

    ::EliminateSinglePathFromStartingPoints(theCurrentPath.ForwardPathRelative(), theCurrentPath, theStartingPointEliminatioMatrix, true, relativeWorldWidth);
    ::EliminateSinglePathFromStartingPoints(theCurrentPath.BackwardPathRelative(), theCurrentPath, theStartingPointEliminatioMatrix, false, relativeWorldWidth);
}

// Halutaan laskea polun yksinkertaistuksessa k‰ytetty kolmion alueen koko zoomatulle kartta-alueelle.
// theMaxAreaLimit on maksimi mit‰ k‰ytet‰‰n joka tapauksessa, jos ollaan vaikka maailman kartalla
// theLenghtLimitInKM t‰m‰ on sellaisen alueen leveys/korkeus, mit‰ pienemm‰lle alueelle lasketaan jo 
// tarkempaa eli pienemp‰‰ alueen koko rajoitusta (= lasketaan tarkempaa polkua yksinkertaistuksessa).
static double CalcSimplificationAreaLimit(boost::shared_ptr<NFmiArea> &theArea, double theMaxAreaLimit, double theLenghtLimitInKM)
{
    double widthInKM = theArea->WorldXYWidth() / 1000.;
    double heightInKM = theArea->WorldXYHeight() / 1000.;
    double zoomedAreaLenghtInKM = ::sqrt(widthInKM*widthInKM + heightInKM*heightInKM);
    if(zoomedAreaLenghtInKM > theLenghtLimitInKM)
        return theMaxAreaLimit;
    else
    {
        return theMaxAreaLimit * (zoomedAreaLenghtInKM/theLenghtLimitInKM);
    }
}

void NFmiStreamLineView::DoStartingPointCalcualtions(const StreamlineCalculationParameters &theCalcParams, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, std::vector<NFmiStreamlineData> &theLatlonPaths, int timeStepInMinutes, double theProximityLimit, NFmiDataMatrix<NFmiStartPointEliminationData> &theStartingPointEliminatioMatrix, size_t xIndex, size_t yIndex, double theLengthLimitInKM)
{
    // debuggaus koodia: est‰‰ laskemasta muut kuin halutut ongelma pisteet
//    if(!((xIndex == 4 && yIndex == 2) || (xIndex == 5 && yIndex == 5)))
//        return ;

    if(xIndex >= theStartingPointEliminatioMatrix.NX() || yIndex >= theStartingPointEliminatioMatrix.NY())
        return ;
    if(theStartingPointEliminatioMatrix[xIndex][yIndex].fWasOutOfZoomedAreaStartingPoint)
        return ;

    // debuggaus koodia, piirret‰‰n aloituspisteisiin merkit
    if(gDrawDebugInfo)
        DrawDebugRect(5, theStartingPointEliminatioMatrix[xIndex][yIndex].itsStartingPointLatlon, NFmiColor(1,0,0));

    double maxAreaLimit = theCalcParams.itsMaxAreaLimit;
    double maxLengthInKMLimit = theCalcParams.itsMaxLengthInKMLimit;
    double simplificationAreaLimit = ::CalcSimplificationAreaLimit(itsArea, maxAreaLimit, maxLengthInKMLimit);

    if(!theStartingPointEliminatioMatrix[xIndex][yIndex].fWasUsedAsStartingPoint && !::StartingPointEliminated(theStartingPointEliminatioMatrix, xIndex, yIndex))
    {
        bool edgeOfDataPoint = (xIndex == 0) || (yIndex == 0) || (xIndex == theStartingPointEliminatioMatrix.NX()-1) || (yIndex == theStartingPointEliminatioMatrix.NY()-1);
        NFmiStreamlineData currentPath = CalcSingleStreamLinePath(theCalcParams, theInfo, theStartingPointEliminatioMatrix[xIndex][yIndex].itsStartingPointLatlon, NFmiPoint(static_cast<double>(xIndex), static_cast<double>(yIndex)), timeStepInMinutes, theProximityLimit, edgeOfDataPoint, simplificationAreaLimit);
        if(currentPath.HasRealPath())
        {
            currentPath.SimplifyData(this, simplificationAreaLimit);
////            CropOvelappings(currentPath, theLatlonPaths, theProximityLimit);
////            if(!currentPath.IsShorty(theLengthLimitInKM)) // Jos polut ovat kutistuneet tosi lyhyeksi (n. yhteen viivaan), j‰tet‰‰n turhana ja h‰iritsev‰n‰ pois
            {
                ::EliminateStartingPoints(currentPath, theStartingPointEliminatioMatrix, itsArea->XYArea().Width());
                itsDebugTotalStreamLineCount++;
                itsDebugTotalSimplifiedPoints += currentPath.BackwardPathLatlon().size();
                itsDebugTotalSimplifiedPoints += currentPath.ForwardPathLatlon().size();
                theLatlonPaths.push_back(currentPath);
            }
//            else
//                theStartingPointEliminatioMatrix[xIndex][yIndex].fMarkedShortly = true;
        }
    }
}

static double CalcLengthLimitInKM(boost::shared_ptr<NFmiArea> &theArea)
{
    double widthInKM = theArea->WorldXYWidth() / 1000.;
    double heightInKM = theArea->WorldXYHeight() / 1000.;
    const double dividor = 200; // kuinka mones osa kartan koosta luokitellaan ns. lyhyeksi poluksi
    double lengthLimitInKM = ::sqrt(widthInKM*widthInKM + heightInKM*heightInKM) / dividor;
    return lengthLimitInKM;
}

static size_t CountEliminatedPoints(NFmiDataMatrix<NFmiStartPointEliminationData> &theStartingPointEliminatioMatrix)
{
    size_t eliminatedCount = 0;
    for(size_t j=0; j < theStartingPointEliminatioMatrix.NY(); j++)
    {
        for(size_t i=0; i < theStartingPointEliminatioMatrix.NX(); i++)
        {
            if(theStartingPointEliminatioMatrix[i][j].fEliminated)
                eliminatedCount++;
        }
    }
    return eliminatedCount;
}

static NFmiPoint GetCenterStartingPointFromZoomedArea(NFmiDataMatrix<NFmiStartPointEliminationData> &theStartingPointEliminatioMatrix, boost::shared_ptr<NFmiArea> &theZoomedArea)
{
    NFmiPoint zoomedAreaCenterLatlon = theZoomedArea->ToLatLon(theZoomedArea->XYArea().Center());
    NFmiLocation zoomedAreaCenterLocation(zoomedAreaCenterLatlon);
    double minDistanceInMetres = 999999999;
    NFmiPoint centerStartingPointIndexies = NFmiPoint::gMissingLatlon;
    for(size_t j=0; j < theStartingPointEliminatioMatrix.NY(); j++)
    {
        for(size_t i=0; i < theStartingPointEliminatioMatrix.NX(); i++)
        {
            if(!theStartingPointEliminatioMatrix[i][j].fWasOutOfZoomedAreaStartingPoint)
            {
                double currentDistanceInMeters = zoomedAreaCenterLocation.Distance(theStartingPointEliminatioMatrix[i][j].itsStartingPointLatlon);
                if(currentDistanceInMeters < minDistanceInMetres)
                {
                    minDistanceInMetres = currentDistanceInMeters;
                    centerStartingPointIndexies = NFmiPoint(static_cast<double>(i), static_cast<double>(j));
                }
            }
        }
    }
    return centerStartingPointIndexies;
}

void NFmiStreamLineView::DoSingleThreadCalculations(const StreamlineCalculationParameters &theCalcParams, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiDataMatrix<NFmiStartPointEliminationData> &theStartingPointEliminatioMatrix, int timeStepInMinutes, std::vector<NFmiStreamlineData> &theLatlonPathsOut, double theProximityLimit, double theLengthLimitInKM)
{
    // K‰ytet‰‰n aluksi aloitus pisteet aloitusPiste matriisin keski pysty ja vaaka janoilta, siten ett‰ 
    // aloitetaan kaskelta ja edet‰‰n reinoja kohden, n‰in toivottavasti saadaan aluksi edustavimmat polut, 
    // jotka sitten alkavat blokkaamaan muita aloitus pisteit‰.
    size_t xMidInd = ::GetMatrixMidIndex(theStartingPointEliminatioMatrix, true);
    size_t yMidInd = ::GetMatrixMidIndex(theStartingPointEliminatioMatrix, false);

    // Ensin keskipiste
    DoStartingPointCalcualtions(theCalcParams, theInfo, theLatlonPathsOut, timeStepInMinutes, theProximityLimit, theStartingPointEliminatioMatrix, xMidInd, yMidInd, theLengthLimitInKM);
    // Sitten loput keski pysty rivist‰ (alas ja ylˆs p‰in erikseen)
    for(size_t k = 1; k < yMidInd; k++)
    {
        size_t usedYIndex = yMidInd - k;
        DoStartingPointCalcualtions(theCalcParams, theInfo, theLatlonPathsOut, timeStepInMinutes, theProximityLimit, theStartingPointEliminatioMatrix, xMidInd, usedYIndex, theLengthLimitInKM);
        usedYIndex = yMidInd + k;
        DoStartingPointCalcualtions(theCalcParams, theInfo, theLatlonPathsOut, timeStepInMinutes, theProximityLimit, theStartingPointEliminatioMatrix, xMidInd, usedYIndex, theLengthLimitInKM);
    }
    // Sitten loput keski vaaka rivist‰ (vasemmalle ja oikealle erikseen)
    for(size_t k = 1; k < xMidInd; k++)
    {
        size_t usedXIndex = xMidInd - k;
        DoStartingPointCalcualtions(theCalcParams, theInfo, theLatlonPathsOut, timeStepInMinutes, theProximityLimit, theStartingPointEliminatioMatrix, usedXIndex, yMidInd, theLengthLimitInKM);
        usedXIndex = xMidInd + k;
        DoStartingPointCalcualtions(theCalcParams, theInfo, theLatlonPathsOut, timeStepInMinutes, theProximityLimit, theStartingPointEliminatioMatrix, usedXIndex, yMidInd, theLengthLimitInKM);
    }

    for(size_t j = 0; j < theStartingPointEliminatioMatrix.NY(); j++)
    {
        for(size_t i = 0; i < theStartingPointEliminatioMatrix.NX(); i++)
        {
            DoStartingPointCalcualtions(theCalcParams, theInfo, theLatlonPathsOut, timeStepInMinutes, theProximityLimit, theStartingPointEliminatioMatrix, i, j, theLengthLimitInKM);
        }
    }
}

struct StreamlineMultiThreadCalculationData
{
    StreamlineMultiThreadCalculationData(NFmiDataMatrix<NFmiPoint> &thePotencialStartingPoints, NFmiDataMatrix<NFmiStartPointEliminationData> &theStartingPointEliminatioMatrix)
    :itsPotencialStartingPoints(thePotencialStartingPoints)
    ,itsStartingPointEliminatioMatrix(theStartingPointEliminatioMatrix)
    ,fEliminatedByHigherStreamline(false)
    {
    }

    int itsPriority; // luku 0-n, mit‰ pienempi luku, sit‰ korkeampi prioriteetti. Ota t‰h‰n arvo t‰m‰n x- ja y-indksin osoittamasta NFmiStartPointEliminationData-rakenteesta.
    size_t itsXIndex; // k‰sitelt‰v‰n aloituspisteen x- ja y-indeksit
    size_t itsYIndex;
    NFmiDataMatrix<NFmiPoint> &itsPotencialStartingPoints;
    NFmiDataMatrix<NFmiStartPointEliminationData> &itsStartingPointEliminatioMatrix;
    bool fEliminatedByHigherStreamline;
};

void NFmiStreamLineView::DoMultiThreadCalculations(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiDataMatrix<NFmiStartPointEliminationData> &theStartingPointEliminatioMatrix, int timeStepInMinutes, std::vector<NFmiStreamlineData> &theLatlonPathsOut, double theProximityLimit, double theLengthLimitInKM)
{
    // Multi-thread laskuissa tehd‰‰n enemm‰n tˆit‰, mutta rinnakkain. 
    // Nyt ei voi eliminoida toisia aloituspisteit‰ laskujen aikana, vaan kaikki aloituspisteet lasketaan.
    // Jokaiselle streamline oliolle on laskettu prioriteetti indeksi, keskimm‰inen on korkein, keskijanoilla
    // keskustasta ulosp‰in on seuraavta ja jne.
    // Kun kaikki pisteet on laskettu, katsotaan prioriteetti j‰rjestyksess‰, eliminoiko jokin streamline jonkin aloitus pisteen.
    // Jos eliminoi, otetaan kyseisen aloituspisteen streamline pois lopullisista piirrett‰vist‰ viivoista.

	unsigned int usedThreadCount = NFmiQueryDataUtil::GetReasonableWorkingThreadCount(75);
    NFmiPoint dummyPoint = itsInfo->LatLon(); // Varmistetaan ett‰ NFmiQueryDatan itsLatLonCache on alustettu!!

    // 1. Tee jokaiselle threadille kopio itsInfo:sta
	std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infos(usedThreadCount);
	for(unsigned int i = 0; i < usedThreadCount; i++)
        infos[i] = boost::shared_ptr<NFmiFastQueryInfo>(new NFmiFastQueryInfo(*itsInfo));
}

std::vector<NFmiStreamlineData> NFmiStreamLineView::CalcStreamLinePaths(const StreamlineCalculationParameters &theCalcParams)
{ 
    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, __FUNCTION__);
    ResetDebugData();
    int timeStepInMinutes = theCalcParams.itsTimeStepInMinutes;
    if(timeStepInMinutes < 1)
        timeStepInMinutes = 1;
    std::vector<NFmiStreamlineData> latlonPaths;
    NFmiDataMatrix<NFmiPoint> potencialStartingPoints = CalcPotencialStartingPoints(theCalcParams);
    if(potencialStartingPoints.NX() && potencialStartingPoints.NY())
    {
        itsDebugTotalStartingPoints = potencialStartingPoints.NX() * potencialStartingPoints.NY();
        NFmiDataMatrix<NFmiStartPointEliminationData> startingPointEliminatioMatrix = CalcStartingPointEliminationMatrix(potencialStartingPoints, itsArea);
        double proximityLimit = ::CalcProximityLimit(potencialStartingPoints);
        double lengthLimitInKM = ::CalcLengthLimitInKM(itsArea);

        bool doMTCalculations = false;
        if(doMTCalculations)
            DoMultiThreadCalculations(itsInfo, startingPointEliminatioMatrix, timeStepInMinutes, latlonPaths, proximityLimit, lengthLimitInKM);
        else
            DoSingleThreadCalculations(theCalcParams, itsInfo, startingPointEliminatioMatrix, timeStepInMinutes, latlonPaths, proximityLimit, lengthLimitInKM);

        // Debuggaus koodia, piirret‰‰n ns. eliminaatio matriisin tiedot ruudulle
        if(gDrawDebugInfo)
            DrawDebugEliminationMatrixInfo(startingPointEliminatioMatrix);
        itsDebugEliminatedStartingPoints = ::CountEliminatedPoints(startingPointEliminatioMatrix);
    }

    return latlonPaths;
}

static std::string MakePointIndexString(const NFmiPoint &thePointIndex)
{
    std::string str("(");
    str += NFmiStringTools::Convert(static_cast<int>(thePointIndex.X()));
    str += ",";
    str += NFmiStringTools::Convert(static_cast<int>(thePointIndex.Y()));
    str += ")";

    return str;
}

// Piirret‰‰n laatikko josta n‰hd‰‰n eliminointi alue ja eri v‰reill‰ riippuen oliko piste eliminoitu.
// Jos oli eliminoitu, llis‰t‰‰n eliminointi tietoa: mik‰ polku ja mik‰ sen pisteist‰.
void NFmiStreamLineView::DrawDebugEliminationInfo(const NFmiStartPointEliminationData &theEliminatioInfo)
{
    NFmiColor fontColor(0,0,0);
    double fontSizeInMM = 3;
    double relativeFontHeight = theEliminatioInfo.itsStartPointArea.Height() / 6.;
    NFmiPoint p(theEliminatioInfo.itsStartPointArea.Center());
    p.Y(theEliminatioInfo.itsStartPointArea.Top() + relativeFontHeight/1.5);

    Gdiplus::Rect rectInPixels = CtrlView::Relative2GdiplusRect(itsToolBox, theEliminatioInfo.itsStartPointArea);
    if(theEliminatioInfo.fWasUsedAsStartingPoint)
    {
        NFmiColor rectColor(0.2f,0.7f,0.4f);
        CtrlView::DrawRect(*itsGdiPlusGraphics, rectInPixels, rectColor, rectColor, false, true, 2);
        DrawDebugString(fontSizeInMM, p, fontColor, ::MakePointIndexString(theEliminatioInfo.itsStartingPointIndex), false);
    }
    else if(theEliminatioInfo.fWasOutOfZoomedAreaStartingPoint)
    {
        NFmiColor rectColor(1.f,0.5f,0.f);
        CtrlView::DrawRect(*itsGdiPlusGraphics, rectInPixels, rectColor, rectColor, false, true, 1);
    }
    else if(theEliminatioInfo.fMarkedShortly)
    {
        NFmiColor rectColor(0.85f,0.4f,0.95f);
        CtrlView::DrawRect(*itsGdiPlusGraphics, rectInPixels, rectColor, rectColor, false, true, 1);
    }
    else if(theEliminatioInfo.fEliminated)
    {
        NFmiColor rectColor(0.9f,0.9f,0.1f);
        CtrlView::DrawRect(*itsGdiPlusGraphics, rectInPixels, rectColor, rectColor, false, true, 1);
        DrawDebugString(fontSizeInMM, p, fontColor, ::MakePointIndexString(theEliminatioInfo.itsStartingPointIndexEliminated), false);

        p.Y(p.Y() + relativeFontHeight);
        std::string str = theEliminatioInfo.fForwardPathEliminate ? "F" : "B";
        DrawDebugString(fontSizeInMM, p, fontColor, str, false);
        p.Y(p.Y() + relativeFontHeight);
        str = "ind:";
        str += NFmiStringTools::Convert(static_cast<int>(theEliminatioInfo.itsEliminationPointIndex));
        DrawDebugString(fontSizeInMM, p, fontColor, str, false);
    }
    else
    {
        NFmiColor rectColor(0.7f,0.7f,0.7f);
        CtrlView::DrawRect(*itsGdiPlusGraphics, rectInPixels, rectColor, rectColor, false, true, 1);
    }
}

void NFmiStreamLineView::DrawDebugEliminationMatrixInfo(const NFmiDataMatrix<NFmiStartPointEliminationData> &theStartingPointEliminatioMatrix)
{
    for(size_t j=0; j < theStartingPointEliminatioMatrix.NY(); j++)
    {
        for(size_t i=0; i < theStartingPointEliminatioMatrix.NX(); i++)
        {
            DrawDebugEliminationInfo(theStartingPointEliminatioMatrix[i][j]);
        }
    }
}

void NFmiStreamLineView::DrawDebugPath(const NFmiStreamlineData &thePath, int rectSizeInPixels, const NFmiColor &theRectColor, const NFmiColor &theFontColor)
{
    DrawDebugPath(thePath.ForwardPathLatlon(), rectSizeInPixels, theRectColor, theFontColor);
    DrawDebugPath(thePath.BackwardPathLatlon(), rectSizeInPixels, theRectColor, theFontColor);
}

void NFmiStreamLineView::DrawDebugPath(const std::vector<NFmiPoint>& theVectorPath, int rectSizeInPixels, const NFmiColor &theRectColor, const NFmiColor &theFontColor)
{
    for(size_t i = 0; i < theVectorPath.size(); i++)
    {
        DrawDebugRect(rectSizeInPixels, theVectorPath[i], theRectColor);
        std::string str(" ");
        str += NFmiStringTools::Convert(i);
        DrawDebugString(3.0, theVectorPath[i], theFontColor, str, true);
    }
}

static std::list<std::vector<NFmiPoint> > SplitPathsAtEdgeOfWorld(const std::vector<NFmiPoint> &theOneWayPath, bool fPacificView)
{
    std::list<std::vector<NFmiPoint> > possibleSplittedPaths;
    if(theOneWayPath.size() > 1)
    {
        size_t startIndexOfSplit = 0;
        size_t endIndexOfSplit = 0;
        double lastLongitude = theOneWayPath[0].X();
        for(size_t i = 1; i < theOneWayPath.size(); i++)
        {
            double currentLongitude = theOneWayPath[i].X();
            if(::CrossesOverWorldsEdge(lastLongitude, currentLongitude))
            { // tehd‰‰n splitti
                endIndexOfSplit = i-1;
                std::vector<NFmiPoint> splitPath(theOneWayPath.data() + startIndexOfSplit, theOneWayPath.data() + endIndexOfSplit);
                possibleSplittedPaths.push_back(splitPath);
                startIndexOfSplit = i;
            }
            lastLongitude = currentLongitude;
        }
        if(possibleSplittedPaths.size())
        { // jos oli tehty splittej‰, laitetaan viel‰ loppu p‰tk‰ mukaan
            endIndexOfSplit = theOneWayPath.size()-1;
            std::vector<NFmiPoint> splitPath(theOneWayPath.data() + startIndexOfSplit, theOneWayPath.data() + endIndexOfSplit);
            possibleSplittedPaths.push_back(splitPath);
        }
    }
    return possibleSplittedPaths;
}

void NFmiStreamLineView::DrawOneWayPath(const StreamlineCalculationParameters &theCalcParams, const std::vector<NFmiPoint> &theOneWayPath, const GdiPlusLineInfo &theLineInfo, bool forwardDirection, float theLineWidthInMM)
{
    std::list<std::vector<NFmiPoint> > possibleSplittedPaths = SplitPathsAtEdgeOfWorld(theOneWayPath, itsArea->PacificView());
    if(possibleSplittedPaths.size())
    {
        std::list<std::vector<NFmiPoint> >::iterator iter = possibleSplittedPaths.begin();
        for(; iter != possibleSplittedPaths.end(); ++iter)
            DrawOneWayPath(theCalcParams, CtrlView::ConvertLatLon2GdiPoints(this, *iter), theLineInfo, forwardDirection, theLineWidthInMM);
    }
    else
        DrawOneWayPath(theCalcParams, CtrlView::ConvertLatLon2GdiPoints(this, theOneWayPath), theLineInfo, forwardDirection, theLineWidthInMM);
}

void NFmiStreamLineView::DrawStreamLinePaths(const StreamlineCalculationParameters &theCalcParams, const std::vector<NFmiStreamlineData> &theLatlonPaths)
{ 
    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, __FUNCTION__);
    for(size_t i = 0; i < theLatlonPaths.size(); ++i)
    {
        const NFmiStreamlineData &streamlineData = theLatlonPaths[i];
        float lineWidthInMM = theCalcParams.itsLineWidthInMM;
		float lineWidthInPixels = static_cast<float>(lineWidthInMM * itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_x);
        GdiPlusLineInfo lineInfo(lineWidthInPixels, theCalcParams.itsLineColor, 0); // line style 0 => yhten‰inen viiva
        lineInfo.Tension(0.f);
        DrawOneWayPath(theCalcParams, streamlineData.ForwardPathLatlon(), lineInfo, true, lineWidthInMM);
        DrawOneWayPath(theCalcParams, streamlineData.BackwardPathLatlon(), lineInfo, false, lineWidthInMM);

        // debuggaus koodia, piirret‰‰n polkujen pisteet ja indeksit
////        if(gDrawDebugInfo)
////            DrawDebugPath(streamlineData, 3, NFmiColor(0,0.5f,0), NFmiColor(0,0,0));
    }

    if(gDrawDebugInfo)
        DrawDebugDataOnMap();
}

void NFmiStreamLineView::DrawOneWayPath(const StreamlineCalculationParameters &theCalcParams, std::vector<Gdiplus::PointF> &gdiPoints, const GdiPlusLineInfo &lineInfo, bool forwardDirection, float theLineWidthInMM)
{
    if(gdiPoints.size() > 1)
    {
        CtrlView::DrawGdiplusCurve(*itsGdiPlusGraphics, gdiPoints, lineInfo, false, 0, itsToolBox->GetDC()->IsPrinting() == TRUE);
        DrawArroyHeads(theCalcParams, gdiPoints, lineInfo, forwardDirection, theLineWidthInMM);
    }
}

// Antaa skaalausarvon, jolla suurennetaan/pienennet‰‰n tuulen suuntaa osoittavaa nuolta.
// Pienill‰ tuulen nopeuksilla saa arvon 0-1 ja suurilla > 1
static float CalcWindArrowSizeFactor(float WS, float noScaleLimit)
{
    if(WS == kFloatMissing)
        return 0.f;
    float wsLimit2 = noScaleLimit; // raja miss‰ kerroin saa arvon 1
    if(wsLimit2 == 0)
        return 1; // drawParamin oletus arvo t‰lle s‰‰dˆlle on 0, jos t‰m‰ siis 0, annetaan kaikille arvoilla kerroin 1
    float wsLimit1 = 0;
    float factorMaxLimit = 1.4f;
    float factorLimit2 = 1.0f;
    float factorLimit1 = 0.6f;
    float angleFactor = (factorLimit2- factorLimit1) / (wsLimit2 - wsLimit1);
    float sizeFactor = factorLimit1 + angleFactor * WS;
    if(sizeFactor > factorMaxLimit)
        sizeFactor = factorMaxLimit;
    return sizeFactor;
}

void NFmiStreamLineView::DrawArroyHeads(const StreamlineCalculationParameters &theCalcParams, std::vector<Gdiplus::PointF> &theStreamLineGdiPoints, const GdiPlusLineInfo &theLineInfo, bool forwardDirection, float theLineWidthInMM)
{
    FrontType pathType = kFmiFrontTypeStreamLine;
// 1. laske viivan paksuuden ja rintaman tyypin avulla
	// a) toivottu pallukan koko [mm]
	// b) toivottu alku/loppu tyhj‰ v‰li [mm]
	// c) toivottu v‰li [mm]
	float startGapInMM = 0;
	float objectSizeInMM = 0;
	float gapInMM = 0;
    NFmiConceptualDataView::CalculateIdealPathObjectMeasures(theLineWidthInMM, pathType, startGapInMM, objectSizeInMM, gapInMM);
// 2. Laske koko viivan pituus [mm]
	// laske myˆs eri osien pituudet taulukkoon
	float pixelLengthInMM = 1.f/static_cast<float>(itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_x);
	std::vector<float> lengthsInMM;
	float totalLengthInMM = 0;
	NFmiConceptualDataView::CalculatePathLengths(pixelLengthInMM, theStreamLineGdiPoints, lengthsInMM, totalLengthInMM);
// 3. Laske kuinka monta pallukkaa mahtuu annetun rintaman alueelle
	// jos alle 1, sovita pallukan koko, niin ett‰ mahtuu yksi (laske kerroin jolla skaalataan ja piirr‰ viivan keskelle pallukka)
	// jos alle kaksi, sovita pallukan koko, niin ett‰ mahtuu kaksi (laske skaala kerroin ja piirr‰ kahteen kohtaan pallukat)
	// muuten pallukat laske l‰hinp‰‰n kokonaislukuun sopiva skaala kerroin
	int objectCount = NFmiConceptualDataView::CalculateUsedSymbolCountAndMeasures(totalLengthInMM, startGapInMM, objectSizeInMM, gapInMM);
	if(objectCount > 0)
	{
	// 4. hae pallukoiden piirto polku (GraphicsPath) rintamatyypin mukaan
        Gdiplus::GraphicsPath decorationPath;
        NFmiConceptualDataView::GetDecorationPath(objectSizeInMM/pixelLengthInMM, pathType, decorationPath);
	// 5. Laske pallukoiden lukum‰‰r‰n ja muiden mittojen avulla pallukoiden kohdat ja talleta sijainnit taulukkoon
	// 6. laske tarvittavat pallukoiden rotaatio kulmat haluttuihin kohtiin viivaa [asteissa]
		std::vector<Gdiplus::PointF> decoratorPoints;
		std::vector<float> rotationAngles;
		NFmiConceptualDataView::CalcPathObjectPoints(theStreamLineGdiPoints, objectCount, startGapInMM, objectSizeInMM, gapInMM, lengthsInMM, totalLengthInMM, decoratorPoints, rotationAngles);
	// 7. Piirr‰ pallukat skaalan, sijainnin ja rotaation avulla
		// jos left/right suunta, s‰‰d‰ rotaatiota 180 asteella
        Gdiplus::SolidBrush aBrush(CtrlView::NFmiColor2GdiplusColor(theCalcParams.itsArrowHeadColor));
        float usedLineWidthInPixels = theLineInfo.Thickness();
		Gdiplus::Pen aPen(CtrlView::NFmiColor2GdiplusColor(theLineInfo.Color()), usedLineWidthInPixels);
		aPen.SetLineCap(Gdiplus::LineCapRound, Gdiplus::LineCapRound, Gdiplus::DashCapFlat);

		Gdiplus::Matrix transMat; // originaali muutos matriisi t‰h‰n
		itsGdiPlusGraphics->GetTransform(&transMat);
		float baseRotationFix = 90;
        if(!forwardDirection)
            baseRotationFix = -90;
		float usedRotationFix = baseRotationFix;
        float WS = kFloatMissing; 
        float WD = kFloatMissing;
        float noScaleLimit = theCalcParams.itsNoScaleLimit;

		for(size_t j = 0; j < decoratorPoints.size(); j++)
		{
			Gdiplus::PointF &p1 = decoratorPoints[j];
            CalcWindValues(itsInfo, CtrlView::ConvertGdiPlusPointToLatlon(this, p1), WS, WD);
            float sizeFactor = ::CalcWindArrowSizeFactor(WS, noScaleLimit);
			itsGdiPlusGraphics->TranslateTransform(p1.X, p1.Y);
			itsGdiPlusGraphics->RotateTransform(rotationAngles[j] + usedRotationFix);
			itsGdiPlusGraphics->ScaleTransform(sizeFactor, sizeFactor);

//            itsGdiPlusGraphics->DrawPath(&aPen, &decorationPath);
            itsGdiPlusGraphics->FillPath(&aBrush, &decorationPath);

            itsGdiPlusGraphics->SetTransform(&transMat); // palautetaan aina originaali muutos msatriisi k‰yttˆˆn, sill‰ muuten transformaatiot kumuloituvat
		}
	}
}

static float CalcTooltipValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiFastInfoUtils::MetaWindParamUsage &metaWindParamUsage, unsigned long theParamId, const NFmiPoint &theLatLon, const NFmiMetTime &theTime)
{
    if(metaWindParamUsage.ParamNeedsMetaCalculations(theParamId))
    {
        return NFmiFastInfoUtils::GetMetaWindValue(theInfo, theTime, theLatLon, metaWindParamUsage, theParamId);
    }
    else
    {
        theInfo->Param(static_cast<FmiParameterName>(theParamId));
        return theInfo->InterpolatedValue(theLatLon, theTime);
    }
}

std::string NFmiStreamLineView::ComposeToolTipText(const NFmiPoint& theRelativePoint)
{
	std::string tabStr = "	";
	std::string str;
	if(itsDrawParam)
	{
		bool showExtraInfo = CtrlView::IsKeyboardKeyDown(VK_CONTROL); // jos CTRL-n‰pp‰in on pohjassa, laitetaan lis‰‰ infoa n‰kyville
		auto parNameStr = CtrlViewUtils::GetParamNameString(itsDrawParam, false, showExtraInfo, true, 0, false, true, true, nullptr);
        parNameStr = DoBoldingParameterNameTooltipText(parNameStr);
        auto fontColor = CtrlViewUtils::GetParamTextColor(itsDrawParam->DataType(), itsDrawParam->UseArchiveModelData());
        parNameStr = AddColorTagsToString(parNameStr, fontColor, true);
        str += parNameStr + tabStr;
        itsInfo = itsCtrlViewDocumentInterface->InfoOrganizer()->Info(itsDrawParam, false, false);
        if(itsInfo && itsInfo->Grid() && itsInfo->TimeDescriptor().IsInside(itsTime))
        {
            float WS = ::CalcTooltipValue(itsInfo, metaWindParamUsage, kFmiWindSpeedMS,  itsCtrlViewDocumentInterface->ToolTipLatLonPoint(), itsTime);
            str += " WS=";
            str += GetToolTipValueStr(WS, itsInfo, itsDrawParam);
            float WD = ::CalcTooltipValue(itsInfo, metaWindParamUsage, kFmiWindDirection, itsCtrlViewDocumentInterface->ToolTipLatLonPoint(), itsTime);
            str += " WD=";
            str += GetToolTipValueStr(WD, itsInfo, itsDrawParam);
        }
        else
            str += "?";
    }
    return str;
}

void NFmiStreamLineView::DrawDebugDataOnMap(void)
{
    NFmiColor fontColor(0,0,0);
    double fontSizeInMM = 6;
    long fontSizeInPixels = FmiRound(itsCtrlViewDocumentInterface->GetGraphicalInfo(itsMapViewDescTopIndex).itsPixelsPerMM_y * fontSizeInMM);
    double relativeFontHeight = itsToolBox->SY(fontSizeInPixels);
    NFmiPoint p =  LatLonToViewPoint(itsArea->CenterLatLon());
    std::string str = "TotalStartingPoints: ";
    str += NFmiStringTools::Convert(itsDebugTotalStartingPoints);
    DrawDebugString(fontSizeInMM, p, fontColor, str, false);

    p.Y(p.Y() + relativeFontHeight);
    str = "OutOfAreaStartingPoints: ";
    str += NFmiStringTools::Convert(itsDebugOutOfAreaStartingPoints);
    DrawDebugString(fontSizeInMM, p, fontColor, str, false);

    p.Y(p.Y() + relativeFontHeight);
    str = "CalculatedStartingPoints: ";
    str += NFmiStringTools::Convert(itsDebugCalculatedStartingPoints);
    DrawDebugString(fontSizeInMM, p, fontColor, str, false);

    p.Y(p.Y() + relativeFontHeight);
    str = "EliminatedStartingPoints: ";
    str += NFmiStringTools::Convert(itsDebugEliminatedStartingPoints);
    DrawDebugString(fontSizeInMM, p, fontColor, str, false);

    p.Y(p.Y() + relativeFontHeight);
    str = "TotalPointsCalculated: ";
    str += NFmiStringTools::Convert(itsDebugTotalPointsCalculated);
    DrawDebugString(fontSizeInMM, p, fontColor, str, false);

    p.Y(p.Y() + relativeFontHeight);
    str = "Timer1: ";
    str += itsDebugTimer1.EasyTimeDiffStr();
    DrawDebugString(fontSizeInMM, p, fontColor, str, false);

    p.Y(p.Y() + relativeFontHeight);
    str = "CombinedTimer1: ";
    str += NFmiMilliSecondTimer::EasyTimeDiffStr(static_cast<int>(itsDebugCombinedTimerMS1), false);
    DrawDebugString(fontSizeInMM, p, fontColor, str, false);

    p.Y(p.Y() + relativeFontHeight);
    str = "Timer2: ";
    str += itsDebugTimer2.EasyTimeDiffStr();
    DrawDebugString(fontSizeInMM, p, fontColor, str, false);

    p.Y(p.Y() + relativeFontHeight);
    str = "CombinedTimer2: ";
    str += NFmiMilliSecondTimer::EasyTimeDiffStr(static_cast<int>(itsDebugCombinedTimerMS2), false);
    DrawDebugString(fontSizeInMM, p, fontColor, str, false);

    p.Y(p.Y() + relativeFontHeight);
    str = "Timer3: ";
    str += itsDebugTimer3.EasyTimeDiffStr();
    DrawDebugString(fontSizeInMM, p, fontColor, str, false);

    p.Y(p.Y() + relativeFontHeight);
    str = "CombinedTimer3: ";
    str += NFmiMilliSecondTimer::EasyTimeDiffStr(static_cast<int>(itsDebugCombinedTimerMS3), false);
    DrawDebugString(fontSizeInMM, p, fontColor, str, false);
}

void NFmiStreamLineView::ResetDebugData(void)
{
    itsDebugTotalStartingPoints = 0;
    itsDebugOutOfAreaStartingPoints = 0;
    itsDebugCalculatedStartingPoints = 0;
    itsDebugEliminatedStartingPoints = 0;
    itsDebugTotalPointsCalculated = 0;
    itsDebugTotalSimplifiedPoints = 0;
    itsDebugTotalStreamLineCount = 0;
    itsDebugTotalRelativeJumpPointsCalculated = 0;
    itsDebugCombinedTimerMS1 = 0;
    itsDebugCombinedTimerMS2 = 0;
    itsDebugCombinedTimerMS3 = 0;
}
