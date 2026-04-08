//� Ilmatieteenlaitos/Marko.
//Original 12.8.2005
//
// Luokka pit�� huolta trajektoreihin liittyvist� asioista.
//---------------------------------------------------------- NFmiTrajectorySystem.h

#pragma once

#include "NFmiTimeBag.h"
#include "NFmiPoint.h"
#include "NFmiProducer.h"
#include "NFmiRawTempStationInfoSystem.h"
#include "NFmiTempBalloonTrajectorSettings.h"
#include <memory>
#include "boost/shared_ptr.hpp"

class NFmiInfoOrganizer;
class NFmiFastQueryInfo;
class NFmiProducerSystem;
class NFmiTrajectory;
class NFmiSingleTrajector;

//_________________________________________________________ NFmiTrajectorySystem
class NFmiTrajectorySystem
{
 public:
	static double itsLatestVersionNumber;
	mutable double itsCurrentVersionNumber;

	NFmiTrajectorySystem(NFmiInfoOrganizer *theInfoOrganizer, NFmiProducerSystem *theProducerSystem);
	~NFmiTrajectorySystem(void);

	void Init(const NFmiTrajectorySystem &theOther, bool disableWindowManipulations);
	void InitializeFromSettings(const std::string &basePathAbsolute);
	void StoreSettings(void);
	bool SaveXML(const std::string &theFileName);
	void ClearTrajectories(void);
	void SetSelectedValuesToAllTrajectories(void);
	void SetSelectedValuesToLastTrajectory(void);
	void ReCalculateTrajectories(void);
	bool TrajectoryViewOn(void) const {return fTrajectoryViewOn;}
	void TrajectoryViewOn(bool newValue) {fTrajectoryViewOn = newValue;}
	const NFmiPoint& SelectedLatLon(void) const {return itsSelectedLatLon;}
	void SelectedLatLon(const NFmiPoint &newValue) {itsSelectedLatLon = newValue;}
	const NFmiMetTime& SelectedTime(void) const {return itsSelectedTime;}
	void SelectedTime(const NFmiMetTime &newValue) {fTrajectoryViewTimeBagDirty = true; itsSelectedTime = newValue;}
	const NFmiProducer& SelectedProducer(void) const {return itsSelectedProducer;}
	void SelectedProducer(const NFmiProducer &newValue) {itsSelectedProducer = newValue;}
	int SelectedDataType(void) const {return itsSelectedDataType;}
	void SelectedDataType(int newValue) {itsSelectedDataType = newValue;}
	const std::vector<boost::shared_ptr<NFmiTrajectory> >& Trajectories(void) const {return itsTrajectories;}
	const NFmiTrajectory& Trajectory(int theIndex) const;
	void AddTrajectory(bool fCalculateData);
	int SelectedTimeStepInMinutes(void) const {return itsSelectedTimeStepInMinutes;}
	void SelectedTimeStepInMinutes(int newValue) {itsSelectedTimeStepInMinutes = newValue;}
	int SelectedTimeLengthInHours(void) const {return itsSelectedTimeLengthInHours;}
	void SelectedTimeLengthInHours(int newValue) {fTrajectoryViewTimeBagDirty = true; itsSelectedTimeLengthInHours = newValue;}
	double SelectedPlumeProbFactor(void) const {return itsSelectedPlumeProbFactor;}
	void SelectedPlumeProbFactor(double newValue) {itsSelectedPlumeProbFactor = newValue;}
	int SelectedPlumeParticleCount(void) const {return itsSelectedPlumeParticleCount;}
	void SelectedPlumeParticleCount(int newValue) {fTrajectoryViewTimeBagDirty = true; itsSelectedPlumeParticleCount = newValue;}
	bool PlumesUsed(void) const {return fPlumesUsed;}
	void PlumesUsed(bool newValue) {fTrajectoryViewTimeBagDirty = true; fPlumesUsed = newValue;}
	double SelectedStartLocationRangeInKM(void) const {return itsSelectedStartLocationRangeInKM;}
	void SelectedStartLocationRangeInKM(double newValue) {itsSelectedStartLocationRangeInKM = newValue;}
	int SelectedStartTimeRangeInMinutes(void) const {return itsSelectedStartTimeRangeInMinutes;}
	void SelectedStartTimeRangeInMinutes(int newValue) {fTrajectoryViewTimeBagDirty = true; itsSelectedStartTimeRangeInMinutes = newValue;}
	const NFmiTimeBag& TrajectoryViewTimeBag(void);
	void TrajectoryViewTimeBag(const NFmiTimeBag &newTimeBag);
	bool TrajectoryViewTimeBagDirty(void) const {return fTrajectoryViewTimeBagDirty;}
	void TrajectoryViewTimeBagDirty(bool newValue) {fTrajectoryViewTimeBagDirty = newValue;}
	double SelectedPressureLevel(void) const {return itsSelectedPressureLevel;}
	void SelectedPressureLevel(double newValue) {itsSelectedPressureLevel = newValue;}
	double SelectedStartPressureLevelRange(void) const {return itsSelectedStartPressureLevelRange;}
	void SelectedStartPressureLevelRange(double newValue) {itsSelectedStartPressureLevelRange = newValue;}
	FmiDirection SelectedDirection(void) const {return itsSelectedDirection;}
	void SelectedDirection(FmiDirection newValue) {itsSelectedDirection = newValue;}
	bool ShowTrajectoryArrows(void) const {return fShowTrajectoryArrows;}
	void ShowTrajectoryArrows(bool newValue) {fShowTrajectoryArrows = newValue;}
	bool ShowTrajectoryAnimationMarkers(void) const {return fShowTrajectoryAnimationMarkers;}
	void ShowTrajectoryAnimationMarkers(bool newValue) {fShowTrajectoryAnimationMarkers = newValue;}
	bool SelectedTrajectoryIsentropic(void) const {return fSelectedTrajectoryIsentropic;}
	void SelectedTrajectoryIsentropic(bool newValue) {fSelectedTrajectoryIsentropic = newValue;}
	bool ShowTrajectoriesInCrossSectionView(void) const {return fShowTrajectoriesInCrossSectionView;}
	void ShowTrajectoriesInCrossSectionView(bool newValue) {fShowTrajectoriesInCrossSectionView = newValue;}
	NFmiTempBalloonTrajectorSettings& TempBalloonTrajectorSettings(void) {return itsTempBalloonTrajectorSettings;}
	bool CalcTempBalloonTrajectors(void) const {return fCalcTempBalloonTrajectors;}
	void CalcTempBalloonTrajectors(bool newValue) {fCalcTempBalloonTrajectors = newValue;}
	std::shared_ptr<NFmiFastQueryInfo> GetWantedInfo(boost::shared_ptr<NFmiTrajectory> &theTrajectory);
	bool TrajectorySaveEnabled(void) const {return fTrajectorySaveEnabled;}
	void TrajectorySaveEnabled(bool newValue) {fTrajectorySaveEnabled = newValue;}
	const std::string& TrajectorySavePath(void) const {return itsTrajectorySavePath;}
	void TrajectorySavePath(const std::string &newValue) {itsTrajectorySavePath = newValue;}
	const std::string& TrajectorySaveFilePattern(void) const {return itsTrajectorySaveFilePattern;}
	void TrajectorySaveFilePattern(const std::string &newValue){itsTrajectorySaveFilePattern = newValue;}
	const std::string& LastTrajectoryLegendStr(void) const {return itsLastTrajectoryLegendStr;}
	void LastTrajectoryLegendStr(const std::string &newValue) {itsLastTrajectoryLegendStr = newValue;}
	std::string MakeCurrentTrajectorySaveFileName(void);
	NFmiSilamStationList& NuclearPlants(void) {return itsNuclearPlants;}
	NFmiSilamStationList& OtherPlaces(void) {return itsOtherPlaces;}
	void MakeSureThatTrajectoriesAreCalculated(void); // t�m� on viewmakro optimointia varten tehty varmistus funktio. T�t� kutsutaan trajektoryView-luokassa ennen varsinaista trajektorien piirtoa
	void SetCaseStudyTimes(const NFmiMetTime &theCaseStudyTime);
	void CalculateTrajectory(boost::shared_ptr<NFmiTrajectory> &theTrajectory);
	static void CalculateTrajectory(boost::shared_ptr<NFmiTrajectory> &theTrajectory, std::shared_ptr<NFmiFastQueryInfo> &theInfo);

