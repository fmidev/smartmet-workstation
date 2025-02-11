//**********************************************************
// C++ definitions for nmetedit-library
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: FmiNMeteditLibraryDefinitions.h
//
//  Author         : pietarin
//  Creation Date  : Mon - Nov 16, 1998
//
//
//  Description:
//   Header that will include all the meteditor specific definitions.
//
//  Change Log:
//
//**********************************************************


#pragma once

typedef enum
{
	kFmiNoCommand = 0,
	kFmiAddView = 1,
	kFmiAddAsOnlyView = 2,
	kFmiRemoveView = 3,
	kFmiRemoveAllViews = 4,
	kFmiEditParameter = 5,
	kFmiAddMask = 6,
	kFmiRemoveMask = 7,
	kFmiModifyMask = 8,
	kFmiRemoveAllMasks = 9,
	kFmiDisAbleMask = 10,
	kFmiEnAbleMask = 11,
	kFmiHideView = 12,
	kFmiShowView = 13,
	kFmiModifyView = 14,
	kFmiAddAsOnlyMask = 15,
	kFmiModifyDrawParam = 16,
	kFmiAddTimeSerialView = 17,
	kFmiRemoveTimeSerialView = 18,
	kFmiActivateView = 19, // 1999.09.20/Marko
	// kFmiToggleShowDifference = 20, // Parametrin vaihtelu aika-askeleen aikana poistettus ominaisuus
	kFmiRemoveAllTimeSerialViews = 21, // 1999.03.14/Marko
	kFmiEnableCP = 22, // 2000.11.28/Marko
	kFmiDisableCP = 23, // 2000.11.28/Marko
	kFmiMakeCPChangeInTime = 24, // 2000.11.28/Marko
	kFmiMakeCPNotChangeInTime = 25, // 2000.11.28/Marko
	kFmiModifyCPAttributes = 26, // 2000.11.28/Marko
	kFmiActivateCP = 27, // 2000.11.28/Marko
	kFmiDeactivateCP = 28, // 2000.11.28/Marko
	kFmiShowCPAllwaysOnTimeView = 29, // 2001.04.17/Marko
	kFmiDontShowCPAllwaysOnTimeView = 30, // 2001.04.17/Marko
	kFmiShowAllCPsAllwaysOnTimeView = 31, // 2001.04.17/Marko
	kFmiShowOnlyActiveCPOnTimeView = 32, // 2001.04.17/Marko
	kFmiShowHelperDataOnTimeSerialView = 33, // 2002.01.28/Marko // näytetään mm. viimeisin kepa-data ja klimatologia aikasarjassa editoinnin avuksi
	kFmiDontShowHelperDataOnTimeSerialView = 34, // 2002.01.28/Marko
	kFmiToggleShowDifferenceToOriginalData = 35, // 2002.03.14/Marko // näytä kartalla parametrin erotus alkuperäiseen dataan
	kFmiShowHelperData2OnTimeSerialView = 36, // 2004.05.5/Marko // näytetään fuzzy + verifikaatio datat
	kFmiDontShowHelperData2OnTimeSerialView = 37, // 2002.05.5/Marko

	// Alkaa - poikkileikkaus näyttöön liittyviä (popup-)käskyjä
	kFmiAddParamCrossSectionView = 38,
	kFmiAddAsOnlyParamCrossSectionView = 39,
	kFmiDontShowPressureLevelsCrossSectionView = 40,
	kFmiShowPressureLevelsUnderCrossSectionView = 41,
	kFmiShowPressureLevelsOverCrossSectionView = 42,
	kFmiRemoveParamCrossSectionView = 43, // poistaa riviltä halutun parametrin
	kFmiRemoveAllParamsCrossSectionView = 44, // poistaa riviltä kaikki parametrit
	kFmiHideParamCrossSectionView = 45, // piilottaa halutun parametrin riviltä (näkyy tooltipissä)
	kFmiShowParamCrossSectionView = 46, // näyttää piilotetun parametrin
	kFmiModifyCrossSectionDrawParam = 47,
	// Loppuu - poikkileikkaus näyttöön liittyviä käskyjä

	kFmiHideAllMapViewObservations = 48,
	kFmiShowAllMapViewObservations = 49,
	kFmiHideAllMapViewForecasts = 50,
	kFmiShowAllMapViewForecasts = 51,
	kFmiCrossSectionSetTrajectoryTimes = 52,
	kFmiCrossSectionSetTrajectoryParams = 53,
	kFmiCopyDrawParamOptions = 54, // copy/paste toiminnot piirto-ominaisuuksia varten (toimivat parmetrinäytön popup-valikosta)
	kFmiPasteDrawParamOptions = 55,
	kFmiCopyDrawParamOptionsCrossSectionView = 56, // copy/paste toiminnot piirto-ominaisuuksia varten (toimivat parmetrinäytön popup-valikosta)
	kFmiPasteDrawParamOptionsCrossSectionView = 57,
	kFmiCopyDrawParamsFromMapViewRow = 59,
	kFmiPasteDrawParamsToMapViewRow = 60,
	kFmiCopyMapViewDescTopParams = 61,
	kFmiPasteMapViewDescTopParams = 62,
	kFmiChangeAllProducersInMapRow = 63,
	kFmiChangeAllProducersInCrossSectionRow = 64,
	kFmiCopyDrawParamsFromCrossSectionViewRow = 65,
	kFmiPasteDrawParamsToCrossSectionViewRow = 66,

	kFmiModelRunOffsetPrevious = 67,
	kFmiTimeSerialModelRunCountSet = 68,
	kFmiModelRunOffsetNext = 69,
	kFmiModelRunFindNearestBefore = 70,
	kFmiActivateCrossSectionDrawParam = 71,

	kFmiChangeAllDataTypesInMapRow = 72,
	kFmiChangeAllDataTypesInCrossSectionRow = 73,
	//kFmiDiffBetweenModelRuns = 74, // Parametrin vaihtelu malliajojen välillä on poistettus ominaisuus
	//kFmiDiffToAnalyzeData = 75, // Parametrin vaihtelu verrattuna johonkin analyysi dataan on poistettus ominaisuus
    kFmiSelectCPManagerFromSet = 76,
    kFmiFixedDrawParam = 77, // Ladataan käyttöön haluttu fixedDrawParam
    kFmiStoreDrawParam = 78, // Tällä talletetaan tehdyt muutokset parametrin piirto-ominaisuuksiin (tiedostoon asti), jotka on tehty DrawParamOptions -dialogin ulkopuolella (esim. valittu joku FixedDrawParam)
    kFmiReloadDrawParam = 79, // Jos on valittu joku FixeddrawParam käyttöön, mutta halutaan palauttaa originaali asetukset käyttöön
    kFmiRemoveAllParamsFromAllRows = 80,
    kFmiShowHelperData3OnTimeSerialView = 81,
    kFmiDontShowHelperData3OnTimeSerialView = 82,
    kAddViewWithRealRowNumber = 83,
    kFmiShowHelperData4OnTimeSerialView = 84,
    kFmiDontShowHelperData4OnTimeSerialView = 85,
    kFmiObservationStationsToCpPoints = 86,
    kFmiToggleShowLegendState = 87,
	kFmiSwapViewRows = 88,
	kFmiAddBorderLineLayer = 89,
	kFmiMoveBorderLineLayer = 90,
	kFmiInsertParamLayer = 91,
	kFmiSelectBackgroundMapLayer = 92,
	kFmiSelectOverlayMapLayer = 93,
	kFmiAddTimeSerialSideParam = 94,
	kFmiRemoveSelectedTimeSerialSideParam = 95,
	kFmiRemoveAllTimeSerialRowSideParams = 96,
	kFmiChangeParam = 97,
	kFmiSetTimeBoxLocation = 98,
	kFmiSetTimeBoxTextSizeFactor = 99,
	kFmiSetTimeBoxFillColor = 100,
	kFmiSetTimeBoxFillColorAlpha = 101,
	kFmiSetTimeBoxCustomFillColor = 102,
	kFmiSetTimeBoxToDefaultValues = 103,
	kFmiModifyMacroParamFormula = 104
}FmiMenuCommandType;

