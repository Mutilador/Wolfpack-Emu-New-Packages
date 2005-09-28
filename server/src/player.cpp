/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2005 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#include "player.h"
#include "persistentbroker.h"
#include "dbdriver.h"

#include "guilds.h"
#include "console.h"
#include "network/network.h"
#include "network/uosocket.h"
#include "network/uotxpackets.h"
#include "basics.h"
#include "muls/maps.h"
#include "serverconfig.h"
#include "world.h"
#include "definitions.h"
#include "corpse.h"
#include "multi.h"
#include "mapobjects.h"
#include "party.h"
#include "npc.h"
#include "combat.h"
#include "muls/tilecache.h"
#include "guilds.h"
#include "skills.h"
#include "pythonscript.h"
#include "log.h"
#include "scriptmanager.h"
#include "inlines.h"

cPlayer::cPlayer()
{
	account_ = NULL;
	logoutTime_ = 0;
	objectDelay_ = 0;
	additionalFlags_ = 0;
	trackingTime_ = 0;
	socket_ = NULL;
	visualRange_ = VISRANGE;
	profile_ = ( char * ) 0;
	fixedLightLevel_ = 0;
	party_ = 0;
	guild_ = 0;
	strengthLock_ = 0;
	dexterityLock_ = 0;
	intelligenceLock_ = 0;
	maxControlSlots_ = 5;
}

cPlayer::cPlayer( const cPlayer& right )
{
	Q_UNUSED( right );
}

cPlayer::~cPlayer()
{
}

cPlayer& cPlayer::operator=( const cPlayer& right )
{
	Q_UNUSED( right );
	return *this;
}

static FactoryRegistration<cPlayer> registration( "cPlayer" );

unsigned char cPlayer::classid;

void cPlayer::buildSqlString( const char* objectid, QStringList& fields, QStringList& tables, QStringList& conditions )
{
	cBaseChar::buildSqlString( objectid, fields, tables, conditions );
	fields.push_back( "players.account,players.additionalflags,players.visualrange" );
	fields.push_back( "players.profile,players.fixedlight" );
	fields.push_back( "players.strlock,players.dexlock,players.intlock,players.maxcontrolslots" );
	tables.push_back( "players" );
	conditions.push_back( "uobjectmap.serial = players.serial" );
}

void cPlayer::postload( unsigned int version )
{
	cBaseChar::postload( version );

	if ( !pos_.isInternalMap() )
	{
		MapObjects::instance()->add( this );
	}

	// account removed?
	if ( account() == 0 )
	{
		Console::instance()->log( LOG_WARNING, tr( "Removing player %1 (0x%2) because of missing account.\n" ).arg( orgName_ ).arg( serial_, 0, 16 ) );
		remove();
		return;
	}

	if (kills() > 0) {
		setMurdererTime(Server::instance()->time()); // Reset murderer decay count
	}
}

void cPlayer::load( cBufferedReader& reader )
{
	load( reader, reader.version() );
	World::instance()->registerObject( this );
}

void cPlayer::load( cBufferedReader& reader, unsigned int version )
{
	cBaseChar::load( reader, version );

	setAccount( Accounts::instance()->getRecord( reader.readUtf8() ) );
	additionalFlags_ = reader.readInt();
	visualRange_ = reader.readByte();
	profile_ = reader.readUtf8();
	fixedLightLevel_ = reader.readByte();
	strengthLock_ = reader.readByte();
	dexterityLock_ = reader.readByte();
	intelligenceLock_ = reader.readByte();

	if ( version > 7 )
		maxControlSlots_ = reader.readByte();
}

void cPlayer::save( cBufferedWriter& writer, unsigned int version )
{
	cBaseChar::save( writer, version );
	writer.writeUtf8( account_ ? account_->login() : QString::null );
	writer.writeInt( additionalFlags_ );
	writer.writeByte( visualRange_ );
	writer.writeUtf8( profile_ );
	writer.writeByte( fixedLightLevel_ );
	writer.writeByte( strengthLock_ );
	writer.writeByte( dexterityLock_ );
	writer.writeByte( intelligenceLock_ );
	if ( version > 7 )
	{
		writer.writeByte( maxControlSlots_ );
	}
}

void cPlayer::load( char** result, Q_UINT16& offset )
{
	cBaseChar::load( result, offset );

	setAccount( Accounts::instance()->getRecord( result[offset++] ) );
	additionalFlags_ = atoi( result[offset++] );
	visualRange_ = atoi( result[offset++] );
	profile_ = result[offset++];
	fixedLightLevel_ = atoi( result[offset++] );
	strengthLock_ = atoi( result[offset++] );
	dexterityLock_ = atoi( result[offset++] );
	intelligenceLock_ = atoi( result[offset++] );
	maxControlSlots_ = atoi( result[offset++] );

	changed_ = false;
}

void cPlayer::save()
{
	if ( changed_ )
	{
		initSave;
		setTable( "players" );

		addField( "serial", serial() );

		if ( account_ )
		{
			addStrField( "account", account_->login() );
		}
		else
		{
			addStrField( "account", QString::null );
		}

		addField( "additionalflags", additionalFlags_ );
		addField( "visualrange", visualRange_ );
		addStrField( "profile", profile_ );
		addField( "fixedlight", fixedLightLevel_ );
		addField( "strlock", strengthLock_ );
		addField( "dexlock", dexterityLock_ );
		addField( "intlock", intelligenceLock_ );
		addField( "maxcontrolslots", maxControlSlots_ );

		addCondition( "serial", serial() );
		saveFields;
	}
	cBaseChar::save();
}

bool cPlayer::del()
{
	if ( !isPersistent )
		return false; // We didn't need to delete the object

	PersistentBroker::instance()->addToDeleteQueue( "players", QString( "serial = '%1'" ).arg( serial() ) );
	changed_ = true;
	return cBaseChar::del();
}

// Update flags etc.
void cPlayer::update( bool excludeself )
{
	cUOTxUpdatePlayer update;
	update.fromChar( this );

	for ( cUOSocket*socket = Network::instance()->first(); socket; socket = Network::instance()->next() )
	{
		if ( socket != socket_ && socket->canSee( this ) )
		{
			update.setHighlight( notoriety( socket->player() ) );
			socket->send( &update );
		}
	}

	if ( !excludeself && socket_ )
	{
		//socket_->updatePlayer();
		socket_->updatePlayer();
	}
}

