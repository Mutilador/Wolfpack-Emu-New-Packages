#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-" {{ ;"`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
from wolfpack.gumps import cGump
from wolfpack.utilities import isValidPosition

def onUse( char, item ):
	# check distance
	if( char.distanceto( item ) > 3):
		char.socket.sysmessage( "You are too far away to use it." )
		return 1

	# send gate gump
	char.socket.closegump( 0x87654321 )

	sendGump( char, item )
	return 1

def onCollide( char, item ):
	# npc would not travel through moongate ;)
	if( char.npc ):
		return 1

	char.socket.closegump( 0x87654321 )

	sendGump( char, item )
	return 1

def sendGump( char, item ):
	char.soundeffect( 0x58 )
	# noclose, nomove gump
	gateGump = cGump( 1, 1, 0, 100, 120 )
	# trammel
	gateGump.startPage( 1 )
	gateGump.addBackground( 9200, 380, 280 )
	gateGump.addText( 5, 10, "Pick your destination:" )
	gateGump.addButton( 10, 210, 0xFA5, 0xFA7, 1 )
	gateGump.addText( 45, 210, "OKAY", 0 )
	gateGump.addButton( 10, 235, 0xFA5, 0xFA7, 0 )
	gateGump.addText( 45, 235, "CANCEL" )
	# facets
	gateGump.addText( 35, 35, "Trammel", 61 )
	gateGump.addPageButton( 10, 35, 0x4B9, 0x4BA, 1 )
	gateGump.addText( 35, 60, "Felucca" )
	gateGump.addPageButton( 10, 60, 0x4B9, 0x4BA, 2 )
	gateGump.addText( 35, 85, "Ilshenar" )
	gateGump.addPageButton( 10, 85, 0x4B9, 0x4BA, 3 )
	gateGump.addText( 35, 110, "Malas" )
	gateGump.addPageButton( 10, 110, 0x4B9, 0x4BA, 4 )
	gateGump.addText( 35, 135, "Tokuno Islands")
	gateGump.addPageButton( 10, 135, 0x4B9, 0x4BA, 5 )
	# gates
	gateGump.addText( 225, 40, "Britain" )
	gateGump.addRadioButton(200, 40, 210, 211, 10 )
	gateGump.addText( 225, 65, "Magincia" )
	gateGump.addRadioButton(200, 65, 210, 211, 11 )
	gateGump.addText( 225, 90, "Moonglow" )
	gateGump.addRadioButton(200, 90, 210, 211, 12 )
	gateGump.addText( 225, 115, "Skara Brae" )
	gateGump.addRadioButton(200, 115, 210, 211, 13 )
	gateGump.addText( 225, 140, "Trinsic" )
	gateGump.addRadioButton(200, 140, 210, 211, 14 )
	gateGump.addText( 225, 165, "Vesper" )
	gateGump.addRadioButton(200, 165, 210, 211, 15 )
	gateGump.addText( 225, 190, "Yew" )
	gateGump.addRadioButton(200, 190, 210, 211, 16 )
	gateGump.addText( 225, 215, "Jhelom" )
	gateGump.addRadioButton(200, 215, 210, 211, 17 )
	# felucca
	gateGump.startPage( 2 )
	gateGump.addBackground( 9200, 380, 280 )
	gateGump.addText( 5, 10, "Pick your destination:" )
	gateGump.addButton( 10, 210, 0xFA5, 0xFA7, 1 )
	gateGump.addText( 45, 210, "OKAY", 0 )
	gateGump.addButton( 10, 235, 0xFA5, 0xFA7, 0 )
	gateGump.addText( 45, 235, "CANCEL" )
	# facets
	gateGump.addText( 35, 35, "Trammel" )
	gateGump.addPageButton( 10, 35, 0x4B9, 0x4BA, 1 )
	gateGump.addText( 35, 60, "Felucca", 61 )
	gateGump.addPageButton( 10, 60, 0x4B9, 0x4BA, 2 )
	gateGump.addText( 35, 85, "Ilshenar" )
	gateGump.addPageButton( 10, 85, 0x4B9, 0x4BA, 3 )
	gateGump.addText( 35, 110, "Malas" )
	gateGump.addPageButton( 10, 110, 0x4B9, 0x4BA, 4 )
	gateGump.addText( 35, 135, "Tokuno Islands" )
	gateGump.addPageButton( 10, 135, 0x4B9, 0x4BA, 5 )
	# gates
	gateGump.addText( 225, 40, "Britain" )
	gateGump.addRadioButton(200, 40, 210, 211, 18 )
	gateGump.addText( 225, 65, "Magincia" )
	gateGump.addRadioButton(200, 65, 210, 211, 19 )
	gateGump.addText( 225, 90, "Moonglow" )
	gateGump.addRadioButton(200, 90, 210, 211, 20 )
	gateGump.addText( 225, 115, "Skara Brae" )
	gateGump.addRadioButton(200, 115, 210, 211, 21 )
	gateGump.addText( 225, 140, "Trinsic" )
	gateGump.addRadioButton(200, 140, 210, 211, 22 )
	gateGump.addText( 225, 165, "Vesper" )
	gateGump.addRadioButton(200, 165, 210, 211, 23 )
	gateGump.addText( 225, 190, "Yew" )
	gateGump.addRadioButton(200, 190, 210, 211, 24 )
	gateGump.addText( 225, 215, "Jhelom" )
	gateGump.addRadioButton(200, 215, 210, 211, 25 )
	# ilshenar
	gateGump.startPage( 3 )
	gateGump.addBackground( 9200, 380, 280 )
	gateGump.addText( 5, 10, "Pick your destination:" )
	gateGump.addButton( 10, 210, 0xFA5, 0xFA7, 1 )
	gateGump.addText( 45, 210, "OKAY", 0 )
	gateGump.addButton( 10, 235, 0xFA5, 0xFA7, 0 )
	gateGump.addText( 45, 235, "CANCEL" )
	# facets
	gateGump.addText( 35, 35, "Trammel" )
	gateGump.addPageButton( 10, 35, 0x4B9, 0x4BA, 1 )
	gateGump.addText( 35, 60, "Felucca" )
	gateGump.addPageButton( 10, 60, 0x4B9, 0x4BA, 2 )
	gateGump.addText( 35, 85, "Ilshenar", 61 )
	gateGump.addPageButton( 10, 85, 0x4B9, 0x4BA, 3 )
	gateGump.addText( 35, 110, "Malas" )
	gateGump.addPageButton( 10, 110, 0x4B9, 0x4BA, 4 )
	gateGump.addText( 35, 135, "Tokuno Islands" )
	gateGump.addPageButton( 10, 135, 0x4B9, 0x4BA, 5 )
	#gates
	gateGump.addText( 225, 40, "Compassion" )
	gateGump.addRadioButton( 200, 40, 210, 211, 26 )
	gateGump.addText( 225, 65, "Honesty" )
	gateGump.addRadioButton( 200, 65, 210, 211, 27 )
	gateGump.addText( 225, 90, "Honor" )
	gateGump.addRadioButton( 200, 90, 210, 211, 28 )
	gateGump.addText( 225, 115, "Humility" )
	gateGump.addRadioButton( 200, 115, 210, 211, 29 )
	gateGump.addText( 225, 140, "Justice" )
	gateGump.addRadioButton( 200, 140, 210, 211, 30 )
	gateGump.addText( 225, 165, "Sacrifice" )
	gateGump.addRadioButton( 200, 165, 210, 211, 31 )
	gateGump.addText( 225, 190, "Spirituality" )
	gateGump.addRadioButton( 200, 190, 210, 211, 32 )
	gateGump.addText( 225, 215, "Valor" )
	gateGump.addRadioButton( 200, 215, 210, 211, 33 )
	gateGump.addText( 225, 240, "Chaos" )
	gateGump.addRadioButton( 200, 240, 210, 211, 34 )
	# Malas
	gateGump.startPage( 4 )
	gateGump.addBackground( 9200, 380, 280 )
	gateGump.addText( 5, 10, "Pick your destination:" )
	gateGump.addButton( 10, 210, 0xFA5, 0xFA7, 1 )
	gateGump.addText( 45, 210, "OKAY", 0 )
	gateGump.addButton( 10, 235, 0xFA5, 0xFA7, 0 )
	gateGump.addText( 45, 235, "CANCEL" )
	# facets
	gateGump.addText( 35, 35, "Trammel" )
	gateGump.addPageButton( 10, 35, 0x4B9, 0x4BA, 1 )
	gateGump.addText( 35, 60, "Felucca" )
	gateGump.addPageButton( 10, 60, 0x4B9, 0x4BA, 2 )
	gateGump.addText( 35, 85, "Ilshenar" )
	gateGump.addPageButton( 10, 85, 0x4B9, 0x4BA, 3 )
	gateGump.addText( 35, 110, "Malas", 61 )
	gateGump.addPageButton( 10, 110, 0x4B9, 0x4BA, 4 )
	gateGump.addText( 35, 110, "Tokuno Islands" )
	gateGump.addPageButton( 10, 110, 0x4B9, 0x4BA, 5 )
	# #gates
	gateGump.addText( 225, 40, "Luna Moongate" )
	gateGump.addRadioButton( 200, 40, 210, 211, 35 )
	gateGump.addText( 225, 65, "Bank of Luna" )
	gateGump.addRadioButton( 200, 65, 210, 211, 36 )
	gateGump.addText( 225, 90, "Stables of Luna" )
	gateGump.addRadioButton( 200, 90, 210, 211, 37 )
	gateGump.addText( 225, 115, "North Entrance" )
	gateGump.addRadioButton( 200, 115, 210, 211, 38 )
	gateGump.addText( 225, 140, "Luna Arena" )
	gateGump.addRadioButton( 200, 140, 210, 211, 39 )
	gateGump.addText( 225, 165, "Umbra Moongate" )
	gateGump.addRadioButton( 200, 165, 210, 211, 40 )
	gateGump.addText( 225, 190, "Umbra Bridge" )
	gateGump.addRadioButton( 200, 190, 210, 211, 41 )
	gateGump.addText( 225, 215, "Stables of Umbra" )
	gateGump.addRadioButton( 200, 215, 210, 211, 42 )

	# tokuno islands
	gateGump.startPage( 5 )
	gateGump.addBackground( 9200, 380, 280 )
	gateGump.addText( 5, 10, "Pick your destination:" )
	gateGump.addButton( 10, 210, 0xFA5, 0xFA7, 1 )
	gateGump.addText( 45, 210, "OKAY", 0 )
	gateGump.addButton( 10, 235, 0xFA5, 0xFA7, 0 )
	gateGump.addText( 45, 235, "CANCEL" )
	# facets
	gateGump.addText( 35, 35, "Trammel" )
	gateGump.addPageButton( 10, 35, 0x4B9, 0x4BA, 1 )
	gateGump.addText( 35, 60, "Felucca" )
	gateGump.addPageButton( 10, 60, 0x4B9, 0x4BA, 2 )
	gateGump.addText( 35, 85, "Ilshenar", 61 )
	gateGump.addPageButton( 10, 85, 0x4B9, 0x4BA, 3 )
	gateGump.addText( 35, 110, "Malas" )
	gateGump.addPageButton( 10, 110, 0x4B9, 0x4BA, 4 )
	gateGump.addText( 35, 135, "Tokuno Islands" )
	gateGump.addPageButton( 10, 135, 0x4B9, 0x4BA, 5 )
	#gates
	gateGump.addText( 225, 40, "Makoto-Jima" )
	gateGump.addRadioButton( 200, 40, 210, 211, 43 )
	gateGump.addText( 225, 65, "Isamu-Jima" )
	gateGump.addRadioButton( 200, 65, 210, 211, 44 )
	gateGump.addText( 225, 90, "Homare-Jima" )
	gateGump.addRadioButton( 200, 90, 210, 211, 45 )


	# set callback function and its arguments
	gateGump.setCallback( gateCallback )
	# send it
	gateGump.setArgs( [ item ] )

	gateGump.setType( 0x87654321 )

	gateGump.send( char )

