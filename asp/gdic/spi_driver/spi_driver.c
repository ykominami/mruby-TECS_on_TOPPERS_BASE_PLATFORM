/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2012 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2015-2017 by TOPPERS PROJECT Educational Working Group.
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
 *  $Id: spi_driver.c 2416 2017-04-15 07:19:38Z roi $
 */

/* 
 *  SPI用デバイスドライバ
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <string.h>
#include <target_syssvc.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "device.h"
#include "spi.h"
#include "pinmode.h"
#include "spi_driver.h"

#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))
#define sil_andw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) & ~(b))
#define sil_modw_mem(a, b, c)	sil_wrw_mem((a), (sil_rew_mem(a) & (~b)) | (c))

#define INIT_RETRY_COUNT    32

#define CRC16POLY           0x1021

static const uint8_t CMD0[5]   = {0x40, 0x00, 0x00, 0x00, 0x00};
static const uint8_t CMD8[5]   = {0x48, 0x00, 0x00, 0x01, 0xAA};
static const uint8_t CMD9[5]   = {0x49, 0x00, 0x00, 0x00, 0x00};
static const uint8_t CMD10[5]  = {0x4A, 0x00, 0x00, 0x00, 0x00};
static const uint8_t CMD16[5]  = {0x50, 0x00, 0x00, 0x02, 0x00};
static const uint8_t CMD55[5]  = {0x77, 0x00, 0x00, 0x00, 0x00};
static const uint8_t ACMD41[5] = {0x69, 0x40, 0x00, 0x00, 0x00};
static const uint8_t CMD58[5]  = {0x7A, 0x00, 0x00, 0x00, 0x00};

static bool_t           sdcard_sense = true;
static SDCARD_Handler_t SdHandle;
static uint8_t          cs2_port;

/* Buffer used for transmission */
uint8_t aTxBuffer[BUFFERSIZE];

/* Buffer used for reception */
uint8_t aRxBuffer[BUFFERSIZE];

/*
 *  GPIO設定関数
 */
static void
pinMode(uint8_t no, uint32_t otype, uint32_t pullmode)
{
	Arduino_PortControlBlock *pgcb = getGpioTable(DIGITAL_PIN, no);
	GPIO_Init_t GPIO_Init_Data;

	if(pgcb == NULL)
		return;
	pinClock(no);
	GPIO_Init_Data.mode  = GPIO_MODE_OUTPUT;
	GPIO_Init_Data.pull  = pullmode;
	GPIO_Init_Data.otype = otype;
	GPIO_Init_Data.speed = GPIO_SPEED_FAST;
	gpio_setup(pgcb->giobase, &GPIO_Init_Data, pgcb->giopin);
}

/*
 *  SPI初期化関数
 */
SPI_Handle_t *
spi_start(SPI_Setup_t *psps)
{
	SPI_Init_t spi_initd;
	SPI_Handle_t  *hspi;

	/*
	 *  CS PORT(D10-PB6)
	 */
	pinMode(psps->cs_pin, psps->otype, psps->pullmode);
	/*
	 *  RS PORT(D8-PA9)
	 */
	pinMode(psps->rs_pin, psps->otype, psps->pullmode);
	/*
	 *  RST PORT(D9-PC7)
	 */
	pinMode(psps->rst_pin, psps->otype, psps->pullmode);
	/*
	 *  CS2 PORT(D4-PB5)
	 */
	pinMode(psps->cs2_pin, psps->otype, psps->pullmode);
	cs2_port = psps->cs2_pin;

	spi_initd.Prescaler     = psps->baud;
	spi_initd.Direction     = SPI_DIRECTION_2LINES;
	spi_initd.CLKPhase      = SPI_PHASE_1EDGE;
	spi_initd.CLKPolarity   = SPI_POLARITY_LOW;
	spi_initd.CRC           = SPI_CRC_DISABLE;
	spi_initd.CRCPolynomial = 7;
	spi_initd.DataSize      = SPI_DATASIZE_8BIT;
	spi_initd.SignBit       = SPI_DATA_MSB;
	spi_initd.NSS           = SPI_NSS_SOFT;
	spi_initd.TIMode        = SPI_TIMODE_DISABLE;
	spi_initd.Mode = SPI_MODE_MASTER;
	spi_initd.semid   = psps->spi_sem;
	spi_initd.semlock = 0;

	if((hspi = spi_init(psps->port, &spi_initd)) == NULL){
		syslog_0(LOG_ERROR, "spi_start initialize error !");
	}
	return hspi;
}

