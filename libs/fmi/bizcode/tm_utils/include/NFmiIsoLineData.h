
#pragma once

#include "stdafx.h"

#include "NFmiDataIdent.h"
#include "NFmiMetTime.h"
#include "NFmiInterpolation.h"
#include "matrix3d.h"
#include "NFmiDataMatrix.h"
#include "boost/shared_ptr.hpp"

class NFmiFastQueryInfo;

class NFmiHatchingSettings
{
public:
	NFmiHatchingSettings() = default;

	bool fUseHatch = false;
	bool fDrawHatchBorders = false;
	int itsHatchPattern = 0;
	int itsHatchColorIndex = 0;
	COLORREF itsHatchColorRef = 0;
	float itsHatchLowerLimit = 0;
	float itsHatchUpperLimit = 1;

};

const size_t s_DefRGBRowCapasity = 100;
const size_t s_DefRGBColumnCapasity = 4;

// NFmiContourUserDrawData luokka pit‰‰ sis‰ll‰‰n contour-user-draw piirron dataa:
// 1. matriisin koordinaateista (suhteelliset tms.), jotka eiv‰t ole normi suorakulmaisen hilan pisteet. 
// 2. Niihin liittyv‰t x- ja y-koordinaateille on vektorit, joihin em. matriisi on purettuna siten kuin Toolmaster x/y-koordinaatit haluaa XuContourUserDraw funktiolle.
// 3. Em. koordinaatteihin liittyv‰ value matriisi, jota siis k‰ytet‰‰n vain contour piirrrossa. 
// Jos k‰ytˆss‰ isoviiva+contour yhdistelm‰, pit‰‰ isoviivoille olla oma suorakulmainen matriisi NFmiIsoLineData luokassa.
// Lis‰ksi on UseUserDraw -metodi, jolla voidaan kysy‰ ett‰ onko user-draw datat k‰ytˆss‰ vai ei.
class NFmiContourUserDrawData
{
public:
	void Init(const NFmiDataMatrix<float>& valueMatrix, const NFmiDataMatrix<NFmiPoint> &coordinateMatrix);
	bool UseUserDraw() const;

	int itsXNumber = 0;
	int itsYNumber = 0;
	NFmiDataMatrix<NFmiPoint> coordinateMatrix_;
	std::vector<float> xCoordinates;
	std::vector<float> yCoordinates;

	// value matriisi
	NFmiDataMatrix<float> itsUserDrawValuesMatrix;
	// Vektori johon on sijoitettu kaikki matriisin arvot siin‰ j‰rjestyksess‰ kuin toolmaster userDraw funktio ne haluaa
	checkedVector<float> itsUserDrawValues; 

	// UserDraw value matriisin min/max arvot
	float itsDataMinValue = 3.4E+38f;
	float itsDataMaxValue = -3.4E+38f;
};

class NFmiIsoLineData
{
public:
	~NFmiIsoLineData()
	{
	}

	void SetIsolineData(const NFmiDataMatrix<float>& isolineData);
	void ResizeDefRGBArray(size_t newRowSize);

	bool Init(const NFmiDataMatrix<float>& theValueMatrix, int theMaxAllowedIsoLineCount = NFmiIsoLineData::DefaultMaxAllowedIsoLineCount);
	void InitDrawOptions(const NFmiIsoLineData& theOther);
	float InterpolatedValue(const NFmiPoint& thePoint);
	void InitContourUserDrawData(const NFmiDataMatrix<float>& valueMatrix, const NFmiDataMatrix<NFmiPoint>& coordinateMatrix);
	bool UseContourUserDraw() const;
	bool UseContourDraw() const;

	// Kuinka monta eriarvoista isoviivaa laitetaan laskuin oletuksena. 
	// Mit‰ isompi luku, sit‰ kauemmin voi tietyiss‰ tilanteissa 'mustan mˆssˆn' piirto kest‰‰ visualisoinneissa.
	// T‰ll‰ siis yritet‰‰n est‰‰ ett‰ v‰‰rin asetetuilla piirtoasetuksilla kone jumittaa mahdottoman kauan ja piirt‰‰ ruudulle tuhansittain isoviivoja vieriviereen.
	static const int DefaultMaxAllowedIsoLineCount = 500;

	// Data osiot on laitettu t‰‰ll‰ julkisiksi
	// ========================================
	checkedVector<float> itsVectorFloatGridData; // itsYNumber * itsXNumber float gridded data
	int itsXNumber = 0;
	int itsYNumber = 0;
	int itsMaxAllowedIsoLineCount = 0; // n‰in paljon varataan tilaa eri arvoisille isoviivoille (siis v‰ritys ja muut attribuutit)

	NFmiDataIdent itsParam;
	NFmiMetTime itsTime;
	NFmiDataMatrix<float> itsIsolineData; // value matriisi