	void Write(std::ostream& os) const;
	void Read(std::istream& is);
private:
	static void Make3DRandomizing(double &WS, double &WD, double &w, int theRandStep, int theCounter, double theRandomFactor, NFmiSingleTrajector &theTrajector);
	void CalculateTrajectoryViewTimeBag(void);
	void SetSelectedValuesToTrajectory(boost::shared_ptr<NFmiTrajectory> &theTrajectory, bool fInitialize, bool fKeepLevelSettings);
	static void CalculateSingleTrajectory(std::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiSingleTrajector &theTrajector, int theTimeStepInMinutes, int theTimeLengthInHours, double theRandFactor, int theRandStep, FmiDirection theDirection, bool fIsentropic, bool fCalcBalloonTrajectory, NFmiTempBalloonTrajectorSettings &theTempBalloonTrajectorSettings);
	static void CalculateSingle3DTrajectory(std::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiSingleTrajector &theTrajector, int theTimeStepInMinutes, int theTimeLengthInHours, double theRandFactor, int theRandStep, FmiDirection theDirection, bool fIsentropic, bool fCalcBalloonTrajectory, NFmiTempBalloonTrajectorSettings &theTempBalloonTrajectorSettings);

	NFmiInfoOrganizer *itsInfoOrganizer; // t��lt� saadaan data, ei omista/tuhoa!
	NFmiProducerSystem *itsProducerSystem;
	NFmiSilamStationList itsNuclearPlants;
	NFmiSilamStationList itsOtherPlaces;
	std::vector<boost::shared_ptr<NFmiTrajectory> > itsTrajectories;
	NFmiPoint itsSelectedLatLon;
	NFmiMetTime itsSelectedTime;
	NFmiProducer itsSelectedProducer;
	int itsSelectedDataType; // 0=pinta, 1=painepinta, 2=mallipinta ja 3=historia dataa
	int itsSelectedTimeStepInMinutes;
	int itsSelectedTimeLengthInHours;
	double itsSelectedPlumeProbFactor;
	int itsSelectedPlumeParticleCount;
	double itsSelectedStartLocationRangeInKM;
	double itsSelectedPressureLevel; // yks. hPa
	double itsSelectedStartPressureLevelRange; // yks. hPa
	FmiDirection itsSelectedDirection;
	int itsSelectedStartTimeRangeInMinutes;
	NFmiTimeBag itsTrajectoryViewTimeBag; // laske t�h�n kaikkien trajektorien yli 1. ja viimeinen aika
	bool fTrajectoryViewTimeBagDirty; // pit��k� timebagi laskea uudestaan

