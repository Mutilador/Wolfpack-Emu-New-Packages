
import wolfpack
from wolfpack.consts import *

dirs = {
	0: [0, -1],
	1: [1, -1],
	2: [1, 0],
	3: [1, 1],
	4: [0, 1],
	5: [-1, 1],
	6: [-1, 0],
	7: [-1, -1]
}

def onLoad():
	wolfpack.registerpackethook( 0x12, openDoor )

def openDoor( socket, packet ):
	if packet.size != 5 and packet.getShort(3) != 0x5800:
		return 0

	if not socket.player:
		return 0

	char = socket.player
	dir = char.direction
	doors = wolfpack.items(char.pos.x + dirs[dir][0], char.pos.y + dirs[dir][1], char.pos.map, 0)

	if not doors:
		return 0

	reach = 0
	for door in doors:
		if char.pos.z == door.pos.z:
			reach = 1
		elif char.pos.z < door.pos.z and char.pos.z >= ( door.pos.z - 5):
			reach = 1
		elif char.pos.z > door.pos.z and char.pos.z <= ( door.pos.z + 5):
			reach = 1
		if reach == 1:
			events = door.events
			for event in events:
				if event == 'door':
					opendoor = 1
					break
		if opendoor == 1:
			wolfpack.callevent( event, EVENT_USE, (char, door) )
			break
	return
