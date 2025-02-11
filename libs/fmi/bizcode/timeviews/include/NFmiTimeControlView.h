//© Ilmatieteenlaitos/software by Marko
//  Original 23.09.1998
// 
//                                  
//Ver. xx.xx.xxxx/Marko
//-------------------------------------------------------------------- NFmiTimeControlView.h

#pragma once

//_________________________________________________________ NFmiTimeControlView

#include "NFmiCtrlView.h"


class NFmiToolBox;
class NFmiTimeScaleView;
class NFmiStepTimeScale;
class NFmiTimeBag;
namespace CtrlViewUtils
{
    struct GraphicalInfo;
}
namespace Gdiplus
{
	class Bitmap;
}

// T‰h‰n luokkaan luetaan ksyeiset kuva-imaget kerran ja k‰ytet‰‰n kaikille NFmiTimeControlView-instansseille yhteisesti.
// Gdiplus::Bitmap-olioita ei tarvitse tuhota, Gdiplus huolehtii siit‰ automaattisesti.
class AnimationButtonImageHolder
{
public:
	AnimationButtonImageHolder(void)
	:itsBitmapFolder()
	,itsPlayButtonImage(0)
	,itsPauseButtonImage(0)
	,itsDelayButtonImage(0)
	,itsCloseButtonImage(0)
	,itsRepeatButtonImage(0)
	,itsAnimationLockButtonImage(0)
	,itsAnimationNoLockButtonImage(0)
	,itsLastFrameDelayButtonImage(0)
	,itsVerticalTimeControlOnImage(0)
	,itsVerticalTimeControlOffImage(0)
    , itsSetFullTimeRangeImage(0)
	,fInitialized(false)
	{
	}

	// initialisoinnissa luetaan bitmapit tiedostoista, kutsu vasta kun itsBitmapFolder-dataosa on asetettu
	void Initialize(void);  // HUOM! heitt‰‰ poikkeuksia ep‰onnistuessaan

	std::string itsBitmapFolder;
	Gdiplus::Bitmap *itsPlayButtonImage;
	Gdiplus::Bitmap *itsPauseButtonImage;
	Gdiplus::Bitmap *itsDelayButtonImage;
	Gdiplus::Bitmap *itsCloseButtonImage;
	Gdiplus::Bitmap *itsRepeatButtonImage;
	Gdiplus::Bitmap *itsAnimationLockButtonImage;
	Gdiplus::Bitmap *itsAnimationNoLockButtonImage;
	Gdiplus::Bitmap *itsLastFrameDelayButtonImage;

	Gdiplus::Bitmap *itsVerticalTimeControlOnImage;
	Gdiplus::Bitmap *itsVerticalTimeControlOffImage;

    Gdiplus::Bitmap *itsSetFullTimeRangeImage;
    bool fInitialized;
};

class NFmiTimeControlView : public NFmiCtrlView
{

public:
	class MouseStatusInfo // joudun tekem‰‰n t‰ll‰isen info paketin, ett‰ saan talteen hiirenk‰sittely informaation, koska NFmiTimeControlView-oliota luodaan ja tuhotaan jatkuvasti ruudun p‰ivityksen yhteydess‰ uudestaan
	{
	public:
	   MouseStatusInfo(void)
	   :fMouseCaptured(false)
	   ,fMouseCapturedMoveTimeLine(false)
	   ,fMouseMovedWhenCaptured(false)
	   ,fMouseCapturedMoveSelectedTime(false)
	   ,fMouseCapturedAnimationBox(false)
	   ,itsAnimationDragDirection(kCenter)
	   ,itsTimeScaleDragDirection(kCenter)
	   ,itsLastMousePosition()
		,fAnimationCloseButtonPressed(false)
		,fAnimationPlayButtonPressed(false)
		,fAnimationDelayButtonPressed(false)
		,fAnimationRunModeButtonPressed(false)
		,fAnimationLastFrameDelayButtonPressed(false)
		,fAnimationLockModeButtonPressed(false)
		,fAnimationVerticalControlButtonPressed(false)
		,itsLeftButtonDownMousePosition()
	   {
	   }

