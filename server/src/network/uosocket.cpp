/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
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

#include "uosocket.h"
#include "uopacket.h"
#include "uotxpackets.h"
#include "asyncnetio.h"
#include "network.h"

// Wolfpack Includes
#include "../corpse.h"
#include "../party.h"
#include "../accounts.h"

#include "../basics.h"
#include "../console.h"
#include "../territories.h"
#include "../inlines.h"
#include "../mapobjects.h"
#include "../multi.h"
#include "../muls/maps.h"
#include "../speech.h"
#include "../commands.h"
#include "../serverconfig.h"
#include "../definitions.h"
#include "../scriptmanager.h"
#include "../walking.h"
#include "../combat.h"
#include "../gumps.h"
#include "../skills.h"
#include "../contextmenu.h"
#include "../timers.h"
#include "../targetrequests.h"
#include "../dragdrop.h"
#include "../trade.h"
#include "../uobject.h"
#include "../player.h"
#include "../basechar.h"
#include "../npc.h"
#include "../log.h"
#include "../ai/ai.h"
#include "../python/pypacket.h"

#include <stdlib.h>
#include <qhostaddress.h>

#include <vector>
#include <qvaluelist.h>
#include <functional>

using namespace std;

/*****************************************************************************
  cUOSocket member functions
 *****************************************************************************/

/*!
  \class cUOSocket uosocket.h

  \brief The cUOSocket class provides an abstraction of Ultima Online network
  connected sockets. It works with Application level packets provided by \sa cAsyncNetIO.

  \ingroup network
  \ingroup mainclass
*/

/*!
  Constructs a cUOSocket attached to \a sDevice system socket.
  Ownership of \a sDevice will be transfered to cUOSocket, that is,
  cUOSocket will call delete on the given pointer when it's destructed.
*/
cUOSocket::cUOSocket( QSocketDevice* sDevice ) : _walkSequence( 0 ), lastPacket( 0xFF ), _state( LoggingIn ), _lang( "ENU" ), targetRequest( 0 ), _account( 0 ), _player( 0 ), _rxBytes( 0 ), _txBytes( 0 ), _socket( sDevice ), _screenWidth( 640 ), _screenHeight( 480 )
{
	_txBytesRaw = 0;
	flags_ = 0;
	_socket->resetStatus();
	_ip = _socket->peerAddress().toString();
	_uniqueId = sDevice->socket();
	tooltipscache_ = new QBitArray;

	// Creation of a new socket counts as activity
	_lastActivity = getNormalizedTime();
}

// Initialize all packet handlers to zero
PyObject* cUOSocket::handlers[255] =
{
0,
};

void cUOSocket::registerPacketHandler( unsigned char packet, PyObject* handler )
{
	if ( handlers[packet] )
	{
		Py_DECREF( handlers[packet] );
		handlers[packet] = 0;
	}

	// Only install callable packet handlers.
	if ( handler && PyCallable_Check( handler ) )
	{
		Py_INCREF( handler );
		handlers[packet] = handler;
	}
	else
	{
		// Clear the packet handler.
		handlers[packet] = 0;
	}
}

void cUOSocket::clearPacketHandlers()
{
	for ( int i = 0; i < 255; ++i )
	{
		if ( handlers[i] )
		{
			Py_DECREF( handlers[i] );
			handlers[i] = 0;
		}
	}
}

/*!
  Destructs the cUOSocket instance.
*/
cUOSocket::~cUOSocket( void )
{
	delete _socket;
	delete targetRequest;
	delete tooltipscache_;

	QMap<SERIAL, cGump*>::iterator it( gumps.begin() );
	while ( it != gumps.end() )
	{
		delete it.data();
		++it;
	}
}

/*!
  Sends \a packet to client.
*/
void cUOSocket::send( cUOPacket* packet )
{
	// Don't send when we're already disconnected
	if ( !_socket || !_socket->isOpen() )
		return;

	// Since the compressed packet will be cached anyway, there
	// is no problem calling it here.
	_txBytesRaw += packet->size();
	if ( _state != LoggingIn ) {
		_txBytes += packet->compressed().size();
	} else {
		_txBytes += packet->size();
	}

	Network::instance()->netIo()->sendPacket( _socket, packet, ( _state != LoggingIn ) );

	// Once send, flush if in Debug mode
#if defined(_DEBUG)
	//Network::instance()->netIo()->flush( _socket );
#endif
}

/*!
  Sends \a gump to client.
*/
void cUOSocket::send( cGump* gump )
{
	if ( gump->serial() == INVALID_SERIAL )
	{
		while ( gump->serial() == INVALID_SERIAL || ( gumps.contains( gump->serial() ) ) )
			gump->setSerial( RandomNum( 0x10000000, 0x1000FFFF ) );
		// I changed this, everything between 0x10000000 and 0x1000FFFF is randomly generated
	}
	else if ( gumps.contains( gump->serial() ) ) // Remove/Timeout the old one first
	{
		QMap<SERIAL, cGump*>::iterator it( gumps.find( gump->serial() ) );
		delete it.data();
		gumps.erase( it );
	}

	gumps.insert( gump->serial(), gump );

	QString layout = gump->layout().join( "" );
	if ( gump->noClose() )
		layout.prepend( "{noclose}" );

	if ( gump->noMove() )
		layout.prepend( "{nomove}" );

	if ( gump->noDispose() )
		layout.prepend( "{nodispose}" );

	Q_UINT32 gumpsize = 24 + layout.length();
	QStringList text = gump->text();
	QStringList::const_iterator it = text.begin();
	while ( it != text.end() )
	{
		gumpsize += ( *it ).length() * 2 + 2;
		++it;
	}
	cUOTxGumpDialog uoPacket( gumpsize );

	uoPacket.setSerial( gump->serial() );
	uoPacket.setType( gump->type() );
	uoPacket.setX( gump->x() );
	uoPacket.setY( gump->y() );
	uoPacket.setContent( layout, text );

	send( &uoPacket );
}

/*!
  Tries to receive and dispatch a packet.
*/
void cUOSocket::recieve()
{
	cUOPacket* packet = Network::instance()->netIo()->recvPacket( _socket );

	if ( !packet )
		return;

	// Increase rx counter
	_rxBytes += packet->size();

	unsigned char packetId = ( *packet )[0];

	// Disconnect harmful clients
	if ( ( _account == 0 ) && ( packetId != 0x80 ) && ( packetId != 0x91 ) )
	{
		log( tr( "Communication error: 0x%1 instead of 0x80 or 0x91\n" ).arg( packetId, 2, 16 ) );

		cUOTxDenyLogin denyLogin;
		denyLogin.setReason( cUOTxDenyLogin::DL_BADCOMMUNICATION );
		send( &denyLogin );

		disconnect();

		return;
	}

	// Switch to encrypted mode if one of the advanced packets is recieved
	if ( packetId == 0x91 )
		_state = LoggedIn;

	// Check for a list of packets that may be sent while no player has been selected
	if ( !_player )
	{
		if ( packetId != 0 && packetId != 0x5D && packetId != 0x73 && packetId != 0x80 && packetId != 0x83 && packetId != 0x91 && packetId != 0xA0 && packetId != 0xA4 && packetId != 0xBD && packetId != 0xBF && packetId != 0xC8 && packetId != 0xD9 )
		{
			return;
		}
	}

	// This is always checked before anything else
	if ( packetId == 0x02 && Config::instance()->antiSpeedHack() )
	{
		if ( _player && !_player->isGM() )
		{
			// There are two different delays for mounted and unmounted players
			unsigned int delay;
			if ( !_player->atLayer( cBaseChar::Mount ) )
			{
				delay = Config::instance()->antiSpeedHackDelay();
			}
			else
			{
				delay = Config::instance()->antiSpeedHackDelayMounted();
			}

			// If the last movement of our player was not X ms in the past,
			// requeue the walk request until we can fullfil it.
			//unsigned int time = getNormalizedTime();
			unsigned int time = Server::instance()->time();
			if ( _player->lastMovement() + delay > time )
			{
				//sysMessage(QString("Delayed Walk Request, Last WalkRequest was %1 ms ago.").arg(time - _player->lastMovement()));
				Network::instance()->netIo()->pushfrontPacket( _socket, packet );
				return;
			}
		}
	}

	if ( handlers[packetId] )
	{
		PyObject* args = Py_BuildValue( "(NN)", PyGetSocketObject( this ), CreatePyPacket( packet ) );
		PyObject* result = PyObject_CallObject( handlers[packetId], args );
		Py_DECREF( args );

		bool handled = result && PyObject_IsTrue( result );
		Py_XDECREF( result );
		reportPythonError();

		// Override the internal packet handler.
		if ( handled )
		{
			_lastActivity = getNormalizedTime();
			delete packet;
			return;
		}
	}

	// Relay it to the handler functions
	switch ( packetId )
	{
		case 0x00:
			handleCreateChar( dynamic_cast<cUORxCreateChar*>( packet ) );
			break;
		case 0x01:
			// Disconnect Notification recieved, should NEVER happen as it's unused now
			disconnect();
			break;
		case 0x02:
			// just want to walk a little.
			handleWalkRequest( dynamic_cast<cUORxWalkRequest*>( packet ) );
			break;
		case 0x05:
			handleRequestAttack( dynamic_cast<cUORxRequestAttack*>( packet ) );
			break;
		case 0x06:
			handleDoubleClick( dynamic_cast<cUORxDoubleClick*>( packet ) );
			break;
		case 0x07:
			DragAndDrop::grabItem( this, dynamic_cast<cUORxDragItem*>( packet ) );
			break;
		case 0x08:
			DragAndDrop::dropItem( this, dynamic_cast<cUORxDropItem*>( packet ) );
			break;
		case 0x09:
			handleRequestLook( dynamic_cast<cUORxRequestLook*>( packet ) );
			break;
		case 0x12:
			handleAction( dynamic_cast<cUORxAction*>( packet ) );
			break;
		case 0x13:
			DragAndDrop::equipItem( this, dynamic_cast<cUORxWearItem*>( packet ) );
			break;
		case 0x22:
			resync();
			break;
		case 0x2C:
			handleResurrectionMenu( dynamic_cast<cUORxResurrectionMenu*>( packet ) ); break;
		case 0x34:
			handleQuery( dynamic_cast<cUORxQuery*>( packet ) );
			break;
		case 0x3A:
			handleSkillLock( dynamic_cast<cUORxSkillLock*>( packet ) );
			break;
		case 0x3B:
			handleBuy( dynamic_cast<cUORxBuy*>( packet ) );
			break;
		case 0x5D:
			handlePlayCharacter( dynamic_cast<cUORxPlayCharacter*>( packet ) );
			break;
		case 0x6c:
			handleTarget( dynamic_cast<cUORxTarget*>( packet ) );
			break;
		case 0x6F:
			handleSecureTrading( dynamic_cast<cUORxSecureTrading*>( packet ) );
			break;
		case 0x72:
			handleChangeWarmode( dynamic_cast<cUORxChangeWarmode*>( packet ) );
			break;
		case 0x73:
			break; // Pings are handeled
		case 0x75:
			handleRename( dynamic_cast<cUORxRename*>( packet ) );
			break;
		case 0x80:
			handleLoginRequest( dynamic_cast<cUORxLoginRequest*>( packet ) );
			break;
		case 0x83:
			handleDeleteCharacter( dynamic_cast<cUORxDeleteCharacter*>( packet ) );
			break;
		case 0x91:
			handleServerAttach( dynamic_cast<cUORxServerAttach*>( packet ) );
			break;
		case 0x98:
			handleAllNames( dynamic_cast<cUORxAllNames*>( packet ) );
			break;
		case 0x9B:
			handleHelpRequest( dynamic_cast<cUORxHelpRequest*>( packet ) );
			break;
		case 0x9F:
			handleSell( dynamic_cast<cUORxSell*>( packet ) ); break;
		case 0xA0:
			handleSelectShard( dynamic_cast<cUORxSelectShard*>( packet ) );
			break;
		case 0xA4:
			handleHardwareInfo( dynamic_cast<cUORxHardwareInfo*>( packet ) );
			break;
		case 0xA7:
			handleGetTip( dynamic_cast<cUORxGetTip*>( packet ) );
			break;
		case 0xAD:
			handleSpeechRequest( dynamic_cast<cUORxSpeechRequest*>( packet ) );
			break;
		case 0xB1:
			handleGumpResponse( dynamic_cast<cUORxGumpResponse*>( packet ) );
			break;
		case 0xB5:
			handleChat( packet ); break;
		case 0xB8:
			handleProfile( dynamic_cast<cUORxProfile*>( packet ) );
			break;
		case 0xBD:
			_version = dynamic_cast<cUORxSetVersion*>( packet )->version();
			break;
		case 0xBF:
			handleMultiPurpose( dynamic_cast<cUORxMultiPurpose*>( packet ) );
			break;
		case 0xC8:
			handleUpdateRange( dynamic_cast<cUORxUpdateRange*>( packet ) );
			break;
		case 0xD7:
			handleAosMultiPurpose( dynamic_cast<cUORxAosMultiPurpose*>( packet ) );
			break;
		case 0xB6:
			break; // Completely ignore the packet.
		case 0xBB:
			break; // Completely ignore the packet.
		default:
			Console::instance()->send( packet->dump( packet->uncompressed() ) );
			delete packet;
			return;
	}

	// We received a packet we know
	_lastActivity = getNormalizedTime();

	delete packet;
}

