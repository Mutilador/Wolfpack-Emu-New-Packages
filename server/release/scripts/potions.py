import wolfpack
from random import randint
from wolfpack.time import *
from wolfpack.consts import *
from wolfpack.utilities import hex2dec, throwobject, energydamage

HEAL_POT_DELAY = 10000 # 10 Seconds
AGILITY_TIME = 120000  # 2 minutes
STRENGTH_TIME = 120000  # 2 minutes
INTELLIGENCE_TIME = 120000 # 2 minutes

# potion [ return_bottle, aggressive, target, name ]
potions = \
{
	0:		[ 1, 0, 0, 1044542 ], # nightsight
	1:		[ 1, 0, 0, 1044543 ], # lesser heal
	2:		[ 1, 0, 0, 1044544 ], # heal
	3:		[ 1, 0, 0, 1044545 ], # greater heal
	4:		[ 1, 0, 0, 1044552 ], # lesser cure
	5:		[ 1, 0, 0, 1044553 ], # cure
	6:		[ 1, 0, 0, 1044554 ], # greater cure
	7:		[ 1, 0, 0, 1044540 ], # agility
	8:		[ 1, 0, 0, 1044541 ], # greater agility
	9:		[ 1, 0, 0, 1044546 ], # strength
	10:	[ 1, 0, 0, 1044547 ], # greater strength
	11:	[ 0, 1, 1, 1044555 ], # lesser explosion
	12:	[ 0, 1, 1, 1044556 ], # explosion
	13:	[ 0, 1, 1, 1044557 ], # greater explosion
	14:	[ 1, 0, 0, 1044548 ], # lesser poison
	15:	[ 1, 0, 0, 1044549 ], # poison
	16:	[ 1, 0, 0, 1044550 ], # greater poison
	17:	[ 1, 0, 0, 1044551 ], # deadly poison
	18:	[ 1, 0, 0, 1044538 ], # refresh
	19:	[ 1, 0, 0, 1044539 ], # total refresh
	20:	[ 1, 0, 0, 'Intellegence' ], # intelligence
	21:	[ 1, 0, 0, 'Greater Intellegence' ], # greater intelligence
	22:	[ 1, 0, 0, 'Lesser Mana' ], # lesser mana
	23:	[ 1, 0, 0, 'Mana' ], # mana
	24:	[ 1, 0, 0, 'Greater Mana' ] # greater mana
}

POT_RETURN_BOTTLE = 0
POT_AGGRESSIVE = 1
POT_TARGET = 2
POT_NAME = 3

# Use the potion
def onUse( char, item ):
	socket = char.socket
	# Potions need to be on your body to use them, or in arms reach.
	if item.getoutmostchar() != char:
		char.message( "This potion is out of your reach..." )
		return

	# Lets make sure the tag exists.
	if not item.hastag( 'potiontype' ):
		return OOPS
	else:
		potiontype = item.gettag( 'potiontype' )
		# Make sure it's in the index
		if not potiontype in potions:
			return OOPS

		# Do we throw this thing?
		if potions[ potiontype ][ POT_TARGET ] == TRUE:
			# Explosion Potion
			if potiontype in [ 11, 12, 13 ]:
				socket.sysmessage( 'Please select a target...', RED )
				socket.attachtarget( "potions.targetexplosionpotion", [ item ] )
			return

		# We just drink this potion...
		else:
			# If it's a drinkable potion we check if there is a free hand.
			if not canUsePotion( char, item ):
				return OOPS
			# Heal Potions
			if potiontype in [ 1, 2, 3 ]:
				if char.hastag('poisoned'):
					# You can not heal yourself in your current state.
					char.socket.clilocmessage(1005000)
					return OOPS
				if char.hitpoints >= char.maxhitpoints:
					# You decide against drinking this potion, as you are already at full health.
					char.socket.clilocmessage(1049547)
				else:
					healPotion(char, item, potiontype)
				return

			# Cure Potions
			elif potiontype in [ 4, 5, 6 ]:
				socket.sysmessage('Drinking a cure potion.')
				return

			# Agility Potions
			elif potiontype in [ 7, 8 ]:
				if not char.hastag('agility_effect'):
					agilityPotion(char, item, potiontype)
				else:
					# You are already under a similar effect.
					char.socket.clilocmessage(502173)
				return

			# Strength Potions
			elif potiontype in [ 9, 10 ]:
				if not char.hastag( 'strength_effect' ):
					strengthPotion( char, item, potiontype )
				else:
					# You are already under a similar effect.
					char.socket.clilocmessage( 502173, '', GRAY )
				return

			# Poison Potions
			elif potiontype in [ 14, 15, 16, 17 ]:
				poisonPotion( char, item, potiontype )
				return

			# Nightsight Potions
			elif potiontype in [ 0 ]:
				nightsightPotion( char, potion )
				return

			# Not Known
			else:
				socket.sysmessage( 'Unknown potion...' )
				return

