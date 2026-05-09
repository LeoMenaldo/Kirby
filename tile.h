#ifndef TILE_H
#define TILE_H

#include "gameobject.h"

class Tile : public GameObject {
public:
    enum TileType { Grass, Dirt, WaterSurface, WaterBody };

    Tile(TileType type, const QPixmap &pixmap);

    TileType tileType() const { return m_type; }
    void updateLogic() override;

private:
    TileType m_type;
};

#endif // TILE_H