// Resend the char to all sockets in range
void cPlayer::resend( bool clean )
{
	cUOTxRemoveObject remove;
	remove.setSerial( serial() );

	for ( cUOSocket*socket = Network::instance()->first(); socket; socket = Network::instance()->next() )
	{
		// Don't send such a packet to ourself
		if ( socket->canSee( this ) )
		{
			if ( socket == socket_ )
			{
				socket_->updatePlayer();
			}

			if ( socket != socket_ || clean )
			{
				cUOTxDrawChar drawChar;
				drawChar.fromChar( this );
				drawChar.setHighlight( notoriety( socket->player() ) );
				socket->send( &drawChar );
				sendTooltip( socket );

				// Send equipment tooltips to other players as well
				for ( ItemContainer::const_iterator it( content_.begin() ); it != content_.end(); ++it )
				{
					it.data()->sendTooltip( socket );
				}
			}
		}
		else if ( socket != socket_ && clean )
		{
			socket->send( &remove );
		}
	}
}

void cPlayer::talk( const QString& message, UI16 color, Q_UINT8 type, bool autospam, cUOSocket* socket )
{
	Q_UNUSED( autospam );
	if ( color == 0xFFFF )
		color = saycolor_;

	QString lang;

	if ( socket_ )
		lang = socket_->lang();

	cUOTxUnicodeSpeech::eSpeechType speechType;

	switch ( type )
	{
	case 0x01:
		speechType = cUOTxUnicodeSpeech::Broadcast; break;
	case 0x06:
		speechType = cUOTxUnicodeSpeech::System; break;
	case 0x09:
		speechType = cUOTxUnicodeSpeech::Yell; break;
	case 0x02:
		speechType = cUOTxUnicodeSpeech::Emote; break;
	case 0x08:
		speechType = cUOTxUnicodeSpeech::Whisper; break;
	case 0x0A:
		speechType = cUOTxUnicodeSpeech::Spell; break;
	default:
		speechType = cUOTxUnicodeSpeech::Regular; break;
	};

	cUOTxUnicodeSpeech* textSpeech = new cUOTxUnicodeSpeech();
	textSpeech->setSource( serial() );
	textSpeech->setModel( body_ );
	textSpeech->setFont( 3 ); // Default Font
	textSpeech->setType( speechType );
	textSpeech->setLanguage( lang );
	textSpeech->setName( name() );
	textSpeech->setColor( color );
	textSpeech->setText( message );

	QString ghostSpeech;
	bool gmSpiritSpeak = skillValue( SPIRITSPEAK ) >= 1000;

	// Generate the ghost-speech *ONCE*
	if ( isDead() && !gmSpiritSpeak )
	{
		for ( Q_UINT32 gI = 0; gI < message.length(); ++gI )
		{
			if ( message.at( gI ) == " " )
				ghostSpeech.append( " " );
			else
				ghostSpeech.append( ( RandomNum( 0, 1 ) == 0 ) ? "o" : "O" );
		}
	}

	if ( socket )
	{
		// Take the dead-status into account
		if ( isDead() && !gmSpiritSpeak )
			if ( !socket->player()->isDead() && !socket->player()->isGMorCounselor() && socket->player()->skillValue( SPIRITSPEAK ) < 1000 )
				textSpeech->setText( ghostSpeech );
			else
				textSpeech->setText( message );

		socket->send( textSpeech );
	}
	else
	{
		// Send to all clients in range
		for ( cUOSocket*mSock = Network::instance()->first(); mSock; mSock = Network::instance()->next() )
		{
			if ( mSock->player() && ( mSock->player()->dist( this ) < 18 ) )
			{
				// Take the dead-status into account
				if ( isDead() && !gmSpiritSpeak )
					if ( !mSock->player()->isDead() && !mSock->player()->isGMorCounselor() && mSock->player()->skillValue( SPIRITSPEAK ) < 1000 )
						textSpeech->setText( ghostSpeech );
					else
						textSpeech->setText( message );

				mSock->send( new cUOTxUnicodeSpeech( *textSpeech ) );
			}
		}
		delete textSpeech;
	}
}

Q_UINT8 cPlayer::notoriety( P_CHAR pChar ) // Gets the notoriety toward another char
{
	// 0x01 Blue, 0x02 Green, 0x03 Grey, 0x05 Orange, 0x06 Red, 0x07 Yellow
	Q_UINT8 result;

	if ( isInvulnerable() )
	{
		return 7;
	}

	if ( isIncognito() ) {
		return 1; // Always Innocent
	}

	if ( isPolymorphed() && !isHuman() ) {
		return 3; // Polymorph Defaults to Grey Name
	}

	// Guilds override kills
	if ( guild_ && pChar != this )
	{
		P_PLAYER player = dynamic_cast<P_PLAYER>( pChar );

		if ( player && player->guild_ )
		{
			// Same Guild => Green
			if ( player->guild_ == guild_ || guild_->isAllied(player->guild_) ) {
				return 0x02;
			} else if ( guild_->isAtWar(player->guild_) ) {
				return 0x05;
			}
		}
	}

	if ( pChar->kills() > Config::instance()->maxkills() )
	{
		result = 0x06; // 6 = Red -> Murderer
	}
	else if ( account_ )
	{
		if ( isCriminal() )
			result = 0x03;
		else if ( karma_ < -2000 )
			result = 0x06;
		else if ( karma_ < 0 )
			result = 0x03;
		else
			result = 0x01;
	}
	else
	{
		// Everything else
		result = 0x03;
	}

	return result;
}

P_NPC cPlayer::unmount()
{
	P_ITEM pi = atLayer( Mount );
	if ( pi && !pi->free )
	{
		P_NPC pMount = dynamic_cast<P_NPC>( FindCharBySerial( pi->getTag( "pet" ).toInt() ) );
		if ( pMount && !pMount->free )
		{
			pMount->setWanderType( enHalt );
			pMount->setStablemasterSerial( INVALID_SERIAL );
			pMount->moveTo( pos() );
			pMount->setDirection( direction_ );
			pMount->resend( false );
			pMount->bark( Bark_Idle );
		}
		pi->remove();
		resend( false );
		return pMount;
	}
	return NULL;
}

