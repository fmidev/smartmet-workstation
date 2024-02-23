// FmiDataQualityCheckerDialog.cpp : implementation file
//

#include "stdafx.h"
//#include "zeditmap2.h"
#include "FmiDataQualityCheckerDialog.h"
#include "FmiWin32TemplateHelpers.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiDataQualityChecker.h"
#include "NFmiQueryDataUtil.h"
#include "CFmiOperationProgressAndCancellationDlg.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiQueryData.h"
#include "FmiGdiPlusHelpers.h"
#include "FmiWin32Helpers.h"
#include "CloneBitmap.h"
#include "NFmiApplicationWinRegistry.h"
#include "NFmiDictionaryFunction.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "CtrlViewFunctions.h"
#include "persist2.h"

#ifdef _MSC_VER
#pragma warning (disable : 4244 4267) // boost:in thread kirjastosta tulee ik‰v‰sti 4244 varoituksia
#endif
#include <boost/thread.hpp>
#ifdef _MSC_VER
#pragma warning (default : 4244 4267) // laitetaan 4244 takaisin p‰‰lle, koska se on t‰rke‰ (esim. double -> int auto castaus varoitus)
#endif

// CFmiDataQualityCheckerDialog dialog

static const int DATA_QUALITY_VIEW_TOOLTIP_ID = 1234359;
static const int gWorkingAreaMargin = 3;

const NFmiViewPosRegistryInfo CFmiDataQualityCheckerDialog::s_ViewPosRegistryInfo(CRect(400, 100, 800, 600), "\\DataQualityCheckerDialog");

IMPLEMENT_DYNAMIC(CFmiDataQualityCheckerDialog, CDialog)

CFmiDataQualityCheckerDialog::CFmiDataQualityCheckerDialog(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent /*=NULL*/)
	: CDialog(CFmiDataQualityCheckerDialog::IDD, pParent)
	,itsSmartMetDocumentInterface(smartMetDocumentInterface)
	,itsDoubleBuffer2ScreenGraphics(0)
	,itsDrawingGraphics(0)
	,itsDoubleBufferBitmap(0)
	,itsClienAreaRect()
	,itsDrawRect()
{

}

CFmiDataQualityCheckerDialog::~CFmiDataQualityCheckerDialog()
{
	delete itsDoubleBuffer2ScreenGraphics;
	delete itsDrawingGraphics;
//	delete itsDoubleBufferBitmap; 
}

void CFmiDataQualityCheckerDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFmiDataQualityCheckerDialog, CDialog)
	ON_BN_CLICKED(IDOK, &CFmiDataQualityCheckerDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_DO_QUALITY_CHECK, &CFmiDataQualityCheckerDialog::OnBnClickedButtonDoQualityCheck)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_LBUTTONUP()
	ON_WM_GETMINMAXINFO()
	ON_WM_PAINT()
	ON_NOTIFY (UDM_TOOLTIP_DISPLAY, NULL, NotifyDisplayTooltip)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CFmiDataQualityCheckerDialog message handlers

void CFmiDataQualityCheckerDialog::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}

static void DoDataChecking(NFmiThreadCallBacks &theThreadCallBacks, NFmiDataQualityChecker &theDataQualityChecker)
{
	try
	{
		// threadien kanssa pit‰‰ kikkailla. t‰ll‰ on odotettava ett‰ p‰‰-threadissa saadaan alustettuaprogress-cancel 
		// dialogi, muuten liian aikaisista funktio kutsuista voi tulla fataaleja
		if(theThreadCallBacks.WaitUntilInitialized())
		{
			theDataQualityChecker.CheckData(&theThreadCallBacks);
			theThreadCallBacks.DoPostMessage(ID_MESSAGE_WORKING_THREAD_COMPLETED);
			return ; // ei saa menn‰ catch:ien j‰lkeiseen viestin l‰hetykseen...
		}
	}
	catch(NFmiStopThreadException & )
	{
	}
	catch(...)
	{
	}
	theThreadCallBacks.DoPostMessage(ID_MESSAGE_WORKING_THREAD_CANCELED);
}

