#include <stdio.h>
#include <stdint.h>
#include <dlfcn.h>                               /* header required for dlsym() */

#include "KPServer.h"
#include "KPHelper.h"

extern KPServer kpServer;

void (*my_glActiveTexture)(GLenum) = NULL;
void (*my_glAttachShader)(GLuint, GLuint) = NULL;
void (*my_glBindAttribLocation)(GLuint, GLuint, const GLchar*) = NULL;
void (*my_glBindBuffer)(GLenum, GLuint) = NULL;
void (*my_glBindFramebuffer)(GLenum, GLuint) = NULL;
void (*my_glBindRenderbuffer)(GLenum, GLuint) = NULL;
void (*my_glBindTexture)(GLenum, GLuint) = NULL;
void (*my_glBlendColor)(GLclampf, GLclampf, GLclampf, GLclampf) = NULL;
void (*my_glBlendEquation)(GLenum) = NULL;
void (*my_glBlendEquationSeparate)(GLenum, GLenum) = NULL;
void (*my_glBlendFunc)(GLenum, GLenum) = NULL;
void (*my_glBlendFuncSeparate)(GLenum, GLenum, GLenum, GLenum) = NULL;
void (*my_glBufferData)(GLenum, GLsizeiptr, const GLvoid*, GLenum) = NULL;
void (*my_glBufferSubData)(GLenum, GLintptr, GLsizeiptr, const GLvoid*) = NULL;
GLenum (*my_glCheckFramebufferStatus)(GLenum) = NULL;
void (*my_glClear)(GLbitfield) = NULL;
void (*my_glClearColor)(GLclampf, GLclampf, GLclampf, GLclampf) = NULL;
void (*my_glClearDepthf)(GLclampf) = NULL;
void (*my_glClearStencil)(GLint) = NULL;
void (*my_glColorMask)(GLboolean, GLboolean, GLboolean, GLboolean) = NULL;
void (*my_glCompileShader)(GLuint) = NULL;
void (*my_glCompressedTexImage2D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid*) = NULL;
void (*my_glCompressedTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid*) = NULL;
void (*my_glCopyTexImage2D)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint) = NULL;
void (*my_glCopyTexSubImage2D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei) = NULL;
GLuint (*my_glCreateProgram)(void) = NULL;
GLuint (*my_glCreateShader)(GLenum) = NULL;
void (*my_glCullFace)(GLenum) = NULL;
void (*my_glDeleteBuffers)(GLsizei, const GLuint*) = NULL;
void (*my_glDeleteFramebuffers)(GLsizei, const GLuint*) = NULL;
void (*my_glDeleteProgram)(GLuint) = NULL;
void (*my_glDeleteRenderbuffers)(GLsizei, const GLuint*) = NULL;
void (*my_glDeleteShader)(GLuint) = NULL;
void (*my_glDeleteTextures)(GLsizei, const GLuint*) = NULL;
void (*my_glDepthFunc)(GLenum) = NULL;
void (*my_glDepthMask)(GLboolean) = NULL;
void (*my_glDepthRangef)(GLclampf, GLclampf) = NULL;
void (*my_glDetachShader)(GLuint, GLuint) = NULL;
void (*my_glDisable)(GLenum) = NULL;
void (*my_glDisableVertexAttribArray)(GLuint) = NULL;
void (*my_glDrawArrays)(GLenum, GLint, GLsizei) = NULL;
void (*my_glDrawElements)(GLenum, GLsizei, GLenum, const GLvoid*) = NULL;
void (*my_glEnable)(GLenum) = NULL;
void (*my_glEnableVertexAttribArray)(GLuint) = NULL;
void (*my_glFinish)(void) = NULL;
void (*my_glFlush)(void) = NULL;
void (*my_glFramebufferRenderbuffer)(GLenum, GLenum, GLenum, GLuint) = NULL;
void (*my_glFramebufferTexture2D)(GLenum, GLenum, GLenum, GLuint, GLint) = NULL;
void (*my_glFrontFace)(GLenum) = NULL;
void (*my_glGenBuffers)(GLsizei, GLuint*) = NULL;
void (*my_glGenerateMipmap)(GLenum) = NULL;
void (*my_glGenFramebuffers)(GLsizei, GLuint*) = NULL;
void (*my_glGenRenderbuffers)(GLsizei, GLuint*) = NULL;
void (*my_glGenTextures)(GLsizei, GLuint*) = NULL;
void (*my_glGetActiveAttrib)(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*) = NULL;
void (*my_glGetActiveUniform)(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*) = NULL;
void (*my_glGetAttachedShaders)(GLuint, GLsizei, GLsizei*, GLuint*) = NULL;
int (*my_glGetAttribLocation)(GLuint, const GLchar*) = NULL;
void (*my_glGetBooleanv)(GLenum, GLboolean*) = NULL;
void (*my_glGetBufferParameteriv)(GLenum, GLenum, GLint*) = NULL;
GLenum (*my_glGetError)(void) = NULL;
void (*my_glGetFloatv)(GLenum, GLfloat*) = NULL;
void (*my_glGetFramebufferAttachmentParameteriv)(GLenum, GLenum, GLenum, GLint*) = NULL;
void (*my_glGetIntegerv)(GLenum, GLint*) = NULL;
void (*my_glGetProgramiv)(GLuint, GLenum, GLint*) = NULL;
void (*my_glGetProgramInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*) = NULL;
void (*my_glGetRenderbufferParameteriv)(GLenum, GLenum, GLint*) = NULL;
void (*my_glGetShaderiv)(GLuint, GLenum, GLint*) = NULL;
void (*my_glGetShaderInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*) = NULL;
void (*my_glGetShaderPrecisionFormat)(GLenum, GLenum, GLint*, GLint*) = NULL;
void (*my_glGetShaderSource)(GLuint, GLsizei, GLsizei*, GLchar*) = NULL;
const GLubyte* (*my_glGetString)(GLenum) = NULL;
void (*my_glGetTexParameterfv)(GLenum, GLenum, GLfloat*) = NULL;
void (*my_glGetTexParameteriv)(GLenum, GLenum, GLint*) = NULL;
void (*my_glGetUniformfv)(GLuint, GLint, GLfloat*) = NULL;
void (*my_glGetUniformiv)(GLuint, GLint, GLint*) = NULL;
int (*my_glGetUniformLocation)(GLuint, const GLchar*) = NULL;
void (*my_glGetVertexAttribfv)(GLuint, GLenum, GLfloat*) = NULL;
void (*my_glGetVertexAttribiv)(GLuint, GLenum, GLint*) = NULL;
void (*my_glGetVertexAttribPointerv)(GLuint, GLenum, GLvoid**) = NULL;
void (*my_glHint)(GLenum, GLenum) = NULL;
GLboolean (*my_glIsBuffer)(GLuint) = NULL;
GLboolean (*my_glIsEnabled)(GLenum) = NULL;
GLboolean (*my_glIsFramebuffer)(GLuint) = NULL;
GLboolean (*my_glIsProgram)(GLuint) = NULL;
GLboolean (*my_glIsRenderbuffer)(GLuint) = NULL;
GLboolean (*my_glIsShader)(GLuint) = NULL;
GLboolean (*my_glIsTexture)(GLuint) = NULL;
void (*my_glLineWidth)(GLfloat) = NULL;
void (*my_glLinkProgram)(GLuint) = NULL;
void (*my_glPixelStorei)(GLenum, GLint) = NULL;
void (*my_glPolygonOffset)(GLfloat, GLfloat) = NULL;
void (*my_glReadPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid*) = NULL;
void (*my_glReleaseShaderCompiler)(void) = NULL;
void (*my_glRenderbufferStorage)(GLenum, GLenum, GLsizei, GLsizei) = NULL;
void (*my_glSampleCoverage)(GLclampf, GLboolean) = NULL;
void (*my_glScissor)(GLint, GLint, GLsizei, GLsizei) = NULL;
void (*my_glShaderBinary)(GLsizei, const GLuint*, GLenum, const GLvoid*, GLsizei) = NULL;
void (*my_glShaderSource)(GLuint, GLsizei, const GLchar**, const GLint*) = NULL;
void (*my_glStencilFunc)(GLenum, GLint, GLuint) = NULL;
void (*my_glStencilFuncSeparate)(GLenum, GLenum, GLint, GLuint) = NULL;
void (*my_glStencilMask)(GLuint) = NULL;
void (*my_glStencilMaskSeparate)(GLenum, GLuint) = NULL;
void (*my_glStencilOp)(GLenum, GLenum, GLenum) = NULL;
void (*my_glStencilOpSeparate)(GLenum, GLenum, GLenum, GLenum) = NULL;
#if defined OS_ANDROID
	void (*my_glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*) = NULL;
