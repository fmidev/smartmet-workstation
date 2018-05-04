
#pragma once

#include <vector>
#include <list>
#include <boost/shared_ptr.hpp>
#include "NFmiDataMatrix.h" // täältä tulee myös checkedVector

class NFmiEditorControlPointManager;
struct NFmiCPGriddingProperties;

class NFmiCPManagerSet
{

public:
	NFmiCPManagerSet(void);

	void InitFromSettings(const std::string &theBaseNameSpace, const std::string &theControlDirectory, const std::string &theSpecialStorageDirectory);
	void StoreToSettings(void) const;

	boost::shared_ptr<NFmiEditorControlPointManager> CPManager(bool getOldSchoolCPManager = false);
	void SetCPManager(boost::shared_ptr<NFmiEditorControlPointManager> &newCPManager);
	void NextCPManager(void);
	void PreviousCPManager(void);
	void SetCPManager(size_t theIndex);
	bool UseOldSchoolStyle(void) const;
	bool HasCPManager(void) const;
	bool StoreAllCurrentCPDataToFiles(void);
	size_t CPSetSize(void) const;
	int Find(const std::string &theName) const;
	boost::shared_ptr<NFmiEditorControlPointManager> CPManagerFromSet(size_t theIndex);
	void UpdateViewMacroCPManager(const boost::shared_ptr<NFmiEditorControlPointManager> &newCPManager);
    void SetCPGriddingProperties(const NFmiCPGriddingProperties& newProperties);
    void SetToolMasterAvailable(bool newValue);
private:
	void DoOldSchoolStyleInit(void);
	void DoMultiCPManagerInit();
	boost::shared_ptr<NFmiEditorControlPointManager> LoadCPDataBackupIfNotTooOld(int theMaxMinutes);
	void SetNewCPManagerSettings(size_t oldIndex, size_t newIndex);
    std::string MakeCpDataFilePath() const;
    void MakeSureCpManagerDirectoryExist();
    void ReadCPManagers(std::list<std::string> &fileList);
    void MakeBackupStuffForCpManagers();
    void MakeDefaultCPManagerSet();

	boost::shared_ptr<NFmiEditorControlPointManager> itsOldSchoolCPManager;
    checkedVector<NFmiPoint> itsOldSchoolCPManagerOrigPoints; // Talletetaan tähän initialisoinnista saadut CP-pisteet, jotta niitä voidaan lopetuksen yhteydessä vertailla sen hetkisiin pisteisiin. Jos ei muutoksia, ei talleteta tiedostoon turhaan.
	std::vector<boost::shared_ptr<NFmiEditorControlPointManager> > itsCPManagers;
    std::vector<checkedVector<NFmiPoint> > itsCPManagersOrigPoints; // Talletetaan tähän initialisoinnista saadut CP-pisteet, jotta niitä voidaan lopetuksen yhteydessä vertailla sen hetkisiin pisteisiin. Jos ei muutoksia, ei talleteta tiedostoon turhaan.
	size_t itsIndex; // osoittaa käytössä olevam CPManagerin paikkaan itsCPManagers -vectorissa
	std::string itsCPManagerDirectory;
    std::string itsControlDirectory; // SmartMetin kontrolli-hakemisto
    std::string itsSpecialStorageDirectory; // SmartMetin erikoistalletus -hakemisto, eli tänne tuupataan kaikki väliaikainen data

	std::string itsBaseNameSpace;
};


