#include "RegisterClientWrapper.h"
#include "Networking/TCPSocket.h"
#include "Messaging/Argument.h"
#include "AbtString.h"
#include "AbtStringList.h"
#include "AbtConvertor.h"
#include "DeviceManager.h"
#include "RSCommunication/RSCommunication.h"
#include "JSONCommon.h"
#include "AbtRegisterServerDef.h"
#include "AbtDeviceDef.h"

RegisterClientWrapper::RegisterClientWrapper(void)
{
}

RegisterClientWrapper::~RegisterClientWrapper(void)
{
}

bool RegisterClientWrapper::sendMsgtoRegisterServer(CMessage& aInMsg, CMessage& aOutMsg)
{
	CTCPSocket lTCPSocket;
	bool lRet = false;
	int lTryTime = 0;

	do 
	{
		// connect to register server
		lTCPSocket.setReceivingTimeOut(60*24);
		lTCPSocket.setSendingTimeOut(15);
		// try connect
		while(lTryTime < 3)
		{
			if(lTCPSocket.connectTo(REGISTER_SERVER_PORT, 0))
			{
				lRet = true;
				break;
			}
			
			lTryTime++;
		}

		if(lRet == false)
			break;

		// send message
		lRet = lTCPSocket.send(aInMsg, aOutMsg);

	} while (false);

	// close connection
	lTCPSocket.close();
	if(!lRet){
		DeviceManager::getInstance()->endProcess();
	}
	return lRet;
}
bool RegisterClientWrapper::getAgentPorts(const char* platform, char* listPorts)
{
	bool ret = false;
	CMessage lMessage;
	const char* actionname="get agent ports";
	const char* seqid="1";

	lMessage.addArgument(0,ARG_ACTIONNAME,actionname,strlen(actionname));
	lMessage.addArgument(0, ARG_SESSIONID, D_IOS_SESSIONID, strlen(D_IOS_SESSIONID));
	lMessage.addArgument(0, ARG_SEG, "1", strlen("1"));
	lMessage.addArgument(0, ARG_PLATFORM, platform, strlen(platform));

	CMessage recMSG ;
	if ( sendMsgtoRegisterServer(lMessage,recMSG))
	{
		ret = true ;
		CArgument arg;
		recMSG.findArgument(4,&arg);
		const char* data = arg.getData();
		if(data){
			strcpy(listPorts,data);
		}
	}

	return ret;
}
bool RegisterClientWrapper::alreadyRegisterProxy()
{
	bool ret = false;
	JSONVALUE jsPlatform;
	JSONVALUE jsEmpty = JSONVALUE(Json::objectValue);
	jsPlatform["webkit_tool"] = jsEmpty;
	AbtString szData;
	TARESULT taRes = RSCommunication::instance()->queryRecordByPath(D_IOS_SESSIONID, jsPlatform.toString(), szData);
	if(TA_SUCCEED(taRes) && !szData.empty()){
		ret = true;
	}
	return ret;
}

bool RegisterClientWrapper::registerAgent(const int nPort, AbtString& szAgentKey)
{
	return registerAgent(PLATFORM_IOS, nPort, szAgentKey);
}
bool RegisterClientWrapper::registerAgent(const char* szPlatform, const int nPort, AbtString& szAgentKey)
{
	AbtString szPort = AbtConvertor::toString(nPort);
	return RSCommunication::instance()->getNewAgentKey(D_IOS_SESSIONID, szPlatform, szPort.c_str(), szAgentKey);
}

bool RegisterClientWrapper::unregisterAgent(const char* szAgentKey)
{
	return unregisterAgent(PLATFORM_IOS, szAgentKey);
}
bool RegisterClientWrapper::unregisterAgent(const char* szPlatform, const char* szAgentKey)
{
	JSONVALUE jsEmpty = JSONVALUE(Json::objectValue);
	JSONVALUE jsAgentKey;
	JSONVALUE jsPlatform;

	jsAgentKey[szAgentKey] = jsEmpty;
	jsPlatform[szPlatform] = jsAgentKey;

	TARESULT taRes = RSCommunication::instance()->unregisterRecord(D_IOS_SESSIONID, jsPlatform.toString());
	
	return TA_SUCCEED(taRes);
}

