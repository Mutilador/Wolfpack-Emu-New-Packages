//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================


#ifndef __UOSOCKET__
#define __UOSOCKET__

// Library Includes
#include "qcstring.h"
#include "qsocketdevice.h"

// Forward Declarations
class cUOPacket;

// Too many Forward Declarations
#include "uorxpackets.h"
#include "uotxpackets.h"
#include "../typedefs.h"

class cUOSocket
{
public:
	enum eSocketState	{ Connecting = 0, LoggingIn, LoggedIn, InGame	};
private:
	QSocketDevice *_socket;
	Q_UINT32 _rxBytes, _txBytes, _uniqueId;
	Q_INT32 _account; // Our account-id > should be a pointer
	P_CHAR _player;
	eSocketState _state;
	Q_UINT8 lastPacket, _viewRange;
	Q_UINT8 _walkSequence;
	QString _lang,_version;

	bool authenticate( const QString &username, const QString &password );
	void giveNewbieItems( cUORxCreateChar *packet, Q_UINT8 skill = 0xFF );

public:
	Q_UINT8 walkSequence( void )			{ return _walkSequence; }
	void setWalkSequence( Q_UINT8 data )	{ _walkSequence = data; }

	cUOSocket( QSocketDevice *sDevice ): 
		_walkSequence( 0xFF ), lastPacket( 0xFF ), _state( LoggingIn ), _lang( "ENU" ),
		_account(-1), _player(0), _rxBytes(0), _txBytes(0), _socket( sDevice ) {}
	virtual ~cUOSocket( void ) { delete _socket; }

	QSocketDevice *socket( void ) { return _socket; }
	void setSocket( QSocketDevice *data ) { _socket = data; }

	eSocketState state( void ) { return _state; }
	void setState( eSocketState data ) { _state = data; }

	QString version( void )		{ return _version; }
	QString lang( void )		{ return _lang; }

	P_CHAR player( void )		{ return _player; }

	Q_UINT32 rxBytes( void )	{ return _rxBytes; }
	Q_UINT32 txBytes( void )	{ return _txBytes; }
	Q_UINT8 viewRange( void )	{ return _viewRange; }
	void setRxBytes( Q_UINT32 data ) { _rxBytes = data; }
	void setTxBytes( Q_UINT32 data ) { _txBytes = data; }

	Q_UINT32 uniqueId( void ) { return _uniqueId; }

	void recieve(); // Tries to recieve one packet and process it
	void send( cUOPacket *packet );

	// Handler
	void handleLoginRequest( cUORxLoginRequest *packet );
	void handleHardwareInfo( cUORxHardwareInfo *packet );
	void handleSelectShard( cUORxSelectShard *packet );
	void handleServerAttach( cUORxServerAttach *packet );
	void handleDeleteCharacter( cUORxDeleteCharacter *packet );
	void handlePlayCharacter( cUORxPlayCharacter *packet );
	void handleCreateChar( cUORxCreateChar *packet );
	void handleUpdateRange( cUORxUpdateRange *packet );
	void handleQuery( cUORxQuery *packet );
	void handleRequestLook( cUORxRequestLook *packet );
	void handleMultiPurpose( cUORxMultiPurpose *packet );
	void handleContextMenuRequest( cUORxContextMenuRequest *packet );
	void handleWalkRequest( cUORxWalkRequest* packet );
	void handleSetLanguage( cUORxSetLanguage* packet );
	void handleSpeechRequest( cUORxSpeechRequest* packet );
	void handleDoubleClick( cUORxDoubleClick* packet );
	void handleGetTip( cUORxGetTip* packet );

	// Utilities
	void updateChar( P_CHAR pChar );
	void sendChar( P_CHAR pChar );
	void showSpeech( cUObject *object, const QString &message, Q_UINT16 color = 0xFFFF, Q_UINT16 font = 0xFFFF, cUOTxUnicodeSpeech::eSpeechType speechType = cUOTxUnicodeSpeech::Regular );
	void sysMessage( const QString &message, Q_UINT16 color = 0x0037 );
	void sendCharList();
	void setPlayer( P_CHAR pChar = NULL ); // Updates the current player
	void updateCharList();
	void disconnect( void ); // Call this whenever the socket should disconnect
	void playChar( P_CHAR player ); // Play a character
	bool isT2A()	{ return true; } // ???

	void allowMove( Q_UINT8 sequence );
	void denyMove( Q_UINT8 sequence );
};

#endif
