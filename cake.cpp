#include "cake.h"

Cake::Cake() {
    // 加载蛋糕图片资源
    setPixmap(QPixmap(":/tu/cake.png"));
}

void Cake::updateLogic() {
    // 静态道具暂时不需要移动逻辑，留空即可
}