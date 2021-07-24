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
 *  @(#) $Id: tusbh_msc.h 698 2017-09-12 18:44:27Z roi $
 */
/*
 *  USB Host Middleware MSC部定義
 */

#ifndef _HUSBH_MSC_H_
#define _HUSBH_MSC_H_

#include "tusbh_base.h"

#ifdef __cplusplus
 extern "C" {
#endif


#ifndef NUM_MAX_MSC_UNIT
#define NUM_MAX_MSC_UNIT                2
#endif

/*
 *  MSCインターフェイス定義
 */
#define MSC_TRANSPARENT_SUBCLASS        0x06
#define MSC_BOT_PROTOCOL                0x50

/*
 *  BOT通信パケット定義
 */ 
#define OPCODE_TEST_UNIT_READY          0x00
#define OPCODE_READ_CAPACITY10          0x25
#define OPCODE_READ10                   0x28
#define OPCODE_WRITE10                  0x2A
#define OPCODE_REQUEST_SENSE            0x03
#define OPCODE_INQUIRY                  0x12

#define DATA_LEN_MODE_TEST_UNIT_READY   0
#define DATA_LEN_READ_CAPACITY10        8
#define DATA_LEN_INQUIRY                36
#define DATA_LEN_REQUEST_SENSE          14

#define CBW_CB_LENGTH                   16
#define CBW_LENGTH                      10

/*
 *  SENSE KEY定義
 */
#define SCSI_SENSE_KEY_NO_SENSE         0x00
#define SCSI_SENSE_KEY_RECOVERED_ERROR  0x01
#define SCSI_SENSE_KEY_NOT_READY        0x02
#define SCSI_SENSE_KEY_MEDIUM_ERROR     0x03
#define SCSI_SENSE_KEY_HARDWARE_ERROR   0x04
#define SCSI_SENSE_KEY_ILLEGAL_REQUEST  0x05
#define SCSI_SENSE_KEY_UNIT_ATTENTION   0x06
#define SCSI_SENSE_KEY_DATA_PROTECT     0x07
#define SCSI_SENSE_KEY_BLANK_CHECK      0x08
#define SCSI_SENSE_KEY_VENDOR_SPECIFIC  0x09
#define SCSI_SENSE_KEY_COPY_ABORTED     0x0A
#define SCSI_SENSE_KEY_ABORTED_COMMAND  0x0B
#define SCSI_SENSE_KEY_VOLUME_OVERFLOW  0x0D
#define SCSI_SENSE_KEY_MISCOMPARE       0x0E

/*
 *  MSCコントロール通信定義
 */
#define USB_REQ_BOT_RESET               0xFF
#define USB_REQ_GET_MAX_LUN             0xFE

#define BOT_REQUEST_GETMAXLUN_TYPE      (USB_DEVICE_TO_HOST | USB_REQUEST_TYPE_CLASS | USB_RECIPIENT_INTERFACE)
#define BOT_REQUEST_RESET_TYPE          (USB_HOST_TO_DEVICE | USB_REQUEST_TYPE_CLASS | USB_RECIPIENT_INTERFACE)

/*
 *  MSCプロセスの状態定義
 */
enum
{
	MSC_PROCESS_INIT = 0,
	MSC_PROCESS_IDLE,
	MSC_PROCESS_READ_WAIT,
	MSC_PROCESS_WRITE_WAIT,
	MSC_PROCESS_REQUEST_SENSE_WAIT
};

/*
 *  MSCユニットの状態定義
 */
typedef enum
{
	MSC_UNIT_INIT = 0,
	MSC_UNIT_TEST_UNIT_READY_WAIT,
	MSC_UNIT_READ_CAPACITY_WAIT,
	MSC_UNIT_READ_INQUIRY_WAIT,
	MSC_UNIT_REQUEST_SENSE_WAIT,
	MSC_UNIT_ERROR,
	MSC_UNIT_IDLE,
	MSC_UNIT_READ_WAIT,
	MSC_UNIT_WRITE_WAIT,
} MSC_UNIT_STATE_t;

/*
 *  BOT状態定義
 */
typedef enum {
	BOT_SEND_CBW  = 1,
	BOT_SEND_CBW_WAIT,
	BOT_DATA_IN_WAIT,
	BOT_DATA_OUT_WAIT,
	BOT_RECEIVE_CSW_WAIT,
	BOT_STATE_ERROR,
	BOT_STATE_ERROR_WAIT
} BOT_STATE_t;

typedef struct
{
	uint32_t                 Signature;
	uint32_t                 Tag;
	uint32_t                 DataTransferLength;
	uint8_t                  Flags;
	uint8_t                  LUN;
	uint8_t                  CBLength;
	uint8_t                  CB[CBW_CB_LENGTH];
} BOT_CBW_t;

typedef struct
{
	uint32_t                 Signature;
	uint32_t                 Tag;
	uint32_t                 DataResidue;
	uint8_t                  Status;
} BOT_CSW_t;

/*
 *  BOTハンドラ型定義
 */
typedef struct
{
	uint32_t                 data[32];
	BOT_CBW_t                cbw;
	uint32_t                 dummy2[8];
	BOT_CSW_t                csw;
	uint8_t                  dummy3[12];
	uint8_t                  *pbuf;
	BOT_STATE_t              state;
} BOT_Handle_t;

/*
 *  MSC ユニット定義
 */
typedef struct
{
	MSC_UNIT_STATE_t            state; 
	MSC_UNIT_STATE_t            pre_state;
	uint32_t                    num_block;
	uint32_t                    block_size;
	uint8_t                     DeviceType;
	uint8_t                     PeripheralQualifier;
	uint8_t                     RemovableMedia;
	uint8_t                     vendor_id[12];
	uint8_t                     product_id[20];
	uint8_t                     revision_id[8];
	TUSBH_ERCODE                rw_status;
	int32_t                     rw_taskid;
} MSC_Unit_t;

/*
 *  MSCハンドラ定義
 */
typedef struct
{
	TUSBH_Device_t            *pDev;
	uint8_t                   state;
	uint8_t                   max_lun;
    uint8_t                   lun;
	uint8_t                   dymmy1;

	uint8_t                   InPipe;
	uint8_t                   OutPipe;
	uint8_t                   OutEp;
	uint8_t                   InEp;
	uint16_t                  OutEpSize;
	uint16_t                  InEpSize;
	uint32_t                  timer;
	uint32_t                  urb_wait;
	uint32_t                  dummy[2];
	BOT_Handle_t              hbot;
	MSC_Unit_t                unit[NUM_MAX_MSC_UNIT];
} MSC_Handle_t;


void         tusbhLinkMSC(TUSBH_Handle_t *phost);
uint8_t      tusbhMscUnitIsReady(TUSBH_Handle_t *phost, uint8_t lun);
TUSBH_ERCODE tusbhMscGetLUNInfo(TUSBH_Handle_t *phost, uint8_t lun, MSC_Unit_t *info);
TUSBH_ERCODE tusbhMscRead(TUSBH_Handle_t *phost, uint8_t lun, uint32_t address, uint8_t *pbuf, uint32_t num);
TUSBH_ERCODE tusbhMscWrite(TUSBH_Handle_t *phost, uint8_t lun, uint32_t address, uint8_t *pbuf, uint32_t num);



#ifdef __cplusplus
}
#endif

#endif /* _HUSBH_MSC_H_ */

