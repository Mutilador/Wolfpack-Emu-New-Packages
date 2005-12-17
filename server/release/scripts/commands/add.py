
import wolfpack
from wolfpack.consts import *
from system.makemenus import MakeItemAction, MakeNPCAction, MakeMenu, MakeAction, findmenu
from wolfpack.utilities import hex2dec
from wolfpack import tr

generated = 0

#
# Target response for adding a npc
#
def addnpc(player, arguments, target):
	if target.item and target.item.container:
		player.socket.sysmessage("You can't add the npc there.")
		return

	npc = wolfpack.addnpc(str(arguments[0]), target.pos)
	npc.update()

	player.log(LOG_MESSAGE, "Adds npc %s (0x%x) at %s.\n" % (str(arguments[0]), npc.serial, str(target.pos)))

#
# Target response for adding a multi
#
def addmulti(player, arguments, target):
	if target.item and target.item.container:
		player.socket.sysmessage("You can't add the multi there.")
		return

	multi = wolfpack.addmulti(str(arguments[0]))

	# If arguments[1] is true, make the item
	# static
	if arguments[1]:
		multi.decay = False
		multi.movable = 3

	multi.moveto(target.pos)
	multi.update()
	
	player.log(LOG_MESSAGE, "Adds multi %s (0x%x) at %s.\n" % (str(arguments[0]), multi.serial, target.pos))

#
# Target response for adding an item
#
def additem(player, arguments, target):
	item = wolfpack.additem(str(arguments[0]))

	# If arguments[1] is true, make the item
	# static
	if arguments[1]:
		item.decay = False
		item.movable = 3

	if target.item:
		if target.item.type == 1:
			target.item.additem(item, 1, 1, 0)

			if arguments[1]:
				player.log(LOG_MESSAGE, "Adds static item %s (0x%x) into 0x%x.\n" % (str(arguments[0]), item.serial, target.item.serial))
			else:
				player.log(LOG_MESSAGE, "Adds item %s (0x%x) into 0x%x.\n" % (str(arguments[0]), item.serial, target.item.serial))
		elif target.item.container:
			if target.item.layer:
				return False
			target.item.container.additem(item, 1, 1, 0)

			if arguments[1]:
				player.log(LOG_MESSAGE, "Adds static item %s (0x%x) into 0x%x.\n" % (str(arguments[0]), item.serial, target.item.container.serial))
			else:
				player.log(LOG_MESSAGE, "Adds item %s (0x%x) into 0x%x.\n" % (str(arguments[0]), item.serial, target.item.container.serial))
		else:
			item.moveto(target.item.pos)

			if arguments[1]:
				player.log(LOG_MESSAGE, "Adds static item %s (0x%x) at %s.\n" % (str(arguments[0]), item.serial, str(target.item.pos)))
			else:
				player.log(LOG_MESSAGE, "Adds item %s (0x%x) at %s.\n" % (str(arguments[0]), item.serial, str(target.item.pos)))			
	elif target.char:
		item.moveto(target.char.pos)

		if arguments[1]:
			player.log(LOG_MESSAGE, "Adds static item %s (0x%x) at %s.\n" % (str(arguments[0]), item.serial, str(target.char.pos)))
		else:
			player.log(LOG_MESSAGE, "Adds item %s (0x%x) at %s.\n" % (str(arguments[0]), item.serial, str(target.char.pos)))
	else:
		item.moveto(target.pos)

		if arguments[1]:
			player.log(LOG_MESSAGE, "Adds static item %s (0x%x) at %s.\n" % (str(arguments[0]), item.serial, str(target.pos)))
		else:
			player.log(LOG_MESSAGE, "Adds item %s (0x%x) at %s.\n" % (str(arguments[0]), item.serial, str(target.pos)))

	item.update()

#
# Add an item and make it nodecay + movable 2
#
def static(socket, command, arguments):
	if len(arguments) > 0:
		if wolfpack.getdefinition(WPDT_ITEM, arguments):
			socket.sysmessage("Where do you want to place the item '%s'?" % arguments)
			socket.attachtarget("commands.add.additem", [arguments, True])
		elif wolfpack.getdefinition(WPDT_MULTI, arguments):
			node = wolfpack.getdefinition(WPDT_MULTI, arguments)
			count = node.childcount
			for i in range(0, count):
				subnode = node.getchild(i)
				if subnode.name == 'id': # Found the display id
					dispid = hex2dec(subnode.value)
			socket.sysmessage("tet")
			socket.sysmessage("Where do you want to place the multi '%s'?" % arguments)
			socket.attachmultitarget("commands.add.addmulti",  dispid - 0x4000, [arguments, True], 0, 0, 0)
		else:
			socket.sysmessage('No Item, NPC or Multi definition by that name found.')
	else:
		socket.sysmessage('Usage: static <id>')

