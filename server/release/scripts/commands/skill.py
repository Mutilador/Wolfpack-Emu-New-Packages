#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: codex                          #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# skill Command 						#
#===============================================================#

"""
	\command skill
	\usage - <code>skill name value</code>
	Name is the name of the skill you want to set.
	Value is the desired integer value. Please note that the value
	is the real skill value multiplied by 10 (i.e. 100.0% = 1000).
	\description Change the skills of a character.
"""

"""
	\command allskills
	\description Sets all skills of your character.
	\usage - <code>allskills [value]</code>
	Value is the value all skills should be set to. It's multiplied by 10 (100.0% = 1000).
"""

import wolfpack
import string
from wolfpack.consts import YELLOW, NORMAL, skillnamesids, SKILLNAMES, ALLSKILLS

usage =   'Usage: skill <skill-name> <value*10>'
example = 'Example: skill mining 1000'

def skill( socket, command, args ):
	args = args.strip()   # Remove trailing and leading whitespaces
	if len( args ) > 0:
		try:
			( skill, value ) = args.split( ' ' )
			skill = str( skill )
			value = int( value )
		except:
			socket.sysmessage( usage )
			socket.sysmessage( example )
			return True

	else:
		socket.sysmessage( usage )
		socket.sysmessage( example )
		return True

	# Uknown skill name ?
	if not skill in skillnamesids:
		socket.clilocmessage( 500156, "", YELLOW, NORMAL ) # Invalid skill
		return True

	# Accept 0 >= value <= 1000 skill value only. 1200 maximum - with power scrolls only
	elif value < 0 or value > 1000:
		socket.clilocmessage( 1005632, "", YELLOW, NORMAL ) # Skill values range from 0 - 1000.
		return True

	socket.clilocmessage( 503403, "", YELLOW, NORMAL ) # Select the body.
	socket.attachtarget( "commands.skill.callback", [ skill, value ] )

	return True

def callback( char, args, target ):
	socket = char.socket

	# What are you targeted ?
	if not target.char:
		socket.clilocmessage( 500931, "", YELLOW, NORMAL ) # Invalid mobile
		return True

	# check for rank
	if target.char.rank > char.rank and not char == target.char:
		socket.clilocmessage( 1005213, "", YELLOW, NORMAL ) # You can't do that
		return False

	( skill, value ) = args
	target.char.skill[ skillnamesids[ skill.lower() ] ] = value

	return True

def allskills( socket, command, args ):
	args = args.strip()   # Remove trailing and leading whitespaces
	char = socket.player
	try:
		if len( args ) > 0:
			value = int( args )
			for skillid in range( 0, ALLSKILLS ):
				char.skill[ skillid ] = value
			char.update()
			return True
		else:
			socket.sysmessage( "Usage: allskills [value]" )
			return False
	except:
		socket.sysmessage( "Usage: allskills [value]" )
		return False

def onLoad():
	wolfpack.registercommand( "skill", skill )
	wolfpack.registercommand( "allskills", allskills )
	return
