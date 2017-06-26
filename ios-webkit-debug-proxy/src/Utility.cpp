#include "Utility.h"
#include <algorithm>
#include <sstream>
#include <string>
#include <AbtString.h>
#include <AbtStringList.h>

void Utility::tolower(std::string& szInputString)
{
	transform(szInputString.begin(), szInputString.end(), szInputString.begin(), ::tolower);
}

int Utility::ishex(int x)
{
	return	(x >= '0' && x <= '9')	||
		(x >= 'a' && x <= 'f')	||
		(x >= 'A' && x <= 'F');
}

bool Utility::compare(const string & s1, const string & s2)
{
	return strcmp(s1.c_str(), s2.c_str()) != 0;
}

vector<string> & Utility::split(const string & szInputString, const string & szDelim, vector<string> &elems) {
	size_t pos = 0;
	string token;
	string szInput = szInputString;
	while ((pos = szInput.find(szDelim)) != std::string::npos) {
		token = szInput.substr(0, pos);
		
		if (token.length() > 0)
		{
			elems.push_back(token);
		}

		szInput.erase(0, pos + szDelim.length());
	}
	if (szInput.length() > 0)
	{
		elems.push_back(szInput);
	}

	return elems;
}

vector<string> Utility::split(const string & szInputString, const string & szDelim) {
	vector<string> elems;
	split(szInputString, szDelim, elems);
	return elems;
}


bool Utility::isProcessRunning(const long & nProcessId)
{
	TARESULT result = TA_FALSE;
#ifdef WIN32
	DWORD processIds[10*MAX_LENGTH] = {0};
	DWORD pBytesReturned;
	if (EnumProcesses(processIds, sizeof(processIds), &pBytesReturned) == 0)
	{
		return false;
	}

	int nProcessCount = pBytesReturned/sizeof(DWORD);

	for (int nProcessIndex = 0; nProcessIndex < nProcessCount; nProcessIndex++)
	{
		long nCurrentProcess = processIds[nProcessIndex];
		if (nCurrentProcess == nProcessId)
		{
			result = TA_OK;
			break;
		}
	}

#else
	pid_t pid = (pid_t)nProcessId;
	waitpid(pid, &result, WNOHANG);

	if (kill(pid, 0) != -1) {
		result = TA_OK;
	}
#endif

	return TA_SUCCEED(result);
}

int Utility::decode(const char *s, char *dec)
{
	char *o;
	const char *end = s + strlen(s);
	int c;

	for (o = dec; s <= end; o++) {
		c = *s++;
		if (c == '+') c = ' ';
		else if (c == '%' && (	!ishex(*s++)	||
			!ishex(*s++)	||
			!sscanf(s - 2, "%2x", &c)))
			return -1;

		if (dec) *o = c;
	}

	return o - dec;
}

/*
 * execute command line and get out put
 * @param szCmd	: command line
 * @param szOut	: out put of command line
 * @return		: TA_OK if succeeded, otherwise return TA_FALSE
 */

TARESULT Utility::executeCommandLine( const char*& szCmd, AbtString& szOut)
{
    szOut = "";
    TARESULT res = TA_FALSE;
    FILE *pfd;
    
#ifdef WIN32
    pfd = _popen( szCmd, "r" );
#else
    pfd = popen( szCmd, "r" );
#endif
    
    if ( pfd == 0 )
    {
        return res;
    }
    else
    {
        while ( !feof(pfd) )
        {
            char buf[1024] = {0};
            
            if ( fgets(buf, sizeof(buf), pfd) > 0 )
            {
                szOut.append(buf);
            }
        }
        
        if(pfd != NULL) {
#ifdef WIN32
            _pclose( pfd );
#else
            pclose( pfd );
#endif
        }
        
        res = TA_OK;
    }

    return res;
}

/*
 * get iOS Simulator Information by command line
 @command line: xcrun simctl list devices | egrep 'Booted|iOS'
 * if there have any simulator is running, result will have more values
 * Below is sample result value of above command line:
-- iOS 9.0 --
 iPhone 6 (654E6222-A533-4E59-A565-E8E31F43CDE5) (Booted)
-- iOS 8.4 --
-- Unavailable: com.apple.CoreSimulator.SimRuntime.iOS-8-1 --
 * Function will help execute above command line and parse result to get information for simulator
 */
TARESULT Utility::getSimulatorInformationByCommandLine( const char* device_id, char** device_name, char** product_model, char** ios_version, char** szMacAddr, char** szSerialNo)
{
    //command line
    const char* command = "xcrun simctl list devices | egrep 'Booted|iOS'";
    AbtString szOut("");
    TARESULT res = Utility::executeCommandLine(command, szOut);
    if(TA_SUCCEED(res))
    {
        //parse out put to get information
        if(szOut.length() > 0)
        {
            //find Booted character to ensure that have a booted device
            std::size_t pos = szOut.find("Booted");
            if(pos != std::string::npos)
            {
                //have booted device, continue to get information
                AbtStringList vText;
                string previousLine = "", currentLine = "";
                szOut.split("\n", vText);
                
                for (int i = 0; i < vText.size();i++)
                {
                    string currentLine = vText[i];
                    pos = currentLine.find("Booted");
                    
                    if(pos != std::string::npos)
                    {
                        //parse data
                        AbtStringList vItem;
                        AbtString::split(currentLine.c_str(), "(", vItem);
                        if (vItem.size() == 3)
                        {
                            //get device name
                            AbtString productModelValue(vItem.getAt(0).c_str());
                            productModelValue.trim(" ");
                            productModelValue.toLowerCase();
                            *product_model = new char[MAX_LENGTH];
                            std::strcpy(*product_model, productModelValue.c_str());
                            
                            //get UDID
                            string deviceUDID = vItem.getAt(1).c_str();
                            std::string::size_type pos2 = deviceUDID.find(")");
                            
                            if(pos2 != std::string::npos)
                            {
                                *szSerialNo = new char[MAX_LENGTH];
                                std::strcpy(*szSerialNo, deviceUDID.substr(0, pos2).c_str());
                            }
                            
                            //we have just support one iOS Simulator, so we break when find out the first booted device
                            break;
                        }
                    }
                    previousLine = currentLine;
                }
                
                //get iOS version
                if (!previousLine.empty()) {
                    //parse data, previous line format: -- iOS 8.3 --
                    //check if previousLine contain "iOS"
                    if(strstr(previousLine.c_str(), "iOS"))
                    {
                        //remove "--"
                        AbtString iOSVersionValue(previousLine.c_str());
                        iOSVersionValue.remove("--");
                        iOSVersionValue.remove("iOS");
                        iOSVersionValue.trim(" ");
                        *ios_version = new char[MAX_LENGTH];
                        std::strcpy(*ios_version, iOSVersionValue.c_str());
                    }
                }
            }
            else
                return TA_FALSE;
        }
    }
    
    //we haven't solution to get all information of iOS Simulator jet
    //therefore temporatory set empty for the informations we can't get
    *device_name = "SIMULATOR";
    *szMacAddr = "";
    if(product_model == NULL)
    {
        *product_model = "";
    }
    if(szSerialNo == NULL)
    {
        *szSerialNo = "";
    }
    if(ios_version == NULL)
    {
        *ios_version = "";
    }

    return TA_OK;
}

