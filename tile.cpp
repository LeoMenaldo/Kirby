#include "tile.h"

Tile::Tile(TileType type, const QPixmap &pixmap)
    : m_type(type)
{
    setPixmap(pixmap);
    vx = 0;
    vy = 0;
}

void Tile::updateLogic() {
    // 静态方块暂无逻辑，可留空
}