#elif defined OS_TIZEN
	void (*my_glTexImage2D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*) = NULL;
#endif
void (*my_glTexParameterf)(GLenum, GLenum, GLfloat) = NULL;
void (*my_glTexParameterfv)(GLenum, GLenum, const GLfloat*) = NULL;
void (*my_glTexParameteri)(GLenum, GLenum, GLint) = NULL;
void (*my_glTexParameteriv)(GLenum, GLenum, const GLint*) = NULL;
void (*my_glTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid*) = NULL;
void (*my_glUniform1f)(GLint, GLfloat) = NULL;
void (*my_glUniform1fv)(GLint, GLsizei, const GLfloat*) = NULL;
void (*my_glUniform1i)(GLint, GLint) = NULL;
void (*my_glUniform1iv)(GLint, GLsizei, const GLint*) = NULL;
void (*my_glUniform2f)(GLint, GLfloat, GLfloat) = NULL;
void (*my_glUniform2fv)(GLint, GLsizei, const GLfloat*) = NULL;
void (*my_glUniform2i)(GLint, GLint, GLint) = NULL;
void (*my_glUniform2iv)(GLint, GLsizei, const GLint*) = NULL;
void (*my_glUniform3f)(GLint, GLfloat, GLfloat, GLfloat) = NULL;
void (*my_glUniform3fv)(GLint, GLsizei, const GLfloat*) = NULL;
void (*my_glUniform3i)(GLint, GLint, GLint, GLint) = NULL;
void (*my_glUniform3iv)(GLint, GLsizei, const GLint*) = NULL;
void (*my_glUniform4f)(GLint, GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (*my_glUniform4fv)(GLint, GLsizei, const GLfloat*) = NULL;
void (*my_glUniform4i)(GLint, GLint, GLint, GLint, GLint) = NULL;
void (*my_glUniform4iv)(GLint, GLsizei, const GLint*) = NULL;
void (*my_glUniformMatrix2fv)(GLint, GLsizei, GLboolean, const GLfloat*) = NULL;
void (*my_glUniformMatrix3fv)(GLint, GLsizei, GLboolean, const GLfloat*) = NULL;
void (*my_glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat*) = NULL;
void (*my_glUseProgram)(GLuint) = NULL;
void (*my_glValidateProgram)(GLuint) = NULL;
void (*my_glVertexAttrib1f)(GLuint, GLfloat) = NULL;
void (*my_glVertexAttrib1fv)(GLuint, const GLfloat*) = NULL;
void (*my_glVertexAttrib2f)(GLuint, GLfloat, GLfloat) = NULL;
void (*my_glVertexAttrib2fv)(GLuint, const GLfloat*) = NULL;
void (*my_glVertexAttrib3f)(GLuint, GLfloat, GLfloat, GLfloat) = NULL;
void (*my_glVertexAttrib3fv)(GLuint, const GLfloat*) = NULL;
void (*my_glVertexAttrib4f)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (*my_glVertexAttrib4fv)(GLuint, const GLfloat*) = NULL;
void (*my_glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*) = NULL;
void (*my_glViewport)(GLint, GLint, GLsizei, GLsizei) = NULL;

// Extensions
void* (*my_glMapBufferOES) (GLenum target, GLenum access) = NULL;
GLboolean (*my_glUnmapBufferOES) (GLenum target) = NULL;

static void* my_handler = NULL;

void KPInitHandler_libGLESv2()
{
	if (!my_handler) my_handler = dlopen("libGLESv2.so", RTLD_LAZY);

	if (!my_glActiveTexture) my_glActiveTexture = (void (*)(GLenum)) dlsym(my_handler, "glActiveTexture");
	if (!my_glAttachShader) my_glAttachShader = (void (*)(GLuint, GLuint)) dlsym(my_handler, "glAttachShader");
	if (!my_glBindAttribLocation) my_glBindAttribLocation = (void (*)(GLuint, GLuint, const GLchar*)) dlsym(my_handler, "glBindAttribLocation");
	if (!my_glBindBuffer) my_glBindBuffer = (void (*)(GLenum, GLuint)) dlsym(my_handler, "glBindBuffer");
	if (!my_glBindFramebuffer) my_glBindFramebuffer = (void (*)(GLenum, GLuint)) dlsym(my_handler, "glBindFramebuffer");
	if (!my_glBindRenderbuffer) my_glBindRenderbuffer = (void (*)(GLenum, GLuint)) dlsym(my_handler, "glBindRenderbuffer");
	if (!my_glBindTexture) my_glBindTexture = (void (*)(GLenum, GLuint)) dlsym(my_handler, "glBindTexture");
	if (!my_glBlendColor) my_glBlendColor = (void (*)(GLclampf, GLclampf, GLclampf, GLclampf)) dlsym(my_handler, "glBlendColor");
	if (!my_glBlendEquation) my_glBlendEquation = (void (*)(GLenum)) dlsym(my_handler, "glBlendEquation");
	if (!my_glBlendEquationSeparate) my_glBlendEquationSeparate = (void (*)(GLenum, GLenum)) dlsym(my_handler, "glBlendEquationSeparate");
	if (!my_glBlendFunc) my_glBlendFunc = (void (*)(GLenum, GLenum)) dlsym(my_handler, "glBlendFunc");
	if (!my_glBlendFuncSeparate) my_glBlendFuncSeparate = (void (*)(GLenum, GLenum, GLenum, GLenum)) dlsym(my_handler, "glBlendFuncSeparate");
	if (!my_glBufferData) my_glBufferData = (void (*)(GLenum, GLsizeiptr, const GLvoid*, GLenum)) dlsym(my_handler, "glBufferData");
	if (!my_glBufferSubData) my_glBufferSubData = (void (*)(GLenum, GLintptr, GLsizeiptr, const GLvoid*)) dlsym(my_handler, "glBufferSubData");
	if (!my_glCheckFramebufferStatus) my_glCheckFramebufferStatus = (GLenum (*)(GLenum)) dlsym(my_handler, "glCheckFramebufferStatus");
	if (!my_glClear) my_glClear = (void (*)(GLbitfield)) dlsym(my_handler, "glClear");
	if (!my_glClearColor) my_glClearColor = (void (*)(GLclampf, GLclampf, GLclampf, GLclampf)) dlsym(my_handler, "glClearColor");
	if (!my_glClearDepthf) my_glClearDepthf = (void (*)(GLclampf)) dlsym(my_handler, "glClearDepthf");
	if (!my_glClearStencil) my_glClearStencil = (void (*)(GLint)) dlsym(my_handler, "glClearStencil");
	if (!my_glColorMask) my_glColorMask = (void (*)(GLboolean, GLboolean, GLboolean, GLboolean)) dlsym(my_handler, "glColorMask");
	if (!my_glCompileShader) my_glCompileShader = (void (*)(GLuint)) dlsym(my_handler, "glCompileShader");
	if (!my_glCompressedTexImage2D) my_glCompressedTexImage2D = (void (*)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid*)) dlsym(my_handler, "glCompressedTexImage2D");
	if (!my_glCompressedTexSubImage2D) my_glCompressedTexSubImage2D = (void (*)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid*)) dlsym(my_handler, "glCompressedTexSubImage2D");
	if (!my_glCopyTexImage2D) my_glCopyTexImage2D = (void (*)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint)) dlsym(my_handler, "glCopyTexImage2D");
	if (!my_glCopyTexSubImage2D) my_glCopyTexSubImage2D = (void (*)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei)) dlsym(my_handler, "glCopyTexSubImage2D");
	if (!my_glCreateProgram) my_glCreateProgram = (GLuint (*)(void)) dlsym(my_handler, "glCreateProgram");
	if (!my_glCreateShader) my_glCreateShader = (GLuint (*)(GLenum)) dlsym(my_handler, "glCreateShader");
	if (!my_glCullFace) my_glCullFace = (void (*)(GLenum)) dlsym(my_handler, "glCullFace");
	if (!my_glDeleteBuffers) my_glDeleteBuffers = (void (*)(GLsizei, const GLuint*)) dlsym(my_handler, "glDeleteBuffers");
	if (!my_glDeleteFramebuffers) my_glDeleteFramebuffers = (void (*)(GLsizei, const GLuint*)) dlsym(my_handler, "glDeleteFramebuffers");
	if (!my_glDeleteProgram) my_glDeleteProgram = (void (*)(GLuint)) dlsym(my_handler, "glDeleteProgram");
	if (!my_glDeleteRenderbuffers) my_glDeleteRenderbuffers = (void (*)(GLsizei, const GLuint*)) dlsym(my_handler, "glDeleteRenderbuffers");
	if (!my_glDeleteShader) my_glDeleteShader = (void (*)(GLuint)) dlsym(my_handler, "glDeleteShader");
	if (!my_glDeleteTextures) my_glDeleteTextures = (void (*)(GLsizei, const GLuint*)) dlsym(my_handler, "glDeleteTextures");
	if (!my_glDepthFunc) my_glDepthFunc = (void (*)(GLenum)) dlsym(my_handler, "glDepthFunc");
	if (!my_glDepthMask) my_glDepthMask = (void (*)(GLboolean)) dlsym(my_handler, "glDepthMask");
	if (!my_glDepthRangef) my_glDepthRangef = (void (*)(GLclampf, GLclampf)) dlsym(my_handler, "glDepthRangef");
	if (!my_glDetachShader) my_glDetachShader = (void (*)(GLuint, GLuint)) dlsym(my_handler, "glDetachShader");
	if (!my_glDisable) my_glDisable = (void (*)(GLenum)) dlsym(my_handler, "glDisable");
	if (!my_glDisableVertexAttribArray) my_glDisableVertexAttribArray = (void (*)(GLuint)) dlsym(my_handler, "glDisableVertexAttribArray");
	if (!my_glDrawArrays) my_glDrawArrays = (void (*)(GLenum, GLint, GLsizei)) dlsym(my_handler, "glDrawArrays");
	if (!my_glDrawElements) my_glDrawElements = (void (*)(GLenum, GLsizei, GLenum, const GLvoid*)) dlsym(my_handler, "glDrawElements");
	if (!my_glEnable) my_glEnable = (void (*)(GLenum)) dlsym(my_handler, "glEnable");
	if (!my_glEnableVertexAttribArray) my_glEnableVertexAttribArray = (void (*)(GLuint)) dlsym(my_handler, "glEnableVertexAttribArray");
	if (!my_glFinish) my_glFinish = (void (*)(void)) dlsym(my_handler, "glFinish");
	if (!my_glFlush) my_glFlush = (void (*)(void)) dlsym(my_handler, "glFlush");
	if (!my_glFramebufferRenderbuffer) my_glFramebufferRenderbuffer = (void (*)(GLenum, GLenum, GLenum, GLuint)) dlsym(my_handler, "glFramebufferRenderbuffer");
	if (!my_glFramebufferTexture2D) my_glFramebufferTexture2D = (void (*)(GLenum, GLenum, GLenum, GLuint, GLint)) dlsym(my_handler, "glFramebufferTexture2D");
	if (!my_glFrontFace) my_glFrontFace = (void (*)(GLenum)) dlsym(my_handler, "glFrontFace");
	if (!my_glGenBuffers) my_glGenBuffers = (void (*)(GLsizei, GLuint*)) dlsym(my_handler, "glGenBuffers");
	if (!my_glGenerateMipmap) my_glGenerateMipmap = (void (*)(GLenum)) dlsym(my_handler, "glGenerateMipmap");
	if (!my_glGenFramebuffers) my_glGenFramebuffers = (void (*)(GLsizei, GLuint*)) dlsym(my_handler, "glGenFramebuffers");
	if (!my_glGenRenderbuffers) my_glGenRenderbuffers = (void (*)(GLsizei, GLuint*)) dlsym(my_handler, "glGenRenderbuffers");
	if (!my_glGenTextures) my_glGenTextures = (void (*)(GLsizei, GLuint*)) dlsym(my_handler, "glGenTextures");
	if (!my_glGetActiveAttrib) my_glGetActiveAttrib = (void (*)(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*)) dlsym(my_handler, "glGetActiveAttrib");
	if (!my_glGetActiveUniform) my_glGetActiveUniform = (void (*)(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*)) dlsym(my_handler, "glGetActiveUniform");
	if (!my_glGetAttachedShaders) my_glGetAttachedShaders = (void (*)(GLuint, GLsizei, GLsizei*, GLuint*)) dlsym(my_handler, "glGetAttachedShaders");
	if (!my_glGetAttribLocation) my_glGetAttribLocation = (int (*)(GLuint, const GLchar*)) dlsym(my_handler, "glGetAttribLocation");
	if (!my_glGetBooleanv) my_glGetBooleanv = (void (*)(GLenum, GLboolean*)) dlsym(my_handler, "glGetBooleanv");
	if (!my_glGetBufferParameteriv) my_glGetBufferParameteriv = (void (*)(GLenum, GLenum, GLint*)) dlsym(my_handler, "glGetBufferParameteriv");
	if (!my_glGetError) my_glGetError = (GLenum (*)(void)) dlsym(my_handler, "glGetError");
	if (!my_glGetFloatv) my_glGetFloatv = (void (*)(GLenum, GLfloat*)) dlsym(my_handler, "glGetFloatv");
	if (!my_glGetFramebufferAttachmentParameteriv) my_glGetFramebufferAttachmentParameteriv = (void (*)(GLenum, GLenum, GLenum, GLint*)) dlsym(my_handler, "glGetFramebufferAttachmentParameteriv");
	if (!my_glGetIntegerv) my_glGetIntegerv = (void (*)(GLenum, GLint*)) dlsym(my_handler, "glGetIntegerv");
	if (!my_glGetProgramiv) my_glGetProgramiv = (void (*)(GLuint, GLenum, GLint*)) dlsym(my_handler, "glGetProgramiv");
	if (!my_glGetProgramInfoLog) my_glGetProgramInfoLog = (void (*)(GLuint, GLsizei, GLsizei*, GLchar*)) dlsym(my_handler, "glGetProgramInfoLog");
	if (!my_glGetRenderbufferParameteriv) my_glGetRenderbufferParameteriv = (void (*)(GLenum, GLenum, GLint*)) dlsym(my_handler, "glGetRenderbufferParameteriv");
	if (!my_glGetShaderiv) my_glGetShaderiv = (void (*)(GLuint, GLenum, GLint*)) dlsym(my_handler, "glGetShaderiv");
	if (!my_glGetShaderInfoLog) my_glGetShaderInfoLog = (void (*)(GLuint, GLsizei, GLsizei*, GLchar*)) dlsym(my_handler, "glGetShaderInfoLog");
	if (!my_glGetShaderPrecisionFormat) my_glGetShaderPrecisionFormat = (void (*)(GLenum, GLenum, GLint*, GLint*)) dlsym(my_handler, "glGetShaderPrecisionFormat");
	if (!my_glGetShaderSource) my_glGetShaderSource = (void (*)(GLuint, GLsizei, GLsizei*, GLchar*)) dlsym(my_handler, "glGetShaderSource");
	if (!my_glGetString) my_glGetString = (const GLubyte* (*)(GLenum)) dlsym(my_handler, "glGetString");
	if (!my_glGetTexParameterfv) my_glGetTexParameterfv = (void (*)(GLenum, GLenum, GLfloat*)) dlsym(my_handler, "glGetTexParameterfv");
	if (!my_glGetTexParameteriv) my_glGetTexParameteriv = (void (*)(GLenum, GLenum, GLint*)) dlsym(my_handler, "glGetTexParameteriv");
	if (!my_glGetUniformfv) my_glGetUniformfv = (void (*)(GLuint, GLint, GLfloat*)) dlsym(my_handler, "glGetUniformfv");
	if (!my_glGetUniformiv) my_glGetUniformiv = (void (*)(GLuint, GLint, GLint*)) dlsym(my_handler, "glGetUniformiv");
	if (!my_glGetUniformLocation) my_glGetUniformLocation = (int (*)(GLuint, const GLchar*)) dlsym(my_handler, "glGetUniformLocation");
	if (!my_glGetVertexAttribfv) my_glGetVertexAttribfv = (void (*)(GLuint, GLenum, GLfloat*)) dlsym(my_handler, "glGetVertexAttribfv");
	if (!my_glGetVertexAttribiv) my_glGetVertexAttribiv = (void (*)(GLuint, GLenum, GLint*)) dlsym(my_handler, "glGetVertexAttribiv");
	if (!my_glGetVertexAttribPointerv) my_glGetVertexAttribPointerv = (void (*)(GLuint, GLenum, GLvoid**)) dlsym(my_handler, "glGetVertexAttribPointerv");
	if (!my_glHint) my_glHint = (void (*)(GLenum, GLenum)) dlsym(my_handler, "glHint");
	if (!my_glIsBuffer) my_glIsBuffer = (GLboolean (*)(GLuint)) dlsym(my_handler, "glIsBuffer");
	if (!my_glIsEnabled) my_glIsEnabled = (GLboolean (*)(GLenum)) dlsym(my_handler, "glIsEnabled");
	if (!my_glIsFramebuffer) my_glIsFramebuffer = (GLboolean (*)(GLuint)) dlsym(my_handler, "glIsFramebuffer");
	if (!my_glIsProgram) my_glIsProgram = (GLboolean (*)(GLuint)) dlsym(my_handler, "glIsProgram");
	if (!my_glIsRenderbuffer) my_glIsRenderbuffer = (GLboolean (*)(GLuint)) dlsym(my_handler, "glIsRenderbuffer");
	if (!my_glIsShader) my_glIsShader = (GLboolean (*)(GLuint)) dlsym(my_handler, "glIsShader");
	if (!my_glIsTexture) my_glIsTexture = (GLboolean (*)(GLuint)) dlsym(my_handler, "glIsTexture");
	if (!my_glLineWidth) my_glLineWidth = (void (*)(GLfloat)) dlsym(my_handler, "glLineWidth");
	if (!my_glLinkProgram) my_glLinkProgram = (void (*)(GLuint)) dlsym(my_handler, "glLinkProgram");
	if (!my_glPixelStorei) my_glPixelStorei = (void (*)(GLenum, GLint)) dlsym(my_handler, "glPixelStorei");
	if (!my_glPolygonOffset) my_glPolygonOffset = (void (*)(GLfloat, GLfloat)) dlsym(my_handler, "glPolygonOffset");
	if (!my_glReadPixels) my_glReadPixels = (void (*)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid*)) dlsym(my_handler, "glReadPixels");
	if (!my_glReleaseShaderCompiler) my_glReleaseShaderCompiler = (void (*)(void)) dlsym(my_handler, "glReleaseShaderCompiler");
	if (!my_glRenderbufferStorage) my_glRenderbufferStorage = (void (*)(GLenum, GLenum, GLsizei, GLsizei)) dlsym(my_handler, "glRenderbufferStorage");
	if (!my_glSampleCoverage) my_glSampleCoverage = (void (*)(GLclampf, GLboolean)) dlsym(my_handler, "glSampleCoverage");
	if (!my_glScissor) my_glScissor = (void (*)(GLint, GLint, GLsizei, GLsizei)) dlsym(my_handler, "glScissor");
	if (!my_glShaderBinary) my_glShaderBinary = (void (*)(GLsizei, const GLuint*, GLenum, const GLvoid*, GLsizei)) dlsym(my_handler, "glShaderBinary");
	if (!my_glShaderSource) my_glShaderSource = (void (*)(GLuint, GLsizei, const GLchar**, const GLint*)) dlsym(my_handler, "glShaderSource");
	if (!my_glStencilFunc) my_glStencilFunc = (void (*)(GLenum, GLint, GLuint)) dlsym(my_handler, "glStencilFunc");
	if (!my_glStencilFuncSeparate) my_glStencilFuncSeparate = (void (*)(GLenum, GLenum, GLint, GLuint)) dlsym(my_handler, "glStencilFuncSeparate");
	if (!my_glStencilMask) my_glStencilMask = (void (*)(GLuint)) dlsym(my_handler, "glStencilMask");
	if (!my_glStencilMaskSeparate) my_glStencilMaskSeparate = (void (*)(GLenum, GLuint)) dlsym(my_handler, "glStencilMaskSeparate");
	if (!my_glStencilOp) my_glStencilOp = (void (*)(GLenum, GLenum, GLenum)) dlsym(my_handler, "glStencilOp");
	if (!my_glStencilOpSeparate) my_glStencilOpSeparate = (void (*)(GLenum, GLenum, GLenum, GLenum)) dlsym(my_handler, "glStencilOpSeparate");
