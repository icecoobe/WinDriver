
#include "driver.h"

#include <stdio.h>

const ULONG kMemPoolTag = 'CCNT'; // 4 bytes, 驱动的内存标记, 便于后期的调试, 检测mem leaking
const ULONG kMaxDevice = 10;

const UNICODE_STRING kUstrDevNameFormat = RTL_CONSTANT_STRING(L"\\Device\\MyDDKDevice2-%02d");
const UNICODE_STRING kUstrSymLinkNameFormat = RTL_CONSTANT_STRING(L"\\??\\HelloDDK2-%02d");

//UNICODE_STRING gDevNameArray[10];
//UNICODE_STRING gSymLinkNameArray[10];

// [Section INIT]
// 驱动加载后可以从内存中移除
// Entry point for the driver.
#pragma code_seg("INIT")
extern "C" 
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath)
{
	NTSTATUS status = STATUS_SUCCESS;

	KdPrint(("-----------------------------------------------------------\n"));
	KdPrint(("DriverEntry of MyDriver!\n"));
	PrintDriverInfo(pDriverObject);

	// 设置I/O管理器需要的回调函数
	// pDriverObject->DriverExtension->AddDevice = AddDevice; // NT驱动添加这个会有问题
	pDriverObject->DriverUnload = DriverUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = IRPDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = IRPDispatchRoutine;

	for (int i = 0; i < 3; i++)
	{
		NTSTATUS status = CreateDevice(pDriverObject, i);
		if (!NT_SUCCESS(status))
		{
			KdPrint(("status=%d\n", status));
			return status;
		}
	}

	return STATUS_SUCCESS;
}
#pragma code_seg() // 复原到默认的.text

// [Section PAGE]
#pragma code_seg("PAGE")
void DriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	KdPrint(("Unloading driver ... %X\n", pDriverObject));
	PDEVICE_OBJECT	pDevObj;
	pDevObj = pDriverObject->DeviceObject;
	KdPrint(("Current DeviceObj=%X\n", pDriverObject->DeviceObject));
	while (pDevObj)
	{
		KdPrint(("DEL DeviceObj=%X\n", pDevObj));
		KdPrint(("    NextDevice = %X\n", pDevObj->NextDevice));
		PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
		PDEVICE_OBJECT pNextDev = pDevObj->NextDevice;
		UNICODE_STRING temp1 = pdx->LinkName;
		UNICODE_STRING temp2 = pdx->DeviceName;

//		IoDeleteSymbolicLink(&pdx->LinkName);
//		ExFreePoolWithTag(pdx->DeviceName.Buffer, kMemPoolTag);
//		ExFreePoolWithTag(pdx->LinkName.Buffer, kMemPoolTag);
//		IoDeleteDevice(pDevObj);
		IoDeleteDevice(pDevObj);
		IoDeleteSymbolicLink(&temp1);// &pdx->LinkName);
		ExFreePoolWithTag(temp1.Buffer, kMemPoolTag);
		ExFreePoolWithTag(temp2.Buffer, kMemPoolTag);
		
	
		pDevObj = pNextDev;
		KdPrint(("    Current DeviceObj=%X\n", pDriverObject->DeviceObject));
	}

//	while (!pNextDevObj)
//	{
//		I
//	}
	KdPrint(("-----------------------------------------------------------\n"));
	KdPrint(("End.\n"));
}
#pragma code_seg() // 复原到默认的.text

NTSTATUS IRPDispatchRoutine(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	KdPrint(("Enter IRPDispatchRoutine\n"));
	NTSTATUS status = STATUS_SUCCESS;

	// 完成IRP
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;	// bytes xfered
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );

	KdPrint(("Leave IRPDispatchRoutine\n"));

	return status;
}