/*!
	\brief This method handles processing of party messages.
*/
void cUOSocket::handleParty( cUOPacket* packet )
{
	cParty::handlePacket( this, packet );
}

/*!
  This method handles cUORxLoginRequest packet types.
  \sa cUORxLoginRequest
*/
void cUOSocket::handleLoginRequest( cUORxLoginRequest* packet )
{
	if ( _account )
	{
		sendCharList();
		return;
	}
	// If we dont authenticate disconnect us
	if ( !authenticate( packet->username(), packet->password() ) )
	{
		disconnect();
		return;
	}

	// Otherwise build the shard-list
	cUOTxShardList shardList;

	QValueVector<ServerList_st> shards = Config::instance()->serverList();

	for ( Q_UINT8 i = 0; i < shards.size(); ++i )
	{
		ServerList_st server = shards[i];
		// we are connecting from the same ip, send 127.0.0.1 as the ip
		if ( server.address.toString() == ip().latin1() )
		{
			shardList.addServer( i, server.sServer, 0x00, server.uiTime, 0x7F000001 );
		}
		else
		{
			shardList.addServer( i, server.sServer, 0x00, server.uiTime, server.address.toIPv4Address() );
		}
	}

	send( &shardList );
}

/*!
  This method handles cUORxHardwareInfo packet types.
  \sa cUORxHardwareInfo
*/
void cUOSocket::handleHardwareInfo( cUORxHardwareInfo* packet )
{
	Q_UNUSED( packet );
	// Do something with the retrieved hardware information here
	// > Hardware Log ??
	//QString hardwareMsg = QString( "Hardware: %1 Processors [Type: %2], %2 MB RAM, %3 MB Harddrive" ).arg( packet->processorCount() ).arg( packet->processorType() ).arg( packet->memoryInMb() ).arg( packet->largestPartitionInMb() );
	//cout << hardwareMsg.latin1() << endl;
}

/*!
  This method provides a way of interrupting the client's connection
  to this server.
*/
void cUOSocket::disconnect()
{
	if ( _account )
		_account->setInUse( false );

	if ( _player )
	{
		_player->onDisconnect();
		_player->setSocket( 0 );

		// Remove the player from it's party
		if ( _player->party() )
		{
			_player->party()->removeMember( _player );
		}
		else
		{
			Timers::instance()->dispel( _player, 0, "cancelpartyinvitation", false, false );
		}
	}

	Network::instance()->netIo()->flush( _socket );
	_socket->close();

	if ( _player )
	{
		_player->removeFromView( true );

		// is the player allowed to logoff instantly?
		if ( _player->isGMorCounselor() || ( _player->region() && _player->region()->isGuarded() ) )
		{
			_player->onLogout();
		}
		else
		{
			// let the player linger...
			_player->setLogoutTime( Server::instance()->time() + Config::instance()->quittime() * 1000 );
		}

		_player->resend( false );
	}
}

/*!
  This method handles cUORxSelectShard packet types.
  \sa cUORxSelectShard
*/
void cUOSocket::handleSelectShard( cUORxSelectShard* packet )
{
	// Relay him - save an auth-id so we recog. him when he relays locally
	QValueVector<ServerList_st> shards = Config::instance()->serverList();

	if ( packet->shardId() >= shards.size() )
	{
		disconnect();
		return;
	}

	cUOTxRelayServer* relay = new cUOTxRelayServer;
	relay->setServerIp( shards[packet->shardId()].address.toIPv4Address() );
	relay->setServerPort( shards[packet->shardId()].uiPort );
	relay->setAuthId( 0xFFFFFFFF ); // This is NO AUTH ID !!!
	// This is the thing it sends next time it connects to
	// know whether it's gameserver or loginserver encryption
	send( relay );
	delete relay;
}

/*!
  This method handles cUORxServerAttach packet types.
  \sa cUORxServerAttach
*/
void cUOSocket::handleServerAttach( cUORxServerAttach* packet )
{
	// Re-Authenticate the user !!
	if ( !authenticate( packet->username(), packet->password() ) )
		disconnect();
	else
	{
		sendCharList();
	}
}

/*!
  This method sends the list of Characters this account have
  during the login process.
  \sa cUOTxCharTownList
*/
void cUOSocket::sendCharList()
{
	// NOTE:
	// Send the server/account features here as well
	// AoS needs it most likely for account creation
	const uint maxChars = wpMin<uint>( 6, Config::instance()->maxCharsPerAccount() );
	cUOTxClientFeatures clientFeatures;
	clientFeatures.setLbr( true );
	clientFeatures.setT2a( true );
	clientFeatures.setAos( true );
	clientFeatures.setAllowPaladinNecromancer( true );
	if ( maxChars == 6 )
		clientFeatures.setSixthCharacterSlot( true );
	//clientFeatures.setShort( 1, 0xFFFF ); // AoS TEST
	clientFeatures.setShort( 1, 0x80FF );
	send( &clientFeatures );

	cUOTxCharTownList charList;
	QValueVector<P_PLAYER> characters = _account->caracterList();

	// Add the characters
	Q_UINT8 i = 0;
	for ( ; i < characters.size(); ++i )
		charList.addCharacter( characters.at( i )->name() );

	// Add the Starting Locations
	vector<StartLocation_st> startLocations = Config::instance()->startLocation();
	for ( i = 0; i < startLocations.size(); ++i )
		charList.addTown( i, startLocations[i].name, startLocations[i].name );

	charList.setAgeOfShadows( true ); // Activates Heaven city
	charList.setContextMenus( true );
	if ( maxChars == 1 )
		charList.setOneCharacter( true );
	else if ( maxChars == 6 )
		charList.setEnableSixthSlot( true );

	charList.compile();
	send( &charList );

	// Ask the client for a viewrange
	cUOTxUpdateRange range;
	range.setRange( VISRANGE );
	send( &range );
}

/*!
  This method handles cUORxDeleteCharacter packet types.
  It will also resend the updated character list
  \sa cUORxDeleteCharacter
*/
void cUOSocket::handleDeleteCharacter( cUORxDeleteCharacter* packet )
{
	QValueVector<P_PLAYER> charList = _account->caracterList();

	if ( packet->index() >= charList.size() )
	{
		cUOTxDenyLogin dLogin;
		dLogin.setReason( cUOTxDenyLogin::DL_BADCOMMUNICATION );
		send( &dLogin );
		return;
	}

	P_PLAYER pChar = charList[packet->index()];

	if ( pChar )
	{
		log(tr("Deleting character %1 (0x%2).\n").arg(pChar->orgName()).arg(pChar->serial(), 0, 16));
		pChar->remove();
	}

	updateCharList();
}

/*!
  This method handles cUORxPlayCharacter packet types.
  \sa cUORxPlayCharacter
*/
void cUOSocket::handlePlayCharacter( cUORxPlayCharacter* packet )
{
	// Check the character the user wants to play
	QValueVector<P_PLAYER> characters = _account->caracterList();

	if ( packet->slot() >= characters.size() )
	{
		cUOTxDenyLogin denyLogin;
		denyLogin.setReason( cUOTxDenyLogin::DL_BADCOMMUNICATION );
		send( &denyLogin );
		return;
	}

	if ( _account->inUse() )
	{
		cUOTxDenyLogin denyLogin;
		denyLogin.setReason( cUOTxDenyLogin::DL_INUSE );
		send( &denyLogin );
		return;
	}

	// Save the flags
	flags_ = packet->flags();

	// the char we want to login with
	P_PLAYER pChar = characters.at( packet->slot() );

	// check if any other account character is still online (lingering)
	for ( QValueVector<P_PLAYER>::const_iterator it = characters.begin(); it != characters.end(); ++it )
	{
		P_PLAYER otherChar = *it;
		if ( pChar == otherChar )
			continue;

		if ( otherChar->isOnline() )
		{
			cUOTxMessageWarning message;
			message.setReason( cUOTxMessageWarning::AlreadyInWorld );
			send( &message );
			return;
		}
	}

	log( LOG_MESSAGE, tr( "Selected character '%1' (0x%2).\n" ).arg( pChar->name() ).arg( pChar->serial(), 0, 16 ) );
	playChar( pChar );

	// if this char was lingering, cancel the auto-logoff and don't send the onLogin() event
	if ( pChar->logoutTime() )
	{
		pChar->onConnect( true );
		pChar->setLogoutTime( 0 );
	}
	else
	{
		pChar->onLogin();
		pChar->onConnect( false );
	}
}

// Set up the necessary stuff to play
void cUOSocket::playChar( P_PLAYER pChar )
{
	if ( !pChar )
		pChar = _player;

	// Minimum Requirements for log in
	// a) Set the map the user is on
	// b) Confirm the Login
	// c) Start the Game
	// d) Set the Game Time

	if ( !Maps::instance()->hasMap( pChar->pos().map ) )
	{
		Coord pos;
		pos.x = 0;
		pos.y = 0;
		pos.z = 0;
		pos.map = 0;
		pChar->moveTo( pos );
	}

	// We're now playing this char.
	setPlayer( pChar );
	pChar->account()->setInUse( true );

	// This needs to be sent once
	cUOTxConfirmLogin confirmLogin;
	confirmLogin.fromChar( pChar );
	confirmLogin.setUnknown3( 0x007f0000 );
	confirmLogin.setUnknown4( 0x00000007 );
	confirmLogin.setUnknown5( "\x60\x00\x00\x00\x00\x00\x00" );
	send( &confirmLogin );

	// Enable Sta+Map Diffs
	cUOTxMapDiffs diffs;
	diffs.addEntry( Maps::instance()->mapPatches( 0 ), Maps::instance()->staticPatches( 0 ) );
	if ( Maps::instance()->hasMap( 1 ) )
	{
		diffs.addEntry( Maps::instance()->mapPatches( 1 ), Maps::instance()->staticPatches( 1 ) );
		if ( Maps::instance()->hasMap( 2 ) )
		{
			diffs.addEntry( Maps::instance()->mapPatches( 2 ), Maps::instance()->staticPatches( 2 ) );
			if ( Maps::instance()->hasMap( 3 ) )
			{
				diffs.addEntry( Maps::instance()->mapPatches( 3 ), Maps::instance()->staticPatches( 3 ) );
			}
		}
	}
	send( &diffs );

	// Which map are we on
	cUOTxChangeMap changeMap;
	changeMap.setMap( pChar->pos().map );
	send( &changeMap );

	cUOTxChangeSeason season;
	if ( Config::instance()->enableFeluccaSeason() && _player->pos().map == 0 )
	{
		season.setSeason( ST_DESOLATION );
	}
	else if ( Config::instance()->enableTrammelSeason() && _player->pos().map == 1 )
	{
		season.setSeason( ST_SPRING );
	}
	else
	{
		season.setSeason( ST_SUMMER );
	}
	send( &season );

	updatePlayer();

	cUOTxChangeServer changeserver;
	changeserver.setX( pChar->pos().x );
	changeserver.setY( pChar->pos().y );
	changeserver.setZ( pChar->pos().z );
	changeserver.setWidth( Maps::instance()->mapTileWidth( pChar->pos().map ) * 8 );
	changeserver.setHeight( Maps::instance()->mapTileHeight( pChar->pos().map ) * 8 );
	send( &changeserver );

	cUOTxDrawChar drawchar;
	drawchar.fromChar( pChar );
	drawchar.setHighlight( pChar->notoriety( pChar ) );
	send( &drawchar );

	pChar->sendTooltip( this );

	// Send us our player and send the rest to us as well.
	pChar->resend( false );
	resendWorld( false );

	// Send the equipment Tooltips
	cBaseChar::ItemContainer content = _player->content();
	cBaseChar::ItemContainer::const_iterator it;

	for ( it = content.begin(); it != content.end(); it++ )
	{
		P_ITEM pItem = it.data();
		if ( pItem->layer() <= 0x19 )
		{
			pItem->update( this );
			pItem->sendTooltip( this );
		}
	}

	cUOTxWarmode warmode;
	warmode.setStatus( pChar->isAtWar() );
	send( &warmode );

	cUOTxOptions unknown;
	unknown.setOption( 1 );
	//	send( &unknown );

	unknown.setOption( 2 );
	//	send( &unknown );

	unknown.setOption( 3 );
	//	send( &unknown );

	// Reset combat information
	pChar->setAttackTarget( 0 );

	// This is required to display strength requirements correctly etc.
	sendStatWindow();

	// Reset the party
	cUOTxPartyRemoveMember updateparty;
	updateparty.setSerial( _player->serial() );
	send( &updateparty );

	// Start the game / Resend
	cUOTxStartGame startGame;
	send( &startGame );

	// Send the gametime
	cUOTxGameTime gameTime;
	gameTime.setTime( 0, 0, 0 );
	send( &gameTime );

	// Request a viewrange from the client
	cUOTxUpdateRange updateRange;
	updateRange.setRange( pChar->visualRange() );
	send( &updateRange );
}

