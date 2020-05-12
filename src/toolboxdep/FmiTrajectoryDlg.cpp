// FmiTrajectoryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FmiTrajectoryDlg.h"
#include "FmiTrajectoryView.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiTrajectorySystem.h"
#include "NFmiTrajectory.h"
#include "NFmiProducerName.h"
#include "FmiTempBalloonSettingsDlg.h"
#include "FmiWin32TemplateHelpers.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiProducerSystem.h"
#include "NFmiMapViewDescTop.h"
#include "NFmiInfoOrganizer.h"
#include "FmiTrajectorySaveDlg.h"
#include "FmiWin32Helpers.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiApplicationWinRegistry.h"
#include "CloneBitmap.h"
#include "persist2.h"
#include "ApplicationInterface.h"

// CFmiTrajectoryDlg dialog

const NFmiViewPosRegistryInfo CFmiTrajectoryDlg::s_ViewPosRegistryInfo(CRect(500, 300, 970, 800), "\\TrajectoryView");

IMPLEMENT_DYNAMIC(CFmiTrajectoryDlg, CDialog)
CFmiTrajectoryDlg::CFmiTrajectoryDlg(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent)
	: CDialog(CFmiTrajectoryDlg::IDD, pParent)
	,itsView(0)
	,itsSmartMetDocumentInterface(smartMetDocumentInterface)
	,itsTooltipCtrl()
	, itsSelectedDataType(1)
	, fUsePlumes(FALSE)
	, itsTrajectoryDirection(0)
	, fShowTrajectoryArrows(FALSE)
	, fShowTrajectoryAnimationMarkers(FALSE)
    , itsTrajectoryConstantVerticalVelocityStrU_(_T(""))
	, fTrajectoryIsentropic(FALSE)
	, fShowTrajectoriesInCrossSectionView(FALSE)
	, fCalcTempBalloonTrajectories(FALSE)
	, fProducerSelectorUsedYet(false)
{
}

CFmiTrajectoryDlg::~CFmiTrajectoryDlg()
{
}

void CFmiTrajectoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_TRAJECTORY_DATA_TYPE1, itsSelectedDataType);
	DDX_Control(pDX, IDC_SPIN_TRAJECTORY_TIME_STEP, itsTimeStepSpinCtrl);
	DDX_Control(pDX, IDC_SPIN_TRAJECTORY_TIME_LENGTH, itsTimeLengthSpinCtrl);
	DDX_Control(pDX, IDC_SPIN_TRAJECTORY_PROB_FACTOR, itsProbFactorSpinCtrl);
	DDX_Control(pDX, IDC_SPIN_TRAJECTORY_PARTICLE_COUNT, itsParticleCountSpinCtrl);
	DDX_Check(pDX, IDC_CHECK_TRAJECTORY_USE_PLUMES, fUsePlumes);
	DDX_Control(pDX, IDC_SPIN_TRAJECTORY_START_DEVIATION_LOCATION, itsStartLocationRangeSpinCtrl);
	DDX_Control(pDX, IDC_SPIN_TRAJECTORY_START_DEVIATION_TIME, itsStartTimeRangeSpinCtrl);
	DDX_Control(pDX, IDC_SPIN_TRAJECTORY_START_DEVIATION_PRESSURE_LEVEL, itsStartPressureLevelRangeSpinCtrl);
	DDX_Radio(pDX, IDC_RADIO_TRAJECTORY_DIRECTION_FORWARD, itsTrajectoryDirection);
	DDX_Check(pDX, IDC_CHECK_TRAJECTORY_SHOW_ARROWS, fShowTrajectoryArrows);
	DDX_Check(pDX, IDC_CHECK_TRAJECTORY_SHOW_ANIMATION_MARKERS, fShowTrajectoryAnimationMarkers);
    DDX_Text(pDX, IDC_EDIT_TRAJECTORY_CONSTANT_VERTICAL_MOTION, itsTrajectoryConstantVerticalVelocityStrU_);
	DDX_Control(pDX, IDC_SPIN_TRAJECTORY_CONSTANT_VERTICAL_MOTION, itsTrajectoryConstantVerticalVelocitySpinCtrl);
	DDX_Check(pDX, IDC_CHECK_TRAJECTORY_ISENTROPIC, fTrajectoryIsentropic);
	DDX_Check(pDX, IDC_CHECK_TRAJECTORY_SHOW_IN_CROSS_SECTION_VIEW, fShowTrajectoriesInCrossSectionView);
	DDX_Check(pDX, IDC_CHECK_TRAJECTORY_TEMP_BALLOON, fCalcTempBalloonTrajectories);
	DDX_Control(pDX, IDC_COMBO_TRAJECTORY_PRODUCER_SELECTOR, itsProducerSelector);
}