#if defined OS_ANDROID
	if (!my_glTexImage2D) my_glTexImage2D = (void (*)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*)) dlsym(my_handler, "glTexImage2D");
#elif defined OS_TIZEN
	if (!my_glTexImage2D) my_glTexImage2D = (void (*)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*)) dlsym(my_handler, "glTexImage2D");
#endif
	
	if (!my_glTexParameterf) my_glTexParameterf = (void (*)(GLenum, GLenum, GLfloat)) dlsym(my_handler, "glTexParameterf");
	if (!my_glTexParameterfv) my_glTexParameterfv = (void (*)(GLenum, GLenum, const GLfloat*)) dlsym(my_handler, "glTexParameterfv");
	if (!my_glTexParameteri) my_glTexParameteri = (void (*)(GLenum, GLenum, GLint)) dlsym(my_handler, "glTexParameteri");
	if (!my_glTexParameteriv) my_glTexParameteriv = (void (*)(GLenum, GLenum, const GLint*)) dlsym(my_handler, "glTexParameteriv");
	if (!my_glTexSubImage2D) my_glTexSubImage2D = (void (*)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid*)) dlsym(my_handler, "glTexSubImage2D");
	if (!my_glUniform1f) my_glUniform1f = (void (*)(GLint, GLfloat)) dlsym(my_handler, "glUniform1f");
	if (!my_glUniform1fv) my_glUniform1fv = (void (*)(GLint, GLsizei, const GLfloat*)) dlsym(my_handler, "glUniform1fv");
	if (!my_glUniform1i) my_glUniform1i = (void (*)(GLint, GLint)) dlsym(my_handler, "glUniform1i");
	if (!my_glUniform1iv) my_glUniform1iv = (void (*)(GLint, GLsizei, const GLint*)) dlsym(my_handler, "glUniform1iv");
	if (!my_glUniform2f) my_glUniform2f = (void (*)(GLint, GLfloat, GLfloat)) dlsym(my_handler, "glUniform2f");
	if (!my_glUniform2fv) my_glUniform2fv = (void (*)(GLint, GLsizei, const GLfloat*)) dlsym(my_handler, "glUniform2fv");
	if (!my_glUniform2i) my_glUniform2i = (void (*)(GLint, GLint, GLint)) dlsym(my_handler, "glUniform2i");
	if (!my_glUniform2iv) my_glUniform2iv = (void (*)(GLint, GLsizei, const GLint*)) dlsym(my_handler, "glUniform2iv");
	if (!my_glUniform3f) my_glUniform3f = (void (*)(GLint, GLfloat, GLfloat, GLfloat)) dlsym(my_handler, "glUniform3f");
	if (!my_glUniform3fv) my_glUniform3fv = (void (*)(GLint, GLsizei, const GLfloat*)) dlsym(my_handler, "glUniform3fv");
	if (!my_glUniform3i) my_glUniform3i = (void (*)(GLint, GLint, GLint, GLint)) dlsym(my_handler, "glUniform3i");
	if (!my_glUniform3iv) my_glUniform3iv = (void (*)(GLint, GLsizei, const GLint*)) dlsym(my_handler, "glUniform3iv");
	if (!my_glUniform4f) my_glUniform4f = (void (*)(GLint, GLfloat, GLfloat, GLfloat, GLfloat)) dlsym(my_handler, "glUniform4f");
	if (!my_glUniform4fv) my_glUniform4fv = (void (*)(GLint, GLsizei, const GLfloat*)) dlsym(my_handler, "glUniform4fv");
	if (!my_glUniform4i) my_glUniform4i = (void (*)(GLint, GLint, GLint, GLint, GLint)) dlsym(my_handler, "glUniform4i");
	if (!my_glUniform4iv) my_glUniform4iv = (void (*)(GLint, GLsizei, const GLint*)) dlsym(my_handler, "glUniform4iv");
	if (!my_glUniformMatrix2fv) my_glUniformMatrix2fv = (void (*)(GLint, GLsizei, GLboolean, const GLfloat*)) dlsym(my_handler, "glUniformMatrix2fv");
	if (!my_glUniformMatrix3fv) my_glUniformMatrix3fv = (void (*)(GLint, GLsizei, GLboolean, const GLfloat*)) dlsym(my_handler, "glUniformMatrix3fv");
	if (!my_glUniformMatrix4fv) my_glUniformMatrix4fv = (void (*)(GLint, GLsizei, GLboolean, const GLfloat*)) dlsym(my_handler, "glUniformMatrix4fv");
	if (!my_glUseProgram) my_glUseProgram = (void (*)(GLuint)) dlsym(my_handler, "glUseProgram");
	if (!my_glValidateProgram) my_glValidateProgram = (void (*)(GLuint)) dlsym(my_handler, "glValidateProgram");
	if (!my_glVertexAttrib1f) my_glVertexAttrib1f = (void (*)(GLuint, GLfloat)) dlsym(my_handler, "glVertexAttrib1f");
	if (!my_glVertexAttrib1fv) my_glVertexAttrib1fv = (void (*)(GLuint, const GLfloat*)) dlsym(my_handler, "glVertexAttrib1fv");
	if (!my_glVertexAttrib2f) my_glVertexAttrib2f = (void (*)(GLuint, GLfloat, GLfloat)) dlsym(my_handler, "glVertexAttrib2f");
	if (!my_glVertexAttrib2fv) my_glVertexAttrib2fv = (void (*)(GLuint, const GLfloat*)) dlsym(my_handler, "glVertexAttrib2fv");
	if (!my_glVertexAttrib3f) my_glVertexAttrib3f = (void (*)(GLuint, GLfloat, GLfloat, GLfloat)) dlsym(my_handler, "glVertexAttrib3f");
	if (!my_glVertexAttrib3fv) my_glVertexAttrib3fv = (void (*)(GLuint, const GLfloat*)) dlsym(my_handler, "glVertexAttrib3fv");
	if (!my_glVertexAttrib4f) my_glVertexAttrib4f = (void (*)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat)) dlsym(my_handler, "glVertexAttrib4f");
	if (!my_glVertexAttrib4fv) my_glVertexAttrib4fv = (void (*)(GLuint, const GLfloat*)) dlsym(my_handler, "glVertexAttrib4fv");
	if (!my_glVertexAttribPointer) my_glVertexAttribPointer = (void (*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*)) dlsym(my_handler, "glVertexAttribPointer");
	if (!my_glViewport) my_glViewport = (void (*)(GLint, GLint, GLsizei, GLsizei)) dlsym(my_handler, "glViewport");

	// Extensions
	if (!my_glMapBufferOES) my_glMapBufferOES = (void* (*)(GLenum, GLenum)) dlsym(my_handler, "glMapBufferOES");
	if (!my_glUnmapBufferOES) my_glUnmapBufferOES = (GLboolean (*)(GLenum)) dlsym(my_handler, "glUnmapBufferOES");

}

