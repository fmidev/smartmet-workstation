#include "MessageTypeInfo.h"

#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <vector>

namespace
{
    std::string removeCharacter(std::string str, char removedChar)
    {
        str.erase(std::remove(str.begin(), str.end(), removedChar), str.end());
        return str;
    }

}

namespace HakeLegacySupport
{
    MessageTypeInfo::MessageTypeInfo(MessageType messageType, int messageCategory, const std::string &messageCategoryName, bool show)
        :messageType_(messageType)
        ,messageCategory_(messageCategory)
        ,messageCategoryName_()
        ,show_(show)
    {
        setMessageCategoryName(messageCategoryName); // does certain check over given description string
    }

    // Setter must remove possible ';' charcter from description because its used as separator in to_string and from_string methods.
    void MessageTypeInfo::setMessageCategoryName(const std::string &messageCategoryName)
    { 
        messageCategoryName_ = ::removeCharacter(messageCategoryName, g_InnerSeparatorString[0]);
        // When MessageTypeInfo vector is stored in higher level (in HakeSystemConfigurations class), they will use '#' character as separator, so we can't them either in description
        messageCategoryName_ = ::removeCharacter(messageCategoryName, g_OuterSeparatorString[0]);
    }

    // Makes following string from these values (kHakeMessage,203,"minor chemical spill",1):
    // 1;203;"minor chemical spill";1
    std::string MessageTypeInfo::to_string() const
    {
        std::string str = std::to_string(messageType_);
        str += g_InnerSeparatorString;
        str += std::to_string(messageCategory_);
        str += g_InnerSeparatorString;
        str += messageCategoryName_;
        str += g_InnerSeparatorString;
        str += std::to_string(show_);

        return str;
    }

    void MessageTypeInfo::from_string(const std::string &str)
    {
        std::vector<std::string> stringParts;
        boost::split(stringParts, str, boost::is_any_of(g_InnerSeparatorString));
        if(stringParts.size() < 4)
            throw std::runtime_error(std::string(__FUNCTION__) + ": given string had less than 4 data parts \"" + str + "\n");
        messageType_ = static_cast<MessageType>(std::stoi(stringParts[0]));
        messageCategory_ = std::stoi(stringParts[1]);
        messageCategoryName_ = stringParts[2];
        show_ = std::stoi(stringParts[3]) != 0;
    }

    bool MessageTypeInfo::operator<(const MessageTypeInfo &messageTypeInfo)
    {
        if(messageType_ != messageTypeInfo.messageType_)
            return messageType_ < messageTypeInfo.messageType_;
        else
            return messageCategory_ < messageTypeInfo.messageCategory_;
    }

}
