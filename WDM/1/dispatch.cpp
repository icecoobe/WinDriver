
#include "dispatch.h"

NTSTATUS 
IrpDispatchRoutine(IN PDEVICE_OBJECT fdo, IN PIRP  pIrp)
{
	DbgPrint("############ IRP dispatch routine.\n");
	//return IoCompleteRequestInfo(pIrp, STATUS_SUCCESS, 0);
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}