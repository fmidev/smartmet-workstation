#include "NFmiCombinedMapHandler.h"
#include "NFmiMapViewDescTop.h"
#include "WmsSupportInterface.h"
#ifndef UNIX
#include "NFmiApplicationWinRegistry.h"
#endif
#include "NFmiMapConfigurationSystem.h"
#include "NFmiProjectionCurvatureInfo.h"
#include "NFmiGeoShape.h"
#include "NFmiPathUtils.h"
#include "NFmiGdiPlusImageMapHandler.h"
#include "NFmiQueryData.h"
#include "NFmiFastQueryInfo.h"
#include "CtrlViewFastInfoFunctions.h"
#include "CtrlViewFunctions.h"
#include "NFmiCrossSectionSystem.h"
#include "ApplicationInterface.h"
#include "NFmiProducerName.h"
#include "CtrlViewDocumentInterface.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiAnimationData.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiMTATempSystem.h"
#include "NFmiMacroParamDataCache.h"
#include "MacroParamDataChecker.h"
#include "TimeSerialModification.h"
#include "NFmiWindTableSystem.h"
#include "NFmiSatelliteImageCacheSystem.h"
#include "NFmiExtraMacroParamData.h"
#include "NFmiQueryDataUtil.h"
#include "CapDataSystem.h"
#include "SpecialDesctopIndex.h"
#include "NFmiMenuItem.h"
#include "NFmiFixedDrawParamSystem.h"
#include "NFmiFastDrawParamList.h"
#include "NFmiMacroParamSystem.h"
#include "NFmiMacroParamFolder.h"
#include "NFmiMacroParam.h"
#ifndef UNIX
#include "FmiModifyDrawParamDlg.h"
#endif
#include "SmartMetDocumentInterface.h"
#include "NFmiMacroPathSettings.h"
#ifndef DISABLE_CPPRESTSDK
#include "wmssupport/WmsSupportState.h"
#include "wmssupport/WmsClient.h"
#include "wmssupport/Setup.h"
#endif // DISABLE_CPPRESTSDK
#include "CtrlViewTimeConsumptionReporter.h"
#ifndef DISABLE_CPPRESTSDK
#include "wmssupport/ChangedLayers.h"
#endif // DISABLE_CPPRESTSDK
#include "NFmiBasicSmartMetConfigurations.h"

#ifndef DISABLE_CPPRESTSDK
#include "wmssupport/WmsSupport.h"
#endif // DISABLE_CPPRESTSDK

#include <boost/algorithm/string.hpp>
#include <boost/math/special_functions.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace
{
	// Laitetaan t�m� piiloon cpp tiedostoon, koska haluan tehd� Wms::WmsSupport -luokan objektin k�ytt��n t�nne,
	// miss� on koko luokan kaikki metodit k�yt�ss�, mutta palauttaa WmsSupportInterface -pointterin ulkopuolelle.
	// Lis�ksi kaikki #ifndef DISABLE_CPPRESTSDK -jutut saa piiloon my�s cpp:hen.
#ifndef DISABLE_CPPRESTSDK
	// T�h�n wmsSupport_:iin laitetaan kulloinkin k�yt�ss� oleva WmsSupport olio.
	// Koska on huomattu ett� esim. Beta-tuotteiden luonnissa jos k�ytet��n paljon wms dataa, on wms osio mennyt rikki 2-3 vuorokauden kuluttua.
	// Nyt on siis tarkoitus ett� WmsSupport olio vaihdetaan m��r�ajoin. T�m� vaihto pit�� tehd� niin ett� vaikka
	// 12 tunnin v�lein MainFramen timer k�ynnist�� vaihto operaation, jolloin tapahtuu seuraavaa:
	// 1) Luodaan uusi WmsSupport olio tmpWmsSupport_ muuttujalle
	// 2) Mutex lukon turvissa vaihdetaan swap:illa wmsSupport_ ja tmpWmsSupport_ olioiden sis�lt�
	// 3) Nyt tmpWmsSupport_ oliolle annetaan k�sky tappaa itsens�
	// 4) Kun se on kuollut, nollataan tmpWmsSupport_ olio
	// 5) T�h�n soppaan luo omat hankaluutensa kun uutta WmsSupport oliota luodaan, t�ll�in pit�� tarkastella onko vanhaa haluttu jo tappaa, jolloin uusi pit�� my�s laittaa tappamaan itsens� ja mit��n vaihtoa ei en�� tehd�.
	std::shared_ptr<WmsSupportInterface> wmsSupport_;
	// capabilityTree_ olion k�ytt� pit�� suojata thread turvallisella lukolla.
	std::mutex wmsSupportMutex_;
	// Koska WmsSupport olion tappaminen vaatii odottelua, pit�� tehd� t�ll�inen tmp-oliolle oma kuolin paikka.
	// Eli kun wmsSupport_ oli on vaihdettu tmpWmsSupport_ olioon, voidaan odotella ett� se tappaa itsens� ja se voidaan deletoida.
	std::shared_ptr<WmsSupportInterface> tmpWmsSupport_;
	// T�m� atomic-flagin avulla estet��n samanaikaisten renewal prosessien k�ynnistyksen
	std::mutex isWmsSupportRenewalProcessRunningMutex_;
#endif // DISABLE_CPPRESTSDK

	static const std::string g_ObservationMenuName = "Observation";

	static const std::string gDummyParamName = "dummyName";

	static const char* CONFIG_MAPSYSTEMS = "MetEditor::MapSystems";
	static const char* CONFIG_MAPSYSTEM_PROJECTION_FILENAME = "MetEditor::MapSystem::%s::Projection::Filename";
	static const char* CONFIG_MAPSYSTEM_PROJECTION_DEFINITION = "MetEditor::MapSystem::%s::Projection::Definition";
	static const char* CONFIG_MAPSYSTEM_MAP = "MetEditor::MapSystem::%s::Map";
	static const char* CONFIG_MAPSYSTEM_MAP_FILENAME = "MetEditor::MapSystem::%s::Map::%s::Filename";
	static const char* CONFIG_MAPSYSTEM_MAP_DRAWINGSTYLE = "MetEditor::MapSystem::%s::Map::%s::DrawingStyle";
	static const char* CONFIG_MAPSYSTEM_LAYER = "MetEditor::MapSystem::%s::Layer";
	static const char* CONFIG_MAPSYSTEM_LAYER_FILENAME = "MetEditor::MapSystem::%s::Layer::%s::Filename";
	static const char* CONFIG_MAPSYSTEM_LAYER_DRAWINGSTYLE = "MetEditor::MapSystem::%s::Layer::%s::DrawingStyle";

	static const NFmiPoint kMissingLatlon(kFloatMissing, kFloatMissing);

	void tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters = " ")
	{
		// Skip delimiters at beginning.
		std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
		// Find first "non-delimiter".
		std::string::size_type pos = str.find_first_of(delimiters, lastPos);

		while(std::string::npos != pos || std::string::npos != lastPos)
		{
			// Found a token, add it to the vector.
			tokens.push_back(str.substr(lastPos, pos - lastPos));
			// Skip delimiters.  Note the "not_of"
			lastPos = str.find_first_not_of(delimiters, pos);
			// Find next "non-delimiter"
			pos = str.find_first_of(delimiters, lastPos);
		}
	}

	bool isTotalWorld(const std::shared_ptr<NFmiArea>& area)
	{
		if(area)
		{
			if(area->ClassId() == kNFmiLatLonArea)
			{
				if(area->PacificView())
				{
					if(area->BottomLeftLatLon() == NFmiPoint(0, -90) && area->TopRightLatLon() == NFmiPoint(360, 90))
						return true;
				}
				else
				{
					if(area->BottomLeftLatLon() == NFmiPoint(-180, -90) && area->TopRightLatLon() == NFmiPoint(180, 90))
						return true;
				}
			}
			return false;
		}
		else
			throw std::runtime_error("Error in IsTotalWorld: zero-pointer given as parameter.");
	}

	// T�ss� lasketaan tietynlaisille datoille 'minimi' n�ytt�jen likaus aikav�li.
// N�it� datoja ovat mm. tutka, mesan analyysi(, laps datat?). Niiden pit�� olla hila dataa.
// Dataa, mill� on yksiselitteiset valitimet, eli yksi havainto per aika.
// Jos palautettavan timebagin resoluutio on 0, ei timebagia k�ytet� jatkossa.
	NFmiTimeBag getDirtyViewTimes(NFmiQueryData* queryData, NFmiInfoData::Type dataType, const NFmiTimeDescriptor& removedDatasTimes)
	{
		NFmiTimeBag times; // t�ss� menee resoluutio 0:ksi, toivottavasti kukaan ei muuta default konstruktorin k�ytt�ytymist�.
		if(dataType == NFmiInfoData::kObservations || dataType == NFmiInfoData::kAnalyzeData)
		{
			if(queryData->Info()->Grid())
			{
				if(removedDatasTimes.LastTime() < queryData->Info()->TimeDescriptor().LastTime())
				{
					NFmiMetTime aTime = removedDatasTimes.LastTime();
					short step = static_cast<short>(queryData->Info()->TimeResolution());
					if(step == 0)
						step = static_cast<short>(removedDatasTimes.Resolution());
					if(step > 0)
					{
						aTime.SetTimeStep(step, true);
						aTime.NextMetTime();
					}
					else if(step == 0)
						step = 1; // asetetaan 0::sta poikkeava, muulla ei ole v�li�
					times = NFmiTimeBag(aTime, queryData->Info()->TimeDescriptor().LastTime(), step);
				}
			}
		}
		return times;
	}

	// HUOM! t�m� ei l�it� p�ivityksi� p��lle Parameter-selection dialogille, se hoidetaan toista kautta
	void setUpdatedViewIdMaskAfterDataLoaded(NFmiFastQueryInfo& fastInfo)
	{
		// Kaikkien datojen kanssa pit�� p�ivitt�� varmuuden vuoksi karttan�yt�t, aikasarja ja case-study
		ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews | SmartMetViewId::TimeSerialView | SmartMetViewId::CaseStudyDlg);
		if(fastInfo.SizeLevels() > 2)
		{
			// vertikaali datojen kanssa pit�� p�ivitt�� my�s luotaus, poikkileikkaus ja trajektori n�yt�t
			ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::SoundingView | SmartMetViewId::CrossSectionView | SmartMetViewId::TrajectoryView);
		}
		else if(fastInfo.SizeLevels() == 1)
		{
			// surface datojen kanssa pit�� p�ivitt�� my�s asema-data-taulukko, wind-table
			ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::StationDataTableView | SmartMetViewId::WindTableDlg);
		}
	}

	bool synopPlotNeedsUpdate(unsigned long drawParamParameterId, NFmiProducer& newDataProducer)
	{
		if((drawParamParameterId == NFmiInfoData::kFmiSpSynoPlot || drawParamParameterId == NFmiInfoData::kFmiSpMinMaxPlot) && newDataProducer.GetIdent() == kFmiSYNOP)
			return true;
		else
			return false;
	}

	bool metarPlotNeedsUpdate(unsigned long drawParamParameterId, NFmiProducer& newDataProducer)
	{
		if(drawParamParameterId == NFmiInfoData::kFmiSpMetarPlot && newDataProducer.GetIdent() == kFmiMETAR)
			return true;
		else
			return false;
	}

	bool soundingPlotNeedsUpdate(unsigned long drawParamParameterId, NFmiProducer& newDataProducer)
	{
		if(drawParamParameterId == NFmiInfoData::kFmiSpSoundingPlot && newDataProducer.GetIdent() == kFmiTEMP)
			return true;
		else
			return false;
	}

	void normalDataDirtiesCacheRowTraceLog(std::shared_ptr<NFmiDrawParam>& drawParam, unsigned int mapViewDescTopIndex, int cacheRowNumber, const std::string& theFileName)
	{
		if(CatLog::doTraceLevelLogging())
		{
			std::string traceLogMessage = "Param '";
			traceLogMessage += CtrlViewUtils::GetParamNameString(drawParam, false, false, false, 0, false, true, true, nullptr);
			traceLogMessage += "' required map-view ";
			traceLogMessage += std::to_string(mapViewDescTopIndex + 1);
			traceLogMessage += " row ";
			traceLogMessage += std::to_string(cacheRowNumber + 1);
			traceLogMessage += " to be updated when data '";
			traceLogMessage += theFileName;
			traceLogMessage += "' was read";
			CatLog::logMessage(traceLogMessage, CatLog::Severity::Trace, CatLog::Category::Visualization);
		}
	}

	// Synop-plot ja muut vastaavat ovat erikoistapauksia, koska sill� erikois param-id (kFmiSpSynoPlot, etc.), ja ne pit�� tarkistaa erikseen.
	// Palauttaa true, jos pit�� tehd� p�ivityksi� n�ytt�riville.
	bool checkAllSynopPlotTypeUpdates(unsigned int mapViewDescTopIndex, std::shared_ptr<NFmiDrawParam>& drawParam, NFmiProducer& newDataProducer, NFmiMapViewDescTop& descTop, int cacheRowNumber, const std::string& theFileName)
	{
		unsigned long parId = drawParam->Param().GetParamIdent();
		bool updateStatus = false;
		if(synopPlotNeedsUpdate(parId, newDataProducer))
		{
			descTop.MapViewCache().MakeRowDirty(cacheRowNumber);// clean cache row
			updateStatus = true;
		}
		if(metarPlotNeedsUpdate(parId, newDataProducer))
		{
			descTop.MapViewCache().MakeRowDirty(cacheRowNumber);// clean cache row
			updateStatus = true;
		}
		if(soundingPlotNeedsUpdate(parId, newDataProducer))
		{
			descTop.MapViewCache().MakeRowDirty(cacheRowNumber);// clean cache row
			updateStatus = true;
		}

		if(updateStatus)
			normalDataDirtiesCacheRowTraceLog(drawParam, mapViewDescTopIndex, cacheRowNumber, theFileName);
		return updateStatus;
	}

	bool makeNormalDataDrawingLayerCahceChecks(unsigned int mapViewDescTopIndex, NFmiFastQueryInfo& fastInfo, NFmiInfoData::Type dataType, const NFmiTimeBag& dirtyViewTimes, const std::string& fileName, std::shared_ptr<NFmiDrawParam>& drawParam, NFmiProducer& dataProducer, NFmiMapViewDescTop& descTop, int cacheRowNumber)
	{
		const NFmiLevel* level = fastInfo.SizeLevels() <= 1 ? 0 : fastInfo.Level(); // ns. pinta datan kanssa ei v�litet� leveleist�
		if(drawParam->DataType() == dataType && *drawParam->Param().GetProducer() == dataProducer && fastInfo.Param(drawParam->Param()) && (level == 0 || drawParam->Level().LevelType() == level->LevelType()))
		{ // jos p�ivitetty data oli samaa tyyppi� ja sill� oli sama tuottaja kuin n�yt�ll� olevalla drawParamilla, laitetaan rivin piirto uusiksi
			if(dirtyViewTimes.Resolution() == 0) // jos dirty-timebagia ei ole alustettu arvoilla, liataan koko rivi
				descTop.MapViewCache().MakeRowDirty(cacheRowNumber);// clean cache row
			else
			{ // jos kyse tutka-havainto / mesan analyysi optimoinnista, liataan vain ne ajat joihin on tullut uutta dataa
				descTop.MapViewCache().MakeTimesDirty(dirtyViewTimes.FirstTime(), dirtyViewTimes.LastTime(), cacheRowNumber);
			}
			::normalDataDirtiesCacheRowTraceLog(drawParam, mapViewDescTopIndex, cacheRowNumber, fileName);
			return true;
		}
		return false;
	}

	static const int g_StatObservationSeekTimeLimitInMinutes = 240;

	void makeObsSeekTimeLimits(const NFmiMetTime& currentTime, int timeStepInMinutes, NFmiMetTime& limit1Out, NFmiMetTime& limit2Out)
	{
		limit1Out = currentTime;
		if(timeStepInMinutes <= g_StatObservationSeekTimeLimitInMinutes)
			limit1Out.ChangeByMinutes(-g_StatObservationSeekTimeLimitInMinutes);
		else
			limit1Out.ChangeByMinutes(-timeStepInMinutes);
		limit2Out = currentTime;
		if(currentTime > limit2Out)
			limit2Out.NextMetTime(); // jos limit2 oli py�ristynyt taaksep�in, laitetaan se askel eteenp�in
	}

	// Liataan descTopin view-cachesta ne ajat uudesta timebagist�, mit� ei ole vanhassa timebagissa
	void makeNewTimesDirtyFromViewCache(NFmiMapViewDescTop& mapViewDescTop, NFmiTimeBag& oldAnimationTimes, NFmiTimeBag& newAnimationTimes)
	{
		for(newAnimationTimes.Reset(); newAnimationTimes.Next(); )
		{
			const NFmiMetTime& aTime = newAnimationTimes.CurrentTime();
			if(oldAnimationTimes.SetCurrent(aTime) == false)
				mapViewDescTop.MapViewCache().MakeTimeDirty(aTime);
		}
	}

	bool isEditedRelatedDataType(NFmiInfoData::Type dataType)
	{
		return (dataType == NFmiInfoData::kEditable) || (dataType == NFmiInfoData::kCopyOfEdited);
	}

	bool drawParamListContainsEditedData(NFmiDrawParamList& drawParamList)
	{
		for(drawParamList.Reset(); drawParamList.Next(); )
		{
			std::shared_ptr<NFmiDrawParam> drawParam = drawParamList.Current();
			if(!drawParam->IsParamHidden())
			{
				if(::isEditedRelatedDataType(drawParam->DataType()))
				{
					return true;
				}
			}
		}
		return false;
	}

	bool getLatestSatelImageTime(const NFmiDataIdent& dataIdent, NFmiMetTime& foundTimeOut)
	{
		auto& satelliteImageCacheSystem = CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->SatelliteImageCacheSystem();
		NFmiMetTime latestTime = satelliteImageCacheSystem.GetLatestImageTime(dataIdent);
		if(latestTime != NFmiMetTime::gMissingTime)
		{
			foundTimeOut = latestTime;
			return true;
		}
		else
			return false;
	}

	// etsi timedescriptorista viimeisin aika joka sopii annettuun aika-steppiin ja joka on lis�ksi annettujen aikarajojen alueella.
	// Jos ei l�ydy, palauttaa false.
	// fDemandExactCheck muuttuja vaatii ett� aikojen pit�� olla juuri tarkalleen aika stepiss�. Mutta jos kyse on
	// esim. salama datasta, haetaan se aika, joka sopii steppiin ja joka on my�h�isin aika.
	bool getLatestValidTimeWithCorrectTimeStep(NFmiTimeDescriptor& checkedTimes, int timeStepInMinutes, bool demandExactCheck, const NFmiMetTime& limit1, const NFmiMetTime& limit2, NFmiMetTime& foundTimeOut)
	{
		checkedTimes.Time(checkedTimes.LastTime()); // asetetaan viimeiseen aikaan
		do
		{
			NFmiMetTime aTime(checkedTimes.Time());
			aTime.SetTimeStep(timeStepInMinutes, true);
			if(demandExactCheck)
			{
				if(aTime == checkedTimes.Time()) // kun vaaditaan tarkaa checkki�, eih�n aika muuttunut, kun sit� rukattiin halutulla timestepill�
				{
					if(aTime >= limit1 && aTime <= limit2) // onko aika annettujen rajojen sis�ll�
					{
						foundTimeOut = aTime;
						return true;
					}
				}
			}
			else
			{
				if(checkedTimes.Time() < aTime)
					aTime.NextMetTime(); // jos aika oli py�ristynyt taaksep�in, laitetaan se askeleen verran eteenp�in
				if(aTime >= limit1 && aTime <= limit2)
				{
					foundTimeOut = aTime;
					return true;
				}
			}
			if(checkedTimes.Time() <= limit1)
				break; // ei tarvetta jatkaa en��, koska loput ajoista ovat kaikki pienempi� kuin ala rajan aika
		} while(checkedTimes.Previous());
		return false;
	}

	void setInfosMask(std::vector<std::shared_ptr<NFmiFastQueryInfo> >& infoVector, unsigned long usedMask)
	{
		for(const auto& info : infoVector)
		{
			info->MaskType(usedMask);
		}
	}

	void macroParamDirtiesCacheRowTraceLog(std::shared_ptr<NFmiDrawParam>& drawParam, const MacroParamDataInfo& macroParamDataInfo, unsigned int mapViewDescTopIndex, int cacheRowNumber, const std::string& fileName)
	{
		if(CatLog::doTraceLevelLogging())
		{
			std::string traceLogMessage = "MacroParam '";
			traceLogMessage += drawParam->ParameterAbbreviation();
			traceLogMessage += "' with variable '";
			traceLogMessage += macroParamDataInfo.variableName_;
			traceLogMessage += "' ";
			if(macroParamDataInfo.usedWithVerticalFunction_)
				traceLogMessage += "in function '" + macroParamDataInfo.possibleVerticalFunctionName_ + "' ";
			traceLogMessage += "required map-view ";
			traceLogMessage += std::to_string(mapViewDescTopIndex + 1);
			traceLogMessage += " row ";
			traceLogMessage += std::to_string(cacheRowNumber + 1);
			traceLogMessage += " to be updated when data '";
			traceLogMessage += fileName;
			traceLogMessage += "' was read";
			CatLog::logMessage(traceLogMessage, CatLog::Severity::Trace, CatLog::Category::Visualization);
		}
	}

	void SetCPCropGridSettings(const std::shared_ptr<NFmiArea>& newArea, unsigned int mapViewDescTopIndex)
	{
		CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->SetCPCropGridSettings(newArea, mapViewDescTopIndex);
	}

#ifndef UNIX
	NFmiApplicationWinRegistry& getApplicationWinRegistry()
	{
		return CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->ApplicationWinRegistry();
	}
#endif

	NFmiCrossSectionSystem& getCrossSectionSystem()
	{
		return *CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->CrossSectionSystem();
	}

	NFmiMacroParamDataCache& getMacroParamDataCache()
	{
		return CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->MacroParamDataCache();
	}

	std::shared_ptr<NFmiMacroParamSystem> getMacroParamSystem()
	{
		return CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->MacroParamSystem();
	}

	NFmiMTATempSystem& getMTATempSystem()
	{
		return CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->GetMTATempSystem();
	}

	NFmiWindTableSystem& getWindTableSystem()
	{
		return CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->WindTableSystem();
	}

	NFmiInfoOrganizer& getInfoOrganizer()
	{
		return *CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->InfoOrganizer();
	}

	NFmiFixedDrawParamSystem& getFixedDrawParamSystem()
	{
		return CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->FixedDrawParamSystem();
	}

	Warnings::CapDataSystem& getCapDataSystem()
	{
		return CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->GetCapDataSystem();
	}

	NFmiMacroPathSettings& getMacroPathSettings()
	{
		return CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->MacroPathSettings();
	}

	std::vector<std::shared_ptr<NFmiFastQueryInfo> > getSortedSynopInfoVector(int producerId, int producerId2 = -1, int producerId3 = -1, int producerId4 = -1)
	{
		return CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->GetSortedSynopInfoVector(producerId, producerId2, producerId3, producerId4);
	}

	void setMacroParamDrawParamSettings(const NFmiMenuItem& menuItem, std::shared_ptr<NFmiDrawParam>& drawParam)
	{
		auto dataType = menuItem.DataType();
		if(NFmiDrawParam::IsMacroParamCase(dataType))
		{
			drawParam->ParameterAbbreviation(menuItem.MenuText()); // macroParamin tapauksessa pit�� nimi asettaa t�ss� (t�t� nimilyhennett� k�ytet��n tunnisteen� my�hemmin!!)
			boost::shared_ptr<NFmiMacroParam> usedMacroParam;
			auto macroParamSystemPtr = ::getMacroParamSystem();
			const auto& macroParamInitFile = menuItem.MacroParamInitName();
			if(!macroParamInitFile.empty())
			{
				// T�m� on toivottu tapa alustaa, koska muuten saman nimiset 
				// macroParamit eri hakemistoissa voivat aiheuttaa p��llekk�isyyksi�
				usedMacroParam = macroParamSystemPtr->GetWantedMacro(macroParamInitFile);
			}
			else
			{
				boost::shared_ptr<NFmiMacroParamFolder> currentFolder = macroParamSystemPtr->GetCurrentFolder();
				if(currentFolder && currentFolder->Find(drawParam->ParameterAbbreviation()))
					usedMacroParam = currentFolder->Current();
			}

			// ei alusteta, jos oli virheellinen macroParam
			if(usedMacroParam != 0 && usedMacroParam->ErrorInMacro() == false) 
			{
				drawParam->Init(usedMacroParam->DrawParam());
				// Datatyyppi pit�� ottaa lopuksi menuItemista, koska niit� on nyt jo 4 erilaista 
				// ja usedMacroParam ei tied� siit� mit��n
				drawParam->DataType(dataType);
			}
		}
	}

	NFmiInfoData::Type getUsableCrossSectionDataType(NFmiInfoData::Type dataType, NFmiInfoData::Type alternateType, const NFmiProducer& givenProducer)
	{
		NFmiInfoData::Type finalDataType = dataType;
		auto &infoOrganizer = ::getInfoOrganizer();
		std::shared_ptr<NFmiFastQueryInfo> info = infoOrganizer.FindInfo(dataType, givenProducer, false);
		if(info == 0)
		{
			info = infoOrganizer.FindInfo(alternateType, givenProducer, false);
			if(info)
				finalDataType = alternateType;
		}
		return finalDataType;
	}

	// jos on katsottu hirlma mallipinta dataa poikkileikkaus ikkunassa ja tuottaja vaihdetaan GFS:ksi,
	// t�ll�in ei tule mit��n n�kyviin, koska GFS:ll� ei ole hybridi dataa. Ja jos on katsottu painepintadataa
	// ja vaihdetaan Arome tuottajaan, t�ll�in ei tule mit��n n�kyviin, koska ei ole kuin hybridi dataa.
	// T�t� varten t�m� funktio tekee viel� viimeiset tarkastelut, l�ytyyk� dataa ja s��t�� datatyyppi� tarvittaessa.
	NFmiInfoData::Type checkCrossSectionLevelData(NFmiInfoData::Type dataType, const NFmiProducer& givenProducer)
	{
		NFmiInfoData::Type finalDataType = dataType;
		if(dataType == NFmiInfoData::kViewable)
			finalDataType = ::getUsableCrossSectionDataType(NFmiInfoData::kViewable, NFmiInfoData::kHybridData, givenProducer);
		else if(dataType == NFmiInfoData::kHybridData)
			finalDataType = ::getUsableCrossSectionDataType(NFmiInfoData::kHybridData, NFmiInfoData::kViewable, givenProducer);
		return finalDataType;
	}

	std::shared_ptr<NFmiFastQueryInfo> getEditedInfo()
	{
		return CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->EditedSmartInfo();
	}

	// Perus-oliolle (list-list) on jo varattu muistia, t�ss� alustetaan vain tyhj�t listat kokonaislistaan.
	std::unique_ptr<NFmiPtrList<NFmiDrawParamList>> createDrawParamListVector(int wantedSize)
	{
		std::unique_ptr<NFmiPtrList<NFmiDrawParamList>> drawParamListVector = std::make_unique<NFmiPtrList<NFmiDrawParamList>>();
		for(int i = 0; i < wantedSize; i++)
			drawParamListVector->AddEnd(new NFmiDrawParamList());
		return drawParamListVector;
	}

	void ActivateFirstNonHiddenViewParam(NFmiDrawParamList* drawParamList)
	{
		for(drawParamList->Reset(); drawParamList->Next(); )
		{
			if(!drawParamList->Current()->IsParamHidden())
			{
				drawParamList->Current()->Activate(true);
				break;
			}
		}
	}

	// Muuta changedDrawParam:ia niin ett� muuten asetukset tulevat newDrawParamSettings:ista, paitsi muutamat erikseen asetettavat on otettava vanhasta.
	void setUpChangedDrawParam(std::shared_ptr<NFmiDrawParam>& changedDrawParam, std::shared_ptr<NFmiDrawParam>& newDrawParamSettings)
	{
		// 1. Ota ensin tietyt asetukset vanhasta uuteen
		newDrawParamSettings->ModelRunIndex(changedDrawParam->ModelRunIndex());
		newDrawParamSettings->TimeSerialModelRunCount(changedDrawParam->TimeSerialModelRunCount());

		// 2. Aseta sitten muutettavan kaikki asetukset uusista asetuksista
		changedDrawParam->Init(newDrawParamSettings);
	}

	bool areDrawParamsSimilarType(std::shared_ptr<NFmiDrawParam>& drawParam, std::shared_ptr<NFmiDrawParam>& targetDrawParam, bool useWithViewMacros)
	{
		if(useWithViewMacros || targetDrawParam->ViewMacroDrawParam() == false)
		{
			if(drawParam->IsMacroParamCase(true))
			{
				if(targetDrawParam->IsMacroParamCase(true) && drawParam->ParameterAbbreviation() == targetDrawParam->ParameterAbbreviation())
					return true;
			}
			else if(drawParam->Param().GetParamIdent() == targetDrawParam->Param().GetParamIdent() && drawParam->Level() == targetDrawParam->Level())
				return true;
		}

		return false;
	}

	void initializeWantedDrawParams(NFmiFastDrawParamList& drawParamList, std::shared_ptr<NFmiDrawParam>& drawParam, bool useWithViewMacros)
	{
		for(auto iter = drawParamList.Begin(); iter != drawParamList.End(); ++iter)
		{
			std::shared_ptr<NFmiDrawParam> aDrawParam = iter->second;
			if(::areDrawParamsSimilarType(drawParam, aDrawParam, useWithViewMacros))
				aDrawParam->Init(drawParam, true);
		}
	}

	void initializeWantedDrawParams(NFmiDrawParamList& drawParamList, std::shared_ptr<NFmiDrawParam>& drawParam, bool useWithViewMacros)
	{
		for(drawParamList.Reset(); drawParamList.Next(); )
		{
			std::shared_ptr<NFmiDrawParam> aDrawParam = drawParamList.Current();
			if(::areDrawParamsSimilarType(drawParam, aDrawParam, useWithViewMacros))
				aDrawParam->Init(drawParam, true);
		}
	}

	void initializeWantedDrawParams(NFmiPtrList<NFmiDrawParamList>& drawParamListVector, std::shared_ptr<NFmiDrawParam>& drawParam, bool useWithViewMacros)
	{
		NFmiPtrList<NFmiDrawParamList>::Iterator iter = drawParamListVector.Start();
		for(; iter.Next(); )
			::initializeWantedDrawParams((*iter.CurrentPtr()), drawParam, useWithViewMacros);
	}

	// Lokaali+wms karttojen yhdistelm� moodiin liittyv�t valitut taustakarttaindeksit kaikille eri kartta-alueille (suomi,skandi,euro,maailma).
	// Teksti on seuraavaa muotoa (t�m� luokka ei tosin parseroi tai tee muuta kuin s�ilytt�� stringin): 
	// mapAreaCount:area1Index,area1Index,area1Index,area1Index     (esim. 4:2,1,4,3)
	std::vector<int> parseSelectedMapIndices(const std::string& indicesString)
	{
		std::vector<std::string> parts;
		boost::split(parts, indicesString, boost::is_any_of(":"));
		if(parts.size() != 2)
		{
			std::string errorMessage = std::string("Error in ") + __FUNCTION__ + ": splitting indicesString with ':' character resulted illegal number of parts with string'";
			errorMessage += indicesString + "'";
			throw std::runtime_error(errorMessage);
		}
		else
		{
			auto mapAreaCount = boost::lexical_cast<int>(parts[0]);
			std::vector<std::string> indexParts;
			boost::split(indexParts, parts[1], boost::is_any_of(","));
			if(indexParts.size() != mapAreaCount)
			{
				std::string errorMessage = std::string("Error in ") + __FUNCTION__ + ": splitting indicesString with ',' character resulted illegal number of parts with string'";
				errorMessage += indicesString + "'";
				throw std::runtime_error(errorMessage);
			}
			else
			{
				std::vector<int> indices;
				for(const auto& indexString : indexParts)
				{
					indices.push_back(boost::lexical_cast<int>(indexString));
				}
				return indices;
			}
		}
	}

	int calcInitialWmsLayerIndex(const NFmiCombinedMapModeState& combinedMapModeState)
	{
		int usedWmsLayerIndex = 0; // Asetetaan oletuksena 0 valituksi wms layerin indeksiksi
		if(!combinedMapModeState.isLocalMapCurrentlyInUse()) // Jos combined-mode indeksi osoittaa wms 'lohkoon'
		{
			// asetetaan wms lohkoon laskettu indeksi valituksi
			usedWmsLayerIndex = combinedMapModeState.currentMapSectionIndex();
		}
		return usedWmsLayerIndex;
	}

