/* ERAM.C    RAM disk ERAM for WindowsNT/2000/XP
      Copyright (c) 1999-2004 by *Error15
   Translated into English by Katayama Hirofumi MZ.
*/

/* Update History:
	v1.00
		Initial creation.
	v1.01
		over 32MB support (upto 64MB).
		Win2000 beta 3 support.
	v1.10
		Win2000 operation check.
	v1.11
	    When failure of memory allocation, make the RAM amount indication dumped
		Fixed calculation of the number of clusters around boundary of FAT12 and FAT16.
	v2.00
		Handle the memory after MAXMEM=nn with switching in 64KB units.
	v2.01
		Enabled 1GB allocation by increasing allocation unit 32.
	v2.02
    	You can specify the start address of memory starting from MAXMEM=nn.
		It can also be swappable by treating the Drive type as a local disk.
		Event log message added.
	v2.10
		FAT32 support.
		Made it return the partition at "FAT16 over 32MB".
		Able to allocate 2GB even in FAT16 by increasing Allocation Unit64...
		Made it set the current date to the volume serial number.
	v2.11
		WinXP support.
	v2.12
		Stand-by support.
			Calling IoReportResourceUsage makes it impossible to standby (even if you later release it).
			IoReportResourceForDetection does not pass over 16MB of internal memory request.
			You can HalTranslateBusAddress without calling IoReportResourceUsage.
			Even without HalTranslateBusAddress, the internal memory shows the same address.
			Standby (power remains on): RAM is ok even outside OS management.
			Hibernate (power off): RAM is not allowed under OS management (ok for OS management).
	v2.20
		Fixed that calculation of external memory amount was signed (no actual damage).
		Add the calculation not to wrap 4GB around detection of external memory.
		Fixed that option character comparator didn't correctly support Unicode.
		Fixed the FAT16 max cluster from 65535 to 65525.
		Corrected the FAT32 maximum clusters to 268435455 (= 0xFFFFFFF) (Win2000=4177918).
		Maximum capacity limited to maximum cluster x Allocation Unit (Strictly afford but ignored).
		Maximum capacity limited to 4GB.
			On NT systems, the maximum capacity of FAT16 is 4GB, but the display of chkdsk etc. was strange.
		Stop rounding up by cluster restriction and change to rounding down.
		Changed FAT32 clusters from 65527 to 65526.
		Modified to be testable even with models with less memory.
			Made it possible to forcibly allocate it when forced to use external memory.
			(On older hardware, the address bus might not be fully 32bit-decoded).
		Modified to be testable using memory-mapped file (Filesystem driver load premise).
			In file I/O, ZwXXX seems to be unable to cope with conflicts well, so I used mapped file.
			Output file change support.
		Changed to target only the management area by clearing the area at startup.
		Option 32bit-nized.
		Made it automatically enable FAT32 at Windows2000+.
		Disabled "Verify processing" (Implemented the size check only)
		Fixed wrong sector/drive info.
		Windows Server 2003 support.
		IOCTL_DISK_GET_LENGTH_INFO processing added.
			XP and later don't calculate the disk capacity, so this correspondence was likely needed.
		IOCTL_DISK_SET_PARTITION_INFO processing added.
			It seems that conversion or formatting to NTFS seems to work.
		Fix the FAT12 max clusters from 4087 to 4086.
		Volume label change support.
			Unable to specify *?/|.,;:+=[]()&^<>" and control codes.
		Simplified the conversion from UNICODE-to-ANSI-to-UNICODE to ANSI-to-UNICODE.
		Made it possible to set the real device without swap relation.
		Addition of correction function when selecting multiple memories.
		FAT12/16: Corrected malfunctioning when the root directory sector exceeded all sectors.
		TEMP directory creation feature added.
		This time (again?) lacking the following features:
			NTFS format.
    			Because it is troublesome, please escape with formatting or conversion.
				When formatting at startup, it may be possible to avoid it with the following settings:
					Around "Run":
						cmd.exe /c convert drv: /fs:ntfs < %systemroot%\system32\eramconv.txt
					Around system32\eramconv.txt:
						volume-label
						y
						n
					Sometimes folders are opened and conversion can not be done.
					If you set HDD (=swappable) formatting can also be used.
			Corporation with volume manager.
				I left it as it if not a PnP driver because it seems difficult.
				Mount point (NTFS 5) can not be used.
				Unable to use hardlink (symlink rktools linkd)
			XP: Unable to put the page file.
				It seemed impossible to deal with it unrecognized by the mount manager.
				"Start" can not be set to 0, even if it is set to Primary Disk.
				In XP and later, you can do without page file, so wish to avoid it.
			/BURNMEMORY=n support.
				http://msdn.microsoft.com/library/en-us/ddtools/hh/ddtools/bootini_9omr.asp
				The means of acquiring memory mounting amount at driver loading is unknown.
				HKLM\HARDWARE\RESOURCEMAP\System Resources\Physical Memory\ is not done at all.
	v2.21 (preliminary version)
		ACPI:ACPI Reclaim/NVS memory exclusion.
		Enabled to get the NT device name from the registry (for forcibly for the case of using multiple ERAMs).
			You can make HKLM\System\CurrentControlSet\Services\ERAM ERAM2 or similar, and
			reboot with setting value "DevName" (REG_SZ) to \Device\ERAM2.
			In Win2000 or later, it is not displayed in the device manager but it can be put in INF for NT.
	v2.22 (preliminary version)
		ACPI: Added error processing at ACPI Reclaim/NVS memory exclusion.
		over4GB: The measures against MAXMEM working over 4GB when /MAXMEM=n and /NOLOWMEM coexisted (Thanks for TENMU SHINRYUUSAI)
			When the physical address of the pool is over 4GB, it is regarded as /PAE designation.
			When /PAE designation is taken, the /NOLOWMEM specification is also searched.
			When /NOLOWMEM is specified, it is regarded as equivalent to /MAXMEM=16.
	v2.23 (preliminary version)
		over4GB: The measures against MAXMEM working over 4GB When /MAXMEM=n and /NOLOWMEM coexist (Thanks for TENMU SHINRYUUSAI).
			/PAE:v2.22 made it 16, but 16 was repaired by check so it was fixed to 17.
		YET:over4GB: The measures against the effect of /NOLOWMEM even without /PAE (Pointed out by TENMU SHINRYUUSAI).
			We can search the /NOLOWMEM designation without /PAE designation?
			It seems that it can not detect the limit value because it is used for PagedPool...
		YET:over4GB: allocate RAM in LME status (?)
		YET:SMBIOS:get maximum amount of memory
		YET:SUM check
*/


#pragma warning(disable : 4100 4115 4201 4214 4514 )
#include <ntddk.h>
#include <ntdddisk.h>
#include <devioctl.h>
#include <ntddstor.h>
#include <ntiologc.h>
#include "eram.h"
#include "eramum.h"
#pragma pack(1)



/* EramCreateClose
		Open/Close Request Entry
	Parameters
		pDevObj	The pointer to device object.
		pIrp	The pointer to IRP packet.
	Return Value
		Results.
*/

NTSTATUS EramCreateClose(
	IN PDEVICE_OBJECT	pDevObj,
	IN PIRP				pIrp
 )
{
	KdPrint(("EramCreateClose start\n"));
	/* Set success */
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	KdPrint(("EramCreateClose end\n"));
	return STATUS_SUCCESS;
}


/* EramDeviceControl
		Device Control Request Entry
	Parameters
		pDevObj	The pointer to device object.
		pIrp	The pointer to IRP packet.
	Return Value
		Results.
*/

NTSTATUS EramDeviceControl(
	IN PDEVICE_OBJECT	pDevObj,
	IN PIRP				pIrp
 )
{
	/* local variables */
	PERAM_EXTENSION		pEramExt;
	PIO_STACK_LOCATION	pIrpSp;
	NTSTATUS			ntStat;
	//KdPrint(("EramDeviceControl start\n"));
	/* Get the head pointer of the structure */
	pEramExt = pDevObj->DeviceExtension;
	/* Get the pointer to the stack */
	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
	/* Set failure */
	pIrp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
	pIrp->IoStatus.Information = 0;
	switch (pIrpSp->Parameters.DeviceIoControl.IoControlCode)	/* Branching by request types */
	{
	case IOCTL_DISK_GET_MEDIA_TYPES:		/* media type getting (array) */
	case IOCTL_DISK_GET_DRIVE_GEOMETRY:		/* media type getting (1) */
		/* geometry getting */
		EramDeviceControlGeometry(pEramExt, pIrp, pIrpSp->Parameters.DeviceIoControl.OutputBufferLength);
		break;
	case IOCTL_DISK_GET_PARTITION_INFO:		/* Get the partition info */
		/* Partition info getting processing */
		EramDeviceControlGetPartInfo(pEramExt, pIrp, pIrpSp->Parameters.DeviceIoControl.OutputBufferLength);
		break;
	case IOCTL_DISK_SET_PARTITION_INFO:		/* Get the partition info */
		/* Partition Info Settings processing */
		EramDeviceControlSetPartInfo(pEramExt, pIrp, pIrpSp->Parameters.DeviceIoControl.InputBufferLength);
		break;
	case IOCTL_DISK_VERIFY:					/* Verify */
		/* Verify processing */
		EramDeviceControlVerify(pEramExt, pIrp, pIrpSp->Parameters.DeviceIoControl.InputBufferLength);
		break;
	case IOCTL_DISK_CHECK_VERIFY:			/* Disk check (Win2000 beta3+) */
		/* Verify processing */
		EramDeviceControlDiskCheckVerify(pEramExt, pIrp, pIrpSp->Parameters.DeviceIoControl.OutputBufferLength);
		break;
	case IOCTL_DISK_GET_LENGTH_INFO:		/* Disk size getting (Win2000+ [necessary when formatting/conversion at WinXP and later]) */
		/* Disk size getting processing */
		EramDeviceControlGetLengthInfo(pEramExt, pIrp, pIrpSp->Parameters.DeviceIoControl.OutputBufferLength);
		break;
	default:								/* misc. */
		/* Ignore */
		KdPrint(("Eram IOCTL 0x%x\n", (UINT)(pIrpSp->Parameters.DeviceIoControl.IoControlCode)));
		break;
	}
	/* Set status */
	ntStat = pIrp->IoStatus.Status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	//KdPrint(("EramDeviceControl end\n"));
	return ntStat;
}


/* EramDeviceControlGeometry
		Geometry Getting Process
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pIrp		The pointer to IRP packet.
		uLen		The buffer size.
	Return Value
		No return value.
*/

VOID EramDeviceControlGeometry(
	PERAM_EXTENSION	pEramExt,
	IN PIRP			pIrp,
	IN ULONG 		uLen
 )
{
	/* local variables */
	PDISK_GEOMETRY pGeom;
	if (uLen < sizeof(*pGeom))		/* size lacking */
	{
		pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		KdPrint(("EramDeviceControlGeometry:size too small\n"));
		return;
	}
	/* Disk Geometry setting */
	pGeom = (PDISK_GEOMETRY)(pIrp->AssociatedIrp.SystemBuffer);
	pGeom->MediaType = FixedMedia;		/* Media type: Fixed disk */
	pGeom->Cylinders.QuadPart = (ULONGLONG)(pEramExt->uAllSector);
	pGeom->TracksPerCylinder = 1;
	pGeom->SectorsPerTrack = 1;			/* sectors per bank */
	pGeom->BytesPerSector = SECTOR;
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = sizeof(*pGeom);
}


/* EramDeviceControlGetPartInfo
		Partition info getting processing.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pIrp		The pointer to IRP packet.
		uLen		The buffer size.
	Return Value
		No return value.
*/

VOID EramDeviceControlGetPartInfo(
	PERAM_EXTENSION	pEramExt,
	IN PIRP			pIrp,
	IN ULONG		uLen
 )
{
	/* local variables */
	PPARTITION_INFORMATION pPart;
	if (uLen < sizeof(*pPart))		/* lacking size */
	{
		pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		KdPrint(("EramDeviceControlGetPartInfo:size too small\n"));
		return;
	}
	/* Partition Info Settings */
	pPart = (PPARTITION_INFORMATION)(pIrp->AssociatedIrp.SystemBuffer);
	pPart->PartitionType = pEramExt->FAT_size;
	pPart->BootIndicator = FALSE;			/* Refuse boot */
	pPart->RecognizedPartition = TRUE;		/* Partition detected */
	pPart->RewritePartition = FALSE;		/* unrewritable partition */
	pPart->StartingOffset.QuadPart = (ULONGLONG)(0);	/* Partition starting position */
	pPart->PartitionLength.QuadPart = UInt32x32To64(pEramExt->uAllSector, SECTOR);	/* the length */
	pPart->HiddenSectors =  pEramExt->bsHiddenSecs;	/* the number of hidden sectors */
	pPart->PartitionNumber = 1;				/* The number of partitions */
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = sizeof(PARTITION_INFORMATION);
}


/* EramDeviceControlSetPartInfo
		Partition info setting processing.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pIrp		The poitner to IRP packet.
		uLen		The buffer size.
	Return Value
		No return value.
*/

VOID EramDeviceControlSetPartInfo(
	PERAM_EXTENSION	pEramExt,
	IN PIRP			pIrp,
	IN ULONG		uLen
 )
{
	/* local variables */
	PSET_PARTITION_INFORMATION pPart;
	if (uLen < sizeof(*pPart))		/* lacking size */
	{
		pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		KdPrint(("EramDeviceControlSetPartInfo:size too small\n"));
		return;
	}
	/* Partition Info Settings */
	pPart = (PSET_PARTITION_INFORMATION)(pIrp->AssociatedIrp.SystemBuffer);
	pEramExt->FAT_size = pPart->PartitionType;
	pIrp->IoStatus.Status = STATUS_SUCCESS;
}


/* EramDeviceControlVerify
		Verify processing.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pIrp		The pointer to IRP packet.
		uLen		The buffer size.
	Return Value
		No return value.
*/

VOID EramDeviceControlVerify(
	PERAM_EXTENSION		pEramExt,
	IN PIRP				pIrp,
	IN ULONG			uLen
 )
{
	/* local variables */
	PVERIFY_INFORMATION	pVerify;
	//KdPrint(("EramDeviceControlVerify start\n"));
	if (uLen < sizeof(*pVerify))		/* lacking size */
	{
		pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		KdPrint(("EramDeviceControlVerify:size too small\n"));
		return;
	}
	/* Verify Info Settings TODO: i think its already using 64bit math to calculate stuff here */
	pVerify = pIrp->AssociatedIrp.SystemBuffer;
	//KdPrint(("Eram offset 0x%x%08x, len 0x%x\n", pVerify->StartingOffset.HighPart, pVerify->StartingOffset.LowPart, pVerify->Length));
	if ((((ULONGLONG)(pVerify->StartingOffset.QuadPart) + (ULONGLONG)(pVerify->Length)) > UInt32x32To64(pEramExt->uAllSector, SECTOR))||
		((pVerify->StartingOffset.LowPart & (SECTOR-1)) != 0)||
		((pVerify->Length & (SECTOR-1)) != 0))	/* Disk capacity exceeded or the starting position or the length is not a multiple of the sector size */
	{
		/* Return error */
		KdPrint(("Eram Invalid I/O parameter\n"));
		pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		return;
	}
	if ((pEramExt->uOptflag.Bits.External != 0)&&	/* OS-Unmanaged Memory usage */
		(pEramExt->uExternalStart != 0)&&			/* OS-Unmanaged Memory setting */
		((pEramExt->uExternalStart + (ULONGLONG)(pVerify->StartingOffset.QuadPart) + (ULONGLONG)(pVerify->Length)) >= 
pEramExt->uExternalEnd))
	{
		//KdPrint(("Eram Invalid I/O address space\n"));
		pIrp->IoStatus.Status = STATUS_DISK_CORRUPT_ERROR;
		return;
	}
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	//KdPrint(("EramDeviceControlVerify end\n"));
}


/* EramDeviceControlDiskCheckVerify
		Disk Replacement Confirming Process.
	Parameters
		pEdskExt	The pointer to an EDSK_EXTENTION structure.
		pIrp		The pointer to IRP packet.
		uLen		The buffer size.
	Return Value
		No return value.
*/

VOID EramDeviceControlDiskCheckVerify(
	PERAM_EXTENSION	pEramExt,
	IN PIRP			pIrp,
	IN ULONG		uLen
 )
{


	/* local variables */
	PULONG puOpt;
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	if (uLen == 0)		/* aux info not needed */
	{
		return;
	}
	if (uLen < sizeof(*puOpt))		/* lacking size */
	{
		pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		KdPrint(("EramDeviceControlDiskCheckVerify:size too small\n"));
		return;
	}
	/* Aux info settings */
	puOpt = (PULONG)(pIrp->AssociatedIrp.SystemBuffer);
	*puOpt = 0;
	pIrp->IoStatus.Information = sizeof(*puOpt);
}


/* EramDeviceControlGetLengthInfo
		Disk Size Getting Process.
	Parameters
		pEdskExt	The pointer to an EDSK_EXTENTION structure.
		pIrp		The pointer to IRP packet.
		uLen		The buffer size.
	Return Value
		No return value.
*/

VOID EramDeviceControlGetLengthInfo(
	PERAM_EXTENSION	pEramExt,
	IN PIRP			pIrp,
	IN ULONG		uLen
 )
{
	/* local variables */
	PGET_LENGTH_INFORMATION pInfo;
	if (uLen < sizeof(*pInfo))		/* lacking size */
	{
		pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		KdPrint(("EramDeviceControlGetLengthInfo:size too small\n"));
		return;
	}
	/* Size Info Settings */
	pInfo = (PGET_LENGTH_INFORMATION)(pIrp->AssociatedIrp.SystemBuffer);
	pInfo->Length.QuadPart = UInt32x32To64(pEramExt->uAllSector, SECTOR);	/* the length */
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = sizeof(*pInfo);
	KdPrint(("EramDeviceControlGetLengthInfo length 0x%x\n", (pEramExt->uAllSector * SECTOR)));
}


/* EramReadWrite
		Read/Write/Verify Request Entry.
	Parameters
		pDevObj		The pointer to device object.
		pIrp		The pointer to IRP packet.
	Return Value
		Results.
*/

