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
#pragma warning(disable : 4786) // poistaa n kpl VC++ k��nt�j�n varoitusta
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
#include "client_to_master_connection.h" // HUOM! t�m�n pit�� olla ennen #include "MultiProcessClientData.h", koska muuten tulee joku outo Winsock.h allready included error (boost asio -juttu)
#include "MultiProcessClientData.h"
#include "ToolMasterHelperFunctions.h"
#include "EditedInfoMaskHandler.h"
#include "NFmiGriddingProperties.h"

#include <boost/math/special_functions/round.hpp>
#include <fstream>

#ifdef _MSC_VER
#pragma warning (disable : 4244 4267 4512) // boost:in thread kirjastosta tulee ik�v�sti 4244 varoituksia
#endif
#include <boost/thread.hpp>
#ifdef _MSC_VER
#pragma warning (default : 4244 4267 4512) // laitetaan 4244 takaisin p��lle, koska se on t�rke� (esim. double -> int auto castaus varoitus)
#endif


//--------------------------------------------------------
// Constructor/Destructor
//--------------------------------------------------------
NFmiDataParamModifier::NFmiDataParamModifier(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiDrawParam> &theDrawParam,
												boost::shared_ptr<NFmiAreaMaskList> &theMaskList,
												unsigned long theAreaMask)
:itsInfo(theInfo)
,itsDrawParam(theDrawParam)
,itsParamMaskList(theMaskList)
,itsMaskType(theAreaMask)
{
}

