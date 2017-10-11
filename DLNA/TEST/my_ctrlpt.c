#include "my_ctrlpt.h"

ithread_mutex_t Device_List_Mutex;
UpnpClient_Handle my_ctrlpt_handle = -1;

int CtrlPoint_Timer_LoopRun = 1;
int default_time = 1801;
struct My_DeviceNode *Global_Device_List = NULL;

int CtrlPoint_Parse(IXML_NodeList *list, struct My_Service *TvService) {
	if ((NULL == list) || (NULL == TvService)) {
		printf("(NULL == list) || (NULL == TvService) !\n");
		return 0;
	}
	int i = 0;
	for (i = 0; i < ixmlNodeList_length(list); i++) {
		IXML_Node *cur1 = ixmlNodeList_item(list, i);
		if (NULL == cur1) {
			printf("cur1 is NULL !\n");
			return 0;
		}
		IXML_Node *cur2 = ixmlNode_getFirstChild(cur1);
		if (NULL == cur2) {
			printf("cur2 is NULL !\n");
			return 0;
		}		
		char *tmp = strdup(ixmlNode_getNodeValue(cur2));
		if (NULL == tmp) {
			printf("tmp is NULL !\n");
			return 0;
		}
		strcpy(TvService[i].ServiceType, tmp);
		free(tmp);
		tmp = NULL;	
	}
	return 1;
}



void My_Parse_StateVariable(xmlDocPtr doc, xmlNodePtr cur, struct My_Service *TvService, int i) {
	//遍历处理根节点的每一个子节点
	cur = cur->xmlChildrenNode;
	while (NULL != cur) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"name"))) {
			xmlChar *key;
			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			printf("name: %s !\n", key);
			strcpy(TvService->VariableName[i], key);
			printf("My_Parse_StateVariable - 1 \n");
			xmlFree(key);	
			printf("My_Parse_StateVariable - 2 \n");
	    	}
	    	cur = cur->next;
	}
}

void My_Parse_ServiceStateTable(xmlDocPtr doc, xmlNodePtr cur, struct My_Service *TvService) {
	int i = 0;
	//遍历处理根节点的每一个子节点
	printf("My_Parse_ServiceStateTable - 1 \n");

	cur = cur->xmlChildrenNode;
	while (NULL != cur) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"stateVariable"))) {
			My_Parse_StateVariable(doc, cur, TvService, i);
			i++;
	    	}
	    	cur = cur->next;
	}	
	printf("My_Parse_ServiceStateTable - 2 \n");

}

void My_Parse_Xml(char *file_name, struct My_Service *TvService) {
	printf("My_Parse_Xml - 1 \n");
	if (NULL == file_name) {
		printf("NULL == file_name !\n");
		return;	
	}
	xmlDocPtr doc;   //xml整个文档的树形结构
    	xmlNodePtr cur;  //xml节点
    	xmlChar *id;     //phone id

    	//获取树形结构
    	doc = xmlParseFile(file_name);
	if (doc == NULL) {
    		printf("Failed to parse xml file: %s !\n", file_name);
		return;
    	}

	//获取根节点

	cur = xmlDocGetRootElement(doc);
	if (NULL == cur) {
		printf("Root is empty !\n");
		xmlFreeDoc(doc);
		return;	
	}

	if ((xmlStrcmp(cur->name, (const xmlChar *)"scpd"))) {
		printf("The root is not scpd !\n");
		xmlFreeDoc(doc);
		return;	
	}
	//遍历处理根节点的每一个子节点

	cur = cur->xmlChildrenNode;
	while (NULL != cur) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"serviceStateTable"))) {
			My_Parse_ServiceStateTable(doc, cur, TvService);
	    	}
	    	cur = cur->next;
	}	
	xmlFreeDoc(doc);	
	printf("My_Parse_Xml - 2 \n");

}



int CtrlPoint_Get_Variable_Ex(struct My_Service *TvService, char *desc_doc_url) {
	IXML_Document *DescDoc = NULL;
	IXML_NodeList *node_list = NULL;
	int i = 0;
	printf("CtrlPoint_Get_Variable_Ex - 1\n");
	if (UPNP_E_SUCCESS != UpnpDownloadXmlDoc(desc_doc_url, &DescDoc)) {
		printf("CtrlPoint_Get_Variable_Ex -- Error Parsing %s !\n", desc_doc_url);
		return 0;
	}
	printf("CtrlPoint_Get_Variable_Ex - 1.1.1\n");
	node_list = ixmlDocument_getElementsByTagName(DescDoc, "dataType");

	if (NULL == node_list) {
		printf("NUL == node_list !\n");
		ixmlDocument_free(DescDoc);		
		return 0;
	}
	printf("CtrlPoint_Get_Variable_Ex - 1.1\n");

	TvService->VariableStrVal = (char **)malloc(ixmlNodeList_length(node_list) * sizeof(char *));
	if (NULL == TvService->VariableStrVal) {
		printf("malloc TvService->VariableStrVal Error !\n");
		ixmlNodeList_free(node_list);
		ixmlDocument_free(DescDoc);
		return 0;
	}
	memset(TvService->VariableStrVal, 0, ixmlNodeList_length(node_list) * sizeof(char *));
	printf("CtrlPoint_Get_Variable_Ex - 1.2\n");

	TvService->VariableName = (char **)malloc(ixmlNodeList_length(node_list) * sizeof(char *));
	if (NULL == TvService->VariableName) {
		printf("malloc TvService->VariableName Error !\n");
		free(TvService->VariableStrVal);
		TvService->VariableStrVal = NULL;
		ixmlNodeList_free(node_list);
		ixmlDocument_free(DescDoc);
		return 0;
	}
	memset(TvService->VariableName, 0, ixmlNodeList_length(node_list) * sizeof(char *));
	printf("CtrlPoint_Get_Variable_Ex - 1.3\n");

	for (i = 0; i < ixmlNodeList_length(node_list); i++) {
		TvService->VariableName[i] = (char *)malloc(My_Max_Name_Len);
		if (NULL == TvService->VariableName[i]) {
			printf("NULL == TvService->VariableName[%d] !\n", i);
			ixmlNodeList_free(node_list);
			ixmlDocument_free(DescDoc);
			return 0;
		}
		memset(TvService->VariableName[i], 0, My_Max_Name_Len);
	}
	// 解析变量名字

	printf("CtrlPoint_Get_Variable_Ex - 1.4\n");
	ixmlDocument_free(DescDoc);		
	printf("CtrlPoint_Get_Variable_Ex - 1.4.1\n");
	
	My_Parse_Xml(desc_doc_url, TvService);
	printf("CtrlPoint_Get_Variable_Ex - 1.5\n");

	TvService->Variable_Size = ixmlNodeList_length(node_list);
	printf("CtrlPoint_Get_Variable_Ex - 1.6\n");
	ixmlNodeList_free(node_list);
	printf("CtrlPoint_Get_Variable_Ex - 1.7\n");

	return 1;
}

int CtrlPoint_Get_Variable(struct My_Service *TvService, IXML_NodeList *list, const char *location) {
	char *ip_address = NULL;
	unsigned short port = 0;
	int i = 0;
	printf("CtrlPoint_Get_Variable - 1 \n");

	if ((NULL == TvService) || (NULL == list) || (NULL == location)) {
		printf("(NULL == TvService) || (NULL == list) || (NULL == location) !\n");
		return 0;
	}

	for (i = 0; i < ixmlNodeList_length(list); i++) {
		char desc_doc_url[200] = {0};
		IXML_Node *cur1 = ixmlNodeList_item(list, i);
		if (NULL == cur1) {

			printf("NULL == cur1 !\n");
			return 0;
		}
		IXML_Node *cur2 = ixmlNode_getFirstChild(cur1);
		if (NULL == cur2) {

			printf("NULL == cur2 !\n");
			return 0;
		}
		char *tmp = strdup(ixmlNode_getNodeValue(cur2));
		if (NULL == tmp) {
			printf("NULL == tmp !\n");
			return 0;
		}
		// 这个就是设备中服务描述文件的路径
		if (UPNP_E_SUCCESS != UpnpResolveURL(location, tmp, desc_doc_url)) {
			SampleUtil_Print("Error generating presURL from %s + %s\n", location, tmp);
			free(tmp);
			tmp = NULL;
			return 0;
		}
		printf("tmp = %s\n", tmp);
		printf("tmp = %s\n", desc_doc_url);
		free(tmp);
		tmp = NULL;
		printf("CtrlPoint_Get_Variable - 1.1 \n");
		if (0 == CtrlPoint_Get_Variable_Ex(TvService + i, desc_doc_url)) {
			printf("0 == Get_Variable_Ex(&TvService[%d], desc_doc_url) !\n", i);
			return 0;
		}
		printf("CtrlPoint_Get_Variable - 1.2 \n");

	}
	printf("CtrlPoint_Get_Variable - 2 \n");

	return 1;
}