#ifndef DISABLE_CPPRESTSDK
	std::string getWmsMapLayerGuiName(int mapLayerIndex, Wms::UserUrlServerSetup& userUrlServerSetup, bool addServerName)
	{
		if(mapLayerIndex >= userUrlServerSetup.parsedServers.size())
			throw std::runtime_error(std::string("Logical error - Illegal mapLayerIndex with Wms parsedServers in function: ") + __FUNCTION__);
		auto& usedLayerParsedServer = userUrlServerSetup.parsedServers[mapLayerIndex];
		std::string layerName;
		if(!usedLayerParsedServer.descriptiveName.empty())
			layerName = usedLayerParsedServer.descriptiveName;
		else if(!usedLayerParsedServer.macroReference.empty())
			layerName = usedLayerParsedServer.macroReference;
		else
		{
			std::for_each(usedLayerParsedServer.layerGroup.begin(), usedLayerParsedServer.layerGroup.end(),
				[&layerName](const auto& groupItem) {layerName += groupItem + ","; });
			layerName.pop_back(); // poistetaan viimeinen pilkku
		}

		if(addServerName)
		{
			// Lis�t��n host:in osoite sulkuihin per��n
			layerName += " (";
			layerName += usedLayerParsedServer.host;
			layerName += ")";
		}
		return layerName;
	}

	std::string makeSelectedMapIndicesString(const std::vector<int>& indices)
	{
		std::string indicesString;
		indicesString += std::to_string(indices.size());
		indicesString += ":";
		for(auto index : indices)
		{
			indicesString += std::to_string(index);
			indicesString += ",";
		}
		// Poistetaan viimeisen numeron j�lkeinen pilkku, pilkku tulee loopissa jokaisen indeksin per��n, ja n�in poppaamalla koodi on yksinkertaisempaa.
		indicesString.pop_back();
		return indicesString;
	}

	std::string makeFunctionAndAreaDescription(std::string functionName, const std::shared_ptr<NFmiArea> &mapArea)
	{
		std::string functionAndAreaDescription = functionName;
		functionAndAreaDescription += " with area: ";
		functionAndAreaDescription += mapArea->AreaStr();
		return functionAndAreaDescription;
	}

	void initializeWmsMapLayerInfos(MapAreaMapLayerRelatedInfo& mapLayerRelatedInfos, Wms::UserUrlServerSetup& wmsMapLayerSetup)
	{
		mapLayerRelatedInfos.clear();
		for(size_t mapLayerIndex = 0; mapLayerIndex < wmsMapLayerSetup.parsedServers.size(); mapLayerIndex++)
		{
			// Lis�t��n t�h�n listaan serverin nimi per��n (viimeinen parametri true)
			auto guiMapLayerName = ::getWmsMapLayerGuiName(static_cast<int>(mapLayerIndex), wmsMapLayerSetup, true);
			auto& usedLayerParsedServer = wmsMapLayerSetup.parsedServers[mapLayerIndex];
			NFmiMapLayerRelatedInfo mapLayerRelatedInfo{ guiMapLayerName, usedLayerParsedServer.macroReference, true };
			mapLayerRelatedInfos.push_back(mapLayerRelatedInfo);
		}
	}
#endif // DISABLE_CPPRESTSDK

	int calcWantedMapLayerIndex(const MapAreaMapLayerRelatedInfo& mapLayerRelatedInfos, const std::string& mapLayerName)
	{
		auto iter = std::find_if(mapLayerRelatedInfos.begin(), mapLayerRelatedInfos.end(),
			[&mapLayerName](const auto& mapLayerRelatedInfo) {return mapLayerRelatedInfo.guiName_ == mapLayerName; });
		if(iter != mapLayerRelatedInfos.end())
			return static_cast<int>(std::distance(mapLayerRelatedInfos.begin(), iter));
		else
			return 0; // T�m� on virhetilanne, palautetaan kuitenkin vain 0 indeksi (ei poikkeusta, vaikka pit�isi)
	}

	std::string getMacroReferenceNameForViewMacro(const NFmiCombinedMapModeState& mapModeState, const MapAreaMapLayerRelatedInfo& staticMapLayerRelatedInfos, const MapAreaMapLayerRelatedInfo& wmsMapLayerRelatedInfos)
	{
		std::string name;
		auto mapLayerSectionIndex = mapModeState.currentMapSectionIndex();
		// Overlay tapauksessa mapLayerSectionIndex voi olla -1 jolloin MacroReference:lla ei ole arvoa
		if(mapLayerSectionIndex >= 0)
		{
			if(mapModeState.isLocalMapCurrentlyInUse())
			{
				if(mapLayerSectionIndex >= 0 && mapLayerSectionIndex < staticMapLayerRelatedInfos.size())
					name = staticMapLayerRelatedInfos[mapLayerSectionIndex].macroReference_;
			}
			else
			{
				if(mapLayerSectionIndex >= 0 && mapLayerSectionIndex < wmsMapLayerRelatedInfos.size())
				{
					name = wmsMapLayerRelatedInfos[mapLayerSectionIndex].prefixedMacroReference_;
				}
			}
		}
		return name;
	}

	// Kun tutkitaan l�ytyyk� macro-referencen perusteella vastinetta, tehd��n tarkastelu varmuuden vuoksi case-insensitiivisesti.
	int getMacroReferenceMapLayerIndex(const std::string& nonPrefixMacroReferenceName, const MapAreaMapLayerRelatedInfo& mapLayerRelatedInfos)
	{
		auto layerIter = std::find_if(mapLayerRelatedInfos.begin(), mapLayerRelatedInfos.end(),
			[&nonPrefixMacroReferenceName](const auto& mapLayerRelatedInfo) {return boost::iequals(nonPrefixMacroReferenceName, mapLayerRelatedInfo.macroReference_); });
		int layerIndex = (layerIter != mapLayerRelatedInfos.end()) ? static_cast<int>(std::distance(mapLayerRelatedInfos.begin(), layerIter)) : -1;
		return layerIndex;
	}

	// T�ss� haetaan macro-referenssi� seuraavin keinoin ja prioriteetein:
	// 1. macroReferenceName ei saa olla tyhj�
	// 2. Tarkistetaan sis�lsik� macroReferenceName wms prefixin "[wms]"
	// 3. Riippuen siit� onko originaali ollut wms pohjainen layer, tehd��n seuraavaa:
	// 3.1. Jos wms pohjaisista l�ytyy vastaava macroReference nimi, palautetaan sen ja static layereiden yhteisindeksi
	// 3.2. Jos l�ytyy vain staattisista layereista vastaava macroReference nimi, palautetaan sen indeksi
	// 4. Jos originaali ei ollut wms pohjainen layer, tehd��n seuraavaa:
	// 4.1. Jos l�ytyy staattisista layereista macroReference nimi, palautetaan sen indeksi
	// 4.2. Jos l�ytyy vain wms pohjaisista vastaava macroReference nimi, palautetaan sen ja static layereiden yhteisindeksi
	int getCombinedMapModeIndexByMacroReferenceName(const std::string& macroReferenceName, const MapAreaMapLayerRelatedInfo& staticMapLayerRelatedInfos, const MapAreaMapLayerRelatedInfo& wmsMapLayerRelatedInfos, bool localOnlyMapModeInUse)
	{
		if(!macroReferenceName.empty())
		{
			bool macroReferenceWasWmsLayer = NFmiMapLayerRelatedInfo::hasWmsMacroReferencePrefix(macroReferenceName);
			auto nonPrefixMacroReferenceName = NFmiMapLayerRelatedInfo::stripWmsMacroReferencePrefix(macroReferenceName);
			int staticLayerIndex = ::getMacroReferenceMapLayerIndex(nonPrefixMacroReferenceName, staticMapLayerRelatedInfos);
			int wmsLayerIndex = ::getMacroReferenceMapLayerIndex(nonPrefixMacroReferenceName, wmsMapLayerRelatedInfos);
			if(macroReferenceWasWmsLayer)
			{
				if(!localOnlyMapModeInUse && wmsLayerIndex >= 0)
					return static_cast<int>(staticMapLayerRelatedInfos.size()) + wmsLayerIndex;
				else if(staticLayerIndex >= 0)
					return staticLayerIndex;
			}
			else
			{
				if(staticLayerIndex >= 0)
					return staticLayerIndex;
				else if(!localOnlyMapModeInUse && wmsLayerIndex >= 0)
					return static_cast<int>(staticMapLayerRelatedInfos.size()) + wmsLayerIndex;
			}
		}
		// Ei l�ytynyt mit��n annetun macroReferenceName:n avulla, -1 on merkki siit�, ett� mit��n ei tehd�.
		return -1;
	}

	bool selectMapLayerByMacroReferenceNameFromViewMacro(NFmiCombinedMapModeState& combinedMapModeState, const std::string& macroReferenceName, const MapAreaMapLayerRelatedInfo& staticMapLayerRelatedInfos, const MapAreaMapLayerRelatedInfo& wmsMapLayerRelatedInfos)
	{
		auto localOnlyMapModeInUse = combinedMapModeState.isLocalOnlyMapModeInUse();
		auto usedCombinedModeMapIndex = getCombinedMapModeIndexByMacroReferenceName(macroReferenceName, staticMapLayerRelatedInfos, wmsMapLayerRelatedInfos, localOnlyMapModeInUse);
		if(usedCombinedModeMapIndex >= 0)
		{
			combinedMapModeState.combinedModeMapIndex(usedCombinedModeMapIndex);
			return true;
		}
		return false;
	}

	bool isGroundDataType(std::shared_ptr<NFmiDrawParam>& drawParam)
	{
		return drawParam->Level().GetIdent() == 0;
	}

	bool checkLastObservationTime(bool *newerTimeFoundInOut, const NFmiMetTime & checkedTime, const NFmiMetTime& currentTime, NFmiMetTime *newLastTimeInOut, int timeStepInMinutes)
	{
		// Tarkasteltu aika pit�� py�rist�� animaatiossa k�ytetyn time-stepin kanssa ja viel� taaksep�in.
		NFmiMetTime backwardRoundedCheckTime(checkedTime);
		backwardRoundedCheckTime.SetTimeStep(timeStepInMinutes, true, kBackward);
		if(*newerTimeFoundInOut == false || backwardRoundedCheckTime > *newLastTimeInOut)
		{
			*newerTimeFoundInOut = true;
			*newLastTimeInOut = backwardRoundedCheckTime;
			if(*newLastTimeInOut >= currentTime)
			{
				// Ei tarvitse en�� jatkaa, koska aika joka l�ytyi on viimeisin mahdollinen.
				return true;
			}
		}
		return false;
	}

	bool isObservationLockModeQueryData(const std::shared_ptr<NFmiDrawParam>& drawParam)
	{
		if(CtrlViewFastInfoFunctions::IsObservationLockModeDataType(drawParam->DataType()))
		{
			// Ignooraa toistaiseksi tuottaja kFmiTEMP, koska niiden par haku tuottaa ajallisesti liian pitk�lle menevi� datoja (viimeinen aika on tyhj��).
			if(!NFmiInfoOrganizer::IsTempData(drawParam->Param().GetProducer()->GetIdent(), true))
			{
				return true;
			}
		}
		return false;
	}

	bool isObservationLockModeSatelData(const std::shared_ptr<NFmiDrawParam>& drawParam, bool ignoreSatelImages)
	{
		if(drawParam->DataType() == NFmiInfoData::kSatelData && ignoreSatelImages == false)
		{
			return true;
		}
		return false;
	}

	bool isObservationLockModeWmsData(const std::shared_ptr<NFmiDrawParam>& drawParam)
	{
		if(drawParam->DataType() == NFmiInfoData::kWmsData && drawParam->TreatWmsLayerAsObservation())
		{
			return true;
		}
		return false;
	}

	void checkEarliestLastObservationTime(bool * anyTimeFoundInOut, const NFmiMetTime & checkedTime, NFmiMetTime* earliestLastTimeInOut, int timeStepInMinutes)
	{
		// Tarkasteltu aika pit�� py�rist�� animaatiossa k�ytetyn time-stepin kanssa ja viel� taaksep�in.
		NFmiMetTime backwardRoundedCheckTime(checkedTime);
		backwardRoundedCheckTime.SetTimeStep(timeStepInMinutes, true, kBackward);
		if(*anyTimeFoundInOut == false || backwardRoundedCheckTime < *earliestLastTimeInOut)
		{
			*anyTimeFoundInOut = true;
			*earliestLastTimeInOut = backwardRoundedCheckTime;
		}
	}


} // nameless namespace ends


NFmiCombinedMapHandler::~NFmiCombinedMapHandler()
{
	// Pit�� hankkiutua eroon NFmiPtrList -luokan k�yt�st�, silloin ei tarvitse erillisi� varatun muistin tuhoamiskomentoja
	if(crossSectionDrawParamListVector_)
	{
		crossSectionDrawParamListVector_->Clear(true);
	}
	if(copyPasteDrawParamListVector_)
	{
		copyPasteDrawParamListVector_->Clear(true);
	}
}

void NFmiCombinedMapHandler::initialize(const std::string & absoluteControlPath)
{
	absoluteControlPath_ = absoluteControlPath;
	doVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		modifiedPropertiesDrawParamList_ = std::make_unique<NFmiFastDrawParamList>();
		copyPasteDrawParam_ = std::make_unique<NFmiDrawParam>();
		copyPasteDrawParamList_ = std::make_unique<NFmiDrawParamList>();
		copyPasteCrossSectionDrawParamList_ = std::make_unique<NFmiDrawParamList>();
		copyPasteDrawParamListVector_ = std::make_unique<NFmiPtrList<NFmiDrawParamList>>();
		timeSerialViewDrawParamList_ = std::make_unique<NFmiDrawParamList>();

		initCrossSectionDrawParamListVector();
		initMapConfigurationSystemMain();
		initProjectionCurvatureInfo();
		initMapViewDescTops();

		initLandBorderDrawingSystem();
		initWmsSupport();
		initCombinedMapStates();
		initializeMapLayerInfos();
	}
	catch(std::exception & e)
	{
		std::string errStr = __FUNCTION__;
		errStr += " - Initialization error in configurations: \n";
		errStr += e.what();
		logAndWarnUser(errStr, "Problems with map view settings", CatLog::Severity::Error, CatLog::Category::Configuration, true);
	}
}

#ifndef UNIX
void NFmiCombinedMapHandler::initMapViewDescTops()
{
	try
	{
		int mapViewCount = ::getApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapViewCount();
		// MapView-indeksit alkavat 0:sta
		std::string baseSettingStr("MetEditor::MapView::");
		for(int mapViewIndex = 0; mapViewIndex < mapViewCount; mapViewIndex++)
		{
			mapViewDescTops_.emplace_back(createMapViewDescTop(baseSettingStr, mapViewIndex));
		}
	}
	catch(std::exception& e)
	{
		std::string errStr = __FUNCTION__;
		errStr += " - cannot initialize map view settings from configurations: \n";
		errStr += e.what();
		NFmiBasicSmartMetConfigurations::DoInitializationAbortMessageBox(errStr, "Map view settings error", true);
	}
}
#endif

#ifndef UNIX
std::pair<unsigned int, NFmiCombinedMapHandler::MapViewCombinedMapModeState> NFmiCombinedMapHandler::makeTotalMapViewCombinedMapModeState(unsigned int mapViewIndex, unsigned int usedWmsMapLayerCount, bool doBackgroundCase)
{
	MapViewCombinedMapModeState mapViewState;

	std::function<bool()> localOnlyMapModeUsedFunction = [this]() {return this->localOnlyMapModeUsed(); };
	auto& mapHandlerVector = getMapViewDescTop(mapViewIndex)->GdiPlusImageMapHandlerList();
	for(auto mapAreaIndex = 0u; mapAreaIndex < mapHandlerVector.size(); mapAreaIndex++)
	{
		NFmiCombinedMapModeState mapAreaState;
		auto& mapHandler = mapHandlerVector[mapAreaIndex];
		auto localLayerCount = doBackgroundCase ? mapHandler->MapSize() : mapHandler->OverMapSize();
		mapAreaState.initialize(localLayerCount, usedWmsMapLayerCount, localOnlyMapModeUsedFunction, doBackgroundCase);
		mapViewState.emplace(mapAreaIndex, mapAreaState);
	}

	return std::make_pair(mapViewIndex, mapViewState);
}

void NFmiCombinedMapHandler::initCombinedMapStates()
{
	auto mapViewCount = getMapViewCount();
	auto wmsBackgroundMapLayerCount = 0;
	auto wmsOverlayMapLayerCount = 0;
	if(wmsSupportAvailable())
	{
		auto& staticMapClientState = getWmsSupport()->getStaticMapClientState(0, 0).state_;
		wmsBackgroundMapLayerCount = static_cast<unsigned int>(staticMapClientState->getBackgroundsLength());
		wmsOverlayMapLayerCount = static_cast<unsigned int>(staticMapClientState->getOverlaysLenght());
	}

	for(auto mapViewIndex = 0u; mapViewIndex < mapViewCount; mapViewIndex++)
	{
		combinedBackgroundMapModeStates_.emplace(makeTotalMapViewCombinedMapModeState(mapViewIndex, wmsBackgroundMapLayerCount, true));
		combinedOverlayMapModeStates_.emplace(makeTotalMapViewCombinedMapModeState(mapViewIndex, wmsOverlayMapLayerCount, false));
	}
	initCombinedMapSelectionIndices();
	initWmsSupportSelectionIndices();
}
#endif

#ifndef UNIX
void NFmiCombinedMapHandler::storeCombinedMapStates()
{
	auto mapViewCount = getMapViewCount();
	for(auto mapViewIndex = 0u; mapViewIndex < mapViewCount; mapViewIndex++)
	{
		std::vector<int> backgroundIndices;
		std::vector<int> overlayIndices;
		auto mapAreaCount = getMapAreaCount();
		for(auto mapAreaIndex = 0u; mapAreaIndex < mapAreaCount; mapAreaIndex++)
		{
			backgroundIndices.push_back(getCombinedMapModeState(mapViewIndex, mapAreaIndex).combinedModeMapIndex());
			overlayIndices.push_back(getCombinedOverlayMapModeState(mapViewIndex, mapAreaIndex).combinedModeMapIndex());
		}
		auto mapViewWinRegistry = getApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(mapViewIndex);
		mapViewWinRegistry->CombinedMapModeSelectedBackgroundIndices(::makeSelectedMapIndicesString(backgroundIndices));
		mapViewWinRegistry->CombinedMapModeSelectedOverlayIndices(::makeSelectedMapIndicesString(overlayIndices));
	}
}
#endif

#ifndef UNIX
std::vector<int> NFmiCombinedMapHandler::getCombinedModeSelectedMapIndicesFromWinRegistry(unsigned int mapViewDescTopIndex, bool doBackgroundMaps)
{
	auto mapViewWinRegistry = getApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(mapViewDescTopIndex);
	auto selectedMapIndicesStr = doBackgroundMaps ? mapViewWinRegistry->CombinedMapModeSelectedBackgroundIndices() : mapViewWinRegistry->CombinedMapModeSelectedOverlayIndices();
	return ::parseSelectedMapIndices(selectedMapIndicesStr);
}
#endif

#ifndef UNIX
void NFmiCombinedMapHandler::initCombinedMapSelectionIndices()
{
	auto mapViewCount = getMapViewCount();
	for(auto mapViewIndex = 0u; mapViewIndex < mapViewCount; mapViewIndex++)
	{
		auto mapViewWinRegistry = getApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(mapViewIndex);
		auto backgroundIndices = getCombinedModeSelectedMapIndicesFromWinRegistry(mapViewIndex, true);
		auto overlayIndices = getCombinedModeSelectedMapIndicesFromWinRegistry(mapViewIndex, false);
		auto mapAreaCount = getMapAreaCount();
		for(auto mapAreaIndex = 0u; mapAreaIndex < mapAreaCount; mapAreaIndex++)
		{
			if(mapAreaIndex < backgroundIndices.size())
				getCombinedMapModeState(mapViewIndex, mapAreaIndex).combinedModeMapIndex(backgroundIndices[mapAreaIndex]);
			if(mapAreaIndex < overlayIndices.size())
				getCombinedOverlayMapModeState(mapViewIndex, mapAreaIndex).combinedModeMapIndex(overlayIndices[mapAreaIndex]);
		}
	}
}
#endif

#ifndef UNIX
void NFmiCombinedMapHandler::initWmsSupportSelectionIndices()
{
	if(!wmsSupportAvailable())
		return;

	auto wmsSupportPtr = getWmsSupport();
	auto mapViewCount = getMapViewCount();
	for(auto mapViewIndex = 0u; mapViewIndex < mapViewCount; mapViewIndex++)
	{
		auto mapAreaCount = getMapAreaCount();
		for(auto mapAreaIndex = 0u; mapAreaIndex < mapAreaCount; mapAreaIndex++)
		{
			auto &staticMapClientState = wmsSupportPtr->getStaticMapClientState(mapViewIndex, mapAreaIndex);
			// Tehd��n ensin background map indeksin asetus
			staticMapClientState.state_->setBackgroundIndex(::calcInitialWmsLayerIndex(getCombinedMapModeState(mapViewIndex, mapAreaIndex)));

			// Tehd��n sitten overlay map indeksin asetus
			staticMapClientState.state_->setOverlayIndex(::calcInitialWmsLayerIndex(getCombinedOverlayMapModeState(mapViewIndex, mapAreaIndex)));
		}
	}
}
#endif

unsigned int NFmiCombinedMapHandler::getMapViewCount() const
{
	return static_cast<unsigned int>(mapViewDescTops_.size());
}

#ifndef UNIX
unsigned int NFmiCombinedMapHandler::getMapAreaCount() const
{
	return static_cast<unsigned int>(getMapViewDescTop(0)->GdiPlusImageMapHandlerList().size());
}
#endif

void NFmiCombinedMapHandler::initCrossSectionDrawParamListVector()
{
	doVerboseFunctionStartingLogReporting(__FUNCTION__);
	crossSectionDrawParamListVector_ = ::createDrawParamListVector(::getCrossSectionSystem().MaxViewRowSize());
}

void NFmiCombinedMapHandler::storeMapViewDescTopToSettings()
{
	for(const auto& mapViewDescTop : mapViewDescTops_)
		mapViewDescTop->StoreMapViewDescTopToSettings();

	storeCombinedMapStates();
}

bool NFmiCombinedMapHandler::wmsSupportAvailable() const
{
	auto wmsSupportPtr = getWmsSupport();
	if(wmsSupportPtr && wmsSupportPtr->isConfigured() && wmsSupportPtr->isTotalMapViewStaticMapClientStateAvailable())
		return true;
	else
		return false;
}

#ifndef UNIX
std::unique_ptr<NFmiMapViewDescTop> NFmiCombinedMapHandler::createMapViewDescTop(const std::string& baseSettingStr, int mapViewIndex)
{
	doVerboseFunctionStartingLogReporting(__FUNCTION__);
	std::string currentSettingStr = getMapViewDescTopSettingString(baseSettingStr, mapViewIndex);
	auto descTop = std::make_unique<NFmiMapViewDescTop>(currentSettingStr, mapConfigurationSystem_.get(), projectionCurvatureInfo_.get(), absoluteControlPath_, mapViewIndex);
	auto& applicationWinRegistry = ::getApplicationWinRegistry();
	descTop->MapViewCache().MaxSizeMB(applicationWinRegistry.MapViewCacheMaxSizeInMB());
	descTop->Init(*applicationWinRegistry.ConfigurationRelatedWinRegistry().MapView(mapViewIndex));
	return descTop;
}
#endif

// Metodi saa 0-pohjaisen mapViewIndeksin (index), mutta se pit�� muuttaa 1-pohjaiseksi, kun tehd��n settings stringi�.
std::string NFmiCombinedMapHandler::getMapViewDescTopSettingString(const std::string& baseStr, int mapViewDescTopIndex)
{
	std::string str(baseStr);
	str += std::to_string(mapViewDescTopIndex + 1);
	return str;
}

void NFmiCombinedMapHandler::initMapConfigurationSystemMain()
{
	doVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		initMapConfigurationSystem();
	}
	catch(std::exception & e)
	{
		logAndWarnUser(e.what(), "Problems with map configurations", CatLog::Severity::Error, CatLog::Category::Configuration, true);
	}
}

void NFmiCombinedMapHandler::initMapConfigurationSystem()
{
	mapConfigurationSystem_ = std::make_unique<NFmiMapConfigurationSystem>();
	// Create config list
	std::vector<std::string> mapsystems;
	::tokenize(NFmiSettings::Require<std::string>(CONFIG_MAPSYSTEMS), mapsystems, ",");

	std::vector<std::string>::iterator msiter = mapsystems.begin();
	char key1[1024]="", key2[1024] = "";
	while(msiter != mapsystems.end())
	{
		std::string mapsystem(*msiter);

		sprintf(key1, CONFIG_MAPSYSTEM_PROJECTION_FILENAME, mapsystem.c_str());
		sprintf(key2, CONFIG_MAPSYSTEM_PROJECTION_DEFINITION, mapsystem.c_str());
		if(!NFmiSettings::IsSet(key1) && !NFmiSettings::IsSet(key2))
		{
			msiter++;
			continue;
		}

		// First read the projection information
		auto mapConfiguration = std::make_shared<NFmiMapConfiguration>();
		if(NFmiSettings::IsSet(key1))
		{
			mapConfiguration->ProjectionFileName(NFmiSettings::Require<std::string>(key1));
		}
		else
		{
			mapConfiguration->Projection(NFmiSettings::Require<std::string>(key2));
		}

		// Then add the map ..
		sprintf(key1, CONFIG_MAPSYSTEM_MAP, mapsystem.c_str());
		std::string baseMapSettingKey = key1;
		std::vector<std::string> maps = NFmiSettings::ListChildren(key1);
		std::vector<std::string>::iterator mapiter = maps.begin();
		while(mapiter != maps.end())
		{
			std::string map(*mapiter);
			std::string baseMapSettingLayerKey = baseMapSettingKey + "::" + map + "::";
			sprintf(key1, CONFIG_MAPSYSTEM_MAP_FILENAME, mapsystem.c_str(), map.c_str());
			sprintf(key2, CONFIG_MAPSYSTEM_MAP_DRAWINGSTYLE, mapsystem.c_str(), map.c_str());
			if(NFmiSettings::IsSet(key1) && NFmiSettings::IsSet(key2))
			{
				mapConfiguration->AddMap(NFmiSettings::Require<std::string>(key1), NFmiSettings::Optional<int>(key2, 0));
			}
			std::string mapSettingLayerDescriptiveNameKey = baseMapSettingLayerKey + "DescriptiveName";
			mapConfiguration->AddBackgroundMapDescriptiveName(NFmiSettings::Optional<std::string>(mapSettingLayerDescriptiveNameKey, ""));
			std::string mapSettingLayerMacroReferenceKey = baseMapSettingLayerKey + "MacroReference";
			mapConfiguration->AddBackgroundMapMacroReferenceNames(NFmiSettings::Optional<std::string>(mapSettingLayerMacroReferenceKey, ""));
			mapiter++;
		}

		// .. and layer configurations
		sprintf(key1, CONFIG_MAPSYSTEM_LAYER, mapsystem.c_str());
		baseMapSettingKey = key1;
		std::vector<std::string> layers = NFmiSettings::ListChildren(key1);
		std::vector<std::string>::iterator layeriter = layers.begin();
		while(layeriter != layers.end())
		{
			std::string layer(*layeriter);
			std::string baseMapSettingLayerKey = baseMapSettingKey + "::" + layer + "::";
			sprintf(key1, CONFIG_MAPSYSTEM_LAYER_FILENAME, mapsystem.c_str(), layer.c_str());
			sprintf(key2, CONFIG_MAPSYSTEM_LAYER_DRAWINGSTYLE, mapsystem.c_str(), layer.c_str());
			if(NFmiSettings::IsSet(key1) && NFmiSettings::IsSet(key2))
			{
				mapConfiguration->AddOverMapDib(NFmiSettings::Require<std::string>(key1), NFmiSettings::Optional<int>(key2, 0));
			}
			std::string mapSettingLayerDescriptiveNameKey = baseMapSettingLayerKey + "DescriptiveName";
			mapConfiguration->AddOverlayMapDescriptiveNames(NFmiSettings::Optional<std::string>(mapSettingLayerDescriptiveNameKey, ""));
			std::string mapSettingLayerMacroReferenceKey = baseMapSettingLayerKey + "MacroReference";
			mapConfiguration->AddOverlayMapMacroReferenceNames(NFmiSettings::Optional<std::string>(mapSettingLayerMacroReferenceKey, ""));
			layeriter++;
		}

		mapConfiguration->InitializeFileNameBasedGuiNameVectors();
		// The map configuration is ready, add it to the mc system
		mapConfigurationSystem_->AddMapConfiguration(mapConfiguration);

		msiter++;
	}

	if(mapConfigurationSystem_->Size() <= 0)
		throw std::runtime_error(std::string(__FUNCTION__) + ": No map configurations were found");
	if(mapConfigurationSystem_->Size() < 4)
		throw std::runtime_error(std::string(__FUNCTION__) + ": There were less than 4 map configuration found from settings.");
}

void NFmiCombinedMapHandler::initProjectionCurvatureInfo()
{
	doVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		projectionCurvatureInfo_ = std::make_unique<NFmiProjectionCurvatureInfo>();
		if(projectionCurvatureInfo_)
		{
			projectionCurvatureInfo_->InitFromSettings("SmartMet::ProjectionLineSettings");
		}
		else
			throw std::runtime_error("Cannot create ProjectionCurvatureInfo (out of memory?).");
	}
	catch(std::exception & e)
	{
		std::string errStr = __FUNCTION__;
		errStr += " - Initialization error in configurations: \n";
		errStr += e.what();
		logMessage(errStr, CatLog::Severity::Error, CatLog::Category::Configuration);
	}
}

void NFmiCombinedMapHandler::initLandBorderDrawingSystem()
{
	doVerboseFunctionStartingLogReporting(__FUNCTION__);
	try
	{
		// release versiolle on m��r�tty eri shape-file kuin debug versiolle, koska 
		// debug versio on tolkuttoman hidas laskiessaan koordinaatteja ja siksi
		// on parempi k�ytt�� harvempaa dataa debug versiolle.
#ifdef NDEBUG 
		landBorderShapeFile_ = NFmiSettings::Require<std::string>("MetEditor::LandBorderShapeFile");
#else // debug versio
		landBorderShapeFile_ = NFmiSettings::Require<std::string>("MetEditor::LandBorderShapeFileDebug");
#endif
		boost::shared_ptr<Imagine::NFmiGeoShape> landBorderGeoShape(new Imagine::NFmiGeoShape());
		logMessage(std::string("Reading country border shape file: ") + landBorderShapeFile_, CatLog::Severity::Debug, CatLog::Category::Configuration);
		landBorderShapeFile_ = PathUtils::makeFixedAbsolutePath(landBorderShapeFile_, absoluteControlPath_);
		landBorderGeoShape->Read(landBorderShapeFile_, Imagine::kFmiGeoShapeEsri, "");

		boost::shared_ptr<Imagine::NFmiPath> landBorderPath(new Imagine::NFmiPath(landBorderGeoShape->Path()));
		boost::shared_ptr<Imagine::NFmiPath> pacificLandBorderPath(new Imagine::NFmiPath(landBorderPath->PacificView(true)));

		landBorderGeoShape_ = landBorderGeoShape;
		landBorderPath_ = landBorderPath;
		pacificLandBorderPath_ = pacificLandBorderPath;
		doCutBorderDrawInitialization();
	}
	catch(std::exception & e)
	{
		std::string errStr = "Error while reading country border shape file: ";
		errStr += landBorderShapeFile_ + "\n";
		errStr += e.what();
		logAndWarnUser(errStr, "Error while reading country border shape file", CatLog::Severity::Error, CatLog::Category::Configuration, true);
	}
}

