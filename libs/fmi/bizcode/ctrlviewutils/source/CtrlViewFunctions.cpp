#include "CtrlViewFunctions.h"
#include "NFmiStringTools.h"
#include "NFmiAngle.h"
#include "NFmiDictionaryFunction.h"
#include "NFmiColor.h"
#include "NFmiRect.h"
#include "NFmiDrawParam.h"
#include "NFmiFastQueryInfo.h"
#include "CtrlViewDocumentInterface.h"
#include "NFmiInfoOrganizer.h"
#include "NFmiProducerSystem.h"

#include <boost/lexical_cast.hpp>
#include <boost/math/special_functions/round.hpp>

namespace CtrlViewUtils
{
    NFmiPoint CalcTimeScaleFontSizeInPixels(double thePixelsPerMMinX)
    {
        static const double statScaleFontSizeInMM = 2.6;
        int fontSizeY = static_cast<int>(statScaleFontSizeInMM * thePixelsPerMMinX * 1.88);
        return NFmiPoint(fontSizeY, fontSizeY);
    }

    std::string GetLatLonValue2MinuteStr(double value, int decimals) // voi olla lat/lon arvo josta on jo otettu absoluuttiarvo
    {

        int valueInt = static_cast<int>(value);
        double valueRemainder = value - valueInt;
        double minutes = valueRemainder*60.;
        if(decimals)
            minutes = round(minutes * 10) / 10.; // n�in saadaan py�ristettyn� arvona luku yhden desimaalin tarkkuudella
        else
            minutes = round(minutes); // ei desimaaleja mukaan
        if(minutes >= 60.)
        { // jos minuutit py�ristyiv�t 60:een, pit�� lis�t� asteisiin 1 ja laittaa minuutit 0:ksi
            valueInt += 1;
            minutes = 0;
        }

        std::string retStr;
        retStr += NFmiStringTools::Convert<int>(valueInt);
        retStr += "�";
        size_t strSize = retStr.size();
        if(minutes < 10.)
            retStr += "0"; // lis�t��n etu nolla
        retStr += NFmiStringTools::Convert<double>(minutes);
        if(decimals)
        {
            if(retStr.size() - strSize == 1)
                retStr += "0"; // lis�t��n desimaali nolla
            else if(retStr.size() - strSize == 2)
                retStr += ".0"; // lis�t��n desimaali piste ja nolla
        }
        retStr += "'";

        return retStr;
    }

    std::string GetLatitudeMinuteStr(double lat, int decimals)
    {
        bool northernSide = lat >= 0.;
        lat = ::fabs(lat);
        std::string retStr;
        if(lat < 10.)
            retStr += "0"; // lis�t��n tarvittaessa etu nolla
        retStr += GetLatLonValue2MinuteStr(lat, decimals);
        if(northernSide)
            retStr += "N";
        else
            retStr += "S";

        return retStr;
    }

    bool IsPacificLongitude(double theLongitude)
    {
        if(theLongitude > 180 && theLongitude <= 360)
            return true;
        else
            return false;
    }

    double GetWantedLongitude(double theLongitude, bool fPacificView)
    {
        double value = theLongitude;
        if(fPacificView)
        {
            if(::fabs(theLongitude - 360.) < 0.0000001)
                value = 360.;
            else if(::fabs(theLongitude + 360.) < 0.0000001)
                value = 0.;
            else if(theLongitude > 360)
                GetWantedLongitude(theLongitude - 360, fPacificView);
            else if(theLongitude < 0)
                GetWantedLongitude(theLongitude + 360, fPacificView);
            else
                value = theLongitude;
        }
        else
        {
            if(::fabs(theLongitude - 180.) < 0.0000001)
                value = 180.;
            else if(::fabs(theLongitude + 180.) < 0.0000001)
                value = -180.;
            else if(theLongitude > 180)
                GetWantedLongitude(theLongitude - 360, fPacificView);
            else if(theLongitude < -180)
                GetWantedLongitude(theLongitude + 360, fPacificView);
            else
                value = theLongitude;
        }
        return value;
    }

