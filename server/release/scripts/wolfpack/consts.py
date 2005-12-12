#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Constants used in Python scripts                              #
#===============================================================#

"""
	\library wolfpack.consts
	\description Contains several constants for scripts.
"""

# Date Format
FORMAT_DATE = '%d.%m.%Y' # European
FORMAT_TIME = '%H:%M' # European
FORMAT_DATETIME = FORMAT_DATE + ' ' + FORMAT_TIME

# Age of Shadows Stuff
# AGEOFSHADOWS, enable Age of Shadows features
# FELUCIA2XRESGAIN, double gain resources
# 1 = True || 0 = False
AGEOFSHADOWS = 1
FELUCIA2XRESGAIN = 0

# These are properties you can get via
# wolfpack.properties.fromchar or
# wolfpack.properties.fromitem
RESISTANCE_PHYSICAL = 1
RESISTANCE_ENERGY = 2
RESISTANCE_COLD = 3
RESISTANCE_POISON = 4
RESISTANCE_FIRE = 5
DAMAGE_ENERGY = 6
DAMAGE_COLD = 7
DAMAGE_POISON = 8
DAMAGE_FIRE = 9
DAMAGEBONUS = 10
SPEEDBONUS = 11
HITBONUS = 12
DEFENSEBONUS = 13
MINDAMAGE = 14
MAXDAMAGE = 15
SPEED = 16
MISSSOUND = 17
HITSOUND = 18
PROJECTILE = 19
PROJECTILEHUE = 20
AMMUNITION = 21
REQSTR = 22
REQDEX = 23
REQINT = 24
LUCK = 25
GOLDINCREASE = 26
LOWERREQS = 27
HITPOINTRATE = 28
STAMINARATE = 29
MANARATE = 30
SPELLDAMAGEBONUS = 31
MATERIALPREFIX = 32
SWING = 33
REFLECTPHYSICAL = 34
DURABILITYBONUS = 35
#WEIGHTBONUS = 36 # VACANT
SPELLCHANNELING = 37
CASTRECOVERYBONUS = 38
CASTSPEEDBONUS = 39
MAGEARMOR = 40
NIGHTSIGHT = 41
REGENHITPOINTS = 42
REGENSTAMINA = 43
REGENMANA = 44
LUCK = 45
BONUSHITPOINTS = 46
BONUSSTAMINA = 47
BONUSMANA = 48
BONUSSTRENGTH = 49
BONUSDEXTERITY = 50
BONUSINTELLIGENCE = 51
LOWERMANACOST = 52
LOWERREAGENTCOST = 53
SELFREPAIR = 54
ENHANCEPOTIONS = 55
BESTSKILL = 56
MAGEWEAPON = 57
SLAYER = 58
LIFELEECH = 59
STAMINALEECH = 60
MANALEECH = 61
SPLASHPHYSICAL = 62
SPLASHCOLD = 63
SPLASHFIRE = 64
SPLASHENERGY = 65
SPLASHPOISON = 66
HITMAGICARROW = 67
HITHARM = 68
HITFIREBALL = 69
HITLIGHTNING = 70
HITDISPEL = 71

# Checks for certain item types
ITEM_ARMOR = 1 # Armor
ITEM_WEAPON = 2 # Weapon
ITEM_SHIELD = 3 # Shield
ITEM_MELEE = 4 # Melee weapon
ITEM_RANGED = 5 # Ranged weapon

ITEM_PIERCING = 6	# fencing : type 1005
ITEM_SLASHING = 7	# sword : type 1001, 1002
ITEM_BASHING = 8	# mace : type 1003, 1004

# Money Resource Types
GOLD_COIN = 0xeed
COPPER_COIN = 0xeea
SILVER_COIN = 0xef0

# Spell Settings
CLUMSY_DURATION = 60000

# How long does it take Cotton plants to regrow (in seconds)
COTTONPLANTS_REGROW = 30

# Maximum Skills
MAX_SKILL_FOR_DUMMIE = 300

