#ifndef PLAYER_H
#define PLAYER_H
#include "gameobject.h"
#include <QPixmap>
#include <QVector>
#include <QPainterPath>

class Player : public GameObject {
public:
    enum State { IDLE, WALKING, JUMPING };

    Player();
    void updateLogic() override;
    void setState(State newState);

    QPainterPath shape() const override;   // 强制固定碰撞箱

    State currentState;
    bool isOnGround = false;
    bool facingRight = true;
    bool isHovering = false;
    bool canDoubleJump = false;
    // vx 和 vy 已继承自 GameObject，此处不再重复声明

private:
    int currentFrame = 0;
    int animTimer = 0;
    QVector<QPixmap> idleFrames;
    QVector<QPixmap> walkFrames;
    QVector<QPixmap> jumpFrames;
};

#endif // PLAYER_H