#ifndef UNIX
void NFmiCombinedMapHandler::doCutBorderDrawInitialization()
{
	{
		// tehd��n omassa blokissa!!
		cutLandBorderPaths_.clear();
		auto &mapHandlerList = mapViewDescTops_[0]->GdiPlusImageMapHandlerList(); // otetaan p��karttan�yt�n mapHandlerList
		// Ensin lasketaan eri karttapohjille leikatut rajaviivat
		for(auto *mapHandler : mapHandlerList)
		{
			std::shared_ptr<NFmiArea> totalMapArea = mapHandler->TotalArea();
			CtrlViewUtils::CtrlViewTimeConsumptionReporter reporter(nullptr, ::makeFunctionAndAreaDescription(__FUNCTION__, totalMapArea));

			boost::shared_ptr<Imagine::NFmiPath> cutPath;
			bool totalWorldArea = ::isTotalWorld(totalMapArea);
			if(totalMapArea->PacificView())
			{
				if(totalWorldArea)
					cutPath = pacificLandBorderPath_;
				else
					cutPath = boost::shared_ptr<Imagine::NFmiPath>(new Imagine::NFmiPath(pacificLandBorderPath_->Clip(totalMapArea.get())));
			}
			else
			{
				if(totalWorldArea)
					cutPath = landBorderPath_;
				else
					cutPath = boost::shared_ptr<Imagine::NFmiPath>(new Imagine::NFmiPath(landBorderPath_->Clip(totalMapArea.get())));
			}
			cutLandBorderPaths_.push_back(cutPath);
		}
	}

	// Sitten asetetaan kaikkien karttan�ytt�jen kaikille karttapohjille leikatut rajaviivat
	for(const auto & mapViewDescTop : mapViewDescTops_)
	{
		std::vector<NFmiGdiPlusImageMapHandler*>& mapHandlerList = mapViewDescTop->GdiPlusImageMapHandlerList();
		for(size_t mapHandlerIndex = 0; mapHandlerIndex < mapHandlerList.size(); mapHandlerIndex++)
		{
			mapHandlerList[mapHandlerIndex]->LandBorderPath(cutLandBorderPaths_[mapHandlerIndex]);
		}
	}
}
#endif

void NFmiCombinedMapHandler::makeNeededDirtyOperationsWhenDataAdded(NFmiQueryData* queryData, NFmiInfoData::Type dataType, const NFmiTimeDescriptor& removedDataTimes, const std::string& fileName)
{
	NFmiTimeBag dirtyViewTimes = ::getDirtyViewTimes(queryData, dataType, removedDataTimes);

	NFmiFastQueryInfo fastInfo(queryData);
	for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
		makeNeededDirtyOperationsWhenDataAdded(mapViewDescTopIndex, fastInfo, dataType, dirtyViewTimes, fileName);

	if(CtrlViewFastInfoFunctions::IsObservationLockModeDataType(dataType))
		checkAnimationLockedModeTimeBags(CtrlViewUtils::kDoAllMapViewDescTopIndex, false);

	getCrossSectionSystem().CheckIfCrossSectionViewNeedsUpdate(queryData, dataType);

	::setUpdatedViewIdMaskAfterDataLoaded(fastInfo);
}

void NFmiCombinedMapHandler::makeNeededDirtyOperationsWhenDataAdded(unsigned int mapViewDescTopIndex, NFmiFastQueryInfo& fastInfo, NFmiInfoData::Type dataType, const NFmiTimeBag& dirtyViewTimes, const std::string& fileName)
{
	auto dataProducer = fastInfo.Producer();
	NFmiMapViewDescTop* descTop = getMapViewDescTop(mapViewDescTopIndex);
	mapViewDirty(mapViewDescTopIndex, false, false, true, false, false, false); // ei laiteta cachea likaiseksi

	int cacheRowNumber = 0; // cache row indeksi alkaa 0:sta!!
	NFmiPtrList<NFmiDrawParamList>* drawParamListVector = descTop->DrawParamListVector();
	NFmiPtrList<NFmiDrawParamList>::Iterator iter = drawParamListVector->Start();
	for(; iter.Next();)
	{
		NFmiDrawParamList* aList = iter.CurrentPtr();
		if(aList)
		{
			for(aList->Reset(); aList->Next(); )
			{
				std::shared_ptr<NFmiDrawParam> drawParam = aList->Current();
				// Ei tarvitse tarkastella mitenk��n, jos drawParam layer on piilossa
				if(drawParam->IsParamHidden())
					continue;

				makeNormalDataDrawingLayerCahceChecks(mapViewDescTopIndex, fastInfo, dataType, dirtyViewTimes, fileName, drawParam, *dataProducer, *descTop, cacheRowNumber);
				if(dataType == NFmiInfoData::kEditable && (drawParam->DataType() == NFmiInfoData::kEditable || drawParam->DataType() == NFmiInfoData::kCopyOfEdited))
				{
					// jos kyseess� oli editoitavan datan p�ivitys, laitetaan uusiksi ne rivit miss� on editoitavan datan ja sen kopion parametreja n�kyviss� (tuottajalla ei ole v�li�)
					descTop->MapViewCache().MakeRowDirty(cacheRowNumber);// clean cache row
				}
				makeMacroParamDrawingLayerCacheChecks(drawParam, fastInfo, dataType, *descTop, mapViewDescTopIndex, cacheRowNumber, fileName);
				checkAllSynopPlotTypeUpdates(mapViewDescTopIndex, drawParam, *dataProducer, *descTop, cacheRowNumber, fileName);
			}
		}
		cacheRowNumber++;
	}
}

// Halutaan vain laittaa viesti lokiin
void NFmiCombinedMapHandler::logMessage(const std::string& logMessage, CatLog::Severity severity, CatLog::Category category, bool flushAlways) const
{
	// Kaikki warning/error/fatal tason viestit pit�� flushata heti lokitiedostoon, jos ongelmista seuraa kaatuminen
	auto flushLogger = (flushAlways ? true : (severity > CatLog::Severity::Info));
	CatLog::logMessage(logMessage, severity, category, flushLogger);
}

// On kohdattu vakava virhe, laitetaan k�ytt�j�lle kysely, ett� lopetetaanko suosiolla ohjelman ajo
void NFmiCombinedMapHandler::logAndWarnUser(const std::string& logMessage, const std::string& titleString, CatLog::Severity severity, CatLog::Category category, bool addAbortOption)
{
	// Lopun parametrit false (kysy k�ytt�j�lt� dialogilla) ja true (flushaa lokiviestit heti tiedostoon, jos vaikka kaatuu kohta)
	CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->LogAndWarnUser(logMessage, titleString, severity, category, false, addAbortOption, true);
}

// T�t� kutsutaan mm. CFmiMainFrame:n OnTimer:ista kerran minuutissa.
// Tarkistaa eri n�ytt�jen animaation tilan ja moodit.
// P�ivitt�� tarvittaessa lukittujen moodien animaatio timebagit.
// 'Likaa' tarvittaessa uudet ajat cache:sta ja tekee ruudun p�ivitykset.
// mapViewDescTopIndex:ill� voidaan antaa jos halutaan tarkistaa vain tietyn n�yt�n p�ivitys tarve.
// Jos mapViewDescTopIndex:in arvo on kDoAllMapViewDescTopIndex, silloin tarkistus tehd��n kaikille n�yt�ille.
void NFmiCombinedMapHandler::checkAnimationLockedModeTimeBags(unsigned int mapViewDescTopIndex, bool ignoreSatelImages)
{
	if(isAnimationTimebagCheckNeeded(mapViewDescTopIndex))
	{ // edellinen metodi tarkisti, onko jossain animaatio boksi n�kyviss�.
		bool needToUpdateViews = false;
		// tutkitaan eri n�ytt�jen animaattoreita ja niiden tiloja
		for(unsigned int checkedDescTopIndex = 0; checkedDescTopIndex < mapViewDescTops_.size(); checkedDescTopIndex++)
		{
			if(mapViewDescTopIndex == CtrlViewUtils::kDoAllMapViewDescTopIndex || checkedDescTopIndex == mapViewDescTopIndex)
			{
				auto &checkedMapViewDescTop = *mapViewDescTops_[checkedDescTopIndex];
				NFmiAnimationData& animData = checkedMapViewDescTop.AnimationDataRef();
				NFmiAnimationData::AnimationLockMode lockMode = animData.LockMode();
				if(lockMode == NFmiAnimationData::kFollowLastObservation || lockMode == NFmiAnimationData::kFollowEarliestLastObservation)
				{
					NFmiMetTime lastTime = animData.Times().LastTime();
					NFmiMetTime currentTime(1); // otetaan viel� sein�kelloaika
					if(currentTime < lastTime || currentTime.DifferenceInMinutes(lastTime) > g_StatObservationSeekTimeLimitInMinutes)
					{ // pit�� fiksata viimeist� aikaa, koska se ei oikeastaan voi olla tulevaisuudessa, tai joku havainto on tulevaisuudesta
						// Lis�ksi pit�� fiksata jos alkuaika oli liian kaukana menneisyydess�, turha k�yd� l�pi esim. viikon verran dataa ja etsi viimeist�
						lastTime = currentTime;
						lastTime.SetTimeStep(static_cast<short>(animData.TimeStepInMinutes()));
						lastTime.ChangeByHours(-2); // l�hdet��n etsim�� viimeist� havaintoa 3 tuntia menneisyydest�
					}
					NFmiMetTime newLastTime(animData.TimeStepInMinutes()); // t�h�n sijoitetaan se aika, josta l�ytyi viimeinen n�yt�ss� olevan parametrin havainto data
					bool foundObservations = false;
					if(lockMode == NFmiAnimationData::kFollowLastObservation)
						foundObservations = findLastObservation(static_cast<unsigned long>(checkedDescTopIndex), animData.TimeStepInMinutes(), newLastTime, ignoreSatelImages);
					else
						foundObservations = findEarliestLastObservation(static_cast<unsigned long>(checkedDescTopIndex), animData.TimeStepInMinutes(), newLastTime, ignoreSatelImages);

					if(foundObservations)
					{ // l�ytyi jotain ennusteita, jossa uusi lastTime (=newLastTime), joten voidaan p�ivitt�� animaation timebagi.
						int timeDiffInMinutes = animData.Times().LastTime().DifferenceInMinutes(animData.Times().FirstTime());
						NFmiMetTime newFirstTime(newLastTime);
						newFirstTime.ChangeByMinutes(-timeDiffInMinutes);
						NFmiTimeBag newAnimTimes(newFirstTime, newLastTime, animData.TimeStepInMinutes());
						NFmiTimeBag oldAnimTimes = animData.Times();
						animData.Times(newAnimTimes);
						// Laitetaan my�s aikakontrolli-ikkuna seuraavaan animaatio ikkunaa, eli keskitet��n aikaikkuna animaatioaikojen keskikohtaan
						int animDiffInMinutes = newAnimTimes.LastTime().DifferenceInMinutes(newAnimTimes.FirstTime());
						NFmiMetTime animMiddleTime(newAnimTimes.FirstTime());
						animMiddleTime.ChangeByMinutes(boost::math::lround(animDiffInMinutes / 2.));
						animMiddleTime.SetTimeStep(animData.TimeStepInMinutes());
						centerTimeControlView(static_cast<unsigned long>(checkedDescTopIndex), animMiddleTime, false);
						// Lis�ksi likaa n�ytt� cachesta uudet l�ytyneet ajat
						::makeNewTimesDirtyFromViewCache(checkedMapViewDescTop, oldAnimTimes, newAnimTimes);
						mapViewDirty(static_cast<unsigned long>(checkedDescTopIndex), false, false, true, false, false, false); // t�m�n pit�isi asettaa n�ytt� p�ivitys tilaan, mutta cachea ei tarvitse en�� erikseen tyhjent��
					}
				}
			}
		}

		if(needToUpdateViews)
			ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Animation related update because locked time mode event occured", getUpdatedViewIdMaskForChangingTime()); // viel� p�ivitet��n n�yt�t
	}
}

NFmiMapViewDescTop* NFmiCombinedMapHandler::getMapViewDescTop(unsigned int mapViewDescTopIndex, bool allowNullptrReturn) const
{
	if(mapViewDescTops_.empty())
		throw std::runtime_error(std::string(__FUNCTION__) + " - itsMapViewDescTopList was empty, error in program.");

	if(mapViewDescTopIndex < mapViewDescTops_.size())
		return mapViewDescTops_[mapViewDescTopIndex].get();
	else
	{
		if(allowNullptrReturn)
		{
			// Eli ei palauteta defaultti arvoa eli p��ikkunan mapViewDescToppia, jos kyse oli ei-karttan�yt�n indeksist�
			return nullptr;
		}
		else
		{
			// Palautetaan erikoisn�ytt�j� (aikasarja-, poikkileikkaus-, luotaus-, datanmuokkaus-dialogit, jne.) varten p��karttanayt�n desctop
			return mapViewDescTops_[0].get();
		}
	}
}

void NFmiCombinedMapHandler::mapViewDirty(unsigned int mapViewDescTopIndex, bool makeNewBackgroundBitmap, bool clearMapViewBitmapCacheRows, bool redrawMapView, bool doClearMacroParamDataCache, bool clearEditedDataDependentCaches, bool updateMapViewDrawingLayers)
{
	if(redrawMapView)
	{
		// Ainakin toistaiseksi laitan aikasarjan likauksen t�nne
		timeSerialViewDirty_ = true;
	}

	if(mapViewDescTopIndex == CtrlViewUtils::kDoAllMapViewDescTopIndex)
		mapViewDirtyForAllDescTops(makeNewBackgroundBitmap, clearMapViewBitmapCacheRows, redrawMapView, doClearMacroParamDataCache, clearEditedDataDependentCaches, updateMapViewDrawingLayers);
	else
	{
		try
		{
			getMapViewDescTop(mapViewDescTopIndex)->MapViewDirty(makeNewBackgroundBitmap, clearMapViewBitmapCacheRows, redrawMapView, updateMapViewDrawingLayers);
		}
		catch(...)
		{
		} // Jos t�t� kutsutaan esim. poikkileikkaus n�yt�lle, lent�� poikkeus, mik� on t�ysin ok

		clearMacroParamDataCache(mapViewDescTopIndex, doClearMacroParamDataCache, clearEditedDataDependentCaches);
	}
}

void NFmiCombinedMapHandler::mapViewDirtyForAllDescTops(bool makeNewBackgroundBitmap, bool clearMapViewBitmapCacheRows, bool redrawMapView, bool doClearMacroParamDataCache, bool clearEditedDataDependentCaches, bool updateMapViewDrawingLayers)
{
	for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
	{
		try
		{
			getMapViewDescTop(mapViewDescTopIndex)->MapViewDirty(makeNewBackgroundBitmap, clearMapViewBitmapCacheRows, redrawMapView, updateMapViewDrawingLayers);
		}
		catch(...)
		{
		} // Jos t�t� kutsutaan esim. poikkileikkaus n�yt�lle, lent�� poikkeus, mik� on t�ysin ok

		clearMacroParamDataCache(mapViewDescTopIndex, doClearMacroParamDataCache, clearEditedDataDependentCaches);
	}
}

void NFmiCombinedMapHandler::setBorderDrawDirtyState(unsigned int mapViewDescTopIndex, CountryBorderDrawDirtyState newState, NFmiDrawParam* separateBorderLayerDrawOptions)
{
	if(mapViewDescTopIndex == CtrlViewUtils::kDoAllMapViewDescTopIndex)
	{
		for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
		{
			getMapViewDescTop(mapViewDescTopIndex)->SetBorderDrawDirtyState(newState, separateBorderLayerDrawOptions);
		}
	}
	else
	{
		try
		{
			getMapViewDescTop(mapViewDescTopIndex)->SetBorderDrawDirtyState(newState, separateBorderLayerDrawOptions);
		}
		catch(...)
		{
		} // Jos t�t� kutsutaan esim. poikkileikkaus n�yt�lle, lent�� poikkeus, mik� on t�ysin ok
	}
}

void NFmiCombinedMapHandler::clearMacroParamDataCache(unsigned int mapViewDescTopIndex, bool doClearMacroParamDataCache, bool clearEditedDataDependentCaches)
{
	if(doClearMacroParamDataCache)
		getMacroParamDataCache().clearView(mapViewDescTopIndex);
	if(clearEditedDataDependentCaches)
	{
		clearAllMacroParamDataCacheDependentOfEditedDataAfterEditedDataChanges();
		clearAllViewRowsWithEditedData();
	}
}

bool NFmiCombinedMapHandler::makeMacroParamDrawingLayerCacheChecks(std::shared_ptr<NFmiDrawParam>& drawParam, NFmiFastQueryInfo& fastInfo, NFmiInfoData::Type dataType, NFmiMapViewDescTop& descTop, unsigned int mapViewDescTopIndex, int cacheRowNumber, const std::string& fileName)
{
	if(drawParam->DataType() == NFmiInfoData::kMacroParam)
	{
		std::string macroParamStr = doGetMacroParamFormula(drawParam, CatLog::Category::Visualization);
		MacroParamDataChecker macroParamDataChecker;
		auto macroParamDataInfoVector = macroParamDataChecker.getCalculationParametersFromMacroPram(macroParamStr);
		for(const auto& macroParamDataInfo : macroParamDataInfoVector)
		{
			if(fastInfo.Param(macroParamDataInfo.dataIdent_))
			{
				// Jos macroParamDataInfo:n level on 'tyhj�' (ident = 0, tarkoittaa pinta parametria) tai jos annettu level l�ytyy infosta
				if(macroParamDataInfo.level_.GetIdent() == 0 || fastInfo.Level(macroParamDataInfo.level_))
				{
					if(doMacroParamVerticalDataChecks(fastInfo, dataType, macroParamDataInfo))
					{
						// clean image cache row
						descTop.MapViewCache().MakeRowDirty(cacheRowNumber);
						// MacroParam data cachen rivit alkavat 1:st�, joten image-cachen riviin on lis�tt�v� +1
						getMacroParamDataCache().clearMacroParamCache(mapViewDescTopIndex, cacheRowNumber + 1, drawParam->InitFileName());
						::macroParamDirtiesCacheRowTraceLog(drawParam, macroParamDataInfo, mapViewDescTopIndex, cacheRowNumber, fileName);
						return true;
					}
				}
			}
		}
	}
	return false;
}

std::string NFmiCombinedMapHandler::doGetMacroParamFormula(std::shared_ptr<NFmiDrawParam>& drawParam, CatLog::Category category)
{
	static std::set<std::string> reportedMissingMacroParams;

	const auto& initFileName = drawParam->InitFileName();
	std::string logErrorMessage;
	try
	{
		return CtrlViewUtils::GetMacroParamFormula(*getMacroParamSystem(), drawParam);
	}
	catch(std::exception & e)
	{
		logErrorMessage = e.what();
	}
	catch(...)
	{
		logErrorMessage = std::string("Couldn't find macroParam '") + initFileName + "'";
	}
	// Raportoidaan puuttuvasta macroParamista vain kerran per ajo
	if(reportedMissingMacroParams.find(initFileName) == reportedMissingMacroParams.end())
	{
		reportedMissingMacroParams.insert(initFileName);
		logMessage(logErrorMessage, CatLog::Severity::Error, category);
	}

	return "";
}

void NFmiCombinedMapHandler::initWmsSupport()
{
#ifndef DISABLE_CPPRESTSDK
	setWmsSupport(createWmsSupport("first time Wms-support system creation"));
#else
	logMessage("CPP-Rest disabled, no Wms-support available", CatLog::Severity::Info, CatLog::Category::Operational);
#endif // DISABLE_CPPRESTSDK
}

std::shared_ptr<WmsSupportInterface> NFmiCombinedMapHandler::createWmsSupport(const std::string& creationName) const
{
#ifndef DISABLE_CPPRESTSDK
	if(!isWmsSupportBeenKilled())
	{
		logMessage(std::string("Starting ") + creationName, CatLog::Severity::Info, CatLog::Category::Operational);
		try
		{
			auto mapViewCount = static_cast<unsigned int>(mapViewDescTops_.size());
			auto mapAreaCount = static_cast<unsigned int>(getMapViewDescTop(0)->GdiPlusImageMapHandlerList().size());
			auto wmsSupportPtr = std::make_shared<Wms::WmsSupport>();
			wmsSupportPtr->initialSetUp(mapViewCount, mapAreaCount, CombinedMapHandlerInterface::verboseLogging());
			return wmsSupportPtr;
		}
		catch(std::exception& e)
		{
			std::string errorMessage = "Problems with ";
			errorMessage += creationName;
			errorMessage += ": ";
			errorMessage += e.what();
			logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Configuration);
		}
	}
#endif // DISABLE_CPPRESTSDK
	return nullptr;
}

#ifndef DISABLE_CPPRESTSDK
void NFmiCombinedMapHandler::setWmsSupport(std::shared_ptr<WmsSupportInterface> wmsSupportPtr)
{
	if(wmsSupportPtr)
	{
		if(isWmsSupportBeenKilled())
		{
			tmpWmsSupport_ = wmsSupportPtr;
			tmpWmsSupport_->kill();
			logMessage("When creating replacement Wms-support system, system wants to shutdown, starting killing created system instead", CatLog::Severity::Debug, CatLog::Category::Operational, true);
			return;
		}

		if(wmsSupport_)
		{
			// Raportoidaan vain jos olio todella vaihdetaan, ei 1. asetuksesta
			logMessage("Replacing currently used Wms-support system with freshly created system, hopefully this way Wms-support will work for longer time period (more than 3 days in row)", CatLog::Severity::Info, CatLog::Category::Operational, true);
		}

		{
			// Vaihdetaan ensin uusi tmp-olioon, oletetaan ett� edellinen vanha on jo ehditty tappaa ja tuhota pois alta.
			tmpWmsSupport_.swap(wmsSupportPtr);

			// Jos vanha tmp-Wms-support olio on viel� hengiss�, sen pakotetusta taposta t�m�n metodin loppuessa tulee varmaan ongelmia, lokitetaan siit�
			if(wmsSupportPtr && !wmsSupportPtr->isDead(std::chrono::milliseconds(0)))
			{
				logMessage("Old already replaced Wms-support object hasn't been killed by the time new replacemnt arrived, destroing it now forcefully will probably cause problems", CatLog::Severity::Warning, CatLog::Category::Operational, true);
			}

			{
				// Vaihdetaan virallinen otus tmp-otukseen
				std::lock_guard<std::mutex> lock(wmsSupportMutex_);
				wmsSupport_.swap(tmpWmsSupport_);
			}
			
			if(tmpWmsSupport_)
			{
				// Jos oli jo k�yt�ss� wmsSupport olio, sit� pit�� alkaa tappamaan
				tmpWmsSupport_->kill();
			}
		}
	}
}

bool NFmiCombinedMapHandler::isWmsSupportBeenKilled() const
{
	std::lock_guard<std::mutex> lock(wmsSupportMutex_);
	if(wmsSupport_ && wmsSupport_->isSetToBeKilled())
		return true;
	else
		return false;
}

void NFmiCombinedMapHandler::startWmsSupportRenewalProcess(bool startedByUser)
{
	static int counter = 1;
	std::string creationName = "creating a new Wms-support system and replacing the currently used with it (";
	creationName += startedByUser ? "started by user" : " started by automatic timer, counter : ";
	if(!startedByUser)
	{
		creationName += std::to_string(counter);
	}
	creationName += ")";
	std::thread t(&NFmiCombinedMapHandler::doWmsSupportRenewalProcessInSeparateThread, this, creationName);
	t.detach();
	counter++;
}

void NFmiCombinedMapHandler::doWmsSupportRenewalProcessInSeparateThread(const std::string& creationName)
{
	// Tarkistetaan ettei systeemi ole jo k�ynniss�
	if(!isWmsSupportRenewalProcessRunningMutex_.try_lock())
	{
		logMessage("Wms-support renewal process was already running, stopping this call", CatLog::Severity::Warning, CatLog::Category::Operational, true);
	}
	else
	{
		// Avaan lukon ja laitan sen lock_guard:iin, jotta ei tarvitse laittaan kaikkiin return kohtiin erillist� avausta
		isWmsSupportRenewalProcessRunningMutex_.unlock();
		std::lock_guard<std::mutex> lock(isWmsSupportRenewalProcessRunningMutex_);

		auto wmsSupportPtr = createWmsSupport(creationName);
		if(wmsSupportPtr)
		{
			for(;;)
			{
				// Odotetaan ett� juuri luotu wmsSupport otus saa tehty� getcapabilities haut ennen kuin otus otetaan k�ytt��n
				if(isWmsSupportBeenKilled())
				{
					// Jos smartmetia ollaan sulkemassa, voidaan t�m� odottelu lopettaa ja sijoitetaan wmsSupportPtr odottelemaan 
					// loppuaan tmpWmsSupport_ olioon...
					tmpWmsSupport_.swap(wmsSupportPtr);
					tmpWmsSupport_->kill();
					return;
				}
				if(wmsSupportPtr->getCapabilitiesHaveBeenRetrieved())
				{
					logMessage("Replacement Wms-support object has been properly initialized and it's getCapabilities have been retrieved once", CatLog::Severity::Debug, CatLog::Category::Operational, true);
					break;
				}
				// T�ss� laitetaan threadi nukkumaan pieneksi aikaa, jotta tarkasteluja voidaan sitten jatkaa
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}
			// Nyt t�ysin toiminta valmis wmsSupport olio voidaan ottaa k�ytt��n
			setWmsSupport(wmsSupportPtr);
			if(tmpWmsSupport_)
			{
				// Lopuksi odotellaan ett� tmp-wms-support olio tappaa itsens� ja ett� se voidaan poistaa ja nollata
				for(;;)
				{
					if(isWmsSupportBeenKilled())
					{
						// Jos smartmetia ollaan sulkemassa, voidaan t�m� odottelu lopettaa
						return;
					}

					// t�ss� pika tarkastus (0 lukko aika), voidaanko nollata
					{
						std::lock_guard<std::mutex> lock(wmsSupportMutex_);
						if(tmpWmsSupport_->isDead(std::chrono::milliseconds(0)))
						{
							tmpWmsSupport_.reset();
							logMessage("Replaced Wms-support object has been killed and disposed", CatLog::Severity::Debug, CatLog::Category::Operational, true);
							break;
						}
					}
					// T�ss� laitetaan threadi nukkumaan pieneksi aikaa, jotta tarkasteluja voidaan sitten jatkaa
					std::this_thread::sleep_for(std::chrono::milliseconds(500));
				}
			}
		}
	}
}

static bool waitWmsSupportToDie(const std::chrono::milliseconds& waitTime, std::shared_ptr<WmsSupportInterface> wmsSupport)
{
	if(wmsSupport)
	{
		return wmsSupport->isDead(waitTime);
	}
	return true;
}


bool NFmiCombinedMapHandler::waitWmsSupportToDie(const std::chrono::milliseconds& waitTime)
{
	auto mainWmsSupportIsDead = ::waitWmsSupportToDie(waitTime, getWmsSupport());
	bool tmpWmsSupportIsDead = true;
	{
		std::lock_guard<std::mutex> lock(wmsSupportMutex_);
		tmpWmsSupportIsDead = ::waitWmsSupportToDie(waitTime, tmpWmsSupport_);
	}
	return (mainWmsSupportIsDead && tmpWmsSupportIsDead);
}
#endif // DISABLE_CPPRESTSDK

bool NFmiCombinedMapHandler::isAnimationTimebagCheckNeeded(unsigned int mapviewDescTopIndex)
{
	if(mapviewDescTopIndex == CtrlViewUtils::kDoAllMapViewDescTopIndex)
	{
		for(const auto &mapViewDescTop : mapViewDescTops_)
		{
			if(mapViewDescTop->AnimationDataRef().ShowTimesOnTimeControl())
				return true;
		}
	}
	else
	{
		try
		{
			auto mapViewDescTop = getMapViewDescTop(mapviewDescTopIndex);
			if(mapViewDescTop)
				return mapViewDescTop->AnimationDataRef().ShowTimesOnTimeControl();
		}
		catch(std::exception & e)
		{
			std::string errorString = "Error in IsAnimationTimebagCheckNeeded: ";
			errorString += e.what();
			logMessage(errorString, CatLog::Severity::Error, CatLog::Category::Operational);
		}
	}
	return false;
}

#ifndef UNIX
bool NFmiCombinedMapHandler::findLastObservation(unsigned long mapViewDescTopIndex, int timeStepInMinutes, NFmiMetTime& newLastTime, bool ignoreSatelImages)
{
	// t�m� on jonkinlainen rajapyykki, eli t�m�n yli kun menn��n (ei pit�isi menn�), lopetetaan havaintojen etsiminene siihen
	NFmiMetTime currentTime(1);
	// haetaan min ja maksimi aika limitit, jotka on n. nykyhetki ja 2h - nykyhetki
	NFmiMetTime timeLimit1;
	NFmiMetTime timeLimit2;
	::makeObsSeekTimeLimits(currentTime, timeStepInMinutes, timeLimit1, timeLimit2);

	NFmiMapViewDescTop& mapViewDescTop = *(getMapViewDescTop(mapViewDescTopIndex));
	NFmiPtrList<NFmiDrawParamList>::Iterator iter = mapViewDescTop.DrawParamListVector()->Start();
	bool newerTimeFound = false;
	int viewRowIndex = 1;
	for(; iter.Next(); viewRowIndex++)
	{
		if(mapViewDescTop.IsVisibleRow(viewRowIndex) == false)
			continue; // ei k�yd� l�pi piilossa olevia rivej�
		NFmiDrawParamList* aList = iter.CurrentPtr();
		if(aList)
		{
			for(aList->Reset(); aList->Next(); )
			{
				std::shared_ptr<NFmiDrawParam> drawParam = aList->Current();
				if(drawParam->IsParamHidden() == false)
				{
					if(::isObservationLockModeQueryData(drawParam))
					{
						std::vector<std::shared_ptr<NFmiFastQueryInfo> > infoVector;
						makeDrawedInfoVectorForMapView(infoVector, drawParam, mapViewDescTop.MapHandler()->Area());
						for(size_t i = 0; i < infoVector.size(); i++)
						{
							bool demandExactTimeChecking = drawParam->DataType() != NFmiInfoData::kFlashData; // t�ss� vaiheessa salama data on sellainen jossa ei vaadita tarkkoja aika tarkasteluja
							std::shared_ptr<NFmiFastQueryInfo>& info = infoVector[i];
							NFmiMetTime dataLastTime;
							NFmiTimeDescriptor timeDesc = info->TimeDescriptor();
							if(::getLatestValidTimeWithCorrectTimeStep(timeDesc, timeStepInMinutes, demandExactTimeChecking, timeLimit1, timeLimit2, dataLastTime))
							{
								if(::checkLastObservationTime(&newerTimeFound, dataLastTime, currentTime, &newLastTime, timeStepInMinutes))
								{
									// Ei tarvitse en�� jatkaa, koska aika joka l�ytyi on viimeisin mahdollinen.
									return true;
								}
							}
						}
					}
					else if(::isObservationLockModeSatelData(drawParam, ignoreSatelImages))
					{ 
						// tutki l�ytyyk� satel-data hakemistosta uudempia datoja, kuin annettu theLastTime
						NFmiMetTime satelLastTime;
						if(::getLatestSatelImageTime(drawParam->Param(), satelLastTime))
						{
							if(::checkLastObservationTime(&newerTimeFound, satelLastTime, currentTime, &newLastTime, timeStepInMinutes))
							{
								// Ei tarvitse en�� jatkaa, koska aika joka l�ytyi on viimeisin mahdollinen.
								return true;
							}
						}
					}
					else if(::isObservationLockModeWmsData(drawParam))
					{ 
						// tutki l�ytyyk� wms-datasta aikoja datoja, kuin annettu theLastTime
						NFmiMetTime wmsLastTime;
						if(getLatestWmsImageTime(drawParam->Param(), wmsLastTime))
						{
							if(::checkLastObservationTime(&newerTimeFound, wmsLastTime, currentTime, &newLastTime, timeStepInMinutes))
							{
								// Ei tarvitse en�� jatkaa, koska aika joka l�ytyi on viimeisin mahdollinen.
								return true;
							}
						}
					}
				}
			}
		}
	}
	return newerTimeFound;
}
#endif // UNIX

