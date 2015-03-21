#include "KPTexture.h"

#include "KPHelper.h"

extern void (*my_glGetIntegerv)(GLenum, GLint*);

KPTexture::KPTexture() : KPGLObject(), m_texType(TEX_NONE)
{
	
}

KPTexture::KPTexture(u32 id) : KPGLObject(id), m_texType(TEX_NONE)
{
	
}

KPTexture::~KPTexture()
{
	clearData();
}

void KPTexture::clearData()
{
	m_texType = TEX_NONE;
	for (int i = 0; i < MAX_MIPMAP_LEVEL_NUMBER; i++)
	{
		m_pMipmaps[i].reset();
	}
}

KPMessage* KPTexture::toMessage()
{
	KPMessage* msg = new KPMessage();
	msg->m_type = KMT_OBJECT_TEXTURE;

	int len = 4/*m_id*/ + 4/*m_texType*/ + 4/*mipmapCount*/;
	
	int mipmapCount = 0;
	for (int i = 0; i < MAX_MIPMAP_LEVEL_NUMBER; i++)
	{
		KPMipmapLevel& mip = m_pMipmaps[i];
		if (mip.m_hasData)
		{
			mipmapCount++;

			len += 4 + 4 + 4 + 1 + 4 + 4 + 1;
			if (mip.m_pPixels)
			{
				len += mip.calculateSize();
			}
		}
	}

	msg->m_length = len;
	msg->m_pData = new char[msg->m_length];
	
	char* buffer = msg->m_pData;
	
	memcpy(buffer, &m_id, 4);			buffer += 4;
	memcpy(buffer, &m_texType, 4);		buffer += 4;
	memcpy(buffer, &mipmapCount, 4);	buffer += 4;

	for (int i = 0; i < MAX_MIPMAP_LEVEL_NUMBER; i++)
	{
		KPMipmapLevel& mip = m_pMipmaps[i];
		if (mip.m_hasData)
		{
			memcpy(buffer, &i, 4);					buffer += 4;
			memcpy(buffer, &mip.m_width, 4);		buffer += 4;
			memcpy(buffer, &mip.m_height, 4);		buffer += 4;
			memcpy(buffer, &mip.m_isCompressed, 1);	buffer += 1;
			memcpy(buffer, &mip.m_format, 4);		buffer += 4;
			memcpy(buffer, &mip.m_type, 4);			buffer += 4;

			if (mip.m_pPixels)
			{
				*buffer = 1; buffer++;

				const int mipSize = mip.calculateSize();
				memcpy(buffer, mip.m_pPixels, mipSize);

				if (!mip.m_isCompressed)
				{
					switch (mip.m_type)
					{
						/*case GL_UNSIGNED_SHORT_4_4_4_4:
						case GL_UNSIGNED_SHORT_5_5_5_1:
						case GL_UNSIGNED_SHORT_5_6_5:
						break;*/

						case GL_UNSIGNED_BYTE:
						{
							switch (mip.m_format)
							{
							case GL_RGBA:
								KPHelper::swapRB(buffer, mip.m_width * mip.m_height, true);
								break;
							case GL_RGB:
								KPHelper::swapRB(buffer, mip.m_width * mip.m_height, false);
								break;
								/*case GL_LUMINANCE_ALPHA:
								break;
								case GL_LUMINANCE:
								case GL_ALPHA:
								break;*/
							}
							break;
						}
					} // switch
				}

				buffer += mipSize;
			}
			else
			{
				*buffer = 0; buffer++;
			}

			
		}
	}

	return msg;
}

KPTextureType KPTexture::getTexType()
{
	return m_texType;
}

void KPTexture::setTexType(KPTextureType texType)
{
	m_texType = texType;
}

int KPTexture::getMipmapCount()
{
	int count = 0;
	for (int i = 0; i < MAX_MIPMAP_LEVEL_NUMBER; i++)
	{
		if (m_pMipmaps[i].m_hasData) count++;
	}
	return count;
}

