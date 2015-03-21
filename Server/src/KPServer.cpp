#include "kp_hook.h"

#include "KPServer.h"

#include "KPHelper.h"

KPServer::KPServer() :
		m_state(IDLE),
		m_client(-1)
{
	
}
		
KPServer::~KPServer()
{
	for (int i = 0; i < MAX_PROGRAMS_NUMBER; i++)
	{
		m_pProgramsList[i].clearData();
	}
	for (int i = 0; i < MAX_SHADERS_NUMBER; i++)
	{
		m_pShadersList[i].clearData();
	}
	for (int i = 0; i < MAX_TEXTURES_NUMBER; i++)
	{
		m_pTexturesList[i].clearData();
	}
	for (int i = 0; i < MAX_VBOS_NUMBER; i++)
	{
		m_pVbosList[i].clearData();
	}
}

void KPServer::makeGLCommandMsg(KPMessageType type)
{
	m_msg.clearData();
	
	m_msg.m_type = type;
}

ServerState KPServer::getState()
{
	KPHelper::LockMutex0();
	ServerState state = m_state;
	KPHelper::UnlockMutex0();
	return state;
}

void KPServer::setState(ServerState state)
{
	KPHelper::LockMutex0();
	m_state = state;
	KPHelper::UnlockMutex0();
}

void KPServer::setClient(int client)
{
	m_client = client;
}

KPProgram* KPServer::getProgram(u32 id)
{
	if (id == 0) return NULL;
	for (int i = 0; i < MAX_PROGRAMS_NUMBER; i++)
	{
		if (m_pProgramsList[i].getId() == id) return &m_pProgramsList[i];
	}
	return NULL;
}

KPProgram* KPServer::getFreeProgram()
{
	for (int i = 0; i < MAX_PROGRAMS_NUMBER; i++)
	{
		if (m_pProgramsList[i].getId() == 0) return &m_pProgramsList[i];
	}
	return NULL;
}

KPShader* KPServer::getShader(u32 id)
{
	if (id == 0) return NULL;
	for (int i = 0; i < MAX_SHADERS_NUMBER; i++)
	{
		if (m_pShadersList[i].getId() == id) return &m_pShadersList[i];
	}
	return NULL;
}

KPShader* KPServer::getFreeShader()
{
	for (int i = 0; i < MAX_SHADERS_NUMBER; i++)
	{
		if (m_pShadersList[i].getId() == 0) return &m_pShadersList[i];
	}
	return NULL;
}

KPTexture* KPServer::getTexture(u32 id)
{
	if (id == 0) return NULL;
	for (int i = 0; i < MAX_TEXTURES_NUMBER; i++)
	{
		if (m_pTexturesList[i].getId() == id) return &m_pTexturesList[i];
	}
	return NULL;
}

KPTexture* KPServer::getFreeTexture()
{
	for (int i = 0; i < MAX_TEXTURES_NUMBER; i++)
	{
		if (m_pTexturesList[i].getId() == 0) return &m_pTexturesList[i];
	}
	return NULL;
}

KPVbo* KPServer::getVbo(u32 id)
{
	if (id == 0) return NULL;
	for (int i = 0; i < MAX_VBOS_NUMBER; i++)
	{
		if (m_pVbosList[i].getId() == id) return &m_pVbosList[i];
	}
	return NULL;
}

KPVbo* KPServer::getFreeVbo()
{
	for (int i = 0; i < MAX_VBOS_NUMBER; i++)
	{
		if (m_pVbosList[i].getId() == 0) return &m_pVbosList[i];
	}
	return NULL;
}

KPOptions& KPServer::getOptions()
{
	return m_options;
}

//========================================================================================

void KPServer::onSwapBuffers()
{
	KPHelper::LockMutex0();

	switch (m_state)
	{
		case PRE_CAPTURE:
		{
			bool error = false;
			for (int i = 0; i < MAX_PROGRAMS_NUMBER; i++) if (m_pProgramsList[i].getId())
			{
				if ( KPHelper::sendGLObjectToClient(m_client, m_pProgramsList[i]) == SOCKET_ERROR )
				{
					error = true;
					goto my_end;
				}
			}
			
			for (int i = 0; i < MAX_SHADERS_NUMBER; i++) if (m_pShadersList[i].getId())
			{
				if ( KPHelper::sendGLObjectToClient(m_client, m_pShadersList[i]) == SOCKET_ERROR  )
				{
					error = true;
					goto my_end;
				}
			}
			
			for (int i = 0; i < MAX_TEXTURES_NUMBER; i++) if (m_pTexturesList[i].getId())
			{
				if ( KPHelper::sendGLObjectToClient(m_client, m_pTexturesList[i]) == SOCKET_ERROR )
				{
					error = true;
					goto my_end;
				}
			}
			
			for (int i = 0; i < MAX_VBOS_NUMBER; i++) if (m_pVbosList[i].getId())
			{
				if ( KPHelper::sendGLObjectToClient(m_client, m_pVbosList[i]) == SOCKET_ERROR )
				{
					error = true;
					goto my_end;
				}
			}
		
			if ( KPHelper::sendCurrentTexturesStatus(m_client) == SOCKET_ERROR )
			{
				error = true;
				goto my_end;
			}
			if ( KPHelper::sendCurrentUsingProgram(m_client) == SOCKET_ERROR )
			{
				error = true;
				goto my_end;
			}
			if ( KPHelper::sendCurrentBindingVbos(m_client) == SOCKET_ERROR )
			{
				error = true;
				goto my_end;
			}

			my_end:

			if (error)
			{
				LOGE("error [%s]", __FUNCTION__);
				m_state = IDLE;
			}
			else
			{
				m_state = CAPTURING;
			}
			break;
		}

		case CAPTURING:
		{
			KPHelper::sendRequestFinishMessageToClient(m_client);
			KPHelper::closeSocket(m_client);

			m_state = IDLE;

			LOGD("================== CAPTURING end ====================");
			break;
		}

		case MODIFYING_PROGRAM:
		{
			LOGD("MODIFYING_PROGRAM");

			//---------------------------------------------------------------------------------------------
			// Receive data from client
			u32 progId, vsId, fsId;
			s32 vsSourceLen, fsSourceLen;
			char* vsSourceString = NULL;
			char* fsSourceString = NULL;
			int len = 0;
			KPProgram* pOldProg = NULL;
			KPProgram newProg;
			char* error = NULL;

			//---------------------------------------------------------------------------------------------
			// Read id(s)
			if ( !KPHelper::recvSocket(m_client, &progId, 4) ) goto my_end_modifying_program;
			if ( !KPHelper::recvSocket(m_client, &vsId, 4) ) goto my_end_modifying_program;
			if ( !KPHelper::recvSocket(m_client, &fsId, 4) ) goto my_end_modifying_program;

			//---------------------------------------------------------------------------------------------
			// Read vsSourceString
			if ( !KPHelper::recvSocket(m_client, &len, 4) ) goto my_end_modifying_program;
			if (len < 0) len = 0;
			vsSourceString = new char[len + 1];
			if (len > 0)
			{
				if ( !KPHelper::recvSocket(m_client, vsSourceString, len) ) goto my_end_modifying_program;
			}
			vsSourceString[len] = 0;
			vsSourceLen = len;

			//---------------------------------------------------------------------------------------------
			// Read fsSourceString
			if ( !KPHelper::recvSocket(m_client, &len, 4) ) goto my_end_modifying_program;
			if (len < 0) len = 0;
			fsSourceString = new char[len + 1];
			if (len > 0)
			{
				if ( !KPHelper::recvSocket(m_client, fsSourceString, len) ) goto my_end_modifying_program;
			}
			fsSourceString[len] = 0;
			fsSourceLen = len;

			//--------------------------------------------------------------------------------------------
			// Process the data received

			pOldProg = getProgram(progId);
			KPHelper::assert(pOldProg != NULL);
			
			error = KPHelper::checkValidModifiedProgram(progId, vsId, fsId,
												vsSourceString, fsSourceString, pOldProg, newProg);

			if (error)
			{
				KPHelper::sendRequestErrorMessageToClient(m_client, error);
			}
			else
			{
				//--------------------------------------------------------------------------------------------
				// Compile the new shader sources

				my_glShaderSource(vsId, 1, (const char**)&vsSourceString, NULL);
				my_glCompileShader(vsId);
				my_glShaderSource(fsId, 1, (const char**)&fsSourceString, NULL);
				my_glCompileShader(fsId);

				//--------------------------------------------------------------------------------------------
				// Copy/bind location of attributes
				int count = pOldProg->getAttributesCount();
				for (int i = 0; i < count; i++)
				{
					const KPAttribute& att = pOldProg->getAttribute(i);
					my_glBindAttribLocation(progId, att.m_location, att.m_szName);
				}

				my_glLinkProgram(progId);

				//--------------------------------------------------------------------------------------------
				u32 oldUsingProgram = KPHelper::getCurrentUsingProgram();
				my_glUseProgram(progId);
				// Copy/upload/restore value of uniforms
				KPHelper::copyProgramUniformsStateToGpu(newProg, *pOldProg);

				my_glUseProgram(oldUsingProgram);

				// NOTE this
				//pOldProg->link();
				//

				getShader(vsId)->setSource(vsSourceString);
				getShader(fsId)->setSource(fsSourceString);

				KPHelper::sendRequestFinishMessageToClient(m_client);
			}

			my_end_modifying_program:

			KPHelper::closeSocket(m_client);

			SAFE_DEL_ARRAY(vsSourceString);
			SAFE_DEL_ARRAY(fsSourceString);

			SAFE_DEL_ARRAY(error);

			m_state = IDLE;
			break;
		}
	} // switch

	KPHelper::UnlockMutex0();
}

//========================================================================================
//========================================================================================
//========================================================================================
//========================================================================================

void KPServer::on_glActiveTexture(GLenum texture)
{
	KPHelper::LockMutex0();

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glActiveTexture);
	
		m_msg.m_length = 4 + 1;
		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &texture, 4); buffer += 4;

		u8 actual = KPHelper::getCurrentActiveTextureUnit();
		memcpy(buffer, &actual, 1); buffer += 1;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}

	KPHelper::UnlockMutex0();
}

