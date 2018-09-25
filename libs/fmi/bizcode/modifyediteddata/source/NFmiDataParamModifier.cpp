//**********************************************************
// C++ Class Name : NFmiDataParamModifier
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiDataParamModifier.cpp
//
//
// GDPro Properties
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class
//  - GD Method         : UML ( 2.1.4 )
//  - GD System Name    : Met-editor Plan 2
//  - GD View Type      : Class Diagram
//  - GD View Name      : dataparam modification and masks
// ---------------------------------------------------
//  Author         : pietarin
//  Creation Date  : Fri - Feb 26, 1999
//
//
//  Description:
//
//  Change Log:
//
//**********************************************************
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ kääntäjän varoitusta
#endif

#include "NFmiDataParamModifier.h"
#include "NFmiDrawParam.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiSmartInfo.h"
#include "NFmiAreaMaskList.h"
#include "NFmiGrid.h"
#include "NFmiObsDataGridding.h"
#include "NFmiEditorControlPointManager.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiSettings.h"
#include "client_to_master_connection.h" // HUOM! tämän pitää olla ennen #include "MultiProcessClientData.h", koska muuten tulee joku outo Winsock.h allready included error (boost asio -juttu)
#include "MultiProcessClientData.h"
#include "ToolMasterHelperFunctions.h"
#include "EditedInfoMaskHandler.h"

#include <fstream>

#ifdef _MSC_VER
#pragma warning (disable : 4244 4267 4512) // boost:in thread kirjastosta tulee ikävästi 4244 varoituksia
#endif
#include <boost/thread.hpp>
#ifdef _MSC_VER
#pragma warning (default : 4244 4267 4512) // laitetaan 4244 takaisin päälle, koska se on tärkeä (esim. double -> int auto castaus varoitus)
#endif


NFmiDataParamModifier::LimitChecker::LimitChecker(float theMin, float theMax, FmiParameterName theParam)
    :itsMin(theMin)
    , itsMax(theMax)
    ,fModularFixNeeded(IsModularParam(theParam))
{}

bool NFmiDataParamModifier::LimitChecker::IsModularParam(FmiParameterName theParam)
{
    return (theParam == kFmiWindDirection) || (theParam == kFmiWaveDirection);
}

#include <math.h>

float NFmiDataParamModifier::LimitChecker::CheckValue(float theCheckedValue) const
{
    const float directionModuloValue = 360.f;

    if(theCheckedValue == kFloatMissing)
        return kFloatMissing;

    if(fModularFixNeeded)
    {
        theCheckedValue = std::fmodf(theCheckedValue, directionModuloValue);
        if(theCheckedValue < 0)
            theCheckedValue += directionModuloValue;
    }
    else
    {
        if(theCheckedValue > itsMax)
            theCheckedValue = itsMax;
        else if(theCheckedValue < itsMin)
            theCheckedValue = itsMin;
    }
    return theCheckedValue;
}



//--------------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------------
NFmiDataParamModifier::NFmiDataParamModifier(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiDrawParam> &theDrawParam,
												boost::shared_ptr<NFmiAreaMaskList> &theMaskList,
												unsigned long theAreaMask,
												const NFmiRect& theSelectedSearchAreaRect)
:itsInfo(theInfo)
,itsDrawParam(theDrawParam)
,itsParamMaskList(theMaskList)
,itsMaskType(theAreaMask)
,itsSelectedSearchAreaRect(theSelectedSearchAreaRect)
{
}

//--------------------------------------------------------
// ModifyData
//--------------------------------------------------------
//   Käy datan läpi ja tekee muutokset Calculate-metodin
//   avulla.
bool NFmiDataParamModifier::ModifyData (void)
{
    EditedInfoMaskHandler editedInfoMaskHandler(itsInfo, itsMaskType);
	itsParamMaskList->CheckIfMaskUsed();
	if(itsParamMaskList->UseMask())
		SyncronizeTimeWithMasks();

	for(itsInfo->ResetLocation(); itsInfo->NextLocation();)
	{
		if(itsParamMaskList->UseMask())
		{
			if(itsParamMaskList->IsMasked(itsInfo->LatLon()))
				itsInfo->FloatValue(static_cast<float>(Calculate(itsInfo->FloatValue())));
		}
		else
			itsInfo->FloatValue(static_cast<float>(Calculate(itsInfo->FloatValue())));
	}

	return true;
}

