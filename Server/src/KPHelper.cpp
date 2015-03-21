#include "KPHelper.h"

#include <unistd.h>
#include <sys/socket.h>

#include "KPProgram.h"

#include "KPTexture.h"

extern void (*my_glGetIntegerv)(GLenum, GLint*);
extern void (*my_glGetFloatv)(GLenum, GLfloat*);
extern void (*my_glReadPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid*);
extern void (*my_glPixelStorei)(GLenum, GLint);
extern GLenum (*my_glGetError)(void);
extern GLboolean (*my_glIsProgram)(GLuint);
extern GLboolean (*my_glIsShader)(GLuint);
extern void (*my_glGetProgramiv)(GLuint, GLenum, GLint*);
extern void (*my_glGetAttachedShaders)(GLuint, GLsizei, GLsizei*, GLuint*);
extern GLuint (*my_glCreateProgram)(void);
extern GLuint (*my_glCreateShader)(GLenum);
extern void (*my_glDeleteProgram)(GLuint);
extern void (*my_glDeleteShader)(GLuint);
extern void (*my_glAttachShader)(GLuint, GLuint);
extern void (*my_glShaderSource)(GLuint, GLsizei, const GLchar**, const GLint*);
extern void (*my_glGetShaderiv)(GLuint, GLenum, GLint*);
extern void (*my_glGetShaderInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
extern void (*my_glGetProgramInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
extern void (*my_glLinkProgram)(GLuint);
extern void (*my_glCompileShader)(GLuint);
extern void (*my_glActiveTexture)(GLenum);
extern void (*my_glFinish)(void);
extern void (*my_glFlush)(void);
extern void (*my_glGetFramebufferAttachmentParameteriv)(GLenum, GLenum, GLenum, GLint*);

//
extern void (*my_glUniform1fv)(GLint, GLsizei, const GLfloat*);
extern void (*my_glUniform1iv)(GLint, GLsizei, const GLint*);
extern void (*my_glUniform2fv)(GLint, GLsizei, const GLfloat*);
extern void (*my_glUniform2iv)(GLint, GLsizei, const GLint*);
extern void (*my_glUniform3fv)(GLint, GLsizei, const GLfloat*);
extern void (*my_glUniform3iv)(GLint, GLsizei, const GLint*);
extern void (*my_glUniform4fv)(GLint, GLsizei, const GLfloat*);
extern void (*my_glUniform4iv)(GLint, GLsizei, const GLint*);
extern void (*my_glUniformMatrix2fv)(GLint, GLsizei, GLboolean, const GLfloat*);
extern void (*my_glUniformMatrix3fv)(GLint, GLsizei, GLboolean, const GLfloat*);
extern void (*my_glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat*);

//
pthread_mutex_t KPHelper::m_sMutexes[MAX_MUTEXES];

int KPHelper::sendMessageToClient(int client, KPMessage& msg)
{
	int tmp = (int)msg.m_type;
	LOGD("sendMessageToClient: type = %d, length = %d", tmp, msg.m_length);

	int ret = SOCKET_SUCCESS;

	if ( send(client, &tmp, 4, 0) == SOCKET_ERROR )
	{
		ret = SOCKET_ERROR;
		goto my_end;
	}

	if ( send(client, &msg.m_length, 4, 0) == SOCKET_ERROR )
	{
		ret = SOCKET_ERROR;
		goto my_end;
	}

	if (msg.m_length > 0 && msg.m_pData)
	{
		if ( send(client, msg.m_pData, msg.m_length, 0) == SOCKET_ERROR )
		{
			ret = SOCKET_ERROR;
			goto my_end;
		}
	}

	my_end:

	if (ret == SOCKET_ERROR)
	{
		LOGI("[%s] Sent failed, client socket id = %d, stop capturing", __FUNCTION__, client);
		closeSocket(client);
	}

	return ret;
}

int KPHelper::sendGLObjectToClient(int client, KPGLObject& obj)
{
	KPMessage* msg = obj.toMessage();
	int result = sendMessageToClient(client, *msg);
	SAFE_DEL(msg);
	return result;
}

int KPHelper::sendRequestFinishMessageToClient(int client)
{
	KPMessage msg;
	msg.m_type = KMT_REQUEST_FINISH;
	return sendMessageToClient(client, msg);
}

int KPHelper::sendRequestErrorMessageToClient(int client, const char* error /* = NULL */ )
{
	KPMessage msg;
	msg.m_type = KMT_REQUEST_ERROR;

	if (error)
	{
		msg.m_length = strlen(error);
		if (msg.m_length > 0)
		{
			msg.m_pData = new char[msg.m_length];
			memcpy(msg.m_pData, error, msg.m_length);
		}
		else
		{
			msg.m_length = 0;
		}
	}

	return sendMessageToClient(client, msg);
}

int KPHelper::sendCurrentTexturesStatus(int client)
{
	const int k_texUnitNum = 32;
	KPMessage msg;
	msg.m_type = KMT_STATE_CURRENT_TEXTURES_STATUS;
	msg.m_length = k_texUnitNum * 4 * 2 +
						1/*1 byte for current active tex unit (0-31)*/;
	msg.m_pData = new char[msg.m_length];
	char* buffer = msg.m_pData;

	int max = 0;
	my_glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max);
	if (max > k_texUnitNum)
	{
		max = k_texUnitNum;
	}

	u8 activeTexUnit = getCurrentActiveTextureUnit();
	for (int i = 0; i < max; i++)
	{
		my_glActiveTexture(GL_TEXTURE0 + i);

		u32 texId_2D = getCurrentBindingTexture2D();
		memcpy(buffer, &texId_2D, 4);
		buffer += 4;

		u32 texId_CubeMap = getCurrentBindingTextureCubeMap();
		memcpy(buffer, &texId_CubeMap, 4);
		buffer += 4;

		LOGI("texUnit = %d, texId_2D = %d, texId_CubeMap = %d", i, texId_2D, texId_CubeMap);
	}
	for (int i = max; i < k_texUnitNum; i++)
	{
		u32 texId = 0;
		memcpy(buffer, &texId, 4); buffer += 4;
		memcpy(buffer, &texId, 4); buffer += 4;
	}
	my_glActiveTexture(GL_TEXTURE0 + activeTexUnit);
	memcpy(buffer, &activeTexUnit, 1);
	
	return sendMessageToClient(client, msg);
}

int KPHelper::sendCurrentUsingProgram(int client)
{
	u32 progId = getCurrentUsingProgram();

	KPMessage msg;
	msg.m_type = KMT_STATE_CURRENT_USING_PROGRAM;
	msg.m_length = 4;
	msg.m_pData = new char[msg.m_length];
	memcpy(msg.m_pData, &progId, 4);

	return sendMessageToClient(client, msg);
}

int KPHelper::sendCurrentBindingVbos(int client)
{
	u32 vboArrayBuffer = getCurrentBindingVbo_ArrayBuffer();
	u32 vboElementArrayBuffer = getCurrentBindingVbo_ElementArrayBuffer();

	KPMessage msg;
	msg.m_type = KMT_STATE_CURRENT_BINDING_VBOS;
	msg.m_length = 4 + 4;
	msg.m_pData = new char[msg.m_length];
	memcpy(msg.m_pData, &vboArrayBuffer, 4);
	memcpy(msg.m_pData + 4, &vboElementArrayBuffer, 4);

	return sendMessageToClient(client, msg);
}

void KPHelper::closeSocket(int sockId)
{
	close(sockId);
}

const int MAX_RETRY_COUNT = 10;

bool KPHelper::recvSocket(int sockId, void* buffer, int len)
{
	char* buf = (char*)buffer;
	int bytesRead = 0;
	int retryCount = 0;
	while (bytesRead < len)
	{
		int r = recv(sockId, buf, len - bytesRead, 0);
		if (r < 0) return false;
		if (r == 0)
		{
			retryCount++;
			if (retryCount > MAX_RETRY_COUNT)
			{
				LOGI("Time out in [%s] socket id = %d", __FUNCTION__, sockId);
				return false;
			}
		}
		else
		{
			retryCount = 0;
		}
		buf += r;
		bytesRead += r;
		KPHelper::assert(bytesRead <= len);
	}

	return true;
}

bool KPHelper::setSocketRecvTimeout(int sockId, int timeoutMs)
{
	struct timeval timeout;
	timeout.tv_sec = timeoutMs / 1000;
	timeout.tv_usec = (timeoutMs % 1000) * 1000;

    if (setsockopt(sockId, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
	{
		LOGE("setsockopt failed SO_RCVTIMEO, timeoutMs = %d", timeoutMs);
		return false;
	}

	return true;
}

void KPHelper::initMutexes()
{
	for (int i = 0; i < MAX_MUTEXES; i++)
	{
		pthread_mutex_init(&m_sMutexes[i], NULL);
	}
}

void KPHelper::destroyMutexes()
{
	for (int i = 0; i < MAX_MUTEXES; i++)
	{
		pthread_mutex_destroy(&m_sMutexes[i]);
	}
}

void KPHelper::LockMutex0()
{
	pthread_mutex_lock(&m_sMutexes[0]);
}

void KPHelper::UnlockMutex0()
{
	pthread_mutex_unlock(&m_sMutexes[0]);
}

void KPHelper::LockMutex1()
{
	pthread_mutex_lock(&m_sMutexes[1]);
}

void KPHelper::UnlockMutex1()
{
	pthread_mutex_unlock(&m_sMutexes[1]);
}

bool KPHelper::assert(bool b, const char* szError)
{
	if (!b)
	{
		if (szError)
		{
			LOGE("Assert Error: %s", szError);
		}
		else
		{
			LOGE("Assert Error");
		}
		return false;
	}
	return true;
}

int KPHelper::calculateShaderSourceSize(GLsizei count, const GLchar** string, const GLint* length)
{
	int sourceLen = 0;
	if (length)
	{
		for (int i = 0; i < count; i++)
		{
			const int len = length[i] < 0 ? strlen(string[i]) : length[i];
			sourceLen += len;
		}
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			sourceLen += strlen(string[i]);
		}
	}
	return sourceLen;
}

void KPHelper::copyShaderSource(char* buffer, GLsizei count, const GLchar** string, const GLint* length)
{
	if (length)
	{
		for (int i = 0; i < count; i++)
		{
			const int len = length[i] < 0 ? strlen(string[i]) : length[i];
			memcpy(buffer, string[i], len);
			buffer += len;
		}
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			const int len = strlen(string[i]);
			memcpy(buffer, string[i], len);
			buffer += len;
		}
	}
}

void KPHelper::getViewport(int& x, int& y, int& width, int& height)
{
	int viewport[4];
	my_glGetIntegerv(GL_VIEWPORT, viewport);
	x = viewport[0];
	y = viewport[1];
	width = viewport[2];
	height = viewport[3];
}

void KPHelper::readPixels(int x, int y, int width, int height, char* dst)
{
	if (width <= 0 || height <= 0)
	{
		return;
	}

	int oldPackAlignment = 4;
	my_glGetIntegerv(GL_PACK_ALIGNMENT, &oldPackAlignment);

	my_glPixelStorei(GL_PACK_ALIGNMENT, 4);

	my_glFlush();
	my_glFinish();

	my_glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, dst);

	my_glPixelStorei(GL_PACK_ALIGNMENT, oldPackAlignment);
}

