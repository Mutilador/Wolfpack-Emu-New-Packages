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

enum eSocketState
{
	SS_CONNECTING = 0,
	SS_LOGGINGIN,
	SS_LOGGEDIN,
	SS_INGAME
};

class cUOSocket
{
private:
	QSocketDevice *_socket;
	Q_UINT32 _rxBytes, _txBytes;
	void *player,*account;
	eSocketState _state;
	Q_UINT32 _uniqueId;

public:
	cUOSocket( QSocketDevice *sDevice ): 
		_state( SS_LOGGINGIN ), account(0), player(0), _rxBytes(0), _txBytes(0), _socket( sDevice ) {}
	virtual ~cUOSocket( void ) { delete _socket; }

	QSocketDevice *socket( void ) { return _socket; }
	void setSocket( QSocketDevice *data ) { _socket = data; }

	eSocketState state( void ) { return _state; }
	void setState( eSocketState data ) { _state = data; }

	Q_UINT32 rxBytes( void ) { return _rxBytes; }
	Q_UINT32 txBytes( void ) { return _txBytes; }
	void setRxBytes( Q_UINT32 data ) { _rxBytes = data; }
	void setTxBytes( Q_UINT32 data ) { _txBytes = data; }

	Q_UINT32 uniqueId( void ) { return _uniqueId; }

	void recieve(); // Tries to recieve one packet and process it
	void send( cUOPacket *packet );

	void disconnect( void ); // Call this whenever the socket should disconnect

	// Handler
	void handleLoginRequest( cUORxLoginRequest *packet );
	void handleHardwareInfo( cUORxHardwareInfo *packet );
	void handleSelectShard( cUORxSelectShard *packet );
	void handleServerAttach( cUORxServerAttach *packet );
	void handleDeleteCharacter( cUORxDeleteCharacter *packet );
	void handlePlayCharacter( cUORxPlayCharacter *packet );

	// Utilities
	void sendCharList( const QString &username );
};

#endif