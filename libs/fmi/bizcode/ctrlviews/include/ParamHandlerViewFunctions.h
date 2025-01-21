#pragma once

#include "stdafx.h"
#include "NFmiParamHandlerView.h"

// Apufunktioita jotka ovat riippuvaisia win32/mfc jutuista
// ja toolboxista ja muusta kivasta.
namespace CtrlView
{
template<typename UsedCtrlview>
static void GeneralInitParamHandlerView(UsedCtrlview* parentView, std::unique_ptr<NFmiParamHandlerView>& paramHandlerView, bool showMaskSection, bool showMapLayer)
{
	auto initialFrame = parentView->CalcParamHandlerViewRect();
	parentView->SetParamHandlerViewRect(initialFrame);

	paramHandlerView.reset(new NFmiParamHandlerView(
		parentView->MapViewDescTopIndex()
		, initialFrame
		, parentView->GetToolBox()
		, parentView->DrawParam()
		, parentView->ViewGridRowNumber()
		, parentView->ViewGridColumnNumber()
		, showMaskSection
		, showMapLayer
	));
	paramHandlerView->Init();
}

template<typename UsedCtrlview>
static void GeneralUpdateParamHandlerView(UsedCtrlview* parentView, std::unique_ptr<NFmiParamHandlerView>& paramHandlerView)
{
	if(paramHandlerView)
	{
		auto initialFrame = parentView->CalcParamHandlerViewRect();
		parentView->SetParamHandlerViewRect(initialFrame);

		paramHandlerView->Update(
			initialFrame
			, parentView->GetToolBox()
			, parentView->GetCtrlViewDocumentInterface()->ParamWindowViewPosition(parentView->MapViewDescTopIndex())
		);
		// NFmiParamHandlerView::Update metodi ottaa täällä esilasketun itsParamHandlerViewRect:in ja tekee siihen muutoksia, 
		// ja se pitää ottaa taas talteen tässä.
		parentView->SetParamHandlerViewRect(paramHandlerView->GetFrame());
	}
}

template<typename UsedCtrlview>
static NFmiRect GeneralCalcParamHandlerViewRect(UsedCtrlview* parentView, const NFmiRect &usedFrame)
{
	double widthInMM = 50.;
	auto* ctrlViewDocumentInterface = parentView->GetCtrlViewDocumentInterface();
	auto mapViewDescTopIndex = parentView->MapViewDescTopIndex();
	int pixelSize = static_cast<int>(widthInMM * ctrlViewDocumentInterface->GetGraphicalInfo(mapViewDescTopIndex).itsPixelsPerMM_x);
	double width = parentView->GetToolBox()->SX(pixelSize);
	NFmiRect newRect(usedFrame);
	newRect.Width(width);
	newRect.Height(0.05);
	newRect = NFmiCtrlView::CalcWantedDirectionalPosition(usedFrame, newRect, ctrlViewDocumentInterface->ParamWindowViewPosition(mapViewDescTopIndex));
	return newRect;
}

}