# callback function
def gateCallback( char, args, target ):
	if( len( args ) < 1 ):
		char.socket.sysmessage( "script error 1. contact GM." )
		return 1
	item = args[0]
	if not item and not char.gm :
		char.socket.sysmessage( "script error 2. contact GM." )
		return 1
	if not char.gm and ( char.distanceto( item ) > 3):
		char.socket.sysmessage( "You are too far away to use it." )
		return 1

	if( target.button != 1 ):
		char.socket.sysmessage( "Canceled." )
		return 1
	if( len( target.switches ) == 0 ):
		return 1

	button = int( target.switches[0] )

	# set world number
	numWorld = 0
	if( ( button > 9 ) and ( button < 18 ) ):
		numWorld = 1
	elif ( ( button > 17 ) and ( button < 26 ) ):
		numWorld = 0
	elif( ( button > 25 ) and ( button < 35 ) ):
		numWorld = 2
	elif( ( button > 34 ) and ( button < 43 ) ):
		numWorld = 3
	elif( ( button > 42 ) and ( button < 46 ) ):
		numWorld = 3
	else:
		char.socket.sysmessage( "script error 4. contact GM." )
		char.socket.sysmessage( "button = %i" % button )
		return 1

	# teleport
	coord = []
	oldcoord = [ char.pos.x, char.pos.y, char.pos.z ]
	if( ( button == 10 ) or ( button == 18 ) ):
		coord = [ 1336, 1998, 5, numWorld ]
	elif( ( button == 11 ) or ( button == 19 ) ):
		coord = [ 3564, 2141, 33, numWorld ]
	elif( ( button == 12 ) or ( button == 20 ) ):
		coord = [ 4467, 1284, 5, numWorld ]
	elif( ( button == 13 ) or ( button == 21 ) ):
		coord = [ 643, 2068, 5, numWorld ]
	elif( ( button == 14 ) or ( button == 22 ) ):
		coord = [ 1828, 2949, -20, numWorld ]
	elif( ( button == 15 ) or ( button == 23 ) ):
		coord = [ 2701, 694, 4, numWorld ]
	elif( ( button == 16 ) or ( button == 24 ) ):
		coord = [ 771, 754, 4, numWorld ]
	elif( ( button == 17 ) or ( button == 25 ) ):
		coord = [ 1499, 3772, 5, numWorld ]
	elif( button == 26 ):
		coord = [ 1216, 468, -13, 2 ]
	elif( button == 27 ):
		coord = [ 722, 1364, -60, 2 ]
	elif( button == 28 ):
		coord = [ 750, 724, -28, 2 ]
	elif( button == 29 ):
		coord = [ 282, 1015, 0, 2 ]
	elif( button == 30 ):
		coord = [ 987, 1010, -32, 2 ]
	elif( button == 31 ):
		coord = [ 1174, 1286, -30, 2 ]
	elif( button == 32 ):
		coord = [1532, 1340, -4, 2 ]
	elif( button == 33 ):
		coord = [ 528, 219, -42, 2 ]
	elif( button == 34 ):
		coord = [ 1721, 218, 96, 2 ]
	elif( button == 35 ):
		coord = [ 1015, 527, -65, 3 ]
	elif( button == 36 ):
		coord = [ 991, 519, -50, 3 ]
	elif( button == 37 ):
		coord = [ 1023, 503, -70, 3 ]
	elif( button == 38 ):
		coord = [ 989, 595, -90, 3 ]
	elif( button == 39 ):
		coord = [ 961, 638, -90, 3 ]
	elif( button == 40 ):
		coord = [ 1997, 1386, -85, 3 ]
	elif( button == 41 ):
		coord = [ 1941, 1321, -88, 3 ]
	elif( button == 42 ):
		coord = [ 1992, 1326, -90, 3 ]
	elif( button == 43 ):
		coord = [ 718, 1159, 25, 4 ]
	elif( button == 44 ):
		coord = [ 1169, 998, 41, 4 ]
	elif( button == 45 ):
		coord = [ 270, 628, 15, 4 ]
	char.soundeffect( 0x1fc )
	char.removefromview()
	pos = wolfpack.coord( coord[0], coord[1], coord[2], coord[3] )
	if not isValidPosition( pos ):
		char.socket.sysmessage( "Destination Invalid!" )
		return False
	char.moveto( pos )
	char.update()
	char.soundeffect( 0x1fc )
	char.effect( 0x372a )
	# move followers
	followers = char.followers
	if( len( followers ) > 0 ):
		for i in range( 0, len( char.followers ) ):
			follower = char.followers[i]
			# only transport follower which is within 5 tile from the character
			if( char.distanceto( follower ) < 5 ):
				follower.removefromview()
				follower.moveto( pos )
				follower.update()
				follower.effect( 0x372a )
			# else it will not follow him/her
			else:
				char.removefollower( follower )

	char.socket.resendplayer()
	char.socket.resendworld()
	return 1