#ifndef DISABLE_CPPRESTSDK
bool NFmiCombinedMapHandler::getLatestWmsImageTime(const NFmiDataIdent& dataIdent, NFmiMetTime& foundTimeOut)
{
	try
	{
		auto layerInfo = getWmsSupport()->getHashedLayerInfo(dataIdent);
		foundTimeOut = layerInfo.endTime;
		return true;
	}
	catch(...)
	{
	}
	return false;
}
#endif // DISABLE_CPPRESTSDK

#ifndef UNIX
// Tutkii k�ikki n�kyv�t havainto datat ja etsii sen ajan, mik� on aikaisin eri datojen viimeisist� ajoista.
// T�ll� on pyrkimys siihen ett� animaatio ei v�lky, kun kaikilta datoilta periaattessa l�ytyy dataa my�s
// t�ll� tavalla etsittyyn viimeiseen aikaan.
bool NFmiCombinedMapHandler::findEarliestLastObservation(unsigned long mapViewDescTopIndex, int timeStepInMinutes, NFmiMetTime& newLastTime, bool ignoreSatelImages)
{
	NFmiMetTime currentTime(timeStepInMinutes); // t�m� on jonkinlainen rajapyykki, eli t�m�n yli kun menn��n (ei pit�isi menn�), lopetetaan havaintojen etsiminene siihen
	// haetaan min ja maksimi aika limitit, jotka on n. nykyhetki ja 2h - nykyhetki
	NFmiMetTime timeLimit1;
	NFmiMetTime timeLimit2;
	::makeObsSeekTimeLimits(currentTime, timeStepInMinutes, timeLimit1, timeLimit2);

	NFmiMapViewDescTop& mapViewDescTop = *(getMapViewDescTop(mapViewDescTopIndex));
	NFmiPtrList<NFmiDrawParamList>* drawParamListVector = mapViewDescTop.DrawParamListVector();
	NFmiPtrList<NFmiDrawParamList>::Iterator iter = drawParamListVector->Start();
	bool anyTimeFound = false;
	NFmiMetTime earliestLastTime(2200, 1, 1); // t�h�n vain iso tulevaisuuden luku
	int viewRowIndex = 1;
	for(; iter.Next(); viewRowIndex++)
	{
		if(mapViewDescTop.IsVisibleRow(viewRowIndex) == false)
			continue; // ei k�yd� l�pi piilossa olevia rivej�
		NFmiDrawParamList* aList = iter.CurrentPtr();
		if(aList)
		{
			for(aList->Reset(); aList->Next(); )
			{
				std::shared_ptr<NFmiDrawParam> drawParam = aList->Current();
				if(drawParam->IsParamHidden() == false)
				{
					if(::isObservationLockModeQueryData(drawParam))
					{
						std::vector<std::shared_ptr<NFmiFastQueryInfo> > infoVector;
						makeDrawedInfoVectorForMapView(infoVector, drawParam, mapViewDescTop.MapHandler()->Area());
						NFmiMetTime lastTimeOfThisDataType; // mm. synop datan tapauksessa haetaan ehk� jopa 6:sta datat tiedostosta viimeisint� aikaa
						bool lastTimeOfThisDataTypeFoundYet = false;
						for(size_t i = 0; i < infoVector.size(); i++)
						{
							if(drawParam->DataType() != NFmiInfoData::kFlashData) // Salamadatoja ei oteta "Latest mutual time" tarkasteluihin
							{
								bool demandExactTimeChecking = true;
								std::shared_ptr<NFmiFastQueryInfo>& info = infoVector[i];
								NFmiMetTime dataLastTime;
								NFmiTimeDescriptor timeDesc = info->TimeDescriptor();
								if(::getLatestValidTimeWithCorrectTimeStep(timeDesc, timeStepInMinutes, demandExactTimeChecking, timeLimit1, timeLimit2, dataLastTime))
								{
									if(lastTimeOfThisDataTypeFoundYet == false || dataLastTime > lastTimeOfThisDataType)
									{
										lastTimeOfThisDataTypeFoundYet = true;
										lastTimeOfThisDataType = dataLastTime;
									}
								}
							}
						}
						// t�ss� drawParam kohtaisten datojen aika tarkastelut
						if(lastTimeOfThisDataTypeFoundYet)
						{
							::checkEarliestLastObservationTime(&anyTimeFound, lastTimeOfThisDataType, &earliestLastTime, timeStepInMinutes);
						}
					}
					else if(::isObservationLockModeSatelData(drawParam, ignoreSatelImages))
					{
						// tutki l�ytyyk� satel-data hakemistosta uudempia datoja, kuin annettu theLastTime
						NFmiMetTime satelLastTime;
						if(::getLatestSatelImageTime(drawParam->Param(), satelLastTime))
						{
							::checkEarliestLastObservationTime(&anyTimeFound, satelLastTime, &earliestLastTime, timeStepInMinutes);
						}
					}
					else if(::isObservationLockModeWmsData(drawParam))
					{
						// tutki l�ytyyk� wms-datasta aikoja datoja, kuin annettu theLastTime
						NFmiMetTime wmsLastTime;
						if(getLatestWmsImageTime(drawParam->Param(), wmsLastTime))
						{
							::checkEarliestLastObservationTime(&anyTimeFound, wmsLastTime, &earliestLastTime, timeStepInMinutes);
						}
					}
				}
			}
		}
	}
	if(anyTimeFound)
	{
		newLastTime = earliestLastTime;
		return true;
	}
	else
		return false;
}
#endif // UNIX

// Funktio s��t�� halutun aikakontrolli ikkunan siten ett� se keskitt�� annetun ajan n�kyviin.
// Eli annettu aika menee aikakontrolli ikkunan keskelle. Halutessa my�s p�ivitet��n
// valittu aika siihen.
void NFmiCombinedMapHandler::centerTimeControlView(unsigned int mapViewDescTopIndex, const NFmiMetTime& wantedTime, bool updateSelectedTime)
{
	auto* mapViewDescTop = getMapViewDescTop(mapViewDescTopIndex);
	NFmiTimeBag times = *(mapViewDescTop->TimeControlViewTimes().ValidTimeBag());
	auto timeStepInMinutes = mapViewDescTop->TimeControlTimeStepInMinutes();
	long timeLengthInMinutes = times.LastTime().DifferenceInMinutes(times.FirstTime());
	long neededChangeInMinutes = times.FirstTime().DifferenceInMinutes(wantedTime) + timeLengthInMinutes / 2;
	times.MoveByMinutes(-neededChangeInMinutes);
	NFmiTimeBag fixedTimeBag = CtrlViewUtils::GetAdjustedTimeBag(times.FirstTime(), times.LastTime(), timeStepInMinutes);
	NFmiTimeDescriptor newTimeDesc(mapViewDescTop->TimeControlViewTimes().OriginTime(), fixedTimeBag);
	mapViewDescTop->TimeControlViewTimes(newTimeDesc);

	if(updateSelectedTime)
	{
		NFmiMetTime newTime(wantedTime);
		newTime.SetTimeStep(timeStepInMinutes, true);
		currentTime(mapViewDescTopIndex, newTime, false);
	}
	mapViewDirty(mapViewDescTopIndex, false, false, true, false, false, false); // t�m�n pit�isi asettaa n�ytt� p�ivitys tilaan, mutta cachea ei tarvitse en�� erikseen tyhjent��
}

SmartMetViewId NFmiCombinedMapHandler::getUpdatedViewIdMaskForChangingTime()
{
	SmartMetViewId updatedViewIds = SmartMetViewId::AllMapViews | SmartMetViewId::TimeSerialView | SmartMetViewId::DataFilterToolDlg | SmartMetViewId::CrossSectionView | SmartMetViewId::StationDataTableView | SmartMetViewId::WarningCenterDlg;
	if(::getMTATempSystem().GetSoundingViewSettingsFromWindowsRegisty().SoundingTimeLockWithMapView())
		updatedViewIds = updatedViewIds | SmartMetViewId::SoundingView;
	if(::getWindTableSystem().UseMapTime())
		updatedViewIds = updatedViewIds | SmartMetViewId::WindTableDlg;
	return updatedViewIds;
}

void NFmiCombinedMapHandler::updateTimeInLockedDescTops(const NFmiMetTime& wantedTime, unsigned int originalMapViewDescTopIndex)
{
	// Jos tehd��n jotain aika muutoksia mihink��n karttan�ytt��n, laitetaan optimoitu update maski p��lle
	ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(getUpdatedViewIdMaskForChangingTime());

	// P�ivitet��n ajan muutoksessa my�s aina luotausn�ytt��, jos s��d�t ovat kohdallaan
	if(::getMTATempSystem().GetSoundingViewSettingsFromWindowsRegisty().SoundingTimeLockWithMapView() && ::getMTATempSystem().TempViewOn())
		ApplicationInterface::GetApplicationInterfaceImplementation()->UpdateTempView();
	
	// Jos originaali ikkunassa on animaatio p��ll� tai on edes animointi alue n�kyviss�, ei tehd� mit��n
	auto& originalMapViewdescTop = *getMapViewDescTop(originalMapViewDescTopIndex);
	if(originalMapViewdescTop.AnimationDataRef().AnimationOn() || originalMapViewdescTop.AnimationDataRef().ShowTimesOnTimeControl())
		return;

	// eli jos origIndex oli p��ikkuna (index = 0) tai apuikkuna oli lukittu p��ikkunaan, silloin tehd��n aika p�ivityksi�
	if(originalMapViewDescTopIndex == 0 || originalMapViewdescTop.LockToMainMapViewTime())
	{
		for(unsigned int currentMapViewDescTopIndex = 0; currentMapViewDescTopIndex < mapViewDescTops_.size(); currentMapViewDescTopIndex++)
		{
			auto& currentMapViewdescTop = *getMapViewDescTop(currentMapViewDescTopIndex);
			if(currentMapViewDescTopIndex == 0 || currentMapViewdescTop.LockToMainMapViewTime())
			{
				// jos l�pik�yt�v�ss� ikkunassa on animointia tai edes anim.-ikkuna n�kyviss�, ei tehd� mit��n
				if(currentMapViewdescTop.AnimationDataRef().AnimationOn() || currentMapViewdescTop.AnimationDataRef().ShowTimesOnTimeControl())
					continue;
				currentMapViewdescTop.CurrentTime(wantedTime);
			}
		}
	}
}

std::vector<std::string> NFmiCombinedMapHandler::makeListOfUsedMacroParamsDependedOnEditedData()
{
	std::vector<std::string> macroParamPathList;
	for(auto& mapViewDescTop : mapViewDescTops_)
	{
		NFmiPtrList<NFmiDrawParamList>::Iterator iter = mapViewDescTop->DrawParamListVector()->Start();

		int cacheRowNumber = 0;
		for(; iter.Next();)
		{
			NFmiDrawParamList* aList = iter.CurrentPtr();
			if(aList)
			{
				for(aList->Reset(); aList->Next(); )
				{
					std::shared_ptr<NFmiDrawParam> drawParam = aList->Current();
					if(isMacroParamDependentOfEditedData(drawParam))
					{
						macroParamPathList.push_back(drawParam->InitFileName());
						// Tyhjennet��n my�s kyseisten n�ytt�rivien bitmap cache (ik�v� kaksois vastuu metodilla)
						mapViewDescTop->MapViewCache().MakeRowDirty(cacheRowNumber);
					}
				}
			}
			cacheRowNumber++;
		}
	}
	return macroParamPathList;
}

void NFmiCombinedMapHandler::clearAllMacroParamDataCacheDependentOfEditedDataAfterEditedDataChanges()
{
	auto macroParamPaths = makeListOfUsedMacroParamsDependedOnEditedData();
	getMacroParamDataCache().clearMacroParamCache(macroParamPaths);
}

bool NFmiCombinedMapHandler::isMacroParamDependentOfEditedData(std::shared_ptr<NFmiDrawParam>& drawParam)
{
	if(drawParam->DataType() == NFmiInfoData::kMacroParam)
	{
		std::string macroParamStr = doGetMacroParamFormula(drawParam, CatLog::Category::Visualization);
		MacroParamDataChecker macroParamDataChecker;
		auto macroParamDataInfoVector = macroParamDataChecker.getCalculationParametersFromMacroPram(macroParamStr);
		for(const auto& macroParamDataInfo : macroParamDataInfoVector)
		{
			if(macroParamDataInfo.type_ == NFmiInfoData::kEditable)
				return true;
		}
	}
	return false;
}

// Tyhjennet��n bitmap cached eri karttan�yt�ist� niilt� riveilt�, miss� on editoitua dataa katseltavana.
// Lis�ksi liataan aikasarja ikkuna, jos siell� on editoitua parametria valittuna.
void NFmiCombinedMapHandler::clearAllViewRowsWithEditedData()
{
	for(auto& mapViewDescTop : mapViewDescTops_)
	{
		// cache row indeksi alkaa 0:sta!!
		int cacheRowNumber = 0; 
		NFmiPtrList<NFmiDrawParamList>::Iterator iter = mapViewDescTop->DrawParamListVector()->Start();
		for(; iter.Next();)
		{
			NFmiDrawParamList* drawParamList = iter.CurrentPtr();
			if(::drawParamListContainsEditedData(*drawParamList))
			{
				mapViewDescTop->MapViewCache().MakeRowDirty(cacheRowNumber);
			}
			cacheRowNumber++;
		}
	}

	// Lopuksi viel� likaus aikasarjaan, jos tarvis
	if(::drawParamListContainsEditedData(*CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->TimeSerialViewDrawParamList()))
		timeSerialViewDirty_ = true;
}

void NFmiCombinedMapHandler::makeDrawedInfoVectorForMapView(std::vector<std::shared_ptr<NFmiFastQueryInfo> >& infoVectorOut, std::shared_ptr<NFmiDrawParam>& drawParam, const std::shared_ptr<NFmiArea>& area)
{
	infoVectorOut.clear();
	auto dataType = drawParam->DataType();
	auto producerId = drawParam->Param().GetProducer()->GetIdent();
	auto paramId = drawParam->Param().GetParamIdent();
	bool notEditedData = ((dataType != NFmiInfoData::kEditable) && (dataType != NFmiInfoData::kCopyOfEdited));
	auto& infoOrganizer = ::getInfoOrganizer();
	if(notEditedData && dataType == NFmiInfoData::kFlashData)
	{
		infoVectorOut = infoOrganizer.GetInfos(NFmiInfoData::kFlashData);
	}
	else if(notEditedData && (producerId == kFmiSYNOP || producerId == NFmiInfoData::kFmiSpSynoXProducer || paramId == NFmiInfoData::kFmiSpSynoPlot || paramId == NFmiInfoData::kFmiSpMinMaxPlot))
	{ // synop-data on aluksi poikkeus, mille tehd��n vektori, miss� useita infoja, jos niit� l�ytyy
		if(producerId == NFmiInfoData::kFmiSpSynoXProducer)
			infoVectorOut = ::getSortedSynopInfoVector(kFmiSYNOP, kFmiTestBed);
		else
			infoVectorOut = ::getSortedSynopInfoVector(kFmiSYNOP, kFmiTestBed, kFmiSHIP, kFmiBUOY);
		for(auto fastInfo : infoVectorOut)
			fastInfo->Param(static_cast<FmiParameterName>(paramId)); // Laitetaan kaikki synop-datat osoittamaan haluttua parametria
	}
	else
	{
		std::shared_ptr<NFmiFastQueryInfo> info = infoOrganizer.Info(drawParam, false, false);
		if(info)
		{
			if(dataType == NFmiInfoData::kMacroParam || dataType == NFmiInfoData::kQ3MacroParam)
			{ 
				// makroparamille pit�� s��t�� laskettavan hilan alue vastaamaan karttan�yt�n aluetta
				if(area)
				{
					NFmiExtraMacroParamData::SetUsedAreaForData(info, area.get());
				}
			}

			infoVectorOut.push_back(info);
		}
	}
	// Lopuksi pit�� viel� s��t�� piirrett�vien infojen maskit knomask-tilaan, koska 
	// olen poistanut ns. aktivationMaskin k�yt�n ja info on voinut j��d� esim. selected-mask tilaan
	::setInfosMask(infoVectorOut, NFmiMetEditorTypes::kFmiNoMask);
}

// Tutkii vertikaali macroParam funktioiden kanssa ett� jos kyse on pressure datasta, ett� l�ytyyk� 
// samalta tuottajalta my�s hybrid dataa, miss� on haluttu parametri.
// Kaikissa muissa tapauksissa palauttaa true (eli tehd��n rivin cache likaus), paitsi jos data 
// pressure dataa ja l�ytyy vastaava hybrid data infoOrganizerista.
bool NFmiCombinedMapHandler::doMacroParamVerticalDataChecks(NFmiFastQueryInfo& info, NFmiInfoData::Type dataType, const MacroParamDataInfo& macroParamDataInfo)
{
	if(macroParamDataInfo.usedWithVerticalFunction_)
	{
		// Jos dataa k�ytetty vertikaali funktioiden kanssa, pit�� siin� olla yli 2 leveli�, muuten sit� ei k�ytet� macroParam laskuissa
		if(info.SizeLevels() > 2)
		{
			if(dataType == NFmiInfoData::kViewable)
			{
				if(info.LevelType() == kFmiPressureLevel)
				{
					auto hybridData = ::getInfoOrganizer().Info(macroParamDataInfo.dataIdent_, nullptr, NFmiInfoData::kHybridData, false, true);
					if(hybridData)
						return false; // l�ytyi vastaava hybrid data, eli ei tehd� rivin p�ivityst� t�lle datalle
				}
			}
		}
		else
			return false;
	}

	return true;
}

NFmiMetTime NFmiCombinedMapHandler::adjustTimeToDescTopTimeStep(unsigned int mapViewDescTopIndex, const NFmiMetTime& wantedTime)
{
	NFmiMetTime aTime(wantedTime);
	auto timeStepInMinutes = getMapViewDescTop(mapViewDescTopIndex)->TimeControlTimeStepInMinutes();
	if(timeStepInMinutes == 0) // ei voi olla 0 timesteppi, muuten kaatuu (negatiivisesta en tied�)
		timeStepInMinutes = 60; // h�t� korjaus defaultti arvoksi jos oli 0
	if(aTime.GetTimeStep() > timeStepInMinutes)
		aTime.SetTimeStep(timeStepInMinutes);
	return aTime;
}

bool NFmiCombinedMapHandler::currentTimeForAllDescTops(const NFmiMetTime& newCurrentTime)
{
	for(unsigned int currentMapViewDescTopIndex = 0; currentMapViewDescTopIndex < mapViewDescTops_.size(); currentMapViewDescTopIndex++)
	{
		NFmiMetTime aTime(adjustTimeToDescTopTimeStep(currentMapViewDescTopIndex, newCurrentTime));
		getMapViewDescTop(currentMapViewDescTopIndex)->CurrentTime(aTime);
	}
	return true;
}

bool NFmiCombinedMapHandler::currentTime(unsigned int mapViewDescTopIndex, const NFmiMetTime& newCurrentTime, bool stayInsideAnimationTimes)
{
	if(mapViewDescTopIndex == CtrlViewUtils::kDoAllMapViewDescTopIndex)
		return currentTimeForAllDescTops(newCurrentTime);

	NFmiMetTime aTime(adjustTimeToDescTopTimeStep(mapViewDescTopIndex, newCurrentTime));
	aTime = calcAnimationRestrictedTime(mapViewDescTopIndex, aTime, stayInsideAnimationTimes);
	getMapViewDescTop(mapViewDescTopIndex)->CurrentTime(aTime);

	updateTimeInLockedDescTops(aTime, mapViewDescTopIndex);
	return true;
}

bool NFmiCombinedMapHandler::doAnimationRestriction(unsigned int mapViewDescTopIndex, bool stayInsideAnimationTimes)
{
	if(stayInsideAnimationTimes)
		return getMapViewDescTop(mapViewDescTopIndex)->AnimationDataRef().ShowTimesOnTimeControl();
	else
		return false;
}

NFmiMetTime NFmiCombinedMapHandler::calcAnimationRestrictedTime(unsigned int mapViewDescTopIndex, const NFmiMetTime& wantedTime, bool stayInsideAnimationTimes)
{
	NFmiMetTime fixedTime = wantedTime;
	if(doAnimationRestriction(mapViewDescTopIndex, stayInsideAnimationTimes))
	{
		NFmiAnimationData& animData = getMapViewDescTop(mapViewDescTopIndex)->AnimationDataRef();
		if(animData.Times().IsInside(fixedTime) == false)
		{
			NFmiTimeBag tmpTimes = animData.Times();
			if(tmpTimes.FindNearestTime(fixedTime))
				fixedTime = tmpTimes.CurrentTime();
		}
	}
	return fixedTime;
}

#ifndef UNIX
void NFmiCombinedMapHandler::setSelectedMap(unsigned int mapViewDescTopIndex, int newMapIndex)
{
	// newMapIndex pit�� ensin asettaa mapViewDescTopiin!
	auto& mapViewDescTop = *getMapViewDescTop(mapViewDescTopIndex);
	mapViewDescTop.SelectedMapIndex(newMapIndex);
	// Sen arvo saattaa muuttua ja lopullinen arvo pit�� tallettaa my�s Windows rekistereihin
	::getApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(mapViewDescTopIndex)->SelectedMapIndex(mapViewDescTop.SelectedMapIndex());

	setMapArea(mapViewDescTopIndex, mapViewDescTop.MapHandler()->Area());
}
#endif

#ifndef UNIX
void NFmiCombinedMapHandler::setSelectedMapHandler(unsigned int mapViewDescTopIndex, unsigned int newMapIndex)
{
	setSelectedMap(mapViewDescTopIndex, newMapIndex);
	mapViewDirty(mapViewDescTopIndex, true, true, true, false, false, false);
	ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(GetWantedMapViewIdFlag(mapViewDescTopIndex));
}

void NFmiCombinedMapHandler::setMapArea(unsigned int mapViewDescTopIndex, const std::shared_ptr<NFmiArea>& newArea)
{
	if(newArea)
	{
		auto *mapDescTop = getMapViewDescTop(mapViewDescTopIndex);
		auto& totalArea = mapDescTop->MapHandler()->TotalArea();
		// t�h�n laitetaan tarkistus, ett� zoomi area ja karttapohjan areat ovat saman tyyppiset
		// Jos eiv�t ole, tehd��n mahd. samanlainen area, mik� newArea on, mutta mik� sopii
		// k�yt�ss� olevan dipmaphandlerin kartta pohjaan
		if(NFmiQueryDataUtil::AreAreasSameKind(newArea.get(), totalArea.get()))
		{
			mapDescTop->MapHandler()->Area(newArea);
			::SetCPCropGridSettings(newArea, mapViewDescTopIndex);
		}
		else
		{ // tehd��n sitten karttapohjalle sopiva area
			std::shared_ptr<NFmiArea> correctTypeArea(totalArea->NewArea(newArea->BottomLeftLatLon(), newArea->TopRightLatLon()));
			if(correctTypeArea)
			{
				if(!totalArea->IsInside(*correctTypeArea))
				{ // pit�� v�h�n viilata areaa, koska se ei mene kartta-alueen sis�lle
					NFmiRect xyRect(totalArea->XYArea(correctTypeArea.get()));
					if(xyRect.Left() < 0)
						xyRect.Left(0);
					if(xyRect.Right() > 1)
						xyRect.Right(1);
					if(xyRect.Top() < 0)
						xyRect.Top(0);
					if(xyRect.Bottom() > 1)
						xyRect.Bottom(1);
					NFmiPoint blLatlon(totalArea->ToLatLon(xyRect.BottomLeft()));
					NFmiPoint trLatlon(totalArea->ToLatLon(xyRect.TopRight()));
					correctTypeArea = std::shared_ptr<NFmiArea>(totalArea->NewArea(blLatlon, trLatlon));
				}
			}
			mapDescTop->MapHandler()->Area(correctTypeArea);
			::SetCPCropGridSettings(correctTypeArea, mapViewDescTopIndex);
		}
		mapDescTop->SetBorderDrawDirtyState(CountryBorderDrawDirtyState::Geometry);
		// laitetaan viela kaikki ajat likaisiksi cachesta
		mapViewDirty(mapViewDescTopIndex, true, true, true, true, false, false);
		mapDescTop->GridPointCache().Clear();
		if(mapDescTop->MapHandler()->MapReallyChanged() && CatLog::doTraceLevelLogging())
		{
			std::string areaChangedString = "Map view ";
			areaChangedString += std::to_string(mapViewDescTopIndex + 1);
			areaChangedString += " area changed to ";
			areaChangedString += mapDescTop->MapHandler()->Area()->AreaStr();
			CatLog::logMessage(areaChangedString, CatLog::Severity::Trace, CatLog::Category::Operational);
		}
	}
}
#endif // UNIX

NFmiProjectionCurvatureInfo* NFmiCombinedMapHandler::projectionCurvatureInfo()
{
	return projectionCurvatureInfo_.get();
}

void NFmiCombinedMapHandler::projectionCurvatureInfo(const NFmiProjectionCurvatureInfo& newValue)
{
	*projectionCurvatureInfo_ = newValue;
}

void NFmiCombinedMapHandler::checkForNewConceptualModelData()
{
	bool needsToUpdateViews = false;
	for(unsigned int currentMapViewDescTopIndex = 0; currentMapViewDescTopIndex < mapViewDescTops_.size(); currentMapViewDescTopIndex++)
		needsToUpdateViews |= checkForNewConceptualModelDataBruteForce(currentMapViewDescTopIndex);

	if(needsToUpdateViews)
		ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: Brute force conceptual model data update required", SmartMetViewId::AllMapViews);
}

bool NFmiCombinedMapHandler::checkForNewConceptualModelDataBruteForce(unsigned int mapViewDescTopIndex)
{
	// T�m� on ns. brute force ratkaisu ei-queryData pohjaisten datojen p�ivitys tarpeelle.
	// Jos n�yt�n rivill� on jotain t�ll�ist� dataa, 'liataan' aina koko rivi varmuuden vuoksi.
	// HUOM! jos jossain karttan�yt�ss� on animaattori p��ll�, ei tehd� likausta, koska animaatioidulle datalle oli omia tarkastuksia.

	// 1. Tarkita onko karttan�yt�n n�ytt�riveill� k�siteanalyysi datoja
	// 2. Jos on, liataan kyseinen rivi
	auto* mapViewDescTop = getMapViewDescTop(mapViewDescTopIndex);
	if(mapViewDescTop->AnimationDataRef().AnimationOn())
		return false;

	bool needsToUpdateViews = false;

	int cacheRowNumber = 0; // cache row indeksi alkaa 0:sta!!
	NFmiPtrList<NFmiDrawParamList>::Iterator iter = mapViewDescTop->DrawParamListVector()->Start();
	for(; iter.Next();)
	{
		NFmiDrawParamList* aList = iter.CurrentPtr();
		if(aList)
		{
			for(aList->Reset(); aList->Next(); )
			{
				std::shared_ptr<NFmiDrawParam> drawParam = aList->Current();
				if(!drawParam->IsParamHidden())
				{
					NFmiInfoData::Type dataType = drawParam->DataType();
					if(dataType == NFmiInfoData::kConceptualModelData || dataType == NFmiInfoData::kCapData)
					{
						mapViewDirty(mapViewDescTopIndex, false, false, true, false, false, false); // liataan mapView, mutta ei viel� t�ll� rivill� cachea (menisi kaikki rivit kerralla)
						mapViewDescTop->MapViewCache().MakeRowDirty(cacheRowNumber);// clean cache row
						needsToUpdateViews = true;
						if(dataType == NFmiInfoData::kCapData)
						{
							::getCapDataSystem().refreshCapData();
						}
					}
				}
			}
		}
		cacheRowNumber++;
	}
	return needsToUpdateViews;
}

void NFmiCombinedMapHandler::updateRowInLockedDescTops(unsigned int originalMapViewDescTopIndex)
{
	// Tehd��n ensin theOrigDescTopIndex:iin liittyv�n mapView p�ivitys, koska t�t� funktiota 
	// k�ytet��n monista rivin vaihtoon liittyvist� funktioista, on se hyv� saada yhteen paikkaan suoritetuksi yhteisesti.
	CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(originalMapViewDescTopIndex);
	auto* originalMapViewDescTop = getMapViewDescTop(originalMapViewDescTopIndex);
	// eli jos origIndex oli p��ikkuna (index = 0) tai apuikkuna oli lukittu p��ikkunaan, silloin tehd��n rivi p�ivityksi�
	if(originalMapViewDescTopIndex == 0 || originalMapViewDescTop->LockToMainMapViewRow())
	{
		auto mapRowStartingIndex = originalMapViewDescTop->MapRowStartingIndex();
		for(unsigned int currentMapViewDescTopIndex = 0; currentMapViewDescTopIndex < mapViewDescTops_.size(); currentMapViewDescTopIndex++)
		{
			auto* currentMapViewDescTop = getMapViewDescTop(currentMapViewDescTopIndex);
			if(currentMapViewDescTopIndex == 0 || currentMapViewDescTop->LockToMainMapViewRow())
			{
				CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(currentMapViewDescTopIndex);
				currentMapViewDescTop->MapRowStartingIndex(mapRowStartingIndex);
			}
		}
	}
}

const NFmiMetTime& NFmiCombinedMapHandler::currentTime(unsigned int mapViewDescTopIndex)
{
	if(mapViewDescTopIndex > CtrlViewUtils::kFmiMaxMapDescTopIndex)
		return getMapViewDescTop(0)->CurrentTime(); // erikois n�yt�ille palautetaan vain p��karttaikkunan valittu aika
	else
		return getMapViewDescTop(mapViewDescTopIndex)->CurrentTime();
}

bool NFmiCombinedMapHandler::isAnimationTimeBoxVisibleOverTimeControlView(unsigned int mapViewDescTopIndex)
{
	auto* mapViewDescTop = getMapViewDescTop(mapViewDescTopIndex);
	const auto& animationTimes = mapViewDescTop->AnimationDataRef().Times();
	auto timeControlTimes = mapViewDescTop->TimeControlViewTimes().ValidTimeBag();
	if(timeControlTimes)
	{
		// Peitt��k� animaatio ajat koko aikakontrolli-ikkunan
		if(animationTimes.IsInside(timeControlTimes->FirstTime()) && animationTimes.IsInside(timeControlTimes->LastTime()))
			return true;
		// Onko animaatio ajat kokonaan aikakontrolli-ikkunassa
		if(timeControlTimes->IsInside(animationTimes.FirstTime()) && timeControlTimes->IsInside(animationTimes.LastTime()))
			return true;
		// Onko animaation 1. aika on selke�sti aikakontrolli-ikkunan sis�ll�
		if(timeControlTimes->IsInside(animationTimes.FirstTime()) && timeControlTimes->FirstTime() < animationTimes.FirstTime() && timeControlTimes->LastTime() > animationTimes.FirstTime())
			return true;
		// Onko animaation viimeinen aika on selke�sti aikakontrolli-ikkunan sis�ll�
		if(timeControlTimes->IsInside(animationTimes.LastTime()) && timeControlTimes->FirstTime() < animationTimes.LastTime() && timeControlTimes->LastTime() > animationTimes.LastTime())
			return true;

		return false;
	}
	else
		return true; // time-control-view timebag missing => return true as inconclusive
}

