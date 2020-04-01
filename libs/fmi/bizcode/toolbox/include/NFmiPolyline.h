//© Ilmatieteenlaitos/Persa.        fpolylin.h     ver. 4.11.1994
//13.9.1994   Persa
//
//Ver. 26.10.1994

//Muutos 4.11.1994     Lisatty SwapX jaSwapY metodit
//Ver. 25.10.1999 /Marko Lis‰sin konstruktorin.
//Ver. 19.01.2000 /Marko Lis‰sin muutaman piirto option t‰nne, koska en keksinyt miten ne saisi parhaiten drawenviin.

//--------------------------------------------------------------------------- NFmiPolyline.h

#pragma once

#include "NFmiShape.h"

//_________________________________________________________ NFmiPolyline
class NFmiPolyline : public NFmiShape
{
public:

    NFmiPolyline(const NFmiPoint& theTopLeftCorner
        , const NFmiPoint& theBottomRightCorner
        , const NFmiView* theEnclosure = 0
        , NFmiDrawingEnvironment* theEnvironment = 0
        , int theBackGroundMode = 1 // opaque
        , int theHatchMode = -1);  // -1 tarkoittaa, ettei k‰ytet‰ kuvioita
    NFmiPolyline(const NFmiRect& theRect
        , const NFmiView* theEnclosure = 0
        , NFmiDrawingEnvironment* theEnvironment = 0
        , int theBackGroundMode = 1 // opaque
        , int theHatchMode = -1);  // -1 tarkoittaa, ettei k‰ytet‰ kuvioita


    virtual ~NFmiPolyline();
    const std::vector<NFmiPoint>& GetPoints(void) const { return itsPoints; };
    void AddPoint(const NFmiPoint& newPoint);
    std::vector<NFmiPoint>::iterator begin() { return itsPoints.begin(); }
    std::vector<NFmiPoint>::const_iterator begin() const { return itsPoints.begin(); }
    std::vector<NFmiPoint>::iterator end() { return itsPoints.end(); }
    std::vector<NFmiPoint>::const_iterator end() const { return itsPoints.end(); }

    int BackGroundMode(void) const { return itsBackGroundMode; };
    void BackGroundMode(int newMode) { itsBackGroundMode = newMode; };
    int HatchMode(void) { return itsHatchMode; };
    void HatchMode(int newMode) { itsHatchMode = newMode; };
    bool UseHatch(void) { return fUseHatch; };
    void UseHatch(bool newStatus) { fUseHatch = newStatus; };

private:
    std::vector<NFmiPoint> itsPoints;
    // opaque (oletus) tai transparent
    int itsBackGroundMode; 
    // millainen kuvio pensseliin, jos k‰ytet‰‰n
    int itsHatchMode; 
    bool fUseHatch = false;
};