/*
 *  SPI終了関数
 */
void
spi_end(SPI_Handle_t *hspi)
{
	spi_deinit(hspi);
}


/*
 *  CRC7計算関数
 */
static uint8_t
calc_CRC7(const uint8_t *data, int len)
{
	uint_t  i, j;
	uint8_t crc = 0, dt;

	for(i = 0; i < len; i++){
		dt = *data++;
		for(j = 0; j < 8; j++){
			crc <<=1;
			if ((crc & 0x80) ^ (dt & 0x80))
				crc ^= 0x09;
			dt <<= 1;
		}
	}
	return ((crc & 0x7f) << 1) | 1;
}

/*
 *  CRC16-CCITT計算関数
 */
static uint16_t
calc_CRC16(const uint8_t *data, int len)
{
	uint16_t crc = 0;
	int i, j;

	for(i = 0; i < len; i++){
		crc ^= (data[i]<<8);
		for(j = 0; j < 8; j++){
			if(crc & 0x8000)
				crc = (crc << 1) ^ CRC16POLY;
			else
				crc <<= 1;
		}
	}
	return crc;
}

/*
 *  SDCARDからのデータ読み込み
 */
static ER
sdcard_read(SPI_Handle_t *hspi, uint8_t *pres, int size)
{
	ER ercd;

#if SPI_WAIT_TIME != 0
	ercd = spi_receive(hspi, (uint8_t*)pres, size);
#else
	if((ercd = spi_receive(hspi, (uint8_t*)pres, size)) == E_OK){
		ercd = spi_wait(hspi, 50);
	}
#endif
	if(ercd != E_OK)
    	syslog_1(LOG_ERROR, "sdcard_read ERROR(%d) !", ercd);
	return ercd;
}

/*
 *  SDCARDへのデータ書き込み
 */
static ER
sdcard_write(SPI_Handle_t *hspi, uint8_t *pres, int size)
{
	ER ercd;

#if SPI_WAIT_TIME != 0
	ercd = spi_transmit(hspi, (uint8_t*)pres, size);
#else
	if((ercd = spi_transmit(hspi, (uint8_t*)pres, size)) == E_OK){
		ercd = spi_wait(hspi, 50);
	}
#endif
	if(ercd != E_OK)
    	syslog_1(LOG_ERROR, "sdcard_write ERROR(%d) !", ercd);
	return ercd;
}

/*
 *  SDCARDへのコマンド送信
 */
static ER
sdcard_sendcommand(SPI_Handle_t *hspi, const uint8_t *cmd, int size)
{
	ER ercd = E_OK;
	uint8_t crc;

	crc = calc_CRC7(cmd, size);
	memcpy(aTxBuffer, cmd, size);
	aTxBuffer[size] = crc;
#if SPI_WAIT_TIME != 0
	ercd = spi_transmit(hspi, (uint8_t*)aTxBuffer, (size+1));
#else
	if((ercd = spi_transmit(hspi, (uint8_t*)aTxBuffer, (size+1))) == E_OK){
		ercd = spi_wait(hspi, 50);
	}
#endif
	return ercd;
}

/*
 *  SDCARDへのダミー書き込み
 */
static ER
sdcard_dummywrite(SPI_Handle_t *hspi)
{
	uint8_t dummy[2];
	dummy[0] = 0xff;
	return sdcard_write(hspi, dummy, 1);
}

/*
 *  SDCARDからのレスポンス受信
 */
static ER
sdcard_response(SPI_Handle_t *hspi, uint8_t *pres, int size, int timeout)
{
	int tick, i = 0;

	timeout *= 100;
	for(tick = 0 ; tick < timeout ; tick++){
		if(sdcard_read(hspi, aRxBuffer, 1) == E_OK){
			pres[i] = aRxBuffer[0];
			if((aRxBuffer[0] & 0x80) == 0){
				i++;
				if(i >= size)
					return E_OK;
			}
		}
		sil_dly_nse(1000*10);
	}
	return E_TMOUT;
}

/*
 *  SDCARDからのデータ受信
 */
