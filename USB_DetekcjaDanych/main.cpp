#include <windows.h>
#include <setupapi.h>
#include <assert.h>
#include <iostream>
#include <cstring>

using namespace std;

template <class T>
inline void releaseMemory(T &x){
    assert(x != NULL);
    delete [] x;
    x = NULL;
}
GUID classGuid;
HMODULE hHidLib;
DWORD memberIndex = 0;
DWORD deviceInterfaceDetailDataSize;
DWORD requiredSize;
HDEVINFO deviceInfoSet;
SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData = NULL;
SP_DEVINFO_DATA deviceInfoData;


HANDLE hidDeviceObject = INVALID_HANDLE_VALUE;
BYTE inputReportBuffer[32];
DWORD numberOfBytesRead = 0;


PBYTE getRegistryPropertyString(HDEVINFO deviceSet, PSP_DEVINFO_DATA deviceData, DWORD property){
    DWORD propertyBufferSize = 0;
    BYTE *propertyBuffer = NULL;
    SetupDiGetDeviceRegistryProperty(deviceSet, deviceData, property, NULL, NULL, 0,&propertyBufferSize);
    propertyBuffer = new BYTE[(propertyBufferSize * sizeof(PBYTE))];
    bool result=SetupDiGetDeviceRegistryProperty(deviceSet, deviceData, property, NULL, propertyBuffer, propertyBufferSize,NULL);
    if(!result)
        releaseMemory(propertyBuffer);
    return propertyBuffer;
}


int main(){
    void (__stdcall *HidD_GetHidGuid)(OUT LPGUID HidGuid);
    bool (__stdcall *HidD_GetNumInputBuffers)(IN HANDLE HidDeviceObject, OUT PULONG NumberBuffers);
    hHidLib = LoadLibrary("C:\\Windows\\system32\\HID.DLL");
    if (!hHidLib) {
        cout << "Blad dolaczenia biblioteki HID.DLL." << endl;
        return 0;
    }

    (FARPROC&) HidD_GetHidGuid = GetProcAddress(hHidLib,"HidD_GetHidGuid");
    (FARPROC &) HidD_GetNumInputBuffers = GetProcAddress(hHidLib,"HidD_GetNumInputBuffers");

    if (!HidD_GetHidGuid){
        FreeLibrary(hHidLib);
        cout << "Nie znaleziono identyfikatora GUID." << endl;
        return 0;
    }

    HidD_GetHidGuid(&classGuid);
    deviceInfoSet = SetupDiGetClassDevs(&classGuid, NULL, NULL,DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        cout << "Nie zidentyfikowano podlaczonych urzadzeñ." << endl;
        return 0;
    }
    deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    cout << "--------------------------------------------LISTA DOSTEPNYCH URZADZEN--------------------------------------------" << endl;
    while(SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &classGuid,memberIndex, &deviceInterfaceData)){
        memberIndex++;
        SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData,NULL, 0, &deviceInterfaceDetailDataSize, NULL);
        deviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) new DWORD[deviceInterfaceDetailDataSize];
        deviceInterfaceDetailData->cbSize=sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        if (!SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData,deviceInterfaceDetailData, deviceInterfaceDetailDataSize,&requiredSize, &deviceInfoData)){
            releaseMemory(deviceInterfaceDetailData);
            SetupDiDestroyDeviceInfoList(deviceInfoSet);
            cout << "Nie mozna pobraæ informacji o interfejsie." << endl;
            return 0;
        }

        cout << endl << "ClassDescr: " << getRegistryPropertyString(deviceInfoSet,&deviceInfoData, SPDRP_CLASS) << endl;
        cout << "ClassGUID: " << getRegistryPropertyString(deviceInfoSet,&deviceInfoData, SPDRP_CLASSGUID) << endl;
        cout << "CompatibileIDs: " << getRegistryPropertyString(deviceInfoSet,&deviceInfoData, SPDRP_COMPATIBLEIDS)<< endl;
        cout << "ConfigFlags: " << getRegistryPropertyString(deviceInfoSet,&deviceInfoData, SPDRP_CONFIGFLAGS)<< endl;
        cout << "DeviceDescr: " << getRegistryPropertyString(deviceInfoSet,&deviceInfoData, SPDRP_DEVICEDESC)<< endl;
        cout << "Driver: " << getRegistryPropertyString(deviceInfoSet,&deviceInfoData, SPDRP_DRIVER)<< endl;
        cout << "HardwareID: " << getRegistryPropertyString(deviceInfoSet,&deviceInfoData, SPDRP_HARDWAREID)<< endl;
        cout << "Mfg: " << getRegistryPropertyString(deviceInfoSet,&deviceInfoData, SPDRP_MFG)<< endl;
        cout << "EnumeratorName: " << getRegistryPropertyString(deviceInfoSet,&deviceInfoData, SPDRP_ENUMERATOR_NAME)<< endl;
        cout << "PhysDevObjName: " << getRegistryPropertyString(deviceInfoSet,&deviceInfoData, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME)<< endl;
        cout << endl;
        releaseMemory(deviceInterfaceDetailData);
    }

    char VID[16];
    char number[16];
    strcpy(VID, "vid_");
    cout << endl << endl << "Prosze wprowadzic vid urzadzenia z ktorego maja byc odczytywane dane: " << VID;
    cin >> number;
    strcat(VID, number);
    cout << endl;

    memberIndex=0;
    while (SetupDiEnumDeviceInterfaces(deviceInfoSet, nullptr, &classGuid, memberIndex, &deviceInterfaceData)) {
        memberIndex++;
        SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData,nullptr, 0, &deviceInterfaceDetailDataSize, nullptr);
        deviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) new DWORD[deviceInterfaceDetailDataSize];
        deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        if (!SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData,deviceInterfaceDetailData, deviceInterfaceDetailDataSize,nullptr, nullptr)) {
            releaseMemory(deviceInterfaceDetailData);
            SetupDiDestroyDeviceInfoList(deviceInfoSet);
            cout << "Nie mozna pobrac informacji o interfejsie." << endl;
            return 0;
        }
        if (strstr(deviceInterfaceDetailData->DevicePath, VID)) {
            hidDeviceObject = CreateFile(deviceInterfaceDetailData->DevicePath, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
            if (hidDeviceObject == INVALID_HANDLE_VALUE){
                cout << "Nie mozna otworzyc urzadzenia do transmisji" << endl;
                releaseMemory(deviceInterfaceDetailData);
                return 0;
            }else{
                break;
            }
        }
        releaseMemory(deviceInterfaceDetailData);
    }

    if(hidDeviceObject == INVALID_HANDLE_VALUE){
        cout << "Nie znaleziono urzadzenia o podanym VID" << endl;
        return 0;
    }
    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    ULONG numberBuffers;
    HidD_GetNumInputBuffers(hidDeviceObject, &numberBuffers);

    while(true){
        memset(&inputReportBuffer, 0x00, sizeof(inputReportBuffer));
        ReadFile(hidDeviceObject, inputReportBuffer, sizeof(inputReportBuffer),&numberOfBytesRead, nullptr);
        printf("%d %d %d %d %d %d %d %d\n", inputReportBuffer[0], inputReportBuffer[1], inputReportBuffer[2], inputReportBuffer[3], inputReportBuffer[4], inputReportBuffer[5], inputReportBuffer[6], inputReportBuffer[7]);
        if (inputReportBuffer[7] == 16)
            break;
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    FreeLibrary(hHidLib);
    cout << endl;
    system("PAUSE");
    return 0;
}
//---------------------------------------------------------
