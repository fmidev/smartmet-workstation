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

    // Smarttool dialogista tehdyt talletukset tekev�t piirto-ominaisuuksien talletuksen vain 1. kerralla.
    // Piirto-ominaisuudet pit�� jatkossa tallettaa erikseen piirto-optio dialogin s��t�jen kautta.
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
        // T�� on ihan tajutonta t�m� cr:ien poisto, en ymm�rr� miten t�� pit�is hoitaa, nyt
        // poistan niit� jo ainakin kahdessa kohtaa kun mik��n ei riit�. cr:ien luku/kirjoitus on ongelma
        // koska niit� ei kai kirjoiteta tiedostoon, mutta jossain niit� aina sitten lis�t�� enk�
        // ole varma miss�.
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
			itsDrawParam->InitFileName(drawParamFileName); // otetaan tiedoston nimi viel� talteen

			// HUOM!! t�m� on quickfix, mutta joskus macron nimi on v��rin nimenlyhenteess�, josta seuraa ongelmia,
			// joten nimen lyhenne pit�� t�ss� muuttaa (jos eri kuin macroparamin nimi) oikeaksi vaikka v�kisin.
			if(itsDrawParam->ParameterAbbreviation() != theName)
				itsDrawParam->ParameterAbbreviation(theName);
		}
		else
			throw runtime_error(string("NFmiMacroParam::Load: Cannot open macroParam file:\n") + drawParamFileName);

		bool status = NFmiFileSystem::ReadFile2String(macroFileName, itsMacroText);
		if(!status)
		{  // kokeillaan viel� l�ytyyk� q3-makroparamia (T�M� on aika skeida koodia, pit�� korjata kokonais rakennetta, kunhan ensin saa alkuun q3-testit)
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
			//itsDrawParam  // TODO drawParamia ei viel� vertailla!!!!
			if(obj1.MacroParamDirectoryPath() == obj2.MacroParamDirectoryPath())
			{
				if(obj1.IsMacroParamDirectory() == obj2.IsMacroParamDirectory())
					return true;
			}
		}
	}
	// mielest�ni fErrorInMacro -dataosaa ei tarvitse vertailla
	return false;
}
