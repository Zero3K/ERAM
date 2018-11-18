/* ERAM.H    RAM disk ERAM for WindowsNT/2000/XP
      Copyright (c) 1999-2004 by *Error15
   Translated into English by Katayama Hirofumi MZ.
*/

#pragma pack(1)

int sprintf(char *s, const char *format, ...);

typedef unsigned int	UINT;
typedef UCHAR			BYTE, *PBYTE, *LPBYTE;
typedef USHORT			WORD, *PWORD, *LPWORD;
typedef ULONG			DWORD, *PDWORD, *LPDWORD;
typedef	int				INT, *PINT, *LPINT;
#define	NumberOf(x)		((sizeof(x))/(sizeof((x)[0])))
#define LOBYTE(w)		((BYTE)(w))
#define HIBYTE(w)		((BYTE)(((WORD)(w) >> 8) & 0xFF))
#ifndef max
#define max(a,b)		(((a) > (b)) ? (a) : (b))
#endif

#ifndef	PARTITION_FAT32
#define	PARTITION_FAT32		0x0B
#endif
#define	MAXDWORD		(MAXULONG)

#ifndef	IOCTL_DISK_GET_LENGTH_INFO
#define	IOCTL_DISK_GET_LENGTH_INFO	CTL_CODE(IOCTL_DISK_BASE, 0x0017, METHOD_BUFFERED, FILE_READ_ACCESS)
typedef struct {
	LARGE_INTEGER	Length;
 } GET_LENGTH_INFORMATION, *PGET_LENGTH_INFORMATION;
#endif

typedef NTSTATUS (*ERAM_READ)(PVOID, PIRP, PIO_STACK_LOCATION, PUCHAR);
typedef NTSTATUS (*ERAM_WRITE)(PVOID, PIRP, PIO_STACK_LOCATION, PUCHAR);
typedef BOOLEAN (*ERAM_NEXT)(PVOID, LPDWORD, LPDWORD);
typedef VOID (*ERAM_UNMAP)(PVOID);


#define FATID_MSG	"RAMdisk driver ERAM version 2.23 Copyright(C)1999-2004 by *Error15"
#define SUBKEY_WSTRING	L"\\Parameters"
#define NT_DEVNAME		L"\\Device\\Eram"
#define WIN32_PATH		L"\\DosDevices\\"
#define DEFAULT_DRV		L"Z:"
#define	DISKMAXCLUSTER_12 (4086)			/* FAT12 Max Cluster */
#define	DISKMAXCLUSTER_16 (65525)			/* FAT16 Max Cluster */
#define	DISKMAXCLUSTER_32 (268435455)		/* FAT32 Max Cluster */
#define RAMDISK_MEDIA_TYPE		0xf8
#define VOLUME_LABEL_RAMDISK	"MS-RAMDRIVE"
#define VOLUME_LABEL_LOCALDISK	"ERAM-DRIVE "
#define TEMPDIR_NAME			"TEMP       "
#define OWNDIR_NAME				".          "
#define PARENTDIR_NAME			"..         "
#define	ERAMEXTFILEPATH		L"C:\\ERAMSWAP.$$$"

#define	DISKMINPAGE	(16)
/* Unable to allocate data area; 2 is minimum at allocation 2 */
#define	SECTOR	(512)
#define	SECTOR_LOG2	(9)
#define	RESV_SECTOR_FAT32	(2)