char* KPHelper::makeScreenshot_RGB(
		const char* src, // RGBA
		int width, int height,
		bool allowScale, bool allowSwapRB,
		int& widthScaled, int& heightScaled,
		int& len
)
{
	const int k_bytesPerPixel_actual = 3; // RGB
	len = width * height * k_bytesPerPixel_actual; // ***

	// Remove alpha to have RGB
	char* buffer = new char[len];
	convert_RGBA_to_RGB(width, height, src, buffer);

	const int k_maxSize = 2 * 1024 * 1024; // 2 MB
	widthScaled		= width;	// ***
	heightScaled	= height;	// ***
	int zoomLevel = 1;

	if (allowScale)
	{
		while ( (widthScaled > 1 && heightScaled > 1) && (widthScaled * heightScaled * k_bytesPerPixel_actual > k_maxSize) )
		{
			widthScaled = widthScaled >> 1; // ***
			heightScaled = heightScaled >> 1; // ***
			zoomLevel = zoomLevel << 1;
		}
	}

	LOGI("zoomLevel = %d, widthScaled = %d, heightScaled = %d", zoomLevel, widthScaled, heightScaled);

	if (zoomLevel > 1)
	{
		len = widthScaled * heightScaled * k_bytesPerPixel_actual; // ***
		char* bufferScaled = new char[len];

		for (int x = 0; x < widthScaled; x++)
		{
			for (int y = 0; y < heightScaled; y++)
			{
				int offset = (y * widthScaled + x) * k_bytesPerPixel_actual;

				int x2 = x * zoomLevel;
				int y2 = y * zoomLevel;
				int offset2 = (y2 * width + x2) * k_bytesPerPixel_actual;

				memcpy(bufferScaled + offset, buffer + offset2, k_bytesPerPixel_actual);
				/*for (int i = 0; i < k_bytesPerPixel_actual; i++)
				{
					bufferScaled[offset + i] = buffer[offset2 + i];
				}*/

			}
		}

		delete [] buffer;
		buffer = bufferScaled;
	}

	if (allowSwapRB)
	{
		swapRB(buffer, widthScaled * heightScaled, false);
	}

	return buffer;
}

