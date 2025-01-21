#include "NFmiCrossSectionManagerView.h"
#include "NFmiToolBox.h"
#include "NFmiCtrlViewList.h"
#include "NFmiCrossSectionSystem.h"
#include "NFmiCrossSectionView.h"
#include "NFmiCrossSectionTimeControlView.h"
#include "NFmiText.h"
#include "NFmiValueString.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiTrajectorySystem.h"
#include "CtrlViewFunctions.h"
#include "SpecialDesctopIndex.h"
#include "CtrlViewDocumentInterface.h"
#include "GraphicalInfo.h"
#include "NFmiTrajectory.h"
#include "CtrlViewTimeConsumptionReporter.h"

using namespace std;

// decimals 0 eli ei desimaaleja ja muuten yhdellä desimaalilla
static NFmiString GetLonLatString(const NFmiPoint &thePoint, int decimals)
{
	NFmiString txt("(");
	txt += CtrlViewUtils::GetLatitudeMinuteStr(thePoint.Y(), decimals);
	txt += ",";
	txt += CtrlViewUtils::GetLongitudeMinuteStr(thePoint.X(), decimals);
	txt += ")";
	return txt;
}

NFmiCrossSectionManagerView::NFmiCrossSectionManagerView(const NFmiRect & theRect
								,NFmiToolBox * theToolBox)
:NFmiCtrlView(CtrlViewUtils::kFmiCrossSectionView, theRect, theToolBox)
,itsFooterRect()
,itsHeaderRect()
,itsViewListRect()
,itsViewList(0)
,itsTimeControlView(0)
,itsTimeControlViewRect()
,itsCrossSectionSystem(GetCtrlViewDocumentInterface()->CrossSectionSystem())
,itsDrawSizeFactorX(1)
,itsDrawSizeFactorY(1)
,itsLastScreenDrawPixelSizeInMM_x(0.1)
,itsLastScreenDrawPixelSizeInMM_y(0.1)
,itsLastScreenDataRectPressureScaleRatio(1)
{
	UpdateSize();
	CreateViewList();
}

void NFmiCrossSectionManagerView::UpdateSize(void)
{
	itsFooterRect = CalcFooterRect();
	itsHeaderRect = CalcHeaderRect();
	itsViewListRect = CalcViewListRect();
	itsTimeControlViewRect = CalcTimeControlViewRect();
	itsCrossSectionSystem->TimeCrossSectionDirty(true);
}

void NFmiCrossSectionManagerView::CalcDrawSizeFactors(void)
{
	CtrlViewUtils::GraphicalInfo &gInfo = itsCrossSectionSystem->GetGraphicalInfo();
	if(itsToolBox->GetDC()->IsPrinting())
	{
		itsDrawSizeFactorX = itsLastScreenDrawPixelSizeInMM_x / (1./gInfo.itsPixelsPerMM_x);
		itsDrawSizeFactorY =  itsLastScreenDrawPixelSizeInMM_y / (1./gInfo.itsPixelsPerMM_y);

		// muuten ehkä pikselien suhteet voisi laskea näin, mutta kun näytönohjaimiin ei voi luottaa että ne antaisivat 
		// näytön koon oikein millimetreissä. Tästä syystä joudun tekemään virityksen, jolla yritetään saada
		// vielä yksi skaala kerroin peliin. Eli lasketaan itsDataRect:in ja itsIndexRectin suhteet ja korjataan skaala kertoimia
		// iteratiivisesti (jos index-rect on siis yleensä olemassa).
		if(itsLastScreenDataRectPressureScaleRatio)
		{
			for(int i=0; i<3; i++)
			{ // iteroidaan kohti sopivia kertoimia
				double currentDataRectPressureScaleRatio = CalcDataRectPressureScaleRatio();
				if(currentDataRectPressureScaleRatio == 0)
					break;
				itsDrawSizeFactorX *= currentDataRectPressureScaleRatio/itsLastScreenDataRectPressureScaleRatio;
				itsDrawSizeFactorY *= currentDataRectPressureScaleRatio/itsLastScreenDataRectPressureScaleRatio;
			}
		}
	}
	else
	{
		itsDrawSizeFactorX = 1;
		itsDrawSizeFactorY = 1;
		itsLastScreenDrawPixelSizeInMM_x = 1./gInfo.itsPixelsPerMM_x;
		itsLastScreenDrawPixelSizeInMM_y = 1./gInfo.itsPixelsPerMM_y;
		itsLastScreenDataRectPressureScaleRatio = CalcDataRectPressureScaleRatio();
	}
}