#define SIZE_KILOBYTE	(1024)			/* 1KB */
#define SIZE_MEGABYTE	(SIZE_KILOBYTE * 1024)	/* 1MB */
#define PAGE_SIZE_4K	(SIZE_KILOBYTE * 4)	/* The size of one page */
#define	PAGE_SIZE_LOG2	(12)
#define	PAGE_SECTOR		(PAGE_SIZE_4K / SECTOR)	/* Sectors per page */
#define	PAGE_SEC_LOG2	(PAGE_SIZE_LOG2 - SECTOR_LOG2)
#define EXT_PAGE_SIZE	(SIZE_KILOBYTE * 64)		/* 64KB unit switch */
#define	EXT_PAGE_SIZE_LOG2	(16)
#define	EXT_PAGE_SECTOR	(EXT_PAGE_SIZE / SECTOR)	/* sectors per bank */
#define	EXT_PAGE_SEC_LOG2	(EXT_PAGE_SIZE_LOG2 - SECTOR_LOG2)
#define	LIMIT_2GBPAGES	(0x7ffff)		/* 2GB pages */
#define	LIMIT_4GBPAGES	(0xfffff)		/* 4GB pages */
#define	BIOS_ADDRESS_START	((DWORD)(0xe0000))	/* E0000..FFFFF */
#define	BIOS_ADDRESS_END	((DWORD)(0xfffff))	/* E0000..FFFFF */
#define	BIOS_SIZE			((DWORD)(BIOS_ADDRESS_END - BIOS_ADDRESS_START + 1))

/* BPB array (FAT12,16) */
typedef	struct {
	WORD	wNumSectorByte;		// The number of sector bytes (BPB, =SECTOR)
	BYTE	byAllocUnit;		// Allocation unit (alloc)
	WORD	wNumResvSector;		// The number of reserved sectors, FAT12/16=1, FAT32=32
	BYTE	byNumFat;			// The number of FATs (=1)
	WORD	wNumDirectory;		// The number of root directory entries (dir, =128)
	WORD	wNumAllSector;		// The number of all sectors (under 32MB)
	BYTE	byMediaId;			// Media ID(media, =f8)
	WORD	wNumFatSector;		// The number of FAT sectors (FAT)
 } bpb_struc1;

typedef	struct {
	WORD	bsSecPerTrack;		// Sectors per bank (=PAGE_SECTOR)
	WORD	bsHeads;			// The number of heads (=1)
	ULONG	bsHiddenSecs;		// The number of hidden sectors (=0)
	ULONG	bsHugeSectors;		// The sectors over 32MB
 } bpb_struc2;

typedef	struct {
	BYTE		bsDriveNumber;		// Drive number
	BYTE		bsReserved1;		// Reserved
	BYTE		bsBootSignature;	// Signature
	ULONG		bsVolumeID; 		// Volume ID
	CHAR		bsLabel[11];		// Label
	CHAR		bsFileSystemType[8];// File system type
 } bpb_struc3;

/* BPB array (FAT32) */
typedef	struct {
	DWORD	dwNumFatSector32;	// The number of FAT sectors (FAT32)
	WORD	wExtFlags;			// Extended flags
	WORD	wFileSystemVersion;	// Filesystem version
	DWORD	dwRootCluster;		// Root cluster number (=2)
	WORD	wFsInfoSector;		// FSINFO sector number (=1)
	WORD	wBackupBootSector;	// Backup boot sector number (=6)
	BYTE	byResv[12];			// Reserved
 } bpb_struc4;

typedef union {
	DWORD	dwOptflag;					// ERAM control
	struct {
		BYTE	NonPaged:1;				// bit 0:NonPagedPool usage
		BYTE	External:1;				// bit 1:External memory usage
		BYTE	SkipExternalCheck:1;	// bit 2:Don't test memory when using external memory
		BYTE	Swapable:1;				// bit 3:Local disk flag
		BYTE	EnableFat32:1;			// bit 4:Allow FAT32 usage
		BYTE	SkipReportUsage:1;		// bit 5:Don't report when using external memory = 2000:Allow stand-by
		BYTE	MakeTempDir:1;			// bit 6:TEMP directory creation
		BYTE	byResv7:1;				// bit 7:
		BYTE	byResv8:8;				// bit 8:
		BYTE	byResv16:8;				// bit16:
		BYTE	byResv24:7;				// bit24:
		BYTE	UseExtFile:1;			// bit31:External file usage
	} Bits;
 } ERAM_OPTFLAG, *PERAM_OPTFLAG;

