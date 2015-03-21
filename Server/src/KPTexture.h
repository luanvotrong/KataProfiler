#ifndef _KP_TEXTURE_H_
#define _KP_TEXTURE_H_

#include "KPGLObject.h"

class KPMipmapLevel
{
public:
	int m_width;
	int m_height;

	u8 m_isCompressed;

	int m_format;
	u32 m_type;

	char* m_pPixels;

	bool m_hasData;

	KPMipmapLevel()
	{
		m_pPixels = NULL;
		reset();
	}

	~KPMipmapLevel()
	{
		reset();
	}

	void reset()
	{
		m_width = 0;
		m_height = 0;
		m_isCompressed = 0;
		m_format = 0;
		m_type = 0;
		SAFE_DEL_ARRAY(m_pPixels);
		m_hasData = false;
	}

	int calculateSize()
	{
		return m_isCompressed == 1 ? (int)m_type : calculateTexSize(m_format, m_width, m_height, m_type);
	}
	int getBPP()
	{
		if (m_isCompressed) return getCompressedTexBPP(m_format);
        return getTexBPP(m_format, m_type);
	}

	// static

	static int calculateTexSize(GLint internalformat, GLsizei width, GLsizei height, GLenum type)
	{
		return width * height * getTexBPP(internalformat, type) / 8;
	}
	static int getTexBPP(GLint internalformat, GLenum type)
	{
		switch (type)
		{
			case GL_UNSIGNED_SHORT_4_4_4_4:
			case GL_UNSIGNED_SHORT_5_5_5_1:
			case GL_UNSIGNED_SHORT_5_6_5:
				return 16;

			case GL_UNSIGNED_BYTE:
			{
				switch (internalformat)
				{
				case GL_RGBA:
					return 32;
				case GL_RGB:
					return 24;
				case GL_LUMINANCE_ALPHA:
					return 16;
				case GL_LUMINANCE:
				case GL_ALPHA:
					return 8;
				}
			}
		}
		return 0;
	}
	static int getCompressedTexBPP(GLint internalformat)
	{
		switch (internalformat)
        {
			case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
            case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
                return 2;

            case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
            case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
			case GL_ETC1_RGB8_OES:
			case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
			case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
                return 4;
            
			case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
			case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
				return 8;
        }
        return 0;
	}
};

#define MAX_MIPMAP_LEVEL_NUMBER 32

enum KPTextureType
{
	TEX_NONE = 0,
	TEX_2D,
	TEX_CUBE_MAP
};

class KPTexture : public KPGLObject
{
public:
	KPTexture();
	KPTexture(u32 id);

	~KPTexture();

	//
	void clearData();
	KPMessage* toMessage();

	//
	void on_glTexImage2D(GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels, bool isRecordPixelData);
	void on_glCompressedTexImage2D(GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data, bool isRecordPixelData);

	void on_glTexSubImage2D(GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);

	void copyMipmapDataFromRGBA(int mipLevel, int width, int height, const char* pixels);
	
	//
	int getMipmapCount();

	KPMipmapLevel* getMip(int level);
	
	KPTextureType getTexType();
	void setTexType(KPTextureType texType);

private:
	KPTextureType m_texType;
	KPMipmapLevel m_pMipmaps[MAX_MIPMAP_LEVEL_NUMBER];
};

#endif
