#include "NFmiCombinedMapHandler.h"
#include "NFmiMapViewDescTop.h"
#include "WmsSupportInterface.h"
#include "NFmiApplicationWinRegistry.h"
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
#include "FmiModifyDrawParamDlg.h"
#include "SmartMetDocumentInterface.h"
#include "NFmiMacroPathSettings.h"
#include "wmssupport/WmsSupportState.h"
#include "wmssupport/WmsClient.h"
#include "wmssupport/Setup.h"
#include "CtrlViewTimeConsumptionReporter.h"
#include "wmssupport/ChangedLayers.h"

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
	// Laitetaan tämä piiloon cpp tiedostoon, koska haluan tehdä Wms::WmsSupport -luokan objektin käyttöön tänne,
	// missä on koko luokan kaikki metodit käytössä, mutta palauttaa WmsSupportInterface -pointterin ulkopuolelle.
	// Lisäksi kaikki #ifndef DISABLE_CPPRESTSDK -jutut saa piiloon myös cpp:hen.
#ifndef DISABLE_CPPRESTSDK
	// Tähän wmsSupport_:iin laitetaan kulloinkin käytössä oleva WmsSupport olio.
	// Koska on huomattu että esim. Beta-tuotteiden luonnissa jos käytetään paljon wms dataa, on wms osio mennyt rikki 2-3 vuorokauden kuluttua.
	// Nyt on siis tarkoitus että WmsSupport olio vaihdetaan määräajoin. Tämä vaihto pitää tehdä niin että vaikka
	// 12 tunnin välein MainFramen timer käynnistää vaihto operaation, jolloin tapahtuu seuraavaa:
	// 1) Luodaan uusi WmsSupport olio tmpWmsSupport_ muuttujalle
	// 2) Mutex lukon turvissa vaihdetaan swap:illa wmsSupport_ ja tmpWmsSupport_ olioiden sisältö
	// 3) Nyt tmpWmsSupport_ oliolle annetaan käsky tappaa itsensä
	// 4) Kun se on kuollut, nollataan tmpWmsSupport_ olio
	// 5) Tähän soppaan luo omat hankaluutensa kun uutta WmsSupport oliota luodaan, tällöin pitää tarkastella onko vanhaa haluttu jo tappaa, jolloin uusi pitää myös laittaa tappamaan itsensä ja mitään vaihtoa ei enää tehdä.
	std::shared_ptr<WmsSupportInterface> wmsSupport_;
	// capabilityTree_ olion käyttö pitää suojata thread turvallisella lukolla.
	std::mutex wmsSupportMutex_;
	// Koska WmsSupport olion tappaminen vaatii odottelua, pitää tehdä tälläinen tmp-oliolle oma kuolin paikka.
	// Eli kun wmsSupport_ oli on vaihdettu tmpWmsSupport_ olioon, voidaan odotella että se tappaa itsensä ja se voidaan deletoida.
	std::shared_ptr<WmsSupportInterface> tmpWmsSupport_;
	// Tämä atomic-flagin avulla estetään samanaikaisten renewal prosessien käynnistyksen
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

	bool isTotalWorld(const boost::shared_ptr<NFmiArea>& area)
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

	// Tässä lasketaan tietynlaisille datoille 'minimi' näyttöjen likaus aikaväli.
// Näitä datoja ovat mm. tutka, mesan analyysi(, laps datat?). Niiden pitää olla hila dataa.
// Dataa, millä on yksiselitteiset valitimet, eli yksi havainto per aika.
// Jos palautettavan timebagin resoluutio on 0, ei timebagia käytetä jatkossa.
	NFmiTimeBag getDirtyViewTimes(NFmiQueryData* queryData, NFmiInfoData::Type dataType, const NFmiTimeDescriptor& removedDatasTimes)
	{
		NFmiTimeBag times; // tässä menee resoluutio 0:ksi, toivottavasti kukaan ei muuta default konstruktorin käyttäytymistä.
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
						step = 1; // asetetaan 0::sta poikkeava, muulla ei ole väliä
					times = NFmiTimeBag(aTime, queryData->Info()->TimeDescriptor().LastTime(), step);
				}
			}
		}
		return times;
	}

	// HUOM! tämä ei läitä päivityksiä päälle Parameter-selection dialogille, se hoidetaan toista kautta
	void setUpdatedViewIdMaskAfterDataLoaded(NFmiFastQueryInfo& fastInfo)
	{
		// Kaikkien datojen kanssa pitää päivittää varmuuden vuoksi karttanäytöt, aikasarja ja case-study
		ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews | SmartMetViewId::TimeSerialView | SmartMetViewId::CaseStudyDlg);
		if(fastInfo.SizeLevels() > 2)
		{
			// vertikaali datojen kanssa pitää päivittää myös luotaus, poikkileikkaus ja trajektori näytöt
			ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::SoundingView | SmartMetViewId::CrossSectionView | SmartMetViewId::TrajectoryView);
		}
		else if(fastInfo.SizeLevels() == 1)
		{
			// surface datojen kanssa pitää päivittää myös asema-data-taulukko, wind-table
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

	void normalDataDirtiesCacheRowTraceLog(boost::shared_ptr<NFmiDrawParam>& drawParam, unsigned int mapViewDescTopIndex, int cacheRowNumber, const std::string& theFileName)
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

	// Synop-plot ja muut vastaavat ovat erikoistapauksia, koska sillä erikois param-id (kFmiSpSynoPlot, etc.), ja ne pitää tarkistaa erikseen.
	// Palauttaa true, jos pitää tehdä päivityksiä näyttöriville.
	bool checkAllSynopPlotTypeUpdates(unsigned int mapViewDescTopIndex, boost::shared_ptr<NFmiDrawParam>& drawParam, NFmiProducer& newDataProducer, NFmiMapViewDescTop& descTop, int cacheRowNumber, const std::string& theFileName)
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

	bool makeNormalDataDrawingLayerCahceChecks(unsigned int mapViewDescTopIndex, NFmiFastQueryInfo& fastInfo, NFmiInfoData::Type dataType, const NFmiTimeBag& dirtyViewTimes, const std::string& fileName, boost::shared_ptr<NFmiDrawParam>& drawParam, NFmiProducer& dataProducer, NFmiMapViewDescTop& descTop, int cacheRowNumber)
	{
		const NFmiLevel* level = fastInfo.SizeLevels() <= 1 ? 0 : fastInfo.Level(); // ns. pinta datan kanssa ei välitetä leveleistä
		if(drawParam->DataType() == dataType && *drawParam->Param().GetProducer() == dataProducer && fastInfo.Param(drawParam->Param()) && (level == 0 || drawParam->Level().LevelType() == level->LevelType()))
		{ // jos päivitetty data oli samaa tyyppiä ja sillä oli sama tuottaja kuin näytöllä olevalla drawParamilla, laitetaan rivin piirto uusiksi
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
			limit2Out.NextMetTime(); // jos limit2 oli pyöristynyt taaksepäin, laitetaan se askel eteenpäin
	}

	// Liataan descTopin view-cachesta ne ajat uudesta timebagistä, mitä ei ole vanhassa timebagissa
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
			boost::shared_ptr<NFmiDrawParam> drawParam = drawParamList.Current();
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

	// etsi timedescriptorista viimeisin aika joka sopii annettuun aika-steppiin ja joka on lisäksi annettujen aikarajojen alueella.
	// Jos ei löydy, palauttaa false.
	// fDemandExactCheck muuttuja vaatii että aikojen pitää olla juuri tarkalleen aika stepissä. Mutta jos kyse on
	// esim. salama datasta, haetaan se aika, joka sopii steppiin ja joka on myöhäisin aika.
	bool getLatestValidTimeWithCorrectTimeStep(NFmiTimeDescriptor& checkedTimes, int timeStepInMinutes, bool demandExactCheck, const NFmiMetTime& limit1, const NFmiMetTime& limit2, NFmiMetTime& foundTimeOut)
	{
		checkedTimes.Time(checkedTimes.LastTime()); // asetetaan viimeiseen aikaan
		do
		{
			NFmiMetTime aTime(checkedTimes.Time());
			aTime.SetTimeStep(timeStepInMinutes, true);
			if(demandExactCheck)
			{
				if(aTime == checkedTimes.Time()) // kun vaaditaan tarkaa checkkiä, eihän aika muuttunut, kun sitä rukattiin halutulla timestepillä
				{
					if(aTime >= limit1 && aTime <= limit2) // onko aika annettujen rajojen sisällä
					{
						foundTimeOut = aTime;
						return true;
					}
				}
			}
			else
			{
				if(checkedTimes.Time() < aTime)
					aTime.NextMetTime(); // jos aika oli pyöristynyt taaksepäin, laitetaan se askeleen verran eteenpäin
				if(aTime >= limit1 && aTime <= limit2)
				{
					foundTimeOut = aTime;
					return true;
				}
			}
			if(checkedTimes.Time() <= limit1)
				break; // ei tarvetta jatkaa enää, koska loput ajoista ovat kaikki pienempiä kuin ala rajan aika
		} while(checkedTimes.Previous());
		return false;
	}

	void setInfosMask(std::vector<boost::shared_ptr<NFmiFastQueryInfo> >& infoVector, unsigned long usedMask)
	{
		for(const auto& info : infoVector)
		{
			info->MaskType(usedMask);
		}
	}

	void macroParamDirtiesCacheRowTraceLog(boost::shared_ptr<NFmiDrawParam>& drawParam, const MacroParamDataInfo& macroParamDataInfo, unsigned int mapViewDescTopIndex, int cacheRowNumber, const std::string& fileName)
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

	void SetCPCropGridSettings(const boost::shared_ptr<NFmiArea>& newArea, unsigned int mapViewDescTopIndex)
	{
		CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->SetCPCropGridSettings(newArea, mapViewDescTopIndex);
	}

	NFmiApplicationWinRegistry& getApplicationWinRegistry()
	{
		return CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->ApplicationWinRegistry();
	}

	NFmiCrossSectionSystem& getCrossSectionSystem()
	{
		return *CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->CrossSectionSystem();
	}

	NFmiMacroParamDataCache& getMacroParamDataCache()
	{
		return CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->MacroParamDataCache();
	}

	NFmiMacroParamSystem& getMacroParamSystem()
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

	std::vector<boost::shared_ptr<NFmiFastQueryInfo> > getSortedSynopInfoVector(int producerId, int producerId2 = -1, int producerId3 = -1, int producerId4 = -1)
	{
		return CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->GetSortedSynopInfoVector(producerId, producerId2, producerId3, producerId4);
	}

	void setMacroParamDrawParamSettings(const NFmiMenuItem& menuItem, boost::shared_ptr<NFmiDrawParam>& drawParam)
	{
		auto dataType = menuItem.DataType();
		if(NFmiDrawParam::IsMacroParamCase(dataType))
		{
			drawParam->ParameterAbbreviation(menuItem.MenuText()); // macroParamin tapauksessa pitää nimi asettaa tässä (tätä nimilyhennettä käytetään tunnisteenä myöhemmin!!)
			boost::shared_ptr<NFmiMacroParam> usedMacroParam;
			auto& macroParamSystem = ::getMacroParamSystem();
			const auto& macroParamInitFile = menuItem.MacroParamInitName();
			if(!macroParamInitFile.empty())
			{
				// Tämä on toivottu tapa alustaa, koska muuten saman nimiset 
				// macroParamit eri hakemistoissa voivat aiheuttaa päällekkäisyyksiä
				usedMacroParam = macroParamSystem.GetWantedMacro(macroParamInitFile);
			}
			else
			{
				boost::shared_ptr<NFmiMacroParamFolder> currentFolder = macroParamSystem.GetCurrentFolder();
				if(currentFolder && currentFolder->Find(drawParam->ParameterAbbreviation()))
					usedMacroParam = currentFolder->Current();
			}

			// ei alusteta, jos oli virheellinen macroParam
			if(usedMacroParam != 0 && usedMacroParam->ErrorInMacro() == false) 
			{
				drawParam->Init(usedMacroParam->DrawParam());
				// Datatyyppi pitää ottaa lopuksi menuItemista, koska niitä on nyt jo 4 erilaista 
				// ja usedMacroParam ei tiedä siitä mitään
				drawParam->DataType(dataType);
			}
		}
	}

	NFmiInfoData::Type getUsableCrossSectionDataType(NFmiInfoData::Type dataType, NFmiInfoData::Type alternateType, const NFmiProducer& givenProducer)
	{
		NFmiInfoData::Type finalDataType = dataType;
		auto &infoOrganizer = ::getInfoOrganizer();
		boost::shared_ptr<NFmiFastQueryInfo> info = infoOrganizer.FindInfo(dataType, givenProducer, false);
		if(info == 0)
		{
			info = infoOrganizer.FindInfo(alternateType, givenProducer, false);
			if(info)
				finalDataType = alternateType;
		}
		return finalDataType;
	}

	// jos on katsottu hirlma mallipinta dataa poikkileikkaus ikkunassa ja tuottaja vaihdetaan GFS:ksi,
	// tällöin ei tule mitään näkyviin, koska GFS:llä ei ole hybridi dataa. Ja jos on katsottu painepintadataa
	// ja vaihdetaan Arome tuottajaan, tällöin ei tule mitään näkyviin, koska ei ole kuin hybridi dataa.
	// Tätä varten tämä funktio tekee vielä viimeiset tarkastelut, löytyykö dataa ja säätää datatyyppiä tarvittaessa.
	NFmiInfoData::Type checkCrossSectionLevelData(NFmiInfoData::Type dataType, const NFmiProducer& givenProducer)
	{
		NFmiInfoData::Type finalDataType = dataType;
		if(dataType == NFmiInfoData::kViewable)
			finalDataType = ::getUsableCrossSectionDataType(NFmiInfoData::kViewable, NFmiInfoData::kHybridData, givenProducer);
		else if(dataType == NFmiInfoData::kHybridData)
			finalDataType = ::getUsableCrossSectionDataType(NFmiInfoData::kHybridData, NFmiInfoData::kViewable, givenProducer);
		return finalDataType;
	}

	boost::shared_ptr<NFmiFastQueryInfo> getEditedInfo()
	{
		return CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->EditedSmartInfo();
	}

	// Perus-oliolle (list-list) on jo varattu muistia, tässä alustetaan vain tyhjät listat kokonaislistaan.
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

	// Muuta changedDrawParam:ia niin että muuten asetukset tulevat newDrawParamSettings:ista, paitsi muutamat erikseen asetettavat on otettava vanhasta.
	void setUpChangedDrawParam(boost::shared_ptr<NFmiDrawParam>& changedDrawParam, boost::shared_ptr<NFmiDrawParam>& newDrawParamSettings)
	{
		// 1. Ota ensin tietyt asetukset vanhasta uuteen
		newDrawParamSettings->ModelRunIndex(changedDrawParam->ModelRunIndex());
		newDrawParamSettings->TimeSerialModelRunCount(changedDrawParam->TimeSerialModelRunCount());

		// 2. Aseta sitten muutettavan kaikki asetukset uusista asetuksista
		changedDrawParam->Init(newDrawParamSettings);
	}

	bool areDrawParamsSimilarType(boost::shared_ptr<NFmiDrawParam>& drawParam, boost::shared_ptr<NFmiDrawParam>& targetDrawParam, bool useWithViewMacros)
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

	void initializeWantedDrawParams(NFmiFastDrawParamList& drawParamList, boost::shared_ptr<NFmiDrawParam>& drawParam, bool useWithViewMacros)
	{
		for(auto iter = drawParamList.Begin(); iter != drawParamList.End(); ++iter)
		{
			boost::shared_ptr<NFmiDrawParam> aDrawParam = iter->second;
			if(::areDrawParamsSimilarType(drawParam, aDrawParam, useWithViewMacros))
				aDrawParam->Init(drawParam, true);
		}
	}

	void initializeWantedDrawParams(NFmiDrawParamList& drawParamList, boost::shared_ptr<NFmiDrawParam>& drawParam, bool useWithViewMacros)
	{
		for(drawParamList.Reset(); drawParamList.Next(); )
		{
			boost::shared_ptr<NFmiDrawParam> aDrawParam = drawParamList.Current();
			if(::areDrawParamsSimilarType(drawParam, aDrawParam, useWithViewMacros))
				aDrawParam->Init(drawParam, true);
		}
	}

	void initializeWantedDrawParams(NFmiPtrList<NFmiDrawParamList>& drawParamListVector, boost::shared_ptr<NFmiDrawParam>& drawParam, bool useWithViewMacros)
	{
		NFmiPtrList<NFmiDrawParamList>::Iterator iter = drawParamListVector.Start();
		for(; iter.Next(); )
			::initializeWantedDrawParams((*iter.CurrentPtr()), drawParam, useWithViewMacros);
	}

	// Lokaali+wms karttojen yhdistelmä moodiin liittyvät valitut taustakarttaindeksit kaikille eri kartta-alueille (suomi,skandi,euro,maailma).
	// Teksti on seuraavaa muotoa (tämä luokka ei tosin parseroi tai tee muuta kuin säilyttää stringin): 
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
			// Lisätään host:in osoite sulkuihin perään
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
		// Poistetaan viimeisen numeron jälkeinen pilkku, pilkku tulee loopissa jokaisen indeksin perään, ja näin poppaamalla koodi on yksinkertaisempaa.
		indicesString.pop_back();
		return indicesString;
	}

	std::string makeFunctionAndAreaDescription(std::string functionName, const boost::shared_ptr<NFmiArea> &mapArea)
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
			// Lisätään tähän listaan serverin nimi perään (viimeinen parametri true)
			auto guiMapLayerName = ::getWmsMapLayerGuiName(static_cast<int>(mapLayerIndex), wmsMapLayerSetup, true);
			auto& usedLayerParsedServer = wmsMapLayerSetup.parsedServers[mapLayerIndex];
			NFmiMapLayerRelatedInfo mapLayerRelatedInfo{ guiMapLayerName, usedLayerParsedServer.macroReference, true };
			mapLayerRelatedInfos.push_back(mapLayerRelatedInfo);
		}
	}

	int calcWantedMapLayerIndex(const MapAreaMapLayerRelatedInfo& mapLayerRelatedInfos, const std::string& mapLayerName)
	{
		auto iter = std::find_if(mapLayerRelatedInfos.begin(), mapLayerRelatedInfos.end(),
			[&mapLayerName](const auto& mapLayerRelatedInfo) {return mapLayerRelatedInfo.guiName_ == mapLayerName; });
		if(iter != mapLayerRelatedInfos.end())
			return static_cast<int>(std::distance(mapLayerRelatedInfos.begin(), iter));
		else
			return 0; // Tämä on virhetilanne, palautetaan kuitenkin vain 0 indeksi (ei poikkeusta, vaikka pitäisi)
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

	// Kun tutkitaan löytyykö macro-referencen perusteella vastinetta, tehdään tarkastelu varmuuden vuoksi case-insensitiivisesti.
	int getMacroReferenceMapLayerIndex(const std::string& nonPrefixMacroReferenceName, const MapAreaMapLayerRelatedInfo& mapLayerRelatedInfos)
	{
		auto layerIter = std::find_if(mapLayerRelatedInfos.begin(), mapLayerRelatedInfos.end(),
			[&nonPrefixMacroReferenceName](const auto& mapLayerRelatedInfo) {return boost::iequals(nonPrefixMacroReferenceName, mapLayerRelatedInfo.macroReference_); });
		int layerIndex = (layerIter != mapLayerRelatedInfos.end()) ? static_cast<int>(std::distance(mapLayerRelatedInfos.begin(), layerIter)) : -1;
		return layerIndex;
	}

	// Tässä haetaan macro-referenssiä seuraavin keinoin ja prioriteetein:
	// 1. macroReferenceName ei saa olla tyhjä
	// 2. Tarkistetaan sisälsikö macroReferenceName wms prefixin "[wms]"
	// 3. Riippuen siitä onko originaali ollut wms pohjainen layer, tehdään seuraavaa:
	// 3.1. Jos wms pohjaisista löytyy vastaava macroReference nimi, palautetaan sen ja static layereiden yhteisindeksi
	// 3.2. Jos löytyy vain staattisista layereista vastaava macroReference nimi, palautetaan sen indeksi
	// 4. Jos originaali ei ollut wms pohjainen layer, tehdään seuraavaa:
	// 4.1. Jos löytyy staattisista layereista macroReference nimi, palautetaan sen indeksi
	// 4.2. Jos löytyy vain wms pohjaisista vastaava macroReference nimi, palautetaan sen ja static layereiden yhteisindeksi
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
		// Ei löytynyt mitään annetun macroReferenceName:n avulla, -1 on merkki siitä, että mitään ei tehdä.
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

	bool isGroundDataType(boost::shared_ptr<NFmiDrawParam>& drawParam)
	{
		return drawParam->Level().GetIdent() == 0;
	}

	bool checkLastObservationTime(bool *newerTimeFoundInOut, const NFmiMetTime & checkedTime, const NFmiMetTime& currentTime, NFmiMetTime *newLastTimeInOut, int timeStepInMinutes)
	{
		// Tarkasteltu aika pitää pyöristää animaatiossa käytetyn time-stepin kanssa ja vielä taaksepäin.
		NFmiMetTime backwardRoundedCheckTime(checkedTime);
		backwardRoundedCheckTime.SetTimeStep(timeStepInMinutes, true, kBackward);
		if(*newerTimeFoundInOut == false || backwardRoundedCheckTime > *newLastTimeInOut)
		{
			*newerTimeFoundInOut = true;
			*newLastTimeInOut = backwardRoundedCheckTime;
			if(*newLastTimeInOut >= currentTime)
			{
				// Ei tarvitse enää jatkaa, koska aika joka löytyi on viimeisin mahdollinen.
				return true;
			}
		}
		return false;
	}

	bool isObservationLockModeQueryData(const boost::shared_ptr<NFmiDrawParam>& drawParam)
	{
		if(CtrlViewFastInfoFunctions::IsObservationLockModeDataType(drawParam->DataType()))
		{
			// Ignooraa toistaiseksi tuottaja kFmiTEMP, koska niiden par haku tuottaa ajallisesti liian pitkälle meneviä datoja (viimeinen aika on tyhjää).
			if(!NFmiInfoOrganizer::IsTempData(drawParam->Param().GetProducer()->GetIdent(), true))
			{
				return true;
			}
		}
		return false;
	}

	bool isObservationLockModeSatelData(const boost::shared_ptr<NFmiDrawParam>& drawParam, bool ignoreSatelImages)
	{
		if(drawParam->DataType() == NFmiInfoData::kSatelData && ignoreSatelImages == false)
		{
			return true;
		}
		return false;
	}

	bool isObservationLockModeWmsData(const boost::shared_ptr<NFmiDrawParam>& drawParam)
	{
		if(drawParam->DataType() == NFmiInfoData::kWmsData && drawParam->TreatWmsLayerAsObservation())
		{
			return true;
		}
		return false;
	}

	void checkEarliestLastObservationTime(bool * anyTimeFoundInOut, const NFmiMetTime & checkedTime, NFmiMetTime* earliestLastTimeInOut, int timeStepInMinutes)
	{
		// Tarkasteltu aika pitää pyöristää animaatiossa käytetyn time-stepin kanssa ja vielä taaksepäin.
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
	// Pitää hankkiutua eroon NFmiPtrList -luokan käytöstä, silloin ei tarvitse erillisiä varatun muistin tuhoamiskomentoja
	crossSectionDrawParamListVector_->Clear(true);
	copyPasteDrawParamListVector_->Clear(true);
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

		int mapViewCount = ::getApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapViewCount();
		// MapView-indeksit alkavat 0:sta
		std::string baseSettingStr("MetEditor::MapView::");
		for(int mapViewIndex = 0; mapViewIndex < mapViewCount; mapViewIndex++)
			mapViewDescTops_.emplace_back(createMapViewDescTop(baseSettingStr, mapViewIndex));

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

std::vector<int> NFmiCombinedMapHandler::getCombinedModeSelectedMapIndicesFromWinRegistry(unsigned int mapViewDescTopIndex, bool doBackgroundMaps)
{
	auto mapViewWinRegistry = getApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(mapViewDescTopIndex);
	auto selectedMapIndicesStr = doBackgroundMaps ? mapViewWinRegistry->CombinedMapModeSelectedBackgroundIndices() : mapViewWinRegistry->CombinedMapModeSelectedOverlayIndices();
	return ::parseSelectedMapIndices(selectedMapIndicesStr);
}

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
			// Tehdään ensin background map indeksin asetus
			staticMapClientState.state_->setBackgroundIndex(::calcInitialWmsLayerIndex(getCombinedMapModeState(mapViewIndex, mapAreaIndex)));

			// Tehdään sitten overlay map indeksin asetus
			staticMapClientState.state_->setOverlayIndex(::calcInitialWmsLayerIndex(getCombinedOverlayMapModeState(mapViewIndex, mapAreaIndex)));
		}
	}
}

