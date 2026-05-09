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

void Player::updateLogic() {
    // 1. 状态切换（不再负责重力/位移）
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

    // 2. 动画播放
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