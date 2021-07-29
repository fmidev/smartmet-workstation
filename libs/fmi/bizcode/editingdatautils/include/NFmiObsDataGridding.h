//**********************************************************
// C++ Class Name : NFmiObsDataGridding 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiObsDataGridding.h 
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

#pragma once

#include "NFmiRect.h"
#include "NFmiDataMatrix.h" // t‰‰lt‰ tulee myˆs std::vector


class NFmiObsDataGridding 
{

public:

	//********************************************************************
	// Luokka joka pit‰‰ sis‰ll‰‰n et‰isyyden ja indeksin.
	// K‰ytet‰‰n griddaus luokan sis‰ll‰, m‰‰ritell‰‰n griddaus-luokassa.
	class DistIndex 
	{

	public:
		DistIndex(float dist=0.f, float relDist=0.f, int index = -1):itsDistance(dist),itsRelativeDistance(relDist),itsIndex(index){};
		bool operator<(const DistIndex& other) const;// {return itsRelativeDistance < other.itsRelativeDistance;};
		bool operator==(const DistIndex& other) const;// {return itsRelativeDistance == other.itsRelativeDistance;};
		inline void Distance(float value) {itsDistance = value;}
		inline float Distance(void) const {return itsDistance;}
		inline void RelativeDistance(float value) {itsRelativeDistance = value;}
		inline float RelativeDistance(void) const {return itsRelativeDistance;}
		inline void Index(int value) {itsIndex = value;}
		inline int Index() const {return itsIndex;}

	private:
		float itsDistance;
		float itsRelativeDistance;
		int itsIndex;
	};
	//********************************************************************


   NFmiObsDataGridding(void) ;
   ~NFmiObsDataGridding(void) ;
   void DoGridding(std::vector<float> &x, std::vector<float> &y, std::vector<float> &z, int count, NFmiDataMatrix<float> &gridData);
   inline void AreaLimits(const NFmiRect& value){itsAreaLimits = value;}
   const NFmiRect& AreaLimits(void) const {return itsAreaLimits;}
   inline void SearchRange(float value) {itsSearchRange = value;}
   inline float SearchRange(void) const {return itsSearchRange;}

private:
   void SmoothGrid(NFmiDataMatrix<float> &gridData, int ySmooth, int xSmooth);
   void DoOneValueGridding(float fillValue, NFmiDataMatrix<float> &gridData);
   bool FindClosestPoints(float xGridPoint, float yGridPoint, float searchRange, int wantedPointCount, std::vector<DistIndex>& theResultVector) ;
   float CalcWeight(float distance, float theMaxDist) const;
   float CalcValue(std::vector<DistIndex>& theClosestPointsResultVector) ;
   float CalcDistance(float lon1, float lat1, float lon2, float lat2) const;
   float CalcRelativeDistance(float x1, float y1, float x2, float y2) const;

	
	// halutun gridin alueen relatiiviset rajat esim. 0,0 - 1,1
   NFmiRect itsAreaLimits;
   // Havaintojen x pisteet (ei omisteta)
   std::vector<float> itsXArray;
   std::vector<float> itsYArray;
   std::vector<float> itsZArray;
   // x,y ja z taulokoiden koko.
   int itsArraySize;
   // Jos halutaan rajata l‰himm‰n pisteen hakua, t‰ll‰ voidaan rajata sit‰.
   // Jos arvo on 0, ei rajausta, muuten t‰m‰ on suhteellinen arvo.
   // Joka on suhteessa rajausalueen leveyteen tai korkeuteen (max)
   float itsSearchRange;

};

