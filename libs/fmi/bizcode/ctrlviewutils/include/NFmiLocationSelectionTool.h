//**********************************************************
// C++ Class Name : NFmiLocationSelectionTool 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: d:/projekti/GDPro/GDTemp/NFmiLocationSelectionTool.h 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 2.1.4 ) 
//  - GD System Name    : Met-editor Plan 3 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : selection tool -luokka 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Thur - Jun 10, 1999 
// 
// 
//  Description: 
//   Luokkaa k�ytet��n valitesemaan (maskaamaan) 
//   haluttuja paikkoja annetusta NFmiDrawParam-luokan NFmiSmartInfo-luokasta. 
//   Valintaa teht�ess� annetaan luokalle latitude-longitude 
//   piste, mista valinnat alkavat. Se miten valinta 
//   tehd��n, riippuu valitusta ty�kalusta. Jos 
//   ty�kalu on single-selection, valituksi tulee 
//   asema/hila, joka on l�hinn� annettua pistett�. 
//   Jos ty�kalu oli circle-selection, valituksi 
//   tulee asemat/hilat tietyn ympyr�n sis�ll�
//   Muuten luokka valitsee pisteet luokkansa perityill� maskaus ominaisuuksilla.
// 
//  Change Log: 
// 
//**********************************************************

#pragma once

#include "NFmiInfoAreaMask.h"

class NFmiFastQueryInfo;

typedef enum
{
	kFmiSingleLocationSelection = 0, // valitaan asema/hila kerrallaa
	kFmiCircleLocationSelection = 1, // valitaan asemia/hiloja tietyn ympyr�n sis�lt�
	kFmiParamMaskLocationSelection = 2 // valitaan asemia/hiloja k�ytt�en parametri maskia
} FmiLocationSearchTool;

typedef enum
{
	kFmiSelectionCombineClearFirst = 0, // nollataan ensin vanha valinta
	kFmiSelectionCombineAdd = 1, // lis�t��n uudet valinnat vanhojen kanssa
	kFmiSelectionCombineRemove = 2, // poistaa vanhoista valinnoista ne paikat
								    //, mitk� ovat uusien joukossa
								    // uusia ei tule valituiksi ollenkaan
	kFmiSelectionCombineSection = 3 // valitaan ne paikat, mitk� ovat sek� vanhojen, ett� uusien valintojen joukossa
} FmiSelectionCombineFunction;

class NFmiArea;

class NFmiLocationSelectionTool : public NFmiInfoAreaMask
{

 public:
   ~NFmiLocationSelectionTool(void){}
   NFmiLocationSelectionTool(void);
   bool SelectLocations(boost::shared_ptr<NFmiFastQueryInfo> &theInfo
							 ,const NFmiPoint& theLatLon
							 ,const boost::shared_ptr<NFmiArea> &theArea
							 ,FmiSelectionCombineFunction theFunction
							 ,unsigned long theMask
							 ,const NFmiPoint& theViewGridSize);
   inline FmiLocationSearchTool SelectedTool(void){return itsSelectedTool;};
   inline void SelectedTool(FmiLocationSearchTool theTool){itsSelectedTool = theTool;};
   inline float SearchRange(void){return itsSearchRange;};
   inline void SearchRange(float newValue){itsSearchRange = newValue;};
   inline bool UseValueFromLocation(void){return fUseValueFromLocation;};
   inline void UseValueFromLocation(bool newValue){fUseValueFromLocation = newValue;};
   inline bool SearchOnlyNeibhors(void){return fSearchOnlyNeibhors;};
   inline void SearchOnlyNeibhors(bool newValue){fSearchOnlyNeibhors = newValue;};
   inline bool LimitSearchWithRange(void){return fLimitSearchWithRange;};
   inline void LimitSearchWithRange(bool newValue){fLimitSearchWithRange = newValue;};

 private:
   void ClearLocationSelection(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, unsigned long theMask);
   void SingleSelection(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiPoint& theLatLon, const boost::shared_ptr<NFmiArea> &theArea
					   ,FmiSelectionCombineFunction theFunction, unsigned long theMask);
   void CircleSelection(boost::shared_ptr<NFmiFastQueryInfo> &theInfo
					   ,const NFmiPoint& theLatLon
					   ,const boost::shared_ptr<NFmiArea> &theArea
	                   ,FmiSelectionCombineFunction theFunction
					   ,unsigned long theMask
					   ,const NFmiPoint& theViewGridSize);
   void ParamSelection(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiPoint& theLatLon
					  ,const boost::shared_ptr<NFmiArea> &theArea, FmiSelectionCombineFunction theFunction
					  ,unsigned long theMask);

   FmiLocationSearchTool itsSelectedTool;
   float itsSearchRange; // suhteellinen arvo ikkunaan suhteutettuna, arvot v�lill� 0-1 (=0-100%)
   bool fUseValueFromLocation;
   bool fSearchOnlyNeibhors;
   bool fLimitSearchWithRange;

};

