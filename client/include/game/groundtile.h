
#if !defined(__GROUNDTILE_H__)
#define __GROUNDTILE_H__

#include "game/entity.h"
#include "muls/tiledata.h"
#include "vector.h"
#include "texture.h"

class cGroundTile : public cEntity {
protected:
	unsigned short id_;
	cLandTileInfo *tiledata_;
	cTexture *texture;
	short left;
	short bottom;
	short right;
	bool stretched;
	bool updated; // Are the normals calculated?
	signed char averagez_;
	
	// clockwise, starting at the top
	cVector normals[4];
public:
	cGroundTile(unsigned short x, unsigned short y, signed char z, enFacet facet);
	virtual ~cGroundTile();

	void updatePriority();

	signed char averagez() const;
	void setAveragez(signed char data);
	unsigned short id() const;
	void setId(unsigned short data);
	cLandTileInfo *tiledata() const;

	// Set the z information of the surrounding tiles for stretch informations
	void updateSurroundings();

	// Override the draw method
	void draw(int cellx, int celly, int leftClip, int topClip, int rightClip, int bottomClip);

	// Do a sophisticated hittest on the internal surface
	bool hitTest(int x, int y);

	void onClick();
	void onRightClick();
};

inline void cGroundTile::setAveragez(signed char data) {
	averagez_ = data;
}

inline signed char cGroundTile::averagez() const {
	return averagez_;
}

inline unsigned short cGroundTile::id() const {
	return id_;
}

inline cLandTileInfo *cGroundTile::tiledata() const {
	return tiledata_;
}

#endif
