/*　ERAMNTUI.C　　RAMディスクERAM for WindowsNT/2000/XP
	コントロールパネルアプレット/クラスインストーラ
　　　Copyright (c) 1999-2004 by *Error15
*/

/*　更新履歴
	v1.00
		新規作成
	v1.01
		over32MB対応(64MBまで)
	v1.10
		Windows2000クラスインストーラ追加
	v1.11
		Windows2000プロパティシートページ追加
		非ページ,ページプールサイズ変更機能追加
	v2.00
		非ページ,ページプールサイズ変更機能廃止
		OS管理外メモリ設定機能追加
		アンインストール機能追加
	v2.01
		アロケーションユニット32を増やして1GB確保可能…
		アンインストール機能のWindows2000対応強化に伴いNT3.51対応廃止
	v2.02
		ドライブ文字が使用中の場合は警告を出すようにした
		ページ数が6桁しか入力できなかったのを修正
		MAXMEM=nn以降のメモリの開始アドレスを書き込み可能にした
		削除後リブートダイアログが表示されていなかったのを修正
		NTで削除直後にRAMディスクが作成されていたのを修正
		メモリ検査スキップするかどうか保存可能にした
		スワップファイルを扱えるようにするかどうか選択可能にした
		NT3.51対応復活
		設定変更後レジストリのドライブ文字から : が欠けていたのを修正
		イベントログメッセージ削除対応
	v2.10
		Win2000以降(=FAT32対応)の場合は自動で対応bitを設定
		最大アロケーションユニット64化
	v2.11
		WinXP対応
		Fastfatドライバのスタートアップ変更機能追加
	v2.12
		メモリ上限検出部のUIを逆転
		スタンバイ対策部追加
	v2.20
		'にに'文字列修正
		文字列リソース米国対応
		16bitFATの最大クラスタを65525に制限
		メッセージボックスをリソース対応
		オプションフラグを32bit化
		4GB制限追加
		スワップ関係無く実デバイス設定可能にした
		TEMPディレクトリ作成の編集追加
		ルートディレクトリ制限確認追加
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

#define RAMDISK_MEDIA_TYPE		0xf8	/*　ERAMNT.H　*/
#define	SECTOR	(512)					/*　ERAMNT.H　*/
#define PAGE_SIZE_4K	(1024 * 4)		/*　1ページの大きさ　*/
#define	DISKMAXCLUSTER_16 (65525)		/*　FAT16の最大クラスタ　*/
#define	DISKMAXCLUSTER_32 (268435455)	/*　FAT32の最大クラスタ　*/
#define	DISKMINPAGE		(16)			/*　データ領域が確保できない;アロケーション2では2が最小　*/
#define	MAXALLOCUNIT	(64)			/*　アロケーションユニット　*/
#define	MAXINSTANCE		(9999)			/*　インスタンス　*/
#define	LIMIT_4GBPAGES	(0xfffff)		/*　4GBページ　*/

#define	EXPORT	__declspec(dllexport)

#define SPPSR_ENUM_ADV_DEVICE_PROPERTIES   3

/*　ERAMレジストリ設定値参照用構造体　*/
typedef union {
	DWORD	dwOptflag;					// ERAM制御
	struct {
		BYTE	NonPaged:1;				// bit 0:NonPagedPool使用
		BYTE	External:1;				// bit 1:外部メモリ使用
		BYTE	SkipExternalCheck:1;	// bit 2:外部メモリ使用時メモリ検査しない
		BYTE	Swapable:1;				// bit 3:ローカルディスクとして扱う
		BYTE	EnableFat32:1;			// bit 4:FAT32の使用を許可
		BYTE	SkipReportUsage:1;		// bit 5:外部メモリ使用時Reportしない=2000:スタンバイ可
		BYTE	MakeTempDir:1;			// bit 6:TEMPディレクトリ作成
		BYTE	byResv7:1;				// bit 7:
		BYTE	byResv8:8;				// bit 8:
		BYTE	byResv16:8;				// bit16:
		BYTE	byResv24:7;				// bit24:
		BYTE	UseExtFile:1;			// bit31:外部ファイル使用
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

/*　SETUPAPI関数型定義　*/
typedef HDEVINFO (WINAPI* LPFNSETUPDIGETCLASSDEVSA)(LPGUID, PCSTR, HWND, DWORD);
typedef BOOL (WINAPI* LPFNSETUPDIENUMDEVICEINFO)(HDEVINFO, DWORD, PSP_DEVINFO_DATA);
typedef BOOL (WINAPI* LPFNSETUPDIREMOVEDEVICE)(HDEVINFO, PSP_DEVINFO_DATA);
typedef BOOL (WINAPI* LPFNSETUPDIDESTROYDEVICEINFOLIST)(HDEVINFO);
typedef HKEY (WINAPI* LPFNSETUPDIOPENDEVREGKEY)(HDEVINFO, PSP_DEVINFO_DATA, DWORD, DWORD, DWORD, REGSAM);

/*　ERAM内関数ポインタ　*/
typedef struct {
	HMODULE								hSetupApi;
	LPFNSETUPDIGETCLASSDEVSA			lpfnSetupDiGetClassDevs;
	LPFNSETUPDIENUMDEVICEINFO			lpfnSetupDiEnumDeviceInfo;
	LPFNSETUPDIREMOVEDEVICE				lpfnSetupDiRemoveDevice;
	LPFNSETUPDIDESTROYDEVICEINFOLIST	lpfnSetupDiDestroyDeviceInfoList;
	LPFNSETUPDIOPENDEVREGKEY			lpfnSetupDiOpenDevRegKey;
 } SETUPAPIENTRYS, FAR *LPSETUPAPIENTRYS, *PSETUPAPIENTRYS;

/*　グローバル変数　*/
HINSTANCE hgInstance = NULL;
BOOL bUpdate = FALSE;
BOOL bReboot = FALSE;
HKEY hgKey = NULL;
BOOL bProp = FALSE;
SETUPAPIENTRYS sSetupApi = { NULL, NULL, NULL, NULL, NULL, NULL };

/*　文字列定数　*/
CHAR szWinName[] = "ERAM for Windows NT/2000/XP";
CHAR szRootDir[] = "RootDirEntries";
CHAR szOption[] = "Option";
CHAR szAllocUnit[] = "AllocUnit";
CHAR szMediaId[] = "MediaId";
CHAR szDefDrv[] = "DriveLetter";
CHAR szPage[] = "Page";
CHAR szExtStart[] = "ExtStart";

/*　プロトタイプ　*/
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


/*　StatusDlgProc
		設定ダイアログ
	引数
		hDlg		ダイアログのウィンドウハンドル
		uMsg		メッセージ
		wParam		引数
		lParam		引数
	戻り値
		結果
*/

BOOL WINAPI StatusDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	HANDLE_MSG(hDlg, WM_INITDIALOG	, WmInitDialog);	/*　ダイアログ作成時　*/
	HANDLE_MSG(hDlg, WM_COMMAND		, WmCommand);		/*　コントロール操作時　*/
	HANDLE_MSG(hDlg, WM_NOTIFY		, WmNotify);		/*　通知メッセージ受信時　*/
	HANDLE_MSG(hDlg, WM_DESTROY		, WmDestroy);		/*　閉じる　*/
	}
	return FALSE;
}


