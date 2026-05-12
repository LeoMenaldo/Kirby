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

    if (!idleFrames.isEmpty()) {
        setPixmap(idleFrames[0]);
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
    if (isRolling || rollFrames.isEmpty()) return;

    isRolling = true;
    setState(ROLLING);                         // 切换到翻滚状态
    rollTimer = rollFrames.size()*2;
    rollCurrentFrame = 0;
    rollAnimTimer = 0;

    // 显示第一帧
    QPixmap img = rollFrames[0];
    if (!facingRight)
        img = img.transformed(QTransform().scale(-1, 1));
    setPixmap(img);
}

void Player::endRoll() {
    isRolling = false;
    // 恢复到地面待机或空中状态（交由下次updateLogic判断）
    setState(isOnGround ? IDLE : JUMPING);
}

void Player::updateLogic() {
    // 翻滚中：只更新翻滚动画，不处理常规状态/动画
    if (isRolling) {
        rollAnimTimer++;
        if (rollAnimTimer >= 2) {
            rollAnimTimer = 0;
            rollCurrentFrame++;
            if (rollCurrentFrame >= rollFrames.size()) {
                rollCurrentFrame = rollFrames.size() - 1; // 停在最后一帧
            }
            QPixmap img = rollFrames[rollCurrentFrame];
            if (!facingRight)
                img = img.transformed(QTransform().scale(-1, 1));
            setPixmap(img);
        }
        return; // 跳过原有状态切换和动画逻辑
    }

    // 原有逻辑（保持不变）
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
            if (!jumpFrames.isEmpty()) {
                if (currentFrame <= 6) currentFrame = 6;
                if (currentFrame >= 15) currentFrame = 6;
                int safeFrame = currentFrame;
                if (safeFrame >= jumpFrames.size()) safeFrame = jumpFrames.size() - 1;
                currentImage = jumpFrames[safeFrame];
            }
            break;
        case WALKING:
            if (!walkFrames.isEmpty()) {
                if (currentFrame >= walkFrames.size()) currentFrame = 0;
                currentImage = walkFrames[currentFrame];
            }
            break;
        case IDLE:
        default:
            if (!idleFrames.isEmpty()) {
                if (currentFrame >= idleFrames.size()) currentFrame = 0;
                currentImage = idleFrames[currentFrame];
            }
            break;
        }

        if (!facingRight && !currentImage.isNull())
            currentImage = currentImage.transformed(QTransform().scale(-1, 1));

        if (!currentImage.isNull())
            setPixmap(currentImage);
    }
}

QPainterPath Player::shape() const {
    QPainterPath path;
    path.addRect(0, 0, 48, 48);
    return path;
}