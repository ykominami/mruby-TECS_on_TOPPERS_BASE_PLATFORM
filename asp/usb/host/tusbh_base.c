/*
 *  TOPPERS BASE PLATFORM MIDDLEWARE
 * 
 *  Copyright (C) 2017-2017 by TOPPERS PROJECT
 *                             Educational Working Group.
 * 
 *  上記著作権者は，以下の(1)〜(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 * 
 *  @(#) $Id: tusbh_base.c 698 2017-10-28 10:50:35Z roi $
 */
/*
 *  USB Host Middleware BASE部
 */

#include "tusbh_base.h"


#define USBH_ADDRESS_DEFAULT            0
#define USBH_MPS_DEFAULT                0x40
#define USBH_DEVICE_ADDRESS(d)          ((d)->idx + 1)

#define USBH_SET_CONFIGURATION(d, i)    tusbhControlRequest((d), 0, SET_CONFIGURATION_TYPE, SET_CONFIGURATION, (i), 0, 0)
#define USBH_SET_ADDRESS(d, a)          tusbhControlRequest((d), 0, SET_ADDRESS_TYPE, SET_ADDRESS, (a), 0, 0)


static TUSBH_Handle_t *usbh_arg;

/*
 *  デバイスハンドラ初期化
 */
static void
tusbhInitDevice(TUSBH_Device_t *pdevice)
{
	memset(pdevice->Data, 0, TUSBH_MAX_DATA_BUFFER);

	pdevice->cntl_pipe_size = USBH_MPS_DEFAULT;
	pdevice->cntl_errcount  = 0;
	pdevice->pClass  = NULL;
	pdevice->address = USBH_ADDRESS_DEFAULT;
	pdevice->speed   = USB_DEVICE_SPEED_FULL;
	pdevice->dstate  = DEVICE_IDLE;
}

/*
 *  アロケート・デバイスハンドラ
 */
static TUSBH_Device_t *
tusbhAllocDevice(TUSBH_Handle_t *phost)
{
	TUSBH_Device_t *pdevice;
	int i;

	if(phost->numDevice >= MAX_DEVICE_CONNECTED)
		return NULL;
	if((pdevice = tusbmalloc(sizeof(TUSBH_Device_t))) == NULL)
		return NULL;
	memset(pdevice, 0, sizeof(TUSBH_Device_t));
	if(phost->numDevice == 0)
		phost->pDevice = pdevice;
	else{
		TUSBH_Device_t *pnext = phost->pDevice;
		for(i = 1 ; i < phost->numDevice ; i++)
			pnext = pnext->pNext;
		pnext->pNext = pdevice;
	}
	pdevice->hub   = 0xff;
	pdevice->port  = 0xff;
	pdevice->pHost = phost;
	pdevice->idx  = phost->numDevice++;
	tusbhInitDevice(pdevice);
	return pdevice;
}

/*
 *  デバイスハンドラ検索
 */
static TUSBH_Device_t*
tusbhFindDevice(TUSBH_Handle_t *phost, uint8_t idx)
{
	TUSBH_Device_t *pdevice = phost->pDevice;
	uint8_t i;

	for(i = 0 ; i < idx ; i++)
		pdevice = pdevice->pNext;
	return pdevice;
}

/*
 *  有効デバイス数
 */
static uint8_t
tusbActivateDevice(TUSBH_Handle_t *phost)
{
	TUSBH_Device_t *pdevice = phost->pDevice;
	uint8_t i, actdevice = 0;

	for(i = 0 ; i < phost->numDevice && pdevice != NULL ; i++){
		if(pdevice->pClass != NULL)
			actdevice++;
		pdevice = pdevice->pNext;
	}
	return actdevice;
}

/*
 *  SEARCH DEVICE
 *  parameter1 phost:     ホストハンドラ
 *  parameter2 classCide: クラスコード
 *  parameter3 plun:      unit#
 *  return     デバイスハンドラ
 */
TUSBH_Device_t *
tusbhSearchDevice(TUSBH_Handle_t *phost, uint8_t classCode, uint8_t *plun)
{
	TUSBH_Device_t *pdevice = phost->pDevice;
	uint8_t i;

	for(i = 0 ; i < phost->numDevice && pdevice != NULL ; i++){
		if(pdevice->type == classCode){
			if(*plun < pdevice->numUnit)
				break;
			else
				*plun -= pdevice->numUnit;
		}
		pdevice = pdevice->pNext;
	}
	return pdevice;
}


/*
 *  HC-PIPEのアロケート
 *  parameter1 pdevice: デバイスハンドラ
 *  parameter2 ep_addr: エンドポイントアドレス
 *  return     PIPE番号(0xFFでエラー)
 */
uint8_t
tusbhAllocPipe(TUSBH_Handle_t *phost, uint8_t ep_addr)
{
	uint8_t idx = 0;

	for(idx = 0 ; idx < phost->numPipes ; idx++){
		if((phost->pipes[idx].attribute & PIPE_ACTIVATE) == 0 && tusbhHDCheckPipe(phost, idx)){
			phost->pipes[idx].attribute = PIPE_ACTIVATE | ep_addr;
			return idx;
		}
	}
	syslog_2(LOG_ERROR, "tusbhAllocPipe no free pipe [%08x] ep(%d) !", phost, ep_addr);
	return 0xFF;
}

