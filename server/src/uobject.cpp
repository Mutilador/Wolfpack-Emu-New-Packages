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
//	Wolfpack Homepage: http://www.wpdev.sf.net/
//========================================================================================

// Platform Specifics
#include "platform.h"

// Wolfpack includes
#include "coord.h"
#include "uobject.h"
#include "iserialization.h"
#include "globals.h"
#include "network.h"
#include "mapobjects.h"
#include "junk.h"
#include "defines.h"
#include "wpdefaultscript.h"
#include "wpscriptmanager.h"
#include "network/uosocket.h"
#include "wpdefmanager.h"
#include "persistentbroker.h"

// Library Includes
#include <qsqlcursor.h>


// Debug includes and defines
#undef  DBGFILE
#define DBGFILE "uobject.cpp"

cUObject::cUObject()
{
	init();
}

cUObject::cUObject( cUObject &src )
{
	// Copy Events
	this->setEvents( src.getEvents() );

	this->serial = src.serial;
	this->multis = src.multis;
	this->name = src.name;
	this->free = src.free;
}

void cUObject::init()
{
	this->serial = INVALID_SERIAL;
	this->multis = INVALID_SERIAL;
	this->free = false;
	this->bindmenu_ = QString::null;
}

void cUObject::moveTo( const Coord_cl& newpos )
{
	cMapObjects::getInstance()->remove( this );
	pos = newpos;
	cMapObjects::getInstance()->add( this );
}

/*!
	Performs persistency layer saves.
*/
void cUObject::save( const QString& )
{
	startSaveSqlStatement("uobject");
	savePersistentStrValue("name",		name);
	savePersistentIntValue("serial",	serial);
	savePersistentIntValue("multis",	multis);
	savePersistentIntValue("pos_x",		pos.x);
	savePersistentIntValue("pos_y",		pos.y);
	savePersistentIntValue("pos_z",		pos.z);
	savePersistentIntValue("pos_map",	pos.map);
	savePersistentStrValue("events",	eventList_.join(","));
	savePersistentStrValue("bindmenu",	bindmenu_);
	endSaveSqlStatement(QString("serial='%1'").arg(serial));
	// Update classes type map so we can properly load this later on.
	if ( !this->isPersistent )
	{
		QString query = QString("INSERT INTO uobjectmap SET serial='%1', type='%2';").arg(serial).arg(objectID());
		persistentBroker->executeQuery(query);
	}

	PersistentObject::save();
}

void cUObject::load( const QString& s )
{
	QSqlQuery query( QString( "SELECT * FROM uobject WHERE serial = '%1'" ).arg( s ) );
	query.first();

	name = query.value( 0 ).toString();
	serial = query.value( 1 ).toInt();
	multis = query.value( 2 ).toInt();
	pos.x = query.value( 3 ).toInt();
	pos.y = query.value( 4 ).toInt();
	pos.z = query.value( 5 ).toInt();
	pos.map = query.value( 6 ).toInt();

	// Load Events
	QString events = query.value( 7 ).toString();
	eventList_ = QStringList::split( ",", events );
	bindmenu_ = query.value( 8 ).toString();

	/*startLoadSqlStatement("uobject", "serial", s)
	{
		loadPersistentStrValue("name",		name);
		loadPersistentIntValue("serial",	serial);
		loadPersistentIntValue("multis",	multis);
		loadPersistentIntValue("pos_x",		pos.x);
		loadPersistentIntValue("pos_y",		pos.y);
		loadPersistentIntValue("pos_z",		pos.z);
		loadPersistentIntValue("pos_map",	pos.map);
		QString events;
		loadPersistentStrValue("events", events);
		eventList_ = QStringList::split(",", events);
		loadPersistentStrValue("bindmenu",	bindmenu_);
	}
	endLoadSqlStatement(s);*/
	PersistentObject::load(s);
}

bool cUObject::del( const QString& s/* = QString::null  */ )
{
	QSqlCursor cursor("uobject");
	cursor.select(QString("serial='%1'").arg(serial));
	while ( cursor.next() )
	{
		cursor.primeDelete();
		if ( cursor.del() > 1 )
		{
			qWarning("More than one record was deleted in table UObject when only 1 was expected, delete criteria was:");
			qWarning(cursor.filter());
		}

	}
	cursor.setName("uobjectmap");
	cursor.select(QString("serial='%1'").arg(serial));
	while ( cursor.next() )
	{
		cursor.primeDelete();
		if ( cursor.del() > 1 )
		{
			qWarning("More than one record was deleted in table uobjectmap when only 1 was expected, delete criteria was:");
			qWarning(cursor.filter());
		}
	}
	return PersistentObject::del( s );
}

/*!
	Clears the script-chain
*/
void cUObject::clearEvents()
{
	scriptChain.clear();
	eventList_.clear();
}

// Method for setting a list of WPDefaultScripts
void cUObject::setEvents( std::vector< WPDefaultScript* > List )
{
	scriptChain.clear();
	eventList_.clear();

	// "Reset" the events
	if( List.size() == 0 )
		return;

	// Walk the List and add the events
	for( UI08 i = 0; i < List.size(); i++ )
		if( List[ i ] != NULL )
		{
			scriptChain.push_back( List[ i ] );
			eventList_.push_back( List[ i ]->getName() );
		}
}

// Gets a vector of all assigned events
const std::vector< WPDefaultScript* > &cUObject::getEvents( void )
{
	/*std::vector< WPDefaultScript* > List;

	for( UI08 i = 0; i < scriptChain.size(); i++ )
		List.push_back( scriptChain[ i ] );*/

	return scriptChain;
}

