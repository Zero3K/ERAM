/*　ERAMNT.C　　RAMディスクERAM for WindowsNT/2000/XP
　　　Copyright (c) 1999-2004 by *Error15
*/

/*　更新履歴
	v1.00
		新規作成
	v1.01
		over32MB対応(64MBまで)
		Win2000β3対応
	v1.10
		Win2000動作確認
	v1.11
		メモリ確保失敗時、RAM量の目安を出すようにした
		FAT12とFAT16の境界あたりのクラスタ数計算にミスがあったのを修正
	v2.00
		MAXMEM=nn以降のメモリを64KB単位で切り替えして扱う
	v2.01
		アロケーションユニット32を増やして1GB確保可能…
	v2.02
		MAXMEM=nn以降のメモリの開始アドレスを指定可能にした
		ドライブタイプをローカルディスク扱いにしてスワップ可能にもできる
		イベントログメッセージ追加
	v2.10
		FAT32対応
		FAT16 over32MBでパーテーションを正しく返すように変更
		FAT16でもアロケーションユニット64を増やして2GB確保可能…
		ボリュームシリアル番号に現在日付を設定するよう変更
	v2.11
		WinXP対応
	v2.12
		スタンバイ対応
			IoReportResourceUsage呼ぶとスタンバイできなくなる(後で開放しても)
			IoReportResourceForDetectionでは、over16MBな内部メモリ要求が通らない
			IoReportResourceUsage呼ばなくてもHalTranslateBusAddressできる
			HalTranslateBusAddressしなくても内部メモリは同一アドレスを示す
			スタンバイ(電源ONのまま)…RAMはOS管理外でもok
			休止状態(電源OFF)…RAMはOS管理外では不可(OS管理ならok)
	v2.20
		外部メモリ量の計算が符号付きになってたのを修正(実害無し)
		外部メモリの検出のあたりで4GBラップしないよう計算追加
		オプション文字比較部が正しくUNICODE対応してなかったのを修正
		FAT16最大クラスタを65535→65525に修正
		FAT32最大クラスタを未検査→268435455(=0xFFFFFFF)に修正 (Win2000=4177918)
		最大容量を最大クラスタxアロケーションユニット分に制限(厳密には余裕あるが無視)
		最大容量を4GBに制限
			NT系ではFAT16最大容量は4GBだが、chkdsk等の表示が変
		クラスタ制限で切り上げしてたのをやめ、切り捨てに変更
		FAT32クラスタが65527～だったのを65526～に変更
		メモリの少ない機種でもテスト可能なように改造
			外部メモリ使用設定時、無理矢理確保できるようにした
			(古いハードではアドレスバスが32bitフルデコードされていないことがある)
		メモリマップトファイルを使ってテスト可能なように改造(ファイルシステムドライバのロード前提)
			ファイルI/OではZwXXXでは競合にうまく対応できないようなのでマップトファイル使用
			出力ファイル変更対応
		起動時の領域クリアで管理領域のみを対象にするよう変更
		オプション32bit化
		Windows2000以降で自動でFAT32を有効化するよう変更
		ベリファイ処理を無効化(大きさチェックのみ実装)
		セクタ/トラック情報が誤っていたのを修正
		Windows Server 2003対応
		IOCTL_DISK_GET_LENGTH_INFO処理追加
			XP以降ではディスク容量を計算してくれなくてこの対応必須の模様
		IOCTL_DISK_SET_PARTITION_INFO処理追加
			NTFSへのconvertとかformatができるようなので一応
		FAT12最大クラスタを4087→4086に修正
		ボリュームラベル変更対応
			*?/|.,;:+=[]()&^<>" とコントロールコードは指定不可
		イベントログ出力で、UNICODE→ANSI[→UNICODE]変換してたのをANSI→UNICODEに改善
		スワップ関係無く実デバイス設定可能にした
		複数メモリ選択時の補正機能追加
		FAT12/16:ルートディレクトリセクタが全セクタを上回るとき誤動作してたのを修正
		TEMPディレクトリ作成機能追加
		今回(は)も落ちた機能
			NTFSフォーマット
				いろいろ面倒なのでformatかconvertで逃げてもらう
				起動時にformatする場合、以下の設定で回避できるかもしれない
					Runあたりに
						cmd.exe /c convert drv: /fs:ntfs < %systemroot%\system32\eramconv.txt
					system32\eramconv.txtあたりに
						volume-label
						y
						n
					フォルダが開かれててconvertできないことがある
					HDD(=スワップ使用可能)にすればformatも使用可能
			ボリュームマネージャ連携
				PnPドライバでないと困難っぽいので放置
				マウントポイント(NTFS5)は使えない
				ハードリンクが使えない symlink rktools linkd
			XP:ページファイルを置けない
				マウントマネージャに認識されてない状態では対処不能の模様
				Startを0に、Primary diskにしても駄目
				XP以降ページファイル無しにできるので、それで回避願う
			/BURNMEMORY=n対応
				http://msdn.microsoft.com/library/en-us/ddtools/hh/ddtools/bootini_9omr.asp
				ドライバロード時にメモリ実装量を取得する普遍的な手段が不明
				HKLM\HARDWARE\RESOURCEMAP\System Resources\Physical Memory\はまだできてない
	v2.21暫定版
		ACPI:ACPI Reclaim/NVSメモリ除外
		NTデバイス名をレジストリから取得できるようにした(無理矢理複数ERAMを使う場合向け)
			HKLM\System\CurrentControlSet\Services\ERAM のあたりを ERAM2 とかにして
			値 DevName (REG_SZ)に \Device\ERAM2 とか入れてリブート
			Win2000以降ではデバイスマネージャに表示されないがNT用のINFで入れる
	v2.22暫定版
		ACPI:ACPI Reclaim/NVSメモリ除外でエラー処理追加
		over4GB:/MAXMEM=nと/NOLOWMEM混在時はMAXMEMがover4GBに効くことの対策(天夢 森流彩さん指摘感謝)
			プールの物理アドレスがover4GBのとき、/PAE指定とみなす
			/PAE指定とみなしたとき、/NOLOWMEM指定も探す
			/NOLOWMEM指定時は/MAXMEM=16と同等と見なす
	v2.23暫定版
		over4GB:/MAXMEM=nと/NOLOWMEM混在時はMAXMEMがover4GBに効くことの対策(天夢 森流彩さん指摘感謝)
			/PAE:v2.22で16にしたが、16はチェックではじいてたので17に修正
		未:over4GB:/PAE無くても/NOLOWMEMが効くことの対策(天夢 森流彩さん指摘)
			/PAE指定無くても/NOLOWMEM指定を探せる?
			PagedPoolに使われているので限界値が検出できないようだが…
		未:over4GB:LME状態でのRAMを確保?
		未:SMBIOS:メモリ最大量取得
		未:SUMチェック
*/


#pragma warning(disable : 4100 4115 4201 4214 4514 )
#include <ntddk.h>
#include <ntdddisk.h>
#include <devioctl.h>
#include <ntddstor.h>
#include <ntiologc.h>
#include "eramnt.h"
#include "eramntum.h"
#pragma pack(1)



/*　EramCreateClose
		オープン/クローズ要求エントリ
	引数
		pDevObj	装置オブジェクトへのポインタ
		pIrp	IRPパケットへのポインタ
	戻り値
		結果
*/

NTSTATUS EramCreateClose(
	IN PDEVICE_OBJECT	pDevObj,
	IN PIRP				pIrp
 )
{
	KdPrint(("EramCreateClose start\n"));
	/*　成功をセット　*/
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	KdPrint(("EramCreateClose end\n"));
	return STATUS_SUCCESS;
}


/*　EramDeviceControl
		デバイスコントロール要求エントリ
	引数
		pDevObj	装置オブジェクトへのポインタ
		pIrp	IRPパケットへのポインタ
	戻り値
		結果
*/

NTSTATUS EramDeviceControl(
	IN PDEVICE_OBJECT	pDevObj,
	IN PIRP				pIrp
 )
{
	/*　ローカル変数　*/
	PERAM_EXTENSION		pEramExt;
	PIO_STACK_LOCATION	pIrpSp;
	NTSTATUS			ntStat;
	//KdPrint(("EramDeviceControl start\n"));
	/*　構造体先頭ポインタ取得　*/
	pEramExt = pDevObj->DeviceExtension;
	/*　スタックへのポインタ取得　*/
	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
	/*　失敗をセット　*/
	pIrp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
	pIrp->IoStatus.Information = 0;
	switch (pIrpSp->Parameters.DeviceIoControl.IoControlCode)	/*　要求タイプにより分岐　*/
	{
	case IOCTL_DISK_GET_MEDIA_TYPES:		/*　メディアタイプ取得(配列)　*/
	case IOCTL_DISK_GET_DRIVE_GEOMETRY:		/*　メディアタイプ取得(1)　*/
		/*　ジオメトリ取得処理　*/
		EramDeviceControlGeometry(pEramExt, pIrp, pIrpSp->Parameters.DeviceIoControl.OutputBufferLength);
		break;
	case IOCTL_DISK_GET_PARTITION_INFO:		/*　パーテーション情報取得　*/
		/*　パーテーション情報取得処理　*/
		EramDeviceControlGetPartInfo(pEramExt, pIrp, pIrpSp->Parameters.DeviceIoControl.OutputBufferLength);
		break;
	case IOCTL_DISK_SET_PARTITION_INFO:		/*　パーテーション情報取得　*/
		/*　パーテーション情報設定処理　*/
		EramDeviceControlSetPartInfo(pEramExt, pIrp, pIrpSp->Parameters.DeviceIoControl.InputBufferLength);
		break;
	case IOCTL_DISK_VERIFY:					/*　ベリファイ　*/
		/*　ベリファイ処理　*/
		EramDeviceControlVerify(pEramExt, pIrp, pIrpSp->Parameters.DeviceIoControl.InputBufferLength);
		break;
	case IOCTL_DISK_CHECK_VERIFY:			/*　ディスク検査(Win2000β3以降)　*/
		/*　ベリファイ処理　*/
		EramDeviceControlDiskCheckVerify(pEramExt, pIrp, pIrpSp->Parameters.DeviceIoControl.OutputBufferLength);
		break;
	case IOCTL_DISK_GET_LENGTH_INFO:		/*　ディスクサイズ取得(Win2000～ [WinXP以降format/convert時必須])　*/
		/*　ディスクサイズ取得処理　*/
		EramDeviceControlGetLengthInfo(pEramExt, pIrp, pIrpSp->Parameters.DeviceIoControl.OutputBufferLength);
		break;
	default:								/*　その他　*/
		/*　無視　*/
		KdPrint(("IOCTL 0x%x\n", (UINT)(pIrpSp->Parameters.DeviceIoControl.IoControlCode)));
		break;
	}
	/*　ステータスをセット　*/
	ntStat = pIrp->IoStatus.Status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	//KdPrint(("EramDeviceControl end\n"));
	return ntStat;
}


/*　EramDeviceControlGeometry
		ジオメトリ取得処理
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pIrp		IRPパケットへのポインタ
		uLen		バッファサイズ
	戻り値
		なし
*/

VOID EramDeviceControlGeometry(
	PERAM_EXTENSION	pEramExt,
	IN PIRP			pIrp,
	IN ULONG		uLen
 )
{
	/*　ローカル変数　*/
	PDISK_GEOMETRY pGeom;
	if (uLen < sizeof(*pGeom))		/*　サイズ不足　*/
	{
		pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		KdPrint(("EramDeviceControlGeometry:size too small\n"));
		return;
	}
	/*　ディスクジオメトリ設定　*/
	pGeom = (PDISK_GEOMETRY)(pIrp->AssociatedIrp.SystemBuffer);
	pGeom->MediaType = FixedMedia;		/*　メディアタイプ:固定ディスク　*/
	pGeom->Cylinders.QuadPart = (ULONGLONG)(pEramExt->uAllSector);
	pGeom->TracksPerCylinder = 1;
	pGeom->SectorsPerTrack = 1;			/*　1バンクあたりのセクタ数　*/
	pGeom->BytesPerSector = SECTOR;
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = sizeof(*pGeom);
}


/*　EramDeviceControlGetPartInfo
		パーテーション情報取得処理
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pIrp		IRPパケットへのポインタ
		uLen		バッファサイズ
	戻り値
		なし
*/

VOID EramDeviceControlGetPartInfo(
	PERAM_EXTENSION	pEramExt,
	IN PIRP			pIrp,
	IN ULONG		uLen
 )
{
	/*　ローカル変数　*/
	PPARTITION_INFORMATION pPart;
	if (uLen < sizeof(*pPart))		/*　サイズ不足　*/
	{
		pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		KdPrint(("EramDeviceControlGetPartInfo:size too small\n"));
		return;
	}
	/*　パーテーション情報設定　*/
	pPart = (PPARTITION_INFORMATION)(pIrp->AssociatedIrp.SystemBuffer);
	pPart->PartitionType = pEramExt->FAT_size;
	pPart->BootIndicator = FALSE;			/*　ブート不可　*/
	pPart->RecognizedPartition = TRUE;		/*　パーテーション認識　*/
	pPart->RewritePartition = FALSE;		/*　再書き込み不可パーテーション　*/
	pPart->StartingOffset.QuadPart = (ULONGLONG)(0);	/*　パーテーション開始位置　*/
	pPart->PartitionLength.QuadPart = UInt32x32To64(pEramExt->uAllSector, SECTOR);	/*　長さ　*/
	pPart->HiddenSectors =  pEramExt->bsHiddenSecs;	/*　隠しセクタ数　*/
	pPart->PartitionNumber = 1;				/*　パーテーション数　*/
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = sizeof(PARTITION_INFORMATION);
}


/*　EramDeviceControlSetPartInfo
		パーテーション情報設定処理
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pIrp		IRPパケットへのポインタ
		uLen		バッファサイズ
	戻り値
		なし
*/

VOID EramDeviceControlSetPartInfo(
	PERAM_EXTENSION	pEramExt,
	IN PIRP			pIrp,
	IN ULONG		uLen
 )
{
	/*　ローカル変数　*/
	PSET_PARTITION_INFORMATION pPart;
	if (uLen < sizeof(*pPart))		/*　サイズ不足　*/
	{
		pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		KdPrint(("EramDeviceControlSetPartInfo:size too small\n"));
		return;
	}
	/*　パーテーション情報設定　*/
	pPart = (PSET_PARTITION_INFORMATION)(pIrp->AssociatedIrp.SystemBuffer);
	pEramExt->FAT_size = pPart->PartitionType;
	pIrp->IoStatus.Status = STATUS_SUCCESS;
}


/*　EramDeviceControlVerify
		ベリファイ処理
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pIrp		IRPパケットへのポインタ
		uLen		バッファサイズ
	戻り値
		なし
*/

VOID EramDeviceControlVerify(
	PERAM_EXTENSION		pEramExt,
	IN PIRP				pIrp,
	IN ULONG			uLen
 )
{
	/*　ローカル変数　*/
	PVERIFY_INFORMATION	pVerify;
	//KdPrint(("EramDeviceControlVerify start\n"));
	if (uLen < sizeof(*pVerify))		/*　サイズ不足　*/
	{
		pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		KdPrint(("EramDeviceControlVerify:size too small\n"));
		return;
	}
	/*　ベリファイ情報設定　*/
	pVerify = pIrp->AssociatedIrp.SystemBuffer;
	//KdPrint(("offset 0x%x%08x, len 0x%x\n", pVerify->StartingOffset.HighPart, pVerify->StartingOffset.LowPart, pVerify->Length));
	if ((((ULONGLONG)(pVerify->StartingOffset.QuadPart) + (ULONGLONG)(pVerify->Length)) > UInt32x32To64(pEramExt->uAllSector, SECTOR))||
		((pVerify->StartingOffset.LowPart & (SECTOR-1)) != 0)||
		((pVerify->Length & (SECTOR-1)) != 0))	/*　ディスク容量を超えたor開始位置/長さがセクタサイズの倍数でない　*/
	{
		/*　エラーを返す　*/
		KdPrint(("Invalid I/O parameter\n"));
		pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		return;
	}
	if ((pEramExt->uOptflag.Bits.External != 0)&&	/*　OS管理外メモリ使用　*/
		(pEramExt->uExternalStart != 0)&&			/*　OS管理外メモリ設定　*/
		((pEramExt->uExternalStart + (ULONGLONG)(pVerify->StartingOffset.QuadPart) + (ULONGLONG)(pVerify->Length)) >= 
pEramExt->uExternalEnd))
	{
		//KdPrint(("Invalid I/O address space\n"));
		pIrp->IoStatus.Status = STATUS_DISK_CORRUPT_ERROR;
		return;
	}
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	//KdPrint(("EramDeviceControlVerify end\n"));
}


/*　EramDeviceControlDiskCheckVerify
		ディスク交換確認処理
	引数
		pEdskExt	EDSK_EXTENTION構造体へのポインタ
		pIrp		IRPパケットへのポインタ
		uLen		バッファサイズ
	戻り値
		なし
*/

VOID EramDeviceControlDiskCheckVerify(
	PERAM_EXTENSION	pEramExt,
	IN PIRP			pIrp,
	IN ULONG		uLen
 )
{
	/*　ローカル変数　*/
	PULONG puOpt;
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	if (uLen == 0)		/*　補足情報不要　*/
	{
		return;
	}
	if (uLen < sizeof(*puOpt))		/*　サイズ不足　*/
	{
		pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		KdPrint(("EramDeviceControlDiskCheckVerify:size too small\n"));
		return;
	}
	/*　補足情報設定　*/
	puOpt = (PULONG)(pIrp->AssociatedIrp.SystemBuffer);
	*puOpt = 0;
	pIrp->IoStatus.Information = sizeof(*puOpt);
}


/*　EramDeviceControlGetLengthInfo
		ディスクサイズ取得処理
	引数
		pEdskExt	EDSK_EXTENTION構造体へのポインタ
		pIrp		IRPパケットへのポインタ
		uLen		バッファサイズ
	戻り値
		なし
*/

VOID EramDeviceControlGetLengthInfo(
	PERAM_EXTENSION	pEramExt,
	IN PIRP			pIrp,
	IN ULONG		uLen
 )
{
	/*　ローカル変数　*/
	PGET_LENGTH_INFORMATION pInfo;
	if (uLen < sizeof(*pInfo))		/*　サイズ不足　*/
	{
		pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		KdPrint(("EramDeviceControlGetLengthInfo:size too small\n"));
		return;
	}
	/*　サイズ情報設定　*/
	pInfo = (PGET_LENGTH_INFORMATION)(pIrp->AssociatedIrp.SystemBuffer);
	pInfo->Length.QuadPart = UInt32x32To64(pEramExt->uAllSector, SECTOR);	/*　長さ　*/
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = sizeof(*pInfo);
	KdPrint(("EramDeviceControlGetLengthInfo length 0x%x\n", (pEramExt->uAllSector * SECTOR)));
}


/*　EramReadWrite
		リード/ライト/ベリファイ要求エントリ
	引数
		pDevObj		装置オブジェクトへのポインタ
		pIrp		IRPパケットへのポインタ
	戻り値
		結果
*/