#
# Add an item or character or
# show the addmenu.
#
def add(socket, command, arguments):
	if len(arguments) > 0:
		if wolfpack.getdefinition(WPDT_ITEM, arguments):
			socket.sysmessage("Where do you want to place the item '%s'?" % arguments)
			socket.attachtarget("commands.add.additem", [arguments, False])
		elif wolfpack.getdefinition(WPDT_NPC, arguments):
			socket.sysmessage("Where do you want to spawn the npc '%s'?" % arguments)
			socket.attachtarget("commands.add.addnpc", [arguments])
		elif wolfpack.getdefinition(WPDT_MULTI, arguments):
			node = wolfpack.getdefinition(WPDT_MULTI, arguments)
			count = node.childcount
			for i in range(0, count):
				subnode = node.getchild(i)
				if subnode.name == 'id': # Found the display id
					dispid = hex2dec(subnode.value)
			socket.sysmessage("Where do you want to place the multi '%s'?" % arguments)
			socket.attachmultitarget("commands.add.addmulti",  dispid - 0x4000, [arguments, False], 0, 0, 0)
		else:
			socket.sysmessage('No Item, NPC or Multi definition by that name found.')
		return

	global generated
	if not generated:
		generated = True
		socket.sysmessage('Generating add menu.')
		socket.sysmessage('Please wait...')
		generateAddMenu(socket.player.serial)
		return

	menu = findmenu('ADDMENU')
	if menu:
		menu.send(socket.player)
	else:
		socket.sysmessage('No ADDMENU menu found.')

#
# This action creates a npc.
#
class AddNpcAction(MakeNPCAction):
	def __init__(self, parent, title, itemid, definition):
		MakeNPCAction.__init__(self, parent, title, itemid, definition)
		self.definition = definition
		self.description = title
		self.other = ''
		self.materials = ''
		self.skills = ''

	def make(self, player, arguments, nodelay=0):
		player.socket.sysmessage("Where do you want to spawn the npc '%s'?" % self.definition)
		player.socket.attachtarget("commands.add.addnpc", [self.definition])
		MakeAction.make(self, player, arguments, nodelay)

#
# This action creates an item
#
class AddItemAction(MakeItemAction):
	def __init__(self, parent, title, itemid, definition):
		MakeItemAction.__init__(self, parent, title, itemid, definition)

	def make(self, player, arguments, nodelay=0):
		player.socket.sysmessage("Where do you want to place the item '%s'?" % self.definition)
		player.socket.attachtarget("commands.add.additem", [self.definition, False])
		MakeAction.make(self, player, arguments, nodelay)

#
# This action creates a multi
#
class AddMultiAction(MakeItemAction):
	def __init__(self, parent, title, itemid, definition):
		MakeItemAction.__init__(self, parent, title, itemid, definition)

	def make(self, player, arguments, nodelay=0):
		node = wolfpack.getdefinition(WPDT_MULTI, self.definition)
		count = node.childcount
		for i in range(0, count):
			subnode = node.getchild(i)
			if subnode.name == 'id': # Found the display id
				dispid = hex2dec(subnode.value)
		player.socket.sysmessage("Where do you want to place the multi '%s'?" % self.definition)
		player.socket.attachmultitarget("commands.add.addmulti",  dispid - 0x4000, [self.definition, False], 0, 0, 0)
		MakeAction.make(self, player, arguments, nodelay)

