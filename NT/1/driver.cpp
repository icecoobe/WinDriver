
#include "driver.h"

// [Section INIT]
// �������غ���Դ��ڴ����Ƴ�
// Entry point for the driver.
#pragma code_seg("INIT")
extern "C" 
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath)
{
	NTSTATUS status = STATUS_SUCCESS;

	KdPrint(("DriverEntry of MyDriver!\n"));
	KdPrint(("Driver Object = %X, RegistryPath =%wZ\n", pDriverObject, pRegistryPath));

	// ����I/O��������Ҫ�Ļص�����
	// pDriverObject->DriverExtension->AddDevice = AddDevice; // NT������������������
	pDriverObject->DriverUnload = DriverUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = IRPDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = IRPDispatchRoutine;

	// ��ʼ���豸��
	// ����-1
	// UNICODE_STRING devName;
	// RtlInitUnicodeString(&devName, L"\\Device\\MyDDKDevice2");
	// ����-2
	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\MyDDKDevice2");
	PDEVICE_OBJECT pFdo = NULL;

	//�����豸 FDO
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

	// 3�ַ�ʽ
	// Direct --- ֱ�Ӷ�д�豸
	// Buffered --- ��������ʽ����Ҫ��Ӧ�ó����ṩ�Ļ��������ݸ��Ƶ��ں�������ڴ���
	// ������ʽ
	// ��������д�豸
	pFdo->Flags |= DO_BUFFERED_IO;

	// Device Extension of FDO
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pFdo->DeviceExtension;
	pDevExt->DeviceObj = pFdo;

	// ����Ӧ�ó���ɷ��ʵ�Symbolic Link
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
#pragma code_seg() // ��ԭ��Ĭ�ϵ�.text

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
#pragma code_seg() // ��ԭ��Ĭ�ϵ�.text

NTSTATUS IRPDispatchRoutine(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	KdPrint(("Enter IRPDispatchRoutine\n"));
	NTSTATUS status = STATUS_SUCCESS;

	// ���IRP
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;	// bytes xfered
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );

	KdPrint(("Leave IRPDispatchRoutine\n"));

	return status;
}