double NFmiCrossSectionManagerView::CalcDataRectPressureScaleRatio(void)
{
	// pyydetään ensimmäiseltä ali-näyttöluokalta tämä arvo, kunhan ensin on päivitetty sen drawFactorit
	if(itsViewList)
	{
		itsViewList->Reset(); 
		itsViewList->Next();
		NFmiRect tmpRect(CalcListViewRect(1));
		NFmiCrossSectionView *aView = static_cast<NFmiCrossSectionView *>(itsViewList->Current());
		if(aView)
		{
			aView->SetFrame(tmpRect);
			aView->SetDrawSizeFactors(itsDrawSizeFactorX, itsDrawSizeFactorY);
			return aView->CalcDataRectPressureScaleRatio();
		}
	}
	return 0;
}

NFmiRect NFmiCrossSectionManagerView::CalcPressureScaleRect(void)
{
	// pyydetään ensimmäiseltä ali-näyttöluokalta paineasteikon laatikko, kunhan ensin on päivitetty sen drawFactorit
	if(itsViewList)
	{
		itsViewList->Reset(); 
		itsViewList->Next();
		NFmiRect tmpRect(CalcListViewRect(1));
		NFmiCrossSectionView *aView = static_cast<NFmiCrossSectionView *>(itsViewList->Current());
		if(aView)
		{
			aView->SetFrame(tmpRect);
			aView->SetDrawSizeFactors(itsDrawSizeFactorX, itsDrawSizeFactorY);
			return aView->CalcPressureScaleRect();
		}
	}
	return NFmiRect();
}

NFmiCrossSectionManagerView::~NFmiCrossSectionManagerView(void)
{
	Destroy();
}

void NFmiCrossSectionManagerView::Update(void)
{
	// ikkunan koko tai rivien lukumäärä muuttunut, pitää päivittää rivien kokoja
	CalcDrawSizeFactors();
	UpdateSize();
	UpdateListViewFrames();
}

void NFmiCrossSectionManagerView::Destroy(void)
{
	DestroyViewList();
	delete itsTimeControlView;
}

void NFmiCrossSectionManagerView::DestroyViewList(void)
{
	if(itsViewList)
	{
		delete itsViewList;
		itsViewList = 0;
	}
}

void NFmiCrossSectionManagerView::Draw(NFmiToolBox* theToolBox)
{
    CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(this, std::string(__FUNCTION__) + ": cross-section view drawing starts");
    if(theToolBox == 0)
		return ;
	itsToolBox = theToolBox;
	Update();
	DrawBackground();
	if(itsViewList)
		itsViewList->Draw(theToolBox);
	DrawHeader();
	DrawFooter();
}

void NFmiCrossSectionManagerView::ActivateCrossSectionView(const NFmiPoint& thePlace)
{
	if (IsIn(thePlace))
		itsCtrlViewDocumentInterface->ActiveMapDescTopIndex(itsMapViewDescTopIndex);
}

bool NFmiCrossSectionManagerView::LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
	ActivateCrossSectionView(thePlace);
	bool status = false;
    // Ensin on hanskattava parametrinäytön ja aikakontrolli-ikkunan raahaukset
    if(itsViewList->IsMouseDraggingOn())
        status = itsViewList->LeftButtonUp(thePlace, theKey);
    else if(itsTimeControlView && itsTimeControlView->IsMouseCaptured())
        status = itsTimeControlView->LeftButtonUp(thePlace, theKey);
    // Lopuksi normaalit hiiren napin päästöt samoille näytöille
    else if(itsViewListRect.IsInside(thePlace))
        status = itsViewList->LeftButtonUp(thePlace, theKey);
    else if(itsTimeControlView)
        status = itsTimeControlView->LeftButtonUp(thePlace, theKey);

    itsCtrlViewDocumentInterface->MouseCaptured(false);
    itsCtrlViewDocumentInterface->LeftMouseButtonDown(false);

	return status;
}

bool NFmiCrossSectionManagerView::LeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey)
{
	bool status = false;
	if(itsViewListRect.IsInside(thePlace))
		status = itsViewList->LeftButtonDown(thePlace, theKey);
	else if(itsTimeControlView)
		status = itsTimeControlView->LeftButtonDown(thePlace, theKey);
	return status;
}

