//**********************************************************
// C++ Class Name : NFmiObsDataGridding 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiObsDataGridding.cpp 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 4.0 ) 
//  - GD System Name    : editori virityksi‰ 2000 syksy 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : interpolation luokka 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Tues - Nov 7, 2000 
// 
//  Change Log     : 
// 
//**********************************************************
#include "NFmiObsDataGridding.h"
#include "NFmiLocation.h"
#include "NFmiDataModifierClasses.h"

#include <algorithm>
//--------------------------------------------------------
// Constructor/Destructor 
//--------------------------------------------------------
NFmiObsDataGridding::NFmiObsDataGridding (void)
{
}
NFmiObsDataGridding::~NFmiObsDataGridding (void)
{
}
//--------------------------------------------------------
// DoGridding 
//--------------------------------------------------------
// Laskee annetusta pistedatasta (x,y,z) hila dataa annettuun hilaan.
// Hilan alue on edell‰ m‰‰r‰tty SetAreaLimits metodilla (oletus 0,0 - 1,1).
void NFmiObsDataGridding::DoGridding (std::vector<float> &x, std::vector<float> &y, std::vector<float> &z, int count, NFmiDataMatrix<float> &gridData)
{
	if(count == 0)
		DoOneValueGridding(0.f, gridData);
	else if(count == 1)
		DoOneValueGridding(z[0], gridData);
	else
	{
		itsXArray = x;
		itsYArray = y;
		itsZArray = z;
		itsArraySize = count;
		float yGridPoint = static_cast<float>(itsAreaLimits.Top());
		float deltaY = static_cast<float>(itsAreaLimits.Height()/(gridData.NY()-1));
		for(unsigned int j = 0; j < gridData.NY(); j++)
		{
			float xGridPoint = static_cast<float>(itsAreaLimits.Left());
			float deltaX = static_cast<float>(itsAreaLimits.Width()/(gridData.NX()-1));
			for(unsigned int i = 0; i < gridData.NX(); i++)
			{
				checkedVector<DistIndex> resultVector;
				(void) FindClosestPoints(xGridPoint, yGridPoint, itsSearchRange, 4, resultVector);
				gridData[i][j] = CalcValue(resultVector);
				xGridPoint += deltaX;
			}
			yGridPoint += deltaY;
		}
		SmoothGrid(gridData, 1, 1); // pakko tasoittaa kun on niin rosoista
	}
}

void NFmiObsDataGridding::SmoothGrid(NFmiDataMatrix<float> &gridData, int ySmooth, int xSmooth)
{
	unsigned int yCount = static_cast<unsigned int>(gridData.NY());
	unsigned int xCount = static_cast<unsigned int>(gridData.NX());
	NFmiDataModifierAvg avg;
	for(unsigned int j = 0; j < yCount; j++)
	{
		for(unsigned int i = 0; i < xCount; i++)
		{
			avg.Clear();
			unsigned int yStart = j-ySmooth >= 0 ? j-ySmooth : 0;
			unsigned int yEnd = j+ySmooth <= yCount ? j+ySmooth : yCount;
			for(unsigned int y = yStart; y < yEnd; y++)
			{
				unsigned int xStart = i-xSmooth >= 0 ? i-xSmooth : 0;
				unsigned int xEnd = i+xSmooth <= xCount ? i+xSmooth : xCount;
				for(unsigned int x = xStart; x < xEnd; x++)
				{
					avg.Calculate(gridData[x][y]);
					avg.Calculate(gridData[i][j]); // painotetaan keskipistett‰!!!
				}
			}
			gridData[i][j] = avg.CalculationResult();
		}
	}
}

//--------------------------------------------------------
// DoOneValueGridding 
//--------------------------------------------------------
// T‰ytt‰‰ annetun hilan annetulla arvolla.
void NFmiObsDataGridding::DoOneValueGridding (float fillValue, NFmiDataMatrix<float> &gridData)
{
	for(unsigned int j=0; j<gridData.NY(); j++)
		for(unsigned int i=0; i<gridData.NX(); i++)
			gridData[i][j] = fillValue;
}
//--------------------------------------------------------
// FindClosestPoints 
//--------------------------------------------------------
bool NFmiObsDataGridding::FindClosestPoints (float xGridPoint, float yGridPoint, float /* searchRange */ , int wantedPointCount, checkedVector<DistIndex>& theResultVector)
{
	for(int i = 0; i < itsArraySize; i++)
	{
		float relDist = CalcRelativeDistance(xGridPoint, yGridPoint, itsXArray[i], itsYArray[i]);
		float dist = CalcDistance(xGridPoint, yGridPoint, itsXArray[i], itsYArray[i]);
		DistIndex dInd(dist, relDist, i);
		theResultVector.push_back(dInd);
	}
	std::sort(theResultVector.begin(), theResultVector.end());
	int size = static_cast<int>(theResultVector.size());
	size = FmiMin(size, wantedPointCount);
	theResultVector.resize(size);
	return theResultVector.size() > 0;
}
//--------------------------------------------------------
// CalcWeight 
//--------------------------------------------------------
// Laskee painon annetulle et‰isyydelle.
float NFmiObsDataGridding::CalcWeight (float distance, float theMaxDist) const
{
	if(distance >= theMaxDist)
		return 0;

	float value = (theMaxDist - distance)/theMaxDist;
	value = value*value*value*value*value;
	return value;
}
//--------------------------------------------------------
// CalcValue 
//--------------------------------------------------------
// Laskee FindClosestPoints:issa annetun pisteen arvon annetun et‰isyys 
// vektorin ja indeksien kautta z-taulukon arvojen avulla.
float NFmiObsDataGridding::CalcValue (checkedVector<DistIndex>& theClosestPointsResultVector)
{
	float returnVal = 0.0;
	float weight = 0;
	float weightSum = 0;
	float weightedChangeSum = 0;
	float value = 0;
	float maxDist = 1.f;
	if(theClosestPointsResultVector.size() >= 2)
		maxDist = theClosestPointsResultVector[theClosestPointsResultVector.size()-1].RelativeDistance();
	for(unsigned int i = 0; i < theClosestPointsResultVector.size(); i++)
	{
		weight = CalcWeight(theClosestPointsResultVector[i].RelativeDistance(), maxDist);
		weightSum += weight;
		value = itsZArray[theClosestPointsResultVector[i].Index()];
		weightedChangeSum += weight * value;
	}
	if(weightSum)
		returnVal = weightedChangeSum/weightSum;
	return returnVal;
}
//--------------------------------------------------------
// CalcDistance 
//--------------------------------------------------------
float NFmiObsDataGridding::CalcDistance(float lon1, float lat1, float lon2, float lat2) const
{
	NFmiLocation loc1(lon1, lat1);
	NFmiLocation loc2(lon2, lat2);
	float dist = static_cast<float>(loc1.Distance(loc2));
	return dist;
}

float NFmiObsDataGridding::CalcRelativeDistance (float x1, float y1, float x2, float y2) const
{
   float returnVal = float(sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)));
   return returnVal;
}

bool NFmiObsDataGridding::DistIndex::operator<(const DistIndex& other)  const
{
	return itsRelativeDistance < other.itsRelativeDistance;
}

bool NFmiObsDataGridding::DistIndex::operator==(const DistIndex& other) const
{
	return itsRelativeDistance == other.itsRelativeDistance;
}