void KPServer::on_glAttachShader(GLuint program, GLuint shader)
{
	KPHelper::LockMutex0();
	u8 error = 0;

	KPProgram* prog = getProgram(program);
	if (prog)
	{
		KPShader* pshader = getShader(shader);
		if (pshader)
		{
			if (pshader->getType() == GL_VERTEX_SHADER)
			{
				if (!prog->attachVs(shader))
				{
					error = 1;
					LOGE("error [%s] can not attach vs=%d, program=%d", __FUNCTION__, shader, program);
				}
			}
			else
			{
				if (!prog->attachFs(shader))
				{
					error = 1;
					LOGE("error [%s] can not attach fs=%d, program=%d", __FUNCTION__, shader, program);
				}
			}
		}
		else
		{
			error = 1;
			LOGE("error [%s] not found the shader id = %d", __FUNCTION__, shader);
		}
	}
	else
	{
		error = 1;
		LOGE("error [%s] not found the program id = %d", __FUNCTION__, program);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glAttachShader);
	
		m_msg.m_length = 1 + 4 * 2;
		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);					buffer += 1;
		memcpy(buffer, &program, 4);				buffer += 4;
		memcpy(buffer, &shader, 4);					buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glBindAttribLocation(GLuint program, GLuint index, const GLchar* name)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glBindAttribLocation);
	
		int strLen = name ? strlen(name) : 0;

		m_msg.m_length = 4 + 4 + strLen;
		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &program, 4);			buffer += 4;
		memcpy(buffer, &index, 4);				buffer += 4;
		if (strLen > 0)
		{
			memcpy(buffer, name, strLen);
			buffer += strLen;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glBindBuffer(GLenum target, GLuint buffer)
{
	KPHelper::LockMutex0();

	u8 error = 0;
	if (target != GL_ARRAY_BUFFER && target != GL_ELEMENT_ARRAY_BUFFER)
	{
		LOGE("error [%s] invalid target = 0x%x", __FUNCTION__, target);
		error = 1;
		goto my_end;
	}

	if (buffer > 0 && !getVbo(buffer))
	{
		LOGE("error [%s] not found the buffer id = %d", __FUNCTION__, buffer);
		error = 1;
		goto my_end;
	}

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glBindBuffer);
	
		m_msg.m_length = 1 + 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		char* _buffer = m_msg.m_pData;

		memcpy(_buffer, &error, 1);					_buffer += 1;
		memcpy(_buffer, &target, 4);				_buffer += 4;
		memcpy(_buffer, &buffer, 4);				_buffer += 4;
		u32 actualBoundBuffer = 0;
		if (target == GL_ARRAY_BUFFER)				actualBoundBuffer = KPHelper::getCurrentBindingVbo_ArrayBuffer();
		else if (target == GL_ELEMENT_ARRAY_BUFFER)	actualBoundBuffer = KPHelper::getCurrentBindingVbo_ElementArrayBuffer();
		memcpy(_buffer, &actualBoundBuffer, 4);		_buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glBindFramebuffer(GLenum target, GLuint framebuffer)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glBindFramebuffer);
	
		m_msg.m_length = 4 * 2;
		m_msg.m_pData = new char[m_msg.m_length];
		memcpy(m_msg.m_pData, &target, 4);
		memcpy(m_msg.m_pData + 4, &framebuffer, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glBindRenderbuffer);
	
		m_msg.m_length = 4 * 2;
		m_msg.m_pData = new char[m_msg.m_length];
		memcpy(m_msg.m_pData, &target, 4);
		memcpy(m_msg.m_pData + 4, &renderbuffer, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glBindTexture(GLenum target, GLuint texture)
{
	KPHelper::LockMutex0();

	u8 error = 0;
	KPTexture* tex;

	if (target != GL_TEXTURE_2D && target != GL_TEXTURE_CUBE_MAP)
	{
		LOGE("error [%s] invalid target = 0x%x", __FUNCTION__, target);
		error = 1;
		goto my_end;
	}

	if (texture > 0)
	{
		tex = getTexture(texture);
		if (!tex)
		{
			LOGE("error [%s] not found the texture id = %d", __FUNCTION__, texture);
			error = 1;
			goto my_end;
		}
		KPTextureType texType = tex->getTexType();
		if (texType != TEX_NONE)
		{
			if (target == GL_TEXTURE_2D && texType != TEX_2D)
			{
				LOGE("error [%s] can not bind a non-2D texture to target GL_TEXTURE_2D", __FUNCTION__);
				error = 1;
				goto my_end;
			}
			if (target == GL_TEXTURE_CUBE_MAP && texType != TEX_CUBE_MAP)
			{
				LOGE("error [%s] can not bind a non-CubeMap texture to target GL_TEXTURE_CUBE_MAP", __FUNCTION__);
				error = 1;
				goto my_end;
			}
		}
		else
		{
			tex->setTexType(target == GL_TEXTURE_2D ? TEX_2D : TEX_CUBE_MAP);
		}
	}

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glBindTexture);
	
		m_msg.m_length = 1 + 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);					buffer += 1;
		memcpy(buffer, &target, 4);					buffer += 4;
		memcpy(buffer, &texture, 4);				buffer += 4;

		u32 actual = 0;
		if (target == GL_TEXTURE_2D)			actual = KPHelper::getCurrentBindingTexture2D();
		else if (target == GL_TEXTURE_CUBE_MAP)	actual = KPHelper::getCurrentBindingTextureCubeMap();
		memcpy(buffer, &actual, 4);					buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glBlendColor);

		m_msg.m_length = 4 * 4;
		m_msg.m_pData = new char[m_msg.m_length];
		memcpy(m_msg.m_pData, &red, 4);
		memcpy(m_msg.m_pData + 4, &green, 4);
		memcpy(m_msg.m_pData + 8, &blue, 4);
		memcpy(m_msg.m_pData + 12, &alpha, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glBlendEquation(GLenum mode)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glBlendEquation);
	
		m_msg.m_length = 4;
		m_msg.m_pData = new char[m_msg.m_length];
		memcpy(m_msg.m_pData, &mode, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glBlendEquationSeparate);
	
		m_msg.m_length = 4 * 2;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &modeRGB, 4);
		memcpy(m_msg.m_pData + 4, &modeAlpha, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glBlendFunc(GLenum sfactor, GLenum dfactor)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glBlendFunc);

		m_msg.m_length = 4 * 2;
		m_msg.m_pData = new char[m_msg.m_length];
		memcpy(m_msg.m_pData, &sfactor, 4);
		memcpy(m_msg.m_pData + 4, &dfactor, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glBlendFuncSeparate);
	
		m_msg.m_length = 4 * 4;
		m_msg.m_pData = new char[m_msg.m_length];
		memcpy(m_msg.m_pData, &srcRGB, 4);
		memcpy(m_msg.m_pData + 4, &dstRGB, 4);
		memcpy(m_msg.m_pData + 8, &srcAlpha, 4);
		memcpy(m_msg.m_pData + 12, &dstAlpha, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glBufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
{
	KPHelper::LockMutex0();

	u32 currentBindingVbo;
	KPVbo* vbo;
	u8 error = 0;

	if (target != GL_ARRAY_BUFFER && target != GL_ELEMENT_ARRAY_BUFFER)
	{
		LOGE("error [%s] Unknown target = 0x%x", __FUNCTION__, target);
		error = 1;
		goto my_end;
	}

	if (size < 0)
	{
		LOGE("error [%s] size is negative, size = %d", __FUNCTION__, size);
		error = 1;
		goto my_end;
	}

	if (usage != GL_STREAM_DRAW && usage != GL_STATIC_DRAW && usage != GL_DYNAMIC_DRAW)
	{
		LOGE("error [%s] Unknown usage = 0x%x", __FUNCTION__, usage);
		error = 1;
		goto my_end;
	}

	currentBindingVbo = KPHelper::getCurrentBindingVboOfTarget(target);

	vbo = getVbo(currentBindingVbo);
	if (vbo)
	{
		vbo->on_glBufferData(size, data, usage);
	}
	else
	{
		LOGE("error [%s] Not found the vbo id: %d", __FUNCTION__, currentBindingVbo);
		error = 1;
		goto my_end;
	}

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glBufferData);
	
		m_msg.m_length = 1/*error*/ + 4/*currentBindingVbo*/ + 4 + 4 + 4 + 4;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;
		
		memcpy(buffer, &error, 1);					buffer += 1;
		memcpy(buffer, &currentBindingVbo, 4);		buffer += 4;
		memcpy(buffer, &target, 4);					buffer += 4;
		memcpy(buffer, &size, 4);					buffer += 4;
		memcpy(buffer, &data, 4);					buffer += 4;
		memcpy(buffer, &usage, 4);					buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data)
{
	KPHelper::LockMutex0();

	u32 currentBindingVbo;
	KPVbo* vbo;
	u8 error = 0;

	if (target != GL_ARRAY_BUFFER && target != GL_ELEMENT_ARRAY_BUFFER)
	{
		LOGE("error [%s] Unknown target = 0x%x", __FUNCTION__, target);
		error = 1;
		goto my_end;
	}

	if (size < 0 || offset < 0)
	{
		LOGE("error [%s] size or offset is negative, size = %d, offset = %d", __FUNCTION__, size, offset);
		error = 1;
		goto my_end;
	}

	if (!data)
	{
		LOGE("error [%s] data is NULL", __FUNCTION__);
		error = 1;
		goto my_end;
	}

	currentBindingVbo = KPHelper::getCurrentBindingVboOfTarget(target);

	vbo = getVbo(currentBindingVbo);
	if (vbo)
	{
		int prevSize = vbo->getSize();
		if ( offset + size <= prevSize )
		{
			vbo->on_glBufferSubData(offset, size, data);
		}
		else
		{
			LOGE("error [%s] extends beyond the buffer object's allocated data store: offset=%d, size=%d, prevSize=%d",
				__FUNCTION__, offset, size, prevSize);
			error = 1;
			goto my_end;
		}
	}
	else
	{
		LOGE("error [%s] Not found the vbo id: %d", __FUNCTION__, currentBindingVbo);
		error = 1;
		goto my_end;
	}

	my_end:
	
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glBufferSubData);
	
		m_msg.m_length = 1/*error*/ + 4/*currentBindingVbo*/ + 4 + 4 + 4 + 4;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);					buffer += 1;
		memcpy(buffer, &currentBindingVbo, 4);		buffer += 4;
		memcpy(buffer, &target, 4);					buffer += 4;
		memcpy(buffer, &offset, 4);					buffer += 4;
		memcpy(buffer, &size, 4);					buffer += 4;
		memcpy(buffer, &data, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glCheckFramebufferStatus(GLenum target, GLenum result)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glCheckFramebufferStatus);

		m_msg.m_length = 4 * 2;
		m_msg.m_pData = new char[m_msg.m_length];
		memcpy(m_msg.m_pData, &target, 4);
		memcpy(m_msg.m_pData + 4, &result, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glClear(GLbitfield mask)
{
	KPHelper::LockMutex0();

	char* pixels = NULL;
	int width, height;

	if (m_state == CAPTURING)
	{
		int x, y;
		KPHelper::getViewport(x, y, width, height);
		pixels = new char[width * height * 4];
		KPHelper::readPixels(x, y, width, height, pixels);

		int widthScaled, heightScaled, imageLen;
		char* screenShot = KPHelper::makeScreenshot_RGB(pixels, width, height, true, true, widthScaled, heightScaled, imageLen);

		//
		makeGLCommandMsg(KMT_glClear);
	
		m_msg.m_length = 4/*fbo*/ + 4 + 4 * 2 + 4 * 2 + 4 + imageLen;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		u32 fbo = KPHelper::getCurrentBindingFbo();
		memcpy(buffer, &fbo, 4);				buffer += 4;

		memcpy(buffer, &mask, 4);				buffer += 4;

		memcpy(buffer, &width, 4);				buffer += 4;
		memcpy(buffer, &height, 4);				buffer += 4;

		memcpy(buffer, &widthScaled, 4);		buffer += 4;
		memcpy(buffer, &heightScaled, 4);		buffer += 4;

		memcpy(buffer, &imageLen, 4);			buffer += 4;

		if (screenShot && imageLen > 0)
		{
			memcpy(buffer, screenShot, imageLen);
		}

		SAFE_DEL_ARRAY(screenShot);
		
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}

	//copyCurrentFboToTexture(pixels, width, height);

	SAFE_DEL_ARRAY(pixels);

	KPHelper::UnlockMutex0();
}

void KPServer::on_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glClearColor);
	
		m_msg.m_length = 4 * 4;
		m_msg.m_pData = new char[m_msg.m_length];
		memcpy(m_msg.m_pData, &red, 4);
		memcpy(m_msg.m_pData + 4, &green, 4);
		memcpy(m_msg.m_pData + 8, &blue, 4);
		memcpy(m_msg.m_pData + 12, &alpha, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glClearDepthf(GLclampf depth)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glClearDepthf);
	
		m_msg.m_length = 4;
		m_msg.m_pData = new char[m_msg.m_length];
		memcpy(m_msg.m_pData, &depth, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glClearStencil(GLint s)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glClearStencil);
	
		m_msg.m_length = 4;
		m_msg.m_pData = new char[m_msg.m_length];
		memcpy(m_msg.m_pData, &s, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glColorMask);
	
		m_msg.m_length = 4;
		m_msg.m_pData = new char[m_msg.m_length];
		memcpy(m_msg.m_pData, &red, 1);
		memcpy(m_msg.m_pData + 1, &green, 1);
		memcpy(m_msg.m_pData + 2, &blue, 1);
		memcpy(m_msg.m_pData + 3, &alpha, 1);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glCompileShader(GLuint shader)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glCompileShader);
	
		m_msg.m_length = 4;
		m_msg.m_pData = new char[m_msg.m_length];
		memcpy(m_msg.m_pData, &shader, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat,
				GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data)
{
	KPHelper::LockMutex0();

	const int maxTexSize = KPHelper::get_MAX_TEXTURE_SIZE();
	u32 id = 0;
	KPTexture* tex;
	u8 error = 0;

	if (target != GL_TEXTURE_2D &&
        target != GL_TEXTURE_CUBE_MAP_POSITIVE_X &&
        target != GL_TEXTURE_CUBE_MAP_NEGATIVE_X &&
        target != GL_TEXTURE_CUBE_MAP_POSITIVE_Y &&
        target != GL_TEXTURE_CUBE_MAP_NEGATIVE_Y &&
        target != GL_TEXTURE_CUBE_MAP_POSITIVE_Z &&
        target != GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
	{
		LOGE("error [%s] Unknown target = 0x%x", __FUNCTION__, target);
		error = 1;
		goto my_end;
	}

	id = target == GL_TEXTURE_2D ? KPHelper::getCurrentBindingTexture2D() : KPHelper::getCurrentBindingTextureCubeMap();

	LOGD("[%s] id=%d, target=0x%x, level=%d, internalformat=0x%x, width=%d, height=%d, border=%d, imageSize=%d, data=%p",
		__FUNCTION__, id, target, level, internalformat, width, height, border, imageSize, data);

	if (level < 0 || level >= MAX_MIPMAP_LEVEL_NUMBER || level > KPHelper::getMaxMipLevel())
	{
		LOGE("error [%s] Invalid level = %d", __FUNCTION__, level);
		error = 1;
		goto my_end;
	}

	if (width < 0 || height < 0 || width > maxTexSize || height > maxTexSize)
	{
		LOGE("error [%s] width = %d, height = %d, maxTexSize = %d", __FUNCTION__, width, height, maxTexSize);
		error = 1;
		goto my_end;
	}

	if (border != 0)
	{
		LOGE("error [%s] Invalid border = %d", __FUNCTION__, border);
		error = 1;
		goto my_end;
	}

	if (imageSize < 0)
	{
		LOGE("error [%s] Invalid imageSize = %d", __FUNCTION__, imageSize);
		error = 1;
		goto my_end;
	}
		
	tex = getTexture(id);
	if (!tex)
	{
		LOGE("error [%s] Not found the texture id = %d", __FUNCTION__, id);
		error = 1;
		goto my_end;
	}

	tex->on_glCompressedTexImage2D(level,  internalformat,  width,  height, border,  imageSize ,  data, m_options.m_isRecordTexturePixel );

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glCompressedTexImage2D);
		m_msg.m_length = 1/*error*/ + 4/*id*/ + 4/*target*/ + 4/*level*/ + 4/*internalformat*/ + 4/*width*/ + 4/*height*/ +
							4/*border*/ + 4/*imageSize*/ + 4 /*data pointer address*/;
		if (!error && imageSize > 0)
		{
			m_msg.m_length += imageSize;
		}

		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);				buffer += 1;
		memcpy(buffer, &id, 4);					buffer += 4;
		memcpy(buffer, &target, 4);				buffer += 4;
		memcpy(buffer, &level, 4);				buffer += 4;
		memcpy(buffer, &internalformat, 4);		buffer += 4;
		memcpy(buffer, &width, 4);				buffer += 4;
		memcpy(buffer, &height, 4);				buffer += 4;
		memcpy(buffer, &border, 4);				buffer += 4;
		memcpy(buffer, &imageSize, 4);			buffer += 4;
		memcpy(buffer, &data, 4);				buffer += 4;

		if (!error && imageSize > 0)
		{
			if (data)
				memcpy(buffer, data, imageSize);
			else
				memset(buffer, 0, imageSize);
			buffer += imageSize;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}

	KPHelper::UnlockMutex0();
}

