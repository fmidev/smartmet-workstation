#pragma once

#include "SmartMetDialogs_resource.h"
#include "TabCtrlSSL.h"
#include "GridCtrl.h"
#include "CheckComboBox.h"

#include <string>
#include <functional>
#include <memory>
#include "afxwin.h"

class NFmiBetaProductionSystem;
class NFmiBetaProductAutomationListItem;
class NFmiBetaProductAutomation;
class SmartMetDocumentInterface;

struct BetaAutomationHeaderParInfo
{
    enum ColumnFunction
    {
        kRowNumber = 0,
        kAutomationName,
        kEnable,
        kNextRuntime,
        kLastRuntime,
        kAutomationStatus,
        kAutomationPath,
    };

    BetaAutomationHeaderParInfo(void)
        :itsHeader()
        , itsColumnFunction(kAutomationName)
        , itsColumnWidth(0)
    {}

    BetaAutomationHeaderParInfo(const std::string &theHeader, ColumnFunction theColumnFunction, int theColumnWidth)
        :itsHeader(theHeader)
        , itsColumnFunction(theColumnFunction)
        , itsColumnWidth(theColumnWidth)
    {}

    std::string itsHeader;
    ColumnFunction itsColumnFunction;
    int itsColumnWidth;
};

// Pit‰‰ peri‰ oma gridCtrl-luokka, ett‰ se osaa hanskata parit kikat
class NFmiBetaAutomationGridCtrl : public CGridCtrl
{
    DECLARE_DYNCREATE(NFmiBetaAutomationGridCtrl)

public:
    NFmiBetaAutomationGridCtrl(int nRows = 0, int nCols = 0, int nFixedRows = 0, int nFixedCols = 0)
        :CGridCtrl(nRows, nCols, nFixedRows, nFixedCols)
        , itsLeftClickUpCallback()
    {}

    void SetLeftClickUpCallback(std::function<void(const CCellID&)> theCallback) { itsLeftClickUpCallback = theCallback; }
    void SetRightClickUpCallback(std::function<void(const CCellID&)> theCallback) { itsRightClickUpCallback = theCallback; }

public:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

private:
    std::function<void(const CCellID&)> itsLeftClickUpCallback;
    std::function<void(const CCellID&)> itsRightClickUpCallback;
};

// CFmiBetaProductAutomationDialog dialog

