//
//	Message Definitions for ERAM
//
//	Header
//	Message Definition
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: ERAM_ERROR_FUNCTIONERROR
//
// MessageText:
//
//  Function %2 returned a failure.%0
//
#define ERAM_ERROR_FUNCTIONERROR       ((NTSTATUS)0xE0FF0001L)

//
// MessageId: ERAM_ERROR_MAXMEM_ALREADY_FREE
//
// MessageText:
//
//  OS-Unmanaged Memory is already released.%0
//
#define ERAM_ERROR_MAXMEM_ALREADY_FREE ((NTSTATUS)0xE0FF0002L)

//
// MessageId: ERAM_ERROR_MAXMEM_MAP_FAILED
//
// MessageText:
//
//  Failed to map OS-Unmanaged Memory.%0
//
#define ERAM_ERROR_MAXMEM_MAP_FAILED   ((NTSTATUS)0xE0FF0003L)

//
// MessageId: ERAM_ERROR_MAXMEM_REPORT_USAGE_FAILED
//
// MessageText:
//
//  Failed to notify usage of OS-Unmanaged Memory.%0
//
#define ERAM_ERROR_MAXMEM_REPORT_USAGE_FAILED ((NTSTATUS)0xE0FF0004L)

//
// MessageId: ERAM_ERROR_MAXMEM_REPORT_USAGE_CONFLICT
//
// MessageText:
//
//  OS-Unmanaged Memory is being partially used by another device. Please shrink the disk size.%0
//
#define ERAM_ERROR_MAXMEM_REPORT_USAGE_CONFLICT ((NTSTATUS)0xE0FF0005L)

//
// MessageId: ERAM_ERROR_CREATE_THREAD
//
// MessageText:
//
//  Failed to create a system thread.%0
//
#define ERAM_ERROR_CREATE_THREAD       ((NTSTATUS)0xE0FF0006L)

//
// MessageId: ERAM_ERROR_MAXMEM_NO_MEMORY
//
// MessageText:
//
//  OS-Unmanaged Memory was not detected.%0
//
#define ERAM_ERROR_MAXMEM_NO_MEMORY    ((NTSTATUS)0xE0FF0007L)

//
// MessageId: ERAM_ERROR_MAXMEM_NOT_DETECTED
//
// MessageText:
//
//  OS-Unmanaged Memory was not found.%0
//
#define ERAM_ERROR_MAXMEM_NOT_DETECTED ((NTSTATUS)0xE0FF0008L)

//
// MessageId: ERAM_ERROR_OPTION_WORK_ALLOC_FAILED
//
// MessageText:
//
//  Failed to allocate the OS startup option(s).%0
//
#define ERAM_ERROR_OPTION_WORK_ALLOC_FAILED ((NTSTATUS)0xE0FF0009L)

//
// MessageId: ERAM_ERROR_OPTION_GET_FAILED
//
// MessageText:
//
//  Failed to get the OS startup option(s).%0
//
#define ERAM_ERROR_OPTION_GET_FAILED   ((NTSTATUS)0xE0FF000AL)

//
// MessageId: ERAM_ERROR_MAXMEM_NO_OPTION
//
// MessageText:
//
//  There was no %bMAXMEM=n%b in starup options.%0
//
#define ERAM_ERROR_MAXMEM_NO_OPTION    ((NTSTATUS)0xE0FF000BL)

//
// MessageId: ERAM_ERROR_MAXMEM_CAPITAL_FAILED
//
// MessageText:
//
//  Failed to capitalize the startup option(s).%0
//
#define ERAM_ERROR_MAXMEM_CAPITAL_FAILED ((NTSTATUS)0xE0FF000CL)

//
// MessageId: ERAM_ERROR_MAXMEM_ATOU
//
// MessageText:
//
//  Failed to numerize MAXMEM.%0
//
#define ERAM_ERROR_MAXMEM_ATOU         ((NTSTATUS)0xE0FF000DL)

//
// MessageId: ERAM_ERROR_MAXMEM_TOO_SMALL
//
// MessageText:
//
//  Please specify 17 or more for MAXMEM.%0
//
#define ERAM_ERROR_MAXMEM_TOO_SMALL    ((NTSTATUS)0xE0FF000EL)

//
// MessageId: ERAM_ERROR_MAXMEM_TOO_BIG
//
// MessageText:
//
//  Please specify the smaller value for MAXMEM than 4095.%0
//
#define ERAM_ERROR_MAXMEM_TOO_BIG      ((NTSTATUS)0xE0FF000FL)

//
// MessageId: ERAM_ERROR_MAXMEM_INVALID
//
// MessageText:
//
//  The startup option(s) has no MAXMEM=n or invalid MAXMEM value.%0
//
#define ERAM_ERROR_MAXMEM_INVALID      ((NTSTATUS)0xE0FF0010L)

//
// MessageId: ERAM_ERROR_EXTSTART_TOO_BIG
//
// MessageText:
//
//  Please specify the smaller value than 4095MB for ExtStart.%0
//
#define ERAM_ERROR_EXTSTART_TOO_BIG    ((NTSTATUS)0xE0FF0011L)

