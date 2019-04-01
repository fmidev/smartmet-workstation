#include "NFmiAnimationData.h"
#include "NFmiDataStoringHelpers.h"

NFmiAnimationData::NFmiAnimationData(void)
    :fShowTimesOnTimeControl(false)
    , fAnimationOn(false)
    , itsFrameDelayInMS(400)
    , itsLastFrameDelayFactor(1.)
    , itsTimes()
    , itsCurrentTime()
    , itsShowTimebag()
    , itsTimeStepInMinutes(60)
    , itsRunMode(kNormal)
    , itsLockMode(kNoLock)
    , itsLockModeTimeDifferenceInMinutes(0)
    , fLastDirectionForward(true)
    , itsTimer()
    , itsMouseDragStartTime()
    , fShowVerticalControl(false)
{
    NFmiMetTime tmpTime1(itsTimeStepInMinutes);
    NFmiMetTime tmpTime2(itsTimeStepInMinutes);
    tmpTime2.ChangeByMinutes(itsTimeStepInMinutes * 10);
    NFmiTimeBag tmpTimes(tmpTime1, tmpTime2, itsTimeStepInMinutes); // luodaan aluksi joku defauultti timebagi
    Times(tmpTimes);
}

// Tarkistetaan, voidaanko aika-askel eteenp‰in tehd‰ ja jos voidaan,
// suoritetaan siihen liittyv‰t operaatiot ja palautetaan true.
// Jos ei voida astua eteenp‰in, palautetaan false (jolloin tehd‰‰n kutsuvassa koodissa jotain muuta).
bool NFmiAnimationData::MoveCurrentTimeForward(int theReducedAnimationTimeSteps)
{
    NFmiMetTime usedLastTime = itsTimes.LastTime();
    usedLastTime.ChangeByMinutes(-theReducedAnimationTimeSteps * itsTimeStepInMinutes); // siirret‰‰n viimeist‰ aikaa halutun m‰‰r‰n verran taaksep‰in (theReducedAnimationTimeSteps on normaali tapauksessa 0, joten t‰llˆin ei siirtoa tapahdu)
    if(usedLastTime < itsTimes.FirstTime())
        return false; // tietyiss‰ tilanteissa siirto voi menn‰ animaation alun ohi menneisyyteen, jolloin on todettava ett‰ eteenp‰in ei voi menn‰
    bool wasOnLastTime = (itsCurrentTime >= usedLastTime); // oltiinko jo viimeisess‰ ajassa tai sen yli
    if(wasOnLastTime == false)
    { // jos ei ollut viimeisess‰ ajassa
        itsCurrentTime.SetTimeStep(static_cast<short>(itsTimeStepInMinutes), true); // jos steppi tms on muuttunut lennossa, pit‰‰ asetell‰ arvoja varmuuden vuoksi
        itsCurrentTime.NextMetTime();
        CurrentTime(itsCurrentTime); // t‰m‰ tekee tietyt tarkastelut ja s‰‰dˆt asetettavalle ajalle
        return true;
    }
    else
        return false;
}
// Tarkistetaan, voidaanko aika-askel taaksep‰in voidaan tehd‰ ja jos voidaan,
// suoritetaan siihen liittyv‰t operaatiot ja palautetaan true.
// Jos ei voida astua taaksep‰in, palautetaan false (jolloin tehd‰‰n kutsuvassa koodissa jotain muuta).
bool NFmiAnimationData::MoveCurrentTimeBackward(void)
{
    bool wasOnFirstTime = (itsCurrentTime == itsTimes.FirstTime()); // oltiinko jo ensimm‰isess‰ ajassa
    if(wasOnFirstTime == false)
    { // jos ei ollut ensimm‰isess‰ ajassa
        itsCurrentTime.SetTimeStep(static_cast<short>(itsTimeStepInMinutes), true); // jos steppi tms on muuttunut lennossa, pit‰‰ asetell‰ arvoja varmuuden vuoksi
        itsCurrentTime.PreviousMetTime();
        CurrentTime(itsCurrentTime); // t‰m‰ tekee tietyt tarkastelut ja s‰‰dˆt asetettavalle ajalle
        return true;
    }
    else
        return false;
}

double NFmiAnimationData::CurrentFrameDelayFactor(void) const
{
    // jos ollaan viimeisess‰ framessa, palauta sille s‰‰detty kerroin, muuten palauta vain 1.
    if(itsCurrentTime == itsTimes.LastTime())
        return itsLastFrameDelayFactor;
    else
        return 1.;
}