bool cPlayer::mount( P_NPC pMount )
{
	if ( !pMount )
		return false;

	if ( isDead() )
	{
		return false;
	}

	unsigned short mountId = pMount->mountId();

	if ( !mountId )
	{
		return false; // Not mountable
	}

	cUOSocket* socket = this->socket();
	if ( !inRange( pMount, Config::instance()->mountRange() ) && !isGM() )
	{
		if ( socket )
			socket->sysMessage( tr( "You are too far away to mount!" ) );
		return true; // Mountable, but not in range
	}

	if ( pMount->owner() == this || isGM() )
	{
		unmount();

		P_ITEM pMountItem = new cItem;
		pMountItem->Init();
		pMountItem->setId( mountId );
		pMountItem->setColor( pMount->skin() );

		if ( direction() != pMount->direction() )
		{
			setDirection( pMount->direction() );
			update();
		}

		this->addItem( cBaseChar::Mount, pMountItem );
		pMountItem->setTag( "pet", cVariant( pMount->serial() ) );
		pMountItem->update();

		// if this is a gm lets tame the animal in the process
		if ( isGM() )
		{
			pMount->setOwner( this );
			pMount->setTamed( true );
		}

		// remove it from screen!
		pMount->bark( Bark_Idle );
		pMount->removeFromView( false );
		pMount->fight( 0 );
		pMount->setStablemasterSerial( serial_ );
	}
	else
		socket->sysMessage( tr( "You dont own that creature." ) );

	return true;
}

bool cPlayer::isGM() const
{
	return account() && ( account()->rank() >= 50 ) && account()->isStaff();
}

bool cPlayer::isCounselor() const
{
	return account() && ( account()->rank() >= 25 && account()->rank() < 50 ) && account()->isStaff();
}

bool cPlayer::isGMorCounselor() const
{
	return account() && ( account()->rank() >= 25 ) && account()->isStaff();
}

void cPlayer::showName( cUOSocket* socket )
{
	if ( !socket->player() )
	{
		return;
	}

	QString charName = name();

	// Lord & Lady Title
	if ( !isIncognito() && fame_ >= 10000 && !isReputationHidden() )
		charName.prepend( gender_ ? tr( "Lady " ) : tr( "Lord " ) );

	QString affix( "" );

	if ( !isIncognito() && guild_ && !guild_->abbreviation().isEmpty() )
	{
		cGuild::MemberInfo* info = guild_->getMemberInfo( this );

		if ( info && info->showSign() )
		{
			affix.append( QString( "[%1]" ).arg( guild_->abbreviation() ) );
		}
	}

	// Append serial for GMs
	if ( socket->account()->isShowSerials() )
		affix.append( QString( "[0x%1]" ).arg( serial(), 4, 16 ) );

	// Append offline flag
	if ( !socket_ && !logoutTime_ )
		affix.append( tr( " [offline]" ) );

	// Guarded
	if ( guardedby_.size() > 0 )
		affix.append( tr( " [guarded]" ) );

	// Guarding
	if ( isTamed() && guarding_ )
		affix.append( tr( " [guarding]" ) );

	Q_UINT16 speechColor;

	// 0x01 Blue, 0x02 Green, 0x03 Grey, 0x05 Orange, 0x06 Red
	if (Config::instance()->sendAsciiNames()) {
		switch ( notoriety( socket->player() ) )
		{
		case 0x01:
			speechColor = 0x63; break; //blue
		case 0x02:
			speechColor = 0x44; break; //green
		case 0x03:
			speechColor = 0x3B2; break; //grey
		case 0x05:
			speechColor = 0x2b; break; //orange
		case 0x06:
			speechColor = 0x0026; break; //red
		default:
			speechColor = 0x3B2; break; // grey
		}

		if ( isInvulnerable() )
		{
			speechColor = 0x37;
		}

		// ASCII Packet
		cUOTxAsciiSpeech speech;
		speech.setId(body_);
		speech.setSerial(serial_);
		speech.setMessage(charName + " " + affix);
		speech.setColor(speechColor);
		speech.setFont(3);
		speech.setType(6); // Object Speech
		speech.setName(name_);
		socket->send(&speech);
		// Show it to the socket
		// socket->showSpeech( this, charName, speechColor, 3, cUOTxUnicodeSpeech::System );
		// Names are presented in ASCII speech, Guild titles are not
		//socket->clilocMessage( 1050045, " \t" + charName + "\t " + affix, speechColor, 3, this, true );
	} else {
		switch ( notoriety( socket->player() ) )
		{
		case 0x01:
			speechColor = 0x59; break; //blue
		case 0x02:
			speechColor = 0x3F; break; //green
		case 0x03:
			speechColor = 0x3B2; break; //grey
		case 0x05:
			speechColor = 0x90; break; //orange
		case 0x06:
			speechColor = 0x22; break; //red
		default:
			speechColor = 0x3B2; break; // grey
		}

		if ( isInvulnerable() )
		{
			speechColor = 0x35;
		}

		// Show it to the socket
		// socket->showSpeech( this, charName, speechColor, 3, cUOTxUnicodeSpeech::System );
		// Names are presented in ASCII speech, Guild titles are not
		socket->clilocMessage( 1050045, " \t" + charName + "\t " + affix, speechColor, 3, this, true );
	}
}

/*!
	Make someone criminal.
*/
void cPlayer::makeCriminal()
{
	if ( !isGMorCounselor() )
	{
		// Murderers dont become criminal, they already are!
		if ( !this->isMurderer() )
		{
			// Notify us if we're not already a criminal
			if ( socket_ && !isCriminal() )
			{
				socket_->clilocMessage( 500167 );
			}
			setCriminalTime( Server::instance()->time() + Config::instance()->crimtime() * MY_CLOCKS_PER_SEC );
			changed_ = true;
		}
	}
}

void cPlayer::disturbMed()
{
	if ( isMeditating() ) //Meditation
	{
		this->setMeditating( false );

		if ( socket_ )
			socket_->sysMessage( tr( "You loose your concentration" ) );
	}
}

int cPlayer::countBankGold()
{
	P_ITEM pi = getBankbox(); //we want gold bankbox.
	return pi->countItems( 0x0EED );
}

bool cPlayer::canPickUp( cItem* pi )
{
	if ( account_ && account_->isAllMove() )
		return true;

	if ( pi->isLockedDown() )
		return false;

	else if ( pi->isAllMovable() )
		return true;

	else if ( pi->isOwnerMovable() && !this->owns( pi ) )	// owner movable or locked down ?
		return false;

	tile_st tile = TileCache::instance()->getTile( pi->id() );
	if ( pi->isGMMovable() || ( tile.weight == 255 && !pi->isAllMovable() ) )
		return false;

	return true;
}