NTSTATUS EramReadWrite(
	IN PDEVICE_OBJECT	pDevObj,
	IN PIRP				pIrp
 )
{
	/*　ローカル変数　*/
	PERAM_EXTENSION		pEramExt;
	PIO_STACK_LOCATION	pIrpSp;
	PUCHAR				pTransAddr;
	NTSTATUS			ntStat;
	//KdPrint(("EramReadWrite start\n"));
	/*　構造体先頭ポインタ取得　*/
	pEramExt = pDevObj->DeviceExtension;
	/*　スタックへのポインタ取得　*/
	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
	if ((((ULONGLONG)(pIrpSp->Parameters.Read.ByteOffset.QuadPart) + (ULONGLONG)(pIrpSp->Parameters.Read.Length)) > UInt32x32To64(pEramExt->uAllSector, SECTOR))||
		((pIrpSp->Parameters.Read.ByteOffset.LowPart & (SECTOR-1)) != 0)||
		((pIrpSp->Parameters.Read.Length & (SECTOR-1)) != 0))	/*　ディスク容量を超えたor開始位置/長さがセクタサイズの倍数でない　*/
	{
		KdPrint(("Invalid I/O parameter, offset 0x%x, length 0x%x, OP=0x%x(R=0x%x, W=0x%x), limit=0x%x\n", pIrpSp->Parameters.Read.ByteOffset.LowPart, pIrpSp->Parameters.Read.Length, pIrpSp->MajorFunction, IRP_MJ_READ, IRP_MJ_WRITE, (pEramExt->uAllSector * SECTOR)));
		/*　エラーを返す　*/
		pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_INVALID_PARAMETER;
	}
	/*　アドレス初期化　*/
	pTransAddr = NULL;
	if (pIrp->MdlAddress != NULL)		/*　アドレスあり　*/
	{
		/*　アドレス変換　*/
		pTransAddr = MmGetSystemAddressForMdl(pIrp->MdlAddress);
	}
	/*　成功をセット　*/
	ntStat = STATUS_SUCCESS;
	/*　データ長を設定　*/
	pIrp->IoStatus.Information = 0;
	switch (pIrpSp->MajorFunction)	/*　ファンクションによる分岐　*/
	{
	case IRP_MJ_READ:				/*　リード　*/
		//KdPrint(("Read start\n"));
		/*　アドレスが有効なことを確認　*/
		if (pTransAddr == NULL)
		{
			KdPrint(("MmGetSystemAddressForMdl failed\n"));
			ntStat = STATUS_INVALID_PARAMETER;
			break;
		}
		/*　リード長を設定　*/
		pIrp->IoStatus.Information = pIrpSp->Parameters.Read.Length;
		/*　リード　*/
		ntStat = (*(pEramExt->EramRead))(pEramExt, pIrp, pIrpSp, pTransAddr);
		//KdPrint(("Read end\n"));
		break;
	case IRP_MJ_WRITE:				/*　ライト　*/
		//KdPrint(("Write start\n"));
		/*　アドレスが有効なことを確認　*/
		if (pTransAddr == NULL)
		{
			KdPrint(("MmGetSystemAddressForMdl failed\n"));
			ntStat = STATUS_INVALID_PARAMETER;
			break;
		}
		/*　リード長を設定　*/
		pIrp->IoStatus.Information = pIrpSp->Parameters.Write.Length;
		/*　ライト　*/
		ntStat = (*(pEramExt->EramWrite))(pEramExt, pIrp, pIrpSp, pTransAddr);
		//KdPrint(("Write end\n"));
		break;
	default:
		KdPrint(("RW default\n"));
		pIrp->IoStatus.Information = 0;
		break;
	}
	if (ntStat != STATUS_PENDING)		/*　保留以外　*/
	{
		/*　ステータスをセット　*/
		pIrp->IoStatus.Status = ntStat;
		/*　I/O完了　*/
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	}
	//KdPrint(("EramReadWrite end\n"));
	return ntStat;
}


/*　EramUnloadDriver
		ドライバ停止時のエントリポイント
	引数
		pDrvObj		装置の代表オブジェクトへのポインタ
	戻り値
		なし
*/

VOID EramUnloadDriver(
	IN PDRIVER_OBJECT	pDrvObj
 )
{
	/*　ローカル変数　*/
	PDEVICE_OBJECT		pDevObj;
	PERAM_EXTENSION		pEramExt;
	KdPrint(("EramUnloadDriver start\n"));
	pDevObj = pDrvObj->DeviceObject;
	pEramExt = (pDevObj != NULL) ? pDevObj->DeviceExtension : NULL;
	/*　デバイス削除　*/
	EramUnloadDevice(pDrvObj, pDevObj, pEramExt);
	KdPrint(("EramUnloadDriver end\n"));
}


/*　EramUnloadDevice
		デバイス削除
	引数
		pDrvObj		装置の代表オブジェクトへのポインタ
		pDevObj		装置オブジェクトへのポインタ
		pEramExt	ERAM_EXTENTION構造体へのポインタ
	戻り値
		なし
*/

VOID EramUnloadDevice(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PDEVICE_OBJECT	pDevObj,
	IN PERAM_EXTENSION	pEramExt
 )
{
	/*　ローカル変数　*/
	LARGE_INTEGER llTime;
	KdPrint(("EramUnloadDevice start\n"));
	if (pEramExt != NULL)			/*　デバイス作成済み　*/
	{
		KdPrint(("Device exist\n"));
		/*　スレッド終了を通知　*/
		pEramExt->bThreadStop = TRUE;
		if (pEramExt->pThreadObject != NULL)		/*　スレッド存在　*/
		{
			/*　セマフォ1つ減らす　*/
			KeReleaseSemaphore(&(pEramExt->IrpSem), 0, 1, TRUE);
			/*　スレッド終了待ち30秒　*/
			llTime.QuadPart = (LONGLONG)(-30 * 10000000);
			/*　スレッド終了待ち　*/
			KeWaitForSingleObject(&(pEramExt->pThreadObject), Executive, KernelMode, FALSE, &llTime);
			/*　スレッドの参照カウントを減らす　*/
			ObDereferenceObject(&(pEramExt->pThreadObject));
			pEramExt->pThreadObject = NULL;
		}
		/*　外部ファイルクローズ　*/
		if (pEramExt->hSection != NULL)
		{
			KdPrint(("File section close\n"));
			ExtFileUnmap(pEramExt);
			ZwClose(pEramExt->hSection);
			pEramExt->hSection = NULL;
		}
		if (pEramExt->hFile != NULL)
		{
			ZwClose(pEramExt->hFile);
			pEramExt->hFile = NULL;
		}
		/*　メモリマップ解放　*/
		ResourceRelease(pDrvObj, pEramExt);
		if (pEramExt->Win32Name.Buffer != NULL)		/*　Win32名作成済　*/
		{
			/*　Win32リンク解放　*/
			IoDeleteSymbolicLink(&(pEramExt->Win32Name));
			/*　Win32名領域解放　*/
			ExFreePool(pEramExt->Win32Name.Buffer);
			pEramExt->Win32Name.Buffer = NULL;
		}
	}
	if (pDevObj != NULL)		/*　デバイス存在　*/
	{
		/*　デバイス削除　*/
		IoDeleteDevice(pDevObj);
	}
	KdPrint(("EramUnloadDevice end\n"));
}


/*　ResourceRelease
		メモリマップ削除
	引数
		pDrvObj		装置の代表オブジェクトへのポインタ
		pEramExt	ERAM_EXTENTION構造体へのポインタ
	戻り値
		なし
*/

VOID ResourceRelease(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt
 )
{
	KdPrint(("ResourceRelease start\n"));
	if (pEramExt->uOptflag.Bits.External != 0)	/*　OS管理外メモリ使用　*/
	{
		/*　資源解放　*/
		ReleaseMemResource(pDrvObj, pEramExt);
	}
	else if (pEramExt->pPageBase != NULL)		/*　メモリ確保中　*/
	{
		/*　メモリ解放　*/
		ExFreePool(pEramExt->pPageBase);
		pEramExt->pPageBase = NULL;
	}
	KdPrint(("ResourceRelease end\n"));
}


/*　ReleaseMemResource
		OS管理外メモリマップ削除
	引数
		pDrvObj		装置の代表オブジェクトへのポインタ
		pEramExt	ERAM_EXTENTION構造体へのポインタ
	戻り値
		なし
*/

VOID ReleaseMemResource(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt
 )
{
	/*　ローカル変数　*/
	CM_RESOURCE_LIST	ResList;	/*　リソースリスト　*/
	BOOLEAN				bResConf;
	/*　アンマップ　*/
	ExtUnmap(pEramExt);
	if (pEramExt->uOptflag.Bits.SkipReportUsage == 0)
	{
		/*　ドライバ資源解放(2000では解放されないようだ)　*/
		RtlZeroBytes(&ResList, sizeof(ResList));
		IoReportResourceUsage(NULL, pDrvObj, &ResList, sizeof(ResList), NULL, NULL, 0, FALSE, &bResConf);
		RtlZeroBytes(&(pEramExt->MapAdr), sizeof(pEramExt->MapAdr));
	}
}


/*　EramReportEvent
		システムイベントログ出力
	引数
		pIoObject	デバイスオブジェクトpDevObj または ドライバオブジェクトpDrvObj
		ntErrorCode	イベントID
		pszString	付加文字列 省略時NULL
	戻り値
		結果
*/

BOOLEAN EramReportEvent(
	IN PVOID	pIoObject,
	IN NTSTATUS	ntErrorCode,
	IN PSTR		pszString
 )
{
	/*　ローカル変数　*/
	ANSI_STRING		AnsiStr;
	UNICODE_STRING	UniStr;
	BOOLEAN			bStat;
	if ((pszString != NULL)&&
		(*pszString != L'\0'))
	{
		RtlInitAnsiString(&AnsiStr, pszString);
		/*　UNICODE文字列化してダンプ　*/
		if (RtlAnsiStringToUnicodeString(&UniStr, &AnsiStr, TRUE) == STATUS_SUCCESS)
		{
			bStat = EramReportEventW(pIoObject, ntErrorCode, UniStr.Buffer);
			RtlFreeUnicodeString(&UniStr);
			return bStat;
		}
	}
	return EramReportEventW(pIoObject, ntErrorCode, NULL);
}


/*　EramReportEventW
		システムイベントログ出力
	引数
		pIoObject	デバイスオブジェクトpDevObj または ドライバオブジェクトpDrvObj
		ntErrorCode	イベントID
		pwStr		付加文字列(UNICODE) 省略時NULL
	戻り値
		結果
*/

BOOLEAN EramReportEventW(
	IN PVOID	pIoObject,
	IN NTSTATUS	ntErrorCode,
	IN PWSTR	pwStr
 )
{
	/*　ローカル変数　*/
	PIO_ERROR_LOG_PACKET	pPacket;
	ULONG					uSize;
	UNICODE_STRING			UniStr;
	KdPrint(("EramReportEventW start, event:%ls\n", (PWSTR)((pwStr != NULL) ? pwStr : (PWSTR)(L""))));
	/*　パケットサイズ初期化　*/
	uSize = sizeof(IO_ERROR_LOG_PACKET);
	if (pwStr != NULL)	/*　付加文字列あり　*/
	{
		RtlInitUnicodeString(&UniStr, pwStr);
		/*　パケットサイズ加算　*/
		uSize += (UniStr.Length + sizeof(WCHAR));
	}
	if (uSize > ERROR_LOG_MAXIMUM_SIZE)		/*　文字列が長すぎる　*/
	{
		KdPrint(("String too long\n"));
		return FALSE;
	}
	/*　パケット確保　*/
	pPacket = IoAllocateErrorLogEntry(pIoObject, (UCHAR)uSize);
	if (pPacket == NULL)	/*　確保失敗　*/
	{
		KdPrint(("IoAllocateErrorLogEntry failed\n"));
		return FALSE;
	}
	/*　標準データ部初期化　*/
	RtlZeroBytes(pPacket, uSize);
	pPacket->ErrorCode = ntErrorCode;
	if (pwStr != NULL)		/*　付加文字列あり　*/
	{
		/*　文字列数を設定　*/
		pPacket->NumberOfStrings = 1;
		/*　文字列開始位置を設定　*/
		pPacket->StringOffset = sizeof(IO_ERROR_LOG_PACKET);
		/*　UNICODE文字列をコピー　*/
		RtlCopyBytes(&(pPacket[1]), UniStr.Buffer, UniStr.Length);
	}
	/*　ログ出力　*/
	IoWriteErrorLogEntry(pPacket);
	KdPrint(("EramReportEventW end\n"));
	return TRUE;
}


/*　ReadPool
		OS管理メモリ読み込み
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pIrp		IRPパケットへのポインタ
		pIrpSp		スタック情報へのポインタ
		lpDest		格納領域へのポインタ
	戻り値
		結果
*/

NTSTATUS ReadPool(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				lpDest
 )
{
	/*　ローカル変数　*/
	PUCHAR lpSrc;
	lpSrc = (PUCHAR)((ULONG)pEramExt->pPageBase + (ULONG)pIrpSp->Parameters.Read.ByteOffset.LowPart);
	RtlCopyBytes(lpDest, lpSrc, pIrpSp->Parameters.Read.Length);
	return STATUS_SUCCESS;
}


/*　WritePool
		OS管理メモリ書き込み
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pIrp		IRPパケットへのポインタ
		pIrpSp		スタック情報へのポインタ
		lpSrc		データ領域へのポインタ
	戻り値
		結果
*/

NTSTATUS WritePool(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				lpSrc
 )
{
	/*　ローカル変数　*/
	PUCHAR lpDest;
	lpDest = (PUCHAR)((ULONG)pEramExt->pPageBase + (ULONG)pIrpSp->Parameters.Write.ByteOffset.LowPart);
	RtlCopyBytes(lpDest, lpSrc, pIrpSp->Parameters.Write.Length);
	return STATUS_SUCCESS;
}


/*　ExtRead1
		OS管理外メモリ読み込み(checkなし)
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pIrp		IRPパケットへのポインタ
		pIrpSp		スタック情報へのポインタ
		lpDest		格納領域へのポインタ
	戻り値
		結果
*/

NTSTATUS ExtRead1(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				lpDest
 )
{
	/*　ローカル変数　*/
	PUCHAR	lpSrc;
	UINT	uLen;
	DWORD	eax, ebx;
	NTSTATUS ntStat;
	ULONG	uMemAdr;
	ASSERT(pEramExt->uExternalStart != 0);
	ASSERT(pEramExt->uExternalEnd != 0);
	/*　Mutex待ち　*/
	ExAcquireFastMutex(&(pEramExt->FastMutex));
	uLen = pIrpSp->Parameters.Read.Length;	/*　転送サイズ(セクタサイズの倍数)　*/
	/*　セクタ番号を計算　*/
	ebx = pIrpSp->Parameters.Read.ByteOffset.LowPart >> SECTOR_LOG2;
	/*　メモリ位置を計算　*/
	uMemAdr = pEramExt->uExternalStart + pIrpSp->Parameters.Read.ByteOffset.LowPart;
	ntStat = STATUS_SUCCESS;
	while (uLen != 0)
	{
		if (uMemAdr >= pEramExt->uExternalEnd)	/*　実メモリを超えている　*/
		{
			ntStat = STATUS_DISK_CORRUPT_ERROR;
			break;
		}
		/*　64KB割り当て　*/
		if (ExtNext1(pEramExt, &eax, &ebx) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "ExtNext1");
			ntStat = STATUS_DISK_CORRUPT_ERROR;
			break;
		}
		lpSrc = (PUCHAR)((ULONG)(pEramExt->pExtPage + eax));
		/*　データ転送　*/
		RtlCopyBytes(lpDest, lpSrc, SECTOR);
		lpDest += SECTOR;
		uLen -= SECTOR;
		uMemAdr += SECTOR;
	}
	/*　アンマップ　*/
	ExtUnmap(pEramExt);
	/*　Mutex解放　*/
	ExReleaseFastMutex(&(pEramExt->FastMutex));
	return ntStat;
}


/*　ExtWrite1
		OS管理外メモリ書き込み(checkなし)
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pIrp		IRPパケットへのポインタ
		pIrpSp		スタック情報へのポインタ
		lpSrc		データ領域へのポインタ
	戻り値
		結果
*/

NTSTATUS ExtWrite1(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				lpSrc
 )
{
	/*　ローカル変数　*/
	PUCHAR lpDest;
	UINT	uLen;
	DWORD	eax, ebx;
	NTSTATUS ntStat;
	ULONG	uMemAdr;
	ASSERT(pEramExt->uExternalStart != 0);
	ASSERT(pEramExt->uExternalEnd != 0);
	/*　Mutex待ち　*/
	ExAcquireFastMutex(&(pEramExt->FastMutex));
	uLen = pIrpSp->Parameters.Write.Length;
	/*　セクタ番号を計算　*/
	ebx = pIrpSp->Parameters.Write.ByteOffset.LowPart >> SECTOR_LOG2;
	/*　メモリ位置を計算　*/
	uMemAdr = pEramExt->uExternalStart + pIrpSp->Parameters.Write.ByteOffset.LowPart;
	ntStat = STATUS_SUCCESS;
	while (uLen != 0)
	{
		if (uMemAdr >= pEramExt->uExternalEnd)	/*　実メモリを超えている　*/
		{
			ntStat = STATUS_DISK_CORRUPT_ERROR;
			break;
		}
		/*　64KB割り当て　*/
		if (ExtNext1(pEramExt, &eax, &ebx) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "ExtNext1");
			ntStat = STATUS_DISK_CORRUPT_ERROR;
			break;
		}
		lpDest = (PUCHAR)((ULONG)(pEramExt->pExtPage + eax));
		/*　データ転送　*/
		RtlCopyBytes(lpDest, lpSrc, SECTOR);
		lpSrc += SECTOR;
		uLen -= SECTOR;
		uMemAdr += SECTOR;
	}
	/*　アンマップ　*/
	ExtUnmap(pEramExt);
	/*　Mutex解放　*/
	ExReleaseFastMutex(&(pEramExt->FastMutex));
	return ntStat;
}


/*　ExtNext1
		OS管理外:該当セクタの割り当て(checkなし時)
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		lpeax		ページ内オフセットを返す領域へのポインタ
		lpebx		セクタ番号へのポインタ(+1される)
	戻り値
		結果
*/

BOOLEAN ExtNext1(
	IN PERAM_EXTENSION	pEramExt,
	IN OUT LPDWORD		lpeax,
	IN OUT LPDWORD		lpebx
 )
{
	/*　ローカル変数　*/
	DWORD eax, ebx, uMapAdr;
	ebx = *lpebx;
	/*　マップすべきバンク番号を計算　*/
	uMapAdr = (ebx >> EXT_PAGE_SEC_LOG2) << EXT_PAGE_SIZE_LOG2;
	/*　マップ　*/
	if (ExtMap(pEramExt, uMapAdr) == FALSE)
	{
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "ExtMap");
		return FALSE;
	}
	/*　オフセット算出　*/
	eax = ebx & (EXT_PAGE_SECTOR - 1);
	eax <<= SECTOR_LOG2;
	/*　セクタ番号を進める　*/
	ebx++;
	*lpeax = eax;
	*lpebx = ebx;
	return TRUE;
}


/*　ExtMap
		OS管理外メモリのマップ
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		uMapAdr		マップする相対バイト位置(64KB単位)
	戻り値
		結果
*/

