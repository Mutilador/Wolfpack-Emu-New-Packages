//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

#ifndef __NEWMAGIC_H__
#define __NEWMAGIC_H__

#include "typedefs.h"
#include "coord.h"
#include "defines.h"

/*!
	Several defines for item effects
*/
#define	EFFECT_BLESS 0x373A
#define EFFECT_CURSE 0x374A

// Prototype for Spell Stubs:
// P_CHAR pMage, Coord_cl &pos = pMage->pos, UINT32 target = INVALID_SERIAL

/*!
	\a pMage is the caster of the spell. This may be NULL.
	\a tPos is the target position (if applicable). 
	If there is no specific location then the location of the 
	caster can be used. \a tSerial is the targetted serial and 
	defaults to INVALID_SERIAL. \a magery is used to override 
	the magery value for damage and duration calculations.
*/
typedef void (*spellStub)( P_CHAR pMage, Coord_cl tPos, UINT16 model = 0, SERIAL tSerial = INVALID_SERIAL, INT32 magery = -1 );

// Defines for the Bitfield
#define NO_TARGET		0x00
#define TARGET_CHAR		0x01
#define TARGET_ITEM		0x02
#define TARGET_GROUND	0x04

#define SPELL_REFLECTABLE	0x01
#define SPELL_AGRESSIVE		0x02
#define SPELL_MOVEMENT		0x04

struct stReagents
{
	UINT8 ginseng, bloodmoss, mandrake, blackpearl, spidersilk, sulfurash, garlic, nightshade;
};

struct stNewSpell
{
	QString name, mantra, target;
	UINT16 booklow, bookhigh, scrolllow, scrollhigh, actiondelay, delay;
	UINT8 action, targets, flags, mana;
	stReagents reagents;
};

struct stSpellStub
{
	spellStub stub;
};

class cUORxTarget;

class cNewMagic
{
private:
	static stSpellStub spellStubs[];
	stNewSpell spells[64];
public:
	cNewMagic() {}
	virtual ~cNewMagic() {}

	// Public functions
	void castSpell( P_CHAR pMage, UINT8 spell );
	void useWand( P_CHAR pMage, P_ITEM pWand );
	void useScroll( P_CHAR pMage, P_ITEM pScroll );
	bool checkTarget( P_CHAR pCaster, stNewSpell *sInfo, cUORxTarget *target );

	void load();
	void unload();
	
	stSpellStub *findSpellStub( UINT8 id );
	stNewSpell	*findSpell( UINT8 id );

    void failSpell( P_CHAR pMage, bool fizzle = true );
	bool useMana( P_CHAR pMage, UINT8 spell );
	bool useReagents( P_CHAR pMage, UINT8 spell );
	bool checkSkill( P_CHAR pMage, UINT8 spell, bool scroll = false );

	// Spell stubs
	static void spellClumsy( P_CHAR pMage, Coord_cl tPos, UINT16 model, SERIAL tSerial = INVALID_SERIAL, INT32 magery = -1 );	
};

extern cNewMagic *NewMagic;

#endif