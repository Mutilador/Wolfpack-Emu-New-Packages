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

// Platform Includes
#include "platform.h"

// Wolfpack Includes
#include "accounts.h"
#include "speech.h"

#include "mapobjects.h"
#include "uotime.h"
#include "serverconfig.h"
#include "network/network.h"
#include "territories.h"
#include "network/uosocket.h"
#include "skills.h"
#include "pythonscript.h"
#include "items.h"
#include "player.h"
#include "npc.h"
#include "ai/ai.h"
#include "world.h"
#include "inlines.h"
#include "console.h"

// Library Includes
#include <qdatetime.h>
#include <qfile.h>
#include <vector>

using namespace std;

#undef  DBGFILE
#define DBGFILE "speech.cpp"

bool InputSpeech( cUOSocket* socket, P_PLAYER pChar, const QString& speech )
{
	if ( pChar->inputMode() == cPlayer::enNone )
		return false;

	P_ITEM pItem = FindItemBySerial( pChar->inputItem() );

	if ( !pItem )
		return false;

	//bool ok = false;
	//Q_INT32 num = speech.toInt( &ok ); // Generally try to convert it
	QString notification;

	switch ( pChar->inputMode() )
	{
		// Pricing an item - PlayerVendors
		case cPlayer::enPricing:
			socket->sysMessage( "Ops, sorry not implemented" );
			pChar->setInputMode( cPlayer::enDescription );
			socket->sysMessage( tr( "Enter a description for this item." ) );
			break;

			// Describing an item
		case cPlayer::enDescription:
			socket->sysMessage( "Description is not used anywhere :( not implemented right now" );
			socket->sysMessage( tr( "This item is now described as %1." ).arg( speech ) );
			pChar->setInputMode( cPlayer::enNone );
			pChar->setInputItem( INVALID_SERIAL );
			break;

			// Renaming ourself
		case cPlayer::enNameDeed:
			pChar->setName( speech );
			socket->sysMessage( tr( "Your new name is: %1" ).arg( speech ) );
			pChar->setInputMode( cPlayer::enNone );
			pChar->setInputItem( INVALID_SERIAL );
			break;

			// Renaming a house sign
		case cPlayer::enHouseSign:
			pItem->setName( speech );
			socket->sysMessage( tr( "Your house has been renamed to: %1" ).arg( speech ) );
			pChar->setInputMode( cPlayer::enNone );
			pChar->setInputItem( INVALID_SERIAL );
			break;

		default:
			break;	// do nothing
	}

	return true;
}

// All this Stuff should be scripted
bool QuestionSpeech( cUOSocket* socket, P_PLAYER pPlayer, P_NPC pChar, const QString& comm )
{
	if ( socket )
	{
		// Warning Fix
	}

	if ( !pChar->isHuman() || pPlayer->dist( pChar ) > 3 )
		return false;

	// Tell the questioner our name
	if ( comm.contains( "NAME" ) )
	{
		pChar->talk( tr( "Hello, my name is %1." ).arg( pChar->name() ) );
		return true;
	}

	// say time and the npChar gives the time.
	if ( comm.contains( "TIME" ) )
	{
		pChar->talk( tr( "It is now %1 hours and %2 minutes." ).arg( UoTime::instance()->hour() ).arg( UoTime::instance()->minute() ) );
		return true;
	}

	if ( comm.contains( "LOCATION" ) )
	{
		cTerritory* Region = pPlayer->region();

		if ( Region )
			pChar->talk( tr( "You are in %1" ).arg( Region->name() ) );
		else
			pChar->talk( tr( "You are in the wilderness" ) );

		return true;
	}

	// We couldn't handle the speech
	return false;
}

bool VendorChkName( P_CHAR pVendor, const QString& comm )
{
	if ( ( comm.contains( "VENDOR" ) ) || ( comm.contains( "SHOPKEEPER" ) ) )
		return true;

	if ( comm.contains( pVendor->name(), false ) )
		return true;
	else
		return false;
}