/*　WmInitDialog
		ダイアログ初期化時の処理
	引数
		hDlg		ダイアログのウィンドウハンドル
		hwndFocus	フォーカスを受け取るコントロールのハンドル
		lInitParam	引数
	戻り値
		結果
*/

BOOL WINAPI WmInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lInitParam)
{
	/*　ローカル変数　*/
	DWORD dwDisp;
	ERAMREGOPT EramOpt;
	CHAR szPath[MAX_PATH], szText[128];
	/*　フラグ初期化　*/
	bUpdate = FALSE;
	bReboot = FALSE;
	/*　プロパティのときは不要なボタンを隠す　*/
	if (bProp != FALSE)
	{
		ShowWindow(GetDlgItem(hDlg, IDOK), SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDCANCEL), SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_UPDATE), SW_HIDE);
	}
	/*　レジストリキーオープン　*/
	wsprintf(szPath, "%s\\Eram\\Parameters", REGSTR_PATH_SERVICES);
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, szPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hgKey, &dwDisp) != ERROR_SUCCESS)	/*　失敗　*/
	{
		MessageBox(hDlg, GetResStr(IDS_ERR_OPEN_REG, szText, sizeof(szText)), szWinName, MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
		EndDialog(hDlg, 0);
		return FALSE;
	}
	/*　レジストリ値取得　*/
	GetRegOption(&EramOpt);
	/*　レジストリ値をコントロールに反映　*/
	SetPageOption(hDlg, &EramOpt);
	return FALSE;
}


/*　GetRegOption
		レジストリ設定値の取得
	引数
		lpEramOpt	ERAM_REGOPT構造体へのポインタ
	戻り値
		なし
*/

VOID WINAPI GetRegOption(LPERAMREGOPT lpEramOpt)
{
	/*　ルートディレクトリ数取得　*/
	ReadRegValues(hgKey, szRootDir, REG_DWORD, &(lpEramOpt->wRootDir), sizeof(lpEramOpt->wRootDir), 128);
	/*　オプション取得　*/
	ReadRegValues(hgKey, szOption, REG_DWORD, &(lpEramOpt->uOption.dwOptflag), sizeof(lpEramOpt->uOption.dwOptflag), 0);
	/*　アロケーションユニット取得　*/
	ReadRegValues(hgKey, szAllocUnit, REG_DWORD, &(lpEramOpt->byAllocUnit), sizeof(lpEramOpt->byAllocUnit), 1024 / SECTOR);
	/*　メディアID取得　*/
	ReadRegValues(hgKey, szMediaId, REG_DWORD, &(lpEramOpt->byMediaId), sizeof(lpEramOpt->byMediaId), RAMDISK_MEDIA_TYPE);
	/*　ドライブ文字取得　*/
	ReadRegValues(hgKey, szDefDrv, REG_SZ, lpEramOpt->szDefDrv, sizeof(lpEramOpt->szDefDrv), (WORD)'Z');
	lpEramOpt->szDefDrv[1] = '\0';
	/*　ページ数取得　*/
	ReadRegValues(hgKey, szPage, REG_DWORD, &(lpEramOpt->dwSizePage), sizeof(lpEramOpt->dwSizePage), DISKMINPAGE);
	/*　外部メモリ開始位置取得　*/
	ReadRegValues(hgKey, szExtStart, REG_DWORD, &(lpEramOpt->dwExtStart), sizeof(lpEramOpt->dwExtStart), 0);
}


