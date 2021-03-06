#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Speech Script for Pets                                        #
#===============================================================#

from wolfpack.consts import *
import wolfpack
import random
from wolfpack import tr
from wolfpack.utilities import hex2dec
import math

# This causes some strange error... O_o
#import food

# When pet's loyalty is decreased
CHECKINTERVAL = 60 * 60 * 1000 

#check if the pet does follow the order 
#Formula from http://uo.stratics.com/content/professions/taming/taming-lore.shtml#loyalty
#return true if pet follow the order
#false if the refuses to follow the order
def checkPetControl(pet, char, text, keywords):
	#GMs can control everything
	if char.gm:
		return True

	if pet.mintaming <= 291:
		return True

	# summoned pets can always be controlled
	if pet.summoned:
		return True

	taming = char.skill[TAMING]
	lore = char.skill[ANIMALLORE]
	diff = pet.mintaming
	weighted = ( taming * 4 + lore) / 5
	bonus = weighted - diff

	if bonus <= 0:
		chance = 700 + (bonus * 14)
	else:
		chance = 700 + (bonus * 6)

	if chance >= 0 and chance < 200:
		chance = 200
	elif chance > 990:
		chance = 990

	loyaltyValue = 1
	if pet.hastag('loyalty') and pet.gettag('loyalty') > 1:
		loyaltyValue = int(pet.gettag('loyalty') - 1) * 10

	chance -= ((100 - loyaltyValue) * 10)

	if random.randint(1,100) <= chance:
		return True
	else:
		pet.sound(SND_STARTATTACK)
		if pet.bodytype == 3: # is animal
			pet.action( 10 )
		elif pet.bodytype == 1: # is monster
			pet.action( 18 )
		return False

def friends(pet):
	friend = []
	for tag in pet.tags:
		if tag.startswith( "friend_" ):
			char = wolfpack.findchar( int(tag.split("_")[1]) )
			# always check on deleted chars every time friends(pet) is called
			if not char:
				pet.deltag(tag)
			friend.append(char)
	return friend

def isPetFriend(char, pet):
	if pet.hastag('friend_' + str(char.serial)):
		return True
	return False

def AllowNewPetFriend(pet):
	return len(friends(pet)) < 5 # your pet can have max. 4 friends

def stopfight(pet):
	pet.attacktarget = None
	pet.war = 0
	pet.updateflags()

def startfight(pet, targetchar):
	pet.attacktarget = targetchar
	pet.war = 1
	pet.updateflags()

def come(char, pet, all=False):
	if all:
		if char.gm:
			pos = char.pos
			pets = wolfpack.chars(pos.x, pos.y, pos.map, 18, False)
			for follower in pets:
				if follower.npc and not follower.frozen:
					come(char, follower, False)
		else:
			for follower in char.followers:
				come(char, follower, False)
	elif pet and (pet.owner == char or char.gm) and pet.distanceto(char) < 18:
		stopfight(pet)
		pet.guarding = None
		pet.goto(char.pos)
		pet.sound(SND_ATTACK)

def stop(char, pet, all=False):
	if all:
		if char.gm:
			pos = char.pos
			pets = wolfpack.chars(pos.x, pos.y, pos.map, 18, False)
			for follower in pets:
				if follower.npc and not follower.frozen:
					stop(char, follower, False)
		else:
			for follower in char.followers:
				stop(char, follower, False)
	elif pet and (pet.owner == char or char.gm or isPetFriend(char, pet)) and pet.distanceto(char) < 18:
		stopfight(pet)
		pet.guarding = None
		pet.wandertype = 0
		pet.sound(SND_ATTACK)

def follow_me(char, pet, all=False):
	if all:
		if char.gm:
			pos = char.pos
			pets = wolfpack.chars(pos.x, pos.y, pos.map, 18, False)
			for follower in pets:
				if follower.npc and not follower.frozen:
					follow_me(char, follower, False)
		else:
			for follower in char.followers:
				follow_me(char, follower, False)
	if pet and (pet.owner == char or char.gm or isPetFriend(char, pet)) and pet.distanceto(char) < 18:
		stopfight(pet)
		pet.follow(char)
		pet.sound(SND_ATTACK)

def go_target(char, arguments, target):
	(pet, all) = arguments
	if all:
		if char.gm:
			pos = char.pos
			pets = wolfpack.chars(pos.x, pos.y, pos.map, 18, False)
			for follower in pets:
				if follower.npc and not follower.frozen:
					go_target(char, [follower.serial, False], target)
		else:
			for follower in char.followers:
				go_target(char, [follower.serial, False], target)
	else:
		pet = wolfpack.findchar(pet)
		if pet and (pet.owner == char or char.gm) and pet.distanceto(char) < 18:
			stopfight(pet)
			pet.guarding = None
			pet.goto(target.pos)
			pet.sound(SND_ATTACK)

