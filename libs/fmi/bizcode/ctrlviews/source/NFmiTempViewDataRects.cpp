#include "NFmiTempViewDataRects.h"
#include "CtrlViewDocumentInterface.h"
#include "NFmiMTATempSystem.h"
#include "NFmiToolBox.h"
#include "NFmiText.h"
#include "catlog/catlog.h"

namespace
{
	int calculateSideViewFontSizeInPixels(bool showSideView, int defaultFontSize, NFmiToolBox *usedToolBox)
	{
		int fontSizeInPixels = 0;
		if(showSideView)
		{
			fontSizeInPixels = defaultFontSize;
			if(usedToolBox->GetDC()->IsPrinting())
			{
				// tehd�� printtausta varten v�h�n isommat fontit indeksi ikkunaan
				fontSizeInPixels = boost::math::iround(fontSizeInPixels * 1.15);
			}
		}
		return fontSizeInPixels;
	}

	bool addMarginOnSecondaryDataRect(const NFmiRect& textualSoundingDataSideViewRect, const NFmiRect& stabilityIndexSideViewRect)
	{
		if(!textualSoundingDataSideViewRect.IsEmpty())
			return true;
		else if(!stabilityIndexSideViewRect.IsEmpty())
			return true;

		return false;
	}

	bool addMarginOnSoundingCurveDataRect(const NFmiRect& secondaryDataFrame, const NFmiRect& textualSoundingDataSideViewRect, const NFmiRect& stabilityIndexSideViewRect)
	{
		if(!secondaryDataFrame.IsEmpty())
			return true;
		else
			return addMarginOnSecondaryDataRect(textualSoundingDataSideViewRect, stabilityIndexSideViewRect);
	}

}

NFmiTempViewDataRects::NFmiTempViewDataRects()
	:ctrlViewDocumentInterface_(CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation())
{
}

NFmiTempViewDataRects::~NFmiTempViewDataRects() = default;

double NFmiTempViewDataRects::calcPressureScaleWidth()
{
	double leftMargin = usedToolBox_->SX(boost::math::iround(42 * drawSizeFactor_.X()));
	return leftMargin;
}

// 0. Laske totalDataRect_, joka on luotausn�yt�n kokonaispiirtoalueesta j��v� se alue kun 
//    vertikaali- ja horisontaaliasteikkojen alueet ja muut marginaalit on poistettu siit�.
NFmiRect NFmiTempViewDataRects::calcTotalDataRect()
{
	double leftMargin = calcPressureScaleWidth();
	double topMargin = usedToolBox_->SY(boost::math::iround(10 * drawSizeFactor_.Y()));
	double bottomMargin = usedToolBox_->SY(boost::math::iround(20 * drawSizeFactor_.Y()));
	NFmiRect finalRect(totalSoundingViewRect_.Left() + leftMargin,
		totalSoundingViewRect_.Top() + topMargin,
		totalSoundingViewRect_.Right() - baseMarginRelativeWidth_,
		totalSoundingViewRect_.Bottom() - bottomMargin);
	return finalRect;
}

//  StabilityIndex laskuun on my�s laitettu s��t� ett� n�ytet��nk� indeksi ikkunaa ollenkaan vai ei
// eli jos ei, laiteaan fontti kooksi 0.
void NFmiTempViewDataRects::calculateStabilityIndexFontSizeInPixels()
{
	NFmiMTATempSystem& mtaTempSystem = ctrlViewDocumentInterface_->GetMTATempSystem();
	stabilityIndexFontSize_ = ::calculateSideViewFontSizeInPixels(
		mtaTempSystem.GetSoundingViewSettingsFromWindowsRegisty().ShowStabilityIndexSideView(), 
		mtaTempSystem.IndexiesFontSize(), 
		usedToolBox_);
}

