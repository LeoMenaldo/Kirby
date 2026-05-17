#include "player.h"
#include <QTransform>

Player::Player() {
    int frameSize = 48;

    // 待机
    QPixmap daijiSheet(":/tu/kirby_daiji.png");
    int daijiCount = daijiSheet.width() / frameSize;
    for (int i = 0; i < daijiCount; i++) {
        idleFrames.push_back(daijiSheet.copy(i * frameSize, 0, frameSize, frameSize));
    }

    // 走路
    QPixmap walkSheet(":/tu/kirby_walk.png");
    int walkCount = walkSheet.width() / frameSize;
    for (int i = 0; i < walkCount; i++) {
        walkFrames.push_back(walkSheet.copy(i * frameSize, 0, frameSize, frameSize));
    }

    // 飞行/跳跃
    QPixmap flySheet(":/tu/kirby_fly.png");
    int flyCount = flySheet.width() / frameSize;
    for (int i = 0; i < flyCount; i++) {
        jumpFrames.push_back(flySheet.copy(i * frameSize, 0, frameSize, frameSize));
    }

    // 翻滚（新增）
    QPixmap rollSheet(":/tu/kirby_fangun.png");
    int rollCount = rollSheet.width() / frameSize;
    for (int i = 0; i < rollCount; i++) {
        rollFrames.push_back(rollSheet.copy(i * frameSize, 0, frameSize, frameSize));
    }

    // 攻击（新增）
    QPixmap attackSheet(":/tu/normal-attack.png");
    // 根据你提供的图，看起来是7帧，我们根据宽度均分
    int attackCount = 7;
    int attackFrameWidth = attackSheet.width() / attackCount;
    for (int i = 0; i < attackCount; i++) {
        attackFrames.push_back(attackSheet.copy(i * attackFrameWidth, 0, attackFrameWidth, attackSheet.height()));
    }

    if (!idleFrames.isEmpty()) {
        setPixmap(idleFrames[0]);
    }
    // 吞噬动画加载 (72x72, 共5帧)
    QPixmap swallowSheet(":/tu/kirby_swallow.png");
    for (int i = 0; i < 1; i++) {
        swallowFrames.push_back(swallowSheet.copy(i * 72, 0, 72, 72));
    }
    for (int i = 0; i < 5; i++) {
        swallowFrames.push_back(swallowSheet.copy(i * 72, 0, 72, 72));
    }

    // Fatty 待机动画加载 (自适应高度切片)
    QPixmap fattyDaijiSheet(":/tu/fatty_daiji.png");
    if (!fattyDaijiSheet.isNull()) {
        int fh = fattyDaijiSheet.height();
        int count = fattyDaijiSheet.width() / fh;
        for (int i = 0; i < count; i++) {
            fattyIdleFrames.push_back(fattyDaijiSheet.copy(i * fh, 0, fh, fh));
        }
    }

    // Fatty 走路动画加载 (自适应高度切片)
    QPixmap fattyWalkSheet(":/tu/fatty_walk.png");
    if (!fattyWalkSheet.isNull()) {
        int fh = fattyWalkSheet.height();
        int count = fattyWalkSheet.width() / fh;
        for (int i = 0; i < count; i++) {
            fattyWalkFrames.push_back(fattyWalkSheet.copy(i * fh, 0, fh, fh));
        }
    }

    // 吐出动画加载 (自适应高度切片，自动识别网格尺寸)
    QPixmap tuchuSheet(":/tu/kirby_tuchu.png");
    if (!tuchuSheet.isNull()) {
        int fh = tuchuSheet.height();
        int count = tuchuSheet.width() / fh;
        for (int i = 0; i < count; i++) {
            spitFrames.push_back(tuchuSheet.copy(i * fh, 0, fh, fh));
        }
    }
    // 消化/变身动画加载 (自适应高度)
    QPixmap digestSheet(":/tu/kirby_digest.png");
    if (!digestSheet.isNull()) {
        int fh = digestSheet.height();
        int count = digestSheet.width() / fh;
        for (int i = 0; i < count; i++) {
            digestFrames.push_back(digestSheet.copy(i * fh, 0, fh, fh));
        }
    }

    // 火形态：待机动画加载 (自适应高度)
    QPixmap fireDaijiSheet(":/tu/fire_daiji.png");
    if (!fireDaijiSheet.isNull()) {
        int fh = fireDaijiSheet.height();
        int count = fireDaijiSheet.width() / fh;
        for (int i = 0; i < count; i++) {
            fireIdleFrames.push_back(fireDaijiSheet.copy(i * fh, 0, fh, fh));
        }
    }

    // 火形态：走路动画加载 (自适应高度)
    QPixmap fireWalkSheet(":/tu/fire_walk.png");
    if (!fireWalkSheet.isNull()) {
        int fh = fireWalkSheet.height();
        int count = fireWalkSheet.width() / fh;
        for (int i = 0; i < count; i++) {
            fireWalkFrames.push_back(fireWalkSheet.copy(i * fh, 0, fh, fh));
        }
    }

    // 火形态：飞行/跳跃动画加载 (自适应高度)
    QPixmap fireFlySheet(":/tu/fire_fly.png");
    if (!fireFlySheet.isNull()) {
        int fh = fireFlySheet.height();
        int count = fireFlySheet.width() / fh;
        for (int i = 0; i < count; i++) {
            fireJumpFrames.push_back(fireFlySheet.copy(i * fh, 0, fh, fh));
        }
    }
    // 火形态：翻滚动画加载 (自适应高度)
    QPixmap fireRollSheet(":/tu/fire_fangun.png");
    if (!fireRollSheet.isNull()) {
        int fh = fireRollSheet.height();
        int count = fireRollSheet.width() / fh;
        for (int i = 0; i < count; i++) {
            fireRollFrames.push_back(fireRollSheet.copy(i * fh, 0, fh, fh));
        }
    }
}

