#ifndef MY_CTRLPT_H
#define MY_CTRLPT_H

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "sample_util.h"
#include "upnp/upnp.h"

#define Name_Size 100
#define My_Max_Val_Len 5
#define My_Max_Name_Len 10

#define My_SUCCESS		0
#define My_ERROR		(-1)

struct My_Service {
    char ServiceId[Name_Size];
    char ServiceType[Name_Size];
//------------------------------------chen------------------------------------//			
    char **VariableStrVal;
    char **VariableName;
    int Variable_Size;
//------------------------------------chen------------------------------------//		

    char EventURL[Name_Size];
    char ControlURL[Name_Size];
    char SID[Name_Size];
};

struct My_Device {
	char UDN[250];
    	char DescDocURL[250];
	char FriendlyName[250];
    	char PresURL[250];
    	int  AdvrTimeOut;
//------------------------------------chen------------------------------------//		
    	struct My_Service *TvService;
	int service_type_size;
//------------------------------------chen------------------------------------//		

};

//------------------------------------chen------------------------------------//		
struct My_DeviceNode {
    struct My_Device device;
    struct My_DeviceNode *next;
};
//------------------------------------chen------------------------------------//		

#define Service_Control 0
#define Service_Picture 1


enum CmdLoop_Cmds {
	Cmd_PRTHELP = 0,
	Cmd_PRTFULLHELP,
	Cmd_POWON,
	Cmd_POWOFF,
	Cmd_SETCHAN,
	Cmd_SETVOL,
	Cmd_SETCOL,
	Cmd_SETTINT,
	Cmd_SETCONT,
	Cmd_SETBRT,
	Cmd_CTRLACTION,
	Cmd_PICTACTION,
	Cmd_CTRLGETVAR,
	Cmd_PICTGETVAR,
	Cmd_SETURL,
	Cmd_PRTDEV,
	Cmd_LSTDEV,
	Cmd_REFRESH,
	Cmd_EXITCMD
};

#define SERVICE_TYPE_LIST "serviceType"
#define SCPDURL_LIST "SCPDURL"
#define DEFAULT_FRIDENDLYNAME "DEFAULT_FRIDENDLYNAME"
#define DEFAULT_PRESURL "DEFAULT_PRESURL"

int CtrlPoint_Start(print_string printFunctionPtr, state_update updateFunctionPtr, int combo);

int CtrlPoint_Stop(void);

void *CtrlPoint_CommandLoop(void *args);

int My_Ctrlpt_Test(int argc, char **argv);

#endif
