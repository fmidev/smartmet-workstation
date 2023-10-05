// FmiLoadDataDialog.cpp : implementation file
//
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ k‰‰nt‰j‰n varoitusta
#endif

//#include <afxpriv.h>
#include "stdafx.h"
#include "FmiLoadDataDialog.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiDataLoadingInfo.h"
#include "NFmiFileSystem.h"
#include "NFmiQueryData.h"
#include "NFmiValueString.h"
#include "FmiFilterDataView.h"
#include "NFmiModelProducerIndexViewWCTR.h"
#include "FmiDataLoadingWarningDlg.h"
#include "NFmiDictionaryFunction.h"
#include "FmiWin32Helpers.h"
#include "FmiGdiPlusHelpers.h"
#include "NFmiModelDataBlender.h"
#include "CloneBitmap.h"
#include "CtrlViewGdiPlusFunctions.h"
#include "CtrlViewWin32Functions.h"

#include <fstream>

/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/
/////////////////////////////////////////////////////////////////////////////
// CFmiLoadDataDialog dialog

using namespace std;

const static int ksgMaxProducers = 4;

static const std::string gStartTimePreviousStr = "StartTimePrevious";
static const std::string gStartTimeNextStr = "StartTimeNext";
static const std::string gEndTimePreviousStr = "EndTimePrevious";
static const std::string gEndTimeNextStr = "EndTimeNext";

template <class ForwardIterator>
static void sequence_delete(ForwardIterator first, ForwardIterator last)
{
	while (first != last)
		delete *first++;
}
// ****** HUOM!!!! n‰ist‰ pit‰‰ tehd‰ yleisi‰, nyt on samat jo generaldatadocissa!!!!!!!!!!!!!***********

NFmiControlButtonImageHolder::ButtonData* NFmiControlButtonImageHolder::Find(const std::string &theName)
{
	Container::iterator it = itsButtons.find(theName);
	if(it != itsButtons.end())
		return &(it->second);
	else
		return 0;
}

NFmiControlButtonImageHolder::ButtonData* NFmiControlButtonImageHolder::Find(const NFmiPoint &thePos)
{
	for(Container::iterator it = itsButtons.begin(); it != itsButtons.end(); ++it)
	{
		if(it->second.itsPosition.IsInside(thePos))
			return &(it->second);
	}
	return 0;
}

void NFmiControlButtonImageHolder::Add(const ButtonData &theData)
{
	itsButtons.insert(std::make_pair(theData.itsName, theData));
}

NFmiMetTime CFmiLoadDataDialog::itsCaseStudyStartTime = NFmiMetTime(1900, 0, 0, 0, 0, 0); // can't use NFmiMetTime::gMissingTime in initialization because which static variable gets initialized first?
BOOL CFmiLoadDataDialog::fSetStartTime = FALSE;

CFmiLoadDataDialog::CFmiLoadDataDialog(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent)
:CDialog(CFmiLoadDataDialog::IDD, pParent)
,itsDataLoadingInfo(0)
,itsSmartMetDocumentInterface(smartMetDocumentInterface)
,itsModelProducerView(0)
,itsModelProducerIndexView(0)
,itsQueryInfoVector()
,itsNonModifieableTimeRangeProducerIndex(0)
,itsActiveProducerIndex(0)
,itsSelectedProducerPriorityTable()
,itsProducerColorTable()
,itsProducerButton1Color()
,itsProducerButton2Color()
,itsProducerButton3Color()
,itsProducerButton4Color()
,itsBitmap1(0)
,itsBitmap2(0)
,itsBitmap3(0)
,itsBitmap4(0)
,itsColorRect1()
,itsColorRect2()
,itsColorRect3()
,itsColorRect4()
,itsActivatedProducer(0)
,itsControlButtonImageHolder()
,itsGdiPlusGraphics(0)
, fUseModelBlender(FALSE)
{
	itsDataLoadingInfo = &(itsSmartMetDocumentInterface->GetUsedDataLoadingInfo());

	//{{AFX_DATA_INIT(CFmiLoadDataDialog)
    itsProducer1U_ = _T("");
    itsProducer2U_ = _T("");
    itsProducer3U_ = _T("");
    itsProducer4U_ = _T("");
	itsPrimaryProducer = -1;
	itsSecondaryProducer = -1;
	itsThirdProducer = -1;
	itsEnableInterpolation = -1;
	fRemoveThundersOnLoad = FALSE;
	//}}AFX_DATA_INIT

// v‰rit t‰ytyy m‰‰ritell‰ kahdesti, ensin meid‰n omille systeemeille
	itsProducerColorTable.resize(ksgMaxProducers);
	itsProducerColorTable[0] = NFmiColor(1, 0, 0);
	itsProducerColorTable[1] = NFmiColor(0, 0.85f, .2f);
	itsProducerColorTable[2] = NFmiColor(0.2f, 0.42f, 0.99f);
	itsProducerColorTable[3] = NFmiColor(1, 1, 0); //
// ja sitten: windowsin dialogin napeille
	itsProducerButton1Color = RGB(255, 0, 0);
	itsProducerButton2Color = RGB(0, 216, 51);
	itsProducerButton3Color = RGB(51, 108, 253);
	itsProducerButton4Color = RGB(255, 255, 0);
}

CFmiLoadDataDialog::~CFmiLoadDataDialog(void)
{
	if(itsBitmap1)
		itsBitmap1->DeleteObject();
	delete itsBitmap1;
	if(itsBitmap2)
		itsBitmap2->DeleteObject();
	delete itsBitmap2;
	if(itsBitmap3)
		itsBitmap3->DeleteObject();
	delete itsBitmap3;
	if(itsBitmap4)
		itsBitmap4->DeleteObject();
	delete itsBitmap4;

	delete itsGdiPlusGraphics;
}

void CFmiLoadDataDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CFmiLoadDataDialog)
    DDX_Control(pDX, IDC_BUTTON_ACTIVATEMODEL_4, itsButtonActivateModel4);
    DDX_Control(pDX, IDC_BUTTON_ACTIVATEMODEL_3, itsButtonActivateModel3);
    DDX_Control(pDX, IDC_BUTTON_ACTIVATEMODEL_2, itsButtonActivateModel2);
    DDX_Control(pDX, IDC_BUTTON_ACTIVATEMODEL_1, itsButtonActivateModel1);
    DDX_Control(pDX, IDC_COMBO_own_files, itsOwnFileNames);
    DDX_Control(pDX, IDC_COMBO_official_files, itsOfficialFileNames);
    DDX_Control(pDX, IDC_COMBO_Hirlam_files, itsHirlamFileNames);
    DDX_Control(pDX, IDC_COMBO_ECMWF_files, itsECMWFFileNames);
    DDX_Text(pDX, IDC_EDIT_Producer1, itsProducer1U_);
    DDX_Text(pDX, IDC_EDIT_producer2, itsProducer2U_);
    DDX_Text(pDX, IDC_EDIT_Producer3, itsProducer3U_);
    DDX_Text(pDX, IDC_EDIT_Producer4, itsProducer4U_);
    DDX_Radio(pDX, IDC_RADIO_1_hirlam, itsPrimaryProducer);
    DDX_Radio(pDX, IDC_RADIO_2_hirlam, itsSecondaryProducer);
    DDX_Radio(pDX, IDC_RADIO_3_hirlam, itsThirdProducer);
    DDX_Radio(pDX, IDC_RADIO_NoInterpolation, itsEnableInterpolation);
    DDX_Check(pDX, IDC_CHECK_REMOVE_THUNDERS_ON_LOAD, fRemoveThundersOnLoad);
    //}}AFX_DATA_MAP
    DDX_Check(pDX, IDC_CHECK_USE_MODEL_BLENDING, fUseModelBlender);
    DDX_Control(pDX, IDC_DATETIMEPICKER_CASESTUDY_START_DATE, itsCaseStudyStartTimeDateCtrl);
    DDX_Control(pDX, IDC_DATETIMEPICKER_CASESTUDY_START_TIME, itsCaseStudyStartTimeCtrl);
    DDX_Check(pDX, IDC_CHECK_SET_START_TIME, fSetStartTime);
}


