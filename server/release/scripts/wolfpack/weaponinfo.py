
from wolfpack.consts import RESISTANCE_PHYSICAL, RESISTANCE_ENERGY, \
	RESISTANCE_COLD, RESISTANCE_POISON, RESISTANCE_FIRE, DAMAGE_PHYSICAL, \
	DAMAGE_ENERGY, DAMAGE_COLD, DAMAGE_POISON, DAMAGE_FIRE, DAMAGEBONUS, \
	SPEEDBONUS, HITBONUS, DEFENSEBONUS, MINDAMAGE, MAXDAMAGE, SPEED, MISSSOUND, \
	HITSOUND, PROJECTILE, PROJECTILEHUE, AMMUNITION, REQSTR, REQDEX, REQINT, \
	LUCK, GOLDINCREASE, LOWERREQS, HITPOINTRATE, STAMINARATE, MANARATE, \
	SPELLDAMAGEBONUS, MATERIALPREFIX, SWING, REFLECTPHYSICAL, \
	DURABILITYBONUS, \
	MAGICRESISTANCE, INSCRIPTION, LAYER_RIGHTHAND, LAYER_MOUNT, \
	ITEM_ARMOR, ITEM_WEAPON, ITEM_SHIELD, ITEM_MELEE, ITEM_RANGED, \
	ITEM_PIERCING, ITEM_SLASHING, ITEM_BASHING, SPELLCHANNELING

WEAPON_RESNAME_BONI = {
	# Metals
	'iron': {
		DAMAGE_FIRE: 0,
		DAMAGE_COLD: 0,
		DAMAGE_POISON: 0,
		DAMAGE_ENERGY: 0
	},
	'dullcopper': {
		DAMAGE_FIRE: 0,
		DAMAGE_COLD: 0,
		DAMAGE_POISON: 0,
		DAMAGE_ENERGY: 0,
		LOWERREQS: 50,
		MATERIALPREFIX: 'Dull Copper',
		DURABILITYBONUS: 100,
	},
	'shadowiron': {
		DAMAGE_FIRE: 0,
		DAMAGE_COLD: 20,
		DAMAGE_POISON: 0,
		DAMAGE_ENERGY: 0,
		MATERIALPREFIX: 'Shadow Iron',
		DURABILITYBONUS: 50,
	},
	'copper': {
		DAMAGE_FIRE: 0,
		DAMAGE_COLD: 0,
		DAMAGE_POISON: 10,
		DAMAGE_ENERGY: 20,
		MATERIALPREFIX: 'Copper',
	},
	'bronze': {
		DAMAGE_FIRE: 40,
		DAMAGE_COLD: 0,
		DAMAGE_POISON: 0,
		DAMAGE_ENERGY: 0,
		MATERIALPREFIX: 'Bronze',
	},
	'gold': {
		DAMAGE_FIRE: 0,
		DAMAGE_COLD: 0,
		DAMAGE_POISON: 0,
		DAMAGE_ENERGY: 0,
		LUCK: 40,
		LOWERREQS: 50,
		MATERIALPREFIX: 'Gold',
	},
	'agapite': {
		DAMAGE_FIRE: 0,
		DAMAGE_COLD: 30,
		DAMAGE_POISON: 0,
		DAMAGE_ENERGY: 20,
		MATERIALPREFIX: 'Agapite',
	},
	'verite': {
		DAMAGE_FIRE: 0,
		DAMAGE_COLD: 0,
		DAMAGE_POISON: 40,
		DAMAGE_ENERGY: 20,
		MATERIALPREFIX: 'Verite',
	},
	'valorite': {
		DAMAGE_FIRE: 10,
		DAMAGE_COLD: 20,
		DAMAGE_POISON: 10,
		DAMAGE_ENERGY: 20,
		MATERIALPREFIX: 'Valorite',
	},
	# Wood
	'beech': {
		MATERIALPREFIX: 'Beech Wood',
		DAMAGEBONUS: 0,
		DURABILITYBONUS: 0,
		SPEEDBONUS: 0,
		LUCK: 0
	},
	'apple': {
		MATERIALPREFIX: 'Apple Wood',
		DAMAGEBONUS: 0,
		DURABILITYBONUS: 20,
		SPEEDBONUS: -10,
		LUCK: 0
	},
	'peach': {
		MATERIALPREFIX: 'Peach Wood',
		DAMAGEBONUS: 5,
		DURABILITYBONUS: 5,
		SPEEDBONUS: -10,
		LUCK: 20
	},
	'pear': {
		MATERIALPREFIX: 'Pear Wood',
		DAMAGEBONUS: 10,
		DURABILITYBONUS: -5,
		SPEEDBONUS: 5,
		LUCK: 20
	},
	'cedar': {
		MATERIALPREFIX: 'Cedar Wood',
		DAMAGEBONUS: 10,
		DURABILITYBONUS: 10,
		SPEEDBONUS: 10,
		LUCK: 0
	},
	'willow': {
		MATERIALPREFIX: 'Willow Wood',
		DAMAGEBONUS: 0,
		DURABILITYBONUS: 40,
		SPEEDBONUS: 0,
		LUCK: 0
	},
	'cypress': {
		MATERIALPREFIX: 'Cypress Wood',
		DAMAGEBONUS: 15,
		DURABILITYBONUS: 0,
		SPEEDBONUS: 0,
		LUCK: 0
	},
	'oak': {
		MATERIALPREFIX: 'Oak Wood',
		DAMAGEBONUS: 20,
		DURABILITYBONUS: 10,
		SPEEDBONUS: -5,
		LUCK: 25
	},
	'walnut': {
		MATERIALPREFIX: 'Walnut Wood',
		DAMAGEBONUS: 15,
		DURABILITYBONUS: 15,
		SPEEDBONUS: 15,
		LUCK: 0
	},
	'yew': {
		MATERIALPREFIX: 'Yew Wood',
		DAMAGEBONUS: 10,
		DURABILITYBONUS: 30,
		SPEEDBONUS: 10,
		LUCK: 40
	},
	'tropical': {
		MATERIALPREFIX: 'Tropical Wood',
		DAMAGEBONUS: 25,
		DURABILITYBONUS: -20,
		SPEEDBONUS: 10,
		LUCK: -20
	}
}

