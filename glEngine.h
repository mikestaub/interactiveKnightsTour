#ifndef GLENGINE_H
#define GLENGINE_H

#include "Light.h"
//#include "Texture.h"

//macro for ease of access
#define iGLEngine GLEngine::getEngine()

class GLEngine
{
public:
    GLEngine();
    ~GLEngine();

    static GLEngine* getEngine(void);

    void Initialize(GLint width, GLint height);
    static GLvoid Uninitialize(void);

    void buildTextureFont(void);
    void drawText(GLint x, GLint y, const char* text, ...);

    GLuint getTextWidth(const char* text);
    GLuint getTextHeight(const char* text);

private:
    GLuint fontBase;

    //Texture* fontTexture;

};

#endif // GLENGINE_H