bool RegisterClientWrapper::registerWindow(const char* szAgentKey, const char *  aDeviceUniqueId, const char* szDeviceInfo)
{
	return registerWindow(PLATFORM_IOS,szAgentKey,D_IOS_SESSIONID,aDeviceUniqueId, szDeviceInfo);

}
bool RegisterClientWrapper::registerWindow(const char* szPlatform, const char* szAgentKey, const char* sessionID, const char *  aDeviceUniqueId, const char* szDeviceInfo, int iPort)
{
	JSONVALUE jsWinKey;
	JSONVALUE jsAgentKey;
	JSONVALUE jsPlatform;
	JSONVALUE jsInfo;
	JSONVALUE jsEmpty = JSONVALUE(Json::objectValue);
	bool isParseSuccessed = false;
	
	if(szDeviceInfo != NULL && !AbtString::equalIgnoreCase("", szDeviceInfo))
	{
		JSONREADER jsReader;
		isParseSuccessed = jsReader.parse(szDeviceInfo,jsInfo, false);
	}

	if(isParseSuccessed)
	{
		jsWinKey[aDeviceUniqueId] = jsInfo;
	}else
	{
		jsWinKey[aDeviceUniqueId] = jsEmpty;
	}
	if(iPort != -1)
	{
		jsWinKey["port"] = iPort;
		jsWinKey["host"] = LOCAL_HOST_IP;
	}
	jsAgentKey[szAgentKey] = jsWinKey;
	jsPlatform[szPlatform] = jsAgentKey;
	
	TARESULT taRes = RSCommunication::instance()->registerRecord(sessionID, jsPlatform.toString());
	if(TA_SUCCEED(taRes)){
		return true;
	}
	return false;
}

bool RegisterClientWrapper::unregisterWindow(const char* szAgentKey, const char* szWinKey)
{
	return unregisterWindow(D_IOS_SESSIONID, PLATFORM_IOS, szAgentKey, szWinKey);
}
bool RegisterClientWrapper::unregisterWindow(const char* sessionID, const char* szPlatform,const char* szAgentKey, const char* szWinKey)
{
	TARESULT taRes = TA_FALSE;
	JSONVALUE jsEmpty = JSONVALUE(Json::objectValue);
	JSONVALUE jsWinKey;
	JSONVALUE jsAgentKey;
	JSONVALUE jsPlatform;

	jsWinKey[szWinKey] = jsEmpty;
	jsAgentKey[szAgentKey] = jsWinKey;
	jsPlatform[szPlatform] = jsAgentKey;

	taRes = RSCommunication::instance()->unregisterRecord(sessionID, jsPlatform.toString());
	if (TA_SUCCEED(taRes))
	{
		return true;
	}
	return false;
}


bool RegisterClientWrapper::unregisterProcess(const char* szDeviceId)
{
	bool lRet = false;
	TARESULT taRes = RSCommunication::instance()->unregisterProcess(D_IOS_SESSIONID, szDeviceId);
	if (TA_SUCCEED(taRes))
	{
		lRet = true;
	}else{
		DeviceManager::getInstance()->endProcess();
	}
	return lRet;
}
bool RegisterClientWrapper::registerProcess(const char* szDeviceId, const int & nProcessId)
{
	bool lRet = false;
	// convert port to string
	AbtString szProcessId = AbtConvertor::toString(nProcessId);

	TARESULT taRes = RSCommunication::instance()->registerProcess(D_IOS_SESSIONID, szDeviceId, szProcessId.c_str());
	if (TA_SUCCEED(taRes))
	{
		lRet = true;
	}else{
		DeviceManager::getInstance()->endProcess();
	}
	return lRet;
}
bool RegisterClientWrapper::getProcess(const char* szDeviceId, char* szBuffer, const int & nMaxLength)
{
	bool lRet = false;
	AbtString szProcess;

	TARESULT taRes = RSCommunication::instance()->getProcess(D_IOS_SESSIONID, szDeviceId, szProcess);
	if (TA_SUCCEED(taRes))
	{
		if(!szProcess.empty()){
			szProcess.copy(szBuffer, nMaxLength);
		}
		lRet = true;
	}else{
		DeviceManager::getInstance()->endProcess();
	}
	return lRet;


}

