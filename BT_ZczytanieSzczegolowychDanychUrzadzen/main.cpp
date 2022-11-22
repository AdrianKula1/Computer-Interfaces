#include <iostream>
#include <assert.h>
#include <winsock2.h>
#include "Ws2bth.h"

using namespace std;

typedef struct {
     char bthDevName[256];
     BTH_ADDR bthAddr;
} BT_DEVICE;


template <class T>
inline void releaseMemory(T &x){
     assert(x != NULL);
     delete x;
     x = NULL;
}


void showError(){
     LPVOID lpMsgBuf;
     FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,NULL, WSAGetLastError(), 0, (LPTSTR) &lpMsgBuf, 0, NULL );
     fprintf(stderr, "\n%s\n", lpMsgBuf);
     free(lpMsgBuf);
     cin.get();
}


int main(){
     BT_DEVICE *bthDev;
     SOCKADDR_BTH *socAddrBTH;
     BTH_ADDR btAddr;
     WORD wVersionRequested;
     WSADATA wsaData;
     wVersionRequested = MAKEWORD(2,2);

     if(WSAStartup(wVersionRequested, &wsaData) != 0){
        showError();
     }

     WSAQUERYSET *lpqsRestrictions = new WSAQUERYSET[sizeof(WSAQUERYSET)];
     memset(lpqsRestrictions, 0, sizeof(WSAQUERYSET));
     lpqsRestrictions->dwSize = sizeof(WSAQUERYSET);
     lpqsRestrictions->dwNameSpace = NS_BTH;
     DWORD dwControlFlags = LUP_CONTAINERS;
     dwControlFlags |= LUP_FLUSHCACHE | LUP_RETURN_NAME | LUP_RETURN_ADDR;
     HANDLE hLookup;

     if(SOCKET_ERROR == WSALookupServiceBegin(lpqsRestrictions, dwControlFlags, &hLookup)){
         WSACleanup();
         return 0;
     }

     BOOL searchResult = FALSE;
     while(!searchResult){
         if(NO_ERROR == WSALookupServiceNext(hLookup,dwControlFlags, &lpqsRestrictions->dwSize, lpqsRestrictions)){
             bthDev = new BT_DEVICE[sizeof(BT_DEVICE)];
             memset(bthDev,0,sizeof(bthDev));
             strcpy (bthDev->bthDevName, lpqsRestrictions->lpszServiceInstanceName);
             printf("\t Nazwa:%s", bthDev->bthDevName);
             socAddrBTH = (SOCKADDR_BTH *)lpqsRestrictions->lpcsaBuffer->RemoteAddr.lpSockaddr;
             bthDev->bthAddr = socAddrBTH->btAddr;
             printf("\t Adres:%X\n", bthDev->bthAddr);
         }else{
             int WSAerror = WSAGetLastError();
             if(WSAerror == WSAEFAULT){
                 releaseMemory(lpqsRestrictions);
                 lpqsRestrictions = new WSAQUERYSET[sizeof(WSAQUERYSET)];
             }else if(WSAerror == WSA_E_NO_MORE){
                searchResult = TRUE;
             }
             else {
                 showError();
                 searchResult = TRUE;
             }
         }
     }
     
     releaseMemory(bthDev);
     WSALookupServiceEnd(hLookup);
     releaseMemory(lpqsRestrictions);
     WSACleanup();
     system("PAUSE");
     return 0;
}