void CFmiDataQualityCheckerDialog::OnBnClickedButtonDoQualityCheck()
{
	UpdateData(TRUE);

	boost::shared_ptr<NFmiFastQueryInfo> editedInfo = itsSmartMetDocumentInterface->EditedSmartInfo();
	if(editedInfo == 0)
	{
		// laita viesti‰, ett‰ ei ole mit‰‰n dataa tarkastettavaksi
		return ;
	}

	// Tehd‰‰n datan chekkaukselle progress ja peruutus dialogi ja toiminnot.
	NFmiStopFunctor stopper;
	CFmiOperationProgressAndCancellationDlg dlg(::GetDictionaryString("Calculating edited data's quality checks"), false, stopper, this);
	NFmiThreadCallBacks threadCallBacks(&stopper, &dlg);

	// Luodaaan ja laitetaan tyˆ-threadi k‰yntiin.

	// pit‰‰ tehd‰ kopio editoidusta datasta (thread turvallinen)
	NFmiDataQualityChecker &dataQualityChecker = itsSmartMetDocumentInterface->DataQualityChecker();
	std::unique_ptr<NFmiQueryData> editedDataPtr(editedInfo->DataReference()->Clone());
	
	dataQualityChecker.SetCheckedData(std::move(editedDataPtr));
	boost::thread wrk_thread(::DoDataChecking, boost::ref(threadCallBacks), boost::ref(dataQualityChecker));
	// k‰ynnistet‰‰n sitten porgress ja cancel dialogi, ett‰ k‰ytt‰j‰ voi tarvittaessa keskeytt‰‰ tyˆt.
	/* int status = */ static_cast<int>(dlg.DoModal());
	RefreshWorkingArea();
//	if(status == IDCANCEL) // Jos k‰ytt‰j‰ keskeytti tyˆn, laita ilmoitus siit‰
//		; // laita ilmoitus tyˆn keskeytyksest‰

}

BOOL CFmiDataQualityCheckerDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

    std::string titleStr = ::GetDictionaryString("Data quality checking");
    SetWindowText(CA2T(titleStr.c_str()));
	CFmiWin32Helpers::InitializeCPPTooltip(this, m_tooltip, DATA_QUALITY_VIEW_TOOLTIP_ID, 400);

	CWnd *doCheckButton = GetDlgItem(IDC_BUTTON_DO_QUALITY_CHECK);
	if(doCheckButton)
		m_tooltip.AddTool(doCheckButton, _TEXT("Do quality check for the edited data."));
	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);

	// Tee paikan asetus vasta tooltipin alustuksen j‰lkeen, niin se toimii ilman OnSize-kutsua.
	std::string errorBaseStr("Error in CFmiDataQualityCheckerDialog::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiDataQualityCheckerDialog::OnOK()
{
	DoWhenClosing();

	CDialog::OnOK();
}

void CFmiDataQualityCheckerDialog::OnCancel()
{
	DoWhenClosing();

	CDialog::OnCancel();
}

void CFmiDataQualityCheckerDialog::OnClose()
{
	DoWhenClosing();

	CDialog::OnClose();
}

void CFmiDataQualityCheckerDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	itsClienAreaRect = GetPaintedRect(gWorkingAreaMargin);
	itsDrawRect = CRect(0, 0, itsClienAreaRect.Width(), itsClienAreaRect.Height());

	delete itsDoubleBuffer2ScreenGraphics;
	itsDoubleBuffer2ScreenGraphics = Gdiplus::Graphics::FromHDC(this->GetDC()->GetSafeHdc());

	delete itsDoubleBufferBitmap;
	itsDoubleBufferBitmap = new Gdiplus::Bitmap(itsDrawRect.Width(), itsDrawRect.Height());
	delete itsDrawingGraphics;
	itsDrawingGraphics = Gdiplus::Graphics::FromImage(itsDoubleBufferBitmap);

	CRect rect;
	GetClientRect(rect);
	m_tooltip.SetToolRect(this, DATA_QUALITY_VIEW_TOOLTIP_ID, rect);

	Invalidate(FALSE);
//	RefreshWorkingArea();
}

void CFmiDataQualityCheckerDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnLButtonUp(nFlags, point);
}

void CFmiDataQualityCheckerDialog::SetDefaultValues(void)
{
    MoveWindow(CFmiDataQualityCheckerDialog::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

void CFmiDataQualityCheckerDialog::DoWhenClosing(void)
{
    itsSmartMetDocumentInterface->DataQualityChecker().ViewOn(false);
	AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttan‰yttˆ eli mainframe
}

void CFmiDataQualityCheckerDialog::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 450;
	lpMMI->ptMinTrackSize.y = 350;

	CDialog::OnGetMinMaxInfo(lpMMI);
}

void CFmiDataQualityCheckerDialog::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here

 // t‰m‰ on pika viritys, kun muuten Calc- ja Close -nappuloiden kohdalta j‰‰ kaista maalaamatta kun laitoin ikkunaan v‰lkkym‰ttˆm‰n p‰ivityksen
	CBrush brush(::GetSysColor(COLOR_3DFACE));
	CRect area(0, 0, itsClienAreaRect.right, itsClienAreaRect.top);
	dc.FillRect(&area, &brush);

	try
	{
		if(itsDoubleBuffer2ScreenGraphics && itsDrawingGraphics)
		{
			// T‰ss‰ double buffer kikka (http://www.codeproject.com/KB/GDI-plus/gdiplus.aspx)
			// piirret‰‰n gdi+ tavarat kuvaan (itsGdiPlusDoubleBufferGraphics oliolla) 
			// ja tulostetaan kuva kerralla ruudulle ilman v‰lkett‰ (itsGdiPlusScreenGraphics-oliolla)
			DoDrawStatistics();
			itsDoubleBuffer2ScreenGraphics->DrawImage(itsDoubleBufferBitmap, CtrlView::CRect2GdiplusRect(itsClienAreaRect));
		}
	}
	catch(...)
	{
	}


	// Do not call CDialog::OnPaint() for painting messages
}

void CFmiDataQualityCheckerDialog::RefreshWorkingArea(void)
{
	Invalidate();
}

CRect CFmiDataQualityCheckerDialog::GetPaintedRect(int theUsedMargin)
{
	CRect rect;
	GetClientRect(rect);
	CWnd* topLimiter = GetDescendantWindow(IDC_BUTTON_DO_QUALITY_CHECK);
	if(topLimiter)
	{
		WINDOWPLACEMENT wplace;
		topLimiter->GetWindowPlacement(&wplace);
		CRect topRect = wplace.rcNormalPosition;
		rect.top = topRect.bottom + theUsedMargin;
//		rect.left += theUsedMargin;
//		rect.right -= theUsedMargin;
//		rect.bottom -= theUsedMargin;
	}
	return rect;
}

void CFmiDataQualityCheckerDialog::DrawBackGround(void)
{
	NFmiColor frameColor(0,0,0);
	NFmiColor fillColor(1,1,1);
	CRect frameRect = itsDrawRect;
	frameRect.bottom -= 1;
	frameRect.right -= 1;
	CtrlView::DrawRect(*itsDrawingGraphics, CtrlView::CRect2GdiplusRect(frameRect), frameColor, fillColor, true, true, 1.f);
}

static const std::wstring gFontNameStr(L"Arial");

static float RoundValue(float value, float rounder)
{
	return ::round(value/rounder) * rounder;
}

