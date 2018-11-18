;//
;//	Message Definitions for ERAMNT
;//

;//	Header
LanguageNames=(English=0x409:ERAMNTUE)
MessageIdTypedef=NTSTATUS
FacilityNames=(System=0x0FF)

;//	Message Definitions
MessageId=0x1
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_FUNCTIONERROR
Language=English
Function %2 returned a failure.%0
.

MessageId=0x2
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_ALREADY_FREE
Language=English
OS-Unmanaged Memory is already released.%0
.

MessageId=0x3
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_MAP_FAILED
Language=English
Failed to map OS-Unmanaged Memory.%0
.

MessageId=0x4
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_REPORT_USAGE_FAILED
Language=English
Failed to notify usage of OS-Unmanaged Memory.%0
.

MessageId=0x5
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_REPORT_USAGE_CONFLICT
Language=English
OS-Unmanaged Memory is being partially used by another device. Please shrink the disk size.%0
.

MessageId=0x6
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_CREATE_THREAD
Language=English
Failed to create a system thread.%0
.

MessageId=0x7
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_NO_MEMORY
Language=English
OS-Unmanaged Memory was not detected.%0
.

MessageId=0x8
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_NOT_DETECTED
Language=English
OS-Unmanaged Memory was not found.%0
.

MessageId=0x9
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_OPTION_WORK_ALLOC_FAILED
Language=English
Failed to allocate the OS startup option(s).%0
.

MessageId=0xa
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_OPTION_GET_FAILED
Language=English
Failed to get the OS startup option(s).%0
.

MessageId=0xb
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_NO_OPTION
Language=English
There was no %bMAXMEM=n%b in startup options.%0
.

MessageId=0xc
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_CAPITAL_FAILED
Language=English
Failed to capitalize the startup option(s).%0
.

MessageId=0xd
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_ATOU
Language=English
Failed to numerize MAXMEM.%0
.

MessageId=0xe
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_TOO_SMALL
Language=English
Please specify 17 or more for MAXMEM.%0
.

MessageId=0xf
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_TOO_BIG
Language=English
Please specify the smaller value for MAXMEM than 4095.%0
.

MessageId=0x10
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_INVALID
Language=English
The startup option(s) has no MAXMEM=n or invalid MAXMEM value.%0
.

MessageId=0x11
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_EXTSTART_TOO_BIG
Language=English
Please specify the smaller value than 4095MB for ExtStart.%0
.

MessageId=0x12
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_WORK_ALLOC_FAILED
Language=English
Failed to allocate the work-area memory.%0
.

MessageId=0x13
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_REG_KEY_APPEND_FAILED
Language=English
Failed to combine the registry key strings.%0
.

MessageId=0x14
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_CREATE_DEVICE_FAILED
Language=English
Failed to create a device.%0
.

MessageId=0x15
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_DISK_SIZE_TOO_SMALL
Language=English
The assigned disk size is too small.%0
.

MessageId=0x16
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_DEVICE_NAME_ALLOC_FAILED
Language=English
Failed to allocate the device name area.%0
.

MessageId=0x17
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_CREATE_SYMBOLIC_LINK_FAILED
Language=English
Failed to create a symbolic link.%0
.

MessageId=0x18
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_DISK_ALLOC_FAILED
Language=English
Failed to allocate the memory to be used for the disk.%0
.

MessageId=0x19
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_DISK_SIZE_IS_0
Language=English
The size of the disk cannot be 0.%0
.

MessageId=0x1a
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_TRANSLATE_ADDRESS_FAILED
Language=English
Failed to convert to OS-Managed Address.%0
.

MessageId=0x1b
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_PORT_MAPPED
Language=English
No support of I/O space mapping.%0
.

MessageId=0x1c
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_CREATE_EXT_FILE
Language=English
Failed to create the external file.%0
.

MessageId=0x1d
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_SET_INFO_EXT_FILE
Language=English
Failed to adjust the size of the external file.%0
.

MessageId=0x1e
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_CREATE_EXT_FILE_SECTION
Language=English
Failed to create a section of the external file.%0
.

MessageId=0x1f
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAP_EXT_FILE
Language=English
Failed to map the external file.%0
.

MessageId=0x20
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_GET_THREAD_OBJECT
Language=English
Failed to get the object of the system thread.%0
.


MessageId=0x6
Severity=Warning
Facility=System
SymbolicName=ERAMNT_WARN_MAXMEM_DISK_SIZE_FIXED
Language=English
The RAM disk was shrinked because OS-Unmanaged Memory was smaller than expected.%0
.


MessageId=0x1c
Severity=Informational
Facility=System
SymbolicName=ERAMNT_INF_MEMORY_SIZE
Language=English
%2KB is able to be allocated.%0
.