// 1999.11.17/Marko Viritetty ympyrä muokkauksille
bool NFmiDataParamModifier::ModifyData2(void)
{
    EditedInfoMaskHandler editedInfoMaskHandler(itsInfo, itsMaskType);
    itsParamMaskList->CheckIfMaskUsed();
	if(itsParamMaskList->UseMask())
		SyncronizeTimeWithMasks();
	PrepareFastIsInsideData();

	for(itsInfo->ResetLocation(); itsInfo->NextLocation();)
		if(IsPossibleInside(itsInfo->RelativePoint()))
			itsInfo->FloatValue(static_cast<float>(Calculate2(itsInfo->FloatValue())));

	return true;
}

//--------------------------------------------------------
// Calculate
//--------------------------------------------------------
//   Tänne annetaan luku mitä on tarkoitus muuttaa
//   ja muutettu luku palautetaan. Perusluokassa ei tehdä mitään
//   vaan palautetaan parametrina annettu luku.
double NFmiDataParamModifier::Calculate (const double& theValue)
{
	return theValue;
}

//--------------------------------------------------------
// SyncronizeTimeWithMasks
//--------------------------------------------------------
//   Tarkistaa onko maskeja käytössä. Jos on,
//   pyytää itsDataParamilta ajan ja antaa masklistalle
//   ajan synkronointia varten. Palauttaa mita masklistan
//   funktio palauttaa. Käytössä saattaa olla useita
//   eri querydatoja, joilla on omat dataparam-otukset
//   ja ne pitää saada samaan aikaan, että maskaus toimisi
//	 oikein.
bool NFmiDataParamModifier::SyncronizeTimeWithMasks (void)
{
	if(itsParamMaskList->CheckIfMaskUsed())
	{
		return itsParamMaskList->SyncronizeMaskTime(itsInfo->Time());
	}
	else
		return false;
}
//--------------------------------------------------------
// ModifyTimeSeriesData
//--------------------------------------------------------

//   Käy muuttamassa valitut paikat ja koko aikasarjan lävitse kerralla.
//   Vain annetun timebagin aktiiviset ajat käydään läpi. double-taulukko
//   on timebagin kokoinen ja siinä on muutos kertoimet.
bool NFmiDataParamModifier::ModifyTimeSeriesData (NFmiTimeDescriptor& theActiveTimes, double* theModifyFactorTable)
{
	int modifyFactorIndex = 0;

    EditedInfoMaskHandler editedInfoMaskHandler(itsInfo, itsMaskType);
    itsParamMaskList->CheckIfMaskUsed();

	for(theActiveTimes.Reset(); theActiveTimes.Next();)
	{
		itsInfo->Time(theActiveTimes.Time());
		SyncronizeTimeWithMasks();
		for(itsInfo->ResetLocation(); itsInfo->NextLocation();)
		{
			if(itsParamMaskList->UseMask())
			{
				if(itsParamMaskList->IsMasked(itsInfo->LatLon()))
					itsInfo->FloatValue(static_cast<float>(Calculate(itsInfo->FloatValue(), theModifyFactorTable[modifyFactorIndex])));
			}
			else
				itsInfo->FloatValue(static_cast<float>(Calculate(itsInfo->FloatValue(), theModifyFactorTable[modifyFactorIndex])));
		}
		modifyFactorIndex++;
	}

	return true;
}