void KPServer::on_glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset,
				GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data)
{
	KPHelper::LockMutex0();

	LOGW("============== [Not supported] TODO [%s] is called ====================", __FUNCTION__);

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glCompressedTexSubImage2D);
		m_msg.m_length = 4/*target*/ + 4/*level*/ + 4/*xoffset*/ + 4/*yoffset*/ +
						4/*width*/ + 4/*height*/ + 4/*format*/ + 4/*imageSize*/ + 4/*data pointer address*/;

		m_msg.m_pData = new char[m_msg.m_length];
		
		char* buffer = m_msg.m_pData;
		
		memcpy(buffer, &target, 4); // 1
		buffer += 4;

		memcpy(buffer, &level, 4); // 2
		buffer += 4;

		memcpy(buffer, &xoffset, 4); // 3
		buffer += 4;

		memcpy(buffer, &yoffset, 4); // 4
		buffer += 4;

		memcpy(buffer, &width, 4); // 5
		buffer += 4;

		memcpy(buffer, &height, 4); // 6
		buffer += 4;

		memcpy(buffer, &format, 4); // 7
		buffer += 4;

		memcpy(buffer, &imageSize, 4); // 8
		buffer += 4;

		memcpy(buffer, &data, 4); // 9
		buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat,
	GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
	KPHelper::LockMutex0();

	LOGI("_______NOTE: [%s] is called________", __FUNCTION__);

	const int maxTexSize = KPHelper::get_MAX_TEXTURE_SIZE();
	u32 id = 0;
	KPTexture* tex;
	int pixelsLen, w2, h2;
	char* pixels = NULL;
	u8 error = 0;

	if (my_glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOGE("error [%s] currently bound framebuffer is not framebuffer complete", __FUNCTION__);
		error = 1;
		goto my_end;
	}

	if (target != GL_TEXTURE_2D &&
        target != GL_TEXTURE_CUBE_MAP_POSITIVE_X &&
        target != GL_TEXTURE_CUBE_MAP_NEGATIVE_X &&
        target != GL_TEXTURE_CUBE_MAP_POSITIVE_Y &&
        target != GL_TEXTURE_CUBE_MAP_NEGATIVE_Y &&
        target != GL_TEXTURE_CUBE_MAP_POSITIVE_Z &&
        target != GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
	{
		LOGE("error [%s] Unknown target = 0x%x", __FUNCTION__, target);
		error = 1;
		goto my_end;
	}

	id = target == GL_TEXTURE_2D ? KPHelper::getCurrentBindingTexture2D() : KPHelper::getCurrentBindingTextureCubeMap();

	if (target != GL_TEXTURE_2D)
	{
		if (width != height)
		{
			LOGE("error [%s] target = 0x%x and width = %d, height = %d", __FUNCTION__, target, width, height);
			error = 1;
			goto my_end;
		}
	}

	if (level < 0 || level >= MAX_MIPMAP_LEVEL_NUMBER || level > KPHelper::getMaxMipLevel())
	{
		LOGE("error [%s] Invalid level = %d", __FUNCTION__, level);
		error = 1;
		goto my_end;
	}

	if (internalformat != GL_ALPHA && internalformat != GL_LUMINANCE && internalformat != GL_LUMINANCE_ALPHA &&
		internalformat != GL_RGB && internalformat != GL_RGBA)
	{
		LOGE("error [%s] internalformat = 0x%x", __FUNCTION__, internalformat);
		error = 1;
		goto my_end;
	}

	if (width < 0 || height < 0 || width > maxTexSize || height > maxTexSize)
	{
		LOGE("error [%s] width = %d, height = %d", __FUNCTION__, width, height);
		error = 1;
		goto my_end;
	}

	if (border != 0)
	{
		LOGE("error [%s] border = %d", __FUNCTION__, border);
		error = 1;
		goto my_end;
	}

	tex = getTexture(id);
	if (!tex)
	{
		LOGE("error [%s] Not found the texture id = %d", __FUNCTION__, id);
		error = 1;
		goto my_end;
	}

	pixelsLen = width * height * 4;
	pixels = new char[pixelsLen];
	KPHelper::readPixels(x, y, width, height, pixels);

	if (internalformat != GL_RGBA)
	{
		char* newPixels = KPHelper::convertTexFormatFromRgba(width, height, internalformat, pixels, pixelsLen);
		SAFE_DEL_ARRAY(pixels);
		pixels = newPixels;
	}

	tex->on_glTexImage2D(level, internalformat, width, height, border, internalformat, GL_UNSIGNED_BYTE, pixels,
		m_options.m_isRecordTexturePixel);

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glCopyTexImage2D);
		m_msg.m_length = 1/*error*/ + 4/*id*/ + 4/*target*/ + 4/*level*/ + 4/*internalformat*/ +
						4/*x*/ + 4/*y*/ + 4/*width*/ + 4/*height*/ + 4/*border*/;
		if (!error && pixelsLen > 0)
		{
			m_msg.m_length += pixelsLen;
		}

		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);						buffer += 1;
		memcpy(buffer, &id, 4);							buffer += 4;
		memcpy(buffer, &target, 4);						buffer += 4;
		memcpy(buffer, &level, 4);						buffer += 4;
		memcpy(buffer, &internalformat, 4);				buffer += 4;
		memcpy(buffer, &x, 4);							buffer += 4;
		memcpy(buffer, &y, 4);							buffer += 4;
		memcpy(buffer, &width, 4);						buffer += 4;
		memcpy(buffer, &height, 4);						buffer += 4;
		memcpy(buffer, &border, 4);						buffer += 4;

		if (!error && pixelsLen > 0)
		{
			memcpy(buffer, pixels, pixelsLen);
			if (internalformat == GL_RGBA || internalformat == GL_RGB)
			{
				KPHelper::swapRB(buffer, width * height, internalformat == GL_RGBA);
			}
			buffer += pixelsLen;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}

	SAFE_DEL_ARRAY(pixels);

	KPHelper::UnlockMutex0();
}

void KPServer::on_glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset,
										GLint x, GLint y, GLsizei width, GLsizei height)
{
	KPHelper::LockMutex0();

	LOGI("_______NOTE: [%s] is called________", __FUNCTION__);

	u32 id = 0;
	KPTexture* tex;
	int pixelsLen, w2, h2;
	char* pixels = NULL;
	u8 error = 0;
	KPMipmapLevel* mip;

	if (my_glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOGE("error [%s] currently bound framebuffer is not framebuffer complete", __FUNCTION__);
		error = 1;
		goto my_end;
	}

	if (target != GL_TEXTURE_2D &&
        target != GL_TEXTURE_CUBE_MAP_POSITIVE_X &&
        target != GL_TEXTURE_CUBE_MAP_NEGATIVE_X &&
        target != GL_TEXTURE_CUBE_MAP_POSITIVE_Y &&
        target != GL_TEXTURE_CUBE_MAP_NEGATIVE_Y &&
        target != GL_TEXTURE_CUBE_MAP_POSITIVE_Z &&
        target != GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
	{
		LOGE("error [%s] target = 0x%x", __FUNCTION__, target);
		error = 1;
		goto my_end;
	}

	id = target == GL_TEXTURE_2D ? KPHelper::getCurrentBindingTexture2D() : KPHelper::getCurrentBindingTextureCubeMap();

	if (level < 0 || level >= MAX_MIPMAP_LEVEL_NUMBER || level > KPHelper::getMaxMipLevel())
	{
		LOGE("error [%s] level = %d", __FUNCTION__, level);
		error = 1;
		goto my_end;
	}

	if (xoffset < 0 || yoffset < 0)
	{
		LOGE("error [%s] xoffset = %d, yoffset = %d", __FUNCTION__, xoffset, yoffset);
		error = 1;
		goto my_end;
	}

	if (width < 0 || height < 0)
	{
		LOGE("error [%s] width = %d, height = %d", __FUNCTION__, width, height);
		error = 1;
		goto my_end;
	}

	tex = getTexture(id);
	if (!tex)
	{
		LOGE("error [%s] Not found the texture id = %d", __FUNCTION__, id);
		error = 1;
		goto my_end;
	}

	mip = tex->getMip(level);

	if (!mip->m_hasData)
	{
		LOGE("error [%s] mipmap does not have data", __FUNCTION__);
		error = 1;
		goto my_end;
	}

	if (mip->m_isCompressed)
	{
		LOGE("error [%s] mipmap is Compressed", __FUNCTION__);
		error = 1;
		goto my_end;
	}

	if (xoffset + width > mip->m_width || yoffset + height > mip->m_height)
	{
		LOGE("error [%s] xoffset=%d, width=%d, mip->m_width=%d, yoffset=%d, height=%d, mip->m_height=%d",
			__FUNCTION__, xoffset, width, mip->m_width, yoffset, height, mip->m_height);
		error = 1;
		goto my_end;
	}

	pixelsLen = width * height * 4;
	pixels = new char[pixelsLen];
	KPHelper::readPixels(x, y, width, height, pixels);
	
	if (mip->m_format != GL_RGBA)
	{
		char* newPixels = KPHelper::convertTexFormatFromRgba(width, height, mip->m_format, pixels, pixelsLen);
		SAFE_DEL_ARRAY(pixels);
		pixels = newPixels;
	}

	tex->on_glTexSubImage2D(level, xoffset, yoffset, width, height, mip->m_format, GL_UNSIGNED_BYTE, pixels);

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glCopyTexSubImage2D);
		m_msg.m_length = 1/*error*/ + 4/*id*/ + 4/*target*/ + 4/*level*/ + 4/*xoffset*/ + 4/*yoffset*/ +
						4/*x*/ + 4/*y*/ + 4/*width*/ + 4/*height*/;
		
		if (!error && pixelsLen > 0)
		{
			m_msg.m_length += pixelsLen;
		}

		m_msg.m_pData = new char[m_msg.m_length];
		
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);						buffer += 1;
		memcpy(buffer, &id, 4);							buffer += 4;
		memcpy(buffer, &target, 4);						buffer += 4;
		memcpy(buffer, &level, 4);						buffer += 4;
		memcpy(buffer, &xoffset, 4);					buffer += 4;
		memcpy(buffer, &yoffset, 4);					buffer += 4;
		memcpy(buffer, &x, 4);							buffer += 4;
		memcpy(buffer, &y, 4);							buffer += 4;
		memcpy(buffer, &width, 4);						buffer += 4;
		memcpy(buffer, &height, 4);						buffer += 4;

		if (!error && pixelsLen > 0)
		{
			memcpy(buffer, pixels, pixelsLen);
			if (mip->m_format == GL_RGBA || mip->m_format == GL_RGB)
			{
				KPHelper::swapRB(buffer, width * height, mip->m_format == GL_RGBA);
			}
			buffer += pixelsLen;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}

	SAFE_DEL_ARRAY(pixels);

	KPHelper::UnlockMutex0();
}

void KPServer::on_glCreateProgram(GLuint result)
{
	KPHelper::LockMutex0();

	LOGD("[%s] program id = %d", __FUNCTION__, result);

	u8 error = 0;

	if (result > 0)
	{
		KPProgram* prog = getProgram(result);
		if (prog)
		{
			LOGW("Warning: [%s] id=%d has aleady exsited, now overwrite it", __FUNCTION__, result);
		}
		else
		{
			prog = getFreeProgram();
		}
		if (prog)
		{
			prog->clearData();
			prog->setId(result);
		}
		else
		{
			error = 1;
			LOGE("error [%s] Not found a free program", __FUNCTION__);
		}
	}
	else
	{
		LOGE("error [%s] result = %d", __FUNCTION__, result);
		error = 1;
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glCreateProgram);
	
		m_msg.m_length = 1 + 4;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);	buffer += 1;
		memcpy(buffer, &result, 4);	buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glCreateShader(GLenum type, GLuint result)
{
	KPHelper::LockMutex0();

	u8 error = 0;
	if (type != GL_VERTEX_SHADER && type != GL_FRAGMENT_SHADER)
	{
		LOGE("error [%s] type = 0x%x", __FUNCTION__, type);
		error = 1;
		goto my_end;
	}

	if (result > 0)
	{
		KPShader* shader = getShader(result);
		if (shader)
		{
			LOGW("Warning: [%s] id=%d has already existed, now overwrite it", __FUNCTION__, result);
		}
		else
		{
			shader = getFreeShader();
		}
		
		if (shader)
		{
			shader->clearData();
			shader->setId(result);
			shader->setType(type);
		}
		else
		{
			error = 1;
			LOGE("error [%s] Not found a free shader", __FUNCTION__);
		}
	}
	else
	{
		error = 1;
		LOGE("error [%s] result = %d", result);
	}

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glCreateShader);
	
		m_msg.m_length = 1 + 4 * 2;
		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);					buffer += 1;
		memcpy(buffer, &type, 4);					buffer += 4;
		memcpy(buffer, &result, 4);					buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glCullFace(GLenum mode)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glCullFace);
	
		m_msg.m_length = 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &mode, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glDeleteBuffers(GLsizei n, const GLuint* buffers)
{
	KPHelper::LockMutex0();
	for (int i = 0; i < n; i++)
	{
		KPVbo* vbo = getVbo(buffers[i]);
		if (vbo)
		{
			vbo->clearData();
			vbo->setId(0);
		}
		else
		{
			LOGE("error [%s] Not found the vbo id: %d", __FUNCTION__, buffers[i]);
		}
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glDeleteBuffers);
	
		m_msg.m_length = 4; // n

		if (n > 0)
		{
			m_msg.m_length += n * 4; // buffers
		}

		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &n, 4); buffer += 4;

		if (n > 0)
		{
			memcpy(buffer, buffers, n * 4);
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glDeleteFramebuffers);
	
		m_msg.m_length = 4; // n
		const int len = n * 4;
		if (n > 0)
		{
			m_msg.m_length += len;
		}
		
		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &n, 4); buffer += 4;
		if (n > 0)
		{
			memcpy(buffer, framebuffers, len); buffer += len;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glDeleteProgram(GLuint program)
{
	KPHelper::LockMutex0();

	LOGD("[%s] program id = %d", __FUNCTION__, program);
	
	u8 error = 0;
	KPProgram* prog = getProgram(program);

	if (prog)
	{
		prog->clearData();
		prog->setId(0);
	}
	else
	{
		error = 1;
		LOGE("error [%s] Not found the program id: %d", __FUNCTION__, program);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glDeleteProgram);
	
		m_msg.m_length = 1 + 4;
		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);				buffer += 1;
		memcpy(buffer, &program, 4);			buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glDeleteRenderbuffers);
	
		m_msg.m_length = 4; // n
		const int len = n * 4;
		if (n > 0)
		{
			m_msg.m_length += len;
		}
		
		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &n, 4); buffer += 4;
		if (n > 0)
		{
			memcpy(buffer, renderbuffers, len); buffer += len;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glDeleteShader(GLuint shader)
{
	KPHelper::LockMutex0();

	u8 error = 0;
	
	KPShader* pshader = getShader(shader);

	if (pshader)
	{
		pshader->clearData();
		pshader->setId(0);
	}
	else
	{
		error = 1;
		LOGE("error [%s] Not found the shader id: %d", __FUNCTION__, shader);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glDeleteShader);
	
		m_msg.m_length = 1 + 4;
		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);				buffer += 1;
		memcpy(buffer, &shader, 4);				buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glDeleteTextures(GLsizei n, const GLuint* textures)
{
	KPHelper::LockMutex0();
	
	for (int i = 0; i < n; i++)
	{
		KPTexture* tex = getTexture(textures[i]);
		if (tex)
		{
			tex->clearData();
			tex->setId(0);
		}
		else
		{
			LOGE("error [%s] Not found the texture id: %d", __FUNCTION__, textures[i]);
		}
	}
	
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glDeleteTextures);

		m_msg.m_length = 4; // n

		if (n > 0)
		{
			m_msg.m_length += 4 * n; // textures
		}
	
		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &n, 4); buffer += 4;

		if (n > 0)
		{
			memcpy(buffer, textures, n * 4);
			buffer += n * 4;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glDepthFunc(GLenum func)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glDepthFunc);
	
		m_msg.m_length = 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &func, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glDepthMask(GLboolean flag)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glDepthMask);
	
		m_msg.m_length = 1;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &flag, 1);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glDepthRangef(GLclampf zNear, GLclampf zFar)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glDepthRangef);

		m_msg.m_length = 4 * 2;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &zNear, 4);
		memcpy(m_msg.m_pData + 4, &zFar, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glDetachShader(GLuint program, GLuint shader)
{
	KPHelper::LockMutex0();
	LOGD("[%s] program = %d, shader = %d", __FUNCTION__, program, shader);

	u8 error = 0;

	KPProgram* prog = getProgram(program);
	if (prog)
	{
		KPShader* pshader = getShader(shader);
		if (pshader)
		{
			if (!prog->detach(shader))
			{
				error = 1;
				LOGE("error [%s] can not detach shader=%d, program=%d", __FUNCTION__, shader, program);
			}
		}
		else
		{
			error = 1;
			LOGE("error [%s] not found the shader id = %d", __FUNCTION__, shader);
		}
	}
	else
	{
		error = 1;
		LOGE("error [%s] not found the program id = %d", __FUNCTION__, program);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glDetachShader);

		m_msg.m_length = 1 + 4 * 2;
		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);			buffer += 1;

		memcpy(buffer, &program, 4);		buffer += 4;
		memcpy(buffer, &shader, 4);			buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glDisable(GLenum cap)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glDisable);

		m_msg.m_length = 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &cap, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glDisableVertexAttribArray(GLuint index)
{
	KPHelper::LockMutex0();

	const int MAX_VERTEX_ATTRIBS = KPHelper::get_MAX_VERTEX_ATTRIBS();
	u8 error = 0;

	if (index >= MAX_VERTEX_ATTRIBS)
	{
		LOGE("error [%s] index = %d, MAX_VERTEX_ATTRIBS = %d", __FUNCTION__, index, MAX_VERTEX_ATTRIBS);
		error = 1;
		goto my_end;
	}

	m_vertexAttributes[index].setEnableVertexAttribArray(false);

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glDisableVertexAttribArray);

		m_msg.m_length = 1 + 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &error, 1);
		memcpy(m_msg.m_pData + 1, &index, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	KPHelper::LockMutex0();

	char* pixels = NULL;
	int width, height;

	if (m_state == CAPTURING)
	{
		KPProgram* prog = getProgram(KPHelper::getCurrentUsingProgram());
		int dataLength = calculateVertexAttribDataLength(prog, count);

		//===========================================================================================
		int x, y;
		KPHelper::getViewport(x, y, width, height);
		pixels = new char[width * height * 4];
		KPHelper::readPixels(x, y, width, height, pixels);

		int widthScaled, heightScaled, imageLen;
		char* screenShot = KPHelper::makeScreenshot_RGB(pixels, width, height, true, true, widthScaled, heightScaled, imageLen);

		//
		makeGLCommandMsg(KMT_glDrawArrays);
	
		m_msg.m_length = 4/*fbo*/ +
			4 * 3 +
			4 * 2 + // zNear and zFar
			4 * 2 + 4 * 2 +
			4 + imageLen +
			/**/dataLength;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		u32 fbo = KPHelper::getCurrentBindingFbo();
		memcpy(buffer, &fbo, 4);			buffer += 4;

		memcpy(buffer, &mode, 4);			buffer += 4;
		memcpy(buffer, &first, 4);			buffer += 4;
		memcpy(buffer, &count, 4);			buffer += 4;

		float n, f;
		KPHelper::getCurrentDepthRange(n, f);
		memcpy(buffer, &n, 4); buffer += 4;
		memcpy(buffer, &f, 4); buffer += 4;

		memcpy(buffer, &width, 4);		buffer += 4;
		memcpy(buffer, &height, 4);		buffer += 4;

		memcpy(buffer, &widthScaled, 4);		buffer += 4;
		memcpy(buffer, &heightScaled, 4);		buffer += 4;

		memcpy(buffer, &imageLen, 4);			buffer += 4;
		if (screenShot && imageLen > 0)
		{
			memcpy(buffer, screenShot, imageLen);	buffer += imageLen;
		}

		SAFE_DEL_ARRAY(screenShot);

		//===========================================================================================
		makeVertexAttribData(buffer, prog,
			false,
			0, 0, NULL,
			first, count);
		//===========================================================================================
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR )
		{
			m_state = IDLE;
		}
		m_msg.clearData();
	}

	//copyCurrentFboToTexture(pixels, width, height);

	SAFE_DEL_ARRAY(pixels);

	KPHelper::UnlockMutex0();
}