/*!
	Checks the \a username and \a password pair. If accepted, initializes this socket
	to track that account, otherwise a proper cUOTxDenyLogin packet is sent. This method
	will create the account if Config::instance()->autoAccountCreate() evaluates to true
	\sa cUOTxDenyLogin
*/
bool cUOSocket::authenticate( const QString& username, const QString& password )
{
	cAccounts::enErrorCode error = cAccounts::NoError;
	cAccount* authRet = Accounts::instance()->authenticate( username, password, &error );

	// Reject login
	if ( !_account && error != cAccounts::NoError )
	{
		cUOTxDenyLogin denyPacket;

		switch ( error )
		{
			case cAccounts::LoginNotFound:
			{
				if ( Config::instance()->autoAccountCreate() )
				{
					authRet = Accounts::instance()->createAccount( username, password );
					_account = authRet;

					log( tr( "Automatically created account '%1'.\n" ).arg( username ) );
					return true;
				}
				else
				{
					denyPacket.setReason( cUOTxDenyLogin::DL_NOACCOUNT );
				}
				break;
			}
			case cAccounts::BadPassword:
			{
				log( tr( "Failed to log in as '%1', wrong password\n" ).arg( username ) );
				denyPacket.setReason( cUOTxDenyLogin::DL_BADPASSWORD );
				break;
			}
			case cAccounts::Wipped:
			case cAccounts::Banned:
				log( tr( "Failed to log in as '%1', Wipped/Banned account\n" ).arg( username ) );
				denyPacket.setReason( cUOTxDenyLogin::DL_BLOCKED );
				break;
			case cAccounts::AlreadyInUse:
				log( tr( "Failed to log in as '%1', account is already in use\n" ).arg( username ) );
				denyPacket.setReason( cUOTxDenyLogin::DL_INUSE );
				break;
			case cAccounts::NoError:
				break;
			default:
				break;
		};

		log( tr( "Failed to log in as '%1'.\n" ).arg( username ) );
		send( &denyPacket );
	}
	else if ( error == cAccounts::NoError )
	{
		log( tr( "Logged in as '%1'.\n" ).arg( username ) );
	}

	_account = authRet;

	return ( error == cAccounts::NoError );
}

/*!
  This method handles Character create request packet types.
  \sa cUORxCreateChar
*/
void cUOSocket::handleCreateChar( cUORxCreateChar* packet )
{
	// Processes a create character request
	// Notes from Lord Binaries packet documentation:
#define cancelCreate( message ) cUOTxDenyLogin denyLogin; denyLogin.setReason( cUOTxDenyLogin::DL_BADCOMMUNICATION ); send( &denyLogin ); sysMessage( message ); disconnect(); return;

	// Several security checks
	if ( !_account )
	{
		this->_socket->close();
		return;
	}

	QValueVector<P_PLAYER> characters = _account->caracterList();

	// If we have more than 6 characters
	const uint maxChars = wpMin<uint>( 6, Config::instance()->maxCharsPerAccount() );
	if ( characters.size() >= maxChars )
	{
		cancelCreate( tr( "You already have more than %1 characters" ).arg( maxChars ) )
	}

	// If another character in the account is still online (lingering)
	for ( QValueVector<P_PLAYER>::const_iterator it = characters.begin(); it != characters.end(); ++it )
	{
		P_PLAYER otherChar = *it;
		if ( otherChar->isOnline() )
		{
			cUOTxMessageWarning message;
			message.setReason( cUOTxMessageWarning::AlreadyInWorld );
			send( &message );
			return;
		}
	}

	// Check the stats
	Q_UINT16 statSum = ( packet->strength() + packet->dexterity() + packet->intelligence() );

	// Every stat needs to be below 60 && the sum lower/equal than 80
	if ( statSum > 80 || ( packet->strength() > 60 ) || ( packet->dexterity() > 60 ) || ( packet->intelligence() > 60 ) )
	{
		log( tr( "Submitted invalid stats during char creation (%1,%2,%3).\n" ).arg( packet->strength() ).arg( packet->dexterity() ).arg( packet->intelligence() ) );
		cancelCreate( tr( "Invalid Character stats" ) )
	}

	// Check the skills
	if ( ( packet->skillId1() >= ALLSKILLS ) || ( packet->skillValue1() > 50 ) || ( packet->skillId2() >= ALLSKILLS ) || ( packet->skillValue2() > 50 ) || ( packet->skillId3() >= ALLSKILLS ) || ( packet->skillValue3() > 50 ) || ( packet->skillValue1() + packet->skillValue2() + packet->skillValue3() > 100 ) )
	{
		log( tr( "Submitted invalid skills during char creation (%1=%2,%3=%4,%5=%6).\n" ).arg( packet->skillId1() ).arg( packet->skillValue1() ).arg( packet->skillId2() ).arg( packet->skillValue2() ).arg( packet->skillId3() ).arg( packet->skillValue3() ) );
		cancelCreate( tr( "Invalid Character skills" ) )
	}

	// Check Hair
	if ( packet->hairStyle() && ( !isHair( packet->hairStyle() ) || !isHairColor( packet->hairColor() ) ) )
	{
		log( tr( "Submitted wrong hair style (%1) or wrong hair color (%2) during char creation.\n" ).arg( packet->hairStyle() ).arg( packet->hairColor() ) );
		cancelCreate( tr( "Invalid hair" ) )
	}

	// Check Beard
	if ( packet->beardStyle() && ( !isBeard( packet->beardStyle() ) || !isHairColor( packet->beardColor() ) ) )
	{
		log( tr( "Submitted wrong beard style (%1) or wrong beard color (%2) during char creation.\n" ).arg( packet->beardStyle() ).arg( packet->beardColor() ) );
		cancelCreate( tr( "Invalid beard" ) )
	}

	// Check color for pants and shirt
	if ( !isNormalColor( packet->shirtColor() ) || !isNormalColor( packet->pantsColor() ) )
	{
		log( tr( "Submitted wrong shirt (%1) or pant (%2) color during char creation.\n" ).arg( packet->shirtColor() ).arg( packet->pantsColor() ) );
		cancelCreate( tr( "Invalid shirt or pant color" ) )
	}

	// Check the start location
	vector<StartLocation_st> startLocations = Config::instance()->startLocation();

	if ( packet->startTown() >= startLocations.size() )
	{
		log( tr( "Submitted wrong starting location (%1) during char creation.\n" ).arg( packet->startTown() ) );
		cancelCreate( tr( "Invalid start location" ) )
	}

	// Finally check the skin
	if ( !isSkinColor( packet->skinColor() ) )
	{
		log( tr( "Submitted a wrong skin color (%1) during char creation.\n" ).arg( packet->skinColor() ) );
		cancelCreate( tr( "Invalid skin color" ) )
	}

	// FINALLY create the char
	P_PLAYER pChar = new cPlayer;
	pChar->Init();
	pChar->setGender( packet->gender() );

	const cElement* playerDefinition = 0;
	if ( packet->gender() == 1 )
	{
		pChar->setBaseid( "player_female" );
		playerDefinition = Definitions::instance()->getDefinition( WPDT_NPC, "player_female" );
	}
	else
	{
		pChar->setBaseid( "player_male" );
		playerDefinition = Definitions::instance()->getDefinition( WPDT_NPC, "player_male" );
	}

	if ( playerDefinition )
		pChar->applyDefinition( playerDefinition );

	pChar->setName( packet->name() );
	pChar->setOrgName( packet->name() );

	pChar->setSkin( packet->skinColor() );
	pChar->setOrgSkin( packet->skinColor() );

	pChar->setBody( ( packet->gender() == 1 ) ? 0x191 : 0x190 );

	pChar->setOrgBody( pChar->body() );

	pChar->moveTo( startLocations[packet->startTown()].pos );
	pChar->setDirection( 4 );

	pChar->setStrength( packet->strength() );
	pChar->setHitpoints( pChar->strength() );
	pChar->setMaxHitpoints( pChar->strength() );

	pChar->setDexterity( packet->dexterity() );
	pChar->setStamina( pChar->dexterity() );
	pChar->setMaxStamina( pChar->dexterity() );

	pChar->setIntelligence( packet->intelligence() );
	pChar->setMana( pChar->intelligence() );
	pChar->setMaxMana( pChar->intelligence() );

	pChar->setSkillValue( packet->skillId1(), packet->skillValue1() * 10 );
	pChar->setSkillValue( packet->skillId2(), packet->skillValue2() * 10 );
	pChar->setSkillValue( packet->skillId3(), packet->skillValue3() * 10 );

	// Create the char equipment (shirt, paint, hair and beard only)
	P_ITEM pItem;

	// Shirt
	pItem = cItem::createFromScript( "1517" );
	pItem->setColor( packet->shirtColor() );
	pItem->setNewbie( true );
	pChar->addItem( cBaseChar::Shirt, pItem );

	// Skirt or Pants
	pItem = cItem::createFromScript( ( packet->gender() != 0 ) ? "1516" : "152e" );
	pItem->setColor( packet->pantsColor() );
	pItem->setNewbie( true );
	pChar->addItem( cBaseChar::Pants, pItem );

	// Hair & Beard
	if ( packet->hairStyle() )
	{
		pItem = cItem::createFromScript( QString( "%1" ).arg( packet->hairStyle(), 0, 16 ) );
		pItem->setNewbie( true );
		pItem->setColor( packet->hairColor() );
		pChar->addItem( cBaseChar::Hair, pItem );
	}

	if ( packet->beardStyle() )
	{
		pItem = cItem::createFromScript( QString( "%1" ).arg( packet->beardStyle(), 0, 16 ) );
		pItem->setNewbie( true );
		pItem->setColor( packet->beardColor() );
		pChar->addItem( cBaseChar::FacialHair, pItem );
	}

	// Automatically create Backpack + Bankbox
	pChar->getBankbox();
	pChar->getBackpack();

	pChar->setAccount( _account );

	uchar skillid = 0xFF;
	uchar skillid2 = 0xFF;

	if ( packet->skillValue1() > packet->skillValue2() )
	{
		if ( packet->skillValue1() > packet->skillValue3() )
		{
			skillid = packet->skillId1();
			if ( packet->skillValue2() > packet->skillValue3() )
				skillid2 = packet->skillId2();
			else
				skillid2 = packet->skillId3();
		}
		else
		{
			skillid = packet->skillId3();
			if ( packet->skillValue2() > packet->skillValue1() )
				skillid2 = packet->skillId2();
			else
				skillid2 = packet->skillId1();
		}
	}
	else
	{
		if ( packet->skillValue2() > packet->skillValue3() )
		{
			skillid = packet->skillId2();
			if ( packet->skillValue1() > packet->skillValue3() )
				skillid2 = packet->skillId1();
			else
				skillid2 = packet->skillId3();
		}
		else
		{
			skillid = packet->skillId3();
			if ( packet->skillValue1() > packet->skillValue2() )
				skillid2 = packet->skillId1();
			else
				skillid2 = packet->skillId2();
		}
	}

	if ( skillid != 0xFF && skillid2 != 0xFF ) // give default stuff too.
		pChar->giveNewbieItems( 0xFF );
	pChar->giveNewbieItems( skillid );
	pChar->giveNewbieItems( skillid2 );

	log( LOG_MESSAGE, tr( "Created character '%1' (0x%2).\n" ).arg( pChar->name() ).arg( pChar->serial(), 0, 16 ) );

	// Start the game with the newly created char -- OR RELAY HIM !!
	playChar( pChar );
	pChar->onCreate( pChar->baseid() ); // Call onCreate before onLogin
	pChar->onLogin();
	pChar->onConnect( false );
#undef cancelCreate
}

/*!
  This method sends a system \a message at the botton of the screen
  \sa cUOTxUnicodeSpeech
*/
void cUOSocket::sysMessage( const QString& message, Q_UINT16 color, Q_UINT16 font ) {
	if ( message.isEmpty() )
		return;
	// Color: 0x0037
	cUOTxUnicodeSpeech speech;
	speech.setSource( 0xFFFFFFFF );
	speech.setModel( 0xFFFF );
	speech.setFont( font );
	speech.setColor( color );
	speech.setType( cUOTxUnicodeSpeech::System );
	speech.setName( "System" );
	speech.setText( message );
	send( &speech );
}

void cUOSocket::updateCharList()
{
	cUOTxUpdateCharList charList;
	QValueVector<P_PLAYER> characters = _account->caracterList();

	// Add the characters
	for ( Q_UINT8 i = 0; i < characters.size(); ++i )
		charList.setCharacter( i, characters.at( i )->name() );

	send( &charList );
}

// Sends either a stat or a skill packet
/*!
  This method handles cUORxQuery packet types.
  \sa cUORxQuery
*/
void cUOSocket::handleQuery( cUORxQuery* packet )
{
	P_CHAR pChar = FindCharBySerial( packet->serial() );

	if ( !pChar )
		return;

	// Skills of other people can only be queried as a gm
	if ( packet->type() == cUORxQuery::Skills )
	{
		if ( ( pChar != _player ) && !_player->isGM() )
			return;

		if ( pChar->onShowSkillGump() )
			return;

		// Send a full skill update
		cUOTxSendSkills skillList;
		skillList.fromChar( pChar );
		send( &skillList );
	}
	else if ( packet->type() == cUORxQuery::Stats )
	{
		sendStatWindow( pChar );
	}
}

/*!
  This method handles cUORxUpdateRange packet types.
  \sa cUORxUpdateRange
*/
void cUOSocket::handleUpdateRange( cUORxUpdateRange* packet )
{
	if ( packet->range() > 18 || packet->range() < 5 )
		return; // Na..

	if ( _player )
	{
		_player->setVisualRange( packet->range() );
	}

	cUOPacket update( 0xc8, 2 );
	update[1] = _player->visualRange();
	send( &update );
}

