/*
BASIC BMFont example implementation with Kerning, for C++ and OpenGL 2.0+

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
--------------------------------------------------------------------------------
These editors can be used to generate BMFonts:
 • http://www.angelcode.com/products/bmfont/ (free, windows)
 • http://glyphdesigner.71squared.com/ (commercial, mac os x)
 • https://github.com/libgdx/libgdx/wiki/Hiero (free, java, multiplatform)


Some code below based on code snippets from this gamedev posting:

http://www.gamedev.net/topic/330742-quick-tutorial-variable-width-bitmap-fonts/

Although I'm giving this away, I'd appreciate an email with fixes or better code!

Updated 2015
*/

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "glext.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include "log.h"
#include "bmfont.h"
#include "gl_basics.h"


#pragma warning (disable : 4996 )



//Parses the font definition file and reads the specified keywords.
//This could be broken out in other, more robust parsing code.
bool BMFont::parseFont(const std::string &fontfile)
{
	std::string Line;
	std::string Read, Key, Value;
	std::size_t i;
	int first, second, amount;

	wrlog("Fontfile here is %s", fontfile.c_str());

	std::ifstream Stream(fontfile.c_str());

	KearningInfo K;
	CharDescriptor C;

	while (!Stream.eof())
	{
		std::stringstream LineStream;
		std::getline(Stream, Line);
		LineStream << Line;

		//read the line's type
		LineStream >> Read;
		if (Read == "common")
		{
			//this holds common data
			while (!LineStream.eof())
			{
				std::stringstream Converter;
				LineStream >> Read;
				i = Read.find('=');
				Key = Read.substr(0, i);
				Value = Read.substr(i + 1);

				//assign the correct value
				Converter << Value;
				if (Key == "lineHeight")
				{
					Converter >> LineHeight;
				}
				else if (Key == "base")
				{
					Converter >> Base;
				}
				else if (Key == "scaleW")
				{
					Converter >> Width;
				}
				else if (Key == "scaleH")
				{
					Converter >> Height;
				}
				else if (Key == "pages")
				{
					Converter >> Pages;
				}
				else if (Key == "outline")
				{
					Converter >> Outline;
				}
			}
		}

		else if (Read == "page")
		{
			while (!LineStream.eof())
			{
				std::stringstream Converter;
				LineStream >> Read;
				i = Read.find('=');
				Key = Read.substr(0, i);
				Value = Read.substr(i + 1);

				//assign the correct value
				Converter << Value;
				if (Key == "file")
				{
					Converter >> mpngname;
					if (mpngname.front() == '"') {
						mpngname.erase(0, 1); // erase the first character
						mpngname.erase(mpngname.size() - 1); // erase the last character
						//To remove the double quotes.
					}
				}
			}
		}

		else if (Read == "char")
		{
			//This is data for each specific character.
			int CharID = 0;

			while (!LineStream.eof())
			{
				std::stringstream Converter;
				LineStream >> Read;
				i = Read.find('=');
				Key = Read.substr(0, i);
				Value = Read.substr(i + 1);

				//Assign the correct value
				Converter << Value;
				if (Key == "id")
				{
					Converter >> CharID;
				}
				else if (Key == "x")
				{
					Converter >> C.x;
				}
				else if (Key == "y")
				{
					Converter >> C.y;
				}
				else if (Key == "width")
				{
					Converter >> C.Width;
				}
				else if (Key == "height")
				{
					Converter >> C.Height;
				}
				else if (Key == "xoffset")
				{
					Converter >> C.XOffset;
				}
				else if (Key == "yoffset")
				{
					Converter >> C.YOffset;
				}
				else if (Key == "xadvance")
				{
					Converter >> C.XAdvance;
				}
				else if (Key == "page")
				{
					Converter >> C.Page;
				}
			}

			Chars.insert(std::map<int, CharDescriptor>::value_type(CharID, C));
		}

		else if (Read == "kernings")
		{
			while (!LineStream.eof())
			{
				std::stringstream Converter;
				LineStream >> Read;
				i = Read.find('=');
				Key = Read.substr(0, i);
				Value = Read.substr(i + 1);

				//assign the correct value
				Converter << Value;
				if (Key == "count")
				{
					Converter >> mkern_count;
				}
			}
		}

		else if (Read == "kerning")
		{
			while (!LineStream.eof())
			{
				std::stringstream Converter;
				LineStream >> Read;
				i = Read.find('=');
				Key = Read.substr(0, i);
				Value = Read.substr(i + 1);

				//assign the correct value
				Converter << Value;
				if (Key == "first")
				{
					Converter >> K.First; Converter >> first;
				}

				else if (Key == "second")
				{
					Converter >> K.Second; Converter >> second;
				}

				else if (Key == "amount")
				{
					Converter >> K.Amount; Converter >> amount;
				}
			}
			//LOG_DEBUG("Done with this pass");
			Kearn.push_back(K);
		}
	}

	Stream.close();
	return true;
}