char* KPHelper::checkValidModifiedProgram(u32 progId, u32 vsId, u32 fsId,
		const char* vsSourceString, const char* fsSourceString, const KPProgram* pOldProg, KPProgram& testProg)
{
	const int maxLength = 1024;
	char* error = new char[maxLength];
	memset(error, 0, maxLength);

	if (! my_glIsProgram(progId) )
	{
		strcpy(error, "This is not a program id");
		return error;
	}
	if (! my_glIsShader(vsId) )
	{
		strcpy(error, "This is not a vertex shader id");
		return error;
	}
	if (! my_glIsShader(fsId) )
	{
		strcpy(error, "This is not a fragment shader id");
		return error;
	}

	int countShaders;
	my_glGetProgramiv(progId, GL_ATTACHED_SHADERS, &countShaders);
	if (countShaders != 2)
	{
		strcpy(error, "The number of shader objects attached to this program is not equal to 2");
		return error;
	}

	u32 shaders[2];
	my_glGetAttachedShaders(progId, 2, NULL, shaders);
	if ( (vsId != shaders[0] && vsId != shaders[1]) || (fsId != shaders[0] && fsId != shaders[1]) )
	{
		strcpy(error, "The shader objects attached to this program have been changed");
		return error;
	}

	//-------------------------------------------------------------------------------
	// Compile shaders
	int compileStatus = 0;
	
	u32 newVsId = my_glCreateShader(GL_VERTEX_SHADER);
	my_glShaderSource(newVsId, 1, &vsSourceString, NULL);
	my_glCompileShader(newVsId);
	my_glGetShaderiv(newVsId, GL_COMPILE_STATUS, &compileStatus);
	if (!compileStatus)
	{
		int length = 0;
		my_glGetShaderInfoLog(newVsId, maxLength, &length, error);
		if (length <= 0) strcpy(error, "[error]");
		my_glDeleteShader(newVsId);
		return error;
	}

	u32 newFsId = my_glCreateShader(GL_FRAGMENT_SHADER);
	my_glShaderSource(newFsId, 1, &fsSourceString, NULL);
	my_glCompileShader(newFsId);
	my_glGetShaderiv(newFsId, GL_COMPILE_STATUS, &compileStatus);
	if (!compileStatus)
	{
		int length = 0;
		my_glGetShaderInfoLog(newFsId, maxLength, &length, error);
		if (length <= 0) strcpy(error, "[error]");
		my_glDeleteShader(newFsId);
		my_glDeleteShader(newVsId);
		return error;
	}

	//-------------------------------------------------------------------------------
	// Link program
	
	int linkStatus = 0;
	u32 newProgramId = my_glCreateProgram();
	my_glAttachShader(newProgramId, newVsId);
	my_glAttachShader(newProgramId, newFsId);
	my_glLinkProgram(newProgramId);
	my_glGetProgramiv(newProgramId, GL_LINK_STATUS, &linkStatus);

	testProg.setId(newProgramId);

	if (!linkStatus)
	{
		int length = 0;
		my_glGetProgramInfoLog(newProgramId, maxLength, &length, error);
		if (length <= 0) strcpy(error, "[error]");
		goto my_end;
	}
	
	testProg.attachVs(newVsId);
	testProg.attachFs(newFsId);
	testProg.link();

	// Note
#if 0
	if (testProg.getAttributesCount() != pOldProg->getAttributesCount())
	{
		strcpy(error, "The attributes count is not the same");
		goto my_end;
	}
#endif

	//-------------------------------------------------------------------------------
	// Everything is ok, free all

	my_end:
	
	my_glDeleteProgram(newProgramId);
	my_glDeleteShader(newFsId);
	my_glDeleteShader(newVsId);

	return strlen(error) > 0 ? error : NULL;
}

