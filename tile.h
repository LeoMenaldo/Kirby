#ifndef TILE_H
#define TILE_H

#include "gameobject.h"

class Tile : public GameObject {
public:
    // 新增 IceBlock 和 RubbleBlock
    enum TileType { Grass, Dirt, WaterSurface, WaterBody, IceBlock, RubbleBlock };

    Tile(TileType type, const QPixmap &pixmap);

    TileType tileType() const { return m_type; }
    void updateLogic() override;

    // 新增方法：用于在运行时改变方块类型和贴图
    void changeType(TileType newType, const QPixmap &newPix);

private:
    TileType m_type;
};
#endif // TILE_H