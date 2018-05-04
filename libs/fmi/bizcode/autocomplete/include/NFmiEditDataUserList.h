#pragma once

#include <vector>

class NFmiEditDataUserList
{
public:
    NFmiEditDataUserList();

    void InitFromSettings(const std::string &configurationBaseDirectory);
    const std::vector<std::string>& UserNameList() const { return mUserNameList; }

private:
    void ReadUserNamesFromFile();

    std::string mUserNameListFilePath;
    std::vector<std::string> mUserNameList;

    bool mInitialized;
};
