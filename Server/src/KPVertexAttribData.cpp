#include "KPVertexAttribData.h"

KPVertexAttribData::KPVertexAttribData():
		m_enableVertexAttribArray	(true),

		m_dataComponentsNumber		(0),
		m_dataType					(0),
		m_dataStride				(0),
		m_dataPointer				(NULL),
		m_vboId						(0),

		m_dataNonArray_Components	(0)
{
}

KPVertexAttribData::~KPVertexAttribData()
{
}

void KPVertexAttribData::on_glVertexAttribPointer(GLint size, GLenum type, GLboolean normalized,
			GLsizei stride, const GLvoid * pointer, u32 vboId)
{
	m_dataComponentsNumber		= size;
	m_dataType					= type;
	m_dataNormalized			= normalized;
	m_dataStride				= stride;
	m_dataPointer				= pointer;
	m_vboId						= vboId;
}

void KPVertexAttribData::on_glVertexAttrib(int components, const float* v)
{
	m_dataNonArray_Components = components;
	for (int i = 0; i < components; i++)
	{
		m_dataNonArray_Values[i] = v[i];
	}
}

void KPVertexAttribData::setEnableVertexAttribArray(bool b)
{
	m_enableVertexAttribArray = b;
}

bool KPVertexAttribData::getEnableVertexAttribArray() const
{
	return m_enableVertexAttribArray;
}

int KPVertexAttribData::getDataComponentsNumber() const
{
	return m_dataComponentsNumber;
}

u32 KPVertexAttribData::getDataType() const
{
	return m_dataType;
}

bool KPVertexAttribData::getDataNormalized() const
{
	return m_dataNormalized;
}

int KPVertexAttribData::getDataStride() const
{
	return m_dataStride;
}
const void* KPVertexAttribData::getDataPointer() const
{
	return m_dataPointer;
}

u32 KPVertexAttribData::getVboId() const
{
	return m_vboId;
}

int KPVertexAttribData::getDataNonArray_Components() const
{
	return m_dataNonArray_Components;
}

const float* KPVertexAttribData::getDataNonArray_Values() const
{
	return (const float*)m_dataNonArray_Values;
}
