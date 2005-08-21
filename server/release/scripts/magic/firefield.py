
import wolfpack
from wolfpack.utilities import energydamage

#
# Attach a tag to the character to note that he entered a firefield.
#
def onCollide(char, item):
	if char.dead or char.invulnerable or char.region.safe:
		return

	if not char.hastag( 'in_firefield' ):
		source = None
		if item.hastag('source'):
			source = wolfpack.findchar( int( item.gettag('source') ) )

		char.settag( 'in_firefield', 1)
		energydamage( char, source, 2, fire=100 )
		char.addtimer( 350, expire, [] )
	return 0

#
# See if we're still in a firefield.
#
def expire(char, arguments):
	if char.dead:
		char.deltag( 'in_firefield' )
		return

	items = wolfpack.items(char.pos.x, char.pos.y, char.pos.map, 0)

	for item in items:
		if item.hasscript( 'magic.firefield' ):
			source = None
			if item.hastag('source'):
				source = wolfpack.findchar( int( item.gettag('source') ) )

			energydamage(char, source, 2, fire=100)
			char.addtimer(350, expire, [])
			char.soundeffect(0x208)
			return

	char.deltag( 'in_firefield' )
