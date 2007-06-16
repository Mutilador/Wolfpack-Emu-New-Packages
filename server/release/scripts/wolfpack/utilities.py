#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Utility functions used in Python scripts                      #
#===============================================================#

"""
	\library wolfpack.utilities
	\description Contains several useful functions for scripts.
"""

# Calculates the Magic Damage (Base Damage + Base + Source)
import wolfpack
from wolfpack import properties, tr
import random
import string
from types import *
from wolfpack.consts import *
import wolfpack.console

payfrompackonly = wolfpack.settings.getbool( "General", "Pay From Pack Only", False, True )

"""
	\function wolfpack.utilities.countGold
	\param char The character whose gold shall be count.
	\return The amount of gold as an integer.
	\description Counts the amount of gold the character possess. If 'Pay From Pack Only' option is enabled gold in characters bankbox is ignored.
"""
def countGold( char ):
	if payfrompackonly:
		return char.countresource( 0xeed )
	else:
		return char.countresource( 0xeed ) + char.getbankbox().countresource( 0xeed )

"""
	\function wolfpack.utilities.consumeGold
	\param char The character whose gold shall be consumed.
	\param amount The amount of gold that shall be consumed.
	\return True if the gold could be consumed, false otherwise.
	\description Consumes a specified amount of gold from character. If 'Pay From Pack Only' option is enabled gold in characters bankbox is ignored.
	Always succeeds for GMs.
"""
def consumeGold( char, amount ):
	if char.gm:
		return True

	if payfrompackonly:
		# check if we have enough gold
		if countgold( char ) > amount:
			char.useresource( amount, 0xeed )
			return True
		else:
			return False

	bankbox = char.getbankbox()
	bankgold = bankbox.countresource( 0xeed )
	packgold = char.countresource( 0xeed )

	if packgold >= amount:
		# gold from pack
		char.useresource( amount, 0xeed )
	elif bankgold >= amount:
		# gold from bank
		bankbox.useresource( amount, 0xeed )
	elif bankgold + packgold >= amount:
		# gold from bank and pack
		char.useresource( packgold, 0xeed )
		bankbox.useresource( amount - packgold, 0xeed )
	else:
		# we don't have enough gold
		return False

	return True

"""
	\function wolfpack.utilities.rolldice
	\param dice This is either the number of dice you want to roll or a string representation of the number of dice, the sides
	and the bonus. One of the following would be valid: <code>"2d8+4", "2d8", "1d4-1", "20d10"</code>.
	\param sides Defaults to 6.
	The number of sides each dice should have.
	\param bonus Defaults to 0.
	The bonus that should be added to the result after the dice have been rolled.
	\return The result of the roll as an integer.
	\description Rolls some dice and returns a result.
"""
def rolldice(dice, sides=6, bonus=0):
	# Try parsing the dice
	if type(dice) == str:
		text = dice
		(dice, sides) = text.split('d')
		dice = int(dice)
		if '+' in sides:
			(sides, bonus) = sides.split('+')
			bonus = int(bonus)
			sides = int(sides)
		elif '-' in sides:
			(sides, bonus) = sides.split('-')
			bonus = - int(bonus)
			sides = int(sides)
	result = 0
	for i in range(0, dice):
		result += random.randint(1, int(sides))
	result += bonus
	return result

"""
	\function wolfpack.utilities.hex2dec
	\param value The string you want to convert.
	\param default Defaults to 0.
	The default value that will be returned if there is an error converting the given string.
	\return The string converted to an integer value.
	\description Converts a string to an integer value and accepts either a normal decimal number (<code>64</code>) or the
	standard hexadecimal notation (<code>0x40</code>).
"""
def hex2dec(value, default = 0):
	if type( value ) is IntType:
		return value

	try:
		value = value.lower()

		if not value.startswith( "0x" ):
			return int( value )
		else:
			return int( value.replace( "0x", "" ), 16 )
	except:
		return default

"""
	\function wolfpack.utilities.evenorodd
	\param value The integer value that should be checked.
	\return A string which is either "even" or "odd".
	\description Takes a given int value and determines if it is even or odd and returns a string with the result.
"""
def evenorodd( value ):
	if value % 2 == 0:
		return "even"
	else:
		return "odd"