static ER
sdcard_receive(SPI_Handle_t *hspi, uint8_t *pdata, int size, uint16_t *crc, int timeout)
{
	ER  ercd = E_OK;
	bool_t ff_count;
	int tick, i = 0;

	for(tick = 0 ; tick < (timeout*1000) ; tick++){
		*crc = 0xffff;
		if((ercd = sdcard_read(hspi, aRxBuffer, 1)) == E_OK){
			if(aRxBuffer[0] == 0xfe){
				if((ercd = sdcard_read(hspi, pdata, size)) != E_OK)
					return ercd;
				ercd = E_TMOUT;
				for(i = 0, ff_count = 0 ; i < 24 ; i++){
					sdcard_read(hspi, aRxBuffer, 1);
					dly_tsk(2);
					if(aRxBuffer[0] != 0xff){
						if(ff_count == 0)
							*crc = aRxBuffer[0]<<8;
						else if(ff_count == 1)
							*crc |= aRxBuffer[0];
						ff_count++;
					}
					if(i >= 1 && aRxBuffer[0] == 0xff && ff_count > 0)
						ercd = E_OK;
				}
				return ercd;
			}
			else if(aRxBuffer[0] != 0xff){
				syslog_2(LOG_NOTICE, "## sdcard_receive i(%d) resp[%02x] ##", i, aRxBuffer[0]);
			}
		}
		sil_dly_nse(1000);
	}
	return E_TMOUT;
}

/*
 *  SDCARDへのデータ送信
 */
static ER
sdcard_send(SPI_Handle_t *hspi, uint8_t *pdata, int size)
{
	ER  ercd = E_OK;
	uint8_t  data[4];
	uint16_t crc;
	int   res_count = 0;
	int i = 0;

	crc = calc_CRC16((const uint8_t *)pdata, size);
	data[0] = 0xff;
	data[1] = 0xfe;
	if((ercd = sdcard_write(hspi, data, 2)) != E_OK)
		return ercd;
	if((ercd = sdcard_write(hspi, pdata, size)) != E_OK)
		return ercd;
	data[0] = crc >> 8;
	data[1] = crc & 0xff;
	if((ercd = sdcard_write(hspi, data, 2)) != E_OK)
		return ercd;
	ercd = E_TMOUT;
	for(i = 0 ; i < 24 ; i++){
		sdcard_read(hspi, aRxBuffer, 1);
		dly_tsk(1);
		if(aRxBuffer[0] != 0xff){
			if(res_count == 0){
				if((aRxBuffer[0] & 0x1F) == 0x05)
					ercd = E_OK;
				else
					ercd = E_SYS;
				res_count++;
			}
			else if(aRxBuffer[0] == 0x00)
				res_count++;
		}
		else if(res_count >= 2)
			break;
	}
	return ercd;
}
/*
 *  SDCARD通信ロック
 */
static ER
sdcard_lock(SDCARD_Handler_t *hsd)
{
	ER ercd = E_OK;
	if(hsd->spi_lock != 0)
		ercd = wai_sem(hsd->spi_lock);
	digitalWrite(cs2_port, PORT_LOW);
	return ercd;
}

/*
 *  SDCARD通信アンロック
 */
static ER
sdcard_unlock(SDCARD_Handler_t *hsd)
{
	ER ercd = E_OK;
	digitalWrite(cs2_port, PORT_HIGH);
	if(hsd->spi_lock != 0)
		ercd = sig_sem(hsd->spi_lock);
	return ercd;
}

/*
 *  SDCARDリセット
 */
static ER
sdcard_reset(SDCARD_Handler_t *hsd)
{
	ER ercd = E_OK;
	uint8_t resp[2];
	int i = 0;

	if((ercd = sdcard_lock(hsd)) != E_OK){
		return ercd;
	}
	for(i = 0 ; i < 20 ; i++){
		if((ercd = sdcard_sendcommand(hsd->hspi, CMD55, sizeof(CMD55))) != E_OK)
			continue;
		if((ercd = sdcard_response(hsd->hspi, resp, 1, 300)) != E_OK)
			continue;
		if((ercd = sdcard_sendcommand(hsd->hspi, ACMD41, sizeof(ACMD41))) != E_OK)
			continue;
		if((ercd = sdcard_response(hsd->hspi, resp, 1, 300)) != E_OK)
			continue;
		if(resp[0] == 0x00)
			break;
		dly_tsk(40);
	}
	sdcard_unlock(hsd);
	return ercd;
}


