#include <stdio.h>
#include <stdarg.h>

#include "upnp/upnp.h"
#include "upnp/ithread.h"
#include "upnp/ixml.h"

#include "sample_util.h"
#include "tv_device.h"
#include "tv_ctrlpt.h"

#include "Parse_Xml.h"

#include "my_ctrlpt.h"

int test_1(int argc, char *argv[]) {
	int rc;
	ithread_t cmdloop_thread;

	int sig;
	sigset_t sigs_to_catch;

	int code;

	rc = device_main(argc, argv);
	if (rc != UPNP_E_SUCCESS) {
		return rc;
	}

	/* start a command loop thread */
	code = ithread_create(&cmdloop_thread, NULL, TvDeviceCommandLoop, NULL);
	if (code !=  0) {
		return UPNP_E_INTERNAL_ERROR;
	}

	/* Catch Ctrl-C and properly shutdown */
	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGINT);
	sigwait(&sigs_to_catch, &sig);
	SampleUtil_Print("Shutting down on signal %d...\n", sig);

	rc = TvDeviceStop();

	return rc;
}

int func(Upnp_EventType EventType, void *Event, void *Cookie) {
	return 0;
}

int test_2(int argc, char *argv[]) {
	char *DescDocURL = "./web/tvdevicedesc.xml";
	IXML_Document *DescDoc = NULL;
	IXML_NodeList *node_list = NULL;


	int device_handle = -1;
	char *ip_address = NULL;
	unsigned short port = 0;
	int ret = UPNP_E_SUCCESS;
	char desc_doc_url[200];

	if (UPNP_E_SUCCESS != UpnpInit(ip_address, port)) {
		printf("UPNP_E_SUCCESS != UpnpInit(ip_address, port) !\n");
		UpnpFinish();
		return 0;
	}
	ip_address = UpnpGetServerIpAddress();
	port = UpnpGetServerPort();

	char *desc_doc_name = "tvdevicedesc.xml";
	char *web_dir_path = "./web";

	snprintf(desc_doc_url, 200, "http://%s:%d/%s", ip_address,
		 port, desc_doc_name);
	if (UPNP_E_SUCCESS != UpnpSetWebServerRootDir(web_dir_path)) {
		printf("UPNP_E_SUCCESS != UpnpSetWebServerRootDir(web_dir_path) !\n");
		UpnpFinish();
		return 0;
	}

	if (UPNP_E_SUCCESS != UpnpRegisterRootDevice(desc_doc_url, func, &device_handle, &device_handle)) {
		printf("UPNP_E_SUCCESS != UpnpRegisterRootDevice(desc_doc_url, NULL, &device_handle, &device_handle) !\n");
		UpnpFinish();
		return 0;
	}


	if (UPNP_E_SUCCESS != UpnpDownloadXmlDoc(desc_doc_url, &DescDoc)) {
		printf("TvDeviceStateTableInit -- Error Parsing %s !\n", desc_doc_url);
		return 0;
	}
	node_list = ixmlDocument_getElementsByTagName(DescDoc, "SCPDURL");
	if (NULL == node_list) {
		ixmlDocument_free(DescDoc);
		printf("node_list is NULL !\n");		
		return 0;
	}
	unsigned int i = 0; 


	for (i = 0; i < ixmlNodeList_length(node_list); i++) {
		IXML_Node *cur1 = ixmlNodeList_item(node_list, i);
		if (NULL == cur1) {
			printf("cur1 is NULL !\n");
			break;
		}
		IXML_Node *cur2 = ixmlNode_getFirstChild(cur1);
		if (NULL == cur2) {
			printf("cur2 is NULL !\n");
			break;
		}		
		char *tmp = strdup(ixmlNode_getNodeValue(cur2));
		if (NULL != tmp) {
			char desc_doc_url1[200];
			IXML_Document *DescDoc1 = NULL;
			printf("tmp is %s !\n", tmp);
			snprintf(desc_doc_url1, 200, "http://%s:%d%s", ip_address, port, tmp);
			printf("desc_doc_url1 is %s !\n", desc_doc_url1);	
			if (UPNP_E_SUCCESS != UpnpDownloadXmlDoc(desc_doc_url1, &DescDoc1)) {
				printf("TvDeviceStateTableInit -- Error Parsing %s !\n", desc_doc_url);
			} else {
				IXML_NodeList *node_list1 = NULL;
				node_list1 = ixmlDocument_getElementsByTagName(DescDoc1, "dataType");
				if (NULL != node_list1) {
					int j = 0;
					for (j = 0; j < ixmlNodeList_length(node_list1); j++) {
						IXML_Node *cur3 = ixmlNodeList_item(node_list1, j);
						if (NULL == cur3) {
							printf("cur3 is NULL !\n");
							break;
						}
						IXML_Node *cur4 = ixmlNode_getFirstChild(cur3);
						if (NULL == cur4) {
							printf("cur4 is NULL !\n");
							break;
						}		
						char *tmp1 = strdup(ixmlNode_getNodeValue(cur4));
						if (NULL != tmp1) {
							printf("tmp1 is %s !\n", tmp1);
							free(tmp1);
							tmp1 = NULL;
						}
					}
					ixmlNodeList_free(node_list1);
				}
				ixmlDocument_free(DescDoc1);			
			}
			free(tmp);
			tmp = NULL;
			Parse_Xml(desc_doc_url1);
		} else {
			printf("tmp is NULL !\n");
		}
	}

	
	ixmlNodeList_free(node_list);
	ixmlDocument_free(DescDoc);

	return 0;
}


int test_3(int argc, char *argv[]) {
	int rc;
	ithread_t cmdloop_thread;
#ifdef WIN32
#else
	int sig;
	sigset_t sigs_to_catch;
#endif
	int code;

	rc = TvCtrlPointStart(linux_print, NULL, 0);
	if (rc != TV_SUCCESS) {
		SampleUtil_Print("Error starting UPnP TV Control Point\n");
		return rc;
	}
	/* start a command loop thread */
	code = ithread_create(&cmdloop_thread, NULL, TvCtrlPointCommandLoop, NULL);
	if (code !=  0) {
		return UPNP_E_INTERNAL_ERROR;
	}
#ifdef WIN32
	ithread_join(cmdloop_thread, NULL);
#else
	/* Catch Ctrl-C and properly shutdown */
	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGINT);
	sigwait(&sigs_to_catch, &sig);
	SampleUtil_Print("Shutting down on signal %d...\n", sig);
#endif
	rc = TvCtrlPointStop();

	return rc;
	argc = argc;
	argv = argv;
}

typedef char TYPE[44];

int test_4(int argc, char *argv[]) {
	TYPE *data;
	
	int size0 = sizeof(data);
	int size1 = sizeof(TYPE);
	int size2 = sizeof(*data);
	int size3 = sizeof(**data);
	printf("size0 = %d\n", size0);
	printf("size1 = %d\n", size1);
	printf("size2 = %d\n", size2);
	printf("size3 = %d\n", size3);
	return 0;
}



int main(int argc, char *argv[]) {
	My_Ctrlpt_Test(argc, argv);

	return 0;
}