BEGIN_MESSAGE_MAP(CFmiTrajectoryDlg, CDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_TRAJECTORY_ADD, OnBnClickedButtonTrajectoryAdd)
	ON_BN_CLICKED(IDC_BUTTON_TRAJECTORY_CLEAR, OnBnClickedButtonTrajectoryClear)
	ON_BN_CLICKED(IDC_BUTTON_TRAJECTORY_SET_ALL, OnBnClickedButtonTrajectorySetAll)
	ON_BN_CLICKED(IDC_BUTTON_TRAJECTORY_RECALCULATE, OnBnClickedButtonTrajectoryRecalculate)
	ON_BN_CLICKED(IDC_BUTTON_TRAJECTORY_SET_LAST, OnBnClickedButtonTrajectorySetLast)
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_CHECK_TRAJECTORY_SHOW_ARROWS, OnBnClickedCheckTrajectoryShowArrows)
	ON_BN_CLICKED(IDC_CHECK_TRAJECTORY_SHOW_ANIMATION_MARKERS, OnBnClickedCheckTrajectoryShowAnimationMarkers)
	ON_BN_CLICKED(IDC_CHECK_TRAJECTORY_SHOW_IN_CROSS_SECTION_VIEW, OnBnClickedCheckTrajectoryShowInCrossSectionView)
	ON_BN_CLICKED(IDC_BUTTON_TRAJECTORY_TEMP_BALLOON_SETTINGS, OnBnClickedButtonTrajectoryConstantSpeedSettings)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_TRAJECTORY, OnBnClickedButtonSaveTrajectory)
	ON_CBN_SELCHANGE(IDC_COMBO_TRAJECTORY_PRODUCER_SELECTOR, OnCbnSelchangeComboTrajectoryProducerSelector)
    ON_BN_CLICKED(IDC_RADIO_TRAJECTORY_DATA_TYPE1, &CFmiTrajectoryDlg::OnBnClickedRadioTrajectoryDataType1)
    ON_BN_CLICKED(IDC_RADIO_TRAJECTORY_DATA_TYPE2, &CFmiTrajectoryDlg::OnBnClickedRadioTrajectoryDataType2)
    ON_BN_CLICKED(IDC_RADIO_TRAJECTORY_DATA_TYPE3, &CFmiTrajectoryDlg::OnBnClickedRadioTrajectoryDataType3)
    ON_BN_CLICKED(IDC_RADIO_TRAJECTORY_DATA_TYPE4, &CFmiTrajectoryDlg::OnBnClickedRadioTrajectoryDataType4)
END_MESSAGE_MAP()


// CFmiTrajectoryDlg message handlers

// pit‰‰ p‰‰tell‰ tuottaja id:n perusteella mik‰ on halutun dadio buttonin indeksi (alkaa 0:sta)
static int GetModelRadioButtonIndex(NFmiProducerSystem &theProducerSystem, const NFmiProducer &theProducer)
{
	unsigned int prodSystemIndex_1_based = theProducerSystem.FindProducerInfo(theProducer);
	if(prodSystemIndex_1_based == 0) // 0 = ei lˆytynyt, vika ohjelmassa, valitaan -1
		return -1;
	else
		return prodSystemIndex_1_based - 1; // pit‰‰ v‰hent‰‰ 1-pohjaisesta indeksist‰ 1, koska radio buttonien indeksit alkavat 0:sta
}

static const int kMinProbValue = 0;
static const int kMaxProbValue = 50;

static const int kMinTimeStep = 1;
static const int kMaxTimeStep = 360;

static const int kMinTimeLength = 1;
static const int kMaxTimeLength = 120;

static const int kMinParticleCount = 2;
static const int kMaxParticleCount = 1000;

static const int kMinStartLocationRange = 0;
static const int kMaxStartLocationRange = 200;

static const int kMinStartTimeRange = 0;
static const int kMaxStartTimeRange = 360;

static const int kMinStartPressureLevelRange = 0;
static const int kMaxStartPressureLevelRange = 150;

static const int kMinConstantVerticalVelocity = -1000;
static const int kMaxConstantVerticalVelocity = 1000;

BOOL CFmiTrajectoryDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CFmiWin32Helpers::SetUsedWindowIconDynamically(this);
	std::string errorBaseStr("Error in CFmiTrajectoryDlg::OnInitDialog while reading dialog size and position values");
    CFmiWin32TemplateHelpers::DoWindowSizeSettingsFromWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), this, false, errorBaseStr, 0);

    CRect rect(CalcClientArea());
	itsView = new CFmiTrajectoryView(this, itsSmartMetDocumentInterface);
	itsView->Create(NULL,NULL, WS_VISIBLE | WS_CHILD, rect, this, NULL);
	itsView->OnInitialUpdate(); // pit‰‰ kutsua erikseen, koska formvieta ei ole sidottu dokumenttiin

	InitFromTrajectorySystem();

	InitDialogTexts();
	InitTooltipControl();

	CWnd *saveButton = GetDlgItem(IDC_BUTTON_SAVE_TRAJECTORY);
	if(saveButton)
	{
		if(itsSmartMetDocumentInterface->TrajectorySystem()->TrajectorySaveEnabled() == false)
			saveButton->EnableWindow(FALSE);
	}

	CWnd *win = GetDlgItem(IDC_RADIO_TRAJECTORY_DATA_TYPE1);
	if(win)
		win->ShowWindow(FALSE); // piilotetaan toistaiseksi ohjelmallisesti pinta datan valinta radio buttoni


	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// HUOM! muutos tuottajalistan tekoon. Nyt sinne kelpuutetaan ainoastaa ne, joille lˆytyy vertikaali dataa!