void KPServer::on_glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
	KPHelper::LockMutex0();

	char* pixels = NULL;
	int width, height;

	if (m_state == CAPTURING)
	{
		const GLvoid* backup_indices = indices;

		u32 currentBindingVbo_ElementArrayBuffer = KPHelper::getCurrentBindingVbo_ElementArrayBuffer();
		if (currentBindingVbo_ElementArrayBuffer > 0)
		{
			int offset = (int)indices;
			KPVbo* vbo = getVbo(currentBindingVbo_ElementArrayBuffer);
			indices = (const GLvoid*)( vbo->getData() + offset );
		}

		u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
		KPProgram* prog = getProgram(currentUsingProgram);
		int dataLength = calculateVertexAttribDataLength(prog, count);

		//===========================================================================================
		int x, y;
		KPHelper::getViewport(x, y, width, height);
		pixels = new char[width * height * 4];
		KPHelper::readPixels(x, y, width, height, pixels);

		int widthScaled, heightScaled, imageLen;
		char* screenShot = KPHelper::makeScreenshot_RGB(pixels, width, height, true, true, widthScaled, heightScaled, imageLen);

		//
		makeGLCommandMsg(KMT_glDrawElements);
	
		m_msg.m_length = 4/*fbo*/ +
			4 * 4 +
			4 * 2/*zNear and zFar*/ +
			4 * 2 + 4 * 2 +
			4 + imageLen +
			/**/dataLength;
			
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		u32 fbo = KPHelper::getCurrentBindingFbo();
		memcpy(buffer, &fbo, 4);		buffer += 4;
		
		memcpy(buffer, &mode, 4);		buffer += 4;
		memcpy(buffer, &count, 4);		buffer += 4;
		memcpy(buffer, &type, 4);		buffer += 4;
		memcpy(buffer, &backup_indices, 4);	buffer += 4;

		float n, f;
		KPHelper::getCurrentDepthRange(n, f);
		memcpy(buffer, &n, 4); buffer += 4;
		memcpy(buffer, &f, 4); buffer += 4;

		memcpy(buffer, &width, 4);		buffer += 4;
		memcpy(buffer, &height, 4);		buffer += 4;

		memcpy(buffer, &widthScaled, 4);		buffer += 4;
		memcpy(buffer, &heightScaled, 4);		buffer += 4;

		memcpy(buffer, &imageLen, 4);			buffer += 4;
		if (screenShot && imageLen > 0)
		{
			memcpy(buffer, screenShot, imageLen);	buffer += imageLen;
		}

		SAFE_DEL_ARRAY(screenShot);

		//===========================================================================================
		makeVertexAttribData(buffer, prog,
			true,
			count, type, indices,
			0, 0);
		//===========================================================================================
		

		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR )
		{
			m_state = IDLE;
		}
		m_msg.clearData();
	}

	//copyCurrentFboToTexture(pixels, width, height);

	SAFE_DEL_ARRAY(pixels);

	KPHelper::UnlockMutex0();
}

void KPServer::on_glEnable(GLenum cap)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glEnable);

		m_msg.m_length = 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &cap, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glEnableVertexAttribArray(GLuint index)
{
	KPHelper::LockMutex0();

	const int MAX_VERTEX_ATTRIBS = KPHelper::get_MAX_VERTEX_ATTRIBS();
	u8 error = 0;

	if (index >= MAX_VERTEX_ATTRIBS)
	{
		LOGE("error [%s] index = %d, MAX_VERTEX_ATTRIBS = %d", __FUNCTION__, index, MAX_VERTEX_ATTRIBS);
		error = 1;
		goto my_end;
	}

	m_vertexAttributes[index].setEnableVertexAttribArray(true);

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glEnableVertexAttribArray);

		m_msg.m_length = 1 + 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &error, 1);
		memcpy(m_msg.m_pData + 1, &index, 4);

		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glFinish(void)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glFinish);
		m_msg.m_length = 0;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glFlush(void)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glFlush);
		m_msg.m_length = 0;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glFramebufferRenderbuffer);

		m_msg.m_length = 4 * 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &target, 4);
		memcpy(m_msg.m_pData + 4, &attachment, 4);
		memcpy(m_msg.m_pData + 8, &renderbuffertarget, 4);
		memcpy(m_msg.m_pData + 12, &renderbuffer, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glFramebufferTexture2D);

		m_msg.m_length = 4 * 5;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &target, 4);
		memcpy(m_msg.m_pData + 4, &attachment, 4);
		memcpy(m_msg.m_pData + 8, &textarget, 4);
		memcpy(m_msg.m_pData + 12, &texture, 4);
		memcpy(m_msg.m_pData + 16, &level, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glFrontFace(GLenum mode)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glFrontFace);

		m_msg.m_length = 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &mode, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGenBuffers(GLsizei n, GLuint* buffers)
{
	KPHelper::LockMutex0();
	
	for (int k = 0; k < n; k++)
	{
		u32 newId = buffers[k];
		KPVbo* vbo = getVbo(newId);
		if (vbo)
		{
			LOGW("Warning: [%s] id=%d has already existed, now overwrite it", __FUNCTION__, newId);
			vbo->clearData();
		}
		else
		{
			vbo = getFreeVbo();
			if (vbo)
			{
				vbo->setId(newId);
			}
			else
			{
				LOGE("error [%s] Not found a free vbo", __FUNCTION__);
			}
		}
	}
	
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGenBuffers);

		m_msg.m_length = 4; // n
		if (n > 0)
		{
			m_msg.m_length += n * 4; // buffers
		}

		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &n, 4);			buffer += 4;
		if (n > 0)
		{
			memcpy(buffer, buffers, n * 4);
			buffer += n * 4;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGenerateMipmap(GLenum target)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGenerateMipmap);

		m_msg.m_length = 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &target, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGenFramebuffers(GLsizei n, GLuint* framebuffers)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGenFramebuffers);

		m_msg.m_length = 4; // n
		const int len = n * 4;
		if (n > 0)
		{
			m_msg.m_length += len;
		}

		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &n, 4); buffer += 4;

		if (n > 0)
		{
			memcpy(buffer, framebuffers, len); buffer += len;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGenRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGenRenderbuffers);

		m_msg.m_length = 4; // n
		const int len = n * 4;
		if (n > 0)
		{
			m_msg.m_length += len;
		}

		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &n, 4); buffer += 4;

		if (n > 0)
		{
			memcpy(buffer, renderbuffers, len); buffer += len;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGenTextures(GLsizei n, GLuint* textures)
{
	KPHelper::LockMutex0();

	for (int k = 0; k < n; k++)
	{
		u32 newId = textures[k];
		KPTexture* tex = getTexture(newId);
		if (tex)
		{
			LOGW("Warning: [%s] id=%d has already existed, now overwrite it", __FUNCTION__, newId);
			tex->clearData();
		}
		else
		{
			tex = getFreeTexture();
			if (!tex)
				LOGE("error [%s] Not found a free texture", __FUNCTION__);
			else
				tex->setId(newId);
		}
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGenTextures);

		m_msg.m_length = 4; // n
		if (n > 0)
		{
			m_msg.m_length += n * 4; // textures
		}

		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &n, 4); buffer += 4;

		if (n > 0)
		{
			memcpy(buffer, textures, 4 * n);
			buffer += 4 * n;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufsize,
									GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetActiveAttrib);

		m_msg.m_length = 4/*program*/ + 4/*index*/ + 4/*bufsize*/ + 4/*length*/ +
						4/*size*/ + 4/*type*/ + 4/*name*/;
		m_msg.m_pData = new char[m_msg.m_length];
		
		char* buffer = m_msg.m_pData;
		
		memcpy(buffer, &program, 4);		buffer += 4;
		memcpy(buffer, &index, 4);			buffer += 4;
		memcpy(buffer, &bufsize, 4);		buffer += 4;
		memcpy(buffer, &length, 4);			buffer += 4;
		memcpy(buffer, &size, 4);			buffer += 4;
		memcpy(buffer, &type, 4);			buffer += 4;
		memcpy(buffer, &name, 4);			buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetActiveUniform(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size,
										GLenum* type, GLchar* name)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetActiveUniform);

		m_msg.m_length = 4/*program*/ + 4/*index*/ + 4/*bufsize*/ + 4/*length*/ +
						4/*size*/ + 4/*type*/ + 4/*name*/;
		m_msg.m_pData = new char[m_msg.m_length];
		
		char* buffer = m_msg.m_pData;
		
		memcpy(buffer, &program, 4);		buffer += 4;
		memcpy(buffer, &index, 4);			buffer += 4;
		memcpy(buffer, &bufsize, 4);		buffer += 4;
		memcpy(buffer, &length, 4);			buffer += 4;
		memcpy(buffer, &size, 4);			buffer += 4;
		memcpy(buffer, &type, 4);			buffer += 4;
		memcpy(buffer, &name, 4);			buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetAttachedShaders(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetAttachedShaders);
		m_msg.m_length = 4 * 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &program, 4);
		memcpy(m_msg.m_pData + 4, &maxcount, 4);
		memcpy(m_msg.m_pData + 8, &count, 4);
		memcpy(m_msg.m_pData + 12, &shaders, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetAttribLocation(GLuint program, const GLchar* name, GLint result)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetAttribLocation);
		const int strLen = name ? strlen(name) : 0;
		m_msg.m_length = 4 + strLen + 4;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &program, 4);			buffer += 4;
		if (strLen > 0)
		{
			memcpy(buffer, name, strLen);		buffer += strLen;
		}
		memcpy(buffer, &result, 4);				buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetBooleanv(GLenum pname, GLboolean* params)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetBooleanv);
		m_msg.m_length = 4 + 1;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &pname, 4);
		memcpy(m_msg.m_pData + 4, params, 1);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetBufferParameteriv);
		m_msg.m_length = 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &target, 4);
		memcpy(m_msg.m_pData + 4, &pname, 4);
		memcpy(m_msg.m_pData + 8, params, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetError(GLenum result)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetError);
		m_msg.m_length = 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &result, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetFloatv(GLenum pname, GLfloat* params)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetFloatv);
		m_msg.m_length = 4 * 2;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &pname, 4);
		memcpy(m_msg.m_pData + 4, params, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetFramebufferAttachmentParameteriv);
		m_msg.m_length = 4 * 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &target, 4);
		memcpy(m_msg.m_pData + 4, &attachment, 4);
		memcpy(m_msg.m_pData + 8, &pname, 4);
		memcpy(m_msg.m_pData + 12, params, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetIntegerv(GLenum pname, GLint* params)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetIntegerv);
		m_msg.m_length = 4 * 2;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &pname, 4);
		memcpy(m_msg.m_pData + 4, params, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetProgramiv(GLuint program, GLenum pname, GLint* params)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetProgramiv);
		m_msg.m_length = 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &program, 4);
		memcpy(m_msg.m_pData + 4, &pname, 4);
		memcpy(m_msg.m_pData + 8, params, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetProgramInfoLog);

		m_msg.m_length = 4 * 4;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;
		
		memcpy(buffer, &program, 4);	buffer += 4;
		memcpy(buffer, &bufsize, 4);	buffer += 4;
		memcpy(buffer, &length, 4);		buffer += 4;
		memcpy(buffer, &infolog, 4);	buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetRenderbufferParameteriv);
		m_msg.m_length = 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &target, 4);
		memcpy(m_msg.m_pData + 4, &pname, 4);
		memcpy(m_msg.m_pData + 8, params, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetShaderiv);
		m_msg.m_length = 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &shader, 4);
		memcpy(m_msg.m_pData + 4, &pname, 4);
		memcpy(m_msg.m_pData + 8, params, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetShaderInfoLog);
		
		m_msg.m_length = 4 * 4;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;
		
		memcpy(buffer, &shader, 4);		buffer += 4;
		memcpy(buffer, &bufsize, 4);	buffer += 4;
		memcpy(buffer, &length, 4);		buffer += 4;
		memcpy(buffer, &infolog, 4);	buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetShaderPrecisionFormat);
		m_msg.m_length = 4 + 4 + 4 * 2 + 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &shadertype, 4);
		memcpy(m_msg.m_pData + 4, &precisiontype, 4);
		memcpy(m_msg.m_pData + 8, range, 4 * 2);
		memcpy(m_msg.m_pData + 16, precision, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetShaderSource(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetShaderSource);
		
		m_msg.m_length = 4 * 4;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &shader, 4);		buffer += 4;
		memcpy(buffer, &bufsize, 4);	buffer += 4;
		memcpy(buffer, &length, 4);		buffer += 4;
		memcpy(buffer, &source, 4);		buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetString(GLenum name, const GLubyte* result)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetString);
		const char* sz = (const char*)result;
		const int strLen = sz ? strlen(sz) : 0;
		m_msg.m_length = 4 + strLen;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &name, 4);
		if (strLen > 0)
		{
			memcpy(m_msg.m_pData + 4, sz, strLen);
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetTexParameterfv);
		m_msg.m_length = 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &target, 4);
		memcpy(m_msg.m_pData + 4, &pname, 4);
		memcpy(m_msg.m_pData + 8, params, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetTexParameteriv(GLenum target, GLenum pname, GLint* params)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetTexParameteriv);
		m_msg.m_length = 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &target, 4);
		memcpy(m_msg.m_pData + 4, &pname, 4);
		memcpy(m_msg.m_pData + 8, params, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetUniformfv(GLuint program, GLint location, GLfloat* params)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetUniformfv);
		m_msg.m_length = 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &program, 4);
		memcpy(m_msg.m_pData + 4, &location, 4);
		memcpy(m_msg.m_pData + 8, &params, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetUniformiv(GLuint program, GLint location, GLint* params)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetUniformiv);
		m_msg.m_length = 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &program, 4);
		memcpy(m_msg.m_pData + 4, &location, 4);
		memcpy(m_msg.m_pData + 8, &params, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetUniformLocation(GLuint program, const GLchar* name, GLint result)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetUniformLocation);
		const int strLen = name ? strlen(name) : 0;
		m_msg.m_length = 4 + 4 + strLen;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &program, 4);			buffer += 4;
		memcpy(buffer, &result, 4);				buffer += 4;
		if (strLen > 0)
		{
			memcpy(buffer, name, strLen);		buffer += strLen;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetVertexAttribfv);
		m_msg.m_length = 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &index, 4);
		memcpy(m_msg.m_pData + 4, &pname, 4);
		memcpy(m_msg.m_pData + 8, &params, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetVertexAttribiv(GLuint index, GLenum pname, GLint* params)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetVertexAttribiv);
		m_msg.m_length = 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &index, 4);
		memcpy(m_msg.m_pData + 4, &pname, 4);
		memcpy(m_msg.m_pData + 8, &params, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glGetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid** pointer)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glGetVertexAttribPointerv);
		m_msg.m_length = 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &index, 4);
		memcpy(m_msg.m_pData + 4, &pname, 4);
		memcpy(m_msg.m_pData + 8, *pointer, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glHint(GLenum target, GLenum mode)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glHint);
		m_msg.m_length = 4 * 2;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &target, 4);
		memcpy(m_msg.m_pData + 4, &mode, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glIsBuffer(GLuint buffer, GLboolean result)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glIsBuffer);
		m_msg.m_length = 4 + 1;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &buffer, 4);
		memcpy(m_msg.m_pData + 4, &result, 1);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glIsEnabled(GLenum cap, GLboolean result)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glIsEnabled);
		m_msg.m_length = 4 + 1;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &cap, 4);
		memcpy(m_msg.m_pData + 4, &result, 1);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glIsFramebuffer(GLuint framebuffer, GLboolean result)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glIsFramebuffer);
		m_msg.m_length = 4 + 1;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &framebuffer, 4);
		memcpy(m_msg.m_pData + 4, &result, 1);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glIsProgram(GLuint program, GLboolean result)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glIsProgram);
		m_msg.m_length = 4 + 1;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &program, 4);
		memcpy(m_msg.m_pData + 4, &result, 1);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glIsRenderbuffer(GLuint renderbuffer, GLboolean result)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glIsRenderbuffer);
		m_msg.m_length = 4 + 1;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &renderbuffer, 4);
		memcpy(m_msg.m_pData + 4, &result, 1);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glIsShader(GLuint shader, GLboolean result)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glIsShader);
		m_msg.m_length = 4 + 1;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &shader, 4);
		memcpy(m_msg.m_pData + 4, &result, 1);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glIsTexture(GLuint texture, GLboolean result)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glIsTexture);
		m_msg.m_length = 4 + 1;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &texture, 4);
		memcpy(m_msg.m_pData + 4, &result, 1);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glLineWidth(GLfloat width)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glLineWidth);
		m_msg.m_length = 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &width, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glLinkProgram(GLuint program)
{
	KPHelper::LockMutex0();

	u8 error = 0;

	KPProgram* prog = getProgram(program);
	if (prog)
	{
		prog->link();
	}
	else
	{
		error = 1;
		LOGE("error [%s] Not found the program id = %d", __FUNCTION__, program);
	}
	
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glLinkProgram);

		m_msg.m_length = 1 + 4;

		if (error)
		{
			m_msg.m_pData = new char[m_msg.m_length];
			char* buffer = m_msg.m_pData;

			memcpy(buffer, &error, 1);		buffer += 1;
			memcpy(buffer, &program, 4);	buffer += 4;
		}
		else
		{
			KPMessage* msgProg = prog->toMessage();
			const int len = msgProg->m_length;

			m_msg.m_length += len;
			m_msg.m_pData = new char[m_msg.m_length];

			char* buffer = m_msg.m_pData;

			memcpy(buffer, &error, 1);		buffer += 1;
			memcpy(buffer, &program, 4);	buffer += 4;

			if (len > 0)
			{
				memcpy(buffer, msgProg->m_pData, len);
				buffer += len;
			}

			delete msgProg;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}

	KPHelper::UnlockMutex0();
}

