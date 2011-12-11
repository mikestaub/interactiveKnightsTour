#include "texture.h"

vector <Texture *> Texture::textures;

Texture::Texture(string in_filename, string in_name, int in_mode)
{
    imageData = NULL;

    if(!loadTGA(in_filename))
    {
        //failed to load texture file
        exit(4);
    }

    name = in_name;
        mode = in_mode;

    textures.push_back(this);
}

Texture::~Texture()
{
    for(vector<Texture *>::iterator it = textures.begin(); it != textures.end(); it++)
        if((*it) == this)
            textures.erase(it);

    if(imageData)
        delete imageData;
}

int Texture::getMode(void)
{
        return mode;
}

bool Texture::loadTGA(string filename)
{
    TGA_Header TGAheader;

    ifstream file(filename.data(), std::ios_base::binary);

    if(!file.is_open())
        return false;

    if( !file.read((char*)&TGAheader, sizeof(TGAheader)))
        return false;

    //needs to be uncompressed
    if(TGAheader.ImageType != 2)
        return false;

    width = TGAheader.ImageWidth;
    height = TGAheader.ImageHeight;
    bpp = TGAheader.PixelDepth;

    if(width <= 0 ||
        height <= 0 ||
        (bpp != 24 && bpp != 32))
    {
        return false;
    }

    GLuint type = GL_RGBA;

    if(bpp == 24)
        type = GL_RGB;

    GLuint bytesPerPixel = bpp / 8;
    GLuint imageSize = width * height * bytesPerPixel;

    imageData = new GLubyte[imageSize];
    //memory allocated ?
    if(imageData == NULL)
        return false;

    if(!file.read((char*)imageData, imageSize))
    {
        delete imageData;
        return false;
    }

    //BGR -> RGB   TGA format stores in reverse
    for(GLuint i = 0; i < (int)imageSize; i += bytesPerPixel)
    {
        GLuint temp = imageData[i];
        imageData[i] = imageData[i+2];
        imageData[i+2] = temp;
    }

        //quick test to see if the texture file was flipped
        bool flipH = ((TGAheader.ImageDescriptor & 0x10) == 0x10);
    bool flipV = ((TGAheader.ImageDescriptor & 0x20) == 0x20);

        flipImage(imageData, flipH, flipV, width, height, bpp);

    createTexture(imageData, width, height, type);

    //everything worked!
    return true;
}

bool Texture::createTexture(unsigned char *imageData, int width, int height, int type)
{

    glGenTextures(1, &texID);

    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, imageData);

    return true;
}

void Texture::flipImage(unsigned char * image, bool flipHorizontal, bool flipVertical, GLushort width, GLushort height, GLbyte bpp)
{
    GLbyte Bpp = bpp / 8;

    if(flipHorizontal)
        {
        for(int h = 0; h < height; h++)
                {
            for(int w = 0; w < width / 2; w++)
                        {
                swap(image + (h * width + w) * Bpp, image + (h * width + width - w - 1)* Bpp, Bpp);
            }
        }
    }

    if(flipVertical)
        {
        for(int w = 0; w < width; w++)
                {
            for(int h = 0; h < height / 2; h++)
                        {
                swap(image + (h * width + w) * Bpp, image + ((height - h - 1) * width + w)* Bpp, Bpp);
            }
        }
    }
}

void Texture::swap(unsigned char* ori, unsigned char* dest, GLint size)
{
    GLubyte* temp = new unsigned char[size];

    memcpy(temp, ori, size);
    memcpy(ori, dest, size);
    memcpy(dest, temp, size);

   delete[] temp;
}