NTSTATUS EramReadWrite(
	IN PDEVICE_OBJECT	pDevObj,
	IN PIRP				pIrp
 )
{
	/* local variables */
	PERAM_EXTENSION		pEramExt;
	PIO_STACK_LOCATION	pIrpSp;
	PUCHAR				pTransAddr;
	NTSTATUS			ntStat;
	//KdPrint(("EramReadWrite start\n"));
	/* Get the pointer to the first structure */
	pEramExt = pDevObj->DeviceExtension;
	/* Get the pointer to the stack */
	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
	if ((((ULONGLONG)(pIrpSp->Parameters.Read.ByteOffset.QuadPart) + (ULONGLONG)(pIrpSp->Parameters.Read.Length)) > UInt32x32To64(pEramExt->uAllSector, SECTOR))||
		((pIrpSp->Parameters.Read.ByteOffset.QuadPart & (SECTOR-1)) != 0)||
		((pIrpSp->Parameters.Read.Length & (SECTOR-1)) != 0))	/* Disk capacity exceeded or the starting position / the length is not a multiple of the sector size */
	{
		KdPrint(("Invalid I/O parameter, offset 0x%x, length 0x%x, OP=0x%x(R=0x%x, W=0x%x), limit=0x%x\n", pIrpSp->Parameters.Read.ByteOffset.LowPart, pIrpSp->Parameters.Read.Length, pIrpSp->MajorFunction, IRP_MJ_READ, IRP_MJ_WRITE, (pEramExt->uAllSector * SECTOR)));
		/* Return error */
		pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_INVALID_PARAMETER;
	}
	/* address initialization */
	pTransAddr = NULL;
	if (pIrp->MdlAddress != NULL)		/* with address */
	{
		/* address translation */
		pTransAddr = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress,NormalPagePriority);
	}
	/* Set success */
	ntStat = STATUS_SUCCESS;
	/* Set the data length */
	pIrp->IoStatus.Information = 0;
	switch (pIrpSp->MajorFunction)	/* Branch by functions */
	{
	case IRP_MJ_READ:				/* reading */
		//KdPrint(("ERam Read start\n"));
		/* Validate the address */
		if (pTransAddr == NULL)
		{
			KdPrint(("Eram MmGetSystemAddressForMdlSafe failed\n"));
			ntStat = STATUS_INVALID_PARAMETER;
			break;
		}
		/* Set the length of reading */
		pIrp->IoStatus.Information = pIrpSp->Parameters.Read.Length;
		/* reading */
		ntStat = (*(pEramExt->EramRead))(pEramExt, pIrp, pIrpSp, pTransAddr);
		//KdPrint(("Eram Read end\n"));
		break;
	case IRP_MJ_WRITE:				/* writing */
		//KdPrint(("Write start\n"));
		/* Validate the address */
		if (pTransAddr == NULL)
		{
			KdPrint(("Eram MmGetSystemAddressForMdlSafe failed\n"));
			ntStat = STATUS_INVALID_PARAMETER;
			break;
		}
		/* Set the length of reading */
		pIrp->IoStatus.Information = pIrpSp->Parameters.Write.Length;
		/* writing */
		ntStat = (*(pEramExt->EramWrite))(pEramExt, pIrp, pIrpSp, pTransAddr);
		//KdPrint(("Write end\n"));
		break;
	default:
		KdPrint(("Eram RW default\n"));
		pIrp->IoStatus.Information = 0;
		break;
	}
	if (ntStat != STATUS_PENDING)		/* Not pending */
	{
		/* Set status */
		pIrp->IoStatus.Status = ntStat;
		/* I/O complete */
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	}
	//KdPrint(("EramReadWrite end\n"));
	return ntStat;
}


/* EramUnloadDriver
		Entry Point for the time of Device Stop
	Parameters
		pDrvObj		The pointert to device representative object.
	Return Value
		No return value.
*/

VOID EramUnloadDriver(
	IN PDRIVER_OBJECT	pDrvObj
 )
{
	/* local variables */
	PDEVICE_OBJECT		pDevObj;
	PERAM_EXTENSION		pEramExt;
	KdPrint(("EramUnloadDriver start\n"));
	pDevObj = pDrvObj->DeviceObject;
	pEramExt = (pDevObj != NULL) ? pDevObj->DeviceExtension : NULL;
	/* Delete the device */
	EramUnloadDevice(pDrvObj, pDevObj, pEramExt);
	KdPrint(("EramUnloadDriver end\n"));
}


/* EramUnloadDevice
		Device Deletion
	Parameters
		pDrvObj		The pointer to device representative object.
		pDevObj		The pointer to device object.
		pEramExt	The pointer to an ERAM_EXTENTION structure.
	Return Value
		No return value.
*/

VOID EramUnloadDevice(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PDEVICE_OBJECT	pDevObj,
	IN PERAM_EXTENSION	pEramExt
 )
{
	/* local variables */
	LARGE_INTEGER llTime;
	KdPrint(("EramUnloadDevice start\n"));
	if (pEramExt != NULL)			/* Device has already created */
	{
		KdPrint(("Eram Device exist\n"));
		/* Notify thread termination */
		pEramExt->bThreadStop = TRUE;
		if (pEramExt->pThreadObject != NULL)		/* Thread exists */
		{
			/* Decrement semaphore */
			KeReleaseSemaphore(&(pEramExt->IrpSem), 0, 1, TRUE);
			/* Wait 30 seconds for thread termination */
			llTime.QuadPart = (LONGLONG)(-30 * 10000000);
			/* Wait for thread termination */
			KeWaitForSingleObject(&(pEramExt->pThreadObject), Executive, KernelMode, FALSE, &llTime);
			/* Decrement the reference count of thread */
			ObDereferenceObject(&(pEramExt->pThreadObject));
			pEramExt->pThreadObject = NULL;
		}
		/* external file closing */
		if (pEramExt->hSection != NULL)
		{
			KdPrint(("Eram File section close\n"));
			ExtFileUnmap(pEramExt);
			ZwClose(pEramExt->hSection);
			pEramExt->hSection = NULL;
		}
		if (pEramExt->hFile != NULL)
		{
			ZwClose(pEramExt->hFile);
			pEramExt->hFile = NULL;
		}
		/* memory map release */
		ResourceRelease(pDrvObj, pEramExt);
		if (pEramExt->Win32Name.Buffer != NULL)		/* Win32 name has already created */
		{
			/* Win32 link release */
			IoDeleteSymbolicLink(&(pEramExt->Win32Name));
			/* Win32 name area release */
			ExFreePool(pEramExt->Win32Name.Buffer);
			pEramExt->Win32Name.Buffer = NULL;
		}
	}
	if (pDevObj != NULL)		/* Device exists */
	{
		/* Delete the device */
		IoDeleteDevice(pDevObj);
	}
	KdPrint(("EramUnloadDevice end\n"));
}


/* ResourceRelease
		Memory Map Deletion.
	Parameters
		pDrvObj		The pointer to device representative object.
		pEramExt	The pointer to an ERAM_EXTENTION structure.
	Return Value
		No return value.
*/

VOID ResourceRelease(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt
 )
{
	KdPrint(("ERam ResourceRelease start\n"));
	if (pEramExt->uOptflag.Bits.External != 0)	/* OS-Unmanaged Memory usage */
	{
		/* Resource release */
		ReleaseMemResource(pDrvObj, pEramExt);
	}
	else if (pEramExt->pPageBase != NULL)		/* Memory allocating */
	{
		/* memory release */
		ExFreePool(pEramExt->pPageBase);
		pEramExt->pPageBase = NULL;
	}
	KdPrint(("Eram ResourceRelease end\n"));
}


/* ReleaseMemResource
		OS-Unmanaged Memory Map Deletion.
	Parameters
		pDrvObj		The pointer to device representative object.
		pEramExt	The pointer to an ERAM_EXTENTION structure.
	Return Value
		No return value.
*/

VOID ReleaseMemResource(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt
 )
{
	/* local variables */
	CM_RESOURCE_LIST	ResList;	/* Resource list */
	BOOLEAN				bResConf;
	/* Unmap */
	ExtUnmap(pEramExt);
	if (pEramExt->uOptflag.Bits.SkipReportUsage == 0)
	{
		/* Driver resource release (Not likely released in 2000) */
		RtlZeroBytes(&ResList, sizeof(ResList));
		IoReportResourceUsage(NULL, pDrvObj, &ResList, sizeof(ResList), NULL, NULL, 0, FALSE, &bResConf);
		RtlZeroBytes(&(pEramExt->MapAdr), sizeof(pEramExt->MapAdr));
	}
}


/* EramReportEvent
		System Event Log Output.
	Parameters
		pIoObject	Device object pDevObj or driver object pDrvObj.
		ntErrorCode	The event ID.
		pszString	The string to be appended. NULL if omitted.
	Return Value
		Results.
*/

BOOLEAN EramReportEvent(
	IN PVOID	pIoObject,
	IN NTSTATUS	ntErrorCode,
	IN PSTR		pszString
 )
{
	/* local variables */
	ANSI_STRING		AnsiStr;
	UNICODE_STRING	UniStr;
	BOOLEAN			bStat;
	if ((pszString != NULL)&&
		(*pszString != L'\0'))
	{
		RtlInitAnsiString(&AnsiStr, pszString);
		/* UNICODE-stringify and dump */
		if (RtlAnsiStringToUnicodeString(&UniStr, &AnsiStr, TRUE) == STATUS_SUCCESS)
		{
			bStat = EramReportEventW(pIoObject, ntErrorCode, UniStr.Buffer);
			RtlFreeUnicodeString(&UniStr);
			return bStat;
		}
	}
	return EramReportEventW(pIoObject, ntErrorCode, NULL);
}


/* EramReportEventW
		System Event Log Output.
	Parameters
		pIoObject	Device object pDevObj or driver object pDrvObj.
		ntErrorCode	The event ID.
		pwStr		The Unicode string to be appended. NULL if omitted.
	Return Value
		Results.
*/

BOOLEAN EramReportEventW(
	IN PVOID	pIoObject,
	IN NTSTATUS	ntErrorCode,
	IN PWSTR	pwStr
 )
{
	/* local variables */
	PIO_ERROR_LOG_PACKET	pPacket;
	ULONG					uSize;
	UNICODE_STRING			UniStr;
	KdPrint(("EramReportEventW start, event:%ls\n", (PWSTR)((pwStr != NULL) ? pwStr : (PWSTR)(L""))));
	/* packet size initialization */
	uSize = sizeof(IO_ERROR_LOG_PACKET);
	if (pwStr != NULL)	/* With appending string  */
	{
		RtlInitUnicodeString(&UniStr, pwStr);
		/* packet size adding */
		uSize += (UniStr.Length + sizeof(WCHAR));
	}
	if (uSize > ERROR_LOG_MAXIMUM_SIZE)		/* string too long */
	{
		KdPrint(("Eram String too long\n"));
		return FALSE;
	}
	/* packet allocation */
	pPacket = IoAllocateErrorLogEntry(pIoObject, (UCHAR)uSize);
	if (pPacket == NULL)	/* allocation failed */
	{
		KdPrint(("Eram IoAllocateErrorLogEntry failed\n"));
		return FALSE;
	}
	/* standard data part initialization */
	RtlZeroBytes(pPacket, uSize);
	pPacket->ErrorCode = ntErrorCode;
	if (pwStr != NULL)		/* with appending string */
	{
		/* Set the number of strings */
		pPacket->NumberOfStrings = 1;
		/* Set the starting position of string */
		pPacket->StringOffset = sizeof(IO_ERROR_LOG_PACKET);
		/* Copy the Unicode string */
		RtlCopyBytes(&(pPacket[1]), UniStr.Buffer, UniStr.Length);
	}
	/* Log output */
	IoWriteErrorLogEntry(pPacket);
	KdPrint(("EramReportEventW end\n"));
	return TRUE;
}


/* ReadPool
		OS-Managed Memory Reading.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pIrp		The pointer to IRP packet.
		pIrpSp		The pointer to stack info.
		lpDest		The pointer to storage area.
	Return Value
		Results.
*/

NTSTATUS ReadPool(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				lpDest
 )
{
	/* local variables */
	PUCHAR lpSrc;
	lpSrc = (PUCHAR)((PBYTE)pEramExt->pPageBase + (ULONG)pIrpSp->Parameters.Read.ByteOffset.QuadPart);
	RtlCopyBytes(lpDest, lpSrc, pIrpSp->Parameters.Read.Length);
	return STATUS_SUCCESS;
}


/* WritePool
		OS-Managed Memory Writing.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pIrp		The pointer to IRP packet.
		pIrpSp		The pointer to stack info.
		lpSrc		The pointer to data area.
	Return Value
		Results.
*/

NTSTATUS WritePool(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				lpSrc
 )
{
	/* local variables */
	PUCHAR lpDest;
	lpDest = (PUCHAR)((PBYTE)pEramExt->pPageBase + (ULONG)pIrpSp->Parameters.Write.ByteOffset.QuadPart);
	RtlCopyBytes(lpDest, lpSrc, pIrpSp->Parameters.Write.Length);
	return STATUS_SUCCESS;
}


/* ExtRead1
		OS-Unmanaged Memory Reading (without check).
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pIrp		The pointer to IRP packet.
		pIrpSp		The pointer to stack info.
		lpDest		The pointer to storage area.
	Return Value
		Results.
*/

NTSTATUS ExtRead1(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				lpDest
 )
{
	/* local variables */
	PUCHAR	lpSrc;
	UINT	uLen;
	DWORD	eax, ebx;
	NTSTATUS ntStat;
	SIZE_T	uMemAdr;
	ASSERT(pEramExt->uExternalStart != 0);
	ASSERT(pEramExt->uExternalEnd != 0);
	/* Mutex wait */
	ExAcquireFastMutex(&(pEramExt->FastMutex));
	uLen = pIrpSp->Parameters.Read.Length;	/* Transfer size (a multiples of sector size) */
	/* Calculate the sector number */
	ebx = pIrpSp->Parameters.Read.ByteOffset.LowPart >> SECTOR_LOG2;
	/* Calculate the memory position */
	uMemAdr = pEramExt->uExternalStart + pIrpSp->Parameters.Read.ByteOffset.QuadPart;
	ntStat = STATUS_SUCCESS;
	while (uLen != 0)
	{
		if (uMemAdr >= pEramExt->uExternalEnd)	/* Beyond real memory */
		{
			ntStat = STATUS_DISK_CORRUPT_ERROR;
			break;
		}
		/* 64KB allocation */
		if (ExtNext1(pEramExt, &eax, &ebx) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "ExtNext1");
			ntStat = STATUS_DISK_CORRUPT_ERROR;
			break;
		}
		lpSrc = (PUCHAR)((pEramExt->pExtPage + eax)); //(ULONG)
		/* data transfer */
		RtlCopyBytes(lpDest, lpSrc, SECTOR);
		lpDest += SECTOR;
		uLen -= SECTOR;
		uMemAdr += SECTOR;
	}
	/* Unmap */
	ExtUnmap(pEramExt);
	/* Mutex release */
	ExReleaseFastMutex(&(pEramExt->FastMutex));
	return ntStat;
}


/* ExtWrite1
		OS-Unmanaged Memory Writing (without check).
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pIrp		The pointer to IRP packet.
		pIrpSp		The pointer to stack info.
		lpSrc		The pointer to data area.
	Return Value
		Results.
*/

NTSTATUS ExtWrite1(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				lpSrc
 )
{
	/* local variables */
	PUCHAR lpDest;
	UINT	uLen;
	DWORD	eax, ebx;
	NTSTATUS ntStat;
	ULONGPTR uMemAdr;
	ASSERT(pEramExt->uExternalStart != 0);
	ASSERT(pEramExt->uExternalEnd != 0);
	/* Mutex wait */
	ExAcquireFastMutex(&(pEramExt->FastMutex));
	uLen = pIrpSp->Parameters.Write.Length;
	/* Calculate the sector number */
	ebx = pIrpSp->Parameters.Write.ByteOffset.LowPart >> SECTOR_LOG2;
	/* Calculate the memory position */
	uMemAdr = pEramExt->uExternalStart + pIrpSp->Parameters.Write.ByteOffset.LowPart;
	ntStat = STATUS_SUCCESS;
	while (uLen != 0)
	{
		if (uMemAdr >= pEramExt->uExternalEnd)	/* Beyond real memory */
		{
			ntStat = STATUS_DISK_CORRUPT_ERROR;
			break;
		}
		/* 64KB allocation */
		if (ExtNext1(pEramExt, &eax, &ebx) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "ExtNext1");
			ntStat = STATUS_DISK_CORRUPT_ERROR;
			break;
		}
		lpDest = (PUCHAR)((pEramExt->pExtPage + eax)); //(ULONG)
		/* data transfer */
		RtlCopyBytes(lpDest, lpSrc, SECTOR);
		lpSrc += SECTOR;
		uLen -= SECTOR;
		uMemAdr += SECTOR;
	}
	/* Unmap */
	ExtUnmap(pEramExt);
	/* Mutex release */
	ExReleaseFastMutex(&(pEramExt->FastMutex));
	return ntStat;
}


/* ExtNext1
		OS-Unmanaged: The corresponding sector allocation (without check)
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		lpeax		The pointer to the area to return the inner page offset.
		lpebx		The pointer to the sector number (incremented).
	Return Value
		Results.
*/

BOOLEAN ExtNext1(
	IN PERAM_EXTENSION	pEramExt,
	IN OUT LPDWORD		lpeax,
	IN OUT LPDWORD		lpebx
 )
{
	/* local variables */
	DWORD eax, ebx, uMapAdr;
	ebx = *lpebx;
	/* calculate the bank number to be mapped */
	uMapAdr = (ebx >> EXT_PAGE_SEC_LOG2) << EXT_PAGE_SIZE_LOG2;
	/* map */
	if (ExtMap(pEramExt, uMapAdr) == FALSE)
	{
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "ExtMap");
		return FALSE;
	}
	/* calculate the offset */
	eax = ebx & (EXT_PAGE_SECTOR - 1);
	eax <<= SECTOR_LOG2;
	/* proceed the sector number */
	ebx++;
	*lpeax = eax;
	*lpebx = ebx;
	return TRUE;
}


/* ExtMap
		OS-Unmanaged Memory Mapping.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		uMapAdr		The relative byte offset to be mapped (64KB unit).
	Return Value
		Results.
*/

BOOLEAN ExtMap(
	IN PERAM_EXTENSION	pEramExt,
	IN SIZE_T			uMapAdr
 )
{
	/* local variables */
	PHYSICAL_ADDRESS	MapAdr;
	if ((pEramExt->pExtPage == NULL)||			/* unmapped */
		(pEramExt->uNowMapAdr != uMapAdr))	/* different page from the currently mapped page */
	{
		/* Unmap the current page */
		ExtUnmap(pEramExt);
		/* fix the mapping position */
		MapAdr = pEramExt->MapAdr;
		if (MapAdr.QuadPart == 0)		/* Already released */
		{
			KdPrint(("Eram Already resource released\n"));
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_MAXMEM_ALREADY_FREE, NULL);
			return FALSE;
		}
		MapAdr.QuadPart += uMapAdr;
		/* map (allow cache) */
		pEramExt->pExtPage = (PBYTE)MmMapIoSpace(MapAdr, EXT_PAGE_SIZE, TRUE);
		if (pEramExt->pExtPage == NULL)		/* failed */
		{
			KdPrint(("Eram MmMapIoSpace failed\n"));
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_MAXMEM_MAP_FAILED, NULL);
			return FALSE;
		}
		pEramExt->uNowMapAdr = uMapAdr;
	}
	return TRUE;
}


/* ExtUnmap
		OS-Unmanaged Memory Unmapping.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
	Return Value
		No return value.
*/