void KPServer::on_glPixelStorei(GLenum pname, GLint param)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glPixelStorei);
		m_msg.m_length = 4 * 2;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &pname, 4);
		memcpy(m_msg.m_pData + 4, &param, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glPolygonOffset(GLfloat factor, GLfloat units)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glPolygonOffset);
		m_msg.m_length = 4 * 2;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &factor, 4);
		memcpy(m_msg.m_pData + 4, &units, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glReadPixels);
		m_msg.m_length = 4 * 7;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &x, 4);
		buffer += 4;

		memcpy(buffer, &y, 4);
		buffer += 4;

		memcpy(buffer, &width, 4);
		buffer += 4;

		memcpy(buffer, &height, 4);
		buffer += 4;

		memcpy(buffer, &format, 4);
		buffer += 4;

		memcpy(buffer, &type, 4);
		buffer += 4;

		memcpy(buffer, &pixels, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glReleaseShaderCompiler(void)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glReleaseShaderCompiler);
		m_msg.m_length = 0;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glRenderbufferStorage);

		m_msg.m_length = 4 * 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &target, 4);
		memcpy(m_msg.m_pData + 4, &internalformat, 4);
		memcpy(m_msg.m_pData + 8, &width, 4);
		memcpy(m_msg.m_pData + 12, &height, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glSampleCoverage(GLclampf value, GLboolean invert)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glSampleCoverage);

		m_msg.m_length = 4 + 1;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &value, 4);
		memcpy(m_msg.m_pData + 4, &invert, 1);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glScissor);

		m_msg.m_length = 4 * 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &x, 4);
		memcpy(m_msg.m_pData + 4, &y, 4);
		memcpy(m_msg.m_pData + 8, &width, 4);
		memcpy(m_msg.m_pData + 12, &height, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glShaderBinary(GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glShaderBinary);

		m_msg.m_length = 4 * 5;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &n, 4);				buffer += 4;
		memcpy(buffer, &shaders, 4);		buffer += 4;
		memcpy(buffer, &binaryformat, 4);	buffer += 4;
		memcpy(buffer, &binary, 4);			buffer += 4;
		memcpy(buffer, &length, 4);			buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length)
{
	KPHelper::LockMutex0();

	u8 error = 0;
	KPShader* pshader = NULL;

	if (count < 0)
	{
		LOGE("error [%s] count = %d", __FUNCTION__, count);
		error = 1;
		goto my_end;
	}

	pshader = getShader(shader);

	if (pshader)
	{
		const int sourceLen = KPHelper::calculateShaderSourceSize(count, string, length);
		char buffer[sourceLen + 1];
		KPHelper::copyShaderSource(buffer, count, string, length);
		buffer[sourceLen] = 0;
		pshader->setSource(buffer);
	}
	else
	{
		LOGE("error [%s] not found the shader id = %d", __FUNCTION__, shader);
		error = 1;
		goto my_end;
	}

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glShaderSource);

		const int sourceLen = KPHelper::calculateShaderSourceSize(count, string, length);

		m_msg.m_length = 1 + 4 + sourceLen;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;
		memcpy(buffer, &error, 1);			buffer += 1;
		memcpy(buffer, &shader, 4);			buffer += 4;

		if (sourceLen > 0)
		{
			KPHelper::copyShaderSource(buffer, count, string, length);
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glStencilFunc);

		m_msg.m_length = 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &func, 4);
		memcpy(m_msg.m_pData + 4, &ref, 4);
		memcpy(m_msg.m_pData + 8, &mask, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glStencilFuncSeparate);

		m_msg.m_length = 4 * 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &face, 4);
		memcpy(m_msg.m_pData + 4, &func, 4);
		memcpy(m_msg.m_pData + 8, &ref, 4);
		memcpy(m_msg.m_pData + 12, &mask, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glStencilMask(GLuint mask)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glStencilMask);

		m_msg.m_length = 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &mask, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glStencilMaskSeparate(GLenum face, GLuint mask)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glStencilMaskSeparate);

		m_msg.m_length = 4 * 2;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &face, 4);
		memcpy(m_msg.m_pData + 4, &mask, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glStencilOp);

		m_msg.m_length = 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &fail, 4);
		memcpy(m_msg.m_pData + 4, &zfail, 4);
		memcpy(m_msg.m_pData + 8, &zpass, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glStencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glStencilOpSeparate);

		m_msg.m_length = 4 * 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &face, 4);
		memcpy(m_msg.m_pData + 4, &fail, 4);
		memcpy(m_msg.m_pData + 8, &zfail, 4);
		memcpy(m_msg.m_pData + 12, &zpass, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
								GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
	KPHelper::LockMutex0();

	const int maxTexSize = KPHelper::get_MAX_TEXTURE_SIZE();
	u32 id = 0;
	KPTexture* tex;
	u8 error = 0;

	if (target != GL_TEXTURE_2D &&
        target != GL_TEXTURE_CUBE_MAP_POSITIVE_X &&
        target != GL_TEXTURE_CUBE_MAP_NEGATIVE_X &&
        target != GL_TEXTURE_CUBE_MAP_POSITIVE_Y &&
        target != GL_TEXTURE_CUBE_MAP_NEGATIVE_Y &&
        target != GL_TEXTURE_CUBE_MAP_POSITIVE_Z &&
        target != GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
	{
		LOGE("error [%s] Unknown target = 0x%x", __FUNCTION__, target);
		error = 1;
		goto my_end;
	}

	id = target == GL_TEXTURE_2D ? KPHelper::getCurrentBindingTexture2D() : KPHelper::getCurrentBindingTextureCubeMap();

	LOGD("[%s] id=%d, target=0x%x, level=%d, internalformat=0x%x, width=%d, height=%d, border=%d, type=0x%x, pixels=%p",
		__FUNCTION__, id, target, level, internalformat, width, height, border, type, pixels);

	if (level < 0 || level >= MAX_MIPMAP_LEVEL_NUMBER || level > KPHelper::getMaxMipLevel())
	{
		LOGE("error [%s] Invalid level = %d", __FUNCTION__, level);
		error = 1;
		goto my_end;
	}

	if (internalformat != format)
	{
		LOGE("error [%s] internalformat = 0x%x, format = 0x%x", __FUNCTION__, internalformat, format);
		error = 1;
		goto my_end;
	}

	if (internalformat == GL_DEPTH_COMPONENT)
	{
		LOGE("error [%s] KataProfiler does not support GL_DEPTH_COMPONENT", __FUNCTION__);
		error = 1;
		goto my_end;
	}
	
	if (internalformat != GL_ALPHA && internalformat != GL_LUMINANCE && internalformat != GL_LUMINANCE_ALPHA &&
		internalformat != GL_RGB && internalformat != GL_RGBA)
	{
		LOGE("error [%s] Invalid internalformat = 0x%x", __FUNCTION__, internalformat);
		error = 1;
		goto my_end;
	}

	if (type != GL_UNSIGNED_BYTE &&
		type != GL_UNSIGNED_SHORT_5_6_5 &&
		type != GL_UNSIGNED_SHORT_4_4_4_4 &&
		type != GL_UNSIGNED_SHORT_5_5_5_1)
	{
		LOGE("error [%s] Invalid type = 0x%x", __FUNCTION__, type);
		error = 1;
		goto my_end;
	}

	if	(
			(type == GL_UNSIGNED_SHORT_5_6_5 && format != GL_RGB) ||
			( (type == GL_UNSIGNED_SHORT_4_4_4_4 || type == GL_UNSIGNED_SHORT_5_5_5_1) && format != GL_RGBA )
		)
	{
		LOGE("error [%s] type = 0x%x, format = 0x%x", __FUNCTION__, type, format);
		error = 1;
		goto my_end;
	}

	if (width < 0 || height < 0 || width > maxTexSize || height > maxTexSize)
	{
		LOGE("error [%s] width = %d, height = %d", __FUNCTION__, width, height);
		error = 1;
		goto my_end;
	}

	if (border != 0)
	{
		LOGE("error [%s] border = %d", __FUNCTION__, border);
		error = 1;
		goto my_end;
	}
	
	tex = getTexture(id);
	if (!tex)
	{
		LOGE("error [%s] Not found the texture id = %d", __FUNCTION__, id);
		error = 1;
		goto my_end;
	}

	tex->on_glTexImage2D(level, internalformat, width, height, border, format, type, pixels, m_options.m_isRecordTexturePixel);

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glTexImage2D);
		m_msg.m_length = 1/*error*/ + 4/*id*/ + 4/*target*/ + 4/*level*/ + 4/*internalformat*/ + 4/*width*/ + 4/*height*/ +
							4/*border*/ + 4/*format*/ + 4/*type*/ + 4/*pointer address*/;
		
		int imageSize = 0;
		if (!error)
		{
			imageSize = KPMipmapLevel::calculateTexSize(internalformat, width, height, type);
			if (imageSize > 0)
			{
				m_msg.m_length += imageSize;
			}
		}

		m_msg.m_pData = new char[m_msg.m_length];
		
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);						buffer += 1;
		memcpy(buffer, &id, 4);							buffer += 4;
		memcpy(buffer, &target, 4);						buffer += 4;
		memcpy(buffer, &level, 4);						buffer += 4;
		memcpy(buffer, &internalformat, 4);				buffer += 4;
		memcpy(buffer, &width, 4);						buffer += 4;
		memcpy(buffer, &height, 4);						buffer += 4;
		memcpy(buffer, &border, 4);						buffer += 4;
		memcpy(buffer, &format, 4);						buffer += 4;
		memcpy(buffer, &type, 4);						buffer += 4;
		memcpy(buffer, &pixels, 4);						buffer += 4;

		if (!error && imageSize > 0)
		{
			if (pixels)
			{
				KPHelper::copyAlignedPixelsData(buffer, pixels, width, height, internalformat, type);
				if (type == GL_UNSIGNED_BYTE && (format == GL_RGBA || format == GL_RGB))
				{
					KPHelper::swapRB(buffer, width * height, format == GL_RGBA);
				}
			}
			else
			{
				memset(buffer, 0, imageSize);
			}
			buffer += imageSize;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glTexParameterf);

		m_msg.m_length = 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &target, 4);
		memcpy(m_msg.m_pData + 4, &pname, 4);
		memcpy(m_msg.m_pData + 8, &param, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glTexParameterfv);

		m_msg.m_length = 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &target, 4);
		memcpy(m_msg.m_pData + 4, &pname, 4);
		memcpy(m_msg.m_pData + 8, params, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glTexParameteri(GLenum target, GLenum pname, GLint param)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glTexParameteri);

		m_msg.m_length = 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &target, 4);
		memcpy(m_msg.m_pData + 4, &pname, 4);
		memcpy(m_msg.m_pData + 8, &param, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glTexParameteriv(GLenum target, GLenum pname, const GLint* params)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glTexParameteriv);

		m_msg.m_length = 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &target, 4);
		memcpy(m_msg.m_pData + 4, &pname, 4);
		memcpy(m_msg.m_pData + 8, params, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset,
							GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
	KPHelper::LockMutex0();
	
	u32 id = 0;
	KPTexture* tex;
	u8 error = 0;
	KPMipmapLevel* mip;
	
	if (target != GL_TEXTURE_2D &&
        target != GL_TEXTURE_CUBE_MAP_POSITIVE_X &&
        target != GL_TEXTURE_CUBE_MAP_NEGATIVE_X &&
        target != GL_TEXTURE_CUBE_MAP_POSITIVE_Y &&
        target != GL_TEXTURE_CUBE_MAP_NEGATIVE_Y &&
        target != GL_TEXTURE_CUBE_MAP_POSITIVE_Z &&
        target != GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
	{
		LOGE("error [%s] target = 0x%x", __FUNCTION__, target);
		error = 1;
		goto my_end;
	}

	id = target == GL_TEXTURE_2D ? KPHelper::getCurrentBindingTexture2D() : KPHelper::getCurrentBindingTextureCubeMap();

	LOGD("[%s] id=%d, target=0x%x, level=%d, xoffset=%d, yoffset=%d, width=%d, height=%d, format=0x%x, type=0x%x, pixels=%p",
		__FUNCTION__, id, target, level, xoffset, yoffset, width, height, format, type, pixels);

	if (level < 0 || level >= MAX_MIPMAP_LEVEL_NUMBER || level > KPHelper::getMaxMipLevel())
	{
		LOGE("error [%s] level = %d", __FUNCTION__, level);
		error = 1;
		goto my_end;
	}

	if (xoffset < 0 || yoffset < 0)
	{
		LOGE("error [%s] xoffset = %d, yoffset = %d", __FUNCTION__, xoffset, yoffset);
		error = 1;
		goto my_end;
	}

	if (width < 0 || height < 0)
	{
		LOGE("error [%s] width = %d, height = %d", __FUNCTION__, width, height);
		error = 1;
		goto my_end;
	}

	if (format == GL_DEPTH_COMPONENT)
	{
		LOGE("error [%s] KataProfiler does not support GL_DEPTH_COMPONENT", __FUNCTION__);
		error = 1;
		goto my_end;
	}

	if (format != GL_ALPHA && format != GL_LUMINANCE && format != GL_LUMINANCE_ALPHA &&
		format != GL_RGB && format != GL_RGBA)
	{
		LOGE("error [%s] Invalid format = 0x%x", __FUNCTION__, format);
		error = 1;
		goto my_end;
	}

	if (type != GL_UNSIGNED_BYTE &&
		type != GL_UNSIGNED_SHORT_5_6_5 &&
		type != GL_UNSIGNED_SHORT_4_4_4_4 &&
		type != GL_UNSIGNED_SHORT_5_5_5_1)
	{
		LOGE("error [%s] Invalid type = 0x%x", __FUNCTION__, type);
		error = 1;
		goto my_end;
	}

	if	(
			(type == GL_UNSIGNED_SHORT_5_6_5 && format != GL_RGB) ||
			( (type == GL_UNSIGNED_SHORT_4_4_4_4 || type == GL_UNSIGNED_SHORT_5_5_5_1) && format != GL_RGBA )
		)
	{
		LOGE("error [%s] type = 0x%x, format = 0x%x", __FUNCTION__, type, format);
		error = 1;
		goto my_end;
	}

	tex = getTexture(id);
	if (!tex)
	{
		LOGE("error [%s] Not found the texture id = %d", __FUNCTION__, id);
		error = 1;
		goto my_end;
	}

	mip = tex->getMip(level);

	if (!mip->m_hasData)
	{
		LOGE("error [%s] mipmap does not have data", __FUNCTION__);
		error = 1;
		goto my_end;
	}

	if (mip->m_isCompressed)
	{
		LOGE("error [%s] mipmap is Compressed", __FUNCTION__);
		error = 1;
		goto my_end;
	}

	if (mip->m_format != format)
	{
		LOGE("error [%s] mipmap format is not matched: format=0x%x, mip->m_format=0x%x", __FUNCTION__, format, mip->m_format);
		error = 1;
		goto my_end;
	}

	if (xoffset + width > mip->m_width || yoffset + height > mip->m_height)
	{
		LOGE("error [%s] xoffset=%d, width=%d, mip->m_width=%d, yoffset=%d, height=%d, mip->m_height=%d",
			__FUNCTION__, xoffset, width, mip->m_width, yoffset, height, mip->m_height);
		error = 1;
		goto my_end;
	}

	tex->on_glTexSubImage2D(level, xoffset, yoffset, width, height, format, type, pixels);

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glTexSubImage2D);
		m_msg.m_length = 1/*error*/ + 4/*id*/ + 4/*target*/ + 4/*level*/ + 4/*xoffset*/ + 4/*yoffset*/ +
						4/*width*/ + 4/*height*/ + 4/*format*/ + 4/*type*/ + 4/*address*/;
		int dataSize = 0;
		if (!error && pixels)
		{
			dataSize = KPMipmapLevel::calculateTexSize(format, width, height, type);
			if (dataSize > 0)
			{
				m_msg.m_length += dataSize;
			}
		}

		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;
		
		memcpy(buffer, &error, 1);					buffer += 1;
		memcpy(buffer, &id, 4);						buffer += 4;
		memcpy(buffer, &target, 4);					buffer += 4;
		memcpy(buffer, &level, 4);					buffer += 4;
		memcpy(buffer, &xoffset, 4);				buffer += 4;
		memcpy(buffer, &yoffset, 4);				buffer += 4;
		memcpy(buffer, &width, 4);					buffer += 4;
		memcpy(buffer, &height, 4);					buffer += 4;
		memcpy(buffer, &format, 4);					buffer += 4;
		memcpy(buffer, &type, 4);					buffer += 4;
		memcpy(buffer, &pixels, 4);					buffer += 4;

		if (!error && pixels && dataSize > 0)
		{
			KPHelper::copyAlignedPixelsData(buffer, pixels, width, height, format, type);
			if (type == GL_UNSIGNED_BYTE && (format == GL_RGBA || format == GL_RGB))
			{
				KPHelper::swapRB(buffer, width * height, format == GL_RGBA);
			}
			buffer += dataSize;
		}

		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

//==============================================================================
// glUniform***

KPProgram* KPServer::validate_glUniform1234fiv(const char* funcName, int comps, bool isFloat, bool isVector,
			GLint location, GLsizei count/* = 0*/)
{
	if (isVector && count < 0)
	{
		return NULL;
	}

	u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
	KPProgram* prog = getProgram(currentUsingProgram);
	if (!prog)
	{
		LOGE("error [%s] not found current using program id = %d", funcName, currentUsingProgram);
		return NULL;
	}

	if (location == -1) return prog;
	
	const KPUniform* pUniform = prog->getUniformByLocation(location);
	if (!pUniform)
	{
		LOGE("error [%s] invalid uniform location = %d", funcName, location);
		return NULL;
	}

	if (pUniform->isTypeMatrix())
	{
		LOGE("error [%s] the uniform is a matrix", funcName);
		return NULL;
	}

	if (comps != pUniform->getTypeComponents())
	{
		LOGE("error [%s] wrong components", funcName);
		return NULL;
	}

	if (isFloat != pUniform->isTypeFloat())
	{
		LOGE("error [%s] wrong type (float - int)", funcName);
		return NULL;
	}

	if (isVector && count > 1 && !pUniform->isArray())
	{
		LOGE("error [%s] count = %d but the uniform is not an array variable", funcName, count);
		return NULL;
	}

	return prog;
}

KPProgram* KPServer::validate_glUniformMatrix(const char* funcName, int size, GLint location, GLsizei count, GLboolean transpose)
{
	if (count < 0) return NULL;
	if (transpose != GL_FALSE) return NULL;

	u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
	KPProgram* prog = getProgram(currentUsingProgram);
	if (!prog)
	{
		LOGE("error [%s] not found current using program id = %d", funcName, currentUsingProgram);
		return NULL;
	}

	if (location == -1) return prog;
	
	const KPUniform* pUniform = prog->getUniformByLocation(location);
	if (!pUniform)
	{
		LOGE("error [%s] invalid uniform location = %d", funcName, location);
		return NULL;
	}

	if (!pUniform->isTypeMatrix())
	{
		LOGE("error [%s] the uniform is not a matrix", funcName);
		return NULL;
	}

	if (size != pUniform->getTypeComponents())
	{
		LOGE("error [%s] the size of matrix is not matched", funcName);
		return NULL;
	}

	return prog;
}

void KPServer::on_glUniform1f(GLint location, GLfloat x)
{
	KPHelper::LockMutex0();

	KPProgram* prog = validate_glUniform1234fiv("on_glUniform1f", 1, true/*isFloat*/, false/*isVector*/, location);
	u8 error = prog ? 0 : 1;
	if (!error)
	{
		prog->on_glUniform1f(location, x);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUniform1f);
		m_msg.m_length = 1 + 4 * 2 + 4 * 1;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);							buffer += 1;
		u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
		memcpy(buffer, &currentUsingProgram, 4);			buffer += 4;
		memcpy(buffer, &location, 4);						buffer += 4;
		memcpy(buffer, &x, 4);								buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glUniform1fv(GLint location, GLsizei count, const GLfloat* v)
{
	KPHelper::LockMutex0();

	KPProgram* prog = validate_glUniform1234fiv("on_glUniform1fv", 1, true/*isFloat*/, true/*isVector*/, location, count);
	u8 error = prog ? 0 : 1;
	if (!error && location != -1 && count > 0)
	{
		prog->on_glUniform1fv(location, count, v);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUniform1fv);
		m_msg.m_length = 1 + 4 + 4 + 4;
		if (count > 0)
		{
			m_msg.m_length += count * 4;
		}
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);							buffer += 1;
		u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
		memcpy(buffer, &currentUsingProgram, 4);			buffer += 4;
		memcpy(buffer, &location, 4);						buffer += 4;
		memcpy(buffer, &count, 4);							buffer += 4;
		if (count > 0)
		{
			memcpy(buffer, v, count * 4);					buffer += count * 4;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glUniform1i(GLint location, GLint x)
{
	KPHelper::LockMutex0();

	KPProgram* prog = validate_glUniform1234fiv("on_glUniform1i", 1, false/*isFloat*/, false/*isVector*/, location);
	u8 error = prog ? 0 : 1;
	if (!error)
	{
		prog->on_glUniform1i(location, x);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUniform1i);
		m_msg.m_length = 1 + 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);						buffer += 1;
		u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
		memcpy(buffer, &currentUsingProgram, 4);		buffer += 4;
		memcpy(buffer, &location, 4);					buffer += 4;
		memcpy(buffer, &x, 4);							buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glUniform1iv(GLint location, GLsizei count, const GLint* v)
{
	KPHelper::LockMutex0();
	
	KPProgram* prog = validate_glUniform1234fiv("on_glUniform1iv", 1, false/*isFloat*/, true/*isVector*/, location, count);
	u8 error = prog ? 0 : 1;
	if (!error && location != -1 && count > 0)
	{
		prog->on_glUniform1iv(location, count, v);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUniform1iv);
		m_msg.m_length = 1 + 4 + 4 + 4;
		if (count > 0)
		{
			m_msg.m_length += count * 4;
		}
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);							buffer += 1;
		u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
		memcpy(buffer, &currentUsingProgram, 4);			buffer += 4;
		memcpy(buffer, &location, 4);						buffer += 4;
		memcpy(buffer, &count, 4);							buffer += 4;
		if (count > 0)
		{
			memcpy(buffer, v, count * 4);					buffer += count * 4;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glUniform2f(GLint location, GLfloat x, GLfloat y)
{
	KPHelper::LockMutex0();
	
	KPProgram* prog = validate_glUniform1234fiv("on_glUniform2f", 2, true/*isFloat*/, false/*isVector*/, location);
	u8 error = prog ? 0 : 1;
	if (!error)
	{
		prog->on_glUniform2f(location, x, y);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUniform2f);
		m_msg.m_length = 1 + 4 * 2 + 4 * 2;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);							buffer += 1;
		u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
		memcpy(buffer, &currentUsingProgram, 4);			buffer += 4;
		memcpy(buffer, &location, 4);						buffer += 4;
		memcpy(buffer, &x, 4);								buffer += 4;
		memcpy(buffer, &y, 4);								buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glUniform2fv(GLint location, GLsizei count, const GLfloat* v)
{
	KPHelper::LockMutex0();
	
	KPProgram* prog = validate_glUniform1234fiv("on_glUniform2fv", 2, true/*isFloat*/, true/*isVector*/, location, count);
	u8 error = prog ? 0 : 1;
	if (!error && location != -1 && count > 0)
	{
		prog->on_glUniform2fv(location, count, v);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUniform2fv);
		m_msg.m_length = 1 + 4 + 4 + 4;
		const int len = count * 4 * 2;
		if (count > 0)
		{
			m_msg.m_length += len;
		}
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);							buffer += 1;
		u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
		memcpy(buffer, &currentUsingProgram, 4);			buffer += 4;
		memcpy(buffer, &location, 4);						buffer += 4;
		memcpy(buffer, &count, 4);							buffer += 4;
		if (count > 0)
		{
			memcpy(buffer, v, len);							buffer += len;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glUniform2i(GLint location, GLint x, GLint y)
{
	KPHelper::LockMutex0();
	
	KPProgram* prog = validate_glUniform1234fiv("on_glUniform2i", 2, false/*isFloat*/, false/*isVector*/, location);
	u8 error = prog ? 0 : 1;
	if (!error)
	{
		prog->on_glUniform2i(location, x, y);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUniform2i);
		m_msg.m_length = 1 + 4 * 2 + 4 * 2;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);						buffer += 1;
		u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
		memcpy(buffer, &currentUsingProgram, 4);		buffer += 4;
		memcpy(buffer, &location, 4);					buffer += 4;
		memcpy(buffer, &x, 4);							buffer += 4;
		memcpy(buffer, &y, 4);							buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glUniform2iv(GLint location, GLsizei count, const GLint* v)
{
	KPHelper::LockMutex0();
	
	KPProgram* prog = validate_glUniform1234fiv("on_glUniform2iv", 2, false/*isFloat*/, true/*isVector*/, location, count);
	u8 error = prog ? 0 : 1;
	if (!error && location != -1 && count > 0)
	{
		prog->on_glUniform2iv(location, count, v);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUniform2iv);
		m_msg.m_length = 1 + 4 + 4 + 4;
		const int len = count * 4 * 2;
		if (count > 0)
		{
			m_msg.m_length += len;
		}
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);							buffer += 1;
		u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
		memcpy(buffer, &currentUsingProgram, 4);			buffer += 4;
		memcpy(buffer, &location, 4);						buffer += 4;
		memcpy(buffer, &count, 4);							buffer += 4;
		if (count > 0)
		{
			memcpy(buffer, v, len);							buffer += len;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glUniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z)
{
	KPHelper::LockMutex0();
	
	KPProgram* prog = validate_glUniform1234fiv("on_glUniform3f", 3, true/*isFloat*/, false/*isVector*/, location);
	u8 error = prog ? 0 : 1;
	if (!error)
	{
		prog->on_glUniform3f(location, x, y, z);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUniform3f);
		m_msg.m_length = 1 + 4 * 2 + 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);							buffer += 1;
		u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
		memcpy(buffer, &currentUsingProgram, 4);			buffer += 4;
		memcpy(buffer, &location, 4);						buffer += 4;
		memcpy(buffer, &x, 4);								buffer += 4;
		memcpy(buffer, &y, 4);								buffer += 4;
		memcpy(buffer, &z, 4);								buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glUniform3fv(GLint location, GLsizei count, const GLfloat* v)
{
	KPHelper::LockMutex0();
	
	KPProgram* prog = validate_glUniform1234fiv("on_glUniform3fv", 3, true/*isFloat*/, true/*isVector*/, location, count);
	u8 error = prog ? 0 : 1;
	if (!error && location != -1 && count > 0)
	{
		prog->on_glUniform3fv(location, count, v);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUniform3fv);
		m_msg.m_length = 1 + 4 + 4 + 4;
		const int len = count * 4 * 3;
		if (count > 0)
		{
			m_msg.m_length += len;
		}
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);							buffer += 1;
		u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
		memcpy(buffer, &currentUsingProgram, 4);			buffer += 4;
		memcpy(buffer, &location, 4);						buffer += 4;
		memcpy(buffer, &count, 4);							buffer += 4;
		if (count > 0)
		{
			memcpy(buffer, v, len);							buffer += len;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glUniform3i(GLint location, GLint x, GLint y, GLint z)
{
	KPHelper::LockMutex0();
	
	KPProgram* prog = validate_glUniform1234fiv("on_glUniform3i", 3, false/*isFloat*/, false/*isVector*/, location);
	u8 error = prog ? 0 : 1;
	if (!error)
	{
		prog->on_glUniform3i(location, x, y, z);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUniform3i);
		m_msg.m_length = 1 + 4 * 2 + 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);						buffer += 1;
		u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
		memcpy(buffer, &currentUsingProgram, 4);		buffer += 4;
		memcpy(buffer, &location, 4);					buffer += 4;
		memcpy(buffer, &x, 4);							buffer += 4;
		memcpy(buffer, &y, 4);							buffer += 4;
		memcpy(buffer, &z, 4);							buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glUniform3iv(GLint location, GLsizei count, const GLint* v)
{
	KPHelper::LockMutex0();
	
	KPProgram* prog = validate_glUniform1234fiv("on_glUniform3iv", 3, false/*isFloat*/, true/*isVector*/, location, count);
	u8 error = prog ? 0 : 1;
	if (!error && location != -1 && count > 0)
	{
		prog->on_glUniform3iv(location, count, v);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUniform3iv);
		m_msg.m_length = 1 + 4 + 4 + 4;
		const int len = count * 4 * 3;
		if (count > 0)
		{
			m_msg.m_length += len;
		}
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);							buffer += 1;
		u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
		memcpy(buffer, &currentUsingProgram, 4);			buffer += 4;
		memcpy(buffer, &location, 4);						buffer += 4;
		memcpy(buffer, &count, 4);							buffer += 4;
		if (count > 0)
		{
			memcpy(buffer, v, len);							buffer += len;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glUniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	KPHelper::LockMutex0();
	
	KPProgram* prog = validate_glUniform1234fiv("on_glUniform4f", 4, true/*isFloat*/, false/*isVector*/, location);
	u8 error = prog ? 0 : 1;
	if (!error)
	{
		prog->on_glUniform4f(location, x, y, z, w);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUniform4f);
		m_msg.m_length = 1 + 4 * 2 + 4 * 4;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);							buffer += 1;
		u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
		memcpy(buffer, &currentUsingProgram, 4);			buffer += 4;
		memcpy(buffer, &location, 4);						buffer += 4;
		memcpy(buffer, &x, 4);								buffer += 4;
		memcpy(buffer, &y, 4);								buffer += 4;
		memcpy(buffer, &z, 4);								buffer += 4;
		memcpy(buffer, &w, 4);								buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glUniform4fv(GLint location, GLsizei count, const GLfloat* v)
{
	KPHelper::LockMutex0();
	
	KPProgram* prog = validate_glUniform1234fiv("on_glUniform4fv", 4, true/*isFloat*/, true/*isVector*/, location, count);
	u8 error = prog ? 0 : 1;
	if (!error && location != -1 && count > 0)
	{
		prog->on_glUniform4fv(location, count, v);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUniform4fv);
		m_msg.m_length = 1 + 4 + 4 + 4;
		const int len = count * 4 * 4;
		if (count > 0)
		{
			m_msg.m_length += len;
		}
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);							buffer += 1;
		u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
		memcpy(buffer, &currentUsingProgram, 4);			buffer += 4;
		memcpy(buffer, &location, 4);						buffer += 4;
		memcpy(buffer, &count, 4);							buffer += 4;
		if (count > 0)
		{
			memcpy(buffer, v, len);							buffer += len;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glUniform4i(GLint location, GLint x, GLint y, GLint z, GLint w)
{
	KPHelper::LockMutex0();
	
	KPProgram* prog = validate_glUniform1234fiv("on_glUniform4i", 4, false/*isFloat*/, false/*isVector*/, location);
	u8 error = prog ? 0 : 1;
	if (!error)
	{
		prog->on_glUniform4i(location, x, y, z, w);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUniform4i);
		m_msg.m_length = 1 + 4 * 2 + 4 * 4;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);						buffer += 1;
		u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
		memcpy(buffer, &currentUsingProgram, 4);		buffer += 4;
		memcpy(buffer, &location, 4);					buffer += 4;
		memcpy(buffer, &x, 4);							buffer += 4;
		memcpy(buffer, &y, 4);							buffer += 4;
		memcpy(buffer, &z, 4);							buffer += 4;
		memcpy(buffer, &w, 4);							buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glUniform4iv(GLint location, GLsizei count, const GLint* v)
{
	KPHelper::LockMutex0();
	
	KPProgram* prog = validate_glUniform1234fiv("on_glUniform4iv", 4, false/*isFloat*/, true/*isVector*/, location, count);
	u8 error = prog ? 0 : 1;
	if (!error && location != -1 && count > 0)
	{
		prog->on_glUniform4iv(location, count, v);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUniform4iv);
		m_msg.m_length = 1 + 4 + 4 + 4;
		const int len = count * 4 * 4;
		if (count > 0)
		{
			m_msg.m_length += len;
		}
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);							buffer += 1;
		u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
		memcpy(buffer, &currentUsingProgram, 4);			buffer += 4;
		memcpy(buffer, &location, 4);						buffer += 4;
		memcpy(buffer, &count, 4);							buffer += 4;
		if (count > 0)
		{
			memcpy(buffer, v, len);							buffer += len;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	KPHelper::LockMutex0();
	
	KPProgram* prog = validate_glUniformMatrix("on_glUniformMatrix2fv", 2, location, count, transpose);
	u8 error = prog ? 0 : 1;
	if (!error && location != -1 && count > 0)
	{
		prog->on_glUniformMatrix2fv(location, count, transpose, value);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUniformMatrix2fv);

		m_msg.m_length = 1 + 4 + 4 + 4 + 1;
		int len = 0;
		if (count > 0)
		{
			len = count * 2 * 2 * 4;
			m_msg.m_length += len;
		}
		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);						buffer += 1;
		u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
		memcpy(buffer, &currentUsingProgram, 4);		buffer += 4;
		memcpy(buffer, &location, 4);					buffer += 4;
		memcpy(buffer, &count, 4);						buffer += 4;
		memcpy(buffer, &transpose, 1);					buffer += 1;
		if (count > 0)
		{
			memcpy(buffer, value, len);					buffer += len;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	KPHelper::LockMutex0();
	
	KPProgram* prog = validate_glUniformMatrix("on_glUniformMatrix3fv", 3, location, count, transpose);
	u8 error = prog ? 0 : 1;
	if (!error && location != -1 && count > 0)
	{
		prog->on_glUniformMatrix3fv(location, count, transpose, value);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUniformMatrix3fv);

		m_msg.m_length = 1 + 4 + 4 + 4 + 1;
		int len = 0;
		if (count > 0)
		{
			len = count * 3 * 3 * 4;
			m_msg.m_length += len;
		}
		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);						buffer += 1;
		u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
		memcpy(buffer, &currentUsingProgram, 4);		buffer += 4;
		memcpy(buffer, &location, 4);					buffer += 4;
		memcpy(buffer, &count, 4);						buffer += 4;
		memcpy(buffer, &transpose, 1);					buffer += 1;
		if (count > 0)
		{
			memcpy(buffer, value, len);					buffer += len;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	KPHelper::LockMutex0();
	
	KPProgram* prog = validate_glUniformMatrix("on_glUniformMatrix4fv", 4, location, count, transpose);
	u8 error = prog ? 0 : 1;
	if (!error && location != -1 && count > 0)
	{
		prog->on_glUniformMatrix4fv(location, count, transpose, value);
	}

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUniformMatrix4fv);

		m_msg.m_length = 1 + 4 + 4 + 4 + 1;
		int len = 0;
		if (count > 0)
		{
			len = count * 4 * 4 * 4;
			m_msg.m_length += len;
		}
		m_msg.m_pData = new char[m_msg.m_length];
		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);						buffer += 1;
		u32 currentUsingProgram = KPHelper::getCurrentUsingProgram();
		memcpy(buffer, &currentUsingProgram, 4);		buffer += 4;
		memcpy(buffer, &location, 4);					buffer += 4;
		memcpy(buffer, &count, 4);						buffer += 4;
		memcpy(buffer, &transpose, 1);					buffer += 1;
		if (count > 0)
		{
			memcpy(buffer, value, len);					buffer += len;
		}
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

// end of glUniform***
//===============================================================================

void KPServer::on_glUseProgram(GLuint program)
{
	KPHelper::LockMutex0();

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUseProgram);

		m_msg.m_length = 4 + 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &program, 4);
		u32 actual = KPHelper::getCurrentUsingProgram();
		memcpy(m_msg.m_pData + 4, &actual, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glValidateProgram(GLuint program)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glValidateProgram);

		m_msg.m_length = 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &program, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}
//===========================================================================================
void KPServer::on_glVertexAttrib1f(GLuint indx, GLfloat x)
{
	KPHelper::LockMutex0();

	const int k_MAX_VERTEX_ATTRIBS = KPHelper::get_MAX_VERTEX_ATTRIBS();
	u8 error = 0;

	if (indx >= k_MAX_VERTEX_ATTRIBS)
	{
		LOGE("error [%s] indx = %d, MAX_VERTEX_ATTRIBS = %d", __FUNCTION__, indx, k_MAX_VERTEX_ATTRIBS);
		error = 1;
		goto my_end;
	}

	m_vertexAttributes[indx].on_glVertexAttrib(1, &x);

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glVertexAttrib1f);

		m_msg.m_length = 1 + 4 * 2;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);				buffer += 1;
		memcpy(buffer, &indx, 4);				buffer += 4;
		memcpy(buffer, &x, 4);					buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glVertexAttrib1fv(GLuint indx, const GLfloat* values)
{
	KPHelper::LockMutex0();

	const int k_MAX_VERTEX_ATTRIBS = KPHelper::get_MAX_VERTEX_ATTRIBS();
	u8 error = 0;

	if (indx >= k_MAX_VERTEX_ATTRIBS)
	{
		LOGE("error [%s] indx = %d, MAX_VERTEX_ATTRIBS = %d", __FUNCTION__, indx, k_MAX_VERTEX_ATTRIBS);
		error = 1;
		goto my_end;
	}

	m_vertexAttributes[indx].on_glVertexAttrib(1, values);

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glVertexAttrib1fv);

		m_msg.m_length = 1 + 4 * 2;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);				buffer += 1;
		memcpy(buffer, &indx, 4);				buffer += 4;
		memcpy(buffer, values, 4);				buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glVertexAttrib2f(GLuint indx, GLfloat x, GLfloat y)
{
	KPHelper::LockMutex0();

	const int k_MAX_VERTEX_ATTRIBS = KPHelper::get_MAX_VERTEX_ATTRIBS();
	u8 error = 0;

	if (indx >= k_MAX_VERTEX_ATTRIBS)
	{
		LOGE("error [%s] indx = %d, MAX_VERTEX_ATTRIBS = %d", __FUNCTION__, indx, k_MAX_VERTEX_ATTRIBS);
		error = 1;
		goto my_end;
	}

	{
		GLfloat tmp[] = { x, y };
		m_vertexAttributes[indx].on_glVertexAttrib(2, tmp);
	}

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glVertexAttrib2f);

		m_msg.m_length = 1 + 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);				buffer += 1;
		memcpy(buffer, &indx, 4);				buffer += 4;
		memcpy(buffer, &x, 4);					buffer += 4;
		memcpy(buffer, &y, 4);					buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glVertexAttrib2fv(GLuint indx, const GLfloat* values)
{
	KPHelper::LockMutex0();

	const int k_MAX_VERTEX_ATTRIBS = KPHelper::get_MAX_VERTEX_ATTRIBS();
	u8 error = 0;

	if (indx >= k_MAX_VERTEX_ATTRIBS)
	{
		LOGE("error [%s] indx = %d, MAX_VERTEX_ATTRIBS = %d", __FUNCTION__, indx, k_MAX_VERTEX_ATTRIBS);
		error = 1;
		goto my_end;
	}

	m_vertexAttributes[indx].on_glVertexAttrib(2, values);

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glVertexAttrib2fv);

		m_msg.m_length = 1 + 4 * 3;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);				buffer += 1;
		memcpy(buffer, &indx, 4);				buffer += 4;
		memcpy(buffer, values, 4);				buffer += 4; values += 4;
		memcpy(buffer, values, 4);				buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glVertexAttrib3f(GLuint indx, GLfloat x, GLfloat y, GLfloat z)
{
	KPHelper::LockMutex0();

	const int k_MAX_VERTEX_ATTRIBS = KPHelper::get_MAX_VERTEX_ATTRIBS();
	u8 error = 0;

	if (indx >= k_MAX_VERTEX_ATTRIBS)
	{
		LOGE("error [%s] indx = %d, MAX_VERTEX_ATTRIBS = %d", __FUNCTION__, indx, k_MAX_VERTEX_ATTRIBS);
		error = 1;
		goto my_end;
	}

	{
		GLfloat tmp[] = { x, y, z };
		m_vertexAttributes[indx].on_glVertexAttrib(3, tmp);
	}

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glVertexAttrib3f);

		m_msg.m_length = 1 + 4 * 4;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);				buffer += 1;
		memcpy(buffer, &indx, 4);				buffer += 4;
		memcpy(buffer, &x, 4);					buffer += 4;
		memcpy(buffer, &y, 4);					buffer += 4;
		memcpy(buffer, &z, 4);					buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glVertexAttrib3fv(GLuint indx, const GLfloat* values)
{
	KPHelper::LockMutex0();

	const int k_MAX_VERTEX_ATTRIBS = KPHelper::get_MAX_VERTEX_ATTRIBS();
	u8 error = 0;

	if (indx >= k_MAX_VERTEX_ATTRIBS)
	{
		LOGE("error [%s] indx = %d, MAX_VERTEX_ATTRIBS = %d", __FUNCTION__, indx, k_MAX_VERTEX_ATTRIBS);
		error = 1;
		goto my_end;
	}

	m_vertexAttributes[indx].on_glVertexAttrib(3, values);

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glVertexAttrib3fv);

		m_msg.m_length = 1 + 4 * 4;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);				buffer += 1;
		memcpy(buffer, &indx, 4);				buffer += 4;
		memcpy(buffer, values, 4);				buffer += 4; values += 4;
		memcpy(buffer, values, 4);				buffer += 4; values += 4;
		memcpy(buffer, values, 4);				buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glVertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	KPHelper::LockMutex0();

	const int k_MAX_VERTEX_ATTRIBS = KPHelper::get_MAX_VERTEX_ATTRIBS();
	u8 error = 0;

	if (indx >= k_MAX_VERTEX_ATTRIBS)
	{
		LOGE("error [%s] indx = %d, MAX_VERTEX_ATTRIBS = %d", __FUNCTION__, indx, k_MAX_VERTEX_ATTRIBS);
		error = 1;
		goto my_end;
	}

	{
		GLfloat tmp[] = { x, y, z, w };
		m_vertexAttributes[indx].on_glVertexAttrib(4, tmp);
	}

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glVertexAttrib4f);

		m_msg.m_length = 1 + 4 * 5;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);				buffer += 1;
		memcpy(buffer, &indx, 4);				buffer += 4;
		memcpy(buffer, &x, 4);					buffer += 4;
		memcpy(buffer, &y, 4);					buffer += 4;
		memcpy(buffer, &z, 4);					buffer += 4;
		memcpy(buffer, &w, 4);					buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glVertexAttrib4fv(GLuint indx, const GLfloat* values)
{
	KPHelper::LockMutex0();

	const int k_MAX_VERTEX_ATTRIBS = KPHelper::get_MAX_VERTEX_ATTRIBS();
	u8 error = 0;

	if (indx >= k_MAX_VERTEX_ATTRIBS)
	{
		LOGE("error [%s] indx = %d, MAX_VERTEX_ATTRIBS = %d", __FUNCTION__, indx, k_MAX_VERTEX_ATTRIBS);
		error = 1;
		goto my_end;
	}

	m_vertexAttributes[indx].on_glVertexAttrib(4, values);

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glVertexAttrib4fv);

		m_msg.m_length = 1 + 4 * 5;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);				buffer += 1;
		memcpy(buffer, &indx, 4);				buffer += 4;
		memcpy(buffer, values, 4);				buffer += 4; values += 4;
		memcpy(buffer, values, 4);				buffer += 4; values += 4;
		memcpy(buffer, values, 4);				buffer += 4; values += 4;
		memcpy(buffer, values, 4);				buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glVertexAttribPointer(GLuint indx, GLint size, GLenum type,
		GLboolean normalized, GLsizei stride, const GLvoid* ptr)
{
	KPHelper::LockMutex0();

	const int k_MAX_VERTEX_ATTRIBS = KPHelper::get_MAX_VERTEX_ATTRIBS();
	u8 error = 0;

	if (type != GL_BYTE &&
		type != GL_UNSIGNED_BYTE &&
		type != GL_SHORT &&
		type != GL_UNSIGNED_SHORT &&
		type != GL_FIXED &&
		type != GL_FLOAT)
	{
		LOGE("error [%s] type = 0x%x", __FUNCTION__, type);
		error = 1;
		goto my_end;
	}

	if (indx >= k_MAX_VERTEX_ATTRIBS)
	{
		LOGE("error [%s] indx = %d, MAX_VERTEX_ATTRIBS = %d", __FUNCTION__, indx, k_MAX_VERTEX_ATTRIBS);
		error = 1;
		goto my_end;
	}

	if (size < 1 || size > 4)
	{
		LOGE("error [%s] size = %d", __FUNCTION__, size);
		error = 1;
		goto my_end;
	}

	if (stride < 0)
	{
		LOGE("error [%s] stride = %d", __FUNCTION__, stride);
		error = 1;
		goto my_end;
	}

	m_vertexAttributes[indx].on_glVertexAttribPointer(size, type, normalized,
		stride, ptr, KPHelper::getCurrentBindingVbo_ArrayBuffer());

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glVertexAttribPointer);

		m_msg.m_length = 1/*error*/ + 4/*indx*/ + 4/*size*/ + 4/*type*/ + 1/*normalized*/ + 4/*stride*/ + 4/*ptr*/;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;

		memcpy(buffer, &error, 1);		buffer += 1;
		memcpy(buffer, &indx, 4);		buffer += 4;
		memcpy(buffer, &size, 4);		buffer += 4;
		memcpy(buffer, &type, 4);		buffer += 4;
		memcpy(buffer, &normalized, 1);	buffer += 1;
		memcpy(buffer, &stride, 4);		buffer += 4;
		memcpy(buffer, &ptr, 4);		buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	KPHelper::LockMutex0();
	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glViewport);

		m_msg.m_length = 4 * 4;
		m_msg.m_pData = new char[m_msg.m_length];

		memcpy(m_msg.m_pData, &x, 4);
		memcpy(m_msg.m_pData + 4, &y, 4);
		memcpy(m_msg.m_pData + 8, &width, 4);
		memcpy(m_msg.m_pData + 12, &height, 4);
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

