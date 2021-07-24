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
 *  @(#) $Id: tusbh_base.h 698 2017-10-28 10:48:57Z roi $
 */
/*
 *  USB Host Middleware BASE部定義
 */

#ifndef _HUSBH_BASE_H_
#define _HUSBH_BASE_H_

#include "tusb_rtos.h"
#include "tusb_types.h"

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef TUSBH_MAX_NUM_ENDPOINTS
#define TUSBH_MAX_NUM_ENDPOINTS           2
#endif
#ifndef TUSBH_MAX_NUM_INTERFACES
#define TUSBH_MAX_NUM_INTERFACES          2
#endif
#ifndef TUSBH_MAX_DATA_BUFFER
#define TUSBH_MAX_DATA_BUFFER             0x200
#endif

/*
 *  STMユーザーイベント
 */
#define HOST_USER_SELECT_CONFIGURATION    1
#define HOST_USER_CLASS_ACTIVE            2
#define HOST_USER_CLASS_SELECTED          3
#define HOST_USER_CONNECTION              4
#define HOST_USER_DISCONNECTION           5
#define HOST_USER_UNRECOVERED_ERROR       6

/*Standard Feature Selector for clear feature command*/
#define FEATURE_SELECTOR_ENDPOINT         0X00
#define FEATURE_SELECTOR_DEVICE           0X01

/*
 *  SUBMITタイプ
 */
#define SUBMIT_SETUP                      ((0<<8) | USB_PID_SETUP)
#define SUBMIT_WRITE                      ((0<<8) | USB_PID_DATA)
#define SUBMIT_READ                       ((1<<8) | USB_PID_DATA)

/*
 *  パイプ属性定義
 */
#define PIPE_ACTIVATE                     0x8000
#define PIPE_ADDRESS                      0x00FF


#define NO_INTERFACE                      0xFF


/*
 *  URBステート定義
 */
typedef enum {
	TUSBH_URB_IDLE = 0,
	TUSBH_URB_DONE,
	TUSBH_URB_NOTREADY,
	TUSBH_URB_NYET,
	TUSBH_URB_ERROR,
	TUSBH_URB_STALL
} TUSBH_URBState_t;

/*
 *  プロセスイベント
 */
enum {
	TUSBH_PORT_EVENT = 1,
	TUSBH_URB_EVENT,
	TUSBH_TIME_EVENT,
	TUSBH_CLASS_EVENT,
	TUSBH_RESET_EVENT
};

enum {
	TUSBH_CONNECT_EVENT = 1,
	TUSBH_DISCONNECT_EVENT,
	TUSBH_IDCHANGE_EVENT,
};

/*
 *  デバイス状態定義
 */
enum {
	DEVICE_IDLE = 0,
	DEVICE_ATTACHED,
	DEVICE_DISCONNECTED,
	DEVICE_CHECK_CLASS,
	DEVICE_CLASS,
	DEVICE_SUSPENDED,
	DEVICE_ABORT_STATE,
};

/*
 *  CONTROL通信状態
 */
enum {
	CONTROL_IDLE = 0,
	CONTROL_SETUP,
	CONTROL_SETUP_WAIT,
	CONTROL_DATA_IN_WAIT,
	CONTROL_DATA_OUT_WAIT,
	CONTROL_STATUS_IN_WAIT,
	CONTROL_STATUS_OUT_WAIT,
	CONTROL_ERROR
};


#define USBH_HandleTypeDef     TUSBH_Handle_t
typedef struct _TUSBH_Handle_t TUSBH_Handle_t;
typedef struct _TUSBH_Class_t  TUSBH_Class_t;
typedef struct _TUSBH_Device_t TUSBH_Device_t;

typedef struct
{
	uint16_t                   attribute;
	uint8_t                    index;
	uint8_t                    type;
} USBH_Pipe_t;

/*
 *  USBホストクラス定義
 */
struct _TUSBH_Class_t
{
	TUSBH_Class_t              *pNext;
	const char                 *Name;
	uint32_t                   classCode;
	TUSBH_ERCODE               (*Init)(TUSBH_Device_t *pdevice);
	TUSBH_ERCODE               (*DeInit)(TUSBH_Device_t *pdevice);
	TUSBH_ERCODE               (*Process)(TUSBH_Device_t *pdevice, uint8_t *mes);
	void                       *subfunc;
};

/*
 *  USBホストデバイスハンドラ
 */
struct _TUSBH_Device_t
{
	TUSBH_Device_t             *pNext;
	TUSBH_Handle_t             *pHost;
	TUSBH_Class_t              *pClass;
	uint8_t                    activate;
	uint8_t                    address;
	uint8_t                    speed;
	volatile uint8_t           is_connected;
	uint8_t                    pre_connected;
	uint8_t                    sel_interface;
	uint8_t                    hub;
	uint8_t                    port;
	uint8_t                    idx;
	volatile uint8_t           dstate;
	uint16_t                   timeid;
	uint32_t                   timecount;

	uint8_t                    cntl_pipe_in;
	uint8_t                    cntl_pipe_out;
	uint8_t                    cntl_pipe_size;
	uint8_t                    cntl_errcount;
    uint8_t                    setupPacket[8];
	uint32_t                   dummy[6];	/* dummy for cache */
	uint8_t                    Data[TUSBH_MAX_DATA_BUFFER];
	DeviceDescriptor           DevDesc;
	ConfigurationDescriptor    CfgDesc;
	InterfaceDescriptor        ItfDesc[TUSBH_MAX_NUM_INTERFACES];
	EndpointDescriptor         EpDesc[TUSBH_MAX_NUM_INTERFACES][TUSBH_MAX_NUM_ENDPOINTS];

	uint8_t                    type;
	uint8_t                    hubid;
	volatile uint8_t           cstate;
	uint8_t                    numUnit;
	void                       *pData;
	uint8_t                    *cbuff;
	char                       Manufacturer[TUSBH_STR_LENGTH];
	char                       Prodeuct[TUSBH_STR_LENGTH];
	char                       Serial[TUSBH_STR_LENGTH];
};

/*
 *  USBホストハンドラ
 */
struct _TUSBH_Handle_t
{
	TUSBH_Device_t             *pDevice;
	TUSBH_Class_t              *pClass;
	USBH_Pipe_t                pipes[MAX_EPS_PIPES];
	uint32_t                   numPipes;
	void                       *pSysData;
	uint8_t                    *pCfgData;
	void                       (*usrcallback)(TUSBH_Handle_t *pHandle, TUSBH_Device_t *pdevice, uint8_t id);

	uint8_t                    id;
	uint8_t                    numDevice;
	uint8_t                    numClass;
	uint8_t                    numHub;
	ID                         processTaskID;
	ID                         eventTaskID;
	ID                         process_event;
	ID                         connect_event;
};

#include "tusbh_conf.h"

/*
 *  関数プロトタイプ宣言
 */
TUSBH_ERCODE tusbhInit(TUSBH_Handle_t *phost, void (*pufunc)(TUSBH_Handle_t*, TUSBH_Device_t *, uint8_t ), uint8_t id);
TUSBH_ERCODE tusbhDeInit(TUSBH_Handle_t *phost);
TUSBH_ERCODE tusbhLinkClass(TUSBH_Handle_t *phost, TUSBH_Class_t *pclass);
TUSBH_ERCODE tusbhStart(TUSBH_Handle_t *phost);
TUSBH_ERCODE tusbhStop(TUSBH_Handle_t *phost);

TUSBH_ERCODE tusbhResetHub(TUSBH_Device_t *pdevice);
TUSBH_ERCODE tusbhSelectInterface(TUSBH_Device_t *pdevice, uint8_t interface);
uint8_t      tusbhFindInterface(TUSBH_Device_t *pdevice, uint8_t Class, uint8_t SubClass, uint8_t Protocol);
uint8_t      tusbhGetActiveClass(TUSBH_Device_t *pdevice);
uint8_t      tusbhFindInterfacebynumber(TUSBH_Device_t *pdevice, uint8_t interface, uint8_t altsetno);

TUSBH_ERCODE tusbhControlRequest(TUSBH_Device_t *pdevice, uint8_t *buff, uint8_t type, uint8_t request, uint16_t value, uint16_t index, uint16_t length);
TUSBH_ERCODE tusbhControlWait(TUSBH_Device_t *pdevice, uint8_t *mes);
TUSBH_ERCODE tusbhGetDescriptor(TUSBH_Device_t *pdevice, uint8_t req_type, uint16_t value, uint8_t* buff, uint16_t length);
TUSBH_ERCODE tusbhSetInterface(TUSBH_Device_t *pdevice, uint8_t ep_num, uint8_t interface);
TUSBH_ERCODE tusbhClearFeature(TUSBH_Device_t *pdevice, uint8_t ep_num);

TUSBH_ERCODE tusbhOpenPipe(TUSBH_Device_t *, uint8_t, uint8_t, uint8_t, uint16_t);
TUSBH_ERCODE tusbhClosePipe(TUSBH_Device_t *, uint8_t pipe);
TUSBH_ERCODE tusbhSubmitURB(TUSBH_Device_t *pdevice, uint8_t, uint16_t, uint8_t, uint8_t*, uint16_t);
TUSBH_ERCODE tusbhControlSendSetup(TUSBH_Device_t *pdevice, uint8_t *buff, uint8_t pipe_num);
TUSBH_ERCODE tusbhControlSendData(TUSBH_Device_t *pdevice, uint8_t *buff, uint16_t length, uint8_t pipe_num);
TUSBH_ERCODE tusbhControlReceiveData(TUSBH_Device_t *pdevice, uint8_t* buff, uint16_t length, uint8_t pipe_num);
TUSBH_ERCODE tusbhBulkWrite(TUSBH_Device_t *pdevice, uint8_t *buff, uint16_t length, uint8_t pipe_num);
TUSBH_ERCODE tusbhBulkRead(TUSBH_Device_t *pdevice, uint8_t *buff, uint16_t length, uint8_t pipe_num);
TUSBH_ERCODE tusbhInterruptWrite(TUSBH_Device_t *pdevice, uint8_t *buff, uint8_t length, uint8_t pipe_num);
TUSBH_ERCODE tusbhInterruptRead(TUSBH_Device_t *pdevice, uint8_t *buff, uint8_t length, uint8_t pipe_num);
TUSBH_ERCODE tusbhIsocWrite(TUSBH_Device_t *pdevice, uint8_t *buff, uint32_t length, uint8_t pipe_num);
TUSBH_ERCODE tusbhIsocRead(TUSBH_Device_t *pdevice, uint8_t *buff, uint32_t length, uint8_t pipe_num);

void tusbhProcessTask(intptr_t exinf);
void tusbhEventTask(intptr_t exinf);
void tusbhCyclicHandler(intptr_t exinf);

TUSBH_Device_t *tusbhSearchDevice(TUSBH_Handle_t *phost, uint8_t classCode, uint8_t *plun);
uint8_t tusbhAllocPipe(TUSBH_Handle_t *phost, uint8_t ep_addr);
TUSBH_ERCODE tusbFreePipe(TUSBH_Handle_t *phost, uint8_t no);


#ifdef __cplusplus
}
#endif

#endif /* _HUSBH_BASE_H_ */