/*!
  This method handles cUORxRequestLook packet types.
  \sa cUORxRequestLook
*/
void cUOSocket::handleRequestLook( cUORxRequestLook* packet )
{
	if ( !_player )
		return;

	// Check if it's a singleclick on items or chars
	if ( isCharSerial( packet->serial() ) )
	{
		P_CHAR pChar = FindCharBySerial( packet->serial() );

		if ( !pChar )
			return;

		pChar->showName( this );
	}
	else
	{
		P_ITEM pItem = FindItemBySerial( packet->serial() );

		if ( !pItem )
			return;

		pItem->showName( this );
	}
}

/*!
  This method handles cUORxMultiPorpuse packet types.
  \sa cUORxMultiPorpuse
*/
void cUOSocket::handleMultiPurpose( cUORxMultiPurpose* packet )
{
	switch ( packet->subCommand() )
	{
		// Screen Size
		case cUORxMultiPurpose::screenSize:
			if ( packet->size() >= 13 )
			{
				_screenWidth = packet->getShort( 7 );
				_screenHeight = packet->getShort( 9 );
			}
			return;

		case 0x0e:
			return;

			// Ignore this packet (Unknown Login Info)
		case cUORxMultiPurpose::unknownLoginInfo:
			return;

			// Ignore this packet (Status gump closed)
		case cUORxMultiPurpose::closedStatusGump:
			return;

		case cUORxMultiPurpose::setLanguage:
			handleSetLanguage( dynamic_cast<cUORxSetLanguage*>( packet ) );
			return;

		case cUORxMultiPurpose::contextMenuRequest:
			handleContextMenuRequest( dynamic_cast<cUORxContextMenuRequest*>( packet ) );
			return;

		case cUORxMultiPurpose::contextMenuSelection:
			handleContextMenuSelection( dynamic_cast<cUORxContextMenuSelection*>( packet ) );
			return;

		case cUORxMultiPurpose::castSpell:
			handleCastSpell( dynamic_cast<cUORxCastSpell*>( packet ) );
			return;

		case cUORxMultiPurpose::toolTip:
			handleToolTip( dynamic_cast<cUORxRequestToolTip*>( packet ) );
			return;

		case cUORxMultiPurpose::customHouseRequest:
			handleCustomHouseRequest( dynamic_cast<cUORxCustomHouseRequest*>( packet ) );
			return;

		case cUORxMultiPurpose::extendedStats:
			handleExtendedStats( dynamic_cast<cUORxExtendedStats*>( packet ) );
			return;

		case cUORxMultiPurpose::partySystem:
			handleParty( packet );
			return;

			// Unknown Packet
		case 0x24:
			return;

		default:
			break;
	};

	QString message;
	message.sprintf( "Receieved unknown multi purpose subcommand: 0x%02x", packet->subCommand() );
	message += packet->dump( packet->uncompressed() ) + "\n";
	log( LOG_WARNING, message );
}

/*!
  This method handles cUORxAosMultiPurpose packet types.
  \sa cUORxAosMultiPurpose
*/
void cUOSocket::handleAosMultiPurpose( cUORxAosMultiPurpose* packet )
{
	if ( !packet ) // Happens if it's not inherited from cUORxAosMultiPurpose
		return;

	switch ( packet->subCommand() )
	{
		case cUORxAosMultiPurpose::CHBackup:
			handleCHBackup( packet );
			break;
		case cUORxAosMultiPurpose::CHRestore:
			handleCHRestore( packet );
			break;
		case cUORxAosMultiPurpose::CHCommit:
			handleCHCommit( packet );
			break;
		case cUORxAosMultiPurpose::CHDelete:
			handleCHDelete( dynamic_cast<cUORxCHDelete*>( packet ) );
			break;
		case cUORxAosMultiPurpose::CHAddElement:
			handleCHAddElement( dynamic_cast<cUORxCHAddElement*>( packet ) );
			break;
		case cUORxAosMultiPurpose::CHClose:
			handleCHClose( packet );
			break;
		case cUORxAosMultiPurpose::CHStairs:
			handleCHStairs( dynamic_cast<cUORxCHStairs*>( packet ) );
			break;
		case cUORxAosMultiPurpose::CHSync:
			handleCHSync( packet );
			break;
		case cUORxAosMultiPurpose::CHClear:
			handleCHClear( packet );
			break;
		case cUORxAosMultiPurpose::CHLevel:
			handleCHLevel( dynamic_cast<cUORxCHLevel*>( packet ) );
			break;
		case cUORxAosMultiPurpose::CHRevert:
			handleCHRevert( packet );
			break;
		case cUORxAosMultiPurpose::GuildButton:
			{
				PyObject *args = Py_BuildValue( "(N)", _player->getPyObject() );
				_player->callEventHandler( EVENT_GUILDBUTTON, args );
				Py_DECREF( args );
			}
			break;
		case cUORxAosMultiPurpose::AbilitySelect:
			{
				PyObject *args = Py_BuildValue( "(Ni)", _player->getPyObject(), packet->getInt(10) );
				_player->callEventHandler( EVENT_SELECTABILITY, args );
				Py_DECREF( args );
			}
			break;
		default:
			Console::instance()->log( LOG_WARNING, packet->dump( packet->uncompressed() ) );
			break;
	};
}
//#pragma message(Reminder "Implement Custom House subcommands here")

void cUOSocket::handleCHBackup( cUORxAosMultiPurpose* packet )
{
	Q_UNUSED( packet );
}

void cUOSocket::handleCHRestore( cUORxAosMultiPurpose* packet )
{
	Q_UNUSED( packet );
}

void cUOSocket::handleCHCommit( cUORxAosMultiPurpose* packet )
{
	Q_UNUSED( packet );
}

void cUOSocket::handleCHDelete( cUORxCHDelete* packet )
{
	Q_UNUSED( packet );
}

void cUOSocket::handleCHAddElement( cUORxCHAddElement* packet )
{
	Q_UNUSED( packet );
}

void cUOSocket::handleCHClose( cUORxAosMultiPurpose* packet )
{
	Q_UNUSED( packet );
}

void cUOSocket::handleCHStairs( cUORxCHStairs* packet )
{
	Q_UNUSED( packet );
}

void cUOSocket::handleCHSync( cUORxAosMultiPurpose* packet )
{
	Q_UNUSED( packet );
}

void cUOSocket::handleCHClear( cUORxAosMultiPurpose* packet )
{
	Q_UNUSED( packet );
}

void cUOSocket::handleCHLevel( cUORxCHLevel* packet )
{
	player()->onCHLevelChange( packet->level() );
}

void cUOSocket::handleCHRevert( cUORxAosMultiPurpose* packet )
{
	Q_UNUSED( packet );
}

void cUOSocket::handleCastSpell( cUORxCastSpell* packet )
{
	if ( !_player )
		return;

	_player->onCastSpell( packet->spell() );
}

void cUOSocket::handleContextMenuSelection( cUORxContextMenuSelection* packet )
{
	if (_player->isJailed()) {
		return; // Don't show context menus to jailed players
	}

	P_CHAR pChar;
	P_ITEM pItem;

	if ( contextMenu_.isEmpty() )
		return;

	Q_UINT16 Tag = packet->entryTag();
	cContextMenu* menu = 0;
	bool found = false;
	for ( menu = contextMenu_.first(); menu; menu = contextMenu_.next() )
	{
		if ( Tag >= menu->count() )
			Tag -= menu->count();
		else
		{
			found = true;
			break;
		}
	}

	if ( !found )
		return;

	pItem = FindItemBySerial( packet->serial() );
	if ( pItem )
	{
		menu->onContextEntry( this->player(), pItem, Tag );
	}
	else
	{
		pChar = FindCharBySerial( packet->serial() );
		if ( !pChar )
			return;
		menu->onContextEntry( this->player(), pChar, Tag );
	}
}
void cUOSocket::handleCustomHouseRequest( cUORxCustomHouseRequest* packet )
{
	Q_UNUSED( packet );
	/*	SERIAL serial = packet->serial();
		cMulti* pMulti = dynamic_cast< cMulti* >( FindItemBySerial( serial ) );
		pMulti->sendCH( this );*/
	return;
}

void cUOSocket::handleToolTip( cUORxRequestToolTip* packet )
{
	cUOTxTooltipList tooltip;
	cUObject* object = World::instance()->findObject( packet->serial() );

	if ( object && player() )
	{
		object->createTooltip( tooltip, player() );
		send( &tooltip );
	}
}

// Show a context menu
/*!
  This method handles cUORxContextMenuRequest packet types.
  \sa cUORxContextMenuRequest
*/
void cUOSocket::handleContextMenuRequest( cUORxContextMenuRequest* packet )
{
	if (_player->isJailed()) {
		return; // Don't show context menus to jailed players
	}

	cUObject* clicked = FindItemBySerial( packet->serial() );
	if ( clicked == 0 )
		clicked = FindCharBySerial( packet->serial() );

	if ( !clicked )
		return;

	if ( clicked->bindmenu().isEmpty() )
		return;

	/*	if( !ContextMenus::instance()->menuExists( clicked->bindmenu() ) )
		{
			clicked->setBindmenu(QString::null);
			return;
		}
	*/
	cUOTxContextMenu menuPacket;
	menuPacket.setSerial( packet->serial() );

	QStringList bindMenus = QStringList::split( ",", clicked->bindmenu() );
	QStringList::const_iterator menuIt = bindMenus.begin();

	contextMenu_.clear();
	unsigned int i = 0;
	unsigned int totalCount = 0;
	for ( ; menuIt != bindMenus.end(); ++menuIt )
	{
		cContextMenu* menu = ContextMenus::instance()->getMenu( *menuIt );

		if ( !menu )
			continue;

		contextMenu_.append( menu );
		uint entryCount = 0;
		cContextMenu::const_iterator it = menu->begin();
		for ( ; it != menu->end(); ++it, ++entryCount, ++i )
		{
			//if ( (*it)->checkVisible() )
			if ( !menu->onCheckVisible( this->player(), clicked, entryCount ) )
			{
				continue;
			}
			bool enabled = true;
			if ( ( *it )->checkEnabled() )
				if ( !menu->onCheckEnabled( this->player(), clicked, entryCount ) )
					enabled = false;
			menuPacket.addEntry( i, ( *it )->cliloc(), enabled ? ( *it )->flags() : ( *it )->flags() | 0x0001, ( *it )->color() );
			totalCount++;
		}
	}
	if ( i && totalCount ) // Won't send empty menus
		send( &menuPacket );
}

/*!
  This method prints \a message on top of \a object using the given \a color and \a speechType
  \sa cUObject, cUOTxUnicodeSpeech, cUOTxUnicodeSpeech::eSpeechType
*/
void cUOSocket::showSpeech( const cUObject* object, const QString& message, Q_UINT16 color, Q_UINT16 font, Q_UINT8 speechType )
{
	cUOTxUnicodeSpeech speech;
	speech.setSource( object->serial() );
	speech.setName( object->name() );
	speech.setFont( font );
	speech.setColor( color );
	speech.setText( message );
	speech.setType( ( cUOTxUnicodeSpeech::eSpeechType ) speechType );
	send( &speech );
}

/*!
  This method sends an moviment acknowleadge allowing the client to move.
  \sa cUOTxAcceptMove.
*/
void cUOSocket::allowMove( Q_UINT8 sequence )
{
	cUOTxAcceptMove acceptMove;
	acceptMove.setSequence( sequence );
	acceptMove.setHighlight( _player->notoriety( _player ) );
	send( &acceptMove );

	if ( sequence == 255 )
	{
		_walkSequence = 1;
	}
	else
	{
		_walkSequence = sequence + 1;
	}
}

/*!
  This method informs the client that the requested movement is not permited.
  \sa cUOTxDenyMove.
*/
void cUOSocket::denyMove( Q_UINT8 sequence )
{
	cUOTxDenyMove deny;
	deny.fromChar( _player );
	deny.setSequence( sequence );
	send( &deny );
	_walkSequence = 0;
}

/*!
  This method handles cUORxWalkRequest packet types.
  \sa cUORxWalkRequest
*/
void cUOSocket::handleWalkRequest( cUORxWalkRequest* packet )
{
	Movement::instance()->Walking( _player, packet->direction(), packet->key() );
}