def go(char, pet, all=False):
	char.socket.sysmessage(tr('Please select a destination.'))
	char.socket.attachtarget("speech.pets.go_target", [pet.serial, all])

def attack_target(char, arguments, target):
	if not target.char or target.char == char:
		char.socket.sysmessage(tr('Your pets cannot attack that.'))
		return

	(pet, all) = arguments
	if all:
		if char.gm:
			pos = char.pos
			pets = wolfpack.chars(pos.x, pos.y, pos.map, 18, False)
			for follower in pets:
				if follower.npc and not follower.frozen:
					attack_target(char, [follower.serial, False], target)
		else:
			for follower in char.followers:
				attack_target(char, [follower.serial, False], target)
	else:
		pet = wolfpack.findchar(pet)
		if target.char == pet:
			char.socket.sysmessage(tr('Your pet refuses to kill itself.'))
			return
		if pet and (pet.owner == char or char.gm) and pet.distanceto(char) < 18:
			startfight(pet, target.char)
			pet.fight(target.char)

			# right here we need to not interrupt the target, but i can't register a fight unless i do, right? not sure.
			target.char.fight(pet)
			target.char.follow(pet)

			go_target(char, arguments, target)
			follow_target(char, arguments, target)
			pet.sound(SND_ATTACK)

def attack(char, pet, all=False):
	char.socket.sysmessage(tr('Please select an attack target.'))
	char.socket.attachtarget("speech.pets.attack_target", [pet.serial, all])

def transfer_target(char, arguments, target):
	if not target.char or target.char == char or not target.char.player:
		char.socket.sysmessage(tr('You can''t transfer your pet to them.'))
		return

	pet = wolfpack.findchar(arguments[0])

	if pet and (pet.owner == char or char.gm):
		if (target.char.controlslots + pet.controlslots) > target.char.maxcontrolslots:
			char.socket.sysmessage(tr('Your target already controls too many pets.'))
			return

		pet.owner = target.char
		# Tame during transfer if gm
		if char.gm and not pet.tamed:
			pet.tamed = True
			pet.resendtooltip()
		pet.sound(SND_ATTACK)
		char.socket.sysmessage(tr('You transfer your pet to %s.') % target.char.name)

def transfer(char, pet):
	if not char.gm and pet.summoned:
		return

	char.socket.sysmessage(tr('Who do you want to transfer your pet to?'))
	char.socket.attachtarget("speech.pets.transfer_target", [pet.serial])

def addfriend_target(char, arguments, target):
	pet = wolfpack.findchar(arguments[0])

	if not target.char or not target.char.player or target.char == char:
		pet.say(502039, '', '', 0, 0x3b2) # *looks confused*

	#
	#if char.young and not target.young:
	#	char.socket.clilocmessage( 502040 ) # As a young player, you may not friend pets to older players.
	#elif not char.young and target.young:
	#	char.socket.clilocmessage( 502041 ) # As an older player, you may not friend pets to young players.
	#

	if pet:
		target = target.char
		if char.hasscript('system.trading'):
			char.socket.clilocmessage( 1070947 ) # You cannot friend a pet with a trade pending
		elif target.hasscript('system.trading'):
			target.socket.clilocmessage( 1070947 ) # You cannot friend a pet with a trade pending
		elif isPetFriend(target, pet):
			char.socket.clilocmessage( 1049691 ) # That person is already a friend.
		elif not AllowNewPetFriend(pet):
			char.socket.clilocmessage( 1005482 ) # Your pet does not seem to be interested in making new friends right now.
		else:
			# ~1_NAME~ will now accept movement commands from ~2_NAME~.
			char.socket.clilocmessage( 1049676, "%s\t%s" % (unicode(pet.name), unicode(target.name)))

			# ~1_NAME~ has granted you the ability to give orders to their pet ~2_PET_NAME~.
			# This creature will now consider you as a friend.
			target.socket.clilocmessage( 1043246, "%s\t%s" % (unicode(char.name), unicode(pet.name)) )

			doaddfriend(target, pet)
			return True

	pet.follow(char)
	return True

def doaddfriend(newfriend, pet):
	pet.settag('friend_' + str(newfriend.serial), 0)
	pet.follow(newfriend)
	return True

def addfriend(char, pet):
	char.socket.attachtarget("speech.pets.addfriend_target", [pet.serial])
	return