#
# Generate a menu structure out of the
# category tags of our item definitions.
#
def generateAddMenu(serial = 0, items = None):
	char = wolfpack.findchar(serial)
	if not char or not char.socket:
		char = None

	"""if not items:
		items = wolfpack.getdefinitions(WPDT_ITEM)
		if char:
			char.socket.sysmessage('Done getting list of definitions.')
		wolfpack.queuecode(generateAddMenu, (serial, items))
		return"""

	addmenu= MakeMenu('ADDMENU', None, 'Add Menu')
	submenus = {}

	# Process 100 at a time
	definitions = wolfpack.definitionsiterator(WPDT_ITEM)
	item = definitions.first
	while item:
		if not item.hasattribute('id'):
			item = definitions.next
			continue

		child = item.findchild('category')
		if not child:
			item = definitions.next
			continue

		categories = ['Items'] + child.text.split('\\')
		description = categories[len(categories)-1] # Name of the action
		categories = categories[:len(categories)-1]

		# Iterate trough the categories and see if they're all there
		category = ''
		if len(categories) > 0 and not submenus.has_key('\\'.join(categories) + '\\'):
			for subcategory in categories:
				if not submenus.has_key(category + subcategory + '\\'):
					# Category is our parent category
					parent = None
					if len(category) == 0:
						parent = addmenu
					elif submenus.has_key(category):
						parent = submenus[category]

					category += subcategory + '\\'
					menu = MakeMenu('ADDMENU_' + category, parent, subcategory)
					submenus[category] = menu
				else:
					category += subcategory + '\\'

		child = item.findchild('id')
		if child:
			try:
				id = int(child.value)
			except:
				id = 0
		else:
			id = 0
		definition = item.getattribute('id')

		# Parse the position of this makemenu entry
		if len(categories) == 0:
			additem = AddItemAction(addmenu, description, id, definition)
		else:
			additem = AddItemAction(submenus['\\'.join(categories) + '\\'], description, id, definition)
		additem.otherhtml = 'Definition: ' + definition
		item = definitions.next

	for menu in submenus.values():
		menu.sort()

	npcs = wolfpack.definitionsiterator(WPDT_NPC)
	submenus = {}

	npc = npcs.first
	while npc:
		if not npc.hasattribute('id'):
			npc = npcs.next
			continue

		child = npc.findchild('category')
		if not child:
			npc = npcs.next
			continue

		id = npc.findchild('id')
		if id:
			try:
				if id.value.startswith('0x'):
					id = wolfpack.bodyinfo(hex2dec(id.value))['figurine']
				else:
					id = wolfpack.bodyinfo(int(id.value))['figurine']
			except:
				id = 0
		else:
			id = 0

		description = npc.findchild('desc')
		if description:
			description = description.value
		else:
			description = tr('No description available.')

		categories = ['NPCs'] + child.text.split('\\')
		title = categories[len(categories)-1] # Name of the action
		categories = categories[:len(categories)-1]

		# Iterate trough the categories and see if they're all there
		category = ''
		if len(categories) > 0 and not submenus.has_key('\\'.join(categories) + '\\'):
			for subcategory in categories:
				if not submenus.has_key(category + subcategory + '\\'):
					# Category is our parent category
					parent = None
					if len(category) == 0:
						parent = addmenu
					elif submenus.has_key(category):
						parent = submenus[category]

					category += subcategory + '\\'
					menu = MakeMenu('ADDMENU_' + category, parent, subcategory)
					submenus[category] = menu
				else:
					category += subcategory + '\\'

		definition = npc.getattribute('id')

		# Parse the position of this makemenu entry
		if len(categories) == 0:
			addnpc = AddNpcAction(addmenu, title , definition, definition)
		else:
			addnpc = AddNpcAction(submenus['\\'.join(categories) + '\\'], title, id, definition)
		addnpc.otherhtml = str(description)
		npc = npcs.next

	for menu in submenus.values():
		menu.sort()

	multis = wolfpack.definitionsiterator(WPDT_MULTI)
	submenus = {}

	multi = multis.first
	while multi:
		if not multi.hasattribute('id'):
			multi = multi.next
			continue

		child = multi.findchild('category')
		if not child:
			multi = multis.next
			continue

		categories = ['Multis'] + child.text.split('\\')
		description = categories[len(categories)-1] # Name of the action
		categories = categories[:len(categories)-1]

		# Iterate trough the categories and see if they're all there
		category = ''
		if len(categories) > 0 and not submenus.has_key('\\'.join(categories) + '\\'):
			for subcategory in categories:
				if not submenus.has_key(category + subcategory + '\\'):
					# Category is our parent category
					parent = None
					if len(category) == 0:
						parent = addmenu
					elif submenus.has_key(category):
						parent = submenus[category]

					category += subcategory + '\\'
					menu = MakeMenu('ADDMENU_' + category, parent, subcategory)
					submenus[category] = menu
				else:
					category += subcategory + '\\'

		child = multi.findchild('id')
		if child:
			try:
				id = int(child.value)
			except:
				id = 0
		else:
			id = 0
		multi = multi.getattribute('id')
		# Parse the position of this makemenu entry
		if len(categories) == 0:
			addmulti = AddMultiAction(addmenu, description, id, multi)
		else:
			addmulti = AddMultiAction(submenus['\\'.join(categories) + '\\'], description, id, multi)
		addmulti.otherhtml = 'Definition: ' + multi
		multi = multis.next

	for menu in submenus.values():
		menu.sort()

	addmenu.sort()
	
	if char:
		addmenu.send(char)

def onLoad():
	wolfpack.registercommand('add', add)
	wolfpack.registercommand('static', static)

def onUnload():
	global generated
	generated = 0

"""
	\command add
	\description Add a npc, item or multi and if no definition was specified, open a menu.
	\usage - <code>add npc-id</code>
	- <code>add item-id</code>
	- <code>add multi-id</code>
	- <code>add</code>
	If neither a npc nor an item id is passed to the add command, a menu with all
	categorized item and npc definitions is shown.
	The menu is automatically generated from the definitions of items and NPCs
	based on the <category> tag.
"""

"""
	\command static
	\description Add an item or multi and make it not decay and not movable.
	\usage - <code>add item-id</code>
	- <code>add multi-id</code>
"""
