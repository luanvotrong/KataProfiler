#ifndef _KP_SHADER_H_
#define _KP_SHADER_H_

#include "KPGLObject.h"

class KPShader : public KPGLObject
{
public:
	KPShader() : KPGLObject(), m_szSource(NULL), m_type(0) {}

	KPShader(u32 id) : KPGLObject(id), m_szSource(NULL), m_type(0) {}

	KPShader(u32 id, const char* kszSource);

	~KPShader();

	//

	void setSource(const char* kszSource);
	void setType(u32 type);
	u32 getType();
	
	void clearData();
	KPMessage* toMessage();

protected:
	char* m_szSource;
	u32 m_type;
};

#endif