void cPlayer::soundEffect( UI16 soundId, bool hearAll )
{
	cUOTxSoundEffect pSoundEffect;
	pSoundEffect.setSound( soundId );
	pSoundEffect.setCoord( pos() );

	if ( !hearAll )
	{
		if ( socket_ )
			socket_->send( &pSoundEffect );
	}
	else
	{
		// Send the sound to all sockets in range
		for ( cUOSocket*s = Network::instance()->first(); s; s = Network::instance()->next() )
			if ( s->player() && s->player()->inRange( this, s->player()->visualRange() ) )
				s->send( &pSoundEffect );
	}
}

void cPlayer::giveGold( Q_UINT32 amount, bool inBank )
{
	P_ITEM pCont = NULL;
	if ( !inBank )
		pCont = getBackpack();
	else
		pCont = getBankbox();

	if ( !pCont )
		return;

	// Begin Spawning
	Q_UINT32 total = amount;

	while ( total > 0 )
	{
		P_ITEM pile = cItem::createFromScript( "eed" );
		pile->setAmount( wpMin<Q_UINT32>( total, static_cast<Q_UINT32>( 65535 ) ) );
		total -= pile->amount();

		pCont->addItem( pile );
		if ( !pile->free )
		{
			pile->update();
		}
	}

	goldSound( amount, false );
}

/*!
	Reduces the specified \a amount of gold from the user
	and returns the amount reduced. If it was successfull, it will return
	the same value passed on \a amount parameter.
*/
Q_UINT32 cPlayer::takeGold( Q_UINT32 amount, bool useBank )
{
	P_ITEM pPack = getBackpack();
	P_ITEM pBank = getBankbox();

	// Count total gold
	unsigned int totalGold = 0;
	totalGold = pPack->countItems(0xEED, 0);

	if (useBank && totalGold < amount) {
		totalGold += pBank->countItems(0xEED, 0);
	}

	if (totalGold < amount) {
		return 0;
	}

	Q_UINT32 dAmount = 0;
	dAmount = pPack->deleteAmount( amount, 0xEED, 0 );

	if (useBank && dAmount > 0) {
		pBank->deleteAmount( dAmount, 0xEED, 0 );
	}

	goldSound(amount, false);
	return amount;
}

bool cPlayer::inWorld()
{
	return socket_ != 0;
}

void cPlayer::giveNewbieItems( Q_UINT8 skill )
{
	const cElement* startItems = Definitions::instance()->getDefinition( WPDT_STARTITEMS, ( skill == 0xFF ) ? QString( "default" ) : Skills::instance()->getSkillDef( skill ).lower() );

	// No Items defined
	if ( !startItems )
		return;

	applyStartItemDefinition( startItems );
}

void cPlayer::applyStartItemDefinition( const cElement* element )
{
	for ( unsigned int i = 0; i < element->childCount(); ++i )
	{
		const cElement* node = element->getChild( i );

		// Apply another startitem definition
		if ( node->name() == "inherit" )
		{
			const cElement* inheritNode = Definitions::instance()->getDefinition( WPDT_STARTITEMS, node->getAttribute( "id" ) );

			if ( inheritNode )
			{
				applyStartItemDefinition( inheritNode );
			}
		}

		// Item related nodes
		else
		{
			P_ITEM pItem = 0;

			if ( node->hasAttribute( "id" ) && node->getAttribute( "id" ) != QString::null )
			{
				pItem = cItem::createFromScript( node->getAttribute( "id" ) );
			}
			else if ( node->hasAttribute( "list" ) && node->getAttribute( "list" ) != QString::null )
			{
				pItem = cItem::createFromList( node->getAttribute( "list" ) );
			}
			else if ( node->hasAttribute( "randomlist" ) && node->getAttribute( "randomlist" ) != QString::null )
			{
				QStringList RandValues = QStringList::split( ",", node->getAttribute( "randomlist" ) );
				pItem = cItem::createFromList( RandValues[RandomNum( 0, RandValues.size() - 1 )] );
			}

			if ( !pItem )
			{
				Console::instance()->log( LOG_ERROR, tr( "Invalid item tag without id or list in startitem definition '%1'" ).arg( element->getAttribute( "id" ) ) );
			}
			else
			{
				pItem->applyDefinition( node );

				if ( node->name() == "item" )
				{
					pItem->toBackpack( this );
				}
				else if ( node->name() == "bankitem" )
				{
					getBackpack()->addItem( pItem );
				}
				else if ( node->name() == "equipment" )
				{
					unsigned char layer = pItem->layer();
					pItem->setLayer( 0 );

					if ( !layer )
					{
						tile_st tile = TileCache::instance()->getTile( pItem->id() );
						layer = tile.layer;
					}

					if ( layer )
					{
						// Check if there is sth there already.
						// Could happen due to inherit.
						P_ITEM existing = atLayer( static_cast<cBaseChar::enLayer>( layer ) );
						if ( existing )
							existing->remove();
						addItem( static_cast<cBaseChar::enLayer>( layer ), pItem );
					}
					else
					{
						Console::instance()->log( LOG_ERROR, tr( "Trying to equip invalid item (%1) in startitem definition '%2'" ).arg( pItem->id(), 0, 16 ).arg( element->getAttribute( "id" ) ) );
					}
				}
				else
				{
					pItem->remove();
					Console::instance()->log( LOG_ERROR, tr( "Unrecognized startitem tag '%1' in definition '%2'." ).arg( node->name() ).arg( element->getAttribute( "id" ) ) );
				}
			}
		}
	}
}

bool cPlayer::checkSkill( UI16 skill, SI32 min, SI32 max, bool advance )
{
	if ( !isJailed() ) {
		if ( isDead() )
		{
			if ( socket_ )
				socket_->clilocMessage( 0x7A12C ); // You cannot use skills while dead.
			return false;
		}

		bool success = cBaseChar::checkSkill( skill, min, max, advance );

		return success;
	} else {
		return false; // In jail, there is no skillgain or skillcheck.
	}
}

void cPlayer::addPet( P_NPC pPet, bool noOwnerChange )
{
	if ( noOwnerChange )
	{
		// Temp Warning Fix
	}

	if ( !pPet )
		return;

	// It may be the follower of someone else already, so
	// check that...
	if ( pPet->owner() && pPet->owner() != this )
		pPet->owner()->removePet( pPet, true );

	// Only reset the owner if we have slots left
	if ( pets_.size() + pPet->controlSlots() <= maxControlSlots() )
	{
		pPet->setOwner( this, true );

		// Check if it already is our follower
		CharContainer::iterator it = std::find( pets_.begin(), pets_.end(), pPet );
		if ( it != pets_.end() )
			return;

		pets_.push_back( pPet );

		if ( socket_ )
		{
			socket_->sendStatWindow();
		}
	}
}