BEGIN_MESSAGE_MAP(CFmiLoadDataDialog, CDialog)
	//{{AFX_MSG_MAP(CFmiLoadDataDialog)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_ACTIVATEMODEL_1, OnUpdateActivateModel1) // oma viritys
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_ACTIVATEMODEL_2, OnUpdateActivateModel2) // oma viritys
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_ACTIVATEMODEL_3, OnUpdateActivateModel3) // oma viritys
	ON_UPDATE_COMMAND_UI(IDC_BUTTON_ACTIVATEMODEL_4, OnUpdateActivateModel4) // oma viritys
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_RADIO_1_hirlam, OnRADIO1hirlam)
	ON_BN_CLICKED(IDC_RADIO_1_official, OnRADIO1official)
	ON_BN_CLICKED(IDC_RADIO_1_own, OnRADIO1own)
	ON_BN_CLICKED(IDC_RADIO_2_ECMWF, OnRadio2Ecmwf)
	ON_BN_CLICKED(IDC_RADIO_2_hirlam, OnRADIO2hirlam)
	ON_BN_CLICKED(IDC_RADIO_2_official, OnRADIO2official)
	ON_BN_CLICKED(IDC_RADIO_2_own, OnRADIO2own)
	ON_BN_CLICKED(IDC_RADIO_3_ECMWF, OnRadio3Ecmwf)
	ON_BN_CLICKED(IDC_RADIO_3_hirlam, OnRADIO3hirlam)
	ON_BN_CLICKED(IDC_RADIO_3_official, OnRADIO3official)
	ON_BN_CLICKED(IDC_RADIO_3_own, OnRADIO3own)
	ON_BN_CLICKED(IDC_RADIO_1_ECMWF, OnRadio1Ecmwf)
	ON_BN_CLICKED(IDC_BUTTON_ACTIVATEMODEL_1, OnButtonActivatemodel1)
	ON_BN_CLICKED(IDC_BUTTON_ACTIVATEMODEL_2, OnButtonActivatemodel2)
	ON_BN_CLICKED(IDC_BUTTON_ACTIVATEMODEL_3, OnButtonActivatemodel3)
	ON_BN_CLICKED(IDC_BUTTON_ACTIVATEMODEL_4, OnButtonActivatemodel4)
	ON_CBN_SELCHANGE(IDC_COMBO_Hirlam_files, OnSelchangeCOMBOHirlamfiles)
	ON_CBN_SELCHANGE(IDC_COMBO_official_files, OnSelchangeCOMBOofficialfiles)
	ON_CBN_SELCHANGE(IDC_COMBO_own_files, OnSelchangeCOMBOownfiles)
	ON_CBN_SELCHANGE(IDC_COMBO_ECMWF_files, OnSelchangeCOMBOECMWFfiles)
	//}}AFX_MSG_MAP
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_BLEND_MAKE_LINEAR, &CFmiLoadDataDialog::OnBnClickedButtonBlendMakeLinear)
	ON_BN_CLICKED(IDC_CHECK_USE_MODEL_BLENDING, &CFmiLoadDataDialog::OnBnClickedCheckUseModelBlending)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_CASESTUDY_START_DATE, &CFmiLoadDataDialog::OnDtnDatetimechangeDatetimepickerCasestudyStartDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_CASESTUDY_START_TIME, &CFmiLoadDataDialog::OnDtnDatetimechangeDatetimepickerCasestudyStartTime)
    ON_BN_CLICKED(IDC_CHECK_SET_START_TIME, &CFmiLoadDataDialog::OnBnClickedCheckSetStartTime)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFmiLoadDataDialog message handlers
void CFmiLoadDataDialog::OnUpdateActivateModel1(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(itsActivatedProducer == 1 ? 1 : 0);
}

void CFmiLoadDataDialog::OnUpdateActivateModel2(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(itsActivatedProducer == 2 ? 1 : 0);
}

void CFmiLoadDataDialog::OnUpdateActivateModel3(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(itsActivatedProducer == 3 ? 1 : 0);
}

void CFmiLoadDataDialog::OnUpdateActivateModel4(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(itsActivatedProducer == 4 ? 1 : 0);
}

BOOL CFmiLoadDataDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	if(!itsDataLoadingInfo)
		return FALSE;

	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	InitDialogTexts();
	SetupTitle(*itsDataLoadingInfo);
	SetupProducerButtons(*itsDataLoadingInfo);
	SetupNameCombos(*itsDataLoadingInfo);
	SetupProducerNames(*itsDataLoadingInfo);
	itsEnableInterpolation = itsDataLoadingInfo->TimeInterpolationSetting();
	CreateModelProducerIndexView();
	DoFullModelProducerIndexViewUpdate();
	InitializeModelBlenderControl();
	InitCaseStudyDateTimeControls();
	auto pDC = GetDC();
	itsGdiPlusGraphics = Gdiplus::Graphics::FromHDC(pDC->GetSafeHdc());
	itsGdiPlusGraphics->SetClip(CtrlView::CRect2GdiplusRect(itsModelBlendCtrlRect));
	ReleaseDC(pDC);
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFmiLoadDataDialog::UpdateCaseStudyStartTime(void)
{
    if(!UseNormalStartTime())
    {
        CTime aDate;
        CTime aTime;
        itsCaseStudyStartTimeDateCtrl.GetTime(aDate);
        itsCaseStudyStartTimeCtrl.GetTime(aTime);
        itsCaseStudyStartTime = NFmiMetTime(
            aDate.GetYear(), aDate.GetMonth(), aDate.GetDay(),
            aTime.GetHour()); // j‰tet‰‰n tarkkuus tahallaan tunti tasolle
        itsDataLoadingInfo->CaseStudyTime(itsCaseStudyStartTime);
    }
	itsDataLoadingInfo->UpdatedTimeDescriptor(!UseNormalStartTime());
	DoFullModelProducerIndexViewUpdate();
	InvalidateBlendControl();
}

