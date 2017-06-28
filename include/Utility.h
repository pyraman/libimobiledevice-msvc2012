#ifndef _ULTILITY_H_
#define _ULTILITY_H_

#include <string>
#include <vector>
#include "AbtString.h"

#ifdef WIN32
#include <Windows.h>
#include <Psapi.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#endif

using namespace std;

class Utility 
{
private:
	static vector<string> & split(const string &s, const string & szDelim, vector<string> &elems);
	static int ishex(int x);
public:
	static void tolower(string& szInputString);
	static bool compare(const string & s1, const string & s2);
	static vector<string> split(const string &szInputString, char delim);
	static vector<string> split(const string &szInputString, const string & delim);
	static bool isProcessRunning(const long & nProcessId);
	static int decode(const char *s, char *dec);

    /*
     * execute command line and get out put
     * @param szCmd	: command line
     * @param szOut	: out put of command line
     * @return		: TA_OK if succeeded, otherwise return TA_FALSE
     */
    static TARESULT executeCommandLine( const char*& szCmd, AbtString& szOut);
    
    /*
     * get iOS Simulator Information by command line
     */
    static TARESULT getSimulatorInformationByCommandLine( const char* device_id, char** device_name, char** product_model, char** ios_version, char** szMacAddr, char** szSerialNo);

};

#endif