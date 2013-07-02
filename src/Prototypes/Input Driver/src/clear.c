#include "clear.h"
#include "common.h"

void ClearQueues()
{
	KIRQL					OldIrql;
	PKBRD_QUEUE_ENTRY		KbrdEntry;
	PMOUSE_QUEUE_ENTRY		MouseEntry;
	PLIST_ENTRY				ListEntry;

	KeAcquireSpinLock(&MouseQueueLock, &OldIrql);
	while(FALSE == IsListEmpty(&MouseQueue))
	{
		ListEntry = RemoveHeadList(&MouseQueue);
		MouseEntry = CONTAINING_RECORD(ListEntry, MOUSE_QUEUE_ENTRY, Entry);
		if(MouseEntry)
		{
			ExFreePool(MouseEntry);
//			DbgPrint("REMOVING Mouse data\n");
		}
	}
	MouseQueueCount = 0;
	KeReleaseSpinLock(&MouseQueueLock,OldIrql);
//	DbgPrint("Mouse queue is cleared\n");

	KeAcquireSpinLock(&KbrdQueueLock, &OldIrql);
	while(FALSE == IsListEmpty(&KbrdQueue))
	{
		ListEntry = RemoveHeadList(&KbrdQueue);
		KbrdEntry = CONTAINING_RECORD(ListEntry, KBRD_QUEUE_ENTRY, Entry);
		if(KbrdEntry)
		{
			ExFreePool(KbrdEntry);
//			DbgPrint("REMOVING Keyboard data\n");
		}
	}
	KbrdQueueCount = 0;
	KeReleaseSpinLock(&KbrdQueueLock,OldIrql);
//	DbgPrint("Keyboard queue is cleared\n");
}