int KPHelper::getSizeInBytesOfGLDataType(u32 type)
{
	switch (type)
	{
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
			return 1;
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
			return 2;
		case GL_INT:
		case GL_UNSIGNED_INT:
		case GL_FLOAT:
		case GL_FIXED:
			return 4;
	}
	assert(false, "KPHelper::getSizeInBytesOfGLDataType");
	return 0;
}

u32 KPHelper::getCurrentBindingTexture2D()
{
	int id = 0;
	my_glGetIntegerv(GL_TEXTURE_BINDING_2D, &id);
	return (u32)id;
}

u32 KPHelper::getCurrentBindingTextureCubeMap()
{
	int id = 0;
	my_glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &id);
	return (u32)id;
}

u32 KPHelper::getCurrentUsingProgram()
{
	int id = 0;
	my_glGetIntegerv(GL_CURRENT_PROGRAM, &id);
	return (u32)id;
}

u8 KPHelper::getCurrentActiveTextureUnit() // From 0 to 31
{
	int id = 0;
	my_glGetIntegerv(GL_ACTIVE_TEXTURE, &id);
	return (u8)(id - GL_TEXTURE0);
}

u32 KPHelper::getCurrentBindingVbo_ArrayBuffer()
{
	int id = 0;
	my_glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &id);
	return (u32)id;
}

u32 KPHelper::getCurrentBindingVbo_ElementArrayBuffer()
{
	int id = 0;
	my_glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &id);
	return (u32)id;
}

u32 KPHelper::getCurrentBindingVboOfTarget(u32 target)
{
	return target == GL_ARRAY_BUFFER ?
			KPHelper::getCurrentBindingVbo_ArrayBuffer() :
			KPHelper::getCurrentBindingVbo_ElementArrayBuffer();
}

