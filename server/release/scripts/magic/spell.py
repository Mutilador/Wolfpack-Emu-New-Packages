
import magic
import random
import wolfpack.console
from wolfpack import properties
import wolfpack.utilities
from magic.utilities import *
import magic.utilities
from wolfpack.consts import *
import time
import arcanegem

# Recursive Function for counting reagents
def countReagents(item, items):
	for key in items.keys():
		if( key == item.baseid ):
			items[ key ] = max(0, items[ key ] - item.amount)
			return items # Reagents normally dont have content

	for subitem in item.content:
		items = countReagents(subitem, items)

	return items


# Recursive Function for removing reagents
def consumeReagents(item, items):
	for (key, value) in items.items():
		if( key == item.baseid ):
			if( item.amount <= value ):
				items[ key ] -= item.amount
				item.delete()
			else:
				item.amount -= value
				item.update()
				del items[key]
			return items # Reagents normally dont have content

	for subitem in item.content:
		items = consumeReagents(subitem, items)
	return items


def callback(char, args):
	char.removescript('magic')

	# target
	if args[3] and type(args[3]) is int:
		target = wolfpack.findobject(args[3])

		# Object went out of scope
		if not target:
			fizzle(char)
			return
	else:
		target = args[3]

	# item
	if args[4] and (type(args[4]) is int):
		item = wolfpack.findobject(args[4])

		# Object went out of scope
		if not item:
			fizzle(char)
			return
	else:
		item = None

	args[0].cast(char, args[1], args[2], target, item)

