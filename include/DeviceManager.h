#ifndef _DEVICE_MANAGER_H_
#define _DEVICE_MANAGER_H_

#include "DeviceInfo.h"
#include "RegisterClientWrapper.h"
#ifdef WIN32
#include <Windows.h>
#endif

#define REQUEST_IS_DEVICE_BUSY	"/isDeviceBusy"
#define REQUEST_CLEANUP_DEVICE	"/cleanupdevicedata"
#define	TA_PRODUCT_VIEWER			"taviewer"
#define	TA_PRODUCT_AUTOMATION		"automation"

#include <string>
#include <vector>
using namespace std;

struct ProxyInfor
{
	int portListener;
	int fd_AppleMobileService;
	int fd_ProxyListen;
};


class DeviceManager {
private:
	vector<DeviceInfo *> * m_devices;
	RegisterClientWrapper * m_RegisterClient;
	static DeviceManager * instance;
	DeviceManager();
	bool m_bRegistered;
	int m_proxyPort;

	bool m_bhasError;
	
	ProxyInfor m_proxyinfo;

	bool isPortReady(int nPort);

	AbtString szAgentKey;

	AbtString getAgentKeyByPortandDeviceID(const char* deviceID, int iPort);

public:

	~DeviceManager();

	static DeviceManager* getInstance();

	/*
	* try to get port ready
	*/
	int getPortReady(int startPort, int endPort);

	ProxyInfor* getProxyInfo();

	void addDevice(DeviceInfo * device);
	void removeDevice(const uint64_t nDeviceId);
	DeviceInfo* getDevice(const char* szDeviceId);
	bool isDeviceBusy(const char* resource);
	void cleanUp(const char* resource);
	void clearAll();

	void registerDevice(DeviceInfo * device);
	void unregisterDevice(DeviceInfo * device);
	//void registerWindow(const char* szWinkey, const char* szSignature, const int & nPort);
	//void unregisterWindow(const char* szWinkey, const int & nPort);

	void registerProcess(const char* szDeviceId, const int & nProcessId);
	void unregisterProcess(const char* szDeviceId);
	int getProcess(const char* szDeviceId, char* szBuffer, const int & nMaxLength);

	void backupData();
	void restoreData(DeviceInfo * device);

	/*
	* get device information with sfd (socket number) information
	*/
	DeviceInfo* getDevice(int sfd);

	/*
	* check the proxy has error
	*/
	bool hasError(){return m_bhasError;};

	void notifyError();

	void notifySocketRemove(int fd);
	

	/*
	* initialize proxy 
	*/
	bool initialize(int port);

	/*
	* uninitialize proxy 
	*/
	void uninitialize(int port);
	void uninitialize()
	{
		uninitialize(-1);
	}


	void endProcess();

};

#endif