"""
	\function wolfpack.utilities.itemsmatch
	\param item1 The first item.
	\param item2 The second item.
	\return True if the items could stack, false otherwise.
	\description Determines if the items match for stacking. This function does not check
	if the stacked amount is larger than the maximum.
"""
def itemsmatch(a, b):
	return a.canstack(b)

"""
	\function wolfpack.utilities.tobackpack
	\param item The item you want to move to the backpack of the character.
	\param char The character in whose backpack you want to place the item.
	\return True if the item was stacked inside the characters backpack and is now invalid.
	False if the item has been put into the characters backpack and needs to be updated.
	\description Moves an object into a characters backpack and stacks it if possible.
"""
def tobackpack(item, char):
	backpack = char.getbackpack()
	return tocontainer(item, backpack)

"""
	\function wolfpack.utilities.tobankbox
	\param item The item you want to move to the bank deposit box of the character.
	\param char The character in whose bank box you want to place the item.
	\return True if the item was stacked inside the characters bank box and is now invalid.
	False if the item has been put into the characters backpack and needs to be updated.
	\description Moves an object into a characters backpack and stacks it if possible.
"""
def tobankbox(item, char):
	backpack = char.getbankbox()
	return tocontainer(item, backpack)


"""
	\function wolfpack.utilities.booleantoggle
	\param boolean
	\return Boolean
	\description Takes a boolean property and toggles it.
"""

def booleantoggle( boolean ):
	if boolean == 1:
		boolean = 0
	elif boolean == 0:
		boolean = 1
	elif boolean.lower() == "true":
		boolean = "false"
	elif boolean.lower() == "false":
		boolean = "true"
	return boolean

"""
	\function wolfpack.utilities.tocontainer
	\param item The item you want to move to the given container.
	\param container The container you want to move the item into.
	\return True if the item was stacked inside the container and has been deleted.
	False if the item has not been stacked and needs to be updated in order to be seen.
	\description Moves an object into a container and stacks it if possible.
"""
def tocontainer( item, container ):
	for content in container.content:
		# Found an item to stack with
		if itemsmatch(content, item):
			if content.amount + item.amount <= 60000:
				content.amount = content.amount + item.amount
				content.update()
				content.resendtooltip()
				item.delete()
				return True # Stacked

	# We couldn't stack
	container.additem( item, 1, 1, 0 )
	return False # Not stacked

"""
	\function wolfpack.utilities.cont2cont
	\param container1 The source container.
	\param container2 The target container.
	\description Moves all objects from container1 to container2.
"""
def cont2cont( container1, container2 ):
	for item in container1.content:
		if not tocontainer( item, container2 ):
			item.update()

"""
	\function wolfpack.utilities.isclothing
	\param item
	\return Boolean
	\description Returns if the item is a known clothing.
"""
def isclothing( item ):

	clothes = [ 0x152e, 0x152f, 0x1530, 0x1531, 0x152e, 0x1537, 0x1538, 0x1539, \
			0x153a, 0x153b, 0x153c, 0x153d, 0x153e, 0x1541, 0x1542, 0x1712, \
			0x170b, 0x170c, 0x170d, 0x170e, 0x170f, 0x1710, 0x1711, 0x1712, \
			0x1f00, 0x1f01, 0x1f02, 0x1f03, 0x1f04, 0x1f7b, 0x1f7c, 0x1f9f, \
			0x1fa0, 0x1fa1, 0x1fa2, 0x1ffd, 0x1ffe, 0x2649, 0x264a, 0x265f, \
			0x2660, 0x2661, 0x2662, 0x2663, 0x2664, 0x2665, 0x2666, 0x267b, \
			0x267c, 0x267d, 0x267e, 0x267f, 0x2680, 0x2681, 0x2682, 0x2683, \
			0x2684, 0x2685, 0x2686, 0x268d, 0x268e, 0x268f, 0x26b0, 0x1515, \
			0x1516, 0x1517, 0x1518, 0x1efe, 0x1eff ]

	if item.id in clothes:
		return True
	else:
		return False