VOID ExtUnmap(
	IN PERAM_EXTENSION	pEramExt
 )
{
	if (pEramExt->pExtPage != NULL)		/* with the mapping page(s) */
	{
		/* 64KB unmapping */
		MmUnmapIoSpace(pEramExt->pExtPage, EXT_PAGE_SIZE);
		pEramExt->pExtPage = NULL;
		pEramExt->uNowMapAdr = 0;
	}
}


/* ExtFilePendingRw
		External File Reading (Pending).
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pIrp		The pointer to IRP packet.
		pIrpSp		The pointer to stack info.
		pTransAddr	The pointer to storage area.
	Return Value
		Results.
*/

NTSTATUS ExtFilePendingRw(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				pTransAddr
 )
{
	//KdPrint(("ExtFilePendingRw start\n"));
	if (pEramExt->bThreadStop != 0)		/* Instructed termination */
	{
		KdPrint(("Eram stop sequence\n"));
		pIrp->IoStatus.Information = 0;
		return STATUS_DEVICE_NOT_READY;
	}
	if (pEramExt->pThreadObject == NULL)	/* without thread */
	{
		KdPrint(("Eram Thread not exist\n"));
		pIrp->IoStatus.Information = 0;
		return STATUS_DEVICE_NOT_READY;
	}
	/* Pending I/O */
	IoMarkIrpPending(pIrp);
	pIrp->IoStatus.Status = STATUS_PENDING;
	/* Queue */
	ExInterlockedInsertTailList(&(pEramExt->IrpList), &(pIrp->Tail.Overlay.ListEntry), &(pEramExt->IrpSpin));
	/* Decrement semaphores */
	KeReleaseSemaphore(&(pEramExt->IrpSem), 0, 1, FALSE);
	//KdPrint(("Eram ExtFilePendingRw end\n"));
	return STATUS_PENDING;
}


/* EramRwThread
		External File Reading/Writing Request Entry (DISPATCH_LEVEL)
	Parameters
		pContext		The pointer to the delivery info.
	Return Value
		No return value.
*/

VOID EramRwThread(
	IN PVOID			pContext
 )
{
	/* local variables */
	PERAM_EXTENSION		pEramExt;
	PIRP				pIrp;
	NTSTATUS			ntStat;
	PLIST_ENTRY			pIrpList;
	KdPrint(("EramRwThread start\n"));
	/* Get the leading pointer */
	pEramExt = pContext;
	ASSERT(pEramExt != NULL);
	/* Make priority */
	KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);		//  Standard is Prior8
	while (pEramExt->bThreadStop == 0)		/* thread in active */
	{
		//KdPrint(("Eram Waiting\n"));
		/* Wait the request and resume the counter */
		KeWaitForSingleObject(&(pEramExt->IrpSem), Executive, KernelMode, FALSE, NULL);
		if (pEramExt->bThreadStop != 0)		/* thread stop request */
		{
			KdPrint(("Eram thread should stop\n"));
			break;
		}
		//KdPrint(("Eram Wake\n"));
		/* Get the head of the IRP list */
		pIrpList = ExInterlockedRemoveHeadList(&(pEramExt->IrpList), &(pEramExt->IrpSpin));
		//KdPrint(("Eram Get list\n"));
		if (pIrpList != NULL)		/* The list is valid */
		{
			ntStat = EramRwThreadIrp(pEramExt, pIrpList);
			//KdPrint(("EramRwThreadIrp return 0x%x\n", ntStat));
		}
	}
	/* Cancel the remainder IRPs */
	for (;;)
	{
		/* Get the head of the IRP list */
		pIrpList = ExInterlockedRemoveHeadList(&(pEramExt->IrpList), &(pEramExt->IrpSpin));
		if (pIrpList == NULL)		/* No more data */
		{
			break;
		}
		/* get IRP */
		pIrp = CONTAINING_RECORD(pIrpList, IRP, Tail.Overlay.ListEntry);
		ASSERT(pIrp != NULL);
		pIrp->IoStatus.Information = 0;
		pIrp->IoStatus.Status = STATUS_DEVICE_NOT_READY;
		/* I/O complete */
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	}
	if (pEramExt->hSection != NULL)
	{
		ExtFileUnmap(pEramExt);
		ZwClose(pEramExt->hSection);
		pEramExt->hSection = NULL;
	}
	if (pEramExt->hFile != NULL)
	{
		ZwClose(pEramExt->hFile);
		pEramExt->hFile = NULL;
	}
	KdPrint(("EramRwThread end\n"));
	PsTerminateSystemThread(STATUS_SUCCESS);
}


/* EramRwThreadIrp
		External File Reading/Writing Request (1IRP)
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pIrpList	The pointer to IRP list info.
	Return Value
		Results.
*/

NTSTATUS EramRwThreadIrp(
	PERAM_EXTENSION		pEramExt,
	PLIST_ENTRY			pIrpList
 )
{
	/* local variables */
	PIRP				pIrp;
	PIO_STACK_LOCATION	pIrpSp;
	NTSTATUS			ntStat;
	PUCHAR				pTransAddr;
	//KdPrint(("EramRwThreadIrp start\n"));
	ASSERT(pEramExt != NULL);
	ASSERT(pIrpList != NULL);
	/* get IRP */
	pIrp = CONTAINING_RECORD(pIrpList, IRP, Tail.Overlay.ListEntry);
	ASSERT(pIrp != NULL);
	pIrp->IoStatus.Information = 0;
	pTransAddr = NULL;
	if (pIrp->MdlAddress != NULL)	/* with address */
	{
		/* address translation */
		pTransAddr = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress,NormalPagePriority);
	}
	if (pTransAddr == NULL)		/* conversion failure */
	{
		KdPrint(("Eram MmGetSystemAddressForMdlSafe failed\n"));
		/* Set status */
		pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		/* I/O complete */
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_INVALID_PARAMETER;
	}
	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
	ASSERT(pIrpSp != NULL);
	switch (pIrpSp->MajorFunction)	/* Branch by functions */
	{
	case IRP_MJ_READ:				/* reading */
		/* Set the length of reading */
		pIrp->IoStatus.Information = pIrpSp->Parameters.Read.Length;
		ntStat = ExtFileRead1(pEramExt, pIrp, pIrpSp, pTransAddr);
		break;
	case IRP_MJ_WRITE:				/* writing */
		/* Set the length of writing */
		pIrp->IoStatus.Information = pIrpSp->Parameters.Write.Length;
		ntStat = ExtFileWrite1(pEramExt, pIrp, pIrpSp, pTransAddr);
		break;
	default:
		//KdPrint(("Eram RW default\n"));
		ntStat = STATUS_SUCCESS;
		break;
	}
	/* Set status */
	pIrp->IoStatus.Status = ntStat;
	/* I/O complete */
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	//KdPrint(("EramRwThreadIrp end\n"));
	return ntStat;
}


/* ExtFileRead1
		External File Reading (without check).
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pIrp		The pointer to IRP packet.
		pIrpSp		The pointer to stack info.
		lpDest		The pointer to storage area.
	Return Value
		Results.
*/

NTSTATUS ExtFileRead1(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				lpDest
 )
{
	/* local variables */
	PUCHAR	lpSrc;
	UINT	uLen;
	DWORD	eax, ebx;
	NTSTATUS ntStat;
	ASSERT(pEramExt != NULL);
	uLen = pIrpSp->Parameters.Read.Length;	/* Transfer size (a multiples of sector size) */
	/* Calculate the sector number */
	ebx = pIrpSp->Parameters.Read.ByteOffset.LowPart >> SECTOR_LOG2;
	ntStat = STATUS_SUCCESS;
	while (uLen != 0)
	{
		/* 64KB allocation */
		if (ExtFileNext1(pEramExt, &eax, &ebx) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "ExtFileNext1");
			ntStat = STATUS_DISK_CORRUPT_ERROR;
			break;
		}
		lpSrc = (PUCHAR)((pEramExt->pExtPage + eax)); //(ULONG)
		/* data transfer */
		RtlCopyBytes(lpDest, lpSrc, SECTOR);
		lpDest += SECTOR;
		uLen -= SECTOR;
	}
	/* Unmap */
	ExtFileUnmap(pEramExt);
	return ntStat;
}


/* ExtFileWrite1
		External File Writing (without check).
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pIrp		The pointer to IRP packet.
		pIrpSp		The pointer to stack info.
		lpSrc		The pointer to data area.
	Return Value
		Results.
*/

NTSTATUS ExtFileWrite1(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				lpSrc
 )
{
	/* local variables */
	PUCHAR lpDest;
	UINT	uLen;
	DWORD	eax, ebx;
	NTSTATUS ntStat;
	ASSERT(pEramExt != NULL);
	uLen = pIrpSp->Parameters.Write.Length;
	/* Calculate the sector number */
	ebx = pIrpSp->Parameters.Write.ByteOffset.LowPart >> SECTOR_LOG2;
	ntStat = STATUS_SUCCESS;
	while (uLen != 0)
	{
		/* 64KB allocation */
		if (ExtFileNext1(pEramExt, &eax, &ebx) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "ExtFileNext1");
			ntStat = STATUS_DISK_CORRUPT_ERROR;
			break;
		}
		lpDest = (PUCHAR)((pEramExt->pExtPage + eax)); //(ULONG)
		/* data transfer */
		RtlCopyBytes(lpDest, lpSrc, SECTOR);
		lpSrc += SECTOR;
		uLen -= SECTOR;
	}
	/* Unmap */
	ExtFileUnmap(pEramExt);
	return ntStat;
}


/* ExtFileNext1
		External File: The corresponding sector allocation (without check).
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		lpeax		The pointer to the area to return the inner page offset.
		lpebx		The pointer to the sector number (incremented).
	Return Value
		Results.
*/

BOOLEAN ExtFileNext1(
	IN PERAM_EXTENSION	pEramExt,
	IN OUT LPDWORD		lpeax,
	IN OUT LPDWORD		lpebx
 )
{
	/* local variables */
	DWORD eax, ebx, uMapAdr;
	ASSERT(pEramExt != NULL);
	ebx = *lpebx;
	/* Calculate the bank number to be mapped */
	uMapAdr = (ebx >> EXT_PAGE_SEC_LOG2) << EXT_PAGE_SIZE_LOG2;
	/* map */
	if (ExtFileMap(pEramExt, uMapAdr) == FALSE)
	{
		KdPrint(("Eram ExtFileMap failed, MapAdr=0x%x, sector=0x%x, SizeSec=0x%x, SizeBytes=0x%x\n", uMapAdr, ebx, (pEramExt->uAllSector << SECTOR_LOG2), (pEramExt->uSizeTotal << PAGE_SIZE_LOG2)));
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "ExtFileMap");
		return FALSE;
	}
	/* Offset calculation */
	eax = ebx & (EXT_PAGE_SECTOR - 1);
	eax <<= SECTOR_LOG2;
	/* Proceed the sector number */
	ebx++;
	*lpeax = eax;
	*lpebx = ebx;
	return TRUE;
}


/* ExtFileMap
		External File Mapping.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		uMapAdr		The relative byte offset to be mapped (64KB unit).
	Return Value
		Results.
*/

BOOLEAN ExtFileMap(
	IN PERAM_EXTENSION	pEramExt,
	IN SIZE_T			uMapAdr
 )
{
	/* local variables */
	LARGE_INTEGER llOfs;
	SIZE_T uView;
	NTSTATUS ntStat;
	ASSERT(pEramExt != NULL);
	if ((pEramExt->pExtPage == NULL)||			/* unmapped */
		(pEramExt->uNowMapAdr != uMapAdr))		/* different page from the current mapping page */
	{
		/* Unmap the current page */
		ExtFileUnmap(pEramExt);
		/* Prepare for mapping position */
		llOfs.QuadPart = (LONGLONG)uMapAdr;
		uView = ((pEramExt->uSizeTotal << PAGE_SIZE_LOG2) - uMapAdr);
		if (uView > EXT_PAGE_SIZE)
		{
			uView = EXT_PAGE_SIZE;
		}
		/* map (allow cache) */
		ntStat = ZwMapViewOfSection(pEramExt->hSection, NtCurrentProcess(), &(pEramExt->pExtPage), 0, uView, &llOfs, &uView, ViewShare, 0, PAGE_READWRITE);
		if (ntStat != STATUS_SUCCESS)		/* failed */
		{
			KdPrint(("Eram ZwMapViewOfSection failed, 0x%x, MapAdr=0x%x, size=0x%x\n", ntStat, uMapAdr, uView));
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_MAP_EXT_FILE, NULL);
			return FALSE;
		}
		ASSERT((pEramExt->pExtPage) != NULL);
		pEramExt->uNowMapAdr = uMapAdr;
	}
	return TRUE;
}


/* ExtFileUnmap
		External File Unmapping.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
	Return Value
		No return value.
*/

VOID ExtFileUnmap(
	IN PERAM_EXTENSION	pEramExt
 )
{
	/* local variables */
	NTSTATUS ntStat;
	ASSERT(pEramExt != NULL);
	if (pEramExt->pExtPage == NULL)		/* no page in the map */
	{
		return;
	}
	/* 64KB unmapping */
	ntStat = ZwUnmapViewOfSection(NtCurrentProcess(), pEramExt->pExtPage);
	pEramExt->pExtPage = NULL;
	pEramExt->uNowMapAdr = 0;
	if (ntStat != STATUS_SUCCESS)
	{
		KdPrint(("Eram ZwMapViewOfSection failed, 0x%x\n", ntStat));
	}
}


/* EramShutdown
		Shutdown Request Entry.
	Parameters
		pDevObj	The pointer to device object.
		pIrp	The pointer to IRP packet.
	Return Value
		Results.
*/

NTSTATUS EramShutdown(
	IN PDEVICE_OBJECT	pDevObj,
	IN PIRP				pIrp
 )
{
	/* local variables */
	PERAM_EXTENSION pEramExt;
	LARGE_INTEGER  	llTime;
	KdPrint(("EramShutdown start\n"));
	pEramExt = pDevObj->DeviceExtension;
	/* Notify thread termination */
	pEramExt->bThreadStop = TRUE;
	if (pEramExt->pThreadObject != NULL)		/* Thread exists */
	{
		/* Decrement semaphore */
		KeReleaseSemaphore(&(pEramExt->IrpSem), 0, 1, TRUE);
		/* Wait 5 seconds for thread termination */
		llTime.QuadPart = (LONGLONG)(-5 * 10000000);
		KeWaitForSingleObject(&(pEramExt->pThreadObject), Executive, KernelMode, FALSE, &llTime);
		/* Decrement the reference count of thread(s) */
		ObDereferenceObject(&(pEramExt->pThreadObject));
		pEramExt->pThreadObject = NULL;
	}
	/* External File Closing */
	if (pEramExt->hSection != NULL)
	{
		KdPrint(("Eram File section close\n"));
		ExtFileUnmap(pEramExt);
		ZwClose(pEramExt->hSection);
		pEramExt->hSection = NULL;
	}
	if (pEramExt->hFile != NULL)
	{
		ZwClose(pEramExt->hFile);
		pEramExt->hFile = NULL;
	}
	/* Set success */
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	KdPrint(("EramShutdown end\n"));
	return STATUS_SUCCESS;
}


//------  Below is functions used at initialization


/* DriverEntry
		The Entry Point When Driver Initialization.
	Parameters
		pDrvObj		The pointer to device representative object.
		pRegPath	The pointer to registry key.
	Return Value
		Results.
*/

