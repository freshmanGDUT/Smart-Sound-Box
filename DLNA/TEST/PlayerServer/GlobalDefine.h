#ifndef GLOBALDEFINE_H_INCLUDE
#define GLOBALDEFINE_H_INCLUDE
#include "upnp.h"
#include "ithread.h"

#define DESC_URL_SIZE 200

#define PLAYER_SERVICE_CONTROL 0

#define PLAYER_SERVICE_NUM 1

#define PLAYER_CONTROL_VARCOUNT 1

#define PLAYER_CONTROL_AUDIOURL 0

UpnpDevice_Handle device_handle = -1;

/*! Mutex for protecting the global state table data
 * in a multi-threaded, asynchronous environment.
 * All functions should lock this mutex before reading
 * or writing the state table data. */
ithread_mutex_t PlayerDevMutex;

const char *service_type[] = {
    "urn:schemas-upnp-org:service:PlayerService:1"
};

typedef int (*upnp_action)(
	/*! [in] Document of action request. */
	IXML_Document *request,
	/*! [out] Action result. */
	IXML_Document **out,
	/*! [out] Error string in case action was unsuccessful. */
	const char **errorString);

/*! Structure for storing Tv Service identifiers and state table. */
struct PlayerService {
	/*! Universally Unique Device Name. */
	char UDN[NAME_SIZE];
	/*! . */
	char ServiceId[NAME_SIZE];
	/*! . */
	char ServiceType[NAME_SIZE];
	/*! . */
	const char *VariableName[1]; 
	/*! . */
	char *VariableStrVal[1];
	/*! . */
	const char *ActionNames[1];
	/*! . */
	upnp_action actions[1];
	/*! . */
	int VariableCount;
};



#endif 