NFmiMetTime NFmiCombinedMapHandler::calcCenterAnimationTimeBoxTime(unsigned int mapViewDescTopIndex)
{
	const auto& animationTimes = getMapViewDescTop(mapViewDescTopIndex)->AnimationDataRef().Times();
	auto totalDiffInMinutes = animationTimes.LastTime().DifferenceInMinutes(animationTimes.FirstTime());
	auto centerTime = animationTimes.FirstTime();
	centerTime.ChangeByMinutes(totalDiffInMinutes / 2);
	return centerTime;
}

// Move animation (light blue box) in the middle of time control view if following criterias are met:
// 1. View is in animation mode and time is changed by mouse wheel (fStayInsideAnimationTimes is on)
// 2. Animation time box is not visible.
// This action shows to the user that moving in time is restricted by animation time frame when using mouse wheel.
void NFmiCombinedMapHandler::setAnimationBoxToVisibleIfNecessary(unsigned int mapViewDescTopIndex, bool stayInsideAnimationTimes)
{
	if(doAnimationRestriction(mapViewDescTopIndex, stayInsideAnimationTimes))
	{
		if(!isAnimationTimeBoxVisibleOverTimeControlView(mapViewDescTopIndex))
		{
			auto newCenterTime = calcCenterAnimationTimeBoxTime(mapViewDescTopIndex);
			centerTimeControlView(mapViewDescTopIndex, newCenterTime, false);
		}
	}
}

// asettaa kaikki datat seuraavaan aikaan (jos mahdollista), riippuen aika-askeleesta
// k�y l�pi kaikki kartalla n�kyv�t datat ja asettaa ne oikeaan aikaan
bool NFmiCombinedMapHandler::setDataToNextTime(unsigned int mapViewDescTopIndex, bool stayInsideAnimationTimes)
{
	NFmiMetTime newTime;
	if(mapViewDescTopIndex == CtrlViewUtils::kDoAllMapViewDescTopIndex)
		return setDataToNextTimeForAllDescTops(stayInsideAnimationTimes);
	else
	{
		activeMapDescTopIndex(mapViewDescTopIndex);
		newTime = currentTime(mapViewDescTopIndex);
		auto usedTimeStep = getMapViewDescTop(mapViewDescTopIndex)->TimeControlTimeStepInMinutes();
		newTime.SetTimeStep(1);
		newTime.ChangeByMinutes(usedTimeStep);
		newTime = calcAnimationRestrictedTime(mapViewDescTopIndex, newTime, stayInsideAnimationTimes);
		currentTime(mapViewDescTopIndex, newTime, stayInsideAnimationTimes);
		setAnimationBoxToVisibleIfNecessary(mapViewDescTopIndex, stayInsideAnimationTimes);
	}
	updateTimeInLockedDescTops(newTime, mapViewDescTopIndex);
	return true;
}

bool NFmiCombinedMapHandler::setDataToNextTimeForAllDescTops(bool stayInsideAnimationTimes)
{
	for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
		setDataToNextTime(mapViewDescTopIndex, stayInsideAnimationTimes);
	return true;
}

// asettaa kaikki datat edelliseen aikaan (jos mahdollista), riippuen aikaaskeleesta
// k�y l�pi kaikki kartalla n�kyv�t datat ja asettaa ne oikeaan aikaan
bool NFmiCombinedMapHandler::setDataToPreviousTime(unsigned int mapViewDescTopIndex, bool stayInsideAnimationTimes)
{
	NFmiMetTime newTime;
	if(mapViewDescTopIndex == CtrlViewUtils::kDoAllMapViewDescTopIndex)
		return setDataToPreviousTimeForAllDescTops(stayInsideAnimationTimes);
	else
	{
		activeMapDescTopIndex(mapViewDescTopIndex);
		newTime = currentTime(mapViewDescTopIndex);
		auto usedTimeStep = getMapViewDescTop(mapViewDescTopIndex)->TimeControlTimeStepInMinutes();
		newTime.SetTimeStep(1);//usedTimeStep);
		newTime.ChangeByMinutes(-usedTimeStep);
		newTime = calcAnimationRestrictedTime(mapViewDescTopIndex, newTime, stayInsideAnimationTimes);
		currentTime(mapViewDescTopIndex, newTime, stayInsideAnimationTimes);
		setAnimationBoxToVisibleIfNecessary(mapViewDescTopIndex, stayInsideAnimationTimes);
	}
	updateTimeInLockedDescTops(newTime, mapViewDescTopIndex);
	return true;
}

bool NFmiCombinedMapHandler::setDataToPreviousTimeForAllDescTops(bool stayInsideAnimationTimes)
{
	for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
		setDataToPreviousTime(mapViewDescTopIndex, stayInsideAnimationTimes);
	return true;
}

unsigned int NFmiCombinedMapHandler::activeMapDescTopIndex()
{
	return activeMapDescTopIndex_;
}

// T�t� ei saa asettaa GenDocin ulkoa suoraan, t�m�n asetukset tapahtuvat t�m�n 
// luokan sis�ll�, joten t�m� ei ole julkinen metodi
void NFmiCombinedMapHandler::activeMapDescTopIndex(unsigned int mapViewDescTopIndex)
{
	if(mapViewDescTopIndex < mapViewDescTops_.size())
		activeMapDescTopIndex_ = mapViewDescTopIndex;
	else
		activeMapDescTopIndex_ = 0;
}

void NFmiCombinedMapHandler::timeControlViewTimes(unsigned int mapViewDescTopIndex, const NFmiTimeDescriptor& newTimeDescriptor)
{
	if(mapViewDescTopIndex == CtrlViewUtils::kDoAllMapViewDescTopIndex)
	{
		for(unsigned int currentMapViewDescTopIndex = 0; currentMapViewDescTopIndex < mapViewDescTops_.size(); currentMapViewDescTopIndex++)
			getMapViewDescTop(currentMapViewDescTopIndex)->TimeControlViewTimes(newTimeDescriptor);
	}
	else
	{
		getMapViewDescTop(mapViewDescTopIndex)->TimeControlViewTimes(newTimeDescriptor);
		ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(GetWantedMapViewIdFlag(mapViewDescTopIndex));
	}
}

const NFmiTimeDescriptor& NFmiCombinedMapHandler::timeControlViewTimes(unsigned int mapViewDescTopIndex)
{
	return 	getMapViewDescTop(mapViewDescTopIndex)->TimeControlViewTimes();
}

// funktio palauttaa oikean rivi numero. Karttan�ytt� systeemiss� on ik�v� virtuaali rivitys
// jolloin karttan�ytt�-luokka voi luulla olevansa rivill� 1 (= yli karttan�yt�ss� oleva rivi)
// vaikka onkin oikeasti vaikka rivill� 4. T�ll�in pit�� k�ytt�� apuna desctopin tietoja ett� 
// voidaan laskea oikea karttarivi.
// Oikeat karttarivit alkavat siis 1:st�.
unsigned int NFmiCombinedMapHandler::getRealRowNumber(unsigned int mapViewDescTopIndex, int rowIndex)
{
	if(mapViewDescTopIndex >= CtrlViewUtils::kFmiSoundingView)
		return rowIndex;
	else
		return rowIndex + getMapViewDescTop(mapViewDescTopIndex)->MapRowStartingIndex() - 1;
}

unsigned int NFmiCombinedMapHandler::getRelativeRowNumber(unsigned int mapViewDescTopIndex, int realRowIndex)
{
	if(realRowIndex >= CtrlViewUtils::kFmiTimeSerialView) // aikasarja rivi numerot ovat erikseen
		return realRowIndex;
	else
		return realRowIndex - getMapViewDescTop(mapViewDescTopIndex)->MapRowStartingIndex() + 1;
}

// muuta k�ytt�m��n DrawParamListWithRealRowNumber-funktiota
NFmiDrawParamList* NFmiCombinedMapHandler::getDrawParamList(unsigned int mapViewDescTopIndex, int rowIndex)
{
	if(mapViewDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
		return getCrossSectionViewDrawParamList(rowIndex);
	if(mapViewDescTopIndex == CtrlViewUtils::kFmiTimeSerialView || rowIndex >= CtrlViewUtils::kFmiTimeSerialView) // haetaan aikasarjaikkunan drawparamlistia CtrlViewUtils::kFmiTimeSerialView on suuri luku (99)
		return &getTimeSerialViewDrawParamList();
	else
		return getDrawParamListWithRealRowNumber(mapViewDescTopIndex, getRealRowNumber(mapViewDescTopIndex, rowIndex));
}

// T�m� on otettu k�ytt��n ,ett� voisi unohtaa tuon kamalan indeksi jupinan, mik� johtuu
// 'virtuaali' karttan�ytt�riveist�.
// Karttarivi indeksit alkavat 1:st�. 1. rivi on 1 ja 2. rivi on kaksi jne.
NFmiDrawParamList* NFmiCombinedMapHandler::getDrawParamListWithRealRowNumber(unsigned int mapViewDescTopIndex, int realRowIndex)
{
	if(mapViewDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
		return getCrossSectionViewDrawParamList(realRowIndex);
	if(realRowIndex >= CtrlViewUtils::kFmiTimeSerialView || mapViewDescTopIndex == CtrlViewUtils::kFmiTimeSerialView) // haetaan aikasarjaikkunan drawparamlistia CtrlViewUtils::kFmiTimeSerialView on suuri luku (99)
		return &getTimeSerialViewDrawParamList();
	NFmiMapViewDescTop* descTop = getMapViewDescTop(mapViewDescTopIndex);
	if(descTop)
	{
		int origListSize = static_cast<int>(descTop->DrawParamListVector()->NumberOfItems());
		if(realRowIndex <= origListSize)
			return descTop->DrawParamListVector()->Index(realRowIndex).CurrentPtr();
		else
			return 0;
	}
	else
		return 0;
}

std::shared_ptr<NFmiDrawParam> NFmiCombinedMapHandler::activeDrawParamFromActiveRow(unsigned int mapViewDescTopIndex)
{
	return activeDrawParamWithRealRowNumber(mapViewDescTopIndex, absoluteActiveViewRow(mapViewDescTopIndex));
}

// T�m� on otettu k�ytt��n ,ett� voisi unohtaa tuon kamalan indeksi jupinan, mik� johtuu
// 'virtuaali' karttan�ytt�riveist�.
// Karttarivi indeksit alkavat 1:st�. 1. rivi on 1 ja 2. rivi on kaksi jne.
std::shared_ptr<NFmiDrawParam> NFmiCombinedMapHandler::activeDrawParamWithRealRowNumber(unsigned int mapViewDescTopIndex, int realRowIndex)
{
	NFmiDrawParamList* drawParamList = getDrawParamListWithRealRowNumber(mapViewDescTopIndex, realRowIndex);
	if(drawParamList)
	{
		for(drawParamList->Reset(); drawParamList->Next(); )
			if(drawParamList->Current()->IsActive())
				return drawParamList->Current();
	}
	return std::shared_ptr<NFmiDrawParam>();
}

#ifndef UNIX
// asettaa zoomausalueen riippuvaiseksi yhden karttaikkunan x/y-suhteesta
void NFmiCombinedMapHandler::doAutoZoom(unsigned int mapViewDescTopIndex)
{
	if(::getApplicationWinRegistry().KeepMapAspectRatio())
	{
		auto* mapViewDescTop = getMapViewDescTop(mapViewDescTopIndex);
		std::shared_ptr<NFmiArea> oldArea = mapViewDescTop->MapHandler()->Area();
		if(oldArea)
		{
			double currentAreaAspectRatio = oldArea->WorldXYAspectRatio();
			double clientAspectRatio = mapViewDescTop->ClientViewXperYRatio();
			if(CtrlViewUtils::IsEqualEnough(clientAspectRatio, currentAreaAspectRatio, 0.000001) == false) // T�ss� pit�isi tutkia mik� on sellainen pieni arvo, jonka verran ratiot saavat heitt��, ettei t�t� kuitenkaan tarvitsisi laskea
			{ // pit�� muuttaa zoomattua areaa niin, ett� sen aspectratio vastaa ikkunan aspectratiota
				std::shared_ptr<NFmiArea> newArea(oldArea->CreateNewArea(mapViewDescTop->ClientViewXperYRatio(), kCenter, true));
				if(newArea)
				{
					setMapArea(mapViewDescTopIndex, newArea);
				}
			}
		}
	}
}
#endif

void NFmiCombinedMapHandler::makeViewRowDirtyActions(unsigned int mapViewDescTopIndex, int realRowIndex, NFmiDrawParamList* drawParamList)
{
	auto bitmapCacheRowIndex = realRowIndex - 1;
	try
	{
		getMapViewDescTop(mapViewDescTopIndex)->MapViewCache().MakeRowDirty(bitmapCacheRowIndex);
	}
	catch(...)
	{
	} // Jos jokin muu kuin karttan�ytt�, MapViewDescTop(mapViewDescTopIndex) -kutsu heitt�� poikkeuksen ja se on ok t�ss�

	if(drawParamList)
		::getMacroParamDataCache().update(mapViewDescTopIndex, realRowIndex, *drawParamList);
	mapViewDirty(mapViewDescTopIndex, false, false, true, false, false, true);
}

void NFmiCombinedMapHandler::drawParamSettingsChangedDirtyActions(unsigned int mapViewDescTopIndex, int realRowIndex, std::shared_ptr<NFmiDrawParam>& drawParam)
{
	auto bitmapCacheRowIndex = realRowIndex - 1;
	try
	{
		getMapViewDescTop(mapViewDescTopIndex)->MapViewCache().MakeRowDirty(bitmapCacheRowIndex);
	}
	catch(...)
	{
	} // Jos jokin muu kuin karttan�ytt�, MapViewDescTop(mapViewDescTopIndex) -kutsu heitt�� poikkeuksen ja se on ok t�ss�

	if(drawParam && drawParam->IsMacroParamCase(true))
	{
		::getMacroParamDataCache().clearMacroParamCache(mapViewDescTopIndex, realRowIndex, drawParam->InitFileName());
	}
	mapViewDirty(mapViewDescTopIndex, false, false, true, false, false, true);
}

#ifndef UNIX
// Liataan vain 1. n�kyv�t karttarivit niist� karttan�yt�ist�, miss� n�yt�-maski on p��ll�
void NFmiCombinedMapHandler::maskChangedDirtyActions()
{
	for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
	{
		auto desctop = getMapViewDescTop(mapViewDescTopIndex);
		if(desctop)
		{
			if(::getApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(mapViewDescTopIndex)->ShowMasksOnMap())
			{
				// Maskit ovat siis n�kyviss� 1. relatiivisella rivill�
				unsigned int firstVisibleRowIndex = 1;
				auto cleanedCacheRowIndex = getRealRowNumber(mapViewDescTopIndex, firstVisibleRowIndex) - 1;
				desctop->MapViewCache().MakeRowDirty(cleanedCacheRowIndex);
				mapViewDirty(mapViewDescTopIndex, false, false, true, false, false, false);
			}
		}
	}
}
#endif

// Next/Previous fixedDrawParam vaihdetaan parametri-laatikosta hiiren rullalla SHIFT nappi pohjassa.
bool NFmiCombinedMapHandler::changeParamSettingsToNextFixedDrawParam(unsigned int mapViewDescTopIndex, int realRowIndex, int paramIndex, bool gotoNext)
{
	NFmiDrawParamList* drawParamList = getDrawParamListWithRealRowNumber(mapViewDescTopIndex, realRowIndex);
	if(drawParamList)
	{
		if(drawParamList->Index(paramIndex))
		{
			std::shared_ptr<NFmiDrawParam> drawParam = drawParamList->Current();
			if(drawParam)
			{
				auto &fixedDrawParamSystem = ::getFixedDrawParamSystem();
				NFmiMenuItem menuItem(mapViewDescTopIndex, "", drawParam->Param(), kFmiModifyView, NFmiMetEditorTypes::View::kFmiTextView, &drawParam->Level(), drawParam->DataType(), paramIndex, drawParam->ViewMacroDrawParam());
				if(gotoNext)
					fixedDrawParamSystem.Next();
				else
					fixedDrawParamSystem.Previous();
				const std::shared_ptr<NFmiDrawParam>& fixedDrawParam = fixedDrawParamSystem.GetCurrentDrawParam();
				CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->ApplyFixeDrawParam(menuItem, getRelativeRowNumber(mapViewDescTopIndex, realRowIndex), fixedDrawParam);
				drawParamSettingsChangedDirtyActions(mapViewDescTopIndex, realRowIndex, drawParam);
				return true;
			}
		}
	}
	return false;
}

// nelj� tilaa:
// 0 = n�yt� aikakontrolliikkuna+teksti
// 1=vain aik.kont.ikkuna
// 2=�l� n�yt� kumpaakaan
// 3= n�yt� vain teksti
// palauttaa currentin tilan
int NFmiCombinedMapHandler::toggleShowTimeOnMapMode(unsigned int mapViewDescTopIndex)
{
	return getMapViewDescTop(mapViewDescTopIndex)->ToggleShowTimeOnMapMode();
}

// aikasarja ikkunat asetetaan samalla likaiseksi kuin karttan�ytt�kin (ainakin toistaiseksi)
bool NFmiCombinedMapHandler::timeSerialViewDirty()
{
	return timeSerialViewDirty_;
}

void NFmiCombinedMapHandler::timeSerialViewDirty(bool newValue)
{
	timeSerialViewDirty_ = newValue;
}

#ifndef UNIX
void NFmiCombinedMapHandler::setMapViewCacheSize(double theNewSizeInMB)
{
	::getApplicationWinRegistry().MapViewCacheMaxSizeInMB(theNewSizeInMB);

	for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
		mapViewDescTops_[mapViewDescTopIndex]->MapViewCache().MaxSizeMB(theNewSizeInMB);
}
#endif

// typeOfChange 0 = minutes, 1 = 6hrs, 2 = day, 3 = week, 4 = month, 5 = year
// direction vaihtoehdot ovat kBackward ja kForward eli ajan siirto suunta
// mapViewDescTopIndex eli karttan�yt�n indeksi, joita on 3 kpl
// amountOfChange eli kuinka monta p�iv��/viikkoa/kuukautta jne. liikutaan ajassa eteen/taakse (yleens� 1)
bool NFmiCombinedMapHandler::changeTime(int typeOfChange, FmiDirection direction, unsigned long mapViewDescTopIndex, double amountOfChange)
{
	NFmiMetTime selectedTime = currentTime(mapViewDescTopIndex);
	if(typeOfChange == 0) // minutes
	{
		selectedTime.ChangeByMinutes(boost::math::lround(direction == kForward ? amountOfChange : -amountOfChange));
	}
	else if(typeOfChange == 1) // 6 hrs
	{
		amountOfChange = 6;
		selectedTime.ChangeByHours(boost::math::lround(direction == kForward ? amountOfChange : -amountOfChange));
	}
	else if(typeOfChange == 2) // day
		selectedTime.ChangeByDays(boost::math::lround(direction == kForward ? amountOfChange : -amountOfChange));
	else if(typeOfChange == 3) // week
		selectedTime.ChangeByDays(boost::math::lround(direction == kForward ? 7 * amountOfChange : -7 * amountOfChange));
	else if(typeOfChange == 4) // month
	{
		NFmiTimePerioid period(0, 1, 0, 0, 0, 0);
		if(direction == kForward)
			selectedTime.NextMetTime(period);  
		else
			selectedTime.PreviousMetTime(period);
		// Previous/NextMetTime(period) metodeissa on bugi, ei osaa ottaa 
		// huomioon eri pituisia kuukausia, siksi pit�� tarkistaa ett� ei mene esim.
		// toukokuun 31. p�iv�st� kes�kuun 31. p�iv��n, jota ei ole olemassa.
		auto daysInMonth = NFmiTime::DaysInMonth(selectedTime.GetMonth(), selectedTime.GetYear());
		if(selectedTime.GetDay() > daysInMonth)
			selectedTime.SetDay(daysInMonth);
	}
	else if(typeOfChange == 5) // year
	{
		NFmiTimePerioid period(1,0,0,0,0,0);
		if(direction == kForward)
			selectedTime.NextMetTime(period);  // HUOM! n�iss� bugi, ei osaa ottaa huomioon eri pituisia kuukausia!!!!!
		else
			selectedTime.PreviousMetTime(period);  // HUOM! n�iss� bugi, ei osaa ottaa huomioon eri pituisia kuukausia!!!!!
	}

	centerTimeControlView(mapViewDescTopIndex, selectedTime, true); // t�m� asettaa ajan, my�s lukittuihin n�ytt�ihin ja likaa n�yt�n
	ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: Times changed by PageUp/Down keys");
	return true;
}

void NFmiCombinedMapHandler::makeMapViewRowDirty(int mapViewDescTopIndex, int viewRowIndex)
{
	// mapview cached alkaa 0:sta ja theViewRowIndex alkaa 1:st�
	getMapViewDescTop(mapViewDescTopIndex)->MapViewCache().MakeRowDirty(getRealRowNumber(mapViewDescTopIndex, viewRowIndex) - 1);
	mapViewDirty(mapViewDescTopIndex, false, false, true, false, false, true);
}

NFmiCombinedMapHandler::MapViewDescTopVector& NFmiCombinedMapHandler::getMapViewDescTops()
{
	return mapViewDescTops_;
}

const NFmiMetTime& NFmiCombinedMapHandler::activeMapTime()
{
	return currentTime(activeMapDescTopIndex_);
}

#ifndef UNIX
// Joitain arvoja s��det��n suoraan NFmiMapViewDescTop-luokan l�pi, joten n�it� pit�� p�ivitt�� takaisin rekistereihin
void NFmiCombinedMapHandler::storeMapViewSettingsToWinRegistry()
{
	for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
		getMapViewDescTop(mapViewDescTopIndex)->StoreToMapViewWinRegistry(*::getApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(mapViewDescTopIndex));
}
#endif

const std::unique_ptr<NFmiFastDrawParamList>& NFmiCombinedMapHandler::getModifiedPropertiesDrawParamList() const
{
	return modifiedPropertiesDrawParamList_;
}

// P�ivitt�� drawParam -parametriin modifiedPropertiesDrawParamList_:ista mahdollisesti l�ytyv�t asetukset.
void NFmiCombinedMapHandler::updateFromModifiedDrawParam(std::shared_ptr<NFmiDrawParam>& drawParam, bool groundData)
{
	if(drawParam)
	{
		if(!drawParam->ViewMacroDrawParam()) // ei p�ivitet� ominaisuuksia modified listasta, jos oli viewmacro-drawparam
		{
			if(modifiedPropertiesDrawParamList_->Find(drawParam, groundData)) // 1999.08.30/Marko
				drawParam->Init(modifiedPropertiesDrawParamList_->Current());
			else
			{ 
				// ei l�ytynyt 'lis�tt�v��' drawParamia listasta, joten lis�t��n t�ss� sen kopio modified-listaan (nyky��n ei lis�t� 
				// j�rjett�m�sti kaikkien datojen kaikki drawParam yhdistelmi�)
				std::shared_ptr<NFmiDrawParam> tmpDrawParam(new NFmiDrawParam(*drawParam.get()));
				modifiedPropertiesDrawParamList_->Add(tmpDrawParam, groundData);
			}
		}
	}
}

// Tein t�m�n version joka muuttaa modifiedPropertiesDrawParamList_:in otusta annetulla drawParamilla.
// Lis�sin t�h�n my�s annetun drawlist-rivin likaamisen.
void NFmiCombinedMapHandler::updateToModifiedDrawParam(unsigned int mapViewDescTopIndex, std::shared_ptr<NFmiDrawParam>& drawParam, int viewRowIndex)
{
	if(drawParam)
	{
		if(!drawParam->ViewMacroDrawParam()) // jos kyseess� oli viewmacro-drawparam, ei p�ivitet� modified-listalla olevaa drawparamia!
		{
			std::shared_ptr<NFmiFastQueryInfo> info = ::getInfoOrganizer().Info(drawParam, false, true);
			bool groundData = ::isGroundDataType(drawParam);
			if(modifiedPropertiesDrawParamList_->Find(drawParam, groundData))
				modifiedPropertiesDrawParamList_->Current()->Init(drawParam);
		}
	}
	NFmiDrawParamList* currentDrawParamList = getDrawParamList(mapViewDescTopIndex, viewRowIndex);
	if(currentDrawParamList)
	{
		currentDrawParamList->Dirty(true);
		drawParamSettingsChangedDirtyActions(mapViewDescTopIndex, getRealRowNumber(mapViewDescTopIndex, viewRowIndex), drawParam);
	}
	if(viewRowIndex == CtrlViewUtils::kFmiTimeSerialView)
		timeSerialViewDirty(true);
}

void NFmiCombinedMapHandler::makeWholeDesctopDirtyActions(unsigned int mapViewDescTopIndex, NFmiPtrList<NFmiDrawParamList>* drawParamListVector)
{
	mapViewDirty(mapViewDescTopIndex, false, true, true, false, false, true);
	if(drawParamListVector)
	{
		::getMacroParamDataCache().update(mapViewDescTopIndex, drawParamListVector);
	}
}

void NFmiCombinedMapHandler::hideShowAllMapViewParams(unsigned int mapViewDescTopIndex, bool hideAllObservations, bool showAllObservations, bool hideAllForecasts, bool showAllForecasts)
{
	NFmiPtrList<NFmiDrawParamList>::Iterator iter = getMapViewDescTop(mapViewDescTopIndex)->DrawParamListVector()->Start();
	for(; iter.Next();)
	{
		NFmiDrawParamList* aList = iter.CurrentPtr();
		if(aList)
		{
			for(aList->Reset(); aList->Next(); )
			{
				std::shared_ptr<NFmiDrawParam> drawParam = aList->Current();
				if(drawParam)
				{
					if(hideAllObservations)
					{
						if(isDrawParamObservation(drawParam))
							drawParam->HideParam(true);
					}
					else if(showAllObservations)
					{
						if(isDrawParamObservation(drawParam))
							drawParam->HideParam(false);
					}
					else if(hideAllForecasts)
					{
						if(isDrawParamForecast(drawParam))
							drawParam->HideParam(true);
					}
					else if(showAllForecasts)
					{
						if(isDrawParamForecast(drawParam))
							drawParam->HideParam(false);
					}

				}
			}
		}
	}
	checkAnimationLockedModeTimeBags(mapViewDescTopIndex, false); // kun parametrin n�kyvyytt� vaihdetaan, pit�� tehd� mahdollisesti animaatio moodin datan tarkistus
	makeWholeDesctopDirtyActions(mapViewDescTopIndex, nullptr);
}

std::shared_ptr<NFmiDrawParam> NFmiCombinedMapHandler::getDrawParamFromViewLists(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	NFmiDrawParamList* drawParamList = getDrawParamList(menuItem.MapViewDescTopIndex(), viewRowIndex);
	if(!drawParamList)
		return std::shared_ptr<NFmiDrawParam>();
	if(drawParamList->Index(menuItem.IndexInViewRow()))
		return drawParamList->Current();
	return std::shared_ptr<NFmiDrawParam>();
}

void NFmiCombinedMapHandler::setModelRunOffset(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	auto modifiedDrawParam = getDrawParamFromViewLists(menuItem, viewRowIndex);
	setModelRunOffset(modifiedDrawParam, menuItem.CommandType(), menuItem.MapViewDescTopIndex(), viewRowIndex);
}

bool NFmiCombinedMapHandler::setModelRunOffset(std::shared_ptr<NFmiDrawParam> &drawParam, FmiMenuCommandType command, unsigned int mapViewDescTopIndex, int viewRowIndex)
{
	if(drawParam)
	{
		auto oldModelRunIndex = drawParam->ModelRunIndex();
		if(command == kFmiModelRunOffsetPrevious)
		{
			drawParam->ModelOriginTime(NFmiMetTime::gMissingTime); // nollataan mahd. fiksattu origin aika
			drawParam->ModelRunIndex(drawParam->ModelRunIndex() - 1); // siirret��n offset edelliseen aikaan
		}
		else if(command == kFmiModelRunOffsetNext)
		{
			drawParam->ModelOriginTime(NFmiMetTime::gMissingTime); // nollataan mahd. fiksattu origin aika
			drawParam->ModelRunIndex(drawParam->ModelRunIndex() + 1); // siirret��n offset seuraavaan aikaan
			if(drawParam->ModelRunIndex() > 0)
				drawParam->ModelRunIndex(0);
		}

		if(oldModelRunIndex != drawParam->ModelRunIndex())
		{
			makeMapViewRowDirty(mapViewDescTopIndex, viewRowIndex);
			return true;
		}
	}
	return false;
}

void NFmiCombinedMapHandler::setModelRunOffsetForAllModelDataOnActiveRow(unsigned int mapViewDescTopIndex, FmiDirection direction)
{
	NFmiDrawParamList* activeDrawParamList = getDrawParamListWithRealRowNumber(mapViewDescTopIndex, absoluteActiveViewRow(mapViewDescTopIndex));
	if(activeDrawParamList)
	{
		bool needsUpdate = false;
		auto command = (direction == kBackward) ? kFmiModelRunOffsetPrevious : kFmiModelRunOffsetNext;
		auto relativeRowNumber = getRelativeRowNumber(mapViewDescTopIndex, absoluteActiveViewRow(mapViewDescTopIndex));
		for(activeDrawParamList->Reset(); activeDrawParamList->Next(); )
		{
			std::shared_ptr<NFmiDrawParam> drawParam = activeDrawParamList->Current();
			if(drawParam && drawParam->IsModelRunDataType())
			{
				auto oldModelRunIndex = drawParam->ModelRunIndex();
				if(setModelRunOffset(drawParam, command, mapViewDescTopIndex, relativeRowNumber))
				{
					needsUpdate = true;
				}
			}
		}

		if(needsUpdate)
		{
			ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: All active row model data model runs changed CTRL + SHIFT + left/right arrow key");
		}
	}
}

// T�m� on otettu k�ytt��n ,ett� voisi unohtaa tuon kamalan indeksi jupinan, mik� johtuu
// 'virtuaali' karttan�ytt�riveist�.
// Karttarivi indeksit alkavat 1:st�. 1. rivi on 1 ja 2. rivi on kaksi jne.
void NFmiCombinedMapHandler::removeAllViewsWithRealRowNumber(unsigned int mapViewDescTopIndex, int realRowIndex)
{
	NFmiDrawParamList* drawParamList = getDrawParamListWithRealRowNumber(mapViewDescTopIndex, realRowIndex);
	if(drawParamList)
	{
		drawParamList->Clear();
		makeViewRowDirtyActions(mapViewDescTopIndex, realRowIndex, drawParamList);
		activeEditedParameterMayHaveChangedViewUpdateFlagSetting(mapViewDescTopIndex);
	}
}

void NFmiCombinedMapHandler::activateView(const NFmiMenuItem& menuItem, int rowIndex)
{
	NFmiDrawParamList* drawParamList = getDrawParamList(menuItem.MapViewDescTopIndex(), rowIndex);
	if(drawParamList)
	{
		drawParamList->DeactivateAll();
		if(drawParamList->Index(menuItem.IndexInViewRow()))
		{
			drawParamList->Current()->Activate(true);
			drawParamList->Dirty(true);
#ifndef UNIX
			if(::getApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(menuItem.MapViewDescTopIndex())->ShowStationPlot())
			{
				// Jos karttan�yt�ll� n�ytet��n aktiivisen datan pisteet, pit�� t�ss� liata kaikki kuva cachet
				auto cacheRowIndex = getRealRowNumber(menuItem.MapViewDescTopIndex(), rowIndex) - 1;
				getMapViewDescTop(menuItem.MapViewDescTopIndex())->MapViewCache().MakeRowDirty(cacheRowIndex);
			}
#endif
			activeEditedParameterMayHaveChangedViewUpdateFlagSetting(menuItem.MapViewDescTopIndex());
			mapViewDirty(menuItem.MapViewDescTopIndex(), false, false, true, false, false, false);
		}
	}
}

static void DoSpecialDataInitializations(std::shared_ptr<NFmiDrawParam>& drawParam, bool normalParameterAdd, const NFmiMenuItem& menuItem)
{
	// N�ytt�makron latauksessa (normalParameterAdd = false) ei satelImagen alphaa saa en�� laiteta  default 80%:iin, koska joku on saattanut s��t�� sen muuksi.
	if(menuItem.DataType() == NFmiInfoData::kSatelData && normalParameterAdd)
	{
		drawParam->Alpha(80.f); // laitetaan satelliitti/kuva tyyppiselle datalle defaulttina 80% opaque eli pikkuisen l�pin�kyv�
	}
}

// T�m� on otettu k�ytt��n ,ett� voisi unohtaa tuon kamalan indeksi jupinan, mik� johtuu
// 'virtuaali' karttan�ytt�riveist�.
// Karttarivi indeksit alkavat 1:st�. 1. rivi on 1 ja 2. rivi on kaksi jne.
// macroParamInitFileName on sit� varten ett� jos viewmacrosta ladataan macroparam, t�h�n pit�� antaa init tiedoston nimi
// muuten macroparamin yhteydess� etsit��n menuitemista annettua nime�
// normalParameterAdd -parametrilla kerrotaan tuleeko normaali lis�ys vai erilaisista viewmakroista lis�ys. T�m�
// haluttiin erottaa viel� isViewMacroDrawParam:ista, jolla merkit��n vain drawParamin ViewMacroDrawParam -asetus.
void NFmiCombinedMapHandler::addViewWithRealRowNumber(bool normalParameterAdd, const NFmiMenuItem& menuItem, int realRowIndex, bool isViewMacroDrawParam)
{
	auto& infoOrganizer = ::getInfoOrganizer();
	std::shared_ptr<NFmiDrawParam> drawParam = infoOrganizer.CreateDrawParam(menuItem.DataIdent(), menuItem.Level(), menuItem.DataType());
	if(!drawParam)
		return; // HUOM!! Ei saisi menn� t�h�n!!!!!!!

	DoSpecialDataInitializations(drawParam, normalParameterAdd, menuItem);
	::setMacroParamDrawParamSettings(menuItem, drawParam);
	bool insertParamCase = (menuItem.CommandType() == kFmiInsertParamLayer);
	bool changeParamCase = (menuItem.CommandType() == kFmiChangeParam);

	if(!activeDrawParamWithRealRowNumber(menuItem.MapViewDescTopIndex(), realRowIndex))
	{
		drawParam->Activate(true);
		activeEditedParameterMayHaveChangedViewUpdateFlagSetting(menuItem.MapViewDescTopIndex());
	}
	drawParam->ViewMacroDrawParam(isViewMacroDrawParam); // asetetaan viewmacrodrawparam-flagin tila
	std::shared_ptr<NFmiFastQueryInfo> info = infoOrganizer.Info(drawParam, false, true);
	NFmiDrawParamList* drawParamList = getDrawParamListWithRealRowNumber(menuItem.MapViewDescTopIndex(), realRowIndex);
	if(drawParamList)
	{
		std::string logStartStr(insertParamCase ? "Insert to specific point into map view " : "Added to map view ");
		if(changeParamCase)
			logStartStr = "Changed parameter to selected ";
		logParameterAction(logStartStr, menuItem, drawParam, info);

		// ChangeParam tapauksessa vaihdettava parametri on jo poistettu listasta, ja siksi t�ss� vain uusi samaan paikkaan
		if(insertParamCase || changeParamCase)
			drawParamList->Add(drawParam, menuItem.IndexInViewRow());
		else if(!normalParameterAdd)
		{
			// jos n�ytt� macrosta kyse, pit�� parametri laittaa tarkalleen siihen mik� rivi oli
			// kyseess� (eli listan per��n j�rjestyksess�). T�m� sen takia ett� satel-kanavat heitet��n aina pohjalle ja
			// n�ytt�makroissa kaksi satelliitti kuvaa samalla rivill� aiheutti ongelmia.
			drawParamList->Add(drawParam, drawParamList->NumberOfItems() + 1);
		}
		else
			drawParamList->Add(drawParam); // laittaa parametrit listan per��n, paitsi satel-kuvat laitetaan keulille (n�in satelkuva ei peit� mahdollisia muita parametreja alleen)
	}

	bool groundData = ::isGroundDataType(drawParam);
	updateFromModifiedDrawParam(drawParam, groundData);
	drawParamSettingsChangedDirtyActions(menuItem.MapViewDescTopIndex(), realRowIndex, drawParam);
}

void NFmiCombinedMapHandler::logParameterAction(const std::string &parameterActionStart, const NFmiMenuItem& menuItem, const std::shared_ptr<NFmiDrawParam> &drawParam, std::shared_ptr<NFmiFastQueryInfo> &info)
{
	std::string logStr = parameterActionStart;
	if(NFmiDrawParam::IsMacroParamCase(drawParam->DataType()))
	{
		logStr += "macro parameter '";
		logStr += drawParam->ParameterAbbreviation();
		logStr += "'";
	}
	else if(info)
		logStr += getSelectedParamInfoString(info, false);
	else 
	{
		bool satelDataCase = drawParam->DataType() == NFmiInfoData::kSatelData;
		// Esim. satelliitti jutuissa tai jos ei l�ydy ladattavaa dataa, ei ole infoa, joten otetaan halutut tiedot muualta
		if(satelDataCase)
			logStr += menuItem.DataIdent().GetParamName();
		else
			logStr += getSelectedParamInfoString(&drawParam->Param(), &drawParam->Level());
	}
	logMessage(logStr, CatLog::Severity::Debug, CatLog::Category::Visualization);
}

void NFmiCombinedMapHandler::toggleShowDifferenceToOriginalData(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	auto modifiedDrawParam = getDrawParamFromViewLists(menuItem, viewRowIndex);
	if(modifiedDrawParam)
	{
		modifiedDrawParam->ShowDifferenceToOriginalData(!(modifiedDrawParam->ShowDifferenceToOriginalData()));
		updateToModifiedDrawParam(menuItem.MapViewDescTopIndex(), modifiedDrawParam, viewRowIndex);
	}
}


void NFmiCombinedMapHandler::addView(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	// lasketaan todellinen rivinumero (johtuu karttan�yt�n virtuaali riveist�)
	addViewWithRealRowNumber(true, menuItem, getRealRowNumber(menuItem.MapViewDescTopIndex(), viewRowIndex), false);

	// lis��n t�m�n CheckAnimationLockedModeTimeBags -kutsun vain perus AddView-metodin yhteyteen, mutta en esim.
	// AddViewWithRealRowNumber -metodin yhteyteen, ett� homma ei mene pelk�ksi tarkasteluksi.
	// AddViewWithRealRowNumber -metodia k�ytet��n varsin laajasti ja tarkasteluja tulisi tehty� liikaa.
	if(CtrlViewFastInfoFunctions::IsObservationLockModeDataType(menuItem.DataType()) || menuItem.DataType() == NFmiInfoData::kSatelData)
		checkAnimationLockedModeTimeBags(menuItem.MapViewDescTopIndex(), false); // kun parametrin n�kyvyytt� vaihdetaan, pit�� tehd� mahdollisesti animaatio moodin datan tarkistus
}

void NFmiCombinedMapHandler::addCrossSectionView(const NFmiMenuItem& menuItem, int viewRowIndex, bool treatAsViewMacro)
{
	std::shared_ptr<NFmiDrawParam> drawParam;
	auto& infoOrganizer = ::getInfoOrganizer();
	if(menuItem.DataType() == NFmiInfoData::kCrossSectionMacroParam)
		drawParam = infoOrganizer.CreateDrawParam(menuItem.DataIdent(), 0, menuItem.DataType());
	else
		drawParam = infoOrganizer.CreateCrossSectionDrawParam(menuItem.DataIdent(), menuItem.DataType());
	if(!drawParam)
		return; // HUOM!! Ei saisi menn� t�h�n!!!!!!!
	::setMacroParamDrawParamSettings(menuItem, drawParam);
	drawParam->ViewMacroDrawParam(treatAsViewMacro);

	std::shared_ptr<NFmiFastQueryInfo> info = infoOrganizer.Info(drawParam, true, true);
	auto* crossSectionViewDrawParamList = getCrossSectionViewDrawParamList(viewRowIndex);
	if(crossSectionViewDrawParamList)
	{
		bool changeParamCase = (menuItem.CommandType() == kFmiChangeParam);
		std::string logStartStr(changeParamCase ? "Changing selected param to " : "Adding to crosssection-view ");
		logParameterAction(logStartStr, menuItem, drawParam, info);
		if(changeParamCase)
			crossSectionViewDrawParamList->Add(drawParam, menuItem.IndexInViewRow());
		else
			crossSectionViewDrawParamList->Add(drawParam);
		crossSectionViewDrawParamList->ActivateOnlyOne(); // varmistaa, ett� yksi ja vain yksi paramtri listassa on aktiivinen
	}
	drawParamSettingsChangedDirtyActions(menuItem.MapViewDescTopIndex(), getRealRowNumber(menuItem.MapViewDescTopIndex(), viewRowIndex), drawParam);
}

void NFmiCombinedMapHandler::changeParamLevel(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	auto* drawParamList = getDrawParamList(menuItem.MapViewDescTopIndex(), viewRowIndex);
	if(drawParamList)
	{
		auto changedParamIndex = menuItem.IndexInViewRow();
		if(drawParamList->Index(changedParamIndex))
		{
			if(drawParamList->Remove())
			{
				if(menuItem.MapViewDescTopIndex() == CtrlViewUtils::kFmiCrossSectionView)
					addCrossSectionView(menuItem, viewRowIndex, false);
				else
					addViewWithRealRowNumber(true, menuItem, getRealRowNumber(menuItem.MapViewDescTopIndex(), viewRowIndex), false);
			}
		}
	}
}

void NFmiCombinedMapHandler::removeView(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	auto mapViewDesctopIndex = menuItem.MapViewDescTopIndex();
	NFmiDrawParamList* drawParamList = getDrawParamList(mapViewDesctopIndex, viewRowIndex);
	if(drawParamList)
	{
		if(drawParamList->Index(menuItem.IndexInViewRow()))
		{
			NFmiInfoData::Type dataType = drawParamList->Current()->DataType();
			drawParamList->Remove();
			if(CtrlViewFastInfoFunctions::IsObservationLockModeDataType(dataType))
				checkAnimationLockedModeTimeBags(mapViewDesctopIndex, false); // kun parametrin n�kyvyytt� vaihdetaan, pit�� tehd� mahdollisesti animaatio moodin datan tarkistus

			std::shared_ptr<NFmiDrawParam> emptyDrawParam;
			drawParamSettingsChangedDirtyActions(mapViewDesctopIndex, getRealRowNumber(mapViewDesctopIndex, viewRowIndex), emptyDrawParam);
		}
		if(drawParamList->NumberOfItems() && (!activeDrawParamWithRealRowNumber(mapViewDesctopIndex, getRealRowNumber(mapViewDesctopIndex, viewRowIndex))))
		{
			// Aktiivinen parametri poistettiin, asetetaan 1. listasta aktiiviseksi
			for(drawParamList->Reset(); drawParamList->Next(); )
			{
				drawParamList->Current()->Activate(true);
				break;
			}
			activeEditedParameterMayHaveChangedViewUpdateFlagSetting(mapViewDesctopIndex);
		}
	}
}

void NFmiCombinedMapHandler::removeAllViews(unsigned int mapViewDescTopIndex, int viewRowIndex)
{
	NFmiDrawParamList* drawParamList = getDrawParamList(mapViewDescTopIndex, viewRowIndex);
	if(drawParamList)
	{
		drawParamList->Clear();
		makeViewRowDirtyActions(mapViewDescTopIndex, getRealRowNumber(mapViewDescTopIndex, viewRowIndex), drawParamList);
		activeEditedParameterMayHaveChangedViewUpdateFlagSetting(mapViewDescTopIndex);
	}
}

void NFmiCombinedMapHandler::removeAllCrossSectionViews(int viewRowIndex)
{
	auto* crossSectionViewDrawParamList = getCrossSectionViewDrawParamList(viewRowIndex);
	if(crossSectionViewDrawParamList)
	{
		crossSectionViewDrawParamList->Clear();
		makeViewRowDirtyActions(CtrlViewUtils::kFmiCrossSectionView, viewRowIndex, crossSectionViewDrawParamList);
	}
}

NFmiDrawParamList* NFmiCombinedMapHandler::getWantedDrawParamList(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams)
{
	if(useCrossSectionParams)
		return getCrossSectionViewDrawParamList(viewRowIndex);
	else
		return getDrawParamList(menuItem.MapViewDescTopIndex(), viewRowIndex);
}

NFmiInfoData::Type getFinalDataType(std::shared_ptr<NFmiDrawParam>& drawParam, const NFmiProducer& givenProducer, bool useCrossSectionParams, bool fGroundData)
{
	auto& infoOrganizer = ::getInfoOrganizer();
	NFmiInfoData::Type finalDataType = drawParam->DataType();
	NFmiInfoData::Type foundBackupDataType = NFmiInfoData::kNoDataType;
	if(useCrossSectionParams)
		return ::checkCrossSectionLevelData(finalDataType, givenProducer);

	auto infos = infoOrganizer.GetInfos(givenProducer.GetIdent());
	if(!infos.empty())
	{
		for(const auto& info : infos)
		{
			if(info->Param(static_cast<FmiParameterName>(drawParam->Param().GetParamIdent())))
			{
				if(!fGroundData && info->SizeLevels() > 1)
				{
					if(info->LevelType() == drawParam->Level().LevelType())
					{
						auto actualInfoDataType = info->DataType();
						if(actualInfoDataType == finalDataType)
							return actualInfoDataType;
						else
							foundBackupDataType = actualInfoDataType;
					}
				}
				else if(fGroundData && info->SizeLevels() == 1)
				{
					auto actualInfoDataType = info->DataType();
					if(actualInfoDataType == finalDataType)
						return actualInfoDataType;
					else
						foundBackupDataType = actualInfoDataType;
				}
			}
		}
	}

	std::shared_ptr<NFmiFastQueryInfo> editedData = ::getEditedInfo();
	if(editedData && givenProducer == *(editedData->Producer()))
		return NFmiInfoData::kEditable;
	std::shared_ptr<NFmiFastQueryInfo> operationalData = infoOrganizer.FindInfo(NFmiInfoData::kKepaData);
	if(operationalData && givenProducer == *(operationalData->Producer()))
		return NFmiInfoData::kKepaData;
	std::shared_ptr<NFmiFastQueryInfo> helpData = infoOrganizer.FindInfo(NFmiInfoData::kEditingHelpData, 0);
	if(helpData && givenProducer == *(helpData->Producer()))
		return NFmiInfoData::kEditingHelpData;

	if(foundBackupDataType != NFmiInfoData::kNoDataType)
		return foundBackupDataType;

	return finalDataType;
}

void NFmiCombinedMapHandler::changeAllProducersInMapRow(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams)
{
	NFmiDrawParamList* wantedDrawParamList = getWantedDrawParamList(menuItem, viewRowIndex, useCrossSectionParams);
	if(wantedDrawParamList)
	{
		auto& infoOrganizer = ::getInfoOrganizer();
		const NFmiProducer& givenProducer = *(menuItem.DataIdent().GetProducer());
		for(wantedDrawParamList->Reset(); wantedDrawParamList->Next(); )
		{
			auto drawParam = wantedDrawParamList->Current();
			auto dataType = drawParam->DataType();
			if(dataType != NFmiInfoData::kSatelData && dataType != NFmiInfoData::kMacroParam && dataType != NFmiInfoData::kQ3MacroParam)
			{
				bool groundData = ::isGroundDataType(drawParam);
				NFmiInfoData::Type finalDataType = ::getFinalDataType(drawParam, givenProducer, useCrossSectionParams, groundData); // pit�� p��tt�� viel� muutentun tuottajan datatyyppi
				// pit�� hakea FindInfo:lla tuottajan mukaan dataa, josta saadaan oikea tuottaja (nimineen kaikkineen)
				std::shared_ptr<NFmiFastQueryInfo> info = infoOrganizer.FindInfo(finalDataType, givenProducer, groundData);
				if(info)
					drawParam->Param().SetProducers(*info->Producer()); // pit�� laittaa tuottaja datasta, koska tuottajan nimikin ratkaisee, kun haetaan dataa
				else // jos ei l�ytynyt dataa, t�m� luultavasti menee pieleen, mutta laitetaan kuitenkin tuottaja kohdalleen
					drawParam->Param().SetProducers(*(menuItem.DataIdent().GetProducer()));
				drawParam->DataType(finalDataType);
			}
		}
		wantedDrawParamList->Dirty(true);
		auto realRowIndex = getRealRowNumber(menuItem.MapViewDescTopIndex(), viewRowIndex);
		makeViewRowDirtyActions(menuItem.MapViewDescTopIndex(), realRowIndex, wantedDrawParamList);
	}
}

void NFmiCombinedMapHandler::changeAllDataTypesInMapRow(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams)
{
	NFmiDrawParamList* wantedDrawParamList = getWantedDrawParamList(menuItem, viewRowIndex, useCrossSectionParams);
	if(wantedDrawParamList)
	{
		NFmiInfoData::Type wantedType = static_cast<NFmiInfoData::Type>(menuItem.Parameter());
		for(wantedDrawParamList->Reset(); wantedDrawParamList->Next(); )
		{
			wantedDrawParamList->Current()->DataType(wantedType);
		}
		wantedDrawParamList->Dirty(true);
		makeViewRowDirtyActions(menuItem.MapViewDescTopIndex(), viewRowIndex, wantedDrawParamList);
	}
}

void NFmiCombinedMapHandler::copyDrawParamOptions(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams)
{
	NFmiDrawParamList* wantedDrawParamList = getWantedDrawParamList(menuItem, viewRowIndex, useCrossSectionParams);
	if(wantedDrawParamList && wantedDrawParamList->Index(menuItem.IndexInViewRow()))
	{
		copyPasteDrawParam_->Init(wantedDrawParamList->Current(), true);
		// Pastettavan drawParamin pit�� aina olla n�kyv�, muuten tulee h�mmennyst� k�ytt�jiss�!
		copyPasteDrawParam_->HideParam(false);
		copyPasteDrawParamAvailableYet_ = true;
	}
}

void NFmiCombinedMapHandler::pasteDrawParamOptions(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams)
{
	NFmiDrawParamList* wantedDrawParamList = getWantedDrawParamList(menuItem, viewRowIndex, useCrossSectionParams);
	if(wantedDrawParamList && wantedDrawParamList->Index(menuItem.IndexInViewRow()))
	{
		std::shared_ptr<NFmiDrawParam> drawParam = wantedDrawParamList->Current();
		drawParam->Init(copyPasteDrawParam_.get(), true);
		wantedDrawParamList->Dirty(true);
		if(useCrossSectionParams == false)
			updateToModifiedDrawParam(menuItem.MapViewDescTopIndex(), drawParam, viewRowIndex);
	}
}

std::shared_ptr<NFmiDrawParam> NFmiCombinedMapHandler::getUsedMapViewDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	NFmiDrawParamList* drawParamList = getDrawParamList(menuItem.MapViewDescTopIndex(), viewRowIndex);
	if(drawParamList && drawParamList->Index(menuItem.IndexInViewRow()))
		return drawParamList->Current();
	else
		return std::shared_ptr<NFmiDrawParam>();
}

void NFmiCombinedMapHandler::copyMapViewDescTopParams(unsigned int mapViewDescTopIndex)
{
	NFmiPtrList<NFmiDrawParamList>* copiedDrawParamsList = getMapViewDescTop(mapViewDescTopIndex)->DrawParamListVector();
	if(copiedDrawParamsList && copyPasteDrawParamListVector_)
	{
		copyPasteDrawParamListVectorUsedYet_ = true;
		CombinedMapHandlerInterface::copyDrawParamsList(copiedDrawParamsList, copyPasteDrawParamListVector_.get());
	}
}

void NFmiCombinedMapHandler::pasteMapViewDescTopParams(unsigned int mapViewDescTopIndex)
{
	NFmiPtrList<NFmiDrawParamList>* copiedDrawParamsList = getMapViewDescTop(mapViewDescTopIndex)->DrawParamListVector();
	if(copiedDrawParamsList && copyPasteDrawParamListVector_)
	{
		CombinedMapHandlerInterface::copyDrawParamsList(copyPasteDrawParamListVector_.get(), copiedDrawParamsList);
		makeWholeDesctopDirtyActions(mapViewDescTopIndex, copiedDrawParamsList);
		activeEditedParameterMayHaveChangedViewUpdateFlagSetting(mapViewDescTopIndex);
		ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: Paste all copyed parameters on this map view");
	}
}

NFmiDrawParamList* NFmiCombinedMapHandler::getCrossSectionViewDrawParamList(int viewRowIndex)
{
	if(crossSectionDrawParamListVector_)
	{
		NFmiPtrList<NFmiDrawParamList>::Iterator it = crossSectionDrawParamListVector_->Index(viewRowIndex);
		return it.CurrentPtr();
	}
	return nullptr;
}

#ifndef UNIX
void NFmiCombinedMapHandler::modifyCrossSectionDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	std::shared_ptr<NFmiDrawParam> modifiedDrawParam = getCrosssectionDrawParamFromViewLists(menuItem, viewRowIndex);
	if(modifiedDrawParam)
	{
		auto mapViewDescTopIndex = menuItem.MapViewDescTopIndex();
		CWnd* parentView = ApplicationInterface::GetApplicationInterfaceImplementation()->GetView(mapViewDescTopIndex);
		auto realRowNumber = getRealRowNumber(mapViewDescTopIndex, viewRowIndex);
		CFmiModifyDrawParamDlg dlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), modifiedDrawParam, ::getMacroPathSettings().DrawParamPath(), false, true, mapViewDescTopIndex, realRowNumber, parentView);
		if(dlg.DoModal() == IDOK)
		{
			drawParamSettingsChangedDirtyActions(mapViewDescTopIndex, realRowNumber, modifiedDrawParam);
		}
	}
}
#endif

