//**********************************************************
// C++ Class Name : NFmiAreaView 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiAreaView.h 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 2.1.4 ) 
//  - GD System Name    : Met-editor Plan 2 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : Markon ehdotus 
// ---------------------------------------------------  
//  Author         : Persa 
//  Creation Date  : 28.04.1997 
// 
// 
//  Description: 
// 
//  Change Log: 
//Ver. 28.04.1997/Persa 
//Ver.  7.05.1997/Persa 
// 
//**********************************************************

#pragma once

#include "NFmiCtrlView.h"

class NFmiArea;

class NFmiAreaView : public NFmiCtrlView
{

 public:
	NFmiAreaView (int theMapViewDescTopIndex, const boost::shared_ptr<NFmiArea> &theArea 
				 , NFmiToolBox* theToolBox
				 , boost::shared_ptr<NFmiDrawParam> &theDrawParam
                 , int viewGridRowNumber
                 , int viewGridColumnNumber);

	NFmiAreaView (const NFmiAreaView & theView);
	NFmiAreaView ();
	~NFmiAreaView ();
	boost::shared_ptr<NFmiArea> GetArea () const override; 
	void SetArea(const boost::shared_ptr<NFmiArea> &theArea) override;

 protected:
	boost::shared_ptr<NFmiArea> itsArea;

};