static void GetCapsule(Gdiplus::RectF baseRect, Gdiplus::GraphicsPath &thePathOut) 
{
	thePathOut.Reset();

	float diameter; 
	Gdiplus::RectF arc; 
	try 
	{ 
		if(baseRect.Width > baseRect.Height ) 
		{ 
			// return horizontal capsule 
			diameter = baseRect.Height; 
			Gdiplus::SizeF sizeF(diameter, diameter);
			Gdiplus::PointF location;
			baseRect.GetLocation(&location);
			arc = Gdiplus::RectF(location, sizeF); 
			thePathOut.AddArc(arc, 90, 180); 
			arc.X = baseRect.GetRight() - diameter; 
			thePathOut.AddArc(arc, 270, 180); 
		} 
		else if( baseRect.Width < baseRect.Height ) 
		{ 
			// return vertical capsule 
			diameter = baseRect.Width;
			Gdiplus::SizeF sizeF(diameter, diameter);
			Gdiplus::PointF location;
			baseRect.GetLocation(&location);
			arc = Gdiplus::RectF(location, sizeF); 
			thePathOut.AddArc(arc, 180, 180); 
			arc.Y = baseRect.GetBottom() - diameter; 
			thePathOut.AddArc(arc, 0, 180); 
		} 
		else
		{ 
			// return circle 
			thePathOut.AddEllipse(baseRect); 
		}
	} 
	catch(...)
	{
		thePathOut.AddEllipse(baseRect); 
	} 
	thePathOut.CloseFigure(); 
} 


static void GetRoundedRect(const Gdiplus::RectF &baseRect, float radius, Gdiplus::GraphicsPath &thePathOut) 
{
	thePathOut.Reset();
	// if corner radius is less than or equal to zero, 
	// return the original rectangle 
	if( radius<=0.0F ) 
	{ 
		thePathOut.AddRectangle(baseRect); 
		thePathOut.CloseFigure(); 
		return ;
	}

	// if the corner radius is greater than or equal to 
	// half the width, or height (whichever is shorter) 
	// then return a capsule instead of a lozenge 
	if( radius>=(FmiMin(baseRect.Width, baseRect.Height))/2.0) 
	{
		::GetCapsule(baseRect, thePathOut);
		return ;
	}

	// create the arc for the rectangle sides and declare 
	// a graphics path object for the drawing 
	float diameter = radius * 2.0F;
	Gdiplus::SizeF sizeF(diameter, diameter);
	Gdiplus::PointF location;
	baseRect.GetLocation(&location);
	Gdiplus::RectF arc(location, sizeF);

	// top left arc
	thePathOut.AddArc(arc, 180, 90);

	// top right arc
	arc.X = baseRect.GetRight() - diameter;
	thePathOut.AddArc(arc, 270, 90);

	// bottom right arc
	arc.Y = baseRect.GetBottom() - diameter;
	thePathOut.AddArc(arc, 0, 90);

	// bottom left arc
	arc.X = baseRect.GetLeft();
	thePathOut.AddArc(arc, 90, 90);

	thePathOut.CloseFigure(); 
} 

static CRect GetParamRect(int index, const CRect &theTotalArea)
{
	int paramAreaHeight = 45;
	int parAreaTop = theTotalArea.top + index * paramAreaHeight;
	return CRect(theTotalArea.left, parAreaTop, theTotalArea.right, parAreaTop + paramAreaHeight);
}

static float GetFirstNormalButtonX(const CRect &theParamAreaRect)
{
	return theParamAreaRect.Width()/30.f;
}