"""
	\function wolfpack.utilities.ishat
	\param item
	\return Boolean
	\description Returns if the item is a known hat.
"""
def ishat ( item ):

	hats = [ 0x153f, 0x1540, 0x1543, 0x1544, 0x1545, 0x1546, 0x1547, 0x1548, \
		0x1713, 0x1714, 0x1715, 0x1716, 0x1717, 0x1718, 0x1719, 0x171a, \
		0x171b, 0x171c ]

	if item.id in hats:
		return True
	else:
		return False

"""
	\function wolfpack.utilities.isarmor
	\param item
	\return Boolean
	\description Returns if the item is a known armor.
"""
def isarmor( item ):

	armors = [ 0x13be, 0x13bf, 0x13c0, 0x13c3, 0x13c4, 0x13c5, 0x13c6, 0x13c7, 0x13cb, \
			0x13cc, 0x13cd, 0x13ce, 0x13d2, 0x13d3, 0x13d4, 0x13d5, 0x13d6, \
			0x13da, 0x13db, 0x13dc, 0x13dd, 0x13e1, 0x13e2, 0x13d2, 0x13eb, \
			0x13ec, 0x13ed, 0x13ee, 0x13ef, 0x13f0, 0x13f1, 0x13f2, 0x1408, \
			0x1409, 0x140a, 0x140b, 0x140c, 0x140d, 0x140e, 0x140f, 0x1410, \
			0x1411, 0x1412, 0x1413, 0x1414, 0x1415, 0x1416, 0x1417, 0x1418, \
			0x1419, 0x141a, 0x144e, 0x144f, 0x1450, 0x1451, 0x1452, 0x1453, \
			0x1454, 0x1455, 0x1456, 0x1457, 0x2642, 0x2643, 0x2644, 0x2645, \
			0x2658, 0x2642, 0x2641, 0x2648, 0x2657, 0x1c00, 0x1c01, 0x1c02, \
			0x1c03, 0x1c04, 0x1c05, 0x1c06, 0x1c07, 0x1c08, 0x1c09, 0x1c0a, \
			0x1c0b, 0x1c0c, 0x1c0d, 0x1f0b, 0x1f0c, 0x1549, 0x154a, 0x154b, \
			0x154c, 0x2646, 0x2647, 0x13bb ]

	if item.id in armors:
		return True
	else:
		return False

"""
	\function wolfpack.utilities.isweapon
	\param item
	\return Boolean
	\description Returns if the item is a known weapon.
"""
def isweapon( item ):

	weapontypes = [ 1001, 1002, 1003, 1004, 1005, 1006, 1007 ]

	if item.type in weapontypes:
		return True
	else:
		return False

"""
	\function wolfpack.utilities.isshield
	\param item
	\return Boolean
	\description Returns if the item is a known shield.
"""
def isshield( item ):
	shields = [ 0x1b72, 0x1b73, 0x1b74, 0x1b75, 0x1b76, 0x1b77, 0x1b78, \
		0x1b79, 0x1b7a, 0x1b7b, 0x1bc3, 0x1bc4, 0x1bc5 ]

	if item.id in shields:
		return True
	else:
		return False

"""
	\function wolfpack.utilities.isspellbook
	\param item
	\return Boolean
	\description Returns if the item is a known spell book.
"""
def isspellbook( item ):
	sbtypes = [ 9 ]
	sbids = [ 0xe3b, 0xefa, 0x2252, 0x2253 ]
	if item.id in sbids and item.type in sbtypes:
		return True
	else:
		return False

"""
	\function wolfpack.utilities.isinstrument
	\param item
	\return Boolean
	\description Returns if the item is a known instrument.
"""
def isinstrument( item ):
	return item.id in [ 0xE9C, 0xE9D, 0xE9E, 0xEB1, 0xEB2, 0xEB3, 0xEB4, 0x2805 ]

"""
	\function wolfpack.utilities.isminingtool
	\param item
	\return Boolean
	\description Returns if the item is a known mining tool.
"""
def isminingtool( item ):
	return item.id in [ 0xf39, 0xf3a, 0xe85, 0xe86 ]

"""
	\function wolfpack.utilities.isoregem
	\param item
	\return Boolean
	\description Returns if the item is a known ore gem.
"""
def isoregem( item ):
	return item.id == 0x1ea7 and item.hastag( 'resource' ) and item.gettag( 'resource' ) == 'ore'

