//
//	ERAMNT用メッセージ定義
//
//	ヘッダ
//	メッセージ定義
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
// MessageId: ERAMNT_ERROR_FUNCTIONERROR
//
// MessageText:
//
//  関数%2で失敗が返されました.%0
//
#define ERAMNT_ERROR_FUNCTIONERROR       ((NTSTATUS)0xE0FF0001L)

//
// MessageId: ERAMNT_ERROR_MAXMEM_ALREADY_FREE
//
// MessageText:
//
//  OS管理外ﾒﾓﾘが既に解放されています.%0
//
#define ERAMNT_ERROR_MAXMEM_ALREADY_FREE ((NTSTATUS)0xE0FF0002L)

//
// MessageId: ERAMNT_ERROR_MAXMEM_MAP_FAILED
//
// MessageText:
//
//  OS管理外ﾒﾓﾘのﾏｯﾌﾟ失敗.%0
//
#define ERAMNT_ERROR_MAXMEM_MAP_FAILED   ((NTSTATUS)0xE0FF0003L)

//
// MessageId: ERAMNT_ERROR_MAXMEM_REPORT_USAGE_FAILED
//
// MessageText:
//
//  OS管理外ﾒﾓﾘの使用通知失敗.%0
//
#define ERAMNT_ERROR_MAXMEM_REPORT_USAGE_FAILED ((NTSTATUS)0xE0FF0004L)

//
// MessageId: ERAMNT_ERROR_MAXMEM_REPORT_USAGE_CONFLICT
//
// MessageText:
//
//  OS管理外ﾒﾓﾘの一部は、他のﾃﾞﾊﾞｲｽにより使用されています.ﾃﾞｨｽｸｻｲｽﾞを小さくしてみてください.%0
//
#define ERAMNT_ERROR_MAXMEM_REPORT_USAGE_CONFLICT ((NTSTATUS)0xE0FF0005L)

//
// MessageId: ERAMNT_ERROR_CREATE_THREAD
//
// MessageText:
//
//  ｼｽﾃﾑｽﾚｯﾄﾞの作成に失敗しました.%0
//
#define ERAMNT_ERROR_CREATE_THREAD       ((NTSTATUS)0xE0FF0006L)

//
// MessageId: ERAMNT_ERROR_MAXMEM_NO_MEMORY
//
// MessageText:
//
//  OS管理外ﾒﾓﾘは検出されませんでした.%0
//
#define ERAMNT_ERROR_MAXMEM_NO_MEMORY    ((NTSTATUS)0xE0FF0007L)

//
// MessageId: ERAMNT_ERROR_MAXMEM_NOT_DETECTED
//
// MessageText:
//
//  OS管理外ﾒﾓﾘが見つかりません.%0
//
#define ERAMNT_ERROR_MAXMEM_NOT_DETECTED ((NTSTATUS)0xE0FF0008L)

//
// MessageId: ERAMNT_ERROR_OPTION_WORK_ALLOC_FAILED
//
// MessageText:
//
//  OS起動ｵﾌﾟｼｮﾝ用ﾒﾓﾘ確保失敗.%0
//
#define ERAMNT_ERROR_OPTION_WORK_ALLOC_FAILED ((NTSTATUS)0xE0FF0009L)

//
// MessageId: ERAMNT_ERROR_OPTION_GET_FAILED
//
// MessageText:
//
//  OS起動ｵﾌﾟｼｮﾝ取得失敗.%0
//
#define ERAMNT_ERROR_OPTION_GET_FAILED   ((NTSTATUS)0xE0FF000AL)

//
// MessageId: ERAMNT_ERROR_MAXMEM_NO_OPTION
//
// MessageText:
//
//  起動ｵﾌﾟｼｮﾝに%bMAXMEM=n%bがありません.%0
//
#define ERAMNT_ERROR_MAXMEM_NO_OPTION    ((NTSTATUS)0xE0FF000BL)

//
// MessageId: ERAMNT_ERROR_MAXMEM_CAPITAL_FAILED
//
// MessageText:
//
//  起動ｵﾌﾟｼｮﾝ大文字化失敗.%0
//
#define ERAMNT_ERROR_MAXMEM_CAPITAL_FAILED ((NTSTATUS)0xE0FF000CL)

//
// MessageId: ERAMNT_ERROR_MAXMEM_ATOU
//
// MessageText:
//
//  MAXMEM数値化失敗.%0
//
#define ERAMNT_ERROR_MAXMEM_ATOU         ((NTSTATUS)0xE0FF000DL)

//
// MessageId: ERAMNT_ERROR_MAXMEM_TOO_SMALL
//
// MessageText:
//
//  MAXMEMは17以上を指定してください.%0
//
#define ERAMNT_ERROR_MAXMEM_TOO_SMALL    ((NTSTATUS)0xE0FF000EL)

//
// MessageId: ERAMNT_ERROR_MAXMEM_TOO_BIG
//
// MessageText:
//
//  MAXMEMは4095未満を指定してください.%0
//
#define ERAMNT_ERROR_MAXMEM_TOO_BIG      ((NTSTATUS)0xE0FF000FL)

//
// MessageId: ERAMNT_ERROR_MAXMEM_INVALID
//
// MessageText:
//
//  起動ｵﾌﾟｼｮﾝに MAXMEM=n が無いか、不正な値です.%0
//
#define ERAMNT_ERROR_MAXMEM_INVALID      ((NTSTATUS)0xE0FF0010L)

//
// MessageId: ERAMNT_ERROR_EXTSTART_TOO_BIG
//
// MessageText:
//
//  ExtStartは4095MB未満を指定してください.%0
//
#define ERAMNT_ERROR_EXTSTART_TOO_BIG    ((NTSTATUS)0xE0FF0011L)