//
#if 1
void          glActiveTexture (GLenum texture) {
	my_glActiveTexture(texture);
	kpServer.on_glActiveTexture(texture);
}

void          glAttachShader (GLuint program, GLuint shader) {
	my_glAttachShader(program, shader);
	kpServer.on_glAttachShader(program, shader);
}

void          glBindAttribLocation (GLuint program, GLuint index, const GLchar* name) {
	my_glBindAttribLocation(program, index, name);
	kpServer.on_glBindAttribLocation(program, index, name);
}

void          glBindBuffer (GLenum target, GLuint buffer) {
	my_glBindBuffer(target, buffer);
	kpServer.on_glBindBuffer(target, buffer);
}

void          glBindFramebuffer (GLenum target, GLuint framebuffer) {
	my_glBindFramebuffer(target, framebuffer);
	kpServer.on_glBindFramebuffer(target, framebuffer);
}

void          glBindRenderbuffer (GLenum target, GLuint renderbuffer) {
	my_glBindRenderbuffer(target, renderbuffer);
	kpServer.on_glBindRenderbuffer(target, renderbuffer);
}

void          glBindTexture (GLenum target, GLuint texture) {
	my_glBindTexture(target, texture);
	kpServer.on_glBindTexture(target, texture);
}

void          glBlendColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
	my_glBlendColor(red, green, blue, alpha);
	kpServer.on_glBlendColor(red, green, blue, alpha);
}