"""
	\function wolfpack.utilities.iswoodgem
	\param item
	\return Boolean
	\description Returns if the item is a known wood gem.
"""
def iswoodgem( item ):
	return item.id == 0x1ea7 and item.hastag( 'resource' ) and item.gettag( 'resource' ) == 'wood'

"""
	\function wolfpack.utilities.ismountainorcave
	\param tile
	\return Boolean
	\description Returns if the tile is a mountain or cave tile.
"""
def ismountainorcave( tile ):
	return tile in range( 220, 232 ) + \
			range( 236, 248 ) + \
			range( 252, 264 ) + \
			range( 268, 280 ) + \
			range( 286, 298 ) + \
			range( 321, 325 ) + \
			range( 467, 475 ) + \
			range( 476, 488 ) + \
			range( 492, 496 ) + \
			range( 543, 580 ) + \
			range( 581, 614 ) + \
			range( 1010, 1011 ) + \
			range( 1339, 1360 ) + \
			range( 1361, 1364 ) + \
			range( 1741, 1758) + \
			range( 1771, 1791 ) + \
			range( 1801, 1825 ) + \
			range( 1831, 1855 ) + \
			range( 1861, 1885 ) + \
			range( 1981, 2005 ) + \
			range( 2028, 2034 ) + \
			range( 2100, 2106 ) + \
			range( 13121, 13178 ) + \
			range( 13354, 13356 ) + \
			range( 13361, 13371 ) + \
			range( 13625, 13638 ) + \
			range( 16233, 16245 )

"""
	\function wolfpack.utilities.issand
	\param tile
	\return Boolean
	\description Returns if the tile is a sand tile.
"""
def issand( tile ):
	return tile in range( 22, 63 ) + \
			range( 68, 76 ) + \
			range( 286, 302 ) + \
			range( 402, 403 ) + \
			range( 424, 428 ) + \
			range( 441, 466 ) + \
			range( 642, 646 ) + \
			range( 650, 658 ) + \
			range( 821, 829 ) + \
			range( 833, 837 ) + \
			range( 845, 853 ) + \
			range( 857, 861 ) + \
			range( 951, 959 ) + \
			range( 967, 971 ) + \
			range( 1447, 1459 ) + \
			range( 1611, 1619 ) + \
			range( 1623, 1627 ) + \
			range( 1635, 1643 ) + \
			range( 1647, 1651 )

"""
	\function wolfpack.utilities.iscrystal
	\param tile
	\return Boolean
	\description Returns if the tile is a crystal tile, AoS artwork.
"""
def iscrystal( tile ):
	return tile in range( 8710, 8750 )

"""
	\function wolfpack.utilities.istree
	\param tile
	\return Boolean
	\description Returns if the tile is a tree object..
"""
def istree( tile ):
	return tile in range( 3220, 3223 ) + \
			range( 3230, 3231 ) + \
			range( 3242, 3243 ) + \
			range( 3274, 3278 ) + \
			range( 3230, 3231 ) + \
			range( 3242, 3243 ) + \
			range( 3274, 3275 ) + \
			range( 3280, 3281 ) + \
			range( 3283, 3284 ) + \
			range( 3286, 3287 ) + \
			range( 3288, 3289 ) + \
			range( 3290, 3291 ) + \
			range( 3293, 3294 ) + \
			range( 3296, 3297 ) + \
			range( 3299, 3300 ) + \
			range( 3302, 3303 ) + \
			range( 3320, 3321 ) + \
			range( 3323, 3324 ) + \
			range( 3326, 3327 ) + \
			range( 3329, 3330 ) + \
			range( 3393, 3397 ) + \
			range( 3415, 3420 ) + \
			range( 3438, 3443 ) + \
			range( 3460, 3463 ) + \
			range( 3476, 3477 ) + \
			range( 3480, 3481 ) + \
			range( 3484, 3485 ) + \
			range( 3486, 3487 ) + \
			range( 3488, 3489 ) + \
			range( 3492, 3493 ) + \
			range( 3496, 3497 ) + \
			range( 4789, 4798 ) + \
			range( 8778, 8782 )

"""
	\function wolfpack.utilities.iswater
	\param tile
	\return Boolean
	\description Returns if the tile is a water tile.
"""
def iswater( tile ):
	return tile in range( 168, 172 ) + \
		range( 6038, 6067 ) + \
		range( 13422, 13446 ) + \
		range( 13456, 13484 ) + \
		range( 13493, 13526 )