# Potion Delays
HEAL_POT_DELAY = 10.0 # 10 Seconds
AGILITY_TIME = 120.0  # 2 minutes
STRENGTH_TIME = 120.0  # 2 minutes
INTELLIGENCE_TIME = 120.0 # 2 minutes
# Potion Ranges
POTION_LESSERHEAL_RANGE = [ 3, 10]
POTION_HEAL_RANGE = [ 6, 20 ]
POTION_GREATERHEAL_RANGE = [ 9, 30 ]
POTION_LESSEREXPLOSION_RANGE = [ 1, 5 ]
POTION_EXPLOSION_RANGE = [ 6, 10 ]
POTION_GREATEREXPLOSION_RANGE = [ 11, 20 ]

# Fishing Settings
FISHING_FISH = [25, 30]	 # Min to Max fish in a 8x8 block
FISHING_REFILLTIME = [300000, 600000]  # 5 to 10 Minutes refresh time
FISHING_MAX_DISTANCE = 6 # Maximum Distance to fishing ground
FISHING_BLOCK_RANGE = 13 # How many Z-Units of free space need to be above the fishing ground

# Lumberjacking Settings
LUMBERJACKING_LOGS = [5, 10] # 5 to 10 chops = 10 or 20 per chop
LUMBERJACKING_SKILL = [0, 850]
LUMBERJACKING_REFILLTIME = [300000, 600000] # 5 to 10 Minutes Refresh time for trees
LUMBERJACKING_MAX_DISTANCE = 1 # Makes sure we are next to the tree

# Mining Settings
MINING_MAX_DISTANCE = 2
MINING_REFILLTIME = [600000, 1200000] # 10 to 20 Minutes Refresh time for trees
MINING_ORE = [10, 34]
MINING_SAND = [5, 13]

"""
	\constants wolfpack.consts Skill Constants
	These constants are shortcuts for the ids of every skill.
"""
ALCHEMY = 0
ANATOMY = 1
ANIMALLORE = 2
ITEMID = 3
ARMSLORE = 4
PARRYING = 5
BEGGING = 6
BLACKSMITHING = 7
BOWCRAFT = 8
PEACEMAKING = 9
CAMPING = 10
CARPENTRY = 11
CARTOGRAPHY = 12
COOKING = 13
DETECTINGHIDDEN = 14
ENTICEMENT = 15
EVALUATINGINTEL = 16
HEALING = 17
FISHING = 18
FORENSICS = 19
HERDING = 20
HIDING = 21
PROVOCATION = 22
INSCRIPTION = 23
LOCKPICKING = 24
MAGERY = 25
MAGICRESISTANCE = 26
TACTICS = 27
SNOOPING = 28
MUSICIANSHIP = 29
POISONING = 30
ARCHERY = 31
SPIRITSPEAK = 32
STEALING = 33
TAILORING = 34
TAMING = 35
TASTEID = 36
TINKERING = 37
TRACKING = 38
VETERINARY = 39
SWORDSMANSHIP = 40
MACEFIGHTING = 41
FENCING = 42
WRESTLING = 43
LUMBERJACKING = 44
MINING = 45
MEDITATION = 46
STEALTH = 47
REMOVETRAPS = 48
NECROMANCY = 49
FOCUS = 50
CHIVALRY = 51
BUSHIDO = 52
NINJITSU = 53
SPELLWEAVING = 54
ALLSKILLS = 55 # skills + 1
"""
	\end
"""

SKILLNAMES = [ 'alchemy', 'anatomy', 'animallore', 'itemid', 'armslore', 'parrying',
	'begging', 'blacksmithing', 'bowcraft', 'peacemaking', 'camping', 'carpentry',
	'cartography', 'cooking', 'detectinghidden', 'enticement', 'evaluatingintel',
	'healing', 'fishing', 'forensics', 'herding', 'hiding', 'provocation',
	'inscription', 'lockpicking', 'magery', 'magicresistance', 'tactics', 'snooping',
	'musicianship', 'poisoning', 'archery', 'spiritspeak', 'stealing', 'tailoring',
	'taming', 'tasteid', 'tinkering', 'tracking', 'veterinary', 'swordsmanship',
	'macefighting', 'fencing', 'wrestling', 'lumberjacking', 'mining', 'meditation',
	'stealth', 'removetraps', 'necromancy', 'focus', 'chivalry', 'bushido', 'ninjitsu', 'spellweaving' ]