void          glBlendEquation ( GLenum mode ) {
	my_glBlendEquation(mode);
	kpServer.on_glBlendEquation(mode);
}

void          glBlendEquationSeparate (GLenum modeRGB, GLenum modeAlpha) {
	my_glBlendEquationSeparate(modeRGB, modeAlpha);
	kpServer.on_glBlendEquationSeparate(modeRGB, modeAlpha);
}

void          glBlendFunc (GLenum sfactor, GLenum dfactor) {
	my_glBlendFunc(sfactor, dfactor);
	kpServer.on_glBlendFunc(sfactor, dfactor);
}

void          glBlendFuncSeparate (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
	my_glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
	kpServer.on_glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
}

void          glBufferData (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) {
	my_glBufferData(target, size, data, usage);
	kpServer.on_glBufferData(target, size, data, usage);
}

void          glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data) {
	my_glBufferSubData(target, offset, size, data);
	kpServer.on_glBufferSubData(target, offset, size, data);
}

GLenum        glCheckFramebufferStatus (GLenum target) {
	GLenum tmp = my_glCheckFramebufferStatus(target);
	kpServer.on_glCheckFramebufferStatus(target, tmp);
	return tmp;
}

void          glClear (GLbitfield mask) {
	my_glClear(mask);
	kpServer.on_glClear(mask);
}

