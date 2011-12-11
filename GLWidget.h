#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QtOpenGL/QGLWidget>
#include <QtOpenGL/QGLShaderProgram>
#include <vector>
#include "model.h"
#include "GL/glext.h"
#include "emitter.h"
#include "texture.h"
#include "light.h"

using std::string;
using std::vector;
using std::ifstream;
using std::istringstream;

class Model;
class Emitter;
class Light;

class GLWidget : public QGLWidget
{
    Q_OBJECT // must include this if you use Qt signals/slots

public:
    GLWidget(QWidget *parent = NULL);
    ~GLWidget();

public slots:
    void calculateFPS();

protected:
    void initializeGL();
    void paintGL();
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void resizeGL(int width, int height);
    void keyPressEvent(QKeyEvent* event);
    void wheelEvent(QWheelEvent* event);
    void drawBoard();
    void drawKnight();
    void updateBoardMap(float x, float z);
    void knightsTour(int row, int column);
    void tourBoard(std::vector<int> &path, int occupancy[128]);

private:
    int windowWidth;
    int windowHeight;
    float frame;
    float fps;
    float xRot;
    float yRot;
    float knightXPos;
    float knightYPos;
    float knightZPos;
    float panSensitivity;
    float rotateSpeed;
    float swivleSensitivity;
    float xPos, yPos, zPos;
    float zoomSensitivity;
    bool boardMap[8][8];
    bool boardTourMap[8][8];
    bool doneTouring;
    int moveCounter;
    bool gotIndex[64];
    int posIndex[64];

    // lights
    Light* mainlight;
    Light* blueLight;
    Light* redLight;
    Light* greenLight;
    Light* purpleLight;

    // textures
    Texture* textureOne;
    Texture* textureTwo;
    Texture* textureThree;
    Texture* textureFour;
    Texture* textureWhiteSquare;
    Texture* textureBlackSquare;
    Texture* textureFloor;
    Texture* textureKnight;
    vector<Texture*> textureNumbers;

    // particle emitters
    Emitter* emitterOne;
    Emitter* emitterTwo;
    Emitter* emitterThree;
    Emitter* emitterFour;
    vector<Model*> models;
    GLdouble camera[3];

    // clocks and timers
    QTime* clock;
    QTime* clock2;
    QTime* animationClock;
    QTimer* timer;

    QPoint lastPos;
    QPoint currentKnightSquare;
    vector<QPoint> finalKnightPath;
    QGLShaderProgram floorShaderProgram;
    QGLShaderProgram knightShaderProgram;
};

#endif // GLWIDGET_H
