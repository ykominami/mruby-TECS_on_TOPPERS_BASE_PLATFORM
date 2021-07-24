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
 *  @(#) $Id: tusbh_rtos.c 698 2017-09-20 18:35:20Z roi $
 */
/*
 *  USB Middleware RTOS依存部
 */

#include "tusb_rtos.h"
#include "kernel_cfg.h"


/*
 *  タスク状態でのメッセージ送信
 */
ER
tusbSendData(ID qid, uint8_t evt, uint8_t prm1, uint8_t prm2, uint8_t prm3)
{
	intptr_t data;
	ER       ercd;

	data = (evt << 24) | (prm1 << 16) | (prm2 << 8) | prm3;
	ercd = psnd_dtq(qid, data);
	if(ercd != E_OK){
		syslog_5(LOG_ERROR, "tusbSendData send(%d) error [%02x][%02x][%02x][%02x]", qid, evt, prm1, prm2, prm3);
	}
	return ercd;
}

/*
 *  非タスク状態でのメッセージ送信
 */
ER
tusbiSendData(ID qid, uint8_t evt, uint8_t prm1, uint8_t prm2, uint8_t prm3)
{
	intptr_t data;
	ER       ercd;

	data = (evt << 24) | (prm1 << 16) | (prm2 << 8) | prm3;
	ercd = ipsnd_dtq(qid, data);
	if(ercd != E_OK){
		syslog_5(LOG_ERROR, "tusbiSendData send(%d) error [%02x][%02x][%02x][%02x]", qid, evt, prm1, prm2, prm3);
	}
	return ercd;
}

/*
 *  タスク状態でのメッセージ受信
 */
ER
tusbRecvData(ID qid, uint8_t *pmes, uint32_t timeout)
{
	intptr_t data;
	ER       ercd;

	if(timeout == 0)
		ercd = rcv_dtq(qid, &data);
	else
		ercd = trcv_dtq(qid, &data, timeout);
	if(ercd != E_OK){
		syslog_1(LOG_ERROR, "tusbRecvData(%d) error !", qid);
	}
	else{
		pmes[0] = (data >> 24) & 0xff;
		pmes[1] = (data >> 16) & 0xff;
		pmes[2] = (data >> 8) & 0xff;
		pmes[3] = data & 0xff;
	}
	return ercd;
}

/*
 *  タスクID取出し
 */
int32_t
tusbGetTaskID(void)
{
	ER ercd;
	ID tid = 0;

	ercd = get_tid(&tid);
	if(ercd != E_OK){
		syslog_1(LOG_ERROR, "tusbhGetTaskID get fail(%d) !", ercd);
		return 0;
	}
	else
		return (int32_t)tid;
}

/*
 *  タスク起動
 */
ER
tusbStartTask(ID taskid)
{
	ER ercd = act_tsk(taskid);

	if(ercd != E_OK)
		syslog_2(LOG_ERROR, "tusbStartTask actvate error id(%d) error(%d) !", taskid, ercd);
	return ercd;
}

/*
 *  タイマー制御
 */
ER
tusbTimerControl(uint8_t activate)
{
	ER ercd;

	if(activate == 0)
		ercd = stp_cyc(TUSBH_CYC_HDR);
	else
		ercd = sta_cyc(TUSBH_CYC_HDR);
	return ercd;
}

/*
 *  ローカルなmemset関数
 */
void
tusbmemset(void *d, const char val, int len)
{
	char *dd = d;
	int i;
	for(i = 0 ; i < len ; i++){
		*dd++ = val;
	}
}

/*
 *  ローカルなmemcpy関数
 */
void
tusbmemcpy(void *d, void *s, int len)
{
	uint8_t *dd = d;
	uint8_t *ss = s;
	int i;
	for(i = 0 ; i < len ; i++){
		*dd++ = *ss++;
	}
}

