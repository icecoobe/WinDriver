
#ifndef _PNP_H_
#define _PNP_H_

//struct DRIVER_OBJECT;
//struct DEVICE_OBJECT;
//struct IRP;
//
//typedef struct DRIVER_OBJECT* PDRIVER_OBJECT;
//typedef struct DEVICE_OBJECT* PDEVICE_OBJECT; 
//typedef struct IRP* PIRP;
#include <wdm.h>

NTSTATUS
PnP_AddDevice(PDRIVER_OBJECT pDriverObj, PDEVICE_OBJECT pPhysicalDeviceObj);

NTSTATUS
PnP_Dispatch(IN PDEVICE_OBJECT pFdo, PIRP pIrp);

#endif // _PNP_H_