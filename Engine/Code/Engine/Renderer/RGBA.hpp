#pragma once

class Vector4;

class RGBA
{
public:
	//CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
	RGBA();
	RGBA(float Red, float Green, float Blue);
	RGBA(float Red, float Green, float Blue, float Alpha);
	RGBA(unsigned int hexColor);
	~RGBA();

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	static inline unsigned char GetRed(unsigned int hexColor);
	static inline unsigned char GetGreen(unsigned int hexColor);
	static inline unsigned char GetBlue(unsigned int hexColor);
	static inline RGBA CreateFromUChars(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
	inline RGBA WithoutAlpha();
	Vector4 ToVec4();

	//OPERATORS//////////////////////////////////////////////////////////////////////////
	//Damp an RGBA color by a single value. Doesn't affect Alpha.
	RGBA& operator-=(unsigned char rhs)
	{
		this->red -= rhs;
		this->green -= rhs;
		this->blue -= rhs;
		return *this;
	}

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char alpha;

	//CONSTANTS//////////////////////////////////////////////////////////////////////////
	static const RGBA WHITE;
	static const RGBA BLACK;
	static const RGBA RED;
	static const RGBA BLUE;
	static const RGBA GREEN;
	static const RGBA MAGENTA;
	static const RGBA YELLOW;
	static const RGBA CYAN;
	static const RGBA ORANGE;
	static const RGBA CORNFLOWER_BLUE;
	static const RGBA TURQUOISE;
	static const RGBA SLATE;
	static const RGBA FOREST_GREEN;
	static const RGBA SEA_GREEN;
	static const RGBA KHAKI;
	static const RGBA GOLD;
	static const RGBA CHOCOLATE;
	static const RGBA SADDLE_BROWN;
	static const RGBA MAROON;
	static const RGBA GRAY;
	static const RGBA VAPORWAVE;
	static const int SHIFT_RED = 6 * 4;
	static const int SHIFT_GREEN = 4 * 4;
	static const int SHIFT_BLUE = 2 * 4;
	static const int RED_MASK = 0xFF000000;
	static const int GREEN_MASK = 0x00FF0000;
	static const int BLUE_MASK = 0x0000FF00;
	static const int ALPHA_MASK = 0x000000FF;
	
};

//---------------------------------------------------------------------------
//Damp an RGBA color by a single value. Doesn't affect Alpha.
inline RGBA operator-(RGBA lhs, unsigned char rhs)
{
	lhs -= rhs;
	return lhs;
}

//---------------------------------------------------------------------------
inline RGBA RGBA::CreateFromUChars(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	return RGBA((red << RGBA::SHIFT_RED) + (green << RGBA::SHIFT_GREEN) + (blue << RGBA::SHIFT_BLUE) + alpha);
}

//---------------------------------------------------------------------------
inline unsigned char RGBA::GetRed(unsigned int hexColor)
{
	return (unsigned char)((hexColor & RED_MASK) >> SHIFT_RED);
}

//---------------------------------------------------------------------------
inline unsigned char RGBA::GetGreen(unsigned int hexColor)
{
	return (unsigned char)((hexColor & GREEN_MASK) >> SHIFT_GREEN);
}

//---------------------------------------------------------------------------
inline unsigned char RGBA::GetBlue(unsigned int hexColor)
{
	return (unsigned char)((hexColor & BLUE_MASK) >> SHIFT_BLUE);
}

//---------------------------------------------------------------------------
inline bool operator==(const RGBA& lhs, const RGBA& rhs)
{ 
	return (lhs.red == rhs.red) && (lhs.green == rhs.green) && (lhs.blue == rhs.blue) && (lhs.alpha == rhs.alpha); 
}

//---------------------------------------------------------------------------
inline bool operator!=(const RGBA& lhs, const RGBA& rhs)
{
	return !(lhs == rhs);
}

//-----------------------------------------------------------------------------------
inline RGBA RGBA::WithoutAlpha()
{
	return RGBA((red << RGBA::SHIFT_RED) + (green << RGBA::SHIFT_GREEN) + (blue << RGBA::SHIFT_BLUE));
}