"""
	\function wolfpack.utilities.isdirt
	\param item
	\return Boolean
	\description Returns if the tile is a dirt tile..
"""
def isdirt( tile ):
	return tile in range( 113, 125 ) + \
			range( 130, 168 ) + \
			range( 220, 228 ) + \
			range( 232, 236 ) + \
			range( 321, 336 ) + \
			range( 361, 377 ) + \
			range( 476, 496 ) + \
			range( 622, 634 ) + \
			range( 638, 642 ) + \
			range( 720, 857 ) + \
			range( 871, 891  ) + \
			range( 901, 937 ) + \
			range( 1035, 1054 ) + \
			range( 1351, 1366 ) + \
			range( 1431, 1447 ) + \
			range( 1571, 1594 ) + \
			range( 1779, 1787 ) + \
			range( 1911, 1937 ) + \
			range( 2444, 2495 ) + \
			range( 12788, 12796 ) + \
			range( 13683, 13695 ) + \
			range( 13742, 13746 )

"""
	\function wolfpack.utilities.checkresources
	\param container The container you want to search in.
	\param baseid The baseid of the resource you are looking for.
	\param amount The amount that is required of the given resource.
	\return False if all the required resources have been found. Otherwise the remaining amount of
	the resource that has not been found.
	\description Recursively searches for items with a given baseid in a container and checks if a
	given amount can be found.
"""
def checkresources(container, baseid, amount):
	for item in container.content:
		if item.baseid == baseid:
			amount -= item.amount
		else:
			amount = checkresources(item, baseid, amount)

		# Don't count negative values
		if amount <= 0:
			amount = 0
			break

	return amount

def consumeresourcesinternal(container, baseid, amount):
	for item in container.content:
		if item.baseid == baseid:
			# Enough to statisfy our consumption
			if item.amount > amount:
				item.amount -= amount
				item.update()
				return False

			amount -= item.amount
			item.delete()
		else:
			amount = consumeresourcesinternal(item, baseid, amount)

	return amount

"""
	\function wolfpack.utilities.consumeresources
	\param container The container you want to remove from.
	\param baseid The baseid of the items you want to remove.
	\param amount The amount of items you want to remove.
	\return True if the resource could be consumed, false otherwise.
	\description Removes a given amount of items with a given baseid recursively from a given container.
"""
def consumeresources(container, baseid, amount):
	if checkresources(container, baseid, amount) == 0:
		return consumeresourcesinternal(container, baseid, amount) == 0

	return False

"""
	\function wolfpack.utilities.energydamage
	\param target The target that is taking damage. Only characters are allowed.
	\param source The source of the damage. May be None.
	\param amount The amount of damage dealt.
	\param physical Defaults to 0.
	The physical fraction of the damage. This is an integer value ranging from 0 to 100.
	\param fire Defaults to 0.
	The fire fraction of the damage. This is an integer value ranging from 0 to 100.
	\param cold Defaults to 0.
	The cold fraction of the damage. This is an integer value ranging from 0 to 100.
	\param poison Defaults to 0.
	The poison fraction of the damage. This is an integer value ranging from 0 to 100.
	\param energy Defaults to 0.
	The energy fraction of the damage. This is an integer value ranging from 0 to 100.
	\param noreflect Defaults to 0. If this parameter is 1, no physical damage will be reflected back to
	the source.
	\param ignorephysical Defaults to False. If this parameter is true, the physical resistance of the target
	will be ignored.
	\param damagetype Defaults to DAMAGE_MAGICAL. This is the damagetype passed on to the internal damage function.
	You can ignore this most of the time.
	\description Deal damage to a target and take the targets energy resistances into account.
"""
def energydamage(target, source, amount, physical=0, fire=0, cold=0, poison=0, energy=0, noreflect=0, damagetype=DAMAGE_MAGICAL, ignorephysical = False):
	if not target:
		raise RuntimeError, "Invalid arguments for energydamage."

	amount = max(1, amount)

	if physical + fire + cold + poison + energy == 0:
		raise RuntimeError, "Invalid arguments for energydamage."

	damage = 0

	if physical > 0:
		physical = amount * (physical / 100.0)
		resistance = properties.fromchar(target, RESISTANCE_PHYSICAL) / 100.0

		if ignorephysical:
			resistance = 0 # Zero resistance

		damage = max(0, physical - (physical * resistance))

		if source and not noreflect and damage > 0:
			reflectphysical = properties.fromchar(target, REFLECTPHYSICAL)
			reflect = reflectphysical / 100.0 * damage

			if reflect > 0:
				energydamage(source, target, reflect, physical=100, noreflect=1)
				#target.effect(0x22c6, 10, 16)
				#target.movingeffect(0x22c6, source, 0, 0, 14)

	if fire > 0:
		fire = amount * (fire / 100.0)
		resistance = properties.fromchar(target, RESISTANCE_FIRE) / 100.0
		damage += max(0, fire - (fire * resistance))

	if cold > 0:
		cold = amount * (cold / 100.0)
		resistance = properties.fromchar(target, RESISTANCE_COLD) / 100.0
		damage += max(0, cold - (cold * resistance))

	if poison > 0:
		poison = amount * (poison / 100.0)
		resistance = properties.fromchar(target, RESISTANCE_POISON) / 100.0
		damage += max(0, poison - (poison * resistance))

	if energy > 0:
		energy = amount * (energy / 100.0)
		resistance = properties.fromchar(target, RESISTANCE_ENERGY) / 100.0
		damage += max(0, energy - (energy * resistance))

	damage = int(max(1, damage))
	return target.damage(damagetype, damage, source)


