/* ERAMNTUI.C    RAM Disk ERAM for WindowsNT/2000/XP
	Control Panel Applet / Class Installer
      Copyright (c) 1999-2004 by *Error15
    Translated into English by Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>.
*/

/* History
	v1.00
		Newly created.
	v1.01
		over32MB support (upto 64MB).
	v1.10
		Windows2000 class installer added.
	v1.11
		Windows2000 property sheet page added.
		Non-page, page pool size modification feature added.
	v2.00
		Non-page, page pool size modification feature obsoleted.
		OS Management Outside Memory setting feature added.
		Uninstaller feature added.
	v2.01
		Increased Allocation Unit 32 and enabled 1GB allocation.
		NT3.51 support was obsolete for uninstaller feature Windows2000 support enhancement.
	v2.02
		Add warning if drive character was in use.
		Fix that page number was 6 digits only.
		Enable to write the starting address of memory after MAXMEM=nn.
		Fixed that the reboot dialog won't be shown after delete.
		Fixed that the RAM disk was created immediately after deleting in NT.
		Enabled to save the info that the memory check was skipped or not.
		Resume the NT3.51 support.
		Fixed the lacking a colon of the drive character of registry after setting changed.
		Supported deletion of event log messages.
	v2.10
		Automatically set supported bits if Win2000 and later (=FAT32 support).
		64-nized max. allocation unit.
	v2.11
		WinXP support.
		Added startup change feature of Fastfat driver
	v2.12
		Reversal of UI of Memory upper limit detector.
		Add standby countermeasure unit.
	v2.20
		String resource american support.
		Maximum cluster size of 16 bit FAT is limited to 65525.
		Resource support of message box.
		32bit-nized option flag(s).
		4GB limitation added.
		Made real device setting possible without swapping.
		Add edit of the TEMP directory creation.
		root directory restriction added.
*/


#define STRICT
#pragma warning(disable : 4001 4054 4055 4100 4115 4201 4214 4514)
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <prsht.h>
#include <cpl.h>
#include <regstr.h>
#include <setupapi.h>
#include <dbt.h>
#include "eramntui.h"

#define RAMDISK_MEDIA_TYPE		0xf8	/* ERAMNT.H */
#define	SECTOR	(512)					/* ERAMNT.H */
#define PAGE_SIZE_4K	(1024 * 4)		/* the size of one page */
#define	DISKMAXCLUSTER_16 (65525)		/* The max cluster(s) of FAT16 */
#define	DISKMAXCLUSTER_32 (268435455)	/* The max cluster(s) of FAT32 */
#define	DISKMINPAGE		(16)			/* Unable to allocate the data area; 2 is minimal value in Allocation 2 */
#define	MAXALLOCUNIT	(64)			/* allocation unit */
#define	MAXINSTANCE		(9999)			/* The max instance(s) */
#define	LIMIT_4GBPAGES	(0xfffff)		/* 4GB pages */

#define	EXPORT	__declspec(dllexport)

#define SPPSR_ENUM_ADV_DEVICE_PROPERTIES   3

/* The structure for referencing the ERAM registry setting values */
typedef union {
	DWORD	dwOptflag;					// ERAM Control.
	struct {
		BYTE	NonPaged:1;				// bit 0:NonPagedPool use.
		BYTE	External:1;				// bit 1:External memory use.
		BYTE	SkipExternalCheck:1;	// bit 2:Do not check memory when using external memory
		BYTE	Swapable:1;				// bit 3:Treat as local disk
		BYTE	EnableFat32:1;			// bit 4:Allow FAT32 use.
		BYTE	SkipReportUsage:1;		// bit 5:When using external memory don't report=2000:stand-by OK
		BYTE	MakeTempDir:1;			// bit 6:TEMP directory creation.
		BYTE	byResv7:1;				// bit 7:
		BYTE	byResv8:8;				// bit 8:
		BYTE	byResv16:8;				// bit16:
		BYTE	byResv24:7;				// bit24:
		BYTE	UseExtFile:1;			// bit31:External file use.
	} Bits;
 } ERAM_OPTFLAG, *PERAM_OPTFLAG;

typedef struct {
	DWORD			dwSizePage;
	DWORD			dwExtStart;
	ERAM_OPTFLAG	uOption;
	WORD			wRootDir;
	BYTE			byAllocUnit;
	BYTE			byMediaId;
	CHAR			szDefDrv[3];
 } ERAMREGOPT, *PERAMREGOPT, FAR *LPERAMREGOPT;

/* SETUPAPI function type definitions */
typedef HDEVINFO (WINAPI* LPFNSETUPDIGETCLASSDEVSA)(LPGUID, PCSTR, HWND, DWORD);
typedef BOOL (WINAPI* LPFNSETUPDIENUMDEVICEINFO)(HDEVINFO, DWORD, PSP_DEVINFO_DATA);
typedef BOOL (WINAPI* LPFNSETUPDIREMOVEDEVICE)(HDEVINFO, PSP_DEVINFO_DATA);
typedef BOOL (WINAPI* LPFNSETUPDIDESTROYDEVICEINFOLIST)(HDEVINFO);
typedef HKEY (WINAPI* LPFNSETUPDIOPENDEVREGKEY)(HDEVINFO, PSP_DEVINFO_DATA, DWORD, DWORD, DWORD, REGSAM);

/* function pointer(s) in ERAM */
typedef struct {
	HMODULE								hSetupApi;
	LPFNSETUPDIGETCLASSDEVSA			lpfnSetupDiGetClassDevs;
	LPFNSETUPDIENUMDEVICEINFO			lpfnSetupDiEnumDeviceInfo;
	LPFNSETUPDIREMOVEDEVICE				lpfnSetupDiRemoveDevice;
	LPFNSETUPDIDESTROYDEVICEINFOLIST	lpfnSetupDiDestroyDeviceInfoList;
	LPFNSETUPDIOPENDEVREGKEY			lpfnSetupDiOpenDevRegKey;
 } SETUPAPIENTRYS, FAR *LPSETUPAPIENTRYS, *PSETUPAPIENTRYS;

/* global variables */
HINSTANCE hgInstance = NULL;
BOOL bUpdate = FALSE;
BOOL bReboot = FALSE;
HKEY hgKey = NULL;
BOOL bProp = FALSE;
SETUPAPIENTRYS sSetupApi = { NULL, NULL, NULL, NULL, NULL, NULL };

/* string constants */
CHAR szWinName[] = "ERAM for Windows NT/2000/XP";
CHAR szRootDir[] = "RootDirEntries";
CHAR szOption[] = "Option";
CHAR szAllocUnit[] = "AllocUnit";
CHAR szMediaId[] = "MediaId";
CHAR szDefDrv[] = "DriveLetter";
CHAR szPage[] = "Page";
CHAR szExtStart[] = "ExtStart";

/* prototypes */
BOOL WINAPI StatusDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL WINAPI WmInitDialog(HWND, HWND, LPARAM);
VOID WINAPI GetRegOption(LPERAMREGOPT);
VOID WINAPI ReadRegValues(HKEY, LPSTR, UINT, LPVOID, ULONG, WORD);
VOID WINAPI SetPageOption(HWND, LPERAMREGOPT);
VOID WINAPI WmCommand(HWND, INT, HWND, UINT);
VOID WINAPI EnableExtGroup(HWND, BOOL);
VOID WINAPI Reboot(HWND);
BOOL WINAPI SettingUpdate(HWND);
BOOL WINAPI GetPageOption(HWND, LPERAMREGOPT);
BOOL WINAPI SetRegOption(LPERAMREGOPT);
LONG WINAPI CplInit(VOID);
VOID WINAPI CplNewInquire(LPNEWCPLINFO);
BOOL WINAPI SystemShutdown(VOID);
DWORD WINAPI WmNotify(HWND, INT, NMHDR FAR*);
VOID WINAPI WmDestroy(HWND);
BOOL WINAPI Eram2000UnInstall(HWND);
BOOL WINAPI GetInfName(HDEVINFO, PSP_DEVINFO_DATA, LPSTR, DWORD);
BOOL WINAPI DeleteInfFiles(LPCSTR);
LPCSTR WINAPI GetEramClass(GUID*);
LPSTR WINAPI GetResStr(WORD, LPSTR, INT);


