#include "KPServer.h"
#include "KPHelper.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>

#include <pthread.h>

#define PORT 38300
#define BACKLOG 5

KPServer kpServer;
void KPInitHandler_libGLESv2();

#define REQUEST_CODE_CAPTURE			0
#define REQUEST_CODE_MODIFY_PROGRAM		1

#define ERROR_BUSY			"Server is busy now, please try again later."

void* KPRunServer(void*)
{
	LOGD("---------- KPRunServer begin ----------");
	
	KPHelper::initMutexes();

	KPInitHandler_libGLESv2();

	sockaddr_in serverAddr;
	int listenFd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(PORT);

	bind(listenFd, (sockaddr*)&serverAddr, sizeof(serverAddr));

	listen(listenFd, BACKLOG);

	while (1)
	{
		LOGD("---------- Now listening...");
		int client = accept(listenFd, (sockaddr*)NULL, NULL);

		LOGD("---------- Accept client = %d", client);
		if (kpServer.getState() != IDLE)
		{
			KPHelper::sendRequestErrorMessageToClient(client, ERROR_BUSY);
			KPHelper::closeSocket(client);
			continue;
		}

		KPHelper::setSocketRecvTimeout(client, 10000);

		int requestCode = 0;
		if (!KPHelper::recvSocket(client, &requestCode, 4))
		{
			KPHelper::closeSocket(client);
			continue;
		}

		switch (requestCode)
		{
			case REQUEST_CODE_CAPTURE:
			{
				LOGD("---------- requestCode == REQUEST_CODE_CAPTURE");
				kpServer.setClient(client);
				kpServer.setState(PRE_CAPTURE);
				break;
			}
			case REQUEST_CODE_MODIFY_PROGRAM:
			{
				LOGD("---------- requestCode == REQUEST_CODE_MODIFY_PROGRAM");
				kpServer.setClient(client);
				kpServer.setState(MODIFYING_PROGRAM);
				break;
			}
		}
		
	} // while (1)

	KPHelper::closeSocket(listenFd);

	KPHelper::destroyMutexes();
	LOGD("---------- KPRunServer end ----------");
}

void KPSwapBuffers()
{
	kpServer.onSwapBuffers();
}

static pthread_t callThd;

class KPDummy
{
public:
	KPDummy()
	{
		pthread_create(&callThd, NULL, KPRunServer, NULL);
	}
	~KPDummy()
	{
		LOGD("---------- ~KPDummy()");
	}
};

KPDummy kpDummy;

extern "C" void C_KPSwapBuffers()
{
	KPSwapBuffers();
}