NTSTATUS DriverEntry(
	IN OUT PDRIVER_OBJECT	pDrvObj,
	IN PUNICODE_STRING		pRegPath
 )
{
	/* local variables */
	NTSTATUS		ntStat;
	UNICODE_STRING	RegParam;		/* Registry path (using Unicode) */
	UNICODE_STRING	RegParamAdd;	/* Registry path (using Unicode) */
	PVOID			pPool;
	PFAT_ID			pFatId;			/* BPB work area */
	KdPrint(("Eram DriverEntry start\n"));
	/* Get the max length of registry path */
	RegParam.MaximumLength = (WORD)(pRegPath->Length + sizeof(SUBKEY_WSTRING));
	/* memory allocation for work */
	pPool = ExAllocatePool(PagedPool, sizeof(*pFatId) + RegParam.MaximumLength);
	if (pPool == NULL)		/* allocation failed */
	{
		KdPrint(("Eram ExAllocatePool failed\n"));
		EramReportEvent(pDrvObj, ERAM_ERROR_WORK_ALLOC_FAILED, NULL);
		/* Return error */
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	/* Pointer setting */
	pFatId = (PFAT_ID)pPool;
	RegParam.Buffer = (PWSTR)(&(pFatId[1]));
	/* Copy the registry key string */
	RtlCopyUnicodeString(&RegParam, pRegPath);
	/* Copy the registry subkey string */
	RtlInitUnicodeString(&RegParamAdd, (PWSTR)SUBKEY_WSTRING);
	if (RtlAppendUnicodeStringToString(&RegParam, &RegParamAdd) != STATUS_SUCCESS)	/* 合成失敗 */
	{
		KdPrint(("Eram RtlAppendUnicodeStringToString failed\n"));
		EramReportEvent(pDrvObj, ERAM_ERROR_REG_KEY_APPEND_FAILED, NULL);
		/* Release the memory for work */
		ExFreePool(pPool);
		/* Return error */
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	/* FatId structure initialization */
	InitFatId(pFatId);
	/* Driver entry point initialization */
	pDrvObj->MajorFunction[IRP_MJ_CREATE] = EramCreateClose;
	pDrvObj->MajorFunction[IRP_MJ_CLOSE] = EramCreateClose;
	pDrvObj->MajorFunction[IRP_MJ_READ] = EramReadWrite;
	pDrvObj->MajorFunction[IRP_MJ_WRITE] = EramReadWrite;
	pDrvObj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = EramDeviceControl;
	pDrvObj->MajorFunction[IRP_MJ_SHUTDOWN] = EramShutdown;
	/* initialize the entry/entries when releasing */
	pDrvObj->DriverUnload = EramUnloadDriver;
	/* RAM disk initialization */
	ntStat = EramInitDisk(pDrvObj, pFatId, &RegParam);
	ASSERT(pPool != NULL);
	/* Release the memory for work */
	ExFreePool(pPool);
	KdPrint(("Eram DriverEntry end\n"));
	/* 初期化終了 */
	return ntStat;
}


/* InitFatId
		FatId Structure Initialization.
	Parameters
		pFatId		The pointer to FAT-ID structure.
	Return Value
		No return value.
*/

VOID InitFatId(
	IN	PFAT_ID		pFatId
 )
{
	KdPrint(("Eram InitFatId start\n"));
	RtlZeroBytes(pFatId, sizeof(*pFatId));
	pFatId->BPB.wNumSectorByte = SECTOR;				/* The number of sector bytes (BPB, =SECTOR) */
	pFatId->BPB.byAllocUnit = 1024 / SECTOR;			/* Allocation Unit(alloc, =1024/SECTOR) */
	pFatId->BPB.wNumResvSector = 1;						/* The number of reserved sectors (=1) */
	pFatId->BPB.byNumFat = 1;							/* The number of FATs (=1) */
	pFatId->BPB.wNumDirectory = 128;					/* The number of root directory entries (dir, =128) */
	pFatId->BPB.byMediaId = RAMDISK_MEDIA_TYPE;			/* Media ID (media, =f8) */
	pFatId->BPB_ext.bsSecPerTrack = 1;					/* Sectors per bank (=PAGE_SECTOR) */
	pFatId->BPB_ext.bsHeads = 1;						/* The number of heads (=1) */
	pFatId->BPB_fat32.dwRootCluster = 2;				/* The starting cluster of root directory */
	pFatId->BPB_fat32.wFsInfoSector = 1;				/* The sector of FSINFO */
	pFatId->BPB_fat32.wBackupBootSector = 0xffff;		/* backup boot sector */
	KdPrint(("Eram InitFatId end\n"));
}


/* EramInitDisk
		ERAM Initialization.
	Parameters
		pDrvObj		The pointer to device representative object.
		pFatId		The pointer to FAT-ID structure.
		pRegParam	The pointer to the registry path string.
	Return Value
		Results.
*/

NTSTATUS EramInitDisk(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PFAT_ID			pFatId,
	IN PUNICODE_STRING	pRegParam
 )
{
	/* local variables */
	UNICODE_STRING	NtDevName;		/* NT device name "\Device\Eram" */
	UNICODE_STRING	Win32Name;		/* Win32 name "\DosDevices\Z:" */
	UNICODE_STRING	DrvStr;			/* Drive character */
	WCHAR			DrvBuf[3];		/* The buffer to get the drive character */
	PDEVICE_OBJECT	pDevObj = NULL;
	PERAM_EXTENSION	pEramExt = NULL;
	NTSTATUS		ntStat;
	ULONGPTR		uMemSize;
	DEVICE_TYPE		dType;
	KdPrint(("EramInitDisk start\n"));
	/* Check whether it is a swappable device */
	dType = CheckSwapable(pRegParam);
	/* Initialize string */
	RtlInitUnicodeString(&NtDevName, (PWSTR)NT_DEVNAME);
	/* Confirm whether to replace the device name */
	CheckDeviceName(pRegParam, &NtDevName);
	/* device creation */
	ntStat = IoCreateDevice(pDrvObj, sizeof(*pEramExt), &NtDevName, dType, 0, FALSE, &pDevObj);
	if (ntStat != STATUS_SUCCESS)	/* failed */
	{
		KdPrint(("Eram IoCreateDevice failed, 0x%x\n", ntStat));
{
	CHAR szBuf[128];
	sprintf(szBuf, "IoCreateDevice failed, 0x%x", ntStat);
	EramReportEvent(pDrvObj, ERAM_ERROR_FUNCTIONERROR, szBuf);	//@@@
	sprintf(szBuf, "Device is \"%ls\"", NtDevName.Buffer);
	EramReportEvent(pDrvObj, ERAM_ERROR_FUNCTIONERROR, szBuf);	//@@@
}

		EramReportEvent(pDrvObj, ERAM_ERROR_CREATE_DEVICE_FAILED, NULL);
		return ntStat;
	}
	/* Get the info pointer */
	pEramExt = (PERAM_EXTENSION)(pDevObj->DeviceExtension);
	/* ERAM info area initialization */
	RtlZeroBytes(pEramExt, sizeof(*pEramExt));
	/* Drive character buffer clear */
	DrvBuf[0] = UNICODE_NULL;
	RtlInitUnicodeString(&DrvStr, DrvBuf);
	DrvStr.MaximumLength = sizeof(DrvBuf);
	/* Get info from registry */
	CheckSwitch(pEramExt, pFatId, pRegParam, &DrvStr);
	pEramExt->uOptflag.Bits.Swapable = (BYTE)((dType == FILE_DEVICE_DISK) ? 1 : 0);
	/* デバイス情報初期化 */
	pDevObj->Flags |= DO_DIRECT_IO;
	pDevObj->AlignmentRequirement = FILE_WORD_ALIGNMENT;
	pEramExt->pDevObj = pDevObj;
	if (pEramExt->uOptflag.Bits.External != 0)				/* OS-Unmanaged Memory usage */
	{
		if (GetExternalStart(pDrvObj, pEramExt) == FALSE)	/* without OS-Unmanaged Memory */
		{
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_MAXMEM_NOT_DETECTED, NULL);
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		if ((pEramExt->uOptflag.Bits.SkipExternalCheck == 0)&&	/* don't skip check */
			(CheckExternalSize(pDrvObj, pEramExt) == FALSE))	/* check failure */
		{
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "CheckExternalSize");
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		/* mutex initialization */
		ExInitializeFastMutex(&(pEramExt->FastMutex));
	}
	/* memory allocation */
	uMemSize = pEramExt->uSizeTotal << PAGE_SIZE_LOG2;
	if (pEramExt->uSizeTotal < DISKMINPAGE)		/* Without memory */
	{
		KdPrint(("Eram Memory size too small, %d\n", pEramExt->uSizeTotal));
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_DISK_SIZE_TOO_SMALL, NULL);
		ntStat = STATUS_INSUFFICIENT_RESOURCES;
		goto EramInitDiskExit;
	}
	/* memory area allocation */
	ntStat = MemSetup(pDrvObj, pEramExt, pFatId, uMemSize);
	if (ntStat != STATUS_SUCCESS)	/* failed */
	{
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "MemSetup");
		goto EramInitDiskExit;
	}
	/* FAT format */
	if (EramFormatFat(pEramExt, pFatId) == FALSE)
	{
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "EramFormatFat");
		ntStat = STATUS_INSUFFICIENT_RESOURCES;
		goto EramInitDiskExit;
	}
	/* ERAM Info Settings */
	pEramExt->bsHiddenSecs = pFatId->BPB_ext.bsHiddenSecs;
	/* Win32 device name area allocation */
	pEramExt->Win32Name.Buffer = ExAllocatePool(PagedPool, (sizeof(WIN32_PATH) + sizeof(DEFAULT_DRV)));
	if (pEramExt->Win32Name.Buffer == NULL)		/* allocation failed */
	{
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_DEVICE_NAME_ALLOC_FAILED, NULL);
		ntStat = STATUS_INSUFFICIENT_RESOURCES;
		goto EramInitDiskExit;
	}
	pEramExt->Win32Name.MaximumLength = sizeof(WIN32_PATH) + sizeof(DEFAULT_DRV);
	/* Use Z: when drive string was invalid */
	if (DrvStr.Buffer[0] == UNICODE_NULL)
	{
		DrvStr.Buffer[0] = L'Z';
	}
	DrvStr.Buffer[1] = L':';
	DrvStr.Buffer[2] = UNICODE_NULL;
	DrvStr.Length = sizeof(DEFAULT_DRV) - sizeof(WCHAR);	/* two characters of "Z:" */
	KdPrint(("Eram Drive %ls\n", DrvStr.Buffer));
	/* Win32 device name integration */
	RtlInitUnicodeString(&Win32Name, (PWSTR)WIN32_PATH);
	RtlCopyUnicodeString(&(pEramExt->Win32Name), &Win32Name);
	RtlAppendUnicodeStringToString(&(pEramExt->Win32Name), &DrvStr);
	/* link creation */
	ntStat = IoCreateSymbolicLink(&(pEramExt->Win32Name), &NtDevName);
	if (ntStat != STATUS_SUCCESS)	/* failed */
	{
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_CREATE_SYMBOLIC_LINK_FAILED, NULL);
		/* Win32 name area release */
		ExFreePool(pEramExt->Win32Name.Buffer);
		pEramExt->Win32Name.Buffer = NULL;
	}
EramInitDiskExit:	/* entry on error */
	if (ntStat != STATUS_SUCCESS)	/* failed */
	{
		if (pEramExt->uOptflag.Bits.External != 0)	/* OS-Unmanaged Memory usage */
		{
			/* Free the memory resource */
			ReleaseMemResource(pDrvObj, pEramExt);
		}
		if (pDevObj != NULL)	/* Device already created */
		{
			/* Delete the device */
			EramUnloadDevice(pDrvObj, pDevObj, pEramExt);
		}
	}
	KdPrint(("EramInitDisk end\n"));
	return ntStat;
}


/* MemSetup
		Memory Area Reservation.
	Parameters
		pDrvObj		The pointer to device representative object.
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pFatId		The pointer to FAT-ID structure.
		uMemSize	The memory size to request.
	Return Value
		Results.
*/

NTSTATUS MemSetup(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId,
	IN SIZE_T			uMemSize
 )
{
	/* local variables */
	FILE_END_OF_FILE_INFORMATION	EofInfo;
	NTSTATUS						ntStat;
	IO_STATUS_BLOCK					IoStat;
	OBJECT_ATTRIBUTES				ObjAttr;
	UNICODE_STRING					uniStr;
	HANDLE							hThread;
	KdPrint(("Eram MemSetup start\n"));
	if (pEramExt->uOptflag.Bits.External != 0)	/* OS-Unmanaged Memory */
	{
		/* Notify resource usage */
		if (ExtReport(pDrvObj, pEramExt) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "ExtReport");
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		return STATUS_SUCCESS;
	}
	if (pEramExt->uOptflag.Bits.UseExtFile != 0)	/* External File Usage */
	{
		/* Firstly prepare filename */
		RtlInitUnicodeString(&uniStr, (PWSTR)(pFatId->wszExtFile));
		InitializeObjectAttributes(&ObjAttr, &uniStr, OBJ_CASE_INSENSITIVE, NULL, NULL);
		/* File open */
		ntStat = ZwCreateFile(&(pEramExt->hFile),
						GENERIC_READ | GENERIC_WRITE,
						&ObjAttr,
						&IoStat,
						NULL,
						FILE_ATTRIBUTE_SYSTEM,
						FILE_SHARE_READ,
						FILE_OVERWRITE_IF,
						FILE_NON_DIRECTORY_FILE | FILE_DELETE_ON_CLOSE,
						NULL,
						0
						);
		if (ntStat != STATUS_SUCCESS)	/* failed */
		{
			KdPrint(("Eram ZwCreateFile failed, 0x%x\n", ntStat));
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_CREATE_EXT_FILE, NULL);
			return ntStat;
		}
		/* file size allocation */
		EofInfo.EndOfFile.QuadPart = (LONGLONG)uMemSize;
		ntStat = ZwSetInformationFile(pEramExt->hFile, &IoStat, &EofInfo, sizeof(EofInfo), FileEndOfFileInformation);
		if (ntStat != STATUS_SUCCESS)	/* failed */
		{
			KdPrint(("Eram ZwSetInformationFile failed, 0x%x\n", ntStat));
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_SET_INFO_EXT_FILE, NULL);
			return ntStat;
		}
		/* mapping object creation */
		ntStat = ZwCreateSection(&(pEramExt->hSection), SECTION_ALL_ACCESS, NULL, NULL, PAGE_READWRITE, SEC_COMMIT, pEramExt->hFile);
		if (ntStat != STATUS_SUCCESS)	/* failed */
		{
			KdPrint(("Eram ZwCreateSection failed, 0x%x\n", ntStat));
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_CREATE_EXT_FILE_SECTION, NULL);
			return ntStat;
		}
		/* spinlock initialization */
		KeInitializeSpinLock(&(pEramExt->IrpSpin));
		/* list initialization */
		InitializeListHead(&(pEramExt->IrpList));
		/* semaphore initialization */
		KeInitializeSemaphore(&(pEramExt->IrpSem), 0, MAXLONG);
		/* system thread creation */
		ntStat = PsCreateSystemThread(&hThread, THREAD_ALL_ACCESS, NULL, NULL, NULL, EramRwThread, pEramExt);
		if (ntStat != STATUS_SUCCESS)	/* failed */
		{
			KdPrint(("Eram PsCreateSystemThread failed\n"));
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_CREATE_THREAD, NULL);
			return ntStat;
		}
		/* Get the thread object */
		ntStat = ObReferenceObjectByHandle(hThread, THREAD_ALL_ACCESS, NULL, KernelMode, &(pEramExt->pThreadObject), NULL);
		if (ntStat != STATUS_SUCCESS)	/* failed */
		{
			KdPrint(("Eram ObReferenceObjectByHandle failed\n"));
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_GET_THREAD_OBJECT, NULL);
			return ntStat;
		}
		/* Release the thread handle */
		ZwClose(hThread);
		/* Enable shutdown notification */
		IoRegisterShutdownNotification(pEramExt->pDevObj);
		return STATUS_SUCCESS;
	}
	/* Use OS-Managed Memory */
	if (OsAlloc(pDrvObj, pEramExt, uMemSize) == FALSE)
	{
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "OsAlloc");
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	KdPrint(("Eram MemSetup end\n"));
	return STATUS_SUCCESS;
}


/* OsAlloc
		OS-Managed Memory Reservation.
	Parameters
		pDrvObj		The pointer to device representative object.
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		uMemSize	The memory size to request.
	Return Value
		Results.
*/

BOOLEAN OsAlloc(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt,
	IN SIZE_T			uMemSize
 )
{
	/* local variables */
	POOL_TYPE	fPool;
	fPool = (pEramExt->uOptflag.Bits.NonPaged != 0) ? NonPagedPool : PagedPool;

	KdPrint(( "Eram Size=%dmb\n" , uMemSize/(1024*1024) ));

	pEramExt->pPageBase = ExAllocatePool(fPool, uMemSize);
	if (pEramExt->pPageBase == NULL)	/* allocation failed */
	{
		KdPrint(("Eram ExAllocatePool failed, %ld bytes, nonpaged=%d\n", uMemSize, (UINT)(pEramExt->uOptflag.Bits.NonPaged)));
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_DISK_ALLOC_FAILED, NULL);
		CalcAvailSize(pDrvObj, fPool, uMemSize);
		return FALSE;
	}
	return TRUE;
}


/* CalcAvailSize
		Report the Memory Size Likely Reservable.
	Parameters
		pDrvObj		The pointer to device representative object.
		fPool		The memory type.
		uMemSize	The memory size to request.
	Return Value
		No return value.
*/

VOID CalcAvailSize(
	IN PDRIVER_OBJECT	pDrvObj,
	IN POOL_TYPE		fPool,
	IN SIZE_T			uMemSize
 )
{
	/* local variables */
	PVOID			pBuf;
	UNICODE_STRING	UniStr;
	WCHAR			wcBuf[32];
	pBuf = NULL;
	while ((uMemSize > (DISKMINPAGE << PAGE_SIZE_LOG2))&&(pBuf == NULL))
	{
		/* memory allocation */
		uMemSize -= (DISKMINPAGE << PAGE_SIZE_LOG2);
		pBuf = ExAllocatePool(fPool, uMemSize);
	}
	if (pBuf == NULL)		/* allocation failed */
	{
		return;
	}
	/* memory release */
	ExFreePool(pBuf);
	/* Limit to about 75% */
	uMemSize = (uMemSize >> 2) * 3;
	/* Report the memory amount */
	wcBuf[0] = UNICODE_NULL;
	RtlInitUnicodeString(&UniStr, wcBuf);
	UniStr.MaximumLength = sizeof(wcBuf);
	if (RtlIntPtrToUnicodeString(uMemSize >> 10, 10, &UniStr) == STATUS_SUCCESS)
	{
		EramReportEventW(pDrvObj, ERAM_INF_MEMORY_SIZE, UniStr.Buffer);
	}
}


/* CheckSwapable
		Registry Reference: Select Whether Swappable Device Or Not
	Parameters
		pRegParam	The pointer to the registry path string.
	Return Value
		The device type.
	Registry Parameter
		Option			The option(s).
*/

DEVICE_TYPE CheckSwapable(
	IN PUNICODE_STRING		pRegParam
 )
{
	/* local variables */
	RTL_QUERY_REGISTRY_TABLE	ParamTable[2];
	ULONG			Option,		defOption = 0;
	NTSTATUS		ntStat;
	ERAM_OPTFLAG	uOptflag;
	KdPrint(("Eram CheckSwapable start\n"));
	/* registry confirmation area initialization */
	RtlZeroBytes(&(ParamTable[0]), sizeof(ParamTable));
	/* collective inquiry area initialization (the last one is NULL) */
	ParamTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
	ParamTable[0].DefaultType = REG_DWORD;
	ParamTable[0].DefaultLength = sizeof(ULONG);
	ParamTable[0].Name = (PWSTR)L"Option";
	ParamTable[0].EntryContext = &Option;
	ParamTable[0].DefaultData = &defOption;
	/* registry values inquiry */
	ntStat = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL, pRegParam->Buffer, &(ParamTable[0]), NULL, NULL);
	if (ntStat != STATUS_SUCCESS)	/* failed */
	{
		KdPrint(("Eram Warning:RtlQueryRegistryValues failed\n"));
		/* Adapt the default value */
		Option = defOption;
	}
	uOptflag.dwOptflag = Option;
	if (uOptflag.Bits.Swapable != 0)		/* Swappable settings */
	{
		KdPrint(("Eram CheckSwapable end, local disk\n"));
		/* Lock it not to be swapped */
#pragma warning(disable : 4054)
		MmLockPagableCodeSection((PVOID)EramCreateClose);
#pragma warning(default : 4054)
		/* Treat as a local disk: swappable */
		return FILE_DEVICE_DISK;
	}
	/* Treat it as a RAM disk */
	KdPrint(("Eram CheckSwapable end, virtual disk\n"));
	return FILE_DEVICE_VIRTUAL_DISK;
}


/* CheckDeviceName
		Registry Reference (Device Name)
	Parameters
		pRegParam	The pointer to the registry path string.
		pNtDevName	The pointer to NT device name.
	Return Value
		No return value.
	Registry Parameter
		DeviceName	The device name
*/

VOID CheckDeviceName(
	IN PUNICODE_STRING		pRegParam,
	IN OUT PUNICODE_STRING	pNtDevName
 )
{
	/* local variables */
	static WCHAR wszDef[] = L"";
	static WCHAR wszDev[32] = L"";		/* \\Device\\～ */
	RTL_QUERY_REGISTRY_TABLE	ParamTable[2];
	NTSTATUS		ntStat;
	UNICODE_STRING	UniDev;
	KdPrint(("Eram CheckDeviceName start\n"));
	/* prepare for the initial valus for inquiry */
	RtlInitUnicodeString(&UniDev, wszDev);
	UniDev.MaximumLength = sizeof(wszDev);
	/* registry confirmation area initialization */
	RtlZeroBytes(&ParamTable, sizeof(ParamTable));
	/* collective inquiry area initialization (the last one is NULL) */
	ParamTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
	ParamTable[0].Name = (PWSTR)L"DeviceName";
	ParamTable[0].EntryContext = &UniDev;
	ParamTable[0].DefaultType = REG_SZ;
	ParamTable[0].DefaultData = (LPWSTR)wszDef;
	ParamTable[0].DefaultLength = sizeof(wszDef);
	/* registry values collective inquiry */
	ntStat = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL, pRegParam->Buffer, &(ParamTable[0]), NULL, NULL);
	if (ntStat != STATUS_SUCCESS)	/* failed */
	{
		KdPrint(("WEram arning:RtlQueryRegistryValues failed, 0x%x\n", ntStat));
		return;
	}
	if (UniDev.Length == 0)		/* No body */
	{
		KdPrint(("Eram No value set\n"));
		return;
	}
	KdPrint(("Eram CheckDeviceName end, device \"%ls\"\n", UniDev.Buffer));
	*pNtDevName = UniDev;
}