unsigned int NFmiCombinedMapHandler::getMapViewCount() const
{
	return static_cast<unsigned int>(mapViewDescTops_.size());
}

unsigned int NFmiCombinedMapHandler::getMapAreaCount() const
{
	return static_cast<unsigned int>(getMapViewDescTop(0)->GdiPlusImageMapHandlerList().size());
}

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

// Metodi saa 0-pohjaisen mapViewIndeksin (index), mutta se pitää muuttaa 1-pohjaiseksi, kun tehdään settings stringiä.
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
		// release versiolle on määrätty eri shape-file kuin debug versiolle, koska 
		// debug versio on tolkuttoman hidas laskiessaan koordinaatteja ja siksi
		// on parempi käyttää harvempaa dataa debug versiolle.
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

void NFmiCombinedMapHandler::doCutBorderDrawInitialization()
{
	{ 
		// tehdään omassa blokissa!!
		cutLandBorderPaths_.clear();
		auto &mapHandlerList = mapViewDescTops_[0]->GdiPlusImageMapHandlerList(); // otetaan pääkarttanäytön mapHandlerList
		// Ensin lasketaan eri karttapohjille leikatut rajaviivat
		for(auto *mapHandler : mapHandlerList)
		{
			boost::shared_ptr<NFmiArea> totalMapArea = mapHandler->TotalArea();
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

	// Sitten asetetaan kaikkien karttanäyttöjen kaikille karttapohjille leikatut rajaviivat
	for(const auto & mapViewDescTop : mapViewDescTops_)
	{
		std::vector<NFmiGdiPlusImageMapHandler*>& mapHandlerList = mapViewDescTop->GdiPlusImageMapHandlerList();
		for(size_t mapHandlerIndex = 0; mapHandlerIndex < mapHandlerList.size(); mapHandlerIndex++)
		{
			mapHandlerList[mapHandlerIndex]->LandBorderPath(cutLandBorderPaths_[mapHandlerIndex]);
		}
	}
}

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
				boost::shared_ptr<NFmiDrawParam> drawParam = aList->Current();
				// Ei tarvitse tarkastella mitenkään, jos drawParam layer on piilossa
				if(drawParam->IsParamHidden())
					continue;

				makeNormalDataDrawingLayerCahceChecks(mapViewDescTopIndex, fastInfo, dataType, dirtyViewTimes, fileName, drawParam, *dataProducer, *descTop, cacheRowNumber);
				if(dataType == NFmiInfoData::kEditable && (drawParam->DataType() == NFmiInfoData::kEditable || drawParam->DataType() == NFmiInfoData::kCopyOfEdited))
				{
					// jos kyseessä oli editoitavan datan päivitys, laitetaan uusiksi ne rivit missä on editoitavan datan ja sen kopion parametreja näkyvissä (tuottajalla ei ole väliä)
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
	// Kaikki warning/error/fatal tason viestit pitää flushata heti lokitiedostoon, jos ongelmista seuraa kaatuminen
	auto flushLogger = (flushAlways ? true : (severity > CatLog::Severity::Info));
	CatLog::logMessage(logMessage, severity, category, flushLogger);
}

// On kohdattu vakava virhe, laitetaan käyttäjälle kysely, että lopetetaanko suosiolla ohjelman ajo
void NFmiCombinedMapHandler::logAndWarnUser(const std::string& logMessage, const std::string& titleString, CatLog::Severity severity, CatLog::Category category, bool addAbortOption)
{
	// Lopun parametrit false (kysy käyttäjältä dialogilla) ja true (flushaa lokiviestit heti tiedostoon, jos vaikka kaatuu kohta)
	CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->LogAndWarnUser(logMessage, titleString, severity, category, false, addAbortOption, true);
}

// Tätä kutsutaan mm. CFmiMainFrame:n OnTimer:ista kerran minuutissa.
// Tarkistaa eri näyttöjen animaation tilan ja moodit.
// Päivittää tarvittaessa lukittujen moodien animaatio timebagit.
// 'Likaa' tarvittaessa uudet ajat cache:sta ja tekee ruudun päivitykset.
// mapViewDescTopIndex:illä voidaan antaa jos halutaan tarkistaa vain tietyn näytön päivitys tarve.
// Jos mapViewDescTopIndex:in arvo on kDoAllMapViewDescTopIndex, silloin tarkistus tehdään kaikille näytöille.
void NFmiCombinedMapHandler::checkAnimationLockedModeTimeBags(unsigned int mapViewDescTopIndex, bool ignoreSatelImages)
{
	if(isAnimationTimebagCheckNeeded(mapViewDescTopIndex))
	{ // edellinen metodi tarkisti, onko jossain animaatio boksi näkyvissä.
		bool needToUpdateViews = false;
		// tutkitaan eri näyttöjen animaattoreita ja niiden tiloja
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
					NFmiMetTime currentTime(1); // otetaan vielä seinäkelloaika
					if(currentTime < lastTime || currentTime.DifferenceInMinutes(lastTime) > g_StatObservationSeekTimeLimitInMinutes)
					{ // pitää fiksata viimeistä aikaa, koska se ei oikeastaan voi olla tulevaisuudessa, tai joku havainto on tulevaisuudesta
						// Lisäksi pitää fiksata jos alkuaika oli liian kaukana menneisyydessä, turha käydä läpi esim. viikon verran dataa ja etsi viimeistä
						lastTime = currentTime;
						lastTime.SetTimeStep(static_cast<short>(animData.TimeStepInMinutes()));
						lastTime.ChangeByHours(-2); // lähdetään etsimää viimeistä havaintoa 3 tuntia menneisyydestä
					}
					NFmiMetTime newLastTime(animData.TimeStepInMinutes()); // tähän sijoitetaan se aika, josta löytyi viimeinen näytössä olevan parametrin havainto data
					bool foundObservations = false;
					if(lockMode == NFmiAnimationData::kFollowLastObservation)
						foundObservations = findLastObservation(static_cast<unsigned long>(checkedDescTopIndex), animData.TimeStepInMinutes(), newLastTime, ignoreSatelImages);
					else
						foundObservations = findEarliestLastObservation(static_cast<unsigned long>(checkedDescTopIndex), animData.TimeStepInMinutes(), newLastTime, ignoreSatelImages);

					if(foundObservations)
					{ // löytyi jotain ennusteita, jossa uusi lastTime (=newLastTime), joten voidaan päivittää animaation timebagi.
						int timeDiffInMinutes = animData.Times().LastTime().DifferenceInMinutes(animData.Times().FirstTime());
						NFmiMetTime newFirstTime(newLastTime);
						newFirstTime.ChangeByMinutes(-timeDiffInMinutes);
						NFmiTimeBag newAnimTimes(newFirstTime, newLastTime, animData.TimeStepInMinutes());
						NFmiTimeBag oldAnimTimes = animData.Times();
						animData.Times(newAnimTimes);
						// Laitetaan myös aikakontrolli-ikkuna seuraavaan animaatio ikkunaa, eli keskitetään aikaikkuna animaatioaikojen keskikohtaan
						int animDiffInMinutes = newAnimTimes.LastTime().DifferenceInMinutes(newAnimTimes.FirstTime());
						NFmiMetTime animMiddleTime(newAnimTimes.FirstTime());
						animMiddleTime.ChangeByMinutes(boost::math::lround(animDiffInMinutes / 2.));
						animMiddleTime.SetTimeStep(animData.TimeStepInMinutes());
						centerTimeControlView(static_cast<unsigned long>(checkedDescTopIndex), animMiddleTime, false);
						// Lisäksi likaa näyttö cachesta uudet löytyneet ajat
						::makeNewTimesDirtyFromViewCache(checkedMapViewDescTop, oldAnimTimes, newAnimTimes);
						mapViewDirty(static_cast<unsigned long>(checkedDescTopIndex), false, false, true, false, false, false); // tämän pitäisi asettaa näyttö päivitys tilaan, mutta cachea ei tarvitse enää erikseen tyhjentää
					}
				}
			}
		}

		if(needToUpdateViews)
			ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Animation related update because locked time mode event occured", getUpdatedViewIdMaskForChangingTime()); // vielä päivitetään näytöt
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
			// Eli ei palauteta defaultti arvoa eli pääikkunan mapViewDescToppia, jos kyse oli ei-karttanäytön indeksistä
			return nullptr;
		}
		else
		{
			// Palautetaan erikoisnäyttöjä (aikasarja-, poikkileikkaus-, luotaus-, datanmuokkaus-dialogit, jne.) varten pääkarttanaytön desctop
			return mapViewDescTops_[0].get();
		}
	}
}