/*　ReadRegValues
		レジストリの取得と既定値設定
	引数
		hKey			アクセスするキー
		lpszValueName	値名文字列へのポインタ
		uType			値タイプ REG_DWORD または REG_SZ
		lpVal			値取得領域へのポインタ
		uSizeOrg		値取得領域のサイズ
		wDefVal			既定値
	戻り値
		なし
*/

VOID WINAPI ReadRegValues(HKEY hKey, LPSTR lpszValueName, UINT uType, LPVOID lpVal, ULONG uSizeOrg, WORD wDefVal)
{
	/*　ローカル変数　*/
	LONG lRet;
	ULONG uSize;
	DWORD dwVal, dwType;
	switch (uType)
	{
	case REG_DWORD:
		uSize = sizeof(dwVal);
		/*　レジストリ値取得　*/
		lRet = RegQueryValueEx(hKey, lpszValueName, NULL, &dwType, (LPBYTE)(&dwVal), &uSize);
		if ((lRet == ERROR_SUCCESS)&&
			(dwType == uType))		/*　取得成功　*/
		{
			/*　取得値を設定　*/
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
		/*　レジストリ値取得　*/
		lRet = RegQueryValueEx(hKey, lpszValueName, NULL, &dwType, lpVal, &uSize);
		if ((lRet == ERROR_SUCCESS)&&
			(dwType == uType))		/*　取得成功　*/
		{
			return;
		}
		break;
	}
	/*　既定値を設定　*/
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


/*　SetPageOption
		レジストリ値をコントロールに反映
	引数
		hDlg		ダイアログのウィンドウハンドル
		lpEramOpt	ERAM_REGOPT構造体へのポインタ
	戻り値
		なし
*/

VOID WINAPI SetPageOption(HWND hDlg, LPERAMREGOPT lpEramOpt)
{
	/*　ローカル変数　*/
	CHAR szId[3], szDrv[2], szAlloc[3];
	HWND hCtl;
	UINT loopi, uAlloc;
	CHAR cDrv;
	INT nSelect;
	/*　ルートディレクトリ数設定　*/
	SetDlgItemInt(hDlg, IDC_EDIT_ROOTDIR, (UINT)(lpEramOpt->wRootDir), FALSE);
	Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT_ROOTDIR), 4);	/*　最大9999　*/
	/*　オプション設定　*/
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
	/*　アロケーションユニット設定　*/
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
		nSelect = 1;	/*　2　*/
	}
	ComboBox_SetCurSel(hCtl, nSelect);
	/*　メディアID設定　*/
	wsprintf((LPSTR)szId, "%X", (UINT)(lpEramOpt->byMediaId));
	hCtl = GetDlgItem(hDlg, IDC_EDIT_MEDIAID);
	Edit_SetText(hCtl, (LPSTR)szId);
	Edit_LimitText(hCtl, sizeof(szId)-1);
	/*　ドライブ文字設定　*/
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
		nSelect = loopi - 1;	/*　Z　*/
	}
	ComboBox_SetCurSel(hCtl, nSelect);
	/*　ページ数設定　*/
	SetDlgItemInt(hDlg, IDC_EDIT_PAGE, (ULONG)(lpEramOpt->dwSizePage << 2), FALSE);
	Edit_LimitText(GetDlgItem(hDlg, IDC_EDIT_PAGE), 7);		/*　1024MB=1048576KB　*/
	/*　外部メモリ開始位置設定　*/
	hCtl = GetDlgItem(hDlg, IDC_EDIT_EXTSTART_MB);
	SetDlgItemInt(hDlg, IDC_EDIT_EXTSTART_MB, (ULONG)(lpEramOpt->dwExtStart / 0x100000), FALSE);
	Edit_LimitText(hCtl, 4);		/*　4095MB　*/
	if ((lpEramOpt->dwExtStart / 0x100000) != 0)		/*　指定あり　*/
	{
		Button_SetCheck(GetDlgItem(hDlg, IDC_CHECK_EXTSTART), 1);
	}
	EnableExtGroup(hDlg, (lpEramOpt->uOption.Bits.External != 0) ? TRUE : FALSE);
}


/*　WmCommand
		コントロール処理
	引数
		hDlg		ダイアログのウィンドウハンドル
		wId			項目ID
		hWndCtl		コントロールのハンドル
		wNotifyCode	通知コード
	戻り値
		なし
*/