skillnamesids = {
	'alchemy' : ALCHEMY,
	'anatomy' : ANATOMY,
	'animallore' : ANIMALLORE,
	'itemid' : ITEMID,
	'armslore' : ARMSLORE,
	'parrying' : PARRYING,
	'begging' : BEGGING,
	'blacksmithing' : BLACKSMITHING,
	'bowcraft' : BOWCRAFT,
	'peacemaking' : PEACEMAKING,
	'camping' : CAMPING,
	'carpentry' : CARPENTRY,
	'cartography' : CARTOGRAPHY,
	'cooking' : COOKING,
	'detectinghidden' : DETECTINGHIDDEN,
	'enticement' : ENTICEMENT,
	'evaluatingintel' : EVALUATINGINTEL,
	'healing' : HEALING,
	'fishing' : FISHING,
	'forensics' : FORENSICS,
	'herding' : HERDING,
	'hiding' : HIDING,
	'provocation' : PROVOCATION,
	'inscription' : INSCRIPTION,
	'lockpicking' : LOCKPICKING,
	'magery' : MAGERY,
	'magicresistance' : MAGICRESISTANCE,
	'tactics' : TACTICS,
	'snooping' : SNOOPING,
	'musicianship' : MUSICIANSHIP,
	'poisoning' : POISONING,
	'archery' : ARCHERY,
	'spiritspeak' : SPIRITSPEAK,
	'stealing' : STEALING,
	'tailoring' : TAILORING,
	'taming' : TAMING,
	'tasteid' : TASTEID,
	'tinkering' : TINKERING,
	'tracking' : TRACKING,
	'veterinary' : VETERINARY,
	'swordsmanship' : SWORDSMANSHIP,
	'macefighting' : MACEFIGHTING,
	'fencing' : FENCING,
	'wrestling' : WRESTLING,
	'lumberjacking' : LUMBERJACKING,
	'mining' : MINING,
	'meditation' : MEDITATION,
	'stealth' : STEALTH,
	'removetraps' : REMOVETRAPS,
	'necromancy' : NECROMANCY,
	'focus' : FOCUS,
	'chivalry' : CHIVALRY,
	'bushido' : BUSHIDO,
	'ninjitsu' : NINJITSU,
	'spellweaving' : SPELLWEAVING
}

STATNAMES = [ 'str', 'int', 'dex' ]

"""
	\constants wolfpack.consts Character Sound Constants
	These constants can be used to let a character make
	a body dependant sound. See the sound method for the
	<object id="char">char</object> object.
"""
SND_STARTATTACK = 0
SND_IDLE = 1
SND_ATTACK = 2
SND_DEFEND = 3
SND_DIE = 4
"""
	\end
"""

"""
	\constants wolfpack.consts Layer Constants
	These constants provide fast access to equipment on certain
	layers of a character. See the itemonlayer method for <object id="char">char</object> objects.
"""
LAYER_RIGHTHAND = 1
LAYER_LEFTHAND = 2
LAYER_SHOES = 3
LAYER_PANTS = 4
LAYER_SHIRT = 5
LAYER_HELM = 6
LAYER_GLOVES = 7
LAYER_RING = 8
LAYER_UNUSED1 = 9
LAYER_NECK = 10
LAYER_HAIR = 11
LAYER_WAIST = 12
LAYER_CHEST = 13
LAYER_BRACELET = 14
LAYER_UNUSED2 = 15
LAYER_BEARD = 16
LAYER_TORSO = 17
LAYER_EARRINGS = 18
LAYER_ARMS = 19
LAYER_CLOAK = 20
LAYER_BACKPACK = 21
LAYER_ROBE = 22
LAYER_SKIRT = 23
LAYER_LEGS = 24
LAYER_MOUNT = 25
LAYER_NPCRESTOCK = 26
LAYER_NPCNORESTOCK = 27
LAYER_NPCSELL = 28
LAYER_BANKBOX = 29
LAYER_DRAGGING = 30
LAYER_TRADING = 31
"""
	\end
"""