void CFmiTrajectoryDlg::UpdateProducerList(void)
{
	int lastSelectedProducer = itsProducerSelector.GetCurSel();
    CString lastProdNameU_;
	if(lastSelectedProducer >= 0)
        itsProducerSelector.GetLBText(lastSelectedProducer, lastProdNameU_);

	// T‰ytet‰‰n ensin potentiaalisten tuottajien lista
	itsProducerList.clear();
	// editoitu tuottaja laitetaan listaan ensin, jos on editoitua dataa
	if(itsSmartMetDocumentInterface->EditedSmartInfo() && itsSmartMetDocumentInterface->EditedSmartInfo()->SizeLevels() > 1)
		itsProducerList.push_back(NFmiProducerHelperInfo(::GetDictionaryString("StationDataTableViewEditedProducerName"), kFmiMETEOR, NFmiInfoData::kEditable));

	// loput tuottajat otetaan ProducerSystemist‰
	int modelCount = static_cast<int>(itsSmartMetDocumentInterface->ProducerSystem().Producers().size());
	int i=0;
	for(i=0; i<modelCount; i++)
	{
		if(itsSmartMetDocumentInterface->ProducerSystem().Producer(i+1).HasRealVerticalData())
			itsProducerList.push_back(NFmiProducerHelperInfo::MakeProducerHelperInfo(itsSmartMetDocumentInterface->ProducerSystem().Producer(i+1), NFmiInfoData::kViewable)); // +1 johtuu producersystemin 1-pohjaisesta indeksi systeemist‰
	}

	// T‰ytet‰‰n sitten lista tuottajista, jotka todellakin lˆytyiv‰t
	itsProducerSelector.ResetContent();
	int ssize = static_cast<int>(itsProducerList.size());
	for(int i=0; i<ssize; i++)
	{ // hae mit‰ tahansa dataa kyseiselt‰ tuottajalta
		if(itsProducerList[i].itsProducerId == kFmiMETEOR)
		{ // inhaa, GetInfos-metodi ei toimikaan editoitavalle datalle, jos kyseess‰ on hila-dataa, enk‰ nyt uskalla muuttaa metodin toimintaa
            boost::shared_ptr<NFmiFastQueryInfo> info = itsSmartMetDocumentInterface->InfoOrganizer()->FindInfo(NFmiInfoData::kEditable);
            if(info && NFmiInfoOrganizer::HasGoodParamsForSoundingData(info, NFmiInfoOrganizer::ParamCheckFlags(false, true)))
				itsProducerSelector.AddString(CA2T(itsProducerList[i].itsName.c_str()));
		}
		else
		{
            boost::shared_ptr<NFmiFastQueryInfo> info = itsSmartMetDocumentInterface->InfoOrganizer()->FindSoundingInfo(NFmiProducer(itsProducerList[i].itsProducerId), 0, NFmiInfoOrganizer::ParamCheckFlags(false, true));
			if(info && NFmiInfoOrganizer::HasGoodParamsForSoundingData(info, NFmiInfoOrganizer::ParamCheckFlags(false, true)))
				itsProducerSelector.AddString(CA2T(itsProducerList[i].itsName.c_str()));
		}
	}

	if(fProducerSelectorUsedYet == false)
	{ // 5. jos k‰ytt‰j‰ ei ole viel‰ k‰ynyt s‰‰t‰m‰ss‰ tuottaja listaa, voidaan asettaa oletus tuottajaksi ensi viimeksi
		// talletettu tuottaja, jos lˆytyy. Sitten tuottaja nro. 2 (1. on edited ja 2. on 1. malli-tuottaja). Muuten 1. tai ei mit‰‰n.
		bool producerSelected = false;
		const NFmiProducer &prod = itsSmartMetDocumentInterface->TrajectorySystem()->SelectedProducer();
		for(int j=0; j < itsProducerSelector.GetCount(); j++)
		{
            CString labelStrU_;
            itsProducerSelector.GetLBText(j, labelStrU_);
            if(labelStrU_ == CA2T(prod.GetName()))
			{
				itsProducerSelector.SetCurSel(j); // laitetaan synop oletus tuottajaksi, koska se kiinnostaa eniten ja on nopein
				producerSelected = true;
				break;
			}
		}
		if(producerSelected == false)
		{
			if(itsProducerSelector.GetCount() > 1)
				itsProducerSelector.SetCurSel(1); // laitetaan 1. malli tuottajaksi
			else if(itsProducerSelector.GetCount() == 1)
				itsProducerSelector.SetCurSel(0); // laitetaan editoitu data tuottajaksi
			else
				itsProducerSelector.SetCurSel(-1); // ei ole mit‰‰, mit‰ laittaa tuottajaksi
		}
	}
	else
	{ // Laita viimeksi valitut taas p‰‰lle listasta
		itsProducerSelector.SetCurSel(-1);
		for(int j=0; j<itsProducerSelector.GetCount(); j++)
		{
            CString tmpNameStrU_;
            itsProducerSelector.GetLBText(j, tmpNameStrU_);
            if(lastProdNameU_ == tmpNameStrU_)
			{
				itsProducerSelector.SetCurSel(j);
				break;
			}
		}
	}

	// MUISTA MY÷S PƒIVITTƒƒ mink‰ tyyppist‰ dataa kyseiselt‰ tuottajalta lˆytyy radio buttonit
	EnableModelAndTypeControls();
}

