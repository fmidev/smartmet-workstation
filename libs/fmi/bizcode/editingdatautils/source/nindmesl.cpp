// ____________________________________________________________________________
// © Ilmatieteenlaitos/Marko				File: nindmesl.cpp
//   Original		27.07.1997
//	 Last Update	27.07.1997
//
//	 Update 01.09.1997/Marko	Changed to use NFmiStringList to store statusStrings.
//
// The implementation of CLASS:	NFmiIndexMessageList
//----------------------------------------------------------------------------

#include "nindmesl.h"
#include "NFmiStatusString.h"

NFmiIndexMessageList::NFmiIndexMessageList(void)
:itsStringList()
{
}

NFmiIndexMessageList::~NFmiIndexMessageList(void)
{
	itsStringList.Clear(kDelete);
}

void NFmiIndexMessageList::Add(const NFmiStatusString& theStatusString)
{
	itsStringList.Add(new NFmiStatusString(theStatusString));
}

NFmiString NFmiIndexMessageList::Message(long theMessageIndex)
{
	if(itsStringList.Reset())
	{
		NFmiStatusString *str;
		do
		{
			str = static_cast<NFmiStatusString *>(itsStringList.Current());
			if(str && theMessageIndex == str->Status())
				return str->String();
		} while(itsStringList.Next());
	}
	return NFmiString("");
}

std::ostream& NFmiIndexMessageList::Write(std::ostream &file) const
{
	file << itsStringList << std::endl;
	return file;
}

std::istream& NFmiIndexMessageList::Read(std::istream &file)
{
	itsStringList.Clear(kDelete);
	file >> itsStringList;
	return file;
}

bool NFmiIndexMessageList::Find(const NFmiString& string)
{
	return itsStringList.Find(string);
}

bool NFmiIndexMessageList::Find(const unsigned long& status)
{
	return itsStringList.FindWithStatus(status);
}

long NFmiIndexMessageList::CurrentStringStatus(void)
{
	return (static_cast<NFmiStatusString*>(itsStringList.Current())->Status());
}
