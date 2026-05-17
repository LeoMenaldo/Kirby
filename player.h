#ifndef PLAYER_H
#define PLAYER_H
#include "gameobject.h"
#include <QPixmap>
#include <QVector>
#include <QPainterPath>
#include "enemy.h"

class Player : public GameObject {
public:
    enum State { IDLE, WALKING, JUMPING, ROLLING, ATTACKING, SWALLOWING, FATTY_IDLE, FATTY_WALKING, SPITTING, DIGESTING };

    static constexpr double rollSpeed = 8;         // 翻滚时的水平速度

    Player();
    int hp = 3;           // 初始3点生命值
    int invulnTimer = 0;  // 受击无敌时间计时器
    void updateLogic() override;
    void setState(State newState);
    void startSpit();                 // 开始吐出动作
    bool isSpitting = false;          // 是否正在播放吐出动画
    bool triggerSpitStar = false;     // 触发主窗口发射星星的信号旗
    void startRoll();                                // 开始翻滚
    void endRoll();                                  // 结束翻滚
    void startAttack(); // 开始攻击
    void endAttack();   // 结束攻击
    bool isAttacking = false; // 是否正在攻击
    QPainterPath shape() const override;
    Enemy::CopyAbility swallowedAbility = Enemy::NONE;
    State currentState;
    bool isOnGround = false;
    bool facingRight = true;
    bool isHovering = false;
    bool canDoubleJump = false;
    bool isRolling = false;                          // 是否正在翻滚
    int rollTimer = 0;                               // 翻滚剩余计时
    void startSwallow();
    void endSwallow();
    bool isSwallowing = false;  // 是否正在吞噬
    bool isFatty = false;       // 是否处于 Fatty 状态
    // ====== 新增：蛋糕带来的攻击能力计时器 ======
    int attackPowerTimer = 0; // 剩余攻击能力的帧数
    bool hasAttackPower() const { return attackPowerTimer > 0; }
    void startDigest();                               // 开始消化能力
    bool isDigesting = false;                         // 是否正在播放消化变身动画
    Enemy::CopyAbility currentForm = Enemy::NONE;     // 核心：当前卡比持有的形态能力（预留后续扩展空间）

private:
    int currentFrame = 0;
    int animTimer = 0;
    QVector<QPixmap> idleFrames;
    QVector<QPixmap> walkFrames;
    QVector<QPixmap> jumpFrames;
    QVector<QPixmap> rollFrames;
    QVector<QPixmap> spitFrames;      // 吐出动画帧容器    // 翻滚动画帧
    QVector<QPixmap> attackFrames; // 攻击动画帧
    int attackCurrentFrame = 0;
    int attackAnimTimer = 0;
    int rollCurrentFrame = 0;                        // 翻滚动画当前帧索引
    int rollAnimTimer = 0;                           // 翻滚动画计时
    QVector<QPixmap> swallowFrames;
    QVector<QPixmap> fattyIdleFrames;
    QVector<QPixmap> fattyWalkFrames;
    bool swallowFirstPassDone = false; // 吞噬第一遍是否播放完
    QVector<QPixmap> digestFrames;    // 消化变身动画
    QVector<QPixmap> fireIdleFrames;  // 火形态待机
    QVector<QPixmap> fireWalkFrames;  // 火形态走路
    QVector<QPixmap> fireJumpFrames;  // 火形态飞行跳跃
    QVector<QPixmap> fireRollFrames;
};

#endif // PLAYER_H