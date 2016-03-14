
// driver.h
// for WDM drivers

#ifndef _DRIVER_H_
#define _DRIVER_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
#include <wdm.h>
#ifdef __cplusplus
}
#endif // __cplusplus

typedef struct _DEVICE_EXTENSION {
	PDEVICE_OBJECT pDeviceObject;
	PDEVICE_OBJECT pLowerDeviceObject;
	UNICODE_STRING ustrDeviceName;
	UNICODE_STRING ustrSymbolicLinkName;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

// Entry point
extern "C"
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject,
					 IN PUNICODE_STRING pRegistryPath);

// Unload
void 
DriverUnload(IN PDRIVER_OBJECT pDriverObject);

const ULONG kMemPoolTag = 'WDM1';
#endif // _DRIVER_H_