# Layer Names
LAYERNAMES = {
	LAYER_RIGHTHAND: 'Right hand',
	LAYER_LEFTHAND: 'Shield hand',
	LAYER_SHOES: 'Shoes',
	LAYER_PANTS: 'Pants',
	LAYER_SHIRT: 'Inner Torso',
	LAYER_HELM: 'Head',
	LAYER_GLOVES: 'Hands',
	LAYER_RING: 'Ring Finger',
	LAYER_UNUSED1: 'Unused',
	LAYER_NECK: 'Neck',
	LAYER_HAIR: 'Hair',
	LAYER_WAIST: 'Waist',
	LAYER_CHEST: 'Middle Torso',
	LAYER_BRACELET: 'Wrist',
	LAYER_UNUSED2: 'Unused 2',
	LAYER_BEARD: 'Beard',
	LAYER_TORSO: 'Outer Torso',
	LAYER_EARRINGS: 'Earrings',
	LAYER_ARMS: 'Arms',
	LAYER_CLOAK: 'Cloak',
	LAYER_BACKPACK: 'Backpack',
	LAYER_ROBE: 'Robe',
	LAYER_SKIRT: 'Skirt',
	LAYER_LEGS: 'Legs',
	LAYER_MOUNT: 'Mount',
	LAYER_NPCRESTOCK: 'NPC Restock',
	LAYER_NPCNORESTOCK: 'NPC No Restock',
	LAYER_NPCSELL: 'NPC Sell',
	LAYER_BANKBOX: 'Bankbox',
	LAYER_DRAGGING: 'Dragging',
	LAYER_TRADING: 'Trading'
}

"""
	\constants wolfpack.consts Event Constants
	These constants represent an event in a python script. These
	are used in the callevent, hasevent and registerglobal functions of the
	<module id="wolfpack">wolfpack</module> module.
"""
EVENT_USE = 0
EVENT_SINGLECLICK = 1
EVENT_COLLIDE = 2
EVENT_WALK = 3
EVENT_CREATE = 4
EVENT_TALK = 5
EVENT_WARMODETOGGLE = 6
EVENT_LOGIN = 7
EVENT_LOGOUT = 8
EVENT_HELP = 9
EVENT_CHAT = 10
EVENT_SKILLUSE = 11
EVENT_SKILLGAIN = 12
EVENT_SHOWPAPERDOLL = 13
EVENT_SHOWSKILLGUMP = 14
EVENT_DEATH = 15
EVENT_SHOWPAPERDOLLNAME = 16
EVENT_CONTEXTENTRY = 17
EVENT_SHOWTOOLTIP = 18
EVENT_CHLEVELCHANGE = 19
EVENT_SPEECH = 20
EVENT_WEARITEM = 21
EVENT_EQUIP = 22
EVENT_UNEQUIP = 23
EVENT_DROPONCHAR = 24
EVENT_DROPONITEM = 25
EVENT_DROPONGROUND = 26
EVENT_PICKUP = 27
EVENT_DAMAGE = 28
EVENT_CASTSPELL = 29
EVENT_TRADE = 30
EVENT_TRADESTART = 31
EVENT_DELETE = 32
EVENT_SWING = 33
EVENT_SHOWSTATUS = 34
EVENT_CHANGEREGION = 35
EVENT_ATTACH = 36
EVENT_DETACH = 37
EVENT_TIMECHANGE = 38
EVENT_DISPEL = 39
EVENT_TELEKINESIS = 40
EVENT_CONTEXTCHECKVISIBLE = 41
EVENT_CONTEXTCHECKENABLED = 42
EVENT_SPAWN = 43
EVENT_UPDATEDATABASE = 44
EVENT_GETSELLPRICE = 45
EVENT_SHOWVIRTUEGUMP = 46
EVENT_RESURRECT = 47
EVENT_CHECKSECURITY = 48
EVENT_CHECKVICTIM = 49
EVENT_DODAMAGE = 50
EVENT_SNOOPING = 51
EVENT_REMOTEUSE = 52
EVENT_CONNECT = 53
EVENT_DISCONNECT = 54
EVENT_GUILDBUTTON = 55
EVENT_SELECTABILITY = 56
EVENT_LOG = 57
EVENT_REGENHITS = 58
EVENT_REGENMANA = 59
EVENT_REGENSTAMINA = 60
EVENT_BECOMECRIMINAL = 61
EVENT_QUESTBUTTON = 62
EVENT_REALDAYCHANGE = 63
EVENT_WORLDSAVE = 64
EVENT_SERVERHOUR = 65
EVENT_SEECHAR = 66
EVENT_SEEITEM = 67
EVENT_COUNT = 68
"""
	\end
"""

"""
	\constants wolfpack.consts Boolean Constants
	Only two constants for true and false.
"""
FALSE = 0
TRUE = 1
"""
	\end
"""

