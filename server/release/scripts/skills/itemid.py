#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Viper                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.utilities import *
from wolfpack.time import *
import wolfpack
import skills

ITEMID_DELAY = 1000

def itemid(char, skill):
	socket = char.socket

	if socket.hastag('skill_delay'):
		if wolfpack.time.currenttime() < socket.gettag('skill_delay'):
			socket.clilocmessage(500118)
			return 1
		else:
			socket.deltag('skill_delay')

	socket.clilocmessage(500343)
	socket.attachtarget("skills.itemid.response")
	return 1

def response(char, args, target):
	if not char:
		return False

	if skills.skilltable[ ITEMID ][ skills.UNHIDE ] and char.hidden:
		char.reveal()

	socket = char.socket
	socket.settag('skill_delay', int( wolfpack.time.currenttime() + ITEMID_DELAY ) )

	# Identify an item and send the buy and sellprice.
	if target.item:
		item = target.item
		if not item.getoutmostchar() == char:
			top = item.getoutmostitem()
			if top.container and top.container.ischar():
				top = top.container
			if not char.canreach(top, 4):
				char.socket.clilocmessage( 0x7A27F, "", 0x3b2, 3 )
				return False

		if not char.checkskill(ITEMID, 0, 1000):
			socket.clilocmessage(500353)
			return

		# Identify the item
		if target.item.hastag('unidentified'):
			socket.sysmessage('You are able to identify the use of this item!')
			target.item.deltag('unidentified')
			target.item.resendtooltip()

		# Display the buyprice
		if target.item.buyprice != 0:
			socket.sysmessage("You could probably buy this for %u gold." % target.item.buyprice)
		else:
			socket.sysmessage("You don't think that anyone would sell this.")

		# Display the sellprice
		if target.item.sellprice != 0:
			socket.sysmessage("You could probably sell this for %u gold." % target.item.sellprice)
		else:
			socket.sysmessage("You don't think anyone would buy this.")

	elif target.char:
		if not char.canreach(target.char, 4):
			socket.clilocmessage(500344)
			return

		target.char.showname(socket)

	else:
		socket.clilocmessage(500353)

def onLoad():
	skills.register(ITEMID, itemid)