bool NFmiCrossSectionManagerView::MiddleButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
	if(itsTimeControlView)
		return itsTimeControlView->MiddleButtonUp(thePlace, theKey);
	return false;
}

bool NFmiCrossSectionManagerView::RightButtonUp(const NFmiPoint & thePlace, unsigned long theKey)
{
	bool status = false;
	if(itsViewListRect.IsInside(thePlace))
		status = itsViewList->RightButtonUp(thePlace, theKey);
	else if(itsTimeControlView)
	{
		status = itsTimeControlView->RightButtonUp(thePlace, theKey);
	}

    itsCtrlViewDocumentInterface->MouseCaptured(false);
    itsCtrlViewDocumentInterface->RightMouseButtonDown(false);

	return status;
}

bool NFmiCrossSectionManagerView::LeftDoubleClick(const NFmiPoint &thePlace, unsigned long theKey)
{
	if(itsViewListRect.IsInside(thePlace))
		return itsViewList->LeftDoubleClick(thePlace, theKey);
	return false;
}

bool NFmiCrossSectionManagerView::RightButtonDown(const NFmiPoint & thePlace, unsigned long theKey)
{
	bool status = false;
	if(itsViewListRect.IsInside(thePlace))
		status = itsViewList->RightButtonDown(thePlace, theKey);
	else if(itsTimeControlView)
	{
		status = itsTimeControlView->RightButtonDown(thePlace, theKey);
	}
	return status;
}

bool NFmiCrossSectionManagerView::MouseMove(const NFmiPoint& thePlace, unsigned long theKey)
{
	bool status = false;

    // Jos itsViewList:in hiiren raahaus on päällä, pitää sille tehdä tarkastelut ensin
    if(itsViewList->IsMouseDraggingOn())
        status = itsViewList->MouseMove(thePlace, theKey);
    else if(itsTimeControlView && itsTimeControlView->IsMouseCaptured()) // 2. Jos aikakontrolli-ikkuna on kaapannut hiiren tehdään sille tarkastelut
	{
		status = itsTimeControlView->MouseMove(thePlace, theKey);
		NFmiTimeControlView::MouseStatusInfo mouseStatus = itsTimeControlView->GetMouseStatusInfo();
		if(mouseStatus.NeedsUpdate())
			status = true;
	}
    else if(itsViewListRect.IsInside(thePlace)) // 3. tehdään normaalit tarkastelut itsViewList:alle
    { 
        status = itsViewList->MouseMove(thePlace, theKey);
    }
    else if(itsTimeControlView) // 4. Lopuksi normaali tarkastelu aikakontrolli-ikkunalle
    {
        status = itsTimeControlView->MouseMove(thePlace, theKey);
        NFmiTimeControlView::MouseStatusInfo mouseStatus = itsTimeControlView->GetMouseStatusInfo();
        if(mouseStatus.NeedsUpdate())
            status = true;
    }
    return status;
}

bool NFmiCrossSectionManagerView::IsMouseDraggingOn(void)
{
    if(itsViewList->IsMouseDraggingOn())
        return true;
    else if(itsTimeControlView && itsTimeControlView->IsMouseDraggingOn())
        return true;
    else
        return false;
}

void NFmiCrossSectionManagerView::DrawData(void)
{
}

void NFmiCrossSectionManagerView::DrawBase(void)
{
}

void NFmiCrossSectionManagerView::DrawBackground(void)
{
	itsDrawingEnvironment.EnableFill();
	itsDrawingEnvironment.SetFillColor(NFmiColor(1.f,1.f,1.f));
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	DrawFrame(itsDrawingEnvironment);
}

// laskee alueen, joka on varattu kaikille listassa oleville näytöille yhteensä
NFmiRect NFmiCrossSectionManagerView::CalcViewListRect(void)
{
	NFmiRect rr(GetFrame());
	rr.Top(itsHeaderRect.Bottom());
	rr.Bottom(itsFooterRect.Top());
	return rr;
}

