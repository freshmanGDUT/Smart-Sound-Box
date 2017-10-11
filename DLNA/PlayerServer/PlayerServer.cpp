#include <stdio.h>
#include <iostream>
#include "upnp.h"
#include "PlayerServer.h"
#include "GlobalDefine.h"

using namespace std;


int PlayerDeviceStop(void) {
    UpnpUnRegisterRootDevice(device_handle);
    UpnpFinish();
    SampleUtil_Finish();
    ithread_mutex_destroy(&PlayerDevMutex);

    return UPNP_E_SUCCESS;
}

/*The Player Device Command Loop
 */
void *PlayerDeviceCommandLoop(void *args) {
    int stoploop = 0;
    char cmdline[100];
    char cmd[100];

    while (!stoploop) {
        sprintf(cmdline, " ");
        sprintf(cmd, " ");
        SampleUtil_Print("\n>> ");
        /* Get a command line */
        char *s = fgets(cmdline, 100, stdin);
        if (!s)
            break;
        sscanf(cmdline, "%s", cmd);
        if (strcasecmp(cmd, "exit") == 0) {
            SampleUtil_Print("Shutting down...\n");
            PlayerDeviceStop();
            exit(0);
        } else {
            SampleUtil_Print("\n   Unknown command: %s\n\n", cmd);
            SampleUtil_Print("   Valid Commands:\n" "     Exit\n\n");
        }
    }
    return NULL;
    args = args;
}


/* Player main thread
 */
int PlayerDLNAMain(int argc, char **argv) {
    /*Initialize the thread lock*/
    ithread_mutex_init(&PlayerDevMutex, NULL);
    /*Define some values*/
    char *ip_address = NULL;
    unsigned short port = 0;
    char desc_doc_url[DESC_URL_SIZE];
    int ret = 0;
    /*Initialize some func tools, like print and parse xml*/
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
    ret = DeviceStateTableInit(desc_doc_url);
    SampleUtil_Print("State Table Initialized\n");
    if (ret != UPNP_E_SUCCESS) {
      exit(3);
    } 
    /*Broadcast the device*/
    ret = DeviceAdvertisement();
    if (ret != UPNP_E_SUCCESS) {
      exit(4);
    }
    return ret;
}

int main(int argc, char **argv) {
    int rc;
    ithread_t player_device_loop_thread;
    int code;
    int sig;
    sigset_t sigs_to_catch;
    rc = PlayerDLNAMain(argc, argv);
    if (rc != UPNP_E_SUCCESS) {
        return rc;
    }
    /* start a command loop thread */
    code = ithread_create(&player_device_loop_thread, NULL, PlayerDeviceCommandLoop, NULL);
    if (code !=  0) {
        return UPNP_E_INTERNAL_ERROR;
    }
    ithread_join(player_device_loop_thread, NULL);
    /* Catch Ctrl-C and properly shutdown */
    sigemptyset(&sigs_to_catch);
    sigaddset(&sigs_to_catch, SIGINT);
    sigwait(&sigs_to_catch, &sig);
    SampleUtil_Print("Shutting down on signal %d...\n", sig);
    /*Stop the Player device*/
    rc = PlayerDeviceStop();
}