static Gdiplus::RectF GetStatusRectF(const CRect &theParamAreaRect, int timeIndex, int timeSize, bool isSumRect)
{
	if(timeSize == 0)
		timeSize = 1;
	float yOffset = 20;
	float buttonMarginX = 2;
	float buttonHeight = 20;
	float buttonWidthMax = 30;
	float firstNormalButtonX = ::GetFirstNormalButtonX(theParamAreaRect);
	float buttonWidth = (theParamAreaRect.right - firstNormalButtonX) / static_cast<float>(timeSize) - buttonMarginX;
	buttonWidth = FmiMin(buttonWidthMax, buttonWidth);
	if(isSumRect)
	{
		buttonWidth = buttonWidth*1.4f;
		return Gdiplus::RectF(theParamAreaRect.left + buttonMarginX, theParamAreaRect.top + yOffset, buttonWidth, buttonHeight);
	}
	else
	{
		float buttonPlaceX = firstNormalButtonX + timeIndex * (buttonWidth + buttonMarginX);
		return Gdiplus::RectF(buttonPlaceX, theParamAreaRect.top + yOffset, buttonWidth, buttonHeight);
	}
}

static NFmiColor GetStatusColor(int status)
{
	if(status == 1) // ok
		return NFmiColor(0, 1, 0);
	else if(status == 2) // varoitus
		return NFmiColor(1, 0.5f, 0);
	else if(status == 3) // virhe
		return NFmiColor(1, 0, 0);
	else if(status == 4) // ei tarkastusta
		return NFmiColor(0.5f, 0.5f, 0.5f);
	else // ei paramia datassa ja muut
		return NFmiColor(0.5, 1, 1);
}

static std::string GetStatusString(int status)
{
	if(status == 1) // ok
		return "Ok";
	else if(status == 2) // varoitus
		return "Warning";
	else if(status == 3) // virhe
		return "Error";
	else if(status == 4) // ei tarkastusta
		return "No check";
	else // ei paramia datassa ja muut
		return "Param not found";
}

static void DrawStatusButton(Gdiplus::Graphics &theGraphics, const CRect &theParamRect, int status, int timeIndex, bool paramTotal, int timeSize)
{
	NFmiColor frameColor(0, 0, 0);
	NFmiColor fillColor(::GetStatusColor(status));

	Gdiplus::RectF statusButtonRect = ::GetStatusRectF(theParamRect,timeIndex, timeSize, paramTotal);
	Gdiplus::GraphicsPath statusButtonPath;
	::GetRoundedRect(statusButtonRect, 4.f, statusButtonPath);
    CtrlView::DrawPath(theGraphics, statusButtonPath, frameColor, fillColor, true, true, 1);
}

// palauttaa statuksen tarkastetulle datalle.
// 1 jos ok, 2 jos warning ja 3 jos error, 4 jos ei ole tarkastettu ja 5 jos parametria ei ollut datassa
static int CalcParamStatus(const NFmiDataParamCheckingInfo &theCheckingInfo, const NFmiGridValuesCheck *theValuesCheck)
{
	if(theValuesCheck == 0)
		return 4;
	else if(theValuesCheck->ChecksDone() == false)
		return 4;
	else if(theValuesCheck->ParamFound() == false)
		return 5;
	else
	{
		int missStatus = 1;
		if(theValuesCheck->MissingValueProsent() > theCheckingInfo.MissingValueErrorLimit())
			missStatus = 3;
		else if(theValuesCheck->MissingValueProsent() > theCheckingInfo.MissingValueWarningLimit())
			missStatus = 2;

		int minStatus = 1;
		if(theValuesCheck->MinValue() != kFloatMissing)
		{
			if(theValuesCheck->MinValue() < theCheckingInfo.MinValueErrorLimit())
				minStatus = 3;
			else if(theValuesCheck->MinValue() < theCheckingInfo.MinValueWarningLimit())
				minStatus = 2;
		}

		int maxStatus = 1;
		if(theValuesCheck->MaxValue() != kFloatMissing)
		{
			if(theValuesCheck->MaxValue() > theCheckingInfo.MaxValueErrorLimit())
				maxStatus = 3;
			else if(theValuesCheck->MaxValue() > theCheckingInfo.MaxValueWarningLimit())
				maxStatus = 2;
		}
		int status = FmiMax(FmiMax(missStatus, minStatus), maxStatus);
		return status;
	}
}

