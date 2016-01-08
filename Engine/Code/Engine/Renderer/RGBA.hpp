#pragma once

class RGBA
{
public:
	RGBA();
	RGBA(float Red, float Green, float Blue);
	RGBA(float Red, float Green, float Blue, float Alpha);
	RGBA(unsigned int hexColor);
	~RGBA();

	static inline unsigned char GetRed(unsigned int hexColor);
	static inline unsigned char GetGreen(unsigned int hexColor);
	static inline unsigned char GetBlue(unsigned int hexColor);

	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char alpha;

	static const RGBA WHITE;
	static const RGBA BLACK;
	static const RGBA RED;
	static const RGBA BLUE;
	static const RGBA GREEN;
	static const RGBA MAGENTA;
	static const RGBA YELLOW;
	static const RGBA CYAN;
	static const RGBA ORANGE;
	static const int SHIFT_RED = 6 * 4;
	static const int SHIFT_GREEN = 4 * 4;
	static const int SHIFT_BLUE = 2 * 4;
	static const int RED_MASK = 0xFF000000;
	static const int GREEN_MASK = 0x00FF0000;
	static const int BLUE_MASK = 0x0000FF00;
	static const int ALPHA_MASK = 0x000000FF;

private:

};

inline unsigned char RGBA::GetRed(unsigned int hexColor)
{
	return (unsigned char)((hexColor & RED_MASK) >> SHIFT_RED);
}

inline unsigned char RGBA::GetGreen(unsigned int hexColor)
{
	return (unsigned char)((hexColor & GREEN_MASK) >> SHIFT_GREEN);
}

inline unsigned char RGBA::GetBlue(unsigned int hexColor)
{
	return (unsigned char)((hexColor & BLUE_MASK) >> SHIFT_BLUE);
}

inline bool operator==(const RGBA& lhs, const RGBA& rhs)
{ 
	return (lhs.red == rhs.red) && (lhs.green == rhs.green) && (lhs.blue == rhs.blue) && (lhs.alpha == rhs.alpha); 
}

inline bool operator!=(const RGBA& lhs, const RGBA& rhs)
{
	return !(lhs == rhs);
}