// tälle alueelle piirretään kaikki yhteinen tieto ppoikkileikkaus datasta mm. mahd. aikakontrolli-näyttö
// tämä lasketaan ensin näistä alueista!
NFmiRect NFmiCrossSectionManagerView::CalcFooterRect(void)
{
	NFmiRect rr(GetFrame());
	// HUOM! Piirrettäessä kuvaa näytölle, tulee mukaan aikakontrolli ikkuna, mutta printatessa se jää pois, jos kyse normaalista moodista
	double emptySpace = itsToolBox->SY(FmiRound(65 * itsDrawSizeFactorY)); // 60-pikseliä pitää olla tilaa alhaalla, teksti rivi + koordinaatit/aikakontrolli-ikkuna
	if(itsToolBox->GetDC()->IsPrinting() && itsCrossSectionSystem->GetCrossMode() == NFmiCrossSectionSystem::kNormal)
	{
		emptySpace = itsToolBox->SY(FmiRound(20 * itsDrawSizeFactorY));
	}
	rr.Top(rr.Bottom() - emptySpace);
	return rr;
}

// otsikkoon laitetaan yhteistä tietoa
NFmiRect NFmiCrossSectionManagerView::CalcHeaderRect(void)
{
	NFmiRect rr(GetFrame());
	double emptySpace = itsToolBox->SY(FmiRound(18 * itsDrawSizeFactorY));
	rr.Bottom(rr.Top() + emptySpace);
	return rr;
}

// laskee itsViewList:issa olevan näytön alueen (riippuu annetusta indeksistä
// ja näkyvien rivien määrästä ja aloitus rivistä)
NFmiRect NFmiCrossSectionManagerView::CalcListViewRect(int theIndex)
{
	if(!itsCrossSectionSystem->IsViewVisible(theIndex))
		return NFmiRect(0, 0, 0, 0);

	double emptySpace = 0; //itsToolBox->SY(3); // pitä olla 3-pikseliä väliä eri ikkunoiden ja yläosan välillä
	double rowCount = itsCrossSectionSystem->RowCount();
	double oneViewHeight = (itsViewListRect.Height() / rowCount) - emptySpace;
	int startRowIndex = itsCrossSectionSystem->StartRowIndex();
	double moveFromTop = (theIndex - startRowIndex + 1) * emptySpace + (theIndex - startRowIndex) * oneViewHeight;
	NFmiRect viewRect(itsViewListRect);
	viewRect.Top(viewRect.Top() + moveFromTop);
	viewRect.Bottom(viewRect.Top() + oneViewHeight);
	return viewRect;
}

// aikakontrolli-ikkunan varaama alue, kuuluu osana footer-aluetta
NFmiRect NFmiCrossSectionManagerView::CalcTimeControlViewRect(void)
{
	// leveys laskut on otettu crosssectionview-luokasta, missä dataalue on laskettu seuraavasti (ja aikakontrolliikkunan leveys pitää laskea sen mukaan)
	// NFmiCrossSectionView::CalcPressureScaleRect -metodista
	double leftPos = CalcPressureScaleRect().Right();

	double spaceForText = itsToolBox->SY(FmiRound(15 * itsDrawSizeFactorY)); // pitää olla tilaa vielä yhdelle teksti riville footerissa
	NFmiRect frame(GetFrame());
	NFmiRect resultRect(itsFooterRect.Left() + leftPos, itsFooterRect.Top() + spaceForText, frame.Right() - frame.Width() / 50., itsFooterRect.Bottom());
	return resultRect;
}

// normaali käytössä alkupiste saadaan crosssection systemiltä,
// mutta trajektori moodissa pitää pyytää alimman näkyvän rivin
// trajektorilta pisteet
NFmiPoint NFmiCrossSectionManagerView::GetStartLatLonPoint(void)
{
	if(itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView())
	{
		static NFmiPoint dummy(kFloatMissing, kFloatMissing);

		int usedRowIndex = itsCrossSectionSystem->LowestVisibleRowIndex();
		const auto &points = itsCtrlViewDocumentInterface->TrajectorySystem()->Trajectory(usedRowIndex-1).CrossSectionTrajectoryPoints();
		if(points.size() > 0)
			return points[0];
		else
			return dummy;
	}
	else
		return itsCrossSectionSystem->StartPoint();
}

NFmiPoint NFmiCrossSectionManagerView::GetEndLatLonPoint(void)
{
	if(itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView())
	{
		static NFmiPoint dummy(kFloatMissing, kFloatMissing);

		int usedRowIndex = itsCrossSectionSystem->LowestVisibleRowIndex();
		const auto &points = itsCtrlViewDocumentInterface->TrajectorySystem()->Trajectory(usedRowIndex-1).CrossSectionTrajectoryPoints();
		if(points.size() > 0)
			return points[points.size()-1];
		else
			return dummy;
	}
	else
		return itsCrossSectionSystem->EndPoint();
}

