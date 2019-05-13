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
	static bool isRequiredWord(const string &word)
	{
		return word.size()>1 && word[0] == '+';
	}

    static bool containsAllSearchedWordsCaseInsensitive(const string &searchFromThis, const std::vector<std::string> &searchedWords)
    {
		bool searchTerms = false; //does the search have unquantified terms
 		bool searchHits = false; //did any of the unquantified terms match
        for(const auto &word : searchedWords)
        {

            if(isExcludeWord(word))
            {
                // Presumes that all word's given here are longer than 1 character
                std::string excludeWord(word.begin() + 1, word.end());
                if(containsStringCaseInsensitive(searchFromThis, excludeWord))
                    return false;
            }
            else if(isRequiredWord(word)) 
            {
				std::string requireWord(word.begin() + 1, word.end());
				if (!containsStringCaseInsensitive(searchFromThis, requireWord))
                    return false;
			}
			else {
				searchTerms= true;
				if (containsStringCaseInsensitive(searchFromThis,word))
					searchHits = true;
			}
        }
        return !searchTerms || searchHits; //a search with no unquantified terms matches everything
    }

    static std::vector<std::string> getSearchedWords(const std::string &searchString)
    {
        std::vector<std::string> words;
        // This will generate, in Debug configuration, about 100 lines of compiler warnings (boost not using secure functions blaa blaa). 
        // Couldn't disable those in any other way but to disable that warning from catlog library's CMakeLists.txt with add_compile_options with /wd4996
        boost::split(words, searchString, boost::is_any_of("\t "));
        // E.g. empty string generates vector with 1 empty element, have to remove all empty values from vector
        words.erase(std::remove_if(words.begin(), words.end(), [](const auto &word) {return word.empty(); }), words.end());
        // Minus character at start of word means excluded word, if user has started to write excluded word with '-' character, ignore it 
        // And also igone word if there is only single character after - sign like "-a", so that not all the messages that have 'a' would be ignored. 
        words.erase(std::remove_if(words.begin(), words.end(), [](const auto &word) {return word == "-"; }), words.end());
        words.erase(std::remove_if(words.begin(), words.end(), [](const auto &word) {return word.size() == 2 && word[0] == '-'; }), words.end());

        return words;
    }
}