#pragma once

#include "NFmiInfoData.h"
#include "NFmiDataMatrix.h"
#include "boost/shared_ptr.hpp"

#include <locale>

namespace MacroParam
{
    const int ci_string_not_found = -1;

    void GetFileNames(const std::string &thePath, const std::string &theName, std::string &theDrawParamFileName, std::string &theMacroFileName, NFmiInfoData::Type theDataType);
    std::string GetRelativePath(const std::string &theRootPath, const std::string &theCurrentPath);
    void RemoveLastPartOfDirectory(std::string &thePath);
    std::string ConvertPathToOneUsedFormat(const std::string &thePath);

    // Case-insensitive-find stringille, haettu webist‰: http://stackoverflow.com/questions/3152241/case-insensitive-stdstring-find
    // templated version of my_equal so it could work with both char and wchar_t
    template<typename charT>
    struct case_insensitive_equal {
        case_insensitive_equal(const std::locale& loc) : loc_(loc) {}
        bool operator()(charT ch1, charT ch2) {
            return std::toupper(ch1, loc_) == std::toupper(ch2, loc_);
        }
    private:
        const std::locale& loc_;
    };

    // find substring (case insensitive)
    template<typename T>
    int ci_find_substr(const T& str1, const T& str2, const std::locale& loc = std::locale())
    {
        typename T::const_iterator it = std::search(str1.begin(), str1.end(), str2.begin(), str2.end(), case_insensitive_equal<typename T::value_type>(loc));
        if(it != str1.end())
            return static_cast<int>(it - str1.begin());
        else
            return ci_string_not_found;
    }

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