void CFmiTrajectoryDlg::InitFromTrajectorySystem(void)
{
	UpdateProducerList();
    auto trajectorySystem = itsSmartMetDocumentInterface->TrajectorySystem();
	itsSelectedDataType = trajectorySystem->SelectedDataType();
	if(itsSelectedDataType == 0)
		itsSelectedDataType = 1;
	itsTimeStepSpinCtrl.SetRange(kMinTimeStep, kMaxTimeStep);
	itsTimeStepSpinCtrl.SetPos(trajectorySystem->SelectedTimeStepInMinutes());
	itsTimeLengthSpinCtrl.SetRange(kMinTimeLength, kMaxTimeLength);
	itsTimeLengthSpinCtrl.SetPos(trajectorySystem->SelectedTimeLengthInHours());
	itsProbFactorSpinCtrl.SetRange(kMinProbValue, kMaxProbValue);
	itsProbFactorSpinCtrl.SetPos(static_cast<int>(trajectorySystem->SelectedPlumeProbFactor()));
	itsParticleCountSpinCtrl.SetRange(kMinParticleCount, kMaxParticleCount);
	itsParticleCountSpinCtrl.SetPos(trajectorySystem->SelectedPlumeParticleCount());
	fUsePlumes = trajectorySystem->PlumesUsed();
	itsStartLocationRangeSpinCtrl.SetRange(kMinStartLocationRange, kMaxStartLocationRange);
	itsStartLocationRangeSpinCtrl.SetPos(static_cast<int>(trajectorySystem->SelectedStartLocationRangeInKM()));
	itsStartTimeRangeSpinCtrl.SetRange(kMinStartTimeRange, kMaxStartTimeRange);
	itsStartTimeRangeSpinCtrl.SetPos(static_cast<int>(trajectorySystem->SelectedStartTimeRangeInMinutes()));
	itsStartPressureLevelRangeSpinCtrl.SetRange(kMinStartPressureLevelRange, kMaxStartPressureLevelRange);
	itsStartPressureLevelRangeSpinCtrl.SetPos(static_cast<int>(trajectorySystem->SelectedStartPressureLevelRange()));
	itsTrajectoryDirection = (trajectorySystem->SelectedDirection() == kForward) ? 0 : 1;
	fShowTrajectoryArrows = trajectorySystem->ShowTrajectoryArrows();
	fShowTrajectoryAnimationMarkers = trajectorySystem->ShowTrajectoryAnimationMarkers();
	itsTrajectoryConstantVerticalVelocitySpinCtrl.SetRange(kMinConstantVerticalVelocity, kMaxConstantVerticalVelocity);
	fTrajectoryIsentropic = trajectorySystem->SelectedTrajectoryIsentropic();
	fCalcTempBalloonTrajectories = trajectorySystem->CalcTempBalloonTrajectors();

	UpdateData(FALSE);
}

CRect CFmiTrajectoryDlg::CalcClientArea(void)
{
	CRect rect;
	GetClientRect(rect);
	CWnd* win = GetDescendantWindow(IDC_STATIC_TRAJECTORY_PLUME_SETTINGS);
	if(win)
	{
		WINDOWPLACEMENT wplace;
		win->GetWindowPlacement(&wplace);
		CRect lowRect = wplace.rcNormalPosition;
		rect.top = lowRect.bottom + 3;
	}
	return rect;
}

void CFmiTrajectoryDlg::Update()
{
    if(IsWindowVisible() && !IsIconic()) // N‰yttˆ‰ p‰ivitet‰‰n vain jos se on n‰kyviss‰ ja se ei ole minimized tilassa
    {
        if(itsView)
			itsView->Update(true);

		// t‰m‰ on saatettu s‰‰t‰‰ poikkileikkaus n‰ytˆst‰ p‰‰lle/pois
		UpdateData(TRUE);
		fShowTrajectoriesInCrossSectionView = itsSmartMetDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView();
		UpdateData(FALSE);

		UpdateProducerList();

		Invalidate(FALSE);
	}
}

// Oletus: selectedModelNumber on validi ja jo tarkastettu.
void CFmiTrajectoryDlg::EnableDataTypeRadioControls(const std::string &theSelectedProducerNameStr, int selectedDataType, int checkedDataTypeNumber, int controlID, bool &selectedDataTypeWasEnabled, int &firstEnabledDataType)
{
	boost::shared_ptr<NFmiTrajectory> trajectoryInfo(boost::shared_ptr<NFmiTrajectory>(new NFmiTrajectory()));
	NFmiProducer prod;
	for(size_t i = 0; i < itsProducerList.size(); i++)
	{
		if(itsProducerList[i].itsName == theSelectedProducerNameStr)
		{
			prod = NFmiProducer(itsProducerList[i].itsProducerId, theSelectedProducerNameStr);
			break;
		}
	}
	trajectoryInfo->Producer(prod);
	trajectoryInfo->DataType(checkedDataTypeNumber);
	if(itsSmartMetDocumentInterface->TrajectorySystem()->GetWantedInfo(trajectoryInfo))
	{
		GetDlgItem(controlID)->EnableWindow(TRUE);
		if(selectedDataType == checkedDataTypeNumber)
			selectedDataTypeWasEnabled = true;
		if(firstEnabledDataType < 1)
			firstEnabledDataType = checkedDataTypeNumber;
	}
	else
		GetDlgItem(controlID)->EnableWindow(FALSE);
}

void CFmiTrajectoryDlg::EnableModelAndTypeControls(void)
{
	UpdateData(TRUE);

	int currentSelectedModel = itsProducerSelector.GetCurSel();
	if(currentSelectedModel >= 0)
	{
        CString selectedProducerNameTmpU_;
        itsProducerSelector.GetLBText(currentSelectedModel, selectedProducerNameTmpU_);
		bool selectedDataTypeWasEnabled = false;
		int firstEnabledDataType = -1;
        std::string selectedProducerNameStr = CT2A(selectedProducerNameTmpU_);

		// Ei k‰ytet‰ pintadataa miss‰‰n olosuhteessa, eli disabloidaan se radio-buttoni
		GetDlgItem(IDC_RADIO_TRAJECTORY_DATA_TYPE1)->EnableWindow(FALSE);
		EnableDataTypeRadioControls(selectedProducerNameStr, itsSelectedDataType, 1, IDC_RADIO_TRAJECTORY_DATA_TYPE2, selectedDataTypeWasEnabled, firstEnabledDataType);
		EnableDataTypeRadioControls(selectedProducerNameStr, itsSelectedDataType, 2, IDC_RADIO_TRAJECTORY_DATA_TYPE3, selectedDataTypeWasEnabled, firstEnabledDataType);
		EnableDataTypeRadioControls(selectedProducerNameStr, itsSelectedDataType, 3, IDC_RADIO_TRAJECTORY_DATA_TYPE4, selectedDataTypeWasEnabled, firstEnabledDataType);

		if(selectedDataTypeWasEnabled == false && firstEnabledDataType >= 1) // firstEnabledDataType >= 1 merkitsee ett‰ pinta dataa ei valita automaattisesti (jos se on ainoa lˆydetty data tyyppi)
			itsSelectedDataType = firstEnabledDataType;
	}
	else
	{
		itsSelectedDataType = 1;
		GetDlgItem(IDC_RADIO_TRAJECTORY_DATA_TYPE1)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_TRAJECTORY_DATA_TYPE2)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_TRAJECTORY_DATA_TYPE3)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_TRAJECTORY_DATA_TYPE4)->EnableWindow(FALSE);
	}

	UpdateData(FALSE);
}