void Player::setState(State newState) {
    if (currentState != newState) {
        currentState = newState;
        currentFrame = 0;
        animTimer = 0;
    }
}

void Player::startRoll() {
    if (isRolling) return;

    // 动态获取当前形态对应的翻滚总帧数
    int frameCount = (currentForm == Enemy::FIRE && !fireRollFrames.isEmpty()) ? fireRollFrames.size() : rollFrames.size();
    if (frameCount == 0) return;

    isRolling = true;
    setState(ROLLING);
    rollTimer = frameCount * 2; // 根据形态的帧数动态决定翻滚持续时长
    rollCurrentFrame = 0;
    rollAnimTimer = 0;

    // 绘制第一帧
    QPixmap img = (currentForm == Enemy::FIRE && !fireRollFrames.isEmpty()) ? fireRollFrames[0] : rollFrames[0];
    if (!facingRight) img = img.transformed(QTransform().scale(-1, 1));
    setOffset((48 - img.width()) / 2.0, 48 - img.height());
    setPixmap(img);
}

void Player::endRoll() {
    isRolling = false;
    // 恢复到地面待机或空中状态（交由下次updateLogic判断）
    setState(isOnGround ? IDLE : JUMPING);
}

void Player::updateLogic() {
    // 受击无敌时间计时与闪烁效果
    if (invulnTimer > 0) {
        invulnTimer--;
        if (invulnTimer % 4 < 2) setVisible(false);
        else setVisible(true);
    } else {
        setVisible(true);
    }

    // ====== 新增：变身消化状态拦截 ======
    if (isDigesting) {
        if (isOnGround) vx = 0;
        animTimer++;
        if (animTimer >= 5) { // 变身抖动速度
            animTimer = 0;
            currentFrame++;

            // 消化动画播放完毕
            if (currentFrame >= digestFrames.size()) {
                isDigesting = false;
                currentForm = swallowedAbility; // 核心：正式继承肚子里的怪物能力！
                swallowedAbility = Enemy::NONE; // 消化干净，清空胃部
                setState(isOnGround ? IDLE : JUMPING);
                return;
            }

            if (!digestFrames.isEmpty()) {
                QPixmap img = digestFrames[currentFrame];
                if (!facingRight) img = img.transformed(QTransform().scale(-1, 1));
                setOffset((48 - img.width()) / 2.0, 48 - img.height());
                setPixmap(img);
            }
        }
        return;
    }

    // 吐出状态拦截
    if (isSpitting) {
        if (isOnGround) vx = 0;
        animTimer++;
        if (animTimer >= 3) {
            animTimer = 0;
            currentFrame++;
            if (currentFrame == 1) triggerSpitStar = true;
            if (currentFrame >= spitFrames.size()) {
                isSpitting = false;
                setState(isOnGround ? IDLE : JUMPING);
                return;
            }
            if (!spitFrames.isEmpty()) {
                QPixmap img = spitFrames[currentFrame];
                if (!facingRight) img = img.transformed(QTransform().scale(-1, 1));
                setOffset((48 - img.width()) / 2.0, 48 - img.height());
                setPixmap(img);
            }
        }
        return;
    }

    // 吞噬状态拦截
    if (isSwallowing) {
        if (isOnGround) vx = 0;
        animTimer++;
        if (animTimer >= 5) {
            animTimer = 0;
            if (!swallowFirstPassDone) {
                currentFrame++;
                if (currentFrame >= 5) {
                    swallowFirstPassDone = true;
                    currentFrame = 3;
                }
            } else {
                currentFrame++;
                if (currentFrame > 4) currentFrame = 3;
            }
            if (!swallowFrames.isEmpty()) {
                QPixmap img = swallowFrames[currentFrame];
                if (!facingRight) img = img.transformed(QTransform().scale(-1, 1));
                setOffset((48 - img.width()) / 2.0, 48 - img.height());
                setPixmap(img);
            }
        }
        return;
    }

    // Fatty 状态拦截
    if (isFatty) {
        State targetState = (qAbs(vx) < 0.1) ? FATTY_IDLE : FATTY_WALKING;
        if (targetState != currentState) {
            currentState = targetState;
            currentFrame = 0;
            animTimer = 0;
        }
        animTimer++;
        if (animTimer >= 6) {
            animTimer = 0;
            currentFrame++;
            QPixmap img;
            if (currentState == FATTY_IDLE) {
                if (!fattyIdleFrames.isEmpty()) {
                    if (currentFrame >= fattyIdleFrames.size()) currentFrame = 0;
                    img = fattyIdleFrames[currentFrame];
                }
            } else {
                if (!fattyWalkFrames.isEmpty()) {
                    if (currentFrame >= fattyWalkFrames.size()) currentFrame = 0;
                    img = fattyWalkFrames[currentFrame];
                }
            }
            if (!facingRight && !img.isNull())
                img = img.transformed(QTransform().scale(-1, 1));
            if (!img.isNull()) {
                setOffset((48 - img.width()) / 2.0, 48 - img.height());
                setPixmap(img);
            }
        }
        return;
    }

    // 攻击中
    if (isAttacking) {
        if (isOnGround) vx = 0;
        attackAnimTimer++;
        if (attackAnimTimer >= 3) {
            attackAnimTimer = 0;
            attackCurrentFrame++;
            if (attackCurrentFrame >= attackFrames.size()) {
                endAttack();
                return;
            }
            QPixmap img = attackFrames[attackCurrentFrame];
            if (!facingRight) img = img.transformed(QTransform().scale(-1, 1));
            setOffset((48 - img.width()) / 2.0, 48 - img.height());
            setPixmap(img);
        }
        return;
    }

    // ====== 修改：翻滚中（支持形态动态切换） ======
    if (isRolling) {
        rollAnimTimer++;
        if (rollAnimTimer >= 2) {
            rollAnimTimer = 0;
            rollCurrentFrame++;

            // 动态判断当前形态的帧数上限
            int frameCount = (currentForm == Enemy::FIRE && !fireRollFrames.isEmpty()) ? fireRollFrames.size() : rollFrames.size();

            if (rollCurrentFrame >= frameCount) {
                rollCurrentFrame = frameCount - 1; // 停留在最后一帧
            }

            // 根据形态选取对应帧
            QPixmap img = (currentForm == Enemy::FIRE && !fireRollFrames.isEmpty()) ? fireRollFrames[rollCurrentFrame] : rollFrames[rollCurrentFrame];
            if (!facingRight) img = img.transformed(QTransform().scale(-1, 1));
            setOffset((48 - img.width()) / 2.0, 48 - img.height());
            setPixmap(img);
        }
        return;
    }

    // 常规物理运动状态切换
    State targetState;
    if (!isOnGround) {
        targetState = JUMPING;
    } else {
        if (qAbs(vx) < 0.1) targetState = IDLE;
        else targetState = WALKING;
    }

    if (targetState != currentState) {
        currentState = targetState;
        currentFrame = 0;
    }

    animTimer++;
    if (animTimer >= 6) {
        animTimer = 0;
        currentFrame++;

        QPixmap currentImage;
        switch (currentState) {
        case JUMPING:
            // ====== 核心修改点：预留形态分支逻辑 ======
            if (currentForm == Enemy::FIRE) {
                if (!fireJumpFrames.isEmpty()) {
                    if (currentFrame >= fireJumpFrames.size()) currentFrame = 0;
                    currentImage = fireJumpFrames[currentFrame];
                }
            } else {
                // 原有的普通形态飞行
                if (!jumpFrames.isEmpty()) {
                    if (currentFrame <= 6) currentFrame = 6;
                    if (currentFrame >= 15) currentFrame = 6;
                    int safeFrame = currentFrame;
                    if (safeFrame >= jumpFrames.size()) safeFrame = jumpFrames.size() - 1;
                    currentImage = jumpFrames[safeFrame];
                }
            }
            break;
        case WALKING:
            if (currentForm == Enemy::FIRE) {
                if (!fireWalkFrames.isEmpty()) {
                    if (currentFrame >= fireWalkFrames.size()) currentFrame = 0;
                    currentImage = fireWalkFrames[currentFrame];
                }
            } else {
                // 原有的普通走路
                if (!walkFrames.isEmpty()) {
                    if (currentFrame >= walkFrames.size()) currentFrame = 0;
                    currentImage = walkFrames[currentFrame];
                }
            }
            break;
        case IDLE:
        default:
            if (currentForm == Enemy::FIRE) {
                if (!fireIdleFrames.isEmpty()) {
                    if (currentFrame >= fireIdleFrames.size()) currentFrame = 0;
                    currentImage = fireIdleFrames[currentFrame];
                }
            } else {
                // 原有的普通待机
                if (!idleFrames.isEmpty()) {
                    if (currentFrame >= idleFrames.size()) currentFrame = 0;
                    currentImage = idleFrames[currentFrame];
                }
            }
            break;
        }

        if (!facingRight && !currentImage.isNull())
            currentImage = currentImage.transformed(QTransform().scale(-1, 1));

        if (!currentImage.isNull()) {
            setOffset((48 - currentImage.width()) / 2.0, 48 - currentImage.height());
            setPixmap(currentImage);
        }
    }
}
QPainterPath Player::shape() const {
    QPainterPath path;
    path.addRect(0, 0, 48, 48);
    return path;
}
void Player::startAttack() {
    if (isAttacking || isRolling || attackFrames.isEmpty()) return; // 翻滚时不能攻击

    isAttacking = true;
    setState(ATTACKING);
    attackCurrentFrame = 0;
    attackAnimTimer = 0;

    // 播放第一帧
    QPixmap img = attackFrames[0];
    if (!facingRight) img = img.transformed(QTransform().scale(-1, 1));
    setOffset((48 - img.width()) / 2.0, 48 - img.height());
    setPixmap(img);
}