// Extensions
void KPServer::on_glMapBufferOES (GLenum target, GLenum access, void* ptr)
{
	KPHelper::LockMutex0();

	u32 currentBindingVbo;
	KPVbo* vbo;
	u8 error = 0;

	if (target != GL_ARRAY_BUFFER && target != GL_ELEMENT_ARRAY_BUFFER)
	{
		LOGE("error [%s] Unknown target = 0x%x", __FUNCTION__, target);
		error = 1;
		goto my_end;
	}

	currentBindingVbo = KPHelper::getCurrentBindingVboOfTarget(target);

	vbo = getVbo(currentBindingVbo);
	if (vbo)
	{
		vbo->on_glMapBufferOES(access, ptr);
	}
	else
	{
		LOGE("error [%s] Not found the vbo id: %d", __FUNCTION__, currentBindingVbo);
		error = 1;
		goto my_end;
	}

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glMapBufferOES);
	
		m_msg.m_length = 1/*error*/ + 4/*currentBindingVbo*/ + 4/*target*/ + 4/*access*/ + 4/*ptr*/;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;
		
		memcpy(buffer, &error, 1);					buffer += 1;
		memcpy(buffer, &currentBindingVbo, 4);		buffer += 4;
		memcpy(buffer, &target, 4);					buffer += 4;
		memcpy(buffer, &access, 4);					buffer += 4;
		memcpy(buffer, &ptr,	4);					buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}