void CtrlPoint_Remove_Variable(struct My_Service *TvService, int size) {
	if ((NULL == TvService) || (0 >= size)) {
		printf("(NULL == TvService) || (0 >= size) !\n");
		return;
	}
	int i = 0;
	int j = 0;
	for (i = 0; i < size; i++) {
		if (NULL != TvService[i].VariableStrVal) {
			for (j = 0; j < TvService[i].Variable_Size; j++) {

				if (NULL != TvService[i].VariableStrVal[j]) {

					free(TvService[i].VariableStrVal[j]);
					TvService[i].VariableStrVal[j] = NULL;
				}
			}
			free(TvService[i].VariableStrVal);
			TvService[i].VariableStrVal = NULL;
		}
		if (NULL != TvService[i].VariableName) {
			for (j = 0; j < TvService[i].Variable_Size; j++) {

				if (NULL != TvService[i].VariableName[j]) {

					free(TvService[i].VariableName[j]);
					TvService[i].VariableName[j] = NULL;
				}
			}
			free(TvService[i].VariableName);
			TvService[i].VariableName = NULL;
		}
		TvService[i].Variable_Size = 0;
	}
}

const char *My_Device_Type[] = {"urn:schemas-upnp-org:device:tvdevice:1", 
	"urn:schemas-upnp-org:device:tvdevice:2",
	"urn:schemas-upnp-org:device:PlayerDevice:1"
	};


int Device_Type(char *deviceType) {
	int i = 0;
	int SIZE = sizeof(My_Device_Type) / sizeof(*My_Device_Type);
	for (i = 0; i < SIZE; i++) {
		if (0 == strcmp(deviceType, My_Device_Type[i])) {
			return 0;
		}
	}
	
	return 1;
}

 //------------------------------------chen------------------------------------//	