void NFmiCombinedMapHandler::mapViewDirty(unsigned int mapViewDescTopIndex, bool makeNewBackgroundBitmap, bool clearMapViewBitmapCacheRows, bool redrawMapView, bool doClearMacroParamDataCache, bool clearEditedDataDependentCaches, bool updateMapViewDrawingLayers)
{
	if(redrawMapView)
	{
		// Ainakin toistaiseksi laitan aikasarjan likauksen tänne
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
		} // Jos tätä kutsutaan esim. poikkileikkaus näytölle, lentää poikkeus, mikä on täysin ok

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
		} // Jos tätä kutsutaan esim. poikkileikkaus näytölle, lentää poikkeus, mikä on täysin ok

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
		} // Jos tätä kutsutaan esim. poikkileikkaus näytölle, lentää poikkeus, mikä on täysin ok
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

bool NFmiCombinedMapHandler::makeMacroParamDrawingLayerCacheChecks(boost::shared_ptr<NFmiDrawParam>& drawParam, NFmiFastQueryInfo& fastInfo, NFmiInfoData::Type dataType, NFmiMapViewDescTop& descTop, unsigned int mapViewDescTopIndex, int cacheRowNumber, const std::string& fileName)
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
				// Jos macroParamDataInfo:n level on 'tyhjä' (ident = 0, tarkoittaa pinta parametria) tai jos annettu level löytyy infosta
				if(macroParamDataInfo.level_.GetIdent() == 0 || fastInfo.Level(macroParamDataInfo.level_))
				{
					if(doMacroParamVerticalDataChecks(fastInfo, dataType, macroParamDataInfo))
					{
						// clean image cache row
						descTop.MapViewCache().MakeRowDirty(cacheRowNumber);
						// MacroParam data cachen rivit alkavat 1:stä, joten image-cachen riviin on lisättävä +1
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

std::string NFmiCombinedMapHandler::doGetMacroParamFormula(boost::shared_ptr<NFmiDrawParam>& drawParam, CatLog::Category category)
{
	static std::set<std::string> reportedMissingMacroParams;

	const auto& initFileName = drawParam->InitFileName();
	std::string logErrorMessage;
	try
	{
		return CtrlViewUtils::GetMacroParamFormula(getMacroParamSystem(), drawParam);
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
			// Vaihdetaan ensin uusi tmp-olioon, oletetaan että edellinen vanha on jo ehditty tappaa ja tuhota pois alta.
			tmpWmsSupport_.swap(wmsSupportPtr);

			// Jos vanha tmp-Wms-support olio on vielä hengissä, sen pakotetusta taposta tämän metodin loppuessa tulee varmaan ongelmia, lokitetaan siitä
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
				// Jos oli jo käytössä wmsSupport olio, sitä pitää alkaa tappamaan
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
	// Tarkistetaan ettei systeemi ole jo käynnissä
	if(!isWmsSupportRenewalProcessRunningMutex_.try_lock())
	{
		logMessage("Wms-support renewal process was already running, stopping this call", CatLog::Severity::Warning, CatLog::Category::Operational, true);
	}
	else
	{
		// Avaan lukon ja laitan sen lock_guard:iin, jotta ei tarvitse laittaan kaikkiin return kohtiin erillistä avausta
		isWmsSupportRenewalProcessRunningMutex_.unlock();
		std::lock_guard<std::mutex> lock(isWmsSupportRenewalProcessRunningMutex_);

		auto wmsSupportPtr = createWmsSupport(creationName);
		if(wmsSupportPtr)
		{
			for(;;)
			{
				// Odotetaan että juuri luotu wmsSupport otus saa tehtyä getcapabilities haut ennen kuin otus otetaan käyttöön
				if(isWmsSupportBeenKilled())
				{
					// Jos smartmetia ollaan sulkemassa, voidaan tämä odottelu lopettaa ja sijoitetaan wmsSupportPtr odottelemaan 
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
				// Tässä laitetaan threadi nukkumaan pieneksi aikaa, jotta tarkasteluja voidaan sitten jatkaa
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}
			// Nyt täysin toiminta valmis wmsSupport olio voidaan ottaa käyttöön
			setWmsSupport(wmsSupportPtr);
			if(tmpWmsSupport_)
			{
				// Lopuksi odotellaan että tmp-wms-support olio tappaa itsensä ja että se voidaan poistaa ja nollata
				for(;;)
				{
					if(isWmsSupportBeenKilled())
					{
						// Jos smartmetia ollaan sulkemassa, voidaan tämä odottelu lopettaa
						return;
					}

					// tässä pika tarkastus (0 lukko aika), voidaanko nollata
					{
						std::lock_guard<std::mutex> lock(wmsSupportMutex_);
						if(tmpWmsSupport_->isDead(std::chrono::milliseconds(0)))
						{
							tmpWmsSupport_.reset();
							logMessage("Replaced Wms-support object has been killed and disposed", CatLog::Severity::Debug, CatLog::Category::Operational, true);
							break;
						}
					}
					// Tässä laitetaan threadi nukkumaan pieneksi aikaa, jotta tarkasteluja voidaan sitten jatkaa
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

bool NFmiCombinedMapHandler::findLastObservation(unsigned long mapViewDescTopIndex, int timeStepInMinutes, NFmiMetTime& newLastTime, bool ignoreSatelImages)
{
	// tämä on jonkinlainen rajapyykki, eli tämän yli kun mennään (ei pitäisi mennä), lopetetaan havaintojen etsiminene siihen
	NFmiMetTime currentTime(1);
	// haetaan min ja maksimi aika limitit, jotka on n. nykyhetki ja 2h - nykyhetki
	NFmiMetTime timeLimit1;
	NFmiMetTime timeLimit2;
	::makeObsSeekTimeLimits(currentTime, mapViewDescTopIndex, timeLimit1, timeLimit2);

	NFmiMapViewDescTop& mapViewDescTop = *(getMapViewDescTop(mapViewDescTopIndex));
	NFmiPtrList<NFmiDrawParamList>::Iterator iter = mapViewDescTop.DrawParamListVector()->Start();
	bool newerTimeFound = false;
	int viewRowIndex = 1;
	for(; iter.Next(); viewRowIndex++)
	{
		if(mapViewDescTop.IsVisibleRow(viewRowIndex) == false)
			continue; // ei käydä läpi piilossa olevia rivejä
		NFmiDrawParamList* aList = iter.CurrentPtr();
		if(aList)
		{
			for(aList->Reset(); aList->Next(); )
			{
				boost::shared_ptr<NFmiDrawParam> drawParam = aList->Current();
				if(drawParam->IsParamHidden() == false)
				{
					if(::isObservationLockModeQueryData(drawParam))
					{
						std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector;
						makeDrawedInfoVectorForMapView(infoVector, drawParam, mapViewDescTop.MapHandler()->Area());
						for(size_t i = 0; i < infoVector.size(); i++)
						{
							bool demandExactTimeChecking = drawParam->DataType() != NFmiInfoData::kFlashData; // tässä vaiheessa salama data on sellainen jossa ei vaadita tarkkoja aika tarkasteluja
							boost::shared_ptr<NFmiFastQueryInfo>& info = infoVector[i];
							NFmiMetTime dataLastTime;
							NFmiTimeDescriptor timeDesc = info->TimeDescriptor();
							if(::getLatestValidTimeWithCorrectTimeStep(timeDesc, mapViewDescTopIndex, demandExactTimeChecking, timeLimit1, timeLimit2, dataLastTime))
							{
								if(::checkLastObservationTime(&newerTimeFound, dataLastTime, currentTime, &newLastTime, timeStepInMinutes))
								{
									// Ei tarvitse enää jatkaa, koska aika joka löytyi on viimeisin mahdollinen.
									return true;
								}
							}
						}
					}
					else if(::isObservationLockModeSatelData(drawParam, ignoreSatelImages))
					{ 
						// tutki löytyykö satel-data hakemistosta uudempia datoja, kuin annettu theLastTime
						NFmiMetTime satelLastTime;
						if(::getLatestSatelImageTime(drawParam->Param(), satelLastTime))
						{
							if(::checkLastObservationTime(&newerTimeFound, satelLastTime, currentTime, &newLastTime, timeStepInMinutes))
							{
								// Ei tarvitse enää jatkaa, koska aika joka löytyi on viimeisin mahdollinen.
								return true;
							}
						}
					}
					else if(::isObservationLockModeWmsData(drawParam))
					{ 
						// tutki löytyykö wms-datasta aikoja datoja, kuin annettu theLastTime
						NFmiMetTime wmsLastTime;
						if(getLatestWmsImageTime(drawParam->Param(), wmsLastTime))
						{
							if(::checkLastObservationTime(&newerTimeFound, wmsLastTime, currentTime, &newLastTime, timeStepInMinutes))
							{
								// Ei tarvitse enää jatkaa, koska aika joka löytyi on viimeisin mahdollinen.
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

// Tutkii käikki näkyvät havainto datat ja etsii sen ajan, mikä on aikaisin eri datojen viimeisistä ajoista.
// Tällä on pyrkimys siihen että animaatio ei välky, kun kaikilta datoilta periaattessa löytyy dataa myös
// tällä tavalla etsittyyn viimeiseen aikaan.
bool NFmiCombinedMapHandler::findEarliestLastObservation(unsigned long mapViewDescTopIndex, int timeStepInMinutes, NFmiMetTime& newLastTime, bool ignoreSatelImages)
{
	NFmiMetTime currentTime(mapViewDescTopIndex); // tämä on jonkinlainen rajapyykki, eli tämän yli kun mennään (ei pitäisi mennä), lopetetaan havaintojen etsiminene siihen
	// haetaan min ja maksimi aika limitit, jotka on n. nykyhetki ja 2h - nykyhetki
	NFmiMetTime timeLimit1;
	NFmiMetTime timeLimit2;
	::makeObsSeekTimeLimits(currentTime, mapViewDescTopIndex, timeLimit1, timeLimit2);

	NFmiMapViewDescTop& mapViewDescTop = *(getMapViewDescTop(mapViewDescTopIndex));
	NFmiPtrList<NFmiDrawParamList>* drawParamListVector = mapViewDescTop.DrawParamListVector();
	NFmiPtrList<NFmiDrawParamList>::Iterator iter = drawParamListVector->Start();
	bool anyTimeFound = false;
	NFmiMetTime earliestLastTime(2200, 1, 1); // tähän vain iso tulevaisuuden luku
	int viewRowIndex = 1;
	for(; iter.Next(); viewRowIndex++)
	{
		if(mapViewDescTop.IsVisibleRow(viewRowIndex) == false)
			continue; // ei käydä läpi piilossa olevia rivejä
		NFmiDrawParamList* aList = iter.CurrentPtr();
		if(aList)
		{
			for(aList->Reset(); aList->Next(); )
			{
				boost::shared_ptr<NFmiDrawParam> drawParam = aList->Current();
				if(drawParam->IsParamHidden() == false)
				{
					if(::isObservationLockModeQueryData(drawParam))
					{
						std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector;
						makeDrawedInfoVectorForMapView(infoVector, drawParam, mapViewDescTop.MapHandler()->Area());
						NFmiMetTime lastTimeOfThisDataType; // mm. synop datan tapauksessa haetaan ehkä jopa 6:sta datat tiedostosta viimeisintä aikaa
						bool lastTimeOfThisDataTypeFoundYet = false;
						for(size_t i = 0; i < infoVector.size(); i++)
						{
							if(drawParam->DataType() != NFmiInfoData::kFlashData) // Salamadatoja ei oteta "Latest mutual time" tarkasteluihin
							{
								bool demandExactTimeChecking = true;
								boost::shared_ptr<NFmiFastQueryInfo>& info = infoVector[i];
								NFmiMetTime dataLastTime;
								NFmiTimeDescriptor timeDesc = info->TimeDescriptor();
								if(::getLatestValidTimeWithCorrectTimeStep(timeDesc, mapViewDescTopIndex, demandExactTimeChecking, timeLimit1, timeLimit2, dataLastTime))
								{
									if(lastTimeOfThisDataTypeFoundYet == false || dataLastTime > lastTimeOfThisDataType)
									{
										lastTimeOfThisDataTypeFoundYet = true;
										lastTimeOfThisDataType = dataLastTime;
									}
								}
							}
						}
						// tässä drawParam kohtaisten datojen aika tarkastelut
						if(lastTimeOfThisDataTypeFoundYet)
						{
							::checkEarliestLastObservationTime(&anyTimeFound, lastTimeOfThisDataType, &earliestLastTime, timeStepInMinutes);
						}
					}
					else if(::isObservationLockModeSatelData(drawParam, ignoreSatelImages))
					{
						// tutki löytyykö satel-data hakemistosta uudempia datoja, kuin annettu theLastTime
						NFmiMetTime satelLastTime;
						if(::getLatestSatelImageTime(drawParam->Param(), satelLastTime))
						{
							::checkEarliestLastObservationTime(&anyTimeFound, satelLastTime, &earliestLastTime, timeStepInMinutes);
						}
					}
					else if(::isObservationLockModeWmsData(drawParam))
					{
						// tutki löytyykö wms-datasta aikoja datoja, kuin annettu theLastTime
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

// Funktio säätää halutun aikakontrolli ikkunan siten että se keskittää annetun ajan näkyviin.
// Eli annettu aika menee aikakontrolli ikkunan keskelle. Halutessa myös päivitetään
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
	mapViewDirty(mapViewDescTopIndex, false, false, true, false, false, false); // tämän pitäisi asettaa näyttö päivitys tilaan, mutta cachea ei tarvitse enää erikseen tyhjentää
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
	// Jos tehdään jotain aika muutoksia mihinkään karttanäyttöön, laitetaan optimoitu update maski päälle
	ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(getUpdatedViewIdMaskForChangingTime());

	// Päivitetään ajan muutoksessa myös aina luotausnäyttöä, jos säädöt ovat kohdallaan
	if(::getMTATempSystem().GetSoundingViewSettingsFromWindowsRegisty().SoundingTimeLockWithMapView() && ::getMTATempSystem().TempViewOn())
		ApplicationInterface::GetApplicationInterfaceImplementation()->UpdateTempView();
	
	// Jos originaali ikkunassa on animaatio päällä tai on edes animointi alue näkyvissä, ei tehdä mitään
	auto& originalMapViewdescTop = *getMapViewDescTop(originalMapViewDescTopIndex);
	if(originalMapViewdescTop.AnimationDataRef().AnimationOn() || originalMapViewdescTop.AnimationDataRef().ShowTimesOnTimeControl())
		return;

	// eli jos origIndex oli pääikkuna (index = 0) tai apuikkuna oli lukittu pääikkunaan, silloin tehdään aika päivityksiä
	if(originalMapViewDescTopIndex == 0 || originalMapViewdescTop.LockToMainMapViewTime())
	{
		for(unsigned int currentMapViewDescTopIndex = 0; currentMapViewDescTopIndex < mapViewDescTops_.size(); currentMapViewDescTopIndex++)
		{
			auto& currentMapViewdescTop = *getMapViewDescTop(currentMapViewDescTopIndex);
			if(currentMapViewDescTopIndex == 0 || currentMapViewdescTop.LockToMainMapViewTime())
			{
				// jos läpikäytävässä ikkunassa on animointia tai edes anim.-ikkuna näkyvissä, ei tehdä mitään
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
					boost::shared_ptr<NFmiDrawParam> drawParam = aList->Current();
					if(isMacroParamDependentOfEditedData(drawParam))
					{
						macroParamPathList.push_back(drawParam->InitFileName());
						// Tyhjennetään myös kyseisten näyttörivien bitmap cache (ikävä kaksois vastuu metodilla)
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

bool NFmiCombinedMapHandler::isMacroParamDependentOfEditedData(boost::shared_ptr<NFmiDrawParam>& drawParam)
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

// Tyhjennetään bitmap cached eri karttanäytöistä niiltä riveiltä, missä on editoitua dataa katseltavana.
// Lisäksi liataan aikasarja ikkuna, jos siellä on editoitua parametria valittuna.
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

	// Lopuksi vielä likaus aikasarjaan, jos tarvis
	if(::drawParamListContainsEditedData(*CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->TimeSerialViewDrawParamList()))
		timeSerialViewDirty_ = true;
}

void NFmiCombinedMapHandler::makeDrawedInfoVectorForMapView(std::vector<boost::shared_ptr<NFmiFastQueryInfo> >& infoVectorOut, boost::shared_ptr<NFmiDrawParam>& drawParam, const boost::shared_ptr<NFmiArea>& area)
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
	{ // synop-data on aluksi poikkeus, mille tehdään vektori, missä useita infoja, jos niitä löytyy
		if(producerId == NFmiInfoData::kFmiSpSynoXProducer)
			infoVectorOut = ::getSortedSynopInfoVector(kFmiSYNOP, kFmiTestBed);
		else
			infoVectorOut = ::getSortedSynopInfoVector(kFmiSYNOP, kFmiTestBed, kFmiSHIP, kFmiBUOY);
		for(auto fastInfo : infoVectorOut)
			fastInfo->Param(static_cast<FmiParameterName>(paramId)); // Laitetaan kaikki synop-datat osoittamaan haluttua parametria
	}
	else
	{
		boost::shared_ptr<NFmiFastQueryInfo> info = infoOrganizer.Info(drawParam, false, false);
		if(info)
		{
			if(dataType == NFmiInfoData::kMacroParam || dataType == NFmiInfoData::kQ3MacroParam)
			{ 
				// makroparamille pitää säätää laskettavan hilan alue vastaamaan karttanäytön aluetta
				if(area)
				{
					NFmiExtraMacroParamData::SetUsedAreaForData(info, area.get());
				}
			}

			infoVectorOut.push_back(info);
		}
	}
	// Lopuksi pitää vielä säätää piirrettävien infojen maskit knomask-tilaan, koska 
	// olen poistanut ns. aktivationMaskin käytön ja info on voinut jäädä esim. selected-mask tilaan
	::setInfosMask(infoVectorOut, NFmiMetEditorTypes::kFmiNoMask);
}

// Tutkii vertikaali macroParam funktioiden kanssa että jos kyse on pressure datasta, että löytyykö 
// samalta tuottajalta myös hybrid dataa, missä on haluttu parametri.
// Kaikissa muissa tapauksissa palauttaa true (eli tehdään rivin cache likaus), paitsi jos data 
// pressure dataa ja löytyy vastaava hybrid data infoOrganizerista.
bool NFmiCombinedMapHandler::doMacroParamVerticalDataChecks(NFmiFastQueryInfo& info, NFmiInfoData::Type dataType, const MacroParamDataInfo& macroParamDataInfo)
{
	if(macroParamDataInfo.usedWithVerticalFunction_)
	{
		// Jos dataa käytetty vertikaali funktioiden kanssa, pitää siinä olla yli 2 leveliä, muuten sitä ei käytetä macroParam laskuissa
		if(info.SizeLevels() > 2)
		{
			if(dataType == NFmiInfoData::kViewable)
			{
				if(info.LevelType() == kFmiPressureLevel)
				{
					auto hybridData = ::getInfoOrganizer().Info(macroParamDataInfo.dataIdent_, nullptr, NFmiInfoData::kHybridData, false, true);
					if(hybridData)
						return false; // löytyi vastaava hybrid data, eli ei tehdä rivin päivitystä tälle datalle
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
	if(timeStepInMinutes == 0) // ei voi olla 0 timesteppi, muuten kaatuu (negatiivisesta en tiedä)
		timeStepInMinutes = 60; // hätä korjaus defaultti arvoksi jos oli 0
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

void NFmiCombinedMapHandler::setSelectedMap(unsigned int mapViewDescTopIndex, int newMapIndex)
{
	// newMapIndex pitää ensin asettaa mapViewDescTopiin!
	auto& mapViewDescTop = *getMapViewDescTop(mapViewDescTopIndex);
	mapViewDescTop.SelectedMapIndex(newMapIndex);
	// Sen arvo saattaa muuttua ja lopullinen arvo pitää tallettaa myös Windows rekistereihin
	::getApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(mapViewDescTopIndex)->SelectedMapIndex(mapViewDescTop.SelectedMapIndex());

	setMapArea(mapViewDescTopIndex, mapViewDescTop.MapHandler()->Area());
}

void NFmiCombinedMapHandler::setSelectedMapHandler(unsigned int mapViewDescTopIndex, unsigned int newMapIndex)
{
	setSelectedMap(mapViewDescTopIndex, newMapIndex);
	mapViewDirty(mapViewDescTopIndex, true, true, true, false, false, false);
	ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(GetWantedMapViewIdFlag(mapViewDescTopIndex));
}

void NFmiCombinedMapHandler::setMapArea(unsigned int mapViewDescTopIndex, const boost::shared_ptr<NFmiArea>& newArea)
{
	if(newArea)
	{
		auto *mapDescTop = getMapViewDescTop(mapViewDescTopIndex);
		auto& totalArea = mapDescTop->MapHandler()->TotalArea();
		// tähän laitetaan tarkistus, että zoomi area ja karttapohjan areat ovat saman tyyppiset
		// Jos eivät ole, tehdään mahd. samanlainen area, mikä newArea on, mutta mikä sopii
		// käytössä olevan dipmaphandlerin kartta pohjaan
		if(NFmiQueryDataUtil::AreAreasSameKind(newArea.get(), totalArea.get()))
		{
			mapDescTop->MapHandler()->Area(newArea);
			::SetCPCropGridSettings(newArea, mapViewDescTopIndex);
		}
		else
		{ // tehdään sitten karttapohjalle sopiva area
			boost::shared_ptr<NFmiArea> correctTypeArea(totalArea->NewArea(newArea->BottomLeftLatLon(), newArea->TopRightLatLon()));
			if(correctTypeArea)
			{
				if(!totalArea->IsInside(*correctTypeArea))
				{ // pitää vähän viilata areaa, koska se ei mene kartta-alueen sisälle
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
					correctTypeArea = boost::shared_ptr<NFmiArea>(totalArea->NewArea(blLatlon, trLatlon));
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
	// Tämä on ns. brute force ratkaisu ei-queryData pohjaisten datojen päivitys tarpeelle.
	// Jos näytön rivillä on jotain tälläistä dataa, 'liataan' aina koko rivi varmuuden vuoksi.
	// HUOM! jos jossain karttanäytössä on animaattori päällä, ei tehdä likausta, koska animaatioidulle datalle oli omia tarkastuksia.

	// 1. Tarkita onko karttanäytön näyttöriveillä käsiteanalyysi datoja
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
				boost::shared_ptr<NFmiDrawParam> drawParam = aList->Current();
				if(!drawParam->IsParamHidden())
				{
					NFmiInfoData::Type dataType = drawParam->DataType();
					if(dataType == NFmiInfoData::kConceptualModelData || dataType == NFmiInfoData::kCapData)
					{
						mapViewDirty(mapViewDescTopIndex, false, false, true, false, false, false); // liataan mapView, mutta ei vielä tällä rivillä cachea (menisi kaikki rivit kerralla)
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
	// Tehdään ensin theOrigDescTopIndex:iin liittyvän mapView päivitys, koska tätä funktiota 
	// käytetään monista rivin vaihtoon liittyvistä funktioista, on se hyvä saada yhteen paikkaan suoritetuksi yhteisesti.
	CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(originalMapViewDescTopIndex);
	auto* originalMapViewDescTop = getMapViewDescTop(originalMapViewDescTopIndex);
	// eli jos origIndex oli pääikkuna (index = 0) tai apuikkuna oli lukittu pääikkunaan, silloin tehdään rivi päivityksiä
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
		return getMapViewDescTop(0)->CurrentTime(); // erikois näytöille palautetaan vain pääkarttaikkunan valittu aika
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
		// Peittääkö animaatio ajat koko aikakontrolli-ikkunan
		if(animationTimes.IsInside(timeControlTimes->FirstTime()) && animationTimes.IsInside(timeControlTimes->LastTime()))
			return true;
		// Onko animaatio ajat kokonaan aikakontrolli-ikkunassa
		if(timeControlTimes->IsInside(animationTimes.FirstTime()) && timeControlTimes->IsInside(animationTimes.LastTime()))
			return true;
		// Onko animaation 1. aika on selkeästi aikakontrolli-ikkunan sisällä
		if(timeControlTimes->IsInside(animationTimes.FirstTime()) && timeControlTimes->FirstTime() < animationTimes.FirstTime() && timeControlTimes->LastTime() > animationTimes.FirstTime())
			return true;
		// Onko animaation viimeinen aika on selkeästi aikakontrolli-ikkunan sisällä
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
// käy läpi kaikki kartalla näkyvät datat ja asettaa ne oikeaan aikaan
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
// käy läpi kaikki kartalla näkyvät datat ja asettaa ne oikeaan aikaan
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

// Tätä ei saa asettaa GenDocin ulkoa suoraan, tämän asetukset tapahtuvat tämän 
// luokan sisällä, joten tämä ei ole julkinen metodi
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

// funktio palauttaa oikean rivi numero. Karttanäyttö systeemissä on ikävä virtuaali rivitys
// jolloin karttanäyttö-luokka voi luulla olevansa rivillä 1 (= yli karttanäytössä oleva rivi)
// vaikka onkin oikeasti vaikka rivillä 4. Tällöin pitää käyttää apuna desctopin tietoja että 
// voidaan laskea oikea karttarivi.
// Oikeat karttarivit alkavat siis 1:stä.
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

// muuta käyttämään DrawParamListWithRealRowNumber-funktiota
NFmiDrawParamList* NFmiCombinedMapHandler::getDrawParamList(unsigned int mapViewDescTopIndex, int rowIndex)
{
	if(mapViewDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
		return getCrossSectionViewDrawParamList(rowIndex);
	if(mapViewDescTopIndex == CtrlViewUtils::kFmiTimeSerialView || rowIndex >= CtrlViewUtils::kFmiTimeSerialView) // haetaan aikasarjaikkunan drawparamlistia CtrlViewUtils::kFmiTimeSerialView on suuri luku (99)
		return &getTimeSerialViewDrawParamList();
	else
		return getDrawParamListWithRealRowNumber(mapViewDescTopIndex, getRealRowNumber(mapViewDescTopIndex, rowIndex));
}

// Tämä on otettu käyttöön ,että voisi unohtaa tuon kamalan indeksi jupinan, mikä johtuu
// 'virtuaali' karttanäyttöriveistä.
// Karttarivi indeksit alkavat 1:stä. 1. rivi on 1 ja 2. rivi on kaksi jne.
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

boost::shared_ptr<NFmiDrawParam> NFmiCombinedMapHandler::activeDrawParamFromActiveRow(unsigned int mapViewDescTopIndex)
{
	return activeDrawParamWithRealRowNumber(mapViewDescTopIndex, absoluteActiveViewRow(mapViewDescTopIndex));
}

// Tämä on otettu käyttöön ,että voisi unohtaa tuon kamalan indeksi jupinan, mikä johtuu
// 'virtuaali' karttanäyttöriveistä.
// Karttarivi indeksit alkavat 1:stä. 1. rivi on 1 ja 2. rivi on kaksi jne.
boost::shared_ptr<NFmiDrawParam> NFmiCombinedMapHandler::activeDrawParamWithRealRowNumber(unsigned int mapViewDescTopIndex, int realRowIndex)
{
	NFmiDrawParamList* drawParamList = getDrawParamListWithRealRowNumber(mapViewDescTopIndex, realRowIndex);
	if(drawParamList)
	{
		for(drawParamList->Reset(); drawParamList->Next(); )
			if(drawParamList->Current()->IsActive())
				return drawParamList->Current();
	}
	return boost::shared_ptr<NFmiDrawParam>();
}

// asettaa zoomausalueen riippuvaiseksi yhden karttaikkunan x/y-suhteesta
void NFmiCombinedMapHandler::doAutoZoom(unsigned int mapViewDescTopIndex)
{
	if(::getApplicationWinRegistry().KeepMapAspectRatio())
	{
		auto* mapViewDescTop = getMapViewDescTop(mapViewDescTopIndex);
		boost::shared_ptr<NFmiArea> oldArea = mapViewDescTop->MapHandler()->Area();
		if(oldArea)
		{
			double currentAreaAspectRatio = oldArea->WorldXYAspectRatio();
			double clientAspectRatio = mapViewDescTop->ClientViewXperYRatio();
			if(CtrlViewUtils::IsEqualEnough(clientAspectRatio, currentAreaAspectRatio, 0.000001) == false) // Tässä pitäisi tutkia mikä on sellainen pieni arvo, jonka verran ratiot saavat heittää, ettei tätä kuitenkaan tarvitsisi laskea
			{ // pitää muuttaa zoomattua areaa niin, että sen aspectratio vastaa ikkunan aspectratiota
				boost::shared_ptr<NFmiArea> newArea(oldArea->CreateNewArea(mapViewDescTop->ClientViewXperYRatio(), kCenter, true));
				if(newArea)
				{
					setMapArea(mapViewDescTopIndex, newArea);
				}
			}
		}
	}
}

void NFmiCombinedMapHandler::makeViewRowDirtyActions(unsigned int mapViewDescTopIndex, int realRowIndex, NFmiDrawParamList* drawParamList)
{
	auto bitmapCacheRowIndex = realRowIndex - 1;
	try
	{
		getMapViewDescTop(mapViewDescTopIndex)->MapViewCache().MakeRowDirty(bitmapCacheRowIndex);
	}
	catch(...)
	{
	} // Jos jokin muu kuin karttanäyttö, MapViewDescTop(mapViewDescTopIndex) -kutsu heittää poikkeuksen ja se on ok tässä

	if(drawParamList)
		::getMacroParamDataCache().update(mapViewDescTopIndex, realRowIndex, *drawParamList);
	mapViewDirty(mapViewDescTopIndex, false, false, true, false, false, true);
}

void NFmiCombinedMapHandler::drawParamSettingsChangedDirtyActions(unsigned int mapViewDescTopIndex, int realRowIndex, boost::shared_ptr<NFmiDrawParam>& drawParam)
{
	auto bitmapCacheRowIndex = realRowIndex - 1;
	try
	{
		getMapViewDescTop(mapViewDescTopIndex)->MapViewCache().MakeRowDirty(bitmapCacheRowIndex);
	}
	catch(...)
	{
	} // Jos jokin muu kuin karttanäyttö, MapViewDescTop(mapViewDescTopIndex) -kutsu heittää poikkeuksen ja se on ok tässä

	if(drawParam && drawParam->IsMacroParamCase(true))
	{
		::getMacroParamDataCache().clearMacroParamCache(mapViewDescTopIndex, realRowIndex, drawParam->InitFileName());
	}
	mapViewDirty(mapViewDescTopIndex, false, false, true, false, false, true);
}

// Liataan vain 1. näkyvät karttarivit niistä karttanäytöistä, missä näytä-maski on päällä
void NFmiCombinedMapHandler::maskChangedDirtyActions()
{
	for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
	{
		auto desctop = getMapViewDescTop(mapViewDescTopIndex);
		if(desctop)
		{
			if(::getApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(mapViewDescTopIndex)->ShowMasksOnMap())
			{
				// Maskit ovat siis näkyvissä 1. relatiivisella rivillä
				unsigned int firstVisibleRowIndex = 1;
				auto cleanedCacheRowIndex = getRealRowNumber(mapViewDescTopIndex, firstVisibleRowIndex) - 1;
				desctop->MapViewCache().MakeRowDirty(cleanedCacheRowIndex);
				mapViewDirty(mapViewDescTopIndex, false, false, true, false, false, false);
			}
		}
	}
}

// Next/Previous fixedDrawParam vaihdetaan parametri-laatikosta hiiren rullalla SHIFT nappi pohjassa.
bool NFmiCombinedMapHandler::changeParamSettingsToNextFixedDrawParam(unsigned int mapViewDescTopIndex, int realRowIndex, int paramIndex, bool gotoNext)
{
	NFmiDrawParamList* drawParamList = getDrawParamListWithRealRowNumber(mapViewDescTopIndex, realRowIndex);
	if(drawParamList)
	{
		if(drawParamList->Index(paramIndex))
		{
			boost::shared_ptr<NFmiDrawParam> drawParam = drawParamList->Current();
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

// neljä tilaa:
// 0 = näytä aikakontrolliikkuna+teksti
// 1=vain aik.kont.ikkuna
// 2=älä näytä kumpaakaan
// 3= näytä vain teksti
// palauttaa currentin tilan
int NFmiCombinedMapHandler::toggleShowTimeOnMapMode(unsigned int mapViewDescTopIndex)
{
	return getMapViewDescTop(mapViewDescTopIndex)->ToggleShowTimeOnMapMode();
}

// aikasarja ikkunat asetetaan samalla likaiseksi kuin karttanäyttökin (ainakin toistaiseksi)
bool NFmiCombinedMapHandler::timeSerialViewDirty()
{
	return timeSerialViewDirty_;
}

void NFmiCombinedMapHandler::timeSerialViewDirty(bool newValue)
{
	timeSerialViewDirty_ = newValue;
}

void NFmiCombinedMapHandler::setMapViewCacheSize(double theNewSizeInMB)
{
	::getApplicationWinRegistry().MapViewCacheMaxSizeInMB(theNewSizeInMB);

	for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
		mapViewDescTops_[mapViewDescTopIndex]->MapViewCache().MaxSizeMB(theNewSizeInMB);
}

// typeOfChange 0 = minutes, 1 = 6hrs, 2 = day, 3 = week, 4 = month, 5 = year
// direction vaihtoehdot ovat kBackward ja kForward eli ajan siirto suunta
// mapViewDescTopIndex eli karttanäytön indeksi, joita on 3 kpl
// amountOfChange eli kuinka monta päivää/viikkoa/kuukautta jne. liikutaan ajassa eteen/taakse (yleensä 1)
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
		// huomioon eri pituisia kuukausia, siksi pitää tarkistaa että ei mene esim.
		// toukokuun 31. päivästä kesäkuun 31. päivään, jota ei ole olemassa.
		auto daysInMonth = NFmiTime::DaysInMonth(selectedTime.GetMonth(), selectedTime.GetYear());
		if(selectedTime.GetDay() > daysInMonth)
			selectedTime.SetDay(daysInMonth);
	}
	else if(typeOfChange == 5) // year
	{
		NFmiTimePerioid period(1,0,0,0,0,0);
		if(direction == kForward)
			selectedTime.NextMetTime(period);  // HUOM! näissä bugi, ei osaa ottaa huomioon eri pituisia kuukausia!!!!!
		else
			selectedTime.PreviousMetTime(period);  // HUOM! näissä bugi, ei osaa ottaa huomioon eri pituisia kuukausia!!!!!
	}

	centerTimeControlView(mapViewDescTopIndex, selectedTime, true); // tämä asettaa ajan, myös lukittuihin näyttöihin ja likaa näytön
	ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: Times changed by PageUp/Down keys");
	return true;
}

void NFmiCombinedMapHandler::makeMapViewRowDirty(int mapViewDescTopIndex, int viewRowIndex)
{
	// mapview cached alkaa 0:sta ja theViewRowIndex alkaa 1:stä
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

// Joitain arvoja säädetään suoraan NFmiMapViewDescTop-luokan läpi, joten näitä pitää päivittää takaisin rekistereihin
void NFmiCombinedMapHandler::storeMapViewSettingsToWinRegistry()
{
	for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
		getMapViewDescTop(mapViewDescTopIndex)->StoreToMapViewWinRegistry(*::getApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(mapViewDescTopIndex));
}

const std::unique_ptr<NFmiFastDrawParamList>& NFmiCombinedMapHandler::getModifiedPropertiesDrawParamList() const
{
	return modifiedPropertiesDrawParamList_;
}

// Päivittää drawParam -parametriin modifiedPropertiesDrawParamList_:ista mahdollisesti löytyvät asetukset.
void NFmiCombinedMapHandler::updateFromModifiedDrawParam(boost::shared_ptr<NFmiDrawParam>& drawParam, bool groundData)
{
	if(drawParam)
	{
		if(!drawParam->ViewMacroDrawParam()) // ei päivitetä ominaisuuksia modified listasta, jos oli viewmacro-drawparam
		{
			if(modifiedPropertiesDrawParamList_->Find(drawParam, groundData)) // 1999.08.30/Marko
				drawParam->Init(modifiedPropertiesDrawParamList_->Current());
			else
			{ 
				// ei löytynyt 'lisättävää' drawParamia listasta, joten lisätään tässä sen kopio modified-listaan (nykyään ei lisätä 
				// järjettömästi kaikkien datojen kaikki drawParam yhdistelmiä)
				boost::shared_ptr<NFmiDrawParam> tmpDrawParam(new NFmiDrawParam(*drawParam.get()));
				modifiedPropertiesDrawParamList_->Add(tmpDrawParam, groundData);
			}
		}
	}
}

// Tein tämän version joka muuttaa modifiedPropertiesDrawParamList_:in otusta annetulla drawParamilla.
// Lisäsin tähän myös annetun drawlist-rivin likaamisen.
void NFmiCombinedMapHandler::updateToModifiedDrawParam(unsigned int mapViewDescTopIndex, boost::shared_ptr<NFmiDrawParam>& drawParam, int viewRowIndex)
{
	if(drawParam)
	{
		if(!drawParam->ViewMacroDrawParam()) // jos kyseessä oli viewmacro-drawparam, ei päivitetä modified-listalla olevaa drawparamia!
		{
			boost::shared_ptr<NFmiFastQueryInfo> info = ::getInfoOrganizer().Info(drawParam, false, true);
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
				boost::shared_ptr<NFmiDrawParam> drawParam = aList->Current();
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
	checkAnimationLockedModeTimeBags(mapViewDescTopIndex, false); // kun parametrin näkyvyyttä vaihdetaan, pitää tehdä mahdollisesti animaatio moodin datan tarkistus
	makeWholeDesctopDirtyActions(mapViewDescTopIndex, nullptr);
}

boost::shared_ptr<NFmiDrawParam> NFmiCombinedMapHandler::getDrawParamFromViewLists(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	NFmiDrawParamList* drawParamList = getDrawParamList(menuItem.MapViewDescTopIndex(), viewRowIndex);
	if(!drawParamList)
		return boost::shared_ptr<NFmiDrawParam>();
	if(drawParamList->Index(menuItem.IndexInViewRow()))
		return drawParamList->Current();
	return boost::shared_ptr<NFmiDrawParam>();
}

void NFmiCombinedMapHandler::setModelRunOffset(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	auto modifiedDrawParam = getDrawParamFromViewLists(menuItem, viewRowIndex);
	setModelRunOffset(modifiedDrawParam, menuItem.CommandType(), menuItem.MapViewDescTopIndex(), viewRowIndex);
}

bool NFmiCombinedMapHandler::setModelRunOffset(boost::shared_ptr<NFmiDrawParam> &drawParam, FmiMenuCommandType command, unsigned int mapViewDescTopIndex, int viewRowIndex)
{
	if(drawParam)
	{
		auto oldModelRunIndex = drawParam->ModelRunIndex();
		if(command == kFmiModelRunOffsetPrevious)
		{
			drawParam->ModelOriginTime(NFmiMetTime::gMissingTime); // nollataan mahd. fiksattu origin aika
			drawParam->ModelRunIndex(drawParam->ModelRunIndex() - 1); // siirretään offset edelliseen aikaan
		}
		else if(command == kFmiModelRunOffsetNext)
		{
			drawParam->ModelOriginTime(NFmiMetTime::gMissingTime); // nollataan mahd. fiksattu origin aika
			drawParam->ModelRunIndex(drawParam->ModelRunIndex() + 1); // siirretään offset seuraavaan aikaan
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
			boost::shared_ptr<NFmiDrawParam> drawParam = activeDrawParamList->Current();
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

// Tämä on otettu käyttöön ,että voisi unohtaa tuon kamalan indeksi jupinan, mikä johtuu
// 'virtuaali' karttanäyttöriveistä.
// Karttarivi indeksit alkavat 1:stä. 1. rivi on 1 ja 2. rivi on kaksi jne.
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
			if(::getApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(menuItem.MapViewDescTopIndex())->ShowStationPlot())
			{
				// Jos karttanäytöllä näytetään aktiivisen datan pisteet, pitää tässä liata kaikki kuva cachet
				auto cacheRowIndex = getRealRowNumber(menuItem.MapViewDescTopIndex(), rowIndex) - 1;
				getMapViewDescTop(menuItem.MapViewDescTopIndex())->MapViewCache().MakeRowDirty(cacheRowIndex);
			}
			activeEditedParameterMayHaveChangedViewUpdateFlagSetting(menuItem.MapViewDescTopIndex());
			mapViewDirty(menuItem.MapViewDescTopIndex(), false, false, true, false, false, false);
		}
	}
}

static void DoSpecialDataInitializations(boost::shared_ptr<NFmiDrawParam>& drawParam, bool normalParameterAdd, const NFmiMenuItem& menuItem)
{
	// Näyttömakron latauksessa (normalParameterAdd = false) ei satelImagen alphaa saa enää laiteta  default 80%:iin, koska joku on saattanut säätää sen muuksi.
	if(menuItem.DataType() == NFmiInfoData::kSatelData && normalParameterAdd)
	{
		drawParam->Alpha(80.f); // laitetaan satelliitti/kuva tyyppiselle datalle defaulttina 80% opaque eli pikkuisen läpinäkyvä
	}
}

// Tämä on otettu käyttöön ,että voisi unohtaa tuon kamalan indeksi jupinan, mikä johtuu
// 'virtuaali' karttanäyttöriveistä.
// Karttarivi indeksit alkavat 1:stä. 1. rivi on 1 ja 2. rivi on kaksi jne.
// macroParamInitFileName on sitä varten että jos viewmacrosta ladataan macroparam, tähän pitää antaa init tiedoston nimi
// muuten macroparamin yhteydessä etsitään menuitemista annettua nimeä
// normalParameterAdd -parametrilla kerrotaan tuleeko normaali lisäys vai erilaisista viewmakroista lisäys. Tämä
// haluttiin erottaa vielä isViewMacroDrawParam:ista, jolla merkitään vain drawParamin ViewMacroDrawParam -asetus.
void NFmiCombinedMapHandler::addViewWithRealRowNumber(bool normalParameterAdd, const NFmiMenuItem& menuItem, int realRowIndex, bool isViewMacroDrawParam)
{
	auto& infoOrganizer = ::getInfoOrganizer();
	boost::shared_ptr<NFmiDrawParam> drawParam = infoOrganizer.CreateDrawParam(menuItem.DataIdent(), menuItem.Level(), menuItem.DataType());
	if(!drawParam)
		return; // HUOM!! Ei saisi mennä tähän!!!!!!!

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
	boost::shared_ptr<NFmiFastQueryInfo> info = infoOrganizer.Info(drawParam, false, true);
	NFmiDrawParamList* drawParamList = getDrawParamListWithRealRowNumber(menuItem.MapViewDescTopIndex(), realRowIndex);
	if(drawParamList)
	{
		std::string logStartStr(insertParamCase ? "Insert to specific point into map view " : "Added to map view ");
		if(changeParamCase)
			logStartStr = "Changed parameter to selected ";
		logParameterAction(logStartStr, menuItem, drawParam, info);

		// ChangeParam tapauksessa vaihdettava parametri on jo poistettu listasta, ja siksi tässä vain uusi samaan paikkaan
		if(insertParamCase || changeParamCase)
			drawParamList->Add(drawParam, menuItem.IndexInViewRow());
		else if(!normalParameterAdd)
		{
			// jos näyttö macrosta kyse, pitää parametri laittaa tarkalleen siihen mikä rivi oli
			// kyseessä (eli listan perään järjestyksessä). Tämä sen takia että satel-kanavat heitetään aina pohjalle ja
			// näyttömakroissa kaksi satelliitti kuvaa samalla rivillä aiheutti ongelmia.
			drawParamList->Add(drawParam, drawParamList->NumberOfItems() + 1);
		}
		else
			drawParamList->Add(drawParam); // laittaa parametrit listan perään, paitsi satel-kuvat laitetaan keulille (näin satelkuva ei peitä mahdollisia muita parametreja alleen)
	}

	bool groundData = ::isGroundDataType(drawParam);
	updateFromModifiedDrawParam(drawParam, groundData);
	drawParamSettingsChangedDirtyActions(menuItem.MapViewDescTopIndex(), realRowIndex, drawParam);
}

void NFmiCombinedMapHandler::logParameterAction(const std::string &parameterActionStart, const NFmiMenuItem& menuItem, const boost::shared_ptr<NFmiDrawParam> &drawParam, boost::shared_ptr<NFmiFastQueryInfo> &info)
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
		// Esim. satelliitti jutuissa tai jos ei löydy ladattavaa dataa, ei ole infoa, joten otetaan halutut tiedot muualta
		if(satelDataCase)
			logStr += menuItem.DataIdent().GetParamName();
		else
			logStr += getSelectedParamInfoString(&drawParam->Param(), &drawParam->Level());
	}
	logMessage(logStr, CatLog::Severity::Debug, CatLog::Category::Visualization);
}

void NFmiCombinedMapHandler::toggleShowDifferenceToOriginalData(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	boost::shared_ptr<NFmiDrawParam>& modifiedDrawParam = getDrawParamFromViewLists(menuItem, viewRowIndex);
	if(modifiedDrawParam)
	{
		modifiedDrawParam->ShowDifferenceToOriginalData(!(modifiedDrawParam->ShowDifferenceToOriginalData()));
		updateToModifiedDrawParam(menuItem.MapViewDescTopIndex(), modifiedDrawParam, viewRowIndex);
	}
}


void NFmiCombinedMapHandler::addView(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	// lasketaan todellinen rivinumero (johtuu karttanäytön virtuaali riveistä)
	addViewWithRealRowNumber(true, menuItem, getRealRowNumber(menuItem.MapViewDescTopIndex(), viewRowIndex), false);

	// lisään tämän CheckAnimationLockedModeTimeBags -kutsun vain perus AddView-metodin yhteyteen, mutta en esim.
	// AddViewWithRealRowNumber -metodin yhteyteen, että homma ei mene pelkäksi tarkasteluksi.
	// AddViewWithRealRowNumber -metodia käytetään varsin laajasti ja tarkasteluja tulisi tehtyä liikaa.
	if(CtrlViewFastInfoFunctions::IsObservationLockModeDataType(menuItem.DataType()) || menuItem.DataType() == NFmiInfoData::kSatelData)
		checkAnimationLockedModeTimeBags(menuItem.MapViewDescTopIndex(), false); // kun parametrin näkyvyyttä vaihdetaan, pitää tehdä mahdollisesti animaatio moodin datan tarkistus
}

void NFmiCombinedMapHandler::addCrossSectionView(const NFmiMenuItem& menuItem, int viewRowIndex, bool treatAsViewMacro)
{
	boost::shared_ptr<NFmiDrawParam> drawParam;
	auto& infoOrganizer = ::getInfoOrganizer();
	if(menuItem.DataType() == NFmiInfoData::kCrossSectionMacroParam)
		drawParam = infoOrganizer.CreateDrawParam(menuItem.DataIdent(), 0, menuItem.DataType());
	else
		drawParam = infoOrganizer.CreateCrossSectionDrawParam(menuItem.DataIdent(), menuItem.DataType());
	if(!drawParam)
		return; // HUOM!! Ei saisi mennä tähän!!!!!!!
	::setMacroParamDrawParamSettings(menuItem, drawParam);
	drawParam->ViewMacroDrawParam(treatAsViewMacro);

	boost::shared_ptr<NFmiFastQueryInfo> info = infoOrganizer.Info(drawParam, true, true);
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
		crossSectionViewDrawParamList->ActivateOnlyOne(); // varmistaa, että yksi ja vain yksi paramtri listassa on aktiivinen
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
				checkAnimationLockedModeTimeBags(mapViewDesctopIndex, false); // kun parametrin näkyvyyttä vaihdetaan, pitää tehdä mahdollisesti animaatio moodin datan tarkistus

			drawParamSettingsChangedDirtyActions(mapViewDesctopIndex, getRealRowNumber(mapViewDesctopIndex, viewRowIndex), boost::shared_ptr<NFmiDrawParam>());
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

NFmiInfoData::Type getFinalDataType(boost::shared_ptr<NFmiDrawParam>& drawParam, const NFmiProducer& givenProducer, bool useCrossSectionParams, bool fGroundData)
{
	auto& infoOrganizer = ::getInfoOrganizer();
	NFmiInfoData::Type finalDataType = drawParam->DataType();
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
						return info->DataType();
					}
				}
				else if(fGroundData && info->SizeLevels() == 1)
				{
					return info->DataType();
				}
			}
		}
	}

	boost::shared_ptr<NFmiFastQueryInfo> editedData = ::getEditedInfo();
	if(editedData && givenProducer == *(editedData->Producer()))
		return NFmiInfoData::kEditable;
	boost::shared_ptr<NFmiFastQueryInfo> operationalData = infoOrganizer.FindInfo(NFmiInfoData::kKepaData);
	if(operationalData && givenProducer == *(operationalData->Producer()))
		return NFmiInfoData::kKepaData;
	boost::shared_ptr<NFmiFastQueryInfo> helpData = infoOrganizer.FindInfo(NFmiInfoData::kEditingHelpData, 0);
	if(helpData && givenProducer == *(helpData->Producer()))
		return NFmiInfoData::kEditingHelpData;

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
				NFmiInfoData::Type finalDataType = ::getFinalDataType(drawParam, givenProducer, useCrossSectionParams, groundData); // pitää päättää vielä muutentun tuottajan datatyyppi
				// pitää hakea FindInfo:lla tuottajan mukaan dataa, josta saadaan oikea tuottaja (nimineen kaikkineen)
				boost::shared_ptr<NFmiFastQueryInfo> info = infoOrganizer.FindInfo(finalDataType, givenProducer, groundData);
				if(info)
					drawParam->Param().SetProducers(*info->Producer()); // pitää laittaa tuottaja datasta, koska tuottajan nimikin ratkaisee, kun haetaan dataa
				else // jos ei löytynyt dataa, tämä luultavasti menee pieleen, mutta laitetaan kuitenkin tuottaja kohdalleen
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
		// Pastettavan drawParamin pitää aina olla näkyvä, muuten tulee hämmennystä käyttäjissä!
		copyPasteDrawParam_->HideParam(false);
		copyPasteDrawParamAvailableYet_ = true;
	}
}

void NFmiCombinedMapHandler::pasteDrawParamOptions(const NFmiMenuItem& menuItem, int viewRowIndex, bool useCrossSectionParams)
{
	NFmiDrawParamList* wantedDrawParamList = getWantedDrawParamList(menuItem, viewRowIndex, useCrossSectionParams);
	if(wantedDrawParamList && wantedDrawParamList->Index(menuItem.IndexInViewRow()))
	{
		boost::shared_ptr<NFmiDrawParam> drawParam = wantedDrawParamList->Current();
		drawParam->Init(copyPasteDrawParam_.get(), true);
		wantedDrawParamList->Dirty(true);
		if(useCrossSectionParams == false)
			updateToModifiedDrawParam(menuItem.MapViewDescTopIndex(), drawParam, viewRowIndex);
	}
}

boost::shared_ptr<NFmiDrawParam> NFmiCombinedMapHandler::getUsedMapViewDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	NFmiDrawParamList* drawParamList = getDrawParamList(menuItem.MapViewDescTopIndex(), viewRowIndex);
	if(drawParamList && drawParamList->Index(menuItem.IndexInViewRow()))
		return drawParamList->Current();
	else
		return boost::shared_ptr<NFmiDrawParam>();
}

void NFmiCombinedMapHandler::copyMapViewDescTopParams(unsigned int mapViewDescTopIndex)
{
	NFmiPtrList<NFmiDrawParamList>* copiedDrawParamsList = getMapViewDescTop(mapViewDescTopIndex)->DrawParamListVector();
	if(copiedDrawParamsList)
	{
		copyPasteDrawParamListVectorUsedYet_ = true;
		CombinedMapHandlerInterface::copyDrawParamsList(copiedDrawParamsList, copyPasteDrawParamListVector_.get());
	}
}

void NFmiCombinedMapHandler::pasteMapViewDescTopParams(unsigned int mapViewDescTopIndex)
{
	NFmiPtrList<NFmiDrawParamList>* copiedDrawParamsList = getMapViewDescTop(mapViewDescTopIndex)->DrawParamListVector();
	if(copiedDrawParamsList)
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

void NFmiCombinedMapHandler::modifyCrossSectionDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	boost::shared_ptr<NFmiDrawParam> modifiedDrawParam = getCrosssectionDrawParamFromViewLists(menuItem, viewRowIndex);
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
	boost::shared_ptr<NFmiDrawParam> drawParam = getUsedMapViewDrawParam(menuItem, viewRowIndex);
	if(drawParam)
	{
		NFmiMetEditorTypes::View viewType = menuItem.ViewType();
		drawParam->ViewType(viewType);
		boost::shared_ptr<NFmiFastQueryInfo> info = ::getInfoOrganizer().Info(drawParam, false, true);
		if(info && info->IsGrid() == false)
			drawParam->StationDataViewType(viewType);
		else
			drawParam->GridDataPresentationStyle(viewType);

		updateToModifiedDrawParam(menuItem.MapViewDescTopIndex(), drawParam, viewRowIndex);
	}
}

void NFmiCombinedMapHandler::toggleShowLegendState(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	boost::shared_ptr<NFmiDrawParam> drawParam = getUsedMapViewDrawParam(menuItem, viewRowIndex);
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
		// HUOM! tosi rivi numerosta pitää vähentää 1, kun manipuloidaan bitmap cache rivejä!!!
		getMapViewDescTop(viewIndex)->MapViewCache().SwapRows(realRowNumber1 - 1, realRowNumber2 - 1);
		getMacroParamDataCache().swapMacroParamCacheRows(viewIndex, realRowNumber1, realRowNumber2);
		activeEditedParameterMayHaveChangedViewUpdateFlagSetting(menuItem.MapViewDescTopIndex());
		mapViewDirty(viewIndex, false, false, true, false, false, true);
	}
}

void NFmiCombinedMapHandler::saveDrawParamSettings(boost::shared_ptr<NFmiDrawParam> &drawParam)
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
			::getMacroParamSystem().ReloadDrawParamFromFile(initFileName);
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
	saveDrawParamSettings(getUsedMapViewDrawParam(menuItem, viewRowIndex));
}

void NFmiCombinedMapHandler::forceStationViewRowUpdate(unsigned int mapViewDescTopIndex, unsigned int theRealRowIndex)
{
	NFmiDrawParamList* drawParamList = getDrawParamListWithRealRowNumber(mapViewDescTopIndex, theRealRowIndex);
	if(drawParamList)
	{
		// Liataan haluttu drawParamList, jotta käytössä karttanäytössä ollut stationView päivittyy oikein tarvittaessa jos esim. isoline piirto vaihtuu teksti esitykseen.
		drawParamList->Dirty(true);
	}
}

void NFmiCombinedMapHandler::reloadDrawParamSettings(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	boost::shared_ptr<NFmiDrawParam> drawParam = getUsedMapViewDrawParam(menuItem, viewRowIndex);
	if(drawParam)
	{
		// Pitää ladata erikseen originaali drawParam asetukset omaan olioon ja sen avulla initialisoida käytössä olevan asetukset
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
	boost::shared_ptr<NFmiDrawParam> drawParam = getUsedMapViewDrawParam(menuItem, viewRowIndex);
	if(drawParam && fixedDrawParam)
	{
		drawParam->Init(fixedDrawParam.get(), true);
		updateToModifiedDrawParam(menuItem.MapViewDescTopIndex(), drawParam, viewRowIndex);
	}
}

boost::shared_ptr<NFmiDrawParam> NFmiCombinedMapHandler::getCrosssectionDrawParamFromViewLists(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	NFmiDrawParamList* drawParamList = getCrossSectionViewDrawParamList(viewRowIndex);
	if(!drawParamList)
		return boost::shared_ptr<NFmiDrawParam>();
	if(drawParamList->Index(menuItem.IndexInViewRow()))
		return drawParamList->Current();
	return boost::shared_ptr<NFmiDrawParam>();
}

void NFmiCombinedMapHandler::removeCrosssectionDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	NFmiDrawParamList* drawParamList = getCrossSectionViewDrawParamList(viewRowIndex);
	if(drawParamList && drawParamList->Index(menuItem.IndexInViewRow()))
	{
		drawParamList->Remove();
		drawParamSettingsChangedDirtyActions(menuItem.MapViewDescTopIndex(), getRealRowNumber(menuItem.MapViewDescTopIndex(), viewRowIndex), boost::shared_ptr<NFmiDrawParam>());
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
		{// deaktivoidaan piilotettu näyttöparametri (jos oli aktiivinen), ettei pensselillä yritetä sutia sitä vahingossa
			drawParamList->Current()->Activate(false);
			ActivateFirstNonHiddenViewParam(drawParamList);
		}
		drawParamList->Dirty(true);
		checkAnimationLockedModeTimeBags(mapViewDesctopIndex, false); // kun parametrin näkyvyyttä vaihdetaan, pitää tehdä mahdollisesti animaatio moodin datan tarkistus
		getMapViewDescTop(mapViewDesctopIndex)->MapViewCache().MakeRowDirty(getRealRowNumber(mapViewDesctopIndex, viewRowIndex));
		mapViewDirty(mapViewDesctopIndex, false, false, true, false, false, false);
	}
	updateToModifiedDrawParam(mapViewDesctopIndex, drawParamList->Current(), viewRowIndex);
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
		checkAnimationLockedModeTimeBags(mapViewDesctopIndex, false); // kun parametrin näkyvyyttä vaihdetaan, pitää tehdä mahdollisesti animaatio moodin datan tarkistus
		getMapViewDescTop(mapViewDesctopIndex)->MapViewCache().MakeRowDirty(getRealRowNumber(mapViewDesctopIndex, viewRowIndex));
		mapViewDirty(mapViewDesctopIndex, false, false, true, false, false, false);
	}
	updateToModifiedDrawParam(mapViewDesctopIndex, drawParamList->Current(), viewRowIndex);
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

// viewRowIndex parametri on 1:stä alkava rivi indeksi.
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

// viewRowIndex parametri on 1:stä alkava rivi indeksi
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

// Lisätty drawParam pitää myös palauttaa lopussa, jotta sille voidaan tehdä tarvittavia jatkoasetuksia.
// Tämä on tarpeen varsinkin kun ladataan näyttömakroja ja niissä eritoten macroParam jutut ovat todella hankalia käsitellä.
boost::shared_ptr<NFmiDrawParam> NFmiCombinedMapHandler::addTimeSerialViewSideParameter(const NFmiMenuItem& menuItem, bool isViewMacroDrawParam)
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
	// Tyhjennetään samalla side-parameter lista
	timeSerialViewSideParameters_.clear();
}

void NFmiCombinedMapHandler::showCrossSectionDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex, bool showParam)
{
	boost::shared_ptr<NFmiDrawParam> modifiedDrawParam = getCrosssectionDrawParamFromViewLists(menuItem, viewRowIndex);
	if(modifiedDrawParam)
	{
		modifiedDrawParam->HideParam(!showParam);
		drawParamSettingsChangedDirtyActions(menuItem.MapViewDescTopIndex(), getRealRowNumber(menuItem.MapViewDescTopIndex(), viewRowIndex), modifiedDrawParam);
	}
}

bool NFmiCombinedMapHandler::modifyDrawParam(const NFmiMenuItem& menuItem, int viewRowIndex)
{
	boost::shared_ptr<NFmiDrawParam> modifiedDrawParam = getDrawParamFromViewLists(menuItem, viewRowIndex);
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
			bool updateStatus = dlg.RefreshPressed(); // myös false:lla halutaan ruudun päivitys, koska jos painettu päivitä-nappia ja sitten cancelia, pitää ruutu päivittää
			drawParamSettingsChangedDirtyActions(mapViewDescTopIndex, realRowNumber, modifiedDrawParam);
			return updateStatus;

			// Huom! Jos on muutettu border-layer piirtoa niin että se muuttuisi kyseisellä näyttörivillä, niin älä kuitenkaan
			// tyhjennä kuvaa cachesta. Jollain muulla parametri rivillä voi olla samat asetukset ja se voi niitä vielä käyttää.
			// Kuvat eivät vie paljoa muistia nyky koneiden RAM määrillä ja aina kun kartta/alue/kuvan geometria muuttuu, ladataan 
			// näyttömakro, menee kaikki nämä cachet uusiksi kuitenkin.
		}
	}
	return false;
}

boost::shared_ptr<NFmiDrawParam> NFmiCombinedMapHandler::createTimeSerialViewDrawParam(const NFmiMenuItem& menuItem, bool isViewMacroDrawParam)
{
	auto& infoOrganizer = ::getInfoOrganizer();
	boost::shared_ptr<NFmiDrawParam> drawParam = infoOrganizer.CreateDrawParam(menuItem.DataIdent(), menuItem.Level(), menuItem.DataType());
	if(drawParam)
	{
		drawParam->ViewMacroDrawParam(isViewMacroDrawParam);
	}
	return drawParam;
}

// laitetaan drawparam aikasarjan omaan listaa ja jos vertailutila käytössä, lisätää
// vielä eri tuottajien drawparamit erilliseen listaan.
// Lisätty drawParam palautetaan, koska joskus lisätylle oliolle pitää tehdä vielä lisä asetuksia,
// varsinkin kun ladataan näyttömakroja ja niiden drawParameja.
boost::shared_ptr<NFmiDrawParam> NFmiCombinedMapHandler::addTimeSerialView(const NFmiMenuItem& menuItem, bool isViewMacroDrawParam)
{
	timeSerialViewDirty(true);
	auto drawParam = createTimeSerialViewDrawParam(menuItem, isViewMacroDrawParam);
	if(drawParam)
	{
		::setMacroParamDrawParamSettings(menuItem, drawParam);
		std::string logStartStr("Adding to time-serial-view ");
		boost::shared_ptr<NFmiFastQueryInfo> info = ::getInfoOrganizer().Info(drawParam, false, false);
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
		checkAnimationLockedModeTimeBags(menuItem.MapViewDescTopIndex(), false); // kun parametrin näkyvyyttä vaihdetaan, pitää tehdä mahdollisesti animaatio moodin datan tarkistus
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

// Palauttaa pair:issa 1. background nimen ja 2. overlay nimen, jos sellaiset on määritetty
std::pair<std::string, std::string> NFmiCombinedMapHandler::getMacroReferenceNamesForViewMacro(unsigned int mapViewDescTopIndex, unsigned int mapAreaIndex)
{
	std::pair<std::string, std::string> macroReferenceNamePair;
	macroReferenceNamePair.first = ::getMacroReferenceNameForViewMacro(getCombinedMapModeState(mapViewDescTopIndex, mapAreaIndex), staticBackgroundMapLayerRelatedInfos_[mapAreaIndex], wmsBackgroundMapLayerRelatedInfos_);
	macroReferenceNamePair.second = ::getMacroReferenceNameForViewMacro(getCombinedOverlayMapModeState(mapViewDescTopIndex, mapAreaIndex), staticOverlayMapLayerRelatedInfos_[mapAreaIndex], wmsOverlayMapLayerRelatedInfos_);
	return macroReferenceNamePair;
}

// Tämä siis tekee vain tarvittavat background ja overlay indeksien asetukset, mutta ei mitään likauksia tai muita juttuja.
// Tätä on tarkoitus käyttää vain kun näyttömakroa ladataan, jolloin kaikki tarvittava liataan jo muutenkin.
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

// scrollaa näyttöriveja halutun määrän (negatiivinen skrollaa ylös ja positiivinen count alas)
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
		// pitää mahdollisesti piirtää uusiksi salama dataa (ja ehkä jotain muuta?), 
		// joten varmuuden vuoksi laitan aina välimuistin likaiseksi
		mapViewDirty(mapViewDescTopIndex, false, true, true, false, false, false);
	}
}
// palauttaa käytetyn aikastepin tunteina. Jos asetuksissa määrätty aikasteppi
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
	if(mapViewDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
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
		// Huom! NFmiPtrList:issä indeksit alkavat 1:stä...
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

void NFmiCombinedMapHandler::makeApplyViewMacroDirtyActions(double drawObjectScaleFactor)
{
	// läjä dirty funktio kutsuja, ota nyt tästä selvää. Pitäisi laittaa uuteen uskoon koko päivitys asetus juttu.
	mapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, true, true, false, false, true);
	for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
	{
		NFmiMapViewDescTop* mapDescTop = getMapViewDescTop(mapViewDescTopIndex);
		mapDescTop->MapViewBitmapDirty(true);
		mapDescTop->SetBorderDrawDirtyState(CountryBorderDrawDirtyState::Geometry);

		// Päivitetään väkisin tämä pixelSize juttu ja siihen liittyvät laskut
		auto currentPixelSize = mapDescTop->MapViewSizeInPixels();
		mapDescTop->MapViewSizeInPixels(currentPixelSize, nullptr, drawObjectScaleFactor, !mapDescTop->IsTimeControlViewVisible());
		mapDescTop->UpdateOneMapViewSize();
	}
	::getMacroParamDataCache().clearAllLayers();
}

void NFmiCombinedMapHandler::makeSwapBaseArea(unsigned int mapViewDescTopIndex)
{
	getMapViewDescTop(mapViewDescTopIndex)->MapHandler()->MakeSwapBaseArea();
	// täällä ei tarvitse liata mitää eikä päivittää mitään
}

void NFmiCombinedMapHandler::swapArea(unsigned int mapViewDescTopIndex)
{
	NFmiMapViewDescTop* mapDescTop = getMapViewDescTop(mapViewDescTopIndex);
	mapDescTop->MapHandler()->SwapArea();

	// sitten vielä tarvittävät likaukset ja päivitykset
	mapDescTop->SetBorderDrawDirtyState(CountryBorderDrawDirtyState::Geometry);
	mapViewDirty(mapViewDescTopIndex, true, true, true, true, false, false); // laitetaan viela kaikki ajat likaisiksi cachesta
	CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(mapViewDescTopIndex);
	mapDescTop->GridPointCache().Clear();
}

void NFmiCombinedMapHandler::removeMacroParamFromDrawParamLists(const std::string& macroParamName)
{
	for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
	{
		NFmiPtrList<NFmiDrawParamList>::Iterator iter = getMapViewDescTop(mapViewDescTopIndex)->DrawParamListVector()->Start();
		for(unsigned long rowIndex = 0; iter.Next(); rowIndex++)
		{
			if(iter.CurrentPtr()->RemoveMacroParam(macroParamName))
			{
				::getMacroParamDataCache().update(mapViewDescTopIndex, rowIndex, iter.Current());
			}
		}
	}
}

// Kun ollaan hiiren kanssa Näytä-ikkunan päällä (karttanäytössä) ja rullataan
// hiiren rullaa, yritetään siirtää sillä rivillä olevaa aktiivista parametria
// ylös/alas riippuen rullauksen suunnasta. Jos raiseParam on true, tuodaan aktiivista
// parametria piirto järjestyksessä pintaan päin.
// Palauttaa true jos siirto onnistui ja pitää päivittää näytöt, muuten false.
bool NFmiCombinedMapHandler::moveActiveMapViewParamInDrawingOrderList(unsigned int mapViewDescTopIndex, int viewRowIndex, bool raiseParam, bool useCrossSectionParams)
{
	NFmiDrawParamList* list = getDrawParamListWithRealRowNumber(mapViewDescTopIndex, viewRowIndex);
	if(useCrossSectionParams)
		list = getCrossSectionViewDrawParamList(viewRowIndex);
	if(list)
	{
		if(list->MoveActiveParam(raiseParam ? -1 : 1))
		{
			getMapViewDescTop(mapViewDescTopIndex)->MapViewCache().MakeRowDirty(viewRowIndex - 1); // täällä rivit alkavat 1:stä, mutta cachessa 0:sta!!!
			makeMacroParamCacheUpdatesForWantedRow(mapViewDescTopIndex, viewRowIndex);
			return true;
		}
	}
	return false;
}

// Kun ollaan hiiren kanssa Näytä-ikkunan päällä (karttanäytössä) ja rullataan
// hiiren rullaa CTRL-nappi pohjassa, yritetään muuttaa sillä rivillä olevaa
// aktiivista parametria seuraavaan/edelliseen mitä datasta löytyy riippuen rullauksen
// suunnasta. Jos nextParam on true, haetaan seuraava parametri (querydatan) parametrilistasta,
// muuten edellinen. Menee päädyistä yli, eli viimeisestä menee 1. parametriin.
// Käy läpi myös aliparametri (TotalWind ja W&C).
// Palauttaa true jos parametrin vaihto onnistui ja pitää päivittää näytöt, muuten false.
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
			boost::shared_ptr<NFmiDrawParam> drawParam = drawParamList->Current();
			auto& infoOrganizer = ::getInfoOrganizer();
			boost::shared_ptr<NFmiFastQueryInfo> info = infoOrganizer.Info(drawParam, useCrossSectionParams, true);
			if(info && info->SizeParams() > 1)
			{
				bool couldChangeParam = nextParam ? info->NextParam(false) : info->PreviousParam(false);
				if(couldChangeParam == false)
					nextParam ? info->FirstParam(false) : info->LastParam(false);

				boost::shared_ptr<NFmiDrawParam> drawParamTmp = infoOrganizer.CreateDrawParam(info->Param(), &drawParam->Level(), drawParam->DataType());
				if(useCrossSectionParams)
					drawParamTmp = infoOrganizer.CreateCrossSectionDrawParam(info->Param(), drawParam->DataType());
				if(drawParamTmp)
				{
					setUpChangedDrawParam(drawParam, drawParamTmp);
					drawParamList->Dirty(true);
					auto cacheMapRow = realRowIndex - 1; // real-map-row alkaa 1:stä ja cache-map-row 0:sta
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
		::getMacroParamDataCache().update(mapViewDescTopIndex, viewRowIndex, *drawParamList);
}

// Kun jonkun parametrin piirto-ominaisuuksia muutetaan, eivät ne tulevoimaan kuin sille yhdelle
// lämpötilalle tai paine parametrille mitä säädetään. Tai jos on erilaisia malli datoja, niillä on jo
// ladattu valmiiksi omat drawparamit vaikka niitä ei katsottaisikaan ja niihinkään ei tule muutokset
// voimaan, ennen kuin editori käynnistetään uudestaan. Tämä mahdollistaa sen että
// piirto-ominaisuudet saadaan heti käyttöön esim. kaikille lämpötila (T eli par id 4) parametreille.
// Tekee siis vain piirto-ominaisuuksien kopioinnin.
void NFmiCombinedMapHandler::takeDrawParamInUseEveryWhere(boost::shared_ptr<NFmiDrawParam>& drawParam, bool useInMap, bool useInTimeSerial, bool useInCrossSection, bool useWithViewMacros)
{
	// 1. käy läpi kartta drawparam listat (ota huomioon view-macrot)
	if(useInMap)
	{
		for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
			::initializeWantedDrawParams(*(getMapViewDescTop(mapViewDescTopIndex)->DrawParamListVector()), drawParam, useWithViewMacros);
	}
	// 2. käy läpi aikasarja drawparam listat
	if(useInTimeSerial)
		::initializeWantedDrawParams(*timeSerialViewDrawParamList_, drawParam, useWithViewMacros);
	// 3. käy läpi poikkileikkaus drawparamit
	if(useInCrossSection)
		::initializeWantedDrawParams(*crossSectionDrawParamListVector_, drawParam, useWithViewMacros);
	// 4. käy läpi alussa (kaikelle datalle) tehty drawparamlista
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
			// Joskus tehdään hatching testejä ja silloi muutetaan testattavan polygonin indeksiä
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

// tämä asettaa uuden karttanäytön hilaruudukon koon.
// tekee tarvittavat 'likaukset' ja palauttaa true, jos
// näyttöjä tarvitsee päivittää, muuten false (eli ruudukko ei muuttunut).
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

CtrlViewUtils::GraphicalInfo& NFmiCombinedMapHandler::getGraphicalInfo(unsigned int mapViewDescTopIndex)
{
	if(mapViewDescTopIndex == CtrlViewUtils::kFmiCrossSectionView)
		return ::getCrossSectionSystem().GetGraphicalInfo();
	else
		return getMapViewDescTop(mapViewDescTopIndex)->GetGraphicalInfo();
}

// tämä on oikeasti toggle funktio, eli näytä/piilota hila/asemapisteet
void NFmiCombinedMapHandler::onShowGridPoints(unsigned int mapViewDescTopIndex)
{
	auto& applicationWinRegistry = ::getApplicationWinRegistry();
	bool newState = !applicationWinRegistry.ConfigurationRelatedWinRegistry().MapView(mapViewDescTopIndex)->ShowStationPlot();
	applicationWinRegistry.ConfigurationRelatedWinRegistry().MapView(mapViewDescTopIndex)->ShowStationPlot(newState);
	getMapViewDescTop(mapViewDescTopIndex)->ShowStationPlotVM(newState); // tämä pitää päivittää molempiin paikkoihin, koska jotkin operaatiot riippuvat että MapViewDescTop:issa on päivitetty arvo
	mapViewDirty(mapViewDescTopIndex, false, true, true, false, false, false); // tämä laittaa cachen likaiseksi
	CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(mapViewDescTopIndex);
	ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Show/hide active parameters data's grid/station points");
}

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

void NFmiCombinedMapHandler::onEditSpaceOut(unsigned int mapViewDescTopIndex)
{
	::getApplicationWinRegistry().ConfigurationRelatedWinRegistry().MapView(mapViewDescTopIndex)->ToggleSpacingOutFactor();
	// MacroParamDataCache ongelma (MAPADACA): Kun harvennusta muutetaan, pitää liata sellaiset macroParamit, jotka piirretään symboleilla
	mapViewDirty(mapViewDescTopIndex, false, true, true, false, false, false);
	CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation()->UpdateOnlyGivenMapViewAtNextGeneralViewUpdate(mapViewDescTopIndex);
	ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Toggle spacing out factor");
}

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

std::shared_ptr<WmsSupportInterface> NFmiCombinedMapHandler::getWmsSupport() const
{
	std::lock_guard<std::mutex> lock(wmsSupportMutex_);
	return wmsSupport_;
}

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

void NFmiCombinedMapHandler::onAcceleratorToggleKeepMapRatio()
{
	auto& applicationWinRegistry = ::getApplicationWinRegistry();
	auto newKeepAspectRatioState = !applicationWinRegistry.KeepMapAspectRatio();
	applicationWinRegistry.KeepMapAspectRatio(newKeepAspectRatioState);

	// keep ratio laskut pitää tehdä kaikille karttanäytöille!!!
	for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
	{
		CRect rect;
		auto* mapViewDescTop = getMapViewDescTop(mapViewDescTopIndex);
		mapViewDescTop->MapView()->GetClientRect(rect);
		mapViewDescTop->CalcClientViewXperYRatio(NFmiPoint(rect.Width(), rect.Height()));
		// tämä 'aiheuttaa' datan harvennuksen. Jos newKeepAspectRatioState on true, tapahtuu silloin 
		// automaattinen kartan zoomaus ja macroParamCacheData pitää silloin tyhjentää kaikille karttanäytöille
		mapViewDirty(mapViewDescTopIndex, true, true, true, newKeepAspectRatioState, false, false);
		ApplicationInterface::GetApplicationInterfaceImplementation()->UpdateMapView(mapViewDescTopIndex);
	}
	ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
	ApplicationInterface::GetApplicationInterfaceImplementation()->RefreshApplicationViewsAndDialogs("Map view: toggle keep map's aspect ratio setting");
}

// Jos datan area ja kartta ovat "samanlaisia", laitetaan zoomiksi editoiavan datan alue
// muuten laitetaan kurrentti kartta kokonaisuudessaan näkyviin.
void NFmiCombinedMapHandler::onButtonDataArea(unsigned int mapViewDescTopIndex)
{
	boost::shared_ptr<NFmiFastQueryInfo> info = ::getEditedInfo();
	bool editedDataExist = info ? true : false;
	bool dataAreaExist = false;
	if(editedDataExist)
		dataAreaExist = info->Area() != 0;
	bool areasAreSameKind = false;
	auto* mapViewDescTop = getMapViewDescTop(mapViewDescTopIndex);
	if(dataAreaExist)
	{
		boost::shared_ptr<NFmiArea> infoArea(info->Area()->Clone());
		areasAreSameKind = NFmiQueryDataUtil::AreAreasSameKind(infoArea.get(), mapViewDescTop->MapHandler()->TotalArea().get());
	}
	if(areasAreSameKind)
	{
		static int counter = 0;
		counter++; // tämän avulla jos kartan alue ja datan alue samat, joka toisella kerralla zoomataan dataa, ja joka toisella kartan alueeseen
		NFmiRect intersectionRect(mapViewDescTop->MapHandler()->TotalArea()->XYArea().Intersection(mapViewDescTop->MapHandler()->TotalArea()->XYArea(info->Area())));
		boost::shared_ptr<NFmiArea> usedArea(mapViewDescTop->MapHandler()->TotalArea()->CreateNewArea(intersectionRect));
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

double NFmiCombinedMapHandler::drawObjectScaleFactor()
{
	return ::getApplicationWinRegistry().DrawObjectScaleFactor();
}

void NFmiCombinedMapHandler::drawObjectScaleFactor(double newValue)
{
	::getApplicationWinRegistry().DrawObjectScaleFactor(newValue);

	// laitetaan säädön yhteydessä kaikki desctop graphicalinfot likaisiksi
	for(unsigned int mapViewDescTopIndex = 0; mapViewDescTopIndex < mapViewDescTops_.size(); mapViewDescTopIndex++)
		getGraphicalInfo(mapViewDescTopIndex).fInitialized = false;
}

boost::shared_ptr<NFmiDrawParam> NFmiCombinedMapHandler::getUsedDrawParamForEditedData(const NFmiDataIdent& dataIdent)
{
	if(modifiedPropertiesDrawParamList_->Find(dataIdent, 0, NFmiInfoData::kEditable, "", true))
		return modifiedPropertiesDrawParamList_->Current(); // katsotaan löytyykö ensin jo käytössä olevista DrawParameista haluttu (jos siinä on muutoksia arvoissa)
	else
		return ::getInfoOrganizer().CreateDrawParam(dataIdent, 0, NFmiInfoData::kEditable);
}

std::string NFmiCombinedMapHandler::getCurrentMapLayerGuiName(int mapViewDescTopIndex, bool backgroundMap)
{
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

bool NFmiCombinedMapHandler::useCombinedMapMode() const
{
	return ::getApplicationWinRegistry().ConfigurationRelatedWinRegistry().UseCombinedMapMode();
}

void NFmiCombinedMapHandler::useCombinedMapMode(bool newValue)
{
	::getApplicationWinRegistry().ConfigurationRelatedWinRegistry().UseCombinedMapMode(newValue);
	// Varmuuden vuoksi kaikki kartta piirrot uusiksi (voi optimoida myöhemmin, koska on tapauksia, missä ei mitkään asiat muutu)
	mapViewDirty(CtrlViewUtils::kDoAllMapViewDescTopIndex, true, true, true, false, false, false);
	ApplicationInterface::GetApplicationInterfaceImplementation()->ApplyUpdatedViewsFlag(SmartMetViewId::AllMapViews);
}

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
	// Onko piirtokoodi oikeassa kohassa ,että voitaisiin piirtää overlay kerros?
	if(getMapViewDescTop(mapViewDescTopIndex)->DrawOverMapMode() == wantedDrawOverMapMode)
	{
		// Onko joku overlay kerros valittuna (indeksi ei saa olla -1, jolloin ei ole tarkoitus piirtää mitään)?
		return getCombinedOverlayMapModeState(mapViewDescTopIndex, getCurrentMapAreaIndex(mapViewDescTopIndex)).currentMapSectionIndex() >= 0;
	}
	return false;
}

bool NFmiCombinedMapHandler::localOnlyMapModeUsed() const
{
	if(wmsSupportAvailable())
	{
		return !::getApplicationWinRegistry().ConfigurationRelatedWinRegistry().UseCombinedMapMode();
	}
	return true;
}

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
	boost::shared_ptr<NFmiDrawParam> drawParam = ::getInfoOrganizer().CreateDrawParam(menuItem.DataIdent(), menuItem.Level(), dataType);
	if(drawParam)
	{
		drawParam->DataType(dataType);
		drawParam->ParameterAbbreviation(::GetDictionaryString("Country border layer"));
		// Laitetaan viivan paksuudeksi 1 pikseli (border-layer tapauksessa yksikkö on siis pikseli)
		drawParam->SimpleIsoLineWidth(1);
		// Oletus väri on musta (mikä on luultavasti muutenkin IsolineColor:in oletusväri, mutta asetus varmuuden vuoksi)
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
	initializeWmsMapLayerInfos();
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

void NFmiCombinedMapHandler::initializeWmsMapLayerInfos()
{
	auto wmsSupportPtr = getWmsSupport();
	if(wmsSupportPtr->isConfigured())
	{
		::initializeWmsMapLayerInfos(wmsBackgroundMapLayerRelatedInfos_, wmsSupportPtr->getSetup()->background);
		::initializeWmsMapLayerInfos(wmsOverlayMapLayerRelatedInfos_, wmsSupportPtr->getSetup()->overlay);
	}
}

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

	// Virhetilanteissa tai jos wms:ää ei ole ollenkaan edes initialisoitu, palautetaan tyhjää
	const static MapAreaMapLayerRelatedInfo emptyDummy;
	return emptyDummy;
}

void NFmiCombinedMapHandler::clearMacroParamCache(unsigned long mapViewDescTopIndex, unsigned long realRowIndex, boost::shared_ptr<NFmiDrawParam>& drawParam)
{
	::getMacroParamDataCache().clearMacroParamCache(mapViewDescTopIndex, realRowIndex, drawParam->InitFileName());
}