	   bool NeedsUpdate(void)
	   {
		   return (fMouseCaptured || fMouseCapturedMoveTimeLine || fMouseCapturedMoveSelectedTime || fMouseCapturedAnimationBox);
	   }

	   bool fMouseCaptured; // t‰m‰ pit‰isi siirt‰‰ dokumenttiin??? siirret‰‰nkˆ aikamuokkauskahvoja hiirell‰ vet‰m‰ll‰
	   bool fMouseCapturedMoveTimeLine; // siirret‰‰nkˆ hiirell‰ koko aikaikkunan alku/loppu aikoja
	   bool fMouseCapturedMoveSelectedTime; // raahataanko hiirell‰ sit‰ vihre‰‰ laatikkoa, jolla osoitetaan 1. valittua aikaa
	   bool fMouseCapturedAnimationBox; // hiirell‰ raahataan animaatio boxia (sinert‰v‰‰ llatikkoa)
	   FmiDirection itsAnimationDragDirection; // jos fMouseCapturedAnimationBox ollessa true, arvo on kLeft, venytet‰‰n vasenta reunaa, jos arvo on kRight, venytet‰‰n oikeaa reunaa ja jos kCenter, raahataa laatikkoa hiirell‰ kokonaisena
	   FmiDirection itsTimeScaleDragDirection;
	   bool fMouseMovedWhenCaptured; // tieto siit‰ onko hiirt‰ liikutettu, kun se on kaapattu aikakontrolliin
	   NFmiPoint itsLastMousePosition; // kun hiirell‰ raahataan aikakontrollia, t‰m‰n avulla lasketaan liike verrattuna edelliseen MouseMove viestiin, ett‰ aikaikkunaa osataan liikuttaa oikea m‰‰r‰
		bool fAnimationCloseButtonPressed;
		bool fAnimationPlayButtonPressed;
		bool fAnimationDelayButtonPressed;
		bool fAnimationRunModeButtonPressed;
		bool fAnimationLastFrameDelayButtonPressed;
		bool fAnimationLockModeButtonPressed;
		bool fAnimationVerticalControlButtonPressed;
		NFmiPoint itsLeftButtonDownMousePosition;
	};

    NFmiTimeControlView (int theMapViewDescTopIndex, const NFmiRect & theRect
						,NFmiToolBox * theToolBox
						,boost::shared_ptr<NFmiDrawParam> &theDrawParam
						,bool theShowResolutionChangerBox
						,bool theDrawViewInMouseMove
						,bool theShowEditTimeHandles
						,bool theShowSelectedTimes
						,bool theDrawTimeFilterTimesInNormalDraw
						,double theAxisShrinkFactor = 0.02);
   ~NFmiTimeControlView (void);

   void Draw (NFmiToolBox * theGTB) override;
 
   bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
   bool LeftButtonDown (const NFmiPoint & thePlace, unsigned long theKey) override;
   bool RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
   bool MiddleButtonUp(const NFmiPoint & thePlace, unsigned long theKey) override;
   bool MouseMove(const NFmiPoint& thePlace, unsigned long theKey) override;
   bool IsMouseCaptured(void){return (fMouseCaptured || fMouseCapturedMoveTimeLine || fMouseCapturedMoveSelectedTime || fMouseCapturedAnimationBox);};
   void MouseCaptured(bool newState){fMouseCaptured = newState;};
   void Initialize(bool theShowSelectedTimes, bool theUseActiveMapTime); // HUOM! ‰l‰ kutsu konstruktorissa, kutsu sen j‰lkeen yritys korjata konstruktorissa olevaa virtuaalisuus ongelmaa 
   void DrawOverBitmapThings(NFmiToolBox * theGTB, bool dummy, int dummy2, float dummy3, void* dummy4);// t‰ll‰ piirret‰‰n tavara, joka tulee myˆs bitmapin p‰‰lle
   bool SetTime(const NFmiPoint & thePlace, bool fStayInsideAnimationTimes = false);
   std::string ComposeToolTipText(const NFmiPoint& theRelativePoint) override;
   MouseStatusInfo GetMouseStatusInfo(void);
   void SetMouseStatusInfo(const MouseStatusInfo &theMouseStatusInfo);
   bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta) override;
   virtual CtrlViewUtils::GraphicalInfo& GetGraphicalInfo(void);