bool NFmiDataParamModifier::ModifyTimeSeriesDataUsingMaskFactors(NFmiTimeDescriptor& theActiveTimes, double* theModifyFactorTable)
{
	int modifyFactorIndex = 0;

    EditedInfoMaskHandler editedInfoMaskHandler(itsInfo, itsMaskType);
	itsParamMaskList->CheckIfMaskUsed();

	double searchRectSize = (itsSelectedSearchAreaRect.Width()+1) * (itsSelectedSearchAreaRect.Height()+1);
	bool doAreaSearchForSelectedPoints = searchRectSize > 1 ? true : false;

	for(theActiveTimes.Reset(); theActiveTimes.Next();)
	{
		if(itsInfo->Time(theActiveTimes.Time()))
			modifyFactorIndex = itsInfo->TimeIndex();
		else
			continue;
		SyncronizeTimeWithMasks();
		double maskFactor = 0;
		for(itsInfo->ResetLocation(); itsInfo->NextLocation();)
		{
			maskFactor = 1; //itsParamMaskList->MaskValue(itsInfo->LatLon());
			if(itsParamMaskList->UseMask())
				maskFactor = itsParamMaskList->MaskValue(itsInfo->LatLon());
			if(doAreaSearchForSelectedPoints)
			{
				int count = dynamic_cast<NFmiSmartInfo*>(itsInfo.get())->MaskedCount(NFmiMetEditorTypes::kFmiSelectionMask, itsInfo->LocationIndex(), itsSelectedSearchAreaRect);
				double selectedFactor = count / searchRectSize;
				selectedFactor = selectedFactor*selectedFactor;
				maskFactor *= selectedFactor;
			}
			itsInfo->FloatValue(static_cast<float>(CalculateWithMaskFactor(itsInfo->FloatValue(), theModifyFactorTable[modifyFactorIndex], maskFactor)));
		}
		modifyFactorIndex++;
	}

	return true;
}

//--------------------------------------------------------
// SetTimeSeriesData
//--------------------------------------------------------
bool NFmiDataParamModifier::SetTimeSeriesData(NFmiTimeDescriptor& theActiveTimes, double* theModifyFactorTable, int theUnchangedValue)
{
	int modifyFactorIndex = 0;

    EditedInfoMaskHandler editedInfoMaskHandler(itsInfo, itsMaskType);
    itsParamMaskList->CheckIfMaskUsed();

	for(theActiveTimes.Reset(); theActiveTimes.Next();)
	{
		itsInfo->Time(theActiveTimes.Time());
		SyncronizeTimeWithMasks();
		for(itsInfo->ResetLocation(); itsInfo->NextLocation();)
		{
			if((itsParamMaskList->UseMask() && itsParamMaskList->IsMasked(itsInfo->LatLon()))
				|| !(itsParamMaskList->UseMask()))
				if(theModifyFactorTable[modifyFactorIndex] != theUnchangedValue)
					itsInfo->FloatValue(static_cast<float>(theModifyFactorTable[modifyFactorIndex]));
		}
		modifyFactorIndex++;
	}

	return true;
}

//--------------------------------------------------------
// Calculate
//--------------------------------------------------------
//   Tätä metodia käytetään timeseries funktiossa.
//
double NFmiDataParamModifier::Calculate (double theDataValue, double theFactor)
{
	double returnValue = kFloatMissing;
	if(theDataValue == kFloatMissing)
		returnValue = theFactor;
	else
	{
		returnValue = theDataValue + theFactor;
		if(itsInfo->Param().GetParam()->GetIdent() == kFmiWindDirection)
		{
			if(returnValue > 360)
				returnValue = int (returnValue) % 360;
			if(returnValue < 0)
				returnValue += 360;
		}
	}
	return FmiMin(FmiMax(returnValue, itsDrawParam->AbsoluteMinValue()), itsDrawParam->AbsoluteMaxValue());
}

double NFmiDataParamModifier::CalculateWithMaskFactor(double theDataValue, double theFactor, double theMaskFactor)
{
	if(theFactor != kFloatMissing)
		return Calculate(theDataValue, theFactor * theMaskFactor);
	else
		return theDataValue; // jos theFactor on puuttuva, palautetaan originaali arvo, EI puuttuvaa!
}

bool NFmiDataParamModifier::Param(const NFmiParam& theParam)
{
	if(itsInfo)
		return itsInfo->Param(theParam);
	else
		return false;
}



// ****************************************************************************************************
// ****************************************************************************************************
// ****************************************************************************************************
// ****************************************************************************************************
// ****************************************************************************************************
// ****************************************************************************************************


NFmiDataParamControlPointModifier::NFmiDataParamControlPointModifier(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiDrawParam> &theDrawParam
																	,boost::shared_ptr<NFmiAreaMaskList> &theMaskList
																	,unsigned long theAreaMask
																	,boost::shared_ptr<NFmiEditorControlPointManager> &theCPManager
																	,const NFmiRect &theCPGridCropRect
																	,bool theUseGridCrop
																	,const NFmiPoint &theCropMarginSize)