    NFmiLocation GetPacificFixedLocation(const NFmiLocation& theLocation, bool usePacificView)
    {
        NFmiLocation loc(theLocation);
        loc.SetLongitude(GetWantedLongitude(loc.GetLongitude(), usePacificView));
        return loc;
    }

    std::string GetLongitudeMinuteStr(double lon, int decimals)
    {
        lon = GetWantedLongitude(lon, false);
        bool eastSide = (lon >= 0. && lon <= 180.);
        lon = ::fabs(lon);
        std::string retStr;
        if(lon < 10.)
            retStr += "00"; // lis�t��n tarvittaessa etu nollia
        else if(lon < 100.)
            retStr += "0"; // lis�t��n tarvittaessa etu nolla
        retStr += GetLatLonValue2MinuteStr(lon, decimals);
        if(eastSide)
            retStr += "E";
        else
            retStr += "W";

        return retStr;
    }

    std::string MakeSizeString(const NFmiPoint &theSize)
    {
        if(std::isnormal(theSize.X()) && std::isnormal(theSize.Y()))
        {
            std::string str;
            str += boost::lexical_cast<std::string>(boost::math::iround(theSize.X()));
            str += " x ";
            str += boost::lexical_cast<std::string>(boost::math::iround(theSize.Y()));
            return str;
        }
        else
            return "";
    }

    std::string MakeMapPortionPixelSizeStringForStatusbar(const NFmiPoint &theSize, bool longVersion)
    {
        std::string str;
        if(longVersion)
        {
            str += ::GetDictionaryString("View's map portion pixel size is");
            str += " ";
            str += MakeSizeString(theSize);
        }
        else
        {
            str += " (" + MakeSizeString(theSize) + " pix)";
        }
        return str;
    }

    std::string ColorFloat2HexStr(float value)
    {
        int iValue = static_cast<unsigned char>(value*255.f);
        return ToHex(iValue, 2, '0');
    }

    std::string Color2HtmlColorStr(const NFmiColor &theColor)
    {
        std::string str("#");
        str += ColorFloat2HexStr(theColor.GetRed());
        str += ColorFloat2HexStr(theColor.GetGreen());
        str += ColorFloat2HexStr(theColor.GetBlue());
        return str;
    }

    bool AreVectorValuesInRisingOrder(const std::vector<float> &theValueVector)
    {
        size_t ssize = theValueVector.size();
        if(ssize > 1)
        {
            for(size_t i = 0; i < ssize - 1; i++)
            {
                if(theValueVector[i] >= theValueVector[i + 1])
                    return false;
            }
        }
        return true;
    }

    NFmiPoint ConvertPointFromRect1ToRect2(const NFmiPoint &thePointInRect1, const NFmiRect &theRect1, const NFmiRect &theRect2)
    { // palauttaa pisteen joka on samassa suhteessa rect2:ssa kuin annettu piste

      // HUOM!!! ei ole t�ydellisesti tehty, molemmat rect:it alkavat t�ss� 0,0 pisteest�!!!!!!!!!
        return NFmiPoint(thePointInRect1.X() * theRect2.Width() / theRect1.Width(), thePointInRect1.Y() * theRect2.Height() / theRect1.Height());
    }

    // t�m� tekee sellaisen timebagin, miss� on annetut alku ja loopu ajat s��dettyin� l�himp��n aikasteppiin
    // kuitenkin siten ett� maksimi aikasteppi on 60 minuuttia.
    NFmiTimeBag GetAdjustedTimeBag(const NFmiMetTime &theStartTime, const NFmiMetTime &theEndTime, int theTimeStepInMinutes)
    {
        int timebagAdjustmentTimeStepInMinutes = (theTimeStepInMinutes > 60) ? 60 : theTimeStepInMinutes; // ei yli 60 minuutin s��t�� lopullisen timebagin aikoihin

        NFmiMetTime newStartTime = theStartTime;
        newStartTime.SetTimeStep(timebagAdjustmentTimeStepInMinutes);
        NFmiMetTime newEndTime = theEndTime;
        newEndTime.SetTimeStep(timebagAdjustmentTimeStepInMinutes);
        if(newStartTime >= newEndTime)
        {
            newEndTime = newStartTime;
            newEndTime.NextMetTime();
        }
        NFmiTimeBag newTimeBag(newStartTime, newEndTime, timebagAdjustmentTimeStepInMinutes);
        return newTimeBag;
    }