void CFmiLoadDataDialog::InitCaseStudyTimePickers(void)
{
    if(itsCaseStudyStartTime == NFmiMetTime::gMissingTime)
    {
        if(itsDataLoadingInfo->CaseStudyTime() != NFmiMetTime::gMissingTime)
            itsCaseStudyStartTime = itsDataLoadingInfo->CaseStudyTime(); // Jos itsDataLoadingInfo:ssa ollut aika ei ollut puuttuva, otetaan se k‰yttˆˆn
        else
            itsCaseStudyStartTime = NFmiMetTime(60); // otataan sein‰kello start-timeksi, jos muualta ei saa sopivaa arvoa
    }
    CTime aTime(itsCaseStudyStartTime.GetYear(), itsCaseStudyStartTime.GetMonth(), itsCaseStudyStartTime.GetDay(), itsCaseStudyStartTime.GetHour(), 0, 0);
    itsCaseStudyStartTimeDateCtrl.SetTime(&aTime);
    itsCaseStudyStartTimeCtrl.SetTime(&aTime);
    UpdateData(FALSE);
}

bool CFmiLoadDataDialog::UseNormalStartTime()
{
    return itsSmartMetDocumentInterface->CaseStudyModeOn() == false && !fSetStartTime;
}

void CFmiLoadDataDialog::InitCaseStudyDateTimeControls(void)
{
    if(UseNormalStartTime())
	{ // aijan s‰‰tˆ‰ ei sallita kun ollaan normaali moodissa (vain case-study moodissa on haluttavaa s‰‰t‰‰ alkuajan hetke‰)
		itsCaseStudyStartTimeDateCtrl.ShowWindow(SW_HIDE);
		itsCaseStudyStartTimeCtrl.ShowWindow(SW_HIDE);
	}
	else
	{
        itsCaseStudyStartTimeDateCtrl.ShowWindow(SW_SHOW);
        itsCaseStudyStartTimeCtrl.ShowWindow(SW_SHOW);
        if(itsSmartMetDocumentInterface->Language() == kFinnish)
			itsCaseStudyStartTimeDateCtrl.SetFormat(_TEXT("dd.MM.yyyy"));
		else
            itsCaseStudyStartTimeDateCtrl.SetFormat(_TEXT("dd/MM/yyyy"));
		InitCaseStudyTimePickers();
	}
}

void CFmiLoadDataDialog::InitializeModelBlenderControl(void)
{
    auto &modelDataBlender = itsSmartMetDocumentInterface->ModelDataBlender();
    fUseModelBlender = modelDataBlender.Use();
	itsModelBlendCtrlRect = CalcModelBlendingControlRect();
	CalcModelBlendScaleRect();
    modelDataBlender.InitWithNewTimes(itsDataLoadingInfo->UpdatedTimeDescriptor(!UseNormalStartTime()), modelDataBlender.GetStartTimeHour(), modelDataBlender.GetEndTimeHour());
	InitializeBlenderButtons();
	itsModelProducerIndexView->SelectSecondProducerFromThisTimeOn(static_cast<int>(modelDataBlender.GetEndTimeIndex()));
}

void CFmiLoadDataDialog::SetupTitle(NFmiDataLoadingInfo& theDataLoadingInfo)
{
	CWnd* win = GetDlgItem(IDC_STATIC_TITLE);
	if(win)
	{
		NFmiString text(" ");
		text += theDataLoadingInfo.Producer().GetName();
		text += " ";
		text += theDataLoadingInfo.AreaIdentifier();
		text += " ";
		NFmiValueString valuStr(theDataLoadingInfo.ForecastingLength(), "%d");
		text += valuStr;
		text += "h";
		win->SetWindowText(CA2T(text));
	}
}

void CFmiLoadDataDialog::CreateModelProducerIndexView(void)
{
	CRect rect;
	CWnd* staticWnd = GetDlgItem(IDC_STATIC_TIME_SELECT_VIEW);
	WINDOWPLACEMENT wplace;
	if(staticWnd)
		staticWnd->GetWindowPlacement(&wplace);
	rect = wplace.rcNormalPosition;

	rect.InflateRect(-1,-1,-1,-1); // 'hieno' upotus tulee n‰kyviin

	itsModelProducerView = new CFmiFilterDataView(0, this, itsSmartMetDocumentInterface, 1, NFmiCtrlView::kModelProducerIndexViewWCTR);
	if(itsModelProducerView)
	{
		itsModelProducerView->Create(NULL,NULL, WS_VISIBLE | WS_CHILD, rect, this, NULL);
		itsModelProducerView->OnInitialUpdate(); // pit‰‰ kutsua erikseen, koska formvieta ei ole sidottu dokumenttiin

		NFmiCtrlView* zoomView = itsModelProducerView->ZoomView();
		if(zoomView)
		{
			itsModelProducerIndexView = static_cast<NFmiModelProducerIndexViewWCTR*>(zoomView);
		}
	}
}

void CFmiLoadDataDialog::DoFullModelProducerIndexViewUpdate()
{
	if(itsModelProducerIndexView)
	{
		UpdateQueryInfoVector();

		itsNonModifieableTimeRangeProducerIndex = 3; // pit‰isikˆ jotenkin tallettaa tieto johonkin eik‰ hardcodata
		itsActiveProducerIndex = 0;
		itsSelectedProducerPriorityTable.resize(3);
		itsSelectedProducerPriorityTable[0] = itsPrimaryProducer;
		itsSelectedProducerPriorityTable[1] = itsSecondaryProducer;
		itsSelectedProducerPriorityTable[2] = itsThirdProducer;

		itsModelProducerIndexView->SetDataFromDialog(itsQueryInfoVector,
													 itsDataLoadingInfo->UpdatedTimeDescriptor(!UseNormalStartTime()),
													 itsSelectedProducerPriorityTable,
													 itsProducerColorTable,
													 itsActiveProducerIndex);
		itsModelProducerIndexView->SelectSecondProducerFromThisTimeOn(static_cast<int>(itsSmartMetDocumentInterface->ModelDataBlender().GetEndTimeIndex()));
		itsModelProducerView->Invalidate(FALSE);
	}
}

void CFmiLoadDataDialog::UpdateQueryInfoVector(void)
{
	itsQueryInfoVector.clear();
	itsQueryInfoVector.resize(ksgMaxProducers);
	std::vector<string> names(GetSelectedFileNames());
	for(int i = 0; i < ksgMaxProducers; i++)
		itsQueryInfoVector[i] = ReadInfoFromFile(names[i]);
}

std::vector<string> CFmiLoadDataDialog::GetSelectedFileNames(void)
{
	std::vector<string> names;
	string tmp;
	if(itsDataLoadingInfo)
	{
		tmp = itsDataLoadingInfo->Model1Path();
		tmp += GetSelectedNameFromCombo(itsHirlamFileNames);
		names.push_back(tmp);

		tmp = itsDataLoadingInfo->Model2Path();
		tmp += GetSelectedNameFromCombo(itsECMWFFileNames);
		names.push_back(tmp);

		tmp = itsDataLoadingInfo->WorkingPath();
		tmp += GetSelectedNameFromCombo(itsOwnFileNames);
		names.push_back(tmp);

		tmp = itsDataLoadingInfo->DataBaseInPath();
		tmp += GetSelectedNameFromCombo(itsOfficialFileNames);
		names.push_back(tmp);
	}
	return names;
}

