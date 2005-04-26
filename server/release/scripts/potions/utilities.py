
import wolfpack
import time
from random import randint, random
from wolfpack.utilities import hex2dec, throwobject, energydamage, checkLoS
from potions.consts import *
from wolfpack.consts import *

def getPotionType( potion ):
	potiontype = None

	if potion.hasintproperty( 'potiontype' ):
		potiontype = potion.getintproperty( 'potiontype' )
	if potion.hastag( 'potiontype' ):
		potiontype = int( potion.gettag( 'potiontype' ) )

	if not potiontype in POTIONS:
		return None
	else:
		return potiontype

	return None

# You have to have one hand free for using a potion
# This is not valid for explosion potions
def canUsePotion( char, item ):
	firsthand = char.itemonlayer( 1 )
	secondhand = char.itemonlayer( 2 )

	if not firsthand and not secondhand:
		return True

	if firsthand and not secondhand and not firsthand.twohanded:
		return True

	if not firsthand and secondhand and not secondhand.twohanded:
		return True

	# You must have a free hand to drink a potion.
	char.socket.clilocmessage( 0x7A99C )
	return False

# Consume the potion
def consumePotion( char, potion, givebottle=True ):
	if potion.amount == 1:
		potion.delete()
	else:
		potion.amount -= 1
		if potion.container != char:
			potion.update()
	# Lets add an empty bottle!
	if givebottle:
		# Empty Bottle Definition
		bottle = wolfpack.additem( 'f0e' )
		if not wolfpack.utilities.tocontainer( bottle, char.getbackpack() ):
			bottle.update()
	return True

# Fill the potion into potionkeg
# if potion is stackable, fill all into potionkeg
def fillPotion( char, keg, potion, givebottle=True ):
	return consumePotion( char, potion, givebottle=True )

# Throw the potion at something
def targetpotion( char, args, target ):
	check = 10
	socket = char.socket
	pitem = wolfpack.finditem( args[0] )
	if not pitem:
		return False
	# Range Check Override
	if pitem.hastag( 'range' ):
		check = int( potion.gettag( 'range' ) )
	# Target Checking
	if not pitem:
		return False
	if target.char:
		if not char.cansee( target.char ):
			return False
		if char.distanceto( target.char ) > check:
			return False
		if not char.canreach( target, check ):
			return False
		pos = target.char.pos
	elif target.item:
		if not char.cansee( target.item ):
			return False
		if char.distanceto( target.item ) > check:
			return False
		if not char.canreach( target, check ):
			return False
		item = target.item.getoutmostitem()
		if item.container:
			pos = item.container.pos
		else:
			pos = item.pos
	else:
		if not char.canreach( target, check ):
			return False
		pos = target.pos
	# Distance Checking
	if char.distanceto( pos ) > check:
		socket.clilocmessage( 1005539 )
		return False
	#verify the potion still exists to be thrown...
	if pitem:
		if pitem.amount == 1:
			throwobject( char, pitem, pos, 1, 3, 5 )
		else:
			#pitem.amount -= 1
			#pitem.update()
			#throwobject( char, pitem, pos, 1, 3, 5 )
			return False

	return True