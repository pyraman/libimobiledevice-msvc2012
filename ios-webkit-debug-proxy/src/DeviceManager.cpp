#include "DeviceManager.h"
#include "AbtString.h"
#include "AbtConvertor.h"
#include "Utility.h"
#include <time.h>

#include "AbtDeviceDef.h"
#include "AbtRegisterServerDef.h"
#include "AgentInfo.h"

DeviceManager * DeviceManager::instance = NULL;

DeviceManager::DeviceManager()
{
	m_devices = new vector<DeviceInfo *>();
	m_RegisterClient = new RegisterClientWrapper();
	m_bRegistered=false;
	m_bhasError=false;

	m_proxyinfo.fd_AppleMobileService=-1;
	m_proxyinfo.fd_ProxyListen=-1;
}

DeviceManager::~DeviceManager()
{
	if(m_devices) {
		delete (m_devices) ;
		m_devices=0;
	}

	if(m_RegisterClient) {
		delete (m_RegisterClient) ;
		m_RegisterClient=0;
	}

}

DeviceManager * DeviceManager::getInstance()
{
	if (instance == NULL)
	{
		instance = new DeviceManager();
	}
	return instance;
}

void DeviceManager::addDevice(DeviceInfo * device)
{
	m_devices->push_back(device);
	this->restoreData(device);
	this->registerDevice(device);
}

bool DeviceManager::isPortReady(int nPort)
{
	return false;
}

/*
* try to get port ready
*/
int DeviceManager::getPortReady(int startPort, int endPort)
{
	for (int nPort = startPort; nPort < endPort; nPort++)
	{
		if(isPortReady(nPort))
		{
			return nPort;
		}
	}
    return -1;
}

void DeviceManager::removeDevice(const uint64_t nDeviceId)
{
	DeviceInfo * deviceInfo = NULL;
	vector<DeviceInfo *>::iterator it = m_devices->begin();
	while (it != m_devices->end())
	{
		deviceInfo = *it;
		if (deviceInfo->getDeviceId() == nDeviceId)
		{
			break;
		}

		it++;
	}

	if (it != m_devices->end())
	{
		this->unregisterDevice(deviceInfo);
		m_devices->erase(it);
		delete deviceInfo;
	}
}

/*
* get device information with sfd (socket number) information
*/
DeviceInfo* DeviceManager::getDevice(int sfd)
{
	vector<DeviceInfo *>::iterator it = m_devices->begin();
	DeviceInfo * deviceInfo = NULL;
	while (it != m_devices->end())
	{
		if ((*it)->getfd()== sfd)
		{
			deviceInfo=(*it);
			break;
		}

		it++;
	}

	return deviceInfo;
}


DeviceInfo* DeviceManager::getDevice(const char* szDeviceId)
{
	vector<DeviceInfo *>::iterator it = m_devices->begin();
	DeviceInfo * deviceInfo = NULL;
	while (it != m_devices->end())
	{
		deviceInfo = *it;

		string szCurrentDeviceUDID = deviceInfo->getDeviceUDID();

		if (strcmp(szCurrentDeviceUDID.c_str(), szDeviceId) == 0)
		{
			break;
		}

		it++;
	}

	if (it == m_devices->end())
	{
		return NULL;
	}

	return deviceInfo;
}

void DeviceManager::cleanUp(const char* resource)
{
	//printf("[DeviceManager][CleanUp] resource = %s\n", resource);
	string szResource = resource;
	vector<string> resourceParams = Utility::split(szResource, "/");
	if (resourceParams.size() < 2)
	{
		return;
	}

	string szDeviceId = resourceParams[1];
	
	unregisterProcess(szDeviceId.c_str());

	DeviceInfo * device = getDevice(szDeviceId.c_str());
	if (device == NULL)
	{
		return ;
	}

	device->cleanUp();

}


/*
* check if the device is busy
*/

