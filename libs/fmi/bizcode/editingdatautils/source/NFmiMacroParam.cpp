#include "NFmiMacroParam.h"
#include "NFmiFileSystem.h"
#include "NFmiDrawParam.h"
#include "NFmiMacroParamfunctions.h"
#include <fstream>

using namespace std;

NFmiMacroParam::NFmiMacroParam(void)
:itsName()
,itsMacroText()
,itsDrawParam(new NFmiDrawParam())
,itsMacroParaDirectoryPath()
,fMacroParamDirectory(false)
,fErrorInMacro(false)
{
}

bool NFmiMacroParam::Store(const std::string& thePath, const std::string& theName)
{
    string drawParamFileName;
    string macroFileName;
    MacroParam::GetFileNames(thePath, theName.empty() ? itsName : theName, drawParamFileName, macroFileName, DrawParam()->DataType());

    // Smarttool dialogista tehdyt talletukset tekevät piirto-ominaisuuksien talletuksen vain 1. kerralla.
    // Piirto-ominaisuudet pitää jatkossa tallettaa erikseen piirto-optio dialogin säätöjen kautta.
    if(!NFmiFileSystem::FileExists(drawParamFileName))
    {
        ofstream out1(drawParamFileName.c_str(), std::ios::binary);
        if(out1)
        {
            out1 << *itsDrawParam << endl;
        }
        else
            throw runtime_error(string("NFmiMacroParam::Store: Can't save/create macroParam file:\n") + drawParamFileName + "\n");
    }

    ofstream out2(macroFileName.c_str(), std::ios::binary);
    if(out2)
    {
        // Tää on ihan tajutonta tämä cr:ien poisto, en ymmärrä miten tää pitäis hoitaa, nyt
        // poistan niitä jo ainakin kahdessa kohtaa kun mikään ei riitä. cr:ien luku/kirjoitus on ongelma
        // koska niitä ei kai kirjoiteta tiedostoon, mutta jossain niitä aina sitten lisätää enkä
        // ole varma missä.
        itsMacroText.erase(std::remove(itsMacroText.begin(), itsMacroText.end(), '\r'), itsMacroText.end());
        out2 << itsMacroText;
    }
    else
        throw runtime_error(string("NFmiMacroParam::Store: Can't save/create macroParam file:\n") + macroFileName + "\n");

    return true;
}

bool NFmiMacroParam::Load(const std::string &thePath, const std::string &theName)
{
	if(theName.empty())
		return false;
	else if(theName[0] == '<')
	{
		itsName = theName;
		itsMacroText = "<Directory>";
		itsMacroParaDirectoryPath = thePath;
		std::string tmp = theName;
		NFmiStringTools::Trim(tmp, '<');
		NFmiStringTools::Trim(tmp, '>');
		tmp += kFmiDirectorySeparator;
		itsMacroParaDirectoryPath += tmp;
		fMacroParamDirectory = true;
		return true;
	}
	else
	{
		string drawParamFileName;
		string macroFileName;
		MacroParam::GetFileNames(thePath, theName.empty() ? itsName : theName, drawParamFileName, macroFileName, DrawParam()->DataType());

		ifstream in1(drawParamFileName.c_str(), std::ios::binary);
		if(in1)
		{
			in1 >> *itsDrawParam;
			if(in1.fail())
				return false;
			itsDrawParam->InitFileName(drawParamFileName); // otetaan tiedoston nimi vielä talteen

			// HUOM!! tämä on quickfix, mutta joskus macron nimi on väärin nimenlyhenteessä, josta seuraa ongelmia,
			// joten nimen lyhenne pitää tässä muuttaa (jos eri kuin macroparamin nimi) oikeaksi vaikka väkisin.
			if(itsDrawParam->ParameterAbbreviation() != theName)
				itsDrawParam->ParameterAbbreviation(theName);
		}
		else
			throw runtime_error(string("NFmiMacroParam::Load: Cannot open macroParam file:\n") + drawParamFileName);

		bool status = NFmiFileSystem::ReadFile2String(macroFileName, itsMacroText);
		if(!status)
		{  // kokeillaan vielä löytyykö q3-makroparamia (TÄMÄ on aika skeida koodia, pitää korjata kokonais rakennetta, kunhan ensin saa alkuun q3-testit)
			MacroParam::GetFileNames(thePath, theName.empty() ? itsName : theName, drawParamFileName, macroFileName, NFmiInfoData::kQ3MacroParam);
			status = NFmiFileSystem::ReadFile2String(macroFileName, itsMacroText);
			if(!status)
				throw runtime_error(string("NFmiMacroParam::Cannot open script file:\n") + macroFileName);
			DrawParam()->DataType(NFmiInfoData::kQ3MacroParam);
		}
		return true;
	}
}

bool operator==(const NFmiMacroParam &obj1, const NFmiMacroParam &obj2)
{
	if(&obj1 == &obj2)
		return true;
	if(obj1.Name() == obj2.Name())
	{
		if(obj1.MacroText() == obj2.MacroText())
		{
			//itsDrawParam  // TODO drawParamia ei vielä vertailla!!!!
			if(obj1.MacroParamDirectoryPath() == obj2.MacroParamDirectoryPath())
			{
				if(obj1.IsMacroParamDirectory() == obj2.IsMacroParamDirectory())
					return true;
			}
		}
	}
	// mielestäni fErrorInMacro -dataosaa ei tarvitse vertailla
	return false;
}