//Tries to obtain the Kerning values for each pair of letters, if loaded and requested.
int BMFont::getKerningPair(int first, int second)
{
	if (mkern_count && usekern) //Only process if there actually is kerning information and we want to use it.
	{
		//Kearning is checked for every character processed. This is expensive in terms of processing time.
		for (int j = 0; j < mkern_count; j++)
		{
			//Check adds processing, but should reduce time considerably for lower ascii letters.
			if (Kearn[j].First > first) return 0;

			if (Kearn[j].First == first && Kearn[j].Second == second)
			{
				// LOG_DEBUG("Kerning Pair Found!!!");kerning._Left
				// LOG_DEBUG("FIRST: %d SECOND: %d offset %d",first,second,Kearn[j].Amount);
				return Kearn[j].Amount;
			}
		}
	}
	return 0;
}



//Returns the width in pixels of the font string being processed.
float BMFont::getStringWidth(const char *string)
{
	float total = 0;
	size_t len = strlen(string);
	CharDescriptor  *f;

	for (size_t i = 0; i != len; ++i)
	{
		f = &Chars[string[i]];

		if (len > 1 && i < len)
		{
			total += getKerningPair(string[i], string[i + 1]);
		}

		total += fscale * f->XAdvance;
	}

	return total;
}


//Simple loader that checks if the file exists and then loads it and the associated png
bool  BMFont::loadFont(std::string fontfile)
{
	wrlog("Starting to parse font");

	std::ifstream Stream(fontfile);
	if (!Stream.is_open())
	{
		wrlog("Cannot Find Font File %s, exiting.", fontfile);
		return false;
	}
	Stream.close();

	wrlog("Passing font");
	//LOG_DEBUG("Starting to Parse Font %s",fontfile);
	parseFont(fontfile);
	//LOG_DEBUG("Finished Parsing Font %s",fontfile);
	mkern_count = (short)Kearn.size();
	wrlog("Finished to parse font");

	sortKerningPairs();

	ftexid = LoadPNG(mpngname.c_str());
	if (!ftexid)
	{
		LOG_DEBUG("Cannot find font texture for loading %s", fontfile);
		return false;
	}

	return true;
}

//Simple Vector Renderer Needs moved to it's own file and class for real world use. (Or changed to use shaders)
void BMFont::Render()
{
	//Select and enable the font texture. (With mipmapping.)
	use_texture(&ftexid, 1, 1);
	//Set type of blending to use with this font.
	setBlendMode(fblend);

	//Enable Client States
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, sizeof(txdata), &txlist[0].x);

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(txdata), &txlist[0].tx);

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(txdata), &txlist[0].colors);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei) txlist.size());

	// Finished Drawing, disable client states.
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	//Caching Check, if we are pre caching the text.
	if (!fcache) txlist.clear();
}


void BMFont::mathTableInit()
{
	for (int a = 0; a < 360; a++)
	{
		_sin[a] = (float)sin((float)a * 3.14159265358979323846264f / 180.0f); //sin(a/57.32484076);
		_cos[a] = (float)cos((float)a * 3.14159265358979323846264f / 180.0f); //cos(a/57.32484076);
	}
}