    std::string GetIdString(unsigned long theId)
    {
        std::string idStr("(=");
        idStr += NFmiStringTools::Convert<unsigned long>(theId);
        idStr += ")";
        return idStr;
    }

    // Jos annettu positiivinen arvo thelongerProducerNameMaxCharCount -parametrille, 
    // t�ll�in haetaan pitemp�� nime� tuottajalta ja lopuksi se leikataan haluttuun maksimi pituuteen.
    std::string GetProducerName(NFmiProducerSystem &theProducerSystem, boost::shared_ptr<NFmiDrawParam> &theDrawParam, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, bool fAddProdId, size_t theLongerProducerNameMaxCharCount)
    {
        bool betaProductCase = theLongerProducerNameMaxCharCount > 0;
        std::string prodNameStr = "X?"; // asetetaan aluksi nimeksi tuntematon tuottaja nimi, laitetaan siihen sitten my�hemmin parempaa, jos l�ytyy
                                        // etsi mallin nimi
        unsigned int modelIndex = theProducerSystem.FindProducerInfo(*(theDrawParam->Param().GetProducer()));
        if(modelIndex > 0)
            prodNameStr = betaProductCase ? theProducerSystem.Producer(modelIndex).Name() : theProducerSystem.Producer(modelIndex).UltraShortName();
        else
        {
            // on tapauksia, miss� drawParamissa ei ole oikeaa tuottajaa (esim. synop-plot), t�ll�in katsotaan l�ytyyk� l�ydetyn infon avulla tuottaja nime�
            if(theInfo)
            {
                unsigned int modelIndex2 = theProducerSystem.FindProducerInfo(*(theInfo->Producer()));
                if(modelIndex2 > 0)
                    prodNameStr = betaProductCase ? theProducerSystem.Producer(modelIndex2).Name() : theProducerSystem.Producer(modelIndex2).UltraShortName();
            }
        }

        if(fAddProdId && theInfo)
            prodNameStr += GetIdString(theInfo->Producer()->GetIdent());

        if(betaProductCase && theLongerProducerNameMaxCharCount < prodNameStr.size())
            prodNameStr.resize(theLongerProducerNameMaxCharCount); // leikataan nime� jos tarvis

        return prodNameStr;
    }