/////////////////
// name:	response
// purpose:	tries to get a response from an npc standing around
// history:	heavily revamped/rewritten by Duke, Oct 2001
// remark:	The new logic tries to minimize the # of strstr() calls by *first* checking
//			what kind of npcs are standing around and then checking only those keywords
//			that they might be interested in.
//			This is especially usefull in crowded places.
bool Speech::response( cUOSocket* socket, P_PLAYER pPlayer, const QString& comm, QValueVector<Q_UINT16>& keywords )
{
	if ( !pPlayer->socket() || pPlayer->isDead() )
	{
		return false;
	}

	QString speechUpr = comm.upper();

	MapCharsIterator ri = MapObjects::instance()->listCharsInCircle( pPlayer->pos(), 18 );
	for ( P_CHAR pChar = ri.first(); pChar; pChar = ri.next() )
	{
		P_NPC pNpc = dynamic_cast<P_NPC>( pChar );

		// We will only process NPCs here
		if ( !pNpc )
			continue;

		// at least they should be on the screen
		if ( pPlayer->dist( pNpc ) > 16 )
			continue;

		if ( pNpc->canHandleEvent( EVENT_SPEECH ) )
		{
			PyObject* pkeywords = PyTuple_New( keywords.size() );

			// Set Items
			for ( unsigned int i = 0; i < keywords.size(); ++i )
				PyTuple_SetItem( pkeywords, i, PyInt_FromLong( keywords[i] ) );

			PyObject* args = Py_BuildValue( "(NNNO)", pNpc->getPyObject(), pPlayer->getPyObject(), QString2Python( comm ), pkeywords );

			bool result = pNpc->callEventHandler( EVENT_SPEECH, args );

			Py_DECREF( args );
			Py_DECREF( pkeywords );

			if ( result )
				return true;
		}

		if ( pNpc->ai() )
		{
			pNpc->ai()->onSpeechInput( pPlayer, speechUpr );
		}

		if ( QuestionSpeech( socket, pPlayer, pNpc, speechUpr ) )
			return true;
	}

	return false;
}

void Speech::talking( P_PLAYER pChar, const QString& lang, const QString& speech, QValueVector<Q_UINT16>& keywords, Q_UINT16 color, Q_UINT16 font, Q_UINT8 type ) // PC speech
{
	// handle things like renaming or describing an item
	if ( !pChar->socket() )
		return;

	cUOSocket* socket = pChar->socket();

	if ( InputSpeech( socket, pChar, speech ) )
		return;

	// Squelched
	if (pChar->isSquelched()) {
		socket->clilocMessage(500168); // You cannot say anything, you have been squelched.
		return;
	}

	// log
	pChar->log( LOG_SPEECH, tr( "Saying '%1' (%2, 0x%3).\n" ).arg( speech ).arg( font ).arg( color, 0, 16 ) );

	pChar->unhide();

	// Check for Bogus Color
	if ( !isNormalColor( color ) )
		color = 0x2;

	if ( type == 0 )
		pChar->setSaycolor( color );
	else if ( type == 2 )
		pChar->setEmoteColor( color );

	if ( pChar->onTalk( type, color, font, speech, lang ) )
		return;

	if ( ( type == 0x09 ) && ( pChar->mayBroadcast() ) )
	{
		pChar->talk( speech, color, type );
		return;
	}

	pChar->talk( speech, color, type );

	QString speechUpr = speech.upper();
	if ( response( socket, pChar, speech, keywords ) )
		return;  // Vendor responded already

	// this makes it so npcs do not respond to isDead people - HEALERS ??
	if ( pChar->isDead() )
		return;

	// 0x0007 -> Speech-id for "Guards"
	for ( QValueVector<Q_UINT16>::const_iterator iter = keywords.begin(); iter != keywords.end(); ++iter )
	{
		Q_UINT16 keyword = *iter;

		if ( keyword == 0x07 )
			pChar->callGuards();
	}

	/*	P_CHAR pc = NULL; ???
		P_CHAR pNpc = NULL;
		RegionIterator4Chars ri( pChar->pos() );
		for( ri.Begin(); !ri.atEnd(); ri++ )
		{
			pc = ri.GetData();
			if (!pc->isSameAs( pChar )
				&& pc->isNpc()
				&& pc->dist( pChar ) <= 2)
			{
				pNpc = pc;
				break;
			}
		}
		*/
}
