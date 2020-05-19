#pragma once

// FmiLoadDataDialog.h : header file
//

#include "SmartMetDialogs_resource.h"
#include "NFmiTimeBag.h"
#include "NFmiColor.h"
#include "NFmiDataMatrix.h"
#include "NFmiRect.h"

#include "boost/shared_ptr.hpp"
#include <map>
#include "afxdtctl.h"

class NFmiDataLoadingInfo;
class SmartMetDocumentInterface;
class NFmiModelProducerIndexViewWCTR;
class CFmiFilterDataView;
class NFmiQueryInfo;

namespace Gdiplus
{
	class Bitmap;
	class Graphics;
}

// T‰h‰n luokkaan luetaan kyseiset kuva-imaget kerran ja k‰ytet‰‰n blendaus kontrollissa.
// Gdiplus::Bitmap-olioita ei tarvitse tuhota, Gdiplus huolehtii siit‰ automaattisesti.
class NFmiControlButtonImageHolder
{
public:
	struct ButtonData
	{
		ButtonData(void)
		:itsImage(0)
		,itsPosition()
		,itsImageFileName()
		,itsName()
		{
		}

		Gdiplus::Bitmap *itsImage; // gdiplus huolehtii tuhoamisesta automaattisesti
		NFmiRect itsPosition; // suhteelinen vai pikeli (?!?) sijainti ja koko 
		std::string itsImageFileName;
		std::string itsName;
	};

	typedef std::map<std::string, ButtonData> Container;
	NFmiControlButtonImageHolder(void)
	:itsButtons()
	{
	}

	ButtonData* Find(const std::string &theName);
	ButtonData* Find(const NFmiPoint &thePos);
	void Add(const ButtonData &theData);

	Container itsButtons; // mapataan nimen mukaan set:iin

};

/////////////////////////////////////////////////////////////////////////////
// CFmiLoadDataDialog dialog

class CFmiLoadDataDialog : public CDialog
{
// Construction
public:
	CFmiLoadDataDialog(SmartMetDocumentInterface *smartMetDocumentInterface, CWnd* pParent = NULL);
	~CFmiLoadDataDialog(void);

	void HirlamFileNames(NFmiDataLoadingInfo& theDataLoadingInfo);
	void EcmwfFileNames(NFmiDataLoadingInfo& theDataLoadingInfo);
	void WorkingFileNames(NFmiDataLoadingInfo& theDataLoadingInfo);
	void DataBaseFileNames1(NFmiDataLoadingInfo& theDataLoadingInfo);

	BOOL DoSetup(void);
	bool RemoveThundersOnLoad(void){return fRemoveThundersOnLoad == TRUE;}

// Dialog Data
	//{{AFX_DATA(CFmiLoadDataDialog)
	enum { IDD = IDD_DIALOG_DATA_LOAD_NEW };
	CButton	itsButtonActivateModel4;
	CButton	itsButtonActivateModel3;
	CButton	itsButtonActivateModel2;
	CButton	itsButtonActivateModel1;
	CComboBox	itsOwnFileNames;
	CComboBox	itsOfficialFileNames;
	CComboBox	itsHirlamFileNames;
	CComboBox	itsECMWFFileNames;
    CString	itsProducer1U_;
    CString	itsProducer2U_;
    CString	itsProducer3U_;
    CString	itsProducer4U_;
	int		itsPrimaryProducer;
	int		itsSecondaryProducer;
	int		itsThirdProducer;
	int		itsEnableInterpolation;
	BOOL	fRemoveThundersOnLoad;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiLoadDataDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFmiLoadDataDialog)
	afx_msg void OnUpdateActivateModel1(CCmdUI* pCmdUI);
	afx_msg void OnUpdateActivateModel2(CCmdUI* pCmdUI);
	afx_msg void OnUpdateActivateModel3(CCmdUI* pCmdUI);
	afx_msg void OnUpdateActivateModel4(CCmdUI* pCmdUI);
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRADIO1hirlam();
	afx_msg void OnRADIO1official();
	afx_msg void OnRADIO1own();
	afx_msg void OnRadio2Ecmwf();
	afx_msg void OnRADIO2hirlam();
	afx_msg void OnRADIO2official();
	afx_msg void OnRADIO2own();
	afx_msg void OnRadio3Ecmwf();
	afx_msg void OnRADIO3hirlam();
	afx_msg void OnRADIO3official();
	afx_msg void OnRADIO3own();
	afx_msg void OnRadio1Ecmwf();
	virtual void OnOK();
	afx_msg void OnButtonActivatemodel1();
	afx_msg void OnButtonActivatemodel2();
	afx_msg void OnButtonActivatemodel3();
	afx_msg void OnButtonActivatemodel4();
	afx_msg void OnSelchangeCOMBOHirlamfiles();
	afx_msg void OnSelchangeCOMBOofficialfiles();
	afx_msg void OnSelchangeCOMBOownfiles();
	afx_msg void OnSelchangeCOMBOECMWFfiles();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnPaint();
	afx_msg void OnBnClickedButtonBlendMakeLinear();
	afx_msg void OnBnClickedCheckUseModelBlending();
	afx_msg void OnDtnDatetimechangeDatetimepickerCasestudyStartDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDatetimechangeDatetimepickerCasestudyStartTime(NMHDR *pNMHDR, LRESULT *pResult);