void cUOSocket::resendPlayer( bool quick )
{
	Q_UNUSED( quick );
	P_CHAR pChar = _player;

	// Which map are we on
	cUOTxChangeMap changeMap;
	changeMap.setMap( pChar->pos().map );
	send( &changeMap );

	cUOTxChangeSeason season;
	if ( Config::instance()->enableFeluccaSeason() && _player->pos().map == 0 )
	{
		season.setSeason( ST_DESOLATION );
	}
	else if ( Config::instance()->enableTrammelSeason() && _player->pos().map == 1 )
	{
		season.setSeason( ST_SPRING );
	}
	else
	{
		season.setSeason( ST_SUMMER );
	}
	send( &season );

	updatePlayer();

	cUOTxChangeServer changeserver;
	changeserver.setX( pChar->pos().x );
	changeserver.setY( pChar->pos().y );
	changeserver.setZ( pChar->pos().z );
	changeserver.setWidth( Maps::instance()->mapTileWidth( pChar->pos().map ) * 8 );
	changeserver.setHeight( Maps::instance()->mapTileHeight( pChar->pos().map ) * 8 );
	send( &changeserver );

	cUOTxDrawChar drawchar;
	drawchar.fromChar( pChar );
	drawchar.setHighlight( pChar->notoriety( pChar ) );
	send( &drawchar );

	// Send us our player and send the rest to us as well.
	pChar->moveTo( pChar->pos() );
	pChar->resend( false );
	resendWorld( false );
	pChar->resendTooltip();

	cUOTxWarmode warmode;
	warmode.setStatus( pChar->isAtWar() );
	send( &warmode );

	/*	if (!quick) {
			cUOTxChangeMap changemap;
			changemap.setMap(pos_.map);
			socket_->send(&changemap);
			cUOTxChangeServer changeserver;
			changeserver.setX(pos_.x);
			changeserver.setY(pos_.y);
			changeserver.setZ(pos_.z);
			changeserver.setWidth(Maps::instance()->mapTileWidth(pos_.map) * 8);
			changeserver.setHeight(Maps::instance()->mapTileHeight(pos_.map) * 8);
			socket_->send(&changeserver);
		}
		updatePlayer();
		// Send the equipment Tooltips
		cBaseChar::ItemContainer content = _player->content();
		cBaseChar::ItemContainer::const_iterator it;
		for (it = content.begin(); it != content.end(); it++) {
			P_ITEM pItem = it.data();
			if (pItem->layer() <= 0x19) {
				pItem->update(this);
				pItem->sendTooltip(this);
			}
		}
		updateLightLevel();
		// Set the warmode status
		if (!quick) {
			cUOTxWarmode warmode;
			warmode.setStatus(_player->isAtWar());
			send(&warmode);
			resendWorld(false);
		}*/
}

void cUOSocket::updateChar( P_CHAR pChar )
{
	if ( canSee( pChar ) )
	{
		cUOTxUpdatePlayer updatePlayer;
		updatePlayer.fromChar( pChar );
		updatePlayer.setHighlight( pChar->notoriety( _player ) );
		send( &updatePlayer );
	}
}

// Sends a foreign char including equipment
void cUOSocket::sendChar( P_CHAR pChar )
{
	if ( pChar == _player )
	{
		updatePlayer();
		return;
	}

	if ( canSee( pChar ) )
	{
		// Then completely resend it
		cUOTxDrawChar drawChar;
		drawChar.fromChar( pChar );
		drawChar.setHighlight( pChar->notoriety( _player ) );
		send( &drawChar );
		pChar->sendTooltip( this );

		// Send item tooltips
		cBaseChar::ItemContainer content = pChar->content();
		for ( cBaseChar::ItemContainer::const_iterator it = content.begin(); it != content.end(); ++it )
		{
			P_ITEM item = it.data();
			if ( item->layer() <= 0x19 )
			{
				item->sendTooltip( this );
			}
		}
	}
}

/*!
  This method handles cUORxSetLanguage packet types.
  \sa cUORxSetLanguage
*/
void cUOSocket::handleSetLanguage( cUORxSetLanguage* packet )
{
	_lang = packet->language();
}

void cUOSocket::setPlayer( P_PLAYER pChar )
{
	// If we're already playing a char and changing reset the socket status of that player
	if ( !pChar && !_player )
		return;

	// If the player is changing
	if ( pChar && ( pChar != _player ) )
	{
		if ( _player )
		{
			// This should never happen, we should deny logins while there's a lingering
			// char; but just in case, as this may avoid problems:
			_player->onLogout();
			_player->removeFromView( true );
			_player->setSocket( 0 );
			_player->setLogoutTime( 0 );
			_player->resend( false );
		}

		_player = pChar;
		_player->setSocket( this );
		_player->resend( false );
	}

	_state = InGame;
}

/*!
  This method handles cUORxSpeechRequest packet types.
  \sa cUORxSpeechRequest
*/
void cUOSocket::handleSpeechRequest( cUORxSpeechRequest* packet )
{
	if ( !_player )
		return;

	// Check if it's a command, then dispatch it to the command system
	// if it's normal speech send it to the normal speech dispatcher
	QString speech = packet->message();
	QValueVector<Q_UINT16> keywords;
	if ( packet->type() & 0xc0 )
		keywords = packet->keywords();
	Q_UINT16 color = packet->color();
	Q_UINT16 font = packet->font();
	Q_UINT16 type = packet->type() & 0x3f; // Pad out the Tokenized speech flag

	// There is one special case. if the user has the body 0x3db and the first char
	// of the speech is = then it's always a command
	if ( ( _player->body() == 0x3DB ) && speech.startsWith( "=" ) )
		Commands::instance()->process( this, speech.right( speech.length() - 1 ) );
	else if ( speech.startsWith( Config::instance()->commandPrefix() ) )
		Commands::instance()->process( this, speech.right( speech.length() - 1 ) );
	else
		Speech::talking( _player, packet->language(), speech, keywords, color, font, type );
}

/*!
  This method handles cUORxDoubleClick packet types.
  \sa cUORxDoubleClick
*/
void cUOSocket::handleDoubleClick( cUORxDoubleClick* packet ) {
	P_CHAR pChar = World::instance()->findChar( packet->serial() );

	if ( pChar ) {
		pChar->showPaperdoll( this, packet->keyboard() );
	} else {
		P_ITEM item = World::instance()->findItem( packet->serial() );
		if ( item ) {
			useItem( item );
		}
	}
}

/*!
  This method handles cUORxGetTip packet types.
  \sa cUORxGetTip
*/
void cUOSocket::handleGetTip( cUORxGetTip* packet )
{
	if ( packet->isTip() )
	{
		UI32 tip = packet->lastTip();

		if ( tip == 0 )
			tip = 1;

		QStringList tipList = Definitions::instance()->getList( "TIPS" );
		if ( tipList.size() == 0 )
			return;
		else if ( tip > tipList.size() )
			tip = tipList.size();

		QString tipText = Definitions::instance()->getText( tipList[tip - 1] );
		cUOTxTipWindow packet;
		packet.setType( cUOTxTipWindow::Tip );
		packet.setNumber( tip );
		packet.setMessage( tipText.latin1() );
		send( &packet );
	}
}

void cUOSocket::sendPaperdoll( P_CHAR pChar )
{
	cUOTxOpenPaperdoll oPaperdoll;
	oPaperdoll.fromChar( pChar, _player );
	send( &oPaperdoll );
}

/*!
  This method handles cUORxChangeWarmode packet types.
  \sa cUORxChangeWarmode
*/
void cUOSocket::handleChangeWarmode( cUORxChangeWarmode* packet )
{
	// Warmode didn't change
	if ( packet->warmode() == _player->isAtWar() )
	{
		return;
	}

	_player->setAtWar( packet->warmode() );
	_player->onWarModeToggle( packet->warmode() );

	cUOTxWarmode warmode;
	warmode.setStatus( _player->isAtWar() ? 1 : 0 );
	send( &warmode );

	playMusic();
	_player->disturbMed();

	// Something changed
	if ( _player->isDead() && _player->isAtWar() )
		_player->resend( false );
	else if ( _player->isDead() && !_player->isAtWar() )
	{
		_player->removeFromView( false );
		_player->resend( false );
	}
	else
		_player->update( true );

	// Always stop fighting. If we changed warmode,
	// we cannot fight anyone
	_player->fight( 0 );
}

void cUOSocket::playMusic()
{
	if ( !_player )
		return;

	cTerritory* Region = _player->region();
	Q_UINT32 midi = 0;

	if ( _player->isAtWar() )
		midi = Definitions::instance()->getRandomListEntry( "MIDI_COMBAT" ).toInt();
	else if ( Region )
		midi = Definitions::instance()->getRandomListEntry( Region->midilist() ).toInt();

	if ( midi )
	{
		cUOTxPlayMusic pMusic;
		pMusic.setId( midi );
		send( &pMusic );
	}
}

void cUOSocket::sendContainer( P_ITEM pCont )
{
	if ( !pCont || !canSee( pCont ) )
		return;

	// Get the container gump
	Q_UINT16 gump = 0x3D;

	switch ( pCont->id() )
	{
		case 0x0E75: // Backpack
		case 0x0E79: // Box/Pouch
			gump = 0x3C;
			break;

		case 0x0E76: // Leather Bag
			gump = 0x3D;
			break;

		case 0x0E77: // Barrel
		case 0x0E7F: // Keg
		case 0xFAE:
			gump = 0x3E;
			break;

		case 0x0E7A: // Picknick Basket
			gump = 0x3F;
			break;

		case 0x0E7C: // Silver Chest
			gump = 0x4A;
			break;

		case 0x0E7D: // Wooden Box
			gump = 0x43;
			break;

		case 0x0E3D: // Large Wooden Crate
		case 0x0E3C: // Large Wooden Crate
		case 0x0E3F: // Small Wooden Crate
		case 0x0E3E: // Small Wooden Crate
		case 0x0E7E: // Wooden Crate
			gump = 0x44;
			break;

		case 0x0E80: // Brass Box
			gump = 0x4B;
			break;

		case 0x0E40: // Metal & Gold Chest
		case 0x0E41: // Metal & Gold Chest
			gump = 0x42;
			break;

		case 0x0E43: // Wooden & Gold chest
		case 0x0E42: // Wooden & Gold Chest
			gump = 0x49;
			break;

		case 0x0990: // Round Basket
		case 0x9AC:
		case 0x9B1:
			gump = 0x41;
			break;

		case 0x09B2: // Backpack 2
			gump = 0x3C;
			break;

		case 0x09AA: // Wooden Box
			gump = 0x43;
			break;

		case 0x09A8: // Metal Box
			gump = 0x40;
			break;

		case 0x09AB: // Metal/Silver Chest
			gump = 0x4A;
			break;

		case 0x09A9: // Small Wooden Crate
			gump = 0x44;
			break;

		case 0x2006: // Coffin
			gump = 0x09;
			break;

		case 0x0A97: // Bookcase
		case 0x0A98: // Bookcase
		case 0x0A99: // Bookcase
		case 0x0A9a: // Bookcase
		case 0x0A9b: // Bookcase
		case 0x0A9c: // Bookcase
		case 0x0A9d: // Bookcase
		case 0x0A9e: // Bookcase
			gump = 0x4d;
			break;

		case 0x0A4d: // Fancy Armoire
		case 0x0A51: // Fancy Armoire
		case 0x0A4c: // Fancy Armoire
		case 0x0A50: // Fancy Armoire
			gump = 0x4e;
			break;

		case 0x0A4f: // Wooden Armoire
		case 0x0A53: // Wooden Armoire
		case 0x0A4e: // Wooden Armoire
		case 0x0A52: // Wooden Armoire
			gump = 0x4f;
			break;

		case 0x0A30: // chest of drawers (fancy)
		case 0x0A38: // chest of drawers (fancy)
			gump = 0x48;
			break;

		case 0x0A2c: // chest of drawers (wood)
		case 0x0A34: // chest of drawers (wood)
		case 0x0A3c: // Dresser
		case 0x0A3d: // Dresser
		case 0x0A44: // Dresser
		case 0x0A35: // Dresser
			gump = 0x51;
			break;

		default:
			if ( ( ( pCont->id() & 0xFF00 ) >> 8 ) == 0x3E )
				gump = 0x4C;
			break;
	}

	// If its one of the "invisible" layers send an equip item packet first
	if ( pCont->layer() == 0x1D )
	{
		cUOTxCharEquipment equipment;
		equipment.fromItem( pCont );
		send( &equipment );
	}

	// If it's the bankbox, there's a fixed box for it
	if ( pCont->layer() == cBaseChar::BankBox )
	{
		gump = 0x4A;
	}

	// Draw the container
	cUOTxDrawContainer dContainer;
	dContainer.setSerial( pCont->serial() );
	dContainer.setGump( gump );
	send( &dContainer );

	// Add all items to the container
	cUOTxItemContent itemContent;
	Q_INT32 count = 0;

	QPtrList<cItem> tooltipItems;

	for ( ContainerIterator it( pCont ); !it.atEnd(); ++it )
	{
		P_ITEM pItem = *it;

		if ( !pItem || !canSee( pItem ) )
			continue;

		itemContent.addItem( pItem );
		tooltipItems.append( pItem );
		++count;
	}

	cCorpse* pCorpse = dynamic_cast<cCorpse*>( pCont );

	if ( pCorpse )
	{
		if ( pCorpse->hairStyle() )
		{
			itemContent.addItem( 0x4FFFFFFE, pCorpse->hairStyle(), pCorpse->hairColor(), 0, 0, 1, pCorpse->serial() );
			++count;
		}

		if ( pCorpse->beardStyle() )
		{
			itemContent.addItem( 0x4FFFFFFF, pCorpse->beardStyle(), pCorpse->beardColor(), 0, 0, 1, pCorpse->serial() );
			++count;
		}
	}

	// Only send if there is content
	if ( count )
	{
		send( &itemContent );

		for ( P_ITEM pItem = tooltipItems.first(); pItem; pItem = tooltipItems.next() )
		{
			pItem->sendTooltip( this );
		}
	}
}

void cUOSocket::removeObject( cUObject* object )
{
	cUOTxRemoveObject rObject;
	rObject.setSerial( object->serial() );
	send( &rObject );
}

// if flags etc. have changed
void cUOSocket::updatePlayer()
{
	if ( _player )
	{
		// Reset the walking sequence
		_walkSequence = 0;

		cUOTxDrawPlayer playerupdate;
		playerupdate.fromChar( _player );
		send( &playerupdate );

		updateLightLevel();
	}
}