"""
	\function wolfpack.utilities.createkeysecret
	\description Returns a random key secret used to create locks, or change lock secrets
"""
def createkeysecret():
    parts = ['A','B','C','D','E','F','G','H','J','K','L','M','N','P','Q','R','S','T','U','V','W','X','Y','Z','2','3','4','5','6','7','8','9']
    rkeyid = ''
    for i in range(1,8):
       	rkeyid += random.choice(parts)
    return rkeyid
    
"""
	\function wolfpack.utilities.createlockandkey
	\param container The container you want to make lockable.
	\description Makes a current container lockable and creates a key in it.
"""
def createlockandkey( container ):
	if container.type == 1:
		if not container.hastag('lock'):
			rkeyid = createkeysecret()
			key = wolfpack.additem('1010')
			key.settag('lock', rkeyid)
			tocontainer( key, container )
			container.settag( 'lock', rkeyid )
			container.addscript( 'lock' )
	return

"""
	\function wolfpack.utilities.throwobject
	\param char
	\param object
	\param pos The target position.
	\param sendobject
	\param movable To change the item.movable value
	\param speed
	\param fixeddir
	\param explodes
	\param hue
	\param rendermode
	\return none
	\description Animates a character's throwing of an object at a given target.
"""
def throwobject(char, object, pos, sendobject=0, movable=1, speed=10, fixeddir=0, explodes=0, hue=0, rendermode=0):
	# This will make the object leave the character's pack and land at the target location.
	if sendobject > 0:
		if not movable == object.movable and movable >= 0:
			object.movable = movable

		object.container = 0
		object.moveto(pos)
		object.update()

	char.turnto(pos)
	char.action(0x9)
	char.movingeffect(object.id, pos, fixeddir, explodes, speed, hue, rendermode)
	return

"""
	\function wolfpack.utilities.checkLoS
	\param object1 First object, char/item
	\param object2 Second object, char/item
	\param rangecheck Default 10.
	\return boolean
	\description Checks the line of sight between two objects and returns true or false if in sight.
"""
#
def checkLoS( object1, object2, rangecheck=10 ):
	# Char -> Char Check
	if ( object1.ischar() and object2.ischar() ):
		char1 = wolfpack.findchar(object1.serial)
		char2 = wolfpack.findchar(object2.serial)
		if not char1.cansee(char2):
			return False
		if char1.distanceto(char2) > rangecheck:
			return False
		if not char1.canreach(char2, rangecheck):
			return False
		return True
	# Item -> Item Check
	elif ( object1.isitem() and object2.isitem() ):
		item1 = wolfpack.finditem(object1.serial)
		item2 = wolfpack.finditem(object2.serial)
		if item1.distanceto(item2) > rangecheck:
			return False
		return True
	# Char -> Item Check
	elif ( object1.ischar() and object2.isitem() ):
		char = wolfpack.findchar(object1.serial)
		item = wolfpack.finditem(object2.serial)
		if not char.cansee(item):
			return False
		if (char.distanceto(item) > rangecheck) or (item.distanceto(char) > rangecheck):
			return False
		if not char.canreach(item, rangecheck):
			return False
		return True
	# Item -> Char Check
	elif ( object1.isitem() and object2.ischar() ):
		item = wolfpack.finditem(object1.serial)
		char = wolfpack.findchar(object2.serial)
		if not char.cansee(item):
			return False
		if (item.distanceto(char) > rangecheck) or (char.distanceto(item) > rangecheck):
			return False
		if not char.canreach(item, rangecheck):
			return False
		return True
	# Failed Object Check
	else:
		return False