/* ERAM disk device specific info */
typedef struct {
	PDEVICE_OBJECT		pDevObj;
	ERAM_READ			EramRead;
	ERAM_WRITE			EramWrite;
	ERAM_NEXT			EramNext;
	ERAM_UNMAP			EramUnmap;
	ULONG				uNowMapAdr;		// OS-Unmanaged/file, the currently mapping address
	LPBYTE				pExtPage;		// OS-Unmanaged/file, the memory mapping address
	LPBYTE				pPageBase;		// OS-Managed Memory
	ULONG				uSizeTotal;		// Total size (4KB unit)
	UNICODE_STRING		Win32Name;
	ULONG				uAllSector;		// The number of all sectors
	ULONG				uExternalStart;	// External memory starting position
	ULONG				uExternalEnd;	// External memory ending position (of detected)
	FAST_MUTEX			FastMutex;		// Fast mutex
	PHYSICAL_ADDRESS	MapAdr;			// OS-Unmanaged Memory map position top
	ULONG				bsHiddenSecs;	// The number of hidden sectors (=0)
	HANDLE				hFile;			// External file handle (in system process)
	HANDLE				hSection;		// External file mapping handle (in system process)
	LIST_ENTRY			IrpList;		// Irp list top
	KSPIN_LOCK			IrpSpin;		// Irp list spinlock
	KSEMAPHORE			IrpSem;			// Irp list semaphore
	PVOID				pThreadObject;	// Irp list thred object
	ERAM_OPTFLAG		uOptflag;		// Option
	BYTE				FAT_size;		// PARTITION_... FAT_12,FAT_16,HUGE,FAT32
	BYTE				bThreadStop;	// Thread Stop Request
 } ERAM_EXTENSION, *PERAM_EXTENSION;

/* MBR-inside partitoin table (PC/AT) */
typedef struct {
	BYTE	byBootInd;
	BYTE	byFirstHead;
	BYTE	byFirstSector;
	BYTE	byFirstTrack;
	BYTE	byFileSystem;
	BYTE	byLastHead;
	BYTE	byLastSector;
	BYTE	byLastTrack;
	DWORD	dwStartSector;
	DWORD	dwNumSectors;
 } MBR_PARTITION, *PMBR_PARTITION;

/* Info for boot sector */
typedef struct {
	bpb_struc1	BPB;						/* BPB array */
	bpb_struc2	BPB_ext;					/* DOS5 extended area */
	bpb_struc3	BPB_ext2;					/* Extended area */
	bpb_struc4	BPB_fat32;					/* FAT32 only */
	TIME_FIELDS TimeInfo;					/* date and time */
	CHAR		bsLabel[11];				/* label */
	WCHAR		wszExtFile[4];				/* \\??\\ */
	WCHAR		wszExtFileMain[260];		/* C:\\ERAMSWAP.$$$ */
 } FAT_ID, *PFAT_ID;

typedef struct {
	BYTE		bsJump[3];			// jmp $(=eb, fe, 90)
	BYTE		bsOemName[8];		// Device name (='ERAM    ')
	bpb_struc1	BPB;				// BPB array
	bpb_struc2	BPB_ext;			// DOS5 extended area
	bpb_struc3	BPB_ext2;			// Extended area
	BYTE		byResv1[194];
	BYTE		szMsg[128];			// 0x100
	BYTE		byResv2[126];		// 0x180
	BYTE		bsSig2[2];			/* 55,AA */
 } BOOTSECTOR_FAT16, *PBOOTSECTOR_FAT16;

