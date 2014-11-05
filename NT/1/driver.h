
#ifndef _DRIVER_H_
#define _DRIVER_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <wdm.h>
#ifdef __cplusplus
}
#endif

// Device Extension
typedef struct _DEVICE_EXTENSION {
	PDEVICE_OBJECT DeviceObj;
	UNICODE_STRING LinkName;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

void 
DriverUnload(PDRIVER_OBJECT pDriverObject);

NTSTATUS 
IRPDispatchRoutine(
	IN PDEVICE_OBJECT pDevObj, 
	IN PIRP pIrp
	);

#endif // _DRIVER_H_