void CFmiTrajectoryDlg::DoWhenClosing(void)
{
	UpdateTrajectorySystem();
    itsSmartMetDocumentInterface->TrajectorySystem()->TrajectoryViewOn(false);
    itsSmartMetDocumentInterface->MapViewDirty(0, false, true, true, false, false, false);
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("TrajectoryDlg: Closing view");
	AfxGetMainWnd()->SetActiveWindow(); // aktivoidaan karttan‰yttˆ eli mainframe
}

void CFmiTrajectoryDlg::OnClose()
{
	DoWhenClosing();

	CDialog::OnClose();
}

void CFmiTrajectoryDlg::OnBnClickedOk()
{
	DoWhenClosing();
	OnOK();
}

static int GetLimitedValue(int value, int theMin, int theMax)
{
	if(value < theMin)
		return theMin;
	else if(value > theMax)
		return theMax;
	return value;
}

static double GetValueFromString(const std::string &theStr)
{
	if(theStr.empty())
		return kFloatMissing;
	else
	{
		try
		{
			double value =  NFmiStringTools::Convert<double>(theStr);
			return value;
		}
		catch(std::exception & /* e */)
		{
			return kFloatMissing;
		}
	}
}

static std::string GetSelectedProducerName(const CComboBox &theProducerSelector)
{
	int index = theProducerSelector.GetCurSel();
    CString tmpNameU_;
	if(index >= 0)
        theProducerSelector.GetLBText(index, tmpNameU_);
    std::string selectedNameStr = CT2A(tmpNameU_);
	return selectedNameStr;
}

static const NFmiProducerHelperInfo* GetProducerInfoByName(const std::vector<NFmiProducerHelperInfo> &theProducerList, const std::string &theProdName)
{
	for(size_t i = 0; i < theProducerList.size(); i++)
	{
		if(theProducerList[i].itsName == theProdName)
			return &theProducerList[i];
	}
	return 0;
}

void CFmiTrajectoryDlg::UpdateTrajectorySystem(void)
{
	UpdateData(TRUE);

	std::string selectedProdName = ::GetSelectedProducerName(itsProducerSelector);
	const NFmiProducerHelperInfo *prodInfo = ::GetProducerInfoByName(itsProducerList, selectedProdName);
	NFmiProducer prod((prodInfo == 0) ? 0 : prodInfo->itsProducerId); // jos ei oltu mit‰‰n valittuna, on 0-tuottaja id 'tyhj‰'
	prod.SetName(selectedProdName);
    auto trajectorySystem = itsSmartMetDocumentInterface->TrajectorySystem();
    if(fProducerSelectorUsedYet)
    { 
        // P‰ivitet‰‰n n‰it‰ osia vain, jos k‰ytt‰j‰ on k‰ynyt tekem‰ss‰ jotain s‰‰tˆj‰ (ja avannut trajektori dialogin, kun on ollut dataa k‰ytˆss‰). 
        // N‰m‰ menev‰t konffitiedostoihin talteen ja jos k‰ytt‰j‰ ei ole avannut trajektori n‰yttˆ‰, n‰iss‰ on nyt luultavasti tyhj‰‰, eik‰ sellaisia 
        // arvoja haluta talteen konffeihin.
        trajectorySystem->SelectedProducer(prod);
        trajectorySystem->SelectedDataType(itsSelectedDataType);
    }
	int timeStep = GetLimitedValue(itsTimeStepSpinCtrl.GetPos(), kMinTimeStep, kMaxTimeStep);
	itsTimeStepSpinCtrl.SetPos(timeStep);
    trajectorySystem->SelectedTimeStepInMinutes(timeStep);
	int timeLength = GetLimitedValue(itsTimeLengthSpinCtrl.GetPos(), kMinTimeLength, kMaxTimeLength);
	itsTimeLengthSpinCtrl.SetPos(timeLength);
    trajectorySystem->SelectedTimeLengthInHours(timeLength);
	int probFactor = GetLimitedValue(itsProbFactorSpinCtrl.GetPos(), kMinProbValue, kMaxProbValue);
	itsProbFactorSpinCtrl.SetPos(probFactor);
    trajectorySystem->SelectedPlumeProbFactor(probFactor);
	int particleCount = GetLimitedValue(itsParticleCountSpinCtrl.GetPos(), kMinParticleCount, kMaxParticleCount);
	itsParticleCountSpinCtrl.SetPos(particleCount);
    trajectorySystem->SelectedPlumeParticleCount(particleCount);
    trajectorySystem->PlumesUsed(fUsePlumes == TRUE);
	int startLocRange = GetLimitedValue(itsStartLocationRangeSpinCtrl.GetPos(), kMinStartLocationRange, kMaxStartLocationRange);
	itsStartLocationRangeSpinCtrl.SetPos(startLocRange);
    trajectorySystem->SelectedStartLocationRangeInKM(startLocRange);
	int startTimeRange = GetLimitedValue(itsStartTimeRangeSpinCtrl.GetPos(), kMinStartTimeRange, kMaxStartTimeRange);
	itsStartTimeRangeSpinCtrl.SetPos(startTimeRange);
    trajectorySystem->SelectedStartTimeRangeInMinutes(startTimeRange);
	int startPressureLevelRange = GetLimitedValue(itsStartPressureLevelRangeSpinCtrl.GetPos(), kMinStartPressureLevelRange, kMaxStartPressureLevelRange);
	itsStartPressureLevelRangeSpinCtrl.SetPos(startPressureLevelRange);
    trajectorySystem->SelectedStartPressureLevelRange(startPressureLevelRange);
    trajectorySystem->SelectedDirection((itsTrajectoryDirection == 0) ? kForward : kBackward);
    trajectorySystem->SelectedTrajectoryIsentropic(fTrajectoryIsentropic == TRUE);
    trajectorySystem->CalcTempBalloonTrajectors(fCalcTempBalloonTrajectories == TRUE);
    trajectorySystem->SelectedTime(itsSmartMetDocumentInterface->ActiveMapTime());
}