/*
 *  HC-PIPEの解放
 *  parameter1 pdevice: デバイスハンドラ
 *  parameter2 no:      PIPE番号
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
tusbFreePipe(TUSBH_Handle_t *phost, uint8_t no)
{
	if(no < phost->numPipes){
		phost->pipes[no].attribute &= ~PIPE_ACTIVATE;
	}
	return TUSBH_E_OK;
}


/*
 *  USBホスト初期化処理
 *  parameter1 phost:   ホストハンドラ
 *  parameter2 pufunc:ユーザポインタ関数
 *  parameter3 id:      USBホストID
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
tusbhInit(TUSBH_Handle_t *phost, void (*pufunc)(TUSBH_Handle_t *phost, TUSBH_Device_t *pdevice, uint8_t ), uint8_t id)
{
	uint8_t i;

	/*
	 *  USBホスト構造体の確認
	 */
	if(phost == NULL){
		syslog_0(LOG_ERROR, "Invalid Host handle !");
		return TUSBH_E_PAR; 
	}

	/*
	 *  ホストIDをセット
	 */
	phost->id = id;

	/*
	 *  リンクカウントを初期化
	 */
	phost->numClass = 0;
	phost->numDevice = 0;
	phost->numHub    = 0;

	/*
	 *  PIPEの初期化
	 */
	for(i = 0 ; i < MAX_EPS_PIPES; i++){
		phost->pipes[i].attribute = 0;
		phost->pipes[i].index     = 0;
	}

	/*
	 *  ユーザー関数の保存
	 */
	if(pufunc != NULL)
		phost->usrcallback = pufunc;

	/*
	 * タスク用にUSBホストハンドラを保存
	 */
	usbh_arg = phost;

	/*
	 *  LOWレベルドライバの初期化
	 */
	tusbhHDInit(phost);

	/*
	 *  タスクの起動
	 */
	tusbStartTask(phost->eventTaskID);
	tusbStartTask(phost->processTaskID);
	return TUSBH_E_OK;
}

/*
 *  USBホスト終了処理
 *  parameter1 phost:   ホストハンドラ
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
tusbhDeInit(TUSBH_Handle_t *phost)
{
	TUSBH_Device_t *pdevice;
	uint8_t idx;

	for(idx = 0, pdevice = phost->pDevice ; idx < phost->numDevice ; idx++){
		tusbfree(pdevice);
		pdevice = pdevice->pNext;
	}
	phost->pDevice = 0;

	if(phost->pSysData != NULL){
		tusbhHDStop(phost);
	}
	return TUSBH_E_OK;
}

/*
 *  クラス実行モジュールのリンク
 *  parameter1 phost:   ホストハンドラ
 *  parameter2 pclass:  クラス構造体へのポインタ
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
tusbhLinkClass(TUSBH_Handle_t *phost, TUSBH_Class_t *pclass)
{
	if(pclass == NULL)
		return TUSBH_E_ERR;
	pclass->pNext = NULL;
	if(phost->numClass == 0)
		phost->pClass = pclass;
	else{
		TUSBH_Class_t *pnext = phost->pClass;
		uint32_t i;
		for(i = 1 ; i < phost->numClass ; i++)
			pnext = pnext->pNext;
		pnext->pNext = pclass;
	}
	phost->numClass++;
	return TUSBH_E_OK;
}

/*
 *  USBホスト開始
 *  parameter1 phost:   ホストハンドラ
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
tusbhStart(TUSBH_Handle_t *phost)
{
	/*
	 *  LOWレベルドライバをスタート
	 */
	tusbhHDStart(phost);

	/*
	 *  VBUSを有効化
	 */
	tusbhHDControlVBUS(phost, true);
	return TUSBH_E_OK;
}

/*
 *  USBホスト停止
 *  parameter1 phost:   ホストハンドラ
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
tusbhStop(TUSBH_Handle_t *phost)
{
	TUSBH_Device_t *pdevice = phost->pDevice;
	uint8_t i = 0;

	/*
	 *  LOWレベルドライバ停止
	 */
	tusbhHDStop(phost);

	/*
	 *  VBUSの停止
	 */ 
	tusbhHDControlVBUS(phost, false);

	/*
	 *  PIPEの解放
	 */
	for(i = 0 ; i < phost->numDevice && pdevice != NULL ; i++){
		tusbFreePipe(phost, pdevice->cntl_pipe_in);
		tusbFreePipe(phost, pdevice->cntl_pipe_out);
		pdevice = pdevice->pNext;
	}
	return TUSBH_E_OK;
}

