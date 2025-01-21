//**********************************************************
// C++ Class Name : NFmiMaskParamsView 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiMaskParamsView.cpp 
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
//  Author         : pietarin 
//  Creation Date  : Thur - Jan 28, 1999 
// 
// 
//  Description: 
// 
//  Change Log: 
// 
//**********************************************************
#include "NFmiMaskParamsView.h"
#include "NFmiString.h"
#include "NFmiToolBox.h"
#include "NFmiAreaMaskList.h"
#include "NFmiAreaMask.h"
#include "NFmiText.h"
#include "CtrlViewDocumentInterface.h"
#include "ToolBoxStateRestorer.h"

//--------------------------------------------------------
// Constructor/Destructor 
//--------------------------------------------------------
NFmiMaskParamsView::NFmiMaskParamsView(int theMapViewDescTopIndex, const NFmiRect & theRect, NFmiToolBox * theToolBox, boost::shared_ptr<NFmiDrawParam> &theDrawParam, int theRowIndex, int theColumnIndex)
:NFmiParamCommandView(theMapViewDescTopIndex, theRect, theToolBox, theDrawParam, theRowIndex, theColumnIndex, false)
{
}

//--------------------------------------------------------
// Draw 
//--------------------------------------------------------
void NFmiMaskParamsView::DrawData(void)
{
    ToolBoxStateRestorer toolBoxStateRestorer(*itsToolBox, itsToolBox->GetTextAlignment(), true, &GetFrame());

    boost::shared_ptr<NFmiAreaMaskList> maskList = itsCtrlViewDocumentInterface->ParamMaskListMT();
    if(maskList)
    {
        itsDrawingEnvironment.SetFontSize(itsFontSize);
        itsDrawingEnvironment.SetFontType(kArial);
        int zeroBasedRowIndex = 0;
		for(maskList->Reset(); maskList->Next(); zeroBasedRowIndex++)
        {
            auto parameterRowRect = CalcParameterRowRect(zeroBasedRowIndex);
            boost::shared_ptr<NFmiAreaMask> mask = maskList->Current();
            NFmiInfoData::Type dataType = mask->GetDataType();
            if(dataType == NFmiInfoData::kViewable || dataType == NFmiInfoData::kStationary)
                itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f, 0.5f, 0.f));
            else
                itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f, 0.f, 0.f));
            NFmiString str = mask->MaskString();
            NFmiText text(LineTextPlace(zeroBasedRowIndex, parameterRowRect, true), str, true, 0, &itsDrawingEnvironment);
            itsToolBox->Convert(&text);
            DrawCheckBox(parameterRowRect, mask->IsEnabled());
        }
    }
}

//--------------------------------------------------------
// RightButtonDown
//--------------------------------------------------------
bool NFmiMaskParamsView::RightButtonUp(const NFmiPoint& thePlace, unsigned long theKey)
{
	if(IsIn(thePlace))
	{
		int index = CalcParameterRowIndex(thePlace);
		return itsCtrlViewDocumentInterface->CreateMaskParamsPopup(itsViewGridRowNumber, index) == true;
	}
	return false;
}

bool NFmiMaskParamsView::LeftButtonUp(const NFmiPoint& thePlace, unsigned long theKey)
{
	if(IsIn(thePlace))
	{
        int index = CalcParameterRowIndex(thePlace);
		boost::shared_ptr<NFmiAreaMaskList> maskList = itsCtrlViewDocumentInterface->ParamMaskListMT();
		if(maskList->Index(index))
		{
			boost::shared_ptr<NFmiAreaMask> mask = maskList->Current();
			if(mask)
			{
				mask->Enable(!mask->IsEnabled());
				itsCtrlViewDocumentInterface->MapViewDirty(itsMapViewDescTopIndex, false, true, true, false, false, false);
				return true;
			}
		}
	}
	return false;
}

//--------------------------------------------------------
// CalcSize
//--------------------------------------------------------
NFmiRect NFmiMaskParamsView::CalcSize(void)
{
    NFmiRect returnRect(GetFrame());
    int lineCount = 1; // minimi
    boost::shared_ptr<NFmiAreaMaskList> maskList = itsCtrlViewDocumentInterface->ParamMaskListMT();
    if(maskList && maskList->NumberOfItems())
        lineCount = maskList->NumberOfItems();

    // ruudun korkeus on rivien m‰‰r‰*rivinkorkeus + viidesosa rivin korkeudesta (v‰h‰n tilaa pohjalle)
    double heigth = lineCount * itsLineHeight + 0.5 * itsLineHeight;
    returnRect.Bottom(returnRect.Top() + heigth);
    return returnRect;
}