protected:
	NFmiRect CalcResolutionChangerBoxRect(void);
	NFmiRect CalcTimeFilterBoxRect(void);
	bool IsInsideAnyAnimationButton(const NFmiPoint &thePlace);
	bool AnimationButtonReleased(const NFmiPoint & thePlace,unsigned long theKey);
	bool IsAnimationButtonPressed(void);
	NFmiPoint CalcAnimationButtonRelativeEdgeOffset(const NFmiPoint &theButtonRelaviteSize);
	NFmiPoint CalcAnimationButtonRelativeSize(double theSizeFactorX = 1, double theSizeFactorY = 1);
	NFmiRect CalcAnimationVerticalControlButtonRect(void);
    NFmiRect CalcFullTimeRangeButtonRect(void);
    NFmiRect CalcAnimationCloseButtonRect(void);
	NFmiRect CalcAnimationPlayButtonRect(void);
	NFmiRect CalcAnimationDelayButtonRect(void);
	NFmiRect CalcAnimationRunModeButtonRect(void);
	NFmiRect CalcAnimationLockModeButtonRect(void);
	NFmiRect CalcLastFrameDelayFactorButtonRect(void);
	NFmiRect CalcAnimationButtonRect(int theIndex);
	FmiDirection TimeScaleOperationPosition(const NFmiPoint & thePlace, double theHotSpotWidthInMM);
	bool TimeScaleMouseMove(const NFmiPoint& thePlace, unsigned long theKey, bool &fixLastPosition);
	void ClearAllMouseCaptureFlags(void);
	void DrawAnimationBox(void);
	bool AnimationMouseMove(const NFmiPoint& thePlace, unsigned long theKey, bool &fixLastPosition);
	bool AnimationLeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey);
	bool TimeScaleLeftButtonDown(const NFmiPoint & thePlace, unsigned long theKey);
	void Clear(void);
	NFmiRect CalcTimeAxisRect(void);
	NFmiRect CalcAnimationBoxRect(void);
	// Sallitaanko ylip‰‰ns‰ animaatio boxin n‰ky‰ aikasarjaikkunassa. Normalisti palauttaa
	// true, mutta mm. trajektori versioss‰, t‰m‰ palauttaa false.
	virtual bool AllowAnimationBoxShown(void) const {return true;}
	virtual NFmiPoint GetViewSizeInPixels(void);
	virtual const NFmiTimeBag& GetUsedTimeBag(void);
	virtual void SetUsedTimeBag(const NFmiTimeBag &newTimeBag);
	void DrawNoDataAvailable(void);
	virtual const NFmiMetTime& StartTime(void) const;
	virtual const NFmiMetTime& EndTime(void) const;
	virtual void StartTime(const NFmiMetTime &newValue);
	virtual void EndTime(const NFmiMetTime &newValue);
	virtual bool IsTimeFiltersDrawn(void);
	double GetAxisWidthShrinkFactor(void);
	double Time2Value(const NFmiMetTime& theTime);
	NFmiMetTime GetTimeFromPosition(const NFmiPoint & thePlace, int theUsedResolution);
	NFmiRect CalcSelectedTimeArea(const NFmiMetTime &theTime) const;
    void DrawFullTimeRangeButton(Gdiplus::Graphics *theUsedGdiPlusGraphics);
    bool IsAnimationViewVisible() const;
	void DrawVirtualTimeData();
	void DrawVirtualTimeDataBox();
	void DrawVirtualTimeSlider();
	void DrawVirtualTimeMarker();
	bool HandlePossibleVirtualTimeSet(const NFmiPoint& thePlace, unsigned long theKey);
	bool HandlePossibleVirtualTimeBoxCloseButtonClick(const NFmiPoint& thePlace, unsigned long theKey);
	bool DoPrimaryLeftButtonUpChecks(const NFmiPoint& thePlace, unsigned long theKey);
	std::pair<double, int> ConvertMilliMetersToRelativeAndPixels(double valueInMM, bool doDirectionX);
	NFmiRect CalcAnimationButtonTopLeftRect(const NFmiRect& baseRect);
	NFmiRect CalcVirtualTimeCloseButtonRect();

   NFmiTimeScaleView * itsTimeView;
   NFmiStepTimeScale * itsTimeAxis;
   NFmiTimeBag * itsTimeBag;
   bool fHasData; // jos ei ole dataa olemassa, ei tehd‰ tiettyj‰ asioita
   bool fShowEditTimeHandles; // niin...
   bool fMouseCaptured; // t‰m‰ pit‰isi siirt‰‰ dokumenttiin??? siirret‰‰nkˆ aikamuokkauskahvoja hiirell‰ vet‰m‰ll‰
   bool fMouseCapturedMoveTimeLine; // siirret‰‰nkˆ hiirell‰ koko aikaikkunan alku/loppu aikoja
   bool fMouseCapturedMoveSelectedTime; // raahataanko hiirell‰ sit‰ vihre‰‰ laatikkoa, jolla osoitetaan 1. valittua aikaa
   bool fMouseCapturedAnimationBox = false; // hiirell‰ raahataan animaatio boxia (sinert‰v‰‰ llatikkoa)
   FmiDirection itsAnimationDragDirection; // jos fMouseCapturedAnimationBox ollessa true, 
										// arvo on kLeft, venytet‰‰n vasenta reunaa, 
										// jos arvo on kRight, venytet‰‰n oikeaa reunaa ja 
										// jos kCenter, raahataa laatikkoa hiirell‰ kokonaisena
										// jos arvo on kBase, luodaan animaatio laatikkoa, ja siin‰ on hieman eri s‰‰nnˆt, eli venytys voi tapahtua molempiin suuntiin
   FmiDirection itsTimeScaleDragDirection; // jos fMouseCapturedAnimationBox ollessa true, 
											// jos arvo on kLeft, siirret‰‰n aikajanan vasenta reunaa
											// jos arvo on kRight, siirret‰‰n aikajanan oikeaa reunaa
											// jos arvo on kCenter, siirret‰‰n aikajanaa kokonaisuutena
   bool fMouseMovedWhenCaptured; // tieto siit‰ onko hiirt‰ liikutettu, kun se on kaapattu aikakontrolliin
   // animaatiossa on eri nappuloita, joita voi painaa
   bool fAnimationCloseButtonPressed;
   bool fAnimationPlayButtonPressed;
   bool fAnimationDelayButtonPressed;
   bool fAnimationRunModeButtonPressed;
   bool fAnimationLastFrameDelayButtonPressed;
   bool fAnimationLockModeButtonPressed;
   bool fAnimationVerticalControlButtonPressed;

   NFmiPoint itsLeftButtonDownMousePosition; // t‰m‰ on viri viri. Jotenkin en saa hiiren aikasarja raahausta ja pelkk‰‰ vasen klik aika siirtoa kuntoon muuten kuin mittaamalla totaali hiiren liike
   NFmiPoint itsLastMousePosition; // kun hiirell‰ raahataan aikakontrollia, t‰m‰n avulla lasketaan liike verrattuna edelliseen MouseMove viestiin, ett‰ aikaikkunaa osataan liikuttaa oikea m‰‰r‰
   NFmiRect itsLeftTimeHandle; // aloitusaikamuokkauskahvan sijainti
   NFmiRect itsRightTimeHandle; // lopuetussaikamuokkauskahvan sijainti
   bool fDrawViewInMouseMove; // eri n‰ytˆiss‰ (esim. p‰‰karttan‰ytˆn alaosa ja muokkaudialogin aikakontrolli osa) ei (n‰ytˆn p‰ivitys-tyylin takia) kannata piirt‰‰ n‰yttˆ‰ aina MouseMove-metodissa kun se piirret‰‰n muutenkin ja k‰ytt‰en double bufferointia
   double itsAxisShrinkFactor; // t‰m‰ pienent‰‰ n‰ytˆn aika-akselin leveytt‰ defaultti 0.02 tulee jostain kokeilusta
   bool fDrawTimeFilterTimesInNormalDraw; // jossain paikoissa aika filtteri jana piirret‰‰n vain DrawOverBitmapThings-metodissa, jossain taas normaali Draw:ssa
   double itsButtonSizeInMM_x; // paino nappuloiden koko millimetreiss‰ x-suunnassa
   double itsButtonSizeInMM_y; // paino nappuloiden koko millimetreiss‰ y-suunnassa
   double itsButtonOffSetFromEdgeFactor; // kuinka kaukana paino nappula on reunoista irti suhteessa napulan kokoon
   static AnimationButtonImageHolder statAnimationButtonImages; // t‰m‰ on staattinen dataosa, koska n‰it‰ timekontrol-instansseja luodaan lennossa jatkuvasti uudelleen ja uudelleen eli bitmapit luetaan vain kerran kaikkien k‰yttˆˆn
   NFmiRect itsVirtualTimeSliderRect; // Kun t‰h‰n klikataan hiirell‰, voidaan virtual-time:a s‰‰t‰‰
   NFmiRect itsVirtualTimeBoxRect;