void cPlayer::removePet( P_NPC pPet, bool noOwnerChange )
{
	if ( !pPet )
		return;

	CharContainer::iterator it = std::find( pets_.begin(), pets_.end(), pPet );
	if ( it != pets_.end() )
		pets_.erase( it );

	if ( !noOwnerChange )
	{
		pPet->setOwner( NULL, true );
	}

	if ( socket_ )
	{
		socket_->sendStatWindow();
	}
}

bool cPlayer::onTradeStart( P_PLAYER partner, P_ITEM firstitem )
{
	bool result = false;

	if ( canHandleEvent( EVENT_TRADESTART ) )
	{
		PyObject* args = Py_BuildValue( "(O&O&O&)", PyGetCharObject, this, PyGetCharObject, partner, PyGetItemObject, firstitem );
		result = callEventHandler( EVENT_TRADESTART, args );
		Py_DECREF( args );
	}

	return result;
}

bool cPlayer::onTrade( unsigned int type, unsigned int buttonstate, SERIAL itemserial )
{
	bool result = false;

	if ( canHandleEvent( EVENT_TRADE ) )
	{
		PyObject* args = Py_BuildValue( "(O&iii)", PyGetCharObject, this, type, buttonstate, itemserial );

		result = callEventHandler( EVENT_TRADE, args );

		Py_DECREF( args );
	}

	return result;
}
bool cPlayer::onPickup( P_ITEM pItem )
{
	bool result = false;

	if ( canHandleEvent( EVENT_PICKUP ) )
	{
		PyObject* args = Py_BuildValue( "O&O&", PyGetCharObject, this, PyGetItemObject, pItem );

		result = callEventHandler( EVENT_PICKUP, args );

		Py_DECREF( args );
	}

	return result;
}

bool cPlayer::onLogin()
{
	// move the char from the offline to the online chars structure
	MapObjects::instance()->updateOnlineStatus( this, true );

	// trigger the script event
	bool result = false;
	if ( canHandleEvent( EVENT_LOGIN ) )
	{
		PyObject* args = Py_BuildValue( "(O&)", PyGetCharObject, this );
		result = callEventHandler( EVENT_LOGIN, args );
		Py_DECREF( args );
	}
	return result;
}

bool cPlayer::onConnect( bool reconnecting )
{
	bool result = false;
	if ( canHandleEvent( EVENT_CONNECT ) )
	{
		PyObject* args = Py_BuildValue( "(O&i)", PyGetCharObject, this, reconnecting );
		result = callEventHandler( EVENT_CONNECT, args );
		Py_DECREF( args );
	}
	return result;
}

bool cPlayer::onDisconnect()
{
	bool result = false;
	if ( canHandleEvent( EVENT_DISCONNECT ) )
	{
		PyObject* args = Py_BuildValue( "(O&)", PyGetCharObject, this );
		result = callEventHandler( EVENT_DISCONNECT, args );
		Py_DECREF( args );
	}
	return result;
}

bool cPlayer::onLogout()
{
	// move the char from the online to the offline chars structure
	if ( !pos_.isInternalMap() )
	{
		MapObjects::instance()->updateOnlineStatus( this, false );
	}

	// trigger the script event
	bool result = false;
	if ( canHandleEvent( EVENT_LOGOUT ) )
	{
		PyObject* args = Py_BuildValue( "(O&)", PyGetCharObject, this );
		result = callEventHandler( EVENT_LOGOUT, args );
		Py_DECREF( args );
	}
	return result;
}

// The character wants help
bool cPlayer::onHelp()
{
	bool result = false;

	if ( canHandleEvent( EVENT_HELP ) )
	{
		PyObject* args = Py_BuildValue( "(O&)", PyGetCharObject, this );

		result = callEventHandler( EVENT_HELP, args );

		Py_DECREF( args );
	}

	return result;
}

// The character wants to chat
bool cPlayer::onChat()
{
	bool result = false;

	if ( canHandleEvent( EVENT_CHAT ) )
	{
		PyObject* args = Py_BuildValue( "(O&)", PyGetCharObject, this );
		result = callEventHandler( EVENT_CHAT, args );
		Py_DECREF( args );
	}

	return result;
}

bool cPlayer::onUse( P_ITEM pItem )
{
	bool result = false;

	if ( canHandleEvent( EVENT_USE ) )
	{
		PyObject* args = Py_BuildValue( "O&O&", PyGetCharObject, this, PyGetItemObject, pItem );
		result = callEventHandler( EVENT_USE, args );
		Py_DECREF( args );
	}

	return result;
}

bool cPlayer::onCastSpell( unsigned int spell )
{
	// In jail we can't cast spells
	if (isJailed()) {
		sysmessage(tr("You cannot cast spells while you are in jail."));
		return false;
	}

	bool result = false;

	if ( canHandleEvent( EVENT_CASTSPELL ) )
	{
		PyObject* args = Py_BuildValue( "(O&i)", PyGetCharObject, this, spell );
		result = callEventHandler( EVENT_CASTSPELL, args );
		Py_DECREF( args );
	}

	return result;
}

void cPlayer::setStamina( Q_INT16 data, bool notify /* = true */ )
{
	bool update = false;
	if ( data != stamina() && notify )
		update = true;
	cBaseChar::setStamina( data );
	if ( update && socket() )
		socket()->updateStamina( this );
}

