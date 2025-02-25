//**********************************************************
// C++ Class Name : NFmiEditorControlPointManager 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiEditorControlPointManager.h 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 4.0 ) 
//  - GD System Name    : editori virityksi� 2000 syksy 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : interpolation luokka 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Tues - Nov 7, 2000 
// 
//  Change Log     : 
//	11.1.2002/Marko	Muutin luokan k�ytt�m��n Matrix-luokan sijasta NFmiDataMatrix-luokkaa (newbasessa).
// 
//**********************************************************

#pragma once

#include <iosfwd>
#include "NFmiTimeDescriptor.h"
#include "NFmiParamBag.h"
#include "NFmiPoint.h"
#include "NFmiEditorControlPoint.h"
#include "NFmiDataMatrix.h"
#include "NFmiGriddingHelperInterface.h"
#include "ControlPointAcceleratorActions.h"

class NFmiArea;

class NFmiEditorControlPointManager 
{

public:
	class ThreePoints
	{
	public:
		NFmiPoint itsStartPoint;
		NFmiPoint itsMiddlePoint;
		NFmiPoint itsEndPoint;
	};

   NFmiEditorControlPointManager(void);
   ~NFmiEditorControlPointManager(void);
   bool Init(const NFmiTimeDescriptor& theTimes, const NFmiParamBag& theParams, const std::string& theCPFileName, bool fKeepOldValues, bool fKeepOldCPs, int theMaxAllowedTimeCount = 200);
   bool Init(const std::vector<NFmiPoint> &newCPs, bool keepModificationsIfPossible);
   bool Init(const NFmiEditorControlPointManager &theOther);
   bool SetZoomedAreaStationsAsControlPoints(std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &theInfos, boost::shared_ptr<NFmiArea> &theArea);
   bool SetZoomedAreaStationsAsControlPoints(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiArea> &theArea);
   int Size (void) ;
   float ChangeValue (void);
   void ChangeValue (float newValue);
   void CurrentCPChangeValue (float newValue);
   bool Time (const NFmiMetTime& theTime);
   const NFmiMetTime& Time (void) const;
   bool Param (const NFmiDataIdent& theParam);
   const NFmiDataIdent& Param (void) const;
   void LatLon (const NFmiPoint& newLatLon, int theIndex = -1);
   const NFmiPoint& LatLon (int theIndex = -1) const;
   void RelativePoint(const NFmiPoint& newPoint, int theIndex = -1);
   const NFmiPoint& RelativePoint(int theIndex = -1) const;
   const NFmiPoint& StartingRelativeLocation(int theIndex = -1) const;
   const NFmiPoint& ActiveCPLatLon(void) const;
   bool AddCP (const NFmiPoint& theLatLon);
   bool RemoveCP (void);
   void EnableCP (bool newState);
   void EnableCP (int theCPIndex, bool newState);
   bool IsEnabledCP(void);
   bool ShowCPAllwaysOnTimeView(void);
   void ShowCPAllwaysOnTimeView(bool newValue);
   void ShowAllCPsAllwaysOnTimeView(bool newValue);
   void MoveCP (const NFmiPoint& newLatLon);
   void ActivateCP (const NFmiPoint& theLatLon, bool newState);
   void ActivateCP (int theCPIndex, bool newState) ;
   bool IsActivateCP(void);
   bool IsNearestPointActivateCP(const NFmiPoint& theLatLon);
   bool IsCPMovingInTime(int theIndex = -1);
   void CPMovingInTime(bool newState, int theIndex = -1);
   void ActivateAllCPs (bool newState) ;
   bool ChangeValues (std::vector<float>& xValues, std::vector<float>& yValues, std::vector<float>& zValues, int& theArraySize) ;
   std::vector<float>& ActiveCPChangeValues(void); // t�m� on hieman vaarallinen metodi, mutta optimointia varten tehty
   std::vector<float>& CPChangeValues(void); // currentin CP:n muutos arvot
   bool ResetTime (void) ;
   bool NextTime (void) ;
   bool ResetCP(void) ;
   bool NextCP(void) ;
   bool ActivateNextCP();
   bool ActivatePreviousCP();
   bool ActivateCPToward(ControlPointAcceleratorActions direction);
   void ClearAllChangeValues (int clearMethod, float theClearValue) ;
   void CPMovingInTimeHelpPoints(const ThreePoints& thePoints, int theIndex = -1);
   const ThreePoints& CPMovingInTimeHelpPoints(int theIndex = -1) const;
   const NFmiEditorControlPoint& ControlPoint (void) ;
   const NFmiParamBag& ParamBag (void) const;
   const NFmiTimeDescriptor& TimeDescriptor(void) const;
   inline void CPIndex (int value) {itsCPIndex = value;}
   inline int CPIndex (void) const {return itsCPIndex;}
   bool MouseCaptured(void){return fMouseCaptured;};
   void MouseCaptured(bool newStatus){fMouseCaptured = newStatus;};
   const NFmiArea* Area(void) const {return itsArea;};
   bool InitRelativeLocations(void); // laskee latlon pisteist� vastaavan relative point vektorin itsArean avulla
   void Area(const NFmiArea* newArea){itsArea = newArea;};
   bool FindNearestCP(const NFmiPoint& theLatLon, bool fChangeIndex = true);
   bool ReadCPs(std::istream& file);
   bool WriteCPs(std::ostream& file) const;
   bool StoreCPs(void) const;
   bool WriteBackup(std::ostream& file);
   bool ReadBackup(std::istream& file);
   void SetInTimeMovingCPRelativeLocations(const std::vector<NFmiPoint>& theRelativeVector, int theIndex = -1);
   const std::vector<NFmiPoint>& GetInTimeMovingCPRelativeLocations(int theIndex = -1) const;
   const std::vector<NFmiPoint>& CPLocationVector(void) const {return itsCPLocationVector;}
   const std::string& FilePath(void) const {return itsFilePath;}
   void FilePath(const std::string &newValue);
   const std::string& Name(void) const {return itsName;}
   void Name(const std::string &newValue) {itsName = newValue;}
   bool MakeControlPointAcceleratorAction(ControlPointAcceleratorActions action);
   int CPCount() const { return itsCPCount; }

