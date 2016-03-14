
#ifndef _DISPATCH_H_
#define _DISPATCH_H_

#include <wdm.h>

NTSTATUS 
IrpDispatchRoutine(IN PDEVICE_OBJECT fdo, IN PIRP  pIrp);

#endif