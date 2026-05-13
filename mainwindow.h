#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QKeyEvent>
#include <QSet>
#include "player.h"
#include "tile.h"
#include "enemy.h"
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
    void loadLevel(int level);
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QTimer *timer;
    Player *player;
    QList<QGraphicsRectItem*> backgroundLayers; // 新增：用于记录背景图层
    int lastHorizontalKey = 0;
    QList<Tile*> floors;
    QSet<int> keys;
    int jumpBuffer = 0;
    int coyoteTime = 0;
    QList<Enemy*> enemies;
};

#endif // MAINWINDOW_H