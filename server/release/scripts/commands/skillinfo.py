
"""
	\command skillinfo
	\description Show a dialog to modify the skill values and caps of a character.
"""
import wolfpack
import wolfpack.gumps
from wolfpack.utilities import tobackpack
from wolfpack.consts import *
from math import ceil, floor
from commands.info import iteminfo

#
# Gump response
#
def response(player, arguments, response):
	if response.button == 0:
		return

	target = wolfpack.findchar(arguments[0])

	if not target:
		return False

	# Switch Page
	if response.button & 0x1000 != 0:
		page = response.button & 0xFFF
		showgump(player, target, page)
		return

	# Iterate trough all skills and see what changed
	for skill in range( 0, ALLSKILLS ):
		if not response.text.has_key(0x1000 | skill) or not response.text.has_key(0x2000 | skill):
			continue

		try:
			newvalue = int(floor(float(response.text[0x1000 | skill]) * 10))
			newcap = int(floor(float(response.text[0x2000 | skill]) * 10))
		except:
			player.socket.sysmessage('You have entered invalid values for %s.' % SKILLNAMES[skill])
			return False

		oldvalue = target.skill[skill]
		oldcap = target.skillcap[skill]

		if oldvalue != newvalue or oldcap != newcap:
			message = "Changed %s for character 0x%x to value %u [%d] and cap %u [%d].\n"
			message = message % (SKILLNAMES[skill], target.serial, newvalue, newvalue - oldvalue, newcap, newcap - oldcap)
			player.log(LOG_MESSAGE, message)
			player.socket.sysmessage(message)

			target.skill[skill] = newvalue
			target.skillcap[skill] = newcap

def showgump(player, target, page):
	# 80 pixel diameter
	pages = int(ceil(ALLSKILLS / 5.0))

	if page > pages:
		return

	dialog = wolfpack.gumps.cGump()
	dialog.setCallback(response)
	dialog.setArgs([target.serial])

	dialog.startPage(0)
	dialog.addResizeGump(35, 12, 9260, 460, 504)
	dialog.addGump(1, 12, 10421, 0)
	dialog.addGump(30, -1, 10420, 0)
	dialog.addResizeGump(66, 40, 9200, 405, 65)
	dialog.addText(108, 52, unicode("Wolfpack Skillinfo Command"), 2100)
	dialog.addTiledGump(90, 11, 164, 17, 10250, 0)
	dialog.addGump(474, 12, 10431, 0)
	dialog.addGump(439, -1, 10430, 0)
	dialog.addGump(14, 200, 10422, 0)
	dialog.addGump(468, 200, 10432, 0)
	dialog.addGump(249, 11, 10254, 0)
	dialog.addGump(74, 45, 10464, 0)
	dialog.addGump(435, 45, 10464, 0)
	dialog.addGump(461, 408, 10412, 0)
	dialog.addGump(-15, 408, 10402, 0)
	dialog.addTiledGump(281, 11, 158, 17, 10250, 0)
	dialog.addGump(265, 11, 10252, 0)
	dialog.addButton(60, 476, 247, 248, 1)
	dialog.addButton(136, 476, 242, 241, 0)

	if page > 0:
		dialog.addButton(60, 444, 9909, 9911, 0x1000 | page - 1)
		dialog.addText(88, 444, unicode("Previous Page"), 2100)

	if page + 1 < pages:
		dialog.addButton(448, 444, 9903, 9905, 0x1000 | page + 1)
		dialog.addText(376, 448, unicode("Next Page"), 2100)

	yoffset = 0
	for i in range(0, 5):
		skill = int((page * 5) + i)

		if skill >= ALLSKILLS:
			break

		skillname = SKILLNAMES[skill]
		skillname = str( skillname[0].upper() + skillname[1:] )
		skillvalue = ( target.skill[skill] / 10.0 )
		skillcapvalue = ( target.skillcap[skill] / 10.0 )

		dialog.addResizeGump(65, 109 + yoffset, 9200, 405, 62)
		dialog.addText(76, 115 + yoffset, unicode( "Skill: %s (%i)" % (skillname, skill) ), 2100)
		dialog.addResizeGump(123, 135 + yoffset, 9300, 63, 26)
		dialog.addText(76, 137 + yoffset, unicode( "Value:" ), 2100)
		dialog.addText(187, 138 + yoffset, unicode( "%" ), 2100)
		dialog.addInputField(128, 138 + yoffset, 50, 20, 2100, 0x1000 | skill, unicode( "%0.1f" % skillvalue ) )
		dialog.addText(232, 138 + yoffset, unicode( "Cap:" ), 2100)
		dialog.addText(329, 139 + yoffset, unicode( "%" ), 2100)
		dialog.addResizeGump(264, 135 + yoffset, 9300, 63, 26)
		dialog.addInputField(268, 139 + yoffset, 53, 20, 2100, 0x2000 | skill, unicode( "%0.1f" % skillcapvalue ) )

		yoffset += 65

	dialog.send(player)

#
# Show the skillinfo gump
#
def callback(player, arguments, target):
	if not target.char:
		return

	showgump(player, target.char, 0)
	return

#
# Show the skillinfo target
#
def edit(socket, command, arguments):
	socket.sysmessage('Please select a character whose skills you want to view.')
	socket.attachtarget('commands.skillinfo.callback', [])
	return

#
# Skillinfo debugging command
#
def onLoad():
	wolfpack.registercommand('skillinfo', edit)
	return