def removefriend_target(char, arguments, target):
	pet = wolfpack.findchar(arguments[0])

	if not target.char or not target.char.player or target.char == char:
		pet.say(502039, '', '', 0, 0x3b2) # *looks confused*
	elif not isPetFriend(target.char, pet):
		char.socket.clilocmessage( 1070953 ) # That person is not a friend.
	else:
		# ~1_NAME~ will no longer accept movement commands from ~2_NAME~.
		char.socket.clilocmessage( 1070951, "%s\t%s" % (unicode(pet.name), unicode(target.name)))

		# ~1_NAME~ has no longer granted you the ability to give orders to their pet ~2_PET_NAME~.
		# This creature will no longer consider you as a friend.
		target.char.socket.clilocmessage( 1070952, "%s\t%s" % (unicode(char.name), unicode(pet.name)) )

		doremovefriend(oldfriend, pet)

	pet.follow(char)
	return True

def doremovefriend(oldfriend, pet):
	pet.deltag('friend_' + str(oldfriend.serial))
	return True

def removefriend(char, pet):
	char.socket.attachtarget("speech.pets.removefriend_target", [pet.serial])
	return

def follow_target(char, arguments, target):
	if not target.char:
		char.socket.sysmessage(tr('Your pets can only follow characters.'))
		return

	(pet, all) = arguments
	pet = wolfpack.findchar(pet)
	if all:
		if char.gm:
			pos = char.pos
			pets = wolfpack.chars(pos.x, pos.y, pos.map, 18, False)
			for follower in pets:
				if follower.npc and not follower.frozen:
					follow_target(char, [follower.serial, False], target)
		else:
			for follower in char.followers:
				follow_target(char, [follower.serial, False], target)
	elif pet and (pet.owner == char or char.gm or isPetFriend(char, pet)) and pet.distanceto(char) < 18:
		pet.guarding = None
		pet.follow(target.char)
		pet.sound(SND_ATTACK)

def follow(char, pet, all=False):
	if all:
		char.socket.sysmessage(tr('Who do you want your pets to follow?'))
	else:
		char.socket.sysmessage(tr('Who do you want your pet to follow?'))

	char.socket.attachtarget("speech.pets.follow_target", [pet.serial, all])

def drop(pet):
	if pet.summoned:
		return True
	backpack = pet.getbackpack()
	if backpack:
		for item in backpack.content:
			item.container = 0
			item.pos = pet.pos
			item.update()
	return True

def release(pet):
	if pet.summoned:
		pet.delete()
		return

	pet.owner = None
	if pet.tamed:
		pet.tamed = False
	pet.removescript('speech.pets') # Remove this script if it was added dynamically
	pet.wandertype = 1
	pet.sound(SND_ATTACK)

def guard(char, pet, all=False):
	master = pet.owner
	if not master:
		return True

	aggressors = master.getopponents()
	combatant = None
	if len(aggressors) > 0:
		if all:
			for pet in char.followers:
				if pet.distanceto(char) >= 18:
					continue
				master = pet.owner
				if not master:
					continue
				combatant = pet.attacktarget
				for attacker in aggressors:
					if attacker and GetDistanceToSqrt( attacker, pet ) <= pet.getintproperty('range_perception', 16):
						if not combatant or GetDistanceToSqrt( attacker, master ) < GetDistanceToSqrt( combatant, master ):
							combatant = attacker
				if combatant and combatant != pet and combatant != master and not combatant.dead and pet.cansee( combatant ) and combatant.pos.map == pet.pos.map:
					pet.fight( combatant )
				else:
					pet.war = False
					pet.updateflags()
		else:
			combatant = pet.attacktarget
			for attacker in aggressors:
				if attacker and GetDistanceToSqrt( attacker, pet ) <= pet.getintproperty('range_perception', 16):
					if not combatant or GetDistanceToSqrt( attacker, master ) < GetDistanceToSqrt( combatant, master ):
						combatant = attacker

			if combatant and combatant != pet and combatant != master and not combatant.dead and pet.cansee( combatant ) and combatant.pos.map == pet.pos.map:
				pet.fight( combatant )
			else:
				pet.war = False
				pet.updateflags()
	return True

def GetDistanceToSqrt( attacker, mobile ):
	xDelta = attacker.pos.x - mobile.pos.x
	yDelta = attacker.pos.y - mobile.pos.y

	return math.sqrt( (xDelta * xDelta) + (yDelta * yDelta) )

def patrol(char, pet):
	char.socket.sysmessage("This order is not yet coded")
	return True