NFmiPoint NFmiCrossSectionManagerView::GetMiddleLatLonPoint(void)
{
	if(itsCtrlViewDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView())
	{
		static NFmiPoint dummy(kFloatMissing, kFloatMissing);

		int usedRowIndex = itsCrossSectionSystem->LowestVisibleRowIndex();
		const auto &points = itsCtrlViewDocumentInterface->TrajectorySystem()->Trajectory(usedRowIndex-1).CrossSectionTrajectoryPoints();
		if(points.size() > 0)
			return points[points.size()/2];
		else
			return dummy;
	}
	else
		return itsCrossSectionSystem->MiddlePoint();
}

// kirjoittaa mm. pääpisteiden koordinaatit ruudun alareunaan pisteiden kohdalle.
// Aluksi vain 1. ja viimeisen pisteen.
void NFmiCrossSectionManagerView::DrawFooter(void)
{
	if(itsTimeControlView == 0 || itsCrossSectionSystem->TimeCrossSectionDirty())
		CreateTimeControlView();
	if(itsToolBox->GetDC()->IsPrinting() == FALSE || itsCrossSectionSystem->GetCrossMode() != NFmiCrossSectionSystem::kNormal) // ei piirretä printatessa!!!!
	{
		if(itsTimeControlView && itsTimeControlViewRect.Height() > 0.)
			itsTimeControlView->Draw(itsToolBox);
	}

	// ensin alkupiste
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	NFmiString txt = GetLonLatString(GetStartLatLonPoint(), 2);
	if(itsCrossSectionSystem->GetCrossMode() == NFmiCrossSectionSystem::kRoute)
		txt += itsCrossSectionSystem->RouteTimes().operator [](0).ToStr(" HH:mm");

	itsDrawingEnvironment.SetFontSize(NFmiPoint(14 * itsDrawSizeFactorX, 14 * itsDrawSizeFactorY));
	NFmiPoint textPoint(itsFooterRect.TopLeft());
	int moveTextBy = FmiRound(2 * itsDrawSizeFactorY);
	double moveDownward = itsToolBox->SY(moveTextBy); // siirretää vielä kuusi pikseliä alas
	textPoint.Y(textPoint.Y() + moveDownward);
	FmiDirection oldAlignment = itsToolBox->GetTextAlignment();
	itsToolBox->SetTextAlignment(kTopLeft);
	NFmiText text(textPoint, txt, false, 0, &itsDrawingEnvironment);
	itsToolBox->Convert(&text);

	// StartPoint : piirretään vielä kartan väri pallot tänne helpottamaan tunnistusta
	double circleY = textPoint.Y() - moveDownward * 1.;
	NFmiPoint circlePoint(itsCrossSectionSystem->StartXYPoint());
	circlePoint.Y(circleY);
	itsDrawingEnvironment.SetFillColor(itsCrossSectionSystem->StartPointFillColor());
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,1.f));
	itsDrawingEnvironment.EnableFill();
	double xWidth2 = itsToolBox->SX(FmiRound(12 * itsDrawSizeFactorX));
	double yWidth2 = itsToolBox->SY(FmiRound(12 * itsDrawSizeFactorY));
	NFmiRect circleRect(0, 0, xWidth2, yWidth2);
	circleRect.Center(circlePoint);
	itsToolBox->DrawEllipse(circleRect, &itsDrawingEnvironment); // piirretään alkupisteen väripallo

	if(itsCrossSectionSystem->GetCrossMode() != NFmiCrossSectionSystem::kTime && itsCrossSectionSystem->GetCrossMode() != NFmiCrossSectionSystem::kObsAndFor)
	{

		// sitten loppu piste
		itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,0.f));
		NFmiString txt2 = GetLonLatString(GetEndLatLonPoint(), 2);
		if(itsCrossSectionSystem->GetCrossMode() == NFmiCrossSectionSystem::kRoute)
			txt2 += itsCrossSectionSystem->RouteTimes().operator [](itsCrossSectionSystem->RouteTimes().size()-1).ToStr(" HH:mm");
		itsToolBox->SetTextAlignment(kTopRight);
		textPoint.X(itsFooterRect.Right()); // TÄMÄ ON HUUHAATA!!!!
		NFmiText text2(textPoint, txt2, false, 0, &itsDrawingEnvironment);
		itsToolBox->Convert(&text2);

		// EndPoint : piirretään vielä kartan väri pallot tänne helpottamaan tunnistusta
		NFmiPoint circlePoint2(itsCrossSectionSystem->EndXYPoint());
		circlePoint2.Y(circleY);
		itsDrawingEnvironment.SetFillColor(itsCrossSectionSystem->EndPointFillColor());
		itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,1.f));
		itsDrawingEnvironment.EnableFill();
		NFmiRect circleRect2(0, 0, xWidth2, yWidth2);
		circleRect2.Center(circlePoint2);
		itsToolBox->DrawEllipse(circleRect2, &itsDrawingEnvironment); // piirretään alkupisteen väripallo

		if(itsCrossSectionSystem->CrossSectionMode() == NFmiCrossSectionSystem::k3Point)
		{
			// sitten loppu piste
			itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,0.f));
			NFmiString txt3 = GetLonLatString(GetMiddleLatLonPoint(), 2);
			if(itsCrossSectionSystem->GetCrossMode() == NFmiCrossSectionSystem::kRoute)
				txt3 += itsCrossSectionSystem->RouteTimes().operator [](static_cast<int>(itsCrossSectionSystem->RouteTimes().size()/2)).ToStr(" HH:mm");
			itsToolBox->SetTextAlignment(kTopCenter);
			textPoint.X(itsFooterRect.Center().X()); // TÄMÄ ON HUUHAATA!!!!
			NFmiText text3(textPoint, txt3, false, 0, &itsDrawingEnvironment);
			itsToolBox->Convert(&text3);

			// MiddlePoint : piirretään vielä kartan väri pallot tänne helpottamaan tunnistusta
			NFmiPoint circlePoint3(itsCrossSectionSystem->MiddleXYPoint());
			circlePoint3.Y(circleY);
			itsDrawingEnvironment.SetFillColor(itsCrossSectionSystem->MiddlePointFillColor());
			itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,1.f));
			itsDrawingEnvironment.EnableFill();
			NFmiRect circleRect3(0, 0, xWidth2, yWidth2);
			circleRect3.Center(circlePoint3);
			itsToolBox->DrawEllipse(circleRect3, &itsDrawingEnvironment); // piirretään alkupisteen väripallo
		}

		itsToolBox->SetTextAlignment(oldAlignment);

		DrawActivatedMinorPoint();
	}
}

