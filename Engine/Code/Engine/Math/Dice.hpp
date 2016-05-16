#pragma once
#include <string>

class Dice
{
public:
    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    Dice(const std::string& diceString);
    //Dice(const char* diceString);
    Dice(int numDice, int numSides, int bonusModifier = 0);
    Dice() {};
    ~Dice() {};

    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    int Roll() const;
    int Roll(unsigned int numRolls) const;
    int GetMaxRoll() const;
    std::string ToString();
private:
    int m_numDice;
    int m_numSides;
    int m_bonusModifier;
};