boost::shared_ptr<NFmiQueryInfo> CFmiLoadDataDialog::ReadInfoFromFile(const string& theFileName)
{
	if(NFmiFileSystem::FileExists(theFileName) == false)
		return boost::shared_ptr<NFmiQueryInfo>();

	string compareFileName(theFileName);
	NFmiStringTools::LowerCase(compareFileName);
	// Luetaan querydata-tiedoston alusta info-osio
	ifstream in(theFileName.c_str(), ios::binary);
	if(in)
	{
		boost::shared_ptr<NFmiQueryInfo> info(new NFmiQueryInfo());
		if(info)
		{
			try
			{
				in >> *info;
				return info;
			}
			catch(...)
			{
				info = boost::shared_ptr<NFmiQueryInfo>(); // tyhjennet‰‰n info pois kokonaan ennen palautusta
				return info;
			}
		}
	}
	return boost::shared_ptr<NFmiQueryInfo>(); // tyhj‰n palautus virhe tilanteissa
}

void CFmiLoadDataDialog::SetupProducerButtons(NFmiDataLoadingInfo& theDataLoadingInfo)
{
	//radionappulat
	itsPrimaryProducer = theDataLoadingInfo.PrimaryProducerSetting();
	itsSecondaryProducer = theDataLoadingInfo.SecondaryProducerSetting();
	itsActivatedProducer = itsSecondaryProducer + 1;
	itsThirdProducer = theDataLoadingInfo.ThirdProducerSetting();

	//  v‰rinappulat
 	itsButtonActivateModel1.GetWindowRect(itsColorRect1);
	itsBitmap1 = CtrlView::CreateColorBitmap(itsColorRect1.Width() - 6, itsColorRect1.Height() - 6, NFmiColorButtonDrawingData(this, itsProducerButton1Color, itsColorRect1));
	itsButtonActivateModel1.SetBitmap(*itsBitmap1);

 	itsButtonActivateModel2.GetWindowRect(itsColorRect2);
	itsBitmap2 = CtrlView::CreateColorBitmap(itsColorRect2.Width() - 6, itsColorRect2.Height() - 6, NFmiColorButtonDrawingData(this, itsProducerButton2Color, itsColorRect2));
	itsButtonActivateModel2.SetBitmap(*itsBitmap2);

 	itsButtonActivateModel3.GetWindowRect(itsColorRect3);
	itsBitmap3 = CtrlView::CreateColorBitmap(itsColorRect3.Width() - 6, itsColorRect3.Height() - 6, NFmiColorButtonDrawingData(this, itsProducerButton3Color, itsColorRect3));
	itsButtonActivateModel3.SetBitmap(*itsBitmap3);

 	itsButtonActivateModel4.GetWindowRect(itsColorRect4);
	itsBitmap4 = CtrlView::CreateColorBitmap(itsColorRect4.Width() - 6, itsColorRect4.Height() - 6, NFmiColorButtonDrawingData(this, itsProducerButton4Color, itsColorRect4));
	itsButtonActivateModel4.SetBitmap(*itsBitmap4);
}

void CFmiLoadDataDialog::SetupNameCombos(NFmiDataLoadingInfo& theDataLoadingInfo)
{	//tiedostojen nimivalikot
	theDataLoadingInfo.InitHirlamFileNameList();
	theDataLoadingInfo.InitEcmwfFileNameList();
	theDataLoadingInfo.InitWorkingFileNameList();
	theDataLoadingInfo.InitDataBaseInFileNameList1();

	HirlamFileNames(theDataLoadingInfo);
	EcmwfFileNames(theDataLoadingInfo);
	WorkingFileNames(theDataLoadingInfo);
	DataBaseFileNames1(theDataLoadingInfo);
}

void CFmiLoadDataDialog::SetupProducerNames(NFmiDataLoadingInfo& theDataLoadingInfo)
{
		//tuottaja-nimi tekstit
    itsProducer1U_ = CA2T(theDataLoadingInfo.Model1NameInDialog());
    itsProducer2U_ = CA2T(theDataLoadingInfo.Model2NameInDialog());
    itsProducer3U_ = CA2T(theDataLoadingInfo.WorkingNameInDialog());
    itsProducer4U_ = CA2T(theDataLoadingInfo.DataBaseNameInDialog1());
}

void CFmiLoadDataDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnLButtonDown(nFlags, point);
}

// Mik‰ on puuttuva indeksi? Sovitaan -1
size_t CFmiLoadDataDialog::FindClosestTimeIndex(const CPoint &point)
{
	size_t index = gMissingIndex;
	int closestDist = 999999999;
	auto &modelDataBlender = itsSmartMetDocumentInterface->ModelDataBlender();
	for(size_t i = modelDataBlender.GetStarTimeIndex(); i <= modelDataBlender.GetEndTimeIndex(); i++)
	{
		int xLocInPixels = CalcForecastHourXLocation(modelDataBlender.GetForecastHour(i));
		int currentDist = ::abs(xLocInPixels - point.x);
		if(currentDist < closestDist)
		{
			closestDist = currentDist;
			index = i;
		}
	}
	return index;
}

float CFmiLoadDataDialog::CalcBlendFactor(const CPoint &point)
{
	if(point.y < itsModelBlendScaleRect.top)
		return 1.f;
	else if(point.y > itsModelBlendScaleRect.bottom)
		return 0.f;
	else
	{
		float factor = (1.f - (point.y - itsModelBlendScaleRect.top)/static_cast<float>(itsModelBlendScaleRect.Height()));
		return factor;
	}
}

void CFmiLoadDataDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
    auto &modelDataBlender = itsSmartMetDocumentInterface->ModelDataBlender();
	NFmiControlButtonImageHolder::ButtonData *bData = itsControlButtonImageHolder.Find(NFmiPoint(point.x, point.y));
	if(bData)
	{
		bool endTimeMoved = false;
		if(bData->itsName == gStartTimePreviousStr)
			modelDataBlender.MoveStartTime(kBackward);
		else if(bData->itsName == gStartTimeNextStr)
			modelDataBlender.MoveStartTime(kForward);
		else if(bData->itsName == gEndTimePreviousStr)
		{
			modelDataBlender.MoveEndTime(kBackward);
			endTimeMoved = true;
		}
		else if(bData->itsName == gEndTimeNextStr)
		{
			modelDataBlender.MoveEndTime(kForward);
			endTimeMoved = true;
		}

		if(endTimeMoved)
			itsModelProducerIndexView->SelectSecondProducerFromThisTimeOn(static_cast<int>(modelDataBlender.GetEndTimeIndex()));

		itsSmartMetDocumentInterface->ModelDataBlender().MakeBlendLinear(); // laitetaan aina myˆs kertoimet lineaarisiksi, kun alku/loppu aikaa s‰‰det‰‰n....
		InvalidateBlendControl();
	}
	else if(itsModelBlendScaleInflatedRect.PtInRect(point))
	{
		size_t nearestTimeIndex = FindClosestTimeIndex(point);
		if(nearestTimeIndex != gMissingIndex)
		{
			float factor = CalcBlendFactor(point);
			modelDataBlender.SetBlendFactor(nearestTimeIndex, factor);
			InvalidateBlendControl();
		}
	}
	else
		CDialog::OnLButtonUp(nFlags, point);
}

void CFmiLoadDataDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnMouseMove(nFlags, point);
}

void CFmiLoadDataDialog::OnRADIO1hirlam()
{
	int oldValue = itsPrimaryProducer;
	UpdateData(TRUE);
	if(oldValue != itsPrimaryProducer)
		DoFullModelProducerIndexViewUpdate();
}

void CFmiLoadDataDialog::OnRADIO1official()
{
	int oldValue = itsPrimaryProducer;
	UpdateData(TRUE);
	if(oldValue != itsPrimaryProducer)
		DoFullModelProducerIndexViewUpdate();
}

void CFmiLoadDataDialog::OnRADIO1own()
{
	int oldValue = itsPrimaryProducer;
	UpdateData(TRUE);
	if(oldValue != itsPrimaryProducer)
		DoFullModelProducerIndexViewUpdate();
}

void CFmiLoadDataDialog::OnRadio1Ecmwf()
{
	int oldValue = itsPrimaryProducer;
	UpdateData(TRUE);
	if(oldValue != itsPrimaryProducer)
		DoFullModelProducerIndexViewUpdate();
}

void CFmiLoadDataDialog::OnRadio2Ecmwf()
{
	int oldValue = itsSecondaryProducer;
	UpdateData(TRUE);
	if(oldValue != itsSecondaryProducer)
		DoFullModelProducerIndexViewUpdate();
}

void CFmiLoadDataDialog::OnRADIO2hirlam()
{
	int oldValue = itsSecondaryProducer;
	UpdateData(TRUE);
	if(oldValue != itsSecondaryProducer)
		DoFullModelProducerIndexViewUpdate();
}

void CFmiLoadDataDialog::OnRADIO2official()
{
	int oldValue = itsSecondaryProducer;
	UpdateData(TRUE);
	if(oldValue != itsSecondaryProducer)
		DoFullModelProducerIndexViewUpdate();
}

void CFmiLoadDataDialog::OnRADIO2own()
{
	int oldValue = itsSecondaryProducer;
	UpdateData(TRUE);
	if(oldValue != itsSecondaryProducer)
		DoFullModelProducerIndexViewUpdate();
}

void CFmiLoadDataDialog::OnRadio3Ecmwf()
{
	int oldValue = itsThirdProducer;
	UpdateData(TRUE);
	if(oldValue != itsThirdProducer)
		DoFullModelProducerIndexViewUpdate();
}

void CFmiLoadDataDialog::OnRADIO3hirlam()
{
	int oldValue = itsThirdProducer;
	UpdateData(TRUE);
	if(oldValue != itsThirdProducer)
		DoFullModelProducerIndexViewUpdate();
}

void CFmiLoadDataDialog::OnRADIO3official()
{
	int oldValue = itsThirdProducer;
	UpdateData(TRUE);
	if(oldValue != itsThirdProducer)
		DoFullModelProducerIndexViewUpdate();
}

void CFmiLoadDataDialog::OnRADIO3own()
{
	int oldValue = itsThirdProducer;
	UpdateData(TRUE);
	if(oldValue != itsThirdProducer)
		DoFullModelProducerIndexViewUpdate();
}

void CFmiLoadDataDialog::OnOK()
{
	UpdateData(TRUE);
	if(DoSetup()) //talletetan tiedot infoon
		CDialog::OnOK();
}

// muutettu palauttamaan falsea, jos k‰ytt‰j‰ haluaa viel‰ perua latauksen n‰hty‰‰n
// mahdollisen varoitus dialogin (esim. KEPA lataa dataa vain Hirlamista)
BOOL CFmiLoadDataDialog::DoSetup()
{
	//radionappulat
	itsDataLoadingInfo->PrimaryProducerSetting(itsPrimaryProducer);
	itsDataLoadingInfo->SecondaryProducerSetting(itsSecondaryProducer);
	itsDataLoadingInfo->ThirdProducerSetting(itsThirdProducer);

    CString temp1U_;

	itsDataLoadingInfo->HirlamFileNameListIndex(itsHirlamFileNames.GetCurSel());
	if(itsDataLoadingInfo->HirlamFileNameList().Index(itsDataLoadingInfo->HirlamFileNameListIndex()))
	{
		itsHirlamFileNames.GetLBText(itsHirlamFileNames.GetCurSel(), temp1U_);
        NFmiString tmp = CT2A(temp1U_);
		itsDataLoadingInfo->ComboHirlamFileName(tmp);
	}

	itsDataLoadingInfo->EcmwfFileNameListIndex(itsECMWFFileNames.GetCurSel());
	if(itsDataLoadingInfo->EcmwfFileNameList().Index(itsDataLoadingInfo->EcmwfFileNameListIndex()))
	{
        itsECMWFFileNames.GetLBText(itsECMWFFileNames.GetCurSel(), temp1U_);
        NFmiString tmp = CT2A(temp1U_);
		itsDataLoadingInfo->ComboEcmwfFileName(tmp);
	}

	itsDataLoadingInfo->WorkingFileNameListIndex(itsOwnFileNames.GetCurSel());
	if(itsDataLoadingInfo->WorkingFileNameList().Index(itsDataLoadingInfo->WorkingFileNameListIndex()))
	{
        itsOwnFileNames.GetLBText(itsOwnFileNames.GetCurSel(), temp1U_);
        NFmiString tmp = CT2A(temp1U_);
		itsDataLoadingInfo->ComboWorkingFileName(tmp);
	}

	itsDataLoadingInfo->DataBaseFileNameListIndex1(itsOfficialFileNames.GetCurSel());
	if(itsDataLoadingInfo->DataBaseInFileNameList1().Index(itsDataLoadingInfo->DataBaseFileNameListIndex1()))
	{
        itsOfficialFileNames.GetLBText(itsOfficialFileNames.GetCurSel(), temp1U_);
        NFmiString tmp = CT2A(temp1U_);
		itsDataLoadingInfo->ComboDataBase1FileName(tmp);
	}

	//toiset radionappulat
	itsDataLoadingInfo->TimeInterpolationSetting(itsEnableInterpolation);

	if(!itsModelProducerIndexView->IsProducerIndexInTimeVectorFilled())
	{
		if(itsSmartMetDocumentInterface->SmartMetEditingMode() == CtrlViewUtils::kFmiEditingModeNormal) // jos ollaan ns. view-moodissa, disabloidaan varoitus dialogi
		{
			CFmiDataLoadingWarningDlg dlg;
			if(dlg.DoModal() == IDCANCEL)
				return FALSE;
		}
	}

	itsSmartMetDocumentInterface->SetDataLoadingProducerIndexVector(itsModelProducerIndexView->ProducerIndexInTimeVector());

	return TRUE;
}