// Checks if the object has a specific event
bool cUObject::hasEvent( QString Name )
{
	bool hasEvent = false;

	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->getName() == Name )
			hasEvent = true;

	return hasEvent;
}

void cUObject::addEvent( WPDefaultScript *Event )
{
	if( hasEvent( Event->getName() ) )
		return;

	scriptChain.push_back( Event );
	eventList_.push_back( Event->getName() );
}

void cUObject::removeEvent( QString Name )
{
	std::vector< WPDefaultScript* >::iterator myIterator;

	for( myIterator = scriptChain.begin(); myIterator != scriptChain.end(); ++myIterator )
	{
		if( (*myIterator)->getName() == Name )
			scriptChain.erase( myIterator );
	}
 
	// I hope this works
	eventList_.remove( Name );
}

/****************************
 * 
 * Scripting events
 *
 ****************************/

bool cUObject::onUse( cUObject *Target )
{
	// If we dont have any events assigned just skip processing
	if( scriptChain.empty() )
		return false;

	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
	{
		// If we're the Character pass us as the second param
		// if not as the first
		bool Handeled = false;

		if( this->objectID() != "CHARACTER" )
			Handeled = scriptChain[ i ]->onUse( (P_CHAR)Target, (P_ITEM)this );
		else
			Handeled = scriptChain[ i ]->onUse( (P_CHAR)this, (P_ITEM)Target );

		if( Handeled )
			return true;
	}

	return false;
}

bool cUObject::onCollide( cUObject* Obstacle )
{
	// If we dont have any events assigned just skip processing
	if( scriptChain.empty() )
		return false;

	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
	{
		// Items cannot collide with items
		if( this->objectID() != "CHARACTER" ) // Item, so obstacle has to be character
			scriptChain[ i ]->onCollideItem( (P_CHAR)Obstacle, (P_ITEM)this );
		else
			if( Obstacle->objectID() == "ITEM" )
				if( scriptChain[ i ]->onCollideItem( (P_CHAR)this, (P_ITEM)Obstacle ) )
					return true;

			else // Character, Character
				if( scriptChain[ i ]->onCollideChar( (P_CHAR)this, (P_CHAR)Obstacle ) )
					return true;
	}

	return false;
}

// Returns the list of events
QString cUObject::eventList( void )
{
	return eventList_.join( "," );
}

// If the scripts are reloaded call that for each and every existing object
void cUObject::recreateEvents( void )
{
	// Walk the eventList and recreate 
	QStringList::const_iterator myIter;

	scriptChain.clear();

	for( myIter = eventList_.begin(); myIter != eventList_.end(); ++myIter )
	{
		WPDefaultScript *myScript = ScriptManager->find( *myIter );

		// Script not found
		if( myScript == NULL )
			continue;

		scriptChain.push_back( myScript );
	}
}

void cUObject::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	QString Value = this->getNodeValue( Tag );

	// <tag type="string"> also type="value"
	//	    <key>multisection</key>
	//		<value>smallboat</value>
	// </tag>
	if( TagName == "tag" )
	{
		QString tkey = (char*)0;
		QString tvalue = (char*)0;
		QDomNode childNode = Tag.firstChild();
		while( !childNode.isNull() )
		{
			if( childNode.isElement() )
			{
				QDomElement childTag = childNode.toElement();
				QString childValue = this->getNodeValue( childTag );
				QString childName = childNode.nodeName();
				
				if( childName == "key" )
					tkey = childValue;	

				else if( childName == "value" )
					tvalue = childValue;
			}
			childNode = childNode.nextSibling();
		}

		if( !tkey.isNull() && !tvalue.isNull() )
		{
			if( Tag.attribute( "type" ) == "value" )
				this->tags.set( tkey, cVariant( tvalue.toInt() ) );
			else
				this->tags.set( tkey, cVariant( tvalue ) );
		}
	}
}

// Remove it from all in-range sockets
void cUObject::removeFromView( bool clean )
{
	// Get Real pos
	Coord_cl mPos = pos;

	if( isItemSerial( serial ) )
	{
		P_ITEM pCont = GetOutmostCont( (P_ITEM)this, 64 );
		if( pCont )
		{
			mPos = pCont->pos;
			P_CHAR pOwner = FindCharBySerial( pCont->contserial );
			if( pOwner )
				mPos = pOwner->pos;
		}
	}

	for( cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next() )
		if( clean || ( socket->player() && ( socket->player()->pos.distance( mPos ) <= socket->player()->VisRange() ) ) )
			socket->removeObject( this );
}

// Checks if the specified object is in range
bool cUObject::inRange( cUObject *object, UINT32 range )
{
	if( !object ) 
		return false;

	return ( pos.distance( object->pos ) <= range );
}


void cUObject::load( QSqlQuery *result, UINT16 &offset )
{
	name	= result->value( offset++ ).toString();
	serial	= result->value( offset++ ).toInt();
	multis	= result->value( offset++ ).toInt();
	pos.x	= result->value( offset++ ).toInt();
	pos.y	= result->value( offset++ ).toInt();
	pos.z	= result->value( offset++ ).toInt();
	pos.map = result->value( offset++ ).toInt();
	QString events = result->value( offset++ ).toString();
	eventList_	= QStringList::split( ",", events );
	bindmenu_	= result->value( offset++ ).toString();
}

void cUObject::buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions )
{
	fields.push_back( "uobject.name,uobject.serial,uobject.multis,uobject.pos_x,uobject.pos_y,uobject.pos_z,uobject.pos_map,uobject.events,uobject.bindmenu" );
	tables.push_back( "uobject" );
	conditions.push_back( "uobjectmap.serial = uobject.serial" );
}
