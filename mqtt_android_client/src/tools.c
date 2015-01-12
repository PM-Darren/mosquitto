#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <poll.h>
#include <sys/utsname.h>
#include <sys/time.h>
#include <signal.h>


#define LOG_TAG "mqtt-tools"
#include "cutils/log.h"
#include "cutils/memory.h"
#include "cutils/misc.h"
#include "cutils/properties.h"

struct hwaddr_t {
    int addr[6];
};


int str2hwaddr(char* str, struct hwaddr_t * hwaddr)
{
    char mac_char[32] = {0};
    int temp[6];
    char * pp;
    int i;
    memset(&temp, 0, sizeof(struct hwaddr_t));
    if(str == NULL) return -1;

    strcpy(mac_char, str);
    pp = strtok(mac_char, ":");

    sscanf(pp,"%x", &(temp[5]));
    hwaddr->addr[5] = temp[5];

    for (i=4; i>=0; i--) {
        pp=strtok(NULL,":");            
        sscanf(pp,"%x",&(temp[i]));
        hwaddr->addr[i] = temp[i];
    }

    return 0;
}


int getWifiMac(char * mac) {
    FILE *fp = fopen("/sys/class/net/wlan0/address", "r");

    if (mac == NULL) return -1;
    if (fp == NULL) return -1;

    fread(mac, 1, 32, fp);
    mac[strlen(mac)-1] = '\0';
    // ALOGD("/sys/class/net/wlan0/address=%s", mac);
    fclose(fp);
    return 0;
}

int randomIdByMac(void) {
    struct hwaddr_t hwaddr;
    char addr[32] = {0};
    int temp_int;
    int id;

    memset(addr, 0, 32);

    getWifiMac(addr);
    str2hwaddr(addr, &hwaddr);

    int i = 0;
    for (i=0; i<6; i++) {
        srand(hwaddr.addr[i]);
        id += rand();
        // id += rand() * time(0);
    }
    // ALOGD("id: 0x%x", id);

    id &= 0xFFFFFF;

    id += (id < 0x100000) ? 0x100000 : 0;

    return id;
}
// int getID(char * id);

// int main(int argc, char const *argv[])
// {

//     char id[16];
 
//     if (getID(id) == 0)
//     {
//         printf("\n\n*****id is %s\n", id);
//     }
    
//     return 0;
// }

/*


  
/data/misc/wifi/hostapd.conf 

interface=p2p0
driver=nl80211
ctrl_interface=/data/misc/wifi/hostapd
ssid=Coocaa-23730
channel=1
ieee80211n=1
hw_mode=g
ignore_broadcast_ssid=0
wpa=2
rsn_pairwise=CCMP
wpa_psk=c7b01d6321db52b00fec06e0b24980d5da4c6510b8363f84f0230fee058e4e24
 */

int getID(char * id)
{
    FILE *fp = fopen("/data/misc/wifi/hostapd.conf", "r");
    char data[128];
    char * pp;
    int i;

    if (data == NULL) return -1;
    if (fp == NULL) return -1;
    // printf("ok\n");
    fread(data, 127, 1, fp);
    data[127] = '\0';
    fclose(fp);
    // printf("%s\n", data);

    pp = strtok(data, "\n");
    // printf("%s\n", pp);

    for (i = 0; i < 3; ++i)
    {
        pp=strtok(NULL,"\n");
        // printf("id:%d   %s\n", i,pp);  
    }
    strcpy(id,pp+5);

    
    return 0;

}