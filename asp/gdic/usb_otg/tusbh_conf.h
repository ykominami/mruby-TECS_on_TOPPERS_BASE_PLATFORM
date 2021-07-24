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
 *  @(#) $Id: tusbh_conf.h 698 2017-09-23 16:49:00Z roi $
 */
/*
 *  TOPPERS USB HOST MODULE CONFIGURATION インクルードファイル
 */

#ifndef _TUSBH_CONF_H_
#define _TUSBH_CONF_H_

#include "dwc2_host_driver.h"

#define tusbhHDStop(h)              usbo_stophost((h)->pSysData)
#define tusbhHDResetPort(h)         usbo_resetport((h)->pSysData)
#define tusbhHDTrasLength           dwc2_host_gettraslength
#define tusbhHDControlVBUS(h, s)    usbo_driveextvbus((h)->pSysData, s)

#define tusbhHDInit         dwc2_host_init
#define tusbhHDStart        dwc2_host_start
#define tusbhHDGetSpeed     dwc2_host_getspeed
#define tusbhHDOpenPipe     dwc2_host_openpipe
#define tusbhHDClosePipe    dwc2_host_closepipe
#define tsubhHDSubmitURB    dwc2_host_submiturb
#define tusbhHDCheckPipe    dwc2_host_checkpipe
#define tusbhHDSetToggle    dwc2_host_settoggle
#define tusbhHDGetToggle    dwc2_host_gettoggle

#endif /* _TUSBH_CONF_H_ */

