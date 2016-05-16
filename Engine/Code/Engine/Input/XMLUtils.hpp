#pragma once
#include "ThirdParty/Parsers/XMLParser.hpp"
#include <string>
#include <vector>

//Includes code based off of code from Ken Harward

class XMLUtils
{
public:
    static XMLNode GetChildNodeAtPosition(const XMLNode& parentNode, const std::string nodeName, int position = NULL);
    static XMLNode OpenXMLDocument(const std::string& path);
    static XMLNode ParseXMLFromString(const std::string& string);
    static std::string GetAttribute(const XMLNode& node, const std::string& attributeName);
    static std::vector<XMLNode> GetChildren(const XMLNode& parent);

    //================================================================================================================================
    //
    //================================================================================================================================
    std::string			GetXMLErrorDescForErrorCode(XMLError xmlErrorCode);
    bool				GetXMLNodeByNameSearchingFromPosition(const XMLNode& parentNode, const std::string& childName, int& position_inout, XMLNode& childNode_out);
    std::string			GetXMLAttributeAsString(const XMLNode& node, const std::string& attributeName, bool& wasAttributePresent_out);
    void				DestroyXMLDocument(XMLNode& xmlDocumentToDestroy);

    //-----------------------------------------------------------------------------------
    template<typename DestinationType>
    void SetTypeFromString(DestinationType& destinationOut, const std::string& asString)
    {
        const DestinationType constructedFromString(asString);
        destinationOut = constructedFromString;
    }

    //-----------------------------------------------------------------------------------
    template< typename ValueType >
    ValueType GetXMLAttributeOfType(const XMLNode& node, const std::string& propertyName, bool& wasPropertyPresent_out)
    {
        ValueType	outValue;
        std::string	valueAsString = GetXMLAttributeAsString(node, propertyName, wasPropertyPresent_out);
        SetTypeFromString(outValue, valueAsString);

        return outValue;
    }


    //-----------------------------------------------------------------------------------
    template< typename ValueType >
    ValueType ReadXMLAttribute(const XMLNode& node, const std::string& propertyName, const ValueType& defaultValue)
    {
        bool wasPropertyPresent = false;

        ValueType outValue = GetXMLAttributeOfType<ValueType>(node, propertyName, wasPropertyPresent);
        if (!wasPropertyPresent)
            outValue = defaultValue;

        return outValue;
    }


    //-----------------------------------------------------------------------------------
    template< typename ValueType >
    void WriteXMLAttribute(XMLNode& node, const std::string& propertyName, ValueType& value, const ValueType& defaultValue)
    {
        SetStringProperty(node, propertyName, GetTypedObjectAsString(value), GetTypedObjectAsString(defaultValue));
    }




};