void NFmiTempViewDataRects::calculatetextualSoundingDataFontSizeInPixels()
{
	NFmiMTATempSystem& mtaTempSystem = ctrlViewDocumentInterface_->GetMTATempSystem();
	textualSoundingDataFontSize_ = ::calculateSideViewFontSizeInPixels(
		mtaTempSystem.GetSoundingViewSettingsFromWindowsRegisty().ShowTextualSoundingDataSideView(),
		mtaTempSystem.SoundingTextFontSize(),
		usedToolBox_);
}

double NFmiTempViewDataRects::calculateLeftRelativeEdgeOfSideView(double rightEdge, bool showSideView, int sideViewTextWidthCount, int fontSizeInPixels)
{
	// Jos kyseinen osio on kiinni, annetaan leftEdge:lle sama arvo kuin rightEdge:lle, 
	// t�ll�in sivuikkunan alueesta tehd��n t�st� oikeassa horisontaalikohdassa oleva pystyviiva eli tyhj� laatikko.
	double leftEdge = rightEdge;
	if(showSideView)
	{
		// Jos kyseinen osio on auki, lasketaan kuinka leve� sivuikkuna on ja lasketaan leftEdge sen mukaan.
		double fontWidthFactor = (fontSizeInPixels < 18) ? 0.5 : 0.55; // fontti koko 18 ja sen j�lkeen on erikoinen, joten laiton sille eri kertoimen
		long sideViewWidthInPixels = boost::math::iround(fontSizeInPixels * sideViewTextWidthCount * fontWidthFactor); // teksti n�ytt�jen leveys pit�� my�s laskea ett� osataan j�tt�� tilaa niille
		double relativeSideViewWidth = usedToolBox_->SX(boost::math::iround(sideViewWidthInPixels * drawSizeFactor_.X())); // t�h�n vaaditaan tilaa stabiilisuus indeksi n�yt�lle
		leftEdge = rightEdge - relativeSideViewWidth;
	}
	return leftEdge;
}

const std::string g_maximumSampleStringForIndex = "--Most unstable--";

// 1. Laske stabilityIndexSideViewRect_, se on kaikkein oikeanpuoleisin osio.
//    - Se on aina koko piirtoalueen oikeassa reunassa.
//    - Jos se on piilossa, tee alueen leveydest� 0, mutta tee siit� oikean alueen korkuinen ja 
//      sijoita se oikeaan kohtaan x-suunnassa.
NFmiRect NFmiTempViewDataRects::calcStabilityIndexRect()
{
	auto& settings = ctrlViewDocumentInterface_->GetMTATempSystem().GetSoundingViewSettingsFromWindowsRegisty();
	double right = totalDataRect_.Right();
	double top = totalDataRect_.Top();
	double bottom = totalDataRect_.Bottom();
	// Stability index ikkunassa on maksimissaan 19 merkki� per rivi.
	int stabilityIndexSideViewTextWidthCount = 19; 
	double left = calculateLeftRelativeEdgeOfSideView(right, 
		settings.ShowStabilityIndexSideView(), 
		stabilityIndexSideViewTextWidthCount,
		stabilityIndexFontSize_);
	NFmiRect rec(left, top, right, bottom);
	return rec;
}

const std::string g_maximumSampleStringForTextual = "203  -61.4 -74.0 10030 15 351";

// 2. Laske textualSoundingDataSideViewRect_.
//    - Se on stability-rectin vasemmassa reunassa.
//    - Jos se on piilossa, laske sen oikea korkeus ja x-sijainti ja sitten laita leveys 0:ksi (= tyhj� alue).
NFmiRect NFmiTempViewDataRects::calcTextualSoundingDataRect(const NFmiRect& rightSideView)
{
	auto& settings = ctrlViewDocumentInterface_->GetMTATempSystem().GetSoundingViewSettingsFromWindowsRegisty();
	double right = rightSideView.Left();
	auto showThisSideView = settings.ShowTextualSoundingDataSideView();
	if(showThisSideView && !rightSideView.IsEmpty())
	{
		// Jos oikealla ollut ikkuna ei ollut tyhj�, pit�� laittaa marginaali v�liin
		right -= baseMarginRelativeWidth_;
	}
	double top = totalDataRect_.Top();
	double bottom = totalDataRect_.Bottom();
	// Textual sounding data ikkunassa on maksimissaan 31 merkki� per rivi.
	int textualsoundingDataSideViewTextWidthCount = 31;
	double left = calculateLeftRelativeEdgeOfSideView(right,
		showThisSideView,
		textualsoundingDataSideViewTextWidthCount,
		textualSoundingDataFontSize_);
	NFmiRect rec(left, top, right, bottom);
	return rec;
}