// Simple setting and getting of properties for scripts and the set command.
stError* cPlayer::setProperty( const QString& name, const cVariant& value )
{
	changed( TOOLTIP );
	changed_ = true;

	/*
		\property char.account The name of the account assigned to this player.
		You can change the account by assignging a new name to this string property.
		This property is exclusive to player objects.
	*/
	if ( name == "account" )
	{
		setAccount( Accounts::instance()->getRecord( value.toString() ) );
		return 0;
	}
	/*
		\property char.logouttime This integer property is used when a player disconnects in an unsafe area.
		This property indicates the time when the character will disappear from the world.
		This property is exclusive to players.
	*/
	else
		SET_INT_PROPERTY( "logouttime", logoutTime_ )

		/*
		\property char.maxcontrolslots This property indicates how many control slots this character has.
		This property is exclusive to players.
		*/
	else
		SET_INT_PROPERTY( "maxcontrolslots", maxControlSlots_ )

		/*
		\property char.lightbonus This is the lightlevel bonus applied for this player.
		*/
	else
		SET_INT_PROPERTY( "lightbonus", fixedLightLevel_ )
		/*
		\property char.objectdelay If this integer property is not zero, it indicates the servertime in miliseconds
		when the player will be able to use an item again. Containers ignore this value.
		This property is exclusive to player objects.
		*/
	else
		SET_INT_PROPERTY( "objectdelay", objectDelay_ )
		/*
		\property char.visrange This integer property indicates how far the player can see in tiles.
		This property is exclusive to player objects.
		*/
	else
		SET_INT_PROPERTY( "visrange", visualRange_ )
		/*
		\property char.profile This string property contains the players profile.
		This property is exclusive to player objects.
		*/
	else
		SET_STR_PROPERTY( "profile", profile_ )
		/*
		\property char.strengthlock This integer property indicates the lock status for the players strength.
		It can be one of the following values:
		<code>0x00 Up
		0x01 Down
		0x02 Locked</code>
		This property is exclusive to player objects.
		*/
	else
		SET_INT_PROPERTY( "strengthlock", strengthLock_ )
		/*
		\property char.dexteritylock This integer property indicates the lock status for the players dexterity.
		It can be one of the following values:
		<code>0x00 Up
		0x01 Down
		0x02 Locked</code>
		This property is exclusive to player objects.
		*/
	else
		SET_INT_PROPERTY( "dexteritylock", dexterityLock_ )
		/*
		\property char.intelligencelock This integer property indicates the lock status for the players intelligence.
		It can be one of the following values:
		<code>0x00 Up
		0x01 Down
		0x02 Locked</code>
		This property is exclusive to player objects.
		*/
	else
		SET_INT_PROPERTY( "intelligencelock", intelligenceLock_ )

	/*
	\property char.karmalock If this property is True, the karma of this player won't raise anymore. If it's False,
	the normal karma raising occurs.
	*/
	else if ( name == "karmalock" ) {
		setKarmaLock(value.toInt() != 0);
		return 0;
	}

	/*
	\property char.squelched If this property is true, speech sent by the client will be rejected.
	This property is exclusive to player characters.
	*/
	else if ( name == "squelched" ) {
		setSquelched(value.toInt() != 0);
		return 0;
	}

	/*
	\property char.jailed If this property is True, no items can be used by the player,
	no skills can be used by the player, no skillgain is possible, no spells can be cast by the player,
	no combat is possible and no items can be used.
	This property is exclusive to player characters.
	*/
	else if ( name == "jailed" ) {
		setJailed(value.toInt() != 0);
		return 0;
	}

	else if ( name.left( 6 ) == "skill." )
	{
		QString skill = name.right( name.length() - 6 );
		Q_INT16 skillId = Skills::instance()->findSkillByDef( skill );

		if ( skillId != -1 )
		{
			setSkillValue( skillId, value.toInt() );
			if ( socket_ )
				socket_->sendSkill( skillId );
			return 0;
		}

		// skillcap.
	}
	else if ( name.left( 9 ) == "skillcap." )
	{
		QString skill = name.right( name.length() - 9 );
		Q_INT16 skillId = Skills::instance()->findSkillByDef( skill );

		if ( skillId != -1 )
		{
			setSkillCap( skillId, value.toInt() );
			if ( socket_ )
				socket_->sendSkill( skillId );
			return 0;
		}
	}
	else if ( name.startsWith( "account." ) && account_ )
	{
		return account_->setProperty( name.right( name.length() - 8 ), value );
	}
	else
	{
		Q_INT16 skillId = Skills::instance()->findSkillByDef( name );
		if ( skillId != -1 )
		{
			setSkillValue( skillId, value.toInt() );
			if ( socket_ )
				socket_->sendSkill( skillId );
			return 0;
		}
	}

	return cBaseChar::setProperty( name, value );
}

PyObject* cPlayer::getProperty( const QString& name )
{
	PY_PROPERTY( "account", account_ )
	/*
	\rproperty controlslots The amount of controlslots currently used for this
	player.
	This property is only available for player objects.
	*/
	PY_PROPERTY( "controlslots", controlslots() )
	PY_PROPERTY( "logouttime", logoutTime_ )
	PY_PROPERTY( "npc", false )
	PY_PROPERTY( "lightbonus", fixedLightLevel_ )
	PY_PROPERTY( "objectdelay", objectDelay_ )
	PY_PROPERTY( "visrange", visualRange_ )
	PY_PROPERTY( "profile", profile_ )
	PY_PROPERTY( "strengthlock", strengthLock_ )
	PY_PROPERTY( "dexteritylock", dexterityLock_ )
	PY_PROPERTY( "intelligencelock", intelligenceLock_ )
	PY_PROPERTY( "maxcontrolslots", maxControlSlots_ )
	PY_PROPERTY( "karmalock", karmaLock() )
	PY_PROPERTY( "squelched", isSquelched() )
	PY_PROPERTY( "jailed", isJailed() )

	// Forward the property to the account
	if ( name.startsWith( "account." ) && account_ )
	{
		return account_->getProperty( name.right( name.length() - 8 ) );
	}

	return cBaseChar::getProperty( name );
}

void cPlayer::awardFame( short amount, bool showmessage )
{
	if ( Config::instance()->disableFame() )
		return;

	int nCurFame, nChange = 0;
	bool gain = false;

	setFame( wpMin<short>( 10000, fame() ) );

	nCurFame = fame();

	// We already have more than that.
	if ( nCurFame > amount )
		return;

	// Loose Fame when we died
	if ( isDead() )
	{
		// Fame / 25 is our loss
		nChange = nCurFame / 25;
		setFame( wpMax<short>( 0, nCurFame - nChange ) );
		setDeaths( deaths() + 1 );
		gain = false;
	}
	else
	{
		nChange = ( amount - nCurFame ) / 75;
		setFame( nCurFame + nChange );
		gain = true;
	}

	// Nothing changed or we can't recieve the message
	if ( !nChange || !socket() || !showmessage)
		return;

	uint message = 1019051;

	if ( nChange <= 25 )
	{
		if ( gain )
			message = 1019051; //"You have gained a little fame."
		else
			message = 1019055; //"You have lost a little fame."
	}
	else if ( nChange <= 75 )
	{
		if ( gain )
			message = 1019052; //"You have gained some fame."
		else
			message = 1019056; //"You have lost some fame."
	}
	else if ( nChange <= 100 )
	{
		if ( gain )
			message = 1019053; // You have gained a good amount of fame.
		else
			message = 1019057; // You have lost a good amount of fame.
	}
	else if ( nChange > 100 )
	{
		if ( gain )
			message = 1019054; //"You have gained alot of fame."
		else
			message = 1019058; //"You have lost alot of fame."
	}

	socket()->clilocMessage( message );
}

