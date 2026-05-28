#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSet>
#include <QGraphicsTextItem>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "player.h"
#include "tile.h"
#include "enemy.h"
#include "projectile.h"
#include "cake.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void gameUpdate();

private:
    QList<QGraphicsPixmapItem*> lifeIcons; // 存放生命值图标的列表
    void loadLevel(int level);
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QTimer *timer;
    Player *player;
    QPixmap waterBodyPix; // 新增：保存水的贴图以备融冰之用
    QList<QGraphicsRectItem*> backgroundLayers; // 新增：用于记录背景图层
    int lastHorizontalKey = 0;
    QList<Tile*> floors;
    QSet<int> keys;
    int jumpBuffer = 0;
    int coyoteTime = 0;
    QList<Enemy*> enemies;
    QList<Projectile*> projectiles;
    QList<Cake*> cakes;
    QVector<QPixmap> weiqiFrames;
    QList<QGraphicsPixmapItem*> exhaustItems;
    QList<int> exhaustLifetimes;
    QMediaPlayer *bgmPlayer;
    QAudioOutput *audioOutput;
    int aiTimer = 0;
    bool enterPressed = false;

    // ====== 新增：游戏状态机枚举与控制变量 ======
    enum GameState { START_SCREEN, INTRO_PAN, PLAYING };
    GameState currentState = START_SCREEN;
    int introTimer = 0;

    // UI文字元素
    QGraphicsTextItem* titleText = nullptr;
    QGraphicsTextItem* hintText = nullptr;

};

#endif // MAINWINDOW_H