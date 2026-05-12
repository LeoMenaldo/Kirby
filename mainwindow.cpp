#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->resize(1000, 600);

    // 1. 场景
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 5000, 1000);

    view = new QGraphicsView(scene, this);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget(view);

    qreal sceneW = scene->sceneRect().width();
    qreal sceneH = scene->sceneRect().height();

    // 2. 双层背景
    QStringList bgPaths = {":/tu/Background_1.png", ":/tu/Background_2.png"};
    for(int i = 0; i < bgPaths.size(); ++i) {
        QPixmap pix(bgPaths[i]);
        if(!pix.isNull()) {
            QPixmap scaledPix = pix.scaledToHeight(sceneH, Qt::SmoothTransformation);
            QGraphicsRectItem *bg = new QGraphicsRectItem(0, 0, sceneW, sceneH);
            bg->setBrush(QBrush(scaledPix));
            bg->setPen(Qt::NoPen);
            bg->setZValue(-100 + i);
            scene->addItem(bg);
        }
    }

    // 3. 切割方块素材
    QPixmap tileSheet(":/tu/fangkuai.png");
    int originalSize = 24;
    QPixmap grass        = tileSheet.copy(0, 0, originalSize, originalSize);
    QPixmap dirt         = tileSheet.copy(originalSize * 1, 0, originalSize, originalSize);
    QPixmap waterSurface = tileSheet.copy(originalSize * 2, 0, originalSize, originalSize);
    QPixmap waterBody    = tileSheet.copy(originalSize * 3, 0, originalSize, originalSize);

    // 4. 关卡矩阵 (Tile实体)
    QStringList levelData = {
        "000000000000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000000000000",
        "000000000000011110000000000000000000000000000000000000",
        "111000000000022220001111111111111111111111111111111111",
        "222111133311122221112222222222222222222222222222222222",
        "222222244422222222222222222222222222222222222222222222",
        "000000000000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000000000000",
        "111111111110011111111111111111111111111111111111111111",
        "000000000000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000000000000",
        "111111111110011111111111111111111111111111111111111111",
        "000000000000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000000000000",
        "000000000000000000000000000000000000000000000000000000",
        "111111111111111111111111111111111111111111111111111111",




    };

    int renderSize = originalSize * 2;               // 48像素
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
                if (type == '1' || type == '2')
                    floors.append(tile);    // 固体参与碰撞
            }
        }
    }

    // 5. 卡比
    player = new Player();
    player->setPos(100, 200);
    scene->addItem(player);

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
}