:NFmiDataParamModifier(theInfo, theDrawParam, theMaskList, theAreaMask, NFmiRect())
,itsGridData()
,itsCPGridCropRect(theCPGridCropRect)
,fUseGridCrop(theUseGridCrop)
,fCanGridCropUsed(false)
,itsCropMarginSize(theCropMarginSize)
,itsCroppedGridData()
,itsGridCropRelativeRect(0, 0, 1, 1)
,itsObsDataGridding(new NFmiObsDataGridding())
,itsCPManager(theCPManager)
,itsLastTimeIndex(-1)
{
    static NFmiRect emptyRect(0, 0, 1, 1);
	if(itsInfo && itsInfo->IsGrid())
	{
		const NFmiGrid* grid = itsInfo->Grid();
		itsGridData.Resize(grid->XNumber(), grid->YNumber(), 0.f);

		fCanGridCropUsed = itsCPGridCropRect != emptyRect;
        if(fUseGridCrop)
        {
            if(fCanGridCropUsed)
            {
                size_t nx = FmiRound(itsCPGridCropRect.Width()) + 1;
                nx = FmiMin(nx, itsGridData.NX());
                size_t ny = FmiRound(itsCPGridCropRect.Height()) + 1;
                ny = FmiMin(ny, itsGridData.NY());
                itsCroppedGridData.Resize(nx, ny, 0.f);
                if(nx < 4 || ny < 4) // en tiedä mikä olisi pienin loogisin cropattu hila, mutta ei tämä voi olla liian pieni
                    fUseGridCrop = false; // laitetaan pois päältä jos cropatusta hilasta tulisi liian pieni

                double left = itsCPGridCropRect.Left() / itsGridData.NX();
                double right = itsCPGridCropRect.Right() / itsGridData.NX();
                double bottom = itsCPGridCropRect.Bottom() / itsGridData.NY();
                double top = itsCPGridCropRect.Top() / itsGridData.NY();
                itsGridCropRelativeRect = NFmiRect(left, top, right, bottom);
            }
        }
	}
}

NFmiDataParamControlPointModifier::~NFmiDataParamControlPointModifier(void)
{
	delete itsObsDataGridding;
}

bool NFmiDataParamControlPointModifier::PreventGridCropCalculations()
{
    if(fUseGridCrop && fCanGridCropUsed == false)
        return true;
    else
        return false;
}

// HUOM!! eri signerature kuin edellä!!!
bool NFmiDataParamControlPointModifier::ModifyTimeSeriesDataUsingMaskFactors(NFmiTimeDescriptor& theActiveTimes, NFmiThreadCallBacks *theThreadCallBacks)
{
    if(PreventGridCropCalculations())
        return false;

    EditedInfoMaskHandler editedInfoMaskHandler(itsInfo, itsMaskType);
    itsParamMaskList->CheckIfMaskUsed();
	NFmiDataMatrix<float> infoValues;
	NFmiQueryDataUtil::SetRange(theThreadCallBacks, 0, CalcActualModifiedTimes(theActiveTimes), 1);

	for(theActiveTimes.Reset(); theActiveTimes.Next();)
	{
		NFmiQueryDataUtil::CheckIfStopped(theThreadCallBacks);
		if(itsInfo->Time(theActiveTimes.Time()) == false)
			continue;
		SyncronizeTimeWithMasks();
		bool isZeroGrid = !DoDataGridding();
		if(isZeroGrid)
			continue; // tälle ajalle ei muutoksia!
		NFmiQueryDataUtil::CheckIfStopped(theThreadCallBacks);
		NFmiQueryDataUtil::DoStepIt(theThreadCallBacks); // stepataan vasta 0-tarkastuksen jälkeen!
		double maskFactor = 1;
		if(fUseGridCrop)
            DoCroppedGridCalculations();
		else
            DoFullGridCalculations();
	}

	return true;
}