//
// MessageId: ERAMNT_ERROR_WORK_ALLOC_FAILED
//
// MessageText:
//
//  作業用ﾒﾓﾘ確保失敗.%0
//
#define ERAMNT_ERROR_WORK_ALLOC_FAILED   ((NTSTATUS)0xE0FF0012L)

//
// MessageId: ERAMNT_ERROR_REG_KEY_APPEND_FAILED
//
// MessageText:
//
//  ﾚｼﾞｽﾄﾘｷｰ文字列合成失敗.%0
//
#define ERAMNT_ERROR_REG_KEY_APPEND_FAILED ((NTSTATUS)0xE0FF0013L)

//
// MessageId: ERAMNT_ERROR_CREATE_DEVICE_FAILED
//
// MessageText:
//
//  ﾃﾞﾊﾞｲｽ作成失敗.%0
//
#define ERAMNT_ERROR_CREATE_DEVICE_FAILED ((NTSTATUS)0xE0FF0014L)

//
// MessageId: ERAMNT_ERROR_DISK_SIZE_TOO_SMALL
//
// MessageText:
//
//  ﾃﾞｨｽｸへの割り当てﾒﾓﾘが小さすぎます.%0
//
#define ERAMNT_ERROR_DISK_SIZE_TOO_SMALL ((NTSTATUS)0xE0FF0015L)

//
// MessageId: ERAMNT_ERROR_DEVICE_NAME_ALLOC_FAILED
//
// MessageText:
//
//  ﾃﾞﾊﾞｲｽ名領域確保失敗.%0
//
#define ERAMNT_ERROR_DEVICE_NAME_ALLOC_FAILED ((NTSTATUS)0xE0FF0016L)

//
// MessageId: ERAMNT_ERROR_CREATE_SYMBOLIC_LINK_FAILED
//
// MessageText:
//
//  ｼﾝﾎﾞﾘｯｸﾘﾝｸ作成失敗.%0
//
#define ERAMNT_ERROR_CREATE_SYMBOLIC_LINK_FAILED ((NTSTATUS)0xE0FF0017L)

//
// MessageId: ERAMNT_ERROR_DISK_ALLOC_FAILED
//
// MessageText:
//
//  ﾃﾞｨｽｸ用ﾒﾓﾘ確保失敗.%0
//
#define ERAMNT_ERROR_DISK_ALLOC_FAILED   ((NTSTATUS)0xE0FF0018L)

//
// MessageId: ERAMNT_ERROR_DISK_SIZE_IS_0
//
// MessageText:
//
//  確保容量が0です.%0
//
#define ERAMNT_ERROR_DISK_SIZE_IS_0      ((NTSTATUS)0xE0FF0019L)

//
// MessageId: ERAMNT_ERROR_TRANSLATE_ADDRESS_FAILED
//
// MessageText:
//
//  OS管理ｱﾄﾞﾚｽへの変換に失敗.%0
//
#define ERAMNT_ERROR_TRANSLATE_ADDRESS_FAILED ((NTSTATUS)0xE0FF001AL)

//
// MessageId: ERAMNT_ERROR_PORT_MAPPED
//
// MessageText:
//
//  I/O空間のﾏｯﾌﾟには対応していません.%0
//
#define ERAMNT_ERROR_PORT_MAPPED         ((NTSTATUS)0xE0FF001BL)

//
// MessageId: ERAMNT_ERROR_CREATE_EXT_FILE
//
// MessageText:
//
//  外部ﾌｧｲﾙの作成に失敗しました.%0
//
#define ERAMNT_ERROR_CREATE_EXT_FILE     ((NTSTATUS)0xE0FF001CL)

//
// MessageId: ERAMNT_ERROR_SET_INFO_EXT_FILE
//
// MessageText:
//
//  外部ﾌｧｲﾙのｻｲｽﾞ調整に失敗しました.%0
//
#define ERAMNT_ERROR_SET_INFO_EXT_FILE   ((NTSTATUS)0xE0FF001DL)

//
// MessageId: ERAMNT_ERROR_CREATE_EXT_FILE_SECTION
//
// MessageText:
//
//  外部ﾌｧｲﾙのｾｸｼｮﾝ作成に失敗しました.%0
//
#define ERAMNT_ERROR_CREATE_EXT_FILE_SECTION ((NTSTATUS)0xE0FF001EL)

//
// MessageId: ERAMNT_ERROR_MAP_EXT_FILE
//
// MessageText:
//
//  外部ﾌｧｲﾙのﾏｯﾌﾟに失敗しました.%0
//
#define ERAMNT_ERROR_MAP_EXT_FILE        ((NTSTATUS)0xE0FF001FL)

//
// MessageId: ERAMNT_ERROR_GET_THREAD_OBJECT
//
// MessageText:
//
//  ｼｽﾃﾑｽﾚｯﾄﾞのｵﾌﾞｼﾞｪｸﾄ取得に失敗しました.%0
//
#define ERAMNT_ERROR_GET_THREAD_OBJECT   ((NTSTATUS)0xE0FF0020L)

//
// MessageId: ERAMNT_WARN_MAXMEM_DISK_SIZE_FIXED
//
// MessageText:
//
//  OS管理外ﾒﾓﾘが少ないのでRAMﾃﾞｨｽｸは縮小されました.%0
//
#define ERAMNT_WARN_MAXMEM_DISK_SIZE_FIXED ((NTSTATUS)0xA0FF0006L)

//
// MessageId: ERAMNT_INF_MEMORY_SIZE
//
// MessageText:
//
//  %2KB程度確保可能と思われます.%0
//
#define ERAMNT_INF_MEMORY_SIZE           ((NTSTATUS)0x60FF001CL)