TARESULT RegisterClientWrapper::detectAgentFromRegisterServerByDeviceID(const char* szPlatform, const char* szSessionID, AbtAgentInfoList* aAgentInfoList, 
														  const char* aDeviceUniqueId)
{
	TARESULT taRes = TA_FALSE;
	do 
	{

		AbtString lStrData;
		taRes = requestPlatformAgents(szPlatform,szSessionID,  lStrData);
		if (TA_FAILED(taRes))
		{
			break;
		}
		if (!lStrData.empty())
		{
			// list agent info
			Json::Reader jsReader;
			Json::Value jsListAgent;
			bool isParseSuccessed = jsReader.parse(lStrData.c_str(),jsListAgent, false);
			if (!isParseSuccessed)
			{
				taRes = TA_FALSE;
				break;
			}
			// loop list agent info
			for (JSONITERATOR iter = jsListAgent.begin(); iter != jsListAgent.end(); iter++)
			{
				AbtString sAgent((*iter).toString());
				// check contain aDeviceUniqueId
				if(sAgent.contain(aDeviceUniqueId))
				{
					AgentInfo* lAgentInfo = new AgentInfo();
					lAgentInfo->parseAgentInfo(iter);
					aAgentInfoList->add(lAgentInfo);
				}
			}

		}
	} while (false);
	return taRes;
}

TARESULT RegisterClientWrapper::requestPlatformAgents(const char* szPlatform, const char* szSessionID, AbtString& lStrData)
{
	JSONVALUE jsPlatform;
	JSONVALUE jsEmpty = JSONVALUE(Json::objectValue);
	jsPlatform[szPlatform] = jsEmpty;

	// get list agent info
	RSCommunication *rsCom = RSCommunication::instance();
	return rsCom->queryRecordByPath(szSessionID, jsPlatform.toString(), lStrData);
}

TARESULT RegisterClientWrapper::detectAgentFromRegisterServer(const char* szPlatform, const char* szSessionID, AbtAgentInfoList* aAgentInfoList)
{
		TARESULT taRes = TA_FALSE;

	do 
	{
		AbtString lStrData;
		taRes = requestPlatformAgents(szPlatform,szSessionID, lStrData);
		if (TA_FAILED(taRes))
		{
			break;
		}
		if (!lStrData.empty())
		{
			// list agent info
			Json::Reader jsReader;
			Json::Value jsListAgent;
			bool isParseSuccessed = jsReader.parse(lStrData.c_str(),jsListAgent, false);
			if (!isParseSuccessed)
			{
				taRes = TA_FALSE;
				break;
			}
			parseAgent(szPlatform, szSessionID, jsListAgent, aAgentInfoList);
		}
	} while (false);
	taRes  = aAgentInfoList->size();
	return taRes;
}
void RegisterClientWrapper::parseAgent(const char* szPlatform,const char* szSessionID, JSONVALUE jsListAgent, AbtAgentInfoList* tempListDeviceID)
{
	// loop list agent info
	for (JSONITERATOR iter = jsListAgent.begin(); iter != jsListAgent.end(); iter++)
	{
		AgentInfo* lAgentInfo = new AgentInfo();
		TARESULT res = lAgentInfo->parseAgentInfo(iter);
		if (TA_SUCCEED(res))
		{
			if (TA_SUCCEED(lAgentInfo->isValid()))
			{
				// add to list device if the agent information is valid
				tempListDeviceID->push_back(lAgentInfo);
			}else
			{
				// unregister agent
				unregisterAgent(szPlatform, lAgentInfo->getAgentKey());
				DELETE_POINTER(lAgentInfo);
			}
		}else{
			DELETE_POINTER(lAgentInfo);
		}

	}
}