private:
	void InvalidateBlendControl(void);
	int CalcFactorLocation(float theFactor);
	void DrawFactorLabel(size_t theIndex);
	float CalcBlendFactor(const CPoint &point);
	size_t FindClosestTimeIndex(const CPoint &point);
	int CalcForecastHourXLocation(int theHour);
	void DrawButton(const std::string &theName);
	void InitializeModelBlenderControl(void);
	void CalcModelBlendScaleRect(void);
	NFmiRect CalcStartTimePreviousButtonRect(void);
	NFmiRect CalcStartTimeNextButtonRect(void);
	NFmiRect CalcEndTimePreviousButtonRect(void);
	NFmiRect CalcEndTimeNextButtonRect(void);
	void InitializeBlenderButtons(void);
	CRect CalcModelBlendingControlRect(void);
	void DrawModelBlendingControl(CDC * dc);
	void UpdateQueryInfoVector(void);
	void DoFullModelProducerIndexViewUpdate(void);
	void CreateModelProducerIndexView(void);
	void SetupProducerButtons(NFmiDataLoadingInfo& theDataLoadingInfo);
	void SetupNameCombos(NFmiDataLoadingInfo& theDataLoadingInfo);
	void SetupProducerNames(NFmiDataLoadingInfo& theDataLoadingInfo);
	void SetupTitle(NFmiDataLoadingInfo& theDataLoadingInfo);
	std::vector<std::string> GetSelectedFileNames(void);
	boost::shared_ptr<NFmiQueryInfo> ReadInfoFromFile(const std::string& theFileName);
	std::string GetSelectedNameFromCombo(CComboBox& theNameCombo);
	void SetActiveProducerToView(void);
	void InitDialogTexts(void);
	void InitCaseStudyDateTimeControls(void);
	void UpdateCaseStudyStartTime(void);
	void InitCaseStudyTimePickers(void);
    bool UseNormalStartTime();

	NFmiDataLoadingInfo *itsDataLoadingInfo;
    SmartMetDocumentInterface *itsSmartMetDocumentInterface;

	CFmiFilterDataView *itsModelProducerView; // HUOM! ei tuhota, koska windows hoitaa sen. Pit‰‰ sis‰ll‰‰n itsModelProducerIndexView-n‰ytˆn.
	NFmiModelProducerIndexViewWCTR *itsModelProducerIndexView; // t‰ss‰ n‰ytet‰‰n valitut tuottajat kulloisenkiin ajanhetkeen
														   // Muutokset dialogissa p‰ivitet‰‰n suoraan n‰yttˆˆn.
														   // Ei tuhota, koska yll‰oleva n‰yttˆ tuhoaa sen.

	// n‰it‰ tarvitaan NFmiModelProducerIndexView-luokan p‰ivitykseen
	std::vector<boost::shared_ptr<NFmiQueryInfo> > itsQueryInfoVector; // t‰ss‰ on ladattavien datojen tietoja
	int itsNonModifieableTimeRangeProducerIndex; // mill‰ tuottajalla t‰ytet‰‰n mahdollinen ei editoitava alue
	int itsActiveProducerIndex; // mill‰ tuottajalla t‰ytet‰‰n mahdollinen ei editoitava alue
	std::vector<int> itsSelectedProducerPriorityTable; // radio buttonien asetukset
	std::vector<NFmiColor> itsProducerColorTable; // eri tuottajille on eri v‰rit

	COLORREF itsProducerButton1Color;
	COLORREF itsProducerButton2Color;
	COLORREF itsProducerButton3Color;
	COLORREF itsProducerButton4Color;
	CBitmap* itsBitmap1;
	CBitmap* itsBitmap2;
	CBitmap* itsBitmap3;
	CBitmap* itsBitmap4;
	CRect itsColorRect1;
	CRect itsColorRect2;
	CRect itsColorRect3;
	CRect itsColorRect4;

	CRect itsModelBlendCtrlRect; // t‰m‰ on koko blendaus kontrollin reunat
	CRect itsModelBlendScaleRect; // t‰ss‰ on kertoimien s‰‰tˆ asteikon rajat
	CRect itsModelBlendScaleInflatedRect; // t‰m‰n alueen si‰lle pit‰‰ hirell‰ klikata ett‰ voi s‰‰t‰‰ kertoimia, laitoin levitin reunoa jonkin verran

	int itsActivatedProducer; // mik‰ tuottaja v‰ri nappi on pohjassa (0-3)

	NFmiControlButtonImageHolder itsControlButtonImageHolder;
	Gdiplus::Graphics *itsGdiPlusGraphics;
	CDateTimeCtrl itsCaseStudyStartTimeDateCtrl;
	CDateTimeCtrl itsCaseStudyStartTimeCtrl;
	BOOL fUseModelBlender;
	static NFmiMetTime itsCaseStudyStartTime; // Jos editoidun datan alkuaikaa halutaan s‰‰t‰‰, se s‰‰det‰‰n t‰h‰n aika ja kalenteri kontrollien avulla (static = SmartMet muistaa ajan k‰ynniss‰ olo ajan)
    // Annetaanko k‰ytt‰j‰n s‰‰t‰‰ editoidun datan aloitus aika vai ei
    static BOOL fSetStartTime; // T‰m‰ asetus muistetaan SmartMetin k‰ynnisss‰ olo ajan, aluksi se on false tilassa
public:
    afx_msg void OnBnClickedCheckSetStartTime();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

