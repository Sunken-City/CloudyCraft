#pragma once
#include "Engine/TextRendering/TextEffect.hpp"
#include <string>
#include <vector>

//-----------------------------------------------------------------------------------------------
class StringEffectFragment
{
public:
	StringEffectFragment(const std::string& value);
	static std::vector<StringEffectFragment>  GetStringFragmentsFromXML(const struct XMLNode& node);

private:
	static TextEffect GetTextEffect(const struct XMLNode& node);

public:
	std::string m_value;
	TextEffect m_effect;
};