/*
 *  HUBのリセット要求
 *  parameter1 pdevice: デバイスハンドラ
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
tusbhResetHub(TUSBH_Device_t *pdevice)
{
	TUSBH_Handle_t *phost = pdevice->pHost;
	TUSBH_Device_t *pdeviceh;
	uint32_t idx;

	syslog_2(LOG_ERROR, "tusbhResetHub hub(%d) port(%d) !", pdevice->hub, pdevice->port);
	for(idx = 0, pdeviceh = phost->pDevice ; idx < phost->numDevice ; idx++){
		if(pdevice->hub == pdeviceh->hubid){
			tusbSendData(phost->process_event, TUSBH_RESET_EVENT, pdeviceh->idx, 0, pdevice->port);
			break;
		}
		pdeviceh = pdeviceh->pNext;
	}
	return TUSBH_E_OK;
}

/*
 *  デバイスにインターフェイスを設定
 *  parameter1 pdevice: デバイスハンドラ
 *  parameter2 interface: インターフェイス値
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
tusbhSelectInterface(TUSBH_Device_t *pdevice, uint8_t interface)
{
	TUSBH_ERCODE ercd = TUSBH_E_OK;

	if(interface < pdevice->CfgDesc.bNumInterfaces){
		pdevice->sel_interface = interface;
		syslog_2(LOG_NOTICE, "Switching to Interface device(%d) (#%d)", pdevice->idx, interface);
		syslog_1(LOG_NOTICE, "Class    : 0x%02x", pdevice->ItfDesc[interface].bInterfaceClass );
		syslog_1(LOG_NOTICE, "SubClass : 0x%02x", pdevice->ItfDesc[interface].bInterfaceSubClass );
		syslog_1(LOG_NOTICE, "Protocol : 0x%02x", pdevice->ItfDesc[interface].bInterfaceProtocol );
	}
	else{
		syslog_1(LOG_ERROR, "Cannot Select This Interface (%d) !", pdevice->idx);
		ercd = TUSBH_E_ERR; 
	}
	return ercd;
}

/*
 *  クラス番号を取得
 *  parameter1 pdevice: デバイスハンドラ
 *  return     Class code
 */
uint8_t
tusbhGetActiveClass(TUSBH_Device_t *pdevice)
{
	return (pdevice->ItfDesc[0].bInterfaceClass);
}

/*
 *  クラス情報からインターフェイスのインデックスを返す
 *  parameter1 pdevice: デバイスハンドラ
 *  parameter2 Class:   Class code
 *  parameter3 SubClass:Sub Class code
 *  parameter4 Protocal:Protocol code
 *  return     interface index(ない場合はNO_INTERFACE)
 */
uint8_t
tusbhFindInterface(TUSBH_Device_t *pdevice, uint8_t Class, uint8_t SubClass, uint8_t Protocol)
{
	InterfaceDescriptor    *pif = NULL;
	int8_t                 if_ix = 0;

	while(if_ix < TUSBH_MAX_NUM_INTERFACES){
		pif = &pdevice->ItfDesc[if_ix];
		if((pif->bInterfaceClass == Class || Class == 0xFF) &&
			(pif->bInterfaceSubClass == SubClass || SubClass == 0xFF) &&
				(pif->bInterfaceProtocol == Protocol || Protocol == 0xFF)){
			return  if_ix;
		}
		if_ix++;
	}
	return NO_INTERFACE;
}

/*
 *  インターフェイス情報からインターフェイスのインデックスを返す
 *  parameter1 pdevice: デバイスハンドラ
 *  parameter2 interface: interface number
 *  parameter3 altsetnum: alternate setting #
 *  return     interface index(ない場合は0xff)
 */
uint8_t
tusbhFindInterfacebynumber(TUSBH_Device_t *pdevice, uint8_t interface, uint8_t altsetno)
{
	InterfaceDescriptor    *pif = NULL;
	int8_t                 if_ix = 0;

	while (if_ix < TUSBH_MAX_NUM_INTERFACES){
		pif = &pdevice->ItfDesc[if_ix];
		if((pif->bInterfaceNumber == interface) && (pif->bAlternateSetting == altsetno)){
			return  if_ix;
		}
		if_ix++;
	}
	return 0xFF;
}

/*
 *  OPEN PIPE
 *  parameter1 pdevice: デバイスハンドラ
 *  parameter2 pipe:    PIPE#
 *  parameter3 epnum:   EP番号
 *  parameter5 type:    エンドポイントタイプ
 *  parameter6 eps:     MAXパケットサイズ
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
tusbhOpenPipe(TUSBH_Device_t *pdevice, uint8_t pipe, uint8_t epnum, uint8_t type, uint16_t mps)
{
	TUSBH_Handle_t *phost = pdevice->pHost;

	phost->pipes[pipe].type = type;
	return tusbhHDOpenPipe(phost, pipe, epnum, pdevice->address, pdevice->speed, type, mps);
}

/*
 *  CLOSE PIPE
 *  parameter1 pdevice: デバイスハンドラ
 *  parameter2 pipe:    PIPE#
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
tusbhClosePipe(TUSBH_Device_t *pdevice, uint8_t pipe)
{
	return tusbhHDClosePipe(pdevice->pHost, pipe);
}

/*
 *  SUBMIT URB
 *  parameter1 pdevice: デバイスハンドラ
 *  parameter2 pipe:    パイプ番号
 *  parameter3 type:    SUBMITタイプ
 *  parameter4 eptype:  エンドポイントタイプ
 *  parameter5 buff:    データ領域へのポインタ
 *  parameter6 length:  データ長
 *  return     TUSBH_ERCODE
 */
