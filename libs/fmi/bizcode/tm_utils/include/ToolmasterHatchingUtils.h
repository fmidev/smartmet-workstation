#pragma once

class IntPoint
{
public:
    IntPoint();
    IntPoint(int x, int y);

    int x = 0;
    int y = 0;
};

class FloatPoint
{
public:
    FloatPoint();
    FloatPoint(float x, float y);

    float x = 0;
    float y = 0;
};

class BoundingBox
{
public:
    BoundingBox();

    float top = 0;
    float bottom = 0;
    float left = 0;
    float right = 0;
};

class TMWorldLimits
{
public:
    TMWorldLimits();

    float x_min = 0;
    float x_max = 1;
    float y_min = 0;
    float y_max = 1;
    float z_min = 0;
    float z_max = 0;

    float width() const;
    float height() const;
};

// Miten Toolmaster polygoni on suhteessa hilarivin float-datan arvoihin.
// Eli miten pohjahilarivin arvot ovat suhteessa hatch:in arvoihin kun liikutaan 
// vasemmalta oikealle...
enum class PolygonsBottomEdgeRelation
{
    NoValue = 0,
    AlwaysInside = 1,
    AlwaysOutside = 2,
    FirstInside = 3,
    FirstOutside = 4
};

enum class PolygonsBottomEdgeTouching
{
    NoValue = 0,
    AlwaysTouching,
    AlmostAlwaysTouching, // T‰t‰ k‰sitell‰‰n kuten AlwaysTouching:ia, mutta t‰ss‰ on m‰‰r‰tty prosenttiraja, mill‰ alueella polygoni on kosketuksissa pohjakerrokseen
    AlwaysNotTouching,
    FirstTouching,
    FirstNotTouching
};

enum class CoordinateYStatus
{
    NoValue = 0,
    BottomRowValue,
    BottomRowInToolmasterMarginCase,
    NotBottomRowValue
};
