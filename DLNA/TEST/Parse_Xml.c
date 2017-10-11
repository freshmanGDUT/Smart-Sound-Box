#include "Parse_Xml.h"
void Parse_StateVariable(xmlDocPtr doc, xmlNodePtr cur) {
	//遍历处理根节点的每一个子节点
	cur = cur->xmlChildrenNode;
	while (NULL != cur) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"name"))) {
			xmlChar *key;
			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			printf("name: %s !\n", key);
			xmlFree(key);	
	    	}
	    	cur = cur->next;
	}
}

void Parse_ServiceStateTable(xmlDocPtr doc, xmlNodePtr cur) {
	//遍历处理根节点的每一个子节点
	cur = cur->xmlChildrenNode;
	while (NULL != cur) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"stateVariable"))) {
			Parse_StateVariable(doc, cur);
	    	}
	    	cur = cur->next;
	}
}

void Parse_Xml(char *file_name) {
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
			Parse_ServiceStateTable(doc, cur);
	    	}
	    	cur = cur->next;
	}	
	xmlFreeDoc(doc);	
}