// 3. Animaatio rect on n�kyviss� vain jos jompi kumpi stabilityIndex/textualSounding sivuosioista on n�kyviss�.
//    - Se tulee niist� oikeanpuoleisemman alueen alle ja on sen levyinen.
//    - Animaatio-rect osiot tulevat oikeanpuoleisimman-rectin alle alareunaan ja siksi my�s sit� perus rect:i�
//      pit�� s��t��, jonka alle n�m� tulevat.
void NFmiTempViewDataRects::makeAnimationControlRects()
{
	auto& settings = ctrlViewDocumentInterface_->GetMTATempSystem().GetSoundingViewSettingsFromWindowsRegisty();
	auto showStabilitySV = settings.ShowStabilityIndexSideView();
	auto showTextualSV = settings.ShowTextualSoundingDataSideView();
	if(showStabilitySV || showTextualSV)
	{
		NFmiRect *usedBaseRect = &stabilityIndexSideViewRect_;
		if(!showStabilitySV)
		{
			usedBaseRect = &textualSoundingDataSideViewRect_;
		}
		int buttonHeight = boost::math::iround(30 * drawSizeFactor_.Y());
		double buttonHeights = usedToolBox_->SY(buttonHeight);
		double animButtonWidth = usedBaseRect->Width() * 0.66;
		double animStepButtonWidth = usedBaseRect->Width() - animButtonWidth;
		animationButtonRect_ = NFmiRect(usedBaseRect->Left(), usedBaseRect->Bottom() - buttonHeights, usedBaseRect->Left() + animButtonWidth, usedBaseRect->Bottom());
		animationStepButtonRect_ = NFmiRect(usedBaseRect->Right() - animStepButtonWidth, usedBaseRect->Bottom() - buttonHeights, usedBaseRect->Right(), usedBaseRect->Bottom());
		// Laitetaan originaali baseRect viel� animaatio-laatikon p��lle, jotta ne eiv�t ole p��llekk�in
		auto verticalMargin = usedToolBox_->SY(boost::math::iround(1 * drawSizeFactor_.Y()));
		usedBaseRect->Bottom(animationButtonRect_.Top() - verticalMargin);
	}
	else
	{
		animationButtonRect_ = NFmiRect();
		animationStepButtonRect_ = NFmiRect();
	}
}

// 4. Laske secondaryDataFrame_.
//    - Se on textual-rectin vasemmassa reunassa.
//    - Jos se on piilossa, laske sen oikea korkeus ja x-sijainti ja sitten laita leveys 0:ksi (= tyhj� alue).
NFmiRect NFmiTempViewDataRects::calcSecondaryDataRect(const NFmiRect& rightSideView)
{
	auto& mtaSettings = ctrlViewDocumentInterface_->GetMTATempSystem();
	double right = rightSideView.Left();
	bool showThisSideView = mtaSettings.DrawSecondaryData();
	if(showThisSideView && ::addMarginOnSecondaryDataRect(textualSoundingDataSideViewRect_, stabilityIndexSideViewRect_))
	{
		right -= baseMarginRelativeWidth_;
	}
	double top = totalDataRect_.Top();
	double bottom = totalDataRect_.Bottom();
	double secondaryDataRectWidth = 0.;
	if(showThisSideView)
	{
		secondaryDataRectWidth = mtaSettings.UsedSecondaryDataFrameWidthFactor()* totalSoundingViewRect_.Width();
	}
	double left = right - secondaryDataRectWidth;
	return NFmiRect(left, top, right, bottom);
}

