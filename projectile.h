#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "gameobject.h"

class Projectile : public GameObject {
public:
    Projectile(bool movingRight);
    void updateLogic() override;

    int damage = 1;      // 攻击力
    int lifeTime = 60;   // 存活时间（帧数），大概跑1秒左右消失
};

#endif // PROJECTILE_H