void NFmiAnimationData::LastFrameDelayFactor(double newValue)
{
    itsLastFrameDelayFactor = newValue;
    if(itsLastFrameDelayFactor < 1.)
        itsLastFrameDelayFactor = 1.;
}

double NFmiAnimationData::CalcUsedDelayInMSForCurrentFrame(void) const
{
    double usedDelay = FrameDelayInMS() * CurrentFrameDelayFactor();
    if(usedDelay == 0)
        usedDelay = 1 * CurrentFrameDelayFactor();
    return usedDelay;
}

// Lasketaan kuinka kauan on viel‰ odotettava ett‰ seuraava animaatio frame on piirrett‰v‰.
// Jos paluu arvo on positiivinen, on odotettava niin monta millisekuntia.
// Jos paluu arvo on negatiivinen, piirto on aloitettava heti.
// JOS animaatio ei ole k‰ynniss‰, plautetaan vain iso vakio luku.
double NFmiAnimationData::CalcWaitTimeInMSForNextFrame(void) const
{
    if(IsAnimationRunning())
    {
        double usedDelayInMS = CalcUsedDelayInMSForCurrentFrame();
        double waitTimeInMS = usedDelayInMS - TimePassedInMS();
        return waitTimeInMS;
    }
    else
        return kNoAnimationWaitTime;
}

// Kun siirryt‰‰n katsomaan jotain CaseStudya tai tullaan sellaisen katselusta pois takaisin normaaliin moodiin,
// On hyv‰ s‰‰t‰‰ animaatio aikoja vastaavasti. S‰‰dˆt tehd‰‰n niin kuin aikoja s‰‰dett‰isiin uuden sein‰kelloajan mukaan.
// Esim. Jos ollaan normaali moodissa, otetaan nykyhetken aika theFromWallClockTime-parametriin ja laitetaan
// ladattavan CaseStudyn 'sein‰kello' aika theToWallClockTime-parametriin. Lasketaan niiden erotus ja 
// Siirret‰‰n kaikkia aikoja sen mukaan eteen- tai taaksep‰in.
void NFmiAnimationData::AdjustTimesRelatedtoCaseStudyModeChange(const NFmiMetTime &theFromWallClockTime, const NFmiMetTime &theToWallClockTime)
{
    long diffInMinutes = theToWallClockTime.DifferenceInMinutes(theFromWallClockTime);
    itsTimes.MoveByMinutes(diffInMinutes);
    itsCurrentTime.ChangeByMinutes(diffInMinutes);
    itsShowTimebag.MoveByMinutes(diffInMinutes);
}

// tekee yhden aika-askeleen muutokset valittuun aikaan riippuen asetuksista,
// t‰m‰n j‰lkeen kyseist‰ aikaa voidaan kysy‰ CurrentTime-metodilla
// Paluu arvot: 0 jos ei animaatio p‰‰ll‰, 1 jos odotetaan viel‰ delay-ajan kulumista, 2 jos on aika tehd‰ animaatio.
// Tietyiss‰ tilanteissa ei haluta ajaa animaatiota loppuun asti, sit‰ varten theReducedAnimationTimeSteps -parametrilla 
// voidaan animaatio sykli‰ lyhent‰‰ (ks. GeneralDocImpl::CalcReducedAnimationSteps -metodin dokumentaatiota).
int NFmiAnimationData::Animate(int theReducedAnimationTimeSteps)
{
    int status = 0;
    if(IsAnimationRunning())
    {
        double waitTimeInMS = CalcWaitTimeInMSForNextFrame();
        if(waitTimeInMS < 0)
        {
            if(itsRunMode == kNormal)
            { // normaali eteenp‰in menev‰ animaatio
                if(MoveCurrentTimeForward(theReducedAnimationTimeSteps) == false)
                    itsCurrentTime = itsTimes.FirstTime(); // jos oltiin jo viimeisess‰ ajassa, laita aika alkuun
            }
            else if(itsRunMode == kBackWard)
            { // normaali eteenp‰in menev‰ animaatio
                if(MoveCurrentTimeBackward() == false)
                    itsCurrentTime = itsTimes.LastTime(); // jos oltiin jo 1. ajassa, laita aika loppuun takaisin
            }
            else if(itsRunMode == kSweep)
            { // edes takaisin menev‰ animaatio
                if(fLastDirectionForward)
                {
                    if(MoveCurrentTimeForward(theReducedAnimationTimeSteps) == false)
                    {
                        fLastDirectionForward = false;
                        MoveCurrentTimeBackward();
                    }
                }
                else
                {
                    if(MoveCurrentTimeBackward() == false)
                    {
                        fLastDirectionForward = true;
                        MoveCurrentTimeForward(theReducedAnimationTimeSteps);
                    }
                }
            }
            status = 2; // animaatio pit‰‰ tehd‰
            ReStartTimer(); // k‰ynnistet‰‰n timeri uudestaan animaation j‰lkeen
        }
        else
            status = 1; // odotetaan viel‰ ett‰ delay-aika menee ohi
    }
    return status;
}