/* CheckSwitch
		Registry Reference and Option Settings.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pFatId		The pointer to FAT-ID structure.
		pRegParam	The pointer to the registry path string.
		pDrvStr		The pointer to the drive character.
	Return Value
		No return value.
	Registry Parameter
		AllocUnit		The cluster size.
		DriveLetter		Specify the drive.
		RootDirEntries	The number of root directory entries.
		MediaId			The media ID.
		Option			The option(s).
		Page			The page number (4KB unit).
*/

VOID CheckSwitch(
	IN PERAM_EXTENSION		pEramExt,
	IN PFAT_ID				pFatId,
	IN PUNICODE_STRING		pRegParam,
	IN OUT PUNICODE_STRING	pDrvStr
 )
{
	/* local variables */
	PRTL_QUERY_REGISTRY_TABLE		pParamTable;
	ULONG			AllocUnit,		defAllocUnit = 1024 / SECTOR;
	ULONG			RootDir,		defRootDir = 128;
	ULONG			MediaId,		defMediaId = RAMDISK_MEDIA_TYPE;
	ULONG			Option,			defOption = 0;
	ULONG			Page, 			defPage = DISKMINPAGE;
	ULONG			ExtStart,		defExtStart = 0;
	UINT			loopi;
	ULONGLONG		ulPageT;
	NTSTATUS		ntStat;
	BOOLEAN			bDefault;
	KdPrint(("Eram CheckSwitch start\n"));
	bDefault = TRUE;
	#define	REGOPTNUM	(8)
	#define	REGOPTSIZE	(REGOPTNUM * sizeof(*pParamTable))
	/* Allocate the memory for inquiry */
	pParamTable = ExAllocatePool(PagedPool, REGOPTSIZE);
	if (pParamTable != NULL)	/* Success */
	{
		/* registry confirmation area initialization */
		RtlZeroBytes(pParamTable, REGOPTSIZE);
		/* collective inquiry area initialization (the last one is NULL) */
		for (loopi=0; loopi<(REGOPTNUM-1); loopi++)
		{
			pParamTable[loopi].Flags = RTL_QUERY_REGISTRY_DIRECT;
			pParamTable[loopi].DefaultType = REG_DWORD;
			pParamTable[loopi].DefaultLength = sizeof(ULONG);
		}
		pParamTable[0].Name = (PWSTR)L"AllocUnit";
		pParamTable[0].EntryContext = &AllocUnit;
		pParamTable[0].DefaultData = &defAllocUnit;
		pParamTable[1].Name = (PWSTR)L"DriveLetter";
		pParamTable[1].EntryContext = pDrvStr;
		pParamTable[1].DefaultType = REG_NONE;
		pParamTable[1].DefaultLength = 0;
		pParamTable[2].Name = (PWSTR)L"RootDirEntries";
		pParamTable[2].EntryContext = &RootDir;
		pParamTable[2].DefaultData = &defRootDir;
		pParamTable[3].Name = (PWSTR)L"MediaId";
		pParamTable[3].EntryContext = &MediaId;
		pParamTable[3].DefaultData = &defMediaId;
		pParamTable[4].Name = (PWSTR)L"Option";
		pParamTable[4].EntryContext = &Option;
		pParamTable[4].DefaultData = &defOption;
		pParamTable[5].Name = (PWSTR)L"Page";
		pParamTable[5].EntryContext = &Page;
		pParamTable[5].DefaultData = &defPage;
		pParamTable[6].Name = (PWSTR)L"ExtStart";
		pParamTable[6].EntryContext = &ExtStart;
		pParamTable[6].DefaultData = &defExtStart;
		bDefault = FALSE;
		/* registry values collective inquiry */
		ntStat = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL, pRegParam->Buffer, pParamTable, NULL, NULL);
		if (ntStat != STATUS_SUCCESS)	/* failed */
		{
			KdPrint(("Eram Warning:RtlQueryRegistryValues failed\n"));
			bDefault = TRUE;
		}
		/* Release the memory for inquiry */
		ExFreePool(pParamTable);
	}
	if (bDefault != FALSE)	/* Incompletely read */
	{
		/* Adapt the default value */
		AllocUnit = defAllocUnit;
		RootDir = defRootDir;
		MediaId = defMediaId;
		Option = defOption;
		Page = defPage;
		ExtStart = defExtStart;
	}
	#undef	REGOPTNUM
	#undef	REGOPTSIZE
	/* Allocation unit check */
	switch (AllocUnit)
	{
	case 1:
	case 2:
	case 4:
	case 8:
	case 16:
	case 32:
	case 64:
		pFatId->BPB.byAllocUnit = (BYTE)AllocUnit;
		break;
	}
	/* Set directory entry */
	RootDir = (RootDir + 31) & 0xffe0;	/* Make it a multiple of 32 */
	if (RootDir != 0)
	{
		pFatId->BPB.wNumDirectory = (WORD)RootDir;
	}
	/* Set media ID */
	if (MediaId <= 0xff)
	{
		pFatId->BPB.byMediaId = (BYTE)MediaId;
	}
	/* option */
	pEramExt->uOptflag.dwOptflag |= Option;	/* option control */
	/* option adjustment */
	if (pEramExt->uOptflag.Bits.UseExtFile != 0)	/* External File Usage */
	{
		pEramExt->uOptflag.Bits.NonPaged = 0;
		pEramExt->uOptflag.Bits.External = 0;
	}
	else if (pEramExt->uOptflag.Bits.External != 0)	/* OS-Unmanaged Memory usage */
	{
		pEramExt->uOptflag.Bits.NonPaged = 0;
		pEramExt->uExternalStart = ExtStart;
		/* prepare for OS-Unmanaged Memory max address */
		GetMaxAddress(pEramExt, pRegParam);
	}
	if ((WORD)NtBuildNumber >= BUILD_NUMBER_NT50)	/* Windows2000+ */
	{
		/* FAT32 enabled */
		pEramExt->uOptflag.Bits.EnableFat32 = 1;
	}
	
	/* Page setting */
	if (pEramExt->uOptflag.Bits.EnableFat32 == 0)		/* Without using FAT32 */
	{
		ulPageT = ((ULONGLONG)DISKMAXCLUSTER_16 * SECTOR * pFatId->BPB.byAllocUnit) / PAGE_SIZE_4K
;
		if ((ULONGLONG)Page > ulPageT)		/* beyond FAT16 limit (There is some margin) */
		{
			Page = (ULONG)ulPageT;
			KdPrint(("Eram FAT16 limit over, adjust %d pages\n", Page));
		}
	}
	else		/* Use FAT32 */
	{
		ulPageT = ((ULONGLONG)DISKMAXCLUSTER_32 * SECTOR * pFatId->BPB.byAllocUnit) / PAGE_SIZE_4K;
		if ((ULONGLONG)Page > ulPageT)		/* beyond FAT32 limit */
		{
			Page = (ULONG)ulPageT;
			KdPrint(("Eram FAT32 limit over, adjust %d pages\n", Page));
		}
	}
	pEramExt->uSizeTotal = Page;
	/* prepare for volume label */
	PrepareVolumeLabel(pEramExt, pFatId, pRegParam);
	/* Prepare for external filename */
	PrepareExtFileName(pEramExt, pFatId, pRegParam);
	KdPrint(("Eram CheckSwitch end\n"));
}


/* GetMaxAddress
		Registry Reference (Max. Address)
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pRegParam	The pointer to the registry path string.
	Return Value
		No return value.
	Registry Parameter
		MaxAddress	The maximum address to restrict access.
*/

VOID GetMaxAddress(
	IN PERAM_EXTENSION		pEramExt,
	IN PUNICODE_STRING		pRegParam
 )
{
	/* local variables */
	RTL_QUERY_REGISTRY_TABLE	ParamTable[2];
	ULONG			uMaxAdr,	defMaxAdr = 0xffffffff;
	NTSTATUS		ntStat;
	KdPrint(("Eram GetMaxAddress start\n"));
	/* registry confirmation area initialization */
	RtlZeroBytes(&(ParamTable[0]), sizeof(ParamTable));
	/* collective inquiry area initialization (the last one is NULL) */
	ParamTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
	ParamTable[0].DefaultType = REG_DWORD;
	ParamTable[0].DefaultLength = sizeof(ULONG);
	ParamTable[0].Name = (PWSTR)L"MaxAddress";
	ParamTable[0].EntryContext = &uMaxAdr;
	ParamTable[0].DefaultData = &defMaxAdr;
	/* registry value inquiry */
	ntStat = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL, pRegParam->Buffer, &(ParamTable[0]), NULL, NULL);
	if (ntStat != STATUS_SUCCESS)	/* failed */
	{
		KdPrint(("Eram Warning:RtlQueryRegistryValues failed\n"));
		/* Adapt default value */
		uMaxAdr = defMaxAdr;
	}
	if (pEramExt->uExternalStart > uMaxAdr)
	{
		uMaxAdr = defMaxAdr;
	}
	pEramExt->uExternalEnd = uMaxAdr;
	KdPrint(("Eram GetMaxAddress end\n"));
}


/* PrepareVolumeLabel
		Registry Reference (Volume Label)
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pFatId		The pointer to FAT-ID structure.
		pRegParam	The pointer to the registry path string.
	Return Value
		No return value.
	Registry Parameter
		VolumeLabel		The volume label.
*/

VOID PrepareVolumeLabel(
	IN PERAM_EXTENSION		pEramExt,
	IN PFAT_ID				pFatId,
	IN PUNICODE_STRING		pRegParam
 )
{
	/* local variables */
	static WCHAR wszDef[] = L"";
	RTL_QUERY_REGISTRY_TABLE		ParamTable[2];
	NTSTATUS		ntStat;
	UNICODE_STRING	UniVol;
	WCHAR			wszVol[12];
	KdPrint(("Eram PrepareVolumeLabel start\n"));
	/* prepare for the initial valus for inquiry */
	wszVol[0] = UNICODE_NULL;
	RtlInitUnicodeString(&UniVol, wszVol);
	UniVol.MaximumLength = sizeof(wszVol);
	/* registry confirmation area initialization */
	RtlZeroBytes(&ParamTable, sizeof(ParamTable));
	/* collective inquiry area initialization (the last one is NULL) */
	ParamTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
	ParamTable[0].Name = (PWSTR)L"VolumeLabel";
	ParamTable[0].EntryContext = &UniVol;
	ParamTable[0].DefaultType = REG_SZ;
	ParamTable[0].DefaultData = (LPWSTR)wszDef;
	ParamTable[0].DefaultLength = sizeof(wszDef);
	/* registry values collective inquiry */
	ntStat = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL, pRegParam->Buffer, &(ParamTable[0]), NULL, NULL);
	if (ntStat != STATUS_SUCCESS)	/* failed */
	{
		KdPrint(("Eram Warning:RtlQueryRegistryValues failed, 0x%x\n", ntStat));
	}
	/* Volume label */
	RtlFillMemory(pFatId->bsLabel, sizeof(pFatId->bsLabel), ' ');
	if ((UniVol.Length == 0)||						/* No body */
		(CheckVolumeLabel(pEramExt, pFatId, &UniVol) == FALSE))		/* Invalid string specified */
	{
#pragma warning(disable : 4127)
		ASSERT((sizeof(pFatId->bsLabel)+1) == sizeof(VOLUME_LABEL_LOCALDISK));
		ASSERT((sizeof(pFatId->bsLabel)+1) == sizeof(VOLUME_LABEL_RAMDISK));
#pragma warning(default : 4127)
		RtlCopyBytes(pFatId->bsLabel, (PSTR)((pEramExt->uOptflag.Bits.Swapable != 0) ? VOLUME_LABEL_LOCALDISK : VOLUME_LABEL_RAMDISK), sizeof(pFatId->bsLabel));
	}
	KdPrint(("Eram PrepareVolumeLabel end, Volume label \"%s\"\n", pFatId->bsLabel));
}


/* CheckVolumeLabel
		Volume Label Validation and Preparation.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pFatId		The pointer to FAT-ID structure.
		pUniVol		The pointer to the volume label string that was read from registry.
	Return Value
		Results.		TRUE: Ready.
*/

BOOLEAN CheckVolumeLabel(
	IN PERAM_EXTENSION		pEramExt,
	IN PFAT_ID				pFatId,
	IN PUNICODE_STRING		pUniVol
 )
{
	/* local variables */
	static CHAR cBadChars[] = "*?/|.,;:+=[]()&^<>\"";
	ANSI_STRING		AnsiVol;
	DWORD loopi, loopj;
	/* 不正文字検索 */
	for (loopi=0; loopi<(pUniVol->Length / sizeof(WCHAR)); loopi++)
	{
		if (HIBYTE(pUniVol->Buffer[loopi]) == 0)
		{
			if (LOBYTE(pUniVol->Buffer[loopi]) < ' ')
			{
				KdPrint(("Eram Bad char 0x%x detected, index %d\n", LOBYTE(pUniVol->Buffer[loopi]), loopi));
				return FALSE;
			}
			for (loopj=0; loopj<(sizeof(cBadChars)-1); loopj++)
			{
				if (LOBYTE(pUniVol->Buffer[loopi]) == cBadChars[loopj])
				{
					KdPrint(("Eram Bad char \"%c\" detected, index %d\n", cBadChars[loopj], loopi));
					return FALSE;
				}
			}
		}
	}
	/* ANSI文字列化 */
	if (RtlUnicodeStringToAnsiString(&AnsiVol, pUniVol, TRUE) != STATUS_SUCCESS)
	{
		KdPrint(("Eram RtlUnicodeStringToAnsiString failed\n"));
		return FALSE;
	}
	if (AnsiVol.Length == 0)		/* without body */
	{
		KdPrint(("Eram Ansi string 0 byte\n"));
		RtlFreeAnsiString(&AnsiVol);
		return FALSE;
	}
	/* 準備 */
	RtlCopyBytes(pFatId->bsLabel, AnsiVol.Buffer, (AnsiVol.Length > sizeof(pFatId->bsLabel)) ? sizeof(pFatId->bsLabel) : AnsiVol.Length);
	RtlFreeAnsiString(&AnsiVol);
	KdPrint(("Eram CheckVolumeLabel end, Volume label \"%s\"\n", pFatId->bsLabel));
	return TRUE;
}


/* PrepareExtFileName
		Registry Reference (External Filename).
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pFatId		The pointer to FAT-ID structure.
		pRegParam	The pointer to the registry path string.
	Return Value
		No return value.
	Registry Parameter
		ExtFileName		External Filename.
*/

VOID PrepareExtFileName(
	IN PERAM_EXTENSION		pEramExt,
	IN PFAT_ID				pFatId,
	IN PUNICODE_STRING		pRegParam
 )
{
	/* local variables */
	static WCHAR wszDef[] = L"";
	static WCHAR wszExtStub[] = L"\\??\\";
	static WCHAR wszExtPath[] = ERAMEXTFILEPATH;
	RTL_QUERY_REGISTRY_TABLE		ParamTable[2];
	NTSTATUS		ntStat;
	UNICODE_STRING	UniExtFile;
	KdPrint(("Eram PrepareExtFileName start\n"));
	/* prepare for the initial valus for inquiry */
	pFatId->wszExtFileMain[0] = UNICODE_NULL;
	RtlInitUnicodeString(&UniExtFile, pFatId->wszExtFileMain);
	UniExtFile.MaximumLength = sizeof(pFatId->wszExtFileMain);
	/* registry confirmation area initialization */
	RtlZeroBytes(&ParamTable, sizeof(ParamTable));
	/* collective area initialization (the last one is NULL) */
	ParamTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
	ParamTable[0].Name = (PWSTR)L"ExtFileName";
	ParamTable[0].EntryContext = &UniExtFile;
	ParamTable[0].DefaultType = REG_SZ;
	ParamTable[0].DefaultData = (LPWSTR)wszDef;
	ParamTable[0].DefaultLength = sizeof(wszDef);
	/* registry values collective inquiry */
	ntStat = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL, pRegParam->Buffer, &(ParamTable[0]), NULL, NULL);
	if (ntStat != STATUS_SUCCESS)	/* failed */
	{
		KdPrint(("Eram Warning:RtlQueryRegistryValues failed\n"));
	}
	/* External File Name */
#pragma warning(disable : 4127)
	ASSERT(sizeof(pFatId->wszExtFile) == (sizeof(wszExtStub) - sizeof(WCHAR)));
#pragma warning(default : 4127)
	RtlCopyBytes(pFatId->wszExtFile, wszExtStub, sizeof(pFatId->wszExtFile));
	if (UniExtFile.Length == 0)			/* Without External Filename setting */
	{
		RtlCopyBytes(pFatId->wszExtFileMain, wszExtPath, sizeof(wszExtPath));
	}
	KdPrint(("Eram PrepareExtFileName end, External file \"%ls\"\n", pFatId->wszExtFile));
}


/* EramFormatFat
		ERAM Initialization.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pFatId		The pointer to FAT-ID structure.
	Return Value
		Results.
*/

BOOLEAN EramFormatFat(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 )
{
	KdPrint(("EramFormatFat start\n"));
	/* FAT info setup */
	EramSetup(pEramExt, pFatId);
	/* dynamic relocation of routine */
	EramLocate(pEramExt);
	/* ERAM Format */
	if (EramFormat(pEramExt, pFatId) == FALSE)
	{
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "EramFormat");
		return FALSE;
	}
	KdPrint(("EramFormatFat end\n"));
	return TRUE;
}


/* EramSetup
		Disk Info Setup.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pFatId		The pointer to FAT-ID structure.
	Return Value
		No return value.
*/