BOOLEAN ExtMap(
	IN PERAM_EXTENSION	pEramExt,
	IN ULONG			uMapAdr
 )
{
	/*　ローカル変数　*/
	PHYSICAL_ADDRESS	MapAdr;
	if ((pEramExt->pExtPage == NULL)||			/*　未マップ　*/
		(pEramExt->uNowMapAdr != uMapAdr))	/*　現在マップ中のページと異なる　*/
	{
		/*　現在のページをアンマップ　*/
		ExtUnmap(pEramExt);
		/*　マップ位置補正　*/
		MapAdr = pEramExt->MapAdr;
		if (MapAdr.LowPart == 0)		/*　解放済み　*/
		{
			KdPrint(("Already resource released\n"));
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_MAXMEM_ALREADY_FREE, NULL);
			return FALSE;
		}
		MapAdr.LowPart += uMapAdr;
		/*　マップ(キャッシュ許可)　*/
		pEramExt->pExtPage = (PBYTE)MmMapIoSpace(MapAdr, EXT_PAGE_SIZE, TRUE);
		if (pEramExt->pExtPage == NULL)		/*　失敗　*/
		{
			KdPrint(("MmMapIoSpace failed\n"));
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_MAXMEM_MAP_FAILED, NULL);
			return FALSE;
		}
		pEramExt->uNowMapAdr = uMapAdr;
	}
	return TRUE;
}


/*　ExtUnmap
		OS管理外メモリのアンマップ
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
	戻り値
		なし
*/

VOID ExtUnmap(
	IN PERAM_EXTENSION	pEramExt
 )
{
	if (pEramExt->pExtPage != NULL)		/*　マップ中のページあり　*/
	{
		/*　64KBアンマップ　*/
		MmUnmapIoSpace(pEramExt->pExtPage, EXT_PAGE_SIZE);
		pEramExt->pExtPage = NULL;
		pEramExt->uNowMapAdr = 0;
	}
}


/*　ExtFilePendingRw
		外部ファイル読み込み(保留)
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pIrp		IRPパケットへのポインタ
		pIrpSp		スタック情報へのポインタ
		pTransAddr	格納領域へのポインタ
	戻り値
		結果
*/

NTSTATUS ExtFilePendingRw(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				pTransAddr
 )
{
	//KdPrint(("ExtFilePendingRw start\n"));
	if (pEramExt->bThreadStop != 0)		/*　終了指示済み　*/
	{
		KdPrint(("stop sequence\n"));
		pIrp->IoStatus.Information = 0;
		return STATUS_DEVICE_NOT_READY;
	}
	if (pEramExt->pThreadObject == NULL)	/*　スレッド無い　*/
	{
		KdPrint(("Thread not exist\n"));
		pIrp->IoStatus.Information = 0;
		return STATUS_DEVICE_NOT_READY;
	}
	/*　I/O保留　*/
	IoMarkIrpPending(pIrp);
	pIrp->IoStatus.Status = STATUS_PENDING;
	/*　キューに載せる　*/
	ExInterlockedInsertTailList(&(pEramExt->IrpList), &(pIrp->Tail.Overlay.ListEntry), &(pEramExt->IrpSpin));
	/*　セマフォ1つ減らす　*/
	KeReleaseSemaphore(&(pEramExt->IrpSem), 0, 1, FALSE);
	//KdPrint(("ExtFilePendingRw end\n"));
	return STATUS_PENDING;
}


/*　EramRwThread
		外部ファイルリード/ライト要求エントリ(DISPATCH_LEVEL)
	引数
		pContext		引き渡し情報へのポインタ
	戻り値
		なし
*/

VOID EramRwThread(
	IN PVOID			pContext
 )
{
	/*　ローカル変数　*/
	PERAM_EXTENSION		pEramExt;
	PIRP				pIrp;
	NTSTATUS			ntStat;
	PLIST_ENTRY			pIrpList;
	KdPrint(("EramRwThread start\n"));
	/*　先頭ポインタ取得　*/
	pEramExt = pContext;
	ASSERT(pEramExt != NULL);
	/*　優先化　*/
	KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);		//　標準はPrior8
	while (pEramExt->bThreadStop == 0)		/*　スレッド活動中　*/
	{
		//KdPrint(("Waiting\n"));
		/*　要求待ち 兼 カウンタ戻し　*/
		KeWaitForSingleObject(&(pEramExt->IrpSem), Executive, KernelMode, FALSE, NULL);
		if (pEramExt->bThreadStop != 0)		/*　スレッド停止要求　*/
		{
			KdPrint(("thread should stop\n"));
			break;
		}
		//KdPrint(("Wake\n"));
		/*　IRPリストの先頭を取得　*/
		pIrpList = ExInterlockedRemoveHeadList(&(pEramExt->IrpList), &(pEramExt->IrpSpin));
		//KdPrint(("Get list\n"));
		if (pIrpList != NULL)		/*　リスト有効　*/
		{
			ntStat = EramRwThreadIrp(pEramExt, pIrpList);
			//KdPrint(("EramRwThreadIrp return 0x%x\n", ntStat));
		}
	}
	/*　残りのIRPをキャンセル　*/
	for (;;)
	{
		/*　IRPリストの先頭を取得　*/
		pIrpList = ExInterlockedRemoveHeadList(&(pEramExt->IrpList), &(pEramExt->IrpSpin));
		if (pIrpList == NULL)		/*　これ以上は無い　*/
		{
			break;
		}
		/*　IRPを取得　*/
		pIrp = CONTAINING_RECORD(pIrpList, IRP, Tail.Overlay.ListEntry);
		ASSERT(pIrp != NULL);
		pIrp->IoStatus.Information = 0;
		pIrp->IoStatus.Status = STATUS_DEVICE_NOT_READY;
		/*　I/O完了　*/
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


/*　EramRwThreadIrp
		外部ファイルリード/ライト要求(1IRP)
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pIrpList	IRPリスト情報へのポインタ
	戻り値
		結果
*/

NTSTATUS EramRwThreadIrp(
	PERAM_EXTENSION		pEramExt,
	PLIST_ENTRY			pIrpList
 )
{
	/*　ローカル変数　*/
	PIRP				pIrp;
	PIO_STACK_LOCATION	pIrpSp;
	NTSTATUS			ntStat;
	PUCHAR				pTransAddr;
	//KdPrint(("EramRwThreadIrp start\n"));
	ASSERT(pEramExt != NULL);
	ASSERT(pIrpList != NULL);
	/*　IRPを取得　*/
	pIrp = CONTAINING_RECORD(pIrpList, IRP, Tail.Overlay.ListEntry);
	ASSERT(pIrp != NULL);
	pIrp->IoStatus.Information = 0;
	pTransAddr = NULL;
	if (pIrp->MdlAddress != NULL)	/*　アドレスあり　*/
	{
		/*　アドレス変換　*/
		pTransAddr = MmGetSystemAddressForMdl(pIrp->MdlAddress);
	}
	if (pTransAddr == NULL)		/*　変換失敗　*/
	{
		KdPrint(("MmGetSystemAddressForMdl failed\n"));
		/*　ステータスをセット　*/
		pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		/*　I/O完了　*/
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_INVALID_PARAMETER;
	}
	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
	ASSERT(pIrpSp != NULL);
	switch (pIrpSp->MajorFunction)	/*　ファンクションによる分岐　*/
	{
	case IRP_MJ_READ:				/*　リード　*/
		/*　リード長を設定　*/
		pIrp->IoStatus.Information = pIrpSp->Parameters.Read.Length;
		ntStat = ExtFileRead1(pEramExt, pIrp, pIrpSp, pTransAddr);
		break;
	case IRP_MJ_WRITE:				/*　ライト　*/
		/*　ライト長を設定　*/
		pIrp->IoStatus.Information = pIrpSp->Parameters.Write.Length;
		ntStat = ExtFileWrite1(pEramExt, pIrp, pIrpSp, pTransAddr);
		break;
	default:
		//KdPrint(("RW default\n"));
		ntStat = STATUS_SUCCESS;
		break;
	}
	/*　ステータスをセット　*/
	pIrp->IoStatus.Status = ntStat;
	/*　I/O完了　*/
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	//KdPrint(("EramRwThreadIrp end\n"));
	return ntStat;
}


/*　ExtFileRead1
		外部ファイル読み込み(checkなし)
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pIrp		IRPパケットへのポインタ
		pIrpSp		スタック情報へのポインタ
		lpDest		格納領域へのポインタ
	戻り値
		結果
*/

NTSTATUS ExtFileRead1(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				lpDest
 )
{
	/*　ローカル変数　*/
	PUCHAR	lpSrc;
	UINT	uLen;
	DWORD	eax, ebx;
	NTSTATUS ntStat;
	ASSERT(pEramExt != NULL);
	uLen = pIrpSp->Parameters.Read.Length;	/*　転送サイズ(セクタサイズの倍数)　*/
	/*　セクタ番号を計算　*/
	ebx = pIrpSp->Parameters.Read.ByteOffset.LowPart >> SECTOR_LOG2;
	ntStat = STATUS_SUCCESS;
	while (uLen != 0)
	{
		/*　64KB割り当て　*/
		if (ExtFileNext1(pEramExt, &eax, &ebx) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "ExtFileNext1");
			ntStat = STATUS_DISK_CORRUPT_ERROR;
			break;
		}
		lpSrc = (PUCHAR)((ULONG)(pEramExt->pExtPage + eax));
		/*　データ転送　*/
		RtlCopyBytes(lpDest, lpSrc, SECTOR);
		lpDest += SECTOR;
		uLen -= SECTOR;
	}
	/*　アンマップ　*/
	ExtFileUnmap(pEramExt);
	return ntStat;
}


/*　ExtFileWrite1
		外部ファイル書き込み(checkなし)
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pIrp		IRPパケットへのポインタ
		pIrpSp		スタック情報へのポインタ
		lpSrc		データ領域へのポインタ
	戻り値
		結果
*/

NTSTATUS ExtFileWrite1(
	IN PERAM_EXTENSION		pEramExt,
	IN PIRP					pIrp,
	IN PIO_STACK_LOCATION	pIrpSp,
	IN PUCHAR				lpSrc
 )
{
	/*　ローカル変数　*/
	PUCHAR lpDest;
	UINT	uLen;
	DWORD	eax, ebx;
	NTSTATUS ntStat;
	ASSERT(pEramExt != NULL);
	uLen = pIrpSp->Parameters.Write.Length;
	/*　セクタ番号を計算　*/
	ebx = pIrpSp->Parameters.Write.ByteOffset.LowPart >> SECTOR_LOG2;
	ntStat = STATUS_SUCCESS;
	while (uLen != 0)
	{
		/*　64KB割り当て　*/
		if (ExtFileNext1(pEramExt, &eax, &ebx) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "ExtFileNext1");
			ntStat = STATUS_DISK_CORRUPT_ERROR;
			break;
		}
		lpDest = (PUCHAR)((ULONG)(pEramExt->pExtPage + eax));
		/*　データ転送　*/
		RtlCopyBytes(lpDest, lpSrc, SECTOR);
		lpSrc += SECTOR;
		uLen -= SECTOR;
	}
	/*　アンマップ　*/
	ExtFileUnmap(pEramExt);
	return ntStat;
}


/*　ExtFileNext1
		外部ファイル:該当セクタの割り当て(checkなし時)
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		lpeax		ページ内オフセットを返す領域へのポインタ
		lpebx		セクタ番号へのポインタ(+1される)
	戻り値
		結果
*/

BOOLEAN ExtFileNext1(
	IN PERAM_EXTENSION	pEramExt,
	IN OUT LPDWORD		lpeax,
	IN OUT LPDWORD		lpebx
 )
{
	/*　ローカル変数　*/
	DWORD eax, ebx, uMapAdr;
	ASSERT(pEramExt != NULL);
	ebx = *lpebx;
	/*　マップすべきバンク番号を計算　*/
	uMapAdr = (ebx >> EXT_PAGE_SEC_LOG2) << EXT_PAGE_SIZE_LOG2;
	/*　マップ　*/
	if (ExtFileMap(pEramExt, uMapAdr) == FALSE)
	{
		KdPrint(("ExtFileMap failed, MapAdr=0x%x, sector=0x%x, SizeSec=0x%x, SizeBytes=0x%x\n", uMapAdr, ebx, (pEramExt->uAllSector << SECTOR_LOG2), (pEramExt->uSizeTotal << PAGE_SIZE_LOG2)));
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "ExtFileMap");
		return FALSE;
	}
	/*　オフセット算出　*/
	eax = ebx & (EXT_PAGE_SECTOR - 1);
	eax <<= SECTOR_LOG2;
	/*　セクタ番号を進める　*/
	ebx++;
	*lpeax = eax;
	*lpebx = ebx;
	return TRUE;
}


/*　ExtFileMap
		外部ファイルのマップ
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		uMapAdr		マップする相対バイト位置(64KB単位)
	戻り値
		結果
*/

BOOLEAN ExtFileMap(
	IN PERAM_EXTENSION	pEramExt,
	IN ULONG			uMapAdr
 )
{
	/*　ローカル変数　*/
	LARGE_INTEGER llOfs;
	ULONG uView;
	NTSTATUS ntStat;
	ASSERT(pEramExt != NULL);
	if ((pEramExt->pExtPage == NULL)||			/*　未マップ　*/
		(pEramExt->uNowMapAdr != uMapAdr))		/*　現在マップ中のページと異なる　*/
	{
		/*　現在のページをアンマップ　*/
		ExtFileUnmap(pEramExt);
		/*　マップ位置準備　*/
		llOfs.QuadPart = (LONGLONG)uMapAdr;
		uView = ((pEramExt->uSizeTotal << PAGE_SIZE_LOG2) - uMapAdr);
		if (uView > EXT_PAGE_SIZE)
		{
			uView = EXT_PAGE_SIZE;
		}
		/*　マップ(キャッシュ許可)　*/
		ntStat = ZwMapViewOfSection(pEramExt->hSection, NtCurrentProcess(), &(pEramExt->pExtPage), 0, uView, &llOfs, &uView, ViewShare, 0, PAGE_READWRITE);
		if (ntStat != STATUS_SUCCESS)		/*　失敗　*/
		{
			KdPrint(("ZwMapViewOfSection failed, 0x%x, MapAdr=0x%x, size=0x%x\n", ntStat, uMapAdr, uView));
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_MAP_EXT_FILE, NULL);
			return FALSE;
		}
		ASSERT((pEramExt->pExtPage) != NULL);
		pEramExt->uNowMapAdr = uMapAdr;
	}
	return TRUE;
}


/*　ExtFileUnmap
		外部ファイルのアンマップ
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
	戻り値
		なし
*/

VOID ExtFileUnmap(
	IN PERAM_EXTENSION	pEramExt
 )
{
	/*　ローカル変数　*/
	NTSTATUS ntStat;
	ASSERT(pEramExt != NULL);
	if (pEramExt->pExtPage == NULL)		/*　マップ中のページなし　*/
	{
		return;
	}
	/*　64KBアンマップ　*/
	ntStat = ZwUnmapViewOfSection(NtCurrentProcess(), pEramExt->pExtPage);
	pEramExt->pExtPage = NULL;
	pEramExt->uNowMapAdr = 0;
	if (ntStat != STATUS_SUCCESS)
	{
		KdPrint(("ZwMapViewOfSection failed, 0x%x\n", ntStat));
	}
}


/*　EramShutdown
		シャットダウン要求エントリ
	引数
		pDevObj	装置オブジェクトへのポインタ
		pIrp	IRPパケットへのポインタ
	戻り値
		結果
*/

NTSTATUS EramShutdown(
	IN PDEVICE_OBJECT	pDevObj,
	IN PIRP				pIrp
 )
{
	/*　ローカル変数　*/
	PERAM_EXTENSION pEramExt;
	LARGE_INTEGER  	llTime;
	KdPrint(("EramShutdown start\n"));
	pEramExt = pDevObj->DeviceExtension;
	/*　スレッド終了を通知　*/
	pEramExt->bThreadStop = TRUE;
	if (pEramExt->pThreadObject != NULL)		/*　スレッド存在　*/
	{
		/*　セマフォ1つ減らす　*/
		KeReleaseSemaphore(&(pEramExt->IrpSem), 0, 1, TRUE);
		/*　スレッド終了待ち5秒　*/
		llTime.QuadPart = (LONGLONG)(-5 * 10000000);
		KeWaitForSingleObject(&(pEramExt->pThreadObject), Executive, KernelMode, FALSE, &llTime);
		/*　スレッドの参照カウントを減らす　*/
		ObDereferenceObject(&(pEramExt->pThreadObject));
		pEramExt->pThreadObject = NULL;
	}
	/*　外部ファイルクローズ　*/
	if (pEramExt->hSection != NULL)
	{
		KdPrint(("File section close\n"));
		ExtFileUnmap(pEramExt);
		ZwClose(pEramExt->hSection);
		pEramExt->hSection = NULL;
	}
	if (pEramExt->hFile != NULL)
	{
		ZwClose(pEramExt->hFile);
		pEramExt->hFile = NULL;
	}
	/*　成功をセット　*/
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	KdPrint(("EramShutdown end\n"));
	return STATUS_SUCCESS;
}


//------  これ以降は初期化時に使用する関数群


/*　DriverEntry
		ドライバ初期化時のエントリポイント
	引数
		pDrvObj		装置の代表オブジェクトへのポインタ
		pRegPath	レジストリキーへのポインタ
	戻り値
		結果
*/

