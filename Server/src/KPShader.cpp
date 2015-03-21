#include "KPShader.h"

KPShader::KPShader(u32 id, const char* kszSource) : KPGLObject(id), m_szSource(NULL), m_type(0)
{
	if (!kszSource) return;
	int len = strlen(kszSource);
	if (len <= 0) return;

	m_szSource = new char[len + 1];
	strcpy(m_szSource, kszSource);
}

KPShader::~KPShader()
{
	SAFE_DEL_ARRAY(m_szSource);
}

void KPShader::setSource(const char* kszSource)
{
	if (!kszSource) return;
	int len = strlen(kszSource);
	if (len <= 0) return;

	SAFE_DEL_ARRAY(m_szSource);
	m_szSource = new char[len + 1];
	strcpy(m_szSource, kszSource);
}

void KPShader::setType(u32 type)
{
	m_type = type;
}

u32 KPShader::getType()
{
	return m_type;
}

void KPShader::clearData()
{
	SAFE_DEL_ARRAY(m_szSource);
}

KPMessage* KPShader::toMessage()
{
	KPMessage* msg = new KPMessage();
	msg->m_type = KMT_OBJECT_SHADER;
	
	const int strLen = m_szSource ? strlen(m_szSource) : 0;
	msg->m_length = 4 * 2 + strLen;
	if (msg->m_length > 0)
	{
		msg->m_pData = new char[msg->m_length];
		memcpy(msg->m_pData, &m_id, 4);
		memcpy(msg->m_pData + 4, &m_type, 4);
		memcpy(msg->m_pData + 8, m_szSource, strLen);
	}
	return msg;
}
