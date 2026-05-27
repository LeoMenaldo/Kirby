#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "basicenemy.h"
#include <QPainter>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->resize(1000, 600);

    // 1. 场景
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 5000, 1000); // 地图长度设为 5000

    view = new QGraphicsView(scene, this);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 固定高度通常不需要垂直滚动条
    setCentralWidget(view);

    qreal sceneH = scene->sceneRect().height();

    // 2. 四层天空背景（修改部分：记录到 backgroundLayers）
    QStringList bgPaths = {
        ":/tu/Clouds 1/1.png",
        ":/tu/Clouds 1/2.png",
        ":/tu/Clouds 1/3.png",
        ":/tu/Clouds 1/4.png"
    };

    for(int i = 0; i < bgPaths.size(); ++i) {
        QPixmap pix(bgPaths[i]);
        if(!pix.isNull()) {
            // 将背景缩放到场景高度，保持比例
            QPixmap scaledPix = pix.scaledToHeight(sceneH, Qt::SmoothTransformation);

            // 创建一个矩形，宽度使用缩放后的图片宽度
            QGraphicsRectItem *bg = new QGraphicsRectItem(0, 0, scaledPix.width(), sceneH);
            bg->setBrush(QBrush(scaledPix));
            bg->setPen(Qt::NoPen);
            bg->setZValue(-100 + i);
            scene->addItem(bg);

            backgroundLayers.append(bg); // 记录图层
        }
    }

    // 3. 切割方块素材 (保持不变)
    QPixmap tileSheet(":/tu/fangkuai.png");
    int originalSize = 24;
    QPixmap grass        = tileSheet.copy(0, 0, originalSize, originalSize);
    QPixmap dirt         = tileSheet.copy(originalSize * 1, 0, originalSize, originalSize);
    QPixmap waterSurface = tileSheet.copy(originalSize * 2, 0, originalSize, originalSize);
    QPixmap waterBody    = tileSheet.copy(originalSize * 3, 0, originalSize, originalSize);

    // 4. 关卡矩阵 (保持不变，确保地图长度足够)
    QStringList levelData = {
        "000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000", // 让地面大概出现在第10行左右
        "111111111111111111111111111111111111111111111",
    };

    int renderSize = originalSize * 2;
    int totalMapHeight = levelData.size() * renderSize;
    int bottomOffset = sceneH - totalMapHeight;

    for (int r = 0; r < levelData.size(); r++) {
        for (int c = 0; c < levelData[r].length(); c++) {
            Tile *tile = nullptr;
            char type = levelData[r][c].toLatin1();
            if (type == '1')       tile = new Tile(Tile::Grass, grass);
            else if (type == '2')  tile = new Tile(Tile::Dirt, dirt);
            else if (type == '3')  tile = new Tile(Tile::WaterSurface, waterSurface);
            else if (type == '4')  tile = new Tile(Tile::WaterBody, waterBody);
            if (tile) {
                tile->setPos(c * renderSize, r * renderSize + bottomOffset);
                tile->setScale(2.0);
                scene->addItem(tile);
                if (type == '1' || type == '2') floors.append(tile);
            }
        }
    }

    // 5. 卡比
    player = new Player();
    player->setPos(100, 500); // 调整初始高度到地面附近
    scene->addItem(player);
    //生成敌人！
    BasicEnemy* fireEnemy = new BasicEnemy(":/tu/fire_enemy.png", 5, 1.5, Enemy::FIRE);
    fireEnemy->setScale(2);
    fireEnemy->setPos(400, 500);
    scene->addItem(fireEnemy);
    enemies.append(fireEnemy);

    BasicEnemy* iceEnemy = new BasicEnemy(":/tu/Ice_Dude.png", 6, 1.2, Enemy::ICE);
    iceEnemy->setScale(2);
    iceEnemy->setPos(700, 500);
    scene->addItem(iceEnemy);
    enemies.append(iceEnemy);

    BasicEnemy* leafEnemy = new BasicEnemy(":/tu/Leaf_Dude.png", 8, 1.0, Enemy::LEAF);
    leafEnemy->setScale(2);
    leafEnemy->setPos(1000, 500);
    scene->addItem(leafEnemy);
    enemies.append(leafEnemy);

    BasicEnemy* lightningEnemy = new BasicEnemy(":/tu/Lightning_Dude.png", 6, 1.8, Enemy::SPARK);
    lightningEnemy->setScale(2);
    lightningEnemy->setPos(1300, 500);
    scene->addItem(lightningEnemy);
    enemies.append(lightningEnemy);
    // 6. 游戏循环
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::gameUpdate);

    //初始化生命值 HUD 图标
    QPixmap lifePix(":/tu/life.png");
    for (int i = 0; i < 3; ++i) {
        QGraphicsPixmapItem* icon = new QGraphicsPixmapItem(lifePix);
        icon->setZValue(1000); // 确保图层在最上方，不被背景或地图遮挡
        icon->setScale(2.0);   // 如果图片太小，可以像这样放大2倍显示
        scene->addItem(icon);
        lifeIcons.append(icon);
    }
    // ====== 新增：在场景中生成一个测试蛋糕 ======
    Cake* testCake = new Cake();
    testCake->setPos(250, 600);
    scene->addItem(testCake);
    cakes.append(testCake);
    timer->start(16);
}
MainWindow::~MainWindow() { delete ui; }

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) return;
    if (player->isDigesting) {
        return;
    }
    // ====== 核心修复点：处于 Fatty 状态时，移除对 L 键的拦截 ======
    if (player->isFatty || player->isSpitting) {
        // 这里去掉了原本的 || event->key() == Qt::Key_L
        // 这样变胖时，按下 L 键才不会被中途拦截，能顺利传到底部触发变身
        if (event->key() == Qt::Key_W || event->key() == Qt::Key_Up ||
            event->key() == Qt::Key_J || event->key() == Qt::Key_K) {
            return;
        }
    }

    if (player->isSwallowing) {
        if (event->key() != Qt::Key_L) return;
    }

    keys.insert(event->key());

    if (event->key() == Qt::Key_A || event->key() == Qt::Key_Left)
        lastHorizontalKey = event->key();
    else if (event->key() == Qt::Key_D || event->key() == Qt::Key_Right)
        lastHorizontalKey = event->key();
    else if (event->key() == Qt::Key_W || event->key() == Qt::Key_Up)
        jumpBuffer = 6;
    else if (event->key() == Qt::Key_K)
        player->startRoll();
    // ====== J键攻击——四形态天生攻击，普通形态需蛋糕 ======
    else if (event->key() == Qt::Key_J) {
        bool canAttack = !player->isAttacking && !player->isRolling && !player->isSwallowing && !player->isSpitting;
        if (!canAttack) return;

        Enemy::CopyAbility form = player->currentForm;

        if (form == Enemy::FIRE || form == Enemy::ICE || form == Enemy::LEAF || form == Enemy::SPARK) {
            // 四形态：天生攻击能力，不需要吃蛋糕
            player->startAttack();

            // 根据形态选择弹丸颜色
            QColor outerColor, innerColor;
            if (form == Enemy::FIRE) {
                outerColor = QColor(255, 80, 0);
                innerColor = QColor(255, 200, 50);
            } else if (form == Enemy::ICE) {
                outerColor = QColor(30, 120, 255);
                innerColor = QColor(180, 220, 255);
            } else if (form == Enemy::LEAF) {
                outerColor = QColor(30, 180, 60);
                innerColor = QColor(150, 255, 150);
            } else { // SPARK
                outerColor = QColor(255, 200, 30);
                innerColor = QColor(255, 255, 180);
            }

            Projectile* proj = new Projectile(player->facingRight);
            proj->damage = 2;
            QPixmap pix(24, 24);
            pix.fill(Qt::transparent);
            QPainter painter(&pix);
            painter.setBrush(outerColor);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(0, 0, 24, 24);
            painter.setBrush(innerColor);
            painter.drawEllipse(4, 4, 16, 16);
            painter.end();
            proj->setPixmap(pix);
            double startX = player->facingRight ? player->x() + 48 : player->x() - 24;
            double startY = player->y() + 12;
            proj->setPos(startX, startY);
            scene->addItem(proj);
            projectiles.append(proj);

        } else {
            // 普通形态：必须吃蛋糕才有攻击能力
            if (player->hasAttackPower()) {
                player->startAttack();
                Projectile* proj = new Projectile(player->facingRight);
                double startX = player->facingRight ? player->x() + 48 : player->x() - 24;
                double startY = player->y() + 12;
                proj->setPos(startX, startY);
                scene->addItem(proj);
                projectiles.append(proj);
            }
        }
    }
    else if (event->key() == Qt::Key_T) {
        if (player->isFatty && !player->isSpitting) {
            player->startSpit();
        }
    }
    // ====== L键多功能逻辑 ======
    else if (event->key() == Qt::Key_L) {
        // 1. 如果是胖子状态，按 L 消化变身
        if (player->isFatty && !player->isSpitting && !player->isSwallowing) {
            player->startDigest();
        }
        // 2. 如果已拥有形态能力，启动长按计时器（需按住1秒才取消，防误触）
        else if (!player->isFatty && player->currentForm != Enemy::NONE && player->formCancelTimer == 0) {
            player->formCancelTimer = 1;
        }
    }
}
void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) return;
    keys.remove(event->key());
    if (event->key() == lastHorizontalKey)
        lastHorizontalKey = 0;
}