class CFmiBetaProductAutomationDialog : public CTabPageSSL
{
	DECLARE_DYNAMIC(CFmiBetaProductAutomationDialog)

public:
    CFmiBetaProductAutomationDialog(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiBetaProductAutomationDialog();
    void DoWhenClosing(void);
    void SelectedGridCell(const CCellID &theSelectedCell);
    void DeselectGridCell(const CCellID &theSelectedCell);
    void Update();

// Dialog Data
	enum { IDD = IDD_DIALOG_BETA_PRODUCT_AUTOMATION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

private:
    void InitDialogTexts();
    void InitControlsFromDocument();
    void InitControlsFromLoadedBetaAutomation();
    void ApplySelectedBetaAutomation();
    void StoreControlValuesToDocument();
    void InitHeaders(void);
    void UpdateBetaProductPathInfo();
    void UpdateTriggerModeInfo();
    void UpdateStartTimeModeInfo();
    void UpdateEndTimeModeInfo();
    void CheckForSaveButtonEnablations();
    void UpdateAutomationList();
    void SetGridRow(int row, const NFmiBetaProductAutomationListItem &theListItem);
    void HandleEnableAutomationCheckBoxClick(int col, int row);
    void UpdateSelectedAutomationListName();
    void AddAutomationToList(const std::string &theFullFilePath);
    void EnableButtonAddEditedAutomation();
    void EnableButtonRemoveAutomationFromList();
    void MakeAutomationComparisonObject();
    bool HasAutomationChanged();
    void DoNextRuntimeUpdates();

    NFmiBetaAutomationGridCtrl itsGridCtrl;
    std::vector<BetaAutomationHeaderParInfo> itsHeaders;
    bool fGridControlInitialized;
    bool fDialogInitialized; // t‰m‰ on OnSize -s‰‰tˆj‰ varten, 1. kerralla ei saa s‰‰t‰‰ ikkunoita, koska niit‰ ei ole viel‰ olemassa.

    SmartMetDocumentInterface *itsSmartMetDocumentInterface; // ei omista, ei tuhoa
    NFmiBetaProductionSystem *itsBetaProductionSystem; // ei omista, ei tuhoa
    BOOL fAutomationModeOn;
    CString itsBetaProductPathErrorStringU_;
    CString itsBetaProductPathU_;
    std::shared_ptr<NFmiBetaProductAutomation> itsBetaProductAutomation; // T‰h‰n talletetaan kaikki BetaProductAutomation inputit ja t‰m‰ tekee tarkastelut niiden oikeellisuudesta
    std::shared_ptr<NFmiBetaProductAutomation> itsBetaProductAutomationComparisonObject; // T‰h‰n laitetaan originaalin Beta-automaation kopio talteen. Kun k‰ytt‰j‰ editoi automaatiota, voidaan tarkastella ett‰ onko se muuttunut, jolloin voidaan enbloida Save -nappula
    int itsTriggerModeIndex;
    CString itsBetaAutomationFixedTimesStringU_;
    CString itsBetaAutomationTimeStepStringU_;
    CString itsBetaAutomationFirstRunOfDayValueU_;
    CString itsBetaAutomationTriggerModeInfoStringU_;
    int itsBetaAutomationStartTimeModeIndex;
    int itsBetaAutomationEndTimeModeIndex;
    CString itsBetaAutomationStartTimeClockOffsetValueU_;
    CString itsBetaAutomationEndTimeClockOffsetValueU_;
    CString itsBetaAutomationStartTimeClockOffsetErrorU_;
    CString itsBetaAutomationEndTimeClockOffsetErrorU_;
    CCheckComboBox itsMultiDataSelector;
    // Tehd‰‰n Beta-automaatio save buttonille muuttuja, jotta sit‰ voidaan inputtien oikeellisuudesta riippuen enbloida/disabloida
    CButton itsBetaAutomationSaveButton;
    CButton itsBetaAutomationSaveAsButton;
    CString itsAutomationNameU_;
    std::string itsSelectedAutomationFullFilePath; // Kun automaatio ladataan tiedostosta tai valitaan hiirell‰ klikkaamalla automaatiolista, pit‰‰ sen tiedostopolku ottaa talteen, jotta Save-napista osataan tallettaa oikeaan paikkaan.
    CString itsAutomationListNameU_;
    CButton itsAutomationListSaveButton;
    CButton itsAutomationListSaveAsButton;
    CButton itsAddEditedAutomationToListButton;
    CButton itsRemoveAutomationFromListButton;

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedButtonBetaProductPathBrowse();
    afx_msg void OnEnChangeEditBetaProductPath();
    afx_msg void OnDestroy();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnBnClickedRadioTriggerModeFixedTimes();
    afx_msg void OnBnClickedRadioTriggerModeTimesStep();
    afx_msg void OnBnClickedRadioTriggerModeDataEvent();
    afx_msg void OnEnChangeEditBetaAutomationTimeStepValue();
    afx_msg void OnEnChangeEditBetaAutomationFixedTimes();
    afx_msg void OnEnChangeEditBetaAutomationFirstRunOfDayValue();
    afx_msg void OnCbnSelchangeComboBetaAutomationDataTrigger();
    afx_msg void OnCbnCloseupComboBetaAutomationDataTrigger();
    afx_msg void OnBnClickedRadioBetaAutomationStartTimeFromProduct();
    afx_msg void OnBnClickedRadioBetaAutomationStartTimeWallClockOffset();
    afx_msg void OnBnClickedRadioBetaAutomationStartTimeFromModel();
    afx_msg void OnBnClickedRadioBetaAutomationEndTimeFromProduct();
    afx_msg void OnBnClickedRadioBetaAutomationEndTimeWallClockOffset();
    afx_msg void OnBnClickedRadioBetaAutomationEndTimeFromModel();
    afx_msg void OnBnClickedButtonBetaAutomationSave();
    afx_msg void OnBnClickedButtonBetaAutomationLoad();
    afx_msg void OnEnChangeEditBetaAutomationStartTimeWallClockOffsetValue();
    afx_msg void OnEnChangeEditBetaAutomationEndTimeWallClockOffsetValue();
    afx_msg void OnBnClickedButtonAddBetaAutomationToList();
    afx_msg void OnBnClickedButtonRemoveBetaAutomationFromList();
    afx_msg void OnBnClickedButtonSaveAutomationList();
    afx_msg void OnBnClickedButtonLoadAutomationList();
    afx_msg void OnBnClickedButtonBetaAutomationSaveAs();
    afx_msg void OnBnClickedButtonSaveAsAutomationList();
    afx_msg void OnBnClickedButtonAddEditedBetaAutomationToList();
    afx_msg void OnBnClickedCheckAutomatiionModeOn();
    afx_msg void OnBnClickedButtonRunSelectedAutomation();
    afx_msg void OnBnClickedButtonRunAllAutomations();
    afx_msg void OnBnClickedButtonRunEnabledAutomations();
};