// 5. Laske soundingCurveDataRect_, jolle j�� loppu piirtoalueen tila.
//    - No lopputila ainakin sitten vertikaali ja horisontaali asteikkojen j�lkeen.
//    - Se on itsSecondaryDataFrame vasemmassa reunassa.
NFmiRect NFmiTempViewDataRects::calcSoundingCurveDataRect(const NFmiRect& rightSideView)
{
	double left = totalDataRect_.Left();
	double top = totalDataRect_.Top();
	double bottom = totalDataRect_.Bottom();
	double right = rightSideView.Left();
	if(::addMarginOnSoundingCurveDataRect(secondaryDataFrame_, textualSoundingDataSideViewRect_, stabilityIndexSideViewRect_))
	{
		right -= baseMarginRelativeWidth_;
	}

	return NFmiRect(left, top, right, bottom);
}

void LogHorizontalEdges(std::string name, const NFmiRect& rect)
{
	std::string logMessage = name;
	logMessage += " left = ";
	logMessage += std::to_string(rect.Left());
	logMessage += " right = ";
	logMessage += std::to_string(rect.Right());
	CatLog::logMessage(logMessage, CatLog::Severity::Debug, CatLog::Category::Visualization);
}

// Eri data alueet pit�� laskea oikeassa j�rjestyksess�.
// Niiden laskut aloitetaan oikeasta reunasta ja edet��n vasemmalle.
// Eri n�yt�nosiot voivat olla n�kyviss� tai piilossa, joka vaikuttaa aina seuraavien alueiden laskuihin.
// Sijoitetaan kaikki laatikot jopa tyhjin� niiden oikeaan paikkaan, jotta niiden reunoja voidaan aina 
// k�ytt�� loppujen piirtoalueiden laskuissa, t�m� helpottaa logiikkaa laskuissa.
void NFmiTempViewDataRects::calculateAllDataViewRelatedRects(const NFmiRect& totalSoundingViewRect, const NFmiPoint& drawSizeFactor, NFmiToolBox* usedToolBox)
{
	usedToolBox_ = usedToolBox;
	try
	{
		totalSoundingViewRect_ = totalSoundingViewRect;
		drawSizeFactor_ = drawSizeFactor;
		// Annetaan joku pieni osio koko n�yt�n leveydest� suhteelliselle eri osioiden 
		// v�liin j��v�lle marginaali leveydelle.
		baseMarginRelativeWidth_ = usedToolBox_->SX(boost::math::iround(2 * drawSizeFactor_.X()));
		calculateStabilityIndexFontSizeInPixels();
		calculatetextualSoundingDataFontSizeInPixels();

		totalDataRect_ = calcTotalDataRect();
		::LogHorizontalEdges("totalData", totalDataRect_);
		stabilityIndexSideViewRect_ = calcStabilityIndexRect();
		::LogHorizontalEdges("stabIndex", stabilityIndexSideViewRect_);
		textualSoundingDataSideViewRect_ = calcTextualSoundingDataRect(stabilityIndexSideViewRect_);
		::LogHorizontalEdges("textSound", textualSoundingDataSideViewRect_);
		makeAnimationControlRects();
		secondaryDataFrame_ = calcSecondaryDataRect(textualSoundingDataSideViewRect_);
		::LogHorizontalEdges("secondDat", secondaryDataFrame_);
		soundingCurveDataRect_ = calcSoundingCurveDataRect(secondaryDataFrame_);
		::LogHorizontalEdges("soundCurv", soundingCurveDataRect_);
	}
	catch(...)
	{ }
	usedToolBox_ = nullptr;
}