void cPlayer::awardKarma( P_CHAR pKilled, short amount, bool showmessage )
{
	if ( Config::instance()->disableKarma() )
		return;

	if (amount > 0 && karmaLock()) {
		return; // Don't award karma if the karma is locked
	}

	int nCurKarma = 0, nChange = 0, nEffect = 0;

	nCurKarma = karma();

	if ( nCurKarma <amount && amount> 0 )
	{
		nChange = ( ( amount - nCurKarma ) / 75 );
		setKarma( nCurKarma + nChange );
		nEffect = 1;
	}

	if ( ( nCurKarma > amount ) && ( !pKilled ) )
	{
		nChange = ( ( nCurKarma - amount ) / 50 );
		setKarma( nCurKarma - nChange );
		nEffect = 0;
	}
	else if ( ( nCurKarma > amount ) && ( pKilled->karma() > 0 ) )
	{
		nChange = ( ( nCurKarma - amount ) / 50 );
		setKarma( nCurKarma - nChange );
		nEffect = 0;
	}

	// Cap at 10000 or -10000
	if ( karma_ > 10000 )
		karma_ = 10000;
	else if ( karma_ < -10000 )
		karma_ = -10000;

	if ( !nChange || !socket() || !showmessage )
		return;

	Q_UINT32 message = 0xF8CB3;

	if ( nChange <= 25 )
	{
		if ( nEffect )
			message = 0xF8CB3; // You have gained a little karma.
		else
			message = 0xF8CB7; // You have lost a little karma.
	}
	else if ( nChange <= 75 )
	{
		if ( nEffect )
			message = 0xF8CB4; // You have gained some karma.
		else
			message = 0xF8CB8; // You have lost some karma.
	}
	else if ( nChange <= 100 )
	{
		if ( nEffect )
			message = 0xF8CB5; // You have gained a good amount of karma.
		else
			message = 0xF8CB9; // You have lost a good amount of karma.
	}
	else if ( nChange > 100 )
	{
		if ( nEffect )
			message = 0xF8CB6; // You have gained a lot of karma.
		else
			message = 0xF8CBA; // You have lost a lot of karma.
	}

	socket()->clilocMessage( message );
}

void cPlayer::log( eLogLevel loglevel, const QString& string )
{
	Log::instance()->print( loglevel, socket_, string );
}

void cPlayer::log( const QString& string )
{
	log( LOG_NOTICE, string );
}

bool cPlayer::canSeeChar( P_CHAR character )
{
	if ( character != this )
	{
		if ( !character || character->free )
		{
			return false;
		}

		// Check distance
		if ( pos_.distance( character->pos() ) > visualRange() )
		{
			return false;
		}

		P_PLAYER player = dynamic_cast<P_PLAYER>( character );

		// By default we are more privileged than our target if we are a GM
		bool privileged = isGM();

		if ( player )
		{
			if ( privileged )
			{
				// Determine if we are more privileged than the target
				if ( player->account() )
				{
					if ( !account_ || player->account()->rank() > account_->rank() )
					{
						privileged = false;
					}
				}
			}
			else if ( !player->isOnline() )
			{
				// offline players are invisible for normal players
				return false;
			}

			if ( party_ && party_ == player->party() )
			{
				privileged = true;
			}
		}

		// Hidden and invisible characters are invisible unless we are more privileged than them
		if ( character->isInvisible() || character->isHidden() )
		{
			if ( !privileged )
			{
				return false;
			}
		}

		// Dead characters are invisible unless we have more than 100.0% spirit speak
		// or are privileged...
		if ( character->isDead() )
		{
			// Only NPCs with spiritspeak >= 1000 can see dead people
			// or if the AI overrides it
			if ( !character->isAtWar() && skillValue( SPIRITSPEAK ) < 1000 )
			{
				if ( !privileged )
				{
					return false;
				}
			}
		}

		// Check if the target is a npc and currently stabled
		P_NPC npc = dynamic_cast<P_NPC>( character );
		if ( npc && npc->stablemasterSerial() != INVALID_SERIAL )
		{
			return false;
		}
	}

	return true;
}

bool cPlayer::canSeeItem( P_ITEM item )
{
	if ( !item )
	{
		return false;
	}

	if ( item->visible() == 2 )
	{
		if ( !isGM() )
		{
			return false;
		}
	}
	else if ( item->visible() == 1 && item->owner() != this )
	{
		if ( !isGM() )
		{
			return false;
		}
	}

	// Check for container
	if ( item->container() )
	{
		P_ITEM container = dynamic_cast<P_ITEM>( item->container() );

		if ( container )
		{
			return canSeeItem( container );
		}
		else
		{
			P_CHAR character = dynamic_cast<P_CHAR>( item->container() );
			
			// We're equipped by a character. If we're on the mount layer
			// only the equipping character can see us
			if (item->layer() == 0x19 && character != this && !character->isHuman()) {
				return false;
			}
			
			return canSeeChar( character );
		}
	}
	else
	{
		cMulti* multi = dynamic_cast<cMulti*>( item );

		if ( multi )
		{
			if ( pos_.distance( item->pos() ) > BUILDRANGE )
				return false;
		}
		else
		{
			if ( pos_.distance( item->pos() ) > VISRANGE )
				return false;
		}
	}

	return true;
}

PyObject* cPlayer::getPyObject()
{
	return PyGetCharObject( this );
}

const char* cPlayer::className() const
{
	return "player";
}

bool cPlayer::send( cUOPacket* packet )
{
	if ( socket_ )
	{
		socket_->send( packet );
		return true;
	}
	return false;
}