u32 KPHelper::getCurrentBindingFbo()
{
	int id = 0;
	my_glGetIntegerv(GL_FRAMEBUFFER_BINDING, &id);
	return (u32)id;
}

void KPHelper::getCurrentDepthRange(float& n, float& f)
{
	float v[2];
	my_glGetFloatv(GL_DEPTH_RANGE, v);
	n = v[0];
	f = v[1];
}

int KPHelper::get_MAX_COMBINED_TEXTURE_IMAGE_UNITS()
{
	int value = 0;
	my_glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &value);
	return value;
}

int KPHelper::m_sMaxTextureSize = -1;
int KPHelper::get_MAX_TEXTURE_SIZE()
{
	if (m_sMaxTextureSize == -1)
	{
		my_glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_sMaxTextureSize);
	}
	return m_sMaxTextureSize;
}

int KPHelper::m_sMaxMipLevel = -1;
int KPHelper::getMaxMipLevel()
{
	if (m_sMaxMipLevel == -1)
	{
		int texSize = get_MAX_TEXTURE_SIZE();
		m_sMaxMipLevel = 0;
		while (texSize > 1)
		{
			texSize = texSize >> 1;
			m_sMaxMipLevel++;
		}
	}
	return m_sMaxMipLevel;
}

int KPHelper::m_sMaxVertexAttribs = -1;
int KPHelper::get_MAX_VERTEX_ATTRIBS()
{
	if (m_sMaxVertexAttribs == -1)
	{
		my_glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &m_sMaxVertexAttribs);
	}
	return m_sMaxVertexAttribs;
}

int KPHelper::makeTgaHeader(char* _buffer, s16 width, s16 height, bool hasAlpha/* = true*/)
{
	const u8		cGarbage	= 0;
	const s16		iGarbage	= 0;
	const u8		pixelDepth	= hasAlpha ? 32 : 24;
	const u8		type		= 2;

	char* buffer = _buffer;

	memcpy(buffer, &cGarbage, 1);		buffer++;
	memcpy(buffer, &cGarbage, 1);		buffer++;

	memcpy(buffer, &type, 1);			buffer++;

	memcpy(buffer, &iGarbage, 2);		buffer += 2;
	memcpy(buffer, &iGarbage, 2);		buffer += 2;
	memcpy(buffer, &cGarbage, 1);		buffer++;
	memcpy(buffer, &iGarbage, 2);		buffer += 2;
	memcpy(buffer, &iGarbage, 2);		buffer += 2;

	memcpy(buffer, &width, 2);			buffer += 2;
	memcpy(buffer, &height, 2);			buffer += 2;
	memcpy(buffer, &pixelDepth, 1);		buffer++;

	memcpy(buffer, &cGarbage, 1);		buffer++;

	return (int)(buffer - _buffer);
}

void KPHelper::convertToTga(char* buffer, int width, int height, bool hasAlpha/* = true*/)
{
	const int bytesPerPixel = hasAlpha ? 4 : 3;
	const int total = width * height * bytesPerPixel;
	char tmp;
	for (int i = 0; i < total; i += bytesPerPixel)
	{
		tmp = buffer[i];
		buffer[i] = buffer[i + 2];
		buffer[i + 2] = tmp;
	}
}

void KPHelper::swapRB(char* buffer, int pixelsNumber, bool hasAlpha/* = true*/)
{
	const int k_bytesPerPixel = hasAlpha ? 4 : 3;
	const int k_bytesTotal = k_bytesPerPixel * pixelsNumber;
	char tmp;
	for (int i = 0; i < k_bytesTotal; i += k_bytesPerPixel)
	{
		tmp = buffer[i];
		buffer[i] = buffer[i + 2];
		buffer[i + 2] = tmp;
	}
}

void KPHelper::copyAlignedPixelsData(char* buffer, const void* pixels, int width, int height, u32 format, u32 type)
{
	int unPackAlignment = 4;
	my_glGetIntegerv(GL_UNPACK_ALIGNMENT, &unPackAlignment);

	const int k_bytesPerRow = width * KPMipmapLevel::getTexBPP(format, type) / 8;

	int bytesPerRowAligned = k_bytesPerRow;
	while (bytesPerRowAligned % unPackAlignment != 0) bytesPerRowAligned++;

	const char* pix = (const char*)pixels;
	for (int row = 0; row < height; row++)
	{
		memcpy(buffer, pix, k_bytesPerRow);
		buffer += k_bytesPerRow;
		pix += bytesPerRowAligned;
	}
}