NTSTATUS DriverEntry(
	IN OUT PDRIVER_OBJECT	pDrvObj,
	IN PUNICODE_STRING		pRegPath
 )
{
	/*　ローカル変数　*/
	NTSTATUS		ntStat;
	UNICODE_STRING	RegParam;		/*　UNICODEを用いたレジストリパス　*/
	UNICODE_STRING	RegParamAdd;	/*　UNICODEを用いたレジストリパス　*/
	PVOID			pPool;
	PFAT_ID			pFatId;			/*　BPB作業領域　*/
	KdPrint(("DriverEntry start\n"));
	/*　レジストリパス最大長を取得　*/
	RegParam.MaximumLength = (WORD)(pRegPath->Length + sizeof(SUBKEY_WSTRING));
	/*　作業用メモリ確保　*/
	pPool = ExAllocatePool(PagedPool, sizeof(*pFatId) + RegParam.MaximumLength);
	if (pPool == NULL)		/*　確保失敗　*/
	{
		KdPrint(("ExAllocatePool failed\n"));
		EramReportEvent(pDrvObj, ERAMNT_ERROR_WORK_ALLOC_FAILED, NULL);
		/*　エラーを返す　*/
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	/*　ポインタ設定　*/
	pFatId = (PFAT_ID)pPool;
	RegParam.Buffer = (PWSTR)(&(pFatId[1]));
	/*　レジストリキー文字列をコピー　*/
	RtlCopyUnicodeString(&RegParam, pRegPath);
	/*　レジストリサブキー文字列をコピー　*/
	RtlInitUnicodeString(&RegParamAdd, (PWSTR)SUBKEY_WSTRING);
	if (RtlAppendUnicodeStringToString(&RegParam, &RegParamAdd) != STATUS_SUCCESS)	/*　合成失敗　*/
	{
		KdPrint(("RtlAppendUnicodeStringToString failed\n"));
		EramReportEvent(pDrvObj, ERAMNT_ERROR_REG_KEY_APPEND_FAILED, NULL);
		/*　作業用メモリを解放　*/
		ExFreePool(pPool);
		/*　エラーを返す　*/
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	/*　FatId構造体の初期化　*/
	InitFatId(pFatId);
	/*　ドライバのエントリポイントを初期化　*/
	pDrvObj->MajorFunction[IRP_MJ_CREATE] = EramCreateClose;
	pDrvObj->MajorFunction[IRP_MJ_CLOSE] = EramCreateClose;
	pDrvObj->MajorFunction[IRP_MJ_READ] = EramReadWrite;
	pDrvObj->MajorFunction[IRP_MJ_WRITE] = EramReadWrite;
	pDrvObj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = EramDeviceControl;
	pDrvObj->MajorFunction[IRP_MJ_SHUTDOWN] = EramShutdown;
	/*　解放時のエントリを初期化　*/
	pDrvObj->DriverUnload = EramUnloadDriver;
	/*　RAMディスクを初期化　*/
	ntStat = EramInitDisk(pDrvObj, pFatId, &RegParam);
	ASSERT(pPool != NULL);
	/*　作業用メモリを解放　*/
	ExFreePool(pPool);
	KdPrint(("DriverEntry end\n"));
	/*　初期化終了　*/
	return ntStat;
}


/*　InitFatId
		FatId構造体の初期化
	引数
		pFatId		FAT-ID構造体へのポインタ
	戻り値
		なし
*/

VOID InitFatId(
	IN	PFAT_ID		pFatId
 )
{
	KdPrint(("InitFatId start\n"));
	RtlZeroBytes(pFatId, sizeof(*pFatId));
	pFatId->BPB.wNumSectorByte = SECTOR;				/*　セクタバイト数(BPB, =SECTOR)　*/
	pFatId->BPB.byAllocUnit = 1024 / SECTOR;			/*　アロケーションユニット(alloc, =1024/SECTOR)　*/
	pFatId->BPB.wNumResvSector = 1;						/*　予約セクタ数(=1)　*/
	pFatId->BPB.byNumFat = 1;							/*　FAT数(=1)　*/
	pFatId->BPB.wNumDirectory = 128;					/*　ルートディレクトリエントリ数(dir, =128)　*/
	pFatId->BPB.byMediaId = RAMDISK_MEDIA_TYPE;			/*　メディアID(media, =f8)　*/
	pFatId->BPB_ext.bsSecPerTrack = 1;					/*　1バンクあたりのセクタ数(=PAGE_SECTOR)　*/
	pFatId->BPB_ext.bsHeads = 1;						/*　ヘッド数(=1)　*/
	pFatId->BPB_fat32.dwRootCluster = 2;				/*　ルートディレクトリの開始クラスタ　*/
	pFatId->BPB_fat32.wFsInfoSector = 1;				/*　FSINFOのセクタ　*/
	pFatId->BPB_fat32.wBackupBootSector = 0xffff;		/*　バックアップブートセクタ　*/
	KdPrint(("InitFatId end\n"));
}


/*　EramInitDisk
		ERAM初期化
	引数
		pDrvObj		装置の代表オブジェクトへのポインタ
		pFatId		FAT-ID構造体へのポインタ
		pRegParam	レジストリパス文字列へのポインタ
	戻り値
		結果
*/

NTSTATUS EramInitDisk(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PFAT_ID			pFatId,
	IN PUNICODE_STRING	pRegParam
 )
{
	/*　ローカル変数　*/
	UNICODE_STRING	NtDevName;		/*　NTデバイス名 "\Device\Eram"　*/
	UNICODE_STRING	Win32Name;		/*　Win32名 "\DosDevices\Z:"　*/
	UNICODE_STRING	DrvStr;			/*　ドライブ文字　*/
	WCHAR			DrvBuf[3];		/*　ドライブ文字取得用バッファ　*/
	PDEVICE_OBJECT	pDevObj = NULL;
	PERAM_EXTENSION	pEramExt = NULL;
	NTSTATUS		ntStat;
	ULONG			uMemSize;
	DEVICE_TYPE		dType;
	KdPrint(("EramInitDisk start\n"));
	/*　スワップ可能デバイスにするかどうか確認　*/
	dType = CheckSwapable(pRegParam);
	/*　文字列を初期化　*/
	RtlInitUnicodeString(&NtDevName, (PWSTR)NT_DEVNAME);
	/*　デバイス名を差し替えるかどうか確認　*/
	CheckDeviceName(pRegParam, &NtDevName);
	/*　デバイス作成　*/
	ntStat = IoCreateDevice(pDrvObj, sizeof(*pEramExt), &NtDevName, dType, 0, FALSE, &pDevObj);
	if (ntStat != STATUS_SUCCESS)	/*　失敗　*/
	{
		KdPrint(("IoCreateDevice failed, 0x%x\n", ntStat));
{
	CHAR szBuf[128];
	sprintf(szBuf, "IoCreateDevice failed, 0x%x", ntStat);
	EramReportEvent(pDrvObj, ERAMNT_ERROR_FUNCTIONERROR, szBuf);	//@@@
	sprintf(szBuf, "Device is \"%ls\"", NtDevName.Buffer);
	EramReportEvent(pDrvObj, ERAMNT_ERROR_FUNCTIONERROR, szBuf);	//@@@
}

		EramReportEvent(pDrvObj, ERAMNT_ERROR_CREATE_DEVICE_FAILED, NULL);
		return ntStat;
	}
	/*　情報ポインタ取得　*/
	pEramExt = (PERAM_EXTENSION)(pDevObj->DeviceExtension);
	/*　ERAM情報領域初期化　*/
	RtlZeroBytes(pEramExt, sizeof(*pEramExt));
	/*　ドライブ文字バッファクリア　*/
	DrvBuf[0] = UNICODE_NULL;
	RtlInitUnicodeString(&DrvStr, DrvBuf);
	DrvStr.MaximumLength = sizeof(DrvBuf);
	/*　レジストリから情報取得　*/
	CheckSwitch(pEramExt, pFatId, pRegParam, &DrvStr);
	pEramExt->uOptflag.Bits.Swapable = (BYTE)((dType == FILE_DEVICE_DISK) ? 1 : 0);
	/*　デバイス情報初期化　*/
	pDevObj->Flags |= DO_DIRECT_IO;
	pDevObj->AlignmentRequirement = FILE_WORD_ALIGNMENT;
	pEramExt->pDevObj = pDevObj;
	if (pEramExt->uOptflag.Bits.External != 0)				/*　OS管理外メモリ使用　*/
	{
		if (GetExternalStart(pDrvObj, pEramExt) == FALSE)	/*　OS管理外メモリ無し　*/
		{
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_MAXMEM_NOT_DETECTED, NULL);
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		if ((pEramExt->uOptflag.Bits.SkipExternalCheck == 0)&&	/*　チェック飛ばさない　*/
			(CheckExternalSize(pDrvObj, pEramExt) == FALSE))	/*　チェック失敗　*/
		{
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "CheckExternalSize");
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		/*　ミューテックス初期化　*/
		ExInitializeFastMutex(&(pEramExt->FastMutex));
	}
	/*　メモリ確保　*/
	uMemSize = pEramExt->uSizeTotal << PAGE_SIZE_LOG2;
	if (pEramExt->uSizeTotal < DISKMINPAGE)		/*　メモリなし　*/
	{
		KdPrint(("Memory size too small, %d\n", pEramExt->uSizeTotal));
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_DISK_SIZE_TOO_SMALL, NULL);
		ntStat = STATUS_INSUFFICIENT_RESOURCES;
		goto EramInitDiskExit;
	}
	/*　メモリ領域確保　*/
	ntStat = MemSetup(pDrvObj, pEramExt, pFatId, uMemSize);
	if (ntStat != STATUS_SUCCESS)	/*　失敗　*/
	{
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "MemSetup");
		goto EramInitDiskExit;
	}
	/*　FATフォーマット　*/
	if (EramFormatFat(pEramExt, pFatId) == FALSE)
	{
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "EramFormatFat");
		ntStat = STATUS_INSUFFICIENT_RESOURCES;
		goto EramInitDiskExit;
	}
	/*　ERAM情報設定　*/
	pEramExt->bsHiddenSecs = pFatId->BPB_ext.bsHiddenSecs;
	/*　Win32デバイス名領域確保　*/
	pEramExt->Win32Name.Buffer = ExAllocatePool(PagedPool, (sizeof(WIN32_PATH) + sizeof(DEFAULT_DRV)));
	if (pEramExt->Win32Name.Buffer == NULL)		/*　確保失敗　*/
	{
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_DEVICE_NAME_ALLOC_FAILED, NULL);
		ntStat = STATUS_INSUFFICIENT_RESOURCES;
		goto EramInitDiskExit;
	}
	pEramExt->Win32Name.MaximumLength = sizeof(WIN32_PATH) + sizeof(DEFAULT_DRV);
	/*　ドライブ文字列不正の場合はZ:にする　*/
	if (DrvStr.Buffer[0] == UNICODE_NULL)
	{
		DrvStr.Buffer[0] = L'Z';
	}
	DrvStr.Buffer[1] = L':';
	DrvStr.Buffer[2] = UNICODE_NULL;
	DrvStr.Length = sizeof(DEFAULT_DRV) - sizeof(WCHAR);	/*　"Z:"で2文字　*/
	KdPrint(("Drive %ls\n", DrvStr.Buffer));
	/*　Win32デバイス名合成　*/
	RtlInitUnicodeString(&Win32Name, (PWSTR)WIN32_PATH);
	RtlCopyUnicodeString(&(pEramExt->Win32Name), &Win32Name);
	RtlAppendUnicodeStringToString(&(pEramExt->Win32Name), &DrvStr);
	/*　リンク作成　*/
	ntStat = IoCreateSymbolicLink(&(pEramExt->Win32Name), &NtDevName);
	if (ntStat != STATUS_SUCCESS)	/*　失敗　*/
	{
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_CREATE_SYMBOLIC_LINK_FAILED, NULL);
		/*　Win32名領域解放　*/
		ExFreePool(pEramExt->Win32Name.Buffer);
		pEramExt->Win32Name.Buffer = NULL;
	}
EramInitDiskExit:	/*　エラー時エントリ　*/
	if (ntStat != STATUS_SUCCESS)	/*　失敗　*/
	{
		if (pEramExt->uOptflag.Bits.External != 0)	/*　OS管理外メモリ使用　*/
		{
			/*　メモリ資源解放　*/
			ReleaseMemResource(pDrvObj, pEramExt);
		}
		if (pDevObj != NULL)	/*　デバイス作成済　*/
		{
			/*　デバイス削除　*/
			EramUnloadDevice(pDrvObj, pDevObj, pEramExt);
		}
	}
	KdPrint(("EramInitDisk end\n"));
	return ntStat;
}


/*　MemSetup
		メモリ領域の確保
	引数
		pDrvObj		装置の代表オブジェクトへのポインタ
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pFatId		FAT-ID構造体へのポインタ
		uMemSize	要求メモリ量
	戻り値
		結果
*/

NTSTATUS MemSetup(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId,
	IN ULONG			uMemSize
 )
{
	/*　ローカル変数　*/
	FILE_END_OF_FILE_INFORMATION	EofInfo;
	NTSTATUS						ntStat;
	IO_STATUS_BLOCK					IoStat;
	OBJECT_ATTRIBUTES				ObjAttr;
	UNICODE_STRING					uniStr;
	HANDLE							hThread;
	KdPrint(("MemSetup start\n"));
	if (pEramExt->uOptflag.Bits.External != 0)	/*　OS管理外メモリ　*/
	{
		/*　資源使用を通知　*/
		if (ExtReport(pDrvObj, pEramExt) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "ExtReport");
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		return STATUS_SUCCESS;
	}
	if (pEramExt->uOptflag.Bits.UseExtFile != 0)	/*　外部ファイル使用　*/
	{
		/*　とりあえずファイル名を用意　*/
		RtlInitUnicodeString(&uniStr, (PWSTR)(pFatId->wszExtFile));
		InitializeObjectAttributes(&ObjAttr, &uniStr, OBJ_CASE_INSENSITIVE, NULL, NULL);
		/*　ファイル開く　*/
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
		if (ntStat != STATUS_SUCCESS)	/*　失敗　*/
		{
			KdPrint(("ZwCreateFile failed, 0x%x\n", ntStat));
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_CREATE_EXT_FILE, NULL);
			return ntStat;
		}
		/*　ファイルサイズ確保　*/
		EofInfo.EndOfFile.QuadPart = (LONGLONG)uMemSize;
		ntStat = ZwSetInformationFile(pEramExt->hFile, &IoStat, &EofInfo, sizeof(EofInfo), FileEndOfFileInformation);
		if (ntStat != STATUS_SUCCESS)	/*　失敗　*/
		{
			KdPrint(("ZwSetInformationFile failed, 0x%x\n", ntStat));
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_SET_INFO_EXT_FILE, NULL);
			return ntStat;
		}
		/*　マッピングオブジェクト作成　*/
		ntStat = ZwCreateSection(&(pEramExt->hSection), SECTION_ALL_ACCESS, NULL, NULL, PAGE_READWRITE, SEC_COMMIT, pEramExt->hFile);
		if (ntStat != STATUS_SUCCESS)	/*　失敗　*/
		{
			KdPrint(("ZwCreateSection failed, 0x%x\n", ntStat));
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_CREATE_EXT_FILE_SECTION, NULL);
			return ntStat;
		}
		/*　スピンロック初期化　*/
		KeInitializeSpinLock(&(pEramExt->IrpSpin));
		/*　リスト初期化　*/
		InitializeListHead(&(pEramExt->IrpList));
		/*　セマフォ初期化　*/
		KeInitializeSemaphore(&(pEramExt->IrpSem), 0, MAXLONG);
		/*　システムスレッド作成　*/
		ntStat = PsCreateSystemThread(&hThread, THREAD_ALL_ACCESS, NULL, NULL, NULL, EramRwThread, pEramExt);
		if (ntStat != STATUS_SUCCESS)	/*　失敗　*/
		{
			KdPrint(("PsCreateSystemThread failed\n"));
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_CREATE_THREAD, NULL);
			return ntStat;
		}
		/*　スレッドオブジェクト取得　*/
		ntStat = ObReferenceObjectByHandle(hThread, THREAD_ALL_ACCESS, NULL, KernelMode, &(pEramExt->pThreadObject), NULL);
		if (ntStat != STATUS_SUCCESS)	/*　失敗　*/
		{
			KdPrint(("ObReferenceObjectByHandle failed\n"));
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_GET_THREAD_OBJECT, NULL);
			return ntStat;
		}
		/*　スレッドハンドル解放　*/
		ZwClose(hThread);
		/*　シャットダウン通知を有効化　*/
		IoRegisterShutdownNotification(pEramExt->pDevObj);
		return STATUS_SUCCESS;
	}
	/*　OS管理メモリ使用　*/
	if (OsAlloc(pDrvObj, pEramExt, uMemSize) == FALSE)
	{
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "OsAlloc");
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	KdPrint(("MemSetup end\n"));
	return STATUS_SUCCESS;
}


/*　OsAlloc
		OS管理メモリの確保
	引数
		pDrvObj		装置の代表オブジェクトへのポインタ
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		uMemSize	要求メモリ量
	戻り値
		結果
*/

BOOLEAN OsAlloc(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt,
	IN ULONG			uMemSize
 )
{
	/*　ローカル変数　*/
	POOL_TYPE	fPool;
	fPool = (pEramExt->uOptflag.Bits.NonPaged != 0) ? NonPagedPool : PagedPool;
	pEramExt->pPageBase = ExAllocatePool(fPool, uMemSize);
	if (pEramExt->pPageBase == NULL)	/*　確保失敗　*/
	{
		KdPrint(("ExAllocatePool failed, %d bytes, nonpaged=%d\n", uMemSize, (UINT)(pEramExt->uOptflag.Bits.NonPaged)));
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_DISK_ALLOC_FAILED, NULL);
		CalcAvailSize(pDrvObj, fPool, uMemSize);
		return FALSE;
	}
	return TRUE;
}


/*　CalcAvailSize
		確保できそうなメモリ量の報告
	引数
		pDrvObj		装置の代表オブジェクトへのポインタ
		fPool		メモリタイプ
		uMemSize	要求メモリ量
	戻り値
		なし
*/

VOID CalcAvailSize(
	IN PDRIVER_OBJECT	pDrvObj,
	IN POOL_TYPE		fPool,
	IN ULONG			uMemSize
 )
{
	/*　ローカル変数　*/
	PVOID			pBuf;
	UNICODE_STRING	UniStr;
	WCHAR			wcBuf[32];
	pBuf = NULL;
	while ((uMemSize > (DISKMINPAGE << PAGE_SIZE_LOG2))&&(pBuf == NULL))
	{
		/*　メモリ確保　*/
		uMemSize -= (DISKMINPAGE << PAGE_SIZE_LOG2);
		pBuf = ExAllocatePool(fPool, uMemSize);
	}
	if (pBuf == NULL)		/*　確保失敗　*/
	{
		return;
	}
	/*　メモリ解放　*/
	ExFreePool(pBuf);
	/*　75%くらいに制限　*/
	uMemSize = (uMemSize >> 2) * 3;
	/*　メモリ量を報告　*/
	wcBuf[0] = UNICODE_NULL;
	RtlInitUnicodeString(&UniStr, wcBuf);
	UniStr.MaximumLength = sizeof(wcBuf);
	if (RtlIntegerToUnicodeString(uMemSize >> 10, 10, &UniStr) == STATUS_SUCCESS)
	{
		EramReportEventW(pDrvObj, ERAMNT_INF_MEMORY_SIZE, UniStr.Buffer);
	}
}


/*　CheckSwapable
		レジストリの参照:スワップ可能デバイスにするかどうかの選択
	引数
		pRegParam	レジストリパス文字列へのポインタ
	戻り値
		デバイスタイプ
	レジストリパラメータ
		Option			オプション
*/

DEVICE_TYPE CheckSwapable(
	IN PUNICODE_STRING		pRegParam
 )
{
	/*　ローカル変数　*/
	RTL_QUERY_REGISTRY_TABLE	ParamTable[2];
	ULONG			Option,		defOption = 0;
	NTSTATUS		ntStat;
	ERAM_OPTFLAG	uOptflag;
	KdPrint(("CheckSwapable start\n"));
	/*　レジストリ確認領域初期化　*/
	RtlZeroBytes(&(ParamTable[0]), sizeof(ParamTable));
	/*　一括問い合わせ領域初期化(最後はNULL)　*/
	ParamTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
	ParamTable[0].DefaultType = REG_DWORD;
	ParamTable[0].DefaultLength = sizeof(ULONG);
	ParamTable[0].Name = (PWSTR)L"Option";
	ParamTable[0].EntryContext = &Option;
	ParamTable[0].DefaultData = &defOption;
	/*　レジストリ値問い合わせ　*/
	ntStat = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL, pRegParam->Buffer, &(ParamTable[0]), NULL, NULL);
	if (ntStat != STATUS_SUCCESS)	/*　失敗　*/
	{
		KdPrint(("Warning:RtlQueryRegistryValues failed\n"));
		/*　既定値を採用　*/
		Option = defOption;
	}
	uOptflag.dwOptflag = Option;
	if (uOptflag.Bits.Swapable != 0)		/*　スワップ可能な設定　*/
	{
		KdPrint(("CheckSwapable end, local disk\n"));
		/*　スワップされないようロックする　*/
#pragma warning(disable : 4054)
		MmLockPagableCodeSection((PVOID)EramCreateClose);
#pragma warning(default : 4054)
		/*　ローカルディスク扱いにする:スワップ可能　*/
		return FILE_DEVICE_DISK;
	}
	/*　RAMディスク扱いにする　*/
	KdPrint(("CheckSwapable end, virtual disk\n"));
	return FILE_DEVICE_VIRTUAL_DISK;
}


/*　CheckDeviceName
		レジストリの参照(デバイス名)
	引数
		pRegParam	レジストリパス文字列へのポインタ
		pNtDevName	NTデバイス名へのポインタ
	戻り値
		なし
	レジストリパラメータ
		DeviceName	デバイス名
*/

