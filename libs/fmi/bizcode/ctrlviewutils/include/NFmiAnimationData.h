#pragma once

#include "NFmiTimeBag.h"
#include "NFmiMilliSecondTimer.h"

class NFmiAnimationData
{
public:
    static const int kNoAnimationWaitTime = 9999999;
    typedef enum
    {
        kNormal = 1, // animaatio alkaa aina alusta, kun on p‰‰ssyt loppuun
        kSweep = 2, // sweep mode vatkaa animaatiota edestakaisin, eli kun p‰‰see loppuun, peruuttaa se taas askel askeleelta alkuun ja alkaa taas alusta edet‰ loppuun
        kBackWard = 3 // sama kuin normaali moodi, mutta takaperin
    } RunMode; // kuinka perus animaatiota juoksutetaan
    typedef enum
    {
        kNoLock = 1, // normaali moodi, kun animaatio aika on valittu, animointiaikav‰li ei itsest‰‰n muutu mitenk‰‰n
        kFollowLastObservation = 2, // tarkastelee animoitavia datoja ja liikuttaa animointi aikoja niin ett‰ viimeinen animoitu aika on sama kuin viimeinen aika, mist‰ lˆytyy havaintoja (aikav‰li pysyy vakiona eli alkup‰‰ liikkuu samalla eteenp‰in)
        kFollowEarliestLastObservation = 3 // tarkastelee animoitavia datoja ja liikuttaa animointi aikoja niin ett‰ viimeinen animoitu aika on sama kuin sen n‰kyv‰n havainnon, mist‰ lˆytyy aikaisin viimeinen aika
//			kLockToCurrentTimeOffset = 4 // lukitus sein‰kelloon eli otetaan talteen offsett currenttiin aikaan ja liikutetaan animaatio aikoja sen mukaan kun aika kuluu
    } AnimationLockMode; // kuinka animaation alku/loppu ajat liikkuv‰t automaattisesti ajan kuluessa

    NFmiAnimationData(void);
    int Animate(int theReducedAnimationTimeSteps);
    bool ShowTimesOnTimeControl(void) const { return fShowTimesOnTimeControl; }
    void ShowTimesOnTimeControl(bool newValue);
    // t‰ll‰ kysyt‰‰n pyˆriikˆ animaatio, eli pit‰‰ oll‰ animaation‰yttˆ p‰‰ll‰ ja animointi p‰‰ll‰ molemmat
    bool IsAnimationRunning(void) const { return fAnimationOn && fShowTimesOnTimeControl; }
    bool AnimationOn(void) const { return fAnimationOn; }
    void AnimationOn(bool newValue);
    int FrameDelayInMS(void) const { return itsFrameDelayInMS; }
    void FrameDelayInMS(int newValue) { itsFrameDelayInMS = newValue; }
    const NFmiTimeBag& Times(void) const { return itsTimes; }
    void Times(const NFmiTimeBag &newValue);
    const NFmiMetTime& CurrentTime(void) const { return itsCurrentTime; }
    void CurrentTime(const NFmiMetTime &newValue);
    const NFmiTimeBag& ShowTimebag(void) const { return itsShowTimebag; }
    void ShowTimebag(const NFmiTimeBag &newValue);
    int TimeStepInMinutes(void) const { return itsTimeStepInMinutes; }
    void TimeStepInMinutes(int newValue);
    RunMode GetRunMode(void) const { return itsRunMode; }
    void SetRunMode(RunMode newValue) { itsRunMode = newValue; }
    AnimationLockMode LockMode(void) const { return itsLockMode; }
    void LockMode(AnimationLockMode newValue) { itsLockMode = newValue; }
    int LockModeTimeDifferenceInMinutes(void) const { return itsLockModeTimeDifferenceInMinutes; }
    void LockModeTimeDifferenceInMinutes(int newValue) { itsLockModeTimeDifferenceInMinutes = newValue; }
    bool LastDirectionForward(void) const { return fLastDirectionForward; }
    void LastDirectionForward(bool newValue) { fLastDirectionForward = newValue; }
    void ReStartTimer(void) { itsTimer.StartTimer(); }
    int TimePassedInMS(void) const { return itsTimer.CurrentTimeDiffInMSeconds(); }
    const NFmiMetTime& MouseDragStartTime(void) const { return itsMouseDragStartTime; }
    void MouseDragStartTime(const NFmiMetTime &newValue) { itsMouseDragStartTime = newValue; }
    void ToggleRunMode(void);
    void ToggleLockMode(void);
    double LastFrameDelayFactor(void) const { return itsLastFrameDelayFactor; }
    void LastFrameDelayFactor(double newValue);
    bool ShowVerticalControl(void) const { return fShowVerticalControl; }
    void ShowVerticalControl(bool newValue) { fShowVerticalControl = newValue; }
    double CalcUsedDelayInMSForCurrentFrame(void) const;
    double CalcWaitTimeInMSForNextFrame(void) const;
    void AdjustTimesRelatedtoCaseStudyModeChange(const NFmiMetTime &theFromWallClockTime, const NFmiMetTime &theToWallClockTime);

    void Write(std::ostream& os) const;
    void Read(std::istream& is);

private:
    double CurrentFrameDelayFactor(void) const;
    bool MoveCurrentTimeForward(int theReducedAnimationTimeSteps);
    bool MoveCurrentTimeBackward(void);

    bool fShowTimesOnTimeControl; // onko animaatio ajat n‰kyviss‰ aikakontrolli ikkunassa
    bool fAnimationOn; // onko animaatio k‰ynniss‰
    int itsFrameDelayInMS;
    double itsLastFrameDelayFactor; // kuinka paljon pitemp‰‰n viimeist‰ ruutua pidet‰‰n n‰kyvill‰ (1 - n kertaa)
    NFmiTimeBag itsTimes; // timebagi‰ pit‰‰ aina s‰‰t‰‰ niin ett‰ alku- ja loppuajat ovat itsTimeStepInMinutes suhteen oikein ja myˆs bagin timeStep on sama kuin itsTimeStepInMinutes
    NFmiMetTime itsCurrentTime; // t‰ss‰ on tallessa aina nykyisen animaation hetki. Ei voi olla timebagin current-value, koska sit‰ voidaan s‰‰t‰‰ lennossa
    NFmiTimeBag itsShowTimebag; // t‰m‰ timebagi on k‰ytˆss‰ siksi ett‰ saadaan sulavampia mouse drag-efektej‰. Eli t‰m‰n avulla piirret‰‰n sininen laatikko aika-ikkunaan, mutta itsTimes k‰ytet‰‰n animaatiossa
    int itsTimeStepInMinutes;
    RunMode itsRunMode;
    AnimationLockMode itsLockMode;
    int itsLockModeTimeDifferenceInMinutes; // mik‰ on offsetti sein‰kelloon lukitussa animaatio moodissa
    bool fLastDirectionForward; // mihin suuntaan animaatiossa oltiin viimeksi menossa ns. sweep moodissa
    NFmiMilliSecondTimer itsTimer; // t‰ll‰ mit‰t‰‰n animaation aikana, pit‰‰kˆ ruutua jo p‰ivitt‰‰
    NFmiMetTime itsMouseDragStartTime; // t‰st‰ ajan hetkest‰ on alettu luomaan uutta animaatio-boksia
    bool fShowVerticalControl; // piirret‰‰nkˆ kartan p‰‰lle vertikaali kontrolli vai ei
};
