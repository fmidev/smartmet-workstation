
#pragma once

#include "stdafx.h"

#include "NFmiDataIdent.h"
#include "NFmiMetTime.h"
#include "NFmiInterpolation.h"
#include "matrix3d.h"
#include "NFmiDataMatrix.h"
#include "ColorContouringData.h"
#include "IsolineVizualizationData.h"
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

// NFmiContourUserDrawData luokka pit�� sis�ll��n contour-user-draw piirron dataa:
// 1. matriisin koordinaateista (suhteelliset tms.), jotka eiv�t ole normi suorakulmaisen hilan pisteet. 
// 2. Niihin liittyv�t x- ja y-koordinaateille on vektorit, joihin em. matriisi on purettuna siten kuin Toolmaster x/y-koordinaatit haluaa XuContourUserDraw funktiolle.
// 3. Em. koordinaatteihin liittyv� value matriisi, jota siis k�ytet��n vain contour piirrrossa. 
// Jos k�yt�ss� isoviiva+contour yhdistelm�, pit�� isoviivoille olla oma suorakulmainen matriisi NFmiIsoLineData luokassa.
// Lis�ksi on UseUserDraw -metodi, jolla voidaan kysy� ett� onko user-draw datat k�yt�ss� vai ei.
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
	// Vektori johon on sijoitettu kaikki matriisin arvot siin� j�rjestyksess� kuin toolmaster userDraw funktio ne haluaa
	std::vector<float> itsUserDrawValues; 

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

	bool InitIsoLineData(const NFmiDataMatrix<float>& theValueMatrix, NFmiIsoLineData *otherIsoLineData = nullptr);
	void InitContourUserDrawData(const NFmiDataMatrix<float>& valueMatrix, const NFmiDataMatrix<NFmiPoint>& coordinateMatrix);
	bool UseContourUserDraw() const;

	// Data osiot on laitettu t��ll� julkisiksi
	// ========================================
	std::vector<float> itsVectorFloatGridData; // itsYNumber * itsXNumber float gridded data
	int itsXNumber = 0;
	int itsYNumber = 0;

	NFmiDataIdent itsParam;
	NFmiMetTime itsTime;
	NFmiDataMatrix<float> itsIsolineData; // value matriisi

	int fUseIsoLines = 0; // 0= ei isoviivojen piirtoa 1=piirret��n
	int fUseColorContours = false; // 0= ei v�ri sheidausta 1=piirret��n v�ri sheidaus, 2=quickcontour!!!
	int itsTrueIsoLineCount = 0; // pit�� olla pienempi tai yht�suuri kuin kFmiIsoLineMaxNumber
	int itsTrueColorContoursCount = 0; // pit�� olla pienempi tai yht�suuri kuin kFmiIsoLineMaxNumber

	bool fUseLabelBox = false;
	int itsIsoLineBoxFillColorIndex = 0;
	bool fUseSingleColorsWithSimpleIsoLines = true; // jos true, yksiv�riset 'simppelit' isoviivat, muuten k�ytet��n luokka kohtaisia v�rej�
	bool fUseIsoLineFeathering = false;
	bool fUseCustomIsoLineClasses = false; // jos 1, k�ytet��n k�ytt�j�n m��r��mi� rajaarvoja, jos arvo 0, k�ttet��n jotain tasa steppi�
	bool fUseCustomColorContoursClasses = false; // jos 1, k�ytet��n k�ytt�j�n m��r��mi� rajaarvoja, jos arvo 0, k�ttet��n jotain tasa steppi�
	bool fUseSeparatorLinesBetweenColorContourClasses = false; // piirr� viivat arvo/v�ri luokkien v�lille
	bool fUseIsoLineGabWithCustomContours = false;
	bool fDrawLabelsOverContours = false; // t�m� optio on vain imagine piirrolle, ToolMaster piirrossa t�m� hoidetaan toisin. T�m� on siis true (imagine piirrossa), jos k�ytet��n isoline+contour piirtoa

	// itsIsolineData matriisin min/max arvot
	float itsDataMinValue = 3.4E+38f;
	float itsDataMaxValue = -3.4E+38f;
	// Varsinaisten k�ytettyjen isoline/contour rajojen min/max arvot
	float itsClassMinValue = 0;
	float itsClassMaxValue = 0;
	double itsIsolineMinLengthFactor = 1;

	boost::shared_ptr<NFmiFastQueryInfo> itsInfo;

	NFmiHatchingSettings itsHatch1;
	NFmiHatchingSettings itsHatch2;

	float itsDefRGB[s_DefRGBRowCapasity][s_DefRGBColumnCapasity];
	size_t itsDefRGBRowSize = 0; // T�ss� on todellinen v�ri taulu lukum��r�
	Matrix3D<std::pair<int, COLORREF> >* itsUsedColorsCube = nullptr; // ei omista, ei tuhoa
	NFmiContourUserDrawData itsContourUserDrawData;

	// Yhden karttaruudun korkeus millimetreiss�, tarvitaan hatch laskuissa
	double itsSingleSubMapViewHeightInMillimeters = 100.;
	double itsDataGridToViewHeightRatio = 1.;
	ColorContouringData itsColorContouringData;
	IsolineVizualizationData itsIsolineVizualizationData;
	bool fUseOriginalDataInPixelToGridRatioCalculations = false;
private:
	void BaseInitialization(const NFmiDataMatrix<float>& theValueMatrix);
	void DoBaseInitializationReset();
	void InitDrawOptions(const NFmiIsoLineData& theOther);
};