    std::string GetParamNameString(boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool fCrossSectionInfoWanted, bool fAddIdInfos, bool fMakeTooltipXmlEncode, size_t theLongerProducerNameMaxCharCount, bool fTimeSerialViewCase, bool fShowModelOriginTime, boost::shared_ptr<NFmiFastQueryInfo> possibleInfo)
    {
        CtrlViewDocumentInterface* ctrlViewDocumentInterface = CtrlViewDocumentInterface::GetCtrlViewDocumentInterfaceImplementation();
        std::string normalOrigTimeFormat = ::GetDictionaryString("MapViewToolTipOrigTimeNormal");
        std::string minuteOrigTimeFormat = ::GetDictionaryString("MapViewToolTipOrigTimeMinute");
        bool betaProductCase = theLongerProducerNameMaxCharCount > 0;
        NFmiInfoData::Type dataType = theDrawParam->DataType();
        std::string str;
        boost::shared_ptr<NFmiFastQueryInfo> info = possibleInfo ? possibleInfo : ctrlViewDocumentInterface->InfoOrganizer()->Info(theDrawParam, fCrossSectionInfoWanted, true);
        if(theDrawParam->IsModelRunDataType())
        {
            if(info)
            {
                str += GetProducerName(ctrlViewDocumentInterface->ProducerSystem(), theDrawParam, info, fAddIdInfos, theLongerProducerNameMaxCharCount);

                if(theDrawParam->UseArchiveModelData())
                { // laitetaan arkisto datan kohdalle viittaus verrattuna viimeiseen malli dataan esim. H[-1] joka on siis edellinen malli jne.
                    str += "[";
                    if(theDrawParam->ModelRunIndex() != 0)
                        str += NFmiStringTools::Convert<int>(theDrawParam->ModelRunIndex());
                    else
                        str += "?"; // katsotaan my�hemmin miten absoluutti aika laitetaan
                    str += "]";
                }
                else
                {
                    if(fShowModelOriginTime)
                    {
                        if(betaProductCase)
                            str += " ";
                        str += info->OriginTime().ToStr(NFmiString(normalOrigTimeFormat));
                    }
                }

                if(fTimeSerialViewCase && theDrawParam->TimeSerialModelRunCount() > 0)
                { // laitetaan multimodelrun merkit n�kyviin
                    str += " (";
                    str += NFmiStringTools::Convert<int>(theDrawParam->TimeSerialModelRunCount());
                    str += "x)";
                }

                str += " ";
            }
            else // jos infoa ei l�ytynyt, laitetaan merkit mahdollisen tuottaja nimen ymp�rille sen merkiksi
            {
                str += "#";
                str += GetProducerName(ctrlViewDocumentInterface->ProducerSystem(), theDrawParam, info, fAddIdInfos, theLongerProducerNameMaxCharCount);
                str += "# ";
            }
        }
        else if(dataType == NFmiInfoData::kSingleStationRadarData && info)
        { // t�h�n laitetaan toistaiseksi tuottaja suoraan infon parametrista
            NFmiProducer *prod = info->Producer();
            if(prod)
            {
                str += prod->GetName();
                if(fAddIdInfos)
                    str += GetIdString(prod->GetIdent());
                str += " ";
            }
        }
        else if(dataType == NFmiInfoData::kObservations || dataType == NFmiInfoData::kAnalyzeData)
        {
            // etsi havainto tuottajan nimi
            str += GetProducerName(ctrlViewDocumentInterface->ObsProducerSystem(), theDrawParam, info, fAddIdInfos, theLongerProducerNameMaxCharCount);
            str += " ";
        }
        else if(dataType == NFmiInfoData::kEditingHelpData)
        {
            if(betaProductCase)
                str += "HelpEdit";
            else
                str += GetEditingDataString("HE", info, ctrlViewDocumentInterface->Language(), minuteOrigTimeFormat);
        }
        else if(dataType == NFmiInfoData::kKepaData)
        {
            if(betaProductCase)
                str += "Official";
            else
                str += GetEditingDataString("OF", info, ctrlViewDocumentInterface->Language(), minuteOrigTimeFormat);
        }

        if(dataType == NFmiInfoData::kCopyOfEdited)
            str += "(c)"; // kopioidut parametrit merkit��n (c):ll�
        str += theDrawParam->ParameterAbbreviation();
        if(fAddIdInfos)
            str += GetIdString(theDrawParam->Param().GetParamIdent());

        if(fCrossSectionInfoWanted == false)
        { // muualla kuin poikkileikkauksissa halutaan level infoa
            if(dataType == NFmiInfoData::kHybridData || theDrawParam->Level().LevelType() == kFmiHybridLevel)
            { // laitetaan hybrid datalle parametrin per��n viel "xx", miss� xx on hybrid levelin numero
                str += "_L" + NFmiStringTools::Convert(theDrawParam->Level().LevelValue());
            }
            else if(theDrawParam->Level().LevelType() == kFmiHeight)
            { // laitetaan hybrid datalle parametrin per��n viel "xx", miss� xx on hybrid levelin numero
                str += "_" + NFmiStringTools::Convert(theDrawParam->Level().LevelValue()) + "m";
            }
            else if(theDrawParam->Level().LevelType() == kFmiPressureLevel || theDrawParam->Level().LevelType() == kFmiSoundingLevel)
            { // laitetaan painepinta datalle parametrin per��n viel "xxx", miss� xxx on painepinta levelin arvo tarvittavien etu nollien kera
              // tehd��n se my�s kFmiSpSoundingPlot eli sounding plot paramille
                std::string levStr;
                levStr += "_";
                if(theDrawParam->Level().LevelValue() < 10)
                    levStr += "00";
                else if(theDrawParam->Level().LevelValue() < 100)
                    levStr += "0";

                levStr += NFmiStringTools::Convert(theDrawParam->Level().LevelValue());
                str += levStr;
            }
        }
        
        if(theDrawParam->ShowDifferenceToOriginalData())
            str += "(o)";

        if(fMakeTooltipXmlEncode)
            str = CtrlViewUtils::XmlEncode(str);
        return str;
    }