TUSBH_ERCODE
tusbhSubmitURB(TUSBH_Device_t *pdevice, uint8_t pipe, uint16_t type, uint8_t eptype, uint8_t* buff, uint16_t length)
{
	TUSBH_Handle_t *phost = pdevice->pHost;
	uint8_t dir, token;
	uint8_t do_ping = 0;

	phost->pipes[pipe].index = pdevice->idx;
	if(eptype != phost->pipes[pipe].type){
		syslog_3(LOG_ERROR, "tusbhSubmitURB EPTYPE ERROR pipe(%d) rtype(%d) ctype(%d) !", pipe, eptype, phost->pipes[pipe].type);
		return TUSBH_E_ERR;
	}
	if(eptype == USB_EP_TYPE_BULK || eptype == USB_EP_TYPE_CTRL){
		if(pdevice->speed == USB_DEVICE_SPEED_HIGH && type == SUBMIT_WRITE)
			do_ping = 1;
	}
	dir   = type >> 8;
	token = type & 0xff;
	return tsubhHDSubmitURB(phost, pipe, dir, token, buff, length, do_ping);
}

/*
 *  Device Descriporコマンドをデバイスに送信し取得する
 *  parameter1 pdevice: デバイスハンドラ
 *  parameter2 length:  Descripor長
 *  return     TUSBH_ERCODE
 */
static TUSBH_ERCODE
tusbhGetDeviceDescriptor(TUSBH_Device_t *pdevice, uint8_t length)
{
	TUSBH_ERCODE  ercd;
	volatile uint8_t    *buff = pdevice->Data;
	DeviceDescriptor *dev_desc = &pdevice->DevDesc;

	if((ercd = tusbhGetDescriptor(pdevice,
                                  USB_RECIPIENT_DEVICE | USB_REQUEST_TYPE_STANDARD,                          
                                  ((DEVICE_DESCRIPTOR << 8) & 0xFF00),
                                  (uint8_t *)buff,
                                  length)) == TUSBH_E_OK){
		/*
		 *  DEVICE DESCRIPTER取得
		 */
		dev_desc->bLength            = buff[0];
		dev_desc->bDescriptorType    = buff[1];
		dev_desc->bcdUSB             = (buff[2] & 0xff) | (buff[3] << 8);
		dev_desc->bDeviceClass       = buff[4];
		dev_desc->bDeviceSubClass    = buff[5];
		dev_desc->bDeviceProtocol    = buff[6];
		dev_desc->bMaxPacketSize     = buff[7];

		if(length > 8){ /* For 1st time after device connection, Host may issue only 8 bytes for   Device Descriptor Length  */
			dev_desc->idVendor           = (buff[8]  & 0xff) | (buff[9]  << 8);
			dev_desc->idProduct          = (buff[10] & 0xff) | (buff[11] << 8);
			dev_desc->bcdDevice          = (buff[12] & 0xff) | (buff[12] << 8);
			dev_desc->iManufacturer      = buff[14];
			dev_desc->iProduct           = buff[15];
			dev_desc->iSerialNumber      = buff[16];
			dev_desc->bNumConfigurations = buff[17];
		}
	}
	return ercd;
}

/*
 *  Config Descriporコマンドをデバイスに送信し取得する
 *  parameter1 pdevice: デバイスハンドラ
 *  parameter2 length:  Descripor長
 *  return     TUSBH_ERCODE
 */