void NFmiAnimationData::ToggleRunMode(void)
{
    if(itsRunMode == kNormal)
        itsRunMode = kSweep;
    else if(itsRunMode == kSweep)
        itsRunMode = kBackWard;
    else
        itsRunMode = kNormal;
}

void NFmiAnimationData::ToggleLockMode(void)
{
    if(itsLockMode == kNoLock)
        itsLockMode = kFollowLastObservation;
    else if(itsLockMode == kFollowLastObservation)
        itsLockMode = kFollowEarliestLastObservation;
    //	else if(itsLockMode == kFollowLastObservation)
    //		itsLockMode = kLockToCurrentTimeOffset;
    else
        itsLockMode = kNoLock;
}

void NFmiAnimationData::AnimationOn(bool newValue)
{
    fAnimationOn = newValue;
    ReStartTimer();
}

void NFmiAnimationData::ShowTimesOnTimeControl(bool newValue)
{
    fShowTimesOnTimeControl = newValue;
    if(fAnimationOn)
        ReStartTimer();
}

// Onko timebagin resoluutio sama kuin annetun timebagin.
// Lis‰ksi onko alku ja loppuaikojen resoluutiot myˆs samat.
// theTimeRes on resoluutio minuuteissa
static bool IsTimeBagTimeResolutionsOk(const NFmiTimeBag &theTimeBag, int theTimeRes)
{
    if(theTimeBag.Resolution() == theTimeRes && theTimeBag.FirstTime().GetTimeStep() == theTimeRes && theTimeBag.LastTime().GetTimeStep() == theTimeRes)
        return true;
    else
        return false;
}

static NFmiTimeBag CalcFixedTimeBag(const NFmiTimeBag &theTimeBag, int theTimeRes)
{
    NFmiTimeBag retTimes = theTimeBag;
    if(retTimes.FirstTime() > retTimes.LastTime())
    {
        // Jollain ilmeell‰ on saatu aikaan timebagi, jossa on ollut 1. aika on ollut myˆh‰isempi aika kuin viimeinen,
        // t‰ss‰ simppeli ratkaisu ja vain vaihdetaan em. aikojen j‰rjestyst‰...
        retTimes = NFmiTimeBag(retTimes.LastTime(), retTimes.FirstTime(), theTimeRes);
    }
    if(!::IsTimeBagTimeResolutionsOk(retTimes, theTimeRes))
    { // pit‰‰ fiksailla timebagia
        NFmiMetTime tmpTime1(retTimes.FirstTime());
        tmpTime1.SetTimeStep(theTimeRes, true);
        NFmiMetTime tmpTime2(retTimes.LastTime());
        tmpTime2.SetTimeStep(theTimeRes, true);
        if(tmpTime1 == tmpTime2)
            tmpTime2.NextMetTime();
        retTimes = NFmiTimeBag(tmpTime1, tmpTime2, theTimeRes);
    }
    return retTimes;
}

// 1. Pit‰‰ tarkistaa onko annettu timebagi s‰‰detyn aika-askeleen mukainen
// 2. Tarvittaessa lasketaan timebagin alku ja loppu ajat uudestaan
// 3. Lopuksi asetetaan t‰m‰ timebagi myˆs itsShowTimebag:ksi (HUOM! ei saa k‰ytt‰‰ ShowTimebag(timeBag)-metodia!!!!!)
void NFmiAnimationData::Times(const NFmiTimeBag &newValue)
{
    itsTimes = ::CalcFixedTimeBag(newValue, itsTimeStepInMinutes);
    itsShowTimebag = itsTimes; // HUOM! ei saa asettaa ShowTimebag(times) metodilla!!
}

void NFmiAnimationData::CurrentTime(const NFmiMetTime &newValue)
{
    if(itsTimes.IsInside(newValue))
        itsCurrentTime = newValue;
    else
    { // currenttia animointi aikaa pit‰‰ s‰‰t‰‰ niin ett‰ se animointi-aikojen alueella
        if(itsTimes.FindNearestTime(newValue))
            itsCurrentTime = itsTimes.CurrentTime();
        else
        {
            // t‰h‰n ei oikeastaan pit‰isi pysty‰ edes menem‰‰n, mit‰ nyt tehd‰‰n?!?!?
        }
    }
}

