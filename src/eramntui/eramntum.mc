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
Function %2 error.%0
.

MessageId=0x2
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_ALREADY_FREE
Language=English
External memory already free.%0
.

MessageId=0x3
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_MAP_FAILED
Language=English
MmMapIoSpace failed.%0
.

MessageId=0x4
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_REPORT_USAGE_FAILED
Language=English
IoReportResourceUsage failed.%0
.

MessageId=0x5
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_REPORT_USAGE_CONFLICT
Language=English
IoReportResourceUsage conflict.%0
.

MessageId=0x6
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_CREATE_THREAD
Language=English
PsCreateSystemThread failed.%0
.

MessageId=0x7
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_NO_MEMORY
Language=English
External memory not detected.%0
.

MessageId=0x8
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_NOT_DETECTED
Language=English
External memory not found.%0
.

MessageId=0x9
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_OPTION_WORK_ALLOC_FAILED
Language=English
ExAllocatePool(for work) failed.%0
.

MessageId=0xa
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_OPTION_GET_FAILED
Language=English
RtlQueryRegistryValues failed.%0
.

MessageId=0xb
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_NO_OPTION
Language=English
Need MAXMEM.%0
.

MessageId=0xc
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_CAPITAL_FAILED
Language=English
RtlUpcaseUnicodeString failed.%0
.

MessageId=0xd
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_ATOU
Language=English
RtlUnicodeStringToInteger failed.%0
.

MessageId=0xe
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_TOO_SMALL
Language=English
Need MAXMEM >= 17.%0
.

MessageId=0xf
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_TOO_BIG
Language=English
Need MAXMEM < 4095.%0
.

MessageId=0x10
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_INVALID
Language=English
MAXMEM not available.%0
.

MessageId=0x11
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_EXTSTART_TOO_BIG
Language=English
ExtStart too big.%0
.

MessageId=0x12
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_WORK_ALLOC_FAILED
Language=English
ExAllocatePool(for work) failed.%0
.

MessageId=0x13
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_REG_KEY_APPEND_FAILED
Language=English
RtlAppendUnicodeStringToString failed.%0
.

MessageId=0x14
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_CREATE_DEVICE_FAILED
Language=English
IoCreateDevice failed.%0
.

MessageId=0x15
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_DISK_SIZE_TOO_SMALL
Language=English
Disk size too small.%0
.

MessageId=0x16
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_DEVICE_NAME_ALLOC_FAILED
Language=English
ExAllocatePool(for device name) failed.%0
.

MessageId=0x17
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_CREATE_SYMBOLIC_LINK_FAILED
Language=English
IoCreateSymbolicLink failed.%0
.

MessageId=0x18
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_DISK_ALLOC_FAILED
Language=English
ExAllocatePool(for disk space) failed.%0
.

MessageId=0x19
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_DISK_SIZE_IS_0
Language=English
Disk size 0 error.%0
.

MessageId=0x1a
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_TRANSLATE_ADDRESS_FAILED
Language=English
HalTranslateBusAddress failed.%0
.

MessageId=0x1b
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_PORT_MAPPED
Language=English
I/O map not supported.%0
.

MessageId=0x1c
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_CREATE_EXT_FILE
Language=English
ZwCreateFile failed.%0
.

MessageId=0x1d
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_SET_INFO_EXT_FILE
Language=English
ZwSetInformationFile failed.%0
.

MessageId=0x1e
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_CREATE_EXT_FILE_SECTION
Language=English
ZwCreateSection failed.%0
.

MessageId=0x1f
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAP_EXT_FILE
Language=English
ZwMapViewOfSection failed.%0
.

MessageId=0x20
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_GET_THREAD_OBJECT
Language=English
ObReferenceObjectByHandle failed.%0
.


MessageId=0x6
Severity=Warning
Facility=System
SymbolicName=ERAMNT_WARN_MAXMEM_DISK_SIZE_FIXED
Language=English
Disk size fixed.%0
.


MessageId=0x1c
Severity=Informational
Facility=System
SymbolicName=ERAMNT_INF_MEMORY_SIZE
Language=English
Perhaps you can allocate %2KB.%0
.

