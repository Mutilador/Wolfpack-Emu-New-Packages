#if !defined( __SECTORS_H__ )
#define __SECTORS_H__

#include <map>
#include "singleton.h"

class Coord_cl;
class cUObject;
class cItem;
class cBaseChar;

// In reality these hold a list of items you can iterate trough
// So they *can* contain more than one sector!
class cSectorIterator
{
	friend class RegionIterator4Chars;
	friend class RegionIterator4Items;

protected:
	cUObject **items;
	unsigned int count;
	unsigned int pos;

public:
	cSectorIterator( unsigned int count, cUObject **items );
	virtual ~cSectorIterator();
};

class cItemSectorIterator: public cSectorIterator
{
public:
	cItemSectorIterator( unsigned int count, cUObject **items ): cSectorIterator( count, items ) {}

	cItem *first();
	cItem *next();
};

class cCharSectorIterator: public cSectorIterator
{	
public:
	cCharSectorIterator( unsigned int count, cUObject **items ): cSectorIterator( count, items ) {}

	cBaseChar *first();
	cBaseChar *next();
};

class cSectorMaps;

class cSectorMap
{
	friend class cSectorMaps;

protected:
	struct stSector
	{
		unsigned int count;
		cUObject **data;
	};
	
	stSector **grid;
	unsigned int gridWidth_;
	unsigned int gridHeight_;
	char *error_;

	bool addItem( unsigned short x, unsigned short y, cUObject *object ) throw();
	bool removeItem( unsigned short x, unsigned short y, cUObject *object ) throw();
	
	unsigned int countItems( unsigned int id ) throw(); // Returns the count of items in a specific block
	unsigned int getItems( unsigned int id, cUObject **items ) throw(); // Returns the count of items copied
public:
	cSectorMap();
	virtual ~cSectorMap();

	unsigned int gridHeight() const;
	unsigned int gridWidth() const;
	
	bool init( unsigned int width, unsigned int height ) throw();
	const char *error() const { return error_; }

	// Get an iterator for one specific block
	unsigned int calcBlockId( unsigned int x, unsigned int y ) throw();
};

// Manager Class for cSectorMap
class cSectorMaps
{
private:
	std::map< unsigned char, cSectorMap* > itemmaps;
	std::map< unsigned char, cSectorMap* > charmaps;

	void remove( const Coord_cl &pos, cItem *pItem );
	void remove( const Coord_cl &pos, cBaseChar *pChar );

	void add( const Coord_cl &pos, cItem *pItem );
	void add( const Coord_cl &pos, cBaseChar *pChar );

	// Internal functions for searching items
	enum MapType
	{
		MT_CHARS,
		MT_ITEMS
	};

	cSectorIterator *findObjects( MapType type, cSectorMap *map, unsigned int x, unsigned int y );
	cSectorIterator *findObjects( MapType type, cSectorMap *map, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2 );
public:
	cSectorMaps();
	virtual ~cSectorMaps();

	void addMap( unsigned char map, unsigned int width, unsigned int height );
	void add( cUObject *object );
	void remove( cUObject *object );

	// Find Methods
	// These methods help in finding items on the map, either in a specific block
	// or by a center coordinate and a distance parameter
	// or inside of a rectangle defined by x1,y1 and x2,y2
	cItemSectorIterator *findItems( unsigned char map, unsigned int x, unsigned int y ); // Find items in a specific block
	cItemSectorIterator *findItems( unsigned char map, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2 );
	cItemSectorIterator *findItems( const Coord_cl &center, unsigned char distance );

	cCharSectorIterator *findChars( unsigned char map, unsigned int x, unsigned int y ); // Find items in a specific block
	cCharSectorIterator *findChars( unsigned char map, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2 );
	cCharSectorIterator *findChars( const Coord_cl &center, unsigned char distance );
};

//typedef SingletonHolder< cSectorMaps > SectorMaps;
typedef SingletonHolder< cSectorMaps > SectorMaps;
#define MapObjects SectorMaps

// These are deprecated Replacement Classes, but they should help during the transition
class RegionIterator4Chars
{
private:
	cCharSectorIterator *iter;
public:
	RegionIterator4Chars( const Coord_cl &pos, unsigned int distance = 18 ); 
	virtual ~RegionIterator4Chars();

	void		Begin( void );
	bool		atEnd( void ) const;
	cBaseChar	*GetData( void );
	RegionIterator4Chars& operator++( int );
};

class RegionIterator4Items
{
private:
	cItemSectorIterator *iter;
public:
	RegionIterator4Items( const Coord_cl &pos, unsigned int distance = 18 );
	virtual ~RegionIterator4Items();

	void		Begin( void );
	bool		atEnd( void ) const;
	cItem		*GetData( void );

	// Operators
	RegionIterator4Items& operator++( int );
};

#endif
