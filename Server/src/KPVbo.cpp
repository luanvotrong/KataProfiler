#include "KPVbo.h"


KPVbo::KPVbo() : KPGLObject(), m_size(0), m_pData(NULL), m_usage(0)
{
}

KPVbo::KPVbo(u32 id) : KPGLObject(id), m_size(0), m_pData(NULL), m_usage(0)
{
}

KPVbo::~KPVbo()
{
	clearData();
}

void KPVbo::clearData()
{
	m_size = 0;
	SAFE_DEL_ARRAY(m_pData);
	m_usage = 0;
}

void KPVbo::on_glBufferData(int size,  const void * data,  u32 usage)
{
	clearData();
	m_size = size;
	// size can be zero
	if (size > 0)
	{
		m_pData = new char[size];
		if (data)
		{
			memcpy(m_pData, data, size);
		}
		else
		{
			memset(m_pData, 0, size);
		}
	}
	m_usage = usage;
}

void KPVbo::on_glBufferSubData(int offset, int size, const void* data)
{
	// size can be zero
	if (size > 0)
	{
		memcpy(m_pData + offset, data, size);
	}
}

void KPVbo::on_glMapBufferOES (GLenum access, void* ptr)
{
	SAFE_DEL_ARRAY(m_pData);
	m_pData = (char*)ptr;
}

void KPVbo::on_glUnmapBufferOES()
{
	if (m_size > 0 && m_pData)
	{
		char* p = new char[m_size];
		memcpy(p, m_pData, m_size);
		m_pData = p;
	}
}

KPMessage* KPVbo::toMessage()
{
	KPMessage* msg = new KPMessage();
	msg->m_type = KMT_OBJECT_VBO;

	msg->m_length = 4 * 4;
	msg->m_pData = new char[msg->m_length];

	char* buffer = msg->m_pData;
	
	memcpy(buffer, &m_id, 4);		buffer += 4;
	memcpy(buffer, &m_size, 4);		buffer += 4;
	memcpy(buffer, &m_usage, 4);	buffer += 4;
	memcpy(buffer, &m_pData, 4);	buffer += 4;

	return msg;
}

//
int KPVbo::getSize()
{
	return m_size;
}
char* KPVbo::getData()
{
	return m_pData;
}
u32 KPVbo::getUsage()
{
	return m_usage;
}