void NFmiCrossSectionManagerView::DrawActivatedMinorPoint(void)
{
	if(itsCrossSectionSystem->GetCrossMode() != NFmiCrossSectionSystem::kTime && itsCrossSectionSystem->IsMinorPointActivated())
	{
		int index = itsCrossSectionSystem->ActivatedMinorPointIndex();
		int pointCount = static_cast<int>(itsCrossSectionSystem->MinorPoints().size());
		double xCoordinate = itsTimeControlViewRect.Left() + index * itsFooterRect.Width() / (pointCount+1);

		// piirretään sitten alareunaan aktiivisen pisteen koordinaatit
		itsDrawingEnvironment.SetFrameColor(NFmiColor(1,0,0)); // laitetaan teksti punaiseksi
		NFmiString txt = GetLonLatString(itsCrossSectionSystem->ActivatedMinorPoint(), 2);
		if(itsCrossSectionSystem->GetCrossMode() == NFmiCrossSectionSystem::kRoute)
			txt += itsCrossSectionSystem->RouteTimes().operator [](index).ToStr(" HH:mm");
		itsDrawingEnvironment.SetFontSize(NFmiPoint(13 * itsDrawSizeFactorX, 13 * itsDrawSizeFactorY));
		NFmiPoint textPoint(xCoordinate, itsFooterRect.Top());
		int moveTextBy = FmiRound(-3 * itsDrawSizeFactorY);
		double moveDownward = itsToolBox->SY(moveTextBy); // siirretää yksi pikseli alas
		textPoint.Y(textPoint.Y() + moveDownward);
		FmiDirection oldAlignment = itsToolBox->GetTextAlignment();
		FmiDirection alignm = kTopCenter;
		if(index < pointCount/3.)
			alignm = kTopLeft;
		else if(index > 2*pointCount/3.)
			alignm = kTopRight;
		itsToolBox->SetTextAlignment(alignm);
		NFmiText text(textPoint, txt, false, 0, &itsDrawingEnvironment);
		itsToolBox->Convert(&text);
		itsToolBox->SetTextAlignment(oldAlignment);
	}
}