   std::istream& Read (std::istream& file) ;
   std::ostream& Write (std::ostream& file) const;
private:

	NFmiEditorControlPoint& GetControlPoint(int theParamIndex, int theCPIndex);
	bool AreCPIndexiesGood(int theParamIndex, int theCPIndex) const;
	NFmiPoint LatLonToRelative(const NFmiPoint& theLatLon);
	NFmiPoint RelativeToLatLon(const NFmiPoint& thePoint);
	float ChangeValue (int theCPIndex);
	int CalcParamSize(NFmiParamBag& theParams);
	int CalcParamIndex(const NFmiDataIdent& theParam);
	void ClearAllChangeValues(float newValue);
	void ClearIndexedParamChangeValues(float newValue, int paramIndex);
	void ClearIndexedParamIndexedCPChangeValues(float newValue, int paramIndex, int CPIndex);
    void AddZoomedAreaStationsToCPVector(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiArea> &theArea, std::vector<NFmiPoint> &theAddedControlPointsInOut);
    int GetActiveCpIndex() const;
    bool ActivateFirstCp();
    void ResetActivityVector();

   // Kuinka monta controlpoint parametria on parambagiss�.
   int itsParamCount;
   // Mihin parametriin osoitetaan t�ll� hetkell�. N�m� ei ole samoja indeksej� kuin
   // Parambagiss� koska kaikkia parametreja ei muokata controllipisteill� (esim. HASDE1).
   int itsParamIndex;
   NFmiTimeDescriptor itsTimeDescriptor;
   // Muokattavan datan parametrit. T�m�n avulla lasketaan paramcount ja
   // lasketaan halutulle parametrille indeksi
   NFmiParamBag itsParamBag;
   // CP = EditorControlPoint
   int itsCPCount;
   int itsCPIndex;
   // T�ss� on kaikkien CP:en latlon pisteet.
   // HUOM!! ei viel� pid� sis�ll��n muuttuvaa paikkaa ajassa, 
   // t�ll�in muuttuja pit�� muuttaa Matrix tyyppiseksi 2D otukseksi.
   std::vector<NFmiPoint> itsCPLocationVector;
   std::vector<NFmiPoint> itsCPRelativeLocationVector;
   // Onko CP aktiivinen vai ei (muokataanko pistett� mitenk��n vai ei)?
   std::vector<bool> itsCPActivityVector;
   // Onko CP k�yt�ss� (enabloitu) vai ei (otetaanko piste mukaan laskuihin mitenk��n vai ei)?
   std::vector<bool> itsCPEnabledVector;
   // Muuttuuko kyseisen CP:n paikka ajassa vai onko CP paikallaan (eli 
   // kun muutat yhdess� ajassa CP:n paikkaa, muut ajat muuttuvat vastaavasti)?
   std::vector<bool> itsCPChangeInTimeVector;
   NFmiDataMatrix<NFmiPoint> itsCPChangeInTimeRelativeLocationMatrix;
   NFmiDataMatrix<NFmiPoint> itsCPChangeInTimeLatLonLocationMatrix;
   // Kaksiulotteinen taulu Controlpointteja (n kpl CP:t� jokaiselle parametrille)
   // parametri-lkm on 1. Resize parametri (param-lkm = x = rivi-lkm)
   // CP-lkm on 2. Resize parametri (CP-lkm = y = column-lkm)
   NFmiDataMatrix<NFmiEditorControlPoint> itsCPMatrix;
   std::vector<float> itsDummyChangeValueVector; // t�m� palautetaan, jos ei l�ydy aktiivista CP:t� ja kysyt��n aktiivisen CP muutostaulua
   NFmiPoint itsDummyLatlon; // t�m� palautetaan kun mik��n CP ei ole aktiivinen ja pyydet��n latlonia
   NFmiEditorControlPoint itsDummyControlPoint;
   bool fMouseCaptured;
   const NFmiArea* itsArea; //(HUOM! ei omista) t�m�n avulla lasketaan latloneista suhteelliset paikat
   std::vector<ThreePoints> itsCPMovingInTimeHelpPoints; // CP-s��t�dialogi p�ivitt�� ja k�ytt�� n�it�

   std::vector<bool> itsShowCPAllwaysOnTimeView;

   std::string itsFilePath; // jos t�m� ei ole tyhj�, on t�m� CPManageri luettu t�st� tiedostosta (polkuineen kaikkineen)
   std::string itsName; // t�m� otetaan suoraan tiedoston nimest�
};

std::ostream& operator<<(std::ostream& os, const NFmiEditorControlPointManager& item);
std::istream& operator>>(std::istream& is, NFmiEditorControlPointManager& item);