void KPServer::on_glUnmapBufferOES (GLenum target)
{
	KPHelper::LockMutex0();

	u32 currentBindingVbo;
	KPVbo* vbo;
	u8 error = 0;

	if (target != GL_ARRAY_BUFFER && target != GL_ELEMENT_ARRAY_BUFFER)
	{
		LOGE("error [%s] Unknown target = 0x%x", __FUNCTION__, target);
		error = 1;
		goto my_end;
	}

	currentBindingVbo = KPHelper::getCurrentBindingVboOfTarget(target);

	vbo = getVbo(currentBindingVbo);
	if (vbo)
	{
		vbo->on_glUnmapBufferOES();
	}
	else
	{
		LOGE("error [%s] Not found the vbo id: %d", __FUNCTION__, currentBindingVbo);
		error = 1;
		goto my_end;
	}

	my_end:

	if (m_state == CAPTURING)
	{
		makeGLCommandMsg(KMT_glUnmapBufferOES);
	
		m_msg.m_length = 1/*error*/ + 4/*currentBindingVbo*/ + 4/*target*/;
		m_msg.m_pData = new char[m_msg.m_length];

		char* buffer = m_msg.m_pData;
		
		memcpy(buffer, &error, 1);					buffer += 1;
		memcpy(buffer, &currentBindingVbo, 4);		buffer += 4;
		memcpy(buffer, &target, 4);					buffer += 4;
	
		if ( KPHelper::sendMessageToClient(m_client, m_msg) == SOCKET_ERROR ) m_state = IDLE;
		m_msg.clearData();
	}
	KPHelper::UnlockMutex0();
}