/* StatusDlgProc
		Setting Dialog.
	Parameters
		hDlg		The window handle of dialog.
		uMsg		The message.
		wParam		Parameters
		lParam		Parameters
	Return Value
		The results.
*/

BOOL WINAPI StatusDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	HANDLE_MSG(hDlg, WM_INITDIALOG	, WmInitDialog);	/* On dialog creation */
	HANDLE_MSG(hDlg, WM_COMMAND		, WmCommand);		/* On control handling */
	HANDLE_MSG(hDlg, WM_NOTIFY		, WmNotify);		/* On notification received */
	HANDLE_MSG(hDlg, WM_DESTROY		, WmDestroy);		/* Close */
	}
	return FALSE;
}


/* WmInitDialog
		Dialog Initialization Processing.
	Parameters
		hDlg		The window handle of dialog.
		hwndFocus	The control handle to be got focus.
		lInitParam	Parameters
	Return Value
		The results.
*/

BOOL WINAPI WmInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lInitParam)
{
	/* Local variable(s) */
	DWORD dwDisp;
	ERAMREGOPT EramOpt;
	CHAR szPath[MAX_PATH], szText[128];
	/* Initialization of flag(s) */
	bUpdate = FALSE;
	bReboot = FALSE;
	/* Hide the needless button if property */
	if (bProp != FALSE)
	{
		ShowWindow(GetDlgItem(hDlg, IDOK), SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDCANCEL), SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_UPDATE), SW_HIDE);
	}
	/* Open the registry key */
	wsprintf(szPath, "%s\\Eram\\Parameters", REGSTR_PATH_SERVICES);
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, szPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hgKey, &dwDisp) != ERROR_SUCCESS)	/* Failure */
	{
		MessageBox(hDlg, GetResStr(IDS_ERR_OPEN_REG, szText, sizeof(szText)), szWinName, MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
		EndDialog(hDlg, 0);
		return FALSE;
	}
	/* Get the registry value */
	GetRegOption(&EramOpt);
	/* Adapt the registry value(s) to the control(s). */
	SetPageOption(hDlg, &EramOpt);
	return FALSE;
}


/* GetRegOption
		Get the registry setting value.
	Parameters
		lpEramOpt	The pointer to ERAM_REGOPT structure.
	Return Value
		None.
*/

VOID WINAPI GetRegOption(LPERAMREGOPT lpEramOpt)
{
	/* Get the number of root directories */
	ReadRegValues(hgKey, szRootDir, REG_DWORD, &(lpEramOpt->wRootDir), sizeof(lpEramOpt->wRootDir), 128);
	/* Get the option(s) */
	ReadRegValues(hgKey, szOption, REG_DWORD, &(lpEramOpt->uOption.dwOptflag), sizeof(lpEramOpt->uOption.dwOptflag), 0);
	/* Get the allocation unit */
	ReadRegValues(hgKey, szAllocUnit, REG_DWORD, &(lpEramOpt->byAllocUnit), sizeof(lpEramOpt->byAllocUnit), 1024 / SECTOR);
	/* Get the media ID */
	ReadRegValues(hgKey, szMediaId, REG_DWORD, &(lpEramOpt->byMediaId), sizeof(lpEramOpt->byMediaId), RAMDISK_MEDIA_TYPE);
	/* Get the drive character */
	ReadRegValues(hgKey, szDefDrv, REG_SZ, lpEramOpt->szDefDrv, sizeof(lpEramOpt->szDefDrv), (WORD)'Z');
	lpEramOpt->szDefDrv[1] = '\0';
	/* Get the number of pages */
	ReadRegValues(hgKey, szPage, REG_DWORD, &(lpEramOpt->dwSizePage), sizeof(lpEramOpt->dwSizePage), DISKMINPAGE);
	/* Get the starting position of external memory */
	ReadRegValues(hgKey, szExtStart, REG_DWORD, &(lpEramOpt->dwExtStart), sizeof(lpEramOpt->dwExtStart), 0);
}


/* ReadRegValues
		Get the registry and set the default values.
	Parameters
		hKey			The key to be accessed.
		lpszValueName	The pointer to the value name string.
		uType			The value type of REG_DWORD or REG_SZ
		lpVal			The pointer to the value to be retrived.
		uSizeOrg		The size of the value to get.
		wDefVal			The default value.
	Return Value
		None.
*/

VOID WINAPI ReadRegValues(HKEY hKey, LPSTR lpszValueName, UINT uType, LPVOID lpVal, ULONG uSizeOrg, WORD wDefVal)
{
	/* Local variable(s) */
	LONG lRet;
	ULONG uSize;
	DWORD dwVal, dwType;
	switch (uType)
	{
	case REG_DWORD:
		uSize = sizeof(dwVal);
		/* Get the registry value */
		lRet = RegQueryValueEx(hKey, lpszValueName, NULL, &dwType, (LPBYTE)(&dwVal), &uSize);
		if ((lRet == ERROR_SUCCESS)&&
			(dwType == uType))		/* Successfully got */
		{
			/* Set the retrieved value */
			switch (uSizeOrg)
			{
			case sizeof(DWORD):
				*((LPDWORD)lpVal) = (DWORD)dwVal;
				return;
			case sizeof(WORD):
				*((LPWORD)lpVal) = (WORD)((dwVal > MAXWORD) ? MAXWORD : (WORD)dwVal);
				return;
			case sizeof(BYTE):
				*((LPBYTE)lpVal) = (BYTE)((dwVal > MAXBYTE) ? MAXBYTE : (BYTE)dwVal);
				return;
			}
		}
		break;
	case REG_SZ:
		uSize = uSizeOrg;
		/* Get the registry value */
		lRet = RegQueryValueEx(hKey, lpszValueName, NULL, &dwType, lpVal, &uSize);
		if ((lRet == ERROR_SUCCESS)&&
			(dwType == uType))		/* Successfully got */
		{
			return;
		}
		break;
	}
	/* Set the default value */
	switch (uSizeOrg)
	{
	case sizeof(DWORD):
		*((LPDWORD)lpVal) = (DWORD)wDefVal;
		break;
	case sizeof(WORD):
		*((LPWORD)lpVal) = wDefVal;
		break;
	case sizeof(BYTE):
		*((LPBYTE)lpVal) = (BYTE)wDefVal;
		break;
	}
}


/* SetPageOption
		Adapt the registry values to the controls.
	Parameters
		hDlg		The window handle of dialog.
		lpEramOpt	The pointer to ERAM_REGOPT structure.
	Return Value
		None.
*/

