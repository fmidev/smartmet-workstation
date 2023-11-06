//**********************************************************
// C++ Class Name : NFmiDataParamModifierAreaConeChange 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiDataParamModifierAreaConeChange.h 
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

#include "NFmiDataParamModifier.h"
#include "NFmiRect.h"
#include "NFmiMetEditorTypes.h"

class NFmiDrawParam;
class NFmiAreaMaskList;
class NFmiPoint;
class NFmiArea;

class NFmiDataParamModifierAreaConeChange : public NFmiDataParamModifier
{
 public:
    NFmiDataParamModifierAreaConeChange (boost::shared_ptr<NFmiFastQueryInfo> theInfo
										,boost::shared_ptr<NFmiDrawParam> &theDrawParam
										,boost::shared_ptr<NFmiAreaMaskList> &theMaskList
										,const boost::shared_ptr<NFmiArea> &theZoomedArea
										,const boost::shared_ptr<NFmiArea> &theOriginalArea
										,const NFmiPoint& theLatLon
										,double theRadius, double theFactor
										,NFmiMetEditorTypes::Mask theAreaMask
										,int theLimitingOption
										,float theLimitingValue);
   virtual ~NFmiDataParamModifierAreaConeChange (void);

 protected:
	double FixCircularValues(double theValue);
	void PrepareFastIsInsideData(void) override; // 1999.11.17/Marko ModifyData2 k‰ytt‰‰
	bool IsPossibleInside(const NFmiPoint& theRelativePlace) override; // 1999.11.17/Marko ModifyData2 k‰ytt‰‰
	double CalcDistance(void);
	float Calculate (const float& theValue) override;
	float Calculate2(const float& theValue) override; // 1999.11.17/Marko ModifyData2 k‰ytt‰‰
	NFmiPoint itsModifyingCenter;
	NFmiPoint itsModifyingCenterInXY;
	double itsModificationRange;
	double itsModifyFactor;
	boost::shared_ptr<NFmiArea> itsZoomedArea;
	boost::shared_ptr<NFmiArea> itsOriginalArea;
	NFmiRect itsRelativeCheckRect; // k‰ytet‰‰n pikatarkastukseen onko currentti paikka mahdollisesti halutun ympyr‰n sis‰ll‰
	int itsLimitingOption; // 0=ei rajoitusta, 1=ei alle, 2=ei yli ja 3=abs. arvon asetus
	double itsLimitingValue; // yll‰ olevasta asetuksista riippuen t‰m‰ voi olla muokkausta rajoittava raja arvo

	// tuulen suuntaa varten pit‰‰ tehd‰ virityksi‰, ett‰ esim. 350 + 20 olisi 10 eik‰ 360 (eli maksimi) jne.
	bool fCircularValue;
	double itsCircularValueModulor;
};

class NFmiDataParamModifierAreaCircleSetValue : public NFmiDataParamModifierAreaConeChange
{
public:
	NFmiDataParamModifierAreaCircleSetValue(boost::shared_ptr<NFmiFastQueryInfo> theInfo
		, boost::shared_ptr<NFmiDrawParam>& theDrawParam
		, boost::shared_ptr<NFmiAreaMaskList>& theMaskList
		, boost::shared_ptr<NFmiArea>& theArea
		, const NFmiPoint& theLatLon
		, double theRadius, double theValue
		, NFmiMetEditorTypes::Mask theAreaMask = NFmiMetEditorTypes::kFmiNoMask);
	virtual ~NFmiDataParamModifierAreaCircleSetValue(void) {};

protected:
	float Calculate(const float& theValue) override;
	double itsModifyValue;
};