# Basic Spell Class
class Spell:
	# We affect another character
	def affectchar(self, char, mode, target, args=[], item=None):
		return True

	def register(self, id):
		self.spellid = id
		magic.registerspell(self.spellid, self)

	def __init__(self, circle):
		# Set Mana
		self.mana = 0
		self.reagents = {}
		self.tithingpoints = 0
		self.validtarget = TARGET_IGNORE
		self.spellid = None
		self.mantra = None
		self.harmful = 0
		self.affectdead = 0
		self.resistable = 0
		self.range = 12
		self.skill = MAGERY
		self.damageskill = EVALUATINGINTEL
		self.circle = circle
		self.reflectable = 0
		self.inherent = 0
		mana_table = [ 4, 6, 9, 11, 14, 20, 40, 50 ]
		self.mana = mana_table[ self.circle - 1 ]
		self.castaction = ANIM_CASTDIRECTED
		self.castspeed = circle

		# Change this to 0 for AoS behaviour
		self.castrecovery = int(0.5 * circle)

	#
	# Show the cast action
	#
	def docastaction(self, char, mode):
		if char.bodytype == BODY_HUMAN and not char.itemonlayer( LAYER_MOUNT ):
			char.action(self.castaction)

	#
	# Say the mantra
	#
	def saymantra(self, char, mode):
		if char.npc and char.bodytype != BODY_HUMAN:
			return

		if self.mantra and mode in [MODE_BOOK, MODE_SCROLL, MODE_CMD]:
			char.say(self.mantra)

	#
	# Prepare the casting of this spell.
	#
	def precast(self, char, mode=0, args=[], target = None, item = None):
		socket = char.socket

		# Casting from a scroll and no scroll was passed
		if mode == MODE_SCROLL and not item:
			return

		# Casting from a wand and no wand was passed
		if mode == MODE_WAND and not item:
			return

		# We are frozen
		if char.frozen:
			if socket:
				socket.clilocmessage(502643)
			else:
				npc_debug(char, 'Trying to cast while being frozen.')
			return False

		# We are already casting a spell
		if char.hasscript('magic') or (socket and socket.hastag('cast_target')):
			if socket:
				socket.clilocmessage(502642)
			else:
				npc_debug(char, 'Trying to cast spell while already casting a spell.')
			return False

		# If we are using a spellbook to cast, check if we do have
		# the spell in our spellbook (0-based index)
		if not self.inherent and mode == MODE_BOOK and not magic.utilities.hasSpell(char, self.spellid - 1, False):
			return False

		if not self.checkrequirements(char, mode, args, target, item):
			return False

		# Unhide the Caster
		char.reveal()

		# Say the mantra
		self.saymantra(char, mode)

		# Precasting
		char.addscript('magic')

		# Show the cast action
		self.docastaction(char, mode)

		if item:
			item = item.serial

		if target and (type(target).__name__ == 'wpchar' or type(target).__name__ == 'wpitem'):
			target = target.serial

		if mode == MODE_BOOK:
			source = 'book'
		elif mode == MODE_SCROLL:
			source = 'scroll (0x%x)' % item
		elif mode == MODE_WAND:
			source = 'wand (0x%x)' % item
		elif mode == MODE_CMD:
			source = 'command'

		if type(self.spellid) == int:
			char.log(LOG_MESSAGE, "Casting spell %u (%s) from %s.\n" % (self.spellid, self.__class__.__name__, source))
		else:
			char.log(LOG_MESSAGE, "Casting spell %s from %s.\n" % (self.__class__.__name__, source))

		if target and type(target).__name__ == 'wpitem':
			target = target.serial
		elif target and type(target).__name__ == 'wpchar':
			target = target.serial

		char.addtimer(self.calcdelay(char, mode), callback, [self, mode, args, target, item], 0, 0, "cast_delay")
		return True

	#
	# Scale Mana
	#
	def scalemana(self, char, mode, mana):
		if mode == MODE_SCROLL:
			mana = (mana + 1) / 2

		percent = properties.fromchar(char, LOWERMANACOST) / 100.0
		return max(0, mana - int(percent * float(mana)))

	#
	# Calculate the time required to cast this spell
	#
	def calcdelay(self, char, mode):
		# Wands and commands don't have a delay
		if mode == MODE_WAND or mode == MODE_CMD:
			return False

		# Get the AOS bonus from all items the character wears
		castspeed = 3

		# Under the influence of the protection spell
		# spells are cast more slowly
		if char.propertyflags & 0x20000:
			castspeed += 2

		# Cast Speed Bonus is capped at 2
		castspeed -= min(2, properties.fromchar(char, CASTSPEEDBONUS))

		castspeed += self.castspeed

		if castspeed < 1:
			castspeed = 1

		# Was: floor((delay / 4.0) * 1000)
		#char.message(str(castspeed / 4.0))
		return castspeed * 250

	def checkweapon(self, char):
		weapon = char.itemonlayer(LAYER_RIGHTHAND)

		if not weapon or not properties.itemcheck(weapon, ITEM_WEAPON):
			weapon = char.itemonlayer(LAYER_LEFTHAND)

		if not weapon:
			return True

		# Check if the spellchanneling property is true for this item
		if not properties.fromitem(weapon, SPELLCHANNELING):
			if not wolfpack.utilities.tobackpack(weapon, char):
				weapon.update()

			return True
		else:
			return True

	#
	# Set the delay for the next spell
	#
	def setspelldelay(self, char, mode):
		if char.npc:
			pass
		elif char.socket:
			castrecovery = - properties.fromchar(char, CASTRECOVERYBONUS)
			castrecovery += 6
			castrecovery += self.castrecovery

			if castrecovery < 0:
				castrecovery = 0

			delay = float(castrecovery) / 4.0
			#char.message(str(delay))

			char.socket.settag('spell_delay', time.time() + delay)

	def checkrequirements(self, char, mode, args=[], target=None, item=None):
		if char.dead:
			return False

		if char.gm:
			return True

		if mode == MODE_BOOK:
			if not self.checkweapon(char):
				char.message(502626)
				return False

			# Check for Mana
			if char.mana < self.mana:
				char.message(502625)
				return False

			if not self.checkreagents(char, mode, args):
				return False
			
			if not self.checktithingpoints(char, mode, args):
				return False

		elif mode == MODE_SCROLL:
			if not self.checkweapon(char):
				char.message(502626)
				return False

			# Check if the scroll is allright
			if not item or item.getoutmostchar() != char:
				char.message(501625)
				return False

			# Check for Mana
			if char.mana < (self.mana + 1) / 2:
				char.message(502625)
				return False

		elif mode == MODE_WAND:
			# Check if the wand is allright
			if not item or item.getoutmostchar() != char:
				return False

		return True

	# Check for Reagents
	def checkreagents(self, char, mode, args=[]):
		if not char.npc and len(self.reagents) > 0:
			items = countReagents(char.getbackpack(), self.reagents.copy())

			for item in items.keys():
				if items[item] > 0:
					if arcanegem.ConsumeCharges( char, items[item] ):
						return True
					char.message(502630)
					return False

		return True

	# Check for Tithing Points
	def checktithingpoints(self, char, mode, args=[]):
		if not char.npc and self.tithingpoints > 0:
			points = char.gettag('tithing_points')
			if points and points < self.tithingpoints:
				char.socket.clilocmessage(1060173, self.tithingpoints ) # You must have at least ~1_TITHE_REQUIREMENT~ Tithing Points to use this ability,

	# Consume Reagents
	def consumereagents(self, char, mode, args=[]):
		if not char.npc and len(self.reagents) > 0:
			lowerreagentcost = properties.fromchar(char, LOWERREAGENTCOST)

			if lowerreagentcost == 0 or lowerreagentcost < random.randint(0, 99):
				consumeReagents(char.getbackpack(), self.reagents.copy())

	# Consume Tithing Points (Paladins)
	def consumetithingpoints(self, char, mode, args=[]):
		if not char.npc and self.tithingpoints > 0:
			points = char.gettag('tithing_points')
			char.settag('tithing_points', points - self.tithingpoints)

	def consumerequirements(self, char, mode, args=[], target=None, item=None):
		if char.gm:
			return True

		# Check Basic Requirements before proceeding (Includes Death of Caster etc.)
		if not self.checkrequirements(char, mode, args, target, item):
			fizzle(char)
			return False

		# Check Skill
		if self.skill != None:
			if mode == MODE_BOOK:
				circle = self.circle - 1
			else:
				circle = self.circle - 3
			minskill = max(0, int((1000 / 7) * circle - 200))
			maxskill = min(1200, int((1000 / 7) * circle + 200))

			if not char.checkskill(self.skill, minskill, maxskill):
				char.message(502632)
				fizzle(char)
				return False

		# Consume Mana
		if mode == MODE_BOOK:
			if self.mana != 0:
				mana = self.scalemana(char, MODE_BOOK, self.mana)
				char.mana = max(0, char.mana - mana)
				char.updatemana()

			self.consumereagents(char, mode, args)
			self.consumetithinpoints(char, mode, args)

		# Reduced Skill, Reduced Mana, No Reagents
		elif mode == MODE_SCROLL:
			if self.mana != 0:
				mana = self.scalemana(char, MODE_SCROLL, self.mana)
				char.mana = max(0, char.mana - mana)
				char.updatemana()

			# Remove one of the scrolls
			if item.amount == 1:
				item.delete()
			else:
				item.amount -= 1
				item.update()

		# No requirements at all
		elif mode == MODE_WAND:
			pass

		# Set the next spell delay
		self.setspelldelay(char, mode)

		return True

	# Not implemented yet
	def checkreflect(self, char, mode, targettype, target, args, item):
		return False

	#
	# bonus Fixed bonus to the throw.
	# dice The number of dices to roll.
	# sides How many sides has each dice.
	#
	def scaledamage(self, char, target, bonus, dice, sides):
		damage = rolldice(dice, sides, bonus) * 100.0

		bonus = char.skill[INSCRIPTION] / 100.0
		bonus += char.intelligence / 10.0
		spelldamagebonus = properties.fromchar(char, SPELLDAMAGEBONUS)

		# Spell Damage Bonus is capped at 15% against players
		if target and target.player and spelldamagebonus > 15:
			spelldamagebonus = 15

		bonus += spelldamagebonus

		damage *= 1.0 + bonus / 100.0

		char.checkskill(self.damageskill, 0, 1200)
		damage *= (30 + (9 * char.skill[self.damageskill]) / 100.0) / 100.0

		return max(1, int(damage / 100.0))

	#
	# Calculate the chance the given target has to resist
	# this spell.
	#
	def resistchance(self, char, target):
		basechance = target.skill[MAGICRESISTANCE] / 50.0
		evalchance = target.skill[MAGICRESISTANCE] / 10.0 - (((char.skill[self.skill] - 200) / 50.0) + self.circle * 5.0)
		if basechance > evalchance:
			return basechance / 2.0
		else:
			return evalchance / 2.0

	#
	# Check if the target is successful in resisting this spell
	# cast by char.
	#
	def checkresist(self, char, target):
		chance = self.resistchance(char, target)
		chance /= 100.0

		# No chance to resist
		if chance <= 0.0:
			return False

		if chance >= 1.0:
			return True

		maxskill = self.circle * 100
		maxskill += (1 + ((self.circle - 1) / 6)) * 250

		if target.skill[MAGICRESISTANCE] < maxskill:
			target.checkskill(MAGICRESISTANCE, 0, 1200)

		return chance >= random.random()

	def cast(self, char, mode, args=[], target=None, item=None):
		if not target:
			if char.socket:
				char.socket.settag('cast_target', 1)
				if item:
					item = item.serial
				char.socket.attachtarget('magic.target_response', [ self, mode, args, item ], 'magic.target_cancel', 'magic.target_timeout', 30000) # Don't forget the timeout later on
			else:
				# Callback to the NPC AI ??
				wolfpack.console.log(LOG_ERROR, "A NPC is trying to cast a spell.")
		# A target has been supplied
		else:
			targetwrapper = TargetWrapper(target)
			magic.target_response(char, [self, mode, args, item], targetwrapper)

	def target(self, char, mode, targettype, target, args, item):
		raise Exception, "Spell without target method: " + str(self.__class__.__name__)

	#
	# Call this if you harm another character directly
	#
	def harmchar(self, char, victim):
		# Let the victim defend himself, if he isn't fighting yet
		if not( victim.attacktarget ):
			victim.fight(char)