void BMFont::setAngle(int angle)
{
	if (angle > 360 || angle < -360) angle = 0;

	if (angle < 0) angle = 360 + angle;
	fangle = angle;
}


void BMFont::SetCaching(bool en)
{
	if (en) fcache = true;
	else fcache = false;
}


void BMFont::setColor(rgb_t startcolor, rgb_t endcolor, int dir)
{
	fcolor = startcolor;
	fcolor2 = endcolor;
}


void BMFont::setAlign(fontalign align)
{
	if (align < 4) falign = align;
}


void BMFont::setScale(float newscale)
{
	if (newscale > 0.1f && newscale < 10.0f)
	{
		fscale = newscale;
	}
}


void BMFont::setOrigin(fontorigin origin)
{
	if (origin == 0) forigin = false;
	else forigin = true;
}



//This is the main printing process that scales and fills in the vector structure to be printed later.
void BMFont::printString(float x, float y, char *textstr)
{
	CharDescriptor  *f;		   //Pointer to font.
	float CurX = 0.0f;
	float CurY = 0.0f;
	float DstX = 0.0f;
	float DstY = 0.0f;
	size_t Flen = 0;
	float s;
	float t;
	float w;
	float h;

	//Calculate String Length
	Flen = strlen(textstr);

	//Get Origin point, this can be modified to the the center of the string, add more code here.
	vec2fp center = makePoint(x, y);

	int total;
	switch (falign)
	{
	case fontalign::AlignNear:
		x = 1;
		break;
	case fontalign::AlignFar:
		x = (float)(surface_width);
		total = (int)getStringWidth(textstr);
		x -= total;
		break;
	case fontalign::AlignCenter:
		x = (float)(surface_width / 2);
		total = (int)getStringWidth(textstr);
		total = total / 2;
		x -= total;
		break;
	default: break;
	}

	//Adjust for scaling factor
	//If orgin of Ortho View is bottom left.
	if (forigin == fontorigin::Bottom)
	{
		y += (LineHeight * fscale);
	}

	//Makes sure the x and y origin doesn't move with scaling.
	x = x * (1.0f / fscale);
	y = y * (1.0f / fscale);


	//Now scaled X and Y are the new origins.

	for (int i = 0; i != Flen; ++i)
	{

		f = &Chars[textstr[i]];


		CurX = x + f->XOffset;
		CurY = y;
		//Adjust for Origin
		if (forigin == fontorigin::Bottom)
			CurY -= f->YOffset; // - to flip
		else
			CurY += f->YOffset;

		DstX = CurX + f->Width;
		DstY = CurY;
		//Adjust for origin
		if (forigin == fontorigin::Bottom)
			DstY -= f->Height; // - to flip
		else
			DstY += f->Height;

		//Scaling
		CurX *= fscale;
		CurY *= fscale;
		DstX *= fscale;
		DstY *= fscale;

		//Calculate texture coords for this letter
		//This could be pre-calculated.
		s = (float)f->x / Width;
		t = (float) 1.0f - (f->y / (float)Height);
		w = (float)f->Width / Width;
		h = (float)f->Height / (float)Height;

		//If we are printing at an angle, there are a lot more calculations.
		//I havent figured out an elegant way to shorten this yet. So this is a quick fix.
		if (fangle)
		{
			//I'm using a precalculated lookup table for 360 degrees, in 1 degree increments. 
			//It's fast, and I like it. You can use your normal cos,sin and radians here if you need to 
			//use smaller increments.

			vec2fp p0 = rotateAroundPoint(CurX, CurY, center.x, center.y, _cos[fangle], _sin[fangle]);
			vec2fp p1 = rotateAroundPoint(DstX, CurY, center.x, center.y, _cos[fangle], _sin[fangle]);
			vec2fp p2 = rotateAroundPoint(DstX, DstY, center.x, center.y, _cos[fangle], _sin[fangle]);
			vec2fp p3 = rotateAroundPoint(CurX, DstY, center.x, center.y, _cos[fangle], _sin[fangle]);

			//V0
			//txlist.push_back(txdata(CurX, CurY, s, t,  fcolor));
			txlist.emplace_back(txdata(p0.x, p0.y, s, t, fcolor));
			//V1
			txlist.emplace_back(txdata(p1.x, p1.y, s + w, t, fcolor));
			//V2
			txlist.emplace_back(txdata(p2.x, p2.y, s + w, t - h, fcolor));
			//V2
			txlist.emplace_back(txdata(p2.x, p2.y, s + w, t - h, fcolor));
			//V3
			txlist.emplace_back(txdata(p3.x, p3.y, s, t - h, fcolor));
			//V0
			txlist.emplace_back(txdata(p0.x, p0.y, s, t, fcolor));
		}
		else
		{
			//V0
			txlist.emplace_back(txdata(CurX, CurY, s, t, fcolor));
			//V1
			txlist.emplace_back(txdata(DstX, CurY, s + w, t, fcolor));
			//V2
			txlist.emplace_back(txdata(DstX, DstY, s + w, t - h, fcolor));
			//V2
			txlist.emplace_back(txdata(DstX, DstY, s + w, t - h, fcolor));
			//V3
			txlist.emplace_back(txdata(CurX, DstY, s, t - h, fcolor));
			//V0
			txlist.emplace_back(txdata(CurX, CurY, s, t, fcolor));
		}
		//Only check kerning if there is greater then 1 character and 
		//if the check character is 1 less then the end of the string.
		if ((int) Flen > 1 && i < (int) Flen)
		{
			x += getKerningPair(textstr[i], textstr[i + 1]);
		}

		x += f->XAdvance;
	}
}



