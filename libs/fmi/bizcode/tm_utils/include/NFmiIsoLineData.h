
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

class NFmiIsoLineData
{
public:
	NFmiIsoLineData(void)
	:itsIsolineData()
	,fDrawLabelsOverContours(false)
	,itsDefRGBRowSize(0)
	,itsUsedColorsCube(0)
	{ // ei alusteta muita juttuja konstruktorissa, n‰it‰ ei taas alusteta mitenk‰‰n init-metodissa
	}
	~NFmiIsoLineData()
	{
	}

    void SetIsolineData(const NFmiDataMatrix<float> &isolineData);
	void ResizeDefRGBArray(size_t newRowSize);

	void Init(int xCount, int yCount, int theMaxAllowedIsoLineCount, int theParamFieldCount = 1);
	void InitDrawOptions(const NFmiIsoLineData &theOther);
	float InterpolatedValue(const NFmiPoint &thePoint);

	checkedVector<float> itsVectorFloatGridData; // itsYNumber * itsXNumber float gridded data
	int itsXNumber;
	int itsYNumber;
	int itsMaxAllowedIsoLineCount; // n‰in paljon varataan tilaa eri arvoisille isoviivoille (siis v‰ritys ja muut attribuutit)

	NFmiDataIdent itsParam;
	NFmiMetTime itsTime;
	NFmiDataMatrix<float> itsIsolineData; // normaali data, jos kyse flow-chart piirrosta, t‰m‰ prim‰‰ri data eli tuulen (tai jonkun) nopeus, m‰‰r‰‰ nuolen pituuden ja paksuuden

	int fUseIsoLines; // 0= ei isoviivojen piirtoa 1=piirret‰‰n
	int fUseColorContours; // 0= ei v‰ri sheidausta 1=piirret‰‰n v‰ri sheidaus, 2=quickcontour!!!
	int itsTrueIsoLineCount; // pit‰‰ olla pienempi tai yht‰suuri kuin kFmiIsoLineMaxNumber
	int itsTrueColorContoursCount; // pit‰‰ olla pienempi tai yht‰suuri kuin kFmiIsoLineMaxNumber

	checkedVector<int> itsIsoLineColor; // indeksi agX coloriin
	checkedVector<int> itsIsoLineLabelColor; // indeksi agX coloriin
	checkedVector<int> itsIsoLineStyle; // indeksi viiva tyyliin,0=yht.viiva,1=tihe‰ pisteviiva,jne.
	checkedVector<float> itsIsoLineWidth; // t‰m‰ on suhteellinen paksuus riippuu ruudun koosta
	checkedVector<float> itsIsoLineAnnonationHeight; // miten isolla piirret‰‰n isoviivan arvo (0:lla ei piirret‰), suhteellinen ruudun kokoon
	bool fUseLabelBox;
    int itsIsoLineBoxFillColorIndex;
	bool fUseSingleColorsWithSimpleIsoLines; // jos true, yksiv‰riset 'simppelit' isoviivat, muuten k‰ytet‰‰n luokka kohtaisia v‰rej‰
	bool fUseIsoLineFeathering;
	bool fUseCustomIsoLineClasses; // jos 1, k‰ytet‰‰n k‰ytt‰j‰n m‰‰r‰‰mi‰ rajaarvoja, jos arvo 0, k‰ttet‰‰n jotain tasa steppi‰
	bool fUseCustomColorContoursClasses; // jos 1, k‰ytet‰‰n k‰ytt‰j‰n m‰‰r‰‰mi‰ rajaarvoja, jos arvo 0, k‰ttet‰‰n jotain tasa steppi‰
	bool fUseSeparatorLinesBetweenColorContourClasses; // piirr‰ viivat arvo/v‰ri luokkien v‰lille
	bool fUseIsoLineGabWithCustomContours;
	bool fDrawLabelsOverContours; // t‰m‰ optio on vain imagine piirrolle, ToolMaster piirrossa t‰m‰ hoidetaan toisin. T‰m‰ on siis true (imagine piirrossa), jos k‰ytet‰‰n isoline+contour piirtoa
	int itsColorIndexCount; // k‰ytet‰‰n fUseIsoLineGabWithCustomContours-option kanssa
	float itsIsoLineStep; // k‰ytet‰‰n t‰t‰, jos fUseCustomIsolineClasses == 0
	float itsColorContoursStep; // k‰ytet‰‰n t‰t‰, jos fUseCustomIsolineClasses == 0
	checkedVector<float> itsCustomIsoLineClasses; // t‰nne voi m‰‰ritt‰‰ millaiset luokat haluaa esim. 1,2,4,8,...
	checkedVector<float> itsCustomColorContours; // t‰nne voi m‰‰ritt‰‰ millaiset luokat haluaa esim. 1,2,4,8,...
	checkedVector<int> itsCustomColorContoursColorIndexies; // t‰nne voi m‰‰ritt‰‰ millaiset luokat haluaa esim. 1,2,4,8,...

	float itsIsoLineSplineSmoothingFactor; // 0=ei pyˆristyst‰, 10 maksimi (ei kannata k‰ytt‰‰ jos piirret‰‰n samaal colorcontoureja, koska contoureja ei pyˆristet‰)
	int itsIsoLineLabelDecimalsCount; // kuinka monta desimaalia k‰ytet‰‰n
	float itsIsoLineZeroClassValue; // jos steppaavat isoviivat, mink‰ arvon kautta isoviivat menev‰t
	float itsIsoLineStartClassValue; // jos steppaavat isoviivat, t‰m‰ arvo pit‰‰ laskea, t‰st‰ arvosta alkaa steputus
	float itsMinValue; // colorcontour alku arvo (kun k‰ytet‰‰n tasa steppi‰)
	float itsMaxValue; // colorcontour loppu arvo (kun k‰ytet‰‰n tasa steppi‰)
    double itsIsolineMinLengthFactor = 1;

	NFmiIsoLineStatistics itsIsoLineStatistics;
	NFmiIsoLineStatistics itsColorContoursStatistics;
	NFmiIsoLineDrawingSettings itsIsoLineDrawSettings;

	boost::shared_ptr<NFmiFastQueryInfo> itsInfo;

	NFmiHatchingSettings itsHatch1;
	NFmiHatchingSettings itsHatch2;

	float itsDefRGB[s_DefRGBRowCapasity][s_DefRGBColumnCapasity];
	size_t itsDefRGBRowSize; // T‰ss‰ on todellinen v‰ri taulu lukum‰‰r‰
	Matrix3D<std::pair<int, COLORREF> > *itsUsedColorsCube; // ei omista, ei tuhoa
};