//--------------------------------------------------------
// ModifyData
//--------------------------------------------------------
//   K�y datan l�pi ja tekee muutokset Calculate-metodin
//   avulla.
bool NFmiDataParamModifier::ModifyData (void)
{
    EditedInfoMaskHandler editedInfoMaskHandler(itsInfo, itsMaskType);
	itsParamMaskList->CheckIfMaskUsed();
    itsParamMaskList->SyncronizeMaskTime(itsInfo->Time());

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

// 1999.11.17/Marko Viritetty ympyr� muokkauksille
bool NFmiDataParamModifier::ModifyData2(void)
{
    EditedInfoMaskHandler editedInfoMaskHandler(itsInfo, itsMaskType);
    itsParamMaskList->CheckIfMaskUsed();
    itsParamMaskList->SyncronizeMaskTime(itsInfo->Time());
    PrepareFastIsInsideData();

	for(itsInfo->ResetLocation(); itsInfo->NextLocation();)
		if(IsPossibleInside(itsInfo->RelativePoint()))
			itsInfo->FloatValue(Calculate2(itsInfo->FloatValue()));

	return true;
}

//--------------------------------------------------------
// Calculate
//--------------------------------------------------------
//   T�nne annetaan luku mit� on tarkoitus muuttaa
//   ja muutettu luku palautetaan. Perusluokassa ei tehd� mit��n
//   vaan palautetaan parametrina annettu luku.
float NFmiDataParamModifier::Calculate (const float& theValue)
{
	return theValue;
}

//--------------------------------------------------------
// ModifyTimeSeriesData
//--------------------------------------------------------

//   K�y muuttamassa valitut paikat ja koko aikasarjan l�vitse kerralla.
//   Vain annetun timebagin aktiiviset ajat k�yd��n l�pi. float-taulukko
//   on timebagin kokoinen ja siin� on muutos kertoimet.
bool NFmiDataParamModifier::ModifyTimeSeriesData (NFmiTimeDescriptor& theActiveTimes, float* theModifyFactorTable)
{
	int modifyFactorIndex = 0;

    EditedInfoMaskHandler editedInfoMaskHandler(itsInfo, itsMaskType);
    itsParamMaskList->CheckIfMaskUsed();

	for(theActiveTimes.Reset(); theActiveTimes.Next();)
	{
		itsInfo->Time(theActiveTimes.Time());
        itsParamMaskList->SyncronizeMaskTime(itsInfo->Time());
        for(itsInfo->ResetLocation(); itsInfo->NextLocation();)
		{
			if(itsParamMaskList->UseMask())
			{
				if(itsParamMaskList->IsMasked(itsInfo->LatLon()))
					itsInfo->FloatValue(Calculate(itsInfo->FloatValue(), theModifyFactorTable[modifyFactorIndex]));
			}
			else
				itsInfo->FloatValue(Calculate(itsInfo->FloatValue(), theModifyFactorTable[modifyFactorIndex]));
		}
		modifyFactorIndex++;
	}

	return true;
}

bool NFmiDataParamModifier::ModifyTimeSeriesDataUsingMaskFactors(NFmiTimeDescriptor& theActiveTimes, float* theModifyFactorTable)
{
	int modifyFactorIndex = 0;

    EditedInfoMaskHandler editedInfoMaskHandler(itsInfo, itsMaskType);
	itsParamMaskList->CheckIfMaskUsed();

	for(theActiveTimes.Reset(); theActiveTimes.Next();)
	{
		if(itsInfo->Time(theActiveTimes.Time()))
			modifyFactorIndex = itsInfo->TimeIndex();
		else
			continue;
        itsParamMaskList->SyncronizeMaskTime(itsInfo->Time());
		float maskFactor = 0;
		for(itsInfo->ResetLocation(); itsInfo->NextLocation();)
		{
			maskFactor = 1; //itsParamMaskList->MaskValue(itsInfo->LatLon());
			if(itsParamMaskList->UseMask())
				maskFactor = (float)itsParamMaskList->MaskValue(itsInfo->LatLon());
			itsInfo->FloatValue(CalculateWithMaskFactor(itsInfo->FloatValue(), theModifyFactorTable[modifyFactorIndex], maskFactor));
		}
		modifyFactorIndex++;
	}

	return true;
}

//--------------------------------------------------------
// SetTimeSeriesData
//--------------------------------------------------------
bool NFmiDataParamModifier::SetTimeSeriesData(NFmiTimeDescriptor& theActiveTimes, float* theModifyFactorTable, int theUnchangedValue)
{
	int modifyFactorIndex = 0;

    EditedInfoMaskHandler editedInfoMaskHandler(itsInfo, itsMaskType);
    itsParamMaskList->CheckIfMaskUsed();

	for(theActiveTimes.Reset(); theActiveTimes.Next();)
	{
		itsInfo->Time(theActiveTimes.Time());
        itsParamMaskList->SyncronizeMaskTime(itsInfo->Time());
        for(itsInfo->ResetLocation(); itsInfo->NextLocation();)
		{
			if((itsParamMaskList->UseMask() && itsParamMaskList->IsMasked(itsInfo->LatLon()))
				|| !(itsParamMaskList->UseMask()))
				if(theModifyFactorTable[modifyFactorIndex] != theUnchangedValue)
					itsInfo->FloatValue(theModifyFactorTable[modifyFactorIndex]);
		}
		modifyFactorIndex++;
	}

	return true;
}

//--------------------------------------------------------
// Calculate
//--------------------------------------------------------
//   T�t� metodia k�ytet��n timeseries funktiossa.
//
float NFmiDataParamModifier::Calculate (float theDataValue, float theFactor)
{
	float returnValue = kFloatMissing;
	if(theDataValue == kFloatMissing)
		returnValue = theFactor;
	else
	{
		returnValue = theDataValue + theFactor;
		if(itsInfo->Param().GetParam()->GetIdent() == kFmiWindDirection)
		{
			if(returnValue > 360)
				returnValue = static_cast<float>(int (returnValue) % 360);
			if(returnValue < 0)
				returnValue += 360;
		}
	}
	return static_cast<float>(FmiMin(FmiMax(returnValue, itsDrawParam->AbsoluteMinValue()), itsDrawParam->AbsoluteMaxValue()));
}

float NFmiDataParamModifier::CalculateWithMaskFactor(float theDataValue, float theFactor, float theMaskFactor)
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
																	,const NFmiPoint &theCropMarginSize
                                                                    , const NFmiGriddingProperties &griddingProperties)
:NFmiDataParamModifier(theInfo, theDrawParam, theMaskList, theAreaMask)
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
,itsGriddingProperties(griddingProperties)
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
                if(nx < 4 || ny < 4) // en tied� mik� olisi pienin loogisin cropattu hila, mutta ei t�m� voi olla liian pieni
                    fUseGridCrop = false; // laitetaan pois p��lt� jos cropatusta hilasta tulisi liian pieni

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

// HUOM!! eri signerature kuin edell�!!!
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
        itsParamMaskList->SyncronizeMaskTime(itsInfo->Time());
        bool isZeroGrid = !DoDataGridding();
		if(isZeroGrid)
			continue; // t�lle ajalle ei muutoksia!
		NFmiQueryDataUtil::CheckIfStopped(theThreadCallBacks);
		NFmiQueryDataUtil::DoStepIt(theThreadCallBacks); // stepataan vasta 0-tarkastuksen j�lkeen!
        DoLocationGridCalculations(GetUsedGridData());
	}

	return true;
}