// Do periodic stuff for this socket
void cUOSocket::poll()
{
	// Check for timed out target requests
	if ( targetRequest && targetRequest->timeout() > 1 && targetRequest->timeout() < Server::instance()->time() )
	{
		targetRequest->timedout( this );
		delete targetRequest;
		targetRequest = 0;
		cancelTarget();
	}

	// Check for idling/silent sockets
	if ( _lastActivity + 180 * MY_CLOCKS_PER_SEC < Server::instance()->time() )
	{
		log( tr( "Idle for %1 ms. Disconnecting.\n" ).arg( Server::instance()->time() - _lastActivity ) );
		disconnect();
	}
}

void cUOSocket::attachTarget( cTargetRequest* request, std::vector<stTargetItem>& items, Q_INT16 xOffset, Q_INT16 yOffset, Q_INT16 zOffset )
{
	// Let the old one time out
	if ( targetRequest && targetRequest != request )
	{
		targetRequest->canceled( this );
		delete targetRequest;
	}

	// attach the new one
	targetRequest = request;

	cUOTxItemTarget target;
	target.setTargSerial( 1 );
	target.setAllowGround( true ); // Not sure how to handle this
	target.setXOffset( xOffset );
	target.setYOffset( yOffset );
	target.setZOffset( zOffset );

	for ( unsigned int i = 0; i < items.size(); ++i )
		target.addItem( items[i].id, items[i].xOffset, items[i].yOffset, items[i].zOffset, items[i].hue );

	send( &target );
}

void cUOSocket::attachTarget( cTargetRequest* request )
{
	// Let the old one time out
	if ( targetRequest && targetRequest != request )
	{
		targetRequest->canceled( this );
		delete targetRequest;
	}

	// attach the new one
	targetRequest = request;

	cUOTxTarget target;
	target.setTargSerial( 1 );
	target.setAllowGround( true ); // Not sure how to handle this
	send( &target );
}

void cUOSocket::attachTarget( cTargetRequest* request, Q_UINT16 multiid, unsigned short xoffset, unsigned short yoffset, unsigned short zoffset )
{
	if ( multiid < 0x4000 )
		return;

	if ( targetRequest )
	{
		targetRequest->canceled( this );
		delete targetRequest;
	}

	targetRequest = request;

	cUOTxPlace target;
	target.setXOffset( xoffset );
	target.setYOffset( yoffset );
	target.setZOffset( zoffset );
	target.setTargSerial( 1 );
	target.setModelID( multiid - 0x4000 );
	send( &target );
}

/*!
  This method handles cUORxTarget packet types.
  \sa cUORxTarget
*/
void cUOSocket::handleTarget( cUORxTarget* packet )
{
	if ( !_player )
		return;

	if ( !targetRequest )
		return;

	// Check if there really was a response or if it just was canceled
	if ( !packet->serial() && ( ( packet->x() == 0xFFFF ) || ( packet->y() == 0xFFFF ) ) )
		targetRequest->canceled( this );
	else
	{
		// Save the target in a temporary variable
		cTargetRequest* request = targetRequest;
		targetRequest = 0;

		if ( request->responsed( this, packet ) )
		{
			delete request;
		}
		else
			attachTarget( request ); // Resend target.
	}
}

/*!
  This method handles cUORxRequestAttack packet types.
  \sa cUORxRequestAttack
*/
void cUOSocket::handleRequestAttack( cUORxRequestAttack* packet )
{
	if (_player->isJailed()) {
		sysMessage(tr("You cannot fight while you are in jail."));
	} else {
		_player->fight( World::instance()->findChar( packet->serial() ) );
	}
}

void cUOSocket::soundEffect( Q_UINT16 soundId, cUObject* source )
{
	if ( !_player )
		return;

	cUOTxSoundEffect sound;
	sound.setSound( soundId );

	if ( !source )
		sound.setCoord( _player->pos() );
	else
		sound.setCoord( source->pos() );

	send( &sound );
}

void cUOSocket::resendWorld( bool clean )
{
	if ( !_player )
		return;

	// resend items
	MapItemsIterator itemIt = MapObjects::instance()->listItemsInCircle( _player->pos(), _player->visualRange() );
	for ( P_ITEM item = itemIt.first(); item; item = itemIt.next() )
	{
		if ( clean )
			removeObject( item );

		item->update( this );
	}

	// resend multis
	MapMultisIterator multiIt = MapObjects::instance()->listMultisInCircle( _player->pos(), BUILDRANGE );
	for ( P_MULTI multi = multiIt.first(); multi; multi = multiIt.next() )
	{
		if ( clean )
			removeObject( multi );

		multi->update( this );
	}

	// send online characters
	MapCharsIterator charIt = MapObjects::instance()->listCharsInCircle( _player->pos(), _player->visualRange() );
	for ( P_CHAR character = charIt.first(); character; character = charIt.next() )
	{
		if ( character == _player )
			continue;

		if ( clean )
			removeObject( character );

		sendChar( character );
	}
}

void cUOSocket::resync()
{
	resendPlayer( false );
}

P_ITEM cUOSocket::dragging() const
{
	if ( !_player )
		return 0;

	return _player->atLayer( cBaseChar::Dragging );
}

void cUOSocket::bounceItem( P_ITEM pItem, Q_UINT8 reason )
{
	cUOTxBounceItem bounce;
	bounce.setReason( ( eBounceReason ) reason );
	send( &bounce );

	// Only bounce it back if it's on the hand of the char
	if ( dragging() == pItem )
	{
		P_PLAYER player = this->player();
		if ( pItem == player->atLayer( cBaseChar::Dragging ) )
		{
			player->removeItem( cBaseChar::Dragging );
		}

		pItem->toBackpack( player );

		if ( pItem->isInWorld() )
			pItem->soundEffect( 0x42 );
		else
			soundEffect( 0x57 );
	}
}

void cUOSocket::updateStamina( P_CHAR pChar )
{
	if ( !pChar )
		pChar = _player;

	if ( !pChar )
		return;

	cUOTxUpdateStamina update;
	update.setSerial( pChar->serial() );

	if ( pChar == _player )
	{
		update.setMaximum( pChar->maxStamina() );
		update.setCurrent( pChar->stamina() );
	}
	else
	{
		update.setMaximum( 100 );
		update.setCurrent( ( Q_UINT16 ) ( ( pChar->stamina() / pChar->dexterity() ) * 100 ) );
	}

	send( &update );

	// Send the packet to our party members too
	if ( pChar == _player && _player->party() )
	{
		_player->party()->send( &update );
	}
}

void cUOSocket::updateMana( P_CHAR pChar )
{
	if ( !pChar )
		pChar = _player;

	if ( !pChar )
		return;

	cUOTxUpdateMana update;
	update.setSerial( pChar->serial() );

	if ( pChar == _player )
	{
		update.setMaximum( pChar->maxMana() );
		update.setCurrent( pChar->mana() );
	}
	else
	{
		update.setMaximum( 100 );
		update.setCurrent( ( Q_UINT16 ) ( ( pChar->mana() / pChar->intelligence() ) * 100 ) );
	}

	send( &update );

	// Send the packet to our party members too
	if ( pChar == _player && _player->party() )
	{
		_player->party()->send( &update );
	}
}

void cUOSocket::updateHealth( P_CHAR pChar )
{
	if ( !pChar )
		pChar = _player;

	if ( !pChar )
		return;

	cUOTxUpdateHealth update;
	update.setSerial( pChar->serial() );

	if ( pChar != _player )
	{
		float factor = pChar->hitpoints() / ( float ) pChar->maxHitpoints();
		unsigned short max = 256;
		unsigned short cur = ( unsigned short ) ( factor * max );
		update.setMaximum( max );
		update.setCurrent( cur );
	}
	else
	{
		update.setMaximum( pChar->maxHitpoints() );
		update.setCurrent( pChar->hitpoints() );
	}

	send( &update );

	// Send the packet to our party members too
	if ( pChar == _player && _player->party() )
	{
		_player->party()->send( &update );
	}
}

void cUOSocket::sendStatWindow( P_CHAR pChar )
{
	if ( !pChar )
		pChar = _player;

	if ( !pChar )
		return;

	// For other chars we only send the basic stats
	cUOTxSendStats sendStats;

	// TODO: extended packet information
	sendStats.setFullMode( pChar == _player, true );

	// Dont allow rename-self
	sendStats.setAllowRename( ( ( pChar->objectType() == enNPC && dynamic_cast<P_NPC>( pChar )->owner() == _player && !pChar->isHuman() ) || _player->isGM() ) && ( _player != pChar ) );

	sendStats.setName( makeAscii( pChar->name() ) );
	sendStats.setSerial( pChar->serial() );

	if ( pChar != _player )
	{
		float factor = pChar->hitpoints() / ( float ) pChar->maxHitpoints();
		unsigned short max = 256;
		unsigned short cur = ( unsigned short ) ( factor * max );
		sendStats.setMaxHp( max );
		sendStats.setHp( cur );
	}
	else
	{
		sendStats.setMaxHp( pChar->maxHitpoints() );
		sendStats.setHp( pChar->hitpoints() );
		sendStats.setStamina( _player->stamina() );
		sendStats.setMaxStamina( _player->maxStamina() );
		sendStats.setMana( _player->mana() );
		sendStats.setMaxMana( _player->maxMana() );
		sendStats.setStrength( _player->strength() );
		sendStats.setDexterity( _player->dexterity() );
		sendStats.setIntelligence( _player->intelligence() );
		sendStats.setWeight( ( short unsigned int ) _player->weight() );

		// use a different method of counting gold here. (not recursive)
		unsigned int gold = 0;
		ContainerIterator it(_player->getBackpack());
		while (!it.atEnd()) {
			if ((*it)->id() == 0xeed && (*it)->color() == 0) {
				gold += (*it)->amount();
			}
			++it;
		}

		sendStats.setGold(gold);
		sendStats.setSex( _player->gender() );
		sendStats.setPets( _player->controlslots() );
		sendStats.setMaxPets( _player->maxControlSlots() );
		sendStats.setStatCap( Config::instance()->statcap() );

		// Call the callback to insert additional aos combat related info
		cPythonScript* global = ScriptManager::instance()->getGlobalHook( EVENT_SHOWSTATUS );

		if ( global )
		{
			PyObject* args = Py_BuildValue( "NN", PyGetCharObject( _player ), CreatePyPacket( &sendStats ) );
			global->callEvent( EVENT_SHOWSTATUS, args );
			Py_DECREF( args );
		}
	}

	send( &sendStats );

	// Send Statlocks
	if ( pChar == _player )
	{
		cUOTxExtendedStats stats;
		stats.setSerial( _player->serial() );
		stats.setLocks( _player->strengthLock(), _player->dexterityLock(), _player->intelligenceLock() );
		send( &stats );

		// Send the packet to our party members too
		if ( _player->party() )
		{
			QPtrList<cPlayer> members = _player->party()->members();

			for ( P_PLAYER member = members.first(); member; member = members.next() )
			{
				if ( member->socket() && member != _player )
				{
					member->socket()->send( &sendStats );
				}
			}
		}
	}
}

bool cUOSocket::inRange( cUOSocket* socket ) const
{
	if ( !socket || !socket->player() || !_player )
		return false;
	return ( socket->player()->dist( _player ) < socket->player()->visualRange() );
}

void cUOSocket::handleSecureTrading( cUORxSecureTrading* packet )
{
	//	Trade::trademsg( this, packet );
	this->player()->onTrade( packet->type(), packet->buttonstate(), packet->itemserial() );
}

void cUOSocket::sendSkill( Q_UINT16 skill )
{
	if ( !_player )
		return;

	cUOTxUpdateSkill pUpdate;
	pUpdate.setId( skill );
	pUpdate.setValue( _player->skillValue( skill ) );
	pUpdate.setRealValue( _player->skillValue( skill ) );

	Q_UINT8 lock = _player->skillLock( skill );

	if ( lock == 0 )
		pUpdate.setStatus( cUOTxUpdateSkill::Up );
	else if ( lock == 1 )
		pUpdate.setStatus( cUOTxUpdateSkill ::Down );
	else
		pUpdate.setStatus( cUOTxUpdateSkill ::Locked );

	send( &pUpdate );
}

void cUOSocket::handleAction( cUORxAction* packet )
{
	if ( !_player )
		return;

	switch ( packet->type() )
	{
		// Skill use
		case 0x24:
			{
				QStringList skillParts = QStringList::split( " ", packet->action() );
				if ( skillParts.count() > 1 )
					Skills::instance()->SkillUse( this, skillParts[0].toInt() );
			}
			break;

		default:
			break;
	}
	return;
}

void cUOSocket::handleGumpResponse( cUORxGumpResponse* packet )
{
	// There is special handling for the virtue gump button
	// on the paperdoll.
	if ( packet->type() == 461 )
	{
		P_CHAR target = World::instance()->findChar( packet->serial() );
		std::vector<unsigned int> switches = packet->choice().switches;

		if ( switches.size() == 1 )
		{
			target = World::instance()->findChar( switches[0] );
		}

		if ( target )
		{
			PyObject *args = Py_BuildValue( "(NN)", target->getPyObject(), _player->getPyObject() );
			target->callEventHandler( EVENT_SHOWVIRTUEGUMP, args );
			Py_DECREF( args );
		}

		return;
	}

	QMap<SERIAL, cGump*>::iterator it( gumps.find( packet->serial() ) );

	if ( it == gumps.end() )
	{
		sysMessage( tr( "Unexpected button input." ) );
		return;
	}

	cGump* pGump = it.data();

	if ( pGump )
	{
		pGump->handleResponse( this, packet->choice() );
		delete pGump;
		gumps.erase( it );
	}
}