/*
 *  SDCARDのSPI設定
 */
void
sdcard_setspi(ID portid, SPI_Handle_t *hspi)
{
	SdHandle.hspi = hspi;
}

/*
 *  SDCARDの初期化関数、エラーならばカードなし
 */
bool_t
sdcard_init(ID portid)
{
	SDCARD_Handler_t *hsd = &SdHandle;
	SPI_Handle_t *hspi;
	uint8_t resp[8];
	uint8_t checkno = 0;
	ER      ercd;
	int     i;

	if(!sdcard_sense)
		return false;

	sdcard_sense = false;
	if((hspi = hsd->hspi) == NULL)
		return false;
	hsd->hspi = NULL;

	if((ercd = sdcard_lock(hsd)) != E_OK)
		return false;
	if((ercd = sdcard_sendcommand(hspi, CMD0, sizeof(CMD0))) != E_OK){
		sdcard_unlock(hsd);
		return false;
	}
	if((ercd = sdcard_response(hspi, resp, 1, 500)) != E_OK){
		sdcard_unlock(hsd);
		return false;
	}
	sdcard_unlock(hsd);
	sdcard_dummywrite(hspi);

	if((ercd = sdcard_lock(hsd)) != E_OK)
		return false;
	if((ercd = sdcard_sendcommand(hspi, CMD8, sizeof(CMD8))) == E_OK){
		sdcard_response(hspi, resp, 1, 500);
	}
	if(ercd == E_OK && (resp[0] & 0x04) == 0x00){
		ercd = sdcard_read(hspi, resp, 4);
		if(resp[2] != 0x01 || resp[3] != 0xAA){
			syslog_4(LOG_ERROR, "CMD8(1) ERROR [%02x][%02x][%02x][%02x] !", resp[0], resp[1], resp[2], resp[3]);
			sdcard_unlock(hsd);
			return false;
		}
	}
	else{
		sdcard_unlock(hsd);
		sdcard_dummywrite(hspi);

		if((ercd = sdcard_lock(hsd)) != E_OK)
			return false;
		checkno = 1;
		if((ercd = sdcard_sendcommand(hspi, CMD58, sizeof(CMD58))) == E_OK){
			ercd = sdcard_response(hspi, resp, 1, 500);
		}
		if(ercd == E_OK && (resp[0] & 0x04) == 0x00){
			ercd = sdcard_read(hspi, resp, 4);
			if(resp[2] != 0x01){
				syslog_4(LOG_ERROR, "CMD58 ERROR [%02x][%02x][%02x][%02x] !", resp[0], resp[1], resp[2], resp[3]);
				sdcard_unlock(hsd);
				return false;
			}
		}
		else{
			syslog_2(LOG_ERROR, "CMD58 ERROR(%d) [%02x] !", ercd, resp[0]);
			sdcard_unlock(hsd);
			return false;
		}
	}
	sdcard_unlock(hsd);
	sdcard_dummywrite(hspi);

	for(i = 0 ; i < 50 ; i++){	/* 5sec */
		do{
			if((ercd = sdcard_lock(hsd)) != E_OK)
				return false;
			if((ercd = sdcard_sendcommand(hspi, CMD55, sizeof(CMD55))) == E_OK){
				if((ercd = sdcard_response(hspi, resp, 1, 500)) == E_OK){
					sdcard_unlock(hsd);
					sdcard_dummywrite(hspi);

					if((ercd = sdcard_lock(hsd)) != E_OK)
						return false;
					if((ercd = sdcard_sendcommand(hspi, ACMD41, sizeof(ACMD41))) == E_OK){
						if((ercd = sdcard_response(hspi, resp, 1, 500)) == E_OK){
							sdcard_unlock(hsd);
							sdcard_dummywrite(hspi);
							break;
						}
					}
				}
			}
			sdcard_unlock(hsd);
			sdcard_dummywrite(hspi);
			dly_tsk(200);
		}while(ercd != E_OK);
		if(resp[0] == 0x00)
			break;
		dly_tsk(100);
	}

	if(checkno == 1){
		sdcard_sense = true;
		hsd->cardtype = SD_CARD_V20;
		hsd->hspi = hspi;
		return true;
	}

	if((ercd = sdcard_lock(hsd)) != E_OK)
		return false;
	if((ercd = sdcard_sendcommand(hspi, CMD58, sizeof(CMD58))) == E_OK){
		ercd = sdcard_response(hspi, resp, 1, 500);
	}
	if(ercd == E_OK && resp[0] == 0x00){
		sdcard_sense = true;
		ercd = sdcard_read(hspi, resp, 4);
		if((resp[0] & 0x40) == 0)
			hsd->cardtype = SD_CARD_V20;
		else
			hsd->cardtype = SD_CARD_HC;
		hsd->hspi = hspi;
		sdcard_unlock(hsd);
		sdcard_dummywrite(hspi);
		return true;
	}
	sdcard_unlock(hsd);
	return false;
}

