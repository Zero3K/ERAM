;//
;//	ERAMNT用メッセージ定義
;//

;//	ヘッダ
LanguageNames=(English=0x409:ERAMNTUE)
LanguageNames=(Japanese=0x411:ERAMNTUM)
MessageIdTypedef=NTSTATUS
FacilityNames=(System=0x0FF)

;//	メッセージ定義
MessageId=0x1
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_FUNCTIONERROR
Language=Japanese
関数%2で失敗が返されました.%0
.
Language=English
Function %2 error.%0
.

MessageId=0x2
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_ALREADY_FREE
Language=Japanese
OS管理外ﾒﾓﾘが既に解放されています.%0
.
Language=English
External memory already free.%0
.

MessageId=0x3
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_MAP_FAILED
Language=Japanese
OS管理外ﾒﾓﾘのﾏｯﾌﾟ失敗.%0
.
Language=English
MmMapIoSpace failed.%0
.

MessageId=0x4
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_REPORT_USAGE_FAILED
Language=Japanese
OS管理外ﾒﾓﾘの使用通知失敗.%0
.
Language=English
IoReportResourceUsage failed.%0
.

MessageId=0x5
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_REPORT_USAGE_CONFLICT
Language=Japanese
OS管理外ﾒﾓﾘの一部は、他のﾃﾞﾊﾞｲｽにより使用されています.ﾃﾞｨｽｸｻｲｽﾞを小さくしてみてください.%0
.
Language=English
IoReportResourceUsage conflict.%0
.

MessageId=0x6
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_CREATE_THREAD
Language=Japanese
ｼｽﾃﾑｽﾚｯﾄﾞの作成に失敗しました.%0
.
Language=English
PsCreateSystemThread failed.%0
.

MessageId=0x7
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_NO_MEMORY
Language=Japanese
OS管理外ﾒﾓﾘは検出されませんでした.%0
.
Language=English
External memory not detected.%0
.

MessageId=0x8
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_NOT_DETECTED
Language=Japanese
OS管理外ﾒﾓﾘが見つかりません.%0
.
Language=English
External memory not found.%0
.

MessageId=0x9
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_OPTION_WORK_ALLOC_FAILED
Language=Japanese
OS起動ｵﾌﾟｼｮﾝ用ﾒﾓﾘ確保失敗.%0
.
Language=English
ExAllocatePool(for work) failed.%0
.

MessageId=0xa
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_OPTION_GET_FAILED
Language=Japanese
OS起動ｵﾌﾟｼｮﾝ取得失敗.%0
.
Language=English
RtlQueryRegistryValues failed.%0
.

MessageId=0xb
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_NO_OPTION
Language=Japanese
起動ｵﾌﾟｼｮﾝに%bMAXMEM=n%bがありません.%0
.
Language=English
Need MAXMEM.%0
.

MessageId=0xc
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_CAPITAL_FAILED
Language=Japanese
起動ｵﾌﾟｼｮﾝ大文字化失敗.%0
.
Language=English
RtlUpcaseUnicodeString failed.%0
.

MessageId=0xd
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_ATOU
Language=Japanese
MAXMEM数値化失敗.%0
.
Language=English
RtlUnicodeStringToInteger failed.%0
.

MessageId=0xe
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_TOO_SMALL
Language=Japanese
MAXMEMは17以上を指定してください.%0
.
Language=English
Need MAXMEM >= 17.%0
.

MessageId=0xf
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_TOO_BIG
Language=Japanese
MAXMEMは4095未満を指定してください.%0
.
Language=English
Need MAXMEM < 4095.%0
.

MessageId=0x10
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAXMEM_INVALID
Language=Japanese
起動ｵﾌﾟｼｮﾝに MAXMEM=n が無いか、不正な値です.%0
.
Language=English
MAXMEM not available.%0
.

MessageId=0x11
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_EXTSTART_TOO_BIG
Language=Japanese
ExtStartは4095MB未満を指定してください.%0
.
Language=English
ExtStart too big.%0
.

