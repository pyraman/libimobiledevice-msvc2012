
#ifndef REGISTERCLIENTWRAPPER_IOS_H_
#define REGISTERCLIENTWRAPPER_IOS_H_

#include "Messaging/Message.h"
#include "AgentInfo.h"
#include "AbtString.h"
#include <vector>

using namespace std;


class RegisterClientWrapper
{
public:
	RegisterClientWrapper(void);
	~RegisterClientWrapper(void);

	bool sendMsgtoRegisterServer(CMessage& aInMsg, CMessage& aOutMsg);
	bool getAgentPorts(const char* platform, char* listPorts);
	bool alreadyRegisterProxy();

	bool registerAgent(const int nPort, AbtString& szAgentKey);
	bool registerAgent(const char* szplatform, const int nPort, AbtString& szAgentKey);

	bool unregisterAgent( const char* szAgentKey);
	bool unregisterAgent(const char* szplatform, const char* szAgentKey);

	bool registerWindow(const char* szAgentKey, const char *  aDeviceUniqueId, const char* szDeviceInfo);
	bool registerWindow(const char* szplatform, const char* szAgentKey, const char* sessionID, const char *  aDeviceUniqueId, const char* szDeviceInfo, int iPort = -1);

	bool unregisterWindow(const char* szAgentKey, const char* szWinKey);
	bool unregisterWindow(const char* sessionID, const char* szplatform,const char* szAgentKey,const char* szWinKey);

	bool registerProcess(const char* szDeviceId, const int & nProcessId);
	bool unregisterProcess(const char* szDeviceId);
	bool getProcess(const char* szDeviceId, char* szBuffer, const int & nMaxLength);

	TARESULT detectAgentFromRegisterServerByDeviceID(const char* szPlatform, const char* szSessionID, AbtAgentInfoList* aAgentInfoList, const char* aDeviceUniqueId);
	TARESULT requestPlatformAgents(const char* szPlatform,const char* szSessionID, AbtString& lStrData);
	TARESULT detectAgentFromRegisterServer(const char* szPlatform, const char* szSessionID, AbtAgentInfoList* aAgentInfoList);
	void parseAgent(const char* szPlatform, const char* szSessionID, JSONVALUE jsListAgent, AbtAgentInfoList* tempListDeviceID);
};

#endif /* REGISTERCLIENTWRAPPER_IOS_H_ */