void CFmiTrajectoryDlg::OnBnClickedButtonTrajectoryAdd()
{
	CWaitCursor cursor;
    fProducerSelectorUsedYet = true; // merkit‰‰n ett‰ k‰ytt‰j‰ on tehnyt valintoja trajektori dialogissa (tietyt asetukset menev‰t talteen konffitiedostoon)
    UpdateTrajectorySystem();
    itsSmartMetDocumentInterface->TrajectorySystem()->AddTrajectory(true);
	RefreshViewsAndDialogs("TrajectoryDlg: Add trajectory");
}

void CFmiTrajectoryDlg::OnBnClickedButtonTrajectoryClear()
{
    itsSmartMetDocumentInterface->TrajectorySystem()->ClearTrajectories();
	RefreshViewsAndDialogs("TrajectoryDlg: Clear all trajectories");
}

void CFmiTrajectoryDlg::OnBnClickedButtonTrajectorySetAll()
{
	CWaitCursor cursor;
	UpdateTrajectorySystem();
    itsSmartMetDocumentInterface->TrajectorySystem()->SetSelectedValuesToAllTrajectories();
	RefreshViewsAndDialogs("TrajectoryDlg: Set selected options to all trajectories");
}

void CFmiTrajectoryDlg::SetDefaultValues(void)
{
    MoveWindow(CFmiTrajectoryDlg::ViewPosRegistryInfo().DefaultWindowRect());
    Persist2::WriteWindowRectToWinRegistry(itsSmartMetDocumentInterface->ApplicationWinRegistry(), MakeUsedWinRegistryKeyStr(0), this);
}

void CFmiTrajectoryDlg::RefreshViewsAndDialogs(const std::string &reasonForUpdate)
{
	auto &mapViewDescTopList = itsSmartMetDocumentInterface->MapViewDescTopList();
	for(size_t i = 0; i < mapViewDescTopList.size(); i++)
	{
		if(mapViewDescTopList[i]->ShowTrajectorsOnMap())
			mapViewDescTopList[i]->MapViewDirty(false, true, true, false);
	}
	SmartMetViewId updatedViews = SmartMetViewId::AllMapViews | SmartMetViewId::TrajectoryView;
	if(itsSmartMetDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView())
	updatedViews = updatedViews | SmartMetViewId::CrossSectionView;
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs(reasonForUpdate, updatedViews);
}

void CFmiTrajectoryDlg::OnBnClickedButtonTrajectoryRecalculate()
{
	CWaitCursor cursor;
    itsSmartMetDocumentInterface->TrajectorySystem()->ReCalculateTrajectories();
	RefreshViewsAndDialogs("TrajectoryDlg: recalculate trajectories");
}

void CFmiTrajectoryDlg::OnBnClickedButtonTrajectorySetLast()
{
	CWaitCursor cursor;
	UpdateTrajectorySystem();
    itsSmartMetDocumentInterface->TrajectorySystem()->SetSelectedValuesToLastTrajectory();
	RefreshViewsAndDialogs("TrajectoryDlg: Set last trajectory options");
}

void CFmiTrajectoryDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
//	CDialog::OnGetMinMaxInfo(lpMMI);

	CWnd *win = GetDlgItem(IDC_STATIC_TRAJECTORY_START_DEVIATION_SETTINGS);
	if(win)
	{ // oletus yll‰ mainittu kontrolli on kontrolli osan oikea alakulma. Siihen pit‰‰ viel‰
	  // lis‰t‰ v‰h‰n ett‰ saadaan minimi koko

		CRect dlgRect;
		GetWindowRect(dlgRect);

		CRect controlRect;
		win->GetWindowRect(controlRect);

		lpMMI->ptMinTrackSize.x = controlRect.right - dlgRect.left + 8;
		lpMMI->ptMinTrackSize.y = controlRect.bottom - dlgRect.top + 250;
	}
}

void CFmiTrajectoryDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if(itsView)
	{
		CRect rect(CalcClientArea());
		itsView->MoveWindow(rect, FALSE);
	}
}

void CFmiTrajectoryDlg::OnBnClickedCheckTrajectoryShowArrows()
{
	CWaitCursor cursor;
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->TrajectorySystem()->ShowTrajectoryArrows(fShowTrajectoryArrows == TRUE);
	RefreshViewsAndDialogs("TrajectoryDlg: Toggle show arrows option");
}

void CFmiTrajectoryDlg::OnBnClickedCheckTrajectoryShowAnimationMarkers()
{
	CWaitCursor cursor;
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->TrajectorySystem()->ShowTrajectoryAnimationMarkers(fShowTrajectoryAnimationMarkers == TRUE);
	RefreshViewsAndDialogs("TrajectoryDlg: Toggle show animation markers option");
}