struct buyitem_st
{
	buyitem_st() : buyprice( 0 )
	{
	}
	buyitem_st( int bi, const QString& n ) : buyprice( bi ), name( n )
	{
	}
	int buyprice;
	QString name;
};

class SortedSerialList : public QPtrList<cItem>
{
protected:
	virtual int compareItems( QPtrCollection::Item item1, QPtrCollection::Item item2 )
	{
		return ( ( P_ITEM ) item1 )->serial() - ( ( P_ITEM ) item2 )->serial();
	}
};

void cUOSocket::sendVendorCont( P_ITEM pItem )
{
	pItem->update( this ); // Make sure it's visible to the client

	// Only allowed for pItem's contained by a character
	cUOTxItemContent itemContent;
	cUOTxVendorBuy vendorBuy;
	vendorBuy.setSerial( pItem->serial() );

	/* dont ask me, but the order of the items for vendorbuy is reversed */
	QValueList<buyitem_st> buyitems;
	QValueList<buyitem_st>::const_iterator bit;
	QPtrList<cItem> items;

	SortedSerialList sortedList;
	for ( ContainerIterator it( pItem ); !it.atEnd(); ++it )
	{
		sortedList.append( *it );
	}
	sortedList.sort();

	bool restockNow = false;
	if ( pItem->layer() == cBaseChar::BuyRestockContainer )
	{
		if ( pItem->hasTag( "last_restock_time" ) )
		{
			if ( uint( pItem->getTag( "last_restock_time" ).toInt() ) + Config::instance()->shopRestock() * 60 * MY_CLOCKS_PER_SEC < Server::instance()->time() )
			{
				restockNow = true;
				pItem->setTag( "last_restock_time", cVariant( int( Server::instance()->time() ) ) );
			}
		}
		else
		{
			pItem->setTag( "last_restock_time", cVariant( int( Server::instance()->time() ) ) );
		}
	}

	unsigned int i = 0;
	for ( P_ITEM mItem = sortedList.first(); mItem; mItem = sortedList.next() )
	{
		if ( restockNow )
			mItem->setRestock( mItem->amount() );

		unsigned short amount = pItem->layer() == cBaseChar::BuyRestockContainer ? mItem->restock() : mItem->amount();
		if ( amount >= 1 && mItem->buyprice() > 0 )
		{
			itemContent.addItem( mItem->serial(), mItem->id(), mItem->color(), i + 1, 1, amount, pItem->serial() );
			items.append( mItem );
			++i;
		}
	}

	// Reverse items

	send( &itemContent );
	for ( P_ITEM item = items.last(); item; item = items.prev() )
	{
		vendorBuy.addItem( item->buyprice(), "" );
		item->sendTooltip( this );
	}
	send( &vendorBuy );
}

void cUOSocket::sendBuyWindow( P_NPC pVendor )
{
	P_ITEM pBought = pVendor->atLayer( cBaseChar::BuyNoRestockContainer );
	P_ITEM pStock = pVendor->atLayer( cBaseChar::BuyRestockContainer );
	P_ITEM pSell = pVendor->atLayer( cBaseChar::SellContainer );

	if ( !pBought || !pStock || !pSell )
	{
		return;
	}

	unsigned int restockInterval = Config::instance()->shopRestock() * 60 * MY_CLOCKS_PER_SEC;
	unsigned int inventoryDecay = 60 * 60 * MY_CLOCKS_PER_SEC;

	// Perform maintaineance
	int lastRestockTime = pStock->getTag( "last_restock_time" ).toInt();

	// If the next restock interval has been reached or if the last restock time is in the future (server restart)
	// restock the vendor
	if ( (unsigned int)( lastRestockTime + restockInterval ) < Server::instance()->time() || (unsigned int)( lastRestockTime ) > Server::instance()->time() )
	{
		pStock->setTag( "last_restock_time", Server::instance()->time() ); // Set the last restock time

		for ( ContainerIterator it( pStock ); !it.atEnd(); ++it )
		{
			P_ITEM pItem = *it;

			// This increases the maximum amount of this item by a factor of 2 if
			// it has been sold out
			if ( pItem->restock() <= 0 )
			{
				pItem->setAmount( pItem->amount() * 2 );
				unsigned short maxamount = pItem->basedef() ? pItem->basedef()->getIntProperty("max_restock_amount", 999) : 999;
				if ( maxamount > 0 && pItem->amount() > maxamount)
				{
					pItem->setAmount( maxamount );
				}
				// If more than half of the items were still in stock when restocking
				// was issued, half the amount
			}
			else if ( pItem->restock() >= pItem->amount() / 2 )
			{
				if ( pItem->amount() == 999 )
				{
					pItem->setAmount( 640 );
				}
				else if ( pItem->amount() > ( pItem->basedef() ? pItem->basedef()->getIntProperty("min_restock_amount", 10) : 10 )  )
				{
					pItem->setAmount( pItem->amount() / 2 );
				}
			}

			pItem->setRestock( pItem->amount() ); // Restock full
		}
	}

	// Build the list of items to be sent.
	SortedSerialList itemList;

	// Process all items for sale first
	for ( ContainerIterator it( pStock ); !it.atEnd(); ++it )
	{
		if ( itemList.count() >= 250 )
		{
			break; // Only 250 items fit into the buy packet
		}

		if ( ( *it )->buyprice() <= 0 || ( *it )->restock() <= 0 )
		{
			continue; // This item is not for sale
		}

		itemList.append( *it );
	}

	// Now process all items that have been bought by the vendor
	for ( ContainerCopyIterator it2( pBought ); !it2.atEnd(); ++it2 )
	{
		// Check all bought items if they decayed (one hour)
		int buy_time = ( *it2 )->getTag( "buy_time" ).toInt();

		// Remove the item from the vendors inventory if its there for more than one hour or if it has been
		// bought in the future.
		if ( (unsigned int)(buy_time + inventoryDecay ) < Server::instance()->time() || (unsigned int)( buy_time ) > Server::instance()->time() )
		{
			( *it2 )->remove();
			continue;
		}

		// Only 250 items fit into the buy list
		if ( itemList.count() < 250 )
		{
			if ( ( *it2 )->buyprice() <= 0 )
			{
				continue; // This item is not for sale
			}

			itemList.append( *it2 );
		}
	}

	// Process the items we found for sale
	if ( itemList.count() == 0 )
	{
		return; // Nothing for sale
	}

	// Send both containers to the client
	pBought->update( this );
	pStock->update( this );
	pSell->update( this );

	itemList.sort(); // Organize the container content by serial

	// Create the container content
	cUOTxItemContent containerContent;
	containerContent.resize( 5 + itemList.count() * 19 );
	containerContent.setShort( 1, containerContent.size() );
	containerContent.setShort( 3, itemList.count() );
	unsigned int pOffset = containerContent.size() - 19; // Start at the last item
	unsigned int i = itemList.count();

	// This packet has the pricing information
	cUOTxVendorBuy vendorBuy;
	vendorBuy.setSerial( pStock->serial() );

	// This is something i dont understand. Why does it have to be backwards??
	SortedSerialList::const_iterator cit( itemList.begin() );
	while ( cit != itemList.end() )
	{
		P_ITEM pItem = *( cit++ ); // Get the current item and advance to the next
		containerContent.setInt( pOffset, pItem->serial() );
		containerContent.setShort( pOffset + 4, pItem->id() );
		containerContent[pOffset + 6] = 0; // Unknown

		if ( pItem->hasTag( "restock" ) )
		{
			containerContent.setShort( pOffset + 7, pItem->restock() );
		}
		else
		{
			containerContent.setShort( pOffset + 7, pItem->amount() );
		}
		containerContent.setShort( pOffset + 9, i-- ); // Item Id in packet (1 to n)
		containerContent.setShort( pOffset + 11, pItem->amount() );
		containerContent.setInt( pOffset + 13, pStock->serial() );
		containerContent.setShort( pOffset + 17, pItem->color() );
		pOffset -= 19; // Previous item

		if ( !pItem->name().isEmpty() )
			vendorBuy.addItem( pItem->buyprice(), pItem->name() ); // add it to the other packet as well
		else
			vendorBuy.addItem( pItem->buyprice(), QString::number( 1020000 + pItem->id() ) );
	}

	send( &containerContent ); // Send container content
	send( &vendorBuy ); // Send pricing information

	cUOTxDrawContainer drawContainer;
	drawContainer.setSerial( pVendor->serial() );
	drawContainer.setGump( 0x30 );
	send( &drawContainer );

	cit = itemList.begin();
	while ( cit != itemList.end() )
	{
		( *( cit++ ) )->sendTooltip( this );
	}

	// Send status gump with gold info
	sendStatWindow();
}

static void walkSellItems( P_ITEM pCont, P_ITEM pPurchase, QPtrList<cItem>& items )
{
	// For every pack item search for an equivalent sellitem
	for ( ContainerIterator pit( pCont ); !pit.atEnd(); ++pit )
	{
		P_ITEM pItem = *pit;

		// Containers with content are walked and _not_ sold
		if ( pItem->type() == 1 && pItem->content().count() > 0 )
		{
			walkSellItems( pItem, pPurchase, items );
		}
		else
		{
			// Search all sellable items
			for ( ContainerIterator it( pPurchase ); !it.atEnd(); ++it )
			{
				P_ITEM mItem = *it;
				if ( pItem->baseid() == mItem->baseid() && pItem->scriptList() == mItem->scriptList() )
				{
					items.append( pItem );
					break; // Break the inner loop
				}
			}
		}
	}
}

void cUOSocket::sendSellWindow( P_NPC pVendor, P_CHAR pSeller )
{
	P_ITEM pPurchase = pVendor->atLayer( cBaseChar::SellContainer );
	P_ITEM pBackpack = pSeller->getBackpack();

	if ( pPurchase && pBackpack )
	{
		QPtrList<cItem> items;
		cUOTxSellList itemContent;
		itemContent.setSerial( pVendor->serial() );

		walkSellItems( pBackpack, pPurchase, items );

		// Transfer the found items to the sell list
		P_ITEM pItem;
		unsigned int count = 0;
		for ( pItem = items.first(); pItem; pItem = items.next() )
		{
			unsigned int sellprice = pItem->getSellPrice( pVendor );
			if ( sellprice != 0 )
			{
				itemContent.addItem( pItem->serial(), pItem->id(), pItem->color(), pItem->amount(), sellprice, pItem->getName() );
				++count;
			}
		}

		if ( items.count() == 0 )
		{
			pVendor->talk( 501550, 0, 0, false, pVendor->saycolor(), this );
			return;
		}

		pVendor->talk( 501530, 0, 0, false, pVendor->saycolor(), this );
		send( &itemContent );
		for ( P_ITEM item = items.first(); item; item = items.next() )
		{
			item->sendTooltip( this );
		}
	}
}

void cUOSocket::handleHelpRequest( cUORxHelpRequest* packet )
{
	Q_UNUSED( packet );
	player()->onHelp();
}

void cUOSocket::handleSkillLock( cUORxSkillLock* packet )
{
	player()->setSkillLock( packet->skill(), packet->lock() );
}

void cUOSocket::handleBuy( cUORxBuy* packet )
{
	Trade::buyAction( this, packet );
}

void cUOSocket::handleSell( cUORxSell* packet )
{
	Trade::sellAction( this, packet );
}

/*
thanks to codex
*/
void cUOSocket::clilocMessage( const Q_UINT32 MsgID, const QString& params, const Q_UINT16 color, const Q_UINT16 font, cUObject* object, bool system )
{
	cUOTxClilocMsg msg;

	if ( object != 0 )
	{
		msg.setSerial( object->serial() );

		if ( !system )
		{
			msg.setType( cUOTxClilocMsg::OnObject );
		}
		else
		{
			msg.setType( cUOTxClilocMsg::LowerLeft );
		}
		if ( object->isChar() )
		{
			P_CHAR pchar = dynamic_cast<P_CHAR>( object );
			if ( pchar )
			{
				msg.setBody( pchar->body() );
			}
			msg.setName( object->name() );
		}
		else
		{
			msg.setName( "Item" );
		}
	}
	else
	{
		msg.setSerial( 0xFFFFFFFF );
		msg.setType( cUOTxClilocMsg::LowerLeft );
		msg.setName( "System" );
	}
	msg.setBody( 0xFF );
	msg.setHue( color );
	msg.setFont( font );
	msg.setMsgNum( MsgID );

	msg.setParams( params );

	send( &msg );
}

void cUOSocket::clilocMessageAffix( const Q_UINT32 MsgID, const QString& params, const QString& affix, const Q_UINT16 color, const Q_UINT16 font, cUObject* object, bool dontMove, bool prepend, bool system )
{
	cUOTxClilocMsgAffix msg;

	if ( object != 0 )
	{
		msg.setSerial( object->serial() );
		msg.setType( cUOTxClilocMsg::OnObject );
		if ( object->isChar() )
			msg.setName( object->name() );
		else
			msg.setName( "Item" );
	}
	else
	{
		msg.setSerial( 0xFFFFFFFF );
		msg.setType( cUOTxClilocMsg::LowerLeft );
		msg.setName( "System" );
	}
	msg.setBody( 0xFF );
	msg.setHue( color );
	msg.setFont( font );

	Q_UINT8 flags = 0;
	if ( prepend )
		flags |= cUOTxClilocMsgAffix::Prepend;
	if ( dontMove )
		flags |= cUOTxClilocMsgAffix::DontMove;
	if ( system )
		flags |= cUOTxClilocMsgAffix::System;
	msg.setFlags( flags );

	msg.setMsgNum( MsgID );

	msg.setParams( affix, params );

	send( &msg );
}