//This is the main printing process that scales and fills in the vector structure to be printed later.
void BMFont::Print(float x, float y, const char *fmt, ...)
{
	char	text[MAX_CHARS] = "";	                            // Holds Our String

	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	vsprintf_s(text, fmt, ap);						    // And Converts Symbols To Actual Numbers
	va_end(ap);

	printString(x, y, text);
}


void BMFont::Print(float scale, float x, float y, const char *fmt, ...)
{
	char	text[MAX_CHARS] = "";	                            // Holds Our String

	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	vsprintf_s(text, fmt, ap);						    // And Converts Symbols To Actual Numbers
	va_end(ap);

	setScale(scale);
	printString(x, y, text);
}


void BMFont::Print(float scale, rgb_t color, float x, float y, const char *fmt, ...)
{
	char	text[MAX_CHARS] = "";	                            // Holds Our String

	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	vsprintf_s(text, fmt, ap);						    // And Converts Symbols To Actual Numbers
	va_end(ap);

	setScale(scale);
	setColor(color);
	printString(x, y, text);
}


BMFont::~BMFont()
{
	Chars.clear();
	Kearn.clear();
	txlist.clear();
	FreeTexture(ftexid);
}


void BMFont::sortKerningPairs()
{
	//If the pairs are sorted, I can get a speed boost by not having to iterate over the entire list for each character.
	std::sort(Kearn.begin(), Kearn.end(), [](KearningInfo const &a, KearningInfo const &b) {return a.First < b.First; });
}

//Debug printing of kearning.
void BMFont::printKerningPairs()
{

	//Kearning is checked for every character processed. This is expensive in terms of processing time.
	for (unsigned int j = 0; j < Kearn.size(); j++)
		//for (std::vector<KearningInfo>::size_type j = 0; j < Kearn.size(); ++j);
	{
		wrlog("FIRST: %d SECOND: %d offset %d", Kearn[j].First, Kearn[j].Second, Kearn[j].Amount);
	}

}


vec2fp BMFont::rotateAroundPoint(float x, float y, float cx, float cy, float cosTheta, float sinTheta)
{
	vec2fp p;
	p.x = ((((x)-(cx)) * (cosTheta)) - (((y)-(cy)) * (sinTheta))) + (cx);
	p.y = ((((x)-(cx)) * (sinTheta)) + (((y)-(cy)) * (cosTheta))) + (cy);
	return p;
}