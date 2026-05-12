#ifndef PLAYER_H
#define PLAYER_H
#include "gameobject.h"
#include <QPixmap>
#include <QVector>
#include <QPainterPath>

class Player : public GameObject {
public:
    enum State { IDLE, WALKING, JUMPING, ROLLING };  // 新增 ROLLING

    static constexpr double rollSpeed = 8;         // 翻滚时的水平速度

    Player();
    void updateLogic() override;
    void setState(State newState);
    void startRoll();                                // 开始翻滚
    void endRoll();                                  // 结束翻滚

    QPainterPath shape() const override;

    State currentState;
    bool isOnGround = false;
    bool facingRight = true;
    bool isHovering = false;
    bool canDoubleJump = false;
    bool isRolling = false;                          // 是否正在翻滚
    int rollTimer = 0;                               // 翻滚剩余计时

private:
    int currentFrame = 0;
    int animTimer = 0;
    QVector<QPixmap> idleFrames;
    QVector<QPixmap> walkFrames;
    QVector<QPixmap> jumpFrames;
    QVector<QPixmap> rollFrames;                     // 翻滚动画帧

    int rollCurrentFrame = 0;                        // 翻滚动画当前帧索引
    int rollAnimTimer = 0;                           // 翻滚动画计时
};

#endif // PLAYER_H