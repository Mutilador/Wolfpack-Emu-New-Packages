
import wolfpack
from wolfpack import tr
import random
from wolfpack.gumps import cGump
from wolfpack.consts import *

def gump_response(char, args, response):
	if len(args) < 1 or response.button != 1:
		return

	key = wolfpack.finditem(args[0])

	if not char.canreach(key, 5):
		char.socket.clilocmessage(501661)
		return

	# Rename
	new_name = response.text[1][:30] # 30 Chars max.
	key.name = new_name
	char.socket.sysmessage(tr("You renamed the key to '%s'") % new_name)

	# Rekey
	if char.gm:
		new_lock = response.text[2]
		old_lock = ''
		if key.hastag('lock'):
			old_lock = str(key.gettag('lock'))

		if old_lock != new_lock:
			char.log(LOG_TRACE, tr("Changing lock of key 0x%x from '%s' to '%s'.\n") % (key.serial, old_lock, new_lock))
			if len(new_lock) != 0:
				key.settag('lock', new_lock)
				char.socket.sysmessage(tr('This key now unlocks: ') + new_lock)								
			else:
				key.deltag('lock')
				char.socket.sysmessage(tr('You erase the lock information from the key.'))

	key.resendtooltip()

def rename_key(char, key):
# Build the lock gump
	gump = cGump(x=100, y=100, callback=gump_response)
	gump.setArgs([key.serial])

	if not char.gm:
		width = 325
	else:
		width = 425

	# Renaming, blanking and modification of keys allowed for gms
	gump.addBackground(id=0x2436, width=width, height=220)

	if char.gm:
		text = tr('<basefont color="#FECECE"><h3>Manage Key</h3><br><basefont color="#FEFEFE">This dialog will help you to manage or rename this key.')
	else:
		text = tr('<basefont color="#FECECE"><h3>Manage Key</h3><br><basefont color="#FEFEFE">This dialog will help you to rename this key.')

	gump.addHtmlGump(x=20, y=20, width=410, height=90, html=text)

	gump.addText(x=20, y=65, text=tr('The name of this key:'), hue=0x835)
	gump.addResizeGump(x=20, y=88, id=0xBB8, width=200, height=25)
	gump.addInputField(x=25, y=90, width=190, height=20, hue=0x834, id=1, starttext=key.name)

	# InputField for the key id
	if char.gm:
		lock = ''
		if key.hastag('lock'):
			lock = str(key.gettag('lock'))

		gump.addText(x=235, y=65, text=tr('The lock id of this key:'), hue=0x835)
		gump.addResizeGump(x=235, y=88, id=0xBB8, width=160, height=25)
		gump.addInputField(x=240, y=90, width=150, height=20, hue=0x834, id=2, starttext=lock)


	gump.addText(x=50, y=130, text=tr('Modify key'), hue=0x835)
	gump.addButton(x=20, y=130, up=0x26af, down=0x26b1, returncode=1)

	gump.addText(x=50, y=170, text=tr('Cancel'), hue=0x835)
	gump.addButton(x=20, y=170, up=0x26af, down=0x26b1, returncode=0)

	gump.send(char)

def lock_response(char, args, target):
	if len(args) != 1:
		return

	key = wolfpack.finditem(args[0])
	if not key or not char.canreach(key,5):
		char.socket.clilocmessage(501661)
		return

	# Check for an item target.
	if not target.item or not char.canreach(target.item,5):
		char.socket.clilocmessage(501666)
		return

	if target.item == key:
		rename_key(char,key)

	elif target.item.hasscript( 'lock' ) and target.item.hastag('lock'):
		door_lock = str(target.item.gettag('lock'))
		key_lock = str(key.gettag('lock'))
			
		if door_lock == key_lock:			
			if target.item.hastag('locked') and int(target.item.gettag('locked')) == 1:
					target.item.deltag('locked')
					char.log(LOG_TRACE, tr("Accessed locked object 0x%x using key 0x%x.\n") % (target.item.serial, key.serial))
					target.item.say(1048001, "", "", False, 0x3b2, char.socket)
			else:
					target.item.settag('locked',1)
					char.log(LOG_TRACE, tr("Locked object 0x%x using key 0x%x.\n") % (target.item.serial, key.serial))
					target.item.say(1048000, "", "", False, 0x3b2, char.socket)
			char.soundeffect(0x241)
			target.item.resendtooltip()
		else:
			char.socket.clilocmessage(501668)
	else:
		char.socket.clilocmessage(501666)

	return

def copy_response(char, args, target):
	if len(args) != 1:
		return

	key = wolfpack.finditem(args[0])
	if not key or not char.canreach(key,5):
		char.socket.clilocmessage(501661)
		return

	# Check if the targetted item is a key
	if not target.item or not target.item.hasscript( 'key' ):
		char.socket.clilocmessage(501679)
		return

	# Also a blank key?
	if not target.item.hastag('lock'):
		char.socket.clilocmessage(501675)
		return

	# Check if the player can reach the item
	if not char.canreach(target.item,5):
		char.socket.clilocmessage(501661)
		return

	# Tinkering check (15%-30%, 25% chance of loosing the key on failure)
	if char.checkskill(TINKERING, 150, 300):
		key.settag('lock',target.item.gettag('lock'))
		key.resendtooltip()
		char.socket.clilocmessage(501676)
	else:
		char.socket.clilocmessage(501677)

		# 25% chance of destroying the blank key
		if random.randint(1,4) == 1:
			char.socket.clilocmessage(501678)
			key.delete()

def onUse(char, key):
	# Does this key open a lock?
	if not key.hastag('lock'):
		# The key is blank so we wan't to make a copy of it
		char.socket.clilocmessage(501663)
		char.socket.attachtarget('key.copy_response',[key.serial])
	else:
		char.socket.clilocmessage(501662)
		char.socket.attachtarget('key.lock_response',[key.serial])
	
	return 1

def onShowTooltip(char, item, tooltip):
	tooltip.reset()
	tooltip.add(0xF9060 + item.id, '')
	if item.newbie:
		tooltip.add( 1038021, "" ) # Blessed
	# The user defined name
	if len(item.name) > 0:
		tooltip.add(1060847, tr("Unlocks: ") + "\t" + item.name)

	# Add the lock id for gms
	if char.gm:
		if item.hastag('lock'):
			lock = str(item.gettag('lock'))
			tooltip.add(1050045, " \t" + tr("Lock: ") + lock + "\t ")
		else:
			tooltip.add(1050045, " \t" + tr("This key is blank") + "\t ")
