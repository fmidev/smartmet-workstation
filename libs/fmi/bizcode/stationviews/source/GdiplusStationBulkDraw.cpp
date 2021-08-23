#include "GdiplusStationBulkDraw.h"
#include "NFmiSymbolBulkDrawData.h"
#include "NFmiImageMap.h"
#include "NFmiStationIndexTextView.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "catlog/catlog.h"

namespace
{
	// Oletus: gdiPlusGraphics, usedSymbolMap, toolBox ovat tarkastettuja ja käyttövalmiina.
	// Käydään loopissa piirtämässä kaikki annettujen pisteiden arvojen mukaiset symbolit.
	void DoFinalImageBulkDraw(const NFmiSymbolBulkDrawData& sbdData, Gdiplus::Graphics& gdiPlusGraphics, NFmiImageMap& usedSymbolMap, NFmiToolBox& toolBox)
	{
		auto printing = sbdData.printing();
		auto symbolSize = sbdData.symbolSizes().front();
		auto symbolSizeInPixels = symbolSize.Y();
		NFmiRect symbolRect;
		symbolRect.Size(sbdData.relativeDrawObjectSize());
		for(size_t index = 0; index < sbdData.values().size(); index++)
		{
			auto value = sbdData.values()[index];
			std::string codeStr = boost::lexical_cast<std::string>(value);
			Gdiplus::Bitmap* symbolBitmap = usedSymbolMap.GetRightSizeImage(symbolSizeInPixels, printing, codeStr);
			symbolRect.Center(sbdData.relativeStationPointPositions()[index]);
			CtrlView::DrawAnimationButton(symbolRect, symbolBitmap, &gdiPlusGraphics, toolBox, printing, sbdData.mapViewSizeInPixels(), 1.f, true);
		}
	}

	void DoFinalImageDraw(const NFmiSymbolBulkDrawData& sbdData, Gdiplus::Graphics& gdiPlusGraphics, NFmiImageMap* usedSymbolMap, NFmiToolBox& toolBox)
	{
		if(usedSymbolMap == nullptr)
		{
			std::string errorMessage = "Error in ";
			errorMessage += __FUNCTION__;
			errorMessage += " : Given usedSymbolMap was nullptr, nothing to be done...";
			CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Visualization, true);
		}
		else
		{
			::DoFinalImageBulkDraw(sbdData, gdiPlusGraphics, *usedSymbolMap, toolBox);
		}
	}

	void DoFinalDraw(const NFmiSymbolBulkDrawData& sbdData, Gdiplus::Graphics& gdiPlusGraphics, NFmiToolBox& toolBox)
	{
		NFmiImageMap* usedSymbolMap = nullptr;
		switch(sbdData.drawType())
		{
		case NFmiSymbolBulkDrawType::BitmapSymbol1:
		{
			usedSymbolMap = &NFmiBetterWeatherSymbolView::GetBetterWeatherSymbolMap();
			break;
		}
		case NFmiSymbolBulkDrawType::BitmapSymbol2:
		{
			usedSymbolMap = &NFmiSmartSymbolView::GetSmartSymbolMap();
			break;
		}
		case NFmiSymbolBulkDrawType::BitmapSymbol3:
		{
			usedSymbolMap = &NFmiCustomSymbolView::GetCustomSymbolMap();
			break;
		}
		default:
		{
			std::string errorMessage = "Error in ";
			errorMessage += __FUNCTION__;
			errorMessage += " : Given NFmiSymbolBulkDrawData had unknown symbol-draw-type, nothing to be done...";
			CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Visualization, true);
			return;
		}
		}

		::DoFinalImageDraw(sbdData, gdiPlusGraphics, usedSymbolMap, toolBox);
	}
}

namespace GdiplusStationBulkDraw
{
	void Draw(const NFmiSymbolBulkDrawData& sbdData, Gdiplus::Graphics* gdiPlusGraphics, NFmiToolBox& toolBox)
	{
		if(gdiPlusGraphics == nullptr)
		{
			std::string errorMessage = "Error in ";
			errorMessage += __FUNCTION__;
			errorMessage += " : Given gdiPlusGraphics was nullptr, nothing to be done...";
			CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Visualization, true);
		}
		else
		{
			::DoFinalDraw(sbdData, *gdiPlusGraphics, toolBox);
		}
	}
}