void NFmiDataParamControlPointModifier::DoCroppedGridCalculations()
{
    auto useMask = itsParamMaskList->UseMask();
    double maskFactor = 1;
    unsigned long sizeX = itsInfo->GridXNumber();
    for(size_t j = 0; j < itsCroppedGridData.NY(); j++)
    {
        for(size_t i = 0; i < itsCroppedGridData.NX(); i++)
        {
            auto locationIndex = static_cast<unsigned long>(GridPointToLocationIndex(boost::math::iround(itsCPGridCropRect.Left() + i), boost::math::iround(itsCPGridCropRect.Top() + j), sizeX));
            itsInfo->LocationIndex(locationIndex);
            if(useMask)
                maskFactor = itsParamMaskList->MaskValue(itsInfo->LatLon());
            if(maskFactor)
                itsInfo->FloatValue(static_cast<float>(CalculateWithMaskFactor(itsInfo->FloatValue(), itsCroppedGridData[i][j], maskFactor)));
        }
    }
}

size_t NFmiDataParamControlPointModifier::GridPointToLocationIndex(size_t gridPointX, size_t gridPointY, size_t gridSizeX)
{
    if(gridPointX < gridSizeX)
        return (gridPointY * gridSizeX) + gridPointX;
    else
        return gMissingIndex;
}

void NFmiDataParamControlPointModifier::DoFullGridCalculations()
{
    auto useMask = itsParamMaskList->UseMask();
    double maskFactor = 1;
    for(itsInfo->ResetLocation(); itsInfo->NextLocation();)
    {
        if(itsParamMaskList->UseMask())
            maskFactor = itsParamMaskList->MaskValue(itsInfo->LatLon());
        if(maskFactor)
        {
            int locationIndex = itsInfo->LocationIndex();
            itsInfo->FloatValue(static_cast<float>(CalculateWithMaskFactor(itsInfo->FloatValue(), itsGridData[locationIndex%itsGridData.NX()][locationIndex / itsGridData.NX()], maskFactor)));
        }
    }
}


// Huom! tämä pitää kutsua erilliseen threadiin, muuten blokkaa pitemmäksi aikaa
static void StartNewConnection(const std::string &username, bool use_verbose_logging)
{
    try
    {
        log_message("StartNewConnection begins", logging::trivial::debug);
        client_to_master_connection::start_client(username, use_verbose_logging);
        if(client_to_master_connection::client_service().stopped())
            client_to_master_connection::client_service().reset(); // pysäytetty service_io pitää resetoida ennen uutta run -kutsua!
        client_to_master_connection::client_service().run();
        log_message("StartNewConnection client_service().run() ends", logging::trivial::debug);
    }
    catch(...)
    {
        // Älä loggaa täällä enää!!
        // Poikkeus saattaa tulla kun SmartMet lopetetaan ja yhteys katkeaa, 
        // mutta koska tämä on eri theadissa, on lokitus systeemi jo saatettu tuhota
//        log_message("StartNewConnection unknown exception occurs", logging::trivial::debug); 

        // ei tehdä mitään, mutta tuolta saatetaan tulla ulos poikkeuksella ja se ei saa kaataa ohjelmaa, siksi poikkeukset otetaan kiinni tässä
    }
}

// Tarkistaa ensin onko toimiva client->server yhteys jo päällä, jos on, ei tee mitään.
// Jos ei ole, käynnistä uusi sellainen omassa threadissa.
static void CheckClientToServerConnection(const std::string &log_file_path, const std::string &username, bool use_verbose_logging, logging::trivial::severity_level log_level)
{
    static bool firstTime = true;
    if(firstTime)
    {
        firstTime = false;
        init_logger(username, log_level, log_file_path);
    }

    client_to_master_connection::connection_ptr current_connection = client_to_master_connection::currently_used_connection();
    if(current_connection && current_connection->stopped() == false)
        return ; // ei tarvitse käynnistellä yhteyksiä

    boost::thread connection_starting_thread(::StartNewConnection, username, use_verbose_logging);
}