VOID WINAPI WmCommand(HWND hDlg, INT wId, HWND hWndCtl, UINT wNotifyCode)
{
	/*　変更がかかったときにそれを記録　*/
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
		/*　リブート　*/
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


/*　EnableExtGroup
		OS管理外メモリの使用可/不可制御
	引数
		hDlg		ダイアログのウィンドウハンドル
		bEnable		設定可能にするかどうか
	戻り値
		なし
*/

VOID WINAPI EnableExtGroup(HWND hDlg, BOOL bEnable)
{
	/*　ローカル変数　*/
	HWND hCtl;
	Static_Enable(GetDlgItem(hDlg, IDC_STATIC_EXT), bEnable);
	Button_Enable(GetDlgItem(hDlg, IDC_CHECK_NOTSEARCHSKIP), bEnable);
	Button_Enable(GetDlgItem(hDlg, IDC_CHECK_REPORTUSAGE), bEnable);
	hCtl = GetDlgItem(hDlg, IDC_CHECK_EXTSTART);
	Button_Enable(hCtl, bEnable);
	if ((bEnable != FALSE)&&				/*　これから有効化　*/
		(Button_GetCheck(hCtl) == 0))		/*　指定なし　*/
	{
		bEnable = FALSE;
	}
	Edit_Enable(GetDlgItem(hDlg, IDC_EDIT_EXTSTART_MB), bEnable);
	Static_Enable(GetDlgItem(hDlg, IDC_STATIC_EXTSTART_MB), bEnable);
}


/*　Reboot
		リブート処理
	引数
		hDlg		ダイアログのウィンドウハンドル
	戻り値
		なし
*/

VOID WINAPI Reboot(HWND hDlg)
{
	/*　ローカル変数　*/
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


/*　SettingUpdate
		設定更新処理
	引数
		hDlg		ダイアログのウィンドウハンドル
		wId			項目ID
		hWndCtl		コントロールのハンドル
		wNotifyCode	通知コード
	戻り値
		結果
*/

BOOL WINAPI SettingUpdate(HWND hDlg)
{
	/*　ローカル変数　*/
	ERAMREGOPT EramOpt;
	CHAR szText[128];
	if (bUpdate == FALSE)		/*　更新無し　*/
	{
		return TRUE;
	}
	/*　画面情報取得　*/
	if (GetPageOption(hDlg, &EramOpt) == FALSE)
	{
		return FALSE;
	}
	/*　設定情報更新　*/
	if (SetRegOption(&EramOpt) == FALSE)
	{
		MessageBox(hDlg, GetResStr(IDS_ERR_REG_MODIFY, szText, sizeof(szText)), szWinName, MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
		return FALSE;
	}
	/*　再起動が必要なことをセット　*/
	bReboot = TRUE;
	bUpdate = FALSE;
	return TRUE;
}


/*　GetPageOption
		画面情報取得
	引数
		hDlg		ダイアログのウィンドウハンドル
		lpEramOpt	ERAM_REGOPT構造体へのポインタ
	戻り値
		結果
*/

BOOL WINAPI GetPageOption(HWND hDlg, LPERAMREGOPT lpEramOpt)
{
	/*　ローカル変数　*/
	CHAR szId[3], szRoot[4], szMsg[128], szVolLabel[16], szText[128];
	PSTR pEnd;
	ULONGLONG ulPageT;
	OSVERSIONINFO Ver;
	/*　ルートディレクトリ数取得　*/
	lpEramOpt->wRootDir = (WORD)GetDlgItemInt(hDlg, IDC_EDIT_ROOTDIR, NULL, FALSE);
	/*　オプション取得　*/
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
		(Ver.dwMajorVersion >= 5))		/*　Win2000以降　*/
	{
		lpEramOpt->uOption.Bits.EnableFat32 = 1;
	}
	/*　アロケーションユニット取得　*/
	lpEramOpt->byAllocUnit = (BYTE)GetDlgItemInt(hDlg, IDC_COMBO_ALLOCUNIT, NULL, FALSE);
	/*　メディアID取得　*/
	Edit_GetText(GetDlgItem(hDlg, IDC_EDIT_MEDIAID), (LPSTR)szId, sizeof(szId));
	lpEramOpt->byMediaId = (BYTE)strtoul(szId, &pEnd, 16);
	/*　ドライブ文字取得　*/
	ComboBox_GetText(GetDlgItem(hDlg, IDC_COMBO_DRIVE), (LPSTR)(lpEramOpt->szDefDrv), sizeof(lpEramOpt->szDefDrv));
	if (lpEramOpt->szDefDrv[0] != '\0')
	{
		if ((GetLogicalDrives() & (1 << (lpEramOpt->szDefDrv[0] - 'A'))) != 0)	/*　現在使用中　*/
		{
			wsprintf(szRoot, "%c:\\", lpEramOpt->szDefDrv[0]);
			switch (GetDriveType(szRoot))
			{
			case DRIVE_REMOTE:		/*　リモート　*/
			case DRIVE_RAMDISK:		/*　おそらく自身　*/
				break;
			default:				/*　HDD等で利用中　*/
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
	/*　ページ数取得　*/
	lpEramOpt->dwSizePage = (GetDlgItemInt(hDlg, IDC_EDIT_PAGE, NULL, FALSE) + 3) >> 2;
	if (lpEramOpt->uOption.Bits.EnableFat32 == 0)		/*　FAT32禁止　*/
	{
		ulPageT = ((ULONGLONG)DISKMAXCLUSTER_16 * SECTOR * lpEramOpt->byAllocUnit) / PAGE_SIZE_4K;
	}
	else		/*　FAT32　*/
	{
		ulPageT = ((ULONGLONG)DISKMAXCLUSTER_32 * SECTOR * lpEramOpt->byAllocUnit) / PAGE_SIZE_4K;
	}
	if ((ULONGLONG)(lpEramOpt->dwSizePage) > ulPageT)		/*　アロケーションユニット制限超過　*/
	{
		lpEramOpt->dwSizePage = (DWORD)ulPageT;
		wsprintf(szMsg, GetResStr((WORD)((lpEramOpt->byAllocUnit == MAXALLOCUNIT) ? IDS_WARN_LIMIT_SIZE : IDS_WARN_LIMIT_SIZE_THIS_UNIT), szText, sizeof(szText)), ((DWORD)ulPageT << 2));
		MessageBox(hDlg, szMsg, szWinName, MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
		SetDlgItemInt(hDlg, IDC_EDIT_PAGE, ((DWORD)ulPageT << 2), FALSE);
		return FALSE;
	}
	/*　4GBチェック　*/
	if (lpEramOpt->dwSizePage > LIMIT_4GBPAGES)
	{
		lpEramOpt->dwSizePage = LIMIT_4GBPAGES;
		wsprintf(szMsg, GetResStr(IDS_WARN_LIMIT_MAX_SIZE, szText, sizeof(szText)), (LIMIT_4GBPAGES << 2));
		MessageBox(hDlg, szMsg, szWinName, MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
		SetDlgItemInt(hDlg, IDC_EDIT_PAGE, (LIMIT_4GBPAGES << 2), FALSE);
		return FALSE;
	}
	/*　ルートディレクトリエントリ数検査　*/
	if (lpEramOpt->dwSizePage <= (DWORD)(((lpEramOpt->wRootDir * 32) + (PAGE_SIZE_4K - 1)) / PAGE_SIZE_4K))
	{
		MessageBox(hDlg, GetResStr(IDS_WARN_LIMIT_ROOTDIR, szText, sizeof(szText)), szWinName, MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
		SetDlgItemInt(hDlg, IDC_EDIT_ROOTDIR, 128, FALSE);
		return FALSE;
	}
	/*　外部メモリ開始位置取得　*/
	lpEramOpt->dwExtStart = 0;
	if (Button_GetCheck(GetDlgItem(hDlg, IDC_CHECK_EXTSTART)) != 0)
	{
		lpEramOpt->dwExtStart = GetDlgItemInt(hDlg, IDC_EDIT_EXTSTART_MB, NULL, FALSE) * 0x100000;
	}
	return TRUE;
}


/*　SetRegOption
		設定情報更新
	引数
		lpEramOpt	ERAM_REGOPT構造体へのポインタ
	戻り値
		なし
*/

BOOL WINAPI SetRegOption(LPERAMREGOPT lpEramOpt)
{
	/*　ローカル変数　*/
	DWORD dwVal;
	/*　ルートディレクトリ数設定　*/
	dwVal = (DWORD)lpEramOpt->wRootDir;
	if (RegSetValueEx(hgKey, szRootDir, 0, REG_DWORD, (LPBYTE)(&dwVal), sizeof(dwVal)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	/*　オプション設定　*/
	dwVal = (DWORD)lpEramOpt->uOption.dwOptflag;
	if (RegSetValueEx(hgKey, szOption, 0, REG_DWORD, (LPBYTE)(&dwVal), sizeof(dwVal)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	/*　アロケーションユニット設定　*/
	dwVal = (DWORD)lpEramOpt->byAllocUnit;
	if (RegSetValueEx(hgKey, szAllocUnit, 0, REG_DWORD, (LPBYTE)(&dwVal), sizeof(dwVal)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	/*　メディアID設定　*/
	dwVal = (DWORD)lpEramOpt->byMediaId;
	if (RegSetValueEx(hgKey, szMediaId, 0, REG_DWORD, (LPBYTE)(&dwVal), sizeof(dwVal)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	/*　ドライブ文字設定　*/
	lpEramOpt->szDefDrv[1] = ':';
	lpEramOpt->szDefDrv[2] = '\0';
	if (RegSetValueEx(hgKey, szDefDrv, 0, REG_SZ, (LPBYTE)(lpEramOpt->szDefDrv), sizeof(lpEramOpt->szDefDrv)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	/*　ページ数設定　*/
	if (RegSetValueEx(hgKey, szPage, 0, REG_DWORD, (LPBYTE)(&(lpEramOpt->dwSizePage)), sizeof(lpEramOpt->dwSizePage)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	/*　外部メモリ開始位置設定　*/
	if (RegSetValueEx(hgKey, szExtStart, 0, REG_DWORD, (LPBYTE)(&(lpEramOpt->dwExtStart)), sizeof(lpEramOpt->dwExtStart)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	return TRUE;
}


/*　CPlApplet
		コントロールパネルアプレット
	引数
		hwndCPL		コントロールパネル親ウィンドウのハンドル
		uMsg		通知メッセージ
		lParam1		パラメータ1
		lParam2		パラメータ2
	戻り値
		結果
*/

LONG __declspec(dllexport) CALLBACK CPlApplet(HWND hwndCPL, UINT uMsg, LPARAM lParam1, LPARAM lParam2)
{
	switch (uMsg)
	{
	case CPL_INIT:			/*　初期化メッセージ　*/
		return CplInit();
	case CPL_GETCOUNT:		/*　ダイアログボックス数取得要求　*/
		return 1;
	case CPL_NEWINQUIRE:	/*　ダイアログボックス情報取得要求　*/
		CplNewInquire((LPNEWCPLINFO)lParam2);
		break;
	case CPL_SELECT:
		break;
	case CPL_DBLCLK:	/*　ダブルクリックされた　*/
		/*　ダイアログ表示　*/
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
		コントロールパネルアプレット:初期化
	引数
		なし
	戻り値
		結果
*/

LONG WINAPI CplInit(VOID)
{
	/*　ローカル変数　*/
	OSVERSIONINFO Ver;
	Ver.dwOSVersionInfoSize = sizeof(Ver);
	if (GetVersionEx(&Ver) == FALSE)
	{
		return FALSE;
	}
	/*　NT以外はロードしない　*/
	if (Ver.dwPlatformId != VER_PLATFORM_WIN32_NT)
	{
		return FALSE;
	}
	return TRUE;
}


/*　ダイアログボックス情報取得要求　*/
/*	CplNewInquire
		コントロールパネルアプレット:ダイアログボックス情報取得要求
	引数
		lpNewCPlInfo	情報構造体へのポインタ
	戻り値
		なし
*/

VOID WINAPI CplNewInquire(LPNEWCPLINFO lpNewCPlInfo)
{
	/*　情報を返す　*/
	ZeroMemory(lpNewCPlInfo, sizeof(*lpNewCPlInfo));
	lpNewCPlInfo->dwSize = sizeof(*lpNewCPlInfo);
	lpNewCPlInfo->hIcon = LoadIcon(hgInstance, MAKEINTRESOURCE(IDI_ICON));
	lstrcpy(lpNewCPlInfo->szName, "ERAM");
	GetResStr(IDS_INFO_DESC, lpNewCPlInfo->szInfo, sizeof(lpNewCPlInfo->szInfo));
}


/*	SystemShutdown
		リブート指示
	引数
		なし
	戻り値
		結果
*/

BOOL WINAPI SystemShutdown(VOID)
{
	/*　ローカル変数の定義　*/
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	/*　シャットダウン特権を有効にする　*/
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) == FALSE)
	{
		return FALSE;
	}
	/*　LUID取得　*/
	LookupPrivilegeValue("", SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	/*　シャットダウン特権有効化　*/
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
	if (GetLastError() != ERROR_SUCCESS)		/*　有効化失敗　*/
	{
		CloseHandle(hToken);
		return FALSE;
	}
	/*　シャットダウン指示　*/
	if (ExitWindowsEx(EWX_REBOOT, 0) == FALSE)
	{
		CloseHandle(hToken);
		return FALSE;
	}
	/*　シャットダウン特権無効化　*/
	tkp.Privileges[0].Attributes = 0;
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
	CloseHandle(hToken);
	return TRUE;
}


/*　WmNotify
		通知メッセージ処理
	引数
		hDlg		ダイアログのウィンドウハンドル
		idFrom		項目ID
		pnmhdr		通知情報
	戻り値
		結果
*/

DWORD WINAPI WmNotify(HWND hDlg, INT idFrom, NMHDR FAR* pnmhdr)
{
	if (pnmhdr->code == PSN_APPLY)	/*　適用　*/
	{
		/*　更新　*/
		if (SettingUpdate(hDlg) == FALSE)
		{
			SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
			return TRUE;
		}
		/*　リブートが必要ならリブートする　*/
		Reboot(hDlg);
		SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
		return TRUE;
	}
	return FORWARD_WM_NOTIFY(hDlg, idFrom, pnmhdr, DefWindowProc);
}


/*　WmDestroy
		破棄メッセージ処理
	引数
		hDlg		ダイアログのウィンドウハンドル
	戻り値
		なし
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


/*　EramClassInstall
		クラスインストーラ
	引数
		diFctn			機能番号
		hDevInfoSet		デバイス情報のハンドル
		pDevInfoData	デバイス情報へのポインタ
	戻り値
		結果
*/

DWORD EXPORT WINAPI EramClassInstall(DI_FUNCTION diFctn, HDEVINFO hDevInfoSet, PSP_DEVINFO_DATA pDevInfoData)
{
	/*　デフォルト動作させる　*/
	return ERROR_DI_DO_DEFAULT;
}


/*　EnumPropPages32
		プロパティページプロバイダ
	引数
		lplDi		デバイス情報へのポインタ
		lpfnAddPage	ページ追加関数ポインタ
		lParam		引数
	戻り値
		結果
*/

BOOL EXPORT WINAPI EnumPropPages32(PSP_PROPSHEETPAGE_REQUEST pInfo, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
	/*　ローカル変数　*/
	PROPSHEETPAGE Setting;
	HPROPSHEETPAGE hSetting;
	CHAR szText[64];
	if (pInfo->PageRequested == SPPSR_ENUM_ADV_DEVICE_PROPERTIES)
	{
		bProp = TRUE;
		/*　ページを用意　*/
		ZeroMemory(&Setting, sizeof(Setting));
		Setting.dwSize = sizeof(Setting);
		Setting.dwFlags = PSP_USETITLE;
		Setting.hInstance = hgInstance;
		Setting.pszTemplate = MAKEINTRESOURCE(IDD_SETUP);
		Setting.pszTitle = GetResStr(IDS_TAB_TITLE, szText, sizeof(szText));
		Setting.pfnDlgProc = (DLGPROC)StatusDlgProc;
		/*　ページを作成　*/
		hSetting = CreatePropertySheetPage(&Setting);
		if (hSetting == NULL)		/*　作成失敗　*/
		{
			/*　成功を返す　*/
			return TRUE;
		}
		/*　ページを追加　*/
		if ((*lpfnAddPage)(hSetting, lParam) == FALSE)	/*　追加失敗　*/
		{
			DestroyPropertySheetPage(hSetting);
		}
	}
	return TRUE;
}


/*　EramUninstall
		アンインストーラ
	引数
		hWnd		親ウィンドウハンドル
		hInstance	インスタンスハンドル
		lpszCmdLine	引数
		nCmdShow	表示状態
	戻り値
		なし
*/

VOID EXPORT CALLBACK EramUninstall(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, INT nCmdShow)
{
	/*　ローカル変数　*/
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
		(Ver.dwPlatformId != VER_PLATFORM_WIN32_NT))	/*　NT以外　*/
	{
		MessageBox(hWnd, GetResStr(IDS_ERR_DETECT_OS, szText, sizeof(szText)), szWinName, MB_OK);
		return;
	}
	if (Ver.dwMajorVersion >= 5)		/*　Windows2000　*/
	{
		/*　デバイスマネージャからの削除相当　*/
		if (Eram2000UnInstall(hWnd) == FALSE)
		{
			MessageBox(hWnd, GetResStr(IDS_PROMPT_BEFORE_DEVMGR, szText, sizeof(szText)), szWinName, MB_OK);
			return;
		}
	}
	/*　SCM接続　*/
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
	/*　ERAMデバイスオープン　*/
	hEram = OpenService(hScm, "Eram", SERVICE_CHANGE_CONFIG | DELETE);
	if (hEram == NULL)
	{
		dwError = GetLastError();
		if (dwError != ERROR_SERVICE_DOES_NOT_EXIST)		/*　アンインストール済み以外　*/
		{
			wsprintf(szMsg, GetResStr(IDS_ERR_OPEN_ERAM, szText, sizeof(szText)), dwError);
			MessageBox(hWnd, szMsg, szWinName, MB_OK);
			CloseServiceHandle(hScm);
			return;
		}
	}
	else
	{
		/*　スタートアップで起動してこないよう変更　*/
		ChangeServiceConfig(hEram, SERVICE_NO_CHANGE, SERVICE_DISABLED, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		/*　ERAMデバイス削除　*/
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
	/*　ERAM削除　*/
	wsprintf(szFile, "%s\\drivers\\eram.sys", szSysDir);
	if (MoveFileEx(szFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT) == FALSE)
	{
		wsprintf(szMsg, GetResStr(IDS_ERR_REMOVE, szText, sizeof(szText)), szFile, GetLastError());
		MessageBox(hWnd, szMsg, szWinName, MB_OK);
	}
	/*　コントロールパネル削除　*/
	wsprintf(szFile, "%s\\eramnt.cpl", szSysDir);
	if (MoveFileEx(szFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT) == FALSE)
	{
		wsprintf(szMsg, GetResStr(IDS_ERR_REMOVE, szText, sizeof(szText)), szFile, GetLastError());
		MessageBox(hWnd, szMsg, szWinName, MB_OK);
	}
	/*　アンインストールキー削除　*/
	RegDeleteKey(HKEY_LOCAL_MACHINE, "SoftWare\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Eram");
	/*　イベントログ用キー削除　*/
	RegDeleteKey(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\EventLog\\System\\Eram");
	/*　再起動が必要なことをセット　*/
	bReboot = TRUE;
	/*　リブート指示　*/
	Reboot(hWnd);
}


/*　以降はWindows2000以降専用　*/
/*　Eram2000UnInstall
		アンインストーラ(Windows2000:デバイスマネージャ)
	引数
		hWnd		親ウィンドウハンドル
	戻り値
		結果
*/

BOOL WINAPI Eram2000UnInstall(HWND hWnd)
{
	/*　ローカル変数　*/
	static GUID EramClass = { 0xFB6B01E0, 0x3191, 0x11D4, 0x99, 0x10, 0x00, 0x00, 0x4C, 0x67, 0x20, 0x63 };
	LPCSTR lpszEramClass;
	HDEVINFO hDevInfoSet;
	SP_DEVINFO_DATA DevInfoData;
	CHAR szBuf[128], szSubKey[MAX_PATH], szInfPath[MAX_PATH], szText[128];
	HKEY hKey;
	UINT loopi;
	BOOL bStat;
	/*　SETUPAPI.DLLロード　*/
	sSetupApi.hSetupApi = (HMODULE)LoadLibrary("setupapi");
	if (sSetupApi.hSetupApi == NULL)		/*　SETUPAPI.DLLロード失敗　*/
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
	/*　ERAMクラスのデバイス列挙を準備　*/
	hDevInfoSet = (*(sSetupApi.lpfnSetupDiGetClassDevs))(&EramClass, NULL, hWnd, 0);
	if (hDevInfoSet == INVALID_HANDLE_VALUE)		/*　ERAMクラス不在　*/
	{
		return TRUE;
	}
	bStat = FALSE;
	__try
	{
		/*　ANSI構造体を設定　*/
		DevInfoData.cbSize = sizeof(DevInfoData);
		for (loopi=0; loopi<MAXINSTANCE; loopi++)
		{
			/*　ERAMクラスのデバイスを列挙　*/
			if ((*(sSetupApi.lpfnSetupDiEnumDeviceInfo))(hDevInfoSet, loopi, &DevInfoData) == FALSE)
			{
				if (GetLastError() == ERROR_NO_MORE_ITEMS)	/*　列挙終了　*/
				{
					break;
				}
				wsprintf(szBuf, GetResStr(IDS_ERR_DRIVER_ENUM, szText, sizeof(szText)), GetLastError());
				MessageBox(hWnd, szBuf, szWinName, MB_OK);
				__leave;
			}
			/*　対応するINFファイルを取得　*/
			if (GetInfName(hDevInfoSet, &DevInfoData, szInfPath, sizeof(szInfPath)) == FALSE)
			{
				szInfPath[0] = '\0';
			}
			/*　ドライバエントリ1つ削除　*/
			if ((*(sSetupApi.lpfnSetupDiRemoveDevice))(hDevInfoSet, &DevInfoData) == FALSE)
			{
				wsprintf(szBuf, GetResStr(IDS_ERR_DRIVER_REMOVE, szText, sizeof(szText)), GetLastError());
				MessageBox(hWnd, szBuf, szWinName, MB_OK);
				__leave;
			}
			/*　INFファイル削除　*/
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
	if (bStat != FALSE)		/*　削除成功　*/
	{
		/*　ERAMクラス文字列作成　*/
		lpszEramClass = GetEramClass(&EramClass);
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszEramClass, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
		{
			/*　全サブキー削除　*/
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
		/*　ERAMクラス削除　*/
		RegDeleteKey(HKEY_LOCAL_MACHINE, lpszEramClass);
	}
	/*　変更通知　*/
	SendMessage(HWND_BROADCAST, WM_DEVICECHANGE, DBT_DEVNODES_CHANGED, 0);
	return bStat;
}


/*　GetInfName
		アンインストーラ:INFファイル名取得
	引数
		hWnd		親ウィンドウハンドル
	戻り値
		結果
*/

BOOL WINAPI GetInfName(HDEVINFO hDevInfoSet, PSP_DEVINFO_DATA pDevInfoData, LPSTR lpszInfPath, DWORD dwSize)
{
	/*　ローカル変数　*/
	HKEY hKey;
	BOOL bStat;
	LONG lRet;
	DWORD dwType;
	bStat = FALSE;
	/*　レジストリキーオープン　*/
	hKey = (*(sSetupApi.lpfnSetupDiOpenDevRegKey))(hDevInfoSet, pDevInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
	if (hKey != INVALID_HANDLE_VALUE)
	{
		/*　INFファイル名取得　*/
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


/*　DeleteInfFiles
		アンインストーラ:INFファイル削除
	引数
		lpszInf		INFファイル名
	戻り値
		結果
*/

BOOL WINAPI DeleteInfFiles(LPCSTR lpszInf)
{
	/*　ローカル変数　*/
	CHAR szWinDir[MAX_PATH], szBase[_MAX_FNAME], szExt[_MAX_EXT], szPath[MAX_PATH];
	INT nLen;
	/*　Windowsディレクトリ取得　*/
	if (GetWindowsDirectory(szWinDir, sizeof(szWinDir)) == 0)
	{
		return FALSE;
	}
	/*　拡張子を分解　*/
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
	/*　INFファイル削除　*/
	lstrcpy(&(szPath[nLen]), szExt);
	if ((DeleteFile(szPath) == FALSE)||
		(MoveFileEx(szPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT) == FALSE))
	{
		return FALSE;
	}
	/*　PNFファイル削除　*/
	szPath[nLen+1] = 'P';
	if ((DeleteFile(szPath) == FALSE)||
		(MoveFileEx(szPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT) == FALSE))
	{
		return FALSE;
	}
	return TRUE;
}


/*　GetEramClass
		アンインストーラ:ERAMクラス文字列合成
	引数
		pGuid	クラスGUID
	戻り値
		レジストリキー文字列
*/

LPCSTR WINAPI GetEramClass(GUID* pGuid)
{
	/*　ローカル変数　*/
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


/*　以降はWindowsNT4.0以降専用　*/
/*　StartupFastfat
		FASTFATドライバ スタートアップ変更　RUNDLL32 ERAMNT.CPL,StartupFastfat 0～4
	引数
		hWnd		親ウィンドウハンドル
		hInstance	インスタンスハンドル
		lpszCmdLine	引数
		nCmdShow	表示状態
	戻り値
		なし
*/

VOID EXPORT CALLBACK StartupFastfat(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, INT nCmdShow)
{
	/*　ローカル変数　*/
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
		case '0':		/*　SERVICE_BOOT_START　*/
		case '1':		/*　SERVICE_SYSTEM_START　*/
		case '2':		/*　SERVICE_AUTO_START　*/
		case '3':		/*　SERVICE_DEMAND_START　*/
		case '4':		/*　SERVICE_DISABLED　*/
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
	/*　SCM接続　*/
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
		/*　FASTFATドライバオープン　*/
		hFat = OpenService(hScm, "Fastfat", SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG);
		if (hFat == NULL)
		{
			wsprintf(szMsg, GetResStr(IDS_ERR_OPEN_FASTFAT, szText, sizeof(szText)), GetLastError());
			MessageBox(hWnd, szMsg, szWinName, MB_OK);
			__leave;
		}
		/*　スタートアップタイプ確認　*/
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
		/*　スタートアップタイプ変更　*/
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


/*　以降は共通　*/
/*　GetResStr
		リソース文字列取得
	引数
		wId			文字列リソースID
		lpszBuf		バッファのポインタ
		nSize		バッファの大きさ
	戻り値
		バッファのポインタ
*/

LPSTR WINAPI GetResStr(WORD wId, LPSTR lpszBuf, INT nSize)
{
	/*　ローカル変数　*/
	DWORD dwError;
	dwError = GetLastError();
	if (LoadString(hgInstance, wId, lpszBuf, nSize) == 0)
	{
		*lpszBuf = '\0';
	}
	SetLastError(dwError);
	return lpszBuf;
}


/*　DllMain
		DLLエントリ
	引数
		hInstance	このDLLのインスタンスハンドル
		fdwReason	呼び出し理由
		lpvReserved	予約
	戻り値
		結果
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