VOID CheckDeviceName(
	IN PUNICODE_STRING		pRegParam,
	IN OUT PUNICODE_STRING	pNtDevName
 )
{
	/*　ローカル変数　*/
	static WCHAR wszDef[] = L"";
	static WCHAR wszDev[32] = L"";		/*　\\Device\\～　*/
	RTL_QUERY_REGISTRY_TABLE	ParamTable[2];
	NTSTATUS		ntStat;
	UNICODE_STRING	UniDev;
	KdPrint(("CheckDeviceName start\n"));
	/*　問い合わせ用初期値準備　*/
	RtlInitUnicodeString(&UniDev, wszDev);
	UniDev.MaximumLength = sizeof(wszDev);
	/*　レジストリ確認領域初期化　*/
	RtlZeroBytes(&ParamTable, sizeof(ParamTable));
	/*　一括問い合わせ領域初期化(最後はNULL)　*/
	ParamTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
	ParamTable[0].Name = (PWSTR)L"DeviceName";
	ParamTable[0].EntryContext = &UniDev;
	ParamTable[0].DefaultType = REG_SZ;
	ParamTable[0].DefaultData = (LPWSTR)wszDef;
	ParamTable[0].DefaultLength = sizeof(wszDef);
	/*　レジストリ値一括問い合わせ　*/
	ntStat = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL, pRegParam->Buffer, &(ParamTable[0]), NULL, NULL);
	if (ntStat != STATUS_SUCCESS)	/*　失敗　*/
	{
		KdPrint(("Warning:RtlQueryRegistryValues failed, 0x%x\n", ntStat));
		return;
	}
	if (UniDev.Length == 0)		/*　指定無し　*/
	{
		KdPrint(("No value set\n"));
		return;
	}
	KdPrint(("CheckDeviceName end, device \"%ls\"\n", UniDev.Buffer));
	*pNtDevName = UniDev;
}


/*　CheckSwitch
		レジストリの参照とオプション設定
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pFatId		FAT-ID構造体へのポインタ
		pRegParam	レジストリパス文字列へのポインタ
		pDrvStr		ドライブ文字へのポインタ
	戻り値
		なし
	レジストリパラメータ
		AllocUnit		クラスタサイズ
		DriveLetter		ドライブ指定
		RootDirEntries	ルートディレクトリエントリ数
		MediaId			メディアID
		Option			オプション
		Page			4KBページ数
*/

VOID CheckSwitch(
	IN PERAM_EXTENSION		pEramExt,
	IN PFAT_ID				pFatId,
	IN PUNICODE_STRING		pRegParam,
	IN OUT PUNICODE_STRING	pDrvStr
 )
{
	/*　ローカル変数　*/
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
	KdPrint(("CheckSwitch start\n"));
	bDefault = TRUE;
	#define	REGOPTNUM	(8)
	#define	REGOPTSIZE	(REGOPTNUM * sizeof(*pParamTable))
	/*　問い合わせ用メモリ確保　*/
	pParamTable = ExAllocatePool(PagedPool, REGOPTSIZE);
	if (pParamTable != NULL)	/*　成功　*/
	{
		/*　レジストリ確認領域初期化　*/
		RtlZeroBytes(pParamTable, REGOPTSIZE);
		/*　一括問い合わせ領域初期化(最後はNULL)　*/
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
		/*　レジストリ値一括問い合わせ　*/
		ntStat = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL, pRegParam->Buffer, pParamTable, NULL, NULL);
		if (ntStat != STATUS_SUCCESS)	/*　失敗　*/
		{
			KdPrint(("Warning:RtlQueryRegistryValues failed\n"));
			bDefault = TRUE;
		}
		/*　問い合わせ用メモリ解放　*/
		ExFreePool(pParamTable);
	}
	if (bDefault != FALSE)	/*　完全に読めなかった　*/
	{
		/*　既定値を採用　*/
		AllocUnit = defAllocUnit;
		RootDir = defRootDir;
		MediaId = defMediaId;
		Option = defOption;
		Page = defPage;
		ExtStart = defExtStart;
	}
	#undef	REGOPTNUM
	#undef	REGOPTSIZE
	/*　アロケーションユニット検査　*/
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
	/*　ディレクトリエントリ設定　*/
	RootDir = (RootDir + 31) & 0xffe0;	/*　32の倍数にする　*/
	if (RootDir != 0)
	{
		pFatId->BPB.wNumDirectory = (WORD)RootDir;
	}
	/*　メディアIDセット　*/
	if (MediaId <= 0xff)
	{
		pFatId->BPB.byMediaId = (BYTE)MediaId;
	}
	/*　オプション　*/
	pEramExt->uOptflag.dwOptflag |= Option;	/*　オプション制御　*/
	/*　オプション補正　*/
	if (pEramExt->uOptflag.Bits.UseExtFile != 0)	/*　外部ファイル使用　*/
	{
		pEramExt->uOptflag.Bits.NonPaged = 0;
		pEramExt->uOptflag.Bits.External = 0;
	}
	else if (pEramExt->uOptflag.Bits.External != 0)	/*　OS管理外メモリ使用　*/
	{
		pEramExt->uOptflag.Bits.NonPaged = 0;
		pEramExt->uExternalStart = ExtStart;
		/*　OS管理外メモリ最大アドレスの準備　*/
		GetMaxAddress(pEramExt, pRegParam);
	}
	if ((WORD)NtBuildNumber >= BUILD_NUMBER_NT50)	/*　Windows2000以降　*/
	{
		/*　FAT32有効化　*/
		pEramExt->uOptflag.Bits.EnableFat32 = 1;
	}
	if (Page > LIMIT_4GBPAGES)
	{
		Page = LIMIT_4GBPAGES;
		KdPrint(("4GB limit over, adjust %d pages\n", Page));
	}
	/*　ページ設定　*/
	if (pEramExt->uOptflag.Bits.EnableFat32 == 0)		/*　FAT32使わない　*/
	{
		ulPageT = ((ULONGLONG)DISKMAXCLUSTER_16 * SECTOR * pFatId->BPB.byAllocUnit) / PAGE_SIZE_4K
;
		if ((ULONGLONG)Page > ulPageT)		/*　FAT16制限を超えている(若干余裕あり)　*/
		{
			Page = (ULONG)ulPageT;
			KdPrint(("FAT16 limit over, adjust %d pages\n", Page));
		}
	}
	else		/*　FAT32使う　*/
	{
		ulPageT = ((ULONGLONG)DISKMAXCLUSTER_32 * SECTOR * pFatId->BPB.byAllocUnit) / PAGE_SIZE_4K;
		if ((ULONGLONG)Page > ulPageT)		/*　FAT32制限を超えている　*/
		{
			Page = (ULONG)ulPageT;
			KdPrint(("FAT32 limit over, adjust %d pages\n", Page));
		}
	}
	pEramExt->uSizeTotal = Page;
	/*　ボリュームラベルの準備　*/
	PrepareVolumeLabel(pEramExt, pFatId, pRegParam);
	/*　外部ファイル名の準備　*/
	PrepareExtFileName(pEramExt, pFatId, pRegParam);
	KdPrint(("CheckSwitch end\n"));
}


/*　GetMaxAddress
		レジストリの参照(最大アドレス)
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pRegParam	レジストリパス文字列へのポインタ
	戻り値
		なし
	レジストリパラメータ
		MaxAddress	アクセス制限する最大アドレス
*/

VOID GetMaxAddress(
	IN PERAM_EXTENSION		pEramExt,
	IN PUNICODE_STRING		pRegParam
 )
{
	/*　ローカル変数　*/
	RTL_QUERY_REGISTRY_TABLE	ParamTable[2];
	ULONG			uMaxAdr,	defMaxAdr = 0xffffffff;
	NTSTATUS		ntStat;
	KdPrint(("GetMaxAddress start\n"));
	/*　レジストリ確認領域初期化　*/
	RtlZeroBytes(&(ParamTable[0]), sizeof(ParamTable));
	/*　一括問い合わせ領域初期化(最後はNULL)　*/
	ParamTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
	ParamTable[0].DefaultType = REG_DWORD;
	ParamTable[0].DefaultLength = sizeof(ULONG);
	ParamTable[0].Name = (PWSTR)L"MaxAddress";
	ParamTable[0].EntryContext = &uMaxAdr;
	ParamTable[0].DefaultData = &defMaxAdr;
	/*　レジストリ値問い合わせ　*/
	ntStat = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL, pRegParam->Buffer, &(ParamTable[0]), NULL, NULL);
	if (ntStat != STATUS_SUCCESS)	/*　失敗　*/
	{
		KdPrint(("Warning:RtlQueryRegistryValues failed\n"));
		/*　既定値を採用　*/
		uMaxAdr = defMaxAdr;
	}
	if (pEramExt->uExternalStart > uMaxAdr)
	{
		uMaxAdr = defMaxAdr;
	}
	pEramExt->uExternalEnd = uMaxAdr;
	KdPrint(("GetMaxAddress end\n"));
}


/*　PrepareVolumeLabel
		レジストリの参照(ボリュームラベル)
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pFatId		FAT-ID構造体へのポインタ
		pRegParam	レジストリパス文字列へのポインタ
	戻り値
		なし
	レジストリパラメータ
		VolumeLabel		ボリュームラベル
*/

VOID PrepareVolumeLabel(
	IN PERAM_EXTENSION		pEramExt,
	IN PFAT_ID				pFatId,
	IN PUNICODE_STRING		pRegParam
 )
{
	/*　ローカル変数　*/
	static WCHAR wszDef[] = L"";
	RTL_QUERY_REGISTRY_TABLE		ParamTable[2];
	NTSTATUS		ntStat;
	UNICODE_STRING	UniVol;
	WCHAR			wszVol[12];
	KdPrint(("PrepareVolumeLabel start\n"));
	/*　問い合わせ用初期値準備　*/
	wszVol[0] = UNICODE_NULL;
	RtlInitUnicodeString(&UniVol, wszVol);
	UniVol.MaximumLength = sizeof(wszVol);
	/*　レジストリ確認領域初期化　*/
	RtlZeroBytes(&ParamTable, sizeof(ParamTable));
	/*　一括問い合わせ領域初期化(最後はNULL)　*/
	ParamTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
	ParamTable[0].Name = (PWSTR)L"VolumeLabel";
	ParamTable[0].EntryContext = &UniVol;
	ParamTable[0].DefaultType = REG_SZ;
	ParamTable[0].DefaultData = (LPWSTR)wszDef;
	ParamTable[0].DefaultLength = sizeof(wszDef);
	/*　レジストリ値一括問い合わせ　*/
	ntStat = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL, pRegParam->Buffer, &(ParamTable[0]), NULL, NULL);
	if (ntStat != STATUS_SUCCESS)	/*　失敗　*/
	{
		KdPrint(("Warning:RtlQueryRegistryValues failed, 0x%x\n", ntStat));
	}
	/*　ボリュームラベル　*/
	RtlFillMemory(pFatId->bsLabel, sizeof(pFatId->bsLabel), ' ');
	if ((UniVol.Length == 0)||						/*　指定無し　*/
		(CheckVolumeLabel(pEramExt, pFatId, &UniVol) == FALSE))		/*　指定文字列無効　*/
	{
#pragma warning(disable : 4127)
		ASSERT((sizeof(pFatId->bsLabel)+1) == sizeof(VOLUME_LABEL_LOCALDISK));
		ASSERT((sizeof(pFatId->bsLabel)+1) == sizeof(VOLUME_LABEL_RAMDISK));
#pragma warning(default : 4127)
		RtlCopyBytes(pFatId->bsLabel, (PSTR)((pEramExt->uOptflag.Bits.Swapable != 0) ? VOLUME_LABEL_LOCALDISK : VOLUME_LABEL_RAMDISK), sizeof(pFatId->bsLabel));
	}
	KdPrint(("PrepareVolumeLabel end, Volume label \"%s\"\n", pFatId->bsLabel));
}


/*　CheckVolumeLabel
		ボリュームラベルの正当性確認と準備
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pFatId		FAT-ID構造体へのポインタ
		pUniVol		レジストリから読んだボリュームラベル文字列へのポインタ
	戻り値
		結果		TRUE:準備済み
*/

BOOLEAN CheckVolumeLabel(
	IN PERAM_EXTENSION		pEramExt,
	IN PFAT_ID				pFatId,
	IN PUNICODE_STRING		pUniVol
 )
{
	/*　ローカル変数　*/
	static CHAR cBadChars[] = "*?/|.,;:+=[]()&^<>\"";
	ANSI_STRING		AnsiVol;
	DWORD loopi, loopj;
	/*　不正文字検索　*/
	for (loopi=0; loopi<(pUniVol->Length / sizeof(WCHAR)); loopi++)
	{
		if (HIBYTE(pUniVol->Buffer[loopi]) == 0)
		{
			if (LOBYTE(pUniVol->Buffer[loopi]) < ' ')
			{
				KdPrint(("Bad char 0x%x detected, index %d\n", LOBYTE(pUniVol->Buffer[loopi]), loopi));
				return FALSE;
			}
			for (loopj=0; loopj<(sizeof(cBadChars)-1); loopj++)
			{
				if (LOBYTE(pUniVol->Buffer[loopi]) == cBadChars[loopj])
				{
					KdPrint(("Bad char \"%c\" detected, index %d\n", cBadChars[loopj], loopi));
					return FALSE;
				}
			}
		}
	}
	/*　ANSI文字列化　*/
	if (RtlUnicodeStringToAnsiString(&AnsiVol, pUniVol, TRUE) != STATUS_SUCCESS)
	{
		KdPrint(("RtlUnicodeStringToAnsiString failed\n"));
		return FALSE;
	}
	if (AnsiVol.Length == 0)		/*　実体無し　*/
	{
		KdPrint(("Ansi string 0 byte\n"));
		RtlFreeAnsiString(&AnsiVol);
		return FALSE;
	}
	/*　準備　*/
	RtlCopyBytes(pFatId->bsLabel, AnsiVol.Buffer, (AnsiVol.Length > sizeof(pFatId->bsLabel)) ? sizeof(pFatId->bsLabel) : AnsiVol.Length);
	RtlFreeAnsiString(&AnsiVol);
	KdPrint(("CheckVolumeLabel end, Volume label \"%s\"\n", pFatId->bsLabel));
	return TRUE;
}


/*　PrepareExtFileName
		レジストリの参照(外部ファイル名)
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pFatId		FAT-ID構造体へのポインタ
		pRegParam	レジストリパス文字列へのポインタ
	戻り値
		なし
	レジストリパラメータ
		ExtFileName		外部ファイル名
*/

VOID PrepareExtFileName(
	IN PERAM_EXTENSION		pEramExt,
	IN PFAT_ID				pFatId,
	IN PUNICODE_STRING		pRegParam
 )
{
	/*　ローカル変数　*/
	static WCHAR wszDef[] = L"";
	static WCHAR wszExtStub[] = L"\\??\\";
	static WCHAR wszExtPath[] = ERAMEXTFILEPATH;
	RTL_QUERY_REGISTRY_TABLE		ParamTable[2];
	NTSTATUS		ntStat;
	UNICODE_STRING	UniExtFile;
	KdPrint(("PrepareExtFileName start\n"));
	/*　問い合わせ用初期値準備　*/
	pFatId->wszExtFileMain[0] = UNICODE_NULL;
	RtlInitUnicodeString(&UniExtFile, pFatId->wszExtFileMain);
	UniExtFile.MaximumLength = sizeof(pFatId->wszExtFileMain);
	/*　レジストリ確認領域初期化　*/
	RtlZeroBytes(&ParamTable, sizeof(ParamTable));
	/*　一括問い合わせ領域初期化(最後はNULL)　*/
	ParamTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
	ParamTable[0].Name = (PWSTR)L"ExtFileName";
	ParamTable[0].EntryContext = &UniExtFile;
	ParamTable[0].DefaultType = REG_SZ;
	ParamTable[0].DefaultData = (LPWSTR)wszDef;
	ParamTable[0].DefaultLength = sizeof(wszDef);
	/*　レジストリ値一括問い合わせ　*/
	ntStat = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL, pRegParam->Buffer, &(ParamTable[0]), NULL, NULL);
	if (ntStat != STATUS_SUCCESS)	/*　失敗　*/
	{
		KdPrint(("Warning:RtlQueryRegistryValues failed\n"));
	}
	/*　外部ファイル名　*/
#pragma warning(disable : 4127)
	ASSERT(sizeof(pFatId->wszExtFile) == (sizeof(wszExtStub) - sizeof(WCHAR)));
#pragma warning(default : 4127)
	RtlCopyBytes(pFatId->wszExtFile, wszExtStub, sizeof(pFatId->wszExtFile));
	if (UniExtFile.Length == 0)			/*　外部ファイル名指定無し　*/
	{
		RtlCopyBytes(pFatId->wszExtFileMain, wszExtPath, sizeof(wszExtPath));
	}
	KdPrint(("PrepareExtFileName end, External file \"%ls\"\n", pFatId->wszExtFile));
}


/*　EramFormatFat
		ERAM初期化
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pFatId		FAT-ID構造体へのポインタ
	戻り値
		結果
*/

BOOLEAN EramFormatFat(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 )
{
	KdPrint(("EramFormatFat start\n"));
	/*　FAT情報セットアップ　*/
	EramSetup(pEramExt, pFatId);
	/*　ルーチンの動的配置　*/
	EramLocate(pEramExt);
	/*　ERAMフォーマット　*/
	if (EramFormat(pEramExt, pFatId) == FALSE)
	{
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "EramFormat");
		return FALSE;
	}
	KdPrint(("EramFormatFat end\n"));
	return TRUE;
}


/*　EramSetup
		ディスク情報セットアップ
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pFatId		FAT-ID構造体へのポインタ
	戻り値
		なし
*/

