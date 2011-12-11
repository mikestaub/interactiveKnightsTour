#include <QtGui/QMouseEvent>
#include "GLWidget.h"
#include "light.h"
#include <stdio.h>
#include <math.h>
#include <qtimer.h>
#include <QTime>
#include <QMatrix4x4>
#include <QVector4D>

#define M_PI    3.14159265358979323846

GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent)
{
    setMouseTracking(true);
    // initialize control settings
    rotateSpeed = 0.01;
    swivleSensitivity = 0.1;
    zoomSensitivity = 0.5;
    panSensitivity = 1;

    // initialize member variables
    xRot = 30;
    yRot = 0;
    xPos = 0;
    yPos = 8;
    zPos = 20;
    frame = 0;
    fps = 0;
    knightXPos = 0;
    knightYPos = 0;
    knightZPos = 0;
    doneTouring = false;
    moveCounter = 0;
    finalKnightPath.reserve(64);
    windowWidth = 1024;
    windowHeight = 728;

    // initialize boolean arrays
    for(int i = 0; i < 8; i++)
        for(int j = 0; j < 8; j++)
            boardMap[i][j] = false;
    for(int i = 0; i < 8; i++)
        for(int j = 0; j < 8; j++)
            boardTourMap[i][j] = false;
    for(int i = 0 ; i < 64; i++)
        gotIndex[i] = false;
}

GLWidget::~GLWidget()
{
}

void GLWidget::resizeGL(int width, int height)
{
    windowWidth = width;
    windowHeight = height;

    glViewport(0, 0, windowWidth, windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.1f, 200.0f);
}