void KPHelper::copyProgramUniformsStateToGpu(const KPProgram& dstProg, const KPProgram& srcProg)
{
	int dstCount = dstProg.getUniformsCount();
	int srcCount = srcProg.getUniformsCount();

	for (int i = 0; i < dstCount; i++)
	{
		int k = -1;
		{
			const KPUniform& dstUniform = dstProg.getUniform(i);
			for (int j = 0; j < srcCount; j++)
			{
				const KPUniform& srcUniform = srcProg.getUniform(j);
				if ( dstUniform.isTheSamePrototype(srcUniform) )
				{
					k = j;
					break;
				}
			}
		}

		if (k == -1) continue;
					
		const KPUniform& uni = srcProg.getUniform(k);

		switch (uni.m_type)
		{
			case GL_FLOAT:
			{
				KPHelper::assert(uni.m_valueBytesNumber % 4 == 0);
				int count = uni.m_valueBytesNumber / 4;
				my_glUniform1fv(uni.m_location, count, (const float*)uni.m_pValuePointer);
				break;
			}
			case GL_FLOAT_VEC2:
			{
				KPHelper::assert(uni.m_valueBytesNumber % 8 == 0);
				int count = uni.m_valueBytesNumber / 8;
				my_glUniform2fv(uni.m_location, count, (const float*)uni.m_pValuePointer);
				break;
			}
			case GL_FLOAT_VEC3:
			{
				KPHelper::assert(uni.m_valueBytesNumber % 12 == 0);
				int count = uni.m_valueBytesNumber / 12;
				my_glUniform3fv(uni.m_location, count, (const float*)uni.m_pValuePointer);
				break;
			}
			case GL_FLOAT_VEC4:
			{
				KPHelper::assert(uni.m_valueBytesNumber % 16 == 0);
				int count = uni.m_valueBytesNumber / 16;
				my_glUniform4fv(uni.m_location, count, (const float*)uni.m_pValuePointer);
				break;
			}
			case GL_INT:
			case GL_SAMPLER_2D:
			case GL_SAMPLER_3D:
			case GL_SAMPLER_CUBE:
			{
				KPHelper::assert(uni.m_valueBytesNumber % 4 == 0);
				int count = uni.m_valueBytesNumber / 4;
				my_glUniform1iv(uni.m_location, count, (const int*)uni.m_pValuePointer);
				break;
			}
			case GL_INT_VEC2:
			{
				KPHelper::assert(uni.m_valueBytesNumber % 8 == 0);
				int count = uni.m_valueBytesNumber / 8;
				my_glUniform2iv(uni.m_location, count, (const int*)uni.m_pValuePointer);
				break;
			}
			case GL_INT_VEC3:
			{
				KPHelper::assert(uni.m_valueBytesNumber % 12 == 0);
				int count = uni.m_valueBytesNumber / 12;
				my_glUniform3iv(uni.m_location, count, (const int*)uni.m_pValuePointer);
				break;
			}
			case GL_INT_VEC4:
			{
				KPHelper::assert(uni.m_valueBytesNumber % 16 == 0);
				int count = uni.m_valueBytesNumber / 16;
				my_glUniform4iv(uni.m_location, count, (const int*)uni.m_pValuePointer);
				break;
			}
			case GL_BOOL:
			{
				KPHelper::assert(uni.m_valueBytesNumber % 4 == 0);
				int count = uni.m_valueBytesNumber / 4;
				my_glUniform1iv(uni.m_location, count, (const int*)uni.m_pValuePointer);
				break;
			}
			case GL_BOOL_VEC2:
			{
				KPHelper::assert(uni.m_valueBytesNumber % 8 == 0);
				int count = uni.m_valueBytesNumber / 8;
				my_glUniform2iv(uni.m_location, count, (const int*)uni.m_pValuePointer);
				break;
			}
			case GL_BOOL_VEC3:
			{
				KPHelper::assert(uni.m_valueBytesNumber % 12 == 0);
				int count = uni.m_valueBytesNumber / 12;
				my_glUniform3iv(uni.m_location, count, (const int*)uni.m_pValuePointer);
				break;
			}
			case GL_BOOL_VEC4:
			{
				KPHelper::assert(uni.m_valueBytesNumber % 16 == 0);
				int count = uni.m_valueBytesNumber / 16;
				my_glUniform4iv(uni.m_location, count, (const int*)uni.m_pValuePointer);
				break;
			}
			case GL_FLOAT_MAT2:
			{
				const int size = 2;
				const int bytes = size * size * 4;
				KPHelper::assert(uni.m_valueBytesNumber % bytes == 0);
				int count = uni.m_valueBytesNumber / bytes;
				my_glUniformMatrix2fv(uni.m_location, count, GL_FALSE, (const float*)uni.m_pValuePointer);
				break;
			}
			case GL_FLOAT_MAT3:
			{
				const int size = 3;
				const int bytes = size * size * 4;
				KPHelper::assert(uni.m_valueBytesNumber % bytes == 0);
				int count = uni.m_valueBytesNumber / bytes;
				my_glUniformMatrix3fv(uni.m_location, count, GL_FALSE, (const float*)uni.m_pValuePointer);
				break;
			}
			case GL_FLOAT_MAT4:
			{
				const int size = 4;
				const int bytes = size * size * 4;
				KPHelper::assert(uni.m_valueBytesNumber % bytes == 0);
				int count = uni.m_valueBytesNumber / bytes;
				my_glUniformMatrix4fv(uni.m_location, count, GL_FALSE, (const float*)uni.m_pValuePointer);
				break;
			}
						
			default:
			{
				KPHelper::assert(false, "KPHelper::copyProgramUniformsStateToGpu()");
				break;
			}
		} // switch (uni.m_type)
	} // for
}