VOID EramSetup(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 )
{
	/*　ローカル変数　*/
	UINT	AllocLog2;
	DWORD	eax, esi, edi, ebx, edx, dwFatSectorCount, dwFatEntries;
	KdPrint(("EramSetup start\n"));
	/*　log2(ｱﾛｹｰｼｮﾝｻｲｽﾞ)計算　*/
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
	edi = pEramExt->uSizeTotal << PAGE_SEC_LOG2;
	pEramExt->uAllSector = edi;						/*　全ｾｸﾀ数　*/
	pFatId->BPB.wNumAllSector = (WORD)edi;			/*　全ｾｸﾀ数　*/
	if (edi >= 0x10000)								/*　over32MB　*/
	{
		pFatId->BPB.wNumAllSector = 0;
		pFatId->BPB_ext.bsHugeSectors = edi;			/*　全ｾｸﾀ数　*/
		if (pEramExt->uOptflag.Bits.EnableFat32 != 0)	/*　FAT32有効　*/
		{
			/*　BootSectorとFsInfoセクタを除いて予約エントリ2つを加える　*/
			dwFatEntries = ((edi - RESV_SECTOR_FAT32) >> AllocLog2) + 2;
			if (dwFatEntries > DISKMAXCLUSTER_16)		/*　クラスタ数大　*/
			{
				dwFatSectorCount = (dwFatEntries * 4 + (SECTOR - 1)) / SECTOR;
				dwFatEntries -= dwFatSectorCount;
				if (dwFatEntries > DISKMAXCLUSTER_16)		/*　クラスタ数大　*/
				{
					/*　FAT32使用を設定　*/
					pEramExt->FAT_size = PARTITION_FAT32;
					pFatId->BPB.wNumDirectory = 0;
					pFatId->BPB_fat32.dwNumFatSector32 = dwFatSectorCount;
					pFatId->BPB.wNumResvSector = RESV_SECTOR_FAT32;	/*　予約セクタ数(=BootSector+FsInfo)　*/
					KdPrint(("EramSetup end(FAT32)\n"));
					return;
				}
			}
		}
	}
	esi = pFatId->BPB.wNumDirectory;	/*　ﾃﾞｨﾚｸﾄﾘｴﾝﾄﾘ数　*/
	esi >>= (SECTOR_LOG2 - 5);			/*　1ｾｸﾀに16個入る…SI=ﾃﾞｨﾚｸﾄﾘｾｸﾀ数　*/
	if ((esi == 0)||					/*　ディレクトリ指定無し　*/
		((edi >> 1) <= esi))			/*　ディレクトリ指定が全セクタの半分を上回る　*/
	{
		pFatId->BPB.wNumDirectory = 128;						/*　強制的に既定値に変更　*/
		esi = pFatId->BPB.wNumDirectory >> (SECTOR_LOG2 - 5);	/*　1ｾｸﾀに16個入る…SI=ﾃﾞｨﾚｸﾄﾘｾｸﾀ数　*/
	}
	edi -= (esi + 1);					/*　di = 全ｾｸﾀ数 - ﾃﾞｨﾚｸﾄﾘｾｸﾀ数 - 予約ｾｸﾀ数　*/
	edx = edi;							/*　利用可能ｾｸﾀ数　*/
	edi >>= AllocLog2;					/*　概算ｸﾗｽﾀ数 = di / ｸﾗｽﾀ当りのｾｸﾀ　*/
	edi++;								/*　概算ｸﾗｽﾀ数+1　*/
	pEramExt->FAT_size = PARTITION_FAT_12;
	/*　AllocLog2 = log2 ｱﾛｹｰｼｮﾝ･ｻｲｽﾞ
		dx = 利用可能ｾｸﾀ数
		si = ﾙｰﾄﾃﾞｨﾚｸﾄﾘ用のｾｸﾀ数
		di = 概算ｸﾗｽﾀ数 + 1
	*/
	do
	{
		edi--;					/*　概算ｸﾗｽﾀ数-1　*/
		if (edi > DISKMAXCLUSTER_12)	/*　0FF7h以上なら16bit FAT　*/
		{
			pEramExt->FAT_size = PARTITION_FAT_16;
			eax = edi;
			eax <<= 1;			/*　eax=FATﾊﾞｲﾄ数(2倍)　*/
			eax += (SECTOR + 3);
			eax >>= SECTOR_LOG2;/*　eax=FAT ｾｸﾀ数　*/
			pFatId->BPB.wNumFatSector = (WORD)eax;
		}
		else	/*　12bit FAT　*/
		{
			pEramExt->FAT_size = PARTITION_FAT_12;
			eax = edi;
			eax *= 3;
			eax >>= 1;			/*　ax=FATﾊﾞｲﾄ数(1.5倍)　*/
			eax += (SECTOR + 2);
			eax >>= SECTOR_LOG2;	/*　ax=FAT ｾｸﾀ数　*/
			pFatId->BPB.wNumFatSector = (WORD)eax;
		}
		ebx = edi;				/*　bx = ﾃﾞｰﾀ･ｾｸﾀ数　*/
		ebx <<= AllocLog2;
		ebx += eax;				/*　+ FAT  ｾｸﾀ数　*/
	} while (ebx > edx);
	/*　12bit FATでの使用領域の調整
		0FF5h:FD が16bit FATと誤認
	*/
	if ((pEramExt->FAT_size == PARTITION_FAT_12)&&
		(edi > (DISKMAXCLUSTER_12 - 3)))
	{
		edi -= (DISKMAXCLUSTER_12 - 3);
		edi <<= AllocLog2;
		edi <<= (SECTOR_LOG2 - 5);
#pragma warning(disable : 4244)
		pFatId->BPB.wNumDirectory += (WORD)edi;	/*　ﾙｰﾄﾃﾞｨﾚｸﾄﾘ数で調整　*/
#pragma warning(default : 4244)
	}
	/*　16bit FATでの使用領域の調整
		0FFF6h:FAT32
	*/
	if ((pEramExt->FAT_size == PARTITION_FAT_16)&&
		(edi > DISKMAXCLUSTER_16 - 1))
	{
		edi -= DISKMAXCLUSTER_16 - 1;
		edi <<= AllocLog2;
		edi <<= (SECTOR_LOG2 - 5);
#pragma warning(disable : 4244)
		pFatId->BPB.wNumDirectory += (WORD)edi;	/*　ﾙｰﾄﾃﾞｨﾚｸﾄﾘ数で調整　*/
#pragma warning(default : 4244)
	}
	edx -= ebx;			/*　bx=実使用ｾｸﾀ数　*/
	if (edx != 0)		/*　端数ｾｸﾀはdirにまわす　*/
	{
		edx <<= (SECTOR_LOG2 - 5);
#pragma warning(disable : 4244)
		pFatId->BPB.wNumDirectory += (WORD)edx;
#pragma warning(default : 4244)
	}
	if ((pEramExt->FAT_size == PARTITION_FAT_16)&&
		(pFatId->BPB.wNumAllSector == 0))	/*　FAT16 over 32MB　*/
	{
		pEramExt->FAT_size = PARTITION_HUGE;
	}
	KdPrint(("EramSetup end(FAT12,16)\n"));
}


/*　EramLocate
		ルーチンの動的配置
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
	戻り値
		なし
*/

VOID EramLocate(
	IN PERAM_EXTENSION	pEramExt
 )
{
	KdPrint(("EramLocate start\n"));
	if (pEramExt->uOptflag.Bits.External != 0)	/*　OS管理外メモリ使用　*/
	{
		pEramExt->EramRead = (ERAM_READ)ExtRead1;
		pEramExt->EramWrite = (ERAM_WRITE)ExtWrite1;
		pEramExt->EramNext = (ERAM_NEXT)ExtNext1;
		pEramExt->EramUnmap = (ERAM_UNMAP)ExtUnmap;
	}
	else if (pEramExt->uOptflag.Bits.UseExtFile != 0)	/*　ファイル使用　*/
	{
		pEramExt->EramRead = (ERAM_READ)ExtFilePendingRw;
		pEramExt->EramWrite = (ERAM_WRITE)ExtFilePendingRw;
		pEramExt->EramNext = (ERAM_NEXT)ExtFileNext1;
		pEramExt->EramUnmap = (ERAM_UNMAP)ExtFileUnmap;
	}
	else		/*　通常　*/
	{
		pEramExt->EramRead = (ERAM_READ)ReadPool;
		pEramExt->EramWrite = (ERAM_WRITE)WritePool;
	}
	KdPrint(("EramLocate end\n"));
}


/*　EramFormat
		フォーマット
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pFatId		FAT-ID構造体へのポインタ
	戻り値
		結果
*/

BOOLEAN EramFormat(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 )
{
	KdPrint(("EramFormat start\n"));
	/*　管理領域の初期化　*/
	if (EramClearInfo(pEramExt, pFatId) == FALSE)
	{
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "EramClearInfo");
		return FALSE;
	}
	/*　FAT初期化　*/
	if (EramMakeFAT(pEramExt, pFatId) == FALSE)
	{
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "EramMakeFAT");
		return FALSE;
	}
	/*　ボリュームラベルをセット　*/
	if (EramSetLabel(pEramExt, pFatId) == FALSE)
	{
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "EramSetLabel");
		return FALSE;
	}
	KdPrint(("EramFormat end\n"));
	return TRUE;
}


/*　EramClearInfo
		領域クリア
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pFatId		FAT-ID構造体へのポインタ
	戻り値
		結果
*/

BOOLEAN EramClearInfo(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 )
{
	/*　ローカル変数　*/
	ULONG uSize;
	/*　管理領域サイズ計算　*/
	uSize = CalcEramInfoPage(pEramExt, pFatId);
	if (pEramExt->uOptflag.Bits.External != 0)	/*　OS管理外メモリ　*/
	{
		/*　OS管理外メモリ初期化　*/
		if (ExtClear(pEramExt, uSize) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "ExtClear");
			return FALSE;
		}
		return TRUE;
	}
	if (pEramExt->uOptflag.Bits.UseExtFile != 0)	/*　外部ファイル使用　*/
	{
		/*　ファイル初期化　*/
		if (ExtFileClear(pEramExt, uSize) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "ExtFileClear");
			return FALSE;
		}
		return TRUE;
	}
	/*　OS管理メモリ初期化　*/
	RtlZeroBytes(pEramExt->pPageBase, uSize);
	return TRUE;
}


/*　ExtClear
		OS管理外メモリ初期化
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		uSize		管理情報領域のバイト数
	戻り値
		結果
*/

BOOLEAN ExtClear(
	IN PERAM_EXTENSION	pEramExt,
	IN ULONG			uSize
 )
{
	/*　ローカル変数　*/
	ULONG loopi;
	KdPrint(("ExtClear start\n"));
	ASSERT(pEramExt->uExternalStart != 0);
	ASSERT(pEramExt->uExternalEnd != 0);
	for (loopi=0; loopi<uSize; loopi+=EXT_PAGE_SIZE)
	{
		if ((pEramExt->uExternalStart + loopi) >= pEramExt->uExternalEnd)	/*　実メモリを超えている　*/
		{
			KdPrint(("Warning:Address limited\n"));
			break;
		}
		/*　マップ　*/
		if (ExtMap(pEramExt, loopi) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "ExtMap");
			return FALSE;
		}
		//KdPrint(("loop 0x%x, phys 0x%X\n", loopi, (pEramExt->uExternalStart + loopi)));
		/*　0クリア　*/
		RtlZeroBytes(pEramExt->pExtPage, ((uSize - loopi) > EXT_PAGE_SIZE ? EXT_PAGE_SIZE : (uSize - loopi)));
	}
	/*　アンマップ　*/
	ExtUnmap(pEramExt);
	KdPrint(("ExtClear end\n"));
	return TRUE;
}


/*　ExtFileClear
		ファイル初期化
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		uSize		管理情報領域のバイト数
	戻り値
		結果
*/

BOOLEAN ExtFileClear(
	IN PERAM_EXTENSION	pEramExt,
	IN ULONG			uSize
 )
{
	/*　ローカル変数　*/
	ULONG loopi;
	KdPrint(("ExtFileClear start\n"));
	for (loopi=0; loopi<uSize; loopi+=EXT_PAGE_SIZE)
	{
		/*　マップ　*/
		if (ExtFileMap(pEramExt, loopi) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "ExtFileMap");
			return FALSE;
		}
		//KdPrint(("loop 0x%x, phys 0x%X\n", loopi, (pEramExt->uExternalStart + loopi)));
		/*　0クリア　*/
		RtlZeroBytes(pEramExt->pExtPage, ((uSize - loopi) > EXT_PAGE_SIZE ? EXT_PAGE_SIZE : (uSize - loopi)));
	}
	/*　アンマップ　*/
	ExtFileUnmap(pEramExt);
	KdPrint(("ExtFileClear end\n"));
	return TRUE;
}


/*　CalcEramInfoPage
		管理領域サイズ計算
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pFatId		FAT-ID構造体へのポインタ
	戻り値
		結果
*/

DWORD CalcEramInfoPage(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 )
{
	/*　ローカル変数　*/
	DWORD dwBytes, dwPage, dwTmp;
	KdPrint(("CalcEramInfoPage start\n"));
	if (pEramExt->FAT_size == PARTITION_FAT32)	/*　FAT32　*/
	{
		dwPage = pFatId->BPB_fat32.dwNumFatSector32 + pFatId->BPB.wNumResvSector + pFatId->BPB_fat32.dwRootCluster * (pFatId->BPB.byAllocUnit);
	}
	else		/*　FAT12/16　*/
	{
		dwPage = pFatId->BPB.wNumFatSector + pFatId->BPB_fat32.dwNumFatSector32;	/*　FATセクタ数　*/
		dwTmp = pFatId->BPB.wNumDirectory;	/*　ディレクトリエントリ数　*/
		dwTmp >>= (SECTOR_LOG2 - 5);			/*　ディレクトリセクタ数　*/
		dwPage += dwTmp;						/*　FAT＋ディレクトリ　*/
		dwPage += pFatId->BPB.wNumResvSector;	/*　予約セクタ数　*/
	}
	if (pEramExt->uOptflag.Bits.MakeTempDir != 0)	/*　TEMPディレクトリ作成　*/
	{
		/*　1クラスタ分増やす　*/
		dwPage += pFatId->BPB.byAllocUnit;
	}
	dwPage += (PAGE_SECTOR - 1);			/*　切り上げ用　*/
	dwPage >>= PAGE_SEC_LOG2;				/*　ページ数　*/
	dwBytes = (dwPage << PAGE_SIZE_LOG2);	/*　初期化バイト数　*/
	if (dwBytes > (pEramExt->uSizeTotal * PAGE_SIZE_4K))
	{
		dwBytes = pEramExt->uSizeTotal * PAGE_SIZE_4K;
	}
	KdPrint(("CalcEramInfoPage end, 0x%x bytes\n", dwBytes));
	return dwBytes;
}


/*　EramMakeFAT
		FAT作成
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pFatId		FAT-ID構造体へのポインタ
	戻り値
		結果
*/

BOOLEAN EramMakeFAT(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 )
{
	/*　ローカル変数　*/
	WORD wVal;
	PBYTE pDisk;
	PDWORD pdwFatSector;
	PBOOTSECTOR_FAT16 pBootFat16;
	PBOOTSECTOR_FAT32 pBootFat32;
	PFSINFO_SECTOR pFsInfoSector;
	LARGE_INTEGER SystemTime, LocalTime;
	DWORD eax, ebx;
	KdPrint(("EramMakeFAT start\n"));
	/*　日付、時刻を取得　*/
	KeQuerySystemTime(&SystemTime);						/*　システム時刻取得　*/
	ExSystemTimeToLocalTime(&SystemTime, &LocalTime);	/*　ローカル時刻に変換　*/
	RtlTimeToTimeFields(&LocalTime, &(pFatId->TimeInfo));	/*　構造体に変換　*/
	if ((pFatId->TimeInfo.Year < 1980)||(pFatId->TimeInfo.Year > 2079))	/*　年がDOSの範囲を越えた　*/
	{
		/*　2004年をセット　*/
		pFatId->TimeInfo.Year = 2004;
	}
	/*　ボリュームシリアル番号を準備　*/
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
	if ((pEramExt->uOptflag.Bits.External != 0)||		/*　OS管理外メモリ使用　*/
		(pEramExt->uOptflag.Bits.UseExtFile != 0))		/*　外部ファイル使用　*/
	{
		/*　ブートセクタ割り当て　*/
		ebx = 0;
		ASSERT((pEramExt->EramNext) != NULL);
		if ((*(pEramExt->EramNext))(pEramExt, &eax, &ebx) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "EramNext");
			return FALSE;
		}
		pDisk = (PBYTE)((ULONG)(pEramExt->pExtPage + eax));
	}
	/*　ブートセクタ共通部の書き込み　*/
	pBootFat16 = (PBOOTSECTOR_FAT16)pDisk;
	pBootFat16->bsJump[0] = 0xeb;
	pBootFat16->bsJump[1] = 0xfe;
	pBootFat16->bsJump[2] = 0x90;
	RtlCopyBytes(pBootFat16->bsOemName, "ERAM    ", sizeof(pBootFat16->bsOemName));
	RtlCopyBytes(&(pBootFat16->BPB), &(pFatId->BPB), sizeof(pBootFat16->BPB));
	RtlCopyBytes(&(pBootFat16->BPB_ext), &(pFatId->BPB_ext), sizeof(pBootFat16->BPB_ext));
	RtlCopyBytes(pBootFat16->szMsg, FATID_MSG, sizeof(FATID_MSG));
	if (pEramExt->FAT_size != PARTITION_FAT32)	/*　FAT12,16　*/
	{
		/*　ブートセクタ(FAT12,16)の書き込み　*/
		RtlCopyBytes(&(pBootFat16->BPB_ext2), &(pFatId->BPB_ext2), sizeof(pBootFat16->BPB_ext2));
	}
	else										/*　FAT32　*/
	{
		/*　ブートセクタ(FAT32)の書き込み　*/
		pBootFat32 = (PBOOTSECTOR_FAT32)pDisk;
		RtlCopyBytes(&(pBootFat32->BPB_fat32), &(pFatId->BPB_fat32), sizeof(pBootFat32->BPB_fat32));
		RtlCopyBytes(&(pBootFat32->BPB_ext2), &(pFatId->BPB_ext2), sizeof(pBootFat32->BPB_ext2));
		/*　FSINFOセクタの書き込み　*/
		pFsInfoSector = (PFSINFO_SECTOR)((ULONG)pBootFat32 + pBootFat32->BPB_fat32.wFsInfoSector * SECTOR);
		pFsInfoSector->FSInfo_Sig = 0x41615252;				/*　RRaA　*/
		pFsInfoSector->FsInfo.bfFSInf_Sig = 0x61417272;		/*　rrAa　*/
		pFsInfoSector->FsInfo.bfFSInf_free_clus_cnt = 0xffffffff;
		pFsInfoSector->FsInfo.bfFSInf_next_free_clus = 2;
		pFsInfoSector->bsSig2[0] = 0x55;
		pFsInfoSector->bsSig2[1] = 0xaa;
		if ((pBootFat32->BPB_fat32.wBackupBootSector != 0xffff)&&	/*　バックアップブートセクタ存在　*/
			(pBootFat32->BPB.wNumResvSector > (pBootFat32->BPB_fat32.wBackupBootSector + pBootFat32->BPB_fat32.wFsInfoSector))&&
			((pBootFat32->BPB_fat32.wBackupBootSector + pBootFat32->BPB_fat32.wFsInfoSector) < (EXT_PAGE_SIZE / SECTOR)))
		{
			/*　ブートセクタをバックアップ　*/
			RtlCopyBytes((PBYTE)((ULONG)pBootFat32 + pBootFat32->BPB_fat32.wBackupBootSector * SECTOR), pBootFat32, sizeof(*pBootFat32));
			/*　FSINFOセクタをバックアップ　*/
			RtlCopyBytes((PBYTE)((ULONG)pFsInfoSector + pBootFat32->BPB_fat32.wBackupBootSector * SECTOR), pFsInfoSector, sizeof(*pFsInfoSector));
		}
	}
	/*　FATセクタの書き込み　*/
	pdwFatSector = (PDWORD)((ULONG)pBootFat16 + pBootFat16->BPB.wNumResvSector * SECTOR);
	pdwFatSector[0] = 0xffffff00 + pFatId->BPB.byMediaId;
	if (pEramExt->FAT_size == PARTITION_FAT_12)	/*　FAT12　*/
	{
		if (pEramExt->uOptflag.Bits.MakeTempDir != 0)		/*　TEMP作成　*/
		{
			/*　クラスタ2を使用中にする(計36bit)　*/
			((PBYTE)pdwFatSector)[4] = 0xf;
		}
		else
		{
			/*　24bitに絞る　*/
			((PBYTE)pdwFatSector)[3] = 0;
		}
	}
	else if (pEramExt->FAT_size == PARTITION_FAT32)	/*　FAT32　*/
	{
		pdwFatSector[1] = 0xffffffff;
		/*　ルートディレクトリも設定　*/
		pdwFatSector[pFatId->BPB_fat32.dwRootCluster] = 0x0fffffff;
		if (pEramExt->uOptflag.Bits.MakeTempDir != 0)		/*　TEMP作成　*/
		{
			/*　ルートディレクトリの次のクラスタを使用中にする(計96bit)　*/
			pdwFatSector[pFatId->BPB_fat32.dwRootCluster + 1] = 0x0fffffff;
		}
	}
	else if (pEramExt->uOptflag.Bits.MakeTempDir != 0)		/*　FAT16でTEMP作成　*/
	{
		/*　クラスタ2を使用中にする(計48bit)　*/
		pdwFatSector[1] = 0xffff;
	}
	if (pEramExt->uOptflag.Bits.External != 0)	/*　OS管理外メモリ使用　*/
	{
		/*　アンマップ　*/
		ExtUnmap(pEramExt);
	}
	else if (pEramExt->uOptflag.Bits.UseExtFile != 0)	/*　外部ファイル使用　*/
	{
		/*　アンマップ　*/
		ExtFileUnmap(pEramExt);
	}
	KdPrint(("EramMakeFAT end\n"));
	return TRUE;
}