void GLWidget::initializeGL()
{
    // initialize timer and clocks
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(updateGL()));
    timer->start(0);
    clock = new QTime(0, 0, 0, 0);
    clock->start();
    clock2 = new QTime(0, 0, 0, 0);
    clock2->start();
    animationClock = new QTime(0, 0, 0, 0);
    animationClock->start();

    // initialize viewport
    glViewport(0, 0, windowWidth, windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.1f, 200.0f);

    // set enable/disable
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_PERSPECTIVE_CORRECTION_HINT);
    glEnable(GL_POLYGON_SMOOTH_HINT);
    glDisable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glEnable(GL_POLYGON_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0, 0, 0, 0);

    // initialize lights
    Light::Initialize();

    mainlight = new Light(LIGHT_DIRECTIONAL);
    mainlight->setPosition(0, 5, 0);
    mainlight->setDiffuse(2.0f, 2.0f, 2.0f, 1.0f);

    blueLight = new Light(LIGHT_POINT);
    blueLight->setPosition(8, 1.2, -8);
    blueLight->setDiffuse(1, 1, 6.0, 1.0);

    redLight = new Light(LIGHT_POINT);
    redLight->setPosition(-8, 1.2, 8);
    redLight->setDiffuse(6.0, 1, 1, 1.0);

    greenLight = new Light(LIGHT_POINT);
    greenLight->setPosition(8, 1.2, 8);
    greenLight->setDiffuse(1, 6, 1, 1.0);

    purpleLight = new Light(LIGHT_POINT);
    purpleLight->setPosition(-8, 1.2, -8);
    purpleLight->setDiffuse(3, 1, 3, 1.0);

    models.push_back(new Model("res/knight.obj"));

    // initialize textures
    textureOne = new Texture("res/purpleFire.tga");
    textureTwo = new Texture("res/redFire.tga");
    textureThree = new Texture("res/greenFire.tga");
    textureFour = new Texture("res/blueFire.tga");
    textureWhiteSquare = new Texture("res/whiteSquare.tga");
    textureBlackSquare = new Texture("res/blackSquare.tga");
    textureFloor = new Texture("res/floor.tga");
    textureKnight = new Texture("res/knightTex.tga");

    // load all 64 number textures for the board
    string tempString;
    char buffer[3];
    for(int i = 1; i <= 64; i++)
    {
        tempString = "res/textureNumbers/textureNumber";
        //itoa(i, buffer, 10);
        sprintf(buffer, "%d", i);
        tempString += buffer;
        tempString += ".tga";
        textureNumbers.push_back(new Texture(tempString));
    }

    // initialize particle emitters
    emitterOne = new Emitter();
    emitterOne->setTexture(textureOne);
    emitterOne->emissionRate = 750;
    emitterOne->emissionRadius = .6;
    emitterOne->size = 0.50;
    emitterOne->life = 0.5f;
    emitterOne->lifeRange = .2f;
    emitterOne->position.x = -8;
    emitterOne->position.y = 2;
    emitterOne->position.z = -8;

    emitterTwo = new Emitter();
    emitterTwo->setTexture(textureTwo);
    emitterTwo->emissionRate = 750;
    emitterTwo->emissionRadius = 0.6;
    emitterTwo->size = 0.50;
    emitterTwo->life = 0.5f;
    emitterTwo->lifeRange = .2f;
    emitterTwo->position.x = -8;
    emitterTwo->position.y = 2;
    emitterTwo->position.z = 8;

    emitterThree = new Emitter();
    emitterThree->setTexture(textureThree);
    emitterThree->emissionRate = 750;
    emitterThree->emissionRadius = 0.6;
    emitterThree->size = 0.50;
    emitterThree->life = 0.5f;
    emitterThree->lifeRange = .2f;
    emitterThree->position.x = 8;
    emitterThree->position.y = 2;
    emitterThree->position.z = 8;

    emitterFour = new Emitter();
    emitterFour->setTexture(textureFour);
    emitterFour->emissionRate = 750;
    emitterFour->emissionRadius = 0.6;
    emitterFour->size = 0.50;
    emitterFour->life = 0.5f;
    emitterFour->lifeRange = .2f;
    emitterFour->position.x = 8;
    emitterFour->position.y = 2;
    emitterFour->position.z = -8;

    // initialize the vertex and fragment shaders for the floor
    floorShaderProgram.addShaderFromSourceCode(QGLShader::Vertex,
        "varying vec3 Normal;\n"
        "varying vec3 Light1Direction;\n"
        "varying vec3 Light2Direction;\n"
        "varying vec3 Light3Direction;\n"
        "varying vec3 Light4Direction;\n"
        "varying vec2 texCoord;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = ftransform();\n"
        "    texCoord = gl_MultiTexCoord0.xy;\n"
        "    vec3 fvLight1Position = gl_LightSource[1].position.xyz;\n"
        "    vec3 fvLight2Position = gl_LightSource[2].position.xyz;\n"
        "    vec3 fvLight3Position = gl_LightSource[3].position.xyz;\n"
        "    vec3 fvLight4Position = gl_LightSource[4].position.xyz;\n"
        "    vec4 objectPosition = gl_ModelViewMatrix * gl_Vertex;\n"
        "    Light1Direction = (gl_ModelViewMatrix * vec4(fvLight1Position, 1)).xyz - objectPosition.xyz;\n"
        "    Light2Direction = (gl_ModelViewMatrix * vec4(fvLight2Position, 1)).xyz - objectPosition.xyz;\n"
        "    Light3Direction = (gl_ModelViewMatrix * vec4(fvLight3Position, 1)).xyz - objectPosition.xyz;\n"
        "    Light4Direction = (gl_ModelViewMatrix * vec4(fvLight4Position, 1)).xyz - objectPosition.xyz;\n"
        "    Normal = gl_NormalMatrix * gl_Normal;\n"
        "}");

    floorShaderProgram.addShaderFromSourceCode(QGLShader::Fragment,
        "uniform sampler2D squareTexture;\n"
        "varying vec3 Normal;\n"
        "varying vec3 Light1Direction;\n"
        "varying vec3 Light2Direction;\n"
        "varying vec3 Light3Direction;\n"
        "varying vec3 Light4Direction;\n"
        "varying vec2 texCoord;\n"
        "void main(void)\n"
        "{\n"
        "    vec4 diffuseColor = texture2D(squareTexture, texCoord);\n"
        "    float facingRatio1 = max(dot(normalize(Normal), normalize(Light1Direction)), 0.0);\n"
        "    float facingRatio2 = max(dot(normalize(Normal), normalize(Light2Direction)), 0.0);\n"
        "    float facingRatio3 = max(dot(normalize(Normal), normalize(Light3Direction)), 0.0);\n"
        "    float facingRatio4 = max(dot(normalize(Normal), normalize(Light4Direction)), 0.0);\n"
        "    vec4 light1Color = diffuseColor * facingRatio1 * gl_LightSource[1].diffuse;\n"
        "    vec4 light2Color = diffuseColor * facingRatio2 * gl_LightSource[2].diffuse;\n"
        "    vec4 light3Color = diffuseColor * facingRatio3 * gl_LightSource[3].diffuse;\n"
        "    vec4 light4Color = diffuseColor * facingRatio4 * gl_LightSource[4].diffuse;\n"
        "    gl_FragColor = light1Color + light2Color + light3Color + light4Color * diffuseColor;\n"
        "}");

    // initialize vertex and fragment shaders for the knight
    knightShaderProgram.addShaderFromSourceCode(QGLShader::Vertex,
        "uniform vec3 vPosition;\n"
        "varying vec3 Normal;\n"
        "varying vec3 Light1Direction;\n"
        "varying vec3 Light2Direction;\n"
        "varying vec3 Light3Direction;\n"
        "varying vec3 Light4Direction;\n"
        "varying vec2 texCoord;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = ftransform();\n"
        "    texCoord = gl_MultiTexCoord0.xy;\n"
        "    vec3 fvLight1Position = gl_LightSource[1].position.xyz;\n"
        "    vec3 fvLight2Position = gl_LightSource[2].position.xyz;\n"
        "    vec3 fvLight3Position = gl_LightSource[3].position.xyz;\n"
        "    vec3 fvLight4Position = gl_LightSource[4].position.xyz;\n"
        "    fvLight1Position.y += 1.0;\n"
        "    fvLight2Position.y += 1.0;\n"
        "    fvLight3Position.y += 1.0;\n"
        "    fvLight4Position.y += 1.0;\n"
        "    vec4 objectPosition = gl_ModelViewMatrix * vec4(vPosition, 1);\n"
        "    Light1Direction = (gl_ModelViewMatrix * vec4(fvLight1Position, 1)).xyz - objectPosition.xyz;\n"
        "    Light2Direction = (gl_ModelViewMatrix * vec4(fvLight2Position, 1)).xyz - objectPosition.xyz;\n"
        "    Light3Direction = (gl_ModelViewMatrix * vec4(fvLight3Position, 1)).xyz - objectPosition.xyz;\n"
        "    Light4Direction = (gl_ModelViewMatrix * vec4(fvLight4Position, 1)).xyz - objectPosition.xyz;\n"
        "    Normal = gl_NormalMatrix * gl_Normal;\n"
        "}");

    knightShaderProgram.addShaderFromSourceCode(QGLShader::Fragment,
        "uniform sampler2D squareTexture;\n"
        "varying vec3 Normal;\n"
        "varying vec3 Light1Direction;\n"
        "varying vec3 Light2Direction;\n"
        "varying vec3 Light3Direction;\n"
        "varying vec3 Light4Direction;\n"
        "varying vec2 texCoord;\n"
        "void main(void)\n"
        "{\n"
        "    vec4 diffuseColor = texture2D(squareTexture, texCoord);\n"
        "    float facingRatio1 = max(dot(normalize(Normal), normalize(Light1Direction)), 0.0);\n"
        "    float facingRatio2 = max(dot(normalize(Normal), normalize(Light2Direction)), 0.0);\n"
        "    float facingRatio3 = max(dot(normalize(Normal), normalize(Light3Direction)), 0.0);\n"
        "    float facingRatio4 = max(dot(normalize(Normal), normalize(Light4Direction)), 0.0);\n"
        "    vec4 light1Color = diffuseColor * (.5 * facingRatio1) * gl_LightSource[1].diffuse;\n"
        "    vec4 light2Color = diffuseColor * (.5 * facingRatio2) * gl_LightSource[2].diffuse;\n"
        "    vec4 light3Color = diffuseColor * (.5 * facingRatio3) * gl_LightSource[3].diffuse;\n"
        "    vec4 light4Color = diffuseColor * (.5 * facingRatio4) * gl_LightSource[4].diffuse;\n"
        "    gl_FragColor = light1Color + light2Color + light3Color + light4Color * diffuseColor;\n"
        "}");
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(xRot, 1, 0, 0);
    glRotatef(yRot, 0, 1, 0);
    glTranslatef(-xPos, -yPos, -zPos);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    // implementing per-pixel lighting for the floor using GLSL shaders
    floorShaderProgram.bind();
    floorShaderProgram.setUniformValue("squareTexture", 0);
    drawBoard();
    floorShaderProgram.release();

    // draw back emitters
    glEnable(GL_BLEND);
    emitterOne->Update((float)clock2->elapsed());
    emitterFour->Update((float)clock2->elapsed());

    // draw knight using shaders
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    knightShaderProgram.bind();
    knightShaderProgram.setUniformValue("squareTexture", 0);
    glBindTexture(GL_TEXTURE_2D, textureKnight->texID);
    drawKnight();
    knightShaderProgram.release();

    // draw front emitters
    glEnable(GL_BLEND);
    emitterTwo->Update((float)clock2->elapsed());
    emitterThree->Update((float)clock2->elapsed());

    // display text
    if(!doneTouring)
    {
        QString textLine("Double-click a square to begin the knight's tour!");
        glColor3f(1, 1, 1);
        this->renderText((windowWidth/2 - 150), 20, textLine);
    }

    // display fps
    calculateFPS();
    char temp[4] = {"000"};
    sprintf(temp, "%.0f", fps);
    QString temp2(QString("FPS = ") + QString(temp));
    glColor3f(1, 1, 1);
    this->renderText(windowWidth-60, 20, temp2);
}

void GLWidget::drawKnight()
{
    if(!doneTouring)
    {
        models[0]->drawModel();
    }
    else
    {
        if(animationClock->elapsed() > 1000 && moveCounter < 64)
        {
            QPoint tempPoint = finalKnightPath[moveCounter];
            if(moveCounter == 0)
            {
                int countX = 0;
                int countZ = 0;
                int tempX = tempPoint.x();
                int tempZ = tempPoint.y();
                if(tempPoint.x() < 4)
                {
                    while(tempX < 3)
                    {
                        tempX += 1;
                        countX++;
                    }
                    knightXPos -= countX * 2;
                    knightXPos -= 1;
                }
                else
                {
                    while(tempX > 4)
                    {
                        tempX -= 1;
                        countX++;
                    }
                    knightXPos += countX * 2;
                    knightXPos += 1;
                }
                if(tempPoint.y() < 4)
                {
                    while(tempZ < 3)
                    {
                        tempZ += 1;
                        countZ++;
                    }
                    knightZPos += countZ * 2;
                    knightZPos += 1;
                }
                else
                {
                    while(tempZ > 4)
                    {
                        tempZ -= 1;
                        countZ++;
                    }
                    knightZPos -= countZ * 2;
                    knightZPos -= 1;
                }
                currentKnightSquare = finalKnightPath[moveCounter];
            }
            else
            {
                if(tempPoint.x() - currentKnightSquare.x() < 0)
                    knightXPos += (tempPoint.x() - currentKnightSquare.x()) * 2;
                else
                    knightXPos += (tempPoint.x() - currentKnightSquare.x()) * 2;

                if(tempPoint.y() - currentKnightSquare.y() < 0)
                    knightZPos += -((tempPoint.y() - currentKnightSquare.y()) * 2);
                else
                    knightZPos -= (tempPoint.y() - currentKnightSquare.y()) * 2;

                currentKnightSquare = finalKnightPath[moveCounter];
            }

            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
                glTranslated(knightXPos, knightYPos, knightZPos);
                knightShaderProgram.setUniformValue("vPosition", knightXPos, knightYPos, knightZPos);
                models[0]->drawModel();
            glPopMatrix();

            animationClock->restart();
            boardTourMap[tempPoint.x()][tempPoint.y()] = true;
            moveCounter++;
        }
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
            glTranslated(knightXPos, knightYPos, knightZPos);
            knightShaderProgram.setUniformValue("vPosition", knightXPos, knightYPos, knightZPos);
            models[0]->drawModel();
        glPopMatrix();
    }
}

