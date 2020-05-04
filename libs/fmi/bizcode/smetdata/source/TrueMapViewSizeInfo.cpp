#include "StdAfx.h"
#include "TrueMapViewSizeInfo.h"
#include "CtrlViewFunctions.h"
#include "catlog/catlog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


TrueMapViewSizeInfo::TrueMapViewSizeInfo(int mapViewDescTopIndex)
	:mapViewDescTopIndex_(mapViewDescTopIndex)
{
}

TrueMapViewSizeInfo::TrueMapViewSizeInfo(const TrueMapViewSizeInfo&) = default;

TrueMapViewSizeInfo& TrueMapViewSizeInfo::operator=(const TrueMapViewSizeInfo&) = default;


void TrueMapViewSizeInfo::onSize(const NFmiPoint& clientPixelSize, CDC* pDC, const NFmiPoint& viewGridSize, bool isTimeControlViewVisible)
{
	clientAreaSizeInPixels_ = clientPixelSize;
	calculateViewSizeInfo(pDC, viewGridSize, isTimeControlViewVisible);
	if(CatLog::doTraceLevelLogging())
	{
		std::string message = __FUNCTION__;
		message += ": " + std::to_string(clientPixelSize.X()) + "x" + std::to_string(clientPixelSize.Y());
		CatLog::logMessage(message, CatLog::Severity::Trace, CatLog::Category::Operational);
	}
}

void TrueMapViewSizeInfo::onViewGridSizeChange(const NFmiPoint& viewGridSize, bool isTimeControlViewVisible)
{
	updateMapSizes(viewGridSize, isTimeControlViewVisible);
	if(CatLog::doTraceLevelLogging())
	{
		std::string message = __FUNCTION__;
		message += ": " + std::to_string(viewGridSize.X()) + "x" + std::to_string(viewGridSize.Y());
		CatLog::logMessage(message, CatLog::Severity::Trace, CatLog::Category::Operational);
	}
}

void TrueMapViewSizeInfo::calculateViewSizeInfo(CDC* pDC, const NFmiPoint& viewGridSize, bool isTimeControlViewVisible)
{
	// P‰ivitet‰‰n n‰m‰kin perusarvot joka, jos joku on mennyt s‰‰t‰m‰‰n vaikka monitorin asetuksia, 
	// tai ollaan vaikka eri monitorilla.
	monitorSizeInMilliMeters_.X(GetDeviceCaps(pDC->GetSafeHdc(), HORZSIZE));
	monitorSizeInMilliMeters_.Y(GetDeviceCaps(pDC->GetSafeHdc(), VERTSIZE));
	monitorSizeInPixels_.X(GetDeviceCaps(pDC->GetSafeHdc(), HORZRES));
	monitorSizeInPixels_.Y(GetDeviceCaps(pDC->GetSafeHdc(), VERTRES));
	updatePixelsPerMilliMeterValues();
	updateMapSizes(viewGridSize, isTimeControlViewVisible);
}

void TrueMapViewSizeInfo::updateMapSizes(const NFmiPoint &viewGridSize, bool isTimeControlViewVisible)
{
	// Mik‰ on aikakontrolli-ikkunan korkeus pikseleiss‰?
	auto timeControlViewHeightInPixels = isTimeControlViewVisible ? calculateTimeControlViewHeightInPixels(pixelsPerMilliMeter_.X()) : 0;

	// Laske koko karttaosion koko
	totalMapSectionSizeInPixels_ = NFmiPoint(clientAreaSizeInPixels_.X(), clientAreaSizeInPixels_.Y() - timeControlViewHeightInPixels);
	// Laske yhden karttaruudun koko
	singleMapSizeInPixels_ = NFmiPoint(totalMapSectionSizeInPixels_.X() / viewGridSize.X(), totalMapSectionSizeInPixels_.Y() / viewGridSize.Y());
	// Laske yhden karttaruudun koko millimetreiss‰
	singleMapSizeInMM_ = NFmiPoint(singleMapSizeInPixels_.X() / pixelsPerMilliMeter_.X(), singleMapSizeInPixels_.Y() / pixelsPerMilliMeter_.Y());
}


void TrueMapViewSizeInfo::updatePixelsPerMilliMeterValues()
{
	pixelsPerMilliMeter_.X(monitorSizeInPixels_.X() / monitorSizeInMilliMeters_.X());
	pixelsPerMilliMeter_.Y(monitorSizeInPixels_.Y() / monitorSizeInMilliMeters_.Y());
}

double TrueMapViewSizeInfo::calculateTimeControlViewHeightInPixels(double thePixelsPerMilliMeterX)
{
	NFmiPoint fontSizeInPixels = CtrlViewUtils::CalcTimeScaleFontSizeInPixels(thePixelsPerMilliMeterX);
	// Kerrotaan noin tekstirivien lukum‰‰r‰ll‰
	return fontSizeInPixels.X() * 3.5;
}