#
# Weapon Information
# This file includes all known standard weapon information.
# The baseid of the item is used as the key for WEAPONINFO.
# For unspecified entries the default value will be used.
#
WEAPONINFO = {
	# Demonstration entry
	#'baseid': {
	# MINDAMAGE: The minimum damage this weapon deals
	# MAXDAMAGE: The maximum damage this weapon deals
	# HITSOUND: A list of soundeffects played for this weapon if it hits
	# MISSSOUND: A list of soundeffects played for this weapon if it misses
	# SPEED: The weapon speed
	# AMMUNITION: The BASE ID of the ammunition to use for this weapon.
	# PROJECTILE: The item id of the projectile to throw at the defender if shot.
	# PROJECTILEHUE: The hue of the projectile this weapon shoots
	#}

	# Bow
	'13b1': {
		SWING: [ 0x12 ],
		HITSOUND: [ 0x234 ],
		MISSSOUND: [ 0x238 ]
	},
	# Crossbow
	'f4f': {
		SWING: [ 0x13 ],
		HITSOUND: [ 0x234 ],
		MISSSOUND: [ 0x238 ]
	},
	# Heavy Crossbow
	'13fd': {
		SWING: [ 0x13 ],
		HITSOUND: [ 0x234 ],
		MISSSOUND: [ 0x238 ]
	},
	# Composite Bow
	'26c2': {
		SWING: [ 0x12 ],
		HITSOUND: [ 0x234 ],
		MISSSOUND: [ 0x238 ]
	},
	# Repeating Crossbow
	'26cd': {
		SWING: [ 0x13 ],
		HITSOUND: [ 0x234 ],
		MISSSOUND: [ 0x238 ]
	},
	# Katana
	'13ff': {
		SWING: [ 0x9 ],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Axe
	'f49': {
		SWING: [ 0xd ],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Bardiche
	'f4d': {
		SWING: [0xc, 0xd],
		HITSOUND: [ 0x237 ],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Battle Axe
	'f47': {
		SWING: [ 0xd ],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Bladded Staff
	'26c7': {
		SWING: [0xc, 0xd],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x239 ]
	},
	# Bone Harvester
	'26c5': {
		SWING: [0x9],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x239 ]
	},
	# Broadsword
	'f5e': {
		SWING: [ 0x9 ],
		HITSOUND: [0x237],
		MISSSOUND: [0x23a]
	},
	# Butcher Knife
	'13f6': {
		SWING: [ 0x9 ],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Cleaver
	'ec3': {
		SWING: [ 0x9 ],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Crescent Blade
	'26cb': {
		SWING: [0xc, 0xd],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x239 ]
	},
	# Cutlass
	'1441': {
		SWING: [0x9],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Double Axe
	'f4b': {
		SWING: [ 0xd ],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Executioners Axe
	'f45': {
		SWING: [ 0xd ],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Halberd
	'143f': {
		SWING: [0xc, 0xd],
		HITSOUND: [ 0x237 ],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Hatchet
	'f43': {
		SWING: [ 0xd ],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Large Battle Axe
	'13fb': {
		SWING: [ 0xd ],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Long Sword
	'13b8': {
		SWING: [0x9],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Longsword
	'f61': {
		SWING: [0x9],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Paladin Sword
	'26cf': {
		SWING: [0x9],
		HITSOUND: [0x23b, 0x23c, 0x237],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Pickaxe
	'e86': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Scimitar
	'13b6': {
		SWING: [0x9],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Scythe
	'26c4': {
		SWING: [0xc, 0xd],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Skinning Knife
	'ec4': {
		SWING: [0x9],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Two Handed Axe
	'1443': {
		SWING: [ 0xd ],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Viking Sword
	'13b9': {
		SWING: [0x9],
		HITSOUND: [0x23b, 0x23c, 0x237],
		MISSSOUND: [ 0x23a, 0x238, 0x239 ]
	},
	# Dagger
	'f51': {
		SWING: [0xa],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Double Bladed Staff
	'26c9': {
		SWING: [0xe],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Kryss
	'1401': {
		SWING: [0xa],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Lance
	'26ca': {
		SWING: [0xe],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Pike
	'26be': {
		SWING: [0xe],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Pitchfork
	'e87': {
		SWING: [0xe],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Short Spear
	'1403': {
		SWING: [0xe],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Spear
	'f62': {
		SWING: [0xe],
		HITSOUND: [0x23b, 0x23c],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# War Fork
	'1405': {
		SWING: [0xa],
		HITSOUND: [0x23b, 0x23c, 0x237],
		MISSSOUND: [ 0x23a, 0x239, 0x238 ]
	},
	# Black Staff
	'df1': {
		SWING: [0xc],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Club
	'13b4': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Gnarled Staff
	'13f8': {
		SWING: [0xc],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Hammer Pick
	'143d': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Mace
	'f5c': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Maul
	'143b': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Quarter Staff
	'e89': {
		SWING: [0xc],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Scepter
	'26c6': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Shepherds Crook
	'13f5': {
		SWING: [ 0xc ],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# War Axe
	'13b0': {
		SWING: [ 0xd ],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# War Hammer
	'1439': {
		SWING: [ 0xc ],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# War Mace
	'1407': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Smithing Hammer
	'13e3': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Sledge Hammer
	'fb4': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	}
}

SAMURAIWEAPONINFO = {
	# Bokuto
	'27a8': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Daisho
	'27a9': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Fukiya
	'27aa': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Kama
	'27ad': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Lajatang
	'27a7': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# No-Dachi
	'27a2': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Nunchaku
	'27ae': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Sai
	'27af': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Shuriken
	'27ac': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Tekagi
	'27ab': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Tessen
	'27a3': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Tetsubo
	'27a6': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Wakizashi
	'27a4': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	},
	# Yumi
	'27a5': {
		SWING: [0x9],
		HITSOUND: [0x233, 0x232],
		MISSSOUND: [ 0x23a, 0x239 ]
	}
}

WEAPONINFO.update( SAMURAIWEAPONINFO )