void CFmiLoadDataDialog::HirlamFileNames(NFmiDataLoadingInfo& theDataLoadingInfo)
{
	NFmiSortedStringList& hirlamFileNameList = theDataLoadingInfo.HirlamFileNameList();
	if(hirlamFileNameList.Reset())
	{
		do
		{
			NFmiString* temp = hirlamFileNameList.Current();
			itsHirlamFileNames.InsertString(-1, CA2T(*temp));
		} while(hirlamFileNameList.Next());

		string apuFileFilter(theDataLoadingInfo.Model1FilePattern());
		string latestFileName;
		NFmiFileSystem::FindFile(apuFileFilter, true, &latestFileName);
        int index = itsHirlamFileNames.FindString(0, CA2T(latestFileName.c_str()));
		if(index != CB_ERR)
			itsHirlamFileNames.SetCurSel(index);
	}
	return;
}

void CFmiLoadDataDialog::EcmwfFileNames(NFmiDataLoadingInfo& theDataLoadingInfo)
{
	NFmiSortedStringList& ecmwfFileNameList = theDataLoadingInfo.EcmwfFileNameList();
	if(ecmwfFileNameList.Reset())
	{
		do
		{
            itsECMWFFileNames.InsertString(-1, CA2T(*ecmwfFileNameList.Current()));
		} while(ecmwfFileNameList.Next());

		string apuFileFilter(theDataLoadingInfo.Model2FilePattern());
		string latestFileName;
		NFmiFileSystem::FindFile(apuFileFilter, true, &latestFileName);
        int index = itsECMWFFileNames.FindString(0, CA2T(latestFileName.c_str()));

		if(index != CB_ERR)
			itsECMWFFileNames.SetCurSel(index);
	}
	return;
}

void CFmiLoadDataDialog::WorkingFileNames(NFmiDataLoadingInfo& theDataLoadingInfo)
{
	NFmiSortedStringList& workingFileNameList = theDataLoadingInfo.WorkingFileNameList();
	if(workingFileNameList.Reset())
	{
		do
		{
            itsOwnFileNames.InsertString(-1, CA2T(*workingFileNameList.Current()));
		} while(workingFileNameList.Next());

		string apuFileFilter(theDataLoadingInfo.CreateWorkingFileNameFilter());
		string latestFileName;
		NFmiFileSystem::FindFile(apuFileFilter, true, &latestFileName);
        int index = itsOwnFileNames.FindString(0, CA2T(latestFileName.c_str()));

		if(index != CB_ERR)
			itsOwnFileNames.SetCurSel(index);
	}
	return;
}

void CFmiLoadDataDialog::DataBaseFileNames1(NFmiDataLoadingInfo& theDataLoadingInfo)
{
	itsOfficialFileNames.Clear(); //laura lis‰si 8.12.99
	NFmiSortedStringList& dataBaseFileNameList = theDataLoadingInfo.DataBaseInFileNameList1();
	if(dataBaseFileNameList.Reset())
	{
		do
		{
            itsOfficialFileNames.InsertString(-1, CA2T(*dataBaseFileNameList.Current()));
		} while(dataBaseFileNameList.Next());

		string apuFileFilter(theDataLoadingInfo.CreateDataBaseInFileNameFilter(1));
		string latestFileName;
		NFmiFileSystem::FindFile(apuFileFilter, true, &latestFileName);
        int index = itsOfficialFileNames.FindString(0, CA2T(latestFileName.c_str()));

		if(index != CB_ERR)
			itsOfficialFileNames.SetCurSel(index);
	}
	return;
}

string CFmiLoadDataDialog::GetSelectedNameFromCombo(CComboBox& theNameCombo)
{
    CString temp1U_;
	int index = theNameCombo.GetCurSel();
	if(index >= 0)
	{
        theNameCombo.GetLBText(theNameCombo.GetCurSel(), temp1U_);
        return string(CT2A(temp1U_));
	}
	else
		return string();
}

void CFmiLoadDataDialog::OnButtonActivatemodel1()
{
	itsActivatedProducer = 1;
	SetActiveProducerToView();
}

void CFmiLoadDataDialog::OnButtonActivatemodel2()
{
	itsActivatedProducer = 2;
	SetActiveProducerToView();
}

void CFmiLoadDataDialog::OnButtonActivatemodel3()
{
	itsActivatedProducer = 3;
	SetActiveProducerToView();
}

void CFmiLoadDataDialog::OnButtonActivatemodel4()
{
	itsActivatedProducer = 4;
	SetActiveProducerToView();
}

void CFmiLoadDataDialog::SetActiveProducerToView(void)
{
	if(itsModelProducerIndexView)
		itsModelProducerIndexView->ActiveProducerIndex(itsActivatedProducer-1); // -1 shift teht‰v‰, koska indeksi alkaa t‰‰ll‰ 1:st‰ ja viewssa 0:sta!!!!
}

void CFmiLoadDataDialog::OnSelchangeCOMBOHirlamfiles()
{
	DoFullModelProducerIndexViewUpdate();
}

void CFmiLoadDataDialog::OnSelchangeCOMBOofficialfiles()
{
	DoFullModelProducerIndexViewUpdate();
}

void CFmiLoadDataDialog::OnSelchangeCOMBOownfiles()
{
	DoFullModelProducerIndexViewUpdate();
}

void CFmiLoadDataDialog::OnSelchangeCOMBOECMWFfiles()
{
	DoFullModelProducerIndexViewUpdate();
}

// T‰m‰ funktio alustaa kaikki dialogin tekstit editoriin valitulla kielell‰.
// T‰m‰ on ik‰v‰ kyll‰ teht‰v‰ erikseen dialogin muokkaus tyˆkalusta, eli
// tekij‰n pit‰‰ lis‰t‰ erikseen t‰nne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CFmiLoadDataDialog::InitDialogTexts(void)
{
    SetWindowText(CA2T(::GetDictionaryString("DataLoadingDlgTitle").c_str()));

	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "IDOK");
	CFmiWin32Helpers::SetDialogItemText(this, IDCANCEL, "IDCANCEL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_FileNames, "IDC_STATIC_FileNames");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DATA_LOADING_PRIORIZATION_STR, "IDC_STATIC_DATA_LOADING_PRIORIZATION_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_DATA_LOADING_MODEL_ACTIVATION_STR, "IDC_STATIC_DATA_LOADING_MODEL_ACTIVATION_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TimeInterpolation, "IDC_STATIC_TimeInterpolation");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_REMOVE_THUNDERS_ON_LOAD, "IDC_CHECK_REMOVE_THUNDERS_ON_LOAD");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_NoInterpolation, "IDC_RADIO_NoInterpolation");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_Interpolation, "IDC_RADIO_Interpolation");
    CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_SET_START_TIME, "Set start time");
}

static void DrawLine(CDC * dc, int x1, int y1, int x2, int y2)
{
	dc->MoveTo(x1, y1);
	dc->LineTo(x2, y2);
}

static NFmiControlButtonImageHolder::ButtonData MakeButtonData(const std::string &theName, const NFmiRect &theButtonPosition, const std::string &theImageFileName, const std::string &theImageFolder)
{
	NFmiControlButtonImageHolder::ButtonData bData;
	bData.itsName = theName;
	bData.itsImageFileName = theImageFolder + "\\" + theImageFileName;
	bData.itsPosition = theButtonPosition;
	try
	{
		bData.itsImage = CtrlView::CreateBitmapFromFile(theImageFolder, theImageFileName);
	}
	catch(...)
	{
		// ehk‰ imagea ei ollut, jatketaan 0-pointterilla
		bData.itsImage = 0; // pit‰isi olla jo, mutta varmistetaan 0-pointteri
	}

	return bData;
}