/*　EramSetLabel
		ボリュームラベルのセット
	引数
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pFatId		FAT-ID構造体へのポインタ
	戻り値
		結果
*/

BOOLEAN EramSetLabel(
	IN PERAM_EXTENSION	pEramExt,
	IN PFAT_ID			pFatId
 )
{
	/*　ローカル変数　*/
	vol_label VOL_L;
	dir_init DirInit;
	DWORD eax, dwDirSector, dwTempSector;
	KdPrint(("EramSetLabel start\n"));
	/*　初期化　*/
	RtlZeroBytes(&VOL_L, sizeof(VOL_L));
	RtlZeroBytes(&DirInit, sizeof(DirInit));
	/*　ボリュームラベルを準備　*/
	RtlCopyBytes(VOL_L.vol.sName, pFatId->bsLabel, sizeof(VOL_L.vol.sName));
	KdPrint(("Volume label \"%s\"\n", VOL_L.vol.sName));
	/*　属性を設定　*/
	VOL_L.vol.uAttr.Bits.byVol = 1;
	VOL_L.vol.uAttr.Bits.byA = 1;
	/*　日付、時刻を設定　*/
	VOL_L.vol.wUpdMinute |= (pFatId->TimeInfo.Second >> 1);				/*　秒は1ビット落ちる　*/
	VOL_L.vol.wUpdMinute |= (pFatId->TimeInfo.Minute << 5);				/*　分のセット　*/
	VOL_L.vol.wUpdMinute |= (pFatId->TimeInfo.Hour << (5+6));			/*　時のセット　*/
	VOL_L.vol.wUpdDate |= pFatId->TimeInfo.Day;
	VOL_L.vol.wUpdDate |= (pFatId->TimeInfo.Month << 5);
	VOL_L.vol.wUpdDate |= ((pFatId->TimeInfo.Year - 1980) << (5+4));
	/*　ディレクトリセクタ位置を取得　*/
	dwDirSector = pFatId->BPB.wNumFatSector + pFatId->BPB.wNumResvSector;
	if (pEramExt->FAT_size == PARTITION_FAT32)		/*　FAT32　*/
	{
		dwDirSector = pFatId->BPB_fat32.dwNumFatSector32 + pFatId->BPB.wNumResvSector + (pFatId->BPB_fat32.dwRootCluster - 2) * (pFatId->BPB.byAllocUnit);
	}
	if (pEramExt->uOptflag.Bits.MakeTempDir != 0)		/*　TEMPディレクトリ作成　*/
	{
		/*　ディレクトリを準備　*/
		RtlCopyBytes(VOL_L.temp.sName, TEMPDIR_NAME, sizeof(VOL_L.temp.sName));
		/*　属性を設定　*/
		VOL_L.temp.uAttr.Bits.byDir = 1;
		VOL_L.temp.uAttr.Bits.byA = 1;
		/*　日付、時刻を設定　*/
		VOL_L.temp.wUpdMinute = VOL_L.vol.wUpdMinute;
		VOL_L.temp.wUpdDate = VOL_L.vol.wUpdDate;
		if (pEramExt->FAT_size == PARTITION_FAT32)		/*　FAT32　*/
		{
			/*　先頭クラスタ番号を設定　*/
			VOL_L.temp.wCluster = (WORD)(pFatId->BPB_fat32.dwRootCluster + 1);
			/*　先頭セクタ数を計算　*/
			dwTempSector = pFatId->BPB_fat32.dwNumFatSector32 + pFatId->BPB.wNumResvSector + (VOL_L.temp.wCluster - 2) * (pFatId->BPB.byAllocUnit);
		}
		else
		{
			/*　先頭クラスタ番号を設定　*/
			VOL_L.temp.wCluster = 2;
			/*　先頭セクタ数を計算　*/
			dwTempSector = dwDirSector + (pFatId->BPB.wNumDirectory >> (SECTOR_LOG2 - 5));
		}
		/*　ディレクトリを準備　*/
		RtlCopyBytes(DirInit.own.sName, OWNDIR_NAME, sizeof(DirInit.own.sName));
		RtlCopyBytes(DirInit.parent.sName, PARENTDIR_NAME, sizeof(DirInit.parent.sName));
		/*　属性を設定　*/
		DirInit.own.uAttr.byAttr = VOL_L.temp.uAttr.byAttr;
		DirInit.parent.uAttr.byAttr = VOL_L.temp.uAttr.byAttr;
		/*　日付、時刻を設定　*/
		DirInit.own.wUpdMinute = VOL_L.vol.wUpdMinute;
		DirInit.own.wUpdDate = VOL_L.vol.wUpdDate;
		DirInit.parent.wUpdMinute = VOL_L.vol.wUpdMinute;
		DirInit.parent.wUpdDate = VOL_L.vol.wUpdDate;
		/*　先頭クラスタ番号を設定　*/
		DirInit.own.wCluster = VOL_L.temp.wCluster;
	}
	if ((pEramExt->uOptflag.Bits.External != 0)||		/*　OS管理外メモリ使用　*/
		(pEramExt->uOptflag.Bits.UseExtFile != 0))		/*　外部ファイル使用　*/
	{
		/*　ディレクトリセクタを割り当て　*/
		ASSERT((pEramExt->EramNext) != NULL);
		if ((*(pEramExt->EramNext))(pEramExt, &eax, &dwDirSector) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "EramNext");
			return FALSE;
		}
		/*　ボリュームラベル書き込み　*/
		RtlCopyBytes((LPBYTE)((ULONG)(pEramExt->pExtPage) + eax), &VOL_L, sizeof(VOL_L));
		if (pEramExt->uOptflag.Bits.MakeTempDir != 0)		/*　TEMPディレクトリ作成　*/
		{
			/*　TEMPディレクトリセクタを割り当て　*/
			if ((*(pEramExt->EramNext))(pEramExt, &eax, &dwTempSector) == FALSE)
			{
				EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "EramNext");
				return FALSE;
			}
			/*　ディレクトリ書き込み　*/
			RtlCopyBytes((LPBYTE)((ULONG)(pEramExt->pExtPage) + eax), &DirInit, sizeof(DirInit));
		}
		/*　アンマップ　*/
		ASSERT((pEramExt->EramUnmap) != NULL);
		(*(pEramExt->EramUnmap))(pEramExt);
	}
	else
	{
		/*　ディレクトリセクタにボリュームラベル書き込み　*/
		RtlCopyBytes((LPBYTE)((ULONG)(pEramExt->pPageBase) + (dwDirSector << SECTOR_LOG2)), &VOL_L, sizeof(VOL_L));
		if (pEramExt->uOptflag.Bits.MakeTempDir != 0)		/*　TEMPディレクトリ作成　*/
		{
			/*　TEMPディレクトリセクタにディレクトリ書き込み　*/
			RtlCopyBytes((LPBYTE)((ULONG)(pEramExt->pPageBase) + (dwTempSector << SECTOR_LOG2)), &DirInit, sizeof(DirInit));
		}
	}
	KdPrint(("EramSetLabel end\n"));
	return TRUE;
}


/*　GetExternalStart
		OS管理外メモリ開始位置検出
	引数
		pDrvObj		装置の代表オブジェクトへのポインタ
		pEramExt	ERAM_EXTENTION構造体へのポインタ
	戻り値
		結果	TRUE:OS管理外メモリあり
*/

BOOLEAN GetExternalStart(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt
 )
{
	/*　ローカル変数　*/
	RTL_QUERY_REGISTRY_TABLE	ParamTable[2];
	NTSTATUS					ntStat;
	UNICODE_STRING				uniOption, uniOptionUp;
	PWSTR						pBuf, pwStr;
	ULONG						uSize, uMaxMem, loopi, uRemain, uNoLowMem, uStart;
	BOOLEAN						bStat, bNoLowMem;
	PHYSICAL_ADDRESS			phys;
	static WCHAR		szwMaxMem[] = L"MAXMEM=";
	static WCHAR		szwNoLowMem[] = L"NOLOWMEM";
	KdPrint(("GetExternalStart start\n"));
	uSize = 512 * sizeof(WCHAR);
	pBuf = ExAllocatePool(PagedPool, uSize);
	if (pBuf == NULL)		/*　確保失敗　*/
	{
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_OPTION_WORK_ALLOC_FAILED, NULL);
		return FALSE;
	}
	/*　情報取得領域準備　*/
	RtlInitUnicodeString(&uniOption, UNICODE_NULL);
	uniOption.Buffer = pBuf;
	uniOption.MaximumLength = (USHORT)uSize;
	/*　レジストリ確認領域初期化　*/
	RtlZeroBytes(&(ParamTable[0]), sizeof(ParamTable));
	ParamTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
	ParamTable[0].Name = (PWSTR)L"SystemStartOptions";
	ParamTable[0].EntryContext = &uniOption;
	/*　レジストリ値一括問い合わせ　*/
	ntStat = RtlQueryRegistryValues(RTL_REGISTRY_CONTROL, NULL, &(ParamTable[0]), NULL, NULL);
	if (ntStat != STATUS_SUCCESS)	/*　失敗　*/
	{
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_OPTION_GET_FAILED, NULL);
		/*　メモリ解放　*/
		ExFreePool(pBuf);
		return FALSE;
	}
	if (uniOption.Length == 0)	/*　オプション無し　*/
	{
		KdPrint(("No startup option\n"));
		/*　メモリ解放　*/
		ExFreePool(pBuf);
		return FALSE;
	}
	/*　物理アドレスから/PAE判断　*/
	uNoLowMem = 0;
	phys = MmGetPhysicalAddress(pBuf);
	if (phys.HighPart != 0)		/*　over4GB　*/
	{
		/*　少なくとも/PAEは存在しているので、/NOLOWMEMを探す　*/
		uNoLowMem = sizeof(szwNoLowMem) - sizeof(WCHAR);
	}
	uMaxMem = sizeof(szwMaxMem) - sizeof(WCHAR);
	if (max(uMaxMem, uNoLowMem) >= uniOption.Length)	/*　オプションにMAXMEM/NOLOWMEMは含まれない　*/
	{
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_MAXMEM_NO_OPTION, NULL);
		/*　メモリ解放　*/
		ExFreePool(pBuf);
		return FALSE;
	}
	if (uNoLowMem == 0)
	{
		uNoLowMem = MAXDWORD;
	}
	KdPrint(("Startup option exist\n"));
	/*　大文字変換(NT4では既にされているようだ)　*/
	if (RtlUpcaseUnicodeString(&uniOptionUp , &uniOption, TRUE) != STATUS_SUCCESS)
	{
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_MAXMEM_CAPITAL_FAILED, NULL);
		/*　メモリ解放　*/
		ExFreePool(pBuf);
		return FALSE;
	}
	/*　メモリ解放　*/
	ExFreePool(pBuf);
	KdPrint(("Start Parse\n"));
	pwStr = uniOptionUp.Buffer;
	bStat = FALSE;
	bNoLowMem = FALSE;
	uStart = 0;
	uRemain = uniOptionUp.Length;
	for (loopi=0; loopi<(uniOptionUp.Length /sizeof(WCHAR)); loopi++, pwStr++, uRemain-=sizeof(WCHAR))
	{
		if ((uMaxMem < uRemain)&&
			(RtlCompareMemory(pwStr, szwMaxMem, uMaxMem) == uMaxMem))	/*　一致　*/
		{
			KdPrint(("Match, MAXMEM\n"));
			pwStr += (uMaxMem / sizeof(WCHAR));
			/*　MAXMEM=nのnを取得　*/
			if (GetMaxMem(pDrvObj, pEramExt, pwStr, &uStart) == FALSE)
			{
				EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "GetMaxMem");
			}
			if (uNoLowMem == MAXDWORD)
			{
				break;
			}
		}
		if ((uNoLowMem < uRemain)&&
			(RtlCompareMemory(pwStr, szwNoLowMem, uNoLowMem) == uNoLowMem))	/*　一致　*/
		{
			KdPrint(("Match, NOLOWMEM\n"));
			bNoLowMem = TRUE;
			break;
		}
	}
	KdPrint(("loop end\n"));
	if (bNoLowMem != FALSE)		/*　/NOLOWMEM　*/
	{
		/*　17MBあたりから使ってみる　*/
		uStart = 17;
	}
	if (uStart != 0)	/*　MAXMEM=nまたはNOLOWMEMあり　*/
	{
		bStat = CheckMaxMem(pDrvObj, pEramExt, uStart);
		if (bStat == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "CheckMaxMem");
		}
	}
	/*　メモリ解放　*/
	RtlFreeUnicodeString(&uniOptionUp);
	if (bStat == FALSE)
	{
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_MAXMEM_INVALID, NULL);
	}
	KdPrint(("GetExternalStart end\n"));
	return bStat;
}


/*　GetMaxMem
		MAXMEM=nオプション解析
	引数
		pDrvObj		装置の代表オブジェクトへのポインタ
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		pwStr		MAXMEM=nのnの文字列へのポインタ
		puSize		nを格納する領域へのポインタ
	戻り値
		結果
*/

BOOLEAN GetMaxMem(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt,
	IN PWSTR			pwStr,
	OUT PULONG			puSize
 )
{
	/*　ローカル変数　*/
	UNICODE_STRING	uniOptionMem;
	PWSTR			pwStrSp;
	pwStrSp = pwStr;
	while ((*pwStrSp != L' ')&&(*pwStrSp != L'\0'))
	{
		pwStrSp++;
	}
	*pwStrSp = L'\0';
	RtlInitUnicodeString(&uniOptionMem, pwStr);
	/*　数値化　*/
	if (RtlUnicodeStringToInteger(&uniOptionMem, 0, puSize) != STATUS_SUCCESS)
	{
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_MAXMEM_ATOU, NULL);
		return FALSE;
	}
	return TRUE;
}


/*　CheckMaxMem
		MAXMEM=nオプション解析
	引数
		pDrvObj		装置の代表オブジェクトへのポインタ
		pEramExt	ERAM_EXTENTION構造体へのポインタ
		uSize		MAXMEM=nのn
	戻り値
		結果	TRUE:OS管理外メモリあり
*/

BOOLEAN CheckMaxMem(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt,
	IN ULONG			uSize
 )
{
	if (uSize <= 16)		/*　小さすぎ　*/
	{
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_MAXMEM_TOO_SMALL, NULL);
		return FALSE;
	}
	if (uSize >= 4095)		/*　大きすぎ　*/
	{
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_MAXMEM_TOO_BIG, NULL);
		return FALSE;
	}
	/*　MB単位補正　*/
	pEramExt->uExternalStart /= SIZE_MEGABYTE;
	if (pEramExt->uExternalStart >= 4095)
	{
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_EXTSTART_TOO_BIG, NULL);
		return FALSE;
	}
	if (pEramExt->uExternalStart >= uSize)	/*　後ろ側に補正必要　*/
	{
		/*　後ろ側に補正　*/
		KdPrint(("System %dMB\n", uSize));
		uSize = pEramExt->uExternalStart;
	}
	pEramExt->uExternalStart = uSize * SIZE_MEGABYTE;
	KdPrint(("System %dMB, External start 0x%x\n", uSize, pEramExt->uExternalStart));
	uSize = pEramExt->uExternalStart / PAGE_SIZE_4K;
	if ((uSize + pEramExt->uSizeTotal) > (4096 * (SIZE_MEGABYTE / PAGE_SIZE_4K)))	/*　over4GB　*/
	{
		/*　4GBに収める(末尾1MBは除外…)　*/
		pEramExt->uSizeTotal = (4096 * (SIZE_MEGABYTE / PAGE_SIZE_4K)) - uSize;
	}
	return TRUE;
}


/*　CheckExternalSize
		OS管理外メモリ開始位置検出
	引数
		pDrvObj		装置の代表オブジェクトへのポインタ
		pEramExt	ERAM_EXTENTION構造体へのポインタ
	戻り値
		結果		TRUE:OS管理外メモリあり
*/

BOOLEAN CheckExternalSize(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt
 )
{
	/*　ローカル変数　*/
	CM_RESOURCE_LIST	ResList;	/*　リソースリスト　*/
	ULONG				uSize, uRealSize;
	BOOLEAN				bResConf, bStat;
	NTSTATUS			ntStat;
	DWORD				dwMaxAddr;
	ULARGE_INTEGER		ulMix;
	KdPrint(("CheckExternalSize start\n"));
	/*　確保すべきバイト数を計算　*/
	uSize = pEramExt->uSizeTotal * PAGE_SIZE_4K;
	if (uSize == 0)		/*　不正　*/
	{
		KdPrint(("Total is 0\n"));
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_DISK_SIZE_IS_0, NULL);
		return FALSE;
	}
	ulMix.QuadPart = (ULONGLONG)(pEramExt->uExternalStart) + (ULONGLONG)uSize;
	if (ulMix.HighPart != 0)	/*　over4GB　*/
	{
		uSize = (DWORD)(0 - (pEramExt->uExternalStart));
		pEramExt->uSizeTotal = uSize / PAGE_SIZE_4K;
		KdPrint(("Wrap 4GB, limit size 0x%x (%dpages)\n", uSize, pEramExt->uSizeTotal));
	}
	/*　ACPI予約メモリ推測　*/
	dwMaxAddr = GetAcpiReservedMemory(pDrvObj);
	KdPrint(("ACPI max 0x%x\n", dwMaxAddr));
	if (pEramExt->uExternalStart >= dwMaxAddr)	/*　開始位置がACPIメモリより後　*/
	{
		KdPrint(("Invalid start address\n"));
		EramReportEvent(pDrvObj, ERAMNT_ERROR_MAXMEM_NO_MEMORY, NULL);
		return FALSE;
	}
	if (ulMix.LowPart > dwMaxAddr)	/*　ACPIと重なる　*/
	{
		/*　ACPIの前までに制限　*/
		uSize -= (ulMix.LowPart - dwMaxAddr);
		pEramExt->uSizeTotal = uSize / PAGE_SIZE_4K;
		ulMix.LowPart = dwMaxAddr;
		KdPrint(("Wrap ACPI, limit size 0x%x (%dpages)\n", uSize, pEramExt->uSizeTotal));
	}
	/*　リソース要求設定　*/
	ResourceInitTiny(pDrvObj, &ResList, pEramExt->uExternalStart, uSize);
	bResConf = FALSE;
	/*　資源が使用可能かどうか調べる　*/
	if (pEramExt->uOptflag.Bits.SkipReportUsage == 0)
	{
		ntStat = IoReportResourceUsage(NULL, pDrvObj, &ResList, sizeof(ResList), NULL, NULL, 0, TRUE, &bResConf);
		if (ntStat != STATUS_SUCCESS)	/*　失敗　*/
		{
			KdPrint(("IoReportResourceUsage failed, %x\n", ntStat));
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_MAXMEM_REPORT_USAGE_FAILED, NULL);
			return FALSE;
		}
	}
	bStat = FALSE;
	if (bResConf != FALSE)	/*　競合あり　*/
	{
		KdPrint(("Conflict\n"));
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_MAXMEM_REPORT_USAGE_CONFLICT, NULL);
	}
	else
	{
		KdPrint(("No conflict\n"));
		if (CheckExternalMemoryExist(pDrvObj, pEramExt->uExternalStart, uSize, &uRealSize, dwMaxAddr) == FALSE)
		{
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "CheckExternalMemoryExist");
		}
		else
		{
			KdPrint(("extend memory 0x%08X - 0x%08X (0x%X bytes)\n", pEramExt->uExternalStart, (pEramExt->uExternalStart)+uRealSize-1, uRealSize));
			if (uSize > uRealSize)		/*　メモリ不足　*/
			{
				/*　実メモリの範囲にとどめる　*/
				pEramExt->uSizeTotal = uRealSize / PAGE_SIZE_4K;
				KdPrint(("size compress\n"));
				EramReportEvent(pEramExt->pDevObj, ERAMNT_WARN_MAXMEM_DISK_SIZE_FIXED, NULL);
			}
			bStat = TRUE;
		}
	}
	/*　ドライバ資源解放　*/
	if (pEramExt->uOptflag.Bits.SkipReportUsage == 0)
	{
		RtlZeroBytes(&ResList, sizeof(ResList));
		IoReportResourceUsage(NULL, pDrvObj, &ResList, sizeof(ResList), NULL, NULL, 0, FALSE, &bResConf);
	}
	KdPrint(("CheckExternalSize end\n"));
	return bStat;
}