void NFmiCombinedMapHandler::activateCrossSectionParam(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	NFmiDrawParamList* drawParamList = getCrossSectionViewDrawParamList(viewRowIndex);
	if(drawParamList)
	{
		drawParamList->DeactivateAll();
		if(drawParamList->Index(menuItem.IndexInViewRow()))
		{
			drawParamList->Current()->Activate(true);
			drawParamList->Dirty(true);
		}
	}
}

void NFmiCombinedMapHandler::modifyView(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	std::shared_ptr<NFmiDrawParam> drawParam = getUsedMapViewDrawParam(menuItem, viewRowIndex);
	if(drawParam)
	{
		NFmiMetEditorTypes::View viewType = menuItem.ViewType();
		drawParam->ViewType(viewType);
		std::shared_ptr<NFmiFastQueryInfo> info = ::getInfoOrganizer().Info(drawParam, false, true);
		if(info && info->IsGrid() == false)
			drawParam->StationDataViewType(viewType);
		else
			drawParam->GridDataPresentationStyle(viewType);

		updateToModifiedDrawParam(menuItem.MapViewDescTopIndex(), drawParam, viewRowIndex);
	}
}

void NFmiCombinedMapHandler::toggleShowLegendState(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	std::shared_ptr<NFmiDrawParam> drawParam = getUsedMapViewDrawParam(menuItem, viewRowIndex);
	if(drawParam)
	{
		drawParam->ShowColorLegend(!drawParam->ShowColorLegend());
		updateToModifiedDrawParam(menuItem.MapViewDescTopIndex(), drawParam, viewRowIndex);
	}
}

void NFmiCombinedMapHandler::swapViewRows(const NFmiMenuItem& menuItem)
{
	auto viewIndex = menuItem.MapViewDescTopIndex();
	int realRowNumber1 = menuItem.IndexInViewRow();
	auto drawParamList1 = getDrawParamListWithRealRowNumber(viewIndex, realRowNumber1);
	int realRowNumber2 = static_cast<int>(menuItem.ExtraParam());
	auto drawParamList2 = getDrawParamListWithRealRowNumber(viewIndex, realRowNumber2);
	if(drawParamList1 && drawParamList2)
	{
		drawParamList1->Swap(drawParamList2);
		// HUOM! tosi rivi numerosta pit�� v�hent�� 1, kun manipuloidaan bitmap cache rivej�!!!
		getMapViewDescTop(viewIndex)->MapViewCache().SwapRows(realRowNumber1 - 1, realRowNumber2 - 1);
		getMacroParamDataCache().swapMacroParamCacheRows(viewIndex, realRowNumber1, realRowNumber2);
		activeEditedParameterMayHaveChangedViewUpdateFlagSetting(menuItem.MapViewDescTopIndex());
		mapViewDirty(viewIndex, false, false, true, false, false, true);
	}
}

void NFmiCombinedMapHandler::saveDrawParamSettings(std::shared_ptr<NFmiDrawParam> &drawParam)
{
	if(drawParam)
	{
		const auto& initFileName = drawParam->InitFileName();
		if(drawParam->ViewMacroDrawParam())
		{
			std::string msgStr = ::GetDictionaryString("the given DrawParam was in a viewmacro, cannot store directly, you must save the changes made to the viewMacro");
			std::string dialogTitleStr = ::GetDictionaryString("Cannot store the drawParam");
			logAndWarnUser(msgStr, dialogTitleStr, CatLog::Severity::Error, CatLog::Category::Macro, false);
		}
		else if(drawParam->StoreData(initFileName))
		{
			::getMacroParamSystem()->ReloadDrawParamFromFile(initFileName);
		}
		else
		{
			std::string msgStr = ::GetDictionaryString("Unknown error while trying to store drawParam settings to file: ");
			auto initFileStr = initFileName.empty() ? drawParam->ParameterAbbreviation() : initFileName;
			msgStr += initFileStr;
			std::string dialogTitleStr = ::GetDictionaryString("Error storing the drawParam");
			logAndWarnUser(msgStr, dialogTitleStr, CatLog::Severity::Error, CatLog::Category::Macro, false);
		}
	}
	else
	{
		std::string msgStr = ::GetDictionaryString("Cannot store the given drawParam, it was empty, error in application logic?");
		std::string dialogTitleStr = ::GetDictionaryString("Error storing the drawParam");
		logAndWarnUser(msgStr, dialogTitleStr, CatLog::Severity::Error, CatLog::Category::Macro, false);
	}
}


void NFmiCombinedMapHandler::saveDrawParamSettings(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	auto drawParam = getUsedMapViewDrawParam(menuItem, viewRowIndex);
	saveDrawParamSettings(drawParam);
}

void NFmiCombinedMapHandler::forceStationViewRowUpdate(unsigned int mapViewDescTopIndex, unsigned int theRealRowIndex)
{
	NFmiDrawParamList* drawParamList = getDrawParamListWithRealRowNumber(mapViewDescTopIndex, theRealRowIndex);
	if(drawParamList)
	{
		// Liataan haluttu drawParamList, jotta k�yt�ss� karttan�yt�ss� ollut stationView p�ivittyy oikein tarvittaessa jos esim. isoline piirto vaihtuu teksti esitykseen.
		drawParamList->Dirty(true);
	}
}

void NFmiCombinedMapHandler::reloadDrawParamSettings(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	std::shared_ptr<NFmiDrawParam> drawParam = getUsedMapViewDrawParam(menuItem, viewRowIndex);
	if(drawParam)
	{
		// Pit�� ladata erikseen originaali drawParam asetukset omaan olioon ja sen avulla initialisoida k�yt�ss� olevan asetukset
		NFmiDrawParam origDrawParam;
		origDrawParam.Init(drawParam->InitFileName());
		drawParam->Init(&origDrawParam, true);
		drawParamSettingsChangedDirtyActions(menuItem.MapViewDescTopIndex(), getRealRowNumber(menuItem.MapViewDescTopIndex(), viewRowIndex), drawParam);
	}
}

void NFmiCombinedMapHandler::applyFixeDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	const std::shared_ptr<NFmiDrawParam>& fixedDrawParam = ::getFixedDrawParamSystem().GetDrawParam(menuItem.MacroParamInitName());
	applyFixeDrawParam(menuItem, viewRowIndex, fixedDrawParam);
}

void NFmiCombinedMapHandler::applyFixeDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex, const std::shared_ptr<NFmiDrawParam>& fixedDrawParam)
{
	std::shared_ptr<NFmiDrawParam> drawParam = getUsedMapViewDrawParam(menuItem, viewRowIndex);
	if(drawParam && fixedDrawParam)
	{
		drawParam->Init(fixedDrawParam.get(), true);
		updateToModifiedDrawParam(menuItem.MapViewDescTopIndex(), drawParam, viewRowIndex);
	}
}

std::shared_ptr<NFmiDrawParam> NFmiCombinedMapHandler::getCrosssectionDrawParamFromViewLists(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	NFmiDrawParamList* drawParamList = getCrossSectionViewDrawParamList(viewRowIndex);
	if(!drawParamList)
		return std::shared_ptr<NFmiDrawParam>();
	if(drawParamList->Index(menuItem.IndexInViewRow()))
		return drawParamList->Current();
	return std::shared_ptr<NFmiDrawParam>();
}

void NFmiCombinedMapHandler::removeCrosssectionDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	NFmiDrawParamList* drawParamList = getCrossSectionViewDrawParamList(viewRowIndex);
	if(drawParamList && drawParamList->Index(menuItem.IndexInViewRow()))
	{
		drawParamList->Remove();
		std::shared_ptr<NFmiDrawParam> emptyDrawParam;
		drawParamSettingsChangedDirtyActions(menuItem.MapViewDescTopIndex(), getRealRowNumber(menuItem.MapViewDescTopIndex(), viewRowIndex), emptyDrawParam);
	}
}

void NFmiCombinedMapHandler::hideView(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	auto mapViewDesctopIndex = menuItem.MapViewDescTopIndex();
	NFmiDrawParamList* drawParamList = getDrawParamList(mapViewDesctopIndex, viewRowIndex);
	if(!drawParamList)
		return;
	if(drawParamList->Index(menuItem.IndexInViewRow()))
	{
		drawParamList->Current()->HideParam(true);
		if(drawParamList->Current()->IsActive())
		{// deaktivoidaan piilotettu n�ytt�parametri (jos oli aktiivinen), ettei pensselill� yritet� sutia sit� vahingossa
			drawParamList->Current()->Activate(false);
			ActivateFirstNonHiddenViewParam(drawParamList);
		}
		drawParamList->Dirty(true);
		checkAnimationLockedModeTimeBags(mapViewDesctopIndex, false); // kun parametrin n�kyvyytt� vaihdetaan, pit�� tehd� mahdollisesti animaatio moodin datan tarkistus
		getMapViewDescTop(mapViewDesctopIndex)->MapViewCache().MakeRowDirty(getRealRowNumber(mapViewDesctopIndex, viewRowIndex));
		mapViewDirty(mapViewDesctopIndex, false, false, true, false, false, false);
	}
	auto currentDrawParam = drawParamList->Current();
	updateToModifiedDrawParam(mapViewDesctopIndex, currentDrawParam, viewRowIndex);
}

void NFmiCombinedMapHandler::showView(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	auto mapViewDesctopIndex = menuItem.MapViewDescTopIndex();
	NFmiDrawParamList* drawParamList = getDrawParamList(mapViewDesctopIndex, viewRowIndex);
	if(!drawParamList)
		return;
	if(drawParamList->Index(menuItem.IndexInViewRow()))
	{
		drawParamList->Current()->HideParam(false);
		drawParamList->Dirty(true);
		checkAnimationLockedModeTimeBags(mapViewDesctopIndex, false); // kun parametrin n�kyvyytt� vaihdetaan, pit�� tehd� mahdollisesti animaatio moodin datan tarkistus
		getMapViewDescTop(mapViewDesctopIndex)->MapViewCache().MakeRowDirty(getRealRowNumber(mapViewDesctopIndex, viewRowIndex));
		mapViewDirty(mapViewDesctopIndex, false, false, true, false, false, false);
	}
	auto currentDrawParam = drawParamList->Current();
	updateToModifiedDrawParam(mapViewDesctopIndex, currentDrawParam, viewRowIndex);
}

NFmiPtrList<NFmiDrawParamList>& NFmiCombinedMapHandler::getCrossSectionDrawParamListVector()
{
	return *crossSectionDrawParamListVector_;
}

