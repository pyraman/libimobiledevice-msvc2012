#include "DeviceInfo.h"
#include "AbtString.h"
#include "AbtUtilities.h"
#include "AbtStringList.h"
#include "JSONCommon.h"
#include "AbtDeviceDef.h"

#define SEPARATOR_CHAR			"="


void DeviceInfo::initializeDeviceTypes()
{
	char szInstallDir [ MAX_LENGTH+1 ] = { 0 } ;
	AbtUtilities::getInstallDirPath( szInstallDir , MAX_LENGTH ) ;
	string initFilePath = string(szInstallDir) + "/binclient/data/configuration/iOSDeviceList.inf";

	TARESULT nLength = (int)AbtUtilities::getFileSize(initFilePath.c_str()) ;
	// Can not open this file
	if(TA_SUCCEED(nLength)) 
	{
		char* szText = new char[nLength + 2];

		TARESULT ret = AbtUtilities::readFile(initFilePath.c_str(), szText, nLength + 1);
		if (TA_SUCCEED(ret))
		{
			AbtStringList vText;

			AbtString::split(szText, "\r\n", vText);
			DELETE_VECTOR(szText);

			for (int i = 0; i < vText.size();i++)
			{
				string item = vText[i];
				AbtStringList vItem;

				AbtString::split(item.c_str(), SEPARATOR_CHAR, vItem);
				if (vItem.size() == 2)
				{
					AbtString productModel = vItem.getAt(0);
					AbtString modelName = vItem.getAt(1);
					m_deviceTypes.insert(pair<string, string>(productModel.c_str(), modelName.c_str()));
				}
			}	
		}
		else
		{
			DELETE_VECTOR(szText);
		}		
	}
}


DeviceInfo::DeviceInfo(const uint64_t & nDeviceId, 
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
					   const char * szMemoryUsager,
					   int sfd			)
{
	initializeDeviceTypes();
	this->nDeviceId = nDeviceId;
	this->szHostAddr = szHostAddr;
	this->nPort = nPort;

	this->szDeviceName = szDeviceName;
	this->szDeviceSerialNo = szDeviceSerialNo;
	this->szManufacturer = MANUFACTURER;
	this->szProductModel = szProductModel;
	this->szOS = szOS;
	this->szMacAddr = szMACAddr;
	this->szProductSerialNumber = szProductSerialNumber;
	this->szCPUUsage = szCPUUsage;
	this->szMemoryUsage = szMemoryUsager;
	this->m_nsfd=sfd;

	map<string, string>::iterator it = m_deviceTypes.find(this->szProductModel);

	if (it != m_deviceTypes.end())
	{
		this->szModelName = string(it->second);
	}
	else
	{
		this->szModelName = this->szProductModel;
	}
	this->szUDID = this->szDeviceName + "_" + this->szModelName;
	this->szDeviceInfo = "";
}

uint64_t DeviceInfo::getDeviceId()
{
	return this->nDeviceId;
}
void DeviceInfo::setDeviceId(const uint64_t & nDeviceId)
{
	this->nDeviceId = nDeviceId;
}

const char* DeviceInfo::getHostAddr()
{
	return szHostAddr.c_str();
}
void DeviceInfo::setHostAddr(const char* szHostAddr)
{
	this->szHostAddr = szHostAddr;
}

const unsigned short DeviceInfo::getPort()
{
	return this->nPort;
}
void DeviceInfo::setPort(const unsigned short & nPort)
{
	this->nPort = nPort;
}

const char* DeviceInfo::getDeviceName()
{
	return szDeviceName.c_str();
}
void DeviceInfo::setDeviceName(const char* szDeviceName)
{
	this->szDeviceName = szDeviceName;
}

const char* DeviceInfo::getDeviceSerialNo()
{
	return szDeviceSerialNo.c_str();
}
void DeviceInfo::setDeviceSerialNo(const char* szDeviceSerialNo)
{
	this->szDeviceSerialNo = szDeviceSerialNo;
}

const char* DeviceInfo::getAgentKey()
{
	return szAgentKey.c_str();
}
void DeviceInfo::setAgentKey(const char* szAgentKey)
{
	this->szAgentKey = szAgentKey;
}

const char* DeviceInfo::getManufacturer()
{
	return szManufacturer.c_str();
}
void DeviceInfo::setManufacturer(const char* szManufacturer)
{
	this->szManufacturer = szManufacturer;
}

const char* DeviceInfo::getProductModel()
{
	return szProductModel.c_str();
}
void DeviceInfo::setProductModel(const char* szProductModel)
{
	this->szProductModel = szProductModel;
}

const char* DeviceInfo::getOS()
{
	return szOS.c_str();
}
void DeviceInfo::setOS(const char* szOS)
{
	this->szOS = szOS;
}

const char* DeviceInfo::getMacAddr()
{
	return szMacAddr.c_str();
}
void DeviceInfo::setMacAdd(const char* szMacAdd)
{
	this->szMacAddr = szMacAddr;
}

const char* DeviceInfo::getProductSerialNumber()
{
	return szProductSerialNumber.c_str();
}
void DeviceInfo::setProductSerialNumber(const char* szProductSerialNumber)
{
	this->szProductSerialNumber = szProductSerialNumber;
}

const char* DeviceInfo::getCPUUsage()
{
	return szCPUUsage.c_str();
}
void DeviceInfo::setCPUUsage(const char* szCPUUsage)
{
	this->szCPUUsage = szCPUUsage;
}

const char* DeviceInfo::getMemoryUsage()
{
	return szMemoryUsage.c_str();
}
void DeviceInfo::setMemoryUsage(const char* szMemoryUsage)
{
	this->szMemoryUsage = szMemoryUsage;
}

const char* DeviceInfo::getDeviceUDID()
{
	return szUDID.c_str();
}

int DeviceInfo::getfd()
{
	return this->m_nsfd;
}

AbtString DeviceInfo::getDeviceInfo(const char* szSessionID)
{
	JSONVALUE jsEmpty = JSONVALUE(Json::objectValue);    
    JSONVALUE deviceinfo;
	AbtString szIOS = AbtString::formatString("iOS %s", this->szOS.c_str());
    
    deviceinfo["ro.product.manufacturer"] = this->szManufacturer.c_str();
	deviceinfo["ro.os"] = szIOS.c_str();
	deviceinfo["ro.product.model"] = szModelName.c_str();
	deviceinfo["ro.serialno"] = this->szProductSerialNumber.c_str();
	deviceinfo["macaddr"] = this->szMacAddr.c_str();
	deviceinfo["cpuusage"] = this->szCPUUsage.c_str();
	deviceinfo["memoryusage"] = this->szMemoryUsage.c_str();
	deviceinfo["ro.name"] = this->szDeviceName.c_str();
	deviceinfo[N_SESSIONID_ID] = szSessionID;

	AbtString strDeviceInfo(deviceinfo.toString());
	return strDeviceInfo;
}


bool DeviceInfo::isDeviceBusy()
{
	if(strcmp(m_szServingProcess.c_str(), "") == 0)
	{
		return false;
	}

	long nServingProcessId = atoi(m_szServingProcess.c_str());
	if(Utility::isProcessRunning(nServingProcessId) == false)
	{
		return false;
	}

	return true;
}

void DeviceInfo::setServingProcess(const char* szServingProcess)
{
	m_szServingProcess = szServingProcess;
}

const char* DeviceInfo::getServingProcess()
{
	return m_szServingProcess.c_str();
}

void DeviceInfo::cleanUp()
{
	m_szServingProcess = "";
}