void MainWindow::gameUpdate() {
    // ====== L键长按：吞噬 vs 取消形态 ======
    if (keys.contains(Qt::Key_L) && player->currentForm == Enemy::NONE && player->isOnGround && !player->isFatty && !player->isRolling && !player->isAttacking && !player->isDigesting && !player->isSpitting) {
        player->startSwallow();
    } else if ((!keys.contains(Qt::Key_L) || !player->isOnGround) && player->isSwallowing) {
        player->endSwallow();
    }

    // 长按L取消形态（需持续按住60帧=1秒，松手即重置）
    if (player->formCancelTimer > 0) {
        if (keys.contains(Qt::Key_L)) {
            player->formCancelTimer++;
            if (player->formCancelTimer >= 60) {
                player->currentForm = Enemy::NONE;
                player->formCancelTimer = 0;
            }
        } else {
            player->formCancelTimer = 0; // 松手就重置，防误触
        }
    }

    // ====== 运动状态速度驱动 ======
    if (player->isRolling) {
        player->vx = player->facingRight ? Player::rollSpeed : -Player::rollSpeed;

        if (player->rollTimer >= 0) {
            if (jumpBuffer > 0) {
                if (player->isOnGround) {
                    player->vy = -16; coyoteTime = 0; jumpBuffer = 0; player->endRoll();
                } else if (player->canDoubleJump) {
                    player->vy = -15; player->canDoubleJump = false; jumpBuffer = 0; player->endRoll();
                }
            }
        } else {
            jumpBuffer = 0; coyoteTime = 0; player->isHovering = false;
        }
    }
    else if (player->isSwallowing) {
        player->vx = 0;
        jumpBuffer = 0; // 核心：吞噬时清除跳跃缓冲，防止吞噬结束后突然起跳
        coyoteTime = 0; // 核心：清除土狼时间
    }
    // ====== 新增：吐出动画播放期间，水平移动物理速度归零 ======
    else if (player->isSpitting) {
        player->vx = 0;
        jumpBuffer = 0;
        coyoteTime = 0;
    }
    // ====== 新增：在播放变身动画期间，物理移动速度、跳跃缓冲全部归零 ======
    else if (player->isDigesting) {
        player->vx = 0;
        jumpBuffer = 0;
        coyoteTime = 0;
    }
    else {
        // 水平输入（Fatty 状态下正常允许走动）
        int activeKey = 0;
        if (keys.contains(lastHorizontalKey)) activeKey = lastHorizontalKey;
        else {
            if (keys.contains(Qt::Key_A) || keys.contains(Qt::Key_Left)) activeKey = Qt::Key_A;
            else if (keys.contains(Qt::Key_D) || keys.contains(Qt::Key_Right)) activeKey = Qt::Key_D;
        }

        if (activeKey == Qt::Key_A || activeKey == Qt::Key_Left) {
            player->vx = -5; player->facingRight = false;
        } else if (activeKey == Qt::Key_D || activeKey == Qt::Key_Right) {
            player->vx = 5; player->facingRight = true;
        } else {
            player->vx = 0;
        }
    }
    // ====== 翻滚状态特殊处理 ======
    if (player->isRolling) {
        player->vx = player->facingRight ? Player::rollSpeed : -Player::rollSpeed;

        if (player->rollTimer>=0) {
            // 允许跳跃打断
            if (jumpBuffer > 0) {
                if (player->isOnGround) {
                    player->vy = -16;
                    coyoteTime = 0;
                    jumpBuffer = 0;
                    player->endRoll();
                } else if (player->canDoubleJump) {
                    player->vy = -15;
                    player->canDoubleJump = false;
                    jumpBuffer = 0;
                    player->endRoll();
                }
            }
        } else {
            jumpBuffer = 0;
            coyoteTime = 0;
            player->isHovering = false;
        }
    } else if (!player->isSwallowing && !player->isSpitting && !player->isDigesting) {
        // 1. 水平输入（原有逻辑）—— 吞噬/吐出/变身时跳过，防止覆盖第一段运动逻辑的vx=0
        int activeKey = 0;
        if (keys.contains(lastHorizontalKey)) activeKey = lastHorizontalKey;
        else {
            if (keys.contains(Qt::Key_A) || keys.contains(Qt::Key_Left)) activeKey = Qt::Key_A;
            else if (keys.contains(Qt::Key_D) || keys.contains(Qt::Key_Right)) activeKey = Qt::Key_D;
        }

        if (activeKey == Qt::Key_A || activeKey == Qt::Key_Left) {
            player->vx = -5; player->facingRight = false;
        } else if (activeKey == Qt::Key_D || activeKey == Qt::Key_Right) {
            player->vx = 5; player->facingRight = true;
        } else {
            player->vx = 0;
        }
    }

    // 2. 重力机制拦截
    if (player->isFatty) {
        player->isHovering = false; // Fatty 绝对禁止悬浮
    } else {
        player->isHovering = (keys.contains(Qt::Key_W) || keys.contains(Qt::Key_Up))
        && !player->isRolling && !player->isSwallowing;
    }

    if (!player->isOnGround) {
        if (player->isHovering && player->vy > 0) {
            player->vy += 0.1;
            if (player->vy > 2.5) player->vy = 2.5;
        } else {
            player->vy += 0.8;
            if (player->vy > 15) player->vy = 15;
        }
    }

    // 3. 土狼时间与跳跃缓冲
    if (player->isFatty) {
        // 彻底清洗所有与离地跳跃相关的变量
        jumpBuffer = 0;
        coyoteTime = 0;
        player->canDoubleJump = false;
    } else if (!player->isRolling && !player->isSwallowing) {
        if (player->isOnGround) {
            coyoteTime = 6;
            player->canDoubleJump = true;
        } else {
            if (coyoteTime > 0) coyoteTime--;
        }

        if (jumpBuffer > 0) {
            if (coyoteTime > 0) {
                player->vy = -16; coyoteTime = 0; jumpBuffer = 0;
            } else if (player->canDoubleJump) {
                player->vy = -15; player->canDoubleJump = false; jumpBuffer = 0;
                player->currentState = Player::IDLE;
            }
            if (jumpBuffer > 0) jumpBuffer--;
        }
    }
    // 4. 水平移动 + 碰撞
    player->setPos(player->x() + player->vx, player->y());
    const auto &constFloors = floors;
    for (Tile *tile : constFloors) {
        if (player->collidesWithItem(tile)) {
            QRectF tRect = tile->sceneBoundingRect();
            if (player->vx > 0)
                player->setPos(tRect.left() - 48, player->y()); // 将 pRect.width() 替换为固定的 48
            else if (player->vx < 0)
                player->setPos(tRect.right(), player->y());
        }
    }

    // 5. 垂直检测
    bool onGround = false;

    player->setPos(player->x(), player->y() + 1);
    for (Tile *tile : constFloors) {
        if (player->collidesWithItem(tile)) {
            if (player->vy >= 0) {
                QRectF tRect = tile->sceneBoundingRect();
                player->setPos(player->x(), tRect.top() - 48); // 将 pRect.height() 替换为固定的 48
                player->vy = 0;
                onGround = true;
            }
            break;
        }
    }

    if (!onGround) {
        player->setPos(player->x(), player->y() - 1 + player->vy);
        for (Tile *tile : constFloors) {
            if (player->collidesWithItem(tile)) {
                QRectF tRect = tile->sceneBoundingRect();
                if (player->vy >= 0) {
                    player->setPos(player->x(), tRect.top() - 48); // 将 pRect.height() 替换为固定的 48
                    player->vy = 0;
                    onGround = true;
                } else {
                    player->setPos(player->x(), tRect.bottom()); // 撞头时，由于逻辑原点在头顶，直接用 bottom 没问题
                    player->vy = 0;
                }
            }
        }
    }
    player->isOnGround = onGround;

    // 6. 翻滚结束判定
    if (player->isRolling) {
        player->rollTimer--;
        if (player->rollTimer <= 0) {
            player->endRoll();
        }
    }

    // 7. 状态更新与视角
    player->isHovering = (keys.contains(Qt::Key_W) || keys.contains(Qt::Key_Up))
                         && !player->isRolling;  // 翻滚时禁止漂浮
    player->updateLogic();
    view->centerOn(player);


    // ====== 敌人物理与逻辑计算 ======
    for (Enemy* enemy : enemies) {
        if (enemy->isDead) continue;

        // 1. 更新敌人自身逻辑 (决定 vx 和动画)
        enemy->updateLogic();

        // 2. 水平移动与墙壁碰撞
        enemy->setPos(enemy->x() + enemy->vx, enemy->y());
        QRectF eRect = enemy->sceneBoundingRect();
        for (Tile *tile : floors) {
            if (enemy->collidesWithItem(tile)) {
                QRectF tRect = tile->sceneBoundingRect();
                // 物理阻挡
                if (enemy->vx > 0) {
                    enemy->setPos(tRect.left() - eRect.width(), enemy->y());
                } else if (enemy->vx < 0) {
                    enemy->setPos(tRect.right(), enemy->y());
                }
                // 撞墙后掉头
                enemy->reverseDirection();
                break;
            }
        }

        // 3. 应用重力 (自由落体算法)
        enemy->vy += 0.8;
        if (enemy->vy > 15) enemy->vy = 15; // 终端速度限制

        // 4. 垂直移动与地面碰撞
        enemy->setPos(enemy->x(), enemy->y() + enemy->vy);
        eRect = enemy->sceneBoundingRect();
        for (Tile *tile : floors) {
            if (enemy->collidesWithItem(tile)) {
                QRectF tRect = tile->sceneBoundingRect();
                if (enemy->vy > 0) { // 往下掉时踩到地板
                    enemy->setPos(enemy->x(), tRect.top() - eRect.height());
                    enemy->vy = 0; // 落地速度清零
                }
                break;
            }
        }
    }
    // ====== 相机边界限位与背景固定逻辑 ======

    // 1. 获取基础参数
    qreal cameraX = player->x();
    qreal cameraY = 850; // 你之前设定的固定高度中心
    qreal sceneW = scene->sceneRect().width();  // 场景总宽度（如 5000）

    // 获取视口（窗口）的一半宽度
    // 使用 view->viewport()->width() 能得到更精确的内部绘图区域宽度
    qreal halfViewW = view->viewport()->width() / 2.0;

    // 2. 边界检查：如果相机的目标位置会让视角超出地图边缘，则强制锁定相机
    // 处理左边缘
    if (cameraX < halfViewW) {
        cameraX = halfViewW;
    }
    // 处理右边缘
    else if (cameraX > sceneW - halfViewW) {
        cameraX = sceneW - halfViewW;
    }

    // 3. 执行视角居中
    view->centerOn(cameraX, cameraY);

    // 4. 背景图层跟随“锁定后”的相机坐标
    // 这样当相机停止移动时，背景也会因为坐标不再变化而相对于屏幕静止
    for (QGraphicsRectItem* bg : backgroundLayers) {
        qreal bgX = cameraX - bg->rect().width() / 2.0;
        bg->setPos(bgX, 0);
    }
    // ====== 子弹逻辑与伤害判定 ======
    // 逆序遍历，方便在遍历中安全地删除元素
    for (int i = projectiles.size() - 1; i >= 0; i--) {
        Projectile* proj = projectiles[i];
        proj->updateLogic();

        bool hitEnemy = false;

        // 1. 检测是否打到敌人
        for (int j = enemies.size() - 1; j >= 0; j--) {
            Enemy* enemy = enemies[j];
            if (!enemy->isDead && proj->collidesWithItem(enemy)) {
                // 扣血
                enemy->takeDamage(proj->damage);
                hitEnemy = true;

                // 敌人死亡
                if (enemy->isDead) {
                    scene->removeItem(enemy);
                    enemies.removeAt(j);
                    delete enemy;
                }
                break; // 一颗光球只能打一个敌人，打中就退出内层循环
            }
        }

        // 2. 检测光球是否寿命耗尽 或 击中了敌人
        if (hitEnemy || proj->lifeTime <= 0) {
            scene->removeItem(proj);
            projectiles.removeAt(i);
            delete proj;
        }
    }
    // ====== 新增：玩家与敌人碰撞检测 (受击扣血) ======
    if (player->invulnTimer == 0 && player->hp>0) {
        for (Enemy* enemy : enemies) {
            // 只有活着的目标才能对玩家造成伤害
            if (!enemy->isDead && player->collidesWithItem(enemy)) {

                // 【机制平衡】如果卡比正在翻滚或吞噬，视为无敌/免伤，不扣血
                if (player->isRolling) continue;
                if (player->isSwallowing) continue;

                // 扣除生命值并给予1秒(60帧)无敌时间
                player->hp--;
                player->invulnTimer = 60;

                // 受击效果：给卡比一个微微向上的弹跳击退感
                player->vy = -6;

                // 检查游戏结束
                if (player->hp <= 0) {
                    timer->stop(); // 停止游戏主循环
                    QMessageBox::critical(this, "Game Over", "卡比没命了！游戏结束！");
                    this->close(); // 关闭游戏窗口
                    return;        // 直接返回，防止后续代码报错
                }
                break; // 单帧内只承受一次伤害
            }
        }
    }

    // ====== 新增：生命值图标动态固定在屏幕左上角 (HUD) ======
    qreal screenLeft = cameraX - halfViewW;
    qreal screenTop = cameraY - view->viewport()->height()/1.2;

    for (int i = 0; i < lifeIcons.size(); ++i) {
        if (i < player->hp) {
            lifeIcons[i]->setVisible(true);
            // 依次排列在屏幕左上角，留出 20 像素边距，每个图标间隔 40 像素
            lifeIcons[i]->setPos(screenLeft + 20 + i * 40, screenTop + 20);
        } else {
            // 命扣掉了就不显示
            lifeIcons[i]->setVisible(false);
        }
    }
    // ====== 修改后：同时支持吞噬怪物与拉扯蛋糕的吞噬逻辑 ======
    if (player->isSwallowing) {
        // 1. 获取卡比 48x48 的物理身体矩形
        QRectF playerRect(player->x(), player->y(), 48, 48);

        // 2. 建立吸力矩形：起点在卡比身上，往面朝方向延伸 60 像素
        QRectF swallowWindRect;
        if (player->facingRight) {
            swallowWindRect = QRectF(player->x(), player->y(), 48 + 60, 48);
        } else {
            swallowWindRect = QRectF(player->x() - 60, player->y(), 60 + 48, 48);
        }

        // 3. 逆序检查场景中的怪物（拉扯与进肚）
        for (int j = enemies.size() - 1; j >= 0; j--) {
            Enemy* enemy = enemies[j];
            if (!enemy->isDead) {
                QRectF enemyRect = enemy->sceneBoundingRect();

                // 【怪物阶段一】：触碰身体 -> 真正吸进肚子里
                if (playerRect.intersects(enemyRect)) {
                    player->swallowedAbility = enemy->ability; // 记住能力
                    enemy->isDead = true;
                    scene->removeItem(enemy);
                    enemies.removeAt(j);
                    delete enemy;

                    player->isFatty = true; // 吞到怪物，进入变胖状态
                    player->endSwallow();
                    break;
                }
                // 【怪物阶段二】：处于吸气范围内 -> 强行拉扯
                else if (swallowWindRect.intersects(enemyRect)) {
                    double pullSpeed = 5.5;
                    if (player->facingRight) {
                        enemy->setPos(enemy->x() - pullSpeed, enemy->y());
                    } else {
                        enemy->setPos(enemy->x() + pullSpeed, enemy->y());
                    }
                }
            }
        }

        // 4. 【全新添加】：逆序检查场景中的蛋糕（拉扯与进肚）
        for (int j = cakes.size() - 1; j >= 0; j--) {
            Cake* cake = cakes[j];
            QRectF cakeRect = cake->sceneBoundingRect();

            // 【蛋糕阶段一】：蛋糕触碰到了卡比的物理身体 -> 真正吃进肚子
            if (playerRect.intersects(cakeRect)) {
                // 赋予卡比 20 秒（1200帧）的攻击能力时效！
                player->attackPowerTimer = 1200;

                // 从场景中彻底移除并销毁内存
                scene->removeItem(cake);
                cakes.removeAt(j);
                delete cake;

                // 【核心机制】：蛋糕是直接消化的营养品，不进入Fatty状态
                player->isFatty = false;
                player->endSwallow(); // 直接结束吸气，恢复常规状态以释放光球
                break; // 单帧内只吞噬一个蛋糕
            }
            // 【蛋糕阶段二】：蛋糕处于吸气范围内 -> 产生无法反抗的拉扯飞行效果
            else if (swallowWindRect.intersects(cakeRect)) {
                double pullSpeed = 5.5; // 与吸怪速度保持一致，形成统一的黑洞吸力感
                if (player->facingRight) {
                    cake->setPos(cake->x() - pullSpeed, cake->y());
                } else {
                    cake->setPos(cake->x() + pullSpeed, cake->y());
                }
            }
        }
    }
    // ====== 新增：监听卡比吐出动画的触发帧，生成对应的星星子弹 ======
    if (player->triggerSpitStar) {
        player->triggerSpitStar = false; // 消费信号，立即复位

        // 1. 创建高伤害、高速度的星星子弹
        Projectile* spitStar = new Projectile(player->facingRight);
        spitStar->damage = 3;
        spitStar->vx = player->facingRight ? 16 : -16;

        // 2. 根据肚子里怪物的能力，绘制不同颜色的星星
        QPixmap starPix(32, 32);
        starPix.fill(Qt::transparent);
        QPainter painter(&starPix);

        if (player->swallowedAbility == Enemy::FIRE) {
            painter.setBrush(Qt::red);   // 如果是火系怪，喷出火红的伤害星
        } else {
            painter.setBrush(Qt::cyan);  // 普通怪喷出青色星
        }
        painter.setPen(Qt::NoPen);

        // 绘制钻石交叉星几何
        QPolygonF diamond;
        diamond << QPointF(16, 0) << QPointF(32, 16) << QPointF(16, 32) << QPointF(0, 16);
        painter.drawPolygon(diamond);
        painter.end();
        spitStar->setPixmap(starPix);

        // 3. 将星星精准定位在卡比的嘴部前方
        double startX = player->facingRight ? player->x() + 48 : player->x() - 32;
        double startY = player->y() + 8;
        spitStar->setPos(startX, startY);

        scene->addItem(spitStar);
        projectiles.append(spitStar);

        // 4. 发射完毕，彻底清洗肚子里的复制能力缓存
        player->swallowedAbility = Enemy::NONE;
    }
    // ====== 新增：吃蛋糕碰撞判定与计时器赋予 ======
    QRectF playerRect(player->x(), player->y(), 48, 48); // 抓取卡比包围盒

    // ====== 修改后：给蛋糕加上重力与地面碰撞阻挡 ======
    for (int i = cakes.size() - 1; i >= 0; i--) {
        Cake* cake = cakes[i];
        cake->updateLogic(); // 刷新可能存在的每帧逻辑

        // 1. 应用重力 (采用与敌人相同的自由落体算法)
        cake->vy += 0.8;
        if (cake->vy > 15) cake->vy = 15; // 终端垂直下落速度限制

        // 2. 垂直方向位移
        cake->setPos(cake->x(), cake->y() + cake->vy);

        // 3. 地板碰撞检测：防止蛋糕穿模掉出地图
        QRectF cRect = cake->sceneBoundingRect();
        for (Tile *tile : floors) {
            if (cake->collidesWithItem(tile)) {
                QRectF tRect = tile->sceneBoundingRect();
                if (cake->vy > 0) { // 只有在向下掉落时踩到地板才触发阻挡
                    // 将蛋糕的底部精准贴在方块的上边缘
                    cake->setPos(cake->x(), tRect.top() - cRect.height());
                    cake->vy = 0; // 落地后垂直速度清零，停止下落
                }
                break; // 踩到一块地板后就跳出当前的碰撞检查
            }
        }
    }
}