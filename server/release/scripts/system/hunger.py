
import wolfpack
import random
import wolfpack.time
from wolfpack import tr

hungerrate = wolfpack.settings.getnumber("Game Speed", "Hunger Rate", 120) # Every 120 minutes
hungerdamage = wolfpack.settings.getnumber("General", "Hunger Damage", 0) # Damage?

def onTimeChange( player ):
	global hungerrate
	global hungerdamage

	# Use the minutes
	lasthunger = 0
	if player.hastag('lasthunger'):
		lasthunger = int(player.gettag('lasthunger'))
		
	if lasthunger + hungerrate < wolfpack.time.minutes():
		if player.socket and not player.gm:
			if player.hunger <= 8:
				player.socket.sysmessage( tr("Your stomach growls...") )
			if player.hunger >= 1 and player.hunger <= 20:
				player.hunger -= 1
			elif player.hunger == 0:
				player.damage( 3, random.randint( 0, hungerdamage ) )
				player.socket.sysmessage( tr("Your stomach hurts from the lack of food...") )
			player.settag('lasthunger', wolfpack.time.minutes())
	return False