# Definitions for Magic (Casting Source)
CAST_BOOK = 0
CAST_SCROLL = 1

CAST_TARGET_CHAR = 0x01 # Agressive Spells, Heal, Etc.
CAST_TARGET_ITEM = 0x02 # Recall
CAST_TARGET_GROUND = 0x04 # Other

"""
	\constants wolfpack.consts Damage Constants
	These constants specify a damage type for the
	damage method in the <object id="char">char</object>
	object.
"""
DAMAGE_PHYSICAL = 0
DAMAGE_MAGICAL = 1
DAMAGE_GODLY = 2
DAMAGE_HUNGER = 3
"""
	\end
"""

# Some nice animation definitions
"""
	\constants wolfpack.consts Animation Constants
	These constants are animation ids for the
	action method in the <object id="char">char</object>
	object.
"""
ANIM_WALKUNARMED = 0x0
ANIM_WALKARMED = 0x1
ANIM_RUNUNARMED = 0x2
ANIM_RUNARMED = 0x3
ANIM_STAND = 0x4
ANIM_FIDGET1 = 0x5
ANIM_FIDGET2 = 0x6
ANIM_ATTACKREADY1 = 0x7
ANIM_ATTACKREADY2 = 0x8
ANIM_ATTACK1 = 0x9
ANIM_ATTACK2 = 0xa
ANIM_ATTACK3 = 0xb
ANIM_ATTACK4 = 0xc
ANIM_ATTACK5 = 0xd
ANIM_ATTACK6 = 0xe
ANIM_ATTACKWALK = 0xf
ANIM_CASTDIRECTED = 0x10
ANIM_CASTAREA = 0x11
ANIM_ATTACKBOW = 0x12
ANIM_ATTACKXBOX = 0x13
ANIM_TAKEHIT = 0x14
ANIM_DIE1 = 0x15
ANIM_DIE2 = 0x16
ANIM_TURN = 0x1e
ANIM_ATTACK7 = 0x1f
ANIM_BOW = 0x20
ANIM_SALUTE = 0x21
ANIM_FIDGET3 = 0x22
"""
	\end
"""

# Definitions for the Sextant
SEXTANT_CENTER_X = 5936
SEXTANT_CENTER_Y = 3112
SEXTANT_MAP_HEIGHT = 5120
SEXTANT_MAP_WIDTH = 4096

"""
	\constants wolfpack.consts Log Constants
	These constants represent a loglevel for the console
	and the logfile.
"""
LOG_MESSAGE = 0x01
LOG_ERROR	= 0x02
LOG_PYTHON  = 0x04
LOG_WARNING = 0x08
LOG_NOTICE  = 0x10
LOG_TRACE   = 0x20
LOG_DEBUG   = 0x40
"""
	\end
"""

# Colors
BLACK = 1
DARKBLUE = 2
BLUE = 3
RED = 38
DARKRED = 37
PINK = 19
DARKGREEN = 67
GREEN = 68
YELLOW = 55
GRAY = 0x3b2

# Fonts
NORMAL = 3

#Stat gain delay = 15 minutes ( 900 seconds )
STATGAINDELAY = 900

"""
	\constants wolfpack.consts Action Constants
	These constants can be used to queue an action to be executed in the next mainloop iteration.
	See the queueaction function in the <module id="wolfpack">wolfpack</module> module.
"""
RELOAD_SCRIPTS = 0 # Reload definitions and python scripts.
RELOAD_PYTHON = 1 # Reload python scripts.
RELOAD_ACCOUNTS = 2 # Reload accounts.
RELOAD_CONFIGURATION = 3 # Reload all of the above.
SAVE_WORLD = 4 # Save the world.
SAVE_ACCOUNTS = 5 # Save the accounts.
"""
	\end
"""

#5 minutes
ANTIMACRODELAY = 300000
#3 uses of object
ANTIMACROALLOWANCE = 3