bool DeviceManager::isDeviceBusy(const char* resource)
{
	//printf("[DeviceManager][isDeviceBusy] resource = %s\n", resource);
	string szResource = resource;
	vector<string> resourceParams = Utility::split(szResource, "/");

	if (resourceParams.size() < 4)
	{
		return false;
	}

	string szDeviceId = resourceParams[1];
	string szProduct = resourceParams[2];
	string szProcessId = resourceParams[3];

	// if the current product is Automation, set data and return false;
	if (strcmp(szProduct.c_str(), TA_PRODUCT_AUTOMATION) == 0)
	{
		DeviceInfo * device = getDevice(szDeviceId.c_str());
		if (device == NULL)
		{
			return false;
		}

		// if there are 2 bat files running at the same time, the second one should be prevent due to device is busy (serving first bat file)
		if (device->isDeviceBusy() == true)
		{
			return true;
		}
		device->setServingProcess(szProcessId.c_str());

		return false;
	}

	// if the automation product is viewer, check if the device is busy.
	// if the device is busy but the serving process is no long alive, then the device is not busy.
	else if (strcmp(szProduct.c_str(), TA_PRODUCT_VIEWER) == 0)
	{
		DeviceInfo * device = getDevice(szDeviceId.c_str());
		if (device == NULL)
		{
			return false;
		}

		if (device->isDeviceBusy() == false)
		{
			printf("[DeviceManager][isDeviceBusy] not busy 1\n");
			return false;
		}

		const char* szServingProcess = device->getServingProcess();
		long nProcessId = atoi(szServingProcess);

		if (Utility::isProcessRunning(nProcessId))
		{
			printf("[DeviceManager][isDeviceBusy] busy\n");
			return true;
		}

		printf("[DeviceManager][isDeviceBusy] not busy 2\n");
		return false;

	}
    return false;
}

/*
* before kill TAWebkitProxy, we need to back up all information about serving process of each device.
*/
void DeviceManager::backupData()
{
	printf("[DeviceManager][backupData] BEGIN\n");
	vector<DeviceInfo *>::iterator it = m_devices->begin();
	
	for(; it != m_devices->end(); it++)
	{
		DeviceInfo * device = *it;
		const char* szServingProcess = device->getServingProcess();
		if (strcmp(szServingProcess, "") == 0)
		{
			continue;
		}

		int nProcessId = atoi(szServingProcess);
		registerProcess(device->getDeviceName(), nProcessId);
	}

	printf("[DeviceManager][backupData] END\n");
}

void DeviceManager::restoreData(DeviceInfo * device)
{
	printf("[DeviceManager][restoreData] BEGIN\n");
	const int nMaxLength = 1024;
	char szServingProcess[nMaxLength] = {0};

	int len = getProcess(device->getDeviceName(), szServingProcess, nMaxLength);
	if (len > 0)
	{
		device->setServingProcess(szServingProcess);
	}
	printf("[DeviceManager][restoreData] END\n");
}

void DeviceManager::clearAll()
{
	vector<DeviceInfo *>::iterator it = m_devices->begin();
	for(;it!=m_devices->end();it++)
	{
		unregisterDevice(*it);
	}

	m_devices->clear();
	
}

/*
* register device to Register Server
*/

void DeviceManager::registerDevice(DeviceInfo * device)
{
	if(!m_RegisterClient->alreadyRegisterProxy()){
		DeviceManager::getInstance()->endProcess();
	}

	int nPort = device->getPort();
	AbtString agentKey = getAgentKeyByPortandDeviceID(device->getDeviceUDID(), nPort);

	if(agentKey.empty())
	{
		m_RegisterClient->registerAgent(nPort, agentKey);
	}
	device->setAgentKey(agentKey.c_str());

	long nCurrentTime = time(NULL) * 1000;

	string szSessionId = AbtConvertor::toString(nCurrentTime).get();
	AbtString jsDeviceInfo = device->getDeviceInfo(szSessionId.c_str());

	m_RegisterClient->registerWindow(agentKey.c_str(), device->getDeviceUDID(), jsDeviceInfo.c_str());
}
AbtString DeviceManager::getAgentKeyByPortandDeviceID(const char* deviceID, int iPort)
{
	AbtAgentInfoList agentList;
	AbtString agentKey;
	m_RegisterClient->detectAgentFromRegisterServer(PLATFORM_IOS, D_IOS_SESSIONID, &agentList);
	if (agentList.size() > 0)
	{
		agentList.startCycle();
		while (!agentList.endCycle())
		{
			AgentInfo* info = agentList.nextCycle();
			if (AbtString::equalIgnoreCase(LOCAL_HOST_IP, info->getHost())
				|| AbtString::equalIgnoreCase(LOCAL_HOST_DOMAIN, info->getHost()))
			{
				if(info->getPort() == iPort)
				{
					WindowInfo* winInfo = info->getLastWindowInfo();
					if (winInfo == NULL)
					{
						continue;
					}
					if (AbtString::equal(winInfo->getUniqueKey(), deviceID))
					{
						agentKey = info->getAgentKey();
					}else
					{
						m_RegisterClient->unregisterAgent( info->getAgentKey());
					}
				}
			}
		}
		clear(&agentList);
	}
	return agentKey;
}
void DeviceManager::unregisterDevice(DeviceInfo * device)
{
	if(m_RegisterClient->alreadyRegisterProxy()){
		m_RegisterClient->unregisterAgent(device->getAgentKey());
	}
	else{
		DeviceManager::getInstance()->endProcess();
	}
}