void NFmiDataParamControlPointModifier::DoLocationGridCalculations(const NFmiDataMatrix<float> &usedData)
{
    if(fUseGridCrop)
        DoCroppedGridCalculations(usedData);
    else
        DoFullGridCalculations(usedData);
}

void NFmiDataParamControlPointModifier::DoCroppedPointCalculations(const NFmiDataMatrix<float> &usedData, size_t xIndex, size_t yIndex, float maskFactor)
{
    itsInfo->FloatValue(static_cast<float>(CalculateWithMaskFactor(itsInfo->FloatValue(), itsCroppedGridData[xIndex][yIndex], maskFactor)));
}

void NFmiDataParamControlPointModifier::DoNormalPointCalculations(const NFmiDataMatrix<float> &usedData, unsigned long locationIndex, float maskFactor)
{
    itsInfo->FloatValue(static_cast<float>(CalculateWithMaskFactor(itsInfo->FloatValue(), itsGridData[locationIndex % itsGridData.NX()][locationIndex / itsGridData.NX()], maskFactor)));
}

void NFmiDataParamControlPointModifier::DoCroppedGridCalculations(const NFmiDataMatrix<float> &usedData)
{
    auto useMask = itsParamMaskList->UseMask();
    float maskFactor = 1;
    unsigned long sizeX = itsInfo->GridXNumber();
    for(size_t j = 0; j < usedData.NY(); j++)
    {
        for(size_t i = 0; i < usedData.NX(); i++)
        {
            auto locationIndex = static_cast<unsigned long>(GridPointToLocationIndex(boost::math::iround(itsCPGridCropRect.Left() + i), boost::math::iround(itsCPGridCropRect.Top() + j), sizeX));
            itsInfo->LocationIndex(locationIndex);
            if(useMask)
                maskFactor = static_cast<float>(itsParamMaskList->MaskValue(itsInfo->LatLon()));
            if(maskFactor)
                DoCroppedPointCalculations(usedData, i, j, maskFactor);
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

void NFmiDataParamControlPointModifier::DoFullGridCalculations(const NFmiDataMatrix<float> &usedData)
{
    auto useMask = itsParamMaskList->UseMask();
    float maskFactor = 1;
    for(itsInfo->ResetLocation(); itsInfo->NextLocation();)
    {
        if(useMask)
            maskFactor = static_cast<float>(itsParamMaskList->MaskValue(itsInfo->LatLon()));
        if(maskFactor)
        {
            DoNormalPointCalculations(usedData, itsInfo->LocationIndex(), maskFactor);
        }
    }
}


// Huom! t�m� pit�� kutsua erilliseen threadiin, muuten blokkaa pitemm�ksi aikaa
static void StartNewConnection(const std::string &username, bool use_verbose_logging)
{
    try
    {
        log_message("StartNewConnection begins", logging::trivial::debug);
        client_to_master_connection::start_client(username, use_verbose_logging);
        if(client_to_master_connection::client_service().stopped())
            client_to_master_connection::client_service().reset(); // pys�ytetty service_io pit�� resetoida ennen uutta run -kutsua!
        client_to_master_connection::client_service().run();
        log_message("StartNewConnection client_service().run() ends", logging::trivial::debug);
    }
    catch(...)
    {
        // �l� loggaa t��ll� en��!!
        // Poikkeus saattaa tulla kun SmartMet lopetetaan ja yhteys katkeaa, 
        // mutta koska t�m� on eri theadissa, on lokitus systeemi jo saatettu tuhota
//        log_message("StartNewConnection unknown exception occurs", logging::trivial::debug); 

        // ei tehd� mit��n, mutta tuolta saatetaan tulla ulos poikkeuksella ja se ei saa kaataa ohjelmaa, siksi poikkeukset otetaan kiinni t�ss�
    }
}

// Tarkistaa ensin onko toimiva client->server yhteys jo p��ll�, jos on, ei tee mit��n.
// Jos ei ole, k�ynnist� uusi sellainen omassa threadissa.
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
        return ; // ei tarvitse k�ynnistell� yhteyksi�

    boost::thread connection_starting_thread(::StartNewConnection, username, use_verbose_logging);
}

// Tehd��n laskut prosessi poolilla, k�ytt�en Tcp tiedonsiirtoa
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
    std::string relativeAreaRectStr("0,0,1,1"); // ilmeisesti muokkaus tehd��n aina 0,0 - 1,1 laatikossa (paitsi jos olisi cropattuja alueita)
    NFmiStaticTime timeAtWorkStarted;
    auto griddingPropertiesStr = itsGriddingProperties.toString();
    auto cpRangeLimitRelative = static_cast<float>(NFmiGriddingProperties::ConvertLengthInKmToRelative(itsGriddingProperties.rangeLimitInKm(), itsInfo->Area()));
    jobIndex++;

    // T�ytet��n ensin task-jono
    auto &tasks = client_to_master_connection::task_queue();

    // Tehd��n ensin griddaus ty�t ja laitetaan ne serverille
	for(theActiveTimes.Reset(); theActiveTimes.Next();)
	{
		NFmiQueryDataUtil::CheckIfStopped(theThreadCallBacks);

		if(itsInfo->Time(theActiveTimes.Time()) == false)
			continue;
        itsParamMaskList->SyncronizeMaskTime(itsInfo->Time());
        if(!GetChangeValuesWithWork(theActiveTimes.Time(), xValues, yValues, zValues))
			continue; // t�lle ajalle ei muutoksia!
        tasks.push(tcp_tools::task_structure(jobIndex, itsInfo->TimeIndex(), timeAtWorkStarted.EpochTime(), relativeAreaRectStr, itsGridData.NX(), itsGridData.NY(), xValues, yValues, zValues, theGuidStr, griddingPropertiesStr, cpRangeLimitRelative));
	}

    if(tasks.size() == 0)
    {
        log_message("No actual task at this time, stopping MP-CP editing action...", boost::log::trivial::info);
        return false;
    }

    // K�ynnistet��n clint->server yhteys
    std::string username = tcp_tools::g_client_login_base_string + theGuidStr;
    ::CheckClientToServerConnection(theMultiProcessClientData.MultiProcessLogPath(), username, theMultiProcessClientData.MultiProcessPoolOptions().verbose_logging, theMultiProcessClientData.MppLogLevel());

    // Ruvetaan lukemaan ty�ntuloiksia takaisin serverilt�, sit� mukaa kuin niit� tulee
    int timeLimitInSeconds = static_cast<int>(theMultiProcessClientData.MultiProcessPoolOptions().total_wait_time_limit_in_seconds);
    // Haetaa loopissa vastauksia (tietyn aikarajan sis�ll�), kunnes on saatu niit� yht� plajon kuin t�it� l�hetettiin.
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

        // haetaan t�h�n client:iin liittyv�t ty�ntulokset
        std::list<tcp_tools::work_result_structure> results_list = client_to_master_connection::result_queue().get_basic_list_copy(true);

        if(results_list.size())
        {
            for(auto iter = results_list.begin(); iter != results_list.end(); ++iter)
            {
                tcp_tools::work_result_structure &result = *iter;
                if(result.job_index_ != jobIndex)
                    break; // eri ty� numero, ei voi k�ytt�� t�ss� (vanhasta ty�st� tullut vasta nyt?)

                resultsReceived++;
		        NFmiQueryDataUtil::DoStepIt(theThreadCallBacks);

                if(result.values_.size() != itsInfo->SizeLocations())
                {
                    // jos jokin menee vikaan workeriss�, voi sielt� tulla esim. 0 kokoinen vastaus vektori
                    log_message("Non matching size result received from worker, continuing CP modifications...", boost::log::trivial::error);
                    continue;
                }

                itsInfo->TimeIndex(static_cast<unsigned long>(result.data_time_index_)); // asetetaan muokattava aika
				float maskFactor = 1;
		        for(itsInfo->ResetLocation(); itsInfo->NextLocation();)
		        {
			        if(itsParamMaskList->UseMask())
				        maskFactor = static_cast<float>(itsParamMaskList->MaskValue(itsInfo->LatLon()));
			        if(maskFactor)
			        {
				        int locationIndex = itsInfo->LocationIndex();
                        itsInfo->FloatValue(CalculateWithMaskFactor(itsInfo->FloatValue(), result.values_[locationIndex], maskFactor));
			        }
		        }
            }
        }
        else
            boost::this_thread::sleep(boost::posix_time::milliseconds(30)); // jos ei tullut t�it�, nukutaan v�h�n
        if(resultsReceived >= jobCount)
            break;
    }

    return status;
}