char* KPHelper::convertTexFormatFromRgba(int width, int height, GLenum internalformat, char* pixels, int& pixelsLen)
{
	char* newPixels = NULL;

	switch (internalformat)
	{
		case GL_RGB:
		{
			pixelsLen = width * height * 3;

			if (pixelsLen > 0 && pixels)
			{
				newPixels = new char[pixelsLen];
				
				for (int i = 0; i < width * height; i++)
				{
					int j = i * 4;
					int k = i * 3;
					newPixels[k++] = pixels[j++];
					newPixels[k++] = pixels[j++];
					newPixels[k++] = pixels[j++];
				}
			}

			break;
		}
		case GL_ALPHA:
		{
			pixelsLen = width * height * 1;
			
			if (pixelsLen > 0 && pixels)
			{
				newPixels = new char[pixelsLen];
				
				for (int i = 0; i < width * height; i++)
				{
					int j = i * 4;
					int k = i * 1;
					newPixels[k] = pixels[j + 3];
				}
			}

			break;
		}
		case GL_LUMINANCE:
		{
			pixelsLen = width * height * 1;
			
			if (pixelsLen > 0 && pixels)
			{
				newPixels = new char[pixelsLen];
				for (int i = 0; i < width * height; i++)
				{
					int j = i * 4;
					int k = i * 1;
					newPixels[k] = (char)(
						(int)(pixels[j] + pixels[j + 1] + pixels[j + 2]) / 3
					);
				}
			}
			
			break;
		}
		case GL_LUMINANCE_ALPHA:
		{
			pixelsLen = width * height * 2;
			
			if (pixelsLen > 0 && pixels)
			{
				newPixels = new char[pixelsLen];
				
				for (int i = 0; i < width * height; i++)
				{
					int j = i * 4;
					int k = i * 2;
					newPixels[k] = (char)(
						(int)(pixels[j] + pixels[j + 1] + pixels[j + 2]) / 3
					);
					newPixels[k + 1] = pixels[j + 3];
				}
			}
			
			break;
		}
	}

	return newPixels;
}

#define CONVERT(srcValue, srcBits, dstBits) ( (u16)(srcValue) * (u16)((1 << (dstBits)) - 1) / (u16)((1 << (srcBits)) - 1) )

void KPHelper::convert_RGBA_to_RGB(int width, int height, const char* src, char* dst)
{
	for (int i = 0; i < width * height; i++)
	{
		int j = i * 4;
		int k = i * 3;
		dst[k++] = src[j++];
		dst[k++] = src[j++];
		dst[k++] = src[j++];
	}
}

void KPHelper::convert_RGBA_to_565(int width, int height, const char* src, char* dst)
{
	for (int i = 0; i < width * height; i++)
	{
		int j = i * 4;
		int k = i * 2;
		u16* dst16 = (u16*)(dst + k);
		*dst16 =	(u16)CONVERT(src[j], 8, 5) << 11;
		*dst16 |=	(u16)CONVERT(src[j + 1], 8, 6) << 5;
		*dst16 |=	(u16)CONVERT(src[j + 2], 8, 5);
	}
}

void KPHelper::convert_RGBA_to_RGB(int width, int height, char* src)
{
	for (int i = 0; i < width * height; i++)
	{
		int j = i * 4;
		int k = i * 3;
		src[k++] = src[j++];
		src[k++] = src[j++];
		src[k++] = src[j++];
	}
}