# A Huge Sound List!
#
# Notes: 0x4d4 appears to be the final sound ID
"""
	\constants wolfpack.consts Sound Constants
	This list of constants describes some commonly used sounds.
"""
SOUND_JUNGLE_BIRDS_1 = 0x0
SOUND_JUNGLE_BIRDS_2 = 0x1
SOUND_JUNGLE_BIRDS_3 = 0x2
SOUND_FOREST_BIRDS_1 = 0x3
SOUND_FOREST_BIRDS_2 = 0x4
SOUND_FOREST_BIRDS_3 = 0x5
SOUND_FOREST_BIRDS_4 = 0x6
SOUND_SWAMP_BIRDS_1 = 0x7
SOUND_SWAMP_BIRDS_2 = 0x8
SOUND_SWAMP_BIRDS_3 = 0x9
SOUND_SWAMP_BIRDS_4 = 0xa
SOUND_SWAMP_BIRDS_5 = 0xc
SOUND_SWAMP_BIRDS_6 = 0xd
SOUND_SWAMP_BIRDS_7 = 0xe
SOUND_SWAMP_BIRDS_8 = 0xf
SOUND_RIVER_BIRDS = 0xb

SOUND_RIVER = 0x10
SOUND_STREAM = 0x11
SOUND_BEACH = 0x12

SOUND_BOAT_OCEAN = 0x13

SOUND_STORM_1 = 0x14 # Light Wind
SOUND_STORM_2 = 0x15 # Heavy Wind
SOUND_STORM_3 = 0x16 # Moderate Wind

SOUND_BIRDCALL_1 = 0x17
SOUND_BIRDCALL_2 = 0x18
SOUND_BIRDCALL_3 = 0x19
SOUND_BIRDCALL_4 = 0x1a
SOUND_BIRDCALL_5 = 0x1b
SOUND_BIRDCALL_6 = 0x1c
SOUND_BIRDCALL_7 = 0x1d
SOUND_BIRDCALL_8 = 0x1e
SOUND_BIRDCALL_9 = 0x1f

SOUND_WATERBUBBLE_1 = 0x20 # Slow
SOUND_WATERBUBBLE_2 = 0x21 # Fast

SOUND_WATER_DROP_1 = 0x22 #
SOUND_WATER_DROP_2 = 0x23 #
SOUND_WATER_DROP_3 = 0x24 # Two drops
SOUND_WATER_DROP_4 = 0x25 # Small object into water
SOUND_WATER_DROP_5 = 0x26 # Large object into water
SOUND_WATER_DROP_6 = 0x27 # Medium obj

SOUND_THUNDER_1 = 0x28 # Shorter/Deeper thunder
SOUND_THUNDER_1 = 0x29 # Long/Sharp thunder

SOUND_HAMMER_1 = 0x2a # Blacksmith Sound

SOUND_DRINK1 = 0x30
SOUND_DRINK2 = 0x31
SOUND_DRINK3 = 0x50

SOUND_MINING = 0x126

SOUND_AGILITY_DOWN = 0x1ee
SOUND_AGILITY_UP = 0x1ed
SOUND_STRENGTH_UP = 0x1e7
SOUND_FIZZLE_SPELL = 0x5c
SOUND_FILL_BOTTLE = 0x240
SOUND_INSCRIBE1 = 0x241
SOUND_INSCRIBE2 = 0x4f
SOUND_MOTAR = 0x242
SOUND_MEDITATE = 0x24a
SOUND_FELUCIA = 0x245

SOUND_EXPLOSION_3 = 0x306
SOUND_EXPLOSION_4 = 0x307
SOUND_EXPLOSION_5 = 0x308
SOUND_EXPLOSION_6 = 0x309

SOUND_MAGICAL_01 = 0x3bd # A Summoning Sound
SOUND_MAGICAL_02 = 0x3c4 # A Summoning Sound
SOUND_MAGICAL_03 = 0x456 # Like an energy field...

SOUND_WARP_01 = 0x474 # A more modern recall sound?

SOUND_FIREBALL_01 = 0x44b #

SOUND_BREAKING_1 = 0x38d
SOUND_BREAKING_2 = 0x38e
SOUND_BREAKING_3 = 0x38f
SOUND_BREAKING_4 = 0x390

# I think this is the new lute sound anways...
SOUND_LUTE_01 = 0x391 # Cool tune Battle Music
SOUND_LUTE_02 = 0x392 # Cool tune Peace Music
# I think this is the harp sounds...
SOUND_HARP_01 = 0x403 # Cool tune Battle Music
SOUND_HARP_02 = 0x40b # Cool tune Peace Music
SOUND_HARP_03 = 0x418 # Cool tune