    NFmiColor GetParamTextColor(NFmiInfoData::Type dataType, bool useArchiveModelData, CtrlViewDocumentInterface *theCtrlViewDocumentInterface)
    {
        if(useArchiveModelData)
            return NFmiColor(0.016f, 0.64f, 0.68f);

        if(NFmiDrawParam::IsMacroParamCase(dataType))
            return NFmiColor(0.5f, 0.f, 0.5f);

        switch(dataType)
        {
        case NFmiInfoData::kSatelData:
            return NFmiColor(0.f, 0.45f, 0.9f);
        case NFmiInfoData::kViewable:
        case NFmiInfoData::kModelHelpData:
        case NFmiInfoData::kHybridData:
        case NFmiInfoData::kTrajectoryHistoryData:
            return NFmiColor(0.f, 0.5f, 0.f);
        case NFmiInfoData::kObservations:
        case NFmiInfoData::kFlashData:
        case NFmiInfoData::kSingleStationRadarData:
            return NFmiColor(0.f, 0.f, 1.f);
        case NFmiInfoData::kAnalyzeData:
            return NFmiColor(0.94f, 0.139f, 0.126f);
        case NFmiInfoData::kKepaData:
            return NFmiColor(0.78f, 0.082f, 0.52f);
        case NFmiInfoData::kConceptualModelData:
        case NFmiInfoData::kCapData:
            return NFmiColor(1.f, 0.f, 0.5f);
        case NFmiInfoData::kEditingHelpData:
            return theCtrlViewDocumentInterface->HelpColor();
        case NFmiInfoData::kClimatologyData:
            return NFmiColor(0.f, 0.5f, 0.f);
        case NFmiInfoData::kWmsData:
            return NFmiColor(0.67f, 0.3f, 0.07f);
        case NFmiInfoData::kMapLayer:
            return NFmiColor(0.8f, 0.22f, 0.f);
        default:
            return NFmiColor(0.f, 0.f, 0.f);
        }
    }

    std::string GetEditingDataString(const std::string &theNameStr, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, FmiLanguage lang, const std::string &theOrigTimeFormat)
    {
        std::string returnStr = theNameStr;
        if(theInfo)
        {
            returnStr += theInfo->OriginTime().ToStr(NFmiString(theOrigTimeFormat), lang);
        }
        returnStr += " ";

        return returnStr;
    }