typedef struct {
	BYTE			bsJump[3];		// jmp $(=eb, fe, 90)
	BYTE			bsOemName[8];	// Device name (='ERAM    ')
	bpb_struc1		BPB;			// BPB array
	bpb_struc2		BPB_ext;		// DOS5 extended area
	bpb_struc4		BPB_fat32;		// FAT32 only
	bpb_struc3		BPB_ext2;		// Extended area
	BYTE			byResv1[166];
	BYTE			szMsg[128];		// 0x100
	BYTE			byResv[62];		// 0x180
	MBR_PARTITION	Parts[4];		// 0x1BE, 1CE, 1DE, 1EE
	BYTE			bsSig2[2];		/* 55,AA */
 } BOOTSECTOR_FAT32, *PBOOTSECTOR_FAT32;

/* FSINFO (FAT32) */
typedef struct {
	DWORD	bfFSInf_Sig;				/*	"rrAa" */
	DWORD	bfFSInf_free_clus_cnt;		/*	Free clusters -1:Unknown */
	DWORD	bfFSInf_next_free_clus;		/*	Cluster reserved last time */
	DWORD	bfFSInf_resvd[3];
 } BIGFATBOOTFSINFO, *PBIGFATBOOTFSINFO;

typedef struct {
	DWORD				FSInfo_Sig;		/*	"RRaA" */
	BYTE				byResv[480];
	BIGFATBOOTFSINFO	FsInfo;			/* 1E4.. */
	BYTE				byResv2[2];
	BYTE				bsSig2[2];		/* 55,AA */
 } FSINFO_SECTOR, *PFSINFO_SECTOR;

/* Directory entry */
typedef struct {
	CHAR	sName[11];		/* Filename */
	union {
		BYTE	byAttr;		/* File attributes */
		struct {
			BYTE	byR:1;
			BYTE	byH:1;
			BYTE	byS:1;
			BYTE	byVol:1;
			BYTE	byDir:1;
			BYTE	byA:1;
		} Bits;
	} uAttr;
	BYTE	byResv1[10];
	WORD	wUpdMinute;		/* Update Time */
	WORD	wUpdDate;		/* Update Date */
	WORD	wCluster;		/* First cluster */
	DWORD	dwFileSize;		/* The file size */
 } DIR_ENTRY, *PDIR_ENTRY;

/* Root Directory Entry */
typedef struct {
	DIR_ENTRY	vol;
	DIR_ENTRY	temp;
 } vol_label;

/* Subdirectory Entry */
typedef struct {
	DIR_ENTRY	own;
	DIR_ENTRY	parent;
 } dir_init;


/* Prototypes */

//------  Oldie hidden API

extern __declspec(dllimport) ULONG NtBuildNumber;

#define	BUILD_NUMBER_NT40	(1381)
#define	BUILD_NUMBER_NT50	(2195)
#define	BUILD_NUMBER_NT51	(2600)

NTSTATUS
NTAPI
ZwCreateSection(
	OUT PHANDLE				SectionHandle,
	IN ACCESS_MASK			DesiredAccess,
	IN POBJECT_ATTRIBUTES	ObjectAttributes OPTIONAL,
	IN PLARGE_INTEGER		MaximumSize OPTIONAL,
	IN ULONG				SectionPageProtection,
	IN ULONG				AllocationAttributes,
	IN HANDLE				FileHandle OPTIONAL
 );
#ifndef	SEC_COMMIT
#define	SEC_COMMIT	(0x8000000)
#endif

//------  Functions to be used at all times in normal use

NTSTATUS EramCreateClose(
	IN PDEVICE_OBJECT	pDevObj,
	IN PIRP 			pIrp
 );

NTSTATUS EramDeviceControl(
	IN PDEVICE_OBJECT	pDevObj,
	IN PIRP 			pIrp
 );

VOID EramDeviceControlGeometry(
	PERAM_EXTENSION	pEramExt,
	IN PIRP			pIrp,
	IN ULONG		uLen
 );

VOID EramDeviceControlGetPartInfo(
	PERAM_EXTENSION	pEramExt,
	IN PIRP			pIrp,
	IN ULONG		uLen
 );

VOID EramDeviceControlSetPartInfo(
	PERAM_EXTENSION	pEramExt,
	IN PIRP			pIrp,
	IN ULONG		uLen
 );