// tooltip feikki viritys
void NFmiCrossSectionManagerView::DrawOverBitmapThings(NFmiToolBox *theGTB, const NFmiPoint &thePlace)
{
	if(itsViewList)
	{
		for(itsViewList->Reset(); itsViewList->Next();)
		{
			if(itsViewList->Current()->IsIn(thePlace))
				((NFmiCrossSectionView*)itsViewList->Current())->DrawOverBitmapThings(theGTB, thePlace);
		}
	}
}

std::string NFmiCrossSectionManagerView::ComposeToolTipText(const NFmiPoint& theRelativePoint)
{
	if(itsTimeControlView && itsTimeControlView->IsIn(theRelativePoint))
		return itsTimeControlView->ComposeToolTipText(theRelativePoint);
	if(itsViewList)
	{
		for(itsViewList->Reset(); itsViewList->Next();)
		{
			if(itsViewList->Current()->IsIn(theRelativePoint))
				return ((NFmiCrossSectionView*)itsViewList->Current())->ComposeToolTipText(theRelativePoint);
		}
	}
	return std::string();
}

// tällä luodaan kerralla kaikki itsViewList:issä olevat näyttö-luokat
// Kutsutaan konstruktorissa, kaikki näytöt luodaan kerralla, vaikka niitä kaikkia ei näytetäkään.
void NFmiCrossSectionManagerView::CreateViewList(void)
{
	DestroyViewList();
	itsViewList = new NFmiCtrlViewList(itsMapViewDescTopIndex, itsViewListRect, itsToolBox,
										boost::shared_ptr<NFmiDrawParam>());
	int maxSize = itsCrossSectionSystem->MaxViewRowSize();
	NFmiCrossSectionView *tmpView = 0;
    // At the point there are no columns in cross section view, so column index is set to 1 for all sub views.
    int viewColumnIndex = 1;
	for(int i=1 ; i <= maxSize ; i++)
	{
		tmpView = new NFmiCrossSectionView(itsToolBox, i, viewColumnIndex);
		NFmiRect tmpRect(CalcListViewRect(i));
		tmpView->SetFrame(tmpRect);
		itsViewList->Add(tmpView, false);
	}
}

// kutsu tätä kun muutetaan näyttörivien lukumäärää tai vaihdetaan rivejä.
// Laskee uudet frame-rectit itsViewList:issä oleville näytöille
void NFmiCrossSectionManagerView::UpdateListViewFrames(void)
{
	if(!itsViewList)
		return ;
	int i = 1;
	for(itsViewList->Reset(); itsViewList->Next(); i++)
	{
		NFmiRect tmpRect(CalcListViewRect(i));
		itsViewList->Current()->SetFrame(tmpRect);
		static_cast<NFmiCrossSectionView *>(itsViewList->Current())->SetDrawSizeFactors(itsDrawSizeFactorX, itsDrawSizeFactorY);
		itsViewList->Current()->Update();
	}
}

