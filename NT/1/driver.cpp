
#include "driver.h"

// [Section INIT]
// 驱动加载后可以从内存中移除
// Entry point for the driver.
#pragma code_seg("INIT")
extern "C" 
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath)
{
	NTSTATUS status = STATUS_SUCCESS;

	KdPrint(("DriverEntry of MyDriver!\n"));
	KdPrint(("Driver Object = %X, RegistryPath =%wZ\n", pDriverObject, pRegistryPath));

	// 设置I/O管理器需要的回调函数
	// pDriverObject->DriverExtension->AddDevice = AddDevice; // NT驱动添加这个会有问题
	pDriverObject->DriverUnload = DriverUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = IRPDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = IRPDispatchRoutine;

	// 初始化设备名
	// 方法-1
	// UNICODE_STRING devName;
	// RtlInitUnicodeString(&devName, L"\\Device\\MyDDKDevice2");
	// 方法-2
	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\MyDDKDevice2");
	PDEVICE_OBJECT pFdo = NULL;

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

	// 3种方式
	// Direct --- 直接读写设备
	// Buffered --- 缓冲区方式，需要将应用程序提供的缓冲区内容复制到内核区域的内存中
	// 其它方式
	// 缓冲区读写设备
	pFdo->Flags |= DO_BUFFERED_IO;

	// Device Extension of FDO
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pFdo->DeviceExtension;
	pDevExt->DeviceObj = pFdo;

	// 创建应用程序可访问的Symbolic Link
	UNICODE_STRING symLinkName = RTL_CONSTANT_STRING(L"\\??\\HelloDDK2");
	status = IoCreateSymbolicLink(&symLinkName, &devName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(pFdo);
		return status;
	}
	pDevExt->LinkName = symLinkName;

	return STATUS_SUCCESS;
}
#pragma code_seg() // 复原到默认的.text

// [Section PAGE]
#pragma code_seg("PAGE")
void DriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	KdPrint(("Unloading driver ... %X\n", pDriverObject));
	PDEVICE_OBJECT	pObj;
	pObj = pDriverObject->DeviceObject;
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)pObj->DeviceExtension;
	IoDeleteSymbolicLink(&pdx->LinkName);
	IoDeleteDevice(pObj);
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