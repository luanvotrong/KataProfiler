#ifndef _KP_VBO_H_
#define _KP_VBO_H_

#include "KPGLObject.h"

class KPVbo : public KPGLObject
{
public:
	KPVbo();
	KPVbo(u32 id);

	virtual ~KPVbo();

	//
	void clearData();
	KPMessage* toMessage();

	//
	void on_glBufferData(int size,  const void * data,  u32 usage);
	void on_glBufferSubData(int offset, int size, const void* data);

	void on_glMapBufferOES (GLenum access, void* ptr);
	void on_glUnmapBufferOES();

	//
	int getSize();
	char* getData();
	u32 getUsage();

private:
	int		m_size;
	char*	m_pData;
	u32		m_usage;
};

#endif