void NFmiAnimationData::TimeStepInMinutes(int newValue)
{
    if(newValue <= 0)
        newValue = 60;
    itsTimeStepInMinutes = newValue;
    Times(itsTimes);
}

void NFmiAnimationData::ShowTimebag(const NFmiTimeBag &newValue)
{
    NFmiTimeBag usedNewTimeBag(newValue);
    if(usedNewTimeBag.FirstTime() > usedNewTimeBag.LastTime())
    { // hiirell‰ raahatessa voi timebagin alku ja loppu ajat olla v‰‰rin p‰in, t‰llˆin ajat pit‰‰ k‰‰nt‰‰
        usedNewTimeBag = NFmiTimeBag(usedNewTimeBag.LastTime(), usedNewTimeBag.FirstTime(), usedNewTimeBag.Resolution());
    }
    itsShowTimebag = usedNewTimeBag;
    itsTimes = ::CalcFixedTimeBag(itsShowTimebag, itsTimeStepInMinutes); // HUOM! itsTimes:ia ei saa asettaa Timebag(times) metodilla!!
}


void NFmiAnimationData::Write(std::ostream& os) const
{
    // HUOM!! t‰h‰n ei saa laittaa kommentteja, koska t‰m‰ talletetaan n‰yttˆmakrossa extra-data stringiin ja
    // niiden luvussa ei osata ottaa huomioon, ett‰ joku on poistanut kaikki kommentit tekstist‰ luku vaiheessa.

    os << fShowTimesOnTimeControl << " " << fAnimationOn << " " << itsFrameDelayInMS << std::endl;
    NFmiMetTime usedViewMacroTime = NFmiDataStoringHelpers::GetUsedViewMacroTime();
    NFmiDataStoringHelpers::WriteTimeBagWithOffSets(usedViewMacroTime, itsTimes, os);
    os << itsTimeStepInMinutes << " " << itsRunMode << " " << itsLockMode << " " << itsLockModeTimeDifferenceInMinutes << std::endl;

    NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
                                                            // Kun tulee uusia muuttujia, tee t‰h‰n extradatan t‰yttˆ‰, jotta se saadaan talteen tiedopstoon siten ett‰
                                                            // edelliset versiot eiv‰t mene solmuun vaikka on tullut uutta dataa.

    extraData.Add(itsLastFrameDelayFactor); // itsLastFrameDelayFactor on 1. uusi double arvo
    extraData.Add(static_cast<double>(fShowVerticalControl)); // fShowVerticalControl on 2. uusi 'double' arvo

    os << extraData;

    if(os.fail())
        throw std::runtime_error("NFmiAnimationData::Write failed");
}

void NFmiAnimationData::Read(std::istream& is)
{
    is >> fShowTimesOnTimeControl >> fAnimationOn >> itsFrameDelayInMS;
    NFmiMetTime usedViewMacroTime = NFmiDataStoringHelpers::GetUsedViewMacroTime();
    NFmiTimeBag times;
    NFmiDataStoringHelpers::ReadTimeBagWithOffSets(usedViewMacroTime, times, is);
    int tmpRunMode = 0;
    int tmpLockMode = 0;
    is >> itsTimeStepInMinutes >> tmpRunMode >> tmpLockMode >> itsLockModeTimeDifferenceInMinutes;
    itsRunMode = static_cast<RunMode>(tmpRunMode);
    itsLockMode = static_cast<AnimationLockMode>(tmpLockMode);
    // T‰ss‰ tehd‰‰n tarvittavat tarkastelut ja asetukset luetulle timebagille
    Times(times); 

    if(is.fail())
        throw std::runtime_error("NFmiAnimationData::Read failed");

    NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi viel‰ mahdollinen extra data
    is >> extraData;
    // T‰ss‰ sitten otetaaan extradatasta talteen uudet muuttujat, mit‰ on mahdollisesti tullut
    // eli jos uusia muutujia tai arvoja, k‰sittele t‰ss‰.

    itsLastFrameDelayFactor = 1.;
    if(extraData.itsDoubleValues.size() >= 1)
        itsLastFrameDelayFactor = extraData.itsDoubleValues[0];
    fShowVerticalControl = false;
    if(extraData.itsDoubleValues.size() >= 2)
        fShowVerticalControl = (extraData.itsDoubleValues[1] == 0) ? false : true;

    if(is.fail())
        throw std::runtime_error("NFmiAnimationData::Read failed");
}
