﻿"""
	\command disconnect
	\description Disconnect the client of the chosen char.
	\usage - <code>disconnect</code>
"""

import wolfpack

def disconnect( socket, command, argstring):
	socket.sysmessage( "Which client do you want to disconnect?" )
	socket.attachtarget( "commands.disconnect.disconnecttarget", [] )

def disconnecttarget( char, args, target ):
	if not target.char or target.char.npc:
		char.socket.sysmessage( "That is not a valid character." )
	if target.char:
		if target.char.socket:
			target.char.socket.disconnect()
		target.char.logouttime = 1
		target.char.removefromview()
	return 1

def onLoad():
	wolfpack.registercommand( "disconnect", disconnect )