"""
	\function wolfpack.utilities.itemsincontainer
	\param serial
	\return list
	\description Returns a list of item serials inside the container.
"""
def itemsincontainer( serial ):
	container = wolfpack.finditem( serial )
	itemlist = []
	if container.type == 1:
		for item in container.content:
			itemlist += [ item.serial ]
		return itemlist
	else:
		return itemlist

# Class for Wrapping Chars or Items in Argument Lists
class ObjectWrapper:
	def __init__(self, wrapped):
		self.serial = wrapped.serial

	def get(self):
		if self.serial > 0 and self.serial < 0x40000000:
			char = wolfpack.findchar(self.serial)
			if not char.free:
				return char
		elif serial > 0x40000000:
			item = wolfpack.finditem(self.serial)
			if not item.free:
				return item

		return None

"""
	Show a smoke puff for the given character.
	Teleportation effect.
"""
def smokepuff(char, position):
	if char.invisible:
		return

	wolfpack.effect(0x3728, position, 10, 15)
	# Only do it once
	if position == char.pos:
		char.soundeffect(0x1fe)

"""
	Check if the given chars are allies.
"""
def checkAllies(char1, char2):
	# For NPCs, check for the owner instead
	if char1.npc:
		if char1.owner:
			char1 = char1.owner
		else:
			return False # Unowned NPCs are never allied with anyone

	# For NPCs, check the owner instead
	if char2.npc:
		if char2.owner:
			char2 = char2.owner
		else:
			return False # Unowned NPCs are never allied with anyone

	# Check if they're the same first.
	if char1 == char2:
		return True

	# Then check for their Party
	party = char1.party
	if party and char2.party == party:
		return True # Same Party -> Allies

	# Then check for their guilds
	guild = char1.guild
	if guild and char2.guild == guild:
		return True

	# TODO: Check for allied guilds

	return False

"""
	Check if the given chars are enemies
"""
def checkEnemies(char1, char2):
	if char1 == char2:
		return False

	# Check the attacktarget first (quick)
	if char1.attacktarget == char2 or char2.attacktarget == char1:
		return True

	# Check the list of opponents (for char1, makes no sense otherwise)
	opponents = char1.getopponents()
	if char2 in opponents:
		return True

	# Do the same check for the owner of char1, if char1 is a npc.
	if char1.npc and char1.owner and checkEnemies(char1.owner, char2):
		return True

	# Do the same check for the owner of char2 if char2 is a npc.
	if char2.npc and char2.owner and checkEnemies(char1, char2.owner):
		return True

	# Everyone being a murderer, a criminal or a npc with red karma
	# is an enemy.
	if char2.ismurderer() or char2.iscriminal() or (char2.npc and char2.notoriety(char1) in [4, 5, 6] ):
		return True

	return False

#
# Checks if the given character may be harmed by the given player
# using an area effect
#
def mayAreaHarm(player, char, excludeself = True, includeinnocents = False):
	# If we shouldn't be included, skip us ahead
	if char == player and excludeself:
		return False

	# Invulnerable, Invisible and Hidden creatures aren't affected
	if char.dead or char.invulnerable or (char.region and char.region.safe) or not player.cansee(char):
		return False

	# If we're allied with the given character, Return right away
	if checkAllies(player, char):
		return False

	# If we're enemies, always include
	if checkEnemies(player, char):
		return True

	# If we're neutral, it depends on the excludeneutrals flag
	return includeinnocents

