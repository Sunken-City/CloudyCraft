#include "Engine/Input/XMLUtils.hpp"

//-----------------------------------------------------------------------------------
XMLNode XMLUtils::GetChildNodeAtPosition(const XMLNode& parentNode, const std::string nodeName, int position /*= NULL*/)
{
    return parentNode.getChildNode(nodeName.c_str(), position);
}

//-----------------------------------------------------------------------------------
XMLNode XMLUtils::OpenXMLDocument(const std::string& path)
{
    return XMLNode::openFileHelper(path.c_str());
}

//-----------------------------------------------------------------------------------
XMLNode XMLUtils::ParseXMLFromString(const std::string& string)
{
    return XMLNode::parseString(string.c_str());
}

//-----------------------------------------------------------------------------------
std::string XMLUtils::GetAttribute(const XMLNode& node, const std::string& attributeName)
{
    const char* attrVal = node.getAttribute(attributeName.c_str());
    if (!attrVal)
    {
        return "";
    }

    return std::string(attrVal);
}

//-----------------------------------------------------------------------------------
std::vector<XMLNode> XMLUtils::GetChildren(const XMLNode& parent)
{
    int childIndex = 0;
    XMLNode childNode = parent.getChildNode(childIndex);
    std::vector<XMLNode> children;
    while (!childNode.isEmpty())
    {
        children.push_back(childNode);
        childNode = parent.getChildNode(++childIndex);
    }
    children.push_back(childNode);
    return children;
}

///----------------------------------------------------------
/// Destroys all the allocated data for an XMLNode tree. 
///----------------------------------------------------------
void XMLUtils::DestroyXMLDocument(XMLNode& xmlDocumentToDestroy)
{
    while (!xmlDocumentToDestroy.getParentNode().isEmpty())
    {
        xmlDocumentToDestroy = xmlDocumentToDestroy.getParentNode();
    }

    xmlDocumentToDestroy.deleteNodeContent();
}

///----------------------------------------------------------
/// 
///----------------------------------------------------------
std::string XMLUtils::GetXMLErrorDescForErrorCode(XMLError xmlErrorCode)
{
    switch (xmlErrorCode)
    {
    case eXMLErrorNone:
    case eXMLErrorMissingEndTag:					return "MissingEndTag";
    case eXMLErrorNoXMLTagFound:					return "NoXMLTagFound";
    case eXMLErrorEmpty:							return "Empty";
    case eXMLErrorMissingTagName:					return "MissingTagName";
    case eXMLErrorMissingEndTagName:				return "MissingEndTagName";
    case eXMLErrorUnmatchedEndTag:					return "UnmatchedEndTag";
    case eXMLErrorUnmatchedEndClearTag:				return "UnmatchedEndClearTag";
    case eXMLErrorUnexpectedToken:					return "UnexpectedToken";
    case eXMLErrorNoElements:						return "NoElements";
    case eXMLErrorFileNotFound:						return "FileNotFound";
    case eXMLErrorFirstTagNotFound:					return "FirstTagNotFound";
    case eXMLErrorUnknownCharacterEntity:			return "UnknownCharacterEntity";
    case eXMLErrorCharConversionError:				return "CharConversionError";
    case eXMLErrorCannotOpenWriteFile:				return "CannotOpenWriteFile";
    case eXMLErrorCannotWriteFile:					return "CannotWriteFile";
    case eXMLErrorBase64DataSizeIsNotMultipleOf4:	return "Base64DataSizeIsNotMultipleOf4";
    case eXMLErrorBase64DecodeIllegalCharacter:		return "Base64DecodeIllegalCharacter";
    case eXMLErrorBase64DecodeTruncatedData:		return "Base64DecodeTruncatedData";
    case eXMLErrorBase64DecodeBufferTooSmall:		return "Base64DecodeBufferTooSmall";
    default:										return "Unknown XML error code";
    };
}

///------------------------------------------------------------------
/// Retrieves the attribute as a string, returns empty string if not found.
///
/// This function first attempts to fetch the named attribute as a
/// traditional XML attribute (like type="").  If that fails, it 
/// goes further to see if there's a child element with that same name
/// with a value="" attribute.  This function would return "circle"
/// with either of the two Shape XMLNodes:
/// 
/// <Shape type="circle"/>
///          - or - 
/// <Shape>
///     <Type value="circle"/>
/// </Shape>
/// 
/// Historically, many XML authors preferred to not use a lot of attributes, 
/// and would instead switch to using elements of the same name with value="".
///
/// Some complex data types may want to allow the user to set their
/// value simply as an attribute (rotationDegrees="90") but may also
/// want to allow additional attributes that are specific to it.  In
/// these cases, the more complex version of the data type can be moved
/// to a separate element:
///
/// <Shape type="triangle" rotationDegrees="90" ... />
///         - or - 
/// <Shape type="triangle" ...>
///     <RotationDegrees value="90" interpolationSeconds="3.5" .../>
/// </Shape>
///------------------------------------------------------------------
std::string XMLUtils::GetXMLAttributeAsString(const XMLNode& node, const std::string& attributeName, bool& wasAttributePresent_out)
{
    std::string	attributeValue;
    wasAttributePresent_out = false;

    // First try to get it as an attribute with that property name
    if (node.getAttribute(attributeName.c_str()))
    {
        attributeValue = node.getAttribute(attributeName.c_str());
        wasAttributePresent_out = true;
        node.markUsed();
    }
    else
    {
        // Second try to get it as a child node with that name
        XMLNode		childNode = node.getChildNode(attributeName.c_str());
        if (childNode.hasText())
        {
            attributeValue = childNode.getText();
            wasAttributePresent_out = true;
            node.markUsed();
        }
        // Third, try to get it as an attribute named "Value" of the child node with that property name
        else if (!childNode.isEmpty())
        {
            attributeValue = GetXMLAttributeAsString(childNode, "value", wasAttributePresent_out);
            node.markUsed();
        }
    }

    return attributeValue;
}



///----------------------------------------------------------
/// Retrieves the childNode with the specified name at the 
/// specified position of the parent.  Returns true if the child
/// is not empty (and therefore likely to be a valid child.
///----------------------------------------------------------
bool XMLUtils::GetXMLNodeByNameSearchingFromPosition(const XMLNode& parentNode, const std::string& childName, int& position_inout, XMLNode& childNode_out)
{
    bool wasFound = false;
    parentNode.markUsed();

    childNode_out = parentNode.getChildNode(childName.c_str(), &position_inout);

    if (!childNode_out.isEmpty())
    {
        childNode_out.markUsed();
        wasFound = true;
    }

    return wasFound;
}