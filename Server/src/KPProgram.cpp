#include "KPProgram.h"

#include "KPHelper.h"

extern void (*my_glGetProgramiv)(GLuint, GLenum, GLint*);

extern void (*my_glGetActiveAttrib)(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*);
extern int (*my_glGetAttribLocation)(GLuint, const GLchar*);
extern void (*my_glGetActiveUniform)(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*);
extern int (*my_glGetUniformLocation)(GLuint, const GLchar*);

void KPProgram::clearData()
{
	m_vsId = 0;
	m_fsId = 0;

	m_attributesCount = 0;
	m_uniformsCount = 0;
}

bool KPProgram::attachVs(u32 vs)
{
	if (m_vsId > 0) return false;
	m_vsId = vs;
	return true;
}

bool KPProgram::attachFs(u32 fs)
{
	if (m_fsId > 0) return false;
	m_fsId = fs;
	return true;
}

bool KPProgram::detach(u32 shader)
{
	if (shader == m_vsId)
	{
		m_vsId = 0;
		return true;
	}
	if (shader == m_fsId)
	{
		m_fsId = 0;
		return true;
	}
	return false;
}

KPMessage* KPProgram::toMessage()
{
	KPMessage* msg = new KPMessage();
	msg->m_type = KMT_OBJECT_PROGRAM;
	msg->m_length = 4 * 3;

	msg->m_length += 4;
	for (int i = 0; i < m_attributesCount; i++)
	{
		msg->m_length += 4 * 3 +
			4 + strlen(m_attributes[i].m_szName);
		// No need value for attributes
	}

	msg->m_length += 4;
	for (int i = 0; i < m_uniformsCount; i++)
	{
		msg->m_length += 4 * 3 +
			4 + strlen(m_uniforms[i].m_szName) +
			4 + m_uniforms[i].m_valueBytesNumber;
	}

	//
	msg->m_pData = new char[msg->m_length];

	char* buffer = msg->m_pData;

	memcpy(buffer, &m_id, 4); buffer += 4;
	memcpy(buffer, &m_vsId, 4); buffer += 4;
	memcpy(buffer, &m_fsId, 4); buffer += 4;

	//
	memcpy(buffer, &m_attributesCount, 4); buffer += 4;
	for (int i = 0; i < m_attributesCount; i++)
	{
		KPAttribute& var = m_attributes[i];

		memcpy(buffer, &var.m_location, 4); buffer += 4;
		memcpy(buffer, &var.m_size, 4); buffer += 4;
		memcpy(buffer, &var.m_type, 4); buffer += 4;
		
		int strLen = strlen(var.m_szName);
		memcpy(buffer, &strLen, 4); buffer += 4;
		if (strLen > 0)
		{
			memcpy(buffer, var.m_szName, strLen); buffer += strLen;
		}
		// No need value for attributes
	}

	//
	memcpy(buffer, &m_uniformsCount, 4); buffer += 4;
	for (int i = 0; i < m_uniformsCount; i++)
	{
		KPUniform& var = m_uniforms[i];

		memcpy(buffer, &var.m_location, 4); buffer += 4;
		memcpy(buffer, &var.m_size, 4); buffer += 4;
		memcpy(buffer, &var.m_type, 4); buffer += 4;
		
		// name
		int len = strlen(var.m_szName);
		memcpy(buffer, &len, 4); buffer += 4;
		if (len > 0)
		{
			memcpy(buffer, var.m_szName, len); buffer += len;
		}

		// value
		len = var.m_valueBytesNumber;
		memcpy(buffer, &len, 4); buffer += 4;
		if (len > 0)
		{
			memcpy(buffer, var.m_pValuePointer, len); buffer += len;
		}
	}
	
	return msg;
}

void KPProgram::link()
{
	if (m_id == 0) return;
	if (m_vsId == 0 || m_fsId == 0) return;

	int activeCount = 0;
	
	//
	my_glGetProgramiv(m_id, GL_ACTIVE_ATTRIBUTES, &activeCount);
	m_attributesCount = activeCount;
	for (int index = 0; index < m_attributesCount; index++)
	{
		KPAttribute& var = m_attributes[index];
		int length = 0;
		my_glGetActiveAttrib(m_id, index, MAX_VAR_NAME_LENGTH, &length, &var.m_size, &var.m_type, var.m_szName);
		if (length >= 0) var.m_szName[length] = 0;
		var.m_location = my_glGetAttribLocation(m_id, var.m_szName);
	}

	//
	my_glGetProgramiv(m_id, GL_ACTIVE_UNIFORMS, &activeCount);
	m_uniformsCount = activeCount;
	for (int index = 0; index < m_uniformsCount; index++)
	{
		KPUniform& var = m_uniforms[index];

		var.freeValue();

		int length = 0;
		my_glGetActiveUniform(m_id, index, MAX_VAR_NAME_LENGTH, &length, &var.m_size, &var.m_type, var.m_szName);
		if (length < 0) length = 0;
		var.m_szName[length] = 0;
		var.m_location = my_glGetUniformLocation(m_id, var.m_szName);
	}
}