VOID EramDeviceControlVerify(
	PERAM_EXTENSION	pEramExt,
	IN PIRP			pIrp,
	IN ULONG		uLen
 );

VOID EramDeviceControlDiskCheckVerify(
	PERAM_EXTENSION	pEramExt,
	IN PIRP			pIrp,
	IN ULONG		uLen
 );

VOID EramDeviceControlGetLengthInfo(
	PERAM_EXTENSION	pEramExt,
	IN PIRP			pIrp,
	IN ULONG		uLen
 );

NTSTATUS EramReadWrite(
	IN PDEVICE_OBJECT	pDevObj,
	IN PIRP 			pIrp
 );

VOID EramUnloadDriver(
	IN PDRIVER_OBJECT	pDrvObj
 );

VOID EramUnloadDevice(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PDEVICE_OBJECT	pDevObj,
	IN PERAM_EXTENSION	pEramExt
 );

VOID ResourceRelease(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt
 );

VOID ReleaseMemResource(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt
 );

BOOLEAN EramReportEvent(
	IN PVOID	pIoObject,
	IN NTSTATUS	ntErrorCode,
	IN PSTR		pszString
 );

BOOLEAN EramReportEventW(
	IN PVOID	pIoObject,
	IN NTSTATUS	ntErrorCode,
	IN PWSTR	pwStr
 );

NTSTATUS ReadPool(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				lpDest
 );

NTSTATUS WritePool(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				lpSrc
 );

NTSTATUS ExtRead1(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				lpDest
 );

NTSTATUS ExtWrite1(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				lpSrc
 );

BOOLEAN ExtNext1(
	IN PERAM_EXTENSION	pEramExt,
	IN OUT LPDWORD		lpeax,
	IN OUT LPDWORD		lpebx
 );

BOOLEAN ExtMap(
	IN PERAM_EXTENSION	pEramExt,
	IN ULONG			uMapAdr
 );

VOID ExtUnmap(
	IN PERAM_EXTENSION	pEramExt
 );

NTSTATUS ExtFilePendingRw(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				pTransAddr
 );

VOID EramRwThread(
	IN PVOID			pContext
 );

NTSTATUS EramRwThreadIrp(
	PERAM_EXTENSION		pEramExt,
	PLIST_ENTRY			pIrpList
 );

NTSTATUS ExtFileRead1(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				lpDest
 );

NTSTATUS ExtFileWrite1(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				lpSrc
 );

BOOLEAN ExtFileNext1(
	IN PERAM_EXTENSION	pEramExt,
	IN OUT LPDWORD		lpeax,
	IN OUT LPDWORD		lpebx
 );

BOOLEAN ExtFileMap(
	IN PERAM_EXTENSION	pEramExt,
	IN ULONG			uMapAdr
 );

VOID ExtFileUnmap(
	IN PERAM_EXTENSION	pEramExt
 );

NTSTATUS EramShutdown(
	IN PDEVICE_OBJECT	pDevObj,
	IN PIRP				pIrp
 );

