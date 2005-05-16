
import wolfpack
from wolfpack.consts import LOG_ERROR, EVENT_CASTSPELL
from magic.spell import *
from magic.utilities import *
import time

# Spell Registry
spells = {}

# Register spell information in the global registry
def registerspell( id, spell ):
	global spells

	if spells.has_key( id ):
		return

	spells[ id ] = spell

#
# Set up our global hook on script load
#
def onLoad():
	wolfpack.registerglobal(EVENT_CASTSPELL, "magic")

#
# A Spell has been Selected from our SpellBook
# This part of the code has to be npc safe!
#
# Mode: 0 = Book
# Mode: 1 = Scroll
# Mode: 2 = Wand
# Mode: 3 = Command
def castSpell( char, spell, mode = 0, args = [], target = None, item = None ):
	if char.dead:
		return

	socket = char.socket

	if not spells.has_key(spell):
		if socket:
			socket.log(LOG_ERROR, "Trying to cast unknown spell: %d.\n" % spell)
			socket.sysmessage('ERROR: Unknown Spell')
		else:
			npc_debug(char, "Trying to cast unknown spell: %d." % spell)
		return

	# Deny casting if spell delay hasn't yet expired
	if socket and socket.hastag('spell_delay'):
		spell_delay = float(socket.gettag('spell_delay'))
		if spell_delay > time.time():
			socket.clilocmessage(502644)
			return False
		else:
			socket.deltag('spell_delay')

	spells[spell].precast(char, mode, args, target, item)
	return True

# Target Cancel
def target_cancel(char):
	char.socket.deltag('cast_target')
	char.dispel(char, 1, 'cast_delay')
	char.removescript('magic')

# Target Timeout
def target_timeout(char):
	if char.socket.hastag('cast_target'):
		char.socket.clilocmessage(500641)
		fizzle(char)

# Target Response
def target_response( char, args, target ):
	if char.socket:
		char.socket.deltag('cast_target')

	spell = args[0]
	mode = args[1]
	item = args[3]
	args = args[2]
	if type(item) == int:
		item = wolfpack.findobject(item)
		# Object went out of scope
		if not item:
			fizzle(char)
			return

	# Char Targets
	if target.char and (spell.validtarget == TARGET_IGNORE or spell.validtarget == TARGET_CHAR):
		if target.char.invulnerable and spell.harmful:
			if char.socket:
				char.socket.clilocmessage(1061621)
			return False
		if target.char.dead and not spell.affectdead:
			if char.socket:
				char.socket.clilocmessage(501857)
			return False
		if not char.cansee(target.char):
			if char.socket:
				char.socket.clilocmessage(500237)
			return False
		if char.distanceto(target.char) > spell.range:
			if char.socket:
				char.socket.clilocmessage(500446)
			return False
		if not char.canreach(target.char, spell.range):
			if char.socket:
				char.socket.clilocmessage(500237)
			return False

		if type(spell.spellid) == int:
			message = "Casting spell %u (%s) on character %s (0x%x).\n"  % (spell.spellid, spell.__class__.__name__, target.char.name, target.char.serial)
		else:
			message = "Casting spell %s on character %s (0x%x).\n"  % (spell.__class__.__name__, target.char.name, target.char.serial)
		char.log(LOG_MESSAGE, message)
		spell.target(char, mode, TARGET_CHAR, target.char, args, item)

	# Item Target
	elif target.item and ( spell.validtarget == TARGET_IGNORE or spell.validtarget == TARGET_ITEM ):
		if not char.cansee(target.item):
			if char.socket:
				char.socket.clilocmessage(500237)
			return False
		if not char.canreach(target.item, spell.range):
			if char.socket:
				char.socket.clilocmessage(500237)
			return False

		message = u"Casting spell %u (%s) on item %s (0x%x).\n"  % (spell.spellid, spell.__class__.__name__, target.item.getname(), target.item.serial)
		char.log(LOG_MESSAGE, message)
		spell.target(char, mode, TARGET_ITEM, target.item, args, item)

	# Ground Target
	elif (target.item or target.char or target.pos) and (spell.validtarget == TARGET_IGNORE or spell.validtarget == TARGET_GROUND):
		pos = target.pos
		if target.item:
			item = target.item.getoutmostitem()
			if not item.container:
				pos = item.pos
			else:
				pos = item.container.pos
		elif target.char:
			pos = target.char.pos

		# See if the target is accesible
		if char.distanceto(pos) > spell.range:
			if char.socket:
				char.socket.clilocmessage(500446)
			return False
		if not char.canreach(pos, spell.range, target.model):
			if char.socket:
				char.socket.clilocmessage(500237)
			return False

		if type(spell.spellid) == int:
			message = "Casting spell %u (%s) on coordinate %s.\n"  % (spell.spellid, spell.__class__.__name__, str(pos))
		else:
			message = "Casting spell %s on coordinate %s.\n"  % (spell.__class__.__name__, str(pos))
		char.log(LOG_MESSAGE, message)
		spell.target(char, mode, TARGET_GROUND, pos, args, item)

	else:
		char.socket.clilocmessage(501857)
		return False

def onCastSpell(char, spell, mode = 0, args = [], target = None, item = None):
	castSpell(char, spell, mode, args, target, item)

# These Events happen for characters	who are casting a spell right now
def onDamage(char, type, amount, source):
	# You cannot be disturbed while using protection
	if char.propertyflags & 0x20000:
		return amount

	if char.socket:
		char.socket.clilocmessage(500641)
	fizzle(char)
	return amount

def maymove(char, direction, sequence):
	if char.npc or char.gm:
		return True

	return False

def onWalk( char, direction, sequence ):
	if maymove(char, direction, sequence):
		return False

	# Disallow movement for players
	packet = wolfpack.packet(0x21, 8)
	packet.setbyte(1, sequence)
	packet.setshort(2, char.pos.x)
	packet.setshort(4, char.pos.y)
	packet.setbyte(6, char.direction)
	packet.setbyte(7, char.pos.z)
	packet.send(char.socket)
	char.socket.walksequence = 0
	return True

	running = direction & 0x80
	direction &= 0x7F

	# Just turning
	if direction != char.direction:
		return

	char.socket.clilocmessage(500641)
	fizzle(char)

def onWarModeToggle(char, warmode):
	char.socket.clilocmessage(500641)
	fizzle(char)

def onLogin(char):
	fizzle(char)
	return False