private:
   int RelativeLength2Minutes(double theLength);
   int GetUsedTimeResolutionInMinutes(void);
   NFmiString GetResolutionText(void);
   void ChangeResolution(bool fLeftClicked, bool ctrlKeyDown);
   void DrawTimeFilterTimes(void);
   void DrawBackground (void);
   void DrawTimeAxis (void);
   void DrawResolutionChangerBox(void);
   NFmiStepTimeScale MakeEmptySelectedTimeScale(void);

// resolution changer box on laatikko, jossa n‰ytet‰‰n k‰ytetty aikaresoluutio
// ja kun laatikkoa klikkaa vasemmalla hiiren painikkeella, resoluutio pienenee ja oikealla kasvaa
   bool fShowResolutionChangerBox; 
   NFmiRect itsResolutionChangerBox;

};

class NFmiTimeSerialTimeControlView : public NFmiTimeControlView
{

public:
    NFmiTimeSerialTimeControlView(int theMapViewDescTopIndex,const NFmiRect & theRect
								,NFmiToolBox * theToolBox
								,boost::shared_ptr<NFmiDrawParam> &theDrawParam
								,bool theShowResolutionChangerBox
								,bool theDrawViewInMouseMove
								,bool theShowEditTimeHandles
								,bool theShowSelectedTimes);
   ~NFmiTimeSerialTimeControlView(void);

   bool LeftButtonDown (const NFmiPoint & thePlace, unsigned long theKey) override;
   bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;

protected:
	NFmiPoint GetViewSizeInPixels(void);
	const NFmiTimeBag& GetUsedTimeBag(void);
	void SetUsedTimeBag(const NFmiTimeBag &newTimeBag);

private:

};

class NFmiTrajectoryTimeControlView : public NFmiTimeControlView
{

public:
    NFmiTrajectoryTimeControlView(int theMapViewDescTopIndex, const NFmiRect & theRect
								,NFmiToolBox * theToolBox
								,boost::shared_ptr<NFmiDrawParam> &theDrawParam);
   ~NFmiTrajectoryTimeControlView(void);


protected:
	bool AllowAnimationBoxShown(void) const {return false;}
	const NFmiTimeBag& GetUsedTimeBag(void);
	void SetUsedTimeBag(const NFmiTimeBag &newTimeBag);

private:

};

