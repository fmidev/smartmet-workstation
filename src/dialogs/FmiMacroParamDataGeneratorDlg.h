#pragma once
#include "afxdialogex.h"
#include "NFmiViewPosRegistryInfo.h"

class SmartMetDocumentInterface;
class NFmiMacroParamDataGenerator;

// CFmiMacroParamDataGeneratorDlg dialog

class CFmiMacroParamDataGeneratorDlg : public CDialogEx
{
	// näillä talletetaan sijainti ja koko rekisteriin
	static const NFmiViewPosRegistryInfo s_ViewPosRegistryInfo;

	DECLARE_DYNAMIC(CFmiMacroParamDataGeneratorDlg)

public:
	static const NFmiViewPosRegistryInfo& ViewPosRegistryInfo(void) { return s_ViewPosRegistryInfo; }
	static std::string MakeUsedWinRegistryKeyStr(unsigned int /* theMapViewDescTopIndex */) { return ViewPosRegistryInfo().WinRegistryKeyStr(); }

	CFmiMacroParamDataGeneratorDlg(SmartMetDocumentInterface* smartMetDocumentInterface, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CFmiMacroParamDataGeneratorDlg();
	void SetDefaultValues(void);
	void Update();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CONSEPT_DATA_GENERATOR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	void InitDialogTexts();
	void InitControlsFromDocument();
	void DoFullInputChecks();

	// itsSmartMetDocumentInterface ei omista, ei tuhoa
	SmartMetDocumentInterface* itsSmartMetDocumentInterface;
	// itsMacroParamDataGenerator ei omista, ei tuhoa
	NFmiMacroParamDataGenerator* itsMacroParamDataGenerator;
	// Annetaan esim. arvo T_Ec, joka tarkoittaa se Ecmwf pintadata, jossa on lämpötila parametri
	CString itsBaseDataParamProducerLevelString;
	// Annetaan tuotettavan MacroParam datan tuottaja tiedot, esim. 5601,My Producer Name
	CString itsProducerIdNamePairString;
	// Polku siihen smarttool skriptiin, jonka avulla MacroParam dataan lasketaan arvot
	CString itsUsedDataGenerationSmarttoolPath;
	// Annetaan pilkuilla erotettu lista parametreja (id,name pareina), joista muodostuu MacroParam datan parametrit
	CString itsUsedParameterListString;
	// Tuotettavan datan polku ja file filter, esim. c:\data\*_mydata.sqd, missä * korvataan YYYYMMDDHHmmSS timestamp tekohetkestä
	CString itsGeneratedDataStorageFileFilter;
public:
	virtual BOOL OnInitDialog();
};