// n‰ist‰ pit‰‰ p‰‰st‰ joskus eroon, ett‰ nappulat voisivat olla mink‰ kokoisia tahansa
static const int gButtonSizeX = 16; 
static const int gButtonSizeY = 16;

void CFmiLoadDataDialog::InitializeBlenderButtons(void)
{
	std::string bitmapFolder = itsSmartMetDocumentInterface->HelpDataPath() + "\\res";
	itsControlButtonImageHolder.Add(::MakeButtonData(gStartTimePreviousStr, CalcStartTimePreviousButtonRect(), "control_reverse_play.png", bitmapFolder));
	itsControlButtonImageHolder.Add(::MakeButtonData(gStartTimeNextStr, CalcStartTimeNextButtonRect(), "control_play.png", bitmapFolder));
	itsControlButtonImageHolder.Add(::MakeButtonData(gEndTimePreviousStr, CalcEndTimePreviousButtonRect(), "control_reverse_play.png", bitmapFolder));
	itsControlButtonImageHolder.Add(::MakeButtonData(gEndTimeNextStr, CalcEndTimeNextButtonRect(), "control_play.png", bitmapFolder));
}

NFmiRect CFmiLoadDataDialog::CalcStartTimePreviousButtonRect(void)
{
	UINT topLeftX = itsModelBlendScaleRect.left - gButtonSizeX;
	UINT topLeftY = itsModelBlendScaleRect.top - 32;
	NFmiRect rect = NFmiRect(topLeftX, topLeftY, topLeftX + gButtonSizeX, topLeftY + gButtonSizeY);
	return rect;
}

NFmiRect CFmiLoadDataDialog::CalcStartTimeNextButtonRect(void)
{
	NFmiRect prevRect = CalcStartTimePreviousButtonRect();

	NFmiRect rect(prevRect);
	rect.Left(prevRect.Right());
	rect.Size(prevRect.Size());
	return rect;
}

NFmiRect CFmiLoadDataDialog::CalcEndTimePreviousButtonRect(void)
{
	NFmiRect prevRect = CalcStartTimePreviousButtonRect();

	NFmiRect rect(prevRect);
	rect.Left(itsModelBlendScaleRect.right - gButtonSizeX);
	rect.Size(prevRect.Size());
	return rect;
}

NFmiRect CFmiLoadDataDialog::CalcEndTimeNextButtonRect(void)
{
	NFmiRect prevRect = CalcEndTimePreviousButtonRect();

	NFmiRect rect(prevRect);
	rect.Left(prevRect.Right());
	rect.Size(prevRect.Size());
	return rect;
}

void CFmiLoadDataDialog::CalcModelBlendScaleRect(void)
{
    int startTimeX = static_cast<int>(itsModelBlendCtrlRect.left + ::round(itsModelBlendCtrlRect.Width() / 15.));
    int endTimeX = static_cast<int>(itsModelBlendCtrlRect.right - ::round(itsModelBlendCtrlRect.Width() / 15.));
    int lineBaseY = static_cast<int>(itsModelBlendCtrlRect.bottom - ::round(itsModelBlendCtrlRect.Height() / 7.));
    int lineTopY = static_cast<int>(itsModelBlendCtrlRect.top + ::round(itsModelBlendCtrlRect.Height() / 3.));
	itsModelBlendScaleRect = CRect(startTimeX, lineTopY, endTimeX, lineBaseY);

	// Lasketaan lis‰ksi pumpattu hiiren klikkaus alue
	itsModelBlendScaleInflatedRect = itsModelBlendScaleRect;
	itsModelBlendScaleInflatedRect.InflateRect(10, 10); // levitet‰‰n klikkaus aluetta 10 pikselia reunojen yli
}

// yksikkˆ on pikseleiss‰
int CFmiLoadDataDialog::CalcForecastHourXLocation(int theHour)
{
    auto &modelDataBlender = itsSmartMetDocumentInterface->ModelDataBlender();
	int axelLenInHours = modelDataBlender.GetEndTimeHour() - modelDataBlender.GetStartTimeHour();
	float hourLenInPixels = itsModelBlendScaleRect.Width() / static_cast<float>(axelLenInHours);
    int xLocInPixels = static_cast<int>(::round(itsModelBlendScaleRect.left + hourLenInPixels * (theHour - modelDataBlender.GetStartTimeHour())));
	return xLocInPixels;
}

static std::string GetForecastHourStr(int theHour)
{
	std::string str = "+";
	str += NFmiStringTools::Convert(theHour);
	str += "h";
	return str;
}

static std::string GetForecastTimeStampStr(const NFmiMetTime &theTime, FmiLanguage theLanguage)
{
	NFmiString timeFormat = "ww HH utc";
	if(theLanguage != kFinnish)
		timeFormat = "www HH utc";
	return theTime.ToStr(timeFormat, theLanguage).CharPtr();
}

static const std::wstring gFontNameStr(L"Arial");