/* Definitions for paging */
#ifdef ALLOC_PRAGMA
//------  Functions to be used at all times in normal use
#pragma	alloc_text(PAGE, EramCreateClose)
#pragma	alloc_text(PAGE, EramDeviceControl)
#pragma	alloc_text(PAGE, EramDeviceControlGeometry)
#pragma	alloc_text(PAGE, EramDeviceControlGetPartInfo)
#pragma	alloc_text(PAGE, EramDeviceControlSetPartInfo)
#pragma	alloc_text(PAGE, EramDeviceControlVerify)
#pragma	alloc_text(PAGE, EramDeviceControlGetLengthInfo)
#pragma	alloc_text(PAGE, EramDeviceControlDiskCheckVerify)
#pragma	alloc_text(PAGE, EramDeviceControlGetLengthInfo)
#pragma	alloc_text(PAGE, EramReadWrite)
#pragma	alloc_text(PAGE, EramUnloadDriver)
#pragma	alloc_text(PAGE, EramUnloadDevice)
#pragma	alloc_text(PAGE, ResourceRelease)
#pragma	alloc_text(PAGE, ReleaseMemResource)
#pragma	alloc_text(PAGE, EramReportEvent)
#pragma	alloc_text(PAGE, EramReportEventW)
#pragma	alloc_text(PAGE, ReadPool)
#pragma	alloc_text(PAGE, WritePool)
#pragma	alloc_text(PAGE, ExtRead1)
#pragma	alloc_text(PAGE, ExtWrite1)
#pragma	alloc_text(PAGE, ExtNext1)
#pragma	alloc_text(PAGE, ExtMap)
#pragma	alloc_text(PAGE, ExtUnmap)
#pragma	alloc_text(PAGE, ExtFilePendingRw)
#pragma	alloc_text(PAGE, EramRwThread)
#pragma	alloc_text(PAGE, EramRwThreadIrp)
#pragma	alloc_text(PAGE, ExtFileRead1)
#pragma	alloc_text(PAGE, ExtFileWrite1)
#pragma	alloc_text(PAGE, ExtFileNext1)
#pragma	alloc_text(PAGE, ExtFileMap)
#pragma	alloc_text(PAGE, ExtFileUnmap)
#pragma	alloc_text(PAGE, EramShutdown)
#endif	// ALLOC_PRAGMA

//------  Below are the functions used at initialization

NTSTATUS DriverEntry(
	IN OUT PDRIVER_OBJECT	pDrvObj,
	IN PUNICODE_STRING		pRegPath
 );

VOID InitFatId(
	IN PFAT_ID	pFatId
 );

NTSTATUS EramInitDisk(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PFAT_ID			pFatId,
	IN PUNICODE_STRING	pRegParam
 );

NTSTATUS MemSetup(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId,
	IN ULONG			uMemSize
 );

BOOLEAN OsAlloc(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt,
	IN ULONG			uMemSize
 );

VOID CalcAvailSize(
	IN PDRIVER_OBJECT	pDrvObj,
	IN POOL_TYPE		fPool,
	IN ULONG			uMemSize
 );

DEVICE_TYPE CheckSwapable(
	IN PUNICODE_STRING		pRegParam
 );

VOID CheckDeviceName(
	IN PUNICODE_STRING		pRegParam,
	IN OUT PUNICODE_STRING	pNtDevName
 );

VOID CheckSwitch(
	IN PERAM_EXTENSION		pEramExt,
	IN PFAT_ID				pFatId,
	IN PUNICODE_STRING		pRegParam,
	IN OUT PUNICODE_STRING	pDrvStr
 );

VOID GetMaxAddress(
	IN PERAM_EXTENSION		pEramExt,
	IN PUNICODE_STRING		pRegParam
 );

VOID PrepareVolumeLabel(
	IN PERAM_EXTENSION		pEramExt,
	IN PFAT_ID				pFatId,
	IN PUNICODE_STRING		pRegParam
 );

BOOLEAN CheckVolumeLabel(
	IN PERAM_EXTENSION		pEramExt,
	IN PFAT_ID				pFatId,
	IN PUNICODE_STRING		pUniVol
 );

VOID PrepareExtFileName(
	IN PERAM_EXTENSION		pEramExt,
	IN PFAT_ID				pFatId,
	IN PUNICODE_STRING		pRegParam
 );

BOOLEAN EramFormatFat(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 );

VOID EramSetup(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 );

VOID EramLocate(
	IN PERAM_EXTENSION	pEramExt
 );

BOOLEAN EramFormat(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 );

BOOLEAN EramClearInfo(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 );

BOOLEAN ExtClear(
	IN PERAM_EXTENSION	pEramExt,
	IN ULONG			uSize
 );

BOOLEAN ExtFileClear(
	IN PERAM_EXTENSION	pEramExt,
	IN ULONG			uSize
 );

DWORD CalcEramInfoPage(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 );