NFmiDrawParamList& NFmiCombinedMapHandler::getTimeSerialViewDrawParamList()
{
	return *timeSerialViewDrawParamList_;
}

NFmiCombinedMapHandler::SideParametersIterator NFmiCombinedMapHandler::getTimeSerialViewSideParameterIterator(int viewRowIndex)
{
	auto actualListIndex = viewRowIndex - 1;
	if(actualListIndex >= 0 && actualListIndex < timeSerialViewSideParameters_.size())
	{
		auto iter = timeSerialViewSideParameters_.begin();
		std::advance(iter, actualListIndex);
		return iter;
	}

	return timeSerialViewSideParameters_.end();
}

// viewRowIndex parametri on 1:st� alkava rivi indeksi.
NFmiDrawParamList* NFmiCombinedMapHandler::getTimeSerialViewSideParameters(int viewRowIndex)
{
	auto sideParameterIter = getTimeSerialViewSideParameterIterator(viewRowIndex);
	if(sideParameterIter != timeSerialViewSideParameters_.end())
	{
		return sideParameterIter->get();
	}
	else
		return nullptr;
}

CombinedMapHandlerInterface::SideParametersContainer& NFmiCombinedMapHandler::getTimeSerialViewSideParameterList()
{
	return timeSerialViewSideParameters_;
}

// viewRowIndex parametri on 1:st� alkava rivi indeksi
void NFmiCombinedMapHandler::addEmptySideParamList(int viewRowIndex)
{
	auto sideParameterIter = getTimeSerialViewSideParameterIterator(viewRowIndex);
	timeSerialViewSideParameters_.insert(sideParameterIter, std::make_unique<NFmiDrawParamList>());
}

void NFmiCombinedMapHandler::removeSideParamList(int viewRowIndex)
{
	auto sideParameterIter = getTimeSerialViewSideParameterIterator(viewRowIndex);
	if(sideParameterIter != timeSerialViewSideParameters_.end())
	{
		timeSerialViewSideParameters_.erase(sideParameterIter);
	}
}

// Lis�tty drawParam pit�� my�s palauttaa lopussa, jotta sille voidaan tehd� tarvittavia jatkoasetuksia.
// T�m� on tarpeen varsinkin kun ladataan n�ytt�makroja ja niiss� eritoten macroParam jutut ovat todella hankalia k�sitell�.
std::shared_ptr<NFmiDrawParam> NFmiCombinedMapHandler::addTimeSerialViewSideParameter(const NFmiMenuItem& menuItem, bool isViewMacroDrawParam)
{
	auto rowSideParameters = getTimeSerialViewSideParameters(timeSerialViewIndex_);
	if(rowSideParameters)
	{
		auto drawParam = createTimeSerialViewDrawParam(menuItem, isViewMacroDrawParam);
		if(drawParam)
		{
			::setMacroParamDrawParamSettings(menuItem, drawParam);
			std::string logStartStr("Adding to time-serial-view row #");
			logStartStr += std::to_string(timeSerialViewIndex_);
			logStartStr += " a side parameter: ";
			auto info = ::getInfoOrganizer().Info(drawParam, false, false);
			logParameterAction(logStartStr, menuItem, drawParam, info);

			rowSideParameters->Add(drawParam);
			timeSerialViewDirty(true);
		}
		return drawParam;
	}
	return nullptr;
}

void NFmiCombinedMapHandler::removeTimeSerialViewSideParameter(const NFmiMenuItem& menuItem)
{
	auto rowSideParameters = getTimeSerialViewSideParameters(timeSerialViewIndex_);
	if(rowSideParameters)
	{
		if(rowSideParameters->Find(menuItem.DataIdent(), menuItem.Level(), menuItem.DataType()))
		{
			rowSideParameters->Remove();
			timeSerialViewDirty(true);
		}
	}
}

void NFmiCombinedMapHandler::removeAllTimeSerialViewSideParameters(int viewRowIndex)
{
	auto rowSideParameters = getTimeSerialViewSideParameters(viewRowIndex);
	if(rowSideParameters)
	{
		rowSideParameters->Clear();
		timeSerialViewDirty(true);
	}
}

void NFmiCombinedMapHandler::removeAllTimeSerialViews()
{
	timeSerialViewDirty(true);
	timeSerialViewDrawParamList_->Clear();
	// Tyhjennet��n samalla side-parameter lista
	timeSerialViewSideParameters_.clear();
}

void NFmiCombinedMapHandler::showCrossSectionDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex, bool showParam)
{
	std::shared_ptr<NFmiDrawParam> modifiedDrawParam = getCrosssectionDrawParamFromViewLists(menuItem, viewRowIndex);
	if(modifiedDrawParam)
	{
		modifiedDrawParam->HideParam(!showParam);
		drawParamSettingsChangedDirtyActions(menuItem.MapViewDescTopIndex(), getRealRowNumber(menuItem.MapViewDescTopIndex(), viewRowIndex), modifiedDrawParam);
	}
}

#ifndef UNIX
bool NFmiCombinedMapHandler::modifyDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	std::shared_ptr<NFmiDrawParam> modifiedDrawParam = getDrawParamFromViewLists(menuItem, viewRowIndex);
	if(modifiedDrawParam)
	{
		auto mapViewDescTopIndex = menuItem.MapViewDescTopIndex();
		CWnd* parentView = ApplicationInterface::GetApplicationInterfaceImplementation()->GetView(mapViewDescTopIndex);
		auto realRowNumber = getRealRowNumber(mapViewDescTopIndex, viewRowIndex);
		CFmiModifyDrawParamDlg dlg(SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation(), modifiedDrawParam, ::getMacroPathSettings().DrawParamPath(), true, false, mapViewDescTopIndex, realRowNumber, parentView);
		if(dlg.DoModal() == IDOK)
		{
			updateToModifiedDrawParam(mapViewDescTopIndex, modifiedDrawParam, viewRowIndex);
			return true;
		}
		else
		{
			bool updateStatus = dlg.RefreshPressed(); // my�s false:lla halutaan ruudun p�ivitys, koska jos painettu p�ivit�-nappia ja sitten cancelia, pit�� ruutu p�ivitt��
			drawParamSettingsChangedDirtyActions(mapViewDescTopIndex, realRowNumber, modifiedDrawParam);
			return updateStatus;

			// Huom! Jos on muutettu border-layer piirtoa niin ett� se muuttuisi kyseisell� n�ytt�rivill�, niin �l� kuitenkaan
			// tyhjenn� kuvaa cachesta. Jollain muulla parametri rivill� voi olla samat asetukset ja se voi niit� viel� k�ytt��.
			// Kuvat eiv�t vie paljoa muistia nyky koneiden RAM m��rill� ja aina kun kartta/alue/kuvan geometria muuttuu, ladataan
			// n�ytt�makro, menee kaikki n�m� cachet uusiksi kuitenkin.
		}
	}
	return false;
}
#endif

std::shared_ptr<NFmiDrawParam> NFmiCombinedMapHandler::createTimeSerialViewDrawParam(const NFmiMenuItem& menuItem, bool isViewMacroDrawParam)
{
	auto& infoOrganizer = ::getInfoOrganizer();
	std::shared_ptr<NFmiDrawParam> drawParam = infoOrganizer.CreateDrawParam(menuItem.DataIdent(), menuItem.Level(), menuItem.DataType());
	if(drawParam)
	{
		drawParam->ViewMacroDrawParam(isViewMacroDrawParam);
	}
	return drawParam;
}

// laitetaan drawparam aikasarjan omaan listaa ja jos vertailutila k�yt�ss�, lis�t��
// viel� eri tuottajien drawparamit erilliseen listaan.
// Lis�tty drawParam palautetaan, koska joskus lis�tylle oliolle pit�� tehd� viel� lis� asetuksia,
// varsinkin kun ladataan n�ytt�makroja ja niiden drawParameja.
std::shared_ptr<NFmiDrawParam> NFmiCombinedMapHandler::addTimeSerialView(const NFmiMenuItem& menuItem, bool isViewMacroDrawParam)
{
	timeSerialViewDirty(true);
	auto drawParam = createTimeSerialViewDrawParam(menuItem, isViewMacroDrawParam);
	if(drawParam)
	{
		::setMacroParamDrawParamSettings(menuItem, drawParam);
		std::string logStartStr("Adding to time-serial-view ");
		std::shared_ptr<NFmiFastQueryInfo> info = ::getInfoOrganizer().Info(drawParam, false, false);
		logParameterAction(logStartStr, menuItem, drawParam, info);

		timeSerialViewDrawParamList_->Add(drawParam, timeSerialViewIndex_);
		addEmptySideParamList(timeSerialViewIndex_);

		bool groundData = ::isGroundDataType(drawParam);
		updateFromModifiedDrawParam(drawParam, groundData);
	}
	return drawParam;
}

void NFmiCombinedMapHandler::removeTimeSerialView(const NFmiMenuItem& menuItem)
{
	timeSerialViewDirty(true);
	if(timeSerialViewDrawParamList_->Index(timeSerialViewIndex_))
	{
		timeSerialViewDrawParamList_->Remove();
		removeSideParamList(timeSerialViewIndex_);
	}
}

void NFmiCombinedMapHandler::timeSerialViewModelRunCountSet(const NFmiMenuItem& menuItem)
{
	if(timeSerialViewDrawParamList_->Index(timeSerialViewIndex_))
	{
		auto drawParam = timeSerialViewDrawParamList_->Current();
		drawParam->TimeSerialModelRunCount(static_cast<int>(menuItem.ExtraParam()));

		timeSerialViewDrawParamList_->Dirty(true);
		checkAnimationLockedModeTimeBags(menuItem.MapViewDescTopIndex(), false); // kun parametrin n�kyvyytt� vaihdetaan, pit�� tehd� mahdollisesti animaatio moodin datan tarkistus
		mapViewDirty(menuItem.MapViewDescTopIndex(), false, false, true, false, false, false);
		updateToModifiedDrawParam(menuItem.MapViewDescTopIndex(), drawParam, timeSerialViewIndex_);
		if(menuItem.MapViewDescTopIndex() == CtrlViewUtils::kFmiTimeSerialView)
			timeSerialViewDirty(true);
	}
}

unsigned long& NFmiCombinedMapHandler::getTimeSerialViewIndexReference()
{
	return timeSerialViewIndex_;
}

void NFmiCombinedMapHandler::changeMapType(unsigned int mapViewDescTopIndex, bool goForward)
{
	auto mapAreaIndex = getCurrentMapAreaIndex(mapViewDescTopIndex);
	auto& combinedMapModeState = getCombinedMapModeState(mapViewDescTopIndex, mapAreaIndex);
	combinedMapModeState.changeLayer(goForward);
	setWantedLayerIndex(combinedMapModeState, mapViewDescTopIndex, true);
	std::string refreshMessage = std::string("Map view ") + std::to_string(mapViewDescTopIndex + 1) + "background map layer changed";
	mapLayerChangedRefreshActions(mapViewDescTopIndex, refreshMessage);
}

void NFmiCombinedMapHandler::selectMapLayer(unsigned int mapViewDescTopIndex, const std::string& mapLayerName, bool backgroundMapCase, bool wmsCase)
{
	auto mapAreaIndex = getCurrentMapAreaIndex(mapViewDescTopIndex);
	auto& combinedMapModeState = backgroundMapCase ? getCombinedMapModeState(mapViewDescTopIndex, mapAreaIndex) : getCombinedOverlayMapModeState(mapViewDescTopIndex, mapAreaIndex);
	int newcombinedMapModeIndex = getSelectedCombinedModeMapIndex(mapViewDescTopIndex, mapLayerName, backgroundMapCase, wmsCase);
	combinedMapModeState.combinedModeMapIndex(newcombinedMapModeIndex);
	setWantedLayerIndex(combinedMapModeState, mapViewDescTopIndex, backgroundMapCase);
	std::string refreshMessage = std::string("Map view ") + std::to_string(mapViewDescTopIndex + 1);
	refreshMessage += backgroundMapCase ? "background" : "overlay"; 
	refreshMessage += " map layer changed to: " + mapLayerName;
	mapLayerChangedRefreshActions(mapViewDescTopIndex, refreshMessage);
}

// Palauttaa pair:issa 1. background nimen ja 2. overlay nimen, jos sellaiset on m��ritetty
std::pair<std::string, std::string> NFmiCombinedMapHandler::getMacroReferenceNamesForViewMacro(unsigned int mapViewDescTopIndex, unsigned int mapAreaIndex)
{
	std::pair<std::string, std::string> macroReferenceNamePair;
	macroReferenceNamePair.first = ::getMacroReferenceNameForViewMacro(getCombinedMapModeState(mapViewDescTopIndex, mapAreaIndex), staticBackgroundMapLayerRelatedInfos_[mapAreaIndex], wmsBackgroundMapLayerRelatedInfos_);
	macroReferenceNamePair.second = ::getMacroReferenceNameForViewMacro(getCombinedOverlayMapModeState(mapViewDescTopIndex, mapAreaIndex), staticOverlayMapLayerRelatedInfos_[mapAreaIndex], wmsOverlayMapLayerRelatedInfos_);
	return macroReferenceNamePair;
}

// T�m� siis tekee vain tarvittavat background ja overlay indeksien asetukset, mutta ei mit��n likauksia tai muita juttuja.
// T�t� on tarkoitus k�ytt�� vain kun n�ytt�makroa ladataan, jolloin kaikki tarvittava liataan jo muutenkin.
void NFmiCombinedMapHandler::selectMapLayersByMacroReferenceNamesFromViewMacro(unsigned int mapViewDescTopIndex, unsigned int mapAreaIndex, const std::string& backgroundMacroReferenceName, const std::string& overlayMacroReferenceName)
{
	selectMapLayerByMacroReferenceNameFromViewMacro(true, mapViewDescTopIndex, mapAreaIndex, getCombinedMapModeState(mapViewDescTopIndex, mapAreaIndex), backgroundMacroReferenceName, staticBackgroundMapLayerRelatedInfos_[mapAreaIndex], wmsBackgroundMapLayerRelatedInfos_);
	selectMapLayerByMacroReferenceNameFromViewMacro(false, mapViewDescTopIndex, mapAreaIndex, getCombinedOverlayMapModeState(mapViewDescTopIndex, mapAreaIndex), overlayMacroReferenceName, staticOverlayMapLayerRelatedInfos_[mapAreaIndex], wmsOverlayMapLayerRelatedInfos_);
}

void NFmiCombinedMapHandler::selectMapLayerByMacroReferenceNameFromViewMacro(bool backgroundMapCase, unsigned int mapViewDescTopIndex, unsigned int mapAreaIndex, NFmiCombinedMapModeState& combinedMapModeState, const std::string& macroReferenceName, const MapAreaMapLayerRelatedInfo& staticMapLayerRelatedInfos, const MapAreaMapLayerRelatedInfo& wmsMapLayerRelatedInfos)
{
	if(::selectMapLayerByMacroReferenceNameFromViewMacro(combinedMapModeState, macroReferenceName, staticMapLayerRelatedInfos, wmsMapLayerRelatedInfos))
	{
		setWantedLayerIndex(combinedMapModeState, mapViewDescTopIndex, mapAreaIndex, backgroundMapCase);
	}
}

void NFmiCombinedMapHandler::selectCombinedMapModeIndices(unsigned int mapViewDescTopIndex, unsigned int mapAreaIndex, int usedCombinedModeMapIndex, int usedCombinedModeOverlayMapIndex)
{
	auto& combinedBackgroundMapModeState = getCombinedMapModeState(mapViewDescTopIndex, mapAreaIndex);
	combinedBackgroundMapModeState.combinedModeMapIndex(usedCombinedModeMapIndex);
	setWantedLayerIndex(combinedBackgroundMapModeState, mapViewDescTopIndex, mapAreaIndex, true);

	auto& combinedOverlayMapModeState = getCombinedOverlayMapModeState(mapViewDescTopIndex, mapAreaIndex);
	combinedOverlayMapModeState.combinedModeMapIndex(usedCombinedModeOverlayMapIndex);
	setWantedLayerIndex(combinedOverlayMapModeState, mapViewDescTopIndex, mapAreaIndex, false);
}

int NFmiCombinedMapHandler::getSelectedCombinedModeMapIndex(int mapViewDescTopIndex, const std::string& mapLayerName, bool backgroundMapCase, bool wmsCase)
{
	const auto& mapLayerRelatedInfos = getCurrentMapLayerRelatedInfos(mapViewDescTopIndex, backgroundMapCase, wmsCase);
	if(backgroundMapCase)
	{
		if(wmsCase)
		{
			auto localWmsMapLayerIndex = ::calcWantedMapLayerIndex(mapLayerRelatedInfos, mapLayerName);
			const auto& staticMapLayerRelatedInfos = getCurrentMapLayerRelatedInfos(mapViewDescTopIndex, backgroundMapCase, false);
			return static_cast<int>(localWmsMapLayerIndex + staticMapLayerRelatedInfos.size());
		}
		else
		{
			return ::calcWantedMapLayerIndex(mapLayerRelatedInfos, mapLayerName);
		}
	}
	else
	{
		auto noneSelectionMenuString = CombinedMapHandlerInterface::getNoneOverlayName();
		if(mapLayerName == noneSelectionMenuString)
			return -1;

		if(wmsCase)
		{
			auto localWmsMapLayerIndex = ::calcWantedMapLayerIndex(mapLayerRelatedInfos, mapLayerName);
			const auto& staticMapLayerRelatedInfos = getCurrentMapLayerRelatedInfos(mapViewDescTopIndex, backgroundMapCase, false);
			return static_cast<int>(localWmsMapLayerIndex + staticMapLayerRelatedInfos.size());
		}
		else
		{
			return ::calcWantedMapLayerIndex(mapLayerRelatedInfos, mapLayerName);
		}
	}
}

#ifndef UNIX
void NFmiCombinedMapHandler::setWantedLayerIndex(const NFmiCombinedMapModeState& combinedMapModeState, unsigned int mapViewDescTopIndex, bool backgroundCase)
{
	setWantedLayerIndex(combinedMapModeState, mapViewDescTopIndex, getCurrentMapAreaIndex(mapViewDescTopIndex), backgroundCase);
}

void NFmiCombinedMapHandler::setWantedLayerIndex(const NFmiCombinedMapModeState& combinedMapModeState, unsigned int mapViewDescTopIndex, unsigned int mapAreaIndex, bool backgroundCase)
{
	if(backgroundCase)
	{
		if(combinedMapModeState.isLocalMapCurrentlyInUse())
			getMapViewDescTop(mapViewDescTopIndex)->MapHandler(mapAreaIndex)->UsedMapIndex(combinedMapModeState.currentMapSectionIndex());
		else
			getWmsSupport()->getStaticMapClientState(mapViewDescTopIndex, mapAreaIndex).state_->setBackgroundIndex(combinedMapModeState.currentMapSectionIndex());
	}
	else
	{
		if(combinedMapModeState.isLocalMapCurrentlyInUse())
			getMapViewDescTop(mapViewDescTopIndex)->MapHandler(mapAreaIndex)->OverMapBitmapIndex(combinedMapModeState.currentMapSectionIndex());
		else
			getWmsSupport()->getStaticMapClientState(mapViewDescTopIndex, mapAreaIndex).state_->setOverlayIndex(combinedMapModeState.currentMapSectionIndex());
	}
}
#endif // UNIX

void NFmiCombinedMapHandler::mapLayerChangedRefreshActions(unsigned int mapViewDescTopIndex, const std::string &refreshMessage)
{
	mapViewDirty(mapViewDescTopIndex, true, true, true, false, false, false);
	CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(mapViewDescTopIndex);
	ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs(refreshMessage);
}

unsigned int NFmiCombinedMapHandler::getCurrentMapAreaIndex(unsigned int mapViewDescTopIndex) const
{
	return getMapViewDescTop(mapViewDescTopIndex)->SelectedMapIndex();
}

void NFmiCombinedMapHandler::onToggleShowNamesOnMap(unsigned int mapViewDescTopIndex, bool goForward)
{
	auto mapAreaIndex = getCurrentMapAreaIndex(mapViewDescTopIndex);
	auto& combinedMapModeState = getCombinedOverlayMapModeState(mapViewDescTopIndex, mapAreaIndex);
	combinedMapModeState.changeLayer(goForward);
	setWantedLayerIndex(combinedMapModeState, mapViewDescTopIndex, false);
	std::string refreshMessage = std::string("Map view ") + std::to_string(mapViewDescTopIndex + 1) + "overlay map layer changed";
	mapLayerChangedRefreshActions(mapViewDescTopIndex, refreshMessage);
}

// scrollaa n�ytt�riveja halutun m��r�n (negatiivinen skrollaa yl�s ja positiivinen count alas)
bool NFmiCombinedMapHandler::scrollViewRow(unsigned int mapViewDescTopIndex, int scrollCount)
{
	if(getMapViewDescTop(mapViewDescTopIndex)->ScrollViewRow(scrollCount))
	{
		updateRowInLockedDescTops(mapViewDescTopIndex);
		mapViewDirty(mapViewDescTopIndex, false, false, true, false, false, true);
		return true;
	}
	else
		return false;
}

void NFmiCombinedMapHandler::timeControlTimeStep(unsigned int mapViewDescTopIndex, float timeStepInMinutes)
{
	auto mapViewDescTop = getMapViewDescTop(mapViewDescTopIndex);
	if(mapViewDescTop->TimeControlTimeStep() != timeStepInMinutes)
	{
		mapViewDescTop->TimeControlTimeStep(timeStepInMinutes);
		// laitetaan viela kaikki ajat likaisiksi cachesta kun aika-askel muuttuu, 
		// pit�� mahdollisesti piirt�� uusiksi salama dataa (ja ehk� jotain muuta?), 
		// joten varmuuden vuoksi laitan aina v�limuistin likaiseksi
		mapViewDirty(mapViewDescTopIndex, false, true, true, false, false, false);
	}
}
// palauttaa k�ytetyn aikastepin tunteina. Jos asetuksissa m��r�tty aikasteppi
// on pienempi, kuin datan aikaresoluutio, palautetaan datan aikaresoluutio tunteina.
float NFmiCombinedMapHandler::timeControlTimeStep(unsigned int mapViewDescTopIndex)
{
	return getMapViewDescTop(mapViewDescTopIndex)->TimeControlTimeStep();
}

void NFmiCombinedMapHandler::copyDrawParamsFromViewRow(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams)
{
	NFmiDrawParamList* drawParamList = 0;
	if(useCrossSectionParams)
		drawParamList = getCrossSectionViewDrawParamList(viewRowIndex);
	else
		drawParamList = getDrawParamList(menuItem.MapViewDescTopIndex(), viewRowIndex);
	if(drawParamList)
	{
		if(useCrossSectionParams)
		{
			copyPasteCrossSectionDrawParamListUsedYet_ = true;
			copyPasteCrossSectionDrawParamList_->CopyList(*drawParamList, true);
		}
		else
		{
			copyPasteDrawParamListUsedYet_ = true;
			copyPasteDrawParamList_->CopyList(*drawParamList, true);
		}
	}
}

void NFmiCombinedMapHandler::pasteDrawParamsToViewRow(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams)
{
	NFmiDrawParamList* drawParamList = 0;
	if(useCrossSectionParams)
		drawParamList = getCrossSectionViewDrawParamList(viewRowIndex);
	else
		drawParamList = getDrawParamList(menuItem.MapViewDescTopIndex(), viewRowIndex);
	if(drawParamList)
	{
		if(useCrossSectionParams)
			drawParamList->CopyList(*copyPasteCrossSectionDrawParamList_, false);
		else
			drawParamList->CopyList(*copyPasteDrawParamList_, false);
		makeViewRowDirtyActions(menuItem.MapViewDescTopIndex(), getRealRowNumber(menuItem.MapViewDescTopIndex(), viewRowIndex), drawParamList);
		activeEditedParameterMayHaveChangedViewUpdateFlagSetting(menuItem.MapViewDescTopIndex());
	}
}

void NFmiCombinedMapHandler::copyDrawParamsFromMapViewRow(unsigned int mapViewDescTopIndex)
{
	NFmiDrawParamList* activeDrawParamList = getDrawParamListWithRealRowNumber(mapViewDescTopIndex, absoluteActiveViewRow(mapViewDescTopIndex));
	if(activeDrawParamList)
	{
		copyPasteDrawParamListUsedYet_ = true;
		copyPasteDrawParamList_->CopyList(*activeDrawParamList, true);
	}
}

void NFmiCombinedMapHandler::pasteDrawParamsToMapViewRow(unsigned int mapViewDescTopIndex)
{
	auto realActiveRowIndex = absoluteActiveViewRow(mapViewDescTopIndex);
	NFmiDrawParamList* activeDrawParamList = getDrawParamListWithRealRowNumber(mapViewDescTopIndex, realActiveRowIndex);
	if(activeDrawParamList)
	{
		activeDrawParamList->CopyList(*copyPasteDrawParamList_, false);
		makeViewRowDirtyActions(mapViewDescTopIndex, realActiveRowIndex, activeDrawParamList);
		activeEditedParameterMayHaveChangedViewUpdateFlagSetting(mapViewDescTopIndex);
		ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: Paste drawParams to map view row");
	}
}

int NFmiCombinedMapHandler::absoluteActiveViewRow(unsigned int mapViewDescTopIndex)
{
	if(mapViewDescTopIndex <= CtrlViewUtils::kFmiMaxMapDescTopIndex)
		return getMapViewDescTop(mapViewDescTopIndex)->AbsoluteActiveViewRow();
	else
		return CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->CurrentCrossSectionRowIndex();
}

void NFmiCombinedMapHandler::absoluteActiveViewRow(unsigned int mapViewDescTopIndex, int theAbsoluteActiveRowIndex)
{
	getMapViewDescTop(mapViewDescTopIndex)->AbsoluteActiveViewRow(theAbsoluteActiveRowIndex);
}

NFmiPtrList<NFmiDrawParamList>* NFmiCombinedMapHandler::getDrawParamListVector(unsigned int mapViewDescTopIndex)
{
	if(mapViewDescTopIndex == CtrlViewUtils::kFmiCrossSectionView && crossSectionDrawParamListVector_)
		return crossSectionDrawParamListVector_.get();
	if(mapViewDescTopIndex == CtrlViewUtils::kFmiTimeSerialView)
		return nullptr;
	else
		return getMapViewDescTop(mapViewDescTopIndex)->DrawParamListVector();
}

void NFmiCombinedMapHandler::clearDesctopsAllParams(unsigned int mapViewDescTopIndex)
{
	auto drawParamListVector = getDrawParamListVector(mapViewDescTopIndex);
	if(drawParamListVector)
	{
		// Huom! NFmiPtrList:iss� indeksit alkavat 1:st�...
		for(unsigned long rowIndex = 1; rowIndex <= drawParamListVector->NumberOfItems(); rowIndex++)
		{
			auto* drawParamList = drawParamListVector->Index(rowIndex).CurrentPtr();
			if(drawParamList)
			{
				drawParamList->Clear();
				makeViewRowDirtyActions(mapViewDescTopIndex, rowIndex, drawParamList);
			}
		}
		activeEditedParameterMayHaveChangedViewUpdateFlagSetting(mapViewDescTopIndex);
	}
}

#ifndef UNIX
void NFmiCombinedMapHandler::makeApplyViewMacroDirtyActions(double drawObjectScaleFactor)
{
	// l�j� dirty funktio kutsuja, ota nyt t�st� selv��. Pit�isi laittaa uuteen uskoon koko p�ivitys asetus juttu.
	mapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, true, true, false, false, true);
	for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
	{
		NFmiMapViewDescTop* mapDescTop = getMapViewDescTop(mapViewDescTopIndex);
		mapDescTop->MapViewBitmapDirty(true);
		mapDescTop->SetBorderDrawDirtyState(CountryBorderDrawDirtyState::Geometry);

		// P�ivitet��n v�kisin t�m� pixelSize juttu ja siihen liittyv�t laskut
		auto currentPixelSize = mapDescTop->MapViewSizeInPixels();
		mapDescTop->MapViewSizeInPixels(currentPixelSize, nullptr, drawObjectScaleFactor, !mapDescTop->IsTimeControlViewVisible());
		mapDescTop->UpdateOneMapViewSize();
	}
	::getMacroParamDataCache().clearAllLayers();
}

void NFmiCombinedMapHandler::makeSwapBaseArea(unsigned int mapViewDescTopIndex)
{
	getMapViewDescTop(mapViewDescTopIndex)->MapHandler()->MakeSwapBaseArea();
	// t��ll� ei tarvitse liata mit�� eik� p�ivitt�� mit��n
}

void NFmiCombinedMapHandler::swapArea(unsigned int mapViewDescTopIndex)
{
	NFmiMapViewDescTop* mapDescTop = getMapViewDescTop(mapViewDescTopIndex);
	mapDescTop->MapHandler()->SwapArea();

	// sitten viel� tarvitt�v�t likaukset ja p�ivitykset
	mapDescTop->SetBorderDrawDirtyState(CountryBorderDrawDirtyState::Geometry);
	mapViewDirty(mapViewDescTopIndex, true, true, true, true, false, false); // laitetaan viela kaikki ajat likaisiksi cachesta
	CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(mapViewDescTopIndex);
	mapDescTop->GridPointCache().Clear();
}
#endif // UNIX

void NFmiCombinedMapHandler::removeMacroParamFromDrawParamLists(const std::string& macroParamName)
{
	for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
	{
		NFmiPtrList<NFmiDrawParamList>::Iterator iter = getMapViewDescTop(mapViewDescTopIndex)->DrawParamListVector()->Start();
		for(unsigned long realRowIndex = 1; iter.Next(); realRowIndex++)
		{
			if(iter.CurrentPtr()->RemoveMacroParam(macroParamName))
			{
				::getMacroParamDataCache().update(mapViewDescTopIndex, realRowIndex, iter.Current());
			}
		}
	}
}

// Kun ollaan hiiren kanssa N�yt�-ikkunan p��ll� (karttan�yt�ss�) ja rullataan
// hiiren rullaa, yritet��n siirt�� sill� rivill� olevaa aktiivista parametria
// yl�s/alas riippuen rullauksen suunnasta. Jos raiseParam on true, tuodaan aktiivista
// parametria piirto j�rjestyksess� pintaan p�in.
// Palauttaa true jos siirto onnistui ja pit�� p�ivitt�� n�yt�t, muuten false.
bool NFmiCombinedMapHandler::moveActiveMapViewParamInDrawingOrderList(unsigned int mapViewDescTopIndex, int viewRowIndex, bool raiseParam, bool useCrossSectionParams)
{
	NFmiDrawParamList* list = getDrawParamListWithRealRowNumber(mapViewDescTopIndex, viewRowIndex);
	if(useCrossSectionParams)
		list = getCrossSectionViewDrawParamList(viewRowIndex);
	if(list)
	{
		if(list->MoveActiveParam(raiseParam ? -1 : 1))
		{
			getMapViewDescTop(mapViewDescTopIndex)->MapViewCache().MakeRowDirty(viewRowIndex - 1); // t��ll� rivit alkavat 1:st�, mutta cachessa 0:sta!!!
			makeMacroParamCacheUpdatesForWantedRow(mapViewDescTopIndex, viewRowIndex);
			return true;
		}
	}
	return false;
}