static TUSBH_ERCODE
tusbhGetConfigDescriptor(TUSBH_Device_t *pdevice, uint16_t length)
{
	ConfigurationDescriptor *cfg_desc = &pdevice->CfgDesc;
	TUSBH_Handle_t     *phost = pdevice->pHost;
	volatile uint8_t   *pdbuff;
	TUSBH_ERCODE ercd;

	if(phost->pCfgData != NULL)
		pdbuff = (volatile uint8_t *)phost->pCfgData;
	else
		pdbuff = (volatile uint8_t *)pdevice->Data;
	if((ercd = tusbhGetDescriptor(pdevice,
                                  USB_RECIPIENT_DEVICE | USB_REQUEST_TYPE_STANDARD,
                                  ((CONFIGURATION_DESCRIPTOR << 8) & 0xFF00),
                                  (uint8_t *)pdbuff,
                                  length)) == TUSBH_E_OK){

		/*
		 *  CONFIGURATION DESCRIPTOR設定
		 */
		cfg_desc->bLength             = pdbuff[0];
		cfg_desc->bDescriptorType     = pdbuff[1];
		cfg_desc->wTotalLength        = (pdbuff[2] & 0xff) | (pdbuff[3] << 8);
		cfg_desc->bNumInterfaces      = pdbuff[4];
		cfg_desc->bConfigurationValue = pdbuff[5];
		cfg_desc->iConfiguration      = pdbuff[6];
		cfg_desc->bmAttributes        = pdbuff[7];
		cfg_desc->bMaxPower           = pdbuff[8];

		if (length > CONFIGURATION_DESCRIPTOR_LENGTH){
			uint16_t ptr = CONFIGURATION_DESCRIPTOR_LENGTH;
			int8_t   if_ix = 0;
			int8_t   ep_ix = 0;

			while(if_ix < TUSBH_MAX_NUM_INTERFACES && ptr < cfg_desc->wTotalLength){
				ptr += pdbuff[0];
				pdbuff = (volatile uint8_t *)(pdbuff + pdbuff[0]);
				if(pdbuff[1] == INTERFACE_DESCRIPTOR){
					InterfaceDescriptor *pif = &pdevice->ItfDesc[if_ix];
					/*
					 *  INTERFACE DESCRIPTOR設定
					 */
  					pif->bLength            = pdbuff[0];
					pif->bDescriptorType    = pdbuff[1];
					pif->bInterfaceNumber   = pdbuff[2];
					pif->bAlternateSetting  = pdbuff[3];
					pif->bNumEndpoints      = pdbuff[4];
					pif->bInterfaceClass    = pdbuff[5];
					pif->bInterfaceSubClass = pdbuff[6];
					pif->bInterfaceProtocol = pdbuff[7];
					pif->iInterface         = pdbuff[8];

					ep_ix = 0;
					while ((ep_ix < pif->bNumEndpoints) && (ptr < cfg_desc->wTotalLength)){
						ptr += pdbuff[0];
						pdbuff = (volatile uint8_t *)(pdbuff + pdbuff[0]);
						if(pdbuff[1] == ENDPOINT_DESCRIPTOR){
							EndpointDescriptor *pep = &pdevice->EpDesc[if_ix][ep_ix];
							/*
							 *  ENDPOINT DESCRIPTOR設定
							 */
							pep->bLength          = pdbuff[0];
							pep->bDescriptorType  = pdbuff[1];
							pep->bEndpointAddress = pdbuff[2];
							pep->bmAttributes     = pdbuff[3];
							pep->wMaxPacketSize   = (pdbuff[4] & 0xff) | (pdbuff[5] << 8);
							pep->bInterval        = pdbuff[6];
							ep_ix++;
						}
					}
					if_ix++;
				}
			}
		}
	}
	return ercd;
}

/*
 *  String Descriporコマンドをデバイスに送信
 *  parameter1 pdevice: デバイスハンドラ
 *  parameter2 string_index:  String index
 *  parameter3 buff:          取り出し用のバッファ領域
 *  parameter4 length:        Descripor長
 *  return     TUSBH_ERCODE
 */
static TUSBH_ERCODE
tusbhGetStringDescriptor(TUSBH_Device_t *pdevice,
                                uint8_t string_index, 
                                uint8_t *buff, 
                                uint16_t length)
{
	uint8_t *psrc = pdevice->Data;
	TUSBH_ERCODE ercd;

	if((ercd = tusbhGetDescriptor(pdevice,
                                  USB_RECIPIENT_DEVICE | USB_REQUEST_TYPE_STANDARD,
                                  ((STRING_DESCRIPTOR << 8) & 0xFF00) | string_index, 
                                  psrc,
                                  length)) == TUSBH_E_OK){
  		/*
		 * UINCODEをアスキー文字列に変換
		 */
		if(psrc[1] == STRING_DESCRIPTOR){ /* String Typeならバッファにセット */
			int strlength = psrc[0]-2;
			uint32_t i;
			for(i = 0 ; i < length && i < strlength ; i += 2){
				*buff++ =  psrc[i+2];
			}
			*buff = 0;
		}
	}
	return ercd;
}

/*
 *  USBのエナミュレートを実行する
 *  parameter1 pdevice: デバイスハンドラ
 *  return     TUSBH_ERCODE
 */
