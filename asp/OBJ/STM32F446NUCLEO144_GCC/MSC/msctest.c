/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2012 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2017 by TOPPERS PROJECT Educational Working Group.
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
 *  $Id: media.c 2416 2012-09-07 08:06:20Z ertl-hiro $
 */

/* 
 *  MEDIA(MSC)テストの本体
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "kernel_cfg.h"
#include <target_syssvc.h>
#include "device.h"
#include "storagedevice.h"
#include "fcntl.h"
#include "mscdiskio.h"
#include "monitor.h"
#include "msctest.h"

/*
 *  サービスコールのエラーのログ出力
 */
Inline void
svc_perror(const char *file, int_t line, const char *expr, ER ercd)
{
	if (ercd < 0) {
		t_perror(LOG_ERROR, file, line, expr, ercd);
	}
}

#define	SVC_PERROR(expr)	svc_perror(__FILE__, __LINE__, #expr, (expr))

uint8_t ukey;
static uint8_t Buffer[512];
static uint32_t heap_area[4*1024];

uint32_t heap_param[2] = {
	(uint32_t)heap_area,
	(4*4*1024)
};

static int_t msc_read(int argc, char **argv);

/*
 *  MSCコマンドテーブル
 */
static const COMMAND_INFO msc_command_info[] = {
	{"READ",		msc_read }		/* MSC SECTOR READ */
};

#define NUM_MSC_CMD      (sizeof(msc_command_info)/sizeof(COMMAND_INFO))

static const char msc_name[] = "MSC";
static const char msc_help[] =
"  Msc     READ     [no]\n";

static COMMAND_LINK msc_command_link = {
	NULL,
	NUM_MSC_CMD,
	msc_name,
	NULL,
	msc_help,
	&msc_command_info[0]
};

static int a2i(char *str)
{
	int num = 0;

	while(*str >= '0' && *str <= '9'){
		num = num * 10 + *str++ - '0';
	}
	return num;
}

/*
 *  MSC SECTOR READ
 */
static int_t
msc_read(int argc, char **argv)
{
	StorageDevice_t *psdev;
	USBH_HandleTypeDef *husbh;
	TUSBH_ERCODE       status;
	char               pdrv;
	int arg1, arg2, i;

	if(argc < 2)
		return -1;
	arg1 = a2i(argv[1]);
	arg2 = 1;
	if(arg1 < 0)
		return -1;
	if(argc >= 3)
		arg2 = a2i(argv[2]);

	psdev = SDMGetStorageDevice(MSC_DEVNO);
	husbh = (USBH_HandleTypeDef *)psdev->_sdev_local[1];
	pdrv  = psdev->_sdev_port - MSC_PORTID;
	while(arg2 > 0){
		status = tusbhMscRead(husbh, pdrv, arg1, (uint8_t *)Buffer, 1);
		printf("\nsec(%d) status(%d) !", arg1, status);
		if(status == TUSBH_E_OK){
			for(i = 0 ; i < 512 ; i++){
				if((i % 16) == 0)
					printf("\n%03x ", i);
				printf(" %02x", Buffer[i]);
			}
			printf("\n");
		}
		dly_tsk(100);
		arg1++;
		arg2--;
	}
	return 0;
}

/*
 *  SW1割込み
 */
void sw_int(void)
{
	ukey = 1;
	syslog_0(LOG_NOTICE, "## sw_int() ##");
}

/*
 *  USB HOST INFORMATION CALLBACK
 *  parameter1:  Host Handler
 *  parameter2:  Device Handler
 *  parameter3:  Host state ID.
 */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, TUSBH_Device_t *pdevice, uint8_t id)
{
	if(pdevice->pClass != NULL){
		syslog_3(LOG_NOTICE, "## CLASS[%s](%d)(%d) ##", pdevice->pClass->Name, pdevice->type, id);
	}
}

/*
 *  メインタスク
 */
void main_task(intptr_t exinf)
{
	StorageDevice_t *psdev;
	USB_OTG_Init_t USB_Data_Init;
	ER_UINT	ercd;
	TUSBH_ERCODE result;
	USB_OTG_Handle_t *husb;
	USBH_HandleTypeDef *phusbh;

	SVC_PERROR(syslog_msk_log(LOG_UPTO(LOG_INFO), LOG_UPTO(LOG_EMERG)));
	syslog(LOG_NOTICE, "Sample program starts (exinf = %d).", (int_t) exinf);

	/*
	 *  シリアルポートの初期化
	 *
	 *  システムログタスクと同じシリアルポートを使う場合など，シリアル
	 *  ポートがオープン済みの場合にはここでE_OBJエラーになるが，支障は
	 *  ない．
	 */
	ercd = serial_opn_por(TASK_PORTID);
	if (ercd < 0 && MERCD(ercd) != E_OBJ) {
		syslog(LOG_ERROR, "%s (%d) reported by `serial_opn_por'.",
									itron_strerror(ercd), SERCD(ercd));
	}
	SVC_PERROR(serial_ctl_por(TASK_PORTID,
							(IOCTL_CRLF | IOCTL_FCSND | IOCTL_FCRCV)));

	dly_tsk(2000);

	/*
	 *  USB OTGハードの初期化
	 */
    USB_Data_Init.usb_otg_mode = USB_OTG_MODE_HOST;
	USB_Data_Init.host_channels = 11;	/* HOST */
	USB_Data_Init.dma_enable = 0;
	USB_Data_Init.low_power_enable = 0;
	USB_Data_Init.phy_itface = USB_PHY_EMBEDDED; 
	USB_Data_Init.sof_enable = 0;
	USB_Data_Init.speed = USB_SPEED_FULL;
	USB_Data_Init.vbus_sensing_enable = 0;	/* HOST/DEV */
	USB_Data_Init.lpm_enable = 0;
	USB_Data_Init.use_external_vbus = 0;
	husb = usbo_init(USB1_PORTID, &USB_Data_Init);
#if 1	/* ROI DEBUG */
	if(husb == NULL){
		syslog_0(LOG_NOTICE, "## usbo_init error STOP ##");
		slp_tsk();
	}
#endif	/* ROI DEBUG */

	/*
	 *  USB HOSTミドルウェア設定
	 */
	phusbh = (USBH_HandleTypeDef *)malloc(sizeof(USBH_HandleTypeDef));
	phusbh->pSysData = husb;
	tusbhInit(phusbh, USBH_UserProcess, 0);

	psdev = SDMGetStorageDevice(MSC_DEVNO);
	psdev->_sdev_local[1] = phusbh;

	/*
	 *  USB HOSTスタート
	 */
	result = tusbhStart(phusbh);
	syslog_1(LOG_NOTICE, "## tusbhStart result(%d) ##", result);
	setup_command(&msc_command_link);

#if 1	/* ROI DEBUG */
	syslog_0(LOG_NOTICE, "## STOP ##");
	slp_tsk();
#endif	/* ROI DEBUG */

	syslog(LOG_NOTICE, "Sample program ends.");
//	SVC_PERROR(ext_ker());
}

