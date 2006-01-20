
import wolfpack
import time
from random import randint, random
from wolfpack.utilities import hex2dec, throwobject, energydamage, checkLoS
from potions.consts import *
from wolfpack.consts import *
from wolfpack import properties

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

	if firsthand and not secondhand and not firsthand.twohanded or properties.fromitem(firsthand, BALANCED):
		return True

	if not firsthand and secondhand and not secondhand.twohanded or properties.fromitem(secondhand, BALANCED):
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
def fillPotion( char, keg, potion, givebottle=True ):
	kegfill = int( keg.gettag( 'kegfill' ) )
	kegfill += 1
	keg.settag( 'kegfill', kegfill )
	return consumePotion( char, potion, givebottle=True )

# Fill bottles with potion
def fillbottle(char, backpack, kegfill, kegtype, potionkeg):
	backpack.removeitems( [ 'f0e' ], 1 )
	newpot = wolfpack.additem( POTIONS[ kegtype ][ POT_DEF ] )
	if not newpot:
		return False
	newpot.decay = 1
	newpot.movable = 1
	char.socket.clilocmessage( 502242 ) # You pour some of the keg's contents into an empty bottle...
	char.socket.clilocmessage( 502243 ) # ...and place it into your backpack.
	if not wolfpack.utilities.tobackpack( newpot, char ):
		newpot.update()
	char.soundeffect(0x240)
	kegfill -= 1
	potionkeg.settag('kegfill', kegfill )
	if kegfill == 0:
		if potionkeg.name != "#1041641":
			potionkeg.name = '#1041641'
		potionkeg.deltag( 'potiontype' )
		#potionkeg.update()
		char.socket.clilocmessage( 502245 ) # The keg is now empty
	potionkeg.resendtooltip()
	return True

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