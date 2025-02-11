#include "NFmiDictionaryFunction.h"
#include "NFmiFileSystem.h"

#include "json_spirit_writer.h"
#include "json_spirit_reader.h"
#include "json_spirit_writer_options.h"
#include <fstream>

namespace JsonUtils
{
    inline std::string MakeErrorString(const std::string& startStr, const std::string& endStr, const std::string& objectName, const std::string& endCharacters)
    {
        std::string str = ::GetDictionaryString(startStr.c_str());
        str += " ";
        str += objectName;
        str += " ";
        str += ::GetDictionaryString(endStr.c_str());
        str += endCharacters;

        return str;
    }

    template<typename T>
    inline void ParseJsonValue(T& object, json_spirit::Value& theValue)
    {
        if(theValue.type() == json_spirit::obj_type)
        {
            json_spirit::Object tmpObj = theValue.get_obj();
            for(json_spirit::Object::iterator it = tmpObj.begin(); it != tmpObj.end(); ++it)
            {
                object.ParseJsonPair(*it);
            }
        }
    }

    template<typename T>
    inline bool StoreObjectInJsonFormat(const T& object, const std::string& theFilePath, const std::string& theObjectName, std::string& theErrorStringOut)
    {
        json_spirit::Object jsonObject = T::MakeJsonObject(object);
        if(jsonObject.size() == 0)
        {
            theErrorStringOut = MakeErrorString("The given", theObjectName, "was completely empty, nothing to store.", "");
            return false;
        }

        std::stringstream outStream;
        json_spirit::write(jsonObject, outStream, json_spirit::pretty_print);

        try
        {
            NFmiFileSystem::SafeFileSave(theFilePath, outStream.str());
            return true;
        }
        catch(std::exception& e)
        {
            theErrorStringOut = MakeErrorString("Error while trying to save given", theObjectName, "to file", ":\n");
            theErrorStringOut += theFilePath;
            theErrorStringOut += "\n";
            theErrorStringOut += e.what();
        }
        catch(...)
        {
            theErrorStringOut = MakeErrorString("Unknown error while trying to save given", theObjectName, "to file", ":\n");
            theErrorStringOut += theFilePath;
        }
        return false;
    }

    template<typename T>
    inline bool ReadObjectInJsonFormat(T& objectOut, const std::string& theFilePath, const std::string& theObjectName, std::string& theErrorStringOut)
    {
        if(theFilePath.empty())
        {
            theErrorStringOut = MakeErrorString("Given", theObjectName, "file name was empty, you must provide absolute path and filename for data.\nE.g.C:\\data\\beta1.bpr", "");
            return false;
        }

        if(NFmiFileSystem::FileExists(theFilePath) == false)
        {
            theErrorStringOut = MakeErrorString("Given", theObjectName, "file doesn't exist", ":\n");
            theErrorStringOut += theFilePath;
            return false;
        }

        std::ifstream in(theFilePath.c_str(), std::ios_base::in | std::ios_base::binary);
        if(!in)
        {
            theErrorStringOut = MakeErrorString("Unable to open given", theObjectName, "file", ":\n");
            theErrorStringOut += theFilePath;
            return false;
        }

        try
        {
            json_spirit::Value jsonValue;
            if(json_spirit::read(in, jsonValue))
            {
                objectOut = T(); // Objekti pit‰‰ nollata oletus arvoilla ennen parsimista
                ParseJsonValue(objectOut, jsonValue);
                return true;
            }
            else
            {
                theErrorStringOut = MakeErrorString("Unable to read", theObjectName, "(malformatted json?) from file", ":\n");
                theErrorStringOut += theFilePath;
            }
        }
        catch(std::exception& e)
        {
            theErrorStringOut = MakeErrorString("Unable to read", theObjectName, "from file", ":\n");
            theErrorStringOut += theFilePath;
            theErrorStringOut += "\n";
            theErrorStringOut += ::GetDictionaryString("Reason");
            theErrorStringOut += ": ";
            theErrorStringOut += e.what();
        }
        catch(...)
        {
            theErrorStringOut = MakeErrorString("Unknown error while reading", theObjectName, "from file", ":\n");
            theErrorStringOut += theFilePath;
        }

        return false;
    }

    inline void AddNonEmptyStringJsonPair(const std::string& value, const std::string& valueJsonName, json_spirit::Object& jsonObject)
    {
        if(!value.empty())
            jsonObject.push_back(json_spirit::Pair(valueJsonName, value));
    }

} // namespace JsonUtils