// draw all 64 squares seperately
void GLWidget::drawBoard()
{
    // square 0, 0 ( front left )
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[0][0] == true)
    {
        if(!gotIndex[0])
        {
            posIndex[0] = moveCounter;
            gotIndex[0] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[0]-1]->texID);
    }
    glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glTexCoord2f(0, 0); glVertex3f(-8.0, 0.5,  8.0);
        glTexCoord2f(1, 0); glVertex3f(-6.0, 0.5,  8.0);
        glTexCoord2f(1, 1); glVertex3f(-6.0, 0.5,  6.0);
        glTexCoord2f(0, 1); glVertex3f(-8.0, 0.5,  6.0);
    glEnd();

    // square 1, 0
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[1][0] == true)
    {
        if(!gotIndex[1])
        {
            posIndex[1] = moveCounter;
            gotIndex[1] = true;
        }

        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[1]-1]->texID);
    }
    glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glTexCoord2f(0, 0); glVertex3f(-6.0, 0.5,  8.0);
        glTexCoord2f(1, 0); glVertex3f(-4.0, 0.5,  8.0);
        glTexCoord2f(1, 1); glVertex3f(-4.0, 0.5,  6.0);
        glTexCoord2f(0, 1); glVertex3f(-6.0, 0.5,  6.0);
    glEnd();

    // square 2, 0
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[2][0] == true)
    {
        if(!gotIndex[2])
        {
            posIndex[2] = moveCounter;
            gotIndex[2] = true;
        }

        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[2]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-4.0, 0.5,  8.0);
        glTexCoord2f(1, 0); glVertex3f(-2.0, 0.5,  8.0);
        glTexCoord2f(1, 1); glVertex3f(-2.0, 0.5,  6.0);
        glTexCoord2f(0, 1); glVertex3f(-4.0, 0.5,  6.0);
    glEnd();

    // square 3, 0
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[3][0] == true)
    {
        if(!gotIndex[3])
        {
            posIndex[3] = moveCounter;
            gotIndex[3] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[3]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-2.0, 0.5,  8.0);
        glTexCoord2f(1, 0); glVertex3f(-0.0, 0.5,  8.0);
        glTexCoord2f(1, 1); glVertex3f(-0.0, 0.5,  6.0);
        glTexCoord2f(0, 1); glVertex3f(-2.0, 0.5,  6.0);
    glEnd();

    // square 4, 0
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[4][0] == true)
    {
        if(!gotIndex[4])
        {
            posIndex[4] = moveCounter;
            gotIndex[4] = true;
        }

        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[4]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(0.0, 0.5,  8.0);
        glTexCoord2f(1, 0); glVertex3f(2.0, 0.5,  8.0);
        glTexCoord2f(1, 1); glVertex3f(2.0, 0.5,  6.0);
        glTexCoord2f(0, 1); glVertex3f(0.0, 0.5,  6.0);
    glEnd();

    // square 5, 0
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[5][0] == true)
    {
        if(!gotIndex[5])
        {
            posIndex[5] = moveCounter;
            gotIndex[5] = true;
        }

        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[5]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(2.0, 0.5,  8.0);
        glTexCoord2f(1, 0); glVertex3f(4.0, 0.5,  8.0);
        glTexCoord2f(1, 1); glVertex3f(4.0, 0.5,  6.0);
        glTexCoord2f(0, 1); glVertex3f(2.0, 0.5,  6.0);
    glEnd();

    // square 6, 0
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[6][0] == true)
    {
        if(!gotIndex[6])
        {
            posIndex[6] = moveCounter;
            gotIndex[6] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[6]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(4.0, 0.5,  8.0);
        glTexCoord2f(1, 0); glVertex3f(6.0, 0.5,  8.0);
        glTexCoord2f(1, 1); glVertex3f(6.0, 0.5,  6.0);
        glTexCoord2f(0, 1); glVertex3f(4.0, 0.5,  6.0);
    glEnd();

    // square 7, 0
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[7][0] == true)
    {
        if(!gotIndex[7])
        {
            posIndex[7] = moveCounter;
            gotIndex[7] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[7]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(6.0, 0.5,  8.0);
        glTexCoord2f(1, 0); glVertex3f(8.0, 0.5,  8.0);
        glTexCoord2f(1, 1); glVertex3f(8.0, 0.5,  6.0);
        glTexCoord2f(0, 1); glVertex3f(6.0, 0.5,  6.0);
    glEnd();

    // square 0, 1
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[0][1] == true)
    {
        if(!gotIndex[8])
        {
            posIndex[8] = moveCounter;
            gotIndex[8] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[8]-1]->texID);
    }
    glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glTexCoord2f(0, 0); glVertex3f(-8.0, 0.5,  6.0);
        glTexCoord2f(1, 0); glVertex3f(-6.0, 0.5,  6.0);
        glTexCoord2f(1, 1); glVertex3f(-6.0, 0.5,  4.0);
        glTexCoord2f(0, 1); glVertex3f(-8.0, 0.5,  4.0);
    glEnd();

    // square 1, 1
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[1][1] == true)
    {
        if(!gotIndex[9])
        {
            posIndex[9] = moveCounter;
            gotIndex[9] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[9]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-6.0, 0.5,  6.0);
        glTexCoord2f(1, 0); glVertex3f(-4.0, 0.5,  6.0);
        glTexCoord2f(1, 1); glVertex3f(-4.0, 0.5,  4.0);
        glTexCoord2f(0, 1); glVertex3f(-6.0, 0.5,  4.0);
    glEnd();

    // square 2, 1
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[2][1] == true)
    {
        if(!gotIndex[10])
        {
            posIndex[10] = moveCounter;
            gotIndex[10] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[10]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-4.0, 0.5,  6.0);
        glTexCoord2f(1, 0); glVertex3f(-2.0, 0.5,  6.0);
        glTexCoord2f(1, 1); glVertex3f(-2.0, 0.5,  4.0);
        glTexCoord2f(0, 1); glVertex3f(-4.0, 0.5,  4.0);
    glEnd();

    // square 3, 1
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[3][1] == true)
    {
        if(!gotIndex[11])
        {
            posIndex[11] = moveCounter;
            gotIndex[11] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[11]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-2.0, 0.5,  6.0);
        glTexCoord2f(1, 0); glVertex3f(-0.0, 0.5,  6.0);
        glTexCoord2f(1, 1); glVertex3f(-0.0, 0.5,  4.0);
        glTexCoord2f(0, 1); glVertex3f(-2.0, 0.5,  4.0);
    glEnd();

    // square 4, 1
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[4][1] == true)
    {
        if(!gotIndex[12])
        {
            posIndex[12] = moveCounter;
            gotIndex[12] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[12]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(0.0, 0.5,  6.0);
        glTexCoord2f(1, 0); glVertex3f(2.0, 0.5,  6.0);
        glTexCoord2f(1, 1); glVertex3f(2.0, 0.5,  4.0);
        glTexCoord2f(0, 1); glVertex3f(0.0, 0.5,  4.0);
    glEnd();

    // square 5, 1
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[5][1] == true)
    {
        if(!gotIndex[13])
        {
            posIndex[13] = moveCounter;
            gotIndex[13] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[13]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(2.0, 0.5,  6.0);
        glTexCoord2f(1, 0); glVertex3f(4.0, 0.5,  6.0);
        glTexCoord2f(1, 1); glVertex3f(4.0, 0.5,  4.0);
        glTexCoord2f(0, 1); glVertex3f(2.0, 0.5,  4.0);
    glEnd();

    // square 6, 1
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[6][1] == true)
    {
        if(!gotIndex[14])
        {
            posIndex[14] = moveCounter;
            gotIndex[14] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[14]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(4.0, 0.5,  6.0);
        glTexCoord2f(1, 0); glVertex3f(6.0, 0.5,  6.0);
        glTexCoord2f(1, 1); glVertex3f(6.0, 0.5,  4.0);
        glTexCoord2f(0, 1); glVertex3f(4.0, 0.5,  4.0);
    glEnd();

    // square 7, 1
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[7][1] == true)
    {
        if(!gotIndex[15])
        {
            posIndex[15] = moveCounter;
            gotIndex[15] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[15]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(6.0, 0.5,  6.0);
        glTexCoord2f(1, 0); glVertex3f(8.0, 0.5,  6.0);
        glTexCoord2f(1, 1); glVertex3f(8.0, 0.5,  4.0);
        glTexCoord2f(0, 1); glVertex3f(6.0, 0.5,  4.0);
    glEnd();

    // square 0, 2
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[0][2] == true)
    {
        if(!gotIndex[16])
        {
            posIndex[16] = moveCounter;
            gotIndex[16] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[16]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-8.0, 0.5,  4.0);
        glTexCoord2f(1, 0); glVertex3f(-6.0, 0.5,  4.0);
        glTexCoord2f(1, 1); glVertex3f(-6.0, 0.5,  2.0);
        glTexCoord2f(0, 1); glVertex3f(-8.0, 0.5,  2.0);
    glEnd();

    // square 1, 2
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[1][2] == true)
    {
        if(!gotIndex[17])
        {
            posIndex[17] = moveCounter;
            gotIndex[17] = true;
        }

        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[17]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-6.0, 0.5,  4.0);
        glTexCoord2f(1, 0); glVertex3f(-4.0, 0.5,  4.0);
        glTexCoord2f(1, 1); glVertex3f(-4.0, 0.5,  2.0);
        glTexCoord2f(0, 1); glVertex3f(-6.0, 0.5,  2.0);
    glEnd();

    // square 2, 2
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[2][2] == true)
    {
        if(!gotIndex[18])
        {
            posIndex[18] = moveCounter;
            gotIndex[18] = true;
        }

        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[18]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-4.0, 0.5,  4.0);
        glTexCoord2f(1, 0); glVertex3f(-2.0, 0.5,  4.0);
        glTexCoord2f(1, 1); glVertex3f(-2.0, 0.5,  2.0);
        glTexCoord2f(0, 1); glVertex3f(-4.0, 0.5,  2.0);
    glEnd();

    // square 3, 2
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[3][2] == true)
    {
        if(!gotIndex[19])
        {
            posIndex[19] = moveCounter;
            gotIndex[19] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[19]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-2.0, 0.5,  4.0);
        glTexCoord2f(1, 0); glVertex3f(-0.0, 0.5,  4.0);
        glTexCoord2f(1, 1); glVertex3f(-0.0, 0.5,  2.0);
        glTexCoord2f(0, 1); glVertex3f(-2.0, 0.5,  2.0);
    glEnd();

    // square 4, 2
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[4][2] == true)
    {
        if(!gotIndex[20])
        {
            posIndex[20] = moveCounter;
            gotIndex[20] = true;
        }

        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[20]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(0.0, 0.5,  4.0);
        glTexCoord2f(1, 0); glVertex3f(2.0, 0.5,  4.0);
        glTexCoord2f(1, 1); glVertex3f(2.0, 0.5,  2.0);
        glTexCoord2f(0, 1); glVertex3f(0.0, 0.5,  2.0);
    glEnd();

    // square 5, 2
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[5][2] == true)
    {
        if(!gotIndex[21])
        {
            posIndex[21] = moveCounter;
            gotIndex[21] = true;
        }

        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[21]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(2.0, 0.5,  4.0);
        glTexCoord2f(1, 0); glVertex3f(4.0, 0.5,  4.0);
        glTexCoord2f(1, 1); glVertex3f(4.0, 0.5,  2.0);
        glTexCoord2f(0, 1); glVertex3f(2.0, 0.5,  2.0);
    glEnd();

    // square 6, 2
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[6][2] == true)
    {
        if(!gotIndex[22])
        {
            posIndex[22] = moveCounter;
            gotIndex[22] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[22]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(4.0, 0.5,  4.0);
        glTexCoord2f(1, 0); glVertex3f(6.0, 0.5,  4.0);
        glTexCoord2f(1, 1); glVertex3f(6.0, 0.5,  2.0);
        glTexCoord2f(0, 1); glVertex3f(4.0, 0.5,  2.0);
    glEnd();

    // square 7, 2
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[7][2] == true)
    {
        if(!gotIndex[23])
        {
            posIndex[23] = moveCounter;
            gotIndex[23] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[23]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(6.0, 0.5,  4.0);
        glTexCoord2f(1, 0); glVertex3f(8.0, 0.5,  4.0);
        glTexCoord2f(1, 1); glVertex3f(8.0, 0.5,  2.0);
        glTexCoord2f(0, 1); glVertex3f(6.0, 0.5,  2.0);
    glEnd();

    // square 0, 3
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[0][3] == true)
    {
        if(!gotIndex[24])
        {
            posIndex[24] = moveCounter;
            gotIndex[24] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[24]-1]->texID);
    }
    glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glTexCoord2f(0, 0); glVertex3f(-8.0, 0.5,  2.0);
        glTexCoord2f(1, 0); glVertex3f(-6.0, 0.5,  2.0);
        glTexCoord2f(1, 1); glVertex3f(-6.0, 0.5,  0.0);
        glTexCoord2f(0, 1); glVertex3f(-8.0, 0.5,  0.0);
    glEnd();

    // square 1, 3
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[1][3] == true)
    {
        if(!gotIndex[25])
        {
            posIndex[25] = moveCounter;
            gotIndex[25] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[25]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-6.0, 0.5,  2.0);
        glTexCoord2f(1, 0); glVertex3f(-4.0, 0.5,  2.0);
        glTexCoord2f(1, 1); glVertex3f(-4.0, 0.5,  0.0);
        glTexCoord2f(0, 1); glVertex3f(-6.0, 0.5,  0.0);
    glEnd();

    // square 2, 3
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[2][3] == true)
    {
        if(!gotIndex[26])
        {
            posIndex[26] = moveCounter;
            gotIndex[26] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[26]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-4.0, 0.5,  2.0);
        glTexCoord2f(1, 0); glVertex3f(-2.0, 0.5,  2.0);
        glTexCoord2f(1, 1); glVertex3f(-2.0, 0.5,  0.0);
        glTexCoord2f(0, 1); glVertex3f(-4.0, 0.5,  0.0);
    glEnd();

    // square 3, 3
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[3][3] == true)
    {
        if(!gotIndex[27])
        {
            posIndex[27] = moveCounter;
            gotIndex[27] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[27]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-2.0, 0.5,  2.0);
        glTexCoord2f(1, 0); glVertex3f(-0.0, 0.5,  2.0);
        glTexCoord2f(1, 1); glVertex3f(-0.0, 0.5,  0.0);
        glTexCoord2f(0, 1); glVertex3f(-2.0, 0.5,  0.0);
    glEnd();

    // square 4, 3
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[4][3] == true)
    {
        if(!gotIndex[28])
        {
            posIndex[28] = moveCounter;
            gotIndex[28] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[28]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(0.0, 0.5,  2.0);
        glTexCoord2f(1, 0); glVertex3f(2.0, 0.5,  2.0);
        glTexCoord2f(1, 1); glVertex3f(2.0, 0.5,  0.0);
        glTexCoord2f(0, 1); glVertex3f(0.0, 0.5,  0.0);
    glEnd();

    // square 5, 3
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[5][3] == true)
    {
        if(!gotIndex[29])
        {
            posIndex[29] = moveCounter;
            gotIndex[29] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[29]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(2.0, 0.5,  2.0);
        glTexCoord2f(1, 0); glVertex3f(4.0, 0.5,  2.0);
        glTexCoord2f(1, 1); glVertex3f(4.0, 0.5,  0.0);
        glTexCoord2f(0, 1); glVertex3f(2.0, 0.5,  0.0);
    glEnd();

    // square 6, 3
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[6][3] == true)
    {
        if(!gotIndex[30])
        {
            posIndex[30] = moveCounter;
            gotIndex[30] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[30]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(4.0, 0.5,  2.0);
        glTexCoord2f(1, 0); glVertex3f(6.0, 0.5,  2.0);
        glTexCoord2f(1, 1); glVertex3f(6.0, 0.5,  0.0);
        glTexCoord2f(0, 1); glVertex3f(4.0, 0.5,  0.0);
    glEnd();

    // square 7, 3
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[7][3] == true)
    {
        if(!gotIndex[31])
        {
            posIndex[31] = moveCounter;
            gotIndex[31] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[31]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(6.0, 0.5,  2.0);
        glTexCoord2f(1, 0); glVertex3f(8.0, 0.5,  2.0);
        glTexCoord2f(1, 1); glVertex3f(8.0, 0.5,  0.0);
        glTexCoord2f(0, 1); glVertex3f(6.0, 0.5,  0.0);
    glEnd();

    // square 0, 4
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[0][4] == true)
    {
        if(!gotIndex[32])
        {
            posIndex[32] = moveCounter;
            gotIndex[32] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[32]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-8.0, 0.5,  0.0);
        glTexCoord2f(1, 0); glVertex3f(-6.0, 0.5,  0.0);
        glTexCoord2f(1, 1); glVertex3f(-6.0, 0.5, -2.0);
        glTexCoord2f(0, 1); glVertex3f(-8.0, 0.5, -2.0);
    glEnd();

    // square 1, 4
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[1][4] == true)
    {
        if(!gotIndex[33])
        {
            posIndex[33] = moveCounter;
            gotIndex[33] = true;
        }

        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[33]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-6.0, 0.5,  0.0);
        glTexCoord2f(1, 0); glVertex3f(-4.0, 0.5,  0.0);
        glTexCoord2f(1, 1); glVertex3f(-4.0, 0.5, -2.0);
        glTexCoord2f(0, 1); glVertex3f(-6.0, 0.5, -2.0);
    glEnd();

    // square 2, 4
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[2][4] == true)
    {
        if(!gotIndex[34])
        {
            posIndex[34] = moveCounter;
            gotIndex[34] = true;
        }

        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[34]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-4.0, 0.5,  0.0);
        glTexCoord2f(1, 0); glVertex3f(-2.0, 0.5,  0.0);
        glTexCoord2f(1, 1); glVertex3f(-2.0, 0.5, -2.0);
        glTexCoord2f(0, 1); glVertex3f(-4.0, 0.5, -2.0);
    glEnd();

    // square 3, 4
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[3][4] == true)
    {
        if(!gotIndex[35])
        {
            posIndex[35] = moveCounter;
            gotIndex[35] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[35]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-2.0, 0.5,  0.0);
        glTexCoord2f(1, 0); glVertex3f(-0.0, 0.5,  0.0);
        glTexCoord2f(1, 1); glVertex3f(-0.0, 0.5, -2.0);
        glTexCoord2f(0, 1); glVertex3f(-2.0, 0.5, -2.0);
    glEnd();

    // square 4, 4
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[4][4] == true)
    {
        if(!gotIndex[36])
        {
            posIndex[36] = moveCounter;
            gotIndex[36] = true;
        }

        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[36]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(0.0, 0.5,  0.0);
        glTexCoord2f(1, 0); glVertex3f(2.0, 0.5,  0.0);
        glTexCoord2f(1, 1); glVertex3f(2.0, 0.5, -2.0);
        glTexCoord2f(0, 1); glVertex3f(0.0, 0.5, -2.0);
    glEnd();

    // square 5, 4
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[5][4] == true)
    {
        if(!gotIndex[37])
        {
            posIndex[37] = moveCounter;
            gotIndex[37] = true;
        }

        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[37]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(2.0, 0.5,  0.0);
        glTexCoord2f(1, 0); glVertex3f(4.0, 0.5,  0.0);
        glTexCoord2f(1, 1); glVertex3f(4.0, 0.5, -2.0);
        glTexCoord2f(0, 1); glVertex3f(2.0, 0.5, -2.0);
    glEnd();

    // square 6, 4
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[6][4] == true)
    {
        if(!gotIndex[38])
        {
            posIndex[38] = moveCounter;
            gotIndex[38] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[38]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(4.0, 0.5,  0.0);
        glTexCoord2f(1, 0); glVertex3f(6.0, 0.5,  0.0);
        glTexCoord2f(1, 1); glVertex3f(6.0, 0.5, -2.0);
        glTexCoord2f(0, 1); glVertex3f(4.0, 0.5, -2.0);
    glEnd();

    // square 7, 4
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[7][4] == true)
    {
        if(!gotIndex[39])
        {
            posIndex[39] = moveCounter;
            gotIndex[39] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[39]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(6.0, 0.5,  0.0);
        glTexCoord2f(1, 0); glVertex3f(8.0, 0.5,  0.0);
        glTexCoord2f(1, 1); glVertex3f(8.0, 0.5, -2.0);
        glTexCoord2f(0, 1); glVertex3f(6.0, 0.5, -2.0);
    glEnd();

    // square 0, 5
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[0][5] == true)
    {
        if(!gotIndex[40])
        {
            posIndex[40] = moveCounter;
            gotIndex[40] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[40]-1]->texID);
    }
    glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glTexCoord2f(0, 0); glVertex3f(-8.0, 0.5, -2.0);
        glTexCoord2f(1, 0); glVertex3f(-6.0, 0.5, -2.0);
        glTexCoord2f(1, 1); glVertex3f(-6.0, 0.5, -4.0);
        glTexCoord2f(0, 1); glVertex3f(-8.0, 0.5, -4.0);
    glEnd();

    // square 1, 5
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[1][5] == true)
    {
        if(!gotIndex[41])
        {
            posIndex[41] = moveCounter;
            gotIndex[41] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[41]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-6.0, 0.5, -2.0);
        glTexCoord2f(1, 0); glVertex3f(-4.0, 0.5, -2.0);
        glTexCoord2f(1, 1); glVertex3f(-4.0, 0.5, -4.0);
        glTexCoord2f(0, 1); glVertex3f(-6.0, 0.5, -4.0);
    glEnd();

    // square 2, 5
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[2][5] == true)
    {
        if(!gotIndex[42])
        {
            posIndex[42] = moveCounter;
            gotIndex[42] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[42]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-4.0, 0.5, -2.0);
        glTexCoord2f(1, 0); glVertex3f(-2.0, 0.5, -2.0);
        glTexCoord2f(1, 1); glVertex3f(-2.0, 0.5, -4.0);
        glTexCoord2f(0, 1); glVertex3f(-4.0, 0.5, -4.0);
    glEnd();

    // square 3, 5
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[3][5] == true)
    {
        if(!gotIndex[43])
        {
            posIndex[43] = moveCounter;
            gotIndex[43] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[43]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-2.0, 0.5, -2.0);
        glTexCoord2f(1, 0); glVertex3f(-0.0, 0.5, -2.0);
        glTexCoord2f(1, 1); glVertex3f(-0.0, 0.5, -4.0);
        glTexCoord2f(0, 1); glVertex3f(-2.0, 0.5, -4.0);
    glEnd();

    // square 4, 5
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[4][5] == true)
    {
        if(!gotIndex[44])
        {
            posIndex[44] = moveCounter;
            gotIndex[44] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[44]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(0.0, 0.5, -2.0);
        glTexCoord2f(1, 0); glVertex3f(2.0, 0.5, -2.0);
        glTexCoord2f(1, 1); glVertex3f(2.0, 0.5, -4.0);
        glTexCoord2f(0, 1); glVertex3f(0.0, 0.5, -4.0);
    glEnd();

    // square 5, 5
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[5][5] == true)
    {
        if(!gotIndex[45])
        {
            posIndex[45] = moveCounter;
            gotIndex[45] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[45]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(2.0, 0.5, -2.0);
        glTexCoord2f(1, 0); glVertex3f(4.0, 0.5, -2.0);
        glTexCoord2f(1, 1); glVertex3f(4.0, 0.5, -4.0);
        glTexCoord2f(0, 1); glVertex3f(2.0, 0.5, -4.0);
    glEnd();

    // square 6, 5
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[6][5] == true)
    {
        if(!gotIndex[46])
        {
            posIndex[46] = moveCounter;
            gotIndex[46] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[46]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(4.0, 0.5, -2.0);
        glTexCoord2f(1, 0); glVertex3f(6.0, 0.5, -2.0);
        glTexCoord2f(1, 1); glVertex3f(6.0, 0.5, -4.0);
        glTexCoord2f(0, 1); glVertex3f(4.0, 0.5, -4.0);
    glEnd();

    // square 7, 5
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[7][5] == true)
    {
        if(!gotIndex[47])
        {
            posIndex[47] = moveCounter;
            gotIndex[47] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[47]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(6.0, 0.5, -2.0);
        glTexCoord2f(1, 0); glVertex3f(8.0, 0.5, -2.0);
        glTexCoord2f(1, 1); glVertex3f(8.0, 0.5, -4.0);
        glTexCoord2f(0, 1); glVertex3f(6.0, 0.5, -4.0);
    glEnd();

    // square 0, 6
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[0][6] == true)
    {
        if(!gotIndex[48])
        {
            posIndex[48] = moveCounter;
            gotIndex[48] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[48]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-8.0, 0.5, -4.0);
        glTexCoord2f(1, 0); glVertex3f(-6.0, 0.5, -4.0);
        glTexCoord2f(1, 1); glVertex3f(-6.0, 0.5, -6.0);
        glTexCoord2f(0, 1); glVertex3f(-8.0, 0.5, -6.0);
    glEnd();

    // square 1, 6
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[1][6] == true)
    {
        if(!gotIndex[49])
        {
            posIndex[49] = moveCounter;
            gotIndex[49] = true;
        }

        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[49]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-6.0, 0.5, -4.0);
        glTexCoord2f(1, 0); glVertex3f(-4.0, 0.5, -4.0);
        glTexCoord2f(1, 1); glVertex3f(-4.0, 0.5, -6.0);
        glTexCoord2f(0, 1); glVertex3f(-6.0, 0.5, -6.0);
    glEnd();

    // square 2, 6
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[2][6] == true)
    {
        if(!gotIndex[50])
        {
            posIndex[50] = moveCounter;
            gotIndex[50] = true;
        }

        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[50]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-4.0, 0.5, -4.0);
        glTexCoord2f(1, 0); glVertex3f(-2.0, 0.5, -4.0);
        glTexCoord2f(1, 1); glVertex3f(-2.0, 0.5, -6.0);
        glTexCoord2f(0, 1); glVertex3f(-4.0, 0.5, -6.0);
    glEnd();

    // square 3, 6
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[3][6] == true)
    {
        if(!gotIndex[51])
        {
            posIndex[51] = moveCounter;
            gotIndex[51] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[51]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-2.0, 0.5, -4.0);
        glTexCoord2f(1, 0); glVertex3f(-0.0, 0.5, -4.0);
        glTexCoord2f(1, 1); glVertex3f(-0.0, 0.5, -6.0);
        glTexCoord2f(0, 1); glVertex3f(-2.0, 0.5, -6.0);
    glEnd();

    // square 4, 6
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[4][6] == true)
    {
        if(!gotIndex[52])
        {
            posIndex[52] = moveCounter;
            gotIndex[52] = true;
        }

        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[52]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(0.0, 0.5, -4.0);
        glTexCoord2f(1, 0); glVertex3f(2.0, 0.5, -4.0);
        glTexCoord2f(1, 1); glVertex3f(2.0, 0.5, -6.0);
        glTexCoord2f(0, 1); glVertex3f(0.0, 0.5, -6.0);
    glEnd();

    // square 5, 6
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[5][6] == true)
    {
        if(!gotIndex[53])
        {
            posIndex[53] = moveCounter;
            gotIndex[53] = true;
        }

        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[53]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(2.0, 0.5, -4.0);
        glTexCoord2f(1, 0); glVertex3f(4.0, 0.5, -4.0);
        glTexCoord2f(1, 1); glVertex3f(4.0, 0.5, -6.0);
        glTexCoord2f(0, 1); glVertex3f(2.0, 0.5, -6.0);
    glEnd();

    // square 6, 6
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[6][6] == true)
    {
        if(!gotIndex[54])
        {
            posIndex[54] = moveCounter;
            gotIndex[54] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[54]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(4.0, 0.5, -4.0);
        glTexCoord2f(1, 0); glVertex3f(6.0, 0.5, -4.0);
        glTexCoord2f(1, 1); glVertex3f(6.0, 0.5, -6.0);
        glTexCoord2f(0, 1); glVertex3f(4.0, 0.5, -6.0);
    glEnd();

    // square 7, 6
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[7][6] == true)
    {
        if(!gotIndex[55])
        {
            posIndex[55] = moveCounter;
            gotIndex[55] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[55]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(6.0, 0.5, -4.0);
        glTexCoord2f(1, 0); glVertex3f(8.0, 0.5, -4.0);
        glTexCoord2f(1, 1); glVertex3f(8.0, 0.5, -6.0);
        glTexCoord2f(0, 1); glVertex3f(6.0, 0.5, -6.0);
    glEnd();

    // square 0, 7
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[0][7] == true)
    {
        if(!gotIndex[56])
        {
            posIndex[56] = moveCounter;
            gotIndex[56] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[56]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-8.0, 0.5, -6.0);
        glTexCoord2f(1, 0); glVertex3f(-6.0, 0.5, -6.0);
        glTexCoord2f(1, 1); glVertex3f(-6.0, 0.5, -8.0);
        glTexCoord2f(0, 1); glVertex3f(-8.0, 0.5, -8.0);
    glEnd();

    // square 1, 7
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[1][7] == true)
    {
        if(!gotIndex[57])
        {
            posIndex[57] = moveCounter;
            gotIndex[57] = true;
        }

        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[57]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-6.0, 0.5, -6.0);
        glTexCoord2f(1, 0); glVertex3f(-4.0, 0.5, -6.0);
        glTexCoord2f(1, 1); glVertex3f(-4.0, 0.5, -8.0);
        glTexCoord2f(0, 1); glVertex3f(-6.0, 0.5, -8.0);
    glEnd();

    // square 2, 7
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[2][7] == true)
    {
        if(!gotIndex[58])
        {
            posIndex[58] = moveCounter;
            gotIndex[58] = true;
        }

        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[58]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-4.0, 0.5, -6.0);
        glTexCoord2f(1, 0); glVertex3f(-2.0, 0.5, -6.0);
        glTexCoord2f(1, 1); glVertex3f(-2.0, 0.5, -8.0);
        glTexCoord2f(0, 1); glVertex3f(-4.0, 0.5, -8.0);
    glEnd();

    // square 3, 7
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[3][7] == true)
    {
        if(!gotIndex[59])
        {
            posIndex[59] = moveCounter;
            gotIndex[59] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[59]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-2.0, 0.5, -6.0);
        glTexCoord2f(1, 0); glVertex3f(-0.0, 0.5, -6.0);
        glTexCoord2f(1, 1); glVertex3f(-0.0, 0.5, -8.0);
        glTexCoord2f(0, 1); glVertex3f(-2.0, 0.5, -8.0);
    glEnd();

    // square 4, 7
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[4][7] == true)
    {
        if(!gotIndex[60])
        {
            posIndex[60] = moveCounter;
            gotIndex[60] = true;
        }

        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[60]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(0.0, 0.5, -6.0);
        glTexCoord2f(1, 0); glVertex3f(2.0, 0.5, -6.0);
        glTexCoord2f(1, 1); glVertex3f(2.0, 0.5, -8.0);
        glTexCoord2f(0, 1); glVertex3f(0.0, 0.5, -8.0);
    glEnd();

    // square 5, 7
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[5][7] == true)
    {
        if(!gotIndex[61])
        {
            posIndex[61] = moveCounter;
            gotIndex[61] = true;
        }

        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[61]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(2.0, 0.5, -6.0);
        glTexCoord2f(1, 0); glVertex3f(4.0, 0.5, -6.0);
        glTexCoord2f(1, 1); glVertex3f(4.0, 0.5, -8.0);
        glTexCoord2f(0, 1); glVertex3f(2.0, 0.5, -8.0);
    glEnd();

    // square 6, 7
    glBindTexture(GL_TEXTURE_2D, textureWhiteSquare->texID);
    if(boardTourMap[6][7] == true)
    {
        if(!gotIndex[62])
        {
            posIndex[62] = moveCounter;
            gotIndex[62] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[62]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(4.0, 0.5, -6.0);
        glTexCoord2f(1, 0); glVertex3f(6.0, 0.5, -6.0);
        glTexCoord2f(1, 1); glVertex3f(6.0, 0.5, -8.0);
        glTexCoord2f(0, 1); glVertex3f(4.0, 0.5, -8.0);
    glEnd();

    // square 7, 7
    glBindTexture(GL_TEXTURE_2D, textureBlackSquare->texID);
    if(boardTourMap[7][7] == true)
    {
        if(!gotIndex[63])
        {
            posIndex[63] = moveCounter;
            gotIndex[63] = true;
        }
        glBindTexture(GL_TEXTURE_2D, textureNumbers[posIndex[63]-1]->texID);
    }
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(6.0, 0.5, -6.0);
        glTexCoord2f(1, 0); glVertex3f(8.0, 0.5, -6.0);
        glTexCoord2f(1, 1); glVertex3f(8.0, 0.5, -8.0);
        glTexCoord2f(0, 1); glVertex3f(6.0, 0.5, -8.0);
    glEnd();

    //draw a wood floor
    glBindTexture(GL_TEXTURE_2D, textureFloor->texID);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-12.0, 0,  12.0);
        glTexCoord2f(1, 0); glVertex3f( 12.0, 0,  12.0);
        glTexCoord2f(1, 1); glVertex3f( 12.0, 0, -12.0);
        glTexCoord2f(0, 1); glVertex3f(-12.0, 0, -12.0);
    glEnd();
}

void GLWidget::updateBoardMap(float x, float z)
{
    // clear the map
    for(int i = 0; i < 8; i++)
        for(int j = 0; j < 8; j++)
            boardMap[i][j] = false;

    // our mouse is not over the board
    if( x > 8.0 || x < -8.0 || z > 8.0 || z < -8.0)
        return;

    int xCoord, zCoord;
    int xCount = 1;
    int zCount = 1;
    bool negX = false;
    bool negZ = false;

    // do some clever arithmetic to find what square we are over
    // all based on the fact that we know each square is 2x2 units
    if(x < 0)
    {
        x = -x;
        negX = true;
    }
    if(z < 0)
    {
        z = -z;
        negZ = true;
    }
    while(x > 2)
    {
        x -= 2;
        xCount++;
    }
    while(z > 2)
    {
        z -= 2;
        zCount++;
    }
    xCoord = xCount;
    zCoord = zCount;
    if(negX)
        xCoord = -xCoord;
    if(negZ)
        zCoord = -zCoord;

    if(negX)
        xCoord = 4 + xCoord;
    else
        xCoord = 3 + xCoord;
    if(negZ)
        zCoord = 3 + -(zCoord);
    else
        zCoord = 4 - zCoord;

    boardMap[xCoord][zCoord] = true;
}

void GLWidget::knightsTour(int column, int row)
{
    vector<int> path;
    int available[128];

    for(int i = 0; i < 128; i++)
        available[i] = 1;

    available[16 * row + column] = 0;

    path.reserve(64);
    path.push_back(16 * row + column);

    // the 8 ways the knight can move
    static const int directionNNW = -1+32;
    static const int directionNNE =  1+32;
    static const int directionWNW = -2+16;
    static const int directionENE =  2+16;
    static const int directionWSW = -2-16;
    static const int directionESE =  2-16;
    static const int directionSSW = -1-32;
    static const int directionSSE =  1-32;

    // store the moves in an array for easy access
    // they are listed in highest priority order,
    // this priority garuntees a closed tour for every starting square on the board
    int knightJumps[8];
    knightJumps[0] = directionWNW;
    knightJumps[1] = directionNNW;
    knightJumps[2] = directionWSW;
    knightJumps[3] = directionSSW;
    knightJumps[4] = directionESE;
    knightJumps[5] = directionSSE;
    knightJumps[6] = directionNNE;
    knightJumps[7] = directionENE;

    int currentPosition = *path.rbegin();
    int pickScore, pickPosition = 0;
    int testPosition, testScore;

    // this implementation uses Warnsdorff algorithm (fixed priority)
    // as well as the 0x88 trick for chess boards
    while(path.size() < 64)
    {
        pickScore = 99;

        testPosition = currentPosition + directionWNW;
        if((testPosition & 0x88) == 0 && available[testPosition] == 1)
        {
            testScore = 0;
            // check all possible valid moves (excluding the one we jumped from)
            for(int i = 0; i < 8; i++)
            {
                if(knightJumps[i] == directionESE)
                    continue;
                // only index valid squares on the board
                if(((testPosition + knightJumps[i]) & 0x88) == 0)
                    testScore += available[testPosition + knightJumps[i]];
            }
            if(testScore < pickScore)
            {
                pickScore = testScore;
                pickPosition = testPosition;
            }
        }

        testPosition = currentPosition + directionNNW;
        if((testPosition & 0x88) == 0 && available[testPosition] == 1)
        {
            testScore = 0;
            for(int i = 0; i < 8; i++)
            {
                if(knightJumps[i] == directionSSE)
                    continue;
                if(((testPosition + knightJumps[i]) & 0x88) == 0)
                    testScore += available[testPosition + knightJumps[i]];
            }
            if(testScore < pickScore)
            {
                pickScore = testScore;
                pickPosition = testPosition;
            }
        }

        testPosition = currentPosition + directionWSW;
        if((testPosition & 0x88) == 0 && available[testPosition] == 1)
        {
            testScore = 0;
            for(int i = 0; i < 8; i++)
            {
                if(knightJumps[i] == directionENE)
                    continue;
                if(((testPosition + knightJumps[i]) & 0x88) == 0)
                    testScore += available[testPosition + knightJumps[i]];
            }
            if(testScore < pickScore)
            {
                pickScore = testScore;
                pickPosition = testPosition;
            }
        }

        testPosition = currentPosition + directionSSW;
        if((testPosition & 0x88) == 0 && available[testPosition] == 1)
        {
            testScore = 0;
            for(int i = 0; i < 8; i++)
            {
                if(knightJumps[i] == directionNNE)
                    continue;
                if(((testPosition + knightJumps[i]) & 0x88) == 0)
                    testScore += available[testPosition + knightJumps[i]];
            }
            if(testScore < pickScore)
            {
                pickScore = testScore;
                pickPosition = testPosition;
            }
        }

        testPosition = currentPosition + directionESE;
        if((testPosition & 0x88) == 0 && available[testPosition] == 1)
        {
            testScore = 0;
            for(int i = 0; i < 8; i++)
            {
                if(knightJumps[i] == directionWNW)
                    continue;
                if(((testPosition + knightJumps[i]) & 0x88) == 0)
                    testScore += available[testPosition + knightJumps[i]];
            }
            if(testScore < pickScore)
            {
                pickScore = testScore;
                pickPosition = testPosition;
            }
        }

        testPosition = currentPosition + directionSSE;
        if((testPosition & 0x88) == 0 && available[testPosition] == 1)
        {
            testScore = 0;
            for(int i = 0; i < 8; i++)
            {
                if(knightJumps[i] == directionNNW)
                    continue;
                if(((testPosition + knightJumps[i]) & 0x88) == 0)
                    testScore += available[testPosition + knightJumps[i]];
            }
            if(testScore < pickScore)
            {
                pickScore = testScore;
                pickPosition = testPosition;
            }
        }

        testPosition = currentPosition + directionNNE;
        if((testPosition & 0x88) == 0 && available[testPosition] == 1)
        {
            testScore = 0;
            for(int i = 0; i < 8; i++)
            {
                if(knightJumps[i] == directionSSW)
                    continue;
                if(((testPosition + knightJumps[i]) & 0x88) == 0)
                    testScore += available[testPosition + knightJumps[i]];
            }
            if(testScore < pickScore)
            {
                pickScore = testScore;
                pickPosition = testPosition;
            }
        }

        testPosition = currentPosition + directionENE;
        if((testPosition & 0x88) == 0 && available[testPosition] == 1)
        {
            testScore = 0;
            for(int i = 0; i < 8; i++)
            {
                if(knightJumps[i] == directionWSW)
                    continue;
                if(((testPosition + knightJumps[i]) & 0x88) == 0)
                    testScore += available[testPosition + knightJumps[i]];
            }
            if(testScore < pickScore)
            {
                pickScore = testScore;
                pickPosition = testPosition;
            }
        }

        currentPosition = pickPosition;
        available[currentPosition] = 0;
        path.push_back(currentPosition);
    }

    // convert the path to actual points, and store in finalKnightPath
    QPoint temp;
    for(int i = 0; i < 64; ++i)
    {
        temp.setX(path[i]%16);
        temp.setY(path[i]/16);
        finalKnightPath.push_back(temp);
    }
    doneTouring = true;
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void GLWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    for(int i = 0; i < 8; i++)
        for(int j = 0; j < 8; j++)
            if(boardMap[i][j] == true && !doneTouring)
            {
                knightsTour(i, j);
                return;
            }
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        int dx = event->x() - lastPos.x();
        int dy = event->y() - lastPos.y();
        xRot += dy * swivleSensitivity;
        yRot += dx * swivleSensitivity;
        lastPos = event->pos();
        updateGL();
    }

    // update the boardMap from screen coordinates
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;
    GLdouble modelMatrix[16];
    GLdouble projMatrix[16];
    GLint viewport[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
    glGetIntegerv(GL_VIEWPORT, viewport);
    winX = (float)event->pos().x();
    winY = (float)viewport[3] - (float)event->pos().y();
    glReadPixels(event->pos().x(), (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
    gluUnProject(winX, winY, winZ, modelMatrix, projMatrix, viewport, &posX, &posY, &posZ);

    updateBoardMap((float)posX, (float)posZ);
}

void GLWidget::keyPressEvent(QKeyEvent* event)
{
    switch(event->key())
    {
        case Qt::Key_Escape:
            close();
            break;

        // zoom in
        case Qt::Key_W:
        {
            float xRotRad, yRotRad;
            yRotRad = (yRot / 180 * M_PI);
            xRotRad = (xRot / 180 * M_PI);
            xPos += float(sin(yRotRad)) * zoomSensitivity;
            zPos -= float(cos(yRotRad)) * zoomSensitivity;
            yPos -= float(sin(xRotRad)) * zoomSensitivity;
            updateGL();
            break;
        }

        // zoom out
        case Qt::Key_S:
        {
            float xRotRad, yRotRad;
            yRotRad = (yRot / 180 * M_PI);
            xRotRad = (xRot / 180 * M_PI);
            xPos -= float(sin(yRotRad)) * zoomSensitivity;
            zPos += float(cos(yRotRad)) * zoomSensitivity;
            yPos += float(sin(xRotRad)) * zoomSensitivity;
            updateGL();
            break;
        }

        default:
            event->ignore();
            break;
    }
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    int delta = event->delta();

    if(event->orientation() == Qt::Vertical)
    {
        if(delta > 0) // we are scrolling in - zoom in
        {
            float xRotRad, yRotRad;
            yRotRad = (yRot / 180 * M_PI);
            xRotRad = (xRot / 180 * M_PI);
            xPos += float(sin(yRotRad)) * zoomSensitivity;
            zPos -= float(cos(yRotRad)) * zoomSensitivity;
            yPos -= float(sin(xRotRad)) * zoomSensitivity;
            updateGL();
        }
        else if(delta < 0) // we are scrolling out - zoom out
        {
            float xRotRad, yRotRad;
            yRotRad = (yRot / 180 * M_PI);
            xRotRad = (xRot / 180 * M_PI);
            xPos -= float(sin(yRotRad)) * zoomSensitivity;
            zPos += float(cos(yRotRad)) * zoomSensitivity;
            yPos += float(sin(xRotRad)) * zoomSensitivity;
            updateGL();
        }
    }
}

void GLWidget::calculateFPS()
{
    frame++;
    if(frame == 100)
    {
        fps = clock->elapsed();
        fps /= 1000;
        fps = 100 / fps;
        clock->restart();
        frame = 0;
    }
}
