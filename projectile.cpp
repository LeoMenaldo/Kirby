#include "projectile.h"
#include <QPainter>

Projectile::Projectile(bool movingRight) {
    // 动态绘制一个黄色的光球（如果你有图片，这里可以换成加载QPixmap）
    QPixmap pix(24, 24);
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    painter.setBrush(Qt::yellow);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, 24, 24);
    setPixmap(pix);

    // 设置速度，向右为正，向左为负
    vx = movingRight ? 12 : -12;
}

void Projectile::updateLogic() {
    lifeTime-=2; // 寿命减少
    setPos(x() + vx, y() + vy); // 水平移动
}