void          glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
	my_glClearColor(red, green, blue, alpha);
	kpServer.on_glClearColor(red, green, blue, alpha);
}

void          glClearDepthf (GLclampf depth) {
	my_glClearDepthf(depth);
	kpServer.on_glClearDepthf(depth);
}

void          glClearStencil (GLint s) {
	my_glClearStencil(s);
	kpServer.on_glClearStencil(s);
}

void          glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
	my_glColorMask(red, green, blue, alpha);
	kpServer.on_glColorMask(red, green, blue, alpha);
}

void          glCompileShader (GLuint shader) {
	my_glCompileShader(shader);
	kpServer.on_glCompileShader(shader);
}

void          glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data) {
	my_glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
	kpServer.on_glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}

void          glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data) {
	my_glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
	kpServer.on_glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

void          glCopyTexImage2D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
	my_glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
	kpServer.on_glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}

void          glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
	my_glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
	kpServer.on_glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

GLuint        glCreateProgram (void) {
	GLuint tmp = my_glCreateProgram();
	kpServer.on_glCreateProgram(tmp);
	return tmp;
}

GLuint        glCreateShader (GLenum type) {
	GLuint tmp = my_glCreateShader(type);
	kpServer.on_glCreateShader(type, tmp);
	return tmp;
}

void          glCullFace (GLenum mode) {
	my_glCullFace(mode);
	kpServer.on_glCullFace(mode);
}

void          glDeleteBuffers (GLsizei n, const GLuint* buffers) {
	my_glDeleteBuffers(n, buffers);
	kpServer.on_glDeleteBuffers(n, buffers);
}

void          glDeleteFramebuffers (GLsizei n, const GLuint* framebuffers) {
	my_glDeleteFramebuffers(n, framebuffers);
	kpServer.on_glDeleteFramebuffers(n, framebuffers);
}

void          glDeleteProgram (GLuint program) {
	my_glDeleteProgram(program);
	kpServer.on_glDeleteProgram(program);
}

void          glDeleteRenderbuffers (GLsizei n, const GLuint* renderbuffers) {
	my_glDeleteRenderbuffers(n, renderbuffers);
	kpServer.on_glDeleteRenderbuffers(n, renderbuffers);
}

void          glDeleteShader (GLuint shader) {
	my_glDeleteShader(shader);
	kpServer.on_glDeleteShader(shader);
}

void          glDeleteTextures (GLsizei n, const GLuint* textures) {
	my_glDeleteTextures(n, textures);
	kpServer.on_glDeleteTextures(n, textures);
}

void          glDepthFunc (GLenum func) {
	my_glDepthFunc(func);
	kpServer.on_glDepthFunc(func);
}

void          glDepthMask (GLboolean flag) {
	my_glDepthMask(flag);
	kpServer.on_glDepthMask(flag);
}

void          glDepthRangef (GLclampf zNear, GLclampf zFar) {
	my_glDepthRangef(zNear, zFar);
	kpServer.on_glDepthRangef(zNear, zFar);
}

void          glDetachShader (GLuint program, GLuint shader) {
	my_glDetachShader(program, shader);
	kpServer.on_glDetachShader(program, shader);
}

void          glDisable (GLenum cap) {
	my_glDisable(cap);
	kpServer.on_glDisable(cap);
}

void          glDisableVertexAttribArray (GLuint index) {
	my_glDisableVertexAttribArray(index);
	kpServer.on_glDisableVertexAttribArray(index);
}

void          glDrawArrays (GLenum mode, GLint first, GLsizei count) {
	my_glDrawArrays(mode, first, count);
	kpServer.on_glDrawArrays(mode, first, count);
}

void          glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid* indices) {
	my_glDrawElements(mode, count, type, indices);
	kpServer.on_glDrawElements(mode, count, type, indices);
}

void          glEnable (GLenum cap) {
	my_glEnable(cap);
	kpServer.on_glEnable(cap);
}

void          glEnableVertexAttribArray (GLuint index) {
	my_glEnableVertexAttribArray(index);
	kpServer.on_glEnableVertexAttribArray(index);
}

void          glFinish (void) {
	my_glFinish();
	kpServer.on_glFinish();
}

void          glFlush (void) {
	my_glFlush();
	kpServer.on_glFlush();
}

void          glFramebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
	my_glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
	kpServer.on_glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}

void          glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
	my_glFramebufferTexture2D(target, attachment, textarget, texture, level);
	kpServer.on_glFramebufferTexture2D(target, attachment, textarget, texture, level);
}

void          glFrontFace (GLenum mode) {
	my_glFrontFace(mode);
	kpServer.on_glFrontFace(mode);
}

void          glGenBuffers (GLsizei n, GLuint* buffers) {
	my_glGenBuffers(n, buffers);
	kpServer.on_glGenBuffers(n, buffers);
}

void          glGenerateMipmap (GLenum target) {
	my_glGenerateMipmap(target);
	kpServer.on_glGenerateMipmap(target);
}

void          glGenFramebuffers (GLsizei n, GLuint* framebuffers) {
	my_glGenFramebuffers(n, framebuffers);
	kpServer.on_glGenFramebuffers(n, framebuffers);
}

void          glGenRenderbuffers (GLsizei n, GLuint* renderbuffers) {
	my_glGenRenderbuffers(n, renderbuffers);
	kpServer.on_glGenRenderbuffers(n, renderbuffers);
}

void          glGenTextures (GLsizei n, GLuint* textures) {
	my_glGenTextures(n, textures);
	kpServer.on_glGenTextures(n, textures);
}

void          glGetActiveAttrib (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) {
	my_glGetActiveAttrib(program, index, bufsize, length, size, type, name);
	kpServer.on_glGetActiveAttrib(program, index, bufsize, length, size, type, name);
}

void          glGetActiveUniform (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) {
	my_glGetActiveUniform(program, index, bufsize, length, size, type, name);
	kpServer.on_glGetActiveUniform(program, index, bufsize, length, size, type, name);
}

void          glGetAttachedShaders (GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders) {
	my_glGetAttachedShaders(program, maxcount, count, shaders);
	kpServer.on_glGetAttachedShaders(program, maxcount, count, shaders);
}

int           glGetAttribLocation (GLuint program, const GLchar* name) {
	int tmp = my_glGetAttribLocation(program, name);
	kpServer.on_glGetAttribLocation(program, name, tmp);
	return tmp;
}

void          glGetBooleanv (GLenum pname, GLboolean* params) {
	my_glGetBooleanv(pname, params);
	kpServer.on_glGetBooleanv(pname, params);
}

void          glGetBufferParameteriv (GLenum target, GLenum pname, GLint* params) {
	my_glGetBufferParameteriv(target, pname, params);
	kpServer.on_glGetBufferParameteriv(target, pname, params);
}

GLenum        glGetError (void) {
	GLenum tmp = my_glGetError();
	kpServer.on_glGetError(tmp);
	return tmp;
}

void          glGetFloatv (GLenum pname, GLfloat* params) {
	my_glGetFloatv(pname, params);
	kpServer.on_glGetFloatv(pname, params);
}

void          glGetFramebufferAttachmentParameteriv (GLenum target, GLenum attachment, GLenum pname, GLint* params) {
	my_glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
	kpServer.on_glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
}