# Explosion Potion Function
def targetexplosionpotion(char, args, target):
	if target.char:
		pos = target.char
	elif target.item:
		item = target.item.getoutmostitem()
		if item.container:
			pos = item.container.pos
		else:
			pos = item.pos
	else:
		pos = target.pos

	if char.distanceto(pos) > 15:
		char.socket.clilocmessage(1005539)
		return

	potion = args[0]
	throwobject(char, potion, pos, 1, 3, 5)
	# char, potion, counter value
	potionexplosion( [ char, potion, 3 ] )
	potion.settag('exploding', 'true')
	return

# Explosion Potion Function
def potionexplosion(args):
	char = args[0]
	potion = args[1]
	counter = args[2]
	if counter > 0:
		wolfpack.addtimer(1000, "potions.potioncountdown", [char.serial, potion.serial, counter] )
		return
	else:
		potion.soundeffect(0x307) # Boom!
		potion.effect(0x36BD, 20, 10)
		potionregion( [char, potion] )
		potion.delete()
		return

# Explosion Potion Function
def potioncountdown( time, args ):
	char = wolfpack.findchar(args[0])
	potion = wolfpack.finditem(args[1])
	counter = args[2]
	if counter >= 0:
		if counter > 0:
			potion.say("%u" % counter)
			counter -= 1
		potionexplosion([char, potion, counter])
	return

# Explosion Potion Function
def potionregion( args ):
	char = args[0]
	potion = args[1]
	if potion.container:
		return OOPS
	if potion.gettag('potiontype') == 11:
		outradius = 1
	elif potion.gettag('potiontype') == 12:
		outradius = 2
	elif potion.gettag('potiontype') == 13:
		outradius = 3
	else:
		outradius = 1
	x1 = int(potion.pos.x - outradius)
	y1 = int(potion.pos.y - outradius)
	x2 = int(potion.pos.x + outradius)
	y2 = int(potion.pos.y + outradius)
	# Character Bombing
	damageregion = wolfpack.charregion( x1, y1, x2, y2, potion.pos.map )
	target = damageregion.first
	while target:
		target.effect(0x36BD, 20, 10)
		potiondamage(char, target, potion)
		target = damageregion.next

	# Chain Reaction Bombing
	chainregion = wolfpack.itemregion( x1, y1, x2, y2, potion.pos.map )
	chainbomb= chainregion.first
	while chainbomb:
		if chainbomb.baseid in [ 'potion_greaterexplosion', 'potion_explosion', 'potion_lesserexplosion', 'f0d' ]:
			if not chainbomb.hastag('exploding'):
				chainbomb.settag('exploding', 'true')
				wolfpack.addtimer(randint(1000, 2250), "potions.potioncountdown", [char, chainbomb, 0] )
				chainbomb = chainregion.next
			else:
				chainbomb = chainregion.next
		else:
			chainbomb = chainregion.next

	return

