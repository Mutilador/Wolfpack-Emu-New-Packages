#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
import wolfpack
import skills
from wolfpack.time import *
from math import floor

ANATOMY_DELAY = 1000

# Button for Anatomy pressed on skill gump
def anatomy( char, skill ):
	# We only handle anatomy
	if skill != ANATOMY:
		return 0

	socket = char.socket

	if char.hastag( 'skill_delay' ):
		cur_time = servertime()
		if cur_time < char.gettag( 'skill_delay' ):
			socket.clilocmessage( 500118, "", 0x3b2, 3 )
			return 1
		else:
			char.deltag( 'skill_delay' )

	# Assign the target request
	socket.clilocmessage( 0x7A261, "", 0x3b2, 3 ) # Whom shall I examine?
	socket.attachtarget( "skills.anatomy.response" )

	return 1

def response( char, args, target ):

	socket = char.socket

	# Check for a valid target
	if not target.char:
		socket.clilocmessage( 0x7A263, "", 0x3b2, 3, char ) # Only living things have anatomies
		return

	if target.char == char:
		socket.clilocmessage( 0x7A264, "", 0x3b2, 3, char ) # You know yourself quite well enough already.
		return

	# You can't reach that (too far away, no los, wrong map)
	if not char.canreach( target.char, 16 ):
		socket.clilocmessage( 0x7A265, "", 0x3b2, 3, target.char ) # I am too far away to do that.
		return

	# Turn toward the char we want to look at
	char.turnto( target.char )
	
	# Invulnerable Characters cannot be examined
	if target.char.dead:
		socket.clilocmessage( 0x7A266, "", 0x3b2, 3, target.char ) # That cannot be inspected.
		return

	cur_time = servertime()
	char.settag( 'skill_delay', cur_time + ANATOMY_DELAY )

	# Make a skillcheck and display the fail or success message above the targets head
	if not char.checkskill( ANATOMY, 0, 1000 ):
		socket.clilocmessage( 0xFE8EA, "", 0x3b2, 3, target.char ) # You cannot quite get a sense of their physical characteristics.
		return

	# Anatomy Messages start: 0xFD6DD
	strId = min( 10, floor( target.char.strength / 10 ) )
	dexId = min( 10, floor( target.char.dexterity / 10 ) )
	msgId = int( 0xFD6DD + strId * 11 + dexId )
	dexRatio = float( 100.0 / target.char.dexterity )
	StamId = floor( ( target.char.stamina * dexRatio ) / 10 )
	msgId2 = int( 0xFD7DF + StamId )

	socket.clilocmessage( msgId, "", 0x3b2, 3, target.char )
	socket.clilocmessage( msgId2, "", 0x3b2, 3, target.char )
	
# Register as a global script
def onLoad():
	skills.register( ANATOMY, anatomy )