MessageId=0x12
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_WORK_ALLOC_FAILED
Language=Japanese
作業用ﾒﾓﾘ確保失敗.%0
.
Language=English
ExAllocatePool(for work) failed.%0
.

MessageId=0x13
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_REG_KEY_APPEND_FAILED
Language=Japanese
ﾚｼﾞｽﾄﾘｷｰ文字列合成失敗.%0
.
Language=English
RtlAppendUnicodeStringToString failed.%0
.

MessageId=0x14
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_CREATE_DEVICE_FAILED
Language=Japanese
ﾃﾞﾊﾞｲｽ作成失敗.%0
.
Language=English
IoCreateDevice failed.%0
.

MessageId=0x15
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_DISK_SIZE_TOO_SMALL
Language=Japanese
ﾃﾞｨｽｸへの割り当てﾒﾓﾘが小さすぎます.%0
.
Language=English
Disk size too small.%0
.

MessageId=0x16
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_DEVICE_NAME_ALLOC_FAILED
Language=Japanese
ﾃﾞﾊﾞｲｽ名領域確保失敗.%0
.
Language=English
ExAllocatePool(for device name) failed.%0
.

MessageId=0x17
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_CREATE_SYMBOLIC_LINK_FAILED
Language=Japanese
ｼﾝﾎﾞﾘｯｸﾘﾝｸ作成失敗.%0
.
Language=English
IoCreateSymbolicLink failed.%0
.

MessageId=0x18
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_DISK_ALLOC_FAILED
Language=Japanese
ﾃﾞｨｽｸ用ﾒﾓﾘ確保失敗.%0
.
Language=English
ExAllocatePool(for disk space) failed.%0
.

MessageId=0x19
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_DISK_SIZE_IS_0
Language=Japanese
確保容量が0です.%0
.
Language=English
Disk size 0 error.%0
.

MessageId=0x1a
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_TRANSLATE_ADDRESS_FAILED
Language=Japanese
OS管理ｱﾄﾞﾚｽへの変換に失敗.%0
.
Language=English
HalTranslateBusAddress failed.%0
.

MessageId=0x1b
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_PORT_MAPPED
Language=Japanese
I/O空間のﾏｯﾌﾟには対応していません.%0
.
Language=English
I/O map not supported.%0
.

MessageId=0x1c
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_CREATE_EXT_FILE
Language=Japanese
外部ﾌｧｲﾙの作成に失敗しました.%0
.
Language=English
ZwCreateFile failed.%0
.

MessageId=0x1d
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_SET_INFO_EXT_FILE
Language=Japanese
外部ﾌｧｲﾙのｻｲｽﾞ調整に失敗しました.%0
.
Language=English
ZwSetInformationFile failed.%0
.

MessageId=0x1e
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_CREATE_EXT_FILE_SECTION
Language=Japanese
外部ﾌｧｲﾙのｾｸｼｮﾝ作成に失敗しました.%0
.
Language=English
ZwCreateSection failed.%0
.

MessageId=0x1f
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_MAP_EXT_FILE
Language=Japanese
外部ﾌｧｲﾙのﾏｯﾌﾟに失敗しました.%0
.
Language=English
ZwMapViewOfSection failed.%0
.

MessageId=0x20
Severity=Error
Facility=System
SymbolicName=ERAMNT_ERROR_GET_THREAD_OBJECT
Language=Japanese
ｼｽﾃﾑｽﾚｯﾄﾞのｵﾌﾞｼﾞｪｸﾄ取得に失敗しました.%0
.
Language=English
ObReferenceObjectByHandle failed.%0
.


MessageId=0x6
Severity=Warning
Facility=System
SymbolicName=ERAMNT_WARN_MAXMEM_DISK_SIZE_FIXED
Language=Japanese
OS管理外ﾒﾓﾘが少ないのでRAMﾃﾞｨｽｸは縮小されました.%0
.
Language=English
Disk size fixed.%0
.


MessageId=0x1c
Severity=Informational
Facility=System
SymbolicName=ERAMNT_INF_MEMORY_SIZE
Language=Japanese
%2KB程度確保可能と思われます.%0
.
Language=English
Perhaps you can allocate %2KB.%0
.

