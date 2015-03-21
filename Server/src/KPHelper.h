#ifndef _KP_HELPER_H_
#define _KP_HELPER_H_

#include "KPGLObject.h"
#include "KPMessage.h"

#include <pthread.h>

#define MAX_MUTEXES 2

class KPProgram;

class KPHelper
{
public:
	static int sendGLObjectToClient(int client, KPGLObject& obj);

	static int sendMessageToClient(int client, KPMessage& msg);

	static int sendRequestFinishMessageToClient(int client);
	static int sendRequestErrorMessageToClient(int client, const char* error = NULL);

	static int sendCurrentTexturesStatus(int client);
	static int sendCurrentUsingProgram(int client);
	static int sendCurrentBindingVbos(int client);
	

	static void closeSocket(int sockId);
	static bool recvSocket(int sockId, void* buffer, int len);
	static bool setSocketRecvTimeout(int sockId, int timeoutMs);

	static void initMutexes();
	static void destroyMutexes();
	
	static void LockMutex0();
	static void UnlockMutex0();

	static void LockMutex1();
	static void UnlockMutex1();

	static bool assert(bool b, const char* szError = NULL);

	static void getViewport(int& x, int& y, int& width, int& height);

	static void readPixels(int x, int y, int width, int height, char* dst);

	static char* makeScreenshot_RGB(
		const char* src, // RGBA
		int width, int height,
		bool allowScale, bool allowSwapRB,
		int& widthScaled, int& heightScaled,
		int& len
	);

	static int calculateShaderSourceSize(GLsizei count, const GLchar** string, const GLint* length);
	static void copyShaderSource(char* buffer, GLsizei count, const GLchar** string, const GLint* length);

	static char* checkValidModifiedProgram(u32 prodId, u32 vsId, u32 fsId,
			const char* vsSourceString, const char* fsSourceString, const KPProgram* pOldProg, KPProgram& newProg);

	static int getSizeInBytesOfGLDataType(u32 type);

	static u32 getCurrentBindingTexture2D();
	static u32 getCurrentBindingTextureCubeMap();
	static u32 getCurrentUsingProgram();
	static u8 getCurrentActiveTextureUnit(); // From 0 to 31

	static u32 getCurrentBindingVbo_ArrayBuffer();
	static u32 getCurrentBindingVbo_ElementArrayBuffer();
	static u32 getCurrentBindingVboOfTarget(u32 target);

	static u32 getCurrentBindingFbo();
	static void getCurrentDepthRange(float& n, float& f);
	static int get_MAX_COMBINED_TEXTURE_IMAGE_UNITS();
	static int get_MAX_TEXTURE_SIZE();
	static int getMaxMipLevel();

	static int get_MAX_VERTEX_ATTRIBS();

	static int makeTgaHeader(char* buffer, s16 width, s16 height, bool hasAlpha = true);
	static void convertToTga(char* buffer, int width, int height, bool hasAlpha = true);

	static void swapRB(char* buffer, int pixelsNumber, bool hasAlpha = true);
	static void convert_RGBA_to_RGB(int width, int height, const char* src, char* dst);
	static void convert_RGBA_to_565(int width, int height, const char* src, char* dst);
	static void convert_RGBA_to_RGB(int width, int height, char* src);

	static void copyAlignedPixelsData(char* buffer, const void* pixels, int width, int height, u32 format, u32 type);
	static void copyProgramUniformsStateToGpu(const KPProgram& dstProg, const KPProgram& srcProg);
	static char* convertTexFormatFromRgba(int width, int height, GLenum internalformat, char* pixels, int& pixelsLen);
	static void copyTexSubDataRow(u8* dst, const u8* src, int width, u32 format, u32 srcType, u32 dstType);

	static bool isRenderingToTexture(u32& texId, int& mipLevel);

	//========================================================================================

	static pthread_mutex_t m_sMutexes[MAX_MUTEXES];

	static const int k_tgaHeaderSize = 18;

private:
	static int m_sMaxTextureSize;
	static int m_sMaxMipLevel;
	static int m_sMaxVertexAttribs;
};

#endif