# Explosion Potion Function
def potiondamage( char, target, potion ):
	if potion.gettag('potiontype') == 11:
		damage = randint(1, 5)
	elif potion.gettag('potiontype') == 12:
		damage = randint(6, 10)
	elif potion.gettag('potiontype') == 13:
		damage = randint(11, 20)
	else:
		damage = randint(1, 20)
	if char.skill[ALCHEMY] == 1200:
		bonus = 10
	elif char.skill[ALCHEMY] >= 1100:
		bonus = randint(8,9)
	elif char.skill[ALCHEMY] >= 1000:
		bonus = randint(6,7)
	else:
		bonus = randint(0,5)
	if potion.amount > 1:
		damage = damage * potion.amount
	damage += bonus
	energydamage(target, char, damage, fire=100)
	return

# You have to have one hand free for using a potion
# This is not valid for explosion potions
def canUsePotion( char, item ):
	firsthand = char.itemonlayer( 1 )
	secondhand = char.itemonlayer( 2 )

	if not firsthand and not secondhand:
		return OK

	if firsthand and not secondhand and not firsthand.twohanded:
		return OK

	if not firsthand and secondhand and not secondhand.twohanded:
		return OK

	char.socket.clilocmessage( 0x7A99C ) # You must have a free hand to drink a potion.
	return OOPS

# Consume the potion
def consumePotion( char, potion, givebottle ):

	if potion.amount == 1:
		potion.delete()
	else:
		potion.amount -= 1
		potion.update()

	if givebottle == TRUE: # Lets add an empty bottle!
		bottle = wolfpack.additem( 'f0e' ) # Empty Bottle Definition
		if not wolfpack.utilities.tocontainer( bottle, char.getbackpack() ):
			bottle.update()

# Nightsight potion
def nightsightPotion( char, potion ):
	char.message( "This potion should help you to see in the dark!" )
	char.message( "But... It's not done yet. Sorry :(" )

# 10 Second Delay
def checkHealTimer( char ):

	if not char.hastag( "heal_timer" ):
		char.settag( "heal_timer", wolfpack.time.servertime() + HEAL_POT_DELAY )
		return

	# Compare
	elapsed = int( char.gettag( "heal_timer" ) )
	if elapsed > wolfpack.time.servertime():
		char.socket.clilocmessage( 500235, '', GRAY ) # You must wait 10 seconds before using another healing potion.
		return OOPS
	else:
		char.settag( "heal_timer", wolfpack.time.servertime() + HEAL_POT_DELAY )

	return

# Heal Potions
def healPotion( char, potion, healtype ):
	if not canUsePotion( char, potion ) or not checkHealTimer( char ):
		return OOPS

	amount = 0

	# Lesser Heal
	if healtype == 1:
		amount = randint( POTION_LESSERHEAL_RANGE[0], POTION_LESSERHEAL_RANGE[1] )

	# Heal
	elif healtype == 2:
		amount = randint( POTION_HEAL_RANGE[0], POTION_HEAL_RANGE[1] )

	# Greater Heal
	elif healtype == 3:
		amount = randint( POTION_GREATERHEAL_RANGE[0], POTION_GREATERHEAL_RANGE[1] )

	char.hitpoints = min( char.hitpoints + amount, char.maxhitpoints ) # We don't heal over our maximum health

	# Resend Health
	char.updatehealth()
	#char.socket.clilocmessage( 1060203, str(amount) , GRAY, NORMAL ) # broken
	char.socket.sysmessage( 'You have had ' + str( amount ) + ' hit points of damage healed.', GRAY )

	char.action( ANIM_FIDGET3 )
	char.soundeffect( SOUND_DRINK1 )
	consumePotion( char, potion, potions[ healtype ][ POT_RETURN_BOTTLE ] )

	return