def onSpeech(pet, char, text, keywords):
	if not char.socket:
		return False

	text = text.lower()

	# Test All
	# Check for keywords
	all = text.startswith('all ')

	# Check if event must be continued
	if not all and not text.startswith(pet.name):
		return False

	#check if can be controlled
	if not checkPetControl(pet, char, text, keywords):
		return False

	if all:
		# Test Ownership / Allow Friends/GMs to control
		if (pet.owner != char or not pet.tamed) and not char.gm:
			return False

		# begin all #
		# all guard
		if 358 in keywords:
			guard(char, pet, True)
			return True
		# All Follow Me
		elif 232 in keywords and 355 in keywords and 364 in keywords:
			follow_me(char, pet, True)
			return True
	
		# All Follow
		elif 232 in keywords and 346 in keywords and 357 in keywords:
			follow(char, pet, True)
			return True
		
		# All Kill, All Attack
		elif (360 in keywords and 349 in keywords) or (361 in keywords and 350 in keywords):
			attack(char, pet, True)
			return True
		# all guard me
		elif 363 in keywords:
			guard(char, pet, True)
			return True
		# All Come (356)
		elif 356 in keywords and 341 in keywords:
			come(char, pet, True)
			return True

		# All Stay, All Stop
		elif (368 in keywords and 367 in keywords) or (359 in keywords and 353 in keywords):
			stop(char, pet, True)
			return True

		# All Go
		elif text == 'all go':
			go(char, pet, True)
			return True

		# end all #
	else:
		# begin not all #
		# Test Name if not all, return false if false
		if not text.startswith(pet.name.lower() + " "):
			return False

		# Test Ownership / Allow Friends/GMs to control
		elif not char.gm and ((pet.owner != char) and not isPetFriend(char, pet) or (not pet.tamed)):
			return False

		# Follow Me
		elif 232 in keywords and 355 in keywords:
			follow_me(char, pet, False)
			return True

		# Stay, Stop
		elif 353 in keywords or 367 in keywords:
			stop(char, pet, False)
			return True

		# Follow
		elif 232 in keywords and 346 in keywords:
			follow(char, pet, False)
			return True

		if isPetFriend(char, pet):
			return False

		# Kill, Attack
		elif 349 in keywords or 350 in keywords:
			attack(char, pet, False)
			return True

		# Come
		if 341 in keywords:
			come(char, pet, False)
			return True

		# Drop
		elif 342 in keywords:
			drop(pet)
			return True

		# Friend
		elif 347 in keywords:
			addfriend(char, pet)
			return True

		# Guard
		elif 348 in keywords:
			guard(char, pet, False)
			return True

		# Patrol
		elif 351 in keywords:
			patrol(char, pet)
			return True

		# Release
		elif 365 in keywords:
			release(pet)
			return True

		# Transfer
		elif 366 in keywords:
			transfer(char, pet)
			return True

		# Go
		elif text.endswith('go'):
			go(char, pet, False)
			return True
		# end some #

	return False

#def onDropOnChar(char, item):
#	if item.baseid == "eed":
#		food.ischecked(char, item)
#		return True
#	return False

def onLoad():
	global magic_pets
	magic_pets = random.random()
	wolfpack.addtimer( CHECKINTERVAL, loyaltyexp, [magic_pets], False )

def onUnload():
	global magic_pets
	magic_pets = random.random()

def loyaltyexp(obj, args):
	if args[0] != magic_pets:
		return # The scripts have been reloaded

	wolfpack.addtimer( CHECKINTERVAL, loyaltyexp, [magic_pets], False )

	iterator = wolfpack.chariterator()
	char = iterator.first
	while char:
		if not char.npc:
			# Do not forget the iteration step
			char = iterator.next
			continue
		# Access char properties here
		if char.tamed:
			char.hunger = 20
			if not char.hastag('loyalty'):
				char.settag('loyalty', 11)
			loyalty = char.gettag('loyalty')
			owner = char.owner
			if owner and owner.gm:
				return False

			if loyalty > 0 and char.pos.map != 0xFF:
				torelease = False
				loyalty_new = loyalty - 1
				if loyalty_new < 1:
					torelease = True
				else:
					char.settag('loyalty', loyalty_new)
				if loyalty_new == 1: # Confused
					char.say(1043270, unicode(char.name) ) # * ~1_NAME~ looks around desperately *
					char.sound(SND_IDLE)

				if torelease:
					char.say( 1043255, unicode(char.name) ) # ~1_NAME~ appears to have decided that is better off without a master!
					char.settag('loyalty', 11) # Wonderfully happy
	#				#c.IsBonded = false;
	#				#c.BondingBegin = DateTime.MinValue;
	#				#c.OwnerAbandonTime = DateTime.MinValue;
					release(char)
		# not tamed
		else:
			# Animals in houses/multis
			if char.multi:
				if not char.dead:
					if char.hastag("removestep"):
						if char.gettag("removestep") >= 20:
							char.delete()
						else:
							char.settag("removestep", char.gettag("removestep") + 1)
					else:
						char.settag("removestep", 1)		
			else:
				if char.hastag("removestep"):
					char.deltag("removestep")
		char = iterator.next

	return