//
// MessageId: ERAM_ERROR_WORK_ALLOC_FAILED
//
// MessageText:
//
//  Failed to allocate the workarea memory.%0
//
#define ERAM_ERROR_WORK_ALLOC_FAILED   ((NTSTATUS)0xE0FF0012L)

//
// MessageId: ERAM_ERROR_REG_KEY_APPEND_FAILED
//
// MessageText:
//
//  Failed to combine the registry key strings.%0
//
#define ERAM_ERROR_REG_KEY_APPEND_FAILED ((NTSTATUS)0xE0FF0013L)

//
// MessageId: ERAM_ERROR_CREATE_DEVICE_FAILED
//
// MessageText:
//
//  Failed to create an device.%0
//
#define ERAM_ERROR_CREATE_DEVICE_FAILED ((NTSTATUS)0xE0FF0014L)

//
// MessageId: ERAM_ERROR_DISK_SIZE_TOO_SMALL
//
// MessageText:
//
//  Too small allocation memory for disk.%0
//
#define ERAM_ERROR_DISK_SIZE_TOO_SMALL ((NTSTATUS)0xE0FF0015L)

//
// MessageId: ERAM_ERROR_DEVICE_NAME_ALLOC_FAILED
//
// MessageText:
//
//  Failed to allocate the device name area.%0
//
#define ERAM_ERROR_DEVICE_NAME_ALLOC_FAILED ((NTSTATUS)0xE0FF0016L)

//
// MessageId: ERAM_ERROR_CREATE_SYMBOLIC_LINK_FAILED
//
// MessageText:
//
//  Failed to create a symbolic link.%0
//
#define ERAM_ERROR_CREATE_SYMBOLIC_LINK_FAILED ((NTSTATUS)0xE0FF0017L)

//
// MessageId: ERAM_ERROR_DISK_ALLOC_FAILED
//
// MessageText:
//
//  Failed to allocate the memory for disk.%0
//
#define ERAM_ERROR_DISK_ALLOC_FAILED   ((NTSTATUS)0xE0FF0018L)

//
// MessageId: ERAM_ERROR_DISK_SIZE_IS_0
//
// MessageText:
//
//  The allocated size was zero.%0
//
#define ERAM_ERROR_DISK_SIZE_IS_0      ((NTSTATUS)0xE0FF0019L)

//
// MessageId: ERAM_ERROR_TRANSLATE_ADDRESS_FAILED
//
// MessageText:
//
//  Failed to convert to OS-Managed Address.%0
//
#define ERAM_ERROR_TRANSLATE_ADDRESS_FAILED ((NTSTATUS)0xE0FF001AL)

//
// MessageId: ERAM_ERROR_PORT_MAPPED
//
// MessageText:
//
//  No support of I/O space mapping.%0
//
#define ERAM_ERROR_PORT_MAPPED         ((NTSTATUS)0xE0FF001BL)

//
// MessageId: ERAM_ERROR_CREATE_EXT_FILE
//
// MessageText:
//
//  Failed to create the external file.%0
//
#define ERAM_ERROR_CREATE_EXT_FILE     ((NTSTATUS)0xE0FF001CL)

//
// MessageId: ERAM_ERROR_SET_INFO_EXT_FILE
//
// MessageText:
//
//  Failed to adjust the size of the external file.%0
//
#define ERAM_ERROR_SET_INFO_EXT_FILE   ((NTSTATUS)0xE0FF001DL)

//
// MessageId: ERAM_ERROR_CREATE_EXT_FILE_SECTION
//
// MessageText:
//
//  Failed to create section of the external file.%0
//
#define ERAM_ERROR_CREATE_EXT_FILE_SECTION ((NTSTATUS)0xE0FF001EL)

//
// MessageId: ERAM_ERROR_MAP_EXT_FILE
//
// MessageText:
//
//  Failed to map the external file.%0
//
#define ERAM_ERROR_MAP_EXT_FILE        ((NTSTATUS)0xE0FF001FL)

//
// MessageId: ERAM_ERROR_GET_THREAD_OBJECT
//
// MessageText:
//
//  Failed to get the object of the system thread.%0
//
#define ERAM_ERROR_GET_THREAD_OBJECT   ((NTSTATUS)0xE0FF0020L)

//
// MessageId: ERAM_WARN_MAXMEM_DISK_SIZE_FIXED
//
// MessageText:
//
// The RAM disk was shrinked because OS-Unmanaged Memory was small.%0
//
#define ERAM_WARN_MAXMEM_DISK_SIZE_FIXED ((NTSTATUS)0xA0FF0006L)

//
// MessageId: ERAM_INF_MEMORY_SIZE
//
// MessageText:
//
//  It seems like it is able to allocate about %2KB.%0
//
#define ERAM_INF_MEMORY_SIZE           ((NTSTATUS)0x60FF001CL)