/*
 *  SDCARDオープン
 */
SDCARD_Handler_t*
sdcard_open(ID portid)
{
	if(SdHandle.hspi != NULL){
		SdHandle.spi_lock    = 0;
		SdHandle.RetryCount  = INIT_RETRY_COUNT;
		SdHandle.read_count  = 0;
		SdHandle.write_count = 0;
		SdHandle.read_retry_count  = 0;
		SdHandle.write_retry_count = 0;
		SdHandle.read_error  = 0;
		SdHandle.write_error = 0;
		return &SdHandle;
	}
	else
		return NULL;
}

/*
 *  SDCARDクローズ(未処理)
 */
ER
sdcard_close(SDCARD_Handler_t *hsd)
{
	return E_OK;
}

/*
 *  SDカードの情報を取り込む
 *  parameter1  hsd: SDCARDハンドラ
 *  parameter2  pCardInfo: カードインフォ構造体へのポインタ
 *  return      ERコード
 */
ER
sdcard_getcardinfo(SDCARD_Handler_t *hsd, SDCARD_CardInfo_t *pCardInfo)
{
	ER ercd = E_OK;
	uint32_t devicesize;
	uint8_t  blocklen, mul;

	pCardInfo->cardtype = (uint8_t)(hsd->cardtype);
	pCardInfo->RCA      = (uint16_t)(hsd->RCA);

	if(hsd->cardtype == SD_CARD_V11 || hsd->cardtype == SD_CARD_V20){
		/* ブロック長を取り込む */
		blocklen       = (uint8_t)(hsd->CSD[5] & 0x0F);
		devicesize = (hsd->CSD[6] & 0x03) << 10;
		devicesize |= hsd->CSD[7] << 2;
		devicesize |= (hsd->CSD[8] & 0xC0) >> 6;
		mul = (hsd->CSD[9] & 0x03) << 1;
		mul |= (hsd->CSD[10] & 0x80) >> 7;
		pCardInfo->capacity  = (devicesize + 1) ;
		pCardInfo->capacity *= (1 << (mul + 2));
		pCardInfo->blocksize = 1 << (blocklen);
		pCardInfo->capacity *= pCardInfo->blocksize;
	}
	else if(hsd->cardtype == SD_CARD_HC){
		devicesize = (hsd->CSD[7] & 0x3F) << 16;
		devicesize |= (hsd->CSD[8] & 0xFF) << 8;
		devicesize |= (hsd->CSD[9] & 0xFF);
		pCardInfo->capacity  = (uint64_t)(devicesize + 1) << 19 /* 512 * 1024*/;
		pCardInfo->blocksize = 512;
	}
	else{
		pCardInfo->capacity  = 0;
		pCardInfo->blocksize = 512;
		ercd = E_OBJ;
	}
	pCardInfo->maxsector = pCardInfo->capacity / 512;
	pCardInfo->status    = (uint8_t)hsd->CSD[14];
	return ercd;
}

/*
 *  CIDの取得
 *  SDカードCMD2,MMCカードCMD1送信後のレスポンス受信
 *  parameter1  hsd: SDCARDハンドラ
 *  return      ERコード
 */