SOUND_TAMBER_01 = 0x4b6 # Short tune
SOUND_TAMBER_02 = 0x4b7 # Longer Tune, Battle?
"""
	\end
"""

"""
	\constants wolfpack.consts Human Male Sounds
	Some handy male sound constants.
	Use the sound method of the <object id="char">char</object> object to play them.
"""
SOUND_MALE_01 = 0x419 # Ah!
SOUND_MALE_02 = 0x41a # Aha!
SOUND_MALE_03 = 0x41b # Clapping
SOUND_MALE_04 = 0x41c # ??
SOUND_MALE_05 = 0x41d # Burp
SOUND_MALE_06 = 0x41e # Woo Hoo!
SOUND_MALE_07 = 0x41f # *Attention cough*
SOUND_MALE_08 = 0x420 # Cough
SOUND_MALE_09 = 0x421 # Choking?
SOUND_MALE_10 = 0x422 # Crying
SOUND_MALE_11 = 0x423 # Dying
SOUND_MALE_12 = 0x424 # Dying
SOUND_MALE_13 = 0x425 # Dying
SOUND_MALE_14 = 0x426 # Dying
SOUND_MALE_15 = 0x427 # Dying
SOUND_MALE_16 = 0x428 # Fart
SOUND_MALE_17 = 0x429 # Gasp
SOUND_MALE_18 = 0x42a # Laugh
SOUND_MALE_19 = 0x42b # Groan
SOUND_MALE_20 = 0x42c # Mean Groan
SOUND_MALE_21 = 0x42d # Hey!
SOUND_MALE_22 = 0x42e # Hicup
SOUND_MALE_23 = 0x42f # Huh?
SOUND_MALE_24 = 0x430 # Blowing a kiss
SOUND_MALE_25 = 0x431 # Laughing
SOUND_MALE_26 = 0x432 # No
SOUND_MALE_27 = 0x433 # Oh!
SOUND_MALE_28 = 0x434 # Groan
SOUND_MALE_29 = 0x435 # Groan
SOUND_MALE_30 = 0x436 # Groan
SOUND_MALE_31 = 0x437 # Groan soft
SOUND_MALE_32 = 0x438 # Groan gross sounding
SOUND_MALE_33 = 0x439 # Oogh...
SOUND_MALE_34 = 0x43a # Oohhhh...
SOUND_MALE_35 = 0x43b # "Hoomph!"
SOUND_MALE_36 = 0x43c # ugh
SOUND_MALE_37 = 0x43d # ooooooooo!
SOUND_MALE_38 = 0x43e # Oops
SOUND_MALE_39 = 0x43f # Throwing up
SOUND_MALE_40 = 0x440 # Ahhhhhhhhhhhh!
SOUND_MALE_41 = 0x441 # Shhhhhhhh
SOUND_MALE_42 = 0x442 # Yawning/Sighing
SOUND_MALE_43 = 0x443 # Sneezing
SOUND_MALE_44 = 0x444 # Sniffing
SOUND_MALE_45 = 0x445 # Snoring
SOUND_MALE_46 = 0x446 # Spitting Sound
SOUND_MALE_47 = 0x447 # Whistles
SOUND_MALE_48 = 0x448 # Yawning
SOUND_MALE_49 = 0x449 # Yeah!
SOUND_MALE_50 = 0x44a # Hoaghhh!
"""
	\end
"""

