
from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from magic.utilities import *
import random
import wolfpack
from wolfpack.utilities import tobackpack, energydamage, mayAreaHarm
from math import ceil
from magic import polymorph

class ChainLightning (Spell):
	def __init__(self):
		Spell.__init__(self, 7)
		self.reagents = {REAGENT_BLACKPEARL: 1, REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'Vas Ort Grav'
		self.validtarget = TARGET_GROUND

	def target(self, char, mode, targettype, target, args, item):
		if not self.consumerequirements(char, mode, args, target, item):
			return

		char.turnto(target)

		targets = []
		damage = self.scaledamage(char, None, 48, 1, 5)

		# Enumerate chars
		chars = wolfpack.chars(target.x, target.y, char.pos.map, 2)
		for target in chars:
			if mayAreaHarm(char, target):
				targets.append(target)

		# Re-scale the damage
		if len(targets) > 1:
			damage = ceil((damage * 2.0) / len(targets))

		for target in targets:
			target.lightning()
			target.soundeffect(0x29)

			self.harmchar(char, target)
			energydamage(target, char, damage, energy=100)

class EnergyField(Spell):
	def __init__(self):
		Spell.__init__(self, 7)
		self.reagents = {REAGENT_BLACKPEARL: 1, REAGENT_MANDRAKE: 1, REAGENT_SPIDERSILK: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'In Sanct Grav'
		self.validtarget = TARGET_GROUND

	def target(self, char, mode, targettype, target, args, item):
		char.turnto(target)

		if not self.consumerequirements(char, mode, args, target, item):
			return

		xdiff = abs(target.x - char.pos.x)
		ydiff = abs(target.y - char.pos.y)

		positions = []

		# West / East
		if xdiff > ydiff:
			itemid = 0x3956
			for i in range(-2, 3):
				positions.append(wolfpack.coord(target.x, target.y + i, target.z, target.map))

		# North South
		else:
			itemid = 0x3946
			for i in range(-2, 3):
				positions.append(wolfpack.coord(target.x + i, target.y, target.z, target.map))

		serials = []

		char.soundeffect(0x20b)

		for pos in positions:
			newitem = wolfpack.newitem(1)
			newitem.id = itemid
			newitem.moveto(pos)
			newitem.decay = 0
			#newitem.direction = 29
			newitem.settag('dispellable_field', 1)
			newitem.update()
			serials.append(newitem.serial)

		duration = int((15 + (char.skill[MAGERY] / 5)) / 0.007)
		wolfpack.addtimer(duration, field_expire, serials, 1)

class FlameStrike (DelayedDamageSpell):
	def __init__(self):
		DelayedDamageSpell.__init__(self, 7)
		self.reagents = {REAGENT_SPIDERSILK: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'Kal Vas Flam'
		self.reflectable = 1

	def damage(self, char, target):
		target.effect(0x3709, 10, 30)
		target.soundeffect(0x208)
		damage = self.scaledamage(char, target, 48, 1, 5)
		energydamage(target, char, damage, fire=100)

class GateTravel (Spell):
	def __init__(self):
		Spell.__init__(self, 7)
		self.reagents = {REAGENT_BLACKPEARL: 1, REAGENT_MANDRAKE: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'Vas Rel Por'
		self.validtarget = TARGET_ITEM

	def cast(self, char, mode, args=[], target=None, item=None):
		if not char.gm:
			if char.iscriminal():
				if char.socket:
					char.socket.clilocmessage(1005561)
				return

			if char.attacktarget:
				if char.socket:
					char.socket.clilocmessage(1005564)
				return

		return Spell.cast(self, char, mode, args, target, item)

	def target(self, char, mode, targettype, target, args, item):
		char.turnto(target)

		# Runebook recall support
		runebook = magic.runebook.isRunebook(target) and len(args) == 1		

		# We can only recall from recall runes or via the runebook
		if not runebook:
			if not target.hasscript('magic.rune'):
				char.message(502357)
				return
	
			if not target.hastag('marked') or target.gettag('marked') != 1:
				char.message(502354)
				return

			location = target.gettag('location')
			location = location.split(",")
			location = wolfpack.coord(int(location[0]), int(location[1]), int(location[2]), int(location[3]))
			char.log(LOG_MESSAGE, 'Tries to gate to %s using rune.\n' % str(location))
		else:
			location = args[0]
			char.log(LOG_MESSAGE, 'Tries to gate to %s using runebook.\n' % str(location))

		if not self.consumerequirements(char, mode, args, target, item):
			return

		region = wolfpack.region(char.pos.x, char.pos.y, char.pos.map)

		if region.nogate:
			char.message(501802)
			fizzle(char)
			return

		region = wolfpack.region(location.x, location.y, location.map)

		if not location.validspawnspot():
			char.message(501942)
			fizzle(char)
			return

		if region.nogate:
			char.message(1019004)
			fizzle(char)
			return

		if char.socket:
			char.socket.clilocmessage(501024)

		serials = []

		# Create the two gates
		gate = wolfpack.additem('f6c')
		gate.moveto(char.pos)
		gate.settag('dispellable_field', 1)
		gate.addscript( 'magic.gate' )
		gate.settag('target', str(location))
		gate.update()
		gate.soundeffect(0x20e)
		serials.append(gate.serial)

		gate = wolfpack.additem('f6c')
		gate.moveto(location)
		gate.settag('target', '%u,%u,%d,%u' % (char.pos.x, char.pos.y, char.pos.z, char.pos.map))
		gate.settag('dispellable_field', 1)
		gate.addscript( 'magic.gate' )
		gate.update()
		gate.soundeffect(0x20e)
		serials.append(gate.serial)

		wolfpack.addtimer(30000, field_expire, serials, 1)

class ManaVampire (DelayedDamageSpell):
	def __init__(self):
		DelayedDamageSpell.__init__(self, 7)
		self.reagents = {REAGENT_BLACKPEARL: 1, REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_SPIDERSILK: 1}
		self.mantra = 'Ort Sanct'
		self.reflectable = 1
		self.delay = None

	def damage(self, char, target):
		target.disturb()
		if target.frozen:
			target.frozen = 0
			target.resendtooltip()

		char.checkskill(self.damageskill, 0, 1200)

		# See if we can gain skill
		maxskill = self.circle * 100
		maxskill += (1 + ((self.circle - 1) / 6)) * 250

		if target.skill[MAGICRESISTANCE] < maxskill:
			target.checkskill(MAGICRESISTANCE, 0, 1200)

		drain = min(target.mana, int((char.skill[self.damageskill] - target.skill[MAGICRESISTANCE]) / 10.0))

		if target.player:
			drain /= 2

		if drain > 0:
			target.mana -= drain
			target.updatemana()
		
			if drain > char.maxmana - char.mana:
				drain = char.maxmana - char.mana
		
			char.mana += drain
			char.updatemana()

		target.effect(0x374a, 1, 15)
		target.soundeffect(0x1f9)

class MassDispel (Spell):
	def __init__(self):
		Spell.__init__(self, 7)
		self.reagents = {REAGENT_GARLIC: 1, REAGENT_BLACKPEARL: 1, REAGENT_MANDRAKE: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'Vas An Ort'
		self.validtarget = TARGET_GROUND

	def target(self, char, mode, targettype, target, args, item):
		if not self.consumerequirements(char, mode, args, target, item):
			return

		char.turnto(target)

		# Enumerate chars
		chars = wolfpack.chars(target.x, target.y, char.pos.map, 8)
		items = wolfpack.items(target.x, target.y, char.pos.map, 8)
		for target in chars:
			if char == target or not target.npc or target.summontimer == 0:
				continue

			if not mayAreaHarm(char, target):
				continue

			if self.checkresist(char, target):
				target.effect(0x3779, 10, 20)
			else:
				wolfpack.effect(0x3728, target.pos, 8, 20)
				target.soundeffect(0x201)
				target.delete()
			
		# Field spells
		for item in items:
			if item.hastag('dispellable_field'):
				wolfpack.effect(0x376a, item.pos, 9, 20)
				item.soundeffect(0x201)
				item.delete()

class MeteorSwarm (Spell):
	def __init__(self):
		Spell.__init__(self, 7)
		self.reagents = {REAGENT_SPIDERSILK: 1, REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'Flam Kal Des Ylem'
		self.validtarget = TARGET_GROUND

	def target(self, char, mode, targettype, target, args, item):
		if not self.consumerequirements(char, mode, args, target, item):
			return

		char.turnto(target)
		targets = []
		damage = self.scaledamage(char, None, 48, 1, 5)

		# Enumerate chars
		chars = wolfpack.chars(target.x, target.y, char.pos.map, 2)
		for target in chars:
			if mayAreaHarm(char, target):
				targets.append(target)

		# Re-scale the damage
		if len(targets) > 1:
			damage = ceil((damage * 2.0) / len(targets))

		for target in targets:
			target.soundeffect(0x160)
			char.movingeffect(0x36D4, target, 7, 0)
			self.harmchar(char, target)
			energydamage(target, char, damage, fire=100)

def polymorph_expire(char, arguments):
	# Remove all timers from this char
	char.dispel(None, True, 'POLYMORPH_EXPIRE')	
	
	# Hidden beard?
	if char.hastag('polymorph_beard_id'):
		current = char.itemonlayer(LAYER_BEARD)
		if current:
			current.delete()
		
		newid = char.gettag('polymorph_beard_id')
		color = 0
		if char.hastag('polymorph_beard_color'):
			color = int(char.gettag('polymorph_beard_color'))
		item = wolfpack.additem(newid)
		item.color = color
		char.additem(LAYER_BEARD, item)
		item.update()
		
		char.deltag('polymorph_beard_id')
		char.deltag('polymorph_beard_color')
	
	char.id = char.orgid
	char.skin = char.orgskin
	char.polymorph = 0
	char.update()

class Polymorph (Spell):
	def __init__(self):
		Spell.__init__(self, 7)
		self.reagents = {REAGENT_GARLIC: 1, REAGENT_GINSENG: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'Vas Ylem Rel'

	def checkrequirements(self, char, mode, args=[], target=None, item=None):
		if char.polymorph:
			if char.socket:
				char.socket.clilocmessage(1005559)
			return 0
		if char.incognito:
			if char.socket:
				char.socket.clilocmessage(502167)
			return 0

		if len(args) == 0:
			polymorph.showmenu(char)
			return 0
		return Spell.checkrequirements(self, char, mode, args, target, item)

	def cast(self, char, mode, args=[], target=None, item=None):
		if char.polymorph:
			if char.socket:
				char.socket.clilocmessage(1005559)
			return 0

		if char.incognito:
			if char.socket:
				char.socket.clilocmessage(502167)
			return 0

		# Select a creature to polymorph into
		char.unmount()
		char.orgid = char.id
		char.id = args[0]
		char.orgskin = char.skin
		if char.id == 0x190 or char.id == 0x191:
			char.skin = random.randint(1002, 1059)
		else:
			char.skin = 0
			
		# Remove the beard if morphing into a female
		if char.id == 0x191:
			beard = char.itemonlayer(LAYER_BEARD)
			if beard:
				char.settag('polymorph_beard_id', beard.baseid)
				char.settag('polymorph_beard_color', beard.color)
				beard.delete()			
						
		char.polymorph = 1
		char.update()

		duration = int(300 + char.skill[MAGERY] * 100)
		char.addtimer(duration, polymorph_expire, [], True, False, 'POLYMORPH_EXPIRE')

def onLoad():
	ChainLightning().register(49)
	EnergyField().register(50)
	FlameStrike().register(51)
	GateTravel().register(52)
	ManaVampire().register(53)
	MassDispel().register(54)
	MeteorSwarm().register(55)
	Polymorph().register(56)