// decimals 0 eli ei desimaaleja ja muuten yhdell‰ desimaalilla
static std::string GetLonLatString(const NFmiPoint &thePoint, int decimals)
{
	std::string txt("lat ");
	txt += CtrlViewUtils::GetLatitudeMinuteStr(thePoint.Y(), decimals);
	txt += ", lon ";
	txt += CtrlViewUtils::GetLongitudeMinuteStr(thePoint.X(), decimals);
	return txt;
}

static std::string MakeGridValuesCheckString(const NFmiGridValuesCheck *theCheckValues, const NFmiDataParamCheckingInfo &theCheckInfo, bool shortVersion, bool mainStatus)
{
	std::string str = theCheckInfo.CheckedParam().GetName().CharPtr();
	str += ":    ";
	str += "Status: ";
	str += ::GetStatusString(::CalcParamStatus(theCheckInfo, theCheckValues));
	if(theCheckValues)
	{
		if(!shortVersion && !mainStatus)
		{
			str += "\nTime: ";
			str += theCheckValues->MinTime().ToStr("ww HH:mm DD.MM.YYYY\n"); // on sama mik‰ aika (min, max) t‰h‰n‰ otetaan koska kyse on yhden aika-askeleen arvoista
		}
		else if(!shortVersion && mainStatus)
			str += "\n";

		if(!shortVersion)
			str += "\n";
		else
			str += " ";
		str += "Max: ";
		str += NFmiStringTools::Convert(::RoundValue(theCheckValues->MaxValue(), .01f));

		if(!shortVersion && mainStatus)
		{
			str += "\nMaxTime: ";
			str += theCheckValues->MaxTime().ToStr("ww HH:mm DD.MM.YYYY"); // on sama mik‰ aika (min, max) t‰h‰n‰ otetaan koska kyse on yhden aika-askeleen arvoista
		}
		if(!shortVersion)
		{
			str += "\nMax location: ";
			str += ::GetLonLatString(theCheckValues->MaxValueLatlon(), 3);
			str += "\n";
		}

		if(!shortVersion)
			str += "\n";
		else
			str += " ";
		str += "Min: ";
		str += NFmiStringTools::Convert(::RoundValue(theCheckValues->MinValue(), .01f));

		if(!shortVersion && mainStatus)
		{
			str += "\nMinTime: ";
			str += theCheckValues->MinTime().ToStr("ww HH:mm DD.MM.YYYY"); // on sama mik‰ aika (min, max) t‰h‰n‰ otetaan koska kyse on yhden aika-askeleen arvoista
		}
		if(!shortVersion)
		{
			str += "\nMin location: ";
			str += ::GetLonLatString(theCheckValues->MinValueLatlon(), 3);
			str += "\n";
		}

		if(!shortVersion)
			str += "\n";
		else
			str += " ";
		str += "Miss %: ";
		str += NFmiStringTools::Convert(::RoundValue(static_cast<float>(theCheckValues->MissingValueProsent()), .1f));

		if(!shortVersion && theCheckValues->MissingValueLatlon() != NFmiPoint::gMissingLatlon)
		{
			if(mainStatus)
			{
				str += "\n1. Missing Time: ";
				str += theCheckValues->MissTime().ToStr("ww HH:mm DD.MM.YYYY"); // on sama mik‰ aika (min, max) t‰h‰n‰ otetaan koska kyse on yhden aika-askeleen arvoista
			}
			str += "\nFirst missing value location: ";
			str += ::GetLonLatString(theCheckValues->MissingValueLatlon(), 3);
		}

	}
	return str;
}

