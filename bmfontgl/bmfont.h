/*
BMFont example implementation with Kerning, for C++ and OpenGL 2.0+

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

#ifndef __BMFONT__
#define __BMFONT__

#include <vector>
#include <map>
#include "colordefs.h"

 const enum fontalign { NONE, AlignNear, AlignCenter, AlignFar };
 const enum fontorigin { Top, Bottom };

//This can and should be substituted by your own Vec2f implementation
class vec2fp
{
public:
	float x;
	float y;
	vec2fp(float x, float y) : x(x), y(y) {}
	vec2fp() : x(0), y(0) { }
};


class txdata {
public:

	float x, y;
	float tx, ty;
	rgb_t colors;

	txdata() : x(0), y(0), tx(0), ty(0), colors(0) { }
	txdata(float x, float y, float tx, float ty, rgb_t colors) : x(x), y(y), tx(tx), ty(ty), colors(colors) {}

};


class KearningInfo
{
public:

	short First;
	short Second;
	short Amount;

	KearningInfo() : First(0), Second(0), Amount(0) { }
};


class CharDescriptor
{

public:
	short x, y;
	short Width;
	short Height;
	short XOffset;
	short YOffset;
	short XAdvance;
	short Page;

	CharDescriptor() : x(0), y(0), Width(0), Height(0), XOffset(0), YOffset(0),
		XAdvance(0), Page(0)
	{ }
};


class BMFont
{

public:

	bool  loadFont(std::string fontfile);
	void  setColor(int r, int g, int b, int a) { fcolor = MAKE_RGBA(r, g, b, a); fcolor2 = 0; }
	void  setColor(rgb_t startcolor, rgb_t endcolor, int dir);
	void  setColor(rgb_t color) { fcolor = color; }
	void  setBlend(int b) { fblend = b; }
	void  setScale(float);
	void  SetCaching(bool en);
	void  clearCache() { txlist.clear(); };
	void  setAngle(int);
	void  setAlign(fontalign align);
	void  setOrigin(fontorigin origin);
	float getHeight() { return LineHeight * fscale; }
	void  useKerning(bool b) { usekern = (b != 0); }

	void  Print(float x, float y, const char *fmt, ...);
	void  Print(float scale, float x, float y, const char *fmt, ...);
	void  Print(float scale, rgb_t color, float x, float y, const char *fmt, ...);

	float getStringWidth(const char *);


	void  Render();

	BMFont(int width, int height)
	{
		surface_width = width;
		surface_height = height;
		mathTableInit();
		setColor(RGB_WHITE);
		fcolor2 = 0;
		mkern_count = 0;
		ftexid = -1;
		fblend = 0;
		fscale = 1.0;
		fangle = 0;
		fcache = false;
		falign = NONE;
		forigin = Bottom;
		usekern = true;
	};
	~BMFont();

private:

	enum { MAX_CHARS = 1024 };

	//Character and page variables
	short LineHeight;
	short Base;
	short Width;
	short Height;
	short Pages;
	short Outline;
	short mkern_count;
	std::map<int, CharDescriptor> Chars;     //Character Descriptor Map
	std::vector<KearningInfo> Kearn;        //Kerning info 
	std::vector<txdata> txlist;             //Vertex Array Data
	std::string mpngname;                    //Png File name storage for loading

	//Modifiers
	bool   usekern;							//Whether or not to process kerning information 
	rgb_t  fcolor;							//Font current color
	rgb_t  fcolor2;							//Second font color for gradient effects
	GLuint ftexid;							//Font Texture ID
	float  fscale;							//Current Font scaling factor
	int    fblend;								//Current Rendering Blending Value"Do I need this?"
	int    falign;								//Font Alignment Variable
	bool   forigin;							//Set Font origin Top Or Bottom
	int    fangle;								//Set font angle
	bool   fcache;                             //Enable Text Caching. WARNING! Text must be pre-staged before entering main program loop!
	int	   surface_width;                      //Width of the orthographic 2D surface the text is going on
	int    surface_height;                     //Height of the orthographic 2D surface the text is going on

	//Internal Functions
	bool parseFont(const std::string &);
	int  getKerningPair(int, int);
	void printKerningPairs();
	void sortKerningPairs();
	void printString(float x, float y, char *textstr);

	//Math Functions
	void   mathTableInit();
	vec2fp makePoint(float x, float y) { vec2fp p; p.x = x; p.y = y; return p; }
	vec2fp rotateAroundPoint(float, float, float, float, float, float);

	//Math Table data for rotation
	float _sin[360];
	float _cos[360];
};


#endif