	int fUseIsoLines = 0; // 0= ei isoviivojen piirtoa 1=piirret‰‰n
	int fUseColorContours = false; // 0= ei v‰ri sheidausta 1=piirret‰‰n v‰ri sheidaus, 2=quickcontour!!!
	int itsTrueIsoLineCount = 0; // pit‰‰ olla pienempi tai yht‰suuri kuin kFmiIsoLineMaxNumber
	int itsTrueColorContoursCount = 0; // pit‰‰ olla pienempi tai yht‰suuri kuin kFmiIsoLineMaxNumber

	checkedVector<int> itsIsoLineColor; // indeksi agX coloriin
	checkedVector<int> itsIsoLineLabelColor; // indeksi agX coloriin
	checkedVector<int> itsIsoLineStyle; // indeksi viiva tyyliin,0=yht.viiva,1=tihe‰ pisteviiva,jne.
	checkedVector<float> itsIsoLineWidth; // t‰m‰ on suhteellinen paksuus riippuu ruudun koosta
	checkedVector<float> itsIsoLineAnnonationHeight; // miten isolla piirret‰‰n isoviivan arvo (0:lla ei piirret‰), suhteellinen ruudun kokoon
	bool fUseLabelBox = false;
	int itsIsoLineBoxFillColorIndex = 0;
	bool fUseSingleColorsWithSimpleIsoLines = true; // jos true, yksiv‰riset 'simppelit' isoviivat, muuten k‰ytet‰‰n luokka kohtaisia v‰rej‰
	bool fUseIsoLineFeathering = false;
	bool fUseCustomIsoLineClasses = false; // jos 1, k‰ytet‰‰n k‰ytt‰j‰n m‰‰r‰‰mi‰ rajaarvoja, jos arvo 0, k‰ttet‰‰n jotain tasa steppi‰
	bool fUseCustomColorContoursClasses = false; // jos 1, k‰ytet‰‰n k‰ytt‰j‰n m‰‰r‰‰mi‰ rajaarvoja, jos arvo 0, k‰ttet‰‰n jotain tasa steppi‰
	bool fUseSeparatorLinesBetweenColorContourClasses = false; // piirr‰ viivat arvo/v‰ri luokkien v‰lille
	bool fUseIsoLineGabWithCustomContours = false;
	bool fDrawLabelsOverContours = false; // t‰m‰ optio on vain imagine piirrolle, ToolMaster piirrossa t‰m‰ hoidetaan toisin. T‰m‰ on siis true (imagine piirrossa), jos k‰ytet‰‰n isoline+contour piirtoa
	int itsColorIndexCount = 0; // k‰ytet‰‰n fUseIsoLineGabWithCustomContours-option kanssa
	float itsIsoLineStep = 0; // k‰ytet‰‰n t‰t‰, jos fUseCustomIsolineClasses == 0
	float itsColorContoursStep = 0; // k‰ytet‰‰n t‰t‰, jos fUseCustomIsolineClasses == 0
	checkedVector<float> itsCustomIsoLineClasses; // t‰nne voi m‰‰ritt‰‰ millaiset luokat haluaa esim. 1,2,4,8,...
	checkedVector<float> itsCustomColorContours; // t‰nne voi m‰‰ritt‰‰ millaiset luokat haluaa esim. 1,2,4,8,...
	checkedVector<int> itsCustomColorContoursColorIndexies; // t‰nne voi m‰‰ritt‰‰ millaiset luokat haluaa esim. 1,2,4,8,...

	float itsIsoLineSplineSmoothingFactor = 0; // 0=ei pyˆristyst‰, 10 maksimi (ei kannata k‰ytt‰‰ jos piirret‰‰n samaal colorcontoureja, koska contoureja ei pyˆristet‰)
	int itsIsoLineLabelDecimalsCount = 0; // kuinka monta desimaalia k‰ytet‰‰n
	float itsIsoLineZeroClassValue = 0; // jos steppaavat isoviivat, mink‰ arvon kautta isoviivat menev‰t
	float itsIsoLineStartClassValue = 0; // jos steppaavat isoviivat, t‰m‰ arvo pit‰‰ laskea, t‰st‰ arvosta alkaa steputus
	// itsIsolineData matriisin min/max arvot
	float itsDataMinValue = 3.4E+38f;
	float itsDataMaxValue = -3.4E+38f;
	// itsIsolineData matriisin min/max arvot
	float itsClassMinValue = 0;
	float itsClassMaxValue = 0;
	double itsIsolineMinLengthFactor = 1;

	boost::shared_ptr<NFmiFastQueryInfo> itsInfo;

	NFmiHatchingSettings itsHatch1;
	NFmiHatchingSettings itsHatch2;

	float itsDefRGB[s_DefRGBRowCapasity][s_DefRGBColumnCapasity];
	size_t itsDefRGBRowSize = 0; // T‰ss‰ on todellinen v‰ri taulu lukum‰‰r‰
	Matrix3D<std::pair<int, COLORREF> >* itsUsedColorsCube = nullptr; // ei omista, ei tuhoa
	NFmiContourUserDrawData itsContourUserDrawData;
private:
	void BaseInitialization(const NFmiDataMatrix<float>& theValueMatrix, int theMaxAllowedIsoLineCount);
	void DoBaseInitializationReset();
};