// Kun ollaan hiiren kanssa N�yt�-ikkunan p��ll� (karttan�yt�ss�) ja rullataan
// hiiren rullaa CTRL-nappi pohjassa, yritet��n muuttaa sill� rivill� olevaa
// aktiivista parametria seuraavaan/edelliseen mit� datasta l�ytyy riippuen rullauksen
// suunnasta. Jos nextParam on true, haetaan seuraava parametri (querydatan) parametrilistasta,
// muuten edellinen. Menee p��dyist� yli, eli viimeisest� menee 1. parametriin.
// K�y l�pi my�s aliparametri (TotalWind ja W&C).
// Palauttaa true jos parametrin vaihto onnistui ja pit�� p�ivitt�� n�yt�t, muuten false.
bool NFmiCombinedMapHandler::changeActiveMapViewParam(unsigned int mapViewDescTopIndex, int realRowIndex, int paramIndex, bool nextParam, bool useCrossSectionParams)
{
	//	TRACE("ChangeActiveMapViewParam 1\n");
	NFmiDrawParamList* drawParamList = getDrawParamListWithRealRowNumber(mapViewDescTopIndex, realRowIndex);
	if(useCrossSectionParams)
		drawParamList = getCrossSectionViewDrawParamList(realRowIndex);
	if(drawParamList)
	{
		if(drawParamList->Index(paramIndex))
		{
			std::shared_ptr<NFmiDrawParam> drawParam = drawParamList->Current();
			auto& infoOrganizer = ::getInfoOrganizer();
			std::shared_ptr<NFmiFastQueryInfo> info = infoOrganizer.Info(drawParam, useCrossSectionParams, true);
			if(info && info->SizeParams() > 1)
			{
				bool couldChangeParam = nextParam ? info->NextParam(false) : info->PreviousParam(false);
				if(couldChangeParam == false)
					nextParam ? info->FirstParam(false) : info->LastParam(false);

				std::shared_ptr<NFmiDrawParam> drawParamTmp = infoOrganizer.CreateDrawParam(info->Param(), &drawParam->Level(), drawParam->DataType());
				if(useCrossSectionParams)
					drawParamTmp = infoOrganizer.CreateCrossSectionDrawParam(info->Param(), drawParam->DataType());
				if(drawParamTmp)
				{
					setUpChangedDrawParam(drawParam, drawParamTmp);
					drawParamList->Dirty(true);
					auto cacheMapRow = realRowIndex - 1; // real-map-row alkaa 1:st� ja cache-map-row 0:sta
					getMapViewDescTop(mapViewDescTopIndex)->MapViewCache().MakeRowDirty(cacheMapRow);
					mapViewDirty(mapViewDescTopIndex, false, false, true, false, false, true);
					return true;
				}
			}
		}
	}
	return false;
}

void NFmiCombinedMapHandler::makeMacroParamCacheUpdatesForWantedRow(int mapViewDescTopIndex, int viewRowIndex)
{
	NFmiDrawParamList* drawParamList = getDrawParamList(mapViewDescTopIndex, viewRowIndex);
	if(drawParamList)
	{
		::getMacroParamDataCache().update(mapViewDescTopIndex, getRealRowNumber(mapViewDescTopIndex, viewRowIndex), *drawParamList);
	}
}

// Kun jonkun parametrin piirto-ominaisuuksia muutetaan, eiv�t ne tulevoimaan kuin sille yhdelle
// l�mp�tilalle tai paine parametrille mit� s��det��n. Tai jos on erilaisia malli datoja, niill� on jo
// ladattu valmiiksi omat drawparamit vaikka niit� ei katsottaisikaan ja niihink��n ei tule muutokset
// voimaan, ennen kuin editori k�ynnistet��n uudestaan. T�m� mahdollistaa sen ett�
// piirto-ominaisuudet saadaan heti k�ytt��n esim. kaikille l�mp�tila (T eli par id 4) parametreille.
// Tekee siis vain piirto-ominaisuuksien kopioinnin.
void NFmiCombinedMapHandler::takeDrawParamInUseEveryWhere(std::shared_ptr<NFmiDrawParam>& drawParam, bool useInMap, bool useInTimeSerial, bool useInCrossSection, bool useWithViewMacros)
{
	// 1. k�y l�pi kartta drawparam listat (ota huomioon view-macrot)
	if(useInMap)
	{
		for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
			::initializeWantedDrawParams(*(getMapViewDescTop(mapViewDescTopIndex)->DrawParamListVector()), drawParam, useWithViewMacros);
	}
	// 2. k�y l�pi aikasarja drawparam listat
	if(useInTimeSerial)
		::initializeWantedDrawParams(*timeSerialViewDrawParamList_, drawParam, useWithViewMacros);
	// 3. k�y l�pi poikkileikkaus drawparamit
	if(useInCrossSection && crossSectionDrawParamListVector_)
		::initializeWantedDrawParams(*crossSectionDrawParamListVector_, drawParam, useWithViewMacros);
	// 4. k�y l�pi alussa (kaikelle datalle) tehty drawparamlista
	::initializeWantedDrawParams(*modifiedPropertiesDrawParamList_, drawParam, useWithViewMacros);
}

void NFmiCombinedMapHandler::borrowParams(unsigned int mapViewDescTopIndex, int realViewRowIndex)
{
	NFmiDrawParamList* drawParamListFrom = getDrawParamListWithRealRowNumber(mapViewDescTopIndex, realViewRowIndex);
	NFmiDrawParamList* drawParamListTo = getDrawParamListWithRealRowNumber(mapViewDescTopIndex, absoluteActiveViewRow(mapViewDescTopIndex));
	bool doDebugging = false;
	if(doDebugging || (drawParamListFrom && drawParamListTo && (drawParamListFrom != drawParamListTo)))
	{
		if(doDebugging)
		{
			// Joskus tehd��n hatching testej� ja silloi muutetaan testattavan polygonin indeksi�
			ApplicationInterface::GetApplicationInterfaceImplementation()->SetHatchingDebuggingPolygonIndex(realViewRowIndex);
		}
		else
		{
			if(drawParamListTo->HasBorrowedParams())
				drawParamListTo->ClearBorrowedParams();
			else
				drawParamListTo->BorrowParams(*drawParamListFrom);
		}

		mapViewDirty(mapViewDescTopIndex, false, true, true, false, false, true); // laitetaan viela kaikki ajat likaisiksi cachesta
		::getMacroParamDataCache().update(mapViewDescTopIndex, realViewRowIndex, *drawParamListTo);
	}
}

#ifndef UNIX
// t�m� asettaa uuden karttan�yt�n hilaruudukon koon.
// tekee tarvittavat 'likaukset' ja palauttaa true, jos
// n�ytt�j� tarvitsee p�ivitt��, muuten false (eli ruudukko ei muuttunut).
bool NFmiCombinedMapHandler::setMapViewGrid(unsigned int mapViewDescTopIndex, const NFmiPoint& newValue)
{
	logMessage("Map view grid changed.", CatLog::Severity::Info, CatLog::Category::Visualization);
	boost::shared_ptr<NFmiMapViewWinRegistry> mapViewWinRegistry = ::getApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(mapViewDescTopIndex);
	if(getMapViewDescTop(mapViewDescTopIndex)->SetMapViewGrid(newValue, mapViewWinRegistry.get()))
	{
		mapViewDirty(mapViewDescTopIndex, true, true, true, true, false, false);
		ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(GetWantedMapViewIdFlag(mapViewDescTopIndex));
		return true;
	}
	else
		return false;
}
#endif

CtrlViewUtils::GraphicalInfo& NFmiCombinedMapHandler::getGraphicalInfo(unsigned int mapViewDescTopIndex)
{
	if(mapViewDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
		return ::getCrossSectionSystem().GetGraphicalInfo();
	else
		return getMapViewDescTop(mapViewDescTopIndex)->GetGraphicalInfo();
}

#ifndef UNIX
// t�m� on oikeasti toggle funktio, eli n�yt�/piilota hila/asemapisteet
void NFmiCombinedMapHandler::onShowGridPoints(unsigned int mapViewDescTopIndex)
{
	auto& applicationWinRegistry = ::getApplicationWinRegistry();
	bool newState = !applicationWinRegistry.ConfigurationRelatedWinRegistry().MapView(mapViewDescTopIndex)->ShowStationPlot();
	applicationWinRegistry.ConfigurationRelatedWinRegistry().MapView(mapViewDescTopIndex)->ShowStationPlot(newState);
	getMapViewDescTop(mapViewDescTopIndex)->ShowStationPlotVM(newState); // t�m� pit�� p�ivitt�� molempiin paikkoihin, koska jotkin operaatiot riippuvat ett� MapViewDescTop:issa on p�ivitetty arvo
	mapViewDirty(mapViewDescTopIndex, false, true, true, false, false, false); // t�m� laittaa cachen likaiseksi
	CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(mapViewDescTopIndex);
	ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Show/hide active parameters data's grid/station points");
}
#endif

void NFmiCombinedMapHandler::onToggleGridPointColor(unsigned int mapViewDescTopIndex)
{
	getMapViewDescTop(mapViewDescTopIndex)->ToggleStationPointColor();
	CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(mapViewDescTopIndex);
	ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Change active parameters data's grid/station points color");
}

void NFmiCombinedMapHandler::onToggleGridPointSize(unsigned int mapViewDescTopIndex)
{
	getMapViewDescTop(mapViewDescTopIndex)->ToggleStationPointSize();
	CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(mapViewDescTopIndex);
	ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Change active parameters data's grid/station points size");
}

#ifndef UNIX
void NFmiCombinedMapHandler::onEditSpaceOut(unsigned int mapViewDescTopIndex)
{
	::getApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(mapViewDescTopIndex)->ToggleSpacingOutFactor();
	// MacroParamDataCache ongelma (MAPADACA): Kun harvennusta muutetaan, pit�� liata sellaiset macroParamit, jotka piirret��n symboleilla
	mapViewDirty(mapViewDescTopIndex, false, true, true, false, false, false);
	CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(mapViewDescTopIndex);
	ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Toggle spacing out factor");
}
#endif

void NFmiCombinedMapHandler::onChangeParamWindowPosition(unsigned int mapViewDescTopIndex, bool forward)
{
	auto* mapViewDescTop = getMapViewDescTop(mapViewDescTopIndex);
	mapViewDescTop->ParamWindowViewPositionChange(forward);
	mapViewDirty(mapViewDescTopIndex, false, false, true, false, false, false); // laitetaan kartta likaiseksi
	CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(mapViewDescTopIndex);
	ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Show/Hide param-view");
}

void NFmiCombinedMapHandler::onMoveTimeBoxLocation(unsigned int mapViewDescTopIndex)
{
	auto* mapViewDescTop = getMapViewDescTop(mapViewDescTopIndex);
	mapViewDescTop->TimeBoxPositionChange();
	setTimeBoxValuesToWinRegistry(mapViewDescTopIndex);
	mapViewDirty(mapViewDescTopIndex, false, false, true, false, false, false); // laitetaan kartta likaiseksi
	CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(mapViewDescTopIndex);
	ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Move time-box to next position on map-view");
}

bool NFmiCombinedMapHandler::onSetTimeBoxLocation(unsigned int mapViewDescTopIndex, FmiDirection newPosition)
{
	auto* mapViewDescTop = getMapViewDescTop(mapViewDescTopIndex);
	if(mapViewDescTop && newPosition != mapViewDescTop->TimeBoxLocation())
	{
		mapViewDescTop->TimeBoxLocation(newPosition);
		setTimeBoxValuesToWinRegistry(mapViewDescTopIndex);
		mapViewDirty(mapViewDescTopIndex, false, false, true, false, false, false); // laitetaan kartta likaiseksi
		CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(mapViewDescTopIndex);
		ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Set time-box location on map-view");
		return true;
	}
	return false;
}

bool NFmiCombinedMapHandler::onSetTimeBoxTextSizeFactor(unsigned int mapViewDescTopIndex, float newSizeFactor)
{
	auto* mapViewDescTop = getMapViewDescTop(mapViewDescTopIndex);
	if(mapViewDescTop && newSizeFactor != mapViewDescTop->TimeBoxTextSizeFactor())
	{
		mapViewDescTop->TimeBoxTextSizeFactor(newSizeFactor);
		setTimeBoxValuesToWinRegistry(mapViewDescTopIndex);
		mapViewDirty(mapViewDescTopIndex, false, false, true, false, false, false); // laitetaan kartta likaiseksi
		CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(mapViewDescTopIndex);
		ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Set time-box text size factor on map-view");
		return true;
	}
	return false;
}

#ifndef UNIX
void NFmiCombinedMapHandler::setTimeBoxValuesToWinRegistry(unsigned int mapViewDescTopIndex)
{
	auto* mapViewDescTop = getMapViewDescTop(mapViewDescTopIndex);
	auto& mapViewWinRegistry = ::getApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(mapViewDescTopIndex);
	if(mapViewDescTop && mapViewWinRegistry)
	{
		mapViewWinRegistry->TimeBoxFillColor(mapViewDescTop->TimeBoxFillColor());
		mapViewWinRegistry->TimeBoxLocation(mapViewDescTop->TimeBoxLocation());
		mapViewWinRegistry->TimeBoxTextSizeFactor(mapViewDescTop->TimeBoxTextSizeFactor());
	}
}
#endif

void NFmiCombinedMapHandler::onSetTimeBoxFillColor(unsigned int mapViewDescTopIndex, NFmiColor newColorNoAlpha)
{
	auto* mapViewDescTop = getMapViewDescTop(mapViewDescTopIndex);
	if(mapViewDescTop)
	{
		mapViewDescTop->SetTimeBoxFillColor(newColorNoAlpha);
		setTimeBoxValuesToWinRegistry(mapViewDescTopIndex);
		mapViewDirty(mapViewDescTopIndex, false, false, true, false, false, false); // laitetaan kartta likaiseksi
		CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(mapViewDescTopIndex);
		ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Set time-box fill color on map-view");
	}
}

bool NFmiCombinedMapHandler::onSetTimeBoxFillColorAlpha(unsigned int mapViewDescTopIndex, float newColorAlpha)
{
	auto* mapViewDescTop = getMapViewDescTop(mapViewDescTopIndex);
	if(mapViewDescTop && mapViewDescTop->GetTimeBoxFillColorAlpha() != newColorAlpha)
	{
		mapViewDescTop->SetTimeBoxFillColorAlpha(newColorAlpha);
		setTimeBoxValuesToWinRegistry(mapViewDescTopIndex);
		mapViewDirty(mapViewDescTopIndex, false, false, true, false, false, false); // laitetaan kartta likaiseksi
		CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(mapViewDescTopIndex);
		ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Set time-box fill color alpha on map-view");
		return true;
	}
	return false;
}

void NFmiCombinedMapHandler::onShowTimeString(unsigned int mapViewDescTopIndex)
{
	toggleShowTimeOnMapMode(mapViewDescTopIndex);
	CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(mapViewDescTopIndex);
	ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Toggle show time on map view mode");
}

#ifndef DISABLE_CPPRESTSDK
std::shared_ptr<WmsSupportInterface> NFmiCombinedMapHandler::getWmsSupport() const
{
	std::lock_guard<std::mutex> lock(wmsSupportMutex_);
	return wmsSupport_;
}
#endif // DISABLE_CPPRESTSDK

void NFmiCombinedMapHandler::onToggleLandBorderDrawColor(unsigned int mapViewDescTopIndex)
{
	getMapViewDescTop(mapViewDescTopIndex)->ToggleLandBorderColor();
	mapViewDirty(mapViewDescTopIndex, false, true, true, false, false, false);
	CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(mapViewDescTopIndex);
	ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: Toggle land border draw color");
}

void NFmiCombinedMapHandler::onToggleLandBorderPenSize(unsigned int mapViewDescTopIndex)
{
	getMapViewDescTop(mapViewDescTopIndex)->ToggleLandBorderPenSize();
	mapViewDirty(mapViewDescTopIndex, false, true, true, false, false, false);
	CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(mapViewDescTopIndex);
	ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: Toggle land border draw line width");
}

void NFmiCombinedMapHandler::updateMapView(unsigned int mapViewDescTopIndex)
{
	ApplicationInterface::GetApplicationInterfaceImplementation()->UpdateMapView(mapViewDescTopIndex);
}

void NFmiCombinedMapHandler::onAcceleratorBorrowParams(unsigned int mapViewDescTopIndex, int viewRowIndex)
{
	borrowParams(mapViewDescTopIndex, viewRowIndex);
	updateMapView(mapViewDescTopIndex);
}

void NFmiCombinedMapHandler::onAcceleratorMapRow(unsigned int mapViewDescTopIndex, int startingViewRow)
{
	getMapViewDescTop(mapViewDescTopIndex)->MapRowStartingIndex(startingViewRow);
	updateRowInLockedDescTops(mapViewDescTopIndex);
	ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: changed starting absolute map row");
}

void NFmiCombinedMapHandler::onToggleOverMapBackForeGround(unsigned int mapViewDescTopIndex)
{
	auto* mapViewDescTop = getMapViewDescTop(mapViewDescTopIndex);
	mapViewDescTop->DrawOverMapMode(mapViewDescTop->DrawOverMapMode() == 0 ? 1 : 0);
	mapViewDirty(mapViewDescTopIndex, true, true, true, false, false, false);
	CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(mapViewDescTopIndex);
	ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: changed map overlay's draw order");
}

#ifndef UNIX
void NFmiCombinedMapHandler::onAcceleratorToggleKeepMapRatio()
{
	auto& applicationWinRegistry = ::getApplicationWinRegistry();
	auto newKeepAspectRatioState = !applicationWinRegistry.KeepMapAspectRatio();
	applicationWinRegistry.KeepMapAspectRatio(newKeepAspectRatioState);

	// keep ratio laskut pit�� tehd� kaikille karttan�yt�ille!!!
	for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
	{
		CRect rect;
		auto* mapViewDescTop = getMapViewDescTop(mapViewDescTopIndex);
		mapViewDescTop->MapView()->GetClientRect(rect);
		mapViewDescTop->CalcClientViewXperYRatio(NFmiPoint(rect.Width(), rect.Height()));
		// t�m� 'aiheuttaa' datan harvennuksen. Jos newKeepAspectRatioState on true, tapahtuu silloin
		// automaattinen kartan zoomaus ja macroParamCacheData pit�� silloin tyhjent�� kaikille karttan�yt�ille
		mapViewDirty(mapViewDescTopIndex, true, true, true, newKeepAspectRatioState, false, false);
		ApplicationInterface::GetApplicationInterfaceImplementation()->UpdateMapView(mapViewDescTopIndex);
	}
	ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
	ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: toggle keep map's aspect ratio setting");
}
#endif

#ifndef UNIX
// Jos datan area ja kartta ovat "samanlaisia", laitetaan zoomiksi editoiavan datan alue
// muuten laitetaan kurrentti kartta kokonaisuudessaan n�kyviin.
void NFmiCombinedMapHandler::onButtonDataArea(unsigned int mapViewDescTopIndex)
{
	std::shared_ptr<NFmiFastQueryInfo> info = ::getEditedInfo();
	bool editedDataExist = info ? true : false;
	bool dataAreaExist = false;
	if(editedDataExist)
		dataAreaExist = info->Area() != 0;
	bool areasAreSameKind = false;
	auto* mapViewDescTop = getMapViewDescTop(mapViewDescTopIndex);
	if(dataAreaExist)
	{
		std::shared_ptr<NFmiArea> infoArea(info->Area()->Clone());
		areasAreSameKind = NFmiQueryDataUtil::AreAreasSameKind(infoArea.get(), mapViewDescTop->MapHandler()->TotalArea().get());
	}
	if(areasAreSameKind)
	{
		static int counter = 0;
		counter++; // t�m�n avulla jos kartan alue ja datan alue samat, joka toisella kerralla zoomataan dataa, ja joka toisella kartan alueeseen
		NFmiRect intersectionRect(mapViewDescTop->MapHandler()->TotalArea()->XYArea().Intersection(mapViewDescTop->MapHandler()->TotalArea()->XYArea(info->Area())));
		std::shared_ptr<NFmiArea> usedArea(mapViewDescTop->MapHandler()->TotalArea()->CreateNewArea(intersectionRect));
		logMessage("Setting zoomed area the same as edited data.", CatLog::Severity::Info, CatLog::Category::Visualization);
		if(counter % 2 == 0)
			setMapArea(mapViewDescTopIndex, mapViewDescTop->MapHandler()->TotalArea());
		else
			setMapArea(mapViewDescTopIndex, usedArea);
	}
	else
	{
		logMessage("Setting zoomed are the same as map area.", CatLog::Severity::Info, CatLog::Category::Visualization);
		setMapArea(mapViewDescTopIndex, mapViewDescTop->MapHandler()->TotalArea());
	}
	ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: data area button pressed", GetWantedMapViewIdFlag(mapViewDescTopIndex));
}
#endif // UNIX

#ifndef UNIX
double NFmiCombinedMapHandler::drawObjectScaleFactor()
{
	return ::getApplicationWinRegistry().DrawObjectScaleFactor();
}

void NFmiCombinedMapHandler::drawObjectScaleFactor(double newValue)
{
	::getApplicationWinRegistry().DrawObjectScaleFactor(newValue);

	// laitetaan s��d�n yhteydess� kaikki desctop graphicalinfot likaisiksi
	for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
		getGraphicalInfo(mapViewDescTopIndex).fInitialized = false;
}
#endif

std::shared_ptr<NFmiDrawParam> NFmiCombinedMapHandler::getUsedDrawParamForEditedData(const NFmiDataIdent& dataIdent)
{
	if(modifiedPropertiesDrawParamList_->Find(dataIdent, 0, NFmiInfoData::kEditable, "", true))
		return modifiedPropertiesDrawParamList_->Current(); // katsotaan l�ytyyk� ensin jo k�yt�ss� olevista DrawParameista haluttu (jos siin� on muutoksia arvoissa)
	else
		return ::getInfoOrganizer().CreateDrawParam(dataIdent, 0, NFmiInfoData::kEditable);
}

std::string NFmiCombinedMapHandler::getCurrentMapLayerGuiName(int mapViewDescTopIndex, bool backgroundMap)
{
#ifndef DISABLE_CPPRESTSDK
	auto useWmsMapLayer = backgroundMap ? useWmsMapDrawForThisDescTop(mapViewDescTopIndex) : useWmsOverlayMapDrawForThisDescTop(mapViewDescTopIndex);
	if(useWmsMapLayer)
	{
		if(backgroundMap)
		{
			auto mapLayerIndex = getCombinedMapModeState(mapViewDescTopIndex, getCurrentMapAreaIndex(mapViewDescTopIndex)).currentMapSectionIndex();
			return ::getWmsMapLayerGuiName(mapLayerIndex, getWmsSupport()->getSetup()->background, false);
		}
		else
		{
			auto mapLayerIndex = getCombinedOverlayMapModeState(mapViewDescTopIndex, getCurrentMapAreaIndex(mapViewDescTopIndex)).currentMapSectionIndex();
			return ::getWmsMapLayerGuiName(mapLayerIndex, getWmsSupport()->getSetup()->overlay, false);
		}
	}
	else
#endif // DISABLE_CPPRESTSDK
	{
		return getMapViewDescTop(mapViewDescTopIndex)->GetCurrentGuiMapLayerText(backgroundMap);
	}
}

std::string NFmiCombinedMapHandler::getCurrentMapLayerGuiText(int mapViewDescTopIndex, bool backgroundMap)
{
	std::string mapLayerText = localOnlyMapModeUsed() ? "-" : "+";
	mapLayerText += backgroundMap ? ::GetDictionaryString("Map") : ::GetDictionaryString("Overlay");
	auto isWmsLayer = backgroundMap ? useWmsMapDrawForThisDescTop(mapViewDescTopIndex) : useWmsOverlayMapDrawForThisDescTop(mapViewDescTopIndex);
	mapLayerText += isWmsLayer ? "[W]" : "[L]";
	mapLayerText += ": ";
	mapLayerText += getCurrentMapLayerGuiName(mapViewDescTopIndex, backgroundMap);

	return mapLayerText;
}

#ifndef UNIX
bool NFmiCombinedMapHandler::useCombinedMapMode() const
{
	return ::getApplicationWinRegistry().ConfigurationRelatedWinRegistry().UseCombinedMapMode();
}

void NFmiCombinedMapHandler::useCombinedMapMode(bool newValue)
{
	::getApplicationWinRegistry().ConfigurationRelatedWinRegistry().UseCombinedMapMode(newValue);
	// Varmuuden vuoksi kaikki kartta piirrot uusiksi (voi optimoida my�hemmin, koska on tapauksia, miss� ei mitk��n asiat muutu)
	mapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, true, true, false, false, false);
	ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
}
#endif

bool NFmiCombinedMapHandler::useWmsMapDrawForThisDescTop(unsigned int mapViewDescTopIndex)
{
	return !getCombinedMapModeState(mapViewDescTopIndex, getCurrentMapAreaIndex(mapViewDescTopIndex)).isLocalMapCurrentlyInUse();
}

bool NFmiCombinedMapHandler::useWmsOverlayMapDrawForThisDescTop(unsigned int mapViewDescTopIndex)
{
	return !getCombinedOverlayMapModeState(mapViewDescTopIndex, getCurrentMapAreaIndex(mapViewDescTopIndex)).isLocalMapCurrentlyInUse();
}

bool NFmiCombinedMapHandler::isOverlayMapDrawnForThisDescTop(unsigned int mapViewDescTopIndex, int wantedDrawOverMapMode)
{
	// Onko piirtokoodi oikeassa kohassa ,ett� voitaisiin piirt�� overlay kerros?
	if(getMapViewDescTop(mapViewDescTopIndex)->DrawOverMapMode() == wantedDrawOverMapMode)
	{
		// Onko joku overlay kerros valittuna (indeksi ei saa olla -1, jolloin ei ole tarkoitus piirt�� mit��n)?
		return getCombinedOverlayMapModeState(mapViewDescTopIndex, getCurrentMapAreaIndex(mapViewDescTopIndex)).currentMapSectionIndex() >= 0;
	}
	return false;
}

#ifndef UNIX
bool NFmiCombinedMapHandler::localOnlyMapModeUsed() const
{
	if(wmsSupportAvailable())
	{
		return !::getApplicationWinRegistry().ConfigurationRelatedWinRegistry().UseCombinedMapMode();
	}
	return true;
}
#endif

NFmiCombinedMapModeState& NFmiCombinedMapHandler::getCombinedMapModeState(unsigned int mapViewDescTopIndex, unsigned int mapAreaIndex)
{
	return combinedBackgroundMapModeStates_.at(mapViewDescTopIndex).at(mapAreaIndex);
}

NFmiCombinedMapModeState& NFmiCombinedMapHandler::getCombinedOverlayMapModeState(unsigned int mapViewDescTopIndex, unsigned int mapAreaIndex)
{
	return combinedOverlayMapModeStates_.at(mapViewDescTopIndex).at(mapAreaIndex);
}

void NFmiCombinedMapHandler::addBorderLineLayer(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	auto dataType = menuItem.DataType();
	std::shared_ptr<NFmiDrawParam> drawParam = ::getInfoOrganizer().CreateDrawParam(menuItem.DataIdent(), menuItem.Level(), dataType);
	if(drawParam)
	{
		drawParam->DataType(dataType);
		drawParam->ParameterAbbreviation(::GetDictionaryString("Country border layer"));
		// Laitetaan viivan paksuudeksi 1 pikseli (border-layer tapauksessa yksikk� on siis pikseli)
		drawParam->SimpleIsoLineWidth(1);
		// Oletus v�ri on musta (mik� on luultavasti muutenkin IsolineColor:in oletusv�ri, mutta asetus varmuuden vuoksi)
		drawParam->IsolineColor(NFmiColor(0, 0, 0));

		auto mapViewDescTopIndex = menuItem.MapViewDescTopIndex();
		auto* drawParamList = getDrawParamList(mapViewDescTopIndex, viewRowIndex);
		if(drawParamList && drawParamList->Add(drawParam, menuItem.IndexInViewRow()))
		{
			makeMapViewRowDirty(mapViewDescTopIndex, viewRowIndex);
		}
	}
}

void NFmiCombinedMapHandler::moveBorderLineLayer(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	auto mapViewDescTopIndex = menuItem.MapViewDescTopIndex();
	auto* drawParamList = getDrawParamList(mapViewDescTopIndex, viewRowIndex);
	if(drawParamList)
	{
		auto layerIndex = CombinedMapHandlerInterface::getBorderLayerIndex(drawParamList);
		if(drawParamList->MoveParam(layerIndex, menuItem.IndexInViewRow()))
		{
			makeMapViewRowDirty(mapViewDescTopIndex, viewRowIndex);
		}
	}
}

void NFmiCombinedMapHandler::insertParamLayer(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	addView(menuItem, viewRowIndex);
}

void NFmiCombinedMapHandler::activeEditedParameterMayHaveChangedViewUpdateFlagSetting(int mapViewDescTopIndex)
{
	auto usedViewUpdateFlag = ::GetWantedMapViewIdFlag(mapViewDescTopIndex) | SmartMetViewId::DataFilterToolDlg | SmartMetViewId::BrushToolDlg;
	ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(usedViewUpdateFlag);
}

void NFmiCombinedMapHandler::initializeMapLayerInfos()
{
	initializeStaticMapLayerInfos(staticBackgroundMapLayerRelatedInfos_, true);
	initializeStaticMapLayerInfos(staticOverlayMapLayerRelatedInfos_, false);
#ifndef DISABLE_CPPRESTSDK
	initializeWmsMapLayerInfos();
#endif // DISABLE_CPPRESTSDK
}

void NFmiCombinedMapHandler::initializeStaticMapLayerInfos(std::vector<MapAreaMapLayerRelatedInfo>& mapLayerRelatedInfos, bool backgroundMapCase)
{
	mapLayerRelatedInfos.clear();
	for(size_t mapAreaIndex = 0; mapAreaIndex < mapConfigurationSystem_->Size(); mapAreaIndex++)
	{
		const auto &mapConfiguration = mapConfigurationSystem_->GetMapConfiguration(mapAreaIndex);
		MapAreaMapLayerRelatedInfo mapAreaMapLayerRelatedInfos;
		auto mapLayerCount = backgroundMapCase ? mapConfiguration->MapLayersCount() : mapConfiguration->MapOverlaysCount();
		for(size_t mapLayerIndex = 0; mapLayerIndex < mapLayerCount; mapLayerIndex++)
		{
			auto bestGuiMapLayerName = mapConfiguration->GetBestGuiUsedMapLayerName(mapLayerIndex, backgroundMapCase);
			auto macroReferenceName = mapConfiguration->GetMacroReferenceName(mapLayerIndex, backgroundMapCase);
			NFmiMapLayerRelatedInfo mapLayerRelatedInfo(bestGuiMapLayerName, macroReferenceName, false);
			mapAreaMapLayerRelatedInfos.push_back(mapLayerRelatedInfo);
		}

		mapLayerRelatedInfos.push_back(mapAreaMapLayerRelatedInfos);
	}
}

#ifndef DISABLE_CPPRESTSDK
void NFmiCombinedMapHandler::initializeWmsMapLayerInfos()
{
	if(wmsSupportAvailable())
	{
		auto wmsSupportPtr = getWmsSupport();
		::initializeWmsMapLayerInfos(wmsBackgroundMapLayerRelatedInfos_, wmsSupportPtr->getSetup()->background);
		::initializeWmsMapLayerInfos(wmsOverlayMapLayerRelatedInfos_, wmsSupportPtr->getSetup()->overlay);
	}
}
#endif // DISABLE_CPPRESTSDK

const MapAreaMapLayerRelatedInfo& NFmiCombinedMapHandler::getCurrentMapLayerRelatedInfos(int mapViewDescTopIndex, bool backgroundMapCase, bool wmsCase)
{
	if(wmsCase)
	{
		if(backgroundMapCase)
			return wmsBackgroundMapLayerRelatedInfos_;
		else
			return wmsOverlayMapLayerRelatedInfos_;
	}
	else
	{
		auto mapAreaIndex = getCurrentMapAreaIndex(mapViewDescTopIndex);
		if(backgroundMapCase)
			return staticBackgroundMapLayerRelatedInfos_[mapAreaIndex];
		else
			return staticOverlayMapLayerRelatedInfos_[mapAreaIndex];
	}

	// Virhetilanteissa tai jos wms:�� ei ole ollenkaan edes initialisoitu, palautetaan tyhj��
	const static MapAreaMapLayerRelatedInfo emptyDummy;
	return emptyDummy;
}

void NFmiCombinedMapHandler::clearMacroParamCache(unsigned long mapViewDescTopIndex, unsigned long realRowIndex, std::shared_ptr<NFmiDrawParam>& drawParam)
{
	::getMacroParamDataCache().clearMacroParamCache(mapViewDescTopIndex, realRowIndex, drawParam->InitFileName());
}