//==================================================================================================
//==================================================================================================
//==================================================================================================

int KPServer::calculateVertexAttribDataLength(KPProgram* prog, int vertexCount)
{
	if (!prog) return 4;

	int attCount = prog->getAttributesCount();
	int dataLength = 4/*attCount*/;

	for (int i = 0; i < attCount; i++)
	{
		const KPAttribute& att = prog->getAttribute(i);

		KPVertexAttribData& vad = m_vertexAttributes[att.m_location];

		u8 enableVertexAttribArray = vad.getEnableVertexAttribArray() ? 1 : 0;

		dataLength += 1/*enableVertexAttribArray*/;
		if (enableVertexAttribArray)
		{
			dataLength +=
				4/*location*/ +
				4/*dataComponentsNumber*/ +
				1/*normalized*/ +
				4/*vboId*/ +
				4/*dataType*/ +
				KPHelper::getSizeInBytesOfGLDataType(vad.getDataType()) * vad.getDataComponentsNumber() * vertexCount;
		}
		else
		{
			dataLength +=
				4/*location*/ +
				4/*m_dataNonArray_Components*/ +
				vad.getDataNonArray_Components() * 4;

		}
	}
	
	return dataLength;
}

void KPServer::makeVertexAttribData(char* buffer, KPProgram* prog,
		bool isDrawElements,
			GLsizei countElements, GLenum type, const GLvoid* indices,
			GLint first, GLsizei countArrays)
{
	if (!prog)
	{
		int attCount = 0;
		memcpy(buffer, &attCount, 4);
		return;
	}

	int attCount = prog->getAttributesCount();
	memcpy(buffer, &attCount, 4); buffer += 4;
			
	for (int i = 0; i < attCount; i++)
	{
		const KPAttribute& att = prog->getAttribute(i);
		int location = att.m_location;
		KPVertexAttribData& vad = m_vertexAttributes[location];

		u8 enableVertexAttribArray = vad.getEnableVertexAttribArray() ? 1 : 0;
		memcpy(buffer, &enableVertexAttribArray, 1);
		buffer += 1;

		if (enableVertexAttribArray)
		{
			int components	= vad.getDataComponentsNumber();
			u32 dataType	= vad.getDataType();
			int dataStride	= vad.getDataStride();
			u8 normalized	= vad.getDataNormalized() ? 1 : 0;

			u32 vboId = vad.getVboId();

			memcpy(buffer, &location, 4);		buffer += 4;
			memcpy(buffer, &components, 4);		buffer += 4;
			memcpy(buffer, &normalized, 1);		buffer += 1;

			memcpy(buffer, &vboId, 4);			buffer += 4;
			memcpy(buffer, &dataType, 4);		buffer += 4;
					
			const int smallStride = KPHelper::getSizeInBytesOfGLDataType(dataType) * components;
			const int bigStride = dataStride > 0 ? dataStride : smallStride;

			const char* p = NULL;
			if (vboId > 0)
			{
				int offset = (int)( vad.getDataPointer() );
				KPVbo* vbo = getVbo(vboId);
				if ( vbo && vbo->getData() && offset >= 0 && offset < vbo->getSize() ) // TODO
				{
					p = (const char*)( vbo->getData() + offset );
				}
				else
				{
					LOGE("error in [%s]", __FUNCTION__);
					p = (const char*)( vad.getDataPointer() );
				}
			}
			else
			{
				p = (const char*)( vad.getDataPointer() );
			}

			if (isDrawElements)
			{
				const u8*	pIndices8	= (const u8*)	indices;
				const u16*	pIndices16	= (const u16*)	indices;
				for (int j = 0; j < countElements; j++)
				{
					int vertexIndex = (int)( type == GL_UNSIGNED_BYTE ? pIndices8[j] : pIndices16[j] );
					memcpy(buffer, p + bigStride * vertexIndex, smallStride);
					buffer += smallStride;
				}
			}
			else
			{
				for (int j = 0; j < countArrays; j++)
				{
					int vertexIndex = first + j;
					memcpy(buffer, p + bigStride * vertexIndex, smallStride);
					buffer += smallStride;
				}
			}
		}
		else
		{
			int components	= vad.getDataNonArray_Components();
			const float* values = vad.getDataNonArray_Values();

			memcpy(buffer, &location, 4);				buffer += 4;
			memcpy(buffer, &components, 4);				buffer += 4;
			memcpy(buffer, values, components * 4);		buffer += components * 4;
		}
	}
}

void KPServer::copyCurrentFboToTexture(const char* pixels, int width, int height)
{
	u32 texId;
	int mipLevel;
	if (KPHelper::isRenderingToTexture(texId, mipLevel))
	{
		KPTexture* pTex = getTexture(texId);
		if (pTex)
		{
			if (pixels)
			{
				pTex->copyMipmapDataFromRGBA(mipLevel, width, height, pixels);
			}
			else
			{
				int x, y;
				KPHelper::getViewport(x, y, width, height);
				char* p = new char[width * height * 4];
				KPHelper::readPixels(x, y, width, height, p);
				pTex->copyMipmapDataFromRGBA(mipLevel, width, height, p);
				SAFE_DEL_ARRAY(p);
			}
		}
	}
}