bool cPlayer::message( const QString& message, unsigned short color, cUObject* source, unsigned short font, unsigned char mode )
{
	if ( socket_ )
	{
		if ( !source )
		{
			socket_->showSpeech( this, message, color, font, mode );
		}
		else
		{
			socket_->showSpeech( source, message, color, font, mode );
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool cPlayer::sysmessage( const QString& message, unsigned short color, unsigned short font )
{
	if ( socket_ )
	{
		socket_->sysMessage( message, color, font );
		return true;
	}
	return false;
}

bool cPlayer::sysmessage( unsigned int message, const QString& params, unsigned short color, unsigned short font )
{
	if ( socket_ )
	{
		socket_->clilocMessage( message, params, color, font );
		return true;
	}
	return false;
}

cBaseChar::FightStatus cPlayer::fight( P_CHAR enemy )
{
	// Jailed players cannot fight.
	if (isJailed()) {
		return FightDenied;
	}

	FightStatus status = cBaseChar::fight( enemy );

	// We have player dependant actions if the fight started
	if ( status != FightDenied )
	{
		// Notoriety handling only if the fight started
		if ( status == FightStarted )
		{
			cFightInfo* fight = findFight( enemy );

			if ( fight && fight->attacker() == this && !fight->legitimate() )
			{
				if (!inNoCriminalCombatArea())
					makeCriminal();
				log(LOG_TRACE, tr("Started fight with character %1 (0x%2).\n").arg(enemy->orgName()).arg(enemy->serial(), 0, 16));
			}
		}

		// Always stop meditating
		disturbMed();
	}

	return status;
}

void cPlayer::createTooltip( cUOTxTooltipList& tooltip, cPlayer* player )
{
	cUObject::createTooltip( tooltip, player );

	QString affix( " " );

	// Append the (frozen) tag
	if ( isFrozen() )
	{
		affix = " (frozen)";
	}

	if ( player->account()->isShowSerials() )
	{
		affix.append( QString( " [0x%1]" ).arg( serial(), 3, 16 ) );
	}

	if ( !isIncognito() && !isPolymorphed() && guild_ && !guild_->abbreviation().isEmpty() )
	{
		cGuild::MemberInfo* info = guild_->getMemberInfo( this );

		if ( info && info->showSign() )
		{
			if ( affix.length() == 1 )
			{
				affix = QString( " [%1]" ).arg( guild_->abbreviation() );
			}
			else
			{
				affix.append( QString( " [%1]" ).arg( guild_->abbreviation() ) );
			}
		}
	}

	// Don't miss lord and lady titles
	if ( !isIncognito() && !isPolymorphed() && fame_ >= 10000 && !isReputationHidden() )
	{
		if ( gender() )
		{
			tooltip.addLine( 1050045, tr( "Lady \t%1\t%2" ).arg( name_ ).arg( affix ) );
		}
		else
		{
			tooltip.addLine( 1050045, tr( "Lord \t%1\t%2" ).arg( name_ ).arg( affix ) );
		}
	}
	else
	{
		tooltip.addLine( 1050045, QString( " \t%1\t%2" ).arg( name_ ).arg( affix ) );
	}

	// Append guild title and name
	if ( !isIncognito() && !isPolymorphed() && guild_ )
	{
		cGuild::MemberInfo* info = guild_->getMemberInfo( this );

		if ( info && info->showSign() )
		{
			if ( !info->guildTitle().isEmpty() )
			{
				tooltip.addLine( 1042971, QString( "%1, %2" ).arg( info->guildTitle() ).arg( guild_->name() ) );
			}
			else
			{
				tooltip.addLine( 1042971, guild_->name() );
			}
		}
	}

	onShowTooltip( player, &tooltip );
}

void cPlayer::poll( unsigned int time, unsigned int events )
{
	cBaseChar::poll( time, events );

	// Process an environmental light change if we're not in a cave.
	if ( socket_ )
	{
		if ( events & EventLight )
		{
			cTerritory* region = Territories::instance()->region( pos() );
			if ( !region || !region->isCave() )
			{
				socket_->updateLightLevel();
			}
		}

		if ( events & EventTime )
		{
			if ( canHandleEvent( EVENT_TIMECHANGE ) )
			{
				PyObject* args = Py_BuildValue( "(N)", getPyObject() );
				callEventHandler( EVENT_TIMECHANGE, args );
				Py_DECREF( args );
			}
		}
	}
}

unsigned char cPlayer::controlslots() const
{
	unsigned char controlslots = 0;
	CharContainer::const_iterator it;
	for ( it = pets_.begin(); it != pets_.end(); ++it )
	{
		P_NPC npc = dynamic_cast<P_NPC>( *it );
		if ( npc )
		{
			controlslots += npc->controlSlots();
		}
	}
	return controlslots;
}

void cPlayer::moveTo( const Coord& pos )
{
	Coord oldpos = pos_;
	cBaseChar::moveTo( pos );

	if ( !pos_.isInternalMap() && socket_ && oldpos.map != pos_.map )
	{
		socket_->resendPlayer( false );
	}
}

void cPlayer::remove()
{
	if ( socket_ )
	{
		socket_->disconnect();
	}

	if ( account_ )
	{
		account_->removeCharacter( this );
	}

	if ( party_ )
	{
		party_->removeMember( this );
	}
	else
	{
		Timers::instance()->dispel( this, 0, "cancelpartyinvitation", false, false );
	}

	if ( guild_ )
	{
		guild_->removeMember( this );
	}

	// Remove the owner tag from all of our followers
	CharContainer::iterator it;
	for ( it = pets_.begin(); it != pets_.end(); ++it )
	{
		P_NPC npc = dynamic_cast<P_NPC>( ( *it ) );

		if ( npc && npc->owner() == this )
		{
			npc->setOwner( NULL, true );
			npc->setTamed( false );

			// Remove his stabled NPCs.
			if ( npc->stablemasterSerial() != INVALID_SERIAL )
			{
				npc->remove();
			}
		}
	}
	pets_.clear();

	cBaseChar::remove();
}

unsigned int cPlayer::damage( eDamageType type, unsigned int amount, cUObject* source )
{
	// Save the hitpoints for further calculation
	unsigned int hitpoints = hitpoints_;
	unsigned int stamina = stamina_;

	amount = cBaseChar::damage( type, amount, source );

	if ( amount != 0 )
	{
		// the more stamina we have, the more we loose
		// the more hitpoints we have, the less we loose
		int value = ( int ) ( amount * ( 100.0 / hitpoints ) * ( stamina / 100.0 ) ) - 5;
		if ( value > 0 )
		{
			stamina_ = wpMax<short>( 0, stamina_ - value );
			if ( socket_ )
			{
				socket_->updateStamina();
			}
		}
	}

	return amount;
}

bool cPlayer::isOverloaded()
{
	if ( isDead() || isGMorCounselor() )
	{
		return false;
	}

	return weight_ > maxWeight();
}

unsigned int cPlayer::maxWeight()
{
	return ( unsigned int ) ( 40 + strength_ * 3.5 );
}
