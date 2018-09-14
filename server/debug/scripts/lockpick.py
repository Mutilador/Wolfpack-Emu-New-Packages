
import wolfpack
from wolfpack.consts import LOCKPICKING
import random

def response(char, args, target):
	lockpick = wolfpack.finditem(args[0])

	if lockpick.getoutmostchar() != char:
		char.socket.clilocmessage(500364)
	elif not char.cansee(lockpick) or not char.canpickup(lockpick):
		char.socket.clilocmessage(500312)
	else:
		if not target.item or not target.item.hasscript( 'lock' ):
			char.socket.clilocmessage(502069)
		elif not char.canreach(target.item, 3):
			char.socket.clilocmessage(502070)
		else:
			lock = None

			if target.item.hastag('lock'):
				lock = str(target.item.gettag('lock'))

			# Getting the difficult to lockpick
			if target.item.hastag('lockpick_difficult'):
				difficult = int(target.item.gettag('lockpick_difficult'))
			else:
				difficult = 0

			if char.skill[LOCKPICKING] < difficult:
				char.socket.clilocmessage(502075) # You are unable to pick the lock.
			elif target.item.secured:
				char.socket.clilocmessage(502071) # That is secure
			elif lock == 'magic':
				char.socket.clilocmessage(502073) # This lock cannot be picked by normal means.
			elif lock != 'lockpick':
				char.socket.clilocmessage(502072) # You don't see how that lock can be manipulated.
			else:
				if char.checkskill(LOCKPICKING, difficult, 1200):
					# SUCCESS: Remove lockpick and the lock
					target.item.removescript( 'lock' )
					target.item.deltag('lock')
					target.item.deltag('locked')
					target.item.resendtooltip()

					if lockpick.amount == 1:
						lockpick.delete()
					else:
						lockpick.amount = lockpick.amount - 1
						lockpick.update()

					char.socket.clilocmessage(502076)
					char.soundeffect(0x241)
				else:
					# FAILURE: 50% chance of loosing item
					if random.randint(0,1):
						# Lost the lockpick
						if lockpick.amount == 1:
							lockpick.delete()
						else:
							lockpick.amount = lockpick.amount - 1
							lockpick.update()

						char.socket.clilocmessage(502074)
					else:
						char.socket.clilocmessage(502075)

def onUse(char, lockpick):
	if lockpick.getoutmostchar() != char:
		char.socket.clilocmessage(500364)
	elif not char.cansee(lockpick) or not char.canpickup(lockpick):
		char.socket.clilocmessage(500312)
	else:
		char.message(502067)
		char.socket.clilocmessage(502068)
		char.socket.attachtarget("lockpick.response", [lockpick.serial])

	return 1