VOID WINAPI SetPageOption(HWND hDlg, LPERAMREGOPT lpEramOpt)
{
	/* Local variable(s) */
	CHAR szId[3], szDrv[2], szAlloc[3];
	HWND hCtl;
	UINT loopi, uAlloc;
	CHAR cDrv;
	INT nSelect;
	/* Set the number of root directories */
	SetDlgItemInt(hDlg, IDC_EDIT_ROOTDIR, (UINT)(lpEramOpt->wRootDir), FALSE);
	Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT_ROOTDIR), 4);	/* max 9999 */
	/* Set the option(s) */
	if (lpEramOpt->uOption.Bits.NonPaged != 0)
	{
		Button_SetCheck(GetDlgItem(hDlg, IDC_RADIO_NONPAGED), 1);
		EnableExtGroup(hDlg, FALSE);
	}
	else if (lpEramOpt->uOption.Bits.External != 0)
	{
		Button_SetCheck(GetDlgItem(hDlg, IDC_RADIO_EXTERNAL), 1);
		EnableExtGroup(hDlg, TRUE);
	}
	else
	{
		Button_SetCheck(GetDlgItem(hDlg, IDC_RADIO_PAGED), 1);
		EnableExtGroup(hDlg, FALSE);
	}
	if (lpEramOpt->uOption.Bits.SkipExternalCheck == 0)
	{
		Button_SetCheck(GetDlgItem(hDlg, IDC_CHECK_NOTSEARCHSKIP), 1);
	}
	if (lpEramOpt->uOption.Bits.Swapable != 0)
	{
		Button_SetCheck(GetDlgItem(hDlg, IDC_CHECK_LOCAL_DISK), 1);
	}
	if (lpEramOpt->uOption.Bits.SkipReportUsage == 0)
	{
		Button_SetCheck(GetDlgItem(hDlg, IDC_CHECK_REPORTUSAGE), 1);
	}
	if (lpEramOpt->uOption.Bits.MakeTempDir != 0)
	{
		Button_SetCheck(GetDlgItem(hDlg, IDC_CHECK_MAKE_TEMP), 1);
	}
	/* Set the allocation unit */
	hCtl = GetDlgItem(hDlg, IDC_COMBO_ALLOCUNIT);
	nSelect = -1;
	for (loopi=0, uAlloc=1; uAlloc<=MAXALLOCUNIT; loopi++, uAlloc<<=1)
	{
		if (lpEramOpt->byAllocUnit == uAlloc)
		{
			nSelect = loopi;
		}
		wsprintf((LPSTR)szAlloc, "%d", uAlloc);
		ComboBox_AddString(hCtl, (LPSTR)szAlloc);
	}
	if (nSelect < 0)
	{
		nSelect = 1;	/* 2 */
	}
	ComboBox_SetCurSel(hCtl, nSelect);
	/* Set the media ID */
	wsprintf((LPSTR)szId, "%X", (UINT)(lpEramOpt->byMediaId));
	hCtl = GetDlgItem(hDlg, IDC_EDIT_MEDIAID);
	Edit_SetText(hCtl, (LPSTR)szId);
	Edit_LimitText(hCtl, sizeof(szId)-1);
	/* Set the drive character */
	hCtl = GetDlgItem(hDlg, IDC_COMBO_DRIVE);
	nSelect = -1;
	lpEramOpt->szDefDrv[0] &= ~0x20;
	szDrv[1] = '\0';
	for (loopi=0, cDrv='A'; cDrv<='Z'; loopi++, cDrv++)
	{
		if (lpEramOpt->szDefDrv[0] == cDrv)
		{
			nSelect = loopi;
		}
		szDrv[0] = cDrv;
		ComboBox_AddString(hCtl, (LPSTR)szDrv);
	}
	if (nSelect < 0)
	{
		nSelect = loopi - 1;	/* Z */
	}
	ComboBox_SetCurSel(hCtl, nSelect);
	/* Set the page number */
	SetDlgItemInt(hDlg, IDC_EDIT_PAGE, (ULONG)(lpEramOpt->dwSizePage << 2), FALSE);
	Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT_PAGE), 7);		/* 1024MB=1048576KB */
	/* Set the starting positioin of external memory */
	hCtl = GetDlgItem(hDlg, IDC_EDIT_EXTSTART_MB);
	SetDlgItemInt(hDlg, IDC_EDIT_EXTSTART_MB, (ULONG)(lpEramOpt->dwExtStart / 0x100000), FALSE);
	Edit_LimitText(hCtl, 4);		/* 4095MB */
	if ((lpEramOpt->dwExtStart / 0x100000) != 0)		/* With specified */
	{
		Button_SetCheck(GetDlgItem(hDlg, IDC_CHECK_EXTSTART), 1);
	}
	EnableExtGroup(hDlg, (lpEramOpt->uOption.Bits.External != 0) ? TRUE : FALSE);
}


/* WmCommand
		The control processing.
	Parameters
		hDlg		The window handle of dialog.
		wId			The item ID.
		hWndCtl		The control handle.
		wNotifyCode	The notification code.
	Return Value
		None.
*/

VOID WINAPI WmCommand(HWND hDlg, INT wId, HWND hWndCtl, UINT wNotifyCode)
{
	/* Record them if there were changes */
	switch (wId)
	{
	case IDC_COMBO_DRIVE:
	case IDC_COMBO_ALLOCUNIT:
		if (wNotifyCode == CBN_SELCHANGE)
		{
			bUpdate = TRUE;
		}
		break;
	case IDC_EDIT_ROOTDIR:
	case IDC_EDIT_MEDIAID:
	case IDC_EDIT_PAGE:
	case IDC_EDIT_EXTSTART_MB:
		if (wNotifyCode == EN_CHANGE)
		{
			bUpdate = TRUE;
		}
		break;
	case IDC_RADIO_PAGED:
		if (wNotifyCode == BN_CLICKED)
		{
			bUpdate = TRUE;
			EnableExtGroup(hDlg, FALSE);
		}
		break;
	case IDC_RADIO_NONPAGED:
		if (wNotifyCode == BN_CLICKED)
		{
			bUpdate = TRUE;
			EnableExtGroup(hDlg, FALSE);
		}
		break;
	case IDC_RADIO_EXTERNAL:
		if (wNotifyCode == BN_CLICKED)
		{
			bUpdate = TRUE;
			EnableExtGroup(hDlg, TRUE);
		}
		break;
	case IDC_CHECK_LOCAL_DISK:
	case IDC_CHECK_NOTSEARCHSKIP:
	case IDC_CHECK_MAKE_TEMP:
		if (wNotifyCode == BN_CLICKED)
		{
			bUpdate = TRUE;
		}
		break;
	case IDC_UPDATE:
		if (bProp != FALSE)
		{
			break;
		}
		SettingUpdate(hDlg);
		break;
	case IDOK:
		if (bProp != FALSE)
		{
			break;
		}
		if (SettingUpdate(hDlg) == FALSE)
		{
			break;
		}
	case IDCANCEL:
		if (bProp != FALSE)
		{
			break;
		}
		EndDialog(hDlg, 0);
		if (hgKey != NULL)
		{
			RegCloseKey(hgKey);
			hgKey = NULL;
		}
		/* Reboot */
		Reboot(hDlg);
		break;
	case IDC_CHECK_EXTSTART:
		if (wNotifyCode == BN_CLICKED)
		{
			bUpdate = TRUE;
		}
		Edit_Enable(GetDlgItem(hDlg, IDC_EDIT_EXTSTART_MB), (Button_GetCheck(hWndCtl) != 0) ? TRUE : FALSE);
		break;
	}
}


/* EnableExtGroup
		Enable/Disable OS Management Outside Memory.
	Parameters
		hDlg		The window handle of dialog.
		bEnable		To be enabled or not to be.
	Return Value
		None.
*/

