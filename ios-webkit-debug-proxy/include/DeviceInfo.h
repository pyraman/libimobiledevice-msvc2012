#ifndef _I_DEVICE_INFO_H_
#define _I_DEVICE_INFO_H_

#include <stdint.h>
#include <string>
#include <map>
#include "Utility.h"
#include "JSONCommon.h"
using namespace std;

#define INFO_DEL		"<>"
#define LOCAL_HOST		"127.0.0.1"
#define MANUFACTURER	"Apple"
#define PROXY_UNKNOWN			"N/A"

class DeviceInfo {
private:
	uint64_t nDeviceId;
	string szHostAddr;
	unsigned short nPort;

	string szDeviceName;
	string szDeviceSerialNo;

	string szAgentKey;

	string szManufacturer;
	string szProductModel;
	string szOS;
	string szMacAddr;
	string szProductSerialNumber;
	string szCPUUsage;
	string szMemoryUsage;
	string szModelName;
	string szUDID;
	string szDeviceInfo;
	int m_nsfd;

	void initializeDeviceTypes();
	map<string, string> m_deviceTypes;

	string m_szServingProcess;

public:
	DeviceInfo(const uint64_t & nDeviceId, 
			const char * szHostAddr,
			const unsigned short & nPort,
			const char * szDeviceName, 
			const char * szDeviceSerialNo, 
			const char * szManufacturer,
			const char * szProductModel,
			const char * szOS,
			const char * szMACAddr,
			const char * szProductSerialNumber,
			const char * szCPUUsage,
			const char * szMemoryUsage,
			int sfd			);

	uint64_t getDeviceId();
	void setDeviceId(const uint64_t & nDeviceId);

	const char* getHostAddr();
	void setHostAddr(const char* szHostAddr);

	const unsigned short getPort();
	void setPort(const unsigned short & nPort);

	const char* getDeviceName();
	void setDeviceName(const char* szDeviceName);

	const char* getDeviceSerialNo();
	void setDeviceSerialNo(const char* szDeviceName);

	const char* getAgentKey();
	void  setAgentKey(const char* szAgentKey);

	const char* getManufacturer();
	void setManufacturer(const char* szManufacturer);

	const char* getProductModel();
	void setProductModel(const char* szProductModel);
	
	const char* getOS();
	void setOS(const char* szOS);

	const char* getMacAddr();
	void setMacAdd(const char* szMacAdd);

	const char* getProductSerialNumber();
	void setProductSerialNumber(const char* szProductSerialNumber);

	const char* getCPUUsage();
	void setCPUUsage(const char* szCPUUsage);

	const char* getMemoryUsage();
	void setMemoryUsage(const char* szMemoryUsage);

	const char* getDeviceUDID();
	AbtString getDeviceInfo(const char* szSessionID);
	
	bool isDeviceBusy();
	void setServingProcess(const char* szServingProcess);
	const char* getServingProcess();

	void cleanUp();
	int getfd();
};

#endif
