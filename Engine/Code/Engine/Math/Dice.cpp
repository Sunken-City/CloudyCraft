#include "Engine/Math/Dice.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------
Dice::Dice(int numDice, int numSides, int bonusModifier /*= 0*/)
    : m_numSides(numSides)
    , m_numDice(numDice)
    , m_bonusModifier(bonusModifier)
{

}

//-----------------------------------------------------------------------------------
Dice::Dice(const std::string& diceString)
    : m_numSides(0)
    , m_numDice(0)
    , m_bonusModifier(0)
{
    std::vector<std::string>* stringPieces = SplitStringOnMultipleDelimiters(diceString, 3, "d", "+", "-");
    m_numDice = stoi(stringPieces->at(0));
    m_numSides = stoi(stringPieces->at(1));
    if (stringPieces->size() == 3)
    {
        m_bonusModifier = stoi(stringPieces->at(2));
        if (diceString.find('-') != std::string::npos)
        {
            m_bonusModifier *= -1;
        }
    }
    delete stringPieces;
}

//-----------------------------------------------------------------------------------
int Dice::Roll() const 
{
    int diceRollResult = 0;
    for (int i = 0; i < m_numDice; ++i)
    {
        diceRollResult += MathUtils::GetRandom(1, m_numSides) + m_bonusModifier;
    }
    return diceRollResult;
}

//-----------------------------------------------------------------------------------
int Dice::Roll(unsigned int numRolls) const
{
    int diceRollResult = 0;
    for (unsigned int i = 0; i < numRolls; ++i)
    {
        diceRollResult += Roll();
    }
    return diceRollResult;
}

int Dice::GetMaxRoll() const
{
    return (m_numDice * m_numSides) + m_bonusModifier;
}

std::string Dice::ToString()
{
    return Stringf("%id%i%c%i", m_numDice, m_numSides, m_bonusModifier < 0 ? '-' : '+', (int)abs(m_bonusModifier));
}