void KPTexture::on_glTexImage2D(GLint level, GLint internalformat,
						GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels,
						bool isRecordPixelData)
{
	KPMipmapLevel& mip = m_pMipmaps[level];
	mip.reset();

	mip.m_isCompressed = 0;

	mip.m_format = internalformat;
	mip.m_width = width;
	mip.m_height = height;

	mip.m_type = type;

	if (isRecordPixelData)
	{
		const int imageSize = mip.calculateSize();
		if (imageSize > 0)
		{
			mip.m_pPixels = new char[imageSize];
			if (pixels)
			{
				KPHelper::copyAlignedPixelsData(mip.m_pPixels, pixels, width, height, internalformat, type);
			}
			else
			{
				memset(mip.m_pPixels, 0, imageSize);
			}
		}
	}

	mip.m_hasData = true;
}

void KPTexture::on_glCompressedTexImage2D(GLint level, GLenum internalformat, GLsizei width, GLsizei height,
						GLint border, GLsizei imageSize, const GLvoid* data, bool isRecordPixelData)
{
	KPMipmapLevel& mip = m_pMipmaps[level];
	mip.reset();

	mip.m_isCompressed = 1;

	mip.m_format = internalformat;
	mip.m_width = width;
	mip.m_height = height;

	mip.m_type = (u32)imageSize;

	if (isRecordPixelData)
	{
		if (imageSize > 0)
		{
			mip.m_pPixels = new char[imageSize];
			if (data)
			{
				memcpy(mip.m_pPixels, data, imageSize);
			}
			else
			{
				memset(mip.m_pPixels, 0, imageSize);
			}
		}
	}

	mip.m_hasData = true;
}

void KPTexture::on_glTexSubImage2D(GLint level,
									GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
									GLenum format, GLenum type, const GLvoid* pixels)
{
	if (width == 0 || height == 0 || !pixels) return;

	KPMipmapLevel& mip = m_pMipmaps[level];

	if (mip.m_width == 0 || mip.m_height == 0 || !mip.m_pPixels) return;

	// Prepare
	int unPackAlignment = 4;
	my_glGetIntegerv(GL_UNPACK_ALIGNMENT, &unPackAlignment);

	const int k_dstBytesPP = mip.getBPP() / 8;

	int srcBytesPerRowAligned = width * KPMipmapLevel::getTexBPP(format, type) / 8;
	while (srcBytesPerRowAligned % unPackAlignment != 0) srcBytesPerRowAligned++;
	
	// Now copy
	const u8* src = (const u8*)pixels;

	for (int row = yoffset; row < yoffset + height; row++)
	{
		// TODO: row order???
		//int row2 = mip.m_height - 1 - row;
		int row2 = row;
		u8* dst = (u8*)mip.m_pPixels + (row2 * mip.m_width + xoffset) * k_dstBytesPP;
		
		KPHelper::copyTexSubDataRow(dst, src, width, format, type, mip.m_type);

		src += srcBytesPerRowAligned;
	}
}

KPMipmapLevel* KPTexture::getMip(int level)
{
	return &m_pMipmaps[level];
}

void KPTexture::copyMipmapDataFromRGBA(int mipLevel, int width, int height, const char* pixels)
{
	if (mipLevel < 0 || mipLevel >= MAX_MIPMAP_LEVEL_NUMBER) return;
	
	KPMipmapLevel& mip = m_pMipmaps[mipLevel];
	if (!mip.m_hasData) return;

	if (mip.m_width != width || mip.m_height != height || mip.m_isCompressed) return;
	
	if (mip.m_type == GL_UNSIGNED_SHORT_5_6_5 || mip.m_type == GL_UNSIGNED_BYTE)
	{
		if (mip.m_type == GL_UNSIGNED_SHORT_5_6_5)
		{
			KPHelper::convert_RGBA_to_565(width, height, pixels, mip.m_pPixels);
		}
		else if (mip.m_format == GL_RGB)
		{
			KPHelper::convert_RGBA_to_RGB(width, height, pixels, mip.m_pPixels);
		}
		else if (mip.m_format == GL_RGBA)
		{
			memcpy(mip.m_pPixels, pixels, width * height * 4);
		}
	}
}
