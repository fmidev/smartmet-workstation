
#pragma once

#include "stdafx.h"

#include "NFmiDataIdent.h"
#include "NFmiMetTime.h"
#include "NFmiInterpolation.h"
#include "matrix3d.h"
#include "NFmiDataMatrix.h"
#include "boost/shared_ptr.hpp"

class NFmiFastQueryInfo;

typedef struct IsoLineStatistics_
{
	void Init(void);

	float itsMissingValue;
	float itsMinValue;
	float itsMinClassValue; // mik‰ on alin luokka arvo
	float itsMaxValue;
	float itsMaxClassValue; // mik‰ on ylin luokka arvo
	float itsMeanValue; // tarvitaanko?
}NFmiIsoLineStatistics;

typedef struct IsoLineDrawingSettings_
{
	void Init(void);

	float itsStartValue;
	float itsEndValue;
	float itsIsoLineStep;
	int itsNumberOfIsoLines;
}NFmiIsoLineDrawingSettings;

class NFmiHatchingSettings
{
public:
	void Init(void)
	{
		fUseHatch = false;
		fDrawHatchBorders = false;
		itsHatchPattern = 0;
		itsHatchColorIndex = 0;
		itsHatchColorRef = 0;
		itsHatchLowerLimit = 0;
		itsHatchUpperLimit = 1;
	}

	bool fUseHatch;
	bool fDrawHatchBorders;
	int itsHatchPattern;
	int itsHatchColorIndex;
	COLORREF itsHatchColorRef;
	float itsHatchLowerLimit;
	float itsHatchUpperLimit;

};

const size_t s_DefRGBRowCapasity = 100;
const size_t s_DefRGBColumnCapasity = 4;

// T‰m‰ luokka pit‰‰ sis‰ll‰‰n matriisin koordinaateista (suhteelliset tms.), jotka eiv‰t ole normi 
// suorakulmaisen hilan pisteet. Lis‰ksi x- ja y-koordinaateille on vektorit, joihin em. matriisi
// on purettuna siten kuin Toolmaster x/y-koordinaatit haluaa XuContourUserDraw ja XuIsolineUserDraw funktioille.
// Lis‰ksi on UseUserDraw -funktio, jolla voidaan kysy‰ ett‰ onko user-draw datat k‰ytˆss‰ vai ei.
class NFmiUserGridData
{
public:
	void Init(const NFmiDataMatrix<NFmiPoint> &coordinateMatrix);
	bool UseUserDraw() const;

	NFmiDataMatrix<NFmiPoint> coordinateMatrix_;
	std::vector<float> xCoordinates;
	std::vector<float> yCoordinates;
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
	void InitUserGridCoordinateData(const NFmiDataMatrix<NFmiPoint>& coordinateMatrix);
	bool UseUserDraw() const;
	static size_t Matrix2ToolmasterIndex(size_t gridSizeX, size_t yIndex, size_t xIndex);

	void GetMinAndMaxValues(float& theMinOut, float& theMaxOut) const;
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
	float itsMinValue = 3.4E+38f; // colorcontour alku arvo (kun k‰ytet‰‰n tasa steppi‰)
	float itsMaxValue = -3.4E+38f; // colorcontour loppu arvo (kun k‰ytet‰‰n tasa steppi‰)
	double itsIsolineMinLengthFactor = 1;

	NFmiIsoLineStatistics itsIsoLineStatistics;
	NFmiIsoLineStatistics itsColorContoursStatistics;
	NFmiIsoLineDrawingSettings itsIsoLineDrawSettings;

	boost::shared_ptr<NFmiFastQueryInfo> itsInfo;

	NFmiHatchingSettings itsHatch1;
	NFmiHatchingSettings itsHatch2;

	float itsDefRGB[s_DefRGBRowCapasity][s_DefRGBColumnCapasity];
	size_t itsDefRGBRowSize = 0; // T‰ss‰ on todellinen v‰ri taulu lukum‰‰r‰
	Matrix3D<std::pair<int, COLORREF> >* itsUsedColorsCube = nullptr; // ei omista, ei tuhoa
	NFmiUserGridData itsUserGridCoordinateData;
private:
	void BaseInitialization(const NFmiDataMatrix<float>& theValueMatrix, int theMaxAllowedIsoLineCount);
	void DoBaseInitializationReset();
};

