#ifndef _KP_GL_OBJECT_H_
#define _KP_GL_OBJECT_H_

#include "kp_macro.h"
#include "KPMessage.h"


class KPGLObject
{
public:
	KPGLObject() : m_id(0) {}
	KPGLObject(u32 id) : m_id(id) {}

	virtual ~KPGLObject() {}

	u32 getId() { return m_id; }
	void setId(u32 id) { m_id = id; }

	virtual void clearData() = 0;
	virtual KPMessage* toMessage() = 0;
	
protected:
	u32 m_id;
};

#endif