BOOLEAN EramMakeFAT(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 );

BOOLEAN EramSetLabel(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 );

BOOLEAN GetExternalStart(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt
 );

BOOLEAN GetMaxMem(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt,
	IN PWSTR			pwStr,
	OUT PULONG			puSize
 );

BOOLEAN CheckMaxMem(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt,
	IN ULONG			uSize
 );

BOOLEAN CheckExternalSize(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt
 );

VOID ResourceInitTiny(
	IN PDRIVER_OBJECT		pDrvObj,
	IN PCM_RESOURCE_LIST	pResList,
	IN ULONG				uStart,
	IN ULONG				uSize
 );

BOOLEAN CheckExternalMemoryExist(
	IN PDRIVER_OBJECT	pDrvObj,
	IN ULONG			uStart,
	IN ULONG			uDiskSize,
	OUT PULONG			puSize,
	IN ULONG			dwMaxAdr
 );

BOOLEAN ResourceSetupTiny(
	IN PDRIVER_OBJECT		pDrvObj,
	IN ULONG				uStart,
	IN PPHYSICAL_ADDRESS	pMapAdr
 );

BOOLEAN ExtReport(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt
 );

DWORD GetAcpiReservedMemory(
	IN PDRIVER_OBJECT	pDrvObj
 );

DWORD CheckAcpiRsdt(
	IN PDRIVER_OBJECT	pDrvObj,
	IN DWORD			dwMinValue,
	IN DWORD			dwRsdt
 );

DWORD CheckRsdtElements(
	IN PDRIVER_OBJECT	pDrvObj,
	IN DWORD			dwMinValue,
	IN DWORD			dwRsdtElement
 );

/* Definitions for releasing after initialization */
#ifdef ALLOC_PRAGMA
#pragma	alloc_text(INIT, DriverEntry)
#pragma	alloc_text(INIT, InitFatId)
#pragma	alloc_text(INIT, EramInitDisk)
#pragma	alloc_text(INIT, MemSetup)
#pragma	alloc_text(INIT, OsAlloc)
#pragma	alloc_text(INIT, CalcAvailSize)
#pragma	alloc_text(INIT, CheckSwapable)
#pragma	alloc_text(INIT, CheckDeviceName)
#pragma	alloc_text(INIT, CheckSwitch)
#pragma	alloc_text(INIT, GetMaxAddress)
#pragma	alloc_text(INIT, PrepareVolumeLabel)
#pragma	alloc_text(INIT, CheckVolumeLabel)
#pragma	alloc_text(INIT, PrepareExtFileName)
#pragma	alloc_text(INIT, EramFormatFat)
#pragma	alloc_text(INIT, EramSetup)
#pragma	alloc_text(INIT, EramLocate)
#pragma	alloc_text(INIT, EramFormat)
#pragma	alloc_text(INIT, EramClearInfo)
#pragma	alloc_text(INIT, ExtClear)
#pragma	alloc_text(INIT, ExtFileClear)
#pragma	alloc_text(INIT, CalcEramInfoPage)
#pragma	alloc_text(INIT, EramMakeFAT)
#pragma	alloc_text(INIT, EramSetLabel)
#pragma	alloc_text(INIT, GetExternalStart)
#pragma	alloc_text(INIT, GetMaxMem)
#pragma	alloc_text(INIT, CheckMaxMem)
#pragma	alloc_text(INIT, CheckExternalSize)
#pragma	alloc_text(INIT, ResourceInitTiny)
#pragma	alloc_text(INIT, CheckExternalMemoryExist)
#pragma	alloc_text(INIT, ResourceSetupTiny)
#pragma	alloc_text(INIT, ExtReport)
#pragma	alloc_text(INIT, GetAcpiReservedMemory)
#pragma	alloc_text(INIT, CheckAcpiRsdt)
#pragma	alloc_text(INIT, CheckRsdtElements)

#endif	// ALLOC_PRAGMA

#pragma pack()
