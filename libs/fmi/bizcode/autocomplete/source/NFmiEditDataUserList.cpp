#include "NFmiEditDataUserList.h"
#include "NFmiSettings.h"
#include "NFmiFileSystem.h"
#include "NFmiPathUtils.h"

NFmiEditDataUserList::NFmiEditDataUserList()
    :mUserNameListFilePath()
    , mUserNameList()
    , mInitialized(false)
{
}

void NFmiEditDataUserList::InitFromSettings(const std::string &configurationBaseDirectory)
{
    if(mInitialized)
        throw std::runtime_error("NFmiEditDataUserList::Init: all ready initialized.");

    mInitialized = true;
    std::string filePath = NFmiSettings::Optional<std::string>("SmartMet::UserNameListFilePath", "");
    mUserNameListFilePath = PathUtils::getTrueFilePath(filePath, configurationBaseDirectory, "");

    ReadUserNamesFromFile();
    if(!mUserNameListFilePath.empty() && mUserNameList.size() == 0)
        throw std::runtime_error(std::string("Error in NFmiEditDataUserList::InitFromSettings - UserNameListFilePath was given (") + mUserNameListFilePath + " -file) in configurations but no user names was found");
}

static bool IsGoodUserName(const std::string &userName)
{
    if(userName.empty())
        return false;
    else
    {
        if(userName[0] == '#') // # kommentti ohitetaan
            return false;
        if(userName.size() > 1 && userName[0] == '/' && userName[1] == '/') // rivi kommentti ohitetaan
            return false;
    }
    return true;
}

void NFmiEditDataUserList::ReadUserNamesFromFile()
{
    if(!mUserNameListFilePath.empty())
    {
        if(!NFmiFileSystem::FileExists(mUserNameListFilePath))
            throw std::runtime_error(std::string("Error in NFmiEditDataUserList::ReadUserNamesFromFile - given UserNameListFilePath (") + mUserNameListFilePath + " -file) was not found");
        else
        {
            std::string fileContent;
            if(NFmiFileSystem::ReadFile2String(mUserNameListFilePath, fileContent))
            {
                std::vector<std::string> fileLines = NFmiStringTools::Split(fileContent, "\n");
                for(auto lineStr : fileLines)
                {
                    std::string userName = NFmiStringTools::TrimAll(lineStr);
                    if(::IsGoodUserName(userName))
                    {
                        mUserNameList.push_back(userName);
                    }
                }
            }
        }
    }
}