void CtrlPoint_AddDevice(IXML_Document *DescDoc, const char *location, int expires)
{
	char *deviceType = NULL;
	char *friendlyName = NULL;
	char presURL[200];
	char *baseURL = NULL;
	char *relURL = NULL;
	char *UDN = NULL;

	char **serviceId = NULL;
	char **eventURL = NULL;
	char **controlURL = NULL;
	Upnp_SID *eventSID = NULL;
	int *TimeOut = NULL;

	struct My_DeviceNode *deviceNode = NULL;
	struct My_DeviceNode *tmpdevnode = NULL;

	int ret = 1;
	int found = 0;

	int service = 0;
	int var = 0;
	
	int SIZE = 0;

	printf("CtrlPoint_AddDevice - 1 \n");
	ithread_mutex_lock(&Device_List_Mutex);
	printf("CtrlPoint_AddDevice - 1.1 \n");
	UDN = SampleUtil_GetFirstDocumentItem(DescDoc, "UDN");
	deviceType = SampleUtil_GetFirstDocumentItem(DescDoc, "deviceType");
	friendlyName = SampleUtil_GetFirstDocumentItem(DescDoc, "friendlyName");
	
//------------------------------------chen------------------------------------//		
	// 例程中的tvdevicedesc.xml 没有URLBase 标识
	baseURL = SampleUtil_GetFirstDocumentItem(DescDoc, "URLBase");
//------------------------------------chen------------------------------------//	

	relURL = SampleUtil_GetFirstDocumentItem(DescDoc, "presentationURL");

	ret = UpnpResolveURL((baseURL ? baseURL : location), relURL, presURL);

	if (UPNP_E_SUCCESS != ret)
		SampleUtil_Print("Error generating presURL from %s + %s\n", baseURL, relURL);


	if (Device_Type(deviceType) == 0) {
		SampleUtil_Print("Found Tv device\n");

	tmpdevnode = Global_Device_List;
	while (tmpdevnode) {
		if (strcmp(tmpdevnode->device.UDN, UDN) == 0) {
			found = 1;
			break;
		}
		tmpdevnode = tmpdevnode->next;
	}

	if (found) {
		/* The device is already there, so just update  */
		/* the advertisement timeout field */
		tmpdevnode->device.AdvrTimeOut = expires;
		printf("CtrlPoint_AddDevice - 2 \n");
	} else {
		printf("CtrlPoint_AddDevice - 2.1 \n");

		IXML_NodeList *serviceType_list = ixmlDocument_getElementsByTagName(DescDoc, SERVICE_TYPE_LIST);

		if (NULL == serviceType_list) {
			SampleUtil_Print("Error: %s Could not find SERVICE_TYPE_LIST !\n", DescDoc);
			goto ERROR;
		}
		
		IXML_NodeList *SCPDURL_list = ixmlDocument_getElementsByTagName(DescDoc, SCPDURL_LIST);
		if (NULL == SCPDURL_list) {
			SampleUtil_Print("Error: %s Could not find SCPDURL_LIST !\n", DescDoc);
			ixmlNodeList_free(serviceType_list);
			serviceType_list = NULL;
			goto ERROR;
		} 
		printf("CtrlPoint_AddDevice - 3 \n");

		/* Create a new device node */
		deviceNode = (struct My_DeviceNode *)malloc(sizeof(struct My_DeviceNode));
		if (NULL == deviceNode) {
			SampleUtil_Print("malloc deviceNode error !\n");
			ixmlNodeList_free(SCPDURL_list);
			SCPDURL_list = NULL;
			ixmlNodeList_free(serviceType_list);
			serviceType_list = NULL;

			goto ERROR;
		}
		memset(deviceNode, 0, sizeof(struct My_DeviceNode));

		deviceNode->device.service_type_size = ixmlNodeList_length(serviceType_list);

		deviceNode->device.TvService = (struct My_Service *)malloc(deviceNode->device.service_type_size * sizeof(struct My_Service));
		if (NULL == deviceNode->device.TvService) {
			SampleUtil_Print("malloc deviceNode->device.TvService error !\n");

			free(deviceNode);
			deviceNode = NULL;
			ixmlNodeList_free(SCPDURL_list);
			SCPDURL_list = NULL;
			ixmlNodeList_free(serviceType_list);
			serviceType_list = NULL;
			goto ERROR;
		}
		memset(deviceNode->device.TvService, 0, deviceNode->device.service_type_size * sizeof(struct My_Service));

//------------------------------------chen------------------------------------//
		/* 确定每个service 中的 Variable 的个数 */ 
		printf("CtrlPoint_AddDevice - 4 \n");

		if (0 == CtrlPoint_Get_Variable(deviceNode->device.TvService, SCPDURL_list, location)) {
			SampleUtil_Print("0 == CtrlPoint_Get_Variable(deviceNode->device.TvService, SCPDURL_list, location) !\n");
			printf("CtrlPoint_AddDevice - 4.1 \n");
			CtrlPoint_Remove_Variable(deviceNode->device.TvService, deviceNode->device.service_type_size);
			printf("CtrlPoint_AddDevice - 4.2 \n");

			free(deviceNode->device.TvService);
			deviceNode->device.TvService = NULL;
			free(deviceNode);
			deviceNode = NULL;
			ixmlNodeList_free(SCPDURL_list);
			SCPDURL_list = NULL;
			ixmlNodeList_free(serviceType_list);
			serviceType_list = NULL;
			goto ERROR;		
		}

		for (service = 0; service < deviceNode->device.service_type_size; service++) {
			int j = 0;
			printf("deviceNode->device.TvService[service].Variable_Size = %d\n", deviceNode->device.TvService[service].Variable_Size);
			for (j = 0; j < deviceNode->device.TvService[service].Variable_Size; j++) {
				printf("%s ", deviceNode->device.TvService[service].VariableName[j]);
			}
			printf("\n");
		}
		
//------------------------------------chen------------------------------------//
		printf("CtrlPoint_AddDevice - 5 \n");

		serviceId = (char **)malloc(deviceNode->device.service_type_size * sizeof(char *));
		if (NULL == serviceId) {
			SampleUtil_Print("malloc serviceId error !\n");
			CtrlPoint_Remove_Variable(deviceNode->device.TvService, deviceNode->device.service_type_size);
			free(deviceNode->device.TvService);
			deviceNode->device.TvService = NULL;
			free(deviceNode);
			deviceNode = NULL;
			ixmlNodeList_free(SCPDURL_list);
			SCPDURL_list = NULL;
			ixmlNodeList_free(serviceType_list);
			serviceType_list = NULL;
			goto ERROR;
		}
		memset(serviceId, 0, deviceNode->device.service_type_size * sizeof(char *));

		eventURL = (char **)malloc(deviceNode->device.service_type_size * sizeof(char *));
		if (NULL == eventURL) {
			SampleUtil_Print("malloc eventURL error !\n");
			free(serviceId);
			serviceId = NULL;
			CtrlPoint_Remove_Variable(deviceNode->device.TvService, deviceNode->device.service_type_size);

			free(deviceNode->device.TvService);
			deviceNode->device.TvService = NULL;
			free(deviceNode);
			deviceNode = NULL;
			ixmlNodeList_free(SCPDURL_list);
			SCPDURL_list = NULL;
			ixmlNodeList_free(serviceType_list);
			serviceType_list = NULL;
			goto ERROR;
		}
		memset(eventURL, 0, deviceNode->device.service_type_size * sizeof(char *));

		controlURL = (char **)malloc(deviceNode->device.service_type_size * sizeof(char *));
		if (NULL == controlURL) {
			SampleUtil_Print("malloc controlURL error !\n");
			free(eventURL);
			eventURL = NULL;
			free(serviceId);
			serviceId = NULL;
			CtrlPoint_Remove_Variable(deviceNode->device.TvService, deviceNode->device.service_type_size);

			free(deviceNode->device.TvService);
			deviceNode->device.TvService = NULL;
			free(deviceNode);
			deviceNode = NULL;
			ixmlNodeList_free(SCPDURL_list);
			SCPDURL_list = NULL;
			ixmlNodeList_free(serviceType_list);
			serviceType_list = NULL;
			goto ERROR;
		}
		memset(controlURL, 0, deviceNode->device.service_type_size * sizeof(char *));
		printf("CtrlPoint_AddDevice - 6 \n");

		TimeOut = (int *)malloc(deviceNode->device.service_type_size * sizeof(int));
		if (NULL == TimeOut) {
			SampleUtil_Print("malloc TimeOut error !\n");
			free(controlURL);
			controlURL = NULL;
			free(eventURL);
			eventURL = NULL;
			free(serviceId);
			serviceId = NULL;
			CtrlPoint_Remove_Variable(deviceNode->device.TvService, deviceNode->device.service_type_size);

			free(deviceNode->device.TvService);
			deviceNode->device.TvService = NULL;
			free(deviceNode);
			deviceNode = NULL;
			ixmlNodeList_free(SCPDURL_list);
			SCPDURL_list = NULL;
			ixmlNodeList_free(serviceType_list);
			serviceType_list = NULL;
			goto ERROR;
		}
		memset(TimeOut, 0, deviceNode->device.service_type_size * sizeof(int));		
		for (service = 0; service < deviceNode->device.service_type_size; service++) {
			TimeOut[service] = default_time;
		}

		eventSID = (Upnp_SID *)malloc(deviceNode->device.service_type_size * sizeof(Upnp_SID));
		if (NULL == eventSID) {
			SampleUtil_Print("malloc eventSID error !\n");
			free(TimeOut);
			TimeOut = NULL;
			free(controlURL);
			controlURL = NULL;
			free(eventURL);
			eventURL = NULL;
			free(serviceId);
			serviceId = NULL;
			CtrlPoint_Remove_Variable(deviceNode->device.TvService, deviceNode->device.service_type_size);

			free(deviceNode->device.TvService);
			deviceNode->device.TvService = NULL;
			free(deviceNode);
			deviceNode = NULL;
			ixmlNodeList_free(SCPDURL_list);
			SCPDURL_list = NULL;
			ixmlNodeList_free(serviceType_list);
			serviceType_list = NULL;
			goto ERROR;
		}
		memset(eventSID, 0, deviceNode->device.service_type_size * sizeof(Upnp_SID));		
		printf("CtrlPoint_AddDevice - 7 \n");

		if (0 == CtrlPoint_Parse(serviceType_list, deviceNode->device.TvService)) {
			SampleUtil_Print("0 == Parse(serviceType_list, deviceNode->device.TvService) !\n");
			free(eventSID);
			eventSID = NULL;
			free(TimeOut);
			TimeOut = NULL;
			free(controlURL);
			controlURL = NULL;
			free(eventURL);
			eventURL = NULL;
			free(serviceId);
			serviceId = NULL;
			CtrlPoint_Remove_Variable(deviceNode->device.TvService, deviceNode->device.service_type_size);

			free(deviceNode->device.TvService);
			deviceNode->device.TvService = NULL;
			free(deviceNode);
			deviceNode = NULL;
			ixmlNodeList_free(SCPDURL_list);
			SCPDURL_list = NULL;
			ixmlNodeList_free(serviceType_list);
			serviceType_list = NULL;
			goto ERROR;
		}

		ixmlNodeList_free(SCPDURL_list);
		SCPDURL_list = NULL;
		ixmlNodeList_free(serviceType_list);
		serviceType_list = NULL;

		SIZE = deviceNode->device.service_type_size;
		printf("CtrlPoint_AddDevice - 8 \n");

		for (service = 0; service < deviceNode->device.service_type_size; service++) {
			if (SampleUtil_FindAndParseService(DescDoc, location, deviceNode->device.TvService[service].ServiceType,
				&serviceId[service], &eventURL[service], &controlURL[service])) {

				SampleUtil_Print("Subscribing to EventURL %s...\n", eventURL[service]);
				ret = UpnpSubscribe(my_ctrlpt_handle, eventURL[service], &TimeOut[service], eventSID[service]);
				if (ret == UPNP_E_SUCCESS) {
					SampleUtil_Print("Subscribed to EventURL with SID = %s\n", eventSID[service]);
				} else {
					SampleUtil_Print("Error Subscribing to EventURL -- %d\n", ret);
					strcpy(eventSID[service], "");
				}
			} else {
				SampleUtil_Print("Error: Could not find Service: %s\n", deviceNode->device.TvService[service].ServiceType);

			}
		}
/*		// Create a new device node
			deviceNode =
			    (struct My_DeviceNode *)
			    malloc(sizeof(struct My_DeviceNode));
*/
		printf("CtrlPoint_AddDevice - 9 \n");

		strcpy(deviceNode->device.UDN, UDN);
		printf("CtrlPoint_AddDevice - 9.1 \n");
		
		strcpy(deviceNode->device.DescDocURL, location);
		printf("CtrlPoint_AddDevice - 9.2 \n");
		if (NULL != friendlyName) {
			strcpy(deviceNode->device.FriendlyName, friendlyName);
		} else {
			strcpy(deviceNode->device.FriendlyName, DEFAULT_FRIDENDLYNAME);
		}
		printf("CtrlPoint_AddDevice - 9.3 \n");
		if (NULL != relURL) {
			strcpy(deviceNode->device.PresURL, presURL);
		} else {
			strcpy(deviceNode->device.PresURL, DEFAULT_PRESURL);
		}
		printf("CtrlPoint_AddDevice - 9.4 \n");

		deviceNode->device.AdvrTimeOut = expires;

		for (service = 0; service < deviceNode->device.service_type_size; service++) {
			if (serviceId[service] == NULL) {
				/* not found */
				continue;
			}
			strcpy(deviceNode->device.TvService[service].ServiceId, serviceId[service]);
//			strcpy(deviceNode->device.TvService[service].ServiceType, TvServiceType[service]);
			strcpy(deviceNode->device.TvService[service].ControlURL, controlURL[service]);
			strcpy(deviceNode->device.TvService[service].EventURL, eventURL[service]);
			strcpy(deviceNode->device.TvService[service].SID, eventSID[service]);

			for (var = 0; var < deviceNode->device.TvService[service].Variable_Size; var++) {
				// Ô­ŽúÂëÖ±œÓŸÍÕâÃŽÐŽ£¿£¿¿ÉÄÜŽæÔÚ·ÖÅäÊ§°ÜµÄÎÊÌâ¡¢
				deviceNode->device.TvService[service].VariableStrVal[var] = (char *)malloc(My_Max_Val_Len);
				if (NULL == deviceNode->device.TvService[service].VariableStrVal[var]) {

					//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				}
				strcpy(deviceNode->device.TvService[service].VariableStrVal[var], "");
			}
		}
		printf("CtrlPoint_AddDevice - 10 \n");

		deviceNode->next = NULL;
		/* Insert the new device node in the list */
		if ((tmpdevnode = Global_Device_List)) {
			while (tmpdevnode) {
				if (tmpdevnode->next) {
					tmpdevnode = tmpdevnode->next;
				} else {
					tmpdevnode->next = deviceNode;
					break;
				}
			}
		} else {
			Global_Device_List = deviceNode;
		}
		printf("CtrlPoint_AddDevice - 11 \n");

		/*Notify New Device Added */
		SampleUtil_StateUpdate(NULL, NULL, deviceNode->device.UDN, DEVICE_ADDED);
	}
//------------------------------------chen------------------------------------//		
	}
//------------------------------------chen------------------------------------//	
	printf("CtrlPoint_AddDevice - 12 \n");

ERROR:
	ithread_mutex_unlock(&Device_List_Mutex);

	if (deviceType)
		free(deviceType);
	if (friendlyName)
		free(friendlyName);
	if (UDN)
		free(UDN);
	if (baseURL)
		free(baseURL);
	if (relURL)
		free(relURL);
	printf("CtrlPoint_AddDevice - 13 \n");

	for (service = 0; service < SIZE; service++) {
		if (serviceId[service])
			free(serviceId[service]);
		if (controlURL[service])
			free(controlURL[service]);
		if (eventURL[service])
			free(eventURL[service]);
	}
	printf("CtrlPoint_AddDevice - 14 \n");

	if (NULL != serviceId) {
		free(serviceId);
	}
	if (NULL != controlURL) {
		free(controlURL);
	}
	if (NULL != eventURL) {
		free(eventURL);
	}
	if (NULL != eventSID) {
		free(eventSID);
	}
	if (NULL != TimeOut) {
		free(TimeOut);
	}
	printf("CtrlPoint_AddDevice - 15 \n");
}
//------------------------------------chen------------------------------------//

//------------------------------------chen------------------------------------//
int CtrlPoint_DeleteNode( struct My_DeviceNode *node )
{
	int rc, service, var;
	printf("CtrlPoint_DeleteNode - 1\n");

	if (NULL == node) {
		SampleUtil_Print
		    ("ERROR: TvCtrlPointDeleteNode: Node is empty\n");
		return My_ERROR;
	}
	printf("CtrlPoint_DeleteNode - 1.1\n");

	for (service = 0; service < node->device.service_type_size; service++) {
		/*
		   If we have a valid control SID, then unsubscribe 
		 */
		if (strcmp(node->device.TvService[service].SID, "") != 0) {
			rc = UpnpUnSubscribe(my_ctrlpt_handle, node->device.TvService[service].SID);
			if (UPNP_E_SUCCESS == rc) {
				SampleUtil_Print
				    ("Unsubscribed from Tv %s EventURL with SID=%s\n",
				     node->device.TvService[service].ServiceType,
				     node->device.TvService[service].SID);
			} else {
				SampleUtil_Print("Error unsubscribing to Tv %s EventURL -- %d\n",
				     node->device.TvService[service].ServiceType, rc);
			}
		}
		printf("CtrlPoint_DeleteNode - 1.2\n");
		printf("node->device.TvService[service].Variable_Size = %d\n", node->device.TvService[service].Variable_Size);
		for (var = 0; var < node->device.TvService[service].Variable_Size; var++) {
			if (node->device.TvService[service].VariableStrVal[var]) {
				free(node->device.TvService[service].VariableStrVal[var]);
			}
		}
		free(node->device.TvService[service].VariableStrVal);
		node->device.TvService[service].VariableStrVal = NULL;
		printf("CtrlPoint_DeleteNode - 1.3\n");

		for (var = 0; var < node->device.TvService[service].Variable_Size; var++) {
			if (node->device.TvService[service].VariableName[var]) {
				free(node->device.TvService[service].VariableName[var]);
			}
		}
		free(node->device.TvService[service].VariableName);
		node->device.TvService[service].VariableName = NULL;
		printf("CtrlPoint_DeleteNode - 1.4\n");
		node->device.TvService[service].Variable_Size = 0;
	}
	printf("CtrlPoint_DeleteNode - 1.5\n");
	/*Notify New Device Added */
	SampleUtil_StateUpdate(NULL, NULL, node->device.UDN, DEVICE_REMOVED);
	printf("CtrlPoint_DeleteNode - 1.6\n");
	free(node->device.TvService);
	node->device.TvService = NULL;
	node->device.service_type_size = 0;
	printf("CtrlPoint_DeleteNode - 1.7\n");

	free(node);
	node = NULL;
	printf("CtrlPoint_DeleteNode - 2\n");

	return My_SUCCESS;
}
//------------------------------------chen------------------------------------//

int CtrlPoint_RemoveDevice(const char *UDN)
{
	struct My_DeviceNode *curdevnode;
	struct My_DeviceNode *prevdevnode;

	ithread_mutex_lock(&Device_List_Mutex);

	curdevnode = Global_Device_List;
	if (!curdevnode) {
		SampleUtil_Print(
			"WARNING: TvCtrlPointRemoveDevice: Device list empty\n");
	} else {
		if (0 == strcmp(curdevnode->device.UDN, UDN)) {
			Global_Device_List = curdevnode->next;
			CtrlPoint_DeleteNode(curdevnode);
		} else {
			prevdevnode = curdevnode;
			curdevnode = curdevnode->next;
			while (curdevnode) {
				if (strcmp(curdevnode->device.UDN, UDN) == 0) {
					prevdevnode->next = curdevnode->next;
					CtrlPoint_DeleteNode(curdevnode);
					break;
				}
				prevdevnode = curdevnode;
				curdevnode = curdevnode->next;
			}
		}
	}

	ithread_mutex_unlock(&Device_List_Mutex);

	return My_SUCCESS;
}


void CtrlPoint_HandleGetVar(
	const char *controlURL,
	const char *varName,
	const DOMString varValue)
{

	struct My_DeviceNode *tmpdevnode;
	int service;

	ithread_mutex_lock(&Device_List_Mutex);

	tmpdevnode = Global_Device_List;
	while (tmpdevnode) {
		for (service = 0; service < tmpdevnode->device.service_type_size; service++) {
			if (strcmp
			    (tmpdevnode->device.TvService[service].ControlURL,
			     controlURL) == 0) {
				SampleUtil_StateUpdate(varName, varValue,
						       tmpdevnode->device.UDN,
						       GET_VAR_COMPLETE);
				break;
			}
		}
		tmpdevnode = tmpdevnode->next;
	}

	ithread_mutex_unlock(&Device_List_Mutex);
}

void CtrlPoint_HandleSubscribeUpdate(
	const char *eventURL,
	const Upnp_SID sid,
	int timeout)
{
	struct My_DeviceNode *tmpdevnode;
	int service;

	ithread_mutex_lock(&Device_List_Mutex);

	tmpdevnode = Global_Device_List;
	while (tmpdevnode) {
		for (service = 0; service < tmpdevnode->device.service_type_size; service++) {
			if (strcmp
			    (tmpdevnode->device.TvService[service].EventURL,
			     eventURL) == 0) {
				SampleUtil_Print
				    ("Received Tv %s Event Renewal for eventURL %s\n",
				     tmpdevnode->device.TvService[service].ServiceType, eventURL);
				strcpy(tmpdevnode->device.TvService[service].SID, sid);
				break;
			}
		}

		tmpdevnode = tmpdevnode->next;
	}

	ithread_mutex_unlock(&Device_List_Mutex);

	return;
	timeout = timeout;
}

// 重点修改、存在大量已知变量数据、

void CtrlPoint_StateUpdate(struct My_DeviceNode *node, char *UDN, int Service, IXML_Document *ChangedVariables,
		   char **State)
{
	IXML_NodeList *properties;
	IXML_NodeList *variables;
	IXML_Element *property;
	IXML_Element *variable;
	long unsigned int length;
	long unsigned int length1;
	long unsigned int i;
	int j;
	char *tmpstate = NULL;
	printf("CtrlPoint_StateUpdate - 1\n");
	
	SampleUtil_Print("Tv State Update (service %d):\n", Service);
	/* Find all of the e:property tags in the document */
	properties = ixmlDocument_getElementsByTagName(ChangedVariables,
		"e:property");
	printf("CtrlPoint_StateUpdate - 2\n");

	if (properties) {
		printf("CtrlPoint_StateUpdate - 3\n");
		
		length = ixmlNodeList_length(properties);
		for (i = 0; i < length; i++) {
			/* Loop through each property change found */
			property = (IXML_Element *)ixmlNodeList_item(
				properties, i);
			/* For each variable name in the state table,
			 * check if this is a corresponding property change */
			for (j = 0; j < node->device.TvService[Service].Variable_Size; j++) {
				variables = ixmlElement_getElementsByTagName(
					property, node->device.TvService[Service].VariableName[j]);
				/* If a match is found, extract 
				 * the value, and update the state table */
				if (variables) {
					length1 = ixmlNodeList_length(variables);
					if (length1) {
						variable = (IXML_Element *)
							ixmlNodeList_item(variables, 0);
						tmpstate =
						    SampleUtil_GetElementValue(variable);
						if (tmpstate) {
							strcpy(State[j], tmpstate);
							SampleUtil_Print(
								" Variable Name: %s New Value:'%s'\n",
								node->device.TvService[Service].VariableName[j], State[j]);
						}
						if (tmpstate)
							free(tmpstate);
						tmpstate = NULL;
					}
					ixmlNodeList_free(variables);
					variables = NULL;
				}
			}
		}
		ixmlNodeList_free(properties);
	}
	printf("CtrlPoint_StateUpdate - 4\n");

	return;
	UDN = UDN;
}

void CtrlPoint_HandleEvent(
	const char *sid,
	int evntkey,
	IXML_Document *changes)
{
	struct My_DeviceNode *tmpdevnode;
	int service;
	printf("CtrlPoint_HandleEvent - 1 \n");

	ithread_mutex_lock(&Device_List_Mutex);
	printf("CtrlPoint_HandleEvent - 1.1 \n");
	
	tmpdevnode = Global_Device_List;
	while (tmpdevnode) {
		for (service = 0; service < tmpdevnode->device.service_type_size; ++service) {
			if (strcmp(tmpdevnode->device.TvService[service].SID, sid) ==  0) {
				SampleUtil_Print("Received Tv %s Event: %d for SID %s\n",
					tmpdevnode->device.TvService[service].ServiceType,
					evntkey,
					sid);
				printf("CtrlPoint_HandleEvent - 1\n");
				CtrlPoint_StateUpdate(tmpdevnode,
					tmpdevnode->device.UDN,
					service,
					changes,
					tmpdevnode->device.TvService[service].VariableStrVal);
				printf("CtrlPoint_HandleEvent - 2\n");
				break;
			}
		}
		tmpdevnode = tmpdevnode->next;
	}
	printf("CtrlPoint_HandleEvent - 2 \n");

	ithread_mutex_unlock(&Device_List_Mutex);
}


int CtrlPoint_CallbackEventHandler(Upnp_EventType EventType, void *Event, void *Cookie)
{
	/*int errCode = 0;*/

	SampleUtil_PrintEvent(EventType, Event);
	switch ( EventType ) {
	/* SSDP Stuff */
	case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
	case UPNP_DISCOVERY_SEARCH_RESULT: {
		struct Upnp_Discovery *d_event = (struct Upnp_Discovery *)Event;
		IXML_Document *DescDoc = NULL;
		int ret;

		if (d_event->ErrCode != UPNP_E_SUCCESS) {
			SampleUtil_Print("Error in Discovery Callback -- %d\n",
				d_event->ErrCode);
		}
		ret = UpnpDownloadXmlDoc(d_event->Location, &DescDoc);
		if (ret != UPNP_E_SUCCESS) {
			SampleUtil_Print("Error obtaining device description from %s -- error = %d\n",
				d_event->Location, ret);
		} else {
			// !!!!!!!!!!!!!!!!!!!!!!!!
			printf("CtrlPoint_CallbackEventHandler - 1\n");
			CtrlPoint_AddDevice(
				DescDoc, d_event->Location, d_event->Expires);
			printf("CtrlPoint_CallbackEventHandler - 2\n");

		}
		if (DescDoc) {
			ixmlDocument_free(DescDoc);
		}
		CtrlPoint_PrintList();
		break;
	}
	case UPNP_DISCOVERY_SEARCH_TIMEOUT:
		/* Nothing to do here... */
		break;
	case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE: {
		struct Upnp_Discovery *d_event = (struct Upnp_Discovery *)Event;

		if (d_event->ErrCode != UPNP_E_SUCCESS) {
			SampleUtil_Print("Error in Discovery ByeBye Callback -- %d\n",
					d_event->ErrCode);
		}
		SampleUtil_Print("Received ByeBye for Device: %s\n", d_event->DeviceId);
		// !!!!!!!!!!!!!!!!!!!!!!!!
		CtrlPoint_RemoveDevice(d_event->DeviceId);
		SampleUtil_Print("After byebye:\n");
		CtrlPoint_PrintList();
		break;
	}
	/* SOAP Stuff */
	case UPNP_CONTROL_ACTION_COMPLETE: {
		struct Upnp_Action_Complete *a_event = (struct Upnp_Action_Complete *)Event;

		if (a_event->ErrCode != UPNP_E_SUCCESS) {
			SampleUtil_Print("Error in  Action Complete Callback -- %d\n",
					a_event->ErrCode);
		}
		/* No need for any processing here, just print out results.
		 * Service state table updates are handled by events. */
		break;
	}
	case UPNP_CONTROL_GET_VAR_COMPLETE: {
		struct Upnp_State_Var_Complete *sv_event = (struct Upnp_State_Var_Complete *)Event;

		if (sv_event->ErrCode != UPNP_E_SUCCESS) {
			SampleUtil_Print("Error in Get Var Complete Callback -- %d\n",
					sv_event->ErrCode);
		} else {
			CtrlPoint_HandleGetVar(
				sv_event->CtrlUrl,
				sv_event->StateVarName,
				sv_event->CurrentVal);
		}
		break;
	}
	/* GENA Stuff */
	case UPNP_EVENT_RECEIVED: {
		struct Upnp_Event *e_event = (struct Upnp_Event *)Event;
		// !!!!!!!!!!!!!!!!!!!!!!!!
		printf("UPNP_EVENT_RECEIVED - 1 \n");
		CtrlPoint_HandleEvent(
			e_event->Sid,
			e_event->EventKey,
			e_event->ChangedVariables);
		printf("UPNP_EVENT_RECEIVED - 2 \n");
		
		break;
	}
	case UPNP_EVENT_SUBSCRIBE_COMPLETE:
	case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
	case UPNP_EVENT_RENEWAL_COMPLETE: {
		printf("UPNP_EVENT_RENEWAL_COMPLETE - 1 \n");
		
		struct Upnp_Event_Subscribe *es_event = (struct Upnp_Event_Subscribe *)Event;

		if (es_event->ErrCode != UPNP_E_SUCCESS) {
			SampleUtil_Print("Error in Event Subscribe Callback -- %d\n",
					es_event->ErrCode);
		} else {
			CtrlPoint_HandleSubscribeUpdate(
				es_event->PublisherUrl,
				es_event->Sid,
				es_event->TimeOut);
		}

		printf("UPNP_EVENT_RENEWAL_COMPLETE - 2 \n");
		
		break;
	}
	case UPNP_EVENT_AUTORENEWAL_FAILED:
	case UPNP_EVENT_SUBSCRIPTION_EXPIRED: {
		struct Upnp_Event_Subscribe *es_event = (struct Upnp_Event_Subscribe *)Event;
		int TimeOut = default_time;
		Upnp_SID newSID;
		int ret;

		ret = UpnpSubscribe(
			my_ctrlpt_handle,
			es_event->PublisherUrl,
			&TimeOut,
			newSID);
		if (ret == UPNP_E_SUCCESS) {
			SampleUtil_Print("Subscribed to EventURL with SID=%s\n", newSID);
			CtrlPoint_HandleSubscribeUpdate(
				es_event->PublisherUrl,
				newSID,
				TimeOut);
		} else {
			SampleUtil_Print("Error Subscribing to EventURL -- %d\n", ret);
		}
		break;
	}
	/* ignore these cases, since this is not a device */
	case UPNP_EVENT_SUBSCRIPTION_REQUEST:
	case UPNP_CONTROL_GET_VAR_REQUEST:
	case UPNP_CONTROL_ACTION_REQUEST:
		break;
	}

	return 0;
	Cookie = Cookie;
}

void CtrlPoint_VerifyTimeouts(int incr)
{
	struct My_DeviceNode *prevdevnode;
	struct My_DeviceNode *curdevnode;
	int ret;

	ithread_mutex_lock(&Device_List_Mutex);

	prevdevnode = NULL;
	curdevnode = Global_Device_List;
	while (curdevnode) {
		curdevnode->device.AdvrTimeOut -= incr;
		if (curdevnode->device.AdvrTimeOut <= 0) {

			if (Global_Device_List == curdevnode)
				Global_Device_List = curdevnode->next;
			else
				prevdevnode->next = curdevnode->next;
			CtrlPoint_DeleteNode(curdevnode);
			if (prevdevnode)
				curdevnode = prevdevnode->next;
			else
				curdevnode = Global_Device_List;
		} else {
			if (curdevnode->device.AdvrTimeOut < 2 * incr) {

				ret = UpnpSearchAsync(my_ctrlpt_handle, incr,
						      curdevnode->device.UDN,
						      NULL);
				if (ret != UPNP_E_SUCCESS)
					SampleUtil_Print
					    ("Error sending search request for Device UDN: %s -- err = %d\n",
					     curdevnode->device.UDN, ret);
			}
			prevdevnode = curdevnode;
			curdevnode = curdevnode->next;
		}
	}

	ithread_mutex_unlock(&Device_List_Mutex);
}

void *CtrlPoint_TimerLoop(void *args)
{
	/* how often to verify the timeouts, in seconds */
	int incr = 30;

	while (CtrlPoint_Timer_LoopRun) {
		isleep((unsigned int)incr);
		CtrlPoint_VerifyTimeouts(incr);
	}

	return NULL;
	args = args;
}

int CtrlPoint_Refresh(void)
{
	int rc;

	CtrlPoint_RemoveAll();
	/* Search for all devices of type tvdevice version 1,
	 * waiting for up to 5 seconds for the response */
//	rc = UpnpSearchAsync(my_ctrlpt_handle, 5, TvDeviceType, NULL);
//	if (UPNP_E_SUCCESS != rc) {
//		SampleUtil_Print("Error sending search request%d\n", rc);

//		return My_ERROR;
//	}

	return My_SUCCESS;
}

int CtrlPoint_Start(print_string printFunctionPtr, state_update updateFunctionPtr, int combo)
{
	ithread_t timer_thread;
	int rc;
	unsigned short port = 0;
	char *ip_address = NULL;

	SampleUtil_Initialize(printFunctionPtr);
	SampleUtil_RegisterUpdateFunction(updateFunctionPtr);

	ithread_mutex_init(&Device_List_Mutex, 0);

	SampleUtil_Print("Initializing UPnP Sdk with\n"
			 "\tipaddress = %s port = %u\n",
			 ip_address ? ip_address : "{NULL}", port);

	rc = UpnpInit(ip_address, port);
	if (rc != UPNP_E_SUCCESS) {
		SampleUtil_Print("WinCEStart: UpnpInit() Error: %d\n", rc);
		if (!combo) {
			UpnpFinish();

			return My_ERROR;
		}
	}
	if (!ip_address) {
		ip_address = UpnpGetServerIpAddress();
	}
	if (!port) {
		port = UpnpGetServerPort();
	}

	SampleUtil_Print("UPnP Initialized\n"
			 "\tipaddress = %s port = %u\n",
			 ip_address ? ip_address : "{NULL}", port);
	SampleUtil_Print("Registering Control Point\n");
	rc = UpnpRegisterClient(CtrlPoint_CallbackEventHandler,
				&my_ctrlpt_handle, &my_ctrlpt_handle);
	if (rc != UPNP_E_SUCCESS) {
		SampleUtil_Print("Error registering CP: %d\n", rc);
		UpnpFinish();

		return My_ERROR;
	}

	SampleUtil_Print("Control Point Registered\n");

	CtrlPoint_Refresh();

	/* start a timer thread */
	ithread_create(&timer_thread, NULL, CtrlPoint_TimerLoop, NULL);
	ithread_detach(timer_thread);

	return My_SUCCESS;
}

int CtrlPoint_RemoveAll(void)
{
	printf("CtrlPoint_RemoveAll - 1\n");

	struct My_DeviceNode *curdevnode, *next;

	ithread_mutex_lock(&Device_List_Mutex);
	printf("CtrlPoint_RemoveAll - 1.1\n");

	curdevnode = Global_Device_List;
	Global_Device_List = NULL;

	while (curdevnode) {
		next = curdevnode->next;
		printf("CtrlPoint_RemoveAll - 1.2\n");

		CtrlPoint_DeleteNode(curdevnode);
		printf("CtrlPoint_RemoveAll - 1.3\n");

		curdevnode = next;
	}
	printf("CtrlPoint_RemoveAll - 1.4\n");
	ithread_mutex_unlock(&Device_List_Mutex);
	printf("CtrlPoint_RemoveAll - 1.5\n");
	return My_SUCCESS;
}

int CtrlPoint_Stop(void)
{
	printf("CtrlPoint_Stop - 1\n");
	CtrlPoint_Timer_LoopRun = 0;
	printf("CtrlPoint_Stop - 1.1\n");
	CtrlPoint_RemoveAll();
	printf("CtrlPoint_Stop - 1.2\n");
	UpnpUnRegisterClient( my_ctrlpt_handle );
	printf("CtrlPoint_Stop - 1.3\n");
	UpnpFinish();
	printf("CtrlPoint_Stop - 1.4\n");
	SampleUtil_Finish();
	printf("CtrlPoint_Stop - 2\n");
	return My_SUCCESS;
}

int CtrlPoint_GetDevice(int devnum, struct My_DeviceNode **devnode)
{
	int count = devnum;
	struct My_DeviceNode *tmpdevnode = NULL;

	if (count)
		tmpdevnode = Global_Device_List;
	while (--count && tmpdevnode) {
		tmpdevnode = tmpdevnode->next;
	}
	if (!tmpdevnode) {
		SampleUtil_Print("Error finding TvDevice number -- %d\n",
				 devnum);
		return My_ERROR;
	}
	*devnode = tmpdevnode;

	return My_SUCCESS;
}

//!!!!!
int CtrlPoint_SendAction(
	int service,
	int devnum,
	const char *actionname,
	const char **param_name,
	char **param_val,
	int param_count)
{
	struct My_DeviceNode *devnode;
	IXML_Document *actionNode = NULL;
	int rc = My_SUCCESS;
	int param;

	ithread_mutex_lock(&Device_List_Mutex);

	rc = CtrlPoint_GetDevice(devnum, &devnode);
	if (My_SUCCESS == rc) {
		if (0 == param_count) {
			actionNode =
			    UpnpMakeAction(actionname, devnode->device.TvService[service].ServiceType,
					   0, NULL);
		} else {
			for (param = 0; param < param_count; param++) {
				if (UpnpAddToAction
				    (&actionNode, actionname,
				     devnode->device.TvService[service].ServiceType, param_name[param],
				     param_val[param]) != UPNP_E_SUCCESS) {
					SampleUtil_Print
					    ("ERROR: TvCtrlPointSendAction: Trying to add action param\n");
					/*return -1; // TBD - BAD! leaves mutex locked */
				}
			}
		}

		rc = UpnpSendActionAsync(my_ctrlpt_handle,
					 devnode->device.
					 TvService[service].ControlURL,
					 devnode->device.TvService[service].ServiceType, NULL,
					 actionNode,
					 CtrlPoint_CallbackEventHandler, NULL);

		if (rc != UPNP_E_SUCCESS) {
			SampleUtil_Print("Error in UpnpSendActionAsync -- %d\n",
					 rc);
			rc = My_ERROR;
		}
	}

	ithread_mutex_unlock(&Device_List_Mutex);

	if (actionNode)
		ixmlDocument_free(actionNode);

	return rc;
}


struct CmdLoop_Commands {
	const char *str;
	int cmdnum;
	int numargs;
	const char *args;
};

static struct CmdLoop_Commands CmdLoop_CmdList[] = {
	{"Help",          Cmd_PRTHELP,     1, ""},
	{"HelpFull",      Cmd_PRTFULLHELP, 1, ""},
	{"ListDev",       Cmd_LSTDEV,      1, ""},
	{"Refresh",       Cmd_REFRESH,     1, ""},
	{"PrintDev",      Cmd_PRTDEV,      2, "<devnum>"},
	{"PowerOn",       Cmd_POWON,       2, "<devnum>"},
	{"PowerOff",      Cmd_POWOFF,      2, "<devnum>"},
	{"SetChannel",    Cmd_SETCHAN,     3, "<devnum> <channel (int)>"},
	{"SetVolume",     Cmd_SETVOL,      3, "<devnum> <volume (int)>"},
	{"SetColor",      Cmd_SETCOL,      3, "<devnum> <color (int)>"},
	{"SetTint",       Cmd_SETTINT,     3, "<devnum> <tint (int)>"},
	{"SetContrast",   Cmd_SETCONT,     3, "<devnum> <contrast (int)>"},
	{"SetBrightness", Cmd_SETBRT,      3, "<devnum> <brightness (int)>"},
	{"CtrlAction",    Cmd_CTRLACTION,  2, "<devnum> <action (string)>"},
	{"PictAction",    Cmd_PICTACTION,  2, "<devnum> <action (string)>"},
	{"CtrlGetVar",    Cmd_CTRLGETVAR,  2, "<devnum> <varname (string)>"},
	{"PictGetVar",    Cmd_PICTGETVAR,  2, "<devnum> <varname (string)>"},
	{"SetURL",    Cmd_SETURL,  2, "<devnum> <varname (string)>"},
	{"Exit", Cmd_EXITCMD, 1, ""}
};


void CtrlPoint_PrintShortHelp(void)
{
	SampleUtil_Print(
		"Commands:\n"
		"  Help\n"
		"  HelpFull\n"
		"  ListDev\n"
		"  Refresh\n"
		"  PrintDev      <devnum>\n"
		"  PowerOn       <devnum>\n"
		"  PowerOff      <devnum>\n"
		"  SetChannel    <devnum> <channel>\n"
		"  SetVolume     <devnum> <volume>\n"
		"  SetColor      <devnum> <color>\n"
		"  SetTint       <devnum> <tint>\n"
		"  SetContrast   <devnum> <contrast>\n"
		"  SetBrightness <devnum> <brightness>\n"
		"  CtrlAction    <devnum> <action>\n"
		"  PictAction    <devnum> <action>\n"
		"  CtrlGetVar    <devnum> <varname>\n"
		"  PictGetVar    <devnum> <action>\n"
		"  SetURL		<devnum> <URL>\n"
		"  Exit\n");
}

void CtrlPoint_PrintLongHelp(void)
{
	SampleUtil_Print(
		"\n"
		"******************************\n"
		"* TV Control Point Help Info *\n"
		"******************************\n"
		"\n"
		"This sample control point application automatically searches\n"
		"for and subscribes to the services of television device emulator\n"
		"devices, described in the tvdevicedesc.xml description document.\n"
		"It also registers itself as a tv device.\n"
		"\n"
		"Commands:\n"
		"  Help\n"
		"       Print this help info.\n"
		"  ListDev\n"
		"       Print the current list of TV Device Emulators that this\n"
		"         control point is aware of.  Each device is preceded by a\n"
		"         device number which corresponds to the devnum argument of\n"
		"         commands listed below.\n"
		"  Refresh\n"
		"       Delete all of the devices from the device list and issue new\n"
		"         search request to rebuild the list from scratch.\n"
		"  PrintDev       <devnum>\n"
		"       Print the state table for the device <devnum>.\n"
		"         e.g., 'PrintDev 1' prints the state table for the first\n"
		"         device in the device list.\n"
		"  PowerOn        <devnum>\n"
		"       Sends the PowerOn action to the Control Service of\n"
		"         device <devnum>.\n"
		"  PowerOff       <devnum>\n"
		"       Sends the PowerOff action to the Control Service of\n"
		"         device <devnum>.\n"
		"  SetChannel     <devnum> <channel>\n"
		"       Sends the SetChannel action to the Control Service of\n"
		"         device <devnum>, requesting the channel to be changed\n"
		"         to <channel>.\n"
		"  SetVolume      <devnum> <volume>\n"
		"       Sends the SetVolume action to the Control Service of\n"
		"         device <devnum>, requesting the volume to be changed\n"
		"         to <volume>.\n"
		"  SetColor       <devnum> <color>\n"
		"       Sends the SetColor action to the Control Service of\n"
		"         device <devnum>, requesting the color to be changed\n"
		"         to <color>.\n"
		"  SetTint        <devnum> <tint>\n"
		"       Sends the SetTint action to the Control Service of\n"
		"         device <devnum>, requesting the tint to be changed\n"
		"         to <tint>.\n"
		"  SetContrast    <devnum> <contrast>\n"
		"       Sends the SetContrast action to the Control Service of\n"
		"         device <devnum>, requesting the contrast to be changed\n"
		"         to <contrast>.\n"
		"  SetBrightness  <devnum> <brightness>\n"
		"       Sends the SetBrightness action to the Control Service of\n"
		"         device <devnum>, requesting the brightness to be changed\n"
		"         to <brightness>.\n"
		"  CtrlAction     <devnum> <action>\n"
		"       Sends an action request specified by the string <action>\n"
		"         to the Control Service of device <devnum>.  This command\n"
		"         only works for actions that have no arguments.\n"
		"         (e.g., \"CtrlAction 1 IncreaseChannel\")\n"
		"  PictAction     <devnum> <action>\n"
		"       Sends an action request specified by the string <action>\n"
		"         to the Picture Service of device <devnum>.  This command\n"
		"         only works for actions that have no arguments.\n"
		"         (e.g., \"PictAction 1 DecreaseContrast\")\n"
		"  CtrlGetVar     <devnum> <varname>\n"
		"       Requests the value of a variable specified by the string <varname>\n"
		"         from the Control Service of device <devnum>.\n"
		"         (e.g., \"CtrlGetVar 1 Volume\")\n"
		"  PictGetVar     <devnum> <action>\n"
		"       Requests the value of a variable specified by the string <varname>\n"
		"         from the Picture Service of device <devnum>.\n"
		"         (e.g., \"PictGetVar 1 Tint\")\n"
		"  SetURL		<devnum> <URL>\n"
		"  Exit\n"
		"       Exits the control point application.\n");
}

int CtrlPoint_SendPowerOn(int devnum)
{
	return CtrlPoint_SendAction(
		Service_Control, devnum, "PowerOn", NULL, NULL, 0);
}

int CtrlPoint_SendPowerOff(int devnum)
{
	return CtrlPoint_SendAction(
		Service_Control, devnum, "PowerOff", NULL, NULL, 0);
}

int CtrlPoint_SendActionNumericArg(int devnum, int service,
	const char *actionName, const char *paramName, int paramValue)
{
	char param_val_a[50];
	char *param_val = param_val_a;

	sprintf(param_val_a, "%d", paramValue);
	return CtrlPoint_SendAction(
		service, devnum, actionName, &paramName,
		&param_val, 1);
}

int CtrlPoint_SendSetChannel(int devnum, int channel)
{
	return CtrlPoint_SendActionNumericArg(
		devnum, Service_Control, "SetChannel", "Channel", channel);
}

int CtrlPoint_SendSetVolume(int devnum, int volume)
{
	return CtrlPoint_SendActionNumericArg(
		devnum, Service_Control, "SetVolume", "Volume", volume);
}


int CtrlPoint_SendSetColor(int devnum, int color)
{
	return CtrlPoint_SendActionNumericArg(
		devnum, Service_Picture, "SetColor", "Color", color);
}

int CtrlPoint_SendSetTint(int devnum, int tint)
{
	return CtrlPoint_SendActionNumericArg(
		devnum, Service_Picture, "SetTint", "Tint", tint);
}


int CtrlPoint_SendSetContrast(int devnum, int contrast)
{
	return CtrlPoint_SendActionNumericArg(
		devnum, Service_Picture, "SetContrast", "Contrast",
		contrast);
}

int CtrlPoint_SendSetBrightness(int devnum, int brightness)
{
	return CtrlPoint_SendActionNumericArg(
		devnum, Service_Picture, "SetBrightness", "Brightness",
		brightness);
}

int CtrlPoint_GetVar(int service, int devnum, const char *varname)
{
	struct My_DeviceNode *devnode;
	int rc;

	ithread_mutex_lock(&Device_List_Mutex);

	rc = CtrlPoint_GetDevice(devnum, &devnode);

	if (My_SUCCESS == rc) {
		rc = UpnpGetServiceVarStatusAsync(
			my_ctrlpt_handle,
			devnode->device.TvService[service].ControlURL,
			varname,
			CtrlPoint_CallbackEventHandler,
			NULL);
		if (rc != UPNP_E_SUCCESS) {
			SampleUtil_Print(
				"Error in UpnpGetServiceVarStatusAsync -- %d\n",
				rc);
			rc = My_ERROR;
		}
	}

	ithread_mutex_unlock(&Device_List_Mutex);

	return rc;
}


int CtrlPoint_SetURL(int service, int devnum, const char *url)
{
	char *actionName = "ReceiveAudioURL";
	char *paramName = "AudioURL";

	char param_val_a[50];
	char *param_val = param_val_a;
	sprintf(param_val_a, "%s", url);
	
	return CtrlPoint_SendAction(
		service, devnum, actionName, &paramName,
		&param_val, 1);
}


int CtrlPoint_PrintDevice(int devnum)
{
	struct My_DeviceNode *tmpdevnode;
	int i = 0, service, var;
	char spacer[15];

	if (devnum <= 0) {
		SampleUtil_Print(
			"Error in CtrlPoint_PrintDevice: "
			"invalid devnum = %d\n",
			devnum);
		return My_ERROR;
	}

	ithread_mutex_lock(&Device_List_Mutex);

	SampleUtil_Print("CtrlPoint_PrintDevice:\n");
	tmpdevnode = Global_Device_List;
	while (tmpdevnode) {
		i++;
		if (i == devnum)
			break;
		tmpdevnode = tmpdevnode->next;
	}
	if (!tmpdevnode) {
		SampleUtil_Print(
			"Error in CtrlPoint_PrintDevice: "
			"invalid devnum = %d  --  actual device count = %d\n",
			devnum, i);
	} else {
		SampleUtil_Print(
			"  TvDevice -- %d\n"
			"    |                  \n"
			"    +- UDN        = %s\n"
			"    +- DescDocURL     = %s\n"
			"    +- FriendlyName   = %s\n"
			"    +- PresURL        = %s\n"
			"    +- Adver. TimeOut = %d\n",
			devnum,
			tmpdevnode->device.UDN,
			tmpdevnode->device.DescDocURL,
			tmpdevnode->device.FriendlyName,
			tmpdevnode->device.PresURL,
			tmpdevnode->device.AdvrTimeOut);
		for (service = 0; service < tmpdevnode->device.service_type_size; service++) {
			if (service < tmpdevnode->device.service_type_size - 1)
				sprintf(spacer, "    |    ");
			else
				sprintf(spacer, "         ");
			SampleUtil_Print(
				"    |                  \n"
				"    +- Tv %s Service\n"
				"%s+- ServiceId       = %s\n"
				"%s+- ServiceType     = %s\n"
				"%s+- EventURL        = %s\n"
				"%s+- ControlURL      = %s\n"
				"%s+- SID             = %s\n"
				"%s+- ServiceStateTable\n",
				tmpdevnode->device.TvService[service].ServiceType,
				spacer,
				tmpdevnode->device.TvService[service].ServiceId,
				spacer,
				tmpdevnode->device.TvService[service].ServiceType,
				spacer,
				tmpdevnode->device.TvService[service].EventURL,
				spacer,
				tmpdevnode->device.TvService[service].ControlURL,
				spacer,
				tmpdevnode->device.TvService[service].SID,
				spacer);
			for (var = 0; var < tmpdevnode->device.TvService[service].Variable_Size; var++) {
				SampleUtil_Print(
					"%s     +- %-10s = %s\n",
					spacer,
					tmpdevnode->device.TvService[service].VariableName[var],
					tmpdevnode->device.TvService[service].VariableStrVal[var]);
			}
		}
	}
	SampleUtil_Print("\n");
	ithread_mutex_unlock(&Device_List_Mutex);

	return My_SUCCESS;
}

int CtrlPoint_PrintList()
{
	struct My_DeviceNode *tmpdevnode;
	int i = 0;

	ithread_mutex_lock(&Device_List_Mutex);

	SampleUtil_Print("CtrlPoint_PrintList:\n");
	tmpdevnode = Global_Device_List;
	while (tmpdevnode) {
		SampleUtil_Print(" %3d -- %s\n", ++i, tmpdevnode->device.UDN);
		tmpdevnode = tmpdevnode->next;
	}
	SampleUtil_Print("\n");
	ithread_mutex_unlock(&Device_List_Mutex);

	return My_SUCCESS;
}

int CtrlPoint_ProcessCommand(char *cmdline)
{
	char cmd[100];
	char strarg[100];
	int arg_val_err = -99999;
	int arg1 = arg_val_err;
	int arg2 = arg_val_err;
	int cmdnum = -1;
	int numofcmds = (sizeof CmdLoop_CmdList) / sizeof(struct CmdLoop_Commands);
	int cmdfound = 0;
	int i;
	int rc;
	int invalidargs = 0;
	int validargs;

	validargs = sscanf(cmdline, "%s %d %d", cmd, &arg1, &arg2);
	for (i = 0; i < numofcmds; ++i) {
		if (strcasecmp(cmd, CmdLoop_CmdList[i].str ) == 0) {
			cmdnum = CmdLoop_CmdList[i].cmdnum;
			cmdfound++;
			if (validargs != CmdLoop_CmdList[i].numargs)
				invalidargs++;
			break;
		}
	}
	if (!cmdfound) {
		SampleUtil_Print("Command not found; try 'Help'\n");
		return My_SUCCESS;
	}
	if (invalidargs) {
		SampleUtil_Print("Invalid arguments; try 'Help'\n");
		return My_SUCCESS;
	}
	switch (cmdnum) {
	case Cmd_PRTHELP:
		CtrlPoint_PrintShortHelp();
		break;
	case Cmd_PRTFULLHELP:
		CtrlPoint_PrintLongHelp();
		break;
	case Cmd_POWON:
		CtrlPoint_SendPowerOn(arg1);
		break;
	case Cmd_POWOFF:
		CtrlPoint_SendPowerOff(arg1);
		break;
	case Cmd_SETCHAN:
		CtrlPoint_SendSetChannel(arg1, arg2);
		break;
	case Cmd_SETVOL:
		CtrlPoint_SendSetVolume(arg1, arg2);
		break;
	case Cmd_SETCOL:
		CtrlPoint_SendSetColor(arg1, arg2);
		break;
	case Cmd_SETTINT:
		CtrlPoint_SendSetTint(arg1, arg2);
		break;
	case Cmd_SETCONT:
		CtrlPoint_SendSetContrast(arg1, arg2);
		break;
	case Cmd_SETBRT:
		CtrlPoint_SendSetBrightness(arg1, arg2);
		break;
	case Cmd_CTRLACTION:
		/* re-parse commandline since second arg is string. */
		validargs = sscanf(cmdline, "%s %d %s", cmd, &arg1, strarg);
		if (validargs == 3)
			CtrlPoint_SendAction(Service_Control, arg1, strarg,
				NULL, NULL, 0);
		else
			invalidargs++;
		break;
	case Cmd_PICTACTION:
		/* re-parse commandline since second arg is string. */
		validargs = sscanf(cmdline, "%s %d %s", cmd, &arg1, strarg);
		if (validargs == 3)
			CtrlPoint_SendAction(Service_Picture, arg1, strarg,
				NULL, NULL, 0);
		else
			invalidargs++;
		break;
	case Cmd_CTRLGETVAR:
		/* re-parse commandline since second arg is string. */
		validargs = sscanf(cmdline, "%s %d %s", cmd, &arg1, strarg);
		if (validargs == 3)
			CtrlPoint_GetVar(Service_Control, arg1, strarg);
		else
			invalidargs++;
		break;
	case Cmd_PICTGETVAR:
		/* re-parse commandline since second arg is string. */
		validargs = sscanf(cmdline, "%s %d %s", cmd, &arg1, strarg);
		if (validargs == 3)
			CtrlPoint_GetVar(Service_Picture, arg1, strarg);
		else
			invalidargs++;
		break;
	case Cmd_PRTDEV:
		CtrlPoint_PrintDevice(arg1);
		break;
	case Cmd_LSTDEV:
		CtrlPoint_PrintList();
		break;
	case Cmd_REFRESH:
		CtrlPoint_Refresh();
		break;
	case Cmd_EXITCMD:
		rc = CtrlPoint_Stop();
		exit(rc);
		break;
 //------------------------------------chen------------------------------------//	
	case Cmd_SETURL: {
		/* re-parse commandline since second arg is string. */
		validargs = sscanf(cmdline, "%s %d %s", cmd, &arg1, strarg);
		if (validargs == 3)
			CtrlPoint_SetURL(0, arg1, strarg);
		else
			invalidargs++;
		break;
	}
 //------------------------------------chen------------------------------------//	
 
	default:
		SampleUtil_Print("Command not implemented; see 'Help'\n");
		break;
	}
	if(invalidargs)
		SampleUtil_Print("Invalid args in command; see 'Help'\n");

	return My_SUCCESS;
}


void *CtrlPoint_CommandLoop(void *args)
{
	char cmdline[100];

	while (1) {
		SampleUtil_Print("\n>> ");
		char *s = fgets(cmdline, 100, stdin);
		if (!s)
			break;
		CtrlPoint_ProcessCommand(cmdline);
	}

	return NULL;
	args = args;
}




int My_Ctrlpt_Test(int argc, char **argv) {
	int rc;
	ithread_t cmdloop_thread;

	int sig;
	sigset_t sigs_to_catch;

	int code;

	rc = CtrlPoint_Start(linux_print, NULL, 0);
	if (rc != My_SUCCESS) {
		SampleUtil_Print("Error starting UPnP TV Control Point\n");
		return rc;
	}
	/* start a command loop thread */
	code = ithread_create(&cmdloop_thread, NULL, CtrlPoint_CommandLoop, NULL);
	if (code !=  0) {
		return UPNP_E_INTERNAL_ERROR;
	}

	/* Catch Ctrl-C and properly shutdown */
	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGINT);
	sigwait(&sigs_to_catch, &sig);
	SampleUtil_Print("Shutting down on signal %d...\n", sig);

	rc = CtrlPoint_Stop();

	return rc;
	argc = argc;
	argv = argv;
}