VOID EramSetup(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 )
{
	/* local variables */
	UINT	AllocLog2;
	DWORD	eax, esi, edi, ebx, edx, dwFatSectorCount, dwFatEntries;
	KdPrint(("EramSetup start\n"));
	/* log2(allocation size) calculation */
	switch (pFatId->BPB.byAllocUnit)
	{
	case 2:
		AllocLog2 = 1;
		break;
	case 4:
		AllocLog2 = 2;
		break;
	case 8:
		AllocLog2 = 3;
		break;
	case 16:
		AllocLog2 = 4;
		break;
	case 32:
		AllocLog2 = 5;
		break;
	case 64:
		AllocLog2 = 6;
		break;
	default:
		AllocLog2 = 0;
	}
	edi = (DWORD)(pEramExt->uSizeTotal << PAGE_SEC_LOG2); //TODO: this must be 32bit?
	pEramExt->uAllSector = edi;						/* The total number of sectors */
	pFatId->BPB.wNumAllSector = (WORD)edi;			/* The total number of sectors */
	if (edi >= 0x10000)								/* over 32MB */
	{
		pFatId->BPB.wNumAllSector = 0;
		pFatId->BPB_ext.bsHugeSectors = edi;			/* The total number of sectors */
		if (pEramExt->uOptflag.Bits.EnableFat32 != 0)	/* FAT32 enabled */
		{
			/* Add the reserved 2 entries excluding boot sectors and FsInfo sectors */
			dwFatEntries = ((edi - RESV_SECTOR_FAT32) >> AllocLog2) + 2;
			if (dwFatEntries > DISKMAXCLUSTER_16)		/* The number of clusters is large */
			{
				dwFatSectorCount = (dwFatEntries * 4 + (SECTOR - 1)) / SECTOR;
				dwFatEntries -= dwFatSectorCount;
				if (dwFatEntries > DISKMAXCLUSTER_16)		/* The number of clusters is large */
				{
					/* Set FAT32 usage */
					pEramExt->FAT_size = PARTITION_FAT32;
					pFatId->BPB.wNumDirectory = 0;
					pFatId->BPB_fat32.dwNumFatSector32 = dwFatSectorCount;
					pFatId->BPB.wNumResvSector = RESV_SECTOR_FAT32;	/* The number of reserved sectors (=BootSector+FsInfo) */
					KdPrint(("EramSetup end(FAT32)\n"));
					return;
				}
			}
		}
	}
	esi = pFatId->BPB.wNumDirectory;	/* The number of directory entries */
	esi >>= (SECTOR_LOG2 - 5);			/* 16 items in 1 sectors ... SI = The number of directory sectors */
	if ((esi == 0)||					/* without specifying directory */
		((edi >> 1) <= esi))			/* Directory specification exceeds half of all sectors */
	{
		pFatId->BPB.wNumDirectory = 128;						/* Forcibly change to default value */
		esi = pFatId->BPB.wNumDirectory >> (SECTOR_LOG2 - 5);	/* 16 items in 1 sector ... SI = The number of directory sectors */
	}
	edi -= (esi + 1);					/* di = The total number of sectors - The number of directory sectors - The number of reserved sectors */
	edx = edi;							/* The number of sectors available */
	edi >>= AllocLog2;					/* The estimated number of clusters = di / sectors per cluster */
	edi++;								/* The estimated number of clusters +1 */
	pEramExt->FAT_size = PARTITION_FAT_12;
	/* AllocLog2 = log2  allocation size
		dx = The number of sectors available
		si = The number of sectors for the root directory
		di = The estimated number of clusters + 1
	*/
	do
	{
		edi--;					/* The estimated number of clusters -1 */
		if (edi > DISKMAXCLUSTER_12)	/* 16bit FAT if 0FF7h or more */
		{
			pEramExt->FAT_size = PARTITION_FAT_16;
			eax = edi;
			eax <<= 1;			/* eax = The number o FAT bytes (twice) */
			eax += (SECTOR + 3);
			eax >>= SECTOR_LOG2;/* eax = The number of FAT sectors */
			pFatId->BPB.wNumFatSector = (WORD)eax;
		}
		else	/* 12bit FAT */
		{
			pEramExt->FAT_size = PARTITION_FAT_12;
			eax = edi;
			eax *= 3;
			eax >>= 1;			/* ax = The number of FAT bytes (x 1.5) */
			eax += (SECTOR + 2);
			eax >>= SECTOR_LOG2;	/* ax = The number of FAT sectors  */
			pFatId->BPB.wNumFatSector = (WORD)eax;
		}
		ebx = edi;				/* bx = The number of data sectors  */
		ebx <<= AllocLog2;
		ebx += eax;				/* + The number of FAT sectors */
	} while (ebx > edx);
	/* Adjustment of usage area with 12bit FAT.
		0FF5h:FD mistaken as 16bit FAT.
	*/
	if ((pEramExt->FAT_size == PARTITION_FAT_12)&&
		(edi > (DISKMAXCLUSTER_12 - 3)))
	{
		edi -= (DISKMAXCLUSTER_12 - 3);
		edi <<= AllocLog2;
		edi <<= (SECTOR_LOG2 - 5);
#pragma warning(disable : 4244)
		pFatId->BPB.wNumDirectory += (WORD)edi;	/* Fix by the number of root directories */
#pragma warning(default : 4244)
	}
	/* Adjustment of usage area in 16bit FAT
		0FFF6h:FAT32
	*/
	if ((pEramExt->FAT_size == PARTITION_FAT_16)&&
		(edi > DISKMAXCLUSTER_16 - 1))
	{
		edi -= DISKMAXCLUSTER_16 - 1;
		edi <<= AllocLog2;
		edi <<= (SECTOR_LOG2 - 5);
#pragma warning(disable : 4244)
		pFatId->BPB.wNumDirectory += (WORD)edi;	/* Fix by the number of root directories */
#pragma warning(default : 4244)
	}
	edx -= ebx;			/* bx = the number of real usage sectors */
	if (edx != 0)		/* Extra sectors will be passed to dir */
	{
		edx <<= (SECTOR_LOG2 - 5);
#pragma warning(disable : 4244)
		pFatId->BPB.wNumDirectory += (WORD)edx;
#pragma warning(default : 4244)
	}
	if ((pEramExt->FAT_size == PARTITION_FAT_16)&&
		(pFatId->BPB.wNumAllSector == 0))	/* FAT16 over 32MB */
	{
		pEramExt->FAT_size = PARTITION_HUGE;
	}
	KdPrint(("EramSetup end(FAT12,16)\n"));
}


/* EramLocate
		Routine Dynamic Arrangement.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
	Return Value
		No return value.
*/

VOID EramLocate(
	IN PERAM_EXTENSION	pEramExt
 )
{
	KdPrint(("EramLocate start\n"));
	if (pEramExt->uOptflag.Bits.External != 0)	/* Use OS-Unmanaged Memory */
	{
		pEramExt->EramRead = (ERAM_READ)ExtRead1;
		pEramExt->EramWrite = (ERAM_WRITE)ExtWrite1;
		pEramExt->EramNext = (ERAM_NEXT)ExtNext1;
		pEramExt->EramUnmap = (ERAM_UNMAP)ExtUnmap;
	}
	else if (pEramExt->uOptflag.Bits.UseExtFile != 0)	/* Use file */
	{
		pEramExt->EramRead = (ERAM_READ)ExtFilePendingRw;
		pEramExt->EramWrite = (ERAM_WRITE)ExtFilePendingRw;
		pEramExt->EramNext = (ERAM_NEXT)ExtFileNext1;
		pEramExt->EramUnmap = (ERAM_UNMAP)ExtFileUnmap;
	}
	else		/* normal */
	{
		pEramExt->EramRead = (ERAM_READ)ReadPool;
		pEramExt->EramWrite = (ERAM_WRITE)WritePool;
	}
	KdPrint(("EramLocate end\n"));
}


/* EramFormat
		Formatting.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pFatId		The pointer to FAT-ID structure.
	Return Value
		Results.
*/

BOOLEAN EramFormat(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 )
{
	KdPrint(("EramFormat start\n"));
	/* management area initialization */
	if (EramClearInfo(pEramExt, pFatId) == FALSE)
	{
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "EramClearInfo");
		return FALSE;
	}
	/* FAT initialization */
	if (EramMakeFAT(pEramExt, pFatId) == FALSE)
	{
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "EramMakeFAT");
		return FALSE;
	}
	/* Set the volume label */
	if (EramSetLabel(pEramExt, pFatId) == FALSE)
	{
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "EramSetLabel");
		return FALSE;
	}
	KdPrint(("EramFormat end\n"));
	return TRUE;
}


/* EramClearInfo
		Area Clear.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pFatId		The pointer to FAT-ID structure.
	Return Value
		Results.
*/

BOOLEAN EramClearInfo(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 )
{
	/* local variables */
	ULONGPTR uSize;
	/* management area size calculation */
	uSize = CalcEramInfoPage(pEramExt, pFatId);
	if (pEramExt->uOptflag.Bits.External != 0)	/* OS-Unmanaged Memory */
	{
		/* OS-Unmanaged Memory initialization */
		if (ExtClear(pEramExt, uSize) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "ExtClear");
			return FALSE;
		}
		return TRUE;
	}
	if (pEramExt->uOptflag.Bits.UseExtFile != 0)	/* External File Usage */
	{
		/* file initialization */
		if (ExtFileClear(pEramExt, uSize) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "ExtFileClear");
			return FALSE;
		}
		return TRUE;
	}
	/* OS-Managed Memory initialization */
	RtlZeroBytes(pEramExt->pPageBase, uSize);
	return TRUE;
}


/* ExtClear
		OS-Unmanaged Memory Initialization.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		uSize		The byte number of management info area. TODO: probabily could be 32bits
	Return Value
		Results.
*/

BOOLEAN ExtClear(
	IN PERAM_EXTENSION	pEramExt,
	IN ULONGPTR			uSize
 )
{
	/* local variables */
	ULONGPTR loopi;
	KdPrint(("Eram ExtClear start\n"));
	ASSERT(pEramExt->uExternalStart != 0);
	ASSERT(pEramExt->uExternalEnd != 0);
	for (loopi=0; loopi<uSize; loopi+=EXT_PAGE_SIZE)
	{
		if ((pEramExt->uExternalStart + loopi) >= pEramExt->uExternalEnd)	/* Beyond real memory */
		{
			KdPrint(("Eram Warning:Address limited\n"));
			break;
		}
		/* map */
		if (ExtMap(pEramExt, loopi) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "ExtMap");
			return FALSE;
		}
		//KdPrint(("Eram loop 0x%x, phys 0x%X\n", loopi, (pEramExt->uExternalStart + loopi)));
		/* zero clear */
		RtlZeroBytes(pEramExt->pExtPage, ((uSize - loopi) > EXT_PAGE_SIZE ? EXT_PAGE_SIZE : (uSize - loopi)));
	}
	/* Unmap */
	ExtUnmap(pEramExt);
	KdPrint(("Eram ExtClear end\n"));
	return TRUE;
}


/* ExtFileClear
		File Initialization.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		uSize		The byte number of management info area.  TODO: probabily can be 32bit
	Return Value
		Results.
*/

BOOLEAN ExtFileClear(
	IN PERAM_EXTENSION	pEramExt,
	IN ULONGPTR			uSize
 )
{
	/* local variables */
	ULONGPTR loopi;
	KdPrint(("Eram ExtFileClear start\n"));
	for (loopi=0; loopi<uSize; loopi+=EXT_PAGE_SIZE)
	{
		/* map */
		if (ExtFileMap(pEramExt, loopi) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "ExtFileMap");
			return FALSE;
		}
		//KdPrint(("Eram loop 0x%x, phys 0x%X\n", loopi, (pEramExt->uExternalStart + loopi)));
		/* zero clear */
		RtlZeroBytes(pEramExt->pExtPage, ((uSize - loopi) > EXT_PAGE_SIZE ? EXT_PAGE_SIZE : (uSize - loopi)));
	}
	/* Unmap */
	ExtFileUnmap(pEramExt);
	KdPrint(("Eram ExtFileClear end\n"));
	return TRUE;
}


/* CalcEramInfoPage
		Management Area Size Calculation.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pFatId		The pointer to FAT-ID structure.
	Return Value
		Results.
*/

ULONGPTR CalcEramInfoPage(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 )
{
	/* local variables */
	DWORD dwPage, dwTmp;
	ULONGPTR dwBytes; //TODO: just a counter right?

	KdPrint(("CalcEramInfoPage start\n"));
	if (pEramExt->FAT_size == PARTITION_FAT32)	/* FAT32 */
	{
		dwPage = pFatId->BPB_fat32.dwNumFatSector32 + pFatId->BPB.wNumResvSector + pFatId->BPB_fat32.dwRootCluster * (pFatId->BPB.byAllocUnit);
	}
	else		/* FAT12/16 */
	{
		dwPage = pFatId->BPB.wNumFatSector + pFatId->BPB_fat32.dwNumFatSector32;	/* The number of FAT sectors */
		dwTmp = pFatId->BPB.wNumDirectory;	/* The number of directory entries */
		dwTmp >>= (SECTOR_LOG2 - 5);			/* The number of directory sectors */
		dwPage += dwTmp;						/* FAT + directory */
		dwPage += pFatId->BPB.wNumResvSector;	/* The number of reserved sectors */
	}
	if (pEramExt->uOptflag.Bits.MakeTempDir != 0)	/* TEMP directory creation */
	{
		/* Increment clusters */
		dwPage += pFatId->BPB.byAllocUnit;
	}
	dwPage += (PAGE_SECTOR - 1);			/* For round-up */
	dwPage >>= PAGE_SEC_LOG2;				/* The number of pages */
	dwBytes = (dwPage << PAGE_SIZE_LOG2);	/* The number of byte for initialization */
	if (dwBytes > (pEramExt->uSizeTotal * PAGE_SIZE_4K))
	{
		dwBytes = pEramExt->uSizeTotal * PAGE_SIZE_4K;
	}
	KdPrint(("CalcEramInfoPage end, 0x%x kb\n", (DWORD)(dwBytes/1024)));
	return dwBytes;
}


/* EramMakeFAT
		FAT Creation.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pFatId		The pointer to FAT-ID structure.
	Return Value
		Results.
*/

BOOLEAN EramMakeFAT(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 )
{
	/* local variables */
	WORD wVal;
	PBYTE pDisk;
	PDWORD pdwFatSector;
	PBOOTSECTOR_FAT16 pBootFat16;
	PBOOTSECTOR_FAT32 pBootFat32;
	PFSINFO_SECTOR pFsInfoSector;
	LARGE_INTEGER SystemTime, LocalTime;
	DWORD eax, ebx;
	KdPrint(("EramMakeFAT start\n"));
	/* Get the date and time */
	KeQuerySystemTime(&SystemTime);						/* Get the system time */
	ExSystemTimeToLocalTime(&SystemTime, &LocalTime);	/* Convert to local time */
	RtlTimeToTimeFields(&LocalTime, &(pFatId->TimeInfo));	/* Convert to the structure */
	if ((pFatId->TimeInfo.Year < 1980)||(pFatId->TimeInfo.Year > 2079))	/* the year is beyond the limit of DOS */
	{
		/* Set the year 2004 */
		pFatId->TimeInfo.Year = 2004;
	}
	/* Prepare for volume serial number */
	wVal = pFatId->TimeInfo.Year;
	pFatId->BPB_ext2.bsVolumeID = (wVal / 1000) << 28;
	wVal %= 1000;
	pFatId->BPB_ext2.bsVolumeID |= ((wVal / 100) << 24);
	wVal %= 100;
	pFatId->BPB_ext2.bsVolumeID |= ((wVal / 10) << 20);
	pFatId->BPB_ext2.bsVolumeID |= ((wVal % 10) << 16);
	pFatId->BPB_ext2.bsVolumeID |= ((pFatId->TimeInfo.Month / 10) << 12);
	pFatId->BPB_ext2.bsVolumeID |= ((pFatId->TimeInfo.Month % 10) << 8);
	pFatId->BPB_ext2.bsVolumeID |= ((pFatId->TimeInfo.Day / 10) << 4);
	pFatId->BPB_ext2.bsVolumeID |= (pFatId->TimeInfo.Day % 10);
	pDisk = pEramExt->pPageBase;
	if ((pEramExt->uOptflag.Bits.External != 0)||		/* OS-Unmanaged Memory usage */
		(pEramExt->uOptflag.Bits.UseExtFile != 0))		/* External File Usage */
	{
		/* boot sector allocation */
		ebx = 0;
		ASSERT((pEramExt->EramNext) != NULL);
		if ((*(pEramExt->EramNext))(pEramExt, &eax, &ebx) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "EramNext");
			return FALSE;
		}
		pDisk = (PBYTE)((pEramExt->pExtPage + eax)); //(ULONG)
	}
	/* Write the boot sector common part */
	pBootFat16 = (PBOOTSECTOR_FAT16)pDisk;
	pBootFat16->bsJump[0] = 0xeb;
	pBootFat16->bsJump[1] = 0xfe;
	pBootFat16->bsJump[2] = 0x90;
	RtlCopyBytes(pBootFat16->bsOemName, "ERAM    ", sizeof(pBootFat16->bsOemName));
	RtlCopyBytes(&(pBootFat16->BPB), &(pFatId->BPB), sizeof(pBootFat16->BPB));
	RtlCopyBytes(&(pBootFat16->BPB_ext), &(pFatId->BPB_ext), sizeof(pBootFat16->BPB_ext));
	RtlCopyBytes(pBootFat16->szMsg, FATID_MSG, sizeof(FATID_MSG));
	if (pEramExt->FAT_size != PARTITION_FAT32)	/* FAT12,16 */
	{
		/* Write the boot sector (FAT12, 16) */
		RtlCopyBytes(&(pBootFat16->BPB_ext2), &(pFatId->BPB_ext2), sizeof(pBootFat16->BPB_ext2));
	}
	else										/* FAT32 */
	{
		/* Write the boot sector (FAT32) */
		pBootFat32 = (PBOOTSECTOR_FAT32)pDisk;
		RtlCopyBytes(&(pBootFat32->BPB_fat32), &(pFatId->BPB_fat32), sizeof(pBootFat32->BPB_fat32));
		RtlCopyBytes(&(pBootFat32->BPB_ext2), &(pFatId->BPB_ext2), sizeof(pBootFat32->BPB_ext2));
		/* Write the FSINFO sector */
		pFsInfoSector = (PFSINFO_SECTOR)((PBYTE)pBootFat32 + pBootFat32->BPB_fat32.wFsInfoSector * SECTOR);
		pFsInfoSector->FSInfo_Sig = 0x41615252;				/* RRaA */
		pFsInfoSector->FsInfo.bfFSInf_Sig = 0x61417272;		/* rrAa */
		pFsInfoSector->FsInfo.bfFSInf_free_clus_cnt = 0xffffffff;
		pFsInfoSector->FsInfo.bfFSInf_next_free_clus = 2;
		pFsInfoSector->bsSig2[0] = 0x55;
		pFsInfoSector->bsSig2[1] = 0xaa;
		if ((pBootFat32->BPB_fat32.wBackupBootSector != 0xffff)&&	/* Backup boot sector exists */
			(pBootFat32->BPB.wNumResvSector > (pBootFat32->BPB_fat32.wBackupBootSector + pBootFat32->BPB_fat32.wFsInfoSector))&&
			((pBootFat32->BPB_fat32.wBackupBootSector + pBootFat32->BPB_fat32.wFsInfoSector) < (EXT_PAGE_SIZE / SECTOR)))
		{
			/* Do backup the boot sector */
			RtlCopyBytes((PBYTE)((PBYTE)pBootFat32 + pBootFat32->BPB_fat32.wBackupBootSector * SECTOR), pBootFat32, sizeof(*pBootFat32)); //(ULONG)
			/* Do backup the FSINFO sector */
			RtlCopyBytes((PBYTE)((PBYTE)pFsInfoSector + pBootFat32->BPB_fat32.wBackupBootSector * SECTOR), pFsInfoSector, sizeof(*pFsInfoSector)); //(ULONG)
		}
	}
	/* Write the FAT sector */
	pdwFatSector = (PDWORD)((PBYTE)pBootFat16 + pBootFat16->BPB.wNumResvSector * SECTOR);
	pdwFatSector[0] = 0xffffff00 + pFatId->BPB.byMediaId;
	if (pEramExt->FAT_size == PARTITION_FAT_12)	/* FAT12 */
	{
		if (pEramExt->uOptflag.Bits.MakeTempDir != 0)		/* TEMP creation */
		{
			/* Make cluster 2 in use (total 36bits) */
			((PBYTE)pdwFatSector)[4] = 0xf;
		}
		else
		{
			/* Limit to 24bit */
			((PBYTE)pdwFatSector)[3] = 0;
		}
	}
	else if (pEramExt->FAT_size == PARTITION_FAT32)	/* FAT32 */
	{
		pdwFatSector[1] = 0xffffffff;
		/* Set root directory also */
		pdwFatSector[pFatId->BPB_fat32.dwRootCluster] = 0x0fffffff;
		if (pEramExt->uOptflag.Bits.MakeTempDir != 0)		/* TEMP creation */
		{
			/* Make the next cluster of the root directory in use (total 96bits) */
			pdwFatSector[pFatId->BPB_fat32.dwRootCluster + 1] = 0x0fffffff;
		}
	}
	else if (pEramExt->uOptflag.Bits.MakeTempDir != 0)		/* TEMP creation in FAT16 */
	{
		/* Make cluster 2 in use (total 48bits) */
		pdwFatSector[1] = 0xffff;
	}
	if (pEramExt->uOptflag.Bits.External != 0)	/* OS-Unmanaged Memory usage */
	{
		/* Unmap */
		ExtUnmap(pEramExt);
	}
	else if (pEramExt->uOptflag.Bits.UseExtFile != 0)	/* External File Usage */
	{
		/* Unmap */
		ExtFileUnmap(pEramExt);
	}
	KdPrint(("EramMakeFAT end\n"));
	return TRUE;
}