// Tehdään laskut prosessi poolilla, käyttäen Tcp tiedonsiirtoa
bool NFmiDataParamControlPointModifier::DoProcessPoolCpModifyingTcp(MultiProcessClientData &theMultiProcessClientData, NFmiTimeDescriptor& theActiveTimes, const std::string &theGuidStr, NFmiThreadCallBacks *theThreadCallBacks)
{
    if(PreventGridCropCalculations())
        return false;

    static size_t jobIndex = 0;

    bool status = true;
    EditedInfoMaskHandler editedInfoMaskHandler(itsInfo, itsMaskType);
    itsParamMaskList->CheckIfMaskUsed();
	NFmiDataMatrix<float> infoValues;
    int jobCount = CalcActualModifiedTimes(theActiveTimes);
    NFmiQueryDataUtil::SetRange(theThreadCallBacks, 0, jobCount, 1);
	std::vector<float> xValues;
	std::vector<float> yValues;
	std::vector<float> zValues;
    std::string relativeAreaRectStr("0,0,1,1"); // ilmeisesti muokkaus tehdään aina 0,0 - 1,1 laatikossa (paitsi jos olisi cropattuja alueita)
    NFmiStaticTime timeAtWorkStarted;
    FmiGriddingFunction griddingFunction = itsCPManager->CPGriddingProperties().Function();
    jobIndex++;

    // Täytetään ensin task-jono
    tcp_tools::concurrent_queue<tcp_tools::task_structure> &tasks = client_to_master_connection::task_queue();

    // Tehdään ensin griddaus työt ja laitetaan ne serverille
	for(theActiveTimes.Reset(); theActiveTimes.Next();)
	{
		NFmiQueryDataUtil::CheckIfStopped(theThreadCallBacks);

		if(itsInfo->Time(theActiveTimes.Time()) == false)
			continue;
		SyncronizeTimeWithMasks();
        if(!GetChangeValuesWithWork(theActiveTimes.Time(), xValues, yValues, zValues))
			continue; // tälle ajalle ei muutoksia!
        tasks.push(tcp_tools::task_structure(jobIndex, itsInfo->TimeIndex(), timeAtWorkStarted.EpochTime(), relativeAreaRectStr, itsGridData.NX(), itsGridData.NY(), xValues, yValues, zValues, theGuidStr, griddingFunction));
	}

    // Käynnistetään clint->server yhteys
    std::string username = tcp_tools::g_client_login_base_string + theGuidStr;
    ::CheckClientToServerConnection(theMultiProcessClientData.MultiProcessLogPath(), username, theMultiProcessClientData.MultiProcessPoolOptions().verbose_logging, theMultiProcessClientData.MppLogLevel());

    // Ruvetaan lukemaan työntuloiksia takaisin serveriltä, sitä mukaa kuin niitä tulee
    int timeLimitInSeconds = static_cast<int>(theMultiProcessClientData.MultiProcessPoolOptions().total_wait_time_limit_in_seconds);
    // Haetaa loopissa vastauksia (tietyn aikarajan sisällä), kunnes on saatu niitä yhtä plajon kuin töitä lähetettiin.
    int resultsReceived = 0;
    for(;;)
    {
		NFmiQueryDataUtil::CheckIfStopped(theThreadCallBacks);
        NFmiStaticTime timeNow;
        int waitTimeInSeconds = static_cast<int>(timeNow.EpochTime() - timeAtWorkStarted.EpochTime());
        if(waitTimeInSeconds > timeLimitInSeconds)
        {
            status = false;
            break;
        }

        // haetaan tähän client:iin liittyvät työntulokset
        std::list<tcp_tools::work_result_structure> results_list = client_to_master_connection::result_queue().get_basic_list_copy(true);

        if(results_list.size())
        {
            for(auto iter = results_list.begin(); iter != results_list.end(); ++iter)
            {
                tcp_tools::work_result_structure &result = *iter;
                if(result.job_index_ != jobIndex)
                    break; // eri työ numero, ei voi käyttää tässä (vanhasta työstä tullut vasta nyt?)

                resultsReceived++;
		        NFmiQueryDataUtil::DoStepIt(theThreadCallBacks);

                if(result.values_.size() != itsInfo->SizeLocations())
                {
                    // jos jokin menee vikaan workerissä, voi sieltä tulla esim. 0 kokoinen vastaus vektori
                    log_message("Non matching size result received from worker, continuing CP modifications...", boost::log::trivial::error);
                    continue;
                }

                itsInfo->TimeIndex(static_cast<unsigned long>(result.data_time_index_)); // asetetaan muokattava aika
		        double maskFactor = 1;
		        for(itsInfo->ResetLocation(); itsInfo->NextLocation();)
		        {
			        if(itsParamMaskList->UseMask())
				        maskFactor = itsParamMaskList->MaskValue(itsInfo->LatLon());
			        if(maskFactor)
			        {
				        int locationIndex = itsInfo->LocationIndex();
                        itsInfo->FloatValue(static_cast<float>(CalculateWithMaskFactor(itsInfo->FloatValue(), result.values_[locationIndex], maskFactor)));
			        }
		        }
            }
        }
        else
            boost::this_thread::sleep(boost::posix_time::milliseconds(30)); // jos ei tullut töitä, nukutaan vähän
        if(resultsReceived >= jobCount)
            break;
    }

    return status;
}

