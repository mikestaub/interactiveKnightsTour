#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <vector>
#include <fstream>
#include "GLWidget.h"

using std::string;
using std::vector;
using std::ifstream;

enum TEXTURE_MODES
{
    MODE_NORMAL = 0,
    MODE_DARKEN
};

struct TGA_Header
{
    GLubyte ID_Length;
    GLubyte ClorMapType;
    GLubyte ImageType;
    GLubyte colorMapSpecification[5];
    GLushort xOrigin;
    GLushort yOrigin;
    GLushort ImageWidth;
    GLushort ImageHeight;
    GLubyte PixelDepth;
    GLubyte ImageDescriptor;
};

class Texture
{

    //Functions
public:
    Texture(string filename, string name = "", int mode = MODE_NORMAL);
    ~Texture();
    int getMode(void);

    //Variables
public:
    unsigned char *imageData;
    unsigned int bpp;
    unsigned int width;
    unsigned int height;
    unsigned int texID;

    bool flipH;
    bool flipV;

    //these two methods are due to strange storing methods of TGA files
    void flipImage(unsigned char* image, bool flipHorizontal,
    bool flipVertical, GLushort width, GLushort height, GLbyte bpp);
    void swap(unsigned char * ori, unsigned char* dest, GLint size);

    string name;

    //global container for all our textures
    static vector<Texture *> textures;

private:
    bool loadTGA(string filename);
    bool createTexture(unsigned char *imageData, int width, int height, int type);

    int mode;

};

#endif // TEXTURE_H
