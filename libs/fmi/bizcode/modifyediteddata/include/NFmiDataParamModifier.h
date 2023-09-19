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
#include "NFmiGriddingProperties.h"

class NFmiParam;
class NFmiDrawParam;
class NFmiAreaMaskList;
class NFmiTimeDescriptor;
class NFmiFastQueryInfo;
class NFmiMetTime;
class NFmiPoint;
class NFmiThreadCallBacks;
class MultiProcessClientData;

class NFmiDataParamModifier
{
 public:
	NFmiDataParamModifier(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiDrawParam> &theDrawParam, boost::shared_ptr<NFmiAreaMaskList> &theMaskList,
							unsigned long theAreaMask);
	virtual ~NFmiDataParamModifier(void){};
	bool ModifyData (void);
	bool ModifyData2(void); // 1999.11.17/Marko viritetty ympyr‰ muokkauksille
	bool ModifyTimeSeriesData (NFmiTimeDescriptor& theActiveTimes, float* theModifyFactorTable);
	bool ModifyTimeSeriesDataUsingMaskFactors(NFmiTimeDescriptor& theActiveTimes, float* theModifyFactorTable);
	bool SetTimeSeriesData(NFmiTimeDescriptor& theActiveTimes, float* theModifyFactorTable, int theUnchangedValue);	// M.K. 6.5.99
	float Calculate (float theDataValue, float theFactor);
	float CalculateWithMaskFactor (float theDataValue, float theFactor, float theMaskFactor);
	bool Param(const NFmiParam& theParam);

 protected:
	virtual void PrepareFastIsInsideData(void){}; // 1999.11.17/Marko ModifyData2 k‰ytt‰‰
	virtual bool IsPossibleInside(const NFmiPoint& /* theRelativePlace */){return true;}; // 1999.11.17/Marko ModifyData2 k‰ytt‰‰
	virtual float Calculate (const float& theValue);
	virtual float Calculate2(const float& theValue){return theValue;}; // 1999.11.17/Marko ModifyData2 k‰ytt‰‰
	boost::shared_ptr<NFmiFastQueryInfo> itsInfo;
	boost::shared_ptr<NFmiDrawParam> itsDrawParam;
	boost::shared_ptr<NFmiAreaMaskList> itsParamMaskList;

// private:

	unsigned long itsMaskType;
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
	NFmiDataParamControlPointModifier(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiDrawParam> &theDrawParam, boost::shared_ptr<NFmiAreaMaskList> &theMaskList,
													unsigned long theAreaMask, boost::shared_ptr<NFmiEditorControlPointManager> &theCPManager, const NFmiRect &theCPGridCropRect,
													bool theUseGridCrop, const NFmiPoint &theCropMarginSize, const NFmiGriddingProperties &griddingProperties);
	virtual ~NFmiDataParamControlPointModifier(void);
	// HUOM!! eri signerature kuin edell‰!!!
	bool ModifyTimeSeriesDataUsingMaskFactors(NFmiTimeDescriptor& theActiveTimes, NFmiThreadCallBacks *theThreadCallBacks);
    bool DoProcessPoolCpModifyingTcp(MultiProcessClientData &theMultiProcessClientData, NFmiTimeDescriptor& theActiveTimes, const std::string &theGuidStr, NFmiThreadCallBacks *theThreadCallBacks);
	static void DoDataGridding(std::vector<float> &xValues, std::vector<float> &yValues, std::vector<float> &zValues, int arraySize, NFmiDataMatrix<float> &gridData, const NFmiRect &theRelativeRect, const NFmiGriddingProperties &griddingProperties, NFmiObsDataGridding *theObsDataGridding, float theObservationRadiusRelative);
    static size_t GridPointToLocationIndex(size_t gridPointX, size_t gridPointY, size_t gridSizeX);
    int CalcActualModifiedTimes(NFmiTimeDescriptor& theActiveTimes);
    static void FixCroppedMatrixMargins(NFmiDataMatrix<float> &theCroppedGridData, const NFmiPoint &theCropMarginSize);
    static bool GetChangeValues(boost::shared_ptr<NFmiEditorControlPointManager> &theCPManager, std::vector<float> &theXValues, std::vector<float> &theYValues, std::vector<float> &theZValues);

 protected:
	bool IsTimeModified(const NFmiMetTime &theTime);
	bool DoDataGridding(void);
	bool GetChangeValuesWithWork(const NFmiMetTime &theTime, std::vector<float> &theXValues, std::vector<float> &theYValues, std::vector<float> &theZValues);
	bool IsZeroModification(const std::vector<float> &theZValues);
    void DoCroppedGridCalculations(const NFmiDataMatrix<float> &usedData);
    void DoFullGridCalculations(const NFmiDataMatrix<float> &usedData);
    bool PreventGridCropCalculations();
    virtual void DoCroppedPointCalculations(const NFmiDataMatrix<float> &usedData, size_t xIndex, size_t yIndex, float maskFactor);
    virtual void DoNormalPointCalculations(const NFmiDataMatrix<float> &usedData, unsigned long locationIndex, float maskFactor);
    void DoLocationGridCalculations(const NFmiDataMatrix<float> &usedData);
    NFmiDataMatrix<float>& GetUsedGridData();

	NFmiDataMatrix<float> itsGridData;
	NFmiRect itsCPGridCropRect; // jos kontrollipiste muokkaukset halutaan rajoittaa tietyn ali-hilan alueelle, k‰ytet‰‰n t‰t‰ hilapiste-rect:i‰. T‰‰ll‰ on siis bottom-left ja top-right editoidun datan hila-indeksit
	bool fUseGridCrop; // flagi ett‰ k‰ytet‰‰nkˆ croppia vai ei, t‰m‰ tulee siis aikasarjaikkunan s‰‰dˆist‰
	bool fCanGridCropUsed; // flagi ett‰ voidaanko croppia k‰ytt‰‰ vai ei, jos fUseGridCrop on true ja t‰m‰ on false, ei tehd‰ mit‰‰n...
	NFmiPoint itsCropMarginSize;
	NFmiDataMatrix<float> itsCroppedGridData;
	NFmiRect itsGridCropRelativeRect;
    // T‰m‰ on Markon tekem‰ surkea griddaus korvike, jos parempaa systeemi‰ (esim. ToolMasteria) ei lˆydy
	NFmiObsDataGridding* itsObsDataGridding;
	boost::shared_ptr<NFmiEditorControlPointManager> itsCPManager;
	int itsLastTimeIndex; // optimointia varten
    NFmiGriddingProperties itsGriddingProperties;
};