ER
sdcard_checkCID(SDCARD_Handler_t *hsd)
{
	ER  ercd = E_OK;
	uint8_t resp[2];
	uint16_t crc;
	int i, timeout = 50;

	if((ercd = sdcard_lock(hsd)) != E_OK)
		return ercd;
	for(i = 0 ; i < INIT_RETRY_COUNT ; i++, timeout = 1000){
	    /*
		 *  CMD10送信
		 */
		if((ercd = sdcard_sendcommand(hsd->hspi, CMD10, sizeof(CMD10))) != E_OK)
			break;
		if((ercd = sdcard_response(hsd->hspi, resp, 1, timeout)) != E_OK)
			continue;
		else if(resp[0] == 0x00){
			sdcard_receive(hsd->hspi, &hsd->CID[0], 16, &crc, 5000);
			sdcard_unlock(hsd);
			return ercd;
		}
	}
	sdcard_unlock(hsd);
	if(i == INIT_RETRY_COUNT)
		ercd = E_TMOUT;
	return ercd;
}

/*
 *  CSD(Card Specific DATA)の取得
 *  parameter1  hsd: SDCARDハンドラ
 *  return      ERコード
 */
ER
sdcard_sendCSD(SDCARD_Handler_t *hsd)
{
	ER  ercd = E_OK;
	uint8_t resp[2];
	uint16_t crc;
	int i, timeout = 50;

	if((ercd = sdcard_lock(hsd)) != E_OK)
		return ercd;
	for(i = 0 ; i < INIT_RETRY_COUNT ; i++, timeout = 1000){
	    /*
		 *  CMD9送信
		 */
		if((ercd = sdcard_sendcommand(hsd->hspi, CMD9, sizeof(CMD9))) != E_OK)
			break;
		if((ercd = sdcard_response(hsd->hspi, resp, 1, timeout)) != E_OK)
			continue;
		else if(resp[0] == 0x00){
			sdcard_receive(hsd->hspi, &hsd->CSD[0], 16, &crc, 5000);
			sdcard_unlock(hsd);
			return E_OK;
		}
		dly_tsk(100);
	}
	if(i == INIT_RETRY_COUNT)
		ercd = E_TMOUT;
	sdcard_unlock(hsd);
	return ercd;
}

/*
 *  SELECT_CARDコマンドの送信(未処理)
 *  parameter1  hsd: SDMMCハンドラ
 *  return      ERコード
 */
ER
sdcard_select_card(SDCARD_Handler_t *hsd, uint64_t addr)
{
	return E_OK;
}

/*
 *  SDCARDコンフィギュレーション
 *  parameter1  hsd: SDMMCハンドラ
 *  return      ERコード
 */
ER
sdcard_configuration(SDCARD_Handler_t *hsd)
{
	ER  ercd = E_OK;
	uint8_t resp[2];
	int     i;

	if((ercd = sdcard_lock(hsd)) != E_OK)
		return ercd;
	for(i = 0 ; i < INIT_RETRY_COUNT ; i++){
		if((ercd = sdcard_sendcommand(hsd->hspi, CMD16, sizeof(CMD16))) != E_OK)
			break;
		ercd = sdcard_response(hsd->hspi, resp, 1, 2000);
		if(ercd == E_OK && resp[0] == 0x00)
			break;
		dly_tsk(100);
	}
	sdcard_unlock(hsd);
	if(i == INIT_RETRY_COUNT && resp[0] != 0x04)	/* SanDisk */
		ercd = E_TMOUT;
	return ercd;
}

/*
 *  SDCARDブロックREAD
 *  parameter1  hsd: SDCARDハンドラ
 *  parameter2  pbuf: 読み出しデータ格納領域のポインタ(uint32_t型)
 *  parameter3  ReadAddr: カード上の読み出し位置
 *  parameter4  blocksize: ブロックサイズ
 *  parameter5  num: 読み出しブロック数
 *  return      ERコード
 */
