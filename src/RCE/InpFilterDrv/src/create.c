#include "create.h"
#include "common.h"
#include "skip.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, OnCreateRequest)
#endif

wchar_t *next(wchar_t* s) {
	if (!s) return NULL;
	while (s[0] && (s[0] != L'\\')) s++;
	while (s[0] && (s[0] == L'\\')) s++;
	if (s[0]) return s;
	return NULL;
}

unsigned int readInt(wchar_t* s) {
	int i = 0;
	if (s == NULL) return 0;
	while ((s[0] <= L'9') && (s[0] >= L'0')) {
		i = 10 * i + ((int) s[0] - L'0');
		s++;
	}
	if ((s[0] == 0) || (s[0] == L'\\')) return i;
	return 0;
}


NTSTATUS OnCreateRequest(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP			Irp
)
{
	PDEVICE_EXTENSION   devExt;
	PIO_STACK_LOCATION	irpStack	= IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS            Status		= STATUS_SUCCESS;
	PFILE_OBJECT fileObject;

    PAGED_CODE();

	DbgPrint("OnCreateRequest: Enter\n");

	devExt =  (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
	fileObject = irpStack->FileObject;

	do {

#ifdef DEBUG
		DebugPrint("Create (%ws)", fileObject->FileName.Buffer);
#endif

/*		if (NULL == devExt->UpperConnectData.ClassService) 
		{
			//
			// No Connection yet.  How can we be enabled?
			//
			DbgPrint("OnCreateRequest: Not Ready\n");

			Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;*/
			return DispatchPassThrough(DeviceObject, Irp);
/*			break;
		}
		else 
		{
			//
			// First time enable here
			//
			DbgPrint("OnCreateRequest: Driver Open \n");

			InterlockedIncrement(&devExt->EnableCount);
			Irp->IoStatus.Status      = STATUS_SUCCESS;
			Irp->IoStatus.Information = 0;	
			IoCompleteRequest(Irp, IO_NO_INCREMENT);
			break;
		}*/
	} while (FALSE);

	DbgPrint("OnCreateRequest: Leave\n");

	return STATUS_SUCCESS;
}