namespace
{
	// ToolMaster griddausta saa tehdä vain yksi threadi kerrallaan, siksi pitää tehdä siihen 
	// unique-lukot varmistamaan tämä
	typedef boost::shared_mutex MutexType;
	typedef boost::shared_lock<MutexType> ReadLock; // Read-lockia ei oikeasti tarvita, mutta laitan sen tähän, jos joskus tarvitaankin
	typedef boost::unique_lock<MutexType> WriteLock;
	MutexType itsToolMasterGriddingMutex;
}

void NFmiDataParamControlPointModifier::DoDataGridding(std::vector<float> &xValues, std::vector<float> &yValues, std::vector<float> &zValues, int arraySize, NFmiDataMatrix<float> &gridData, const NFmiRect &theRelativeRect, int theGriddingFunction, NFmiObsDataGridding *theObsDataGridding, float theObservationRadiusRelative)
{
	if(arraySize == 0)
		return ;
	if(theGriddingFunction == kFmiMarkoGriddingFunction)
	{
		if(theObsDataGridding)
		{
			theObsDataGridding->SearchRange(0.7f);
			theObsDataGridding->AreaLimits(theRelativeRect);
			theObsDataGridding->DoGridding(xValues, yValues, zValues, arraySize, gridData);
		}
	}
	else
	{
		std::vector<float> tmGridData(gridData.NX() * gridData.NY(), kFloatMissing);

		{ 
            // HUOM!!! aloitetaan blokki, missä write-lukko tehdään
			WriteLock lock(itsToolMasterGriddingMutex);
			// Toolmaster griddaus funktioiden käyttö
            Toolmaster::DoToolMasterGridding(xValues, yValues, zValues, arraySize, theRelativeRect, theGriddingFunction, theObservationRadiusRelative, gridData, tmGridData);
		}

		for(unsigned int j=0; j<gridData.NY(); j++)
			for(unsigned int i=0; i<gridData.NX(); i++)
				gridData[i][j] = tmGridData[j*gridData.NX() + i];
	}
}

// Tämän metodin avulla päätellään kuinka monta aikaa on tarkoitus oikeasti muokata, niin että mahdolliselle progress-dialogille osataan antaa
// oikea määrä steppejä tehtäväksi. Jos tietyn ajanhetken kaikki kertoimet ovat 0:ia, ei tapahdu oikeasti mitään datan muokkausta.
int NFmiDataParamControlPointModifier::CalcActualModifiedTimes(NFmiTimeDescriptor& theActiveTimes)
{
	int modifiedTimeCount = 0;
	for(theActiveTimes.Reset(); theActiveTimes.Next(); )
	{
		if(IsTimeModified(theActiveTimes.Time()))
			modifiedTimeCount++;
	}
	return modifiedTimeCount;
}

bool NFmiDataParamControlPointModifier::GetChangeValuesWithWork(const NFmiMetTime &theTime, std::vector<float> &theXValues, std::vector<float> &theYValues, std::vector<float> &theZValues)
{
	if(itsCPManager->Time(theTime))
	{
        theXValues.clear();
        theYValues.clear();
        theZValues.clear();
		if(GetChangeValues(theXValues, theYValues, theZValues))
		{
			if(!IsZeroModification(theZValues))
				return true;
		}
	}
	return false;
}

bool NFmiDataParamControlPointModifier::IsTimeModified(const NFmiMetTime &theTime)
{
	if(itsCPManager->Time(theTime))
	{
		std::vector<float> xValues;
		std::vector<float> yValues;
		std::vector<float> zValues;
		if(GetChangeValues(xValues, yValues, zValues))
		{
			if(IsZeroModification(zValues) == false)
				return true;
		}
	}
	return false;
}