// [Section PAGE]
#pragma code_seg("PAGE")
NTSTATUS CreateDevice(PDRIVER_OBJECT pDriverObject, unsigned int i)
{
	KdPrint(("---------------------\n"));
	NTSTATUS status = STATUS_SUCCESS;
	KdPrint(("[%2d] Enter CreateDevice.\nCurrent DeviceObject=%X\n", 
		i, pDriverObject->DeviceObject));

	PDEVICE_OBJECT pFdo = NULL;
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, kUstrDevNameFormat.Buffer);
	UNICODE_STRING symName;
	RtlInitUnicodeString(&symName, kUstrSymLinkNameFormat.Buffer);
	
	KdPrint(("devName = %X, devName.Buffer = %X, devName.Length = %d, ", 
		&devName, 
		devName.Buffer, 
		devName.Length));
	devName.Buffer = (PWSTR)ExAllocatePoolWithTag(
								PagedPool, 
								kUstrDevNameFormat.Length + sizeof(WCHAR), 
								kMemPoolTag);
	KdPrint(("devName = %X, devName.Buffer = %X, devName.Length = %d, devName.Max=%d", 
		&devName, 
		devName.Buffer, 
		devName.Length,
		devName.MaximumLength));
	//devName.MaximumLength = kUstrDevNameFormat.Length + sizeof(WCHAR);
	//RtlInitEmptyUnicodeString(&devName, devName.Buffer, devName.MaximumLength);
	swprintf(devName.Buffer, kUstrDevNameFormat.Buffer, i);
	KdPrint(("devName.Length = %d\n", devName.Length));

	if (!devName.Buffer)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}


	KdPrint(("symName = %X, symName.Buffer = %X, symName.Length = %d, ", 
		&symName, 
		symName.Buffer, 
		symName.Length));
	symName.Buffer = (PWSTR)ExAllocatePoolWithTag(
								PagedPool, 
								kUstrSymLinkNameFormat.Length + sizeof(WCHAR), 
								kMemPoolTag);
	KdPrint(("symName = %X, symName.Buffer = %X, symName.Length = %d, ", 
		&symName, 
		symName.Buffer, 
		symName.Length));
	//symName.MaximumLength = kUstrSymLinkNameFormat.Length + sizeof(WCHAR);
	//RtlInitEmptyUnicodeString(&symName, symName.Buffer, symName.MaximumLength);
	swprintf(symName.Buffer, kUstrSymLinkNameFormat.Buffer, i);
	KdPrint(("symName.Length = %d\n", symName.Length));

	if (!symName.Buffer)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	KdPrint(("DeviceName = %wZ, SymName = %wZ\n", &devName, &symName));
	//创建设备 FDO
	status = IoCreateDevice(
						pDriverObject,
						sizeof(DEVICE_EXTENSION),
						&(UNICODE_STRING)devName,
						FILE_DEVICE_UNKNOWN,
						0, TRUE,
						&pFdo
						);
	if (!NT_SUCCESS(status))
	{
		return status;
	}
	pFdo->Flags |= DO_BUFFERED_IO;
	KdPrint(("Current DeviceObject = %X, Fdo = %X\n", 
		pDriverObject->DeviceObject,
		pFdo));

	// 创建应用程序可访问的Symbolic Link
	status = IoCreateSymbolicLink(&symName, &devName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(pFdo);
		ExFreePoolWithTag(devName.Buffer, kMemPoolTag);
		ExFreePoolWithTag(symName.Buffer, kMemPoolTag);
		return status;
	}
	KdPrint(("Set Device Extension ...\n"));
	// Device Extension of FDO
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pFdo->DeviceExtension;
	pDevExt->DeviceObj = pFdo;
	pDevExt->DeviceName = devName;
	pDevExt->LinkName = symName;
	
	KdPrint(("DeviceName = %wZ, SymName = %wZ\n", &devName, &symName));
	return status;
}

void PrintDriverInfo(PDRIVER_OBJECT pDriverObject)
{
	//KdPrint(("[%s]\n", __FUNCTION__));
	KdPrint(("Driver Object = %X\n", pDriverObject));
	// 不可访问的数据结构项，不会起作用，以下语句不会有输出
//	KdPrint(("Driver size = %d, HWDataBase=%wZ, DriverName = %wZ, DriverStart=%X\n", pDriverObject->DriverSize, 
//		pDriverObject->HardwareDatabase, 
//		pDriverObject->DriverName, 
//		pDriverObject->DriverStart));

	KdPrint(("DriverEntry=%X, DriverUnload=%X\n", DriverEntry, DriverUnload));
	KdPrint(("HWDataBase=%wZ, DriverInit=%X\n",
		pDriverObject->HardwareDatabase, 
		pDriverObject->DriverInit));
}

#pragma code_seg() // 复原到默认的.text