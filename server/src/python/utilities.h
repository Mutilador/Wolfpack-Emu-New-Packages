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

#if !defined( __UTILITIES_H__ )
#define __UTILITIES_H__

#include "engine.h"
#include "pyerrors.h"
#include <qstring.h>

#include "../typedefs.h"

class cUOSocket;
class cItem;
class cBaseChar;
class Coord;
class cAccount;
class cTerritory;
class cUOTxTooltipList;
class cMulti;
class AbstractAI;

typedef cItem* P_ITEM;
typedef cMulti* P_MULTI;

/*!
	Things commonly used in other python-definition
	source-files.
*/
inline PyObject* PyFalse()
{
	Py_INCREF( Py_False );
	return Py_False;
}

inline PyObject* PyTrue()
{
	Py_INCREF( Py_True );
	return Py_True;
}

#define PyHasMethod(a) if( codeModule == NULL ) return false; if( !PyObject_HasAttrString( codeModule, a ) ) return false;

void wpDealloc( PyObject* self );

PyObject* PyGetTooltipObject( cUOTxTooltipList* );

int PyConvertObject( PyObject* object, cUObject** uobject );

bool checkWpSocket( PyObject* object );
PyObject* PyGetSocketObject( cUOSocket* );
cUOSocket* getWpSocket( PyObject* object );
int PyConvertSocket( PyObject* object, cUOSocket** sock );

bool checkWpCoord( PyObject* object );
int PyConvertCoord( PyObject* object, Coord* pos );
PyObject* PyGetCoordObject( const Coord& coord );
Coord getWpCoord( PyObject* object );

bool checkWpItem( PyObject* object );
PyObject* PyGetItemObject( P_ITEM );
P_ITEM getWpItem( PyObject* );
int PyConvertItem( PyObject*, P_ITEM* item );

PyObject* PyGetObjectObject( cUObject* );

bool checkWpChar( PyObject* object );
PyObject* PyGetCharObject( P_CHAR );
P_CHAR getWpChar( PyObject* );
int PyConvertChar( PyObject* object, P_CHAR* character );
int PyConvertPlayer( PyObject* object, P_PLAYER* player );

bool checkWpAccount( PyObject* object );
PyObject* PyGetAccountObject( cAccount* );
cAccount* getWpAccount( PyObject* );

bool checkWpRegion( PyObject* object );
PyObject* PyGetRegionObject( cTerritory* );
cTerritory* getWpRegion( PyObject* );

bool checkWpMulti( PyObject* object );
PyObject* PyGetMultiObject( P_MULTI );
P_MULTI getWpMulti( PyObject* );

bool checkWpAI( PyObject* object );
PyObject* PyGetAIObject( AbstractAI* );
AbstractAI* getWpAI( PyObject* );

// Argument checks
#define checkArgObject( id ) ( PyTuple_Size( args ) > id && ( checkWpItem( PyTuple_GetItem( args, id ) ) || checkWpChar( PyTuple_GetItem( args, id ) ) ) )
#define checkArgChar( id ) ( PyTuple_Size( args ) > id && checkWpChar( PyTuple_GetItem( args, id ) ) )
#define checkArgItem( id ) ( PyTuple_Size( args ) > id && checkWpItem( PyTuple_GetItem( args, id ) ) )
#define checkArgCoord( id ) ( PyTuple_Size( args ) > id && checkWpCoord( PyTuple_GetItem( args, id ) ) )
#define getArgCoord( id ) getWpCoord( PyTuple_GetItem( args, id ) )
#define getArgItem( id ) getWpItem( PyTuple_GetItem( args, id ) )
#define getArgChar( id ) getWpChar( PyTuple_GetItem( args, id ) )
#define checkArgInt( id ) ( PyTuple_Size( args ) > id && PyInt_Check( PyTuple_GetItem( args, id ) ) )
#define getArgInt( id ) PyInt_AsLong( PyTuple_GetItem( args, id ) )
#define checkArgStr( id ) ( PyTuple_Size( args ) > id && ( PyString_Check( PyTuple_GetItem( args, id ) ) || PyUnicode_Check( PyTuple_GetItem( args, id ) ) ) )
#define checkArgUnicode( id ) ( PyTuple_Size( args ) > id && PyUnicode_Check( PyTuple_GetItem( args, id ) ) )
#define getArgStr( id ) (Python2QString(PyTuple_GetItem(args,id)))
#define getArgUnicode( id ) Python2QString( PyTuple_GetItem( args, id ) )
#define getUnicodeSize( id ) PyUnicode_GetSize( PyTuple_GetItem( args, id ) )
#define checkArgAccount( id ) ( PyTuple_Size( args ) > id && checkWpAccount( PyTuple_GetItem( args, id ) ) )
#define checkArgRegion( id ) ( PyTuple_Size( args ) > id && checkWpRegion( PyTuple_GetItem( args, id ) ) )
#define getArgRegion( id ) getWpRegion( PyTuple_GetItem( args, id ) )
#define getArgAccount( id ) getWpAccount( PyTuple_GetItem( args, id ) )
#define checkArgMulti( id ) ( PyTuple_Size( args ) > id && checkWpMulti( PyTuple_GetItem( args, id ) ) )
#define getArgMulti( id ) getWpMulti( PyTuple_GetItem( args, id ) )

