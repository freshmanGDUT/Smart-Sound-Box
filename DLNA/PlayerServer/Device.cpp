#include <stdio.h>
#include "upnp.h"
#include "common/sample_util.h"

#define DESC_URL_SIZE 200

typedef int (*upnp_action)(
	/*! [in] Document of action request. */
	IXML_Document *request,
	/*! [out] Action result. */
	IXML_Document **out,
	/*! [out] Error string in case action was unsuccessful. */
	const char **errorString);

/*! Structure for storing Tv Service identifiers and state table. */
struct TvService {
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

char c_val[1][1];
char *service_type = NULL;
struct TvService tv_service_table;
UpnpDevice_Handle device_handle = -1;

int InitUPNPSDK(char **ip_address, unsigned short *port) {
  int ret = 0;
  SampleUtil_Initialize(linux_print);
  
  if (( ret = UpnpInit(*ip_address, *port)) != UPNP_E_SUCCESS) {
    SampleUtil_Print("Error with UpnpInit -- %d\n", ret);
    UpnpFinish();
    return ret;
  }
  *ip_address = UpnpGetServerIpAddress();
  *port = UpnpGetServerPort();
  SampleUtil_Print("UPnP Initialized\n"
			 "\tipaddress = %s port = %u\n",
			 *ip_address ? *ip_address : "{NULL}", *port);
  return ret;
}

int UpnpSetServerRootDir(char *web_dir_path) {
  int ret = 0;
  if ((ret = UpnpSetWebServerRootDir( web_dir_path )) != UPNP_E_SUCCESS) {
    SampleUtil_Print("Error specifying webserver root directory -- %s:  %d\n", web_dir_path, ret);
    return ret;
  }
  return ret;
}

int DeviceHandleActionRequest(struct Upnp_Action_Request *ca_event)
{
	/* Defaults if action not found. */
	int action_found = 0;
	int i = 0;
	int service = -1;
	int retCode = 0;
	const char *errorString = NULL;
	const char *devUDN = NULL;
	const char *serviceID = NULL;
	const char *actionName = NULL;

	ca_event->ErrCode = 0;
	ca_event->ActionResult = NULL;

	devUDN     = ca_event->DevUDN;
	serviceID  = ca_event->ServiceID;
	actionName = ca_event->ActionName;
	/* Find and call appropriate procedure based on action name.
	 * Each action name has an associated procedure stored in the
	 * service table. These are set at initialization. */
	for (i = 0; i < 1 && tv_service_table.ActionNames[i] != NULL; i++) {
		if (!strcmp(actionName, tv_service_table.ActionNames[i])) {
          retCode = tv_service_table.actions[i](
          ca_event->ActionRequest,
          &ca_event->ActionResult,
          &errorString);
          action_found = 1;
          break;
		}
	}
	if (!action_found) {
		ca_event->ActionResult = NULL;
		strcpy(ca_event->ErrStr, "Invalid Action");
		ca_event->ErrCode = 401;
	} else {
		if (retCode == UPNP_E_SUCCESS) {
			ca_event->ErrCode = UPNP_E_SUCCESS;
		} else {
			/* copy the error string */
			strcpy(ca_event->ErrStr, errorString);
			switch (retCode) {
			case UPNP_E_INVALID_PARAM:
				ca_event->ErrCode = 402;
				break;
			case UPNP_E_INTERNAL_ERROR:
			default:
				ca_event->ErrCode = 501;
				break;
			}
		}
	}
	return ca_event->ErrCode;
}

int TestCallbackFunc(Upnp_EventType EventType, void *Event, void *Cookie) {
  switch(EventType) {
  case UPNP_EVENT_SUBSCRIPTION_REQUEST:
    break;
  case UPNP_CONTROL_GET_VAR_REQUEST:
    break;
  case UPNP_CONTROL_ACTION_REQUEST:
    DeviceHandleActionRequest((struct Upnp_Action_Request *)Event);
  }
    return 0;
}

int RegisterRootDevice(char *ip_address, unsigned short port, char *desc_doc_url) {
  int ret = 0;
  
  char *desc_doc_name = "test.xml";
  snprintf(desc_doc_url, DESC_URL_SIZE, "http://%s:%d/%s", ip_address, port, desc_doc_name);
  SampleUtil_Print("Registering the RootDevice\n" "\t with desc_doc_url: %s\n", desc_doc_url);
  if ((ret = UpnpRegisterRootDevice(desc_doc_url,
                                                            TestCallbackFunc,
                                                            &device_handle,
                                                            &device_handle)) != UPNP_E_SUCCESS) {
    SampleUtil_Print( "Error registering the rootdevice : %d\n", ret );
    UpnpFinish();
    return ret;
  } else {
    SampleUtil_Print("RootDevice Registered\n"
				 "Initializing State Table\n");
    return ret;
  }
}

int ReceiveAudioURL(IXML_Document * in, IXML_Document ** out, const char **errorString) {
  char *value = NULL;
  (*out) = NULL;
  (*errorString) = NULL;
  /*Get value from CP*/
  if (!(value = SampleUtil_GetFirstDocumentItem(in, "AudioURL"))) {
    (*errorString) = "Invalid AudioURL";
    return UPNP_E_INVALID_PARAM;
  }
  strcpy(tv_service_table.VariableStrVal[0], value);
  /*Notify device??*/
  UpnpNotify(device_handle, tv_service_table.UDN, tv_service_table.ServiceId, (const char **)&tv_service_table.VariableName[0], (const char **)&tv_service_table.VariableStrVal[0], 1);
  /*Create response xml to CP*/
  if (UpnpAddToActionResponse(out, "ReceiveAudioURL", service_type, "r_AudioURL", value) != UPNP_E_SUCCESS) {
    (*out) = NULL;
    (*errorString) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }
  printf("AudioURL is %s\n", value);
  return UPNP_E_SUCCESS;
}

int SetActionTable(struct TvService *out) {
  out->ActionNames[0] = "ReceiveAudioURL";
  out->actions[0] = ReceiveAudioURL;
  return 1;
}

int SetServiceTable(const char *UDN, const char *serviceId, const char *serviceTypeS, struct TvService *out) {
  int i = 0;

  strcpy(out->UDN, UDN);
  strcpy(out->ServiceId, serviceId);
  strcpy(out->ServiceType, serviceTypeS);
  
  out->VariableCount = 1;
  out->VariableName[0] = "AudioURL";
  out->VariableStrVal[0] = c_val[0];
  
  return SetActionTable(out);
}

int DeviceStateTableInit(char *DescDocURL) {
  IXML_Document *DescDoc = NULL;
  char *udn = NULL;
  char *servid_ctrl = NULL;
  char *evnturl_ctrl = NULL;
  char *ctrlurl_ctrl = NULL;
  int ret = 0;
  /*Download description document */
  if (UpnpDownloadXmlDoc(DescDocURL, &DescDoc) != UPNP_E_SUCCESS) {
    SampleUtil_Print("TvDeviceStateTableInit -- Error Parsing %s\n", DescDocURL);
    ret = UPNP_E_INVALID_DESC;
    if (DescDoc) {
      ixmlDocument_free(DescDoc);
    }
  } 
  /*Get device UDN*/
  udn = SampleUtil_GetFirstDocumentItem(DescDoc, "UDN");
  /*Get service type*/
  service_type = SampleUtil_GetFirstDocumentItem(DescDoc, "serviceType");
  /*Parse service*/
  if (!SampleUtil_FindAndParseService(DescDoc, DescDocURL,
                                                                 service_type,
                                                                 &servid_ctrl, &evnturl_ctrl,
                                                                 &ctrlurl_ctrl)) {
      SampleUtil_Print("TvDeviceStateTableInit -- Error: Could not find Service: %s\n", service_type);
      ret = UPNP_E_INVALID_DESC;
      if (DescDoc) {
      ixmlDocument_free(DescDoc);
    }
  }
  printf("service type is %s\n", service_type);
  /*Set service table*/
  SetServiceTable(udn, servid_ctrl, service_type, &tv_service_table);
  return ret;
}

int DeviceAdvertisement() {
  int default_advr_expire = 100;
  int ret = 0;
  if ((ret = UpnpSendAdvertisement(device_handle, default_advr_expire)) != UPNP_E_SUCCESS) {
    SampleUtil_Print("Error sending advertisements: %d\n", ret);
    UpnpFinish();
    return ret;
  }
}

int main(int argv, char **argc) {
  char *ip_address = NULL;
  unsigned short port = 0;
  char desc_doc_url[DESC_URL_SIZE];
  int ret = 0;
  /*Initalize some func tool, like print or analysisi xml*/
  SampleUtil_Initialize(linux_print);
  /*Initalize Upnp SDK*/
  ret = InitUPNPSDK(&ip_address, &port);
  if (ret != UPNP_E_SUCCESS) {
      exit(0);
}
  /*Set up thr root dir*/
  char web_dir_path[] = "./web";
  ret = UpnpSetServerRootDir(web_dir_path);
  if (ret != UPNP_E_SUCCESS) {
    exit(1);
  }  
  /*Register root device*/
  ret = RegisterRootDevice(ip_address, port, desc_doc_url);
  if (ret != UPNP_E_SUCCESS) {
    exit(2);
  }  
  /*Device initialized*/
  DeviceStateTableInit(desc_doc_url);
  SampleUtil_Print("State Table Initialized\n");
  /*Advertising the device*/
  ret = DeviceAdvertisement();
  if (ret != UPNP_E_SUCCESS) {
    exit(3);
  }
  return 0;
}
