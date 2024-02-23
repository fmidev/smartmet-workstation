#pragma once

#include "SmartMetDialogs_resource.h"
#include "NFmiMetTime.h"
#include "TextProgressCtrl.h"
#include "TabCtrlSSL.h"
#include "NFmiBetaProductSystem.h"

#include "afxwin.h"

class NFmiBetaProductionSystem;
class NFmiBetaProduct;
class NFmiBetaProductAutomationListItem;
class SmartMetDocumentInterface;

// CFmiBetaProductDialog dialog

class CFmiBetaProductDialog : public CTabPageSSL // CDialogEx
{
	DECLARE_DYNAMIC(CFmiBetaProductDialog)

public:
    CFmiBetaProductDialog(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFmiBetaProductDialog();
    void DoWhenClosing(void);

// Dialog Data
	enum { IDD = IDD_DIALOG_BETA_PRODUCT };

private:
    void InitDialogTexts();
    void InitControlsFromDocument();
    void InitControlsFromLoadedBetaProduct();
    void StoreControlValuesToDocument();
    bool CheckDestinationDirectory(const std::string &theDestinationDirectory, bool fAllowDestinationDelete, const NFmiBetaProduct &theBetaProduct, const std::string & theImageFileNameBase);
    NFmiMetTime GetFirstModelOrigTime(BetaProductViewIndex selectedViewRadioButtonIndex, int theRealRowIndex);
    NFmiTimeDescriptor GetFirstModelsValidTimeDescriptor(unsigned int theDescTopIndex, int theRealRowIndex);
    int GetViewRowIndex(BetaProductViewIndex selectedViewRadioButtonIndex);
    void SetViewRowIndex(BetaProductViewIndex selectedViewRadioButtonIndex, int rowIndex);
    void UpdateRowInfo(bool fUpdateTimeSectionInTheEnd = false);
    void CheckForGenerateButtonActivation();
    void AddImageRowPath(const NFmiBetaProduct& theBetaProduct, std::string &theImageFileNameBaseInOut, int theRowIndex);
    void UpdateGeneratedImagesText(int theIndex, int theTotalImageCount);
    void UpdateViewSelection();
    void UpdateViewMacroInfo();
    void UpdateFileNameTemplateInfo();
    void UpdateWebSiteInfo();
    void UpdateCommandLine();
    bool RunViewMacro(const NFmiBetaProduct &theBetaProduct, bool justLogMessages);
    void GenerateWebInfoFiles(const std::string &theDestinationDirectory, const NFmiBetaProduct &theBetaProduct);
    void StoreWebInfoFile(const std::string &theDestinationDirectory, const std::string &theFileName, const std::string &theFileText);
    void MakeCommandLineExecution(const std::string &theDestinationDirectory, const std::string &theCommandLine, bool justLogMessages);
    std::string GetFileNameTemplateStampsString();
    void DoBetaProductGenerations(std::vector<std::shared_ptr<NFmiBetaProductAutomationListItem>> &theDueAutomations, const NFmiMetTime &theMakeTime);
    void DoBetaProductGenerationsFinal(const NFmiMetTime &theMakeTime);
    NFmiMetTime GetCurrentViewTime(const NFmiBetaProduct &theBetaProduct);
    NFmiMetTime CalcStartingTime(const NFmiBetaProduct &theBetaProduct, const NFmiBetaProductAutomation::NFmiTimeModeInfo &theTimeMode, const NFmiMetTime &theMakeTime);
    NFmiMetTime CalcEndingTime(const NFmiBetaProduct &theBetaProduct, const NFmiBetaProductAutomation::NFmiTimeModeInfo &theTimeMode, const NFmiMetTime &theStartTime, const NFmiMetTime &theMakeTime);
    bool CheckGenerationTimes(const NFmiMetTime &theStartingTime, const NFmiMetTime &theEndingTime, bool justLogMessages);
    NFmiMetTime GetUsedModelTime(bool fGetStartTime, BetaProductViewIndex selectedViewRadioButtonIndex, int theRealRowIndex, const NFmiMetTime &thePreCalculatedTime);
    void DoImageProducingProcess(std::function<void(const NFmiMetTime&)> &theGenerationFunction, const NFmiMetTime &theMakeTime, bool justLogMessages);
    void InitLocationSelector(CComboBox &theLocationSelector);
    FmiDirection GetSelectedParamBoxLocation();
    void SetBoxLocationSelector(CComboBox &theBoxLocationSelector, FmiDirection newValue);
    bool IsFileNameTemplateStampsOk() const;
    void EnableFileNameTemplateEdit();
    void MakeVisualizationImagesRowLoop(const NFmiBetaProduct &theBetaProduct, const NFmiMetTime &theStartingTime, bool useModelStartTime, const NFmiMetTime &theEndingTime, bool useModelEndTime, const NFmiMetTime &theMakeTime, bool justLogMessages, const std::vector<int> &usedRowIndexies, int totalImageCount, int synopStationId, bool deleteDestinationDirectory);
    bool IsSynopLocationsUsed(BetaProductViewIndex selectedViewRadioButtonIndex, const std::vector<int> &synopLocationIds);
    bool SetStationIdLocation(const NFmiBetaProduct &theBetaProduct, int synopLocationId);
    void UpdateSynopStationIdInfo();
    void DoAllBetaProductUpdates();
    void UpdateSynopStationEditColors();
    void UpdateBetaProductName();
    void DoImagePacking(const std::string &directoryPath);
    void DoFileNameTemplateContainsStationIdIfNeededChecks();

    SmartMetDocumentInterface *itsSmartMetDocumentInterface; // ei omista, ei tuhoa
    NFmiBetaProductionSystem *itsBetaProductionSystem; // ei omista, ei tuhoa
    CString itsImageStoragePathU_;
    CString itsFileNameTemplateU_;
    CString itsFileNameTemplateStampsStringU_;
    bool fFileNameTemplateContainsValidTime;
    // Jos k‰ytˆss‰ Synop asemien lista ja visualisointi aikasarja-, luotaus- tai poikkileikkausn‰yttˆ (tietyt moodit),
    // pit‰isi tiedostonimeen lis‰t‰ 'stationid' sana kyseist‰ asemanumeroa varten.
    // Jos synop-id jutuille ei ole tarvetta, tulee t‰h‰n aina true.
    bool fFileNameTemplateContainsStationIdIfNeeded;
    CString itsTimeRangeInfoTextU_; // T‰h‰n p‰‰tell‰‰n kuvien tuotannon aika-ranget ja niihin liittyv‰t mahdolliset virhetekstit
    CString itsTimeLengthInHoursStringU_;
    CString itsTimeStepInMinutesStringU_;

    // T‰h‰n annetaan rivin indeksit joko pilkulla erotettuna listana (1,3,4) tai rangena (1-5)
    CString itsRowIndexListStringU_;
    CString itsRowSubdirectoryTemplateStringU_; // Jos eri rivien kuvat halutaan omiin alihakemistoihin, t‰h‰n tulee alihakemiston templaatti (Row# => Row1, Row2, ...)
    CString itsRowIndexListInfoTextU_; // T‰h‰n p‰‰tell‰‰n kuvien tuotannon rivi-indeksit, alihakemisto infoa ja niihin liittyv‰t mahdolliset virhetekstit
    CString itsSynopStationIdListTextU_; // T‰h‰n laitetaan halutut synop asema id:t, joista kuvia tuotetaan, input muotoa: luku1,luku2,luku3-luku4,...

    std::shared_ptr<NFmiBetaProduct> itsBetaProduct; // T‰h‰n talletetaan kaikki inputit ja t‰m‰ tekee tarkastelut niiden oikeellisuudesta
    CTextProgressCtrl itsImageGenerationProgressControl;
    // Tehd‰‰n Generate- ja Save -buttoneille muuttujat, jotta ne voidaan tarvittaessa enbloida/disabloida
    CButton itsGenerateImagesButton;
    CButton itsSaveButton;
    CButton itsSaveAsButton;
    // T‰h‰n laitetaan tuotannon ajaksi esim. "2/56 images generated" ja lopuksi "56 images generated (YYYY.MM.DD HH:mm:ss)"
    CString itsImageProductionTextU_;
    int itsSelectedViewIndex;
    CString itsViewMacroErrorTextU_;
    CString itsViewMacroPathU_;
    CString itsWebSiteTitleStringU_;
    CString itsWebSiteDescriptionStringU_;
    CString itsCommandLineStringU_;
    BOOL fUseUtcTimesInTimeBox;
    BOOL fUseAutoFileNames;
    CComboBox itsParamBoxLocationSelector;
    BOOL fDisplayRuntimeInfo;
    BOOL fShowModelOriginTime;
    int itsTotalImagesGenerated;
    std::string itsBetaProductFullFilePath;
    CString itsBetaProductNameU_;
    BOOL fPackImages;
    BOOL fEnsureCurveVisibility;

    std::vector<std::shared_ptr<NFmiBetaProductAutomationListItem>> *itsDueAutomations; // T‰h‰n laitetaan v‰liaikaisesti teht‰vien automaatioiden lista silloin kun SmartMet on automaatio moodissa ja se on huomannut ett‰ on teht‰vi‰ tˆit‰

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    virtual BOOL Create(CWnd* pParentWnd = NULL); // modaalitonta dialogi luontia varten
    virtual BOOL Create(UINT nIDTemplate, CWnd* pParentWnd = NULL); // Tab dialogia varten
    void Update();
    void MakeVisualizationImages(const NFmiBetaProduct &theBetaProduct, const NFmiMetTime &theStartingTime, bool useModelStartTime, const NFmiMetTime &theEndingTime, bool useModelEndTime, const NFmiMetTime &theMakeTime, bool justLogMessages);
    void MakeVisualizationImagesManually(const NFmiMetTime &theMakeTime);

    afx_msg void OnBnClickedButtonImageDirectoryBrowse();
    afx_msg void OnBnClickedButtonGenerateImages();
    afx_msg void OnEnChangeEditTimeStepInMinutes();
    afx_msg void OnEnChangeEditTimeLengthInHours();
    afx_msg void OnEnChangeEditRowIndexList();
    afx_msg void OnEnChangeEditRowSubdirectoryTemplateString();
    afx_msg void OnEnChangeEditFileNameTemplate();
    afx_msg void OnBnClickedRadioMainMapView();
    afx_msg void OnBnClickedRadioMapView2();
    afx_msg void OnBnClickedRadioMapView3();
    afx_msg void OnBnClickedRadioTimeSerialView();
    afx_msg void OnBnClickedRadioSoundingView();
    afx_msg void OnBnClickedRadioCrossSectionView();
    afx_msg void OnBnClickedButtonViewMacroPathBrowse();
    afx_msg void OnEnChangeEditViewMacroPath();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnBnClickedButtonSaveBetaProduct();
    afx_msg void OnBnClickedButtonLoadBetaProduct();
    afx_msg void OnEnChangeEditImageDirectory();
    afx_msg void OnEnChangeEditWebTitleString();
    afx_msg void OnEnChangeEditWebDescriptionString();
    afx_msg void OnDestroy();
    afx_msg void OnEnChangeEditCommandLineString();
    afx_msg void OnBnClickedCheckTimeBoxUseUtcTime();
    afx_msg void OnBnClickedCheckUseAutoFileNames();
    afx_msg void OnCbnSelchangeComboParamBoxLocationSelector();
    afx_msg void OnBnClickedCheckDisplayRuntimeInfo();
    afx_msg void OnEnChangeEditSynopStationIdString();
    afx_msg void OnBnClickedButtonSaveAsBetaProduct();
    afx_msg void OnBnClickedCheckShowModelOriginTime();
    afx_msg void OnBnClickedCheckPackImages();
    afx_msg void OnBnClickedCheckEnsureCurveVisibility();
};