inline PyObject* QString2Python( const QString& string )
{
	if ( string.isEmpty() )
	{
		return PyUnicode_FromWideChar( L"", 0 );
	}
	else
	{
#if defined(Py_UNICODE_WIDE)
		QCString utf = string.utf8();
		PyObject *obj = PyUnicode_DecodeUTF8( utf.data(), utf.length(), "" );
		return obj;
#else
		return PyUnicode_FromUnicode( ( Py_UNICODE * ) string.ucs2(), string.length() );
#endif
	}
}

inline QString Python2QString( PyObject* object )
{
	if ( !object )
	{
		return QString::null;
	}
	else if ( PyUnicode_Check( object ) )
	{
#if defined(Py_UNICODE_WIDE)
		return QString::fromUtf8( PyString_AsString( PyUnicode_AsUTF8String( object ) ) );
#else
		return QString::fromUcs2( ( ushort * ) PyUnicode_AS_UNICODE( object ) );
#endif
	}
	else if ( PyString_Check( object ) )
	{
		return QString::fromLocal8Bit( PyString_AsString( object ) );
	}
	else if ( PyInt_Check( object ) )
	{
		return QString::number( PyInt_AsLong( object ) );
	}
	else if ( PyFloat_Check( object ) )
	{
		return QString::number( PyFloat_AsDouble( object ) );
	}
	else
	{
		return QString::null;
	}
}

class PythonFunction
{
	PyObject* pModule;
	PyObject* pFunc;
	QString sModule;
public:
	Q_EXPLICIT PythonFunction( PyObject* function ) : pModule( 0 ), pFunc( 0 )
	{
		pFunc = function;
		Py_XINCREF( pFunc );
	}

	Q_EXPLICIT PythonFunction( const QString& path ) : pModule( 0 ), pFunc( 0 )
	{
		int position = path.findRev( "." );
		sModule = path.left( position );
		QString sFunction = path.right( path.length() - ( position + 1 ) );

		// The Python string functions don't like null pointers
		if (sModule.latin1()) {
			pModule = PyImport_ImportModule( const_cast<char*>( sModule.latin1() ) );
	
			if ( pModule && sFunction.latin1() )
			{
				pFunc = PyObject_GetAttrString( pModule, const_cast<char*>( sFunction.latin1() ) );
				if ( pFunc && !PyCallable_Check( pFunc ) )
				{
					cleanUp();
				}
			}
		}

	}

	~PythonFunction()
	{
		cleanUp();
	}

	PyObject* function() const
	{
		Py_XINCREF( pFunc );
		return pFunc;
	}

	QString functionPath() const
	{
		if (isValid()) {
			PyObject* module = PyObject_GetAttrString(pFunc, "__module__");
			PyObject* name = PyObject_GetAttrString(pFunc, "__name__");
			QString result = Python2QString(module) + "." + Python2QString(name);
			Py_XDECREF( name );
			Py_XDECREF( module );
			return result;
		} else {
			return QString::null;
		}
	}

	void cleanUp()
	{
		Py_XDECREF( pFunc );
		Py_XDECREF( pModule );
		pFunc = 0;
		pModule = 0;
	}

	bool isValid() const { return pFunc != 0 && PyCallable_Check( pFunc );	}

	PyObject* operator()( PyObject* args )
	{
		PyObject* result = 0;
		Py_XINCREF( args );
		if ( isValid() )
			result = PyEval_CallObject( pFunc, args );
		reportPythonError( sModule );
		Py_XDECREF( args );
		return result;
	}
};

#endif