void Player::endAttack() {
    isAttacking = false;
    setState(isOnGround ? IDLE : JUMPING);
}
void Player::startSwallow() {
    // ====== 修改：增加 currentForm != Enemy::NONE 拦截 ======
    // 只有在初始状态（NONE）时，才能张嘴吞噬。拥有任何能力时绝对禁止吞噬。
    if (isSwallowing || isRolling || isAttacking || isFatty || isDigesting || isSpitting || !isOnGround || currentForm != Enemy::NONE) return;
    isSwallowing = true;
    swallowFirstPassDone = false;
    setState(SWALLOWING);
    currentFrame = 0;
    animTimer = 0;
}
void Player::endSwallow() {
    if (!isSwallowing) return;
    isSwallowing = false;
    setState(isFatty ? FATTY_IDLE : (isOnGround ? IDLE : JUMPING));
}
void Player::startSpit() {
    if (isSpitting) return;
    isSpitting = true;
    isFatty = false;          // 瞬间解除肥胖状态
    triggerSpitStar = false;  // 重置子弹发射旗帜
    setState(SPITTING);
    currentFrame = 0;
    animTimer = 0;
}
void Player::startDigest() {
    if (isDigesting || !isFatty) return;
    isDigesting = true;
    isFatty = false; // 瞬间解除肥胖视觉，由消化动作接管画面
    setState(DIGESTING);
    currentFrame = 0;
    animTimer = 0;
}