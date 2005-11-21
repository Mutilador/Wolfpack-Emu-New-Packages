#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
from wolfpack.consts import ITEMID
from wolfpack import tr

def onUse( char, item ):
	if( not char.socket ):
		return 0

	if( char.distanceto( item ) > 5 ):
		char.socket.clilocmessage( 500295 ) # You are too far away to do that.
		return 1

	if( char.checkskill( ITEMID, 0, 100 ) ):
		# Not animated
		if( item.id == 0x1230 ):
			item.soundeffect( 0x56 )
			item.id = 0x1245
		elif( item.id == 0x1245 ):
			item.id = 0x1230
		else:
			char.socket.sysmessage( tr("Wrong item id") )
			return 1

		item.update()

	return 1
