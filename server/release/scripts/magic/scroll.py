#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Script for Spell Scrolls                                      #
#===============================================================#

import wolfpack
import magic
from magic.utilities import MODE_SCROLL

# calculate the spell-id of a scroll
def calcSpellId( item ):
	tile = wolfpack.tiledata( item.id )
	spell = tile[ "unknown1" ]

	if spell == 0:
		raise

	return ( spell - 1 )

def onUse( char, item ):
	try:
		spell = calcSpellId(item)
	except:
		char.socket.sysmessage('This scroll seems to be broken.')
		return False

	magic.castSpell(char, spell + 1, MODE_SCROLL, [], None, item)

	return True

def onDropOnItem( cont, item ):
	# Who is dragging us?
	char = item.container

	if not char.ischar():
		return False

	if cont == char.getbackpack() or cont == item.container:
		return False

	try:
		spell = calcSpellId( item )
	except:
		char.socket.sysmessage( "Error while calculating scroll id." )
		return False

	if cont.hasscript( 'magic.spellbook' ):
		if spell >= 64:
			char.socket.sysmessage( "Scroll with invalid spell-id: %d" % spell )
		else:
			if not magic.spellbook.hasspell( cont, spell ):
				magic.spellbook.addspell( cont, spell )

				if item.amount > 1:
					item.amount -= 1
					if not wolfpack.utilities.tobackpack(item, char):
						item.update()

				else:
					item.delete() # Consume the scroll
				return True # Do nothing else.
			else:
				return False

	elif cont.hasscript( 'magic.chivalryspellbook' ):
		if spell >= 210:
			char.socket.sysmessage( "Scroll with invalid spell-id: %d" % spell )
		else:
			if not magic.chivalryspellbook.hasspell( cont, spell ):
				magic.chivalryspellbook.addspell( cont, spell )

				if item.amount > 1:
					item.amount -= 1
					if not wolfpack.utilities.tobackpack(item, char):
						item.update()

				else:
					item.delete() # Consume the scroll
				return True # Do nothing else.
			else:
				return False

	elif cont.hasscript( 'magic.necrospellbook' ):
		if spell >= 116:
			char.socket.sysmessage( "Scroll with invalid spell-id: %d" % spell )
		else:
			if not magic.necrospellbook.hasspell( cont, spell ):
				magic.necrospellbook.addspell( cont, spell )

				if item.amount > 1:
					item.amount -= 1
					if not wolfpack.utilities.tobackpack(item, char):
						item.update()

				else:
					item.delete() # Consume the scroll
				return True # Do nothing else.
			else:
				return False

	else:
		return False

	return False