void KPHelper::copyTexSubDataRow(u8* dst, const u8* src, int width, u32 format, u32 srcType, u32 dstType)
{
	const int dst_bytesPP = KPMipmapLevel::getTexBPP(format, dstType) / 8;
	const int src_bytesPP = KPMipmapLevel::getTexBPP(format, srcType) / 8;

	if (dstType == srcType)
	{
		memcpy(dst, src, dst_bytesPP * width);
		return;
	}

	for (int i = 0; i < width; i++)
	{
		if (format == GL_RGBA)
		{
			if (srcType == GL_UNSIGNED_BYTE && dstType == GL_UNSIGNED_SHORT_4_4_4_4)
			{
				u16* dst16 = (u16*)dst;

				*dst16 =		(u16)CONVERT(src[0], 8, 4) << 12;
				*dst16 |=		(u16)CONVERT(src[1], 8, 4) << 8;
				*dst16 |=		(u16)CONVERT(src[2], 8, 4) << 4;
				*dst16 |=		(u16)CONVERT(src[3], 8, 4);
			}
			else if (srcType == GL_UNSIGNED_BYTE && dstType == GL_UNSIGNED_SHORT_5_5_5_1)
			{
				u16* dst16 = (u16*)dst;

				*dst16 =		(u16)CONVERT(src[0], 8, 5) << 11;
				*dst16 |=		(u16)CONVERT(src[1], 8, 5) << 6;
				*dst16 |=		(u16)CONVERT(src[2], 8, 5) << 1;
				*dst16 |=		(u16)CONVERT(src[3], 8, 1);
			}

			else if (srcType == GL_UNSIGNED_SHORT_4_4_4_4 && dstType == GL_UNSIGNED_BYTE)
			{
				u16 src16 = *( (u16*)src );
				
				dst[0] = (u8)CONVERT((src16 >> 12) & 0xF, 4, 8);
				dst[1] = (u8)CONVERT((src16 >>  8) & 0xF, 4, 8);
				dst[2] = (u8)CONVERT((src16 >>  4) & 0xF, 4, 8);
				dst[3] = (u8)CONVERT((src16      ) & 0xF, 4, 8);
			}
			else if (srcType == GL_UNSIGNED_SHORT_4_4_4_4 && dstType == GL_UNSIGNED_SHORT_5_5_5_1)
			{
				u16 src16 = *( (u16*)src );
				u16* dst16 = (u16*)dst;

				*dst16 =	(u16)CONVERT((src16 >> 12) & 0xF, 4, 5) << 11;
				*dst16 |=	(u16)CONVERT((src16 >>  8) & 0xF, 4, 5) <<  6;
				*dst16 |=	(u16)CONVERT((src16 >>  4) & 0xF, 4, 5) <<  1;
				*dst16 |=	(u16)CONVERT((src16      ) & 0xF, 4, 1);
			}

			else if (srcType == GL_UNSIGNED_SHORT_5_5_5_1 && dstType == GL_UNSIGNED_BYTE)
			{
				u16 src16 = *( (u16*)src );

				dst[0] = (u8)CONVERT((src16 >> 11) & 0x1F, 5, 8);
				dst[1] = (u8)CONVERT((src16 >>  6) & 0x1F, 5, 8);
				dst[2] = (u8)CONVERT((src16 >>  1) & 0x1F, 5, 8);
				dst[3] = (u8)CONVERT((src16      ) & 0x01, 1, 8);
			}
			else if (srcType == GL_UNSIGNED_SHORT_5_5_5_1 && dstType == GL_UNSIGNED_SHORT_4_4_4_4)
			{
				u16 src16 = *( (u16*)src );
				u16* dst16 = (u16*)dst;

				*dst16 =	(u16)CONVERT((src16 >> 11) & 0x1F, 5, 4) << 12;
				*dst16 |=	(u16)CONVERT((src16 >> 6) & 0x1F, 5, 4) << 8;
				*dst16 |=	(u16)CONVERT((src16 >> 1) & 0x1F, 5, 4) << 4;
				*dst16 |=	(u16)CONVERT((src16     ) & 0x01, 1, 4);
			}
		}
		else if (format == GL_RGB)
		{
			if (srcType == GL_UNSIGNED_BYTE && dstType == GL_UNSIGNED_SHORT_5_6_5)
			{
				u16* dst16 = (u16*)dst;

				*dst16 =	(u16)CONVERT(src[0], 8, 5) << 11;
				*dst16 |=	(u16)CONVERT(src[1], 8, 6) << 5;
				*dst16 |=	(u16)CONVERT(src[2], 8, 5);
			}
			else if (srcType == GL_UNSIGNED_SHORT_5_6_5 && dstType == GL_UNSIGNED_BYTE)
			{
				u16 src16 = *( (u16*)src );

				dst[0] = (u8)CONVERT((src16 >> 11) & 0x1F, 5, 8);
				dst[1] = (u8)CONVERT((src16 >>  5) & 0x3F, 6, 8);
				dst[2] = (u8)CONVERT((src16      ) & 0x1F, 5, 8);
			}
		}
		else
		{
			LOGE("error [%s] format = 0x%x", __FUNCTION__, format);
		}

		src += src_bytesPP;
		dst += dst_bytesPP;
	}
}

bool KPHelper::isRenderingToTexture(u32& texId, int& mipLevel)
{
	if (KPHelper::getCurrentBindingFbo() == 0) return false;

	GLint params;
	my_glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &params);

	if (params != GL_TEXTURE) return false;

	my_glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &params);
	texId = (u32)params; // ***

	my_glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL, &params);
	mipLevel = params; // ***

	return true;
}