int KPProgram::getUniformsCount() const
{
	return m_uniformsCount;
}

const KPUniform& KPProgram::getUniform(int index) const
{
	return m_uniforms[index];
}
const KPUniform* KPProgram::getUniformByLocation(int location) const
{
	for (int i = 0; i < m_uniformsCount; i++)
	{
		if (m_uniforms[i].m_location == location)
		{
			return &m_uniforms[i];
		}
	}
	return NULL;
}

int KPProgram::getAttributesCount() const
{
	return m_attributesCount;
}

const KPAttribute& KPProgram::getAttribute(int index) const
{
	return m_attributes[index];
}

//=======================================================================================
//=======================================================================================

void KPProgram::on_glUniform1f(GLint location, GLfloat x)
{
	for (int i = 0; i < m_uniformsCount; i++)
	{
		KPUniform& var = m_uniforms[i];
		if (var.m_location == location)
		{
			var.freeValue();

			var.m_valueBytesNumber = 4;
			var.m_pValuePointer = new char[var.m_valueBytesNumber];
			memcpy(var.m_pValuePointer, &x, var.m_valueBytesNumber);

			break;
		}
	}
}

void KPProgram::on_glUniform1fv(GLint location, GLsizei count, const GLfloat* value)
{
	for (int i = 0; i < m_uniformsCount; i++)
	{
		KPUniform& var = m_uniforms[i];
		if (var.m_location == location)
		{
			var.freeValue();

			var.m_valueBytesNumber = 4 * count;
			var.m_pValuePointer = new char[var.m_valueBytesNumber];
			memcpy(var.m_pValuePointer, value, var.m_valueBytesNumber);

			break;
		}
	}
}

void KPProgram::on_glUniform1i(GLint location, GLint x)
{
	for (int i = 0; i < m_uniformsCount; i++)
	{
		KPUniform& var = m_uniforms[i];
		if (var.m_location == location)
		{
			var.freeValue();

			var.m_valueBytesNumber = 4;
			var.m_pValuePointer = new char[var.m_valueBytesNumber];
			memcpy(var.m_pValuePointer, &x, var.m_valueBytesNumber);

			break;
		}
	}
}

void KPProgram::on_glUniform1iv(GLint location, GLsizei count, const GLint* value)
{
	for (int i = 0; i < m_uniformsCount; i++)
	{
		KPUniform& var = m_uniforms[i];
		if (var.m_location == location)
		{
			var.freeValue();

			var.m_valueBytesNumber = 4 * count;
			var.m_pValuePointer = new char[var.m_valueBytesNumber];
			memcpy(var.m_pValuePointer, value, var.m_valueBytesNumber);

			break;
		}
	}
}

void KPProgram::on_glUniform2f(GLint location, GLfloat x, GLfloat y)
{
	for (int i = 0; i < m_uniformsCount; i++)
	{
		KPUniform& var = m_uniforms[i];
		if (var.m_location == location)
		{
			var.freeValue();

			var.m_valueBytesNumber = 4 * 2;
			var.m_pValuePointer = new char[var.m_valueBytesNumber];
			memcpy(var.m_pValuePointer, &x, 4);
			memcpy(var.m_pValuePointer + 4, &y, 4);

			break;
		}
	}
}

void KPProgram::on_glUniform2fv(GLint location, GLsizei count, const GLfloat* value)
{
	for (int i = 0; i < m_uniformsCount; i++)
	{
		KPUniform& var = m_uniforms[i];
		if (var.m_location == location)
		{
			var.freeValue();

			var.m_valueBytesNumber = 4 * 2 * count;
			var.m_pValuePointer = new char[var.m_valueBytesNumber];
			memcpy(var.m_pValuePointer, value, var.m_valueBytesNumber);

			break;
		}
	}
}

void KPProgram::on_glUniform2i(GLint location, GLint x, GLint y)
{
	for (int i = 0; i < m_uniformsCount; i++)
	{
		KPUniform& var = m_uniforms[i];
		if (var.m_location == location)
		{
			var.freeValue();

			var.m_valueBytesNumber = 4 * 2;
			var.m_pValuePointer = new char[var.m_valueBytesNumber];
			memcpy(var.m_pValuePointer, &x, 4);
			memcpy(var.m_pValuePointer + 4, &y, 4);

			break;
		}
	}
}

void KPProgram::on_glUniform2iv(GLint location, GLsizei count, const GLint* value)
{
	for (int i = 0; i < m_uniformsCount; i++)
	{
		KPUniform& var = m_uniforms[i];
		if (var.m_location == location)
		{
			var.freeValue();

			var.m_valueBytesNumber = 4 * 2 * count;
			var.m_pValuePointer = new char[var.m_valueBytesNumber];
			memcpy(var.m_pValuePointer, value, var.m_valueBytesNumber);

			break;
		}
	}
}

void KPProgram::on_glUniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z)
{
	for (int i = 0; i < m_uniformsCount; i++)
	{
		KPUniform& var = m_uniforms[i];
		if (var.m_location == location)
		{
			var.freeValue();

			var.m_valueBytesNumber = 4 * 3;
			var.m_pValuePointer = new char[var.m_valueBytesNumber];
			memcpy(var.m_pValuePointer, &x, 4);
			memcpy(var.m_pValuePointer + 4, &y, 4);
			memcpy(var.m_pValuePointer + 8, &z, 4);

			break;
		}
	}
}