namespace
{
	// ToolMaster griddausta saa tehd� vain yksi threadi kerrallaan, siksi pit�� tehd� siihen 
	// unique-lukot varmistamaan t�m�
	typedef boost::shared_mutex MutexType;
	typedef boost::shared_lock<MutexType> ReadLock; // Read-lockia ei oikeasti tarvita, mutta laitan sen t�h�n, jos joskus tarvitaankin
	typedef boost::unique_lock<MutexType> WriteLock;
	MutexType itsToolMasterGriddingMutex;
}

void NFmiDataParamControlPointModifier::DoDataGridding(std::vector<float> &xValues, std::vector<float> &yValues, std::vector<float> &zValues, int arraySize, NFmiDataMatrix<float> &gridData, const NFmiRect &theRelativeRect, const NFmiGriddingProperties &griddingProperties, NFmiObsDataGridding *theObsDataGridding, float theObservationRadiusRelative)
{
	if(arraySize == 0)
		return ;
	if(griddingProperties.function() == kFmiMarkoGriddingFunction)
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
            // HUOM!!! aloitetaan blokki, miss� write-lukko tehd��n
			WriteLock lock(itsToolMasterGriddingMutex);
			// Toolmaster griddaus funktioiden k�ytt�
            Toolmaster::DoToolMasterGridding(xValues, yValues, zValues, arraySize, theRelativeRect, griddingProperties, theObservationRadiusRelative, gridData, tmGridData);
		}

		for(unsigned int j=0; j<gridData.NY(); j++)
			for(unsigned int i=0; i<gridData.NX(); i++)
				gridData[i][j] = tmGridData[j*gridData.NX() + i];
	}
}

