
"""
	\command resurrect
	\description Resurrects a character.
	\notes If the character is standing on his corpse he will regain
	all his posessions and equipment.
"""

import wolfpack
from wolfpack.consts import LOG_MESSAGE

def resurrect( socket, command, arguments ):
	socket.sysmessage( "Whom do you wish to resurrect?" )
	socket.attachtarget( "commands.resurrect.response", [] )
	return

def response( char, args, target ):
	if not target.char:
		char.socket.clilocmessage(503348)
		return False

	if not target.char.dead:
		char.socket.clilocmessage(501041)
		return False

	if target.char and target.char.dead:
		if target.char.serial != char.serial:
			char.turnto( target.char )
		target.char.soundeffect( 0x215 )
		target.char.resurrect( char )
		char.log( LOG_MESSAGE, "Resurrects character 0x%x.\n" % target.char.serial )
		# Restore the character as well, added bonus.
		target.char.hitpoints = target.char.maxhitpoints
		target.char.mana = target.char.maxmana
		target.char.stamina = target.char.maxstamina
		target.char.update()
		return True

	else:
		char.socket.sysmessage( 'This object can not be resurrected!', GRAY )
		return False
	return True

def onLoad():
	wolfpack.registercommand( "resurrect", resurrect )
	return
