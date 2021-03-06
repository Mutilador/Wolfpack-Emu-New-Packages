
#if !defined(__ASCIFONTS_H__)
#define __ASCIFONTS_H__

#include <QString>
#include <QFile>
#include <QDataStream>

#include "enums.h"
#include "utilities.h"
#include "surface.h"

class cTexture;

class cAsciiFonts {
private:
	// The fonts are all read in at once since 
	// the file has no indexer and is relatively small (ca. 800kb)
	cSurface *characters[10][224];
	unsigned char height[10]; // Font Heights
public:
	cAsciiFonts();
	~cAsciiFonts();

	void load();
	void unload();
	void reload();

	// Get a single character with the given properties
	inline cSurface *getCharacter(unsigned char font, unsigned char ch) {
		if (font < 10) {
			ch -= 32;
			if (ch < 224) {
				return characters[font][ch];
			}
		}
		return 0;
	}

	// Build a text string
	cTexture *buildText(unsigned char font, const QString &text, unsigned short hue = 0, bool shaded = false, enTextAlign align = ALIGN_LEFT, bool hueAll = false);
	cTexture *buildTextWrapped(unsigned char font, const QString &text, unsigned short maxWidth, unsigned short hue = 0, bool shaded = false, enTextAlign align = ALIGN_LEFT, bool hueAll = false);
};

extern cAsciiFonts *AsciiFonts;

#endif