/* EramSetLabel
		Volume Label Setting.
	Parameters
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pFatId		The pointer to FAT-ID structure.
	Return Value
		Results.
*/

BOOLEAN EramSetLabel(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 )
{
	/* local variables */
	vol_label VOL_L;
	dir_init DirInit;
	DWORD eax, dwDirSector, dwTempSector;
	KdPrint(("EramSetLabel start\n"));
	/* Initialization */
	RtlZeroBytes(&VOL_L, sizeof(VOL_L));
	RtlZeroBytes(&DirInit, sizeof(DirInit));
	/* Prepare for volume label */
	RtlCopyBytes(VOL_L.vol.sName, pFatId->bsLabel, sizeof(VOL_L.vol.sName));
	KdPrint(("Eram Volume label \"%s\"\n", VOL_L.vol.sName));
	/* Set attributes */
	VOL_L.vol.uAttr.Bits.byVol = 1;
	VOL_L.vol.uAttr.Bits.byA = 1;
	/* Set date and time */
	VOL_L.vol.wUpdMinute |= (pFatId->TimeInfo.Second >> 1);				/* Chop 1 bit from seconds */
	VOL_L.vol.wUpdMinute |= (pFatId->TimeInfo.Minute << 5);				/* Set minutes */
	VOL_L.vol.wUpdMinute |= (pFatId->TimeInfo.Hour << (5+6));			/* Set seconds */
	VOL_L.vol.wUpdDate |= pFatId->TimeInfo.Day;
	VOL_L.vol.wUpdDate |= (pFatId->TimeInfo.Month << 5);
	VOL_L.vol.wUpdDate |= ((pFatId->TimeInfo.Year - 1980) << (5+4));
	/* Get the directory sector position */
	dwDirSector = pFatId->BPB.wNumFatSector + pFatId->BPB.wNumResvSector;
	if (pEramExt->FAT_size == PARTITION_FAT32)		/* FAT32 */
	{
		dwDirSector = pFatId->BPB_fat32.dwNumFatSector32 + pFatId->BPB.wNumResvSector + (pFatId->BPB_fat32.dwRootCluster - 2) * (pFatId->BPB.byAllocUnit);
	}
	if (pEramExt->uOptflag.Bits.MakeTempDir != 0)		/* TEMP directory creation */
	{
		/* Prepare for directory */
		RtlCopyBytes(VOL_L.temp.sName, TEMPDIR_NAME, sizeof(VOL_L.temp.sName));
		/* Set attributes */
		VOL_L.temp.uAttr.Bits.byDir = 1;
		VOL_L.temp.uAttr.Bits.byA = 1;
		/* Set date and time */
		VOL_L.temp.wUpdMinute = VOL_L.vol.wUpdMinute;
		VOL_L.temp.wUpdDate = VOL_L.vol.wUpdDate;
		if (pEramExt->FAT_size == PARTITION_FAT32)		/* FAT32 */
		{
			/* The first sector number setting */
			VOL_L.temp.wCluster = (WORD)(pFatId->BPB_fat32.dwRootCluster + 1);
			/* The number of first sectors calculation */
			dwTempSector = pFatId->BPB_fat32.dwNumFatSector32 + pFatId->BPB.wNumResvSector + (VOL_L.temp.wCluster - 2) * (pFatId->BPB.byAllocUnit);
		}
		else
		{
			/* The first sector number setting */
			VOL_L.temp.wCluster = 2;
			/* The number of first sectors calculation */
			dwTempSector = dwDirSector + (pFatId->BPB.wNumDirectory >> (SECTOR_LOG2 - 5));
		}
		/* Prepare for directory */
		RtlCopyBytes(DirInit.own.sName, OWNDIR_NAME, sizeof(DirInit.own.sName));
		RtlCopyBytes(DirInit.parent.sName, PARENTDIR_NAME, sizeof(DirInit.parent.sName));
		/* Set attributes */
		DirInit.own.uAttr.byAttr = VOL_L.temp.uAttr.byAttr;
		DirInit.parent.uAttr.byAttr = VOL_L.temp.uAttr.byAttr;
		/* Set date and time */
		DirInit.own.wUpdMinute = VOL_L.vol.wUpdMinute;
		DirInit.own.wUpdDate = VOL_L.vol.wUpdDate;
		DirInit.parent.wUpdMinute = VOL_L.vol.wUpdMinute;
		DirInit.parent.wUpdDate = VOL_L.vol.wUpdDate;
		/* The first sector number setting */
		DirInit.own.wCluster = VOL_L.temp.wCluster;
	}
	if ((pEramExt->uOptflag.Bits.External != 0)||		/* OS-Unmanaged Memory usage */
		(pEramExt->uOptflag.Bits.UseExtFile != 0))		/* External File Usage */
	{
		/* allocate directory sector */
		ASSERT((pEramExt->EramNext) != NULL);
		if ((*(pEramExt->EramNext))(pEramExt, &eax, &dwDirSector) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "EramNext");
			return FALSE;
		}
		/* Write the volume label */
		RtlCopyBytes((LPBYTE)((LPBYTE)(pEramExt->pExtPage) + eax), &VOL_L, sizeof(VOL_L));
		if (pEramExt->uOptflag.Bits.MakeTempDir != 0)		/* TEMP directory creation */
		{
			/* Allocate the TEMP directory sector */
			if ((*(pEramExt->EramNext))(pEramExt, &eax, &dwTempSector) == FALSE)
			{
				EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "EramNext");
				return FALSE;
			}
			/* Write directory */
			RtlCopyBytes((LPBYTE)((LPBYTE)(pEramExt->pExtPage) + eax), &DirInit, sizeof(DirInit));
		}
		/* Unmap */
		ASSERT((pEramExt->EramUnmap) != NULL);
		(*(pEramExt->EramUnmap))(pEramExt);
	}
	else
	{
		/* Write the volume label to the directory sector */
		RtlCopyBytes((LPBYTE)((PBYTE)(pEramExt->pPageBase) + (dwDirSector << SECTOR_LOG2)), &VOL_L, sizeof(VOL_L));
		if (pEramExt->uOptflag.Bits.MakeTempDir != 0)		/* TEMP directory creation */
		{
			/* Write the directory to TEMP directory sector */
			RtlCopyBytes((LPBYTE)((PBYTE)(pEramExt->pPageBase) + (dwTempSector << SECTOR_LOG2)), &DirInit, sizeof(DirInit));
		}
	}
	KdPrint(("EramSetLabel end\n"));
	return TRUE;
}


/* GetExternalStart
		OS-Unmanaged Memory Starting Position Detection.
	Parameters
		pDrvObj		The pointer to device representative object.
		pEramExt	The pointer to an ERAM_EXTENTION structure.
	Return Value
		Results.	TRUE: With OS-Unmanaged Memory.
*/

BOOLEAN GetExternalStart(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt
 )
{
	/* local variables */
	RTL_QUERY_REGISTRY_TABLE	ParamTable[2];
	NTSTATUS					ntStat;
	UNICODE_STRING				uniOption, uniOptionUp;
	PWSTR						pBuf, pwStr;
	ULONG						uSize, uMaxMem, loopi, uRemain, uNoLowMem, uStart;
	BOOLEAN						bStat, bNoLowMem;
	PHYSICAL_ADDRESS			phys;
	static WCHAR		szwMaxMem[] = L"MAXMEM=";
	static WCHAR		szwNoLowMem[] = L"NOLOWMEM";
	KdPrint(("Eram GetExternalStart start\n"));
	uSize = 512 * sizeof(WCHAR);
	pBuf = ExAllocatePool(PagedPool, uSize);
	if (pBuf == NULL)		/* allocation failed */
	{
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_OPTION_WORK_ALLOC_FAILED, NULL);
		return FALSE;
	}
	/* prepare for info area */
	RtlInitUnicodeString(&uniOption, UNICODE_NULL);
	uniOption.Buffer = pBuf;
	uniOption.MaximumLength = (USHORT)uSize;
	/* registry confirmation area initialization */
	RtlZeroBytes(&(ParamTable[0]), sizeof(ParamTable));
	ParamTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
	ParamTable[0].Name = (PWSTR)L"SystemStartOptions";
	ParamTable[0].EntryContext = &uniOption;
	/* registry values collective inquiry */
	ntStat = RtlQueryRegistryValues(RTL_REGISTRY_CONTROL, NULL, &(ParamTable[0]), NULL, NULL);
	if (ntStat != STATUS_SUCCESS)	/* failed */
	{
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_OPTION_GET_FAILED, NULL);
		/* memory release */
		ExFreePool(pBuf);
		return FALSE;
	}
	if (uniOption.Length == 0)	/* Without option */
	{
		KdPrint(("Eram No startup option\n"));
		/* memory release */
		ExFreePool(pBuf);
		return FALSE;
	}
	/* / PAE judgment from physical address */
	uNoLowMem = 0;
	phys = MmGetPhysicalAddress(pBuf);
	
	uMaxMem = sizeof(szwMaxMem) - sizeof(WCHAR);
	if (max(uMaxMem, uNoLowMem) >= uniOption.Length)	/* MAXMEM / NOLOWMEM not included in option */
	{
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_MAXMEM_NO_OPTION, NULL);
		/* memory release */
		ExFreePool(pBuf);
		return FALSE;
	}
	if (uNoLowMem == 0)
	{
		uNoLowMem = MAXDWORD;
	}
	KdPrint(("Eram Startup option exist\n"));
	/* Capitalize (already done at NT4) */
	if (RtlUpcaseUnicodeString(&uniOptionUp , &uniOption, TRUE) != STATUS_SUCCESS)
	{
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_MAXMEM_CAPITAL_FAILED, NULL);
		/* memory release */
		ExFreePool(pBuf);
		return FALSE;
	}
	/* memory release */
	ExFreePool(pBuf);
	KdPrint(("Eram Start Parse\n"));
	pwStr = uniOptionUp.Buffer;
	bStat = FALSE;
	bNoLowMem = FALSE;
	uStart = 0;
	uRemain = uniOptionUp.Length;
	for (loopi=0; loopi<(uniOptionUp.Length /sizeof(WCHAR)); loopi++, pwStr++, uRemain-=sizeof(WCHAR))
	{
		if ((uMaxMem < uRemain)&&
			(RtlCompareMemory(pwStr, szwMaxMem, uMaxMem) == uMaxMem))	/* matched */
		{
			KdPrint(("Eram Match, MAXMEM\n"));
			pwStr += (uMaxMem / sizeof(WCHAR));
			/* Get n of MAXMEM=n */
			if (GetMaxMem(pDrvObj, pEramExt, pwStr, &uStart) == FALSE)
			{
				EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "GetMaxMem");
			}
			if (uNoLowMem == MAXDWORD)
			{
				break;
			}
		}
		if ((uNoLowMem < uRemain)&&
			(RtlCompareMemory(pwStr, szwNoLowMem, uNoLowMem) == uNoLowMem))	/* matched */
		{
			KdPrint(("Eram Match, NOLOWMEM\n"));
			bNoLowMem = TRUE;
			break;
		}
	}
	KdPrint(("Eram loop end\n"));
	if (bNoLowMem != FALSE)		/* /NOLOWMEM */
	{
		/* Try around 17MB */
		uStart = 17;
	}

	/* memory release */
	RtlFreeUnicodeString(&uniOptionUp);
	if (bStat == FALSE)
	{
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_MAXMEM_INVALID, NULL);
	}
	KdPrint(("Eram GetExternalStart end\n"));
	return bStat;
}


/* GetMaxMem
		MAXMEM=n Option Analysis.
	Parameters
		pDrvObj		The pointer to device representative object.
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		pwStr		The pointer to the string of n of MAXMEM=n.
		puSize		The pointer to the area to store n.
	Return Value
		Results.
*/

BOOLEAN GetMaxMem(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt,
	IN PWSTR			pwStr,
	OUT PULONG			puSize
 )
{
	/* local variables */
	UNICODE_STRING	uniOptionMem;
	PWSTR			pwStrSp;
	pwStrSp = pwStr;
	while ((*pwStrSp != L' ')&&(*pwStrSp != L'\0'))
	{
		pwStrSp++;
	}
	*pwStrSp = L'\0';
	RtlInitUnicodeString(&uniOptionMem, pwStr);
	/* Numerization */
	if (RtlUnicodeStringToInteger(&uniOptionMem, 0, puSize) != STATUS_SUCCESS)
	{
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_MAXMEM_ATOU, NULL);
		return FALSE;
	}
	return TRUE;
}


/* CheckMaxMem
		MAXMEM=n Option Analysis.
	Parameters
		pDrvObj		The pointer to device representative object.
		pEramExt	The pointer to an ERAM_EXTENTION structure.
		uSize		n of MAXMEM=n.
	Return Value
		Results.	TRUE: With OS-Unmanaged Memory.
*/

BOOLEAN CheckMaxMem(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt,
	IN ULONGPTR			uSize
 )
{
	if (uSize <= 16)		/* too small */
	{
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_MAXMEM_TOO_SMALL, NULL);
		return FALSE;
	}
	
	/* Fix into MB units */
	pEramExt->uExternalStart /= SIZE_MEGABYTE;
	
	if (pEramExt->uExternalStart >= uSize)	/* backward fix required */
	{
		/* backward fix */
		KdPrint(("Eram System %dMB\n", uSize));
		uSize = pEramExt->uExternalStart;
	}
	pEramExt->uExternalStart = uSize * SIZE_MEGABYTE;
	KdPrint(("Eram System %dMB, External start 0x%x\n", uSize, pEramExt->uExternalStart));
	uSize = pEramExt->uExternalStart / PAGE_SIZE_4K;
}


/* CheckExternalSize
		OS-Unmanaged Memory Starting Position Detection.
	Parameters
		pDrvObj		The pointer to device representative object.
		pEramExt	The pointer to an ERAM_EXTENTION structure.
	Return Value
		Results.		TRUE: With OS-Unmanaged Memory.
*/

BOOLEAN CheckExternalSize(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt
 )
{
	//TODO: understand this!
	/* local variables */
	CM_RESOURCE_LIST	ResList;	/* Resource list */
	ULONGPTR			uSize, uRealSize;
	BOOLEAN				bResConf, bStat;
	NTSTATUS			ntStat;
	DWORD				dwMaxAddr;
	ULARGE_INTEGER		ulMix;
	KdPrint(("Eram CheckExternalSize start\n"));
	/* Calculate the byte number to be reserved */
	uSize = pEramExt->uSizeTotal * PAGE_SIZE_4K;
	if (uSize == 0)		/* invalid */
	{
		KdPrint(("Eram Total is 0\n"));
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_DISK_SIZE_IS_0, NULL);
		return FALSE;
	}
	ulMix.QuadPart = (ULONGLONG)(pEramExt->uExternalStart) + (ULONGLONG)uSize;
	
	/* ACPI reserved memory guessing */
	dwMaxAddr = GetAcpiReservedMemory(pDrvObj);
	KdPrint(("Eram ACPI max 0x%x\n", dwMaxAddr));
	if (pEramExt->uExternalStart >= dwMaxAddr)	/* The starting position is after ACPI memory */
	{
		KdPrint(("Eram Invalid start address\n"));
		EramReportEvent(pDrvObj, ERAM_ERROR_MAXMEM_NO_MEMORY, NULL);
		return FALSE;
	}

	//TODO: already 64bit??
	if (ulMix.LowPart > dwMaxAddr)	/* It overlaps ACPI */
	{
		/* Restricted upto ACPI */
		uSize -= (ulMix.LowPart - dwMaxAddr);
		pEramExt->uSizeTotal = uSize / PAGE_SIZE_4K;
		ulMix.LowPart = dwMaxAddr;
		KdPrint(("Eram Wrap ACPI, limit size 0x%x (%dpages)\n", uSize, pEramExt->uSizeTotal));
	}
	/* Resource Request Settings */
	if (uSize >= (1ull << 32) )	{ KdPrint(("Eram too big size for ResourceInitTiny?\n")); }
	if (pEramExt->uExternalStart >= (1ull << 32)) { KdPrint(("Eram too big size for ResourceInitTiny?\n")); }
	ResourceInitTiny(pDrvObj, &ResList, pEramExt->uExternalStart, uSize);
	bResConf = FALSE;
	/* Check if resources are available */
	if (pEramExt->uOptflag.Bits.SkipReportUsage == 0)
	{
		ntStat = IoReportResourceUsage(NULL, pDrvObj, &ResList, sizeof(ResList), NULL, NULL, 0, TRUE, &bResConf);
		if (ntStat != STATUS_SUCCESS)	/* failed */
		{
			KdPrint(("Eram IoReportResourceUsage failed, %x\n", ntStat));
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_MAXMEM_REPORT_USAGE_FAILED, NULL);
			return FALSE;
		}
	}
	bStat = FALSE;
	if (bResConf != FALSE)	/* conflicted */
	{
		KdPrint(("Eram Conflict\n"));
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_MAXMEM_REPORT_USAGE_CONFLICT, NULL);
	}
	else
	{
		KdPrint(("Eram No conflict\n"));
		if (CheckExternalMemoryExist(pDrvObj, pEramExt->uExternalStart, uSize, &uRealSize, dwMaxAddr) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "CheckExternalMemoryExist");
		}
		else
		{
			KdPrint(("Eram extend memory 0x%08X - 0x%08X (0x%X bytes)\n", pEramExt->uExternalStart, (pEramExt->uExternalStart)+uRealSize-1, uRealSize));
			if (uSize > uRealSize)		/* out of memory */
			{
				/* Stay within the range of real memory */
				pEramExt->uSizeTotal = uRealSize / PAGE_SIZE_4K;
				KdPrint(("Eram size compress\n"));
				EramReportEvent(pEramExt->pDevObj, ERAM_WARN_MAXMEM_DISK_SIZE_FIXED, NULL);
			}
			bStat = TRUE;
		}
	}
	/* driver resource release */
	if (pEramExt->uOptflag.Bits.SkipReportUsage == 0)
	{
		RtlZeroBytes(&ResList, sizeof(ResList));
		IoReportResourceUsage(NULL, pDrvObj, &ResList, sizeof(ResList), NULL, NULL, 0, FALSE, &bResConf);
	}
	KdPrint(("Eram CheckExternalSize end\n"));
	return bStat;
}