ER
sdcard_blockread(SDCARD_Handler_t *hsd, uint32_t *pbuf, uint64_t ReadAddr, uint32_t blocksize, uint32_t num)
{
	ER  ercd = E_OK;
	static uint8_t cmd17[8];
	uint8_t resp[2];
	uint16_t crc1, crc2;
	uint16_t crc3 = 0xffff;
	int     no, i = 0;

	hsd->read_count++;
	for(no = 0 ; no < num ; no++){
		for(i = 0 ; i < hsd->RetryCount ; i++){
			if((ercd = sdcard_lock(hsd)) != E_OK){
				return ercd;
			}

			cmd17[0] = 0x51;
			cmd17[1] = (ReadAddr >> 24) & 0xff;
			cmd17[2] = (ReadAddr >> 16) & 0xff;
			cmd17[3] = (ReadAddr >> 8) & 0xff;
			cmd17[4] = ReadAddr & 0xff;
		    /*
			 *  CMD17送信
			 */
			if((ercd = sdcard_sendcommand(hsd->hspi, cmd17, 5)) == E_OK){
				if((ercd = sdcard_response(hsd->hspi, resp, 1, 50)) == E_OK && resp[0] == 0x00){
					ercd = sdcard_receive(hsd->hspi, (uint8_t *)pbuf, blocksize, &crc1, 5000);
					if(ercd == E_OK){
						dly_tsk(5);
						crc2 = calc_CRC16((const uint8_t *)pbuf, blocksize);
						if(crc1 == crc2){
							sdcard_unlock(hsd);
							break;
						}
						else if(crc3 == crc2){
							hsd->read_error++;
							syslog_4(LOG_ERROR, "sdcard_blockread crc[%04x][%04x] count(%d) error(%d)  !", crc1, crc2, i, hsd->read_error);
							sdcard_unlock(hsd);
							break;
							dly_tsk(100);
						}
						crc3 = crc2;
					}
				}
			}
			hsd->read_retry_count++;
			sdcard_unlock(hsd);
			dly_tsk(10);
			sdcard_reset(hsd);
			dly_tsk(10);
			sdcard_configuration(hsd);
		}
		if(i == hsd->RetryCount && ercd == E_OK){
			ercd = E_TMOUT;
			break;
		}
		ReadAddr += blocksize;
		pbuf += (blocksize/4);
	}
	return ercd;
}

/*
 *  SDCARDブロックWRITE
 *  parameter1  hsd: SDCARDハンドラ
 *  parameter2  pbuf: 書込みデータ格納領域のポインタ(uint32_t型)
 *  parameter3  WritedAddr: カード上の書き込み位置
 *  parameter4  blocksize: ブロックサイズ
 *  parameter5  num: 書込みブロック数
 *  return      ERコード
 */
ER
sdcard_blockwrite(SDCARD_Handler_t *hsd, uint32_t *pbuf, uint64_t WriteAddr, uint32_t blocksize, uint32_t num)
{
	ER  ercd = E_OK;
	static uint8_t cmd24[8];
	uint8_t resp[2];
	int     no, i = 0;

	hsd->write_count++;
	for(no = 0 ; no < num ; no++){
		for(i = 0 ; i < hsd->RetryCount ; i++){
			if((ercd = sdcard_lock(hsd)) != E_OK){
				return ercd;
			}

			cmd24[0] = 0x58;
			cmd24[1] = (WriteAddr >> 24) & 0xff;
			cmd24[2] = (WriteAddr >> 16) & 0xff;
			cmd24[3] = (WriteAddr >> 8) & 0xff;
			cmd24[4] = WriteAddr & 0xff;
		    /*
			 *  CMD24送信
			 */
			if((ercd = sdcard_sendcommand(hsd->hspi, cmd24, 5)) == E_OK){
				if((ercd = sdcard_response(hsd->hspi, resp, 1, 50)) == E_OK && resp[0] == 0x00){
					ercd = sdcard_send(hsd->hspi, (uint8_t *)pbuf, blocksize);
					if(ercd == E_OK){
						sdcard_unlock(hsd);
						break;
					}
				}
			}
			hsd->read_retry_count++;
			sdcard_unlock(hsd);
			dly_tsk(10);
			sdcard_reset(hsd);
			dly_tsk(10);
			sdcard_configuration(hsd);
		}
		if(i == hsd->RetryCount && ercd == E_OK){
			ercd = E_TMOUT;
			break;
		}
		WriteAddr += blocksize;
		pbuf += (blocksize/4);
	}
	return ercd;
}

/*
 *  SDCARDブロック転送終了待ち
 *  parameter1  hsd: SDCARDハンドラ
 *  parameter2  Timeout: タイムアウト値(1msec)
 *  return      ERコード
 */
ER
sdcard_wait_transfar(SDCARD_Handler_t *hsd, uint32_t Timeout)
{
	return E_OK;
}

/*
 *  SDカードダミー処理
 *  parameter1  hsd: SDCARDハンドラ
 *  return      ERコード
 */
ER
sdcard_dummy(SDCARD_Handler_t *hsd)
{
	return E_OK;
}