void CFmiLoadDataDialog::DrawModelBlendingControl(CDC * dc)
{
	// T‰st‰ vanhasta GDI piirrosta siirrytt‰v‰ puhtaaseen GDI+ piirtoihin!!!!
	CPen linePen(PS_SOLID, 0, RGB(0, 0, 0));
	CPen markerPen(PS_SOLID, 0, RGB(255, 0, 0));
	CPen* pOldPen = dc->SelectObject(&linePen);

    auto &modelDataBlender = itsSmartMetDocumentInterface->ModelDataBlender();

	NFmiColor txtColor(0, 0, 0);
	NFmiColor fillColor(1, 1, 1);
	if(modelDataBlender.Use() == false)
		fillColor = NFmiColor(0.7f, 0.7f, 0.7f);

	CtrlView::DrawRect(*itsGdiPlusGraphics, CtrlView::CRect2GdiplusRect(itsModelBlendCtrlRect), txtColor, fillColor, true, true, 1.f);

	// piirret‰‰n pohja viiva
	::DrawLine(dc, itsModelBlendScaleRect.left, itsModelBlendScaleRect.bottom, itsModelBlendScaleRect.right, itsModelBlendScaleRect.bottom);

	// piirret‰‰n pysty viivat jokaiseen blendattavan aika-askeleen kohdalle levitettyn‰
	// koko asteikon pituudelle ja erotettuna oikean pituisilla aika-askelilla (tunti on x-suuntainen yksikkˆ)
	for(size_t i = modelDataBlender.GetStarTimeIndex(); i <= modelDataBlender.GetEndTimeIndex(); i++)
	{
		dc->SelectObject(&linePen);
		int xLocInPixels = CalcForecastHourXLocation(modelDataBlender.GetForecastHour(i));
		::DrawLine(dc, xLocInPixels, itsModelBlendScaleRect.bottom, xLocInPixels, itsModelBlendScaleRect.top);

		DrawFactorLabel(i);

		dc->SelectObject(&markerPen);
        int markerLocY = static_cast<int>(::round(itsModelBlendScaleRect.bottom - (modelDataBlender.GetBlendFactor(i) * itsModelBlendScaleRect.Height())));
		dc->Rectangle(xLocInPixels-2, markerLocY-2, xLocInPixels+2, markerLocY+2); 
	}
	dc->SelectObject(&linePen);

	// piirret‰‰n viel‰ alku ja loppu tunnit
	int hourstrMoveY = 10;
	float fontsizeInPixels = 12;

	// ensin forecast hour asteikon yl‰osaan
	std::string str1 = ::GetForecastHourStr(modelDataBlender.GetStartTimeHour());
	NFmiPoint strPos(itsModelBlendScaleRect.left, itsModelBlendScaleRect.top - hourstrMoveY);
    CtrlView::DrawSimpleText(*itsGdiPlusGraphics, txtColor, fontsizeInPixels, str1, strPos, gFontNameStr, kCenter);
	// sitten aikaleima asteikon alaosaan
    auto language = itsSmartMetDocumentInterface->Language();
	std::string timeStampStr1 = ::GetForecastTimeStampStr(modelDataBlender.GetStartTime(), language);
	NFmiPoint strPos2(itsModelBlendCtrlRect.left, itsModelBlendCtrlRect.bottom - hourstrMoveY + 2);
    CtrlView::DrawSimpleText(*itsGdiPlusGraphics, txtColor, fontsizeInPixels, timeStampStr1, strPos2, gFontNameStr, kLeft);


	// piirr‰ alkuajan s‰‰tˆ napit
	DrawButton(gStartTimePreviousStr);
	DrawButton(gStartTimeNextStr);
	DrawButton(gEndTimePreviousStr);
	DrawButton(gEndTimeNextStr);

	// ensin forecast hour asteikon yl‰osaan
	std::string str2 = ::GetForecastHourStr(modelDataBlender.GetEndTimeHour());
	strPos.X(itsModelBlendScaleRect.right);
    CtrlView::DrawSimpleText(*itsGdiPlusGraphics, txtColor, fontsizeInPixels, str2, strPos, gFontNameStr, kCenter);
	// sitten aikaleima asteikon alaosaan
	std::string timeStampStr2 = ::GetForecastTimeStampStr(modelDataBlender.GetEndTime(), language);
	strPos2.X(itsModelBlendCtrlRect.right);
    CtrlView::DrawSimpleText(*itsGdiPlusGraphics, txtColor, fontsizeInPixels, timeStampStr2, strPos2, gFontNameStr, kRight);

	dc->SelectObject(pOldPen);
}

// laskee blandaus kertoimen (0-1) avulla y-position pikseleiss‰ blendaus akselistossa
int CFmiLoadDataDialog::CalcFactorLocation(float theFactor)
{
    int locY = static_cast<int>(::round(itsModelBlendScaleRect.bottom - (theFactor * itsModelBlendScaleRect.Height())));
	return locY;
}

void CFmiLoadDataDialog::DrawFactorLabel(size_t theIndex)
{
    auto &modelDataBlender = itsSmartMetDocumentInterface->ModelDataBlender();
	NFmiColor txtColor(0, 0, 0);
	float fontsizeInPixels = 10;
	float factor = modelDataBlender.GetBlendFactor(theIndex);
	std::string labelStr = NFmiStringTools::Convert(::round(factor*100));
	int xLocInPixels = CalcForecastHourXLocation(modelDataBlender.GetForecastHour(theIndex));
	xLocInPixels += 3;
	int yLocInPixels = CalcFactorLocation(modelDataBlender.GetBlendFactor(theIndex));
	NFmiPoint strPos(xLocInPixels, yLocInPixels);
    CtrlView::DrawSimpleText(*itsGdiPlusGraphics, txtColor, fontsizeInPixels, labelStr, strPos, gFontNameStr, kLeft);
}

void CFmiLoadDataDialog::DrawButton(const std::string &theName)
{
	NFmiControlButtonImageHolder::ButtonData *bData = itsControlButtonImageHolder.Find(theName);
	if(bData)
	{
		CRect crect(static_cast<int>(bData->itsPosition.Left()), static_cast<int>(bData->itsPosition.Top()), static_cast<int>(bData->itsPosition.Right()), static_cast<int>(bData->itsPosition.Bottom()));
        CtrlView::DrawImageButton(crect, bData->itsImage, itsGdiPlusGraphics, 1.f, itsSmartMetDocumentInterface->Printing());
	}
}

void CFmiLoadDataDialog::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialog::OnPaint() for painting messages
	DrawModelBlendingControl(&dc);
}

CRect CFmiLoadDataDialog::CalcModelBlendingControlRect(void)
{
	CRect rect;
	GetClientRect(rect);
	CWnd* rightLimiter = GetDescendantWindow(IDC_CHECK_USE_MODEL_BLENDING);
	CWnd* topLimiter = GetDescendantWindow(IDC_STATIC_TimeInterpolation);
	if(rightLimiter && topLimiter)
	{
		const int offset = 5;
		WINDOWPLACEMENT wplace;
		BOOL bull = rightLimiter->GetWindowPlacement(&wplace);
		CRect rightRect = wplace.rcNormalPosition;
		bull = topLimiter->GetWindowPlacement(&wplace);
		CRect topRect = wplace.rcNormalPosition;
		rect.top = topRect.bottom + offset;
		rect.left = rect.left + offset;
		rect.right = rightRect.left - offset;
		rect.bottom = rect.bottom - offset;
	}
	return rect;
}

void CFmiLoadDataDialog::OnBnClickedButtonBlendMakeLinear()
{
	itsSmartMetDocumentInterface->ModelDataBlender().MakeBlendLinear();
	InvalidateBlendControl();
}

void CFmiLoadDataDialog::OnBnClickedCheckUseModelBlending()
{
	UpdateData(TRUE);
	itsSmartMetDocumentInterface->ModelDataBlender().Use(fUseModelBlender == TRUE);
	InvalidateBlendControl();
}

void CFmiLoadDataDialog::InvalidateBlendControl(void)
{
	InvalidateRect(itsModelBlendCtrlRect);
	itsModelProducerView->Invalidate(FALSE); // myˆs t‰t‰ pit‰‰ p‰ivitt‰‰, kun blendaus juttuja p‰ivitet‰‰n
}

void CFmiLoadDataDialog::OnDtnDatetimechangeDatetimepickerCasestudyStartDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);

	// TODO: Add your control notification handler code here
	UpdateCaseStudyStartTime();

	*pResult = 0;
}

void CFmiLoadDataDialog::OnDtnDatetimechangeDatetimepickerCasestudyStartTime(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);

	// TODO: Add your control notification handler code here
	UpdateCaseStudyStartTime();

	*pResult = 0;
}


void CFmiLoadDataDialog::OnBnClickedCheckSetStartTime()
{
    UpdateData(TRUE);

    if(!fSetStartTime)
        itsDataLoadingInfo->CaseStudyTime(NFmiMetTime::gMissingTime); // T‰m‰ caseStudy aika pit‰‰ resetoida, kun aikas‰‰timet piilotetaan, muuten alkuaika otetaan CaseStudyTime:esta.
    InitCaseStudyDateTimeControls();
    UpdateCaseStudyStartTime();
    Invalidate(FALSE);
}
