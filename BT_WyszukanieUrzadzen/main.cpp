#include <iostream>
#include <assert.h>
#pragma option push -a1
#include <winsock2.h>
#include "Ws2bth.h"
#pragma option pop
#pragma comment(lib, "ws2_32.lib")
using namespace std;


template <class T>
inline void releaseMemory(T &x)
{
     assert(x!= NULL);
     delete x;
     x = NULL;
}


void showError(){
     LPVOID lpMsgBuf;
     FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, WSAGetLastError(), 0, (LPTSTR)&lpMsgBuf, 0, NULL );
     fprintf(stderr, "\n%s\n", lpMsgBuf);
     free(lpMsgBuf);
     cin.get();
}

int main(){
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2,2);

    if(WSAStartup(wVersionRequested, &wsaData) != 0){//Ustalamy wersje biblioteki jak ma byæ odwzorowana dla naszego procesu
        showError();
    }

    WSAQUERYSET *lpqsRestrictions = new WSAQUERYSET[sizeof(WSAQUERYSET)];
    memset(lpqsRestrictions, 0, sizeof(WSAQUERYSET));
    lpqsRestrictions->dwSize = sizeof(WSAQUERYSET);
    lpqsRestrictions->dwNameSpace = NS_BTH;
    DWORD dwControlFlags = LUP_CONTAINERS;
    dwControlFlags |= LUP_FLUSHCACHE | LUP_RETURN_NAME | LUP_RETURN_ADDR;
    HANDLE hLookup;

    if(SOCKET_ERROR == WSALookupServiceBegin(lpqsRestrictions, dwControlFlags, &hLookup)){//Rozpoczyna proces wykrywania urzadzeñ
        WSACleanup();
        return 0;
    }

    BOOL searchResult = FALSE;

    while(!searchResult){
        if(NO_ERROR == WSALookupServiceNext(hLookup, dwControlFlags, &lpqsRestrictions->dwSize, lpqsRestrictions)){//Uzyskanie informacji o urzadzeniu
            char buffer[40] = {0};
            DWORD bufLength = sizeof(buffer);
            WSAAddressToString(lpqsRestrictions->lpcsaBuffer->RemoteAddr.lpSockaddr, sizeof(SOCKADDR_BTH), NULL, buffer, &bufLength);//Zmiana adresu na stringa
            printf("Address: %s , Device: %s\n", buffer, lpqsRestrictions->lpszServiceInstanceName);
         }else{
            int WSAerror = WSAGetLastError();
            if(WSAerror == WSAEFAULT) {
                releaseMemory(lpqsRestrictions);
                lpqsRestrictions = new
                WSAQUERYSET[sizeof(WSAQUERYSET)];
            }else if(WSAerror == WSA_E_NO_MORE) {
                searchResult = TRUE;
            }
            else{
                showError();
                searchResult = TRUE;
            }
        }
    }

    WSALookupServiceEnd(hLookup);
    releaseMemory(lpqsRestrictions);
    WSACleanup();
    system("PAUSE");
    return 0;
}