void          glGetIntegerv (GLenum pname, GLint* params) {
	my_glGetIntegerv(pname, params);
	kpServer.on_glGetIntegerv(pname, params);
}

void          glGetProgramiv (GLuint program, GLenum pname, GLint* params) {
	my_glGetProgramiv(program, pname, params);
	kpServer.on_glGetProgramiv(program, pname, params);
}

void          glGetProgramInfoLog (GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog) {
	my_glGetProgramInfoLog(program, bufsize, length, infolog);
	kpServer.on_glGetProgramInfoLog(program, bufsize, length, infolog);
}

void          glGetRenderbufferParameteriv (GLenum target, GLenum pname, GLint* params) {
	my_glGetRenderbufferParameteriv(target, pname, params);
	kpServer.on_glGetRenderbufferParameteriv(target, pname, params);
}

void          glGetShaderiv (GLuint shader, GLenum pname, GLint* params) {
	my_glGetShaderiv(shader, pname, params);
	kpServer.on_glGetShaderiv(shader, pname, params);
}

void          glGetShaderInfoLog (GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog) {
	my_glGetShaderInfoLog(shader, bufsize, length, infolog);
	kpServer.on_glGetShaderInfoLog(shader, bufsize, length, infolog);
}

void          glGetShaderPrecisionFormat (GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision) {
	my_glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
	kpServer.on_glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
}

void          glGetShaderSource (GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source) {
	my_glGetShaderSource(shader, bufsize, length, source);
	kpServer.on_glGetShaderSource(shader, bufsize, length, source);
}

const GLubyte*  glGetString (GLenum name) {
	const GLubyte* tmp = my_glGetString(name);
	kpServer.on_glGetString(name, tmp);
	return tmp;
}

void          glGetTexParameterfv (GLenum target, GLenum pname, GLfloat* params) {
	my_glGetTexParameterfv(target, pname, params);
	kpServer.on_glGetTexParameterfv(target, pname, params);
}

void          glGetTexParameteriv (GLenum target, GLenum pname, GLint* params) {
	my_glGetTexParameteriv(target, pname, params);
	kpServer.on_glGetTexParameteriv(target, pname, params);
}

void          glGetUniformfv (GLuint program, GLint location, GLfloat* params) {
	my_glGetUniformfv(program, location, params);
	kpServer.on_glGetUniformfv(program, location, params);
}

void          glGetUniformiv (GLuint program, GLint location, GLint* params) {
	my_glGetUniformiv(program, location, params);
	kpServer.on_glGetUniformiv(program, location, params);
}

int           glGetUniformLocation (GLuint program, const GLchar* name) {
	int tmp = my_glGetUniformLocation(program, name);
	kpServer.on_glGetUniformLocation(program, name, tmp);
	return tmp;
}

void          glGetVertexAttribfv (GLuint index, GLenum pname, GLfloat* params) {
	my_glGetVertexAttribfv(index, pname, params);
	kpServer.on_glGetVertexAttribfv(index, pname, params);
}

void          glGetVertexAttribiv (GLuint index, GLenum pname, GLint* params) {
	my_glGetVertexAttribiv(index, pname, params);
	kpServer.on_glGetVertexAttribiv(index, pname, params);
}

void          glGetVertexAttribPointerv (GLuint index, GLenum pname, GLvoid** pointer) {
	my_glGetVertexAttribPointerv(index, pname, pointer);
	kpServer.on_glGetVertexAttribPointerv(index, pname, pointer);
}

void          glHint (GLenum target, GLenum mode) {
	my_glHint(target, mode);
	kpServer.on_glHint(target, mode);
}

GLboolean     glIsBuffer (GLuint buffer) {
	GLboolean tmp = my_glIsBuffer(buffer);
	kpServer.on_glIsBuffer(buffer, tmp);
	return tmp;
}

GLboolean     glIsEnabled (GLenum cap) {
	GLboolean tmp = my_glIsEnabled(cap);
	kpServer.on_glIsEnabled(cap, tmp);
	return tmp;
}

GLboolean     glIsFramebuffer (GLuint framebuffer) {
	GLboolean tmp = my_glIsFramebuffer(framebuffer);
	kpServer.on_glIsFramebuffer(framebuffer, tmp);
	return tmp;
}

GLboolean     glIsProgram (GLuint program) {
	GLboolean tmp = my_glIsProgram(program);
	kpServer.on_glIsProgram(program, tmp);
	return tmp;
}

GLboolean     glIsRenderbuffer (GLuint renderbuffer) {
	GLboolean tmp = my_glIsRenderbuffer(renderbuffer);
	kpServer.on_glIsRenderbuffer(renderbuffer, tmp);
	return tmp;
}

GLboolean     glIsShader (GLuint shader) {
	GLboolean tmp = my_glIsShader(shader);
	kpServer.on_glIsShader(shader, tmp);
	return tmp;
}

GLboolean     glIsTexture (GLuint texture) {
	GLboolean tmp = my_glIsTexture(texture);
	kpServer.on_glIsTexture(texture, tmp);
	return tmp;
}

void          glLineWidth (GLfloat width) {
	my_glLineWidth(width);
	kpServer.on_glLineWidth(width);
}

void          glLinkProgram (GLuint program) {
	my_glLinkProgram(program);
	kpServer.on_glLinkProgram(program);
}

void          glPixelStorei (GLenum pname, GLint param) {
	my_glPixelStorei(pname, param);
	kpServer.on_glPixelStorei(pname, param);
}

void          glPolygonOffset (GLfloat factor, GLfloat units) {
	my_glPolygonOffset(factor, units);
	kpServer.on_glPolygonOffset(factor, units);
}

void          glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels) {
	my_glReadPixels(x, y, width, height, format, type, pixels);
	kpServer.on_glReadPixels(x, y, width, height, format, type, pixels);
}

void          glReleaseShaderCompiler (void) {
	my_glReleaseShaderCompiler();
	kpServer.on_glReleaseShaderCompiler();
}

void          glRenderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
	my_glRenderbufferStorage(target, internalformat, width, height);
	kpServer.on_glRenderbufferStorage(target, internalformat, width, height);
}

void          glSampleCoverage (GLclampf value, GLboolean invert) {
	my_glSampleCoverage(value, invert);
	kpServer.on_glSampleCoverage(value, invert);
}

void          glScissor (GLint x, GLint y, GLsizei width, GLsizei height) {
	my_glScissor(x, y, width, height);
	kpServer.on_glScissor(x, y, width, height);
}

void          glShaderBinary (GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length) {
	my_glShaderBinary(n, shaders, binaryformat, binary, length);
	kpServer.on_glShaderBinary(n, shaders, binaryformat, binary, length);
}

void          glShaderSource (GLuint shader, GLsizei count, const GLchar** string, const GLint* length) {
	my_glShaderSource(shader, count, string, length);
	kpServer.on_glShaderSource(shader, count, string, length);
}

void          glStencilFunc (GLenum func, GLint ref, GLuint mask) {
	my_glStencilFunc(func, ref, mask);
	kpServer.on_glStencilFunc(func, ref, mask);
}

void          glStencilFuncSeparate (GLenum face, GLenum func, GLint ref, GLuint mask) {
	my_glStencilFuncSeparate(face, func, ref, mask);
	kpServer.on_glStencilFuncSeparate(face, func, ref, mask);
}

void          glStencilMask (GLuint mask) {
	my_glStencilMask(mask);
	kpServer.on_glStencilMask(mask);
}

void          glStencilMaskSeparate (GLenum face, GLuint mask) {
	my_glStencilMaskSeparate(face, mask);
	kpServer.on_glStencilMaskSeparate(face, mask);
}

