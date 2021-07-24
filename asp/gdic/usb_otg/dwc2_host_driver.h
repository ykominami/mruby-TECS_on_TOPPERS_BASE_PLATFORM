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
 *  @(#) $Id: dwc2_host_driver.h 698 2017-09-23 16:44:40Z roi $
 */
/*
 *  DWC2 HIGH DRIVER インクルードファイル
 */


#ifndef _DWC2_HOST_DRIVER_H_
#define _DWC2_HOST_DRIVER_H_

#include "tusb_rtos.h"
#include "usb_otg.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define dwc2_host_gettraslength(h, n)   (((USB_OTG_Handle_t *)((h)->pSysData))->hc[(n)].xfer_count)

/*
 *  DWC2 HOST HIGH DRIVER関数プロトタイプ宣言
 */
extern TUSBH_ERCODE dwc2_host_init(TUSBH_Handle_t *phost);
extern TUSBH_ERCODE dwc2_host_deinit(TUSBH_Handle_t *phost);
extern TUSBH_ERCODE dwc2_host_start(TUSBH_Handle_t *phost);
extern uint8_t      dwc2_host_getspeed(TUSBH_Handle_t *phost);

extern TUSBH_ERCODE dwc2_host_openpipe(TUSBH_Handle_t *phost, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint16_t);
extern TUSBH_ERCODE dwc2_host_closepipe(TUSBH_Handle_t *phost, uint8_t);
extern TUSBH_ERCODE dwc2_host_submiturb(TUSBH_Handle_t *phost, uint8_t, uint8_t,  uint8_t, uint8_t*, uint16_t, uint8_t);
extern bool_t       dwc2_host_checkpipe(TUSBH_Handle_t *phost, uint8_t pipe);
extern TUSBH_ERCODE dwc2_host_settoggle(TUSBH_Handle_t *phost, uint8_t , uint8_t);
extern uint8_t      dwc2_host_gettoggle(TUSBH_Handle_t *phost, uint8_t);

#ifdef __cplusplus
}
#endif

#endif /* _DWC2_HOST_DRIVER_H_ */

