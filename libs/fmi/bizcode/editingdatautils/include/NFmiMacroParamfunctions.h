#pragma once

#include "NFmiInfoData.h"
#include "NFmiDataMatrix.h"
#include "boost/shared_ptr.hpp"

namespace MacroParam
{
    void GetFileNames(const std::string &thePath, const std::string &theName, std::string &theDrawParamFileName, std::string &theMacroFileName, NFmiInfoData::Type theDataType);
    std::string GetRelativePath(const std::string &theRootPath, const std::string &theCurrentPath);
    void RemoveLastPartOfDirectory(std::string &thePath);
    std::string ConvertPathToOneUsedFormat(const std::string &thePath);

    // pit‰‰ tehd‰ oma rutiini vertaamaan vektoreiden samanarvoisuutta, jos on k‰ytetty boost::shared_ptr:eita apuna.
    // T‰m‰ siksi ett‰ shared_ptr tekee vertailun suoraan pointtereilla eik‰ niiden referensseill‰.
    template<typename T>
    static bool CheckSharedPtrContainerEquality(const checkedVector<boost::shared_ptr<T> > &items1, const checkedVector<boost::shared_ptr<T> > &items2)
    {
        typedef checkedVector<boost::shared_ptr<T> >::const_iterator Iter;
        if(items1.size() == items2.size())
        {
            Iter it1 = items1.begin();
            Iter it2 = items2.begin();
            for(; it1 != items1.end(); ++it1, ++it2)
            {
                if((*(*it1) == *(*it2)) == false)
                    return false; // jos yksikin itemi poikkeaa, palautetaan false
            }
            return true; // jos kaikki on k‰yty l‰pi ja kaikki oli samoja, palautetaan true
        }
        return false;
    }

}