void          glStencilOp (GLenum fail, GLenum zfail, GLenum zpass) {
	my_glStencilOp(fail, zfail, zpass);
	kpServer.on_glStencilOp(fail, zfail, zpass);
}

void          glStencilOpSeparate (GLenum face, GLenum fail, GLenum zfail, GLenum zpass) {
	my_glStencilOpSeparate(face, fail, zfail, zpass);
	kpServer.on_glStencilOpSeparate(face, fail, zfail, zpass);
}

#if defined OS_ANDROID
	void          glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels) {
		my_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
		kpServer.on_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
	}
#elif defined OS_TIZEN
	void          glTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels) {
		my_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
		kpServer.on_glTexImage2D(target, level, (GLint)internalformat, width, height, border, format, type, pixels);
	}
#endif

void          glTexParameterf (GLenum target, GLenum pname, GLfloat param) {
	my_glTexParameterf(target, pname, param);
	kpServer.on_glTexParameterf(target, pname, param);
}

void          glTexParameterfv (GLenum target, GLenum pname, const GLfloat* params) {
	my_glTexParameterfv(target, pname, params);
	kpServer.on_glTexParameterfv(target, pname, params);
}

void          glTexParameteri (GLenum target, GLenum pname, GLint param) {
	my_glTexParameteri(target, pname, param);
	kpServer.on_glTexParameteri(target, pname, param);
}

void          glTexParameteriv (GLenum target, GLenum pname, const GLint* params) {
	my_glTexParameteriv(target, pname, params);
	kpServer.on_glTexParameteriv(target, pname, params);
}

void          glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels) {
	my_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
	kpServer.on_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void          glUniform1f (GLint location, GLfloat x) {
	my_glUniform1f(location, x);
	kpServer.on_glUniform1f(location, x);
}

void          glUniform1fv (GLint location, GLsizei count, const GLfloat* v) {
	my_glUniform1fv(location, count, v);
	kpServer.on_glUniform1fv(location, count, v);
}

void          glUniform1i (GLint location, GLint x) {
	my_glUniform1i(location, x);
	kpServer.on_glUniform1i(location, x);
}

void          glUniform1iv (GLint location, GLsizei count, const GLint* v) {
	my_glUniform1iv(location, count, v);
	kpServer.on_glUniform1iv(location, count, v);
}

void          glUniform2f (GLint location, GLfloat x, GLfloat y) {
	my_glUniform2f(location, x, y);
	kpServer.on_glUniform2f(location, x, y);
}

void          glUniform2fv (GLint location, GLsizei count, const GLfloat* v) {
	my_glUniform2fv(location, count, v);
	kpServer.on_glUniform2fv(location, count, v);
}

void          glUniform2i (GLint location, GLint x, GLint y) {
	my_glUniform2i(location, x, y);
	kpServer.on_glUniform2i(location, x, y);
}

void          glUniform2iv (GLint location, GLsizei count, const GLint* v) {
	my_glUniform2iv(location, count, v);
	kpServer.on_glUniform2iv(location, count, v);
}

void          glUniform3f (GLint location, GLfloat x, GLfloat y, GLfloat z) {
	my_glUniform3f(location, x, y, z);
	kpServer.on_glUniform3f(location, x, y, z);
}

void          glUniform3fv (GLint location, GLsizei count, const GLfloat* v) {
	my_glUniform3fv(location, count, v);
	kpServer.on_glUniform3fv(location, count, v);
}

void          glUniform3i (GLint location, GLint x, GLint y, GLint z) {
	my_glUniform3i(location, x, y, z);
	kpServer.on_glUniform3i(location, x, y, z);
}

void          glUniform3iv (GLint location, GLsizei count, const GLint* v) {
	my_glUniform3iv(location, count, v);
	kpServer.on_glUniform3iv(location, count, v);
}

void          glUniform4f (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	my_glUniform4f(location, x, y, z, w);
	kpServer.on_glUniform4f(location, x, y, z, w);
}

void          glUniform4fv (GLint location, GLsizei count, const GLfloat* v) {
	my_glUniform4fv(location, count, v);
	kpServer.on_glUniform4fv(location, count, v);
}

void          glUniform4i (GLint location, GLint x, GLint y, GLint z, GLint w) {
	my_glUniform4i(location, x, y, z, w);
	kpServer.on_glUniform4i(location, x, y, z, w);
}

void          glUniform4iv (GLint location, GLsizei count, const GLint* v) {
	my_glUniform4iv(location, count, v);
	kpServer.on_glUniform4iv(location, count, v);
}

void          glUniformMatrix2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
	my_glUniformMatrix2fv(location, count, transpose, value);
	kpServer.on_glUniformMatrix2fv(location, count, transpose, value);
}

void          glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
	my_glUniformMatrix3fv(location, count, transpose, value);
	kpServer.on_glUniformMatrix3fv(location, count, transpose, value);
}

void          glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
	my_glUniformMatrix4fv(location, count, transpose, value);
	kpServer.on_glUniformMatrix4fv(location, count, transpose, value);
}

void          glUseProgram (GLuint program) {
	my_glUseProgram(program);
	kpServer.on_glUseProgram(program);
}

void          glValidateProgram (GLuint program) {
	my_glValidateProgram(program);
	kpServer.on_glValidateProgram(program);
}

void          glVertexAttrib1f (GLuint indx, GLfloat x) {
	my_glVertexAttrib1f(indx, x);
	kpServer.on_glVertexAttrib1f(indx, x);
}

void          glVertexAttrib1fv (GLuint indx, const GLfloat* values) {
	my_glVertexAttrib1fv(indx, values);
	kpServer.on_glVertexAttrib1fv(indx, values);
}

void          glVertexAttrib2f (GLuint indx, GLfloat x, GLfloat y) {
	my_glVertexAttrib2f(indx, x, y);
	kpServer.on_glVertexAttrib2f(indx, x, y);
}

void          glVertexAttrib2fv (GLuint indx, const GLfloat* values) {
	my_glVertexAttrib2fv(indx, values);
	kpServer.on_glVertexAttrib2fv(indx, values);
}

void          glVertexAttrib3f (GLuint indx, GLfloat x, GLfloat y, GLfloat z) {
	my_glVertexAttrib3f(indx, x, y, z);
	kpServer.on_glVertexAttrib3f(indx, x, y, z);
}

void          glVertexAttrib3fv (GLuint indx, const GLfloat* values) {
	my_glVertexAttrib3fv(indx, values);
	kpServer.on_glVertexAttrib3fv(indx, values);
}

void          glVertexAttrib4f (GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	my_glVertexAttrib4f(indx, x, y, z, w);
	kpServer.on_glVertexAttrib4f(indx, x, y, z, w);
}

void          glVertexAttrib4fv (GLuint indx, const GLfloat* values) {
	my_glVertexAttrib4fv(indx, values);
	kpServer.on_glVertexAttrib4fv(indx, values);
}

void          glVertexAttribPointer (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr) {
	my_glVertexAttribPointer(indx, size, type, normalized, stride, ptr);
	kpServer.on_glVertexAttribPointer(indx, size, type, normalized, stride, ptr);
}

void          glViewport (GLint x, GLint y, GLsizei width, GLsizei height) {
	my_glViewport(x, y, width, height);
	kpServer.on_glViewport(x, y, width, height);
}

//=====================================================================
// Extensions

void* glMapBufferOES (GLenum target, GLenum access)
{
	void* tmp = my_glMapBufferOES(target, access);
	kpServer.on_glMapBufferOES(target, access, tmp);
	return tmp;
}

GLboolean glUnmapBufferOES (GLenum target)
{
	kpServer.on_glUnmapBufferOES(target);
	my_glUnmapBufferOES(target);
}


#endif