//void DeviceManager::registerWindow(const char* szWinkey, const char* szSignature, const int & nPort)
//{
//	m_RegisterClient->registerWindow(szWinkey, szSignature, nPort);
//}

//void DeviceManager::unregisterWindow(const char* szWinkey, const int & nPort)
//{
//	m_RegisterClient->unregisterWindow(szWinkey, "", nPort);
//}

void DeviceManager::registerProcess(const char* szDeviceId, const int & nProcessId)
{
	m_RegisterClient->registerProcess(szDeviceId, nProcessId);
}

void DeviceManager::unregisterProcess(const char* szDeviceId)
{
	m_RegisterClient->unregisterProcess(szDeviceId);
}
int DeviceManager::getProcess(const char* szDeviceId, char* szBuffer, const int & nMaxLength)
{
	m_RegisterClient->getProcess(szDeviceId, szBuffer, nMaxLength);
	return strlen(szBuffer);
}
/*
* initialize proxy 
*/
bool DeviceManager::initialize(int port)
{
	char buffer[256] = { 0 } ;
#ifdef WIN32
	DWORD processID = GetCurrentProcessId();
#elif defined(_UNIX) || defined(_MACOS)
	pid_t processID = getpid();
#endif
	sprintf(buffer, "%d", processID);
    printf("[DeviceManager][initialize] port = %d\n", port);
	AbtString szAgentKey(WEBKIT_AGENT_KEY);
	JSONVALUE jsProperties;
	jsProperties[ARG_SIGNATURE] = buffer;
	m_bRegistered = m_RegisterClient->registerWindow(TOOL_PLATFORM, szAgentKey.c_str(),D_IOS_SESSIONID, TOOL_NAME, jsProperties.toString(), port); 
	if(m_bRegistered)
	{
		m_proxyPort=port;
	}
	return m_bRegistered;
	
}

/*
* uninitialize proxy 
*/
void DeviceManager::uninitialize(int port)
{
	if(m_bRegistered){

		clearAll();
		m_RegisterClient->unregisterAgent(TOOL_PLATFORM, szAgentKey.c_str());
	}
}

/*
* notify when a fd number of server socket was removed (closed)
*/
void DeviceManager::notifySocketRemove(int fd){
	 fprintf(stderr, " DeviceManager::notifySocketRemove: %d\n",fd);
	if(fd== m_proxyinfo.fd_AppleMobileService){
		notifyError();

		 fprintf(stderr, " DeviceManager::notifySocketRemove[AppleMobileService]: %d\n",fd);
	}
	else if(fd== m_proxyinfo.fd_ProxyListen){
		notifyError();
	}
	else{
		//at this case, there is a
        DeviceInfo* devi = getDevice(fd);
        if (devi != NULL) {
            if (strcmp(devi->getDeviceName(),"simulator") == 0) {
                removeDevice(devi->getDeviceId());
            }
            else
                notifyError();
        }
	}
}

void DeviceManager::notifyError()
{
		m_bhasError=true;
}

ProxyInfor* DeviceManager::getProxyInfo(){
		return &m_proxyinfo;
	};


void DeviceManager::endProcess(){
#ifdef WIN32
	::TerminateProcess( ::GetCurrentProcess(),0  );
#else
	exit(0);
#endif
}