static TUSBH_ERCODE
tusbhEnumerate(TUSBH_Device_t *pdevice)
{
	TUSBH_ERCODE ercd = TUSBH_E_OK;

	/*
	 *  8バイトで Device Descriptorを取得
	 */
	if((ercd = tusbhGetDeviceDescriptor(pdevice, 8)) != TUSBH_E_OK)
		return ercd;

	/*
	 *  bMaxPacketSizeでパイプを再オープン
	 */
	pdevice->cntl_pipe_size = pdevice->DevDesc.bMaxPacketSize;
	tusbhOpenPipe(pdevice, pdevice->cntl_pipe_in, 0x80, USB_EP_TYPE_CTRL,
				pdevice->cntl_pipe_size);

	tusbhOpenPipe(pdevice, pdevice->cntl_pipe_out, 0x00, USB_EP_TYPE_CTRL,
				pdevice->cntl_pipe_size);

	/*
	 *  フルサイズで Device Descriptorを取得
	 */
	if((ercd != tusbhGetDeviceDescriptor(pdevice, DEVICE_DESCRIPTOR_LENGTH)) != TUSBH_E_OK)
		return ercd;

	syslog_1(LOG_NOTICE, "VID: 0x%04x", pdevice->DevDesc.idVendor );
	syslog_1(LOG_NOTICE, "PID: 0x%04x", pdevice->DevDesc.idProduct);

	/*
	 *  デバイスにデバイスアドレスをセット
	 */
	if((ercd = USBH_SET_ADDRESS(pdevice, USBH_DEVICE_ADDRESS(pdevice))) != TUSBH_E_OK)
		return ercd;

	/*
	 *  設定アドレスでパイプを再オープン
	 */
	tusbDelay(2);
	pdevice->address = USBH_DEVICE_ADDRESS(pdevice);
	syslog_1(LOG_NOTICE, "Address (#%d) assigned.", pdevice->address);

	tusbhOpenPipe(pdevice, pdevice->cntl_pipe_in, 0x80, USB_EP_TYPE_CTRL,
				pdevice->cntl_pipe_size);

	tusbhOpenPipe(pdevice, pdevice->cntl_pipe_out, 0x00, USB_EP_TYPE_CTRL,
				pdevice->cntl_pipe_size);

	/*
	 *  スタンダードサイズで Configuration descriptorを取得
	 */
	if((ercd = tusbhGetConfigDescriptor(pdevice, CONFIGURATION_DESCRIPTOR_LENGTH)) != TUSBH_E_OK)
		return ercd;

	/*
	 *  フルサイズでConfiguraion Descriptorを取得
	 */
	if((ercd = tusbhGetConfigDescriptor(pdevice, pdevice->CfgDesc.wTotalLength)) != TUSBH_E_OK)
		return ercd;

	/*
	 *  Manufacturer文字列を取得
	 */
	if(pdevice->DevDesc.iManufacturer != 0){ /* Check that Manufacturer String is available */
		if(tusbhGetStringDescriptor(pdevice, pdevice->DevDesc.iManufacturer, 
				pdevice->Data , 0xff) == TUSBH_E_OK){
			strncpy(pdevice->Manufacturer, (const char *)pdevice->Data, TUSBH_STR_LENGTH);
			/*
			 *  Manufacturingを表示
			 */
			syslog_1(LOG_NOTICE, "Manufacturer : %s",  pdevice->Manufacturer);
		}
	}
	else
		syslog_0(LOG_NOTICE, "Manufacturer : N/A");

	/*
	 *  Product文字列を取得
	 */
	if(pdevice->DevDesc.iProduct != 0){ /* Check that Product string is available */
		if(tusbhGetStringDescriptor(pdevice, pdevice->DevDesc.iProduct, 
						pdevice->Data, 0xff) == TUSBH_E_OK){
			strncpy(pdevice->Prodeuct, (const char *)pdevice->Data, TUSBH_STR_LENGTH);
			/* User callback for Product string */
			syslog_1(LOG_NOTICE, "Product : %s",  pdevice->Prodeuct);
		}
	}
	else
		syslog_0(LOG_NOTICE, "Product : N/A");

	/*
	 *  シリアル番号取得
	 */
	if(pdevice->DevDesc.iSerialNumber != 0){ /* Check that Serial number string is available */    
		if((ercd = tusbhGetStringDescriptor(pdevice, pdevice->DevDesc.iSerialNumber, 
						pdevice->Data, 0xff)) == TUSBH_E_OK){
			strncpy(pdevice->Serial, (const char *)pdevice->Data, TUSBH_STR_LENGTH);
			/* User callback for Serial number string */
			syslog_1(LOG_NOTICE, "Serial Number : %s",  pdevice->Serial);
		}
	}
	else{
		syslog_0(LOG_NOTICE, "Serial Number : N/A"); 
		ercd = TUSBH_E_OK;
	}
	return ercd;
}

/*
 *  USBホストプロセスタスク
 */