// T�m�n metodin avulla p��tell��n kuinka monta aikaa on tarkoitus oikeasti muokata, niin ett� mahdolliselle progress-dialogille osataan antaa
// oikea m��r� steppej� teht�v�ksi. Jos tietyn ajanhetken kaikki kertoimet ovat 0:ia, ei tapahdu oikeasti mit��n datan muokkausta.
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
		if(NFmiDataParamControlPointModifier::GetChangeValues(itsCPManager, theXValues, theYValues, theZValues))
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
		if(NFmiDataParamControlPointModifier::GetChangeValues(itsCPManager, xValues, yValues, zValues))
		{
			if(IsZeroModification(zValues) == false)
				return true;
		}
	}
	return false;
}

bool NFmiDataParamControlPointModifier::GetChangeValues(boost::shared_ptr<NFmiEditorControlPointManager> &theCPManager, std::vector<float> &theXValues, std::vector<float> &theYValues, std::vector<float> &theZValues)
{
	int arraySize = 0;
	theCPManager->ChangeValues(theXValues, theYValues, theZValues, arraySize);
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

// Lineaarinen interpolaatio harvemmasta hilasta tihe�mp��n.
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

// Cropatun hilan reunoille halutaan 'pehmennys' editointiin, jotta muokkaukset eiv�t n�ytt�isi rumilta.
// Cropin sis�reunalla muutos (hilapisteen arvo) otetaan sellaisenaan, mutta mit� reunemmaksi menn��n, sit� 
// pienempi osuus hilapisteen arvosta j�� voimaan.
void NFmiDataParamControlPointModifier::FixCroppedMatrixMargins(NFmiDataMatrix<float> &theCroppedGridData, const NFmiPoint &theCropMarginSize)
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
			if(itsGriddingProperties.function() == kFmiErrorGriddingFunction)
				return false;
			std::vector<float> xValues;
			std::vector<float> yValues;
			std::vector<float> zValues;
			if(NFmiDataParamControlPointModifier::GetChangeValues(itsCPManager, xValues, yValues, zValues))
			{
				if(IsZeroModification(zValues) == false)
				{
                    auto cpPointRadiusRelative = static_cast<float>(NFmiGriddingProperties::ConvertLengthInKmToRelative(itsGriddingProperties.rangeLimitInKm(), itsInfo->Area()));
                    NFmiDataParamControlPointModifier::DoDataGridding(xValues, yValues, zValues, static_cast<int>(xValues.size()), GetUsedGridData(), itsGridCropRelativeRect, itsGriddingProperties, itsObsDataGridding, cpPointRadiusRelative);

					if(fUseGridCrop)
						FixCroppedMatrixMargins(GetUsedGridData(), itsCropMarginSize);
					return true;
				}
			}
		}
	}
	return false;
}

NFmiDataMatrix<float>& NFmiDataParamControlPointModifier::GetUsedGridData()
{
    if(fUseGridCrop)
        return itsCroppedGridData;
    else
        return itsGridData;
}