VOID WINAPI EnableExtGroup(HWND hDlg, BOOL bEnable)
{
	/* Local variable(s) */
	HWND hCtl;
	Static_Enable(GetDlgItem(hDlg, IDC_STATIC_EXT), bEnable);
	Button_Enable(GetDlgItem(hDlg, IDC_CHECK_NOTSEARCHSKIP), bEnable);
	Button_Enable(GetDlgItem(hDlg, IDC_CHECK_REPORTUSAGE), bEnable);
	hCtl = GetDlgItem(hDlg, IDC_CHECK_EXTSTART);
	Button_Enable(hCtl, bEnable);
	if ((bEnable != FALSE)&&				/* Enabling now */
		(Button_GetCheck(hCtl) == 0))		/* No specifying */
	{
		bEnable = FALSE;
	}
	Edit_Enable(GetDlgItem(hDlg, IDC_EDIT_EXTSTART_MB), bEnable);
	Static_Enable(GetDlgItem(hDlg, IDC_STATIC_EXTSTART_MB), bEnable);
}


/* Reboot
		The reboot processing.
	Parameters
		hDlg		The window handle of dialog.
	Return Value
		None.
*/

VOID WINAPI Reboot(HWND hDlg)
{
	/* Local variable(s) */
	CHAR szText[128];
	if (bReboot == FALSE)
	{
		return;
	}
	if (MessageBox(hDlg, GetResStr(IDS_PROMPT_REBOOT, szText, sizeof(szText)), szWinName, MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND) != IDYES)
	{
		return;
	}
	if (SystemShutdown() == FALSE)
	{
		MessageBox(hDlg, GetResStr(IDS_ERR_REBOOT, szText, sizeof(szText)), szWinName, MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
	}
	bReboot = FALSE;
}


/* SettingUpdate
		The setting update processing.
	Parameters
		hDlg		The window handle of dialog.
		wId			The item ID.
		hWndCtl		The control handle.
		wNotifyCode	The notification code.
	Return Value
		The results.
*/

BOOL WINAPI SettingUpdate(HWND hDlg)
{
	/* Local variable(s) */
	ERAMREGOPT EramOpt;
	CHAR szText[128];
	if (bUpdate == FALSE)		/* No update */
	{
		return TRUE;
	}
	/* Get the page info */
	if (GetPageOption(hDlg, &EramOpt) == FALSE)
	{
		return FALSE;
	}
	/* Update the setting info */
	if (SetRegOption(&EramOpt) == FALSE)
	{
		MessageBox(hDlg, GetResStr(IDS_ERR_REG_MODIFY, szText, sizeof(szText)), szWinName, MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
		return FALSE;
	}
	/* Set them to reboot. */
	bReboot = TRUE;
	bUpdate = FALSE;
	return TRUE;
}


/* GetPageOption
		Get the page option.
	Parameters
		hDlg		The window handle of dialog.
		lpEramOpt	The pointer to ERAM_REGOPT structure.
	Return Value
		The results.
*/

BOOL WINAPI GetPageOption(HWND hDlg, LPERAMREGOPT lpEramOpt)
{
	/* Local variable(s) */
	CHAR szId[3], szRoot[4], szMsg[128], szVolLabel[16], szText[128];
	PSTR pEnd;
	ULONGLONG ulPageT;
	OSVERSIONINFO Ver;
	/* Get the number of root directories */
	lpEramOpt->wRootDir = (WORD)GetDlgItemInt(hDlg, IDC_EDIT_ROOTDIR, NULL, FALSE);
	/* Get the option(s) */
	lpEramOpt->uOption.dwOptflag = 0;
	if (Button_GetCheck(GetDlgItem(hDlg, IDC_RADIO_NONPAGED)) != 0)
	{
		lpEramOpt->uOption.Bits.NonPaged = 1;
	}
	else if (Button_GetCheck(GetDlgItem(hDlg, IDC_RADIO_EXTERNAL)) != 0)
	{
		lpEramOpt->uOption.Bits.External = 1;
	}
	if (Button_GetCheck(GetDlgItem(hDlg, IDC_CHECK_NOTSEARCHSKIP)) == 0)
	{
		lpEramOpt->uOption.Bits.SkipExternalCheck = 1;
	}
	if (Button_GetCheck(GetDlgItem(hDlg, IDC_CHECK_LOCAL_DISK)) != 0)
	{
		lpEramOpt->uOption.Bits.Swapable = 1;
	}
	if (Button_GetCheck(GetDlgItem(hDlg, IDC_CHECK_REPORTUSAGE)) == 0)
	{
		lpEramOpt->uOption.Bits.SkipReportUsage = 1;
	}
	if (Button_GetCheck(GetDlgItem(hDlg, IDC_CHECK_MAKE_TEMP)) != 0)
	{
		lpEramOpt->uOption.Bits.MakeTempDir = 1;
	}
	Ver.dwOSVersionInfoSize = sizeof(Ver);
	if ((GetVersionEx(&Ver) != FALSE)&&
		(Ver.dwPlatformId == VER_PLATFORM_WIN32_NT)&&
		(Ver.dwMajorVersion >= 5))		/* Win2000+ */
	{
		lpEramOpt->uOption.Bits.EnableFat32 = 1;
	}
	/* Get the allocation unit */
	lpEramOpt->byAllocUnit = (BYTE)GetDlgItemInt(hDlg, IDC_COMBO_ALLOCUNIT, NULL, FALSE);
	/* Get the media ID */
	Edit_GetText(GetDlgItem(hDlg, IDC_EDIT_MEDIAID), (LPSTR)szId, sizeof(szId));
	lpEramOpt->byMediaId = (BYTE)strtoul(szId, &pEnd, 16);
	/* Get the drive character */
	ComboBox_GetText(GetDlgItem(hDlg, IDC_COMBO_DRIVE), (LPSTR)(lpEramOpt->szDefDrv), sizeof(lpEramOpt->szDefDrv));
	if (lpEramOpt->szDefDrv[0] != '\0')
	{
		if ((GetLogicalDrives() & (1 << (lpEramOpt->szDefDrv[0] - 'A'))) != 0)	/* Now using */
		{
			wsprintf(szRoot, "%c:\\", lpEramOpt->szDefDrv[0]);
			switch (GetDriveType(szRoot))
			{
			case DRIVE_REMOTE:		/* Remote */
			case DRIVE_RAMDISK:		/* Likely it's self */
				break;
			default:				/* In use by HDD */
				if ((GetVolumeInformation(szRoot, szVolLabel, sizeof(szVolLabel), NULL, NULL, NULL, NULL, 0) == FALSE)||
					(lstrcmpi(szVolLabel, "ERAM-DRIVE") != 0))
				{
					wsprintf(szMsg, GetResStr(IDS_WARN_DRV_USING, szText, sizeof(szText)), lpEramOpt->szDefDrv[0]);
					if (MessageBox(hDlg, szMsg, szWinName, MB_OKCANCEL | MB_ICONQUESTION | MB_SETFOREGROUND) != IDOK)
					{
						return FALSE;
					}
				}
				break;
			}
		}
	}
	/* Get the page number */
	lpEramOpt->dwSizePage = (GetDlgItemInt(hDlg, IDC_EDIT_PAGE, NULL, FALSE) + 3) >> 2;
	if (lpEramOpt->uOption.Bits.EnableFat32 == 0)		/* Prohibit FAT32 */
	{
		ulPageT = ((ULONGLONG)DISKMAXCLUSTER_16 * SECTOR * lpEramOpt->byAllocUnit) / PAGE_SIZE_4K;
	}
	else		/* FAT32 */
	{
		ulPageT = ((ULONGLONG)DISKMAXCLUSTER_32 * SECTOR * lpEramOpt->byAllocUnit) / PAGE_SIZE_4K;
	}
	if ((ULONGLONG)(lpEramOpt->dwSizePage) > ulPageT)		/* exceed allocation unit restriction */
	{
		lpEramOpt->dwSizePage = (DWORD)ulPageT;
		wsprintf(szMsg, GetResStr((WORD)((lpEramOpt->byAllocUnit == MAXALLOCUNIT) ? IDS_WARN_LIMIT_SIZE : IDS_WARN_LIMIT_SIZE_THIS_UNIT), szText, sizeof(szText)), ((DWORD)ulPageT << 2));
		MessageBox(hDlg, szMsg, szWinName, MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
		SetDlgItemInt(hDlg, IDC_EDIT_PAGE, ((DWORD)ulPageT << 2), FALSE);
		return FALSE;
	}
	/* 4GB check */
	if (lpEramOpt->dwSizePage > LIMIT_4GBPAGES)
	{
		lpEramOpt->dwSizePage = LIMIT_4GBPAGES;
		wsprintf(szMsg, GetResStr(IDS_WARN_LIMIT_MAX_SIZE, szText, sizeof(szText)), (LIMIT_4GBPAGES << 2));
		MessageBox(hDlg, szMsg, szWinName, MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
		SetDlgItemInt(hDlg, IDC_EDIT_PAGE, (LIMIT_4GBPAGES << 2), FALSE);
		return FALSE;
	}
	/* Test the entry number of root directories */
	if (lpEramOpt->dwSizePage <= (DWORD)(((lpEramOpt->wRootDir * 32) + (PAGE_SIZE_4K - 1)) / PAGE_SIZE_4K))
	{
		MessageBox(hDlg, GetResStr(IDS_WARN_LIMIT_ROOTDIR, szText, sizeof(szText)), szWinName, MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
		SetDlgItemInt(hDlg, IDC_EDIT_ROOTDIR, 128, FALSE);
		return FALSE;
	}
	/* Get the starting position of external memory */
	lpEramOpt->dwExtStart = 0;
	if (Button_GetCheck(GetDlgItem(hDlg, IDC_CHECK_EXTSTART)) != 0)
	{
		lpEramOpt->dwExtStart = GetDlgItemInt(hDlg, IDC_EDIT_EXTSTART_MB, NULL, FALSE) * 0x100000;
	}
	return TRUE;
}


/* SetRegOption
		Update the setting info.
	Parameters
		lpEramOpt	The pointer to ERAM_REGOPT structure.
	Return Value
		None.
*/

BOOL WINAPI SetRegOption(LPERAMREGOPT lpEramOpt)
{
	/* Local variable(s) */
	DWORD dwVal;
	/* Set the number of root directories */
	dwVal = (DWORD)lpEramOpt->wRootDir;
	if (RegSetValueEx(hgKey, szRootDir, 0, REG_DWORD, (LPBYTE)(&dwVal), sizeof(dwVal)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	/* Set the option(s) */
	dwVal = (DWORD)lpEramOpt->uOption.dwOptflag;
	if (RegSetValueEx(hgKey, szOption, 0, REG_DWORD, (LPBYTE)(&dwVal), sizeof(dwVal)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	/* Set the allocation unit */
	dwVal = (DWORD)lpEramOpt->byAllocUnit;
	if (RegSetValueEx(hgKey, szAllocUnit, 0, REG_DWORD, (LPBYTE)(&dwVal), sizeof(dwVal)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	/* Set the media ID */
	dwVal = (DWORD)lpEramOpt->byMediaId;
	if (RegSetValueEx(hgKey, szMediaId, 0, REG_DWORD, (LPBYTE)(&dwVal), sizeof(dwVal)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	/* Set the drive character */
	lpEramOpt->szDefDrv[1] = ':';
	lpEramOpt->szDefDrv[2] = '\0';
	if (RegSetValueEx(hgKey, szDefDrv, 0, REG_SZ, (LPBYTE)(lpEramOpt->szDefDrv), sizeof(lpEramOpt->szDefDrv)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	/* Set the page number */
	if (RegSetValueEx(hgKey, szPage, 0, REG_DWORD, (LPBYTE)(&(lpEramOpt->dwSizePage)), sizeof(lpEramOpt->dwSizePage)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	/* Set the starting positioin of external memory */
	if (RegSetValueEx(hgKey, szExtStart, 0, REG_DWORD, (LPBYTE)(&(lpEramOpt->dwExtStart)), sizeof(lpEramOpt->dwExtStart)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	return TRUE;
}


/* CPlApplet
		Control Panel Applet.
	Parameters
		hwndCPL		The parent window handle of Control Panel.
		uMsg		The notification message.
		lParam1		Parameter 1.
		lParam2		Parameter 2.
	Return Value
		The results.
*/

LONG __declspec(dllexport) CALLBACK CPlApplet(HWND hwndCPL, UINT uMsg, LPARAM lParam1, LPARAM lParam2)
{
	switch (uMsg)
	{
	case CPL_INIT:			/* Initialization message */
		return CplInit();
	case CPL_GETCOUNT:		/* Request the number of the dialog boxes */
		return 1;
	case CPL_NEWINQUIRE:	/* Request the number of dialog boxes infos */
		CplNewInquire((LPNEWCPLINFO)lParam2);
		break;
	case CPL_SELECT:
		break;
	case CPL_DBLCLK:	/* Double-clicked */
		/* Display the dialog */
		DialogBox(hgInstance, MAKEINTRESOURCE(IDD_SETUP), hwndCPL, StatusDlgProc);
		break;
	case CPL_STOP:
		break;
	case CPL_EXIT:
		break;
	default:
		break;
	}
	return 0;
}


/*	CplInit
		Control Panel Applet:Initialization.
	Parameters
		None.
	Return Value
		The results.
*/

LONG WINAPI CplInit(VOID)
{
	/* Local variable(s) */
	OSVERSIONINFO Ver;
	Ver.dwOSVersionInfoSize = sizeof(Ver);
	if (GetVersionEx(&Ver) == FALSE)
	{
		return FALSE;
	}
	/* Don't load if not NT */
	if (Ver.dwPlatformId != VER_PLATFORM_WIN32_NT)
	{
		return FALSE;
	}
	return TRUE;
}


/* Request the info of dialog box(es) */
/*	CplNewInquire
		Control Panel Applet: Request the info of dialog box(es).
	Parameters
		lpNewCPlInfo	The pointer to the info structure.
	Return Value
		None.
*/

VOID WINAPI CplNewInquire(LPNEWCPLINFO lpNewCPlInfo)
{
	/* Return the info */
	ZeroMemory(lpNewCPlInfo, sizeof(*lpNewCPlInfo));
	lpNewCPlInfo->dwSize = sizeof(*lpNewCPlInfo);
	lpNewCPlInfo->hIcon = LoadIcon(hgInstance, MAKEINTRESOURCE(IDI_ICON));
	lstrcpy(lpNewCPlInfo->szName, "ERAM");
	GetResStr(IDS_INFO_DESC, lpNewCPlInfo->szInfo, sizeof(lpNewCPlInfo->szInfo));
}


/*	SystemShutdown
		Reboot Instruction.
	Parameters
		None.
	Return Value
		The results.
*/

BOOL WINAPI SystemShutdown(VOID)
{
	/* The definition(s) of local variable(s) */
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	/* Enable shutdown privilege */
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) == FALSE)
	{
		return FALSE;
	}
	/* Get LUID */
	LookupPrivilegeValue("", SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	/* Enable the shutdown privilege(s) */
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
	if (GetLastError() != ERROR_SUCCESS)		/* Failed to enable */
	{
		CloseHandle(hToken);
		return FALSE;
	}
	/* Shutdown instruction */
	if (ExitWindowsEx(EWX_REBOOT, 0) == FALSE)
	{
		CloseHandle(hToken);
		return FALSE;
	}
	/* shutdown privilege(s) */
	tkp.Privileges[0].Attributes = 0;
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
	CloseHandle(hToken);
	return TRUE;
}


/* WmNotify
		The notification message processing.
	Parameters
		hDlg		The window handle of dialog.
		idFrom		The item ID.
		pnmhdr		The notification info.
	Return Value
		The results.
*/

DWORD WINAPI WmNotify(HWND hDlg, INT idFrom, NMHDR FAR* pnmhdr)
{
	if (pnmhdr->code == PSN_APPLY)	/* Apply */
	{
		/* Update */
		if (SettingUpdate(hDlg) == FALSE)
		{
			SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
			return TRUE;
		}
		/* Reboot if necessary */
		Reboot(hDlg);
		SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
		return TRUE;
	}
	return FORWARD_WM_NOTIFY(hDlg, idFrom, pnmhdr, DefWindowProc);
}


/* WmDestroy
		Destruction Message Processing.
	Parameters
		hDlg		The window handle of dialog.
	Return Value
		None.
*/

VOID WINAPI WmDestroy(HWND hDlg)
{
	if (hgKey != NULL)
	{
		RegCloseKey(hgKey);
		hgKey = NULL;
	}
	FORWARD_WM_DESTROY(hDlg, DefWindowProc);
}


/* EramClassInstall
		The class installer.
	Parameters
		diFctn			The function number.
		hDevInfoSet		The handle of the device info.
		pDevInfoData	The pointer to the device info.
	Return Value
		The results.
*/

DWORD EXPORT WINAPI EramClassInstall(DI_FUNCTION diFctn, HDEVINFO hDevInfoSet, PSP_DEVINFO_DATA pDevInfoData)
{
	/* Do default */
	return ERROR_DI_DO_DEFAULT;
}


/* EnumPropPages32
		Property Page Provider.
	Parameters
		lplDi		The pointer to the device info.
		lpfnAddPage	The page addition function pointer.
		lParam		The parameter.
	Return Value
		The results.
*/

BOOL EXPORT WINAPI EnumPropPages32(PSP_PROPSHEETPAGE_REQUEST pInfo, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
	/* Local variable(s) */
	PROPSHEETPAGE Setting;
	HPROPSHEETPAGE hSetting;
	CHAR szText[64];
	if (pInfo->PageRequested == SPPSR_ENUM_ADV_DEVICE_PROPERTIES)
	{
		bProp = TRUE;
		/* Prepare the page */
		ZeroMemory(&Setting, sizeof(Setting));
		Setting.dwSize = sizeof(Setting);
		Setting.dwFlags = PSP_USETITLE;
		Setting.hInstance = hgInstance;
		Setting.pszTemplate = MAKEINTRESOURCE(IDD_SETUP);
		Setting.pszTitle = GetResStr(IDS_TAB_TITLE, szText, sizeof(szText));
		Setting.pfnDlgProc = (DLGPROC)StatusDlgProc;
		/* Create the page */
		hSetting = CreatePropertySheetPage(&Setting);
		if (hSetting == NULL)		/* Failed to create */
		{
			/* Return success */
			return TRUE;
		}
		/* Add the page */
		if ((*lpfnAddPage)(hSetting, lParam) == FALSE)	/* Failed to add */
		{
			DestroyPropertySheetPage(hSetting);
		}
	}
	return TRUE;
}


/* EramUninstall
		Uninstaller
	Parameters
		hWnd		The parent window handle.
		hInstance	The instance handle.
		lpszCmdLine	The parameters
		nCmdShow	The display status.
	Return Value
		None.
*/

VOID EXPORT CALLBACK EramUninstall(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, INT nCmdShow)
{
	/* Local variable(s) */
	SC_HANDLE hScm, hEram;
	CHAR szMsg[256], szSysDir[MAX_PATH], szFile[MAX_PATH], szText[128];
	DWORD dwError;
	OSVERSIONINFO Ver;
	if (MessageBox(hWnd, GetResStr(IDS_PROMPT_ERAM_REMOVE, szText, sizeof(szText)), szWinName, MB_OKCANCEL) != IDOK)
	{
		return;
	}
	Ver.dwOSVersionInfoSize = sizeof(Ver);
	if ((GetVersionEx(&Ver) == FALSE)||
		(Ver.dwPlatformId != VER_PLATFORM_WIN32_NT))	/* Not NT */
	{
		MessageBox(hWnd, GetResStr(IDS_ERR_DETECT_OS, szText, sizeof(szText)), szWinName, MB_OK);
		return;
	}
	if (Ver.dwMajorVersion >= 5)		/* Windows2000 */
	{
		/* Delete from device manager? */
		if (Eram2000UnInstall(hWnd) == FALSE)
		{
			MessageBox(hWnd, GetResStr(IDS_PROMPT_BEFORE_DEVMGR, szText, sizeof(szText)), szWinName, MB_OK);
			return;
		}
	}
	/* Connect SCM */
	hScm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hScm == NULL)
	{
		dwError = GetLastError();
		if (dwError == ERROR_ACCESS_DENIED)
		{
			MessageBox(hWnd, GetResStr(IDS_ERR_NEED_REMOVE_PRIV, szText, sizeof(szText)), szWinName, MB_OK);
			return;
		}
		wsprintf(szMsg, GetResStr(IDS_ERR_OPEN_SCM, szText, sizeof(szText)), dwError);
		MessageBox(hWnd, szMsg, szWinName, MB_OK);
		return;
	}
	/* ERAM device open */
	hEram = OpenService(hScm, "Eram", SERVICE_CHANGE_CONFIG | DELETE);
	if (hEram == NULL)
	{
		dwError = GetLastError();
		if (dwError != ERROR_SERVICE_DOES_NOT_EXIST)		/* Not uninstalled */
		{
			wsprintf(szMsg, GetResStr(IDS_ERR_OPEN_ERAM, szText, sizeof(szText)), dwError);
			MessageBox(hWnd, szMsg, szWinName, MB_OK);
			CloseServiceHandle(hScm);
			return;
		}
	}
	else
	{
		/* Make it not to be started by startup */
		ChangeServiceConfig(hEram, SERVICE_NO_CHANGE, SERVICE_DISABLED, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		/* Delete the ERAM device */
		if (DeleteService(hEram) == FALSE)
		{
			wsprintf(szMsg, GetResStr(IDS_ERR_ERAM_REMOVE, szText, sizeof(szText)), GetLastError());
			MessageBox(hWnd, szMsg, szWinName, MB_OK);
			CloseServiceHandle(hEram);
			return;
		}
		CloseServiceHandle(hEram);
	}
	CloseServiceHandle(hScm);
	if (GetSystemDirectory(szSysDir, sizeof(szSysDir)) == 0)
	{
		wsprintf(szMsg, GetResStr(IDS_ERR_GET_SYSDIR, szText, sizeof(szText)), GetLastError());
		MessageBox(hWnd, szMsg, szWinName, MB_OK);
		return;
	}
	/* Delete the ERAM */
	wsprintf(szFile, "%s\\drivers\\eram.sys", szSysDir);
	if (MoveFileEx(szFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT) == FALSE)
	{
		wsprintf(szMsg, GetResStr(IDS_ERR_REMOVE, szText, sizeof(szText)), szFile, GetLastError());
		MessageBox(hWnd, szMsg, szWinName, MB_OK);
	}
	/* Delete Control Panel */
	wsprintf(szFile, "%s\\eramnt.cpl", szSysDir);
	if (MoveFileEx(szFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT) == FALSE)
	{
		wsprintf(szMsg, GetResStr(IDS_ERR_REMOVE, szText, sizeof(szText)), szFile, GetLastError());
		MessageBox(hWnd, szMsg, szWinName, MB_OK);
	}
	/* Delete the uninstall key */
	RegDeleteKey(HKEY_LOCAL_MACHINE, "SoftWare\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Eram");
	/* Delete the key for event log */
	RegDeleteKey(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\EventLog\\System\\Eram");
	/* Set to be rebooted */
	bReboot = TRUE;
	/* Reboot instruction */
	Reboot(hWnd);
}


/* The followings are for Windows2000 and later only */
/* Eram2000UnInstall
		Uninstaller(Windows2000:Device Manager)
	Parameters
		hWnd		The parent window handle.
	Return Value
		The results.
*/

BOOL WINAPI Eram2000UnInstall(HWND hWnd)
{
	/* Local variable(s) */
	static GUID EramClass = { 0xFB6B01E0, 0x3191, 0x11D4, 0x99, 0x10, 0x00, 0x00, 0x4C, 0x67, 0x20, 0x63 };
	LPCSTR lpszEramClass;
	HDEVINFO hDevInfoSet;
	SP_DEVINFO_DATA DevInfoData;
	CHAR szBuf[128], szSubKey[MAX_PATH], szInfPath[MAX_PATH], szText[128];
	HKEY hKey;
	UINT loopi;
	BOOL bStat;
	/* Load SETUPAPI.DLL */
	sSetupApi.hSetupApi = (HMODULE)LoadLibrary("setupapi");
	if (sSetupApi.hSetupApi == NULL)		/* Failed to load SETUPAPI.DLL */
	{
		wsprintf(szBuf, GetResStr(IDS_ERR_LOAD_SETUPAPI, szText, sizeof(szText)), GetLastError());
		MessageBox(hWnd, szBuf, szWinName, MB_OK);
		return FALSE;
	}
	sSetupApi.lpfnSetupDiGetClassDevs = (LPFNSETUPDIGETCLASSDEVSA)GetProcAddress(sSetupApi.hSetupApi, "SetupDiGetClassDevsA");
	sSetupApi.lpfnSetupDiEnumDeviceInfo = (LPFNSETUPDIENUMDEVICEINFO)GetProcAddress(sSetupApi.hSetupApi, "SetupDiEnumDeviceInfo");
	sSetupApi.lpfnSetupDiRemoveDevice = (LPFNSETUPDIREMOVEDEVICE)GetProcAddress(sSetupApi.hSetupApi, "SetupDiRemoveDevice");
	sSetupApi.lpfnSetupDiDestroyDeviceInfoList = (LPFNSETUPDIDESTROYDEVICEINFOLIST)GetProcAddress(sSetupApi.hSetupApi, "SetupDiDestroyDeviceInfoList");
	sSetupApi.lpfnSetupDiOpenDevRegKey = (LPFNSETUPDIOPENDEVREGKEY)GetProcAddress(sSetupApi.hSetupApi, "SetupDiOpenDevRegKey");
	if ((sSetupApi.lpfnSetupDiGetClassDevs == NULL)||
		(sSetupApi.lpfnSetupDiEnumDeviceInfo == NULL)||
		(sSetupApi.lpfnSetupDiRemoveDevice == NULL)||
		(sSetupApi.lpfnSetupDiDestroyDeviceInfoList == NULL)||
		(sSetupApi.lpfnSetupDiOpenDevRegKey == NULL))
	{
		wsprintf(szBuf, GetResStr(IDS_ERR_GETPROC_SETUPAPI, szText, sizeof(szText)), GetLastError());
		MessageBox(hWnd, szBuf, szWinName, MB_OK);
		return FALSE;
	}
	/* Prepare for ERAM class device enumeration */
	hDevInfoSet = (*(sSetupApi.lpfnSetupDiGetClassDevs))(&EramClass, NULL, hWnd, 0);
	if (hDevInfoSet == INVALID_HANDLE_VALUE)		/* Without ERAM classes */
	{
		return TRUE;
	}
	bStat = FALSE;
	__try
	{
		/* Set the ANSI structure */
		DevInfoData.cbSize = sizeof(DevInfoData);
		for (loopi=0; loopi<MAXINSTANCE; loopi++)
		{
			/* Enumerate the ERAM class devices */
			if ((*(sSetupApi.lpfnSetupDiEnumDeviceInfo))(hDevInfoSet, loopi, &DevInfoData) == FALSE)
			{
				if (GetLastError() == ERROR_NO_MORE_ITEMS)	/* Enumeration done */
				{
					break;
				}
				wsprintf(szBuf, GetResStr(IDS_ERR_DRIVER_ENUM, szText, sizeof(szText)), GetLastError());
				MessageBox(hWnd, szBuf, szWinName, MB_OK);
				__leave;
			}
			/* Get the corresponding INF file */
			if (GetInfName(hDevInfoSet, &DevInfoData, szInfPath, sizeof(szInfPath)) == FALSE)
			{
				szInfPath[0] = '\0';
			}
			/* Delete one driver entry */
			if ((*(sSetupApi.lpfnSetupDiRemoveDevice))(hDevInfoSet, &DevInfoData) == FALSE)
			{
				wsprintf(szBuf, GetResStr(IDS_ERR_DRIVER_REMOVE, szText, sizeof(szText)), GetLastError());
				MessageBox(hWnd, szBuf, szWinName, MB_OK);
				__leave;
			}
			/* Delete the INF file */
			if (szInfPath[0] != '\0')
			{
				DeleteInfFiles(szInfPath);
			}
		}
		bStat = TRUE;
	}
	__finally
	{
		(*(sSetupApi.lpfnSetupDiDestroyDeviceInfoList))(hDevInfoSet);
	}
	if (bStat != FALSE)		/* Succeeded to delete */
	{
		/* Create the ERAM class string */
		lpszEramClass = GetEramClass(&EramClass);
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszEramClass, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
		{
			/* Delete all the subkeys */
			for (;;)
			{
				if (RegEnumKey(hKey, 0, szSubKey, sizeof(szSubKey)) != ERROR_SUCCESS)
				{
					break;
				}
				RegDeleteKey(hKey, szSubKey);
			}
			RegCloseKey(hKey);
		}
		/* Delete the ERAM class */
		RegDeleteKey(HKEY_LOCAL_MACHINE, lpszEramClass);
	}
	/* Notify change */
	SendMessage(HWND_BROADCAST, WM_DEVICECHANGE, DBT_DEVNODES_CHANGED, 0);
	return bStat;
}


/* GetInfName
		Uninstaller:Get the INF filename.
	Parameters
		hWnd		The parent window handle.
	Return Value
		The results.
*/

BOOL WINAPI GetInfName(HDEVINFO hDevInfoSet, PSP_DEVINFO_DATA pDevInfoData, LPSTR lpszInfPath, DWORD dwSize)
{
	/* Local variable(s) */
	HKEY hKey;
	BOOL bStat;
	LONG lRet;
	DWORD dwType;
	bStat = FALSE;
	/* Open the registry key */
	hKey = (*(sSetupApi.lpfnSetupDiOpenDevRegKey))(hDevInfoSet, pDevInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
	if (hKey != INVALID_HANDLE_VALUE)
	{
		/* Get the INF file name */
		lRet = RegQueryValueEx(hKey, "InfPath", NULL, &dwType, (LPBYTE)lpszInfPath, &dwSize);
        if ((lRet == ERROR_SUCCESS)&&(dwType == REG_SZ))
		{
			bStat = TRUE;
		}
		RegCloseKey(hKey);
	}
	if (bStat == FALSE)
	{
		*lpszInfPath = '\0';
	}
	return bStat;
}


/* DeleteInfFiles
		Uninstaller:INF file deletion.
	Parameters
		lpszInf		The INF filename.
	Return Value
		The results.
*/

BOOL WINAPI DeleteInfFiles(LPCSTR lpszInf)
{
	/* Local variable(s) */
	CHAR szWinDir[MAX_PATH], szBase[_MAX_FNAME], szExt[_MAX_EXT], szPath[MAX_PATH];
	INT nLen;
	/* Get the Windows directory */
	if (GetWindowsDirectory(szWinDir, sizeof(szWinDir)) == 0)
	{
		return FALSE;
	}
	/* Extract filename extension */
	_splitpath(lpszInf, NULL, NULL, szBase, szExt);
	if (lstrcmpi(szExt, ".INF") != 0)
	{
		return FALSE;
	}
	nLen = wsprintf(szPath, "%s\\INF\\%s", szWinDir, szBase);
	if (nLen == 0)
	{
		return FALSE;
	}
	/* Delete the INF file */
	lstrcpy(&(szPath[nLen]), szExt);
	if ((DeleteFile(szPath) == FALSE)||
		(MoveFileEx(szPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT) == FALSE))
	{
		return FALSE;
	}
	/* Delete the PNF file */
	szPath[nLen+1] = 'P';
	if ((DeleteFile(szPath) == FALSE)||
		(MoveFileEx(szPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT) == FALSE))
	{
		return FALSE;
	}
	return TRUE;
}


/* GetEramClass
		Uninstaller:Build the ERAM class string
	Parameters
		pGuid	The class GUID
	Return Value
		Registry key string.
*/

LPCSTR WINAPI GetEramClass(GUID* pGuid)
{
	/* Local variable(s) */
	static CHAR szEramClass[MAX_PATH];
	wsprintf(szEramClass,
			"SYSTEM\\CurrentControlSet\\Control\\Class\\{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
			pGuid->Data1,
			pGuid->Data2,
			pGuid->Data3,
			pGuid->Data4[0], pGuid->Data4[1],
			pGuid->Data4[2], pGuid->Data4[3], pGuid->Data4[4], pGuid->Data4[5], pGuid->Data4[6], pGuid->Data4[7]);
	return (LPCSTR)szEramClass;
}


/* The followings are for WindowsNT4.0 and later only */
/* StartupFastfat
		FASTFAT driver StartUp change RUNDLL32 ERAMNT.CPL,StartupFastfat 0-to-4
	Parameters
		hWnd		The parent window handle.
		hInstance	The instance handle.
		lpszCmdLine	The parameters.
		nCmdShow	The display status.
	Return Value
		None.
*/

VOID EXPORT CALLBACK StartupFastfat(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, INT nCmdShow)
{
	/* Local variable(s) */
	static WORD wTypes[] = {
		IDS_START_TYPE_0, IDS_START_TYPE_1, IDS_START_TYPE_2, IDS_START_TYPE_3, IDS_START_TYPE_4
	};
	SC_HANDLE hScm, hFat;
	CHAR szMsg[256], szText[128], szType[32];
	DWORD dwError, dwStartType, dwNeed;
	BOOL bCmdOk;
	LPQUERY_SERVICE_CONFIG lpConf;
	bCmdOk = FALSE;
	if (lpszCmdLine != NULL)
	{
		switch (*lpszCmdLine)
		{
		case '0':		/* SERVICE_BOOT_START */
		case '1':		/* SERVICE_SYSTEM_START */
		case '2':		/* SERVICE_AUTO_START */
		case '3':		/* SERVICE_DEMAND_START */
		case '4':		/* SERVICE_DISABLED */
			if (lpszCmdLine[1] == '\0')
			{
				bCmdOk = TRUE;
			}
			break;
		}
	}
	if (bCmdOk == FALSE)
	{
		MessageBox(hWnd, GetResStr(IDS_INFO_FASTFAT_USAGE, szText, sizeof(szText)), szWinName, MB_OK);
		return;
	}
	dwStartType = *lpszCmdLine - '0';
	/* Connect SCM */
	hScm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (hScm == NULL)
	{
		dwError = GetLastError();
		if (dwError == ERROR_ACCESS_DENIED)
		{
			MessageBox(hWnd, GetResStr(IDS_ERR_NEED_CTRL_PRIV, szText, sizeof(szText)), szWinName, MB_OK);
			return;
		}
		wsprintf(szMsg, GetResStr(IDS_ERR_OPEN_SCM, szText, sizeof(szText)), dwError);
		MessageBox(hWnd, szMsg, szWinName, MB_OK);
		return;
	}
	hFat = NULL;
	__try
	{
		/* FASTFAT driver open */
		hFat = OpenService(hScm, "Fastfat", SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG);
		if (hFat == NULL)
		{
			wsprintf(szMsg, GetResStr(IDS_ERR_OPEN_FASTFAT, szText, sizeof(szText)), GetLastError());
			MessageBox(hWnd, szMsg, szWinName, MB_OK);
			__leave;
		}
		/* Confirm the startup type */
		QueryServiceConfig(hFat, NULL, 0, &dwNeed);
		if (dwNeed != 0)
		{
			lpConf = (LPQUERY_SERVICE_CONFIG)_alloca(dwNeed);
			if ((lpConf != NULL)&&
				(QueryServiceConfig(hFat, lpConf, dwNeed, &dwNeed) != FALSE)&&
				(lpConf->dwStartType == dwStartType))
			{
				wsprintf(szMsg, GetResStr(IDS_INFO_FASTFAT_ALREADY, szText, sizeof(szText)), GetResStr(wTypes[dwStartType], szType, sizeof(szType)));
				MessageBox(hWnd, szMsg, szWinName, MB_OK);
				__leave;
			}
		}
		/* Change the startup type */
		if (ChangeServiceConfig(hFat, SERVICE_NO_CHANGE, dwStartType, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == FALSE)
		{
			wsprintf(szMsg, GetResStr(IDS_ERR_FASTFAT_MODIFY, szText, sizeof(szText)), GetLastError());
			MessageBox(hWnd, szMsg, szWinName, MB_OK);
			__leave;
		}
		wsprintf(szMsg, GetResStr(IDS_INFO_FASTFAT_MODIFY, szText, sizeof(szText)), GetResStr(wTypes[dwStartType], szType, sizeof(szType)));
		MessageBox(hWnd, szMsg, szWinName, MB_OK);
	}
	__finally
	{
		if (hFat != NULL)
		{
			CloseServiceHandle(hFat);
		}
		CloseServiceHandle(hScm);
	}
}


/* The followings are common. */
/* GetResStr
		Get Resource String.
	Parameters
		wId			String resource ID.
		lpszBuf		The pointer of buffer.
		nSize		The size of buffer.
	Return Value
		The pointer of buffer.
*/

LPSTR WINAPI GetResStr(WORD wId, LPSTR lpszBuf, INT nSize)
{
	/* Local variable(s) */
	DWORD dwError;
	dwError = GetLastError();
	if (LoadString(hgInstance, wId, lpszBuf, nSize) == 0)
	{
		*lpszBuf = '\0';
	}
	SetLastError(dwError);
	return lpszBuf;
}


/* DllMain
		The DLL entry.
	Parameters
		hInstance	The instance handle of this DLL.
		fdwReason	The reason of calling.
		lpvReserved	Reserved.
	Return Value
		The results.
*/

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		hgInstance = hInstance;
		DisableThreadLibraryCalls(hInstance);
		break;
	}
	return TRUE;
}