/* ResourceInitTiny
		Resource Map Initialization.
	Parameters
		pDrvObj		The pointer to device representative object.
		pResList	The pointer to the resource structure.
		uStart		OS-Unmanaged Memory starting position.
		uSize		The size to handed over to the disk.
	Return Value
		No return value.
*/

VOID ResourceInitTiny(
	IN PDRIVER_OBJECT		pDrvObj,
	IN PCM_RESOURCE_LIST	pResList,
	IN ULONGPTR				uStart,
	IN ULONGPTR				uSize
 )
{
	/* local variables */
	PCM_PARTIAL_RESOURCE_DESCRIPTOR	pDesc;
	PHYSICAL_ADDRESS				PortAdr;
	KdPrint(("Eram ResourceInitTiny start\n"));
	//PortAdr.HighPart = 0;
	/* header settings */
	pResList->Count = 1;	/* Internal Only */
	pResList->List[0].InterfaceType = Internal;
	pResList->List[0].BusNumber = 0;
	pResList->List[0].PartialResourceList.Count = 1;	/* memory */
	/* Get the pointer to the top descriptor */
	pDesc = &(pResList->List[0].PartialResourceList.PartialDescriptors[0]);
	/* Settingss for inner memory request */
	pDesc->Type = CmResourceTypeMemory;
	pDesc->ShareDisposition = CmResourceShareDriverExclusive;
	pDesc->Flags = CM_RESOURCE_MEMORY_READ_WRITE;
	
	//TODO: what is tiny? its forcing 32bit here hum...
	PortAdr.QuadPart  = uStart;
	pDesc->u.Memory.Start = PortAdr;
	
	if (uSize >= (1ull << 32)) { KdPrint(("Eram too big size for ResourceInitTiny? (internal)\n")); }
	pDesc->u.Memory.Length = (DWORD)uSize; //TODO: this is always 32?
	KdPrint(("Eram ResourceInitTiny end\n"));
}


/* CheckExternalMemoryExist
		Memory Test Main Body.
	Parameters
		pDrvObj		The pointer to device representative object.
		uStart		OS-Unmanaged Memory starting position.
		uDiskSize	The size to handed over to the disk.
		puSize		The memory capacity.
		dwMaxAddr	The lowest address of ACPI usage memory.
	Return Value
		Results.
*/

BOOLEAN CheckExternalMemoryExist(
	IN PDRIVER_OBJECT	pDrvObj,
	IN ULONGPTR			uStart,
	IN ULONGPTR			uDiskSize, //TODO: all those as 64bit are correct?
	OUT PULONGPTR		puSize,
	IN ULONGPTR			dwMaxAddr
 )
{
	/* local variables */
	ULONGPTR			loopi, loopj;
	BOOLEAN				bExist;
	PHYSICAL_ADDRESS	MapAdr;
	volatile PBYTE		pBase;
	static BYTE			byTests[] = { 0, 0x55, 0xaa, 0 };
	KdPrint(("Eram CheckExternalMemoryExist start\n"));
	/* resource usage settings */
	if (ResourceSetupTiny(pDrvObj, uStart, &MapAdr) == FALSE)
	{
		KdPrint(("Eram ResourceSetupTiny failed\n"));
		EramReportEvent(pDrvObj, ERAM_ERROR_FUNCTIONERROR, "ResourceSetupTiny");
		return FALSE;
	}
	KdPrint(("Eram Memory check start\n"));
	*puSize = 0;
	for (loopi=0; loopi<uDiskSize; loopi+=SIZE_MEGABYTE)
	{
		/* map */
		pBase = (PBYTE)MmMapIoSpace(MapAdr, PAGE_SIZE_4K, FALSE);
		if (pBase == NULL)	/* Not map or failure */
		{
			EramReportEvent(pDrvObj, ERAM_ERROR_MAXMEM_MAP_FAILED, NULL);
			return FALSE;
		}
		/* RAM Existence Test */
		bExist = TRUE;
		for (loopj=0; loopj<sizeof(byTests); loopj++)
		{
			*pBase = byTests[loopj];
			if (*pBase != byTests[loopj])		/* different in value ... RAM doesn't exist */
			{
				bExist = FALSE;
				break;
			}
		}
		/* Unmap */
		MmUnmapIoSpace(pBase, PAGE_SIZE_4K);
		if (bExist == FALSE)	/* absent */
		{
			break;
		}
		if ((uStart + loopi + SIZE_MEGABYTE) >= dwMaxAddr)	/* No 1MB space */
		{
			KdPrint(("Eram ACPI memory detected, adjusted\n"));
			*puSize += (dwMaxAddr - (uStart + loopi));
			break;
		}
		/* Next 1MB */
		*puSize += SIZE_MEGABYTE;
		MapAdr.QuadPart += SIZE_MEGABYTE;
	}
	if (*puSize == 0)		/* Not detected */
	{
		KdPrint(("Eram extend memory 0 bytes\n"));
		EramReportEvent(pDrvObj, ERAM_ERROR_MAXMEM_NO_MEMORY, NULL);
		return FALSE;
	}
	KdPrint(("Eram CheckExternalMemoryExist end, %dKB(=%dMB) detected\n", (*puSize) / SIZE_KILOBYTE, (*puSize) / SIZE_MEGABYTE));
	return TRUE;
}


/* ResourceSetupTiny
		I/O Resource Initial Setting
	Parameters
		pDrvObj		The pointer to device representative object.
		uStart		OS-Unmanaged Memory starting position.
		pMapAdr		The pointer to the conversion address.
	Return Value
		Results.
*/

BOOLEAN ResourceSetupTiny(
	IN PDRIVER_OBJECT		pDrvObj,
	IN ULONGPTR				uStart,
	OUT PPHYSICAL_ADDRESS	pMapAdr
 )
{
	/* local variables */
	PHYSICAL_ADDRESS	PortAdr;
	UINT				MemType;
	KdPrint(("Eram ResourceSetupTiny start\n"));	
	//PortAdr.HighPart = 0;
	/* Internal memory settings */
	MemType = 0;			/* Memory space */
	
	// TODO: more tiny this wont ever reach >32bit?
	PortAdr.QuadPart  = uStart;
	/* It has translated at least, but, the results were likely same */
	if (HalTranslateBusAddress(Internal, 0, PortAdr, &MemType, pMapAdr) == FALSE)
	{
		KdPrint(("Eram Memory 0x%x, HalTranslateBusAddress failed\n", PortAdr.LowPart));
		EramReportEvent(pDrvObj, ERAM_ERROR_TRANSLATE_ADDRESS_FAILED, NULL);
		return FALSE;
	}
	if (MemType != 0)		/* map */
	{
		KdPrint(("!Eram Map type\n"));
		EramReportEvent(pDrvObj, ERAM_ERROR_PORT_MAPPED, NULL);
		return FALSE;
	}
	KdPrint(("Eram ResourceSetupTiny end\n"));
	return TRUE;
}


/* ExtReport
		OS-Unmanaged Memory Starting Position Detection.
	Parameters
		pDrvObj		The pointer to device representative object.
		pEramExt	The pointer to an ERAM_EXTENTION structure.
	Return Value
		Results.
*/

BOOLEAN ExtReport(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt
 )
{
	/* local variables */
	CM_RESOURCE_LIST	ResList;	/* Resource list */
	ULONGPTR			uSize;
	BOOLEAN				bResConf;
	NTSTATUS			ntStat;
	KdPrint(("Eram ExtReport start\n"));
	/* Calculate the byte number to be reserved */
	uSize = pEramExt->uSizeTotal * PAGE_SIZE_4K;
	if (uSize == 0)		/* invalid */
	{
		KdPrint(("Eram Total is 0\n"));
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_DISK_SIZE_IS_0, NULL);
		return FALSE;
	}
	/* Resource Request Settings */
	ResourceInitTiny(pDrvObj, &ResList, pEramExt->uExternalStart, uSize);
	/* Notify resource usage */
	if (pEramExt->uOptflag.Bits.SkipReportUsage == 0)
	{
		ntStat = IoReportResourceUsage(NULL, pDrvObj, &ResList, sizeof(ResList), NULL, NULL, 0, FALSE, &bResConf);
		if ((ntStat == STATUS_SUCCESS)&&
			(bResConf != FALSE))	/* conflict happens */
		{
			KdPrint(("Eram Conflict\n"));
			/* Set error */
			ntStat = STATUS_DEVICE_CONFIGURATION_ERROR;
		}
		if (ntStat != STATUS_SUCCESS)	/* unusable */
		{
			KdPrint(("Eram IoReportResourceUsage failed, %x\n", ntStat));
			EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_MAXMEM_REPORT_USAGE_FAILED, NULL);
			return FALSE;
		}
	}
	/* resource usage settings */
	if (ResourceSetupTiny(pDrvObj, pEramExt->uExternalStart, &(pEramExt->MapAdr)) == FALSE)
	{
		KdPrint(("Eram ResourceSetupTiny failed\n"));
		EramReportEvent(pEramExt->pDevObj, ERAM_ERROR_FUNCTIONERROR, "ResourceSetupTiny");
		/* driver resource release */
		if (pEramExt->uOptflag.Bits.SkipReportUsage == 0)
		{
			RtlZeroBytes(&ResList, sizeof(ResList));
			IoReportResourceUsage(NULL, pDrvObj, &ResList, sizeof(ResList), NULL, NULL, 0, FALSE, &bResConf);
		}
		return FALSE;
	}
	KdPrint(("Eram ExtReport end\n"));
	return TRUE;
}


/* GetAcpiReservedMemory
		ACPI Reserved Memory Test.
	Parameters
		pDrvObj			The pointer to device representative object.
	Return Value
		The lowest address of ACPI usage memory.
*/

DWORD GetAcpiReservedMemory(
	IN PDRIVER_OBJECT	pDrvObj
 )
{
	/* local variables */
	ULONG				loopi;
	PHYSICAL_ADDRESS	MapAdr;
	PBYTE				pBase;
	PDWORD				pdwBios;
	DWORD				dwMaxAdr;
	KdPrint(("Eram GetAcpiReservedMemory start\n"));
	dwMaxAdr = 0xffff0000;
	/* resource usage settings */
	if (ResourceSetupTiny(pDrvObj, BIOS_ADDRESS_START, &MapAdr) == FALSE)
	{
		KdPrint(("Eram 3ResourceSetupTiny failed\n"));
		EramReportEvent(pDrvObj, ERAM_ERROR_FUNCTIONERROR, "ResourceSetupTiny");
		return dwMaxAdr;
	}
	/* map (allow cache) */
	pBase = (PBYTE)MmMapIoSpace(MapAdr, BIOS_SIZE, TRUE);
	if (pBase == NULL)	/* Not map or mapping failure */
	{
		EramReportEvent(pDrvObj, ERAM_ERROR_MAXMEM_MAP_FAILED, NULL);
		return dwMaxAdr;
	}
	pdwBios = (PDWORD)pBase;
	for (loopi=0; loopi<BIOS_SIZE; loopi+=16)
	{
		if ((pdwBios[0] == 0x20445352)&&	/* 'RSD PTR ' */
			(pdwBios[1] == 0x20525450))
		{
			KdPrint(("Eram RSDT found, 0x%X\n", pdwBios[4]));
			if (pdwBios[4] != 0)	/* 32bit address valid */
			{
				/* Check ahead of RSDT */
				dwMaxAdr = CheckAcpiRsdt(pDrvObj, dwMaxAdr, pdwBios[4]) & 0xffff0000;
			}
			break;
		}
		pdwBios = &(pdwBios[4]);
	}
	/* Unmap */
	MmUnmapIoSpace(pBase, BIOS_SIZE);
	return dwMaxAdr;
}


/* CheckAcpiRsdt
		RSDT Test.
	Parameters
		pDrvObj			The pointer to The pointer to device representative object.
		dwMinValue		The lowest address of ACPI usage memory.
		dwRsdt			The address of RSDT.
	Return Value
		The lowest address of ACPI usage memory.
*/

DWORD CheckAcpiRsdt(
	IN PDRIVER_OBJECT	pDrvObj,
	IN DWORD			dwMinValue,
	IN DWORD			dwRsdt
 )
{
	/* local variables */
	static ULONG		uRsdtSize = PAGE_SIZE_4K * 2;
	ULONG				loopi, uCnt;
	PHYSICAL_ADDRESS	MapAdr;
	PBYTE				pBase;
	PDWORD				pdwRsdt;
	DWORD				dwRsdtBase, dwRsdtOfs;
	KdPrint(("Eram CheckAcpiRsdt start\n"));
	dwRsdtBase = dwRsdt & ~(PAGE_SIZE_4K - 1);
	dwRsdtOfs = dwRsdt - dwRsdtBase;
	dwMinValue = dwRsdt;
	/* resource usage settings */
	if (ResourceSetupTiny(pDrvObj, dwRsdtBase, &MapAdr) == FALSE)
	{
		KdPrint(("Eram ResourceSetupTiny failed\n"));
		EramReportEvent(pDrvObj, ERAM_ERROR_FUNCTIONERROR, "ResourceSetupTiny");
		return dwMinValue;
	}
	/* Map (allow cache) */
	pBase = (PBYTE)MmMapIoSpace(MapAdr, uRsdtSize, TRUE);
	if (pBase == NULL)	/* Not map or failure */
	{
		EramReportEvent(pDrvObj, ERAM_ERROR_MAXMEM_MAP_FAILED, NULL);
		return dwMinValue;
	}
	pdwRsdt = (PDWORD)((PBYTE)pBase + dwRsdtOfs);
	if ((pdwRsdt[0] != 0x54445352)||	/* 'RSDT' */
		(pdwRsdt[1] < 0x24))
	{
		KdPrint(("!Eram RSDT\n"));
		return dwMinValue;
	}
	KdPrint(("Eram RSDT found, 0x%X\n", dwRsdt));
	uCnt = ((pdwRsdt[1] - 0x24) / sizeof(DWORD));
	pdwRsdt = &(pdwRsdt[0x24 / sizeof(DWORD)]);
	/* repeat for array */
	for (loopi=0; loopi<uCnt; loopi++, pdwRsdt++)
	{
		dwMinValue = CheckRsdtElements(pDrvObj, dwMinValue, *pdwRsdt);
	}
	/* Unmap */
	MmUnmapIoSpace(pBase, uRsdtSize);
	KdPrint(("Eram CheckAcpiRsdt end, 0x%X\n", dwMinValue));
	return dwMinValue;
}


/* CheckRsdtElements
		RSDT element tests
	Parameters
		pDrvObj			The pointer to The pointer to device representative object.
		dwMinValue		The lowest address of ACPI usage memory
		dwRsdtElement	The address of RSDT array elements
	Return Value
		The lowest address of ACPI usage memory
*/

DWORD CheckRsdtElements(
	IN PDRIVER_OBJECT	pDrvObj,
	IN DWORD			dwMinValue,
	IN DWORD			dwRsdtElement
 )
{
	/* local variables */
	static ULONG		uRsdtSize = PAGE_SIZE_4K * 2;
	PHYSICAL_ADDRESS	MapAdr;
	PBYTE				pBase;
	PDWORD				pdwRsdt;
	DWORD				dwRsdtBase, dwRsdtOfs;
	KdPrint(("Eram CheckRsdtElements start, min=0x%X, elem=0x%X\n", dwMinValue, dwRsdtElement));
	if (dwRsdtElement == 0)
	{
		return dwMinValue;
	}
	dwRsdtBase = dwRsdtElement & ~(PAGE_SIZE_4K - 1);
	dwRsdtOfs = dwRsdtElement - dwRsdtBase;
	dwMinValue = (dwMinValue > dwRsdtElement) ? dwRsdtElement : dwMinValue;
	/* resource usage settings */
	if (ResourceSetupTiny(pDrvObj, dwRsdtBase, &MapAdr) == FALSE)
	{
		KdPrint(("Eram ResourceSetupTiny failed\n"));
		EramReportEvent(pDrvObj, ERAM_ERROR_FUNCTIONERROR, "ResourceSetupTiny");
		return dwMinValue;
	}
	/* Map (allow cache) */
	pBase = (PBYTE)MmMapIoSpace(MapAdr, uRsdtSize, TRUE);
	if (pBase == NULL)	/* not map or failure */
	{
		EramReportEvent(pDrvObj, ERAM_ERROR_MAXMEM_MAP_FAILED, NULL);
		return dwMinValue;
	}
	pdwRsdt = (PDWORD)((PBYTE)pBase + dwRsdtOfs); //(DWORD)
	KdPrint(("Eram Element 0x%X\n", *pdwRsdt));
	if ((pdwRsdt[0] == 0x50434146)&&	/* 'FACP' */
		(pdwRsdt[1] >= 0x74))
	{
		/* FADT found */
		if (pdwRsdt[9] != 0)
		{
			dwMinValue = (dwMinValue > pdwRsdt[9]) ? pdwRsdt[9] : dwMinValue;
		}
		if (pdwRsdt[10] != 0)
		{
			dwMinValue = (dwMinValue > pdwRsdt[10]) ? pdwRsdt[10] : dwMinValue;
		}
		KdPrint(("Eram FADT found, FACS=0x%X, DSDT=0x%X\n", pdwRsdt[9], pdwRsdt[10]));
	}
	/* Unmap */
	MmUnmapIoSpace(pBase, uRsdtSize);
	KdPrint(("Eram CheckRsdtElements end, min 0x%X\n", dwMinValue));
	return dwMinValue;
}
