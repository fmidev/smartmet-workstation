#pragma once
#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include <vector>

using namespace std;

namespace CatLogUtils
{
    static bool containsStringCaseInsensitive(const string &searchFromThis, const string &findThis)
    {
        auto findRange = boost::algorithm::ifind_first(searchFromThis, findThis);
        return !findRange.empty();
    }

    static bool isExcludeWord(const string &word)
    {
        return word.size() && word[0] == '-';
    }

    static bool containsAllSearchedWordsCaseInsensitive(const string &searchFromThis, const std::vector<std::string> &searchedWords)
    {
        for(const auto &word : searchedWords)
        {
            if(isExcludeWord(word))
            {
                // Presumes that all word's given here are longer than 1 character
                std::string excludeWord(word.begin() + 1, word.end());
                if(containsStringCaseInsensitive(searchFromThis, excludeWord))
                    return false;
            }
            else
            {
                if(!containsStringCaseInsensitive(searchFromThis, word))
                    return false;
            }
        }
        return true;
    }
}