void CFmiTrajectoryDlg::OnBnClickedCheckTrajectoryShowInCrossSectionView()
{
	UpdateData(TRUE);
    itsSmartMetDocumentInterface->TrajectorySystem()->ShowTrajectoriesInCrossSectionView(fShowTrajectoriesInCrossSectionView == TRUE);
    ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews | SmartMetViewId::CrossSectionView | SmartMetViewId::TrajectoryView);
    itsSmartMetDocumentInterface->RefreshApplicationViewsAndDialogs("TrajectoryDlg: Toggle show in cross section view option", false, false);
}

void CFmiTrajectoryDlg::OnBnClickedButtonTrajectoryConstantSpeedSettings()
{
	CFmiTempBalloonSettingsDlg dlg(itsSmartMetDocumentInterface->TrajectorySystem()->TempBalloonTrajectorSettings());
	if(dlg.DoModal() == IDOK)
	{
		RefreshViewsAndDialogs("TrajectoryDlg: Change balloon options");
	}
}

CBitmap* CFmiTrajectoryDlg::MemoryBitmap(void)
{
	if(itsView)
		return itsView->MemoryBitmap();
	return 0;
}


// controllin teksti asetetaan suoraan
void CFmiTrajectoryDlg::SetDlgItemText2(int theDlgItemId, const std::string &theStr)
{
	CWnd *win = GetDlgItem(theDlgItemId);
	if(win)
	{
		win->SetWindowText(CA2T(theStr.c_str()));
		if(theStr.empty())
			win->EnableWindow(FALSE);
		else
			win->EnableWindow(TRUE);
	}
}

const std::string GetModelName(NFmiProducerSystem &theProducerSystem, int theModelIndex)
{
	if(theProducerSystem.ExistProducer(theModelIndex))
		if(theProducerSystem.Producer(theModelIndex).HasRealVerticalData())
			return theProducerSystem.Producer(theModelIndex).Name();
	return "";

}

// T‰m‰ funktio alustaa kaikki dialogin tekstit editoriin valitulla kielell‰.
// T‰m‰ on ik‰v‰ kyll‰ teht‰v‰ erikseen dialogin muokkaus tyˆkalusta, eli
// tekij‰n pit‰‰ lis‰t‰ erikseen t‰nne kaikki dialogin osat, joihin
// kieli valinta voi vaikuttaa.
void CFmiTrajectoryDlg::InitDialogTexts(void)
{
	SetWindowText(CA2T(::GetDictionaryString("CFmiTrajectoryDlgTitle").c_str()));
	CFmiWin32Helpers::SetDialogItemText(this, IDOK, "NormalWordCapitalClose");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_TRAJECTORY_ADD, "IDC_BUTTON_TRAJECTORY_ADD");

	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_TRAJECTORY_CLEAR, "IDC_BUTTON_TRAJECTORY_CLEAR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_TRAJECTORY_RECALCULATE, "IDC_BUTTON_TRAJECTORY_RECALCULATE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_TRAJECTORY_SET_LAST, "IDC_BUTTON_TRAJECTORY_SET_LAST");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_TRAJECTORY_SET_ALL, "IDC_BUTTON_TRAJECTORY_SET_ALL");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_TRAJECTORY_PROPERTIES, "IDC_BUTTON_TRAJECTORY_PROPERTIES");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_BUTTON_TRAJECTORY_TEMP_BALLOON_SETTINGS, "IDC_BUTTON_TRAJECTORY_TEMP_BALLOON_SETTINGS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_TRAJECTORY_DIRECTION_FORWARD, "IDC_RADIO_TRAJECTORY_DIRECTION_FORWARD");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_TRAJECTORY_DIRECTION_BACKWARD, "IDC_RADIO_TRAJECTORY_DIRECTION_BACKWARD");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TRAJECTORY_MODEL, "IDC_STATIC_TRAJECTORY_MODEL");

	// 3 ensimm‰ist‰ tuottaja nime‰ otetaan ProducerSystemilt‰, 4. on editoitu
	SetDlgItemText2(IDC_RADIO_TRAJECTORY_MODEL1, ::GetModelName(itsSmartMetDocumentInterface->ProducerSystem(), 1));
	SetDlgItemText2(IDC_RADIO_TRAJECTORY_MODEL2, ::GetModelName(itsSmartMetDocumentInterface->ProducerSystem(), 2));
	SetDlgItemText2(IDC_RADIO_TRAJECTORY_MODEL3, ::GetModelName(itsSmartMetDocumentInterface->ProducerSystem(), 3));

	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_TRAJECTORY_MODEL4, "IDC_RADIO_TRAJECTORY_MODEL4");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TRAJECTORY_DATA_TYPE, "IDC_STATIC_TRAJECTORY_DATA_TYPE");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_TRAJECTORY_DATA_TYPE1, "IDC_RADIO_TRAJECTORY_DATA_TYPE1");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_TRAJECTORY_DATA_TYPE2, "IDC_RADIO_TRAJECTORY_DATA_TYPE2");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_TRAJECTORY_DATA_TYPE3, "IDC_RADIO_TRAJECTORY_DATA_TYPE3");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_RADIO_TRAJECTORY_DATA_TYPE4, "IDC_RADIO_TRAJECTORY_DATA_TYPE4");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_TRAJECTORY_SHOW_ARROWS, "IDC_CHECK_TRAJECTORY_SHOW_ARROWS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_TRAJECTORY_SHOW_ANIMATION_MARKERS, "IDC_CHECK_TRAJECTORY_SHOW_ANIMATION_MARKERS");

	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_TRAJECTORY_SHOW_IN_CROSS_SECTION_VIEW, "IDC_CHECK_TRAJECTORY_SHOW_IN_CROSS_SECTION_VIEW");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_TRAJECTORY_ISENTROPIC, "IDC_CHECK_TRAJECTORY_ISENTROPIC");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_TRAJECTORY_TEMP_BALLOON, "IDC_CHECK_TRAJECTORY_TEMP_BALLOON");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TRAJECTORY_PLUME_SETTINGS, "IDC_STATIC_TRAJECTORY_PLUME_SETTINGS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_CHECK_TRAJECTORY_USE_PLUMES, "IDC_CHECK_TRAJECTORY_USE_PLUMES");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TRAJECTORY_PROB_FACTOR_STR, "IDC_STATIC_TRAJECTORY_PROB_FACTOR_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TRAJECTORY_PARTICLE_COUNT_STR, "IDC_STATIC_TRAJECTORY_PARTICLE_COUNT_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TRAJECTORY_TIME_SETTINGS, "IDC_STATIC_TRAJECTORY_TIME_SETTINGS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TRAJECTORY_TIME_STEP_STR, "IDC_STATIC_TRAJECTORY_TIME_STEP_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TRAJECTORY_TIME_LENGTH_STR, "IDC_STATIC_TRAJECTORY_TIME_LENGTH_STR");

	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TRAJECTORY_CONSTANT_VERTICAL_MOTION_STR, "IDC_STATIC_TRAJECTORY_CONSTANT_VERTICAL_MOTION_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TRAJECTORY_START_DEVIATION_SETTINGS, "IDC_STATIC_TRAJECTORY_START_DEVIATION_SETTINGS");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TRAJECTORY_START_DEVIATION_TIME_STR, "IDC_STATIC_TRAJECTORY_START_DEVIATION_TIME_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TRAJECTORY_START_DEVIATION_LOCATION_STR, "IDC_STATIC_TRAJECTORY_START_DEVIATION_LOCATION_STR");
	CFmiWin32Helpers::SetDialogItemText(this, IDC_STATIC_TRAJECTORY_START_DEVIATION_PRESSURE_LEVEL_STR, "IDC_STATIC_TRAJECTORY_START_DEVIATION_PRESSURE_LEVEL_STR");
}

