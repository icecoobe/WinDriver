
#include "pnp.h"
#include "driver.h"

#define UNICODE_DEVICE_NAME			L"\\Device\\MyWDMDevice"
#define UNICODE_SYMBOL_LINK_NAME	L"\\??\\LinkToMyWDMDevice"

// PnP_AddDevice
// Callback func to be registerd for PnP Manager to call.
//
// pDriverObj: the driver object passed from PnP manager.
// pPhysicalDeviceObj: the PhysicalDeviceObj(PDO) passed from PnP manager.
#pragma code_seg("PAGE")
NTSTATUS
PnP_AddDevice(PDRIVER_OBJECT pDriverObj, PDEVICE_OBJECT pPhysicalDeviceObj)
{
	PAGED_CODE();
	KdPrint(("Pnp_AddDevice.\n"));

	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT pFdo = NULL;
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, UNICODE_DEVICE_NAME);
	status = IoCreateDevice(
				pDriverObj,
				sizeof(DEVICE_EXTENSION),
				&devName,
				FILE_DEVICE_UNKNOWN,
				0,
				FALSE,
				&pFdo);
	if (STATUS_SUCCESS != status)
	{
		KdPrint(("Failed to add device for"));
		return status;
	}
	
	// From this point forward, any error will have side effects that need to
	// be cleaned up. Using a try-finally block allows us to modify the program
	// easily without losing track of the side effects.
//	__try
//	{
		PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pFdo->DeviceExtension;
		pDevExt->pDeviceObject = pFdo;
		pDevExt->pLowerDeviceObject = IoAttachDeviceToDeviceStack(
										pFdo, 
										pPhysicalDeviceObj);

		if (!pDevExt->pLowerDeviceObject)
		{
			status = STATUS_DEVICE_REMOVED;
			//__leave;
		}
		UNICODE_STRING symLinkName;
		RtlInitUnicodeString(&symLinkName, UNICODE_SYMBOL_LINK_NAME);

		pDevExt->ustrDeviceName = devName;
		pDevExt->ustrSymbolicLinkName = symLinkName;
		status = IoCreateSymbolicLink(&symLinkName, &devName);
		if (!NT_SUCCESS(status))
		{
			IoDeleteSymbolicLink(&pDevExt->ustrSymbolicLinkName);
			status = IoCreateSymbolicLink(&symLinkName, &devName);
			if (!NT_SUCCESS(status))
			{
				return status;
			}
		}

		pFdo->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;
		// Clear the "initializing" flag so that we can get IRPs
		pFdo->Flags &= ~DO_DEVICE_INITIALIZING;
//	}
//
//	__finally
//	{
//		if (!NT_SUCCESS(status))
//		{
//		}
//	}

	return STATUS_SUCCESS;
}

NTSTATUS
PnP_Dispatch(IN PDEVICE_OBJECT pFdo, PIRP pIrp)
{
	PAGED_CODE();
	DbgPrint(("################### Enter PnP_dispatch.\n"));
	//NTSTATUS status = STATUS_SUCCESS;
	// PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	
	IoSkipCurrentIrpStackLocation(pIrp);
    return IoCallDriver(((DEVICE_EXTENSION *)pFdo->DeviceExtension)->pLowerDeviceObject,
								pIrp);

	//return status;
}

#pragma code_seg()