class TargetWrapper:
	def __init__(self, obj):
		if type(obj).__name__ == 'wpitem':
			self.item = obj
			self.char = None
			self.pos = obj.pos
			self.model = obj.id
		elif type(obj).__name__ == 'wpchar':
			self.item = None
			self.char = obj
			self.pos = obj.pos
			self.model = 0
		else:
			self.item = None
			self.char = None
			self.pos = obj
			self.model = 0

class CharEffectSpell (Spell):
	def __init__(self, circle):
		Spell.__init__(self, circle)
		self.validtarget = TARGET_CHAR
		self.resistable = 1 # Most of them are resistable

	def effect(self, char, target, mode, args, item):
		raise Exception, "CharEffectSpell with unimplemented effect method: " + str(self.__clas__.__name__)

	def target(self, char, mode, targettype, target, args, item):
		if not self.consumerequirements(char, mode, args, target, item):
			return

		if not self.affectchar(char, mode, target, item):
			return

		char.turnto(target)

		if self.reflectable and self.checkreflect(char, mode, targettype, target, args, item):
			target = char

		if self.harmful:
			self.harmchar(char, target)

		self.effect(char, target, mode, args, item)

#
# A damage spell. It can be delayed but doesnt need
# to be.
#
class DelayedDamageSpell(CharEffectSpell):
	def __init__(self, circle):
		CharEffectSpell.__init__(self, circle)
		self.validtarget = TARGET_CHAR
		self.harmful = 1 # All of them are harmful
		self.missile = None # If set, a missile will be shot [ id, fixed-direction, explode, speed ]
		self.delay = 1000 # Default Delay
		self.sound = None

	def effect(self, char, target, mode, args, item):
		# Shoot a missile?
		if self.missile:
			char.movingeffect(self.missile[0], target, self.missile[1], self.missile[2], self.missile[3])

		if self.sound:
			char.soundeffect(self.sound)

		# The damage will be dealt in one second
		if not self.delay:
			self.damage(char, target)
		else:
			target.addtimer(self.delay, damage_callback, [ self.spellid, char.serial ], 0, 0)

#
# Callback for delayed damage spells
#
def damage_callback(target, args):
	spell = magic.spells[ args[0] ]
	char = wolfpack.findchar(args[1])

	# Something went out of scope
	if not char or not spell:
		return

	spell.damage(char, target)