    std::string GetTotalMapViewStatusBarStr(CtrlViewDocumentInterface *theCtrlViewDocumentInterface, const NFmiPoint &theLatlon)
    {
        std::string str(" Location: ");
        double lat = theLatlon.Y();
        str += GetLatitudeMinuteStr(lat, 1);
        str += " ";
        double lon = theLatlon.X();
        lon = GetWantedLongitude(lon, false);
        str += GetLongitudeMinuteStr(lon, 1);
        str += " [";

        std::stringstream out;
        out << std::fixed << std::setprecision(4) << std::setw(7) << std::setfill('0') << std::right << ::fabs(lat) << ((lat >= 0) ? "N" : "S");
        out << " ";
        out << std::fixed << std::setprecision(4) << std::setw(8) << std::setfill('0') << std::right << ::fabs(lon) << ((lon >= 0 && lon < 180) ? "E" : "W");
        str += out.str();

        str += "] Elevation: ";
        float elevation = kFloatMissing;
        boost::shared_ptr<NFmiFastQueryInfo> topoInfo = theCtrlViewDocumentInterface->InfoOrganizer()->FindInfo(NFmiInfoData::kStationary);
        if(topoInfo)
        {
            if(topoInfo->Param(kFmiTopoGraf))
                elevation = topoInfo->InterpolatedValue(theLatlon);
        }

        if(elevation == kFloatMissing)
            str += "?";
        else
        {
            str += NFmiStringTools::Convert<int>(static_cast<int>(elevation));
            str += " m";
        }
        return str;
    }

    boost::shared_ptr<NFmiFastQueryInfo> GetLatestLastTimeObservation(boost::shared_ptr<NFmiDrawParam> &theDrawParam, CtrlViewDocumentInterface *theCtrlViewDocumentInterface, bool fCrossSectionInfoWanted)
    {
        NFmiInfoData::Type dataType = theDrawParam->DataType();
        if(dataType == NFmiInfoData::kObservations || dataType == NFmiInfoData::kAnalyzeData || dataType == NFmiInfoData::kSingleStationRadarData || dataType == NFmiInfoData::kFlashData)
        {
            auto producerId = theDrawParam->Param().GetProducer()->GetIdent();
            if(producerId != kFmiSYNOP && producerId != kFmiFlashObs)
                return theCtrlViewDocumentInterface->InfoOrganizer()->Info(theDrawParam, fCrossSectionInfoWanted, true);
            else
            {
                boost::shared_ptr<NFmiArea> dummyArea;
                std::vector<boost::shared_ptr<NFmiFastQueryInfo>> infos;
                theCtrlViewDocumentInterface->MakeDrawedInfoVectorForMapView(infos, theDrawParam, dummyArea);
                if(infos.size())
                {
                    size_t latestIndex = 0;
                    NFmiMetTime latestLastTime = infos[0]->TimeDescriptor().LastTime();
                    for(size_t i = 1; i < infos.size(); i++)
                    {
                        if(latestLastTime < infos[i]->TimeDescriptor().LastTime())
                        {
                            latestLastTime = infos[i]->TimeDescriptor().LastTime();
                            latestIndex = i;
                        }
                    }
                    return infos[latestIndex];
                }
            }
        }
        return boost::shared_ptr<NFmiFastQueryInfo>();
    }

    std::string GetLatestObservationTimeString(boost::shared_ptr<NFmiDrawParam> &theDrawParam, CtrlViewDocumentInterface *theCtrlViewDocumentInterface, const std::string &theTimeFormat, bool fCrossSectionInfoWanted)
    {
        std::string str;
        boost::shared_ptr<NFmiFastQueryInfo> info = GetLatestLastTimeObservation(theDrawParam, theCtrlViewDocumentInterface, fCrossSectionInfoWanted);
        if(info)
        {
            str += " (";
            str += ::GetDictionaryString("latest obs.");
            str += ": ";
            str += info->TimeDescriptor().LastTime().ToStr(NFmiString(theTimeFormat));
            str += ")";
        }
        return str;
    }

