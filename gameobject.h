#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <QGraphicsPixmapItem>

class GameObject : public QGraphicsPixmapItem {
public:
    GameObject();

    double vx = 0;  // 水平速度
    double vy = 0;  // 垂直速度

    virtual void updateLogic() = 0;
};

#endif // GAMEOBJECT_H