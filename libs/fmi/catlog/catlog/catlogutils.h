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

    template<typename Container, typename Predicate>
    void removeFromContainer(Container& container, const Predicate& predicate)
    {
        if(!container.empty())
            container.erase(std::remove_if(container.begin(), container.end(), predicate), container.end());
    }

    static std::vector<std::string> getSearchedWords(const std::string &searchString)
    {
        // We want to ignore really short words and use shortWordLength to do that. Short words are ignored
        // so that log-viewer or other systems won't do unwanted work when user is just starting to type search words.
        const size_t shortWordLength = 1;
        std::vector<std::string> words;
        // This will generate, in Debug configuration, about 100 lines of compiler warnings (boost not using secure functions blaa blaa). 
        // Couldn't disable those in any other way but to disable that warning from catlog library's CMakeLists.txt with add_compile_options with /wd4996
        boost::split(words, searchString, boost::is_any_of("\t "));
        // Removing just short words, this will include the following special cases too:
        // 1) empty words, 2) '+' and '-' words
        removeFromContainer(words, [=](const auto& word) {return word.size() <= shortWordLength; });
        // Removing short words with signs in front
        removeFromContainer(words, [=](const auto& word) {return (word[0] == '-' || word[0] == '+') && word.size() <= shortWordLength + 1; });

        return words;
    }
}