	bool fPlumesUsed;
	bool fTrajectoryViewOn;
	bool fShowTrajectoryArrows;
	bool fShowTrajectoryAnimationMarkers;
	bool fSelectedTrajectoryIsentropic;
	bool fShowTrajectoriesInCrossSectionView;

	NFmiTempBalloonTrajectorSettings itsTempBalloonTrajectorSettings;
	bool fCalcTempBalloonTrajectors;

	// Trajektorien talletukseen liittyvia muuttujia
	// HUOM n�m� luetaan settingeist�, mutta niit� ei viel� talleteta takaisin.
	// HUOM2 N�it� ei ole tarkoituskaan vied� viewMakroihin mitenk��n.
	bool fTrajectorySaveEnabled; // onko talletus systeemi k�yt�ss� vai ei
	std::string itsTrajectorySavePath; // vain polku, ei file filtteri�
	std::string itsTrajectorySaveFilePattern; // pelkk� lopullisen nimen patterni, miss� aikaleiman kohdalla esim. "traj_stuk_YYYYMMDDHHmm.txt"
	std::string itsLastTrajectoryLegendStr; // talletetaan aina viimeisin legenda talteen, ett� sit� ei tarvitse alusta l�htien keksi�
};

inline std::ostream& operator<<(std::ostream& os, const NFmiTrajectorySystem& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiTrajectorySystem& item){item.Read(is); return is;}

