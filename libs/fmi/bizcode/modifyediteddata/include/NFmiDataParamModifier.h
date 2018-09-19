//**********************************************************
// C++ Class Name : NFmiDataParamModifier
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiDataParamModifier.h
//
//
// GDPro Properties
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class
//  - GD Method         : UML ( 2.1.4 )
//  - GD System Name    : Met-editor Plan 2
//  - GD View Type      : Class Diagram
//  - GD View Name      : dataparam modification and masks
// ---------------------------------------------------
//  Author         : pietarin
//  Creation Date  : Fri - Feb 26, 1999
//
//
//  Description:
//
//  Change Log:
//
//**********************************************************

#pragma once

#include "NFmiRect.h"
#include "NFmiDataMatrix.h"
#include "boost/shared_ptr.hpp"
#include "TimeSerialModification.h"

class NFmiParam;
class NFmiDrawParam;
class NFmiAreaMaskList;
class NFmiTimeDescriptor;
class NFmiFastQueryInfo;
class NFmiMetTime;
class NFmiPoint;
class NFmiThreadCallBacks;

class NFmiDataParamModifier
{

 public:
	NFmiDataParamModifier(boost::shared_ptr<NFmiFastQueryInfo> theInfo, boost::shared_ptr<NFmiDrawParam> &theDrawParam, boost::shared_ptr<NFmiAreaMaskList> &theMaskList,
							unsigned long theAreaMask, const NFmiRect& theSelectedSearchAreaRect);
	virtual ~NFmiDataParamModifier(void){};
	bool ModifyData (void);
	bool ModifyData2(void); // 1999.11.17/Marko viritetty ympyr‰ muokkauksille
	bool ModifyTimeSeriesData (NFmiTimeDescriptor& theActiveTimes, double* theModifyFactorTable);
	bool ModifyTimeSeriesDataUsingMaskFactors(NFmiTimeDescriptor& theActiveTimes, double* theModifyFactorTable);
	bool SetTimeSeriesData(NFmiTimeDescriptor& theActiveTimes, double* theModifyFactorTable, int theUnchangedValue);	// M.K. 6.5.99
	double Calculate (double theDataValue, double theFactor);
	double CalculateWithMaskFactor (double theDataValue, double theFactor, double theMaskFactor);
	bool Param(const NFmiParam& theParam);

 protected:
	virtual void PrepareFastIsInsideData(void){}; // 1999.11.17/Marko ModifyData2 k‰ytt‰‰
	virtual bool IsPossibleInside(const NFmiPoint& /* theRelativePlace */){return true;}; // 1999.11.17/Marko ModifyData2 k‰ytt‰‰
	virtual double Calculate (const double& theValue);
	virtual double Calculate2(const double& theValue){return theValue;}; // 1999.11.17/Marko ModifyData2 k‰ytt‰‰
	boost::shared_ptr<NFmiFastQueryInfo> itsInfo;
	boost::shared_ptr<NFmiDrawParam> itsDrawParam;
	boost::shared_ptr<NFmiAreaMaskList> itsParamMaskList;

// private:

	unsigned long itsMaskType;
	NFmiRect itsSelectedSearchAreaRect; // alue valinta pehmennys tehd‰‰n t‰m‰n avulla. Kun editoidaan aÌkasarjalla
										// dataa, haetaan joka pisteeseen l‰hialueiden valitut pisteet. Riippuen paljon
										// ymp‰rill‰ olevissa pisteiss‰ on valittuja pisteit‰, sit‰ voimakkaampi
										// muokkaus on. V‰h‰n niin kuin maskien kanssa. Keskell‰ valinta aluetta muutos
										// on voimakas ja reunoilla heikompi.

	bool SyncronizeTimeWithMasks (void);
};

// ****************************************************************************************************
// ****************************************************************************************************
// ****************************************************************************************************
// ****************************************************************************************************
// ****************************************************************************************************
// ****************************************************************************************************

class NFmiEditorControlPointManager;
class NFmiObsDataGridding;

class NFmiDataParamControlPointModifier : public NFmiDataParamModifier
{
 public:
	NFmiDataParamControlPointModifier(boost::shared_ptr<NFmiFastQueryInfo> theInfo, boost::shared_ptr<NFmiDrawParam> &theDrawParam, boost::shared_ptr<NFmiAreaMaskList> &theMaskList,
													unsigned long theAreaMask, boost::shared_ptr<NFmiEditorControlPointManager> theCPManager, const NFmiRect &theCPGridCropRect,
													bool theUseGridCrop, const NFmiPoint &theCropMarginSize);
	virtual ~NFmiDataParamControlPointModifier(void);
	// HUOM!! eri signerature kuin edell‰!!!
	bool ModifyTimeSeriesDataUsingMaskFactors(NFmiTimeDescriptor& theActiveTimes, NFmiThreadCallBacks *theThreadCallBacks);
    bool DoProcessPoolCpModifyingTcp(MultiProcessClientData &theMultiProcessClientData, NFmiTimeDescriptor& theActiveTimes, const std::string &theGuidStr, NFmiThreadCallBacks *theThreadCallBacks);
	static void DoDataGridding(std::vector<float> &xValues, std::vector<float> &yValues, std::vector<float> &zValues, int arraySize, NFmiDataMatrix<float> &gridData, const NFmiRect &theRelativeRect, int theGriddingFunction, NFmiObsDataGridding *theObsDataGridding, float theObservationRadiusRelative);
	int CalcActualModifiedTimes(NFmiTimeDescriptor& theActiveTimes);

 protected:
	bool IsTimeModified(const NFmiMetTime &theTime);
	bool DoDataGridding(void);
	bool GetChangeValues(std::vector<float> &theXValues, std::vector<float> &theYValues, std::vector<float> &theZValues);
	bool GetChangeValuesWithWork(const NFmiMetTime &theTime, std::vector<float> &theXValues, std::vector<float> &theYValues, std::vector<float> &theZValues);
	bool IsZeroModification(const std::vector<float> &theZValues);
    void DoCroppedGridCalculations();
    void DoFullGridCalculations();

	NFmiDataMatrix<float> itsGridData;
	NFmiRect itsCPGridCropRect; // jos kontrollipiste muokkaukset halutaan rajoittaa tietyn ali-hilan alueelle, k‰ytet‰‰n t‰t‰ hilapiste-rect:i‰. T‰‰ll‰ on siis bottom-left ja top-right editoidun datan hila-indeksit
	bool fUseGridCrop; // flagi ett‰ k‰ytet‰‰nkˆ croppia vai ei, t‰m‰ tulee siis aikasarjaikkunan s‰‰dˆist‰
	bool fCanGridCropUsed; // flagi ett‰ voidaanko croppia k‰ytt‰‰ vai ei, jos fUseGridCrop on true ja t‰m‰ on false, ei tehd‰ mit‰‰n...
	NFmiPoint itsCropMarginSize;
	NFmiDataMatrix<float> itsCroppedGridData;
	NFmiRect itsGridCropRelativeRect;
	NFmiObsDataGridding* itsObsDataGridding; // omistaa
	boost::shared_ptr<NFmiEditorControlPointManager> itsCPManager;
	int itsLastTimeIndex; // optimointia varten
};