/*　ResourceInitTiny
		リソースマップ初期化
	引数
		pDrvObj		装置の代表オブジェクトへのポインタ
		pResList	リソース構造体へのポインタ
		uStart		OS管理外メモリ開始位置
		uSize		ディスクに回す容量
	戻り値
		なし
*/

VOID ResourceInitTiny(
	IN PDRIVER_OBJECT		pDrvObj,
	IN PCM_RESOURCE_LIST	pResList,
	IN ULONG				uStart,
	IN ULONG				uSize
 )
{
	/*　ローカル変数　*/
	PCM_PARTIAL_RESOURCE_DESCRIPTOR	pDesc;
	PHYSICAL_ADDRESS				PortAdr;
	KdPrint(("ResourceInitTiny start\n"));
	PortAdr.HighPart = 0;
	/*　ヘッダ部設定　*/
	pResList->Count = 1;	/*　Internalのみ　*/
	pResList->List[0].InterfaceType = Internal;
	pResList->List[0].BusNumber = 0;
	pResList->List[0].PartialResourceList.Count = 1;	/*　メモリ　*/
	/*　ディスクリプタ先頭へのポインタを取得　*/
	pDesc = &(pResList->List[0].PartialResourceList.PartialDescriptors[0]);
	/*　内部メモリ要求のための設定　*/
	pDesc->Type = CmResourceTypeMemory;
	pDesc->ShareDisposition = CmResourceShareDriverExclusive;
	pDesc->Flags = CM_RESOURCE_MEMORY_READ_WRITE;
	PortAdr.LowPart  = uStart;
	pDesc->u.Memory.Start = PortAdr;
	pDesc->u.Memory.Length = uSize;
	KdPrint(("ResourceInitTiny end\n"));
}


/*　CheckExternalMemoryExist
		メモリ検査本体
	引数
		pDrvObj		装置の代表オブジェクトへのポインタ
		uStart		OS管理外メモリ開始位置
		uDiskSize	ディスクに回す容量
		puSize		メモリ容量
		dwMaxAddr	ACPI使用メモリの最下位アドレス
	戻り値
		結果
*/

BOOLEAN CheckExternalMemoryExist(
	IN PDRIVER_OBJECT	pDrvObj,
	IN ULONG			uStart,
	IN ULONG			uDiskSize,
	OUT PULONG			puSize,
	IN ULONG			dwMaxAddr
 )
{
	/*　ローカル変数　*/
	ULONG				loopi, loopj;
	BOOLEAN				bExist;
	PHYSICAL_ADDRESS	MapAdr;
	volatile PBYTE		pBase;
	static BYTE			byTests[] = { 0, 0x55, 0xaa, 0 };
	KdPrint(("CheckExternalMemoryExist start\n"));
	/*　資源使用設定　*/
	if (ResourceSetupTiny(pDrvObj, uStart, &MapAdr) == FALSE)
	{
		KdPrint(("ResourceSetupTiny failed\n"));
		EramReportEvent(pDrvObj, ERAMNT_ERROR_FUNCTIONERROR, "ResourceSetupTiny");
		return FALSE;
	}
	KdPrint(("Memory check start\n"));
	*puSize = 0;
	for (loopi=0; loopi<uDiskSize; loopi+=SIZE_MEGABYTE)
	{
		/*　マップ　*/
		pBase = (PBYTE)MmMapIoSpace(MapAdr, PAGE_SIZE_4K, FALSE);
		if (pBase == NULL)	/*　マップ以外orマップ失敗　*/
		{
			EramReportEvent(pDrvObj, ERAMNT_ERROR_MAXMEM_MAP_FAILED, NULL);
			return FALSE;
		}
		/*　RAM存在検査　*/
		bExist = TRUE;
		for (loopj=0; loopj<sizeof(byTests); loopj++)
		{
			*pBase = byTests[loopj];
			if (*pBase != byTests[loopj])		/*　値が異なる…RAM不在　*/
			{
				bExist = FALSE;
				break;
			}
		}
		/*　アンマップ　*/
		MmUnmapIoSpace(pBase, PAGE_SIZE_4K);
		if (bExist == FALSE)	/*　不在　*/
		{
			break;
		}
		if ((uStart + loopi + SIZE_MEGABYTE) >= dwMaxAddr)	/*　1MBは空き無い　*/
		{
			KdPrint(("ACPI memory detected, adjusted\n"));
			*puSize += (dwMaxAddr - (uStart + loopi));
			break;
		}
		/*　次の1MBに進める　*/
		*puSize += SIZE_MEGABYTE;
		MapAdr.LowPart += SIZE_MEGABYTE;
	}
	if (*puSize == 0)		/*　検出できず　*/
	{
		KdPrint(("extend memory 0 bytes\n"));
		EramReportEvent(pDrvObj, ERAMNT_ERROR_MAXMEM_NO_MEMORY, NULL);
		return FALSE;
	}
	KdPrint(("CheckExternalMemoryExist end, %dKB(=%dMB) detected\n", (*puSize) / SIZE_KILOBYTE, (*puSize) / SIZE_MEGABYTE));
	return TRUE;
}


/*　ResourceSetupTiny
		I/Oリソース初期設定
	引数
		pDrvObj		装置の代表オブジェクトへのポインタ
		uStart		OS管理外メモリ開始位置
		pMapAdr		変換アドレスへのポインタ
	戻り値
		結果
*/

BOOLEAN ResourceSetupTiny(
	IN PDRIVER_OBJECT		pDrvObj,
	IN ULONG				uStart,
	OUT PPHYSICAL_ADDRESS	pMapAdr
 )
{
	/*　ローカル変数　*/
	PHYSICAL_ADDRESS	PortAdr;
	ULONG				MemType;
	KdPrint(("ResourceSetupTiny start\n"));
	PortAdr.HighPart = 0;
	/*　内部メモリ設定　*/
	MemType = 0;			/*　メモリ空間　*/
	PortAdr.LowPart  = uStart;
	/*　一応Translateしているが結果は同じ模様　*/
	if (HalTranslateBusAddress(Internal, 0, PortAdr, &MemType, pMapAdr) == FALSE)
	{
		KdPrint(("Memory 0x%x, HalTranslateBusAddress failed\n", PortAdr.LowPart));
		EramReportEvent(pDrvObj, ERAMNT_ERROR_TRANSLATE_ADDRESS_FAILED, NULL);
		return FALSE;
	}
	if (MemType != 0)		/*　マップ　*/
	{
		KdPrint(("!Map type\n"));
		EramReportEvent(pDrvObj, ERAMNT_ERROR_PORT_MAPPED, NULL);
		return FALSE;
	}
	KdPrint(("ResourceSetupTiny end\n"));
	return TRUE;
}


/*　ExtReport
		OS管理外メモリ開始位置検出
	引数
		pDrvObj		装置の代表オブジェクトへのポインタ
		pEramExt	ERAM_EXTENTION構造体へのポインタ
	戻り値
		結果
*/

BOOLEAN ExtReport(
	IN PDRIVER_OBJECT	pDrvObj,
	IN PERAM_EXTENSION	pEramExt
 )
{
	/*　ローカル変数　*/
	CM_RESOURCE_LIST	ResList;	/*　リソースリスト　*/
	ULONG				uSize;
	BOOLEAN				bResConf;
	NTSTATUS			ntStat;
	KdPrint(("ExtReport start\n"));
	/*　確保すべきバイト数を計算　*/
	uSize = pEramExt->uSizeTotal * PAGE_SIZE_4K;
	if (uSize == 0)		/*　不正　*/
	{
		KdPrint(("Total is 0\n"));
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_DISK_SIZE_IS_0, NULL);
		return FALSE;
	}
	/*　リソース要求設定　*/
	ResourceInitTiny(pDrvObj, &ResList, pEramExt->uExternalStart, uSize);
	/*　資源使用を通知　*/
	if (pEramExt->uOptflag.Bits.SkipReportUsage == 0)
	{
		ntStat = IoReportResourceUsage(NULL, pDrvObj, &ResList, sizeof(ResList), NULL, NULL, 0, FALSE, &bResConf);
		if ((ntStat == STATUS_SUCCESS)&&
			(bResConf != FALSE))	/*　競合発生　*/
		{
			KdPrint(("Conflict\n"));
			/*　エラーを設定　*/
			ntStat = STATUS_DEVICE_CONFIGURATION_ERROR;
		}
		if (ntStat != STATUS_SUCCESS)	/*　使用不可　*/
		{
			KdPrint(("IoReportResourceUsage failed, %x\n", ntStat));
			EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_MAXMEM_REPORT_USAGE_FAILED, NULL);
			return FALSE;
		}
	}
	/*　資源使用設定　*/
	if (ResourceSetupTiny(pDrvObj, pEramExt->uExternalStart, &(pEramExt->MapAdr)) == FALSE)
	{
		KdPrint(("ResourceSetupTiny failed\n"));
		EramReportEvent(pEramExt->pDevObj, ERAMNT_ERROR_FUNCTIONERROR, "ResourceSetupTiny");
		/*　ドライバ資源解放　*/
		if (pEramExt->uOptflag.Bits.SkipReportUsage == 0)
		{
			RtlZeroBytes(&ResList, sizeof(ResList));
			IoReportResourceUsage(NULL, pDrvObj, &ResList, sizeof(ResList), NULL, NULL, 0, FALSE, &bResConf);
		}
		return FALSE;
	}
	KdPrint(("ExtReport end\n"));
	return TRUE;
}


/*　GetAcpiReservedMemory
		ACPI予約メモリ検査
	引数
		pDrvObj			装置の代表オブジェクトへのポインタ
	戻り値
		ACPI使用メモリの最下位アドレス
*/

DWORD GetAcpiReservedMemory(
	IN PDRIVER_OBJECT	pDrvObj
 )
{
	/*　ローカル変数　*/
	ULONG				loopi;
	PHYSICAL_ADDRESS	MapAdr;
	PBYTE				pBase;
	PDWORD				pdwBios;
	DWORD				dwMaxAdr;
	KdPrint(("GetAcpiReservedMemory start\n"));
	dwMaxAdr = 0xffff0000;
	/*　資源使用設定　*/
	if (ResourceSetupTiny(pDrvObj, BIOS_ADDRESS_START, &MapAdr) == FALSE)
	{
		KdPrint(("ResourceSetupTiny failed\n"));
		EramReportEvent(pDrvObj, ERAMNT_ERROR_FUNCTIONERROR, "ResourceSetupTiny");
		return dwMaxAdr;
	}
	/*　マップ(キャッシュ許可)　*/
	pBase = (PBYTE)MmMapIoSpace(MapAdr, BIOS_SIZE, TRUE);
	if (pBase == NULL)	/*　マップ以外orマップ失敗　*/
	{
		EramReportEvent(pDrvObj, ERAMNT_ERROR_MAXMEM_MAP_FAILED, NULL);
		return dwMaxAdr;
	}
	pdwBios = (PDWORD)pBase;
	for (loopi=0; loopi<BIOS_SIZE; loopi+=16)
	{
		if ((pdwBios[0] == 0x20445352)&&	/*　'RSD PTR '　*/
			(pdwBios[1] == 0x20525450))
		{
			KdPrint(("RSDT found, 0x%X\n", pdwBios[4]));
			if (pdwBios[4] != 0)	/*　32bitアドレス有効　*/
			{
				/*　RSDTの先を調べる　*/
				dwMaxAdr = CheckAcpiRsdt(pDrvObj, dwMaxAdr, pdwBios[4]) & 0xffff0000;
			}
			break;
		}
		pdwBios = &(pdwBios[4]);
	}
	/*　アンマップ　*/
	MmUnmapIoSpace(pBase, BIOS_SIZE);
	return dwMaxAdr;
}


/*　CheckAcpiRsdt
		RSDT検査
	引数
		pDrvObj			装置の代表オブジェクトへのポインタ
		dwMinValue		ACPI使用メモリの最下位アドレス
		dwRsdt			RSDTのアドレス
	戻り値
		ACPI使用メモリの最下位アドレス
*/

DWORD CheckAcpiRsdt(
	IN PDRIVER_OBJECT	pDrvObj,
	IN DWORD			dwMinValue,
	IN DWORD			dwRsdt
 )
{
	/*　ローカル変数　*/
	static ULONG		uRsdtSize = PAGE_SIZE_4K * 2;
	ULONG				loopi, uCnt;
	PHYSICAL_ADDRESS	MapAdr;
	PBYTE				pBase;
	PDWORD				pdwRsdt;
	DWORD				dwRsdtBase, dwRsdtOfs;
	KdPrint(("CheckAcpiRsdt start\n"));
	dwRsdtBase = dwRsdt & ~(PAGE_SIZE_4K - 1);
	dwRsdtOfs = dwRsdt - dwRsdtBase;
	dwMinValue = dwRsdt;
	/*　資源使用設定　*/
	if (ResourceSetupTiny(pDrvObj, dwRsdtBase, &MapAdr) == FALSE)
	{
		KdPrint(("ResourceSetupTiny failed\n"));
		EramReportEvent(pDrvObj, ERAMNT_ERROR_FUNCTIONERROR, "ResourceSetupTiny");
		return dwMinValue;
	}
	/*　マップ(キャッシュ許可)　*/
	pBase = (PBYTE)MmMapIoSpace(MapAdr, uRsdtSize, TRUE);
	if (pBase == NULL)	/*　マップ以外orマップ失敗　*/
	{
		EramReportEvent(pDrvObj, ERAMNT_ERROR_MAXMEM_MAP_FAILED, NULL);
		return dwMinValue;
	}
	pdwRsdt = (PDWORD)((DWORD)pBase + dwRsdtOfs);
	if ((pdwRsdt[0] != 0x54445352)||	/*　'RSDT'　*/
		(pdwRsdt[1] < 0x24))
	{
		KdPrint(("!RSDT\n"));
		return dwMinValue;
	}
	KdPrint(("RSDT found, 0x%X\n", dwRsdt));
	uCnt = ((pdwRsdt[1] - 0x24) / sizeof(DWORD));
	pdwRsdt = &(pdwRsdt[0x24 / sizeof(DWORD)]);
	/*　配列分繰り返し　*/
	for (loopi=0; loopi<uCnt; loopi++, pdwRsdt++)
	{
		dwMinValue = CheckRsdtElements(pDrvObj, dwMinValue, *pdwRsdt);
	}
	/*　アンマップ　*/
	MmUnmapIoSpace(pBase, uRsdtSize);
	KdPrint(("CheckAcpiRsdt end, 0x%X\n", dwMinValue));
	return dwMinValue;
}


/*　CheckRsdtElements
		RSDT要素検査
	引数
		pDrvObj			装置の代表オブジェクトへのポインタ
		dwMinValue		ACPI使用メモリの最下位アドレス
		dwRsdtElement	RSDT配列要素のアドレス
	戻り値
		ACPI使用メモリの最下位アドレス
*/

DWORD CheckRsdtElements(
	IN PDRIVER_OBJECT	pDrvObj,
	IN DWORD			dwMinValue,
	IN DWORD			dwRsdtElement
 )
{
	/*　ローカル変数　*/
	static ULONG		uRsdtSize = PAGE_SIZE_4K * 2;
	PHYSICAL_ADDRESS	MapAdr;
	PBYTE				pBase;
	PDWORD				pdwRsdt;
	DWORD				dwRsdtBase, dwRsdtOfs;
	KdPrint(("CheckRsdtElements start, min=0x%X, elem=0x%X\n", dwMinValue, dwRsdtElement));
	if (dwRsdtElement == 0)
	{
		return dwMinValue;
	}
	dwRsdtBase = dwRsdtElement & ~(PAGE_SIZE_4K - 1);
	dwRsdtOfs = dwRsdtElement - dwRsdtBase;
	dwMinValue = (dwMinValue > dwRsdtElement) ? dwRsdtElement : dwMinValue;
	/*　資源使用設定　*/
	if (ResourceSetupTiny(pDrvObj, dwRsdtBase, &MapAdr) == FALSE)
	{
		KdPrint(("ResourceSetupTiny failed\n"));
		EramReportEvent(pDrvObj, ERAMNT_ERROR_FUNCTIONERROR, "ResourceSetupTiny");
		return dwMinValue;
	}
	/*　マップ(キャッシュ許可)　*/
	pBase = (PBYTE)MmMapIoSpace(MapAdr, uRsdtSize, TRUE);
	if (pBase == NULL)	/*　マップ以外orマップ失敗　*/
	{
		EramReportEvent(pDrvObj, ERAMNT_ERROR_MAXMEM_MAP_FAILED, NULL);
		return dwMinValue;
	}
	pdwRsdt = (PDWORD)((DWORD)pBase + dwRsdtOfs);
	KdPrint(("Element 0x%X\n", *pdwRsdt));
	if ((pdwRsdt[0] == 0x50434146)&&	/*　'FACP'　*/
		(pdwRsdt[1] >= 0x74))
	{
		/*　FADT発見　*/
		if (pdwRsdt[9] != 0)
		{
			dwMinValue = (dwMinValue > pdwRsdt[9]) ? pdwRsdt[9] : dwMinValue;
		}
		if (pdwRsdt[10] != 0)
		{
			dwMinValue = (dwMinValue > pdwRsdt[10]) ? pdwRsdt[10] : dwMinValue;
		}
		KdPrint(("FADT found, FACS=0x%X, DSDT=0x%X\n", pdwRsdt[9], pdwRsdt[10]));
	}
	/*　アンマップ　*/
	MmUnmapIoSpace(pBase, uRsdtSize);
	KdPrint(("CheckRsdtElements end, min 0x%X\n", dwMinValue));
	return dwMinValue;
}