void CFmiDataQualityCheckerDialog::DoDrawStatistics(void)
{
	DrawBackGround();

	NFmiColor txtColor(0, 0, 0);
	NFmiColor frameColor(0, 0, 0);
	float fontSizeInPixels = 16;

	NFmiDataQualityChecker &dataQualityChecker = itsSmartMetDocumentInterface->DataQualityChecker();
	const auto & dataParamCheckingInfos = dataQualityChecker.DataParamCheckingInfos();
	const auto &combinedParamChecks = dataQualityChecker.CombinedParamChecks();
	const NFmiDataMatrix<NFmiGridValuesCheck> &valueCheckMatrix = dataQualityChecker.ValueCheckMatrix();
	for(int i = 0; i < static_cast<int>(dataParamCheckingInfos.size()); i++)
	{
		CRect paramAreaRect = ::GetParamRect(i, itsDrawRect);
		CtrlView::DrawRect(*itsDrawingGraphics, CtrlView::CRect2GdiplusRect(paramAreaRect), frameColor, frameColor, false, true, 1.f);

		const NFmiGridValuesCheck *checkValues = 0;
		if(i < static_cast<int>(combinedParamChecks.size()))
			checkValues = &(combinedParamChecks[i]);
		std::string statusStr = MakeGridValuesCheckString(checkValues, dataParamCheckingInfos[i], true, true);
		NFmiPoint strPos(fontSizeInPixels * 1.2, paramAreaRect.top);
        CtrlView::DrawSimpleText(*itsDrawingGraphics, txtColor, fontSizeInPixels, statusStr, strPos, gFontNameStr, kTopLeft, Gdiplus::FontStyleBold);

		int status = 4;
		if(i < static_cast<int>(combinedParamChecks.size()))
			status = ::CalcParamStatus(dataParamCheckingInfos[i], &(combinedParamChecks[i]));
		::DrawStatusButton(*itsDrawingGraphics, paramAreaRect, status, 0, true, static_cast<int>(valueCheckMatrix.NY()));
		for(int j = 0; j < static_cast<int>(valueCheckMatrix.NY()); j++)
		{
			status = 4;
			if(i < static_cast<int>(valueCheckMatrix.NX()) && j < static_cast<int>(valueCheckMatrix.NY()))
				status = ::CalcParamStatus(dataParamCheckingInfos[i], &(valueCheckMatrix[i][j]));
			::DrawStatusButton(*itsDrawingGraphics, paramAreaRect, status, j, false, static_cast<int>(valueCheckMatrix.NY()));
		}
	}
}

BOOL CFmiDataQualityCheckerDialog::PreTranslateMessage(MSG* pMsg)
{
	m_tooltip.RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}

std::string CFmiDataQualityCheckerDialog::ComposeToolTipText(CPoint point)
{
	point.y -= itsClienAreaRect.top; 
	Gdiplus::PointF pointF(static_cast<float>(point.x), static_cast<float>(point.y));
	// 1. mink‰ parametrin alueella
	NFmiDataQualityChecker &dataQualityChecker = itsSmartMetDocumentInterface->DataQualityChecker();
	const auto & dataParamCheckingInfos = dataQualityChecker.DataParamCheckingInfos();
	const auto &combinedParamChecks = dataQualityChecker.CombinedParamChecks();
	const NFmiDataMatrix<NFmiGridValuesCheck> &valueCheckMatrix = dataQualityChecker.ValueCheckMatrix();
	for(int i = 0; i < static_cast<int>(dataParamCheckingInfos.size()); i++)
	{
		CRect paramAreaRect = ::GetParamRect(i, itsDrawRect);
		if(paramAreaRect.PtInRect(point))
		{
			const NFmiDataParamCheckingInfo &checkingInfo = dataParamCheckingInfos[i];
			const NFmiGridValuesCheck *valuesCheck = 0;
			bool wasInsideStatusRect = false;
			bool mainStatus = false;
		// 2. onko yhteis status alueella vai aika-kohtaisella
			float firstNormalButtonX = ::GetFirstNormalButtonX(paramAreaRect);
			if(point.x < firstNormalButtonX)
			{ // oli oli parametrin kokooma alueella, onko se laatikossa?
				Gdiplus::RectF statusButtonRect = ::GetStatusRectF(paramAreaRect, 0, static_cast<int>(valueCheckMatrix.NY()), true);
				if(statusButtonRect.Contains(pointF))
				{
					if(i < static_cast<int>(combinedParamChecks.size()))
					{
						valuesCheck = &(combinedParamChecks[i]);
						wasInsideStatusRect = true;
						mainStatus = true;
					}
				}
			}
			else
			{
		// 3. jos aika-kohtaisella, mik‰ aika indeksi
				for(int j = 0; j < static_cast<int>(valueCheckMatrix.NY()); j++)
				{ // k‰yd‰‰n taarvittaessa jokainen aikaan liittyv‰ status nappi l‰pi
					Gdiplus::RectF statusButtonRect = ::GetStatusRectF(paramAreaRect, j, static_cast<int>(valueCheckMatrix.NY()), false);
					if(statusButtonRect.Contains(pointF))
					{
						if(i < static_cast<int>(valueCheckMatrix.NX()) && j < static_cast<int>(valueCheckMatrix.NY()))
						{
							valuesCheck = &(valueCheckMatrix[i][j]);
							wasInsideStatusRect = true;
						}
						break;
					}
				}
			}
		// 4. tee status stringi pitk‰ll‰ kaavalla
			if(wasInsideStatusRect)
			{
				std::string statusStr = MakeGridValuesCheckString(valuesCheck, checkingInfo, false, mainStatus);
				return statusStr;
			}
		}
	}
	return "";
}

