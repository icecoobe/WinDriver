
#include "driver.h"
#include "pnp.h"
#include "dispatch.h"

// WDM version list supported by windows
//Operating system		WDM major version	WDM minor version
//Windows 7					6					0x00
//Windows Server 2008 R2	6					0x00
//Windows Server 2008		6					0x00
//Windows Vista				6					0x00
//Windows Server 2003		1					0x30
//Windows XP				1					0x20
//Windows 2000				1					0x10
//Windows Me				1					0x05
//Windows 98				1					0x00

#pragma code_seg("INIT")
extern "C"
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath)
{
	KdPrint(("########## Enter Driver Entry.\n"));
	KdPrint(("########## DriverObject = %X, RegistryPath = %wZ.\n", pDriverObject, pRegistryPath));

	NTSTATUS status = STATUS_SUCCESS;
	pDriverObject->DriverExtension->AddDevice = PnP_AddDevice;
	pDriverObject->DriverUnload = DriverUnload;

	// here handle Minor request of pnp?
	// IRP_MN_XXXXX ?
	pDriverObject->MajorFunction[IRP_MJ_PNP] = PnP_Dispatch;
	
	// here handle Major request of pnp?
	pDriverObject->MajorFunction[IRP_MJ_CREATE]			= 
    pDriverObject->MajorFunction[IRP_MJ_CLOSE]          = 
    pDriverObject->MajorFunction[IRP_MJ_READ]           = 
    pDriverObject->MajorFunction[IRP_MJ_WRITE]          = 
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = 
	pDriverObject->MajorFunction[IRP_MJ_CLEANUP]        = IrpDispatchRoutine;

	// TODO: Add addtional initialization here.

	KdPrint(("########## Leave Driver Entry.\n"));
	return status;
}
#pragma code_seg()

#pragma code_seg("PAGE")
void DriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	PAGED_CODE();
	KdPrint(("########## Unload Driver.\n"));
}

// todo(chencai): 改成传递操作系统型号的接口形式
NTSTATUS CheckWDMVersion()
{
	// Determine OS version through WDM version information
	// If some operations are not supported in all versions of windows,
	// handle that carefully here.
    if (IoIsWdmVersionAvailable(0x01, 0x10) == FALSE)
    {
        if (IoIsWdmVersionAvailable(0x01, 0x00) == TRUE)
        {
            DbgPrint("OS supports WDM 1.0 (Windows 98)\n");
        }
        else
		{
            DbgPrint("WARNING - OS does not support WDM 1.0\n");
		}
	}
    else
	{
        DbgPrint("OS supports WDM 1.10 (Windows 2000 or above)\n");
	}
	return STATUS_SUCCESS;
}

#pragma code_seg()