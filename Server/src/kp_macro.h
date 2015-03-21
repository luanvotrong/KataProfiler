#ifndef _KP_MACRO_H_
#define _KP_MACRO_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAFE_DEL(x) if (x) { delete x; x = NULL; }
#define SAFE_DEL_ARRAY(x) if (x) { delete [] x; x = NULL; }

#define SOCKET_ERROR	(-1)
#define SOCKET_SUCCESS	(0)

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

#if defined OS_ANDROID
	#include <GLES2/gl2.h>

	#include <jni.h>
	#include <android/log.h>

	#define  LOG_TAG    "KataProfiler"
	#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,		LOG_TAG,	__VA_ARGS__)
	#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,		LOG_TAG,	__VA_ARGS__)
	#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,		LOG_TAG,	__VA_ARGS__)
	#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,		LOG_TAG,	__VA_ARGS__)

#elif defined OS_TIZEN
	#include <gl2.h>
	typedef char             GLchar;

	#include <FBaseLog.h>

	#define  LOGI			AppLog
	#define  LOGE			AppLogException
	#define  LOGD			AppLog
	#define  LOGW			AppLog

#endif

//=====================================================================================================================
// EXT
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG                      0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG                      0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG                     0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG                     0x8C03

#define GL_ETC1_RGB8_OES					0x8D64

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT		0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT	0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT	0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT	0x83F3

//
#define GL_SAMPLER_3D 0x8B5F

#endif // _KP_MACRO_H_