BOOL CFmiTrajectoryDlg::InitTooltipControl(void)
{
	if(!itsTooltipCtrl.Create(this))
	{
		TRACE("Unable To create ToolTipCtrl in CFmiTrajectoryDlg-class\n");
		return FALSE;
	}
	itsTooltipCtrl.SetDelayTime(TTDT_INITIAL, 150); // lyhyt alku aika (150 ms) ennen kuin tootip tulee n‰kyviin
	itsTooltipCtrl.SetDelayTime(TTDT_AUTOPOP, 30 * 1000); // 30 sekuntia on pisin saika mink‰ sain tooltipin pysym‰‰n n‰kyviss‰ (jos suurempi, menee defaultti arvoon 5 sekuntia)
	itsTooltipCtrl.Activate(TRUE);

	return TRUE;
}

void CFmiTrajectoryDlg::InitControlsTooltip(int theControlId, const char *theDictionaryStr)
{
	CWnd *win = GetDlgItem(theControlId);
	if(win)
		itsTooltipCtrl.AddTool(win, CA2T(::GetDictionaryString(theDictionaryStr).c_str()));
}

BOOL CFmiTrajectoryDlg::PreTranslateMessage(MSG* pMsg)
{
	itsTooltipCtrl.RelayEvent(pMsg); // tooltipit pit‰‰ forwardoida n‰in
	return CDialog::PreTranslateMessage(pMsg);
}

void CFmiTrajectoryDlg::OnBnClickedButtonSaveTrajectory()
{
	CFmiTrajectorySaveDlg dlg(itsSmartMetDocumentInterface, this);
	if(dlg.DoModal() == IDOK)
	{
		// Save toiminto on jo tehty CFmiTrajectorySaveDlg-luokan OnOK-metodissa
        CatLog::logMessage("Trajectory save has been done.", CatLog::Severity::Info, CatLog::Category::Operational);
	}
}

void CFmiTrajectoryDlg::OnCbnSelchangeComboTrajectoryProducerSelector()
{
	EnableModelAndTypeControls();
	fProducerSelectorUsedYet = true;
}

void CFmiTrajectoryDlg::OnBnClickedRadioTrajectoryDataType1()
{
	fProducerSelectorUsedYet = true; // merkit‰‰n ett‰ k‰ytt‰j‰ on tehnyt valintoja trajektori dialogissa (tietyt asetukset menev‰t talteen konffitiedostoon)
}

void CFmiTrajectoryDlg::OnBnClickedRadioTrajectoryDataType2()
{
	fProducerSelectorUsedYet = true; // merkit‰‰n ett‰ k‰ytt‰j‰ on tehnyt valintoja trajektori dialogissa (tietyt asetukset menev‰t talteen konffitiedostoon)
}

void CFmiTrajectoryDlg::OnBnClickedRadioTrajectoryDataType3()
{
	fProducerSelectorUsedYet = true; // merkit‰‰n ett‰ k‰ytt‰j‰ on tehnyt valintoja trajektori dialogissa (tietyt asetukset menev‰t talteen konffitiedostoon)
}

void CFmiTrajectoryDlg::OnBnClickedRadioTrajectoryDataType4()
{
	fProducerSelectorUsedYet = true; // merkit‰‰n ett‰ k‰ytt‰j‰ on tehnyt valintoja trajektori dialogissa (tietyt asetukset menev‰t talteen konffitiedostoon)
}