// sää koodi annetaan tänne. Jos se on AWS wawa koodia (arvo 100-199), konvertoidaan se WW-koodiksi, muuten
// palautetaan annettu koodi.
// konversio taulukko otettu fortran funktiosta:
//	INTEGER FUNCTION wawa2ww(wawa, miva)
//
//	IMPLICIT none
//
//	INTEGER ww4677(99)/
//	1	1, 2, 3, 4, 5, 0, 0, 0, 0,
//	2   10, 0,13, 0, 0, 0, 0, 0,18, 0,
//	3   28,21,20,21,22,24,29, 0, 0, 0,
//	4   42,41,43,45,47,48, 0, 0, 0, 0,
//	5   61,63,65,61,65,71,75,66,67, 0,
//	6   50,51,53,55,56,57,57,58,59, 0,
//	7   60,61,63,65,66,67,67,68,69, 0,
//	8   70,71,73,75,79,79,79,77,78, 0,
//	9   80,80,81,81,82,85,86,86, 0,89,
//	9   92,17,93,96,17,97,99, 0, 0, 8/
//
//	INTEGER wawa, miva
//
//	IF (wawa .LT. 0 .OR. wawa .GE. 100) THEN
//	    wawa2ww = miva
//	ELSE IF (wawa .EQ. 0) THEN
//	    wawa2ww = 0
//	ELSE
//	    wawa2ww = ww4677(wawa)
//	END IF
//
//	RETURN
//	END
inline float ConvertPossible_WaWa_2_WW(float theValue)
{
	static const float wwCodeArray[100] =
	{0, 1, 2, 3, 4, 5, 0, 0, 0, 0,
	10, 0,13, 0, 0, 0, 0, 0,18, 0,
	28,21,20,21,22,24,29, 0, 0, 0,
	42,41,43,45,47,48, 0, 0, 0, 0,
	61,63,65,61,65,71,75,66,67, 0,
	50,51,53,55,56,57,57,58,59, 0,
	60,61,63,65,66,67,67,68,69, 0,
	70,71,73,75,79,79,79,77,78, 0,
	80,80,81,81,82,85,86,86, 0,89,
	92,17,93,96,17,97,99, 0, 0, 8};

	if(theValue >= 100 && theValue <= 199)
		return wwCodeArray[static_cast<int>(theValue-100)];
	return theValue;
}