# Dexterity Effect Timer
def effecttimer( time, args ):
	char = args[0]
	effecttype = args[1]
	bonus = args[2]

	if effecttype == 6 or effecttype == 7:
		if not char.hastag( 'agility_effect' ):
			return OOPS
		char.dexterity = char.dexterity - bonus
		char.maxstamina = char.maxstamina - bonus
		if char.stamina > char.maxstamina:
			char.stamina = char.maxstamina
		char.deltag( 'agility_effect' )
		char.updatestamina()
		char.updatestats()
		return

	elif effecttype == 8 or effecttype == 9:
		if not char.hastag( 'strength_effect' ):
			return OOPS
		char.strength = char.strength - bonus
		if char.hitpoints > char.maxhitpoints:
			char.hitpoints = char.maxhitpoints
		char.deltag( 'strength_effect' )
		char.updatehealth()
		char.updatestats()
		return

	elif effecttype == 20 or effecttype == 21:
		if not char.hastag( 'intelligence_effect' ):
			return OOPS
		char.intelligence = char.intelligence - bonus
		char.maxmana = char.maxmana - bonus
		if char.mana > char.maxmana:
			char.mana = char.maxmana
		char.deltag( 'intelligence_effect' )
		char.updatemana()
		char.updatestats()
		return

# Agility Potion
def agilityPotion( char, potion, agilitytype ):
	if not canUsePotion( char, potion ):
		return OOPS

	bonus = 0

	# Agility
	if agilitytype == 6:
		bonus = 10

	# Greater Agility
	elif agilitytype == 7:
		bonus = 20

	else:
		return OOPS

	char.dexterity = char.dexterity + bonus
	char.maxstamina = char.maxstamina + bonus
	wolfpack.addtimer( AGILITY_TIME, "potions.effecttimer", [ char, agilitytype, bonus ], 1 )
	char.settag( 'agility_effect', 'true' )
	char.updatestamina()
	char.updatestats()

	char.action( ANIM_FIDGET3 )
	char.soundeffect( SOUND_DRINK1 )
	char.effect( 0x375a, 10, 15 )
	char.soundeffect( SOUND_AGILITY_UP )
	consumePotion( char, potion, potions[ agilitytype ][ POT_RETURN_BOTTLE ] )

	return

# Strength Potion
def strengthPotion( char, potion, strengthtype ):
	if not canUsePotion( char, potion ):
		return OOPS

	bonus = 0

	# Agility
	if strengthtype == 8:
		bonus = 10

	# Greater Agility
	elif strengthtype == 9:
		bonus = 20

	else:
		return OOPS

	char.strength = char.strength + bonus
	char.maxhitpoints = char.maxhitpoints + bonus
	wolfpack.addtimer( STRENGTH_TIME, "potions.effecttimer", [ char, strengthtype, bonus ], 1 )
	char.settag( 'strength_effect', 'true' )
	char.updatehealth()
	char.updatestats()

	char.action( ANIM_FIDGET3 )
	char.soundeffect( SOUND_DRINK1 )
	char.effect( 0x375a, 10, 15 )
	char.soundeffect( SOUND_STRENGTH_UP )
	consumePotion( char, potion, potions[ strengthtype ][ POT_RETURN_BOTTLE ] )

	return

# Poison Potions
def poisonPotion( char, potion, poisontype ):
	char.socket.sysmessage( "You shouldn't drink that..." )
	return

# Potion Kegs
def onDropOnItem(target, potion):
 	# check for potionkeg event on object
 	if not 'potionkeg' in target.events:
		# This isn't a potion keg
		return OOPS
	if not potion.hastag('potiontype'):
		# 502232	The keg is not designed to hold that type of object.
		return OOPS

	if target.hastag('kegfill') and target.hastag('potiontype'):
		if potion.gettag('potiontype') == target.gettag('potiontype'):
			kegfill = target.gettag('kegfill')
			if kegfill < 100:
				kegfill += 1
				target.settag('kegfill', kegfill)
				potion.delete()
			else:
				# 502233	The keg will not hold any more!
				return OOPS
		else:
			# 502236	You decide that it would be a bad idea to mix different types of potions.
			return OOPS
	else:
		target.settag( 'potiontype', potion.gettag('potiontype') )
		target.settag( 'kegfill', 1 )

		return
 	return


# INVIS POTION
# ID: 0x7A9A3 (0)
# Your skin becomes extremely sensitive to light, changing to mirror the colors of things around you.