bool NFmiDataParamControlPointModifier::GetChangeValues(std::vector<float> &theXValues, std::vector<float> &theYValues, std::vector<float> &theZValues)
{
	int arraySize = 0;
	itsCPManager->ChangeValues(theXValues, theYValues, theZValues, arraySize);
	if(arraySize > 0)
		return true;
	return false;
}

bool NFmiDataParamControlPointModifier::IsZeroModification(const std::vector<float> &theZValues)
{
	bool fZeroGrid = true;
	size_t size = theZValues.size();
	for(size_t i=0; i< size; i++)
	{
		if(theZValues[i] != 0)
		{
			fZeroGrid = false;
			break;
		}
	}
	return fZeroGrid;
}

// Lineaarinen interpolaatio harvemmasta hilasta tiheämpään.
static void InterpolateMatrix(const NFmiDataMatrix<float> &source, NFmiDataMatrix<float> &dest)
{
	double destSizeX = static_cast<double>(dest.NX());
	double destSizeY = static_cast<double>(dest.NY());
	for(size_t j=0; j < dest.NY(); j++)
	{
		for(size_t i=0; i < dest.NX(); i++)
		{
			NFmiPoint location(i/destSizeX, j/destSizeY);
			dest[i][j] = source.InterpolatedValue(location, kFmiTemperature, true);
		}
	}
}

// Cropatun hilan reunoille halutaan 'pehmennys' editointiin, jotta muokkaukset eivät näyttäisi rumilta.
// Cropin sisäreunalla muutos (hilapisteen arvo) otetaan sellaisenaan, mutta mitä reunemmaksi mennään, sitä 
// pienempi osuus hilapisteen arvosta jää voimaan.
static void FixCroppedMatrixMargins(NFmiDataMatrix<float> &theCroppedGridData, const NFmiPoint &theCropMarginSize)
{
	size_t destSizeX = theCroppedGridData.NX();
	size_t destSizeY = theCroppedGridData.NY();
	for(size_t j=0; j < destSizeY; j++)
	{
		for(size_t i=0; i < destSizeX; i++)
		{
			float value = theCroppedGridData[i][j];
			if(value != kFloatMissing)
			{
				float factorX = 0;
				float factorY = 0;
				if(i < theCropMarginSize.X())
					factorX = static_cast<float>((i + 1) / (theCropMarginSize.X() + 1));
				if(i >= destSizeX - theCropMarginSize.X())
					factorX = static_cast<float>((destSizeX - 1 - i + 1) / (theCropMarginSize.X() + 1));
				if(j < theCropMarginSize.Y())
					factorY = static_cast<float>((j + 1) / (theCropMarginSize.Y() + 1));
				if(j >= destSizeY - theCropMarginSize.Y())
					factorY = static_cast<float>((destSizeY - 1 - j + 1) / (theCropMarginSize.Y() + 1));

				if(factorX && factorY)
				{
					value = value * sqrt(factorX * factorY);
					theCroppedGridData[i][j] = value;
				}
				else if(factorX)
					theCroppedGridData[i][j] = value * factorX;
				else if(factorY)
					theCroppedGridData[i][j] = value * factorY;
			}
		}
	}
}

bool NFmiDataParamControlPointModifier::DoDataGridding(void)
{
	if(itsObsDataGridding && itsInfo && itsCPManager)
	{
		if(itsCPManager->Time(itsInfo->Time()))
		{
			FmiGriddingFunction griddingFunction = itsCPManager->CPGriddingProperties().Function();
			if(griddingFunction == kFmiErrorGriddingFunction)
				return false;
			std::vector<float> xValues;
			std::vector<float> yValues;
			std::vector<float> zValues;
			if(GetChangeValues(xValues, yValues, zValues))
			{
				if(IsZeroModification(zValues) == false)
				{
					NFmiDataMatrix<float> *usedGridData = &itsGridData;
                    if(fUseGridCrop)
                    {
                        usedGridData = &itsCroppedGridData;
                    }

                    NFmiDataParamControlPointModifier::DoDataGridding(xValues, yValues, zValues, static_cast<int>(xValues.size()), *usedGridData, itsGridCropRelativeRect, griddingFunction, itsObsDataGridding, kFloatMissing);

					if(fUseGridCrop)
						::FixCroppedMatrixMargins(itsCroppedGridData, itsCropMarginSize);
					return true;
				}
			}
		}
	}
	return false;
}
