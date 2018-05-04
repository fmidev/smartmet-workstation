#pragma once

#include <string>

namespace HakeLegacySupport
{
	// There are different types of messages used in SmartMet, this enum will differentiate them
    enum MessageType
    {
        kNoMessage = 0,
        kHakeMessage = 1, // "Halytys keskus" warnings
        kKahaMessage = 2 // "Kansalais havainnot" messages
    };

    // Tämä erottaa yhden MessageTypeInfo:n stringiksi muutettuja sisäisiä kenttiä
    const std::string g_InnerSeparatorString = ";";
    // Tämä erottaa eri MessageTypeInfo:jen stringiksi muutettuja yhdistelmä kenttiä
    const std::string g_OuterSeparatorString = "|";


    class MessageTypeInfo
    {
		MessageType messageType_ = kNoMessage;
        // E.g. in Hake messages 203 could mean "minor chemical spill"
        int messageCategory_ = 0; 
        // Descriptive name for this message category like "minor chemical spill",
        // don't include ';' character to it, it will be removed, because it's field marker in to_string and from_string methods.
		std::string messageCategoryName_; 
		// on/off kytkin, käytetäänkö/näytetäänkö tämän kategorian sanomia vai ei (jos ollaan "vain valitut kategoriat näytetään" moodissa)
        bool show_ = true;
		
    public:

        MessageTypeInfo() = default;
        MessageTypeInfo(MessageType messageType, int messageCategory, const std::string &messageCategoryName, bool show);
        ~MessageTypeInfo() = default;

        MessageType messageType() const { return messageType_; }
        void setMessageType(MessageType messageType) { messageType_ = messageType; }
        int messageCategory() const { return messageCategory_; }
        void setMessageCategory(int messageCategory) { messageCategory_ = messageCategory; }
        const std::string& messageCategoryName() const { return messageCategoryName_; }
        void setMessageCategoryName(const std::string &messageCategoryName);
        bool show() const { return show_; }
        void setShow(bool show) { show_ = show; }

        std::string to_string() const;
        void from_string(const std::string &str);

        bool operator<(const MessageTypeInfo &messageTypeInfo);
    private:

    };
}