    std::string GetArchiveOrigTimeString(boost::shared_ptr<NFmiDrawParam> &theDrawParam, CtrlViewDocumentInterface *theCtrlViewDocumentInterface, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, bool fGetCurrentDataFromQ2Server, const std::string &theDictionaryTokenForOrigTimeFormat)
    {
        std::string str;
        if(theDrawParam->UseArchiveModelData())
        {
            str += "<b><font color=";
            str += Color2HtmlColorStr(NFmiColor(0.016f, 0.64f, 0.68f));
            str += ">";
            str += "\nOrigTime: ";
            NFmiMetTime archiveOrigTime;
            if(fGetCurrentDataFromQ2Server == false && theInfo)
                archiveOrigTime = theInfo->OriginTime();
            else
                archiveOrigTime = theCtrlViewDocumentInterface->GetModelOrigTime(theDrawParam);
            str += archiveOrigTime.ToStr(::GetDictionaryString(theDictionaryTokenForOrigTimeFormat.c_str()), theCtrlViewDocumentInterface->Language());
            str += "</font></b>";
        }
        return str;
    }

    // E.g. tooltip system uses html. So if original text has characters like '<' or '>', they have to
    // be replaced with &lt; and &gt; strings so that html output won't be broken.
    std::string XmlEncode(const std::string &src)
    {
        std::stringstream dst;
        for(char ch : src) 
        {
            switch(ch) {
            case '&': dst << "&amp;"; break;
            case '\'': dst << "&apos;"; break;
            case '"': dst << "&quot;"; break;
            case '<': dst << "&lt;"; break;
            case '>': dst << "&gt;"; break;
            default: dst << ch; break;
            }
        }
        return dst.str();
    }

    FmiDirection CalcFollowingParamWindowViewPosition(FmiDirection currentPosition, bool forward)
    {
        if(forward)
        {
            // kTopLeft -> kTopRight -> kBottomRight -> kBottomLeft -> kNoDirection (piilossa) -> kTopLeft...
            switch(currentPosition)
            {
            case kTopLeft:
                return kTopRight;
                break;
            case kTopRight:
                return kBottomRight;
                break;
            case kBottomRight:
                return kBottomLeft;
                break;
            case kBottomLeft:
                return kNoDirection;
                break;
            default:
                return kTopLeft;
                break;
            }
        }
        else
        {
            // kTopLeft -> kNoDirection (piilossa) -> kBottomLeft -> kBottomRight -> kTopRight -> kTopLeft...
            switch(currentPosition)
            {
            case kNoDirection:
                return kBottomLeft;
                break;
            case kTopLeft:
                return kNoDirection;
                break;
            case kBottomRight:
                return kTopRight;
                break;
            case kBottomLeft:
                return kBottomRight;
                break;
            default:
                return kTopLeft;
                break;
            }
        }
    }

    FileNameWithTimeList TimeSortFiles(FileNameWithTimeList filesWithTimesCopy, bool descending)
    {
        filesWithTimesCopy.sort(
            [=](const auto& pair1, const auto& pair2)
            {
                if(descending)
                    return pair1.second > pair2.second;
                else
                    return pair1.second < pair2.second;
            }
        );
        return filesWithTimesCopy;
    }

    // Calculate direction from point1 to point2.
    // Result is in degrees (0 -> 360), 'north' is 0 and goes clockwise (east 90, south 180, west 270).
    // If points are the same, result is 0.
    double CalcAngle(const NFmiPoint& point1, const NFmiPoint& point2)
    {
        auto x = point2.X() - point1.X();
        auto y = point2.Y() - point1.Y();
        return CalcAngle(x, y);
    }

    double CalcAngle(double x, double y)
    {
        if(x == 0 && y == 0)
            return 0;
        auto origDirection = atan2(x, y) * 180 / kPii;
        if(origDirection < 0)
            return 360 + origDirection;
        else
            return origDirection;
    }

    bool IsModularParameter(FmiParameterName theParam)
    {
        switch(theParam)
        {
        case kFmiWindDirection:
        case kFmiWaveDirection:
        case kFmiWaveDirectionBandB:
        case kFmiWaveDirectionBandC:
        case kFmiWaveDirectionSwell0:
        case kFmiWaveDirectionSwell1:
        case kFmiWaveDirectionSwell2:
            return true;
        default:
            return false;
        }
    }
} // namespace CtrlViewUtils