"""
	\constants wolfpack.consts Human Female Sounds
	Some handy female sound constants.
	Use the sound method of the <object id="char">char</object> object to play them.
"""
SOUND_FEMALE_01 = 0x30a # Ooooh!
SOUND_FEMALE_02 = 0x30b # Aha!
SOUND_FEMALE_03 = 0x30c # Clap
SOUND_FEMALE_04 = 0x30d # Sneeze?
SOUND_FEMALE_05 = 0x30e # Burp
SOUND_FEMALE_06 = 0x30f # Woohoo!
SOUND_FEMALE_07 = 0x310 # Cough
SOUND_FEMALE_08 = 0x311 # Cough 2
SOUND_FEMALE_09 = 0x312 # Cough 3
SOUND_FEMALE_10 = 0x313 # Cry
SOUND_FEMALE_11 = 0x314 # Groan
SOUND_FEMALE_12 = 0x315 # Groan 2
SOUND_FEMALE_13 = 0x316 # Groan 3
SOUND_FEMALE_14 = 0x317 # Groan 4
SOUND_FEMALE_15 = 0x318 # Fart Sound
SOUND_FEMALE_16 = 0x319 # Gasp
SOUND_FEMALE_17 = 0x31a # Laugh
SOUND_FEMALE_18 = 0x31b # Groan/Frustrated
SOUND_FEMALE_19 = 0x31c # Aggitated
SOUND_FEMALE_20 = 0x31d # "Hey!"
SOUND_FEMALE_21 = 0x31f # "Huh?"
SOUND_FEMALE_22 = 0x320 # Blowing Kiss
SOUND_FEMALE_23 = 0x321 # Soft Laugh
SOUND_FEMALE_24 = 0x322 # NO!
SOUND_FEMALE_25 = 0x323 # "Oh!"
SOUND_FEMALE_26 = 0x324 # "Ugh!"
SOUND_FEMALE_27 = 0x325 # "Ugh!" 2
SOUND_FEMALE_28 = 0x326 # "Ughhhh" 3
SOUND_FEMALE_29 = 0x327 # "Ughhhhhh" 4
SOUND_FEMALE_30 = 0x328 # "Urraghh!"
SOUND_FEMALE_31 = 0x329 # "Uhh!"
SOUND_FEMALE_32 = 0x32a # Ugh!
SOUND_FEMALE_33 = 0x32b # ooooooooo!
SOUND_FEMALE_34 = 0x32c # Ooops!
SOUND_FEMALE_35 = 0x32d # Gag sound
SOUND_FEMALE_36 = 0x32e # Screaming
SOUND_FEMALE_37 = 0x32f # Shhhhhh!
SOUND_FEMALE_38 = 0x330 # *Sigh*
SOUND_FEMALE_39 = 0x331 # Sneeze
SOUND_FEMALE_40 = 0x332 # Sniffles
SOUND_FEMALE_41 = 0x333 # Snores
SOUND_FEMALE_42 = 0x334 # ???
SOUND_FEMALE_43 = 0x335 # Whistles
SOUND_FEMALE_44 = 0x336 # Yawns
SOUND_FEMALE_45 = 0x337 # Yeah!
SOUND_FEMALE_46 = 0x338 # Really frustrated
"""
	\end
"""

"""
	\constants wolfpack.consts Definition Constants
	These constants represent the definition section types as used in
	getdefinition in the <module id="wolfpack">wolfpack</module> module.
"""
WPDT_ITEM = 0
WPDT_SCRIPT = 1
WPDT_NPC = 2
WPDT_LIST = 3
WPDT_MENU = 4
WPDT_SPELL = 5
WPDT_PRIVLEVEL = 6
WPDT_SPAWNREGION = 7
WPDT_REGION = 8
WPDT_MULTI = 9
WPDT_TEXT = 10
WPDT_STARTITEMS = 11
WPDT_LOCATION = 12
WPDT_SKILL = 13
WPDT_ACTION = 14
WPDT_MAKESECTION = 15
WPDT_MAKEITEM = 16
WPDT_USEITEM = 17
WPDT_SKILLCHECK = 18
WPDT_DEFINE = 19
WPDT_RESOURCE = 20
WPDT_CONTEXTMENU = 21
WPDT_AI = 22
WPDT_TELEPORTER = 23
WPDT_QUEST = 24
"""
	\end
"""

# Constants for body types
BODY_UNKNOWN  = 0
BODY_MONSTER = 1
BODY_SEA = 2
BODY_ANIMAL = 3
BODY_HUMAN = 4

MAX_TITHING_POINTS = 100000 # Maximum amount of tithing points

PLAYER_BODIES_ALIVE = [ 0x190, 0x191, 0x25d, 0x25e ]
PLAYER_BODIES_DEAD = [ 0x192, 0x193, 0x25f, 0x260 ]
PLAYER_BODIES_ALL = PLAYER_BODIES_ALIVE + PLAYER_BODIES_DEAD + [ 0x3db ]
PLAYER_BODIES_ALIVE_MALE = [ 0x190, 0x25d ]
PLAYER_BODIES_ALIVE_FEMALE = [ 0x191, 0x25e ]
PLAYER_BODIES_ALIVE_HUMAN = [ 0x190, 0x191 ]
PLAYER_BODIES_ALIVE_ELVEN = [ 0x25d, 0x25e ]
