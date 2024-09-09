#pragma once

// FmiTMColorIndexDlg.h : header file
//

#include "SmartMetToolboxDep_resource.h"
#include "matrix3d.h"
#include "boost/shared_ptr.hpp"
#include "PPToolTip.h"

class NFmiDrawParam;
class CFmiModifyDrawParamDlg;

/////////////////////////////////////////////////////////////////////////////
// CFmiTMColorIndexDlg dialog

// T‰m‰ luokka pit‰‰ sis‰ll‰ v‰ri ruutujen piirtotiedot ja
// samalla sen avulla voidaan etsi‰, mit‰ v‰riruutua on hiirell‰ klikattu.
// Tein struct:in sijasta 'public' luokan, koska joskus struct:in kanssa
// tulee syntaksi kikkailua.
class ColorRectInfo
{
public:
	int itsColorIndex;
	COLORREF itsColor;
	CRect itsColorRect;
};

class CFmiTMColorIndexDlg : public CDialog
{
// Construction
public:
	CFmiTMColorIndexDlg(CFmiModifyDrawParamDlg *theModifyDrawParamDlg, const std::string &theTitleStr,	const std::string theHelpStr,
		                Matrix3D<std::pair<int, COLORREF> >* theColorsCube, boost::shared_ptr<NFmiDrawParam> &theDrawParam,
						bool doIsolineModifications, CWnd* pParent = NULL);   // standard constructor
	~CFmiTMColorIndexDlg(void);

	const std::vector<ColorRectInfo>& SelectedColorsRectVector(void) const {return itsSelectedColorsRectVector;}
// Dialog Data
	//{{AFX_DATA(CFmiTMColorIndexDlg)
	enum { IDD = IDD_DIALOG_TOOLMASTER_COLOR_TABLE_INDEX_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFmiTMColorIndexDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void InitializeColorRects(void);
	void AddSelectedColor(const ColorRectInfo &theColorRectInfo);
	CRect CalcSelectedColorRect(size_t theIndex);
	void DeleteSelectedColorRect(void);
	void SetSelectedColorIndex(const CPoint &thePoint);
	void DrawSelectedColorMarker(CDC *theDC);
	void MoveRemainingSelectedColorRectsBackward(int theSelectedColorIndex);
	void DoDraw(CDC *theDC);
    CRect CalcBelowButtonsClientArea(void);
    CRect CalcRealFinalDialogSize();
    int CalcCustomDrawAreaTop(void);
	void MakeViewUpdates(void);
	void MakeColorUpdatesTodrawParam(void);
	void MoveRemainingSelectedColorRectsForward(int theSelectedColorIndex);
	void InsertBeforeSelectedColor(const ColorRectInfo &theColorRectInfo);
    std::string ComposeToolTipText(const CPoint &point);
	const std::vector<float>& GetSpecialClassValues();
	void SetSpecialClassValues(const std::vector<float> &classValues);
	const std::vector<int>& GetSpecialClassColorIndexies();
	void SetSpecialClassColorIndexies(const std::vector<int>& classColorIndexies);
	void ShowDialogControl(UINT controlId, bool show);

	Matrix3D<std::pair<int, COLORREF> >& itsUsedColorsCube;

	std::string itsTitleStr; // n‰m‰ stringit annetaan construktorin yhteydess‰ ja ne on oikealla kielell‰
	std::string itsHelpStr;
	std::vector<ColorRectInfo> itsColorRectVector; // T‰h‰n talletetaan kaikki v‰rikuution v‰rit sijaintineen,
												// ett‰ ne voidaan piirt‰‰ ruudulle ja niihin voidaan klikata hiirell‰.
	std::vector<ColorRectInfo> itsSelectedColorsRectVector; // T‰h‰n laitetaan valitut v‰rit, eli uutena featurena
															// v‰ridialogista rakennetaan valittujen v‰rien lista,
															// jota voidaan k‰ytt‰‰ customContour-v‰rityksess‰.
	boost::shared_ptr<NFmiDrawParam> itsDrawParam;
	int itsSelectedColorIndex; // valituista v‰reist‰ valitun v‰rin indeksi, -1 jos ei mit‰‰n ole valittuna
	int itsLastColorPaletteColorIndex; // mik‰ v‰ripaletin v‰reist‰ on viimeiseksi valittu.
										// Jos klikkaa jotain v‰ripaletin v‰ri‰, t‰h‰n tulee sen indeksi.
										// Jos valitsee jonkun valituista v‰reist‰, t‰h‰n tuleen sit‰ vastaava v‰ripaletista.
										// rulla ylˆs/alas muuttaa t‰t‰ ja valituista valitun v‰ri‰.
										// Jos valitun v‰rin 'deselectoi', t‰m‰ ei muutu mitenk‰‰n.
										// Aluksi ennenkuin mit‰‰n on tehty, t‰m‰ on -1.
	CBitmap* itsMemoryBitmap; // t‰m‰n avulla tehd‰‰n piirtoon double-buffer ja v‰lkynt‰ loppuu.
	bool fDrawButtonArea;
	int itsColorPaletteBottomY;
	BOOL fDoViewUpdates; // jos t‰m‰ on true, p‰ivitet‰‰n muokattavan parametrin n‰yttˆ‰ liven‰ heti kun muutos tapahtuu
	static BOOL fDoViewUpdatesMemory;
	CFmiModifyDrawParamDlg *itsModifyDrawParamDlg; // t‰m‰n avulla muokattavaa parametria p‰ivitet‰‰n (OnBnClickedModifyDrwParamRefresh-metodia kutsumalla)
	bool fDoIsolineModifications; // Voidaan muokata joko isoline/contour asetuksia

	// Generated message map functions
	//{{AFX_MSG(CFmiTMColorIndexDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
public:
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedCheckInstantViewUpdate();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
private:
    CString itsSpecialClassesValuesStrU_;
	std::vector<float> itsLatestAcceptedSpecialClasses; // jos syˆte on hetkellisesti v‰‰r‰, k‰ytet‰‰n viimeist‰ hyv‰ksytty‰ arvo listaa piirrossa
	double itsContourGab;
	BOOL fUseColorBlendingWithCustomContours;
	double itsLatestAcceptedContourGap; // jos syˆte on hetkellisesti v‰‰r‰, k‰ytet‰‰n viimeist‰ hyv‰ksytty‰ arvoa piirrossa
	bool fSpecialClassesHaveInvalidValues; // t‰m‰n muuttujan avulla v‰ritet‰‰n labeli tarvittaessa punaiseksi ett‰
											// k‰ytt‰j‰ n‰kee ett‰ annetut luokka rajat ovat virheellisi‰.
											// Suurin ongelma tulee kun kaikki arvoteiv‰t ole nousevassa j‰rjestyksess‰. T‰llˆin ohjelma toimii oudosti.
    CPPToolTip m_tooltip;
public:
	afx_msg void OnEnChangeSpecialClassesValues();
	afx_msg void OnEnChangeContourGap();
	afx_msg void OnBnClickedCheckUseColorBlendingWithCustomContours();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedButtomRemoveColor();
    afx_msg void NotifyDisplayTooltip(NMHDR * pNMHDR, LRESULT * result);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnSize(UINT nType, int cx, int cy);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