void cUOSocket::cancelTarget()
{
	cUOTxTarget target;
	target.setTargSerial( 0 );
	target[6] = 3; // Cursor Type
	send( &target );

	if ( targetRequest )
	{
		targetRequest->canceled( this );
		delete targetRequest;
		targetRequest = 0;
	}
}

void cUOSocket::updateLightLevel()
{
	if ( _player )
	{
		UI08 level;
		cUOTxLightLevel pLight;
		cTerritory* region = Territories::instance()->region( _player->pos() );
		if ( region && region->isCave() )
		{
			level = wpMin<int>( 0x1f, wpMax<int>( 0, Config::instance()->dungeonLightLevel() - static_cast<int>( _player->fixedLightLevel() ) ) );
		}
		else
		{
			level = wpMin<int>( 0x1f, wpMax<int>( 0, Config::instance()->worldCurrentLevel() - static_cast<int>( _player->fixedLightLevel() ) ) );
		}
		pLight.setLevel( level );
		send( &pLight );
	}
}

void cUOSocket::handleProfile( cUORxProfile* packet )
{
	if ( !_player )
		return;

	// Send the requested profile
	if ( packet->size() <= 8 )
	{
		P_CHAR pChar = FindCharBySerial( packet->serial() );

		if ( pChar )
		{
			cUOTxProfile profile;
			profile.setSerial( packet->serial() );
			profile.setInfo( pChar->name(), pChar->title(), ( pChar->objectType() == enPlayer ) ? dynamic_cast<P_PLAYER>( pChar )->profile() : QString( "" ) );
			send( &profile );
		}
	}
	// Check if it is a update request
	else if ( packet->command() )
	{
		// You can only change your own profile
		if ( packet->serial() != _player->serial() )
			sysMessage( tr( "You can only change your own profile" ) );
		else
			_player->setProfile( packet->text() );
	}
}

void cUOSocket::handleRename( cUORxRename* packet )
{
	if ( !_player )
		return;

	// If we are no GM we can only rename characters
	// we own (only pets here!)
	if ( packet->serial() == _player->serial() )
		sysMessage( tr( "You can't rename yourself" ) );
	else
	{
		P_NPC pChar = dynamic_cast<P_NPC>( FindCharBySerial( packet->serial() ) );
		if ( pChar && pChar->owner() == _player && !pChar->isHuman() )
		{
			pChar->setName( packet->name() );

			if ( pChar->name().length() > 29 )
			{
				pChar->setName( pChar->name().left( 29 ) );
				sysMessage( tr( "This name was too long, i truncated it to 29 characters." ) );
			}

			pChar->resendTooltip();
		}
		else
			sysMessage( tr( "You can't rename this." ) );
	}
}

void cUOSocket::sendQuestArrow( bool show, Q_UINT16 x, Q_UINT16 y )
{
	cUOTxQuestArrow qArrow;
	qArrow.setActive( show ? 1 : 0 );
	qArrow.setPos( Coord( x, y, 0, 0 ) );
	send( &qArrow );
}

void cUOSocket::closeGump( Q_UINT32 type, Q_UINT32 returnCode )
{
	cUOTxCloseGump closegump;
	closegump.setButton( returnCode );
	closegump.setType( type );
	send( &closegump );
}

void cUOSocket::addTooltip( Q_UINT32 data )
{
	if ( data >= tooltipscache_->size() )
		tooltipscache_->resize( data + 2 );

	tooltipscache_->setBit( data );
}

void cUOSocket::log( const QString& message )
{
	Log::instance()->print( LOG_MESSAGE, this, message );
}

void cUOSocket::log( eLogLevel loglevel, const QString& message )
{
	Log::instance()->print( loglevel, this, message );
}

bool cUOSocket::canSee( cUOSocket* socket )
{
	if ( !socket || !socket->player() || !_player )
	{
		return false;
	}
	else
	{
		return _player->canSee( socket->player() );
	}
}

bool cUOSocket::canSee( P_ITEM item )
{
	if ( !item || !_player )
	{
		return false;
	}
	else
	{
		return _player->canSeeItem( item );
	}
}

bool cUOSocket::canSee( P_CHAR character )
{
	if ( !character || !_player )
	{
		return false;
	}
	else
	{
		return _player->canSeeChar( character );
	}
}

bool cUOSocket::canSee( cUObject* object )
{
	if ( !object || !_player )
	{
		return false;
	}
	else
	{
		return _player->canSee( object );
	}
}

void cUOSocket::handleExtendedStats( cUORxExtendedStats* packet )
{
	unsigned char lock = packet->lock();
	unsigned char stat = packet->stat();

	if ( lock > 2 )
	{
		log( LOG_WARNING, tr( "Wrong lock value for extended stats packet: %1\n" ).arg( lock ) );
		return;
	}

	switch ( stat )
	{
		case 0:
			_player->setStrengthLock( lock );
			break;

		case 1:
			_player->setDexterityLock( lock );
			break;

		case 2:
			_player->setIntelligenceLock( lock );
			break;

		default:
			log( LOG_WARNING, tr( "Wrong stat value for extended stats packet: %1\n" ).arg( stat ) );
			break;
	}
}

void cUOSocket::handleChat( cUOPacket* packet )
{
	Q_UNUSED( packet );
	_player->onChat();
}

bool cUOSocket::useItem( P_ITEM item )
{
	if (_player->isJailed()) {
		sysMessage(tr("You cannot use items while you are in jail."));
		return false;
	}

	P_ITEM outmostItem = item->getOutmostItem();

	cMulti *multi = outmostItem->multi();

	// Check security if using items within a multi
	if ( multi && multi->canHandleEvent( EVENT_CHECKSECURITY ) )
	{
		PyObject *args = Py_BuildValue( "(NNN)", _player->getPyObject(), multi->getPyObject(), outmostItem->getPyObject() );
		bool result = multi->callEventHandler( EVENT_CHECKSECURITY, args );
		Py_DECREF( args );

		if ( result )
		{
			return false; // Access Denied
		}
	}

	if ( item->type() != 1 && !item->corpse() )
	{
		if ( !_player->isGM() && _player->objectDelay() >= Server::instance()->time() )
		{
			sysMessage( tr( "You must wait to perform another action." ) );
			return false;
		}
		else
			_player->setObjectDelay( Config::instance()->objectDelay() * MY_CLOCKS_PER_SEC + Server::instance()->time() );
	}

	// Dead ppl can only use ankhs
	if ( _player->isDead() && item->type() != 16 )
	{
		sysMessage( tr( "Your ghostly hand passes trough the object." ) );
		return false;
	}

	// Items in locked items cannot be used
	if (item->isInLockedItem()) {
		sysMessage(tr( "You have to unlock the container before using it."));
		return false;
	}

	// Cant use stuff that isn't in your pack.
	P_CHAR pOutmostChar = item->getOutmostChar();

	// It's not a container and not on use, assume it has a special behaviour that is not activated
	// using snooping
	if ( pOutmostChar && pOutmostChar != _player && item->type() != 1 )
	{
		bool allowed = false;

		PyObject *args = Py_BuildValue( "(NN)", _player->getPyObject(), item->getPyObject() );

		if ( _player->callEventHandler( EVENT_REMOTEUSE, args ) )
		{
			allowed = true;
		}

		if ( !allowed && pOutmostChar->callEvent( EVENT_REMOTEUSE, args ) )
		{
			allowed = true;
		}

		Py_DECREF( args );

		if ( !allowed )
			return false;
	}

	// Call both events here
	if ( _player->onUse( item ) )
		return true;

	if ( item->onUse( _player ) )
		return true;

	// Criminal for looting an innocent corpse & unhidden if not owner...
	if ( item->corpse() )
	{
		if ( !_player->owns( item ) && !_player->isGM() )
		{
			_player->unhide();
		}

		// TODO: Add a XML option for this
		if ( !_player->owns( item ) && !_player->isGM() && _player->isInnocent() )
		{
			// Innocent Corpse and not in the same party && party allowance for looting?
			if ( item->hasTag( "notoriety" ) && item->getTag( "notoriety" ).toInt() == 0x01 )
			{
				P_PLAYER owner = dynamic_cast<P_PLAYER>( item->owner() );
				bool allowed = false;

				if ( owner && owner->party() && owner->party() == _player->party() )
				{
					// Check if the player allowed looting his corpse by party members
					if ( owner->party()->lootingAllowed().contains( owner ) )
					{
						allowed = true;
					}
				}

				if ( !allowed )
				{
					_player->makeCriminal();
				}
			}
		}

		// Get the corpse owner
		P_PLAYER owner = dynamic_cast<P_PLAYER>( item->owner() );

		if ( owner && owner != _player )
		{
			log( LOG_NOTICE, tr( "Looking into corpse of player '%1' ('%2', 0x%3)\n" ).arg( owner->name() ).arg( owner->account() ? owner->account()->login() : QString( "unknown" ) ).arg( owner->serial(), 0, 16 ) );
		}
	}

	// You can only use equipment on your own char
	if ( !_player->isGM() && item->container() && item->container()->isChar() && item->container() != _player )
	{
		if ( item->layer() != 15 || !Config::instance()->stealingEnabled() )
		{
			sysMessage( tr( "You cannot use items equipped by other players." ) );
			return false;
		}
	}

	// Check item behaviour by it's tpye
	switch ( item->type() )
	{
		case 1:
		// normal containers
		{
			_player->setObjectDelay( 0 );	// no delay for opening containers

			if ( _player->isGM() )
			{
				sendContainer( item );
				return true;
			}

			if ( item->layer() > 0x18 )
			{
				sysMessage( tr( "You can't see this." ) );
				return false;
			}

			if ( !item->container() )
			{
				if ( !item->inRange( _player, 2 ) )
				{
					clilocMessage( 0x7A258, "", 0x3b2 ); // You cannot reach that
					return false;
				}
				else if ( !_player->lineOfSight( item ) )
				{
					clilocMessage( 0x7A258, "", 0x3b2 ); // You cannot reach that
					return false;
				}

				sendContainer( item );
				return true;
			}
			else if ( item->container()->isItem() )
			{
				P_ITEM pOCont = item->getOutmostItem();

				// Check if we can reach the top-container
				if ( !pOCont->container() )
				{
					if ( !pOCont->inRange( _player, 2 ) )
					{
						clilocMessage( 0x7A258, "", 0x3b2 ); // You cannot reach that
						return false;
					}

					sendContainer( item );
				}
				else
				{
					P_CHAR pChar = dynamic_cast<P_CHAR>( pOCont->container() );
					if ( pChar && pChar != _player )
					{
						if ( !pChar->inRange( _player, 2 ) )
							sysMessage( tr( "You must stand nearer to snoop!" ) );
						else
							Skills::instance()->Snooping( _player, item );
					}
					else if ( pChar == _player )
						sendContainer( item );
				}

				return true;
			}
			else if ( item->container()->isChar() )
			{
				// Equipped on another character
				P_CHAR pChar = dynamic_cast<P_CHAR>( item->container() );

				if ( pChar && pChar != _player )
				{
					if ( !pChar->inRange( _player, 2 ) )
						sysMessage( tr( "You must stand nearer to snoop!" ) );
					else
						Skills::instance()->Snooping( _player, item );
				}
				else if ( pChar == _player )
					sendContainer( item );

				return true;
			}

			sysMessage( tr( "You can't open this container." ) );
			return false;
		}

		case 16:
			// Check for 'resurrect item type' this is the ONLY type one can use if dead.
			if ( _player->isDead() )
			{
				if ( _player->resurrect( item ) )
				{
					sysMessage( tr( "You have been resurrected." ) );
				}
			}
			else
			{
				sysMessage( tr( "You are already living!" ) );
			}
			return true;
			break;


		case 1001: // 1001: Sword Weapons (Swordsmanship)
		case 1002: // 1002: Axe Weapons (Swordsmanship + Lumberjacking)
		case 1005: // 1005: Fencing
		case 1003: // 1003: Macefighting (Staffs)
		case 1004: // 1004: Macefighting (Maces/WarHammer)
		case 1006: // 1006: Bows
		case 1007: // 1007: Crossbows
		case 1008: // 1008: Shields
			break;

		default:
			break;
	}

	sysMessage( tr( "You can't think of a way to use that item." ) );
	return false;
}

void cUOSocket::handleAllNames( cUORxAllNames* packet )
{
	cUObject* object = World::instance()->findObject( packet->serial() );

	// Send a packet back with the name of the requested object
	if ( object )
	{
		cUOTxAllNames allnames;
		allnames.setSerial( object->serial() );
		allnames.setName( object->name() );
		send( &allnames );
	}
}

void cUOSocket::handleResurrectionMenu( cUORxResurrectionMenu* packet )
{
	if ( packet->choice() != cUORxResurrectionMenu::Manifest )
	{
		return; // Only 0x00 toggles visibility
	}

	cUOTxWarmode warmode;
	warmode.setStatus( _player->isAtWar() );
	send( &warmode );

	return;
}