void NFmiCrossSectionManagerView::DrawHeader(void)
{
	NFmiString txt;
	if(itsCrossSectionSystem->GetCrossMode() == NFmiCrossSectionSystem::kTime)
	{
		txt += itsCrossSectionSystem->CrossSectionTimeControlTimeBag().FirstTime().ToStr(::GetDictionaryString("CrossSectionDlgTimeStr"), itsCtrlViewDocumentInterface->Language());
		txt += " - ";
		txt += itsCrossSectionSystem->CrossSectionTimeControlTimeBag().LastTime().ToStr(::GetDictionaryString("CrossSectionDlgTimeStr"), itsCtrlViewDocumentInterface->Language());
		txt += " ";

		// alkupiste
		txt += ::GetDictionaryString("CrossSectionDlgLocationStr");
		txt += ":";
		txt += GetLonLatString(itsCrossSectionSystem->StartPoint(), 2);
	}
	else if(itsCrossSectionSystem->GetCrossMode() == NFmiCrossSectionSystem::kNormal)
        txt += itsCtrlViewDocumentInterface->ActiveMapTime().ToStr(::GetDictionaryString("CrossSectionDlgTimeStr"), itsCtrlViewDocumentInterface->Language());
	else if(itsCrossSectionSystem->GetCrossMode() == NFmiCrossSectionSystem::kObsAndFor)
	{
		if(itsCrossSectionSystem->ObsForModeLocation().GetIdent() == 0)
		{
			txt += "No sounding station found.";
		}
		else
		{
			txt += itsCrossSectionSystem->CrossSectionTimeControlTimeBag().FirstTime().ToStr(::GetDictionaryString("CrossSectionDlgTimeStr"), itsCtrlViewDocumentInterface->Language());
			txt += " - ";
			txt += itsCrossSectionSystem->CrossSectionTimeControlTimeBag().LastTime().ToStr(::GetDictionaryString("CrossSectionDlgTimeStr"), itsCtrlViewDocumentInterface->Language());
			txt += " ";

			txt += itsCrossSectionSystem->ObsForModeLocation().GetName();
			txt += " (";
			double distInKm = itsCrossSectionSystem->ObsForModeLocation().Distance(itsCrossSectionSystem->StartPoint()) / 1000.;
			txt += NFmiValueString::GetStringWithMaxDecimalsSmartWay(distInKm, 1);
			txt += " km)";
		}
	}
	else
	{
		txt += itsCrossSectionSystem->CrossSectionTimeControlTimeBag().FirstTime().ToStr(::GetDictionaryString("CrossSectionDlgTimeStr"), itsCtrlViewDocumentInterface->Language());
		txt += " -> ";
		txt += itsCrossSectionSystem->CrossSectionTimeControlTimeBag().LastTime().ToStr(::GetDictionaryString("CrossSectionDlgTimeStr"), itsCtrlViewDocumentInterface->Language());
	}
	txt += "  vert.-pts:";
	txt += NFmiValueString::GetStringWithMaxDecimalsSmartWay(itsCrossSectionSystem->VerticalPointCount(), 0);
	txt += " hor.-pts:";
	txt += NFmiValueString::GetStringWithMaxDecimalsSmartWay(static_cast<double>(itsCrossSectionSystem->MinorPoints().size()), 0);

	itsDrawingEnvironment.SetFontSize(NFmiPoint(15 * itsDrawSizeFactorX, 15 * itsDrawSizeFactorY));
	itsDrawingEnvironment.SetFrameColor(NFmiColor(0.f,0.f,0.f));
	NFmiPoint textPoint(itsHeaderRect.TopLeft());
	double moveDownward = itsToolBox->SY(FmiRound(1 * itsDrawSizeFactorY)); // siirretään vielä pikseli alas
	textPoint.Y(textPoint.Y() + moveDownward);
	textPoint.X(textPoint.X() + 2*moveDownward); // siirretään myös pari pikseliä oikeaan
	FmiDirection oldAlignment = itsToolBox->GetTextAlignment();
	itsToolBox->SetTextAlignment(kTopLeft);
	NFmiText text(textPoint, txt, false, 0, &itsDrawingEnvironment);
	itsToolBox->Convert(&text);
	itsToolBox->SetTextAlignment(oldAlignment);
}

bool NFmiCrossSectionManagerView::CreateTimeControlView(void)
{
	NFmiTimeControlView::MouseStatusInfo mouseStatusInfo;
	if(itsTimeControlView)
		mouseStatusInfo = itsTimeControlView->GetMouseStatusInfo();
	delete itsTimeControlView;
	itsTimeControlView = 0;
	if(itsCtrlViewDocumentInterface->DefaultEditedDrawParam())
	{
		itsTimeControlView = new NFmiCrossSectionTimeControlView(itsMapViewDescTopIndex, itsTimeControlViewRect
													,itsToolBox
													, itsCtrlViewDocumentInterface->DefaultEditedDrawParam()
													,false
													,false
													,true
													,true
													,0.);
		itsTimeControlView->Initialize(true, true); // tämä on konstruktori virtuaali funktio ongelma fixi yritys
		itsTimeControlView->SetMouseStatusInfo(mouseStatusInfo);
        itsCrossSectionSystem->TimeCrossSectionDirty(false);
		return true;
	}
	return false;
}

bool NFmiCrossSectionManagerView::MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta)
{
	if(itsRect.IsInside(thePlace))
	{
		if(itsTimeControlView && itsTimeControlView->IsIn(thePlace))
			return itsTimeControlView->MouseWheel(thePlace, theKey, theDelta);
		else if(itsHeaderRect.IsInside(thePlace) || itsFooterRect.IsInside(thePlace))
			return itsCrossSectionSystem->MouseWheel(thePlace, theKey, theDelta);
		else if(itsViewListRect.IsInside(thePlace))
			return itsViewList->MouseWheel(thePlace, theKey, theDelta);
	}
	return false;
}