void
tusbhProcessTask(intptr_t exinf)
{
	TUSBH_Handle_t *phost = usbh_arg;
	TUSBH_Device_t *pdevice;
	TUSBH_Class_t  *pnext;
	TUSBH_ERCODE ercd = TUSBH_E_OK;
	ER           rercd;
	uint8_t mes[4];
	uint32_t no;

	for(;;){
		rercd = tusbRecvData(phost->process_event, mes, 0);
		if(rercd == E_OK){
			if((pdevice = tusbhFindDevice(phost, mes[1])) == NULL){
				syslog_1(LOG_NOTICE, "USBH no device message(%d) !", mes[1]);
				continue;
			}
			pdevice = tusbhFindDevice(phost, mes[1]);
			switch (pdevice->dstate){
			case DEVICE_ATTACHED:
				syslog_2(LOG_NOTICE, "USBH Device Attached(%d) speed(%d) !", pdevice->idx, pdevice->speed);
				if(phost->usrcallback != NULL)
					phost->usrcallback(phost, pdevice, HOST_USER_CONNECTION);

				/*
				 *  100ms待ちコントロール通信用PIPEをアサイン
				 */
				tusbDelay(100); 

				pdevice->cntl_pipe_out = tusbhAllocPipe(phost, 0x00);
				pdevice->cntl_pipe_in  = tusbhAllocPipe(phost, 0x80);

				/*
				 *  IN/OUTのPIPEをオープン
				 */
				tusbhOpenPipe(pdevice, pdevice->cntl_pipe_in, 0x80, USB_EP_TYPE_CTRL,
								pdevice->cntl_pipe_size);

				tusbhOpenPipe(pdevice, pdevice->cntl_pipe_out, 0x00, USB_EP_TYPE_CTRL,
								pdevice->cntl_pipe_size);
				pdevice->numUnit = 1;
				pdevice->pre_connected = 0;

				/*
				 *  エナミュレーション実行
				 */
				if((ercd = tusbhEnumerate(pdevice)) == TUSBH_E_OK){
					syslog_1(LOG_NOTICE, "Enumeration done(%d) !", pdevice->idx);
					pdevice->sel_interface = 0;
					if(pdevice->DevDesc.bNumConfigurations != 1){
						if(phost->usrcallback != NULL)
							phost->usrcallback(phost, pdevice, HOST_USER_SELECT_CONFIGURATION);
					}
					else
						syslog_1(LOG_NOTICE, "This device has only 1 configuration(%d) !", pdevice->idx);
					if(USBH_SET_CONFIGURATION(pdevice, pdevice->CfgDesc.bConfigurationValue) == TUSBH_E_OK){
						pdevice->dstate  = DEVICE_CHECK_CLASS;
						syslog_1(LOG_NOTICE, "Default configuration set(%d) !", pdevice->idx);
					}
					else if(ercd == TUSBH_E_ERR){
						pdevice->dstate = DEVICE_DISCONNECTED;
						syslog_1(LOG_ERROR, "Enumeration set configuraion error(%) !", pdevice->idx);
						tusbSendData(phost->process_event, TUSBH_PORT_EVENT, pdevice->idx, 0, 0);
					}
				}
				else{
					pdevice->dstate = DEVICE_DISCONNECTED;
					syslog_2(LOG_ERROR, "Enumeration sequence fail idx(%d) ercd(%d) !", pdevice->idx, ercd);
					tusbSendData(phost->process_event, TUSBH_PORT_EVENT, pdevice->idx, 0, 0);
				}
				break;
			case DEVICE_CHECK_CLASS:
				pdevice->pClass = NULL;
				for(no = 0, pnext = phost->pClass ; no < phost->numClass ; no++){
					if(pnext->classCode == pdevice->ItfDesc[0].bInterfaceClass){
						pdevice->pClass = pnext;
						break;
					}
					pnext = pnext->pNext;
				}
				if(pdevice->pClass != NULL){
					if((ercd = pdevice->pClass->Init(pdevice)) == TUSBH_E_OK){
						syslog_2(LOG_NOTICE, "Class start[%s] (%d) !", pdevice->pClass->Name, pdevice->idx);
						pdevice->dstate  = DEVICE_CLASS;
						pdevice->type    = pdevice->pClass->classCode;
						if(phost->usrcallback != NULL)
							phost->usrcallback(phost, pdevice, HOST_USER_CLASS_SELECTED);
						if(tusbActivateDevice(phost) == 1)
							tusbTimerControl(1);
					}
					else{
						pdevice->dstate  = DEVICE_ABORT_STATE;
						syslog_3(LOG_ERROR, "Class[%s] initial error(%d) (%d) !", pdevice->pClass->Name, ercd, pdevice->idx);
					}
				}
				else{
					pdevice->dstate  = DEVICE_ABORT_STATE;
					syslog_1(LOG_ERROR, "No registered class for this device(%d) !", pdevice->idx);
				}
				break;
			case DEVICE_CLASS:
				/*
				 *  USB HOSTクラス処理
				 */
				if(pdevice->pClass != NULL)
					pdevice->pClass->Process(pdevice, mes);
				break;
			case DEVICE_DISCONNECTED:
#if 1	/* ROI DEBUG */
			syslog_0(LOG_NOTICE, "## EXEC DEVICE_DISCONNECTED ##");
#endif	/* ROI DEBUG */
				/*
				 *  USB HOSTクラスの終了処理
				 */
				if(pdevice->pClass != NULL){
					pdevice->pClass->DeInit(pdevice); 
					pdevice->pClass = NULL;
				}
				tusbhInitDevice(pdevice);
				if(tusbActivateDevice(phost) == 0)
					tusbTimerControl(0);
				pdevice->dstate = DEVICE_IDLE;
				break;

			case DEVICE_IDLE:
			case DEVICE_ABORT_STATE:
			default :
				break;
			}
		}
	}
}

/*
 *  タイマハンドラ
 */
void
tusbhCyclicHandler(intptr_t exinf)
{
	TUSBH_Handle_t *phost = usbh_arg;
	TUSBH_Device_t *pdevice;
	uint8_t i;

	if(phost == NULL)
		return;
	pdevice = phost->pDevice;
	for(i = 0 ; i < phost->numDevice && pdevice != NULL ; i++){
		if(pdevice->pClass != NULL && pdevice->timecount > 0){
			if(--pdevice->timecount == 0)
				tusbiSendData(phost->process_event, TUSBH_TIME_EVENT, pdevice->idx, 0, pdevice->timeid);
		}
		pdevice = pdevice->pNext;
	}
}

/*
 *  USBデバイスコネクト処理
 */
