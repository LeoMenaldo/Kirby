#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "basicenemy.h"
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
        // 传入参数："图片路径", 帧数(你的图是5帧), 速度, 能力标签(FIRE)
    BasicEnemy* fireEnemy = new BasicEnemy(":/tu/fire_enemy.png", 5, 1.5, Enemy::FIRE);
    fireEnemy->setScale(2);
    fireEnemy->setPos(400, 500); // 把它放在卡比右边一点的位置
    scene->addItem(fireEnemy);
    // （重要）你需要把生成的敌人存起来，以便在 gameUpdate 里让它们移动和检测碰撞
    // 记得在 mainwindow.h 里加一个 QList<Enemy*> enemies;
    enemies.append(fireEnemy);
    // 6. 游戏循环
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::gameUpdate);
    timer->start(16);
}
MainWindow::~MainWindow() { delete ui; }
void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) return;
    keys.insert(event->key());

    if (event->key() == Qt::Key_A || event->key() == Qt::Key_Left)
        lastHorizontalKey = event->key();
    else if (event->key() == Qt::Key_D || event->key() == Qt::Key_Right)
        lastHorizontalKey = event->key();
    else if (event->key() == Qt::Key_W || event->key() == Qt::Key_Up)
        jumpBuffer = 6;
    else if (event->key() == Qt::Key_K)            // 新增翻滚
        player->startRoll();
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) return;
    keys.remove(event->key());
    if (event->key() == lastHorizontalKey)
        lastHorizontalKey = 0;
}

void MainWindow::gameUpdate() {
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
    } else {
        // 1. 水平输入（原有逻辑）
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

    // 2. 重力（原样保留，翻滚时 isHovering=false 故走正常下落分支）
    if (!player->isOnGround) {
        if (player->isHovering && player->vy > 0) {
            player->vy += 0.1;
            if (player->vy > 2.5) player->vy = 2.5;
        } else {
            player->vy += 0.8;
            if (player->vy > 15) player->vy = 15;
        }
    }

    // 3. 土狼时间与跳跃缓冲（只有在非翻滚时生效，翻滚时已清零）
    if (!player->isRolling) {
        if (player->isOnGround) {
            coyoteTime = 6;
            player->canDoubleJump = true;
        } else {
            if (coyoteTime > 0) coyoteTime--;
        }

        if (jumpBuffer > 0) {
            if (coyoteTime > 0) {
                player->vy = -16;
                coyoteTime = 0;
                jumpBuffer = 0;
            } else if (player->canDoubleJump) {
                player->vy = -15;
                player->canDoubleJump = false;
                jumpBuffer = 0;
                player->currentState = Player::IDLE;
            }
            if (jumpBuffer > 0) jumpBuffer--;
        }
    }

    // 4. 水平移动 + 碰撞（翻滚时的 vx 也会在此处理）
    player->setPos(player->x() + player->vx, player->y());
    QRectF pRect = player->sceneBoundingRect();
    const auto &constFloors = floors;
    for (Tile *tile : constFloors) {
        if (player->collidesWithItem(tile)) {
            QRectF tRect = tile->sceneBoundingRect();
            if (player->vx > 0)
                player->setPos(tRect.left() - pRect.width(), player->y());
            else if (player->vx < 0)
                player->setPos(tRect.right(), player->y());
            pRect = player->sceneBoundingRect();
        }
    }

    // 5. 垂直检测（保持不变）
    bool onGround = false;

    player->setPos(player->x(), player->y() + 1);
    pRect = player->sceneBoundingRect();
    for (Tile *tile : constFloors) {
        if (player->collidesWithItem(tile)) {
            if (player->vy >= 0) {
                QRectF tRect = tile->sceneBoundingRect();
                player->setPos(player->x(), tRect.top() - pRect.height());
                player->vy = 0;
                onGround = true;
            }
            break;
        }
    }

    if (!onGround) {
        player->setPos(player->x(), player->y() - 1 + player->vy);
        pRect = player->sceneBoundingRect();
        for (Tile *tile : constFloors) {
            if (player->collidesWithItem(tile)) {
                QRectF tRect = tile->sceneBoundingRect();
                if (player->vy >= 0) {
                    player->setPos(player->x(), tRect.top() - pRect.height());
                    player->vy = 0;
                    onGround = true;
                } else {
                    player->setPos(player->x(), tRect.bottom());
                    player->vy = 0;
                }
                pRect = player->sceneBoundingRect();
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
}