#
# Checks if the given character may benefit from the given players
# area effect
#
def mayAreaBenefit(player, char, excludeself = False, includeinnocents = False):
	# If we shouldn't be included, skip us ahead
	if char == player and excludeself:
		return False

	# Invulnerable, Invisible and Hidden creatures aren't affected
	if char.invulnerable or not player.cansee(char):
		return False

	# If we're allied with the given character, Return right away
	if checkAllies(player, char):
		return True

	# If we're enemies, always include
	if checkEnemies(player, char):
		return False

	# If we're neutral, it depends on the excludeneutrals flag
	return includeinnocents

"""
	\function wolfpack.utilities.isValidPosition
	\param pos The wolfpack coord object to check.
	\return Returns a boolean value if the position is valid.
	\description Checks to see if a given coord object is a valid map position.
"""
usesMondaingsLegacyMap = wolfpack.settings.getbool( "General", "Uses Mondains Legacy Map", True, True )

def isValidPosition( pos ):
	# Check if the map is valid.
	if wolfpack.hasmap( pos.map ):
		# Z Checking
		if pos.z > 127 or pos.z < -128:
			return False
		# X & Y Checking
		# Felucca & Trammel
		if pos.map == 0 or pos.map == 1:
			if usesMondaingsLegacyMap:		
				if pos.x >= 7168 or pos.x < 0:
					return False
			else:
				if pos.x >= 6144 or pos.x < 0:
					return False
			if pos.y >= 4096 or pos.y < 0:
				return False
		# Ilshenar
		elif pos.map == 2:
			if pos.x >= 2304 or pos.x < 0:
				return False
			if pos.y >= 1600 or pos.y < 0:
				return False
		# Malas
		elif pos.map == 3:
			if pos.x >= 2560 or pos.x < 0:
				return False
			if pos.y >= 2048 or pos.y < 0:
				return False
		# Tokuno Islands
		elif pos.map == 4:
			if pos.x >= 1448 or pos.x < 0:
				return False
			if pos.y >= 1448 or pos.y < 0:
				return False
	else:
		return False

	return True

"""
	\function wolfpack.utilities.isMapAvailableTo
	\param player The player that this should be checked for.
	\param mapid The mapid that is being checked.
	\return Returns a boolean value if the map is available.
	\description Checks to see if a given map id is available to the connected client.
"""
def isMapAvailableTo(player, mapid):
	# Check if the player can move there based on his socket
	if player.socket:
		if mapid == 2 and player.socket.flags & 0x02 == 0:
			player.socket.sysmessage( tr('You have to install Ultima Online Third Dawn or above to use this gate.') )
			return False
		elif mapid == 3 and player.socket.flags & 0x08 == 0:
			player.socket.sysmessage( tr('You have to install Age of Shadows or above to use this gate.') )
			return False
		elif mapid == 4 and player.socket.flags & 0x10 == 0:
			player.socket.sysmessage( tr('You have to install Samurai Empire or above to use this gate.') )
			return False

	return True

def isWall(x, y, z, map):
	tiles = wolfpack.statics(x, y, map, 1)

	for tile in tiles:
		wall = wolfpack.tiledata(tile[0])["flag1"] & 0x010
		if wall and (z + 16) > tile[3] and (tile[3] + wolfpack.tiledata(tile[0])["height"]) > z:
			return True
	return False

#def isyoung(char):
#	enable_young = int( wolfpack.settings.getbool("General", "Enable Young Status", True, True) )
#	if enable_young and char.hasscript("system.young"):
#		return True
#	return False

"""
	\function wolfpack.utilities.staffmessage
	\param char The char causing the message.
	\param text Text which should be sended.
	\pram mtype The message type. Valid values are so far:
	0 - hearall
	\description Sends a message to every connected staff char depending on the mtype.
"""
def staffmessage(char, text, mtype):
	socket = wolfpack.sockets.first()
	while socket:
		if socket.player.gm:
			if mtype == 0 and char.distanceto(socket.player) > 16 and socket.player.account.rank >= char.account.rank and socket.player.hastag('hearall'):
				socket.sysmessage(unicode(char.name) + ': ' + unicode(text), YELLOW)
		socket = wolfpack.sockets.next()
	return