static void
tusbhHandleConnect(TUSBH_Handle_t *phost, uint8_t hub, uint8_t port, uint8_t prm)
{
	TUSBH_Device_t *pdevice;
	uint32_t no;

	for(no = 0, pdevice = phost->pDevice ; no < phost->numDevice && pdevice != NULL ; no++){
		if(pdevice->activate != 0 && pdevice->hub == hub && pdevice->port == port){
			break;
		}
		pdevice = pdevice->pNext;
	}
	if(pdevice == NULL){
		pdevice = tusbhAllocDevice(phost);
		if(pdevice == NULL || pdevice->idx != no){
			syslog_2(LOG_ERROR, "usbh device allocation error (%d)(%d) !", no, pdevice->idx);
			return;
		}
	}
	if(phost->numClass == 0)
		syslog_0(LOG_ERROR, "No Regist Class !");
	else if(pdevice != NULL){
		if(pdevice->pre_connected == 0){
#if 1	/* ROI DEBUG */
			syslog_4(LOG_NOTICE, "## CONN(1) pdevice[%08x] hub(%d) port(%d) speed(%d) ##", pdevice, hub, port, prm);
#endif	/* ROI DEBUG */
			pdevice->hub  = hub;
			pdevice->port = port;
			pdevice->activate = 1;
			pdevice->pre_connected = 1;

			/*
			 *  接続後 200ms待ち
			 */
			tusbDelay(200);
			if(hub == 0){
				tusbhHDResetPort(phost);
				tusbDelay(100);
				pdevice->speed = tusbhHDGetSpeed(phost);
			}
			else{
				pdevice->speed = prm;
				tusbDelay(200);
			}
			pdevice->is_connected = 1;
			if(pdevice->dstate == DEVICE_IDLE){
				pdevice->dstate = DEVICE_ATTACHED;
				tusbSendData(phost->process_event, TUSBH_PORT_EVENT, pdevice->idx, 0, 0);
			}
		}
		else if(pdevice->dstate == DEVICE_DISCONNECTED)
			tusbSendData(phost->process_event, TUSBH_PORT_EVENT, pdevice->idx, 0, 0);
	}
}

/*
 *  USBデバイスディスコネクト処理
 */
static void
tusbhHandleDisconnect(TUSBH_Handle_t *phost, uint8_t hub, uint8_t port, uint8_t prm)
{
	TUSBH_Device_t *pdevice = phost->pDevice;
	uint32_t no;

	for(no = 0 ; no < phost->numDevice ; no++, pdevice = pdevice->pNext){
		if(pdevice == NULL)
			return;
		if(pdevice->activate != 0 && pdevice->hub == hub && pdevice->port == port)
			break;
	}
	if(no >= phost->numDevice)
		return;
	if(pdevice->is_connected == 0)
		return;

	/*
	 *  Hostペリフェラル停止
	 */
	if(hub == 0)
		tusbhHDStop(phost);

	/*
	 *  コントロール通信用PIPEを解放
	 */
	tusbhClosePipe(pdevice, pdevice->cntl_pipe_in);
	tusbFreePipe(phost, pdevice->cntl_pipe_in);
	tusbhClosePipe(pdevice, pdevice->cntl_pipe_out);
	tusbFreePipe(phost, pdevice->cntl_pipe_out);

	pdevice->is_connected = 0;
	pdevice->pre_connected = 0;

	/*
	 *  ユーザーへ通知
	 */
	if(phost->usrcallback != NULL)
		phost->usrcallback(phost, pdevice, HOST_USER_DISCONNECTION);
	syslog_4(LOG_NOTICE, "USB Device disconnected [%02x][%02x][%02x] device(%d) !", hub, port, prm, pdevice->idx); 

	/*
	 *  Hostペリフェラル再開
	 */
	if(hub == 0)
		tusbhHDStart(phost);

	pdevice->dstate = DEVICE_DISCONNECTED;
	tusbSendData(phost->process_event, TUSBH_PORT_EVENT, pdevice->idx, 0, 0);
}

/*
 *  USBホストイベントタスク
 */
void
tusbhEventTask(intptr_t exinf)
{
	uint8_t       mes[4];
	ER            ercd;

	for(;;){
		ercd = tusbRecvData(usbh_arg->connect_event, mes, 0);

		if(ercd == E_OK){
#if 1	/* ROI DEBUG */
		syslog_4(LOG_NOTICE, "####### usbh_task evt(%d) [%02x][%02x][%02x] #######", mes[0], mes[1], mes[2], mes[3]);
#endif	/* ROI DEBUG */
			if(mes[0] == TUSBH_CONNECT_EVENT)
				tusbhHandleConnect(usbh_arg, mes[1], mes[2], mes[3]);
			else if(mes[0] == TUSBH_DISCONNECT_EVENT){
				dly_tsk(200);
				tusbhHandleDisconnect(usbh_arg, mes[1], mes[2], mes[3]);
			}
			else if(mes[0] == TUSBH_IDCHANGE_EVENT){
				dly_tsk(200);
				tusbhHDStart(usbh_arg);
			}
		}
	}
}


