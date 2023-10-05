//**********************************************************
// C++ Class Name : NFmiMetEditorModeDataWCTR (=With Changing Time Resolution) 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: NFmiMetEditorModeDataWCTR.h 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 4.0 ) 
//  - GD System Name    : nowcast starts planning 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : NFmiMetEditorModeDataWCTR 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Thu - Jan 16, 2003 
// 
// Luokka, joka osaa tehd� vaihtuvan aikaresoluutioisen timedescriptorin
// Alkaen halutusta ajasta halutuilla spekseill�, jotka luetaan 
// konfiguraatio tiedostosta. K�ytt�j� on m��ritellyt n kpl aikasectiota
// joilla on erilainen aikaresoluutio ja t�m� luokka luo aikalistan jossa on 
// otettu huomioon eri aika-alueet eri aikaresoluutiolla.
//
//  Change Log     : 
// 
//**********************************************************

#pragma once

#include "NFmiTimeBag.h"
#include <iosfwd>

class NFmiTimeDescriptor;
class NFmiTimeList;
class NFmiMetTime;

class NFmiMetEditorModeDataWCTR
{

public:

   typedef enum _TimeSectionRoundDirection
   {
	   kForward = 0,  // py�ristetaank� sectionin aloitusaika eteenp�in
	   kBackward,  // taaksep�in
	   kNearest  // l�hinp��n aikaan
   } TimeSectionRoundDirection;

	class TimeSectionData
	{
	public:
		TimeSectionData(void)
		:itsStartTimeResolutionInMinutes(0)
		,itsTimeResolutionInMinutes(0)
		,itsSuggestedSectionLengthInHours(0)
		,itsRoundingRule(kForward)
		{
		}

		int itsStartTimeResolutionInMinutes;
		int itsTimeResolutionInMinutes;
		int itsSuggestedSectionLengthInHours;
		TimeSectionRoundDirection itsRoundingRule;
	};

	// editorin tila
   typedef enum _Mode
   {
	   kNormal = 0,  // tavallinen k�ytt�, kun ladataan dataa tiedostosta (ei automatiikkaa, eik� pakotuksia)
	   kOperativeNormal, // operatiivinen editori tila vanhalla timebag systeemill�
	   kOperativeWCTR, // operatiivinen editori tila uudella timelist systeemill�
	   kNormalAutoLoad  // tavallinen k�ytt�, kun ladataan dataa tiedostosta mutta haluttuja datoja lataillaan automaattisesti taustalla.
   } Mode;

   NFmiMetEditorModeDataWCTR(void);
   NFmiMetEditorModeDataWCTR(const NFmiMetEditorModeDataWCTR &theOther);
   virtual ~NFmiMetEditorModeDataWCTR(void);

   std::ostream& Write (std::ostream &file) const;
   std::istream& Read (std::istream &file) ;

   void Clear(void);
   Mode EditorMode(void) const;
   void UseNormalModeForAWhile(bool newState) {fUseNormalModeForAWhile = newState;};
   bool UseNormalModeForAWhile(void){return fUseNormalModeForAWhile;};
   const NFmiTimeDescriptor& TimeDescriptor(const NFmiMetTime &theCurrentTime, const NFmiMetTime &theOriginTime, bool fMakeUpdate, bool useCurrentAsStart);
   const NFmiTimeDescriptor& TimeDescriptor(void);
   int WantedDataLengthInHours(void) const{return itsWantedDataLengthInHours;}
   int RealDataLengthInHours(void) const{return itsRealDataLengthInHours;}
   const NFmiTimeBag& MaximalCoverageTimeBag(void) const{return itsMaximalCoverageTimeBag;}
   bool InNormalModeStillInDataLoadDialog(void) {return fInNormalModeStillInDataLoadDialog;}
   void InNormalModeStillInDataLoadDialog(bool newValue) {fInNormalModeStillInDataLoadDialog = newValue;}
   const std::vector<TimeSectionData>& TimeSections() const { return itsTimeSections; }


   /**
   * Read configuration data from global setting and initialize the instance based on it.
   */
   void Configure(void);

private:
   bool CreateTimeListSystem(const NFmiMetTime &theCurrentTime, const NFmiMetTime &theOriginTime, bool useCurrentAsStart);
   bool CreateTimeBagSystem(const NFmiMetTime &theCurrentTime, const NFmiMetTime &theOriginTime, bool useCurrentAsStart);
   std::vector<NFmiMetTime> CalcSectionBorderTimes(const NFmiMetTime &theCurrentTime, bool useCurrentAsStart);
   bool MakeMaximalCoverageTimeBag(void);

   Mode itsEditorMode;

   NFmiTimeDescriptor *itsTimeDescriptor; // t�m� rakennetaan editoria varten ladattaessa vaihtuvan aikaresoluution dataa 
   NFmiTimeBag itsMaximalCoverageTimeBag; // t�m� timebagi on rakennettu kattamaan vaihtuvan aikaresoluution kaikki ajat (eli minimi aikaresoluutio alusta loppuun)
   std::vector<TimeSectionData> itsTimeSections;

   int itsWantedDataLengthInHours; // ohjaustiedostossa oli ehdotettu t�t� pituutta
   int itsRealDataLengthInHours;   // t�m� on ladatun datan todellinen pituus

   // en tied� tarvitaanko t�t� en��!!!
   bool fUseNormalModeForAWhile; // editori voidaan asettaa v�liaikaisesti normaalitilaan esim. kun ladataan dataa suoraan tiedostosta
   bool fInNormalModeStillInDataLoadDialog; // T�m� on kiusallisen monimutkainen juttu, mutta kun SmartMet on normaali moodissa
											// ja menn��n latausdialogiin, laitetaan smartMet sellaiseen moodiin v�liaikaisesti ett�
											// datanlataus threadit menev�t p��lle, vaikka ei pit�isi. T�m� asetetaan true:ksi
											// vain kun ollaan normaali moodissa ja laitetaan datanlataus dialogi p��lle.
};