void KPProgram::on_glUniform3fv(GLint location, GLsizei count, const GLfloat* value)
{
	for (int i = 0; i < m_uniformsCount; i++)
	{
		KPUniform& var = m_uniforms[i];
		if (var.m_location == location)
		{
			var.freeValue();

			var.m_valueBytesNumber = 4 * 3 * count;
			var.m_pValuePointer = new char[var.m_valueBytesNumber];
			memcpy(var.m_pValuePointer, value, var.m_valueBytesNumber);

			break;
		}
	}
}

void KPProgram::on_glUniform3i(GLint location, GLint x, GLint y, GLint z)
{
	for (int i = 0; i < m_uniformsCount; i++)
	{
		KPUniform& var = m_uniforms[i];
		if (var.m_location == location)
		{
			var.freeValue();

			var.m_valueBytesNumber = 4 * 3;
			var.m_pValuePointer = new char[var.m_valueBytesNumber];
			memcpy(var.m_pValuePointer, &x, 4);
			memcpy(var.m_pValuePointer + 4, &y, 4);
			memcpy(var.m_pValuePointer + 8, &z, 4);

			break;
		}
	}
}

void KPProgram::on_glUniform3iv(GLint location, GLsizei count, const GLint* value)
{
	for (int i = 0; i < m_uniformsCount; i++)
	{
		KPUniform& var = m_uniforms[i];
		if (var.m_location == location)
		{
			var.freeValue();

			var.m_valueBytesNumber = 4 * 3 * count;
			var.m_pValuePointer = new char[var.m_valueBytesNumber];
			memcpy(var.m_pValuePointer, value, var.m_valueBytesNumber);

			break;
		}
	}
}

void KPProgram::on_glUniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	for (int i = 0; i < m_uniformsCount; i++)
	{
		KPUniform& var = m_uniforms[i];
		if (var.m_location == location)
		{
			var.freeValue();

			var.m_valueBytesNumber = 4 * 4;
			var.m_pValuePointer = new char[var.m_valueBytesNumber];
			memcpy(var.m_pValuePointer, &x, 4);
			memcpy(var.m_pValuePointer + 4, &y, 4);
			memcpy(var.m_pValuePointer + 8, &z, 4);
			memcpy(var.m_pValuePointer + 12, &w, 4);

			break;
		}
	}
}

void KPProgram::on_glUniform4fv(GLint location, GLsizei count, const GLfloat* value)
{
	for (int i = 0; i < m_uniformsCount; i++)
	{
		KPUniform& var = m_uniforms[i];
		if (var.m_location == location)
		{
			var.freeValue();

			var.m_valueBytesNumber = 4 * 4 * count;
			var.m_pValuePointer = new char[var.m_valueBytesNumber];
			memcpy(var.m_pValuePointer, value, var.m_valueBytesNumber);

			break;
		}
	}
}

void KPProgram::on_glUniform4i(GLint location, GLint x, GLint y, GLint z, GLint w)
{
	for (int i = 0; i < m_uniformsCount; i++)
	{
		KPUniform& var = m_uniforms[i];
		if (var.m_location == location)
		{
			var.freeValue();

			var.m_valueBytesNumber = 4 * 4;
			var.m_pValuePointer = new char[var.m_valueBytesNumber];
			memcpy(var.m_pValuePointer, &x, 4);
			memcpy(var.m_pValuePointer + 4, &y, 4);
			memcpy(var.m_pValuePointer + 8, &z, 4);
			memcpy(var.m_pValuePointer + 12, &w, 4);

			break;
		}
	}
}

void KPProgram::on_glUniform4iv(GLint location, GLsizei count, const GLint* value)
{
	for (int i = 0; i < m_uniformsCount; i++)
	{
		KPUniform& var = m_uniforms[i];
		if (var.m_location == location)
		{
			var.freeValue();

			var.m_valueBytesNumber = 4 * 4 * count;
			var.m_pValuePointer = new char[var.m_valueBytesNumber];
			memcpy(var.m_pValuePointer, value, var.m_valueBytesNumber);

			break;
		}
	}
}

void KPProgram::on_glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	on_glUniformMatrix(location, count, transpose, value, 2);
}

void KPProgram::on_glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	on_glUniformMatrix(location, count, transpose, value, 3);
}

void KPProgram::on_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	on_glUniformMatrix(location, count, transpose, value, 4);
}

//
void KPProgram::on_glUniformMatrix(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value, int size)
{
	for (int k = 0; k < m_uniformsCount; k++)
	{
		KPUniform& var = m_uniforms[k];
		if (var.m_location == location)
		{
			var.freeValue();

			var.m_valueBytesNumber = count * size * size * 4;
			var.m_pValuePointer = new char[var.m_valueBytesNumber];
			memcpy(var.m_pValuePointer, value, var.m_valueBytesNumber);

			break;
		}
	}
}