void CFmiDataQualityCheckerDialog::NotifyDisplayTooltip(NMHDR * pNMHDR, LRESULT * result)
{
	*result = 0;
	NM_PPTOOLTIP_DISPLAY * pNotify = (NM_PPTOOLTIP_DISPLAY*)pNMHDR;

	if (pNotify->ti->nIDTool == DATA_QUALITY_VIEW_TOOLTIP_ID)
	{
		CPoint pt = *pNotify->pt;
		ScreenToClient(&pt);

        CString strU_;

		try
		{
			strU_ = CA2T(ComposeToolTipText(pt).c_str());
		}
		catch(std::exception &e)
		{
            strU_ = _TEXT("Error while making the tooltip string:\n");
            strU_ += CA2T(e.what());
		}
		catch(...)
		{
            strU_ = _TEXT("Error (unknown) while making the tooltip string");
		}

        pNotify->ti->sTooltip = strU_;

	} //if
} //End NotifyDisplayTooltip

/*
            //draw round rectangle
            RoundRectangle(objG, objP, 100, 200, 200, 100, 10);
        }       

        public void RoundRectangle(Graphics objG, Pen objP, float h, float v, float width, float height, float radius)
        {
            GraphicsPath objGP = new GraphicsPath();
            objGP.AddLine(h + radius, v, h + width - (radius * 2), v); 
            objGP.AddArc(h + width - (radius * 2), v, radius * 2, radius * 2, 270, 90); 
            objGP.AddLine(h + width, v + radius, h + width, v + height - (radius * 2)); 
            objGP.AddArc(h + width - (radius * 2), v + height - (radius * 2), radius * 2, radius * 2, 0, 90); // Corner
            objGP.AddLine(h + width - (radius * 2), v + height, h + radius, v + height); 
            objGP.AddArc(h, v + height - (radius * 2), radius * 2, radius * 2, 90, 90); 
            objGP.AddLine(h, v + height - (radius * 2), h, v + radius); 
            objGP.AddArc(h, v, radius * 2, radius * 2, 180, 90); 
            objGP.CloseFigure();
            objG.DrawPath(objP, objGP);
            objGP.Dispose();
        }

		*/
BOOL CFmiDataQualityCheckerDialog::OnEraseBkgnd(CDC* pDC)
{
	return FALSE; // n‰in piirto ei v‰lky

//	return CDialog::OnEraseBkgnd(pDC);
}
