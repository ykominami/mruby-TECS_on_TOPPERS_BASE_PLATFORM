/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2008-2011 by Embedded and Real-Time Systems Laboratory
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
 *  @(#) $Id: dfsdm.c 698 2017-03-20 21:50:31Z roi $
 */
/*
 * 
 *  DFSDMドライバ関数群
 *
 */
#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include <target_syssvc.h>

#include "device.h"
#include "dfsdm.h"
#include "wm8994.h"

#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))
#define sil_andw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) & ~(b))
#define sil_modw_mem(a, b, c)	sil_wrw_mem((a), (sil_rew_mem(a) & (~b)) | (c))
#define get_channelno(b)        (((b)-TADR_DFSDM1_BASE) / 0x20)

/*
 *  SAIポート定義
 */
#define SAI1_PORTID           1
#define SAI2_PORTID           2

/*
 *  タイムアウト時間定義
 */ 
#define HAL_MAX_DELAY           0xFFFFFFFFU
#define PLLI2S_TIMEOUT_VALUE    100			/* Timeout value fixed to 100 ms  */
#define DFSDM_CKAB_TIMEOUT      5000

/*
 *  チャネル設定定義
 */
#define DFSDM_LSB_MASK          0x0000FFFF
#define DFSDM1_CHANNEL_NUMBER   8

/*
 *  チャネル情報
 */
static volatile uint32_t       Dfsdm1ChannelCounter = 0;
static DFSDM_Channel_Handle_t* Dfsdm1ChannelHandle[DFSDM1_CHANNEL_NUMBER] = {NULL};


/*
 *  DFSDMクロックコンフィグレーション
 *  parameter1  port:SAI PORTID
 *  parameter2  AudioFreq: Audio周波数
 *  parameter3  Params: 補助パラメータポインタ
 */
void
dfsdm_clockconfig(ID port, uint32_t AudioFreq)
{
	uint32_t PLLI2SN, PLLI2SQ, PLLI2SDivQ;
	uint32_t tick = 0;

	/*
	 *  AUDIO周波数からPLL設定値の取得
	 */
	if((AudioFreq == AUDIO_FREQUENCY_11K) || (AudioFreq == AUDIO_FREQUENCY_22K) || (AudioFreq == AUDIO_FREQUENCY_44K)){
		/* Configure PLLI2S prescalers */
		/* PLLI2S_VCO: VCO_429M
		I2S_CLK(first level) = PLLI2S_VCO/PLLI2SQ = 429/2 = 214.5 Mhz
		I2S_CLK_x = I2S_CLK(first level)/PLLI2SDIVQ = 214.5/19 = 11.289 Mhz */
		PLLI2SN = 429;
		PLLI2SQ = 2;
		PLLI2SDivQ = 19;
	}
	else /* AUDIO_FREQUENCY_8K, AUDIO_FREQUENCY_16K, AUDIO_FREQUENCY_48K, AUDIO_FREQUENCY_96K */{
		/* I2S clock config
		PLLI2S_VCO: VCO_344M
		I2S_CLK(first level) = PLLI2S_VCO/PLLI2SQ = 344/7 = 49.142 Mhz
		I2S_CLK_x = I2S_CLK(first level)/PLLI2SDIVQ = 49.142/1 = 49.142 Mhz */
		PLLI2SN = 344;
		PLLI2SQ = 7;
		PLLI2SDivQ = 1;
	}

	/*
	 *  SAI1/2クロック設定
	 */
	if(port == SAI1_PORTID){
		sil_modw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_DCKCFGR1), RCC_DCKCFGR1_SAI1SEL, RCC_DCKCFGR1_SAI1SEL_0);
	}
	else{
		sil_modw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_DCKCFGR1), RCC_DCKCFGR1_SAI2SEL, RCC_DCKCFGR1_SAI2SEL_0);
	}

	/*
	 *  PLLI2Sデゼーブル
	 */
	sil_andw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_CR), RCC_CR_PLLI2SON);

    /*
	 *  PLLI2S停止待ち
	 */
	tick = 0;
	while((sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_CR)) & RCC_CR_PLLI2SRDY) != 0){
		if(tick > PLLI2S_TIMEOUT_VALUE){
			return;		/* タイムアウトエラー */
		}
		dly_tsk(1);
		tick++;
	}

    /*
	 *  PLLI2SSN/PLLI2SN値設定
	 */
	sil_modw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_PLLI2SCFGR), (RCC_PLLI2SCFGR_PLLI2SN | RCC_PLLI2SCFGR_PLLI2SQ), ((PLLI2SN << 6) | (PLLI2SQ << 24)));

	/*
	 *  PLLI2SDIVQ設定
	 */
	sil_modw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_DCKCFGR1), RCC_DCKCFGR1_PLLI2SDIVQ, (PLLI2SDivQ-1));

	/*
	 *  PLLI2Sイネーブル
	 */
	sil_orw_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_CR), RCC_CR_PLLI2SON);

    /*
	 *  PLLI2S再開待ち
	 */
	tick = 0;
	while((sil_rew_mem((uint32_t *)(TADR_RCC_BASE+TOFF_RCC_CR)) & RCC_CR_PLLI2SRDY) == 0){
		if(tick > PLLI2S_TIMEOUT_VALUE){
			return;		/* タイムアウトエラー */
		}
		dly_tsk(1);
		tick++;
	}
}

/*
 *  DFSDMチャネル初期化
 *  parameter1 hdfsc  DFSDMチャネルハンドラへのポインタ
 *  return            正常終了時、E_OK
 */
ER dfsdm_channel_init(DFSDM_Channel_Handle_t *hdfsc)
{
	/*
	 *  パラメータチェック
	 */
	if(hdfsc == NULL)
		return E_PAR;
	if((hdfsc->Init.OutClockActivation & DFSDM_MASK) != 0)
		return E_PAR;
	if(hdfsc->Init.InputMultiplexer != DFSDM_CHANNEL_EXTERNAL_INPUTS &&
		hdfsc->Init.InputMultiplexer != DFSDM_CHANNEL_INTERNAL_REGISTER)
		return E_PAR;
	if(hdfsc->Init.InputDataPacking != DFSDM_CHANNEL_STANDARD_MODE &&
		hdfsc->Init.InputDataPacking != DFSDM_CHANNEL_INTERLEAVED_MODE &&
			hdfsc->Init.InputDataPacking != DFSDM_CHANNEL_DUAL_MODE)
		return E_PAR;
	if(hdfsc->Init.InputPins != DFSDM_CHANNEL_SAME_CHANNEL_PINS && 
		hdfsc->Init.InputPins != DFSDM_CHANNEL_FOLLOWING_CHANNEL_PINS)
		return E_PAR;
	if(hdfsc->Init.SerialType != DFSDM_CHANNEL_SPI_RISING &&
		hdfsc->Init.SerialType != DFSDM_CHANNEL_SPI_FALLING &&
			hdfsc->Init.SerialType != DFSDM_CHANNEL_MANCHESTER_RISING &&
				hdfsc->Init.SerialType != DFSDM_CHANNEL_MANCHESTER_FALLING)
		return E_PAR;
	if(hdfsc->Init.SerialSpiClock != DFSDM_CHANNEL_SPI_CLOCK_EXTERNAL &&
		hdfsc->Init.SerialSpiClock != DFSDM_CHANNEL_SPI_CLOCK_INTERNAL &&
			hdfsc->Init.SerialSpiClock != DFSDM_CHANNEL_SPI_CLOCK_INTERNAL_DIV2_FALLING &&
				hdfsc->Init.SerialSpiClock != DFSDM_CHANNEL_SPI_CLOCK_INTERNAL_DIV2_RISING)
		return E_PAR;
	if(hdfsc->Init.AwdFilterOrder != DFSDM_CHANNEL_FASTSINC_ORDER &&
		hdfsc->Init.AwdFilterOrder != DFSDM_CHANNEL_SINC1_ORDER &&
			hdfsc->Init.AwdFilterOrder != DFSDM_CHANNEL_SINC2_ORDER &&
				hdfsc->Init.AwdFilterOrder != DFSDM_CHANNEL_SINC3_ORDER)
		return E_PAR;
	if(hdfsc->Init.AwdOversampling < 1 || hdfsc->Init.AwdOversampling > 32)
		return E_PAR;
	if(hdfsc->Init.Offset < -8388608 || hdfsc->Init.Offset > 8388607)
		return E_PAR;
	if(hdfsc->Init.RightBitShift > 0x1F)
		return E_PAR;

	/*
	 *  チャネル設定テーブルのチェック
	 */
	if(Dfsdm1ChannelHandle[get_channelno(hdfsc->base)] != NULL)
		return E_OBJ;

	/*
	 *  グローバルDFSDM設定
	 */
	if(++Dfsdm1ChannelCounter == 1){
		/*
		 *  アウトプット・シリアルクロックソース設定
		 */
		sil_andw_mem((uint32_t *)(TADR_DFSDM1_CHANNEL0_BASE+TOFF_DFSDM_CHCFGR1), DFSDM_CHCFGR1_CKOUTSRC);
		sil_orw_mem((uint32_t *)(TADR_DFSDM1_CHANNEL0_BASE+TOFF_DFSDM_CHCFGR1), hdfsc->Init.OutClockSelection);

		/*
		 *  クロック分周設定
		 */
		sil_andw_mem((uint32_t *)(TADR_DFSDM1_CHANNEL0_BASE+TOFF_DFSDM_CHCFGR1), DFSDM_CHCFGR1_CKOUTDIV);
		if(hdfsc->Init.OutClockActivation == DFSDM_ENABLE)
			sil_orw_mem((uint32_t *)(TADR_DFSDM1_CHANNEL0_BASE+TOFF_DFSDM_CHCFGR1), ((hdfsc->Init.OutClockDivider - 1) << 16));

		/*
		 *  DFSDMグローバルインターフェイス設定
		 */
		sil_orw_mem((uint32_t *)(TADR_DFSDM1_CHANNEL0_BASE+TOFF_DFSDM_CHCFGR1), DFSDM_CHCFGR1_DFSDMEN);
	}

	/*
	 *  チャネル入力設定
	 */
	sil_andw_mem((uint32_t *)(hdfsc->base+TOFF_DFSDM_CHCFGR1), (DFSDM_CHCFGR1_DATPACK | DFSDM_CHCFGR1_DATMPX | DFSDM_CHCFGR1_CHINSEL));
	sil_orw_mem((uint32_t *)(hdfsc->base+TOFF_DFSDM_CHCFGR1), (hdfsc->Init.InputMultiplexer | hdfsc->Init.InputDataPacking | hdfsc->Init.InputPins));

	/*
	 *  シリアルインターフェース設定
	 */
	sil_andw_mem((uint32_t *)(hdfsc->base+TOFF_DFSDM_CHCFGR1), (DFSDM_CHCFGR1_SITP | DFSDM_CHCFGR1_SPICKSEL));
	sil_orw_mem((uint32_t *)(hdfsc->base+TOFF_DFSDM_CHCFGR1), (hdfsc->Init.SerialType | hdfsc->Init.SerialSpiClock));

	/*
	 *  アナログウォッチドッグ設定
	 */
	sil_andw_mem((uint32_t *)(hdfsc->base+TOFF_DFSDM_CHAWSCDR), (DFSDM_CHAWSCDR_AWFORD | DFSDM_CHAWSCDR_AWFOSR));
	sil_orw_mem((uint32_t *)(hdfsc->base+TOFF_DFSDM_CHAWSCDR), (hdfsc->Init.AwdFilterOrder | ((hdfsc->Init.AwdOversampling - 1) << 16)));

	/*
	 *  チャネルオフセット、ライトビットシフト設定
	 */
	sil_andw_mem((uint32_t *)(hdfsc->base+TOFF_DFSDM_CHCFGR2), (DFSDM_CHCFGR2_OFFSET | DFSDM_CHCFGR2_DTRBS));
	sil_orw_mem((uint32_t *)(hdfsc->base+TOFF_DFSDM_CHCFGR2), ((hdfsc->Init.Offset << 8) | (hdfsc->Init.RightBitShift << 3)));

	/*
	 *  DFSDMチャネル有効化
	 */
	sil_orw_mem((uint32_t *)(hdfsc->base+TOFF_DFSDM_CHCFGR1), DFSDM_CHCFGR1_CHEN);
	hdfsc->state = DFSDM_CHANNEL_STATE_READY;

	/*
	 *  DFSDMチャネルテーブル設定
	 */
	Dfsdm1ChannelHandle[get_channelno(hdfsc->base)] = hdfsc;
	return E_OK;
}

/*
 *  DFSDMチャネル無効化
 *  parameter1 hdfsc  DFSDMチャネルハンドラへのポインタ
 *  return            正常終了時、E_OK
 */
ER
dfsdm_channel_deinit(DFSDM_Channel_Handle_t *hdfsc)
{
	if(hdfsc == NULL)
		return E_PAR;

	/*
	 *  DFSDMチャネルテーブルチェック
	 */
	if(Dfsdm1ChannelHandle[get_channelno(hdfsc->base)] == NULL){
		return E_OBJ;
	}

	/*
	 *  DFSDMチャネル無効化
	 */
	sil_andw_mem((uint32_t *)(hdfsc->base+TOFF_DFSDM_CHCFGR1), DFSDM_CHCFGR1_CHEN);

	/*
	 *  グローバルDFSDM無効化
	 */
	if(--Dfsdm1ChannelCounter == 0){
		sil_andw_mem((uint32_t *)(TADR_DFSDM1_CHANNEL0_BASE+TOFF_DFSDM_CHCFGR1), DFSDM_CHCFGR1_DFSDMEN);
	}

	/*
	 *  DFSDMチャネルテーブルリセット
	 */
	hdfsc->state = DFSDM_CHANNEL_STATE_RESET;
	Dfsdm1ChannelHandle[get_channelno(hdfsc->base)] = (DFSDM_Channel_Handle_t *)NULL;
	return E_OK;
}


/*
 *  クロック・アブセンススタート
 *  parameter1 hdfsc  DFSDMチャネルハンドラへのポインタ
 *  return            正常終了時、E_OK
 */
ER
dfsdm_channelCkabStart(DFSDM_Channel_Handle_t *hdfsc)
{
	uint32_t channel;
	uint32_t tick = 0;

	if(hdfsc == NULL)
		return E_PAR;

	if(hdfsc->state != DFSDM_CHANNEL_STATE_READY)	/* 状態判定 */
		return E_OBJ;

    /*
	 *  チャネル番号取得
	 */
    channel = get_channelno(hdfsc->base);

    /*
	 *  クロックアブセンス・フラグクリア
	 */
    while((((sil_rew_mem((uint32_t *)(TADR_DFSDM1_FILTER0_BASE+TOFF_DFSDM_FLTISR)) & DFSDM_FLTISR_CKABF) >> (16 + channel)) & 1) != 0){
		sil_wrw_mem((uint32_t *)(TADR_DFSDM1_FILTER0_BASE+TOFF_DFSDM_FLTICR), (1 << (16 + channel)));
		if(tick++ > DFSDM_CKAB_TIMEOUT)
			return E_TMOUT;	/* タイムアウト */
		dly_tsk(1);
    }

	/*
	 *  クロックアブセンス割込み設定
	 */
	sil_orw_mem((uint32_t *)(TADR_DFSDM1_FILTER0_BASE+TOFF_DFSDM_FLTCR2), DFSDM_FLTCR2_CKABIE);

	/*
	 *  クロックアブセンス・スタート
	 */
	sil_orw_mem((uint32_t *)(hdfsc->base+TOFF_DFSDM_CHCFGR1), DFSDM_CHCFGR1_CKABEN);
	return E_OK;
}

/*
 *  クロック・アブセンスストップ
 *  parameter1 hdfsc  DFSDMチャネルハンドラへのポインタ
 *  return            正常終了時、E_OK
 */
ER
dfsdm_channelCkabStop(DFSDM_Channel_Handle_t *hdfsc)
{
	uint32_t channel;

	if(hdfsc == NULL)
		return E_PAR;

	if(hdfsc->state != DFSDM_CHANNEL_STATE_READY)	/* 状態判定 */
		return E_OBJ;

	/*
	 *  クロックアブセンス・停止
	 */
	sil_andw_mem((uint32_t *)(hdfsc->base+TOFF_DFSDM_CHCFGR1), DFSDM_CHCFGR1_CKABEN);

	/*
	 *  クロックアブセンス・フラグクリア
	 */
    channel = get_channelno(hdfsc->base);
	sil_wrw_mem((uint32_t *)(TADR_DFSDM1_FILTER0_BASE+TOFF_DFSDM_FLTICR), (1 << (16 + channel)));

	/*
	 *  クロックアブセンス割込み停止
	 */
	sil_andw_mem((uint32_t *)(TADR_DFSDM1_FILTER0_BASE+TOFF_DFSDM_FLTCR2), DFSDM_FLTCR2_CKABIE);
	return E_OK;
}

/*
 *  ショート・サーキットスタート
 *  parameter1 hdfsc       DFSDMチャネルハンドラへのポインタ
 *  parameter2 Threshold   ショート・サーキット・スレッシュホールド(0-255)
 *  parameter3 BreakSignal ブレークシグナル
 *  return            正常終了時、E_OK
 */
ER
dfsdm_channelScdStart(DFSDM_Channel_Handle_t *hdfsc, uint32_t Threshold, uint32_t BreakSignal)
{
	if(hdfsc == NULL)
		return E_PAR;
	if(Threshold > 255)
		return E_PAR;
	if(BreakSignal > 15)
		return E_PAR;

	if(hdfsc->state != DFSDM_CHANNEL_STATE_READY)	/* 状態判定 */
		return E_OBJ;

	/*
	 *  ショート・サーキット割込み設定
	 */
	sil_orw_mem((uint32_t *)(TADR_DFSDM1_FILTER0_BASE+TOFF_DFSDM_FLTCR2), DFSDM_FLTCR2_SCDIE);

	/*
	 *  スレッシュホールド、ブレークシグナル設定
	 */
	sil_andw_mem((uint32_t *)(hdfsc->base+TOFF_DFSDM_CHAWSCDR), (DFSDM_CHAWSCDR_BKSCD | DFSDM_CHAWSCDR_SCDT));
	sil_orw_mem((uint32_t *)(hdfsc->base+TOFF_DFSDM_CHAWSCDR), ((BreakSignal << 12) | Threshold));

	/*
	 *  ショート・サーキットスタート
	 */
	sil_orw_mem((uint32_t *)(hdfsc->base+TOFF_DFSDM_CHCFGR1), DFSDM_CHCFGR1_SCDEN);
	return E_OK;
}

/*
 *  ショート・サーキットストップ
 *  parameter1 hdfsc  DFSDMチャネルハンドラへのポインタ
 *  return            正常終了時、E_OK
 */
ER
dfsdm_channelScdStop(DFSDM_Channel_Handle_t *hdfsc)
{
	uint32_t channel;

	if(hdfsc == NULL)
		return E_PAR;

	if(hdfsc->state != DFSDM_CHANNEL_STATE_READY)	/* 状態判定 */
		return E_OBJ;

	/*
	 *  ショート・サーキット停止
	 */
	sil_andw_mem((uint32_t *)(hdfsc->base+TOFF_DFSDM_CHCFGR1), DFSDM_CHCFGR1_SCDEN);

    /*
	 *  ショート・サーキットフラグクリア
	 */
	channel = get_channelno(hdfsc->base);
	sil_wrw_mem((uint32_t *)(TADR_DFSDM1_FILTER0_BASE+TOFF_DFSDM_FLTICR), (1 << (24 + channel)));

	/*
	 *  ショート・サーキット割込み停止
	 */
	sil_andw_mem((uint32_t *)(TADR_DFSDM1_FILTER0_BASE+TOFF_DFSDM_FLTCR2), DFSDM_FLTCR2_SCDIE);
	return E_OK;
}

/*
 *  アナログ・ウォッチドッグ値取得
 *  parameter1 hdfsc  DFSDMチャネルハンドラへのポインタ
 *  return            アナログ・ウォッチドッグ値
 */
int16_t
dfsdm_channelGetAwdValue(DFSDM_Channel_Handle_t *hdfsc)
{
	return (int16_t)sil_rew_mem((uint32_t *)(hdfsc->base+TOFF_DFSDM_CHWDATAR));
}

/*
 *  チャネルオフセット値変更
 *  parameter1 hdfsc  DFSDMチャネルハンドラへのポインタ
 *  parameter2 Offset チャネルオフセット値(-8388608 - 8388607)
 *  return            正常終了時、E_OK
 */
ER
dfsdm_channelModifyOffset(DFSDM_Channel_Handle_t *hdfsc, int32_t Offset)
{
	if(hdfsc == NULL)
		return E_PAR;
	if(Offset < -8388608 || Offset > 8388607)
		return E_PAR;

	if(hdfsc->state != DFSDM_CHANNEL_STATE_READY)
		return E_OBJ;

	/*
	 *  チャネルオフセット値設定
	 */
	sil_andw_mem((uint32_t *)(hdfsc->base+TOFF_DFSDM_CHCFGR2), DFSDM_CHCFGR2_OFFSET);
	sil_orw_mem((uint32_t *)(hdfsc->base+TOFF_DFSDM_CHCFGR2), ((uint32_t) Offset << 8));
	return E_OK;
}


/*
 *  インジェクトチャネル数取得
 */
static uint32_t
DFSDM_GetInjChannelsNbr(uint32_t Channels)
{
	uint32_t nbChannels = 0;
	uint32_t tmp = (Channels & DFSDM_LSB_MASK);

	/*
	 *  チャネルビットフィールドから数を取得
	 */
	while(tmp != 0){
		if((tmp & 1) != 0)
			nbChannels++;
		tmp >>= 1;
	}
	return nbChannels;
}

/*
 *  レギュラー変換設定
 */
static void
DFSDM_RegConvStart(DFSDM_Filter_Handle_t* hdfsf)
{
	/*
	 *  レギュラートリガなら即スタート
	 */
	if(hdfsf->Init.RegTrigger == DFSDM_FILTER_SW_TRIGGER)
		sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_RSWSTART);
	else{	/* シンクロナストリガの場合 */
		/*
		 *  DFSDMフィルターディゼーブル
		 */
		sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_DFEN);

		/*
		 * RSYNCビットセット
		 */
		sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_RSYNC);

		/*
		 *  DFSDMフィルターイネーブル
		 */
		sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_DFEN);

		/*
		 *  インジェクト変換中なら再スタート
		 */
		if(hdfsf->state == DFSDM_FILTER_STATE_INJ){
			if(hdfsf->Init.InjTrigger == DFSDM_FILTER_SW_TRIGGER)
				sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_JSWSTART);
			hdfsf->InjConvRemaining = (hdfsf->Init.InjScanMode == DFSDM_ENABLE) ? hdfsf->InjectedChannelsNbr : 1;
		}
	}
	/*
	 *  DFSDMフィルター状態更新
	 */
	hdfsf->state = (hdfsf->state == DFSDM_FILTER_STATE_READY) ? DFSDM_FILTER_STATE_REG : DFSDM_FILTER_STATE_REG_INJ;
}

/*
 *  レギュラー変換停止
 */
static void
DFSDM_RegConvStop(DFSDM_Filter_Handle_t* hdfsf)
{
	/*
	 *  DFSDMフィルターディゼーブル
	 */
	sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_DFEN);

	/*
	 *  シンクロナストリガなら RSYNCビットリセット
	 */
	if(hdfsf->Init.RegTrigger == DFSDM_FILTER_SYNC_TRIGGER)
		sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_RSYNC);

	/*
	 *  DFSDMフィルターイネーブル
	 */
	sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_DFEN);
  
	/*
	 *  インジェクト変換中ならリスタート
	 */
	if(hdfsf->state == DFSDM_FILTER_STATE_REG_INJ){
		if(hdfsf->Init.InjTrigger == DFSDM_FILTER_SW_TRIGGER)
			sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_JSWSTART);
		hdfsf->InjConvRemaining = (hdfsf->Init.InjScanMode == DFSDM_ENABLE) ? hdfsf->InjectedChannelsNbr : 1;
	}

	/*
	 *  DFSDMフィルター状態更新
	 */
	hdfsf->state = (hdfsf->state == DFSDM_FILTER_STATE_REG) ? DFSDM_FILTER_STATE_READY : DFSDM_FILTER_STATE_INJ;
}

/*
 *  インジェクト変換設定
 */
static void
DFSDM_InjConvStart(DFSDM_Filter_Handle_t* hdfsf)
{
	/*
	 *  インジェクトトリガなら即スタート
	 */
	if(hdfsf->Init.InjTrigger == DFSDM_FILTER_SW_TRIGGER)
		sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_JSWSTART);
	else{	/* 外部またはシンクロナストリガの場合 */
		/*
		 *  DFSDMフィルターディゼーブル
		 */
		sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_DFEN);

		/*
		 *  シンクロナストリガならJSYNCビットセット
		 *  外部トリガならJEXTENビットセット
		 */
		if(hdfsf->Init.InjTrigger == DFSDM_FILTER_SYNC_TRIGGER)
			sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_JSYNC);
		else
			sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), hdfsf->Init.InjExtTriggerEdge);

		/*
		 *  DFSDMフィルターイネーブル
		 */
		sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_DFEN);

		/* If regular conversion was in progress, restart it */
		if(hdfsf->state == DFSDM_FILTER_STATE_REG && hdfsf->Init.RegTrigger == DFSDM_FILTER_SW_TRIGGER)
			sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_RSWSTART);
	}

	/*
	 *  DFSDMフィルター状態更新
	 */
	hdfsf->state = (hdfsf->state == DFSDM_FILTER_STATE_READY) ? DFSDM_FILTER_STATE_INJ : DFSDM_FILTER_STATE_REG_INJ;
}

/*
 *  インジェクト変換停止
 */
static void
DFSDM_InjConvStop(DFSDM_Filter_Handle_t* hdfsf)
{
	/*
	 *  DFSDMフィルターディゼーブル
	 */
	sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_DFEN);

	/*
	 *  シンクロナストリガならJSYNCビットリセット
	 *  外部トリガならJEXTENビットリセット
	 */
	if(hdfsf->Init.InjTrigger == DFSDM_FILTER_SYNC_TRIGGER)
		sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_JSYNC);
	else if(hdfsf->Init.InjTrigger == DFSDM_FILTER_EXT_TRIGGER)
		sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_JEXTEN);

	/*
	 *  DFSDMフィルターイネーブル
	 */
	sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_DFEN);

	/*
	 *  レギュラー変換中なら、リスタート
	 */
	if(hdfsf->state == DFSDM_FILTER_STATE_REG_INJ && hdfsf->Init.RegTrigger == DFSDM_FILTER_SW_TRIGGER)
		sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_RSWSTART);

	/*
	 *  インジェクト状態更新
	 */
	hdfsf->InjConvRemaining = (hdfsf->Init.InjScanMode == DFSDM_ENABLE) ? hdfsf->InjectedChannelsNbr : 1;

	/*
	 *  DFSDMフィルター状態更新
	 */
	hdfsf->state = (hdfsf->state == DFSDM_FILTER_STATE_INJ) ? DFSDM_FILTER_STATE_READY : DFSDM_FILTER_STATE_REG;
}


/*
 *  DFSDMフィルター初期化
 *  parameter1 hdfsc  DFSDMフィルターハンドラへのポインタ
 *  return            正常終了時、E_OK
 */
ER
dfsdm_filter_init(DFSDM_Filter_Handle_t *hdfsf)
{
	if(hdfsf == NULL)
		return E_PAR;
	if(hdfsf->Init.RegTrigger != DFSDM_FILTER_SW_TRIGGER && hdfsf->Init.RegTrigger != DFSDM_FILTER_SYNC_TRIGGER)
		return E_PAR;
	if((hdfsf->Init.RegFastMode & DFSDM_MASK) != 0)
		return E_PAR;
	if((hdfsf->Init.RegDmaMode & DFSDM_MASK) != 0)
		return E_PAR;
	if((hdfsf->Init.InjScanMode & DFSDM_MASK) != 0)
		return E_PAR;
	if((hdfsf->Init.InjDmaMode & DFSDM_MASK) != 0)
		return E_PAR;
	if(hdfsf->Init.FilterSincOrder != DFSDM_FILTER_FASTSINC_ORDER &&
		hdfsf->Init.FilterSincOrder != DFSDM_FILTER_SINC1_ORDER &&
			hdfsf->Init.FilterSincOrder != DFSDM_FILTER_SINC2_ORDER &&
				hdfsf->Init.FilterSincOrder != DFSDM_FILTER_SINC3_ORDER &&
					hdfsf->Init.FilterSincOrder != DFSDM_FILTER_SINC4_ORDER &&
						hdfsf->Init.FilterSincOrder != DFSDM_FILTER_SINC5_ORDER)
		return E_PAR;
	if(hdfsf->Init.FilterOversampling < 1 || hdfsf->Init.FilterOversampling > 1024)
		return E_PAR;
	if(hdfsf->Init.FilterIntOversampling < 1 || hdfsf->Init.FilterIntOversampling > 256)
		return E_PAR;

	/*
	 *  Check parameters compatibility */
	if((hdfsf->base == TADR_DFSDM1_FILTER0_BASE) && 
		((hdfsf->Init.RegTrigger  == DFSDM_FILTER_SYNC_TRIGGER) || 
			(hdfsf->Init.InjTrigger == DFSDM_FILTER_SYNC_TRIGGER)))
		return E_PAR;

	/*
	 *  DFSDMフィルターデフォルト設定
	 */
	hdfsf->RegularContMode     = DFSDM_CONTINUOUS_CONV_OFF;
	hdfsf->InjectedChannelsNbr = 1;
	hdfsf->InjConvRemaining    = 1;
	hdfsf->errorcode           = DFSDM_FILTER_ERROR_NONE;

	/*
	 *  レギュラー変換設定
	 */
	sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_RSYNC);
	if(hdfsf->Init.RegFastMode == DFSDM_ENABLE)
		sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_FAST);
	else
		sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_FAST);

	if(hdfsf->Init.RegDmaMode == DFSDM_ENABLE)
		sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_RDMAEN);
	else
		sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_RDMAEN);

	/*
	 *  インジェクト変換設定
	 */
	sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), (DFSDM_FLTCR1_JSYNC | DFSDM_FLTCR1_JEXTEN | DFSDM_FLTCR1_JEXTSEL));
	if(hdfsf->Init.InjTrigger == DFSDM_FILTER_EXT_TRIGGER){
		if((hdfsf->Init.InjExtTrigger & ~DFSDM_FLTCR1_JEXTSEL) != 0)
			return E_PAR;
		sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), hdfsf->Init.InjExtTrigger);
	}

	if(hdfsf->Init.InjScanMode == DFSDM_ENABLE)
		sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_JSCAN);
	else
		sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_JSCAN);

	if(hdfsf->Init.InjDmaMode == DFSDM_ENABLE)
		sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_JDMAEN);
	else
		sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_JDMAEN);

	/*
	 *  フィルター用パラメータ設定
	 */
	sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTFCR), (DFSDM_FLTFCR_FORD | DFSDM_FLTFCR_FOSR | DFSDM_FLTFCR_IOSR));
	sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTFCR), (hdfsf->Init.FilterSincOrder |
                                    ((hdfsf->Init.FilterOversampling - 1) << 16) |
                                  (hdfsf->Init.FilterIntOversampling - 1)));

	/*
	 *  DFSDMフィルター有効化
	 */
	sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_DFEN);
	hdfsf->state = DFSDM_FILTER_STATE_READY;
	return E_OK;
}

/*
 *  DFSDMフィルター無効化
 *  parameter1 hdfsc  DFSDMフィルターハンドラへのポインタ
 *  return            正常終了時、E_OK
 */
ER
dfsdm_filter_deinit(DFSDM_Filter_Handle_t *hdfsf)
{
	if(hdfsf == NULL)
		return E_PAR;

	/*
	 *  DFSDMフィルター有効化
	 */
	sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_DFEN);
	hdfsf->state = DFSDM_FILTER_STATE_RESET;
	return E_OK;
}

/*
 *  DFSDMフィルターレギュラー変換設定
 *  parameter1 hdfsc    DFSDMフィルターハンドラへのポインタ
 *  parameter2 Channel  チャネル番号
 *  parameter3 ContMode コンティニュー有効無効設定
 *  return              正常終了時、E_OK
 */
ER
dfsdm_filter_config_reg(DFSDM_Filter_Handle_t *hdfsf, uint32_t Channel, uint32_t ContinuousMode)
{
	if(hdfsf == NULL)
		return E_PAR;
	if(Channel < DFSDM_CHANNEL_0 || Channel > DFSDM_CHANNEL_7)
		return E_PAR;
	if(ContinuousMode != DFSDM_CONTINUOUS_CONV_OFF && ContinuousMode != DFSDM_CONTINUOUS_CONV_ON)
		return E_PAR;

	/*
	 *  状態判定
	 */
	if(hdfsf->state != DFSDM_FILTER_STATE_RESET && hdfsf->state != DFSDM_FILTER_STATE_ERROR){
		/*
		 *  レギュラー変換用、チャネルと継続モード設定
		 */
		sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), (DFSDM_FLTCR1_RCH | DFSDM_FLTCR1_RCONT));
		if(ContinuousMode == DFSDM_CONTINUOUS_CONV_ON)
			sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), (((Channel << 8) & DFSDM_FLTCR1_RCH) |
                                                     DFSDM_FLTCR1_RCONT));
		else
			sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), ((Channel << 8) & DFSDM_FLTCR1_RCH));
		hdfsf->RegularContMode = ContinuousMode;
		return E_OK;
	}
	else
		return E_OBJ;
}

/*
 *  DFSDMフィルターインジェクト変換設定
 *  parameter1 hdfsc    DFSDMフィルターハンドラへのポインタ
 *  parameter2 Channel  チャネル番号
 *  return              正常終了時、E_OK
 */
ER dfsdm_filter_config_inj(DFSDM_Filter_Handle_t *hdfsf, uint32_t Channel)
{
	if(hdfsf == NULL)
		return E_PAR;

	if(Channel == 0 || Channel > 0x000F00FF)
		return E_PAR;

	/*
	 *  状態判定
	 */
	if(hdfsf->state != DFSDM_FILTER_STATE_RESET && hdfsf->state != DFSDM_FILTER_STATE_ERROR){
		/*
		 *  インジェクト変換用、チャネル設定
		 */
		sil_wrw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTJCHGR), (Channel & DFSDM_LSB_MASK));
		hdfsf->InjectedChannelsNbr = DFSDM_GetInjChannelsNbr(Channel);
		hdfsf->InjConvRemaining = (hdfsf->Init.InjScanMode == DFSDM_ENABLE) ? hdfsf->InjectedChannelsNbr : 1;
		return E_OK;
	}
	else
		return E_OBJ;
}

/*
 *  DFSDMフィルターレギュラー変換DMAハーフ終了コールバック
 */
static void
DFSDM_DMARegularHalfConvCplt(DMA_Handle_t *hdma)
{
	DFSDM_Filter_Handle_t* hdfsf = (DFSDM_Filter_Handle_t*) ((DMA_Handle_t*)hdma)->localdata;

	/*
	 *  コールバック関数の呼び出し
	 */
	if(hdfsf->regconvhalfcallback != NULL)
		hdfsf->regconvhalfcallback(hdfsf);
}

/*
 *  DFSDMフィルターレギュラー変換DMA終了コールバック
 */
static void
DFSDM_DMARegularConvCplt(DMA_Handle_t *hdma)
{
	DFSDM_Filter_Handle_t* hdfsf = (DFSDM_Filter_Handle_t*) ((DMA_Handle_t*)hdma)->localdata;

	/*
	 *  コールバック関数の呼び出し
	 */
	if(hdfsf->regconvcallback != NULL)
		hdfsf->regconvcallback(hdfsf);
}

/*
 *  DFSDMフィルターインジェクト変換DMAハーフ終了コールバック
 */
static void
DFSDM_DMAInjectedHalfConvCplt(DMA_Handle_t *hdma)
{
	DFSDM_Filter_Handle_t* hdfsf = (DFSDM_Filter_Handle_t*) ((DMA_Handle_t*)hdma)->localdata;

	/*
	 *  コールバック関数の呼び出し
	 */
	if(hdfsf->injconvhalfcallback != NULL)
		hdfsf->injconvhalfcallback(hdfsf);
}

/*
 *  DFSDMフィルターインジェクト変換DMA終了コールバック
 */
static void
DFSDM_DMAInjectedConvCplt(DMA_Handle_t *hdma)
{
	DFSDM_Filter_Handle_t* hdfsf = (DFSDM_Filter_Handle_t*) ((DMA_Handle_t*)hdma)->localdata;

	/*
	 *  コールバック関数の呼び出し
	 */
	if(hdfsf->injconvcallback != NULL)
		hdfsf->injconvcallback(hdfsf);
}

/*
 *  DFSDMフィルターDMAエラーコールバック
 */
static void
DFSDM_DMAError(DMA_Handle_t *hdma)
{
	DFSDM_Filter_Handle_t* hdfsf = (DFSDM_Filter_Handle_t*) ((DMA_Handle_t*)hdma)->localdata;

	/*
	 *  エラーコード設定
	 */
	hdfsf->errorcode = DFSDM_FILTER_ERROR_DMA;

	/*
	 *  DMAエラーコールバック
	 */
	if(hdfsf->errorcallback != NULL)
		hdfsf->errorcallback(hdfsf);
}


/*
 *  DFSDMフィルターレギュラー変換スタート(32BIT変換)
 *  parameter1 hdfsc    DFSDMフィルターハンドラへのポインタ
 *  paramter 2 pData    変換データ設定領域のポインタ
 *  parameter3 Length   領域長
 *  return              正常終了時、E_OK
  */
ER
dfsdm_filterRegularStart(DFSDM_Filter_Handle_t *hdfsf, int32_t *pData, uint32_t Length)
{
	if(hdfsf == NULL || pData == NULL || Length == 0)
		return E_PAR;

	/*
	 *  初期設定値の確認
	 */
	if((hdfsf->Init.RegTrigger == DFSDM_FILTER_SW_TRIGGER) && \
          (hdfsf->RegularContMode == DFSDM_CONTINUOUS_CONV_OFF) && \
          (hdfsf->hdmaReg->Init.Mode == DMA_NORMAL) && Length != 1)
		return E_PAR;

	if((hdfsf->Init.RegTrigger == DFSDM_FILTER_SW_TRIGGER) && \
          (hdfsf->RegularContMode == DFSDM_CONTINUOUS_CONV_OFF) && \
          (hdfsf->hdmaReg->Init.Mode == DMA_CIRCULAR))
		return E_OBJ;

	/*
	 *  DMAの有効確認
	 */
	if((sil_rew_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1)) & DFSDM_FLTCR1_RDMAEN) == 0)
		return E_OBJ;

	/*
	 *  DFSDMフィルターの状態チェック
	 */
	if(hdfsf->state == DFSDM_FILTER_STATE_READY || hdfsf->state == DFSDM_FILTER_STATE_INJ){
		ER ercd = E_OK;
		/*
		 *  コールバック関数の設定
		 */
		hdfsf->hdmaReg->xfercallback = DFSDM_DMARegularConvCplt;
		hdfsf->hdmaReg->errorcallback = DFSDM_DMAError;
		hdfsf->hdmaReg->xferhalfcallback = (hdfsf->hdmaReg->Init.Mode == DMA_CIRCULAR) ? DFSDM_DMARegularHalfConvCplt : NULL;

		/*
		 *  DMAスタート
		 */
		if((ercd = dma_start(hdfsf->hdmaReg, (uint32_t)(hdfsf->base+TOFF_DFSDM_FLTRDATAR), (uint32_t)pData, Length)) != E_OK)
			hdfsf->state = DFSDM_FILTER_STATE_ERROR;	/* DMAエラー */
		else	/* レギュラー変換スタート */
			DFSDM_RegConvStart(hdfsf);
		return ercd;
	}
	else
		return E_OBJ;
}

/*
 *  DFSDMフィルターレギュラー変換スタート(16BIT変換)
 *  parameter1 hdfsc    DFSDMフィルターハンドラへのポインタ
 *  paramter 2 pData    変換データ設定領域のポインタ
 *  parameter3 Length   領域長
 *  return              正常終了時、E_OK
 */
ER
dfsdm_filterRegularMsbStart(DFSDM_Filter_Handle_t *hdfsf, int16_t *pData, uint32_t Length)
{
	if(hdfsf == NULL || pData == NULL || Length == 0)
		return E_PAR;

	/*
	 *  初期設定値の確認
	 */
	if((hdfsf->Init.RegTrigger == DFSDM_FILTER_SW_TRIGGER) && \
          (hdfsf->RegularContMode == DFSDM_CONTINUOUS_CONV_OFF) && \
          (hdfsf->hdmaReg->Init.Mode == DMA_NORMAL) && (Length != 1))
		return E_PAR;

	if((hdfsf->Init.RegTrigger == DFSDM_FILTER_SW_TRIGGER) && \
          (hdfsf->RegularContMode == DFSDM_CONTINUOUS_CONV_OFF) && \
          (hdfsf->hdmaReg->Init.Mode == DMA_CIRCULAR))
		return E_OBJ;

	/*
	 *  DMAの有効確認
	 */
	if((sil_rew_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1)) & DFSDM_FLTCR1_RDMAEN) == 0)
		return E_OBJ;

	/*
	 *  DFSDMフィルターの状態チェック
	 */
	if(hdfsf->state == DFSDM_FILTER_STATE_READY || hdfsf->state == DFSDM_FILTER_STATE_INJ){
		ER ercd = E_OK;
		/*
		 *  コールバック関数の設定
		 */
		hdfsf->hdmaReg->xfercallback = DFSDM_DMARegularConvCplt;
		hdfsf->hdmaReg->errorcallback = DFSDM_DMAError;
		hdfsf->hdmaReg->xferhalfcallback = (hdfsf->hdmaReg->Init.Mode == DMA_CIRCULAR) ? DFSDM_DMARegularHalfConvCplt : NULL;

		/*
		 *  DMAスタート
		 */
		if((ercd = dma_start(hdfsf->hdmaReg, (uint32_t)(hdfsf->base+TOFF_DFSDM_FLTRDATAR+2), (uint32_t)pData, Length)) != E_OK)
			hdfsf->state = DFSDM_FILTER_STATE_ERROR;	/* DMAスタートエラー */
		else	/* レギュラー変換スタート */
			DFSDM_RegConvStart(hdfsf);
		return ercd;
	}
	else
		return E_OBJ;
}

/*
 *  DFSDMフィルターレギュラー変換停止
 *  parameter1 hdfsc    DFSDMフィルターハンドラへのポインタ
 *  return              正常終了時、E_OK
 */
ER
dfsdm_filterRegularStop(DFSDM_Filter_Handle_t *hdfsf)
{
	ER ercd = E_OK;

	if(hdfsf == NULL)
		return E_PAR;

	/*
	 *  DFSDMフィルターの状態チェック
	 */
	if(hdfsf->state != DFSDM_FILTER_STATE_REG && hdfsf->state != DFSDM_FILTER_STATE_REG_INJ)
		return E_OBJ;

	 /*
	  *  DMA停止
	  */
	if((ercd = dma_end(hdfsf->hdmaReg)) != E_OK)	/* Set DFSDM filter in error state */
		hdfsf->state = DFSDM_FILTER_STATE_ERROR;
	else	/* レギュラー変換停止 */
		DFSDM_RegConvStop(hdfsf);
	return ercd;
}

/*
 *  DFSDMフィルターレギュラー変換値取得
 *  parameter1 hdfsc    DFSDMフィルターハンドラへのポインタ
 *  parameter2 Channel  DFSDMチャネル番号設定用ポインタ
 *  return              レギュラー変換値
 */
int32_t
dfsdm_filterGetRegularValue(DFSDM_Filter_Handle_t *hdfsf, uint32_t *Channel)
{
	uint32_t reg = 0;

	if(hdfsf == NULL || Channel == NULL)
		return 0;

	/*
	 *  レギュラー変換値とチャネル番号取出し
	 */
	reg = sil_rew_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTRDATAR));

	/* Extract channel and regular conversion value */
	*Channel = (reg & DFSDM_FLTRDATAR_RDATACH);
	return (int32_t)((reg & DFSDM_FLTRDATAR_RDATA) >> 18);
}

/*
 *  DFSDMフィルターインジェクト変換スタート(32BIT変換)
 *  parameter1 hdfsc    DFSDMフィルターハンドラへのポインタ
 *  paramter 2 pData    変換データ設定領域のポインタ
 *  parameter3 Length   領域長
 *  return              正常終了時、E_OK
 */
ER
dfsdm_filterInjectedStart(DFSDM_Filter_Handle_t *hdfsf, int32_t *pData, uint32_t Length)
{
	if(hdfsf == NULL || pData == NULL || Length == 0)
		return E_PAR;

	/*
	 *  初期設定値の確認
	 */
	if((hdfsf->Init.InjTrigger == DFSDM_FILTER_SW_TRIGGER) && \
          (hdfsf->hdmaInj->Init.Mode == DMA_NORMAL) && Length > hdfsf->InjConvRemaining)
		return E_PAR;

	if((hdfsf->Init.InjTrigger == DFSDM_FILTER_SW_TRIGGER) && hdfsf->hdmaInj->Init.Mode == DMA_CIRCULAR)
		return E_OBJ;

	/*
	 *  DMAの有効確認
	 */
	if((sil_rew_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1)) & DFSDM_FLTCR1_JDMAEN) == 0)
		return  E_OBJ;

	/*
	 *  DFSDMフィルターの状態チェック
	 */
	if(hdfsf->state == DFSDM_FILTER_STATE_READY || hdfsf->state == DFSDM_FILTER_STATE_REG){
		ER ercd = E_OK;
		/*
		 *  コールバック関数の設定
		 */
		hdfsf->hdmaInj->xfercallback = DFSDM_DMAInjectedConvCplt;
		hdfsf->hdmaInj->errorcallback = DFSDM_DMAError;
		hdfsf->hdmaInj->xferhalfcallback = (hdfsf->hdmaInj->Init.Mode == DMA_CIRCULAR) ?\
                                                   DFSDM_DMAInjectedHalfConvCplt : NULL;

		/*
		 *  DMAスタート
		 */
		if((ercd = dma_start(hdfsf->hdmaInj, (uint32_t)(hdfsf->base+TOFF_DFSDM_FLTJDATAR), (uint32_t)pData, Length)) != E_OK)
			hdfsf->state = DFSDM_FILTER_STATE_ERROR;
		else	/* インジェクト変換スタート */
			DFSDM_InjConvStart(hdfsf);
		return ercd;
	}
	else
		return E_OBJ;
}

/*
 *  DFSDMフィルターインジェクト変換スタート(16BIT変換)
 *  parameter1 hdfsc    DFSDMフィルターハンドラへのポインタ
 *  paramter 2 pData    変換データ設定領域のポインタ
 *  parameter3 Length   領域長
 *  return              正常終了時、E_OK
 */
ER
dfsdm_filterInjectedMsbStart(DFSDM_Filter_Handle_t *hdfsf, int16_t *pData, uint32_t Length)
{
	if(hdfsf == NULL || pData == NULL || Length == 0)
		return E_PAR;

	/*
	 *  初期設定値の確認
	 */
	if((hdfsf->Init.InjTrigger == DFSDM_FILTER_SW_TRIGGER) && \
          (hdfsf->hdmaInj->Init.Mode == DMA_NORMAL) && (Length > hdfsf->InjConvRemaining)){
		return E_PAR;
	}
	else if((hdfsf->Init.InjTrigger == DFSDM_FILTER_SW_TRIGGER) && \
          (hdfsf->hdmaInj->Init.Mode == DMA_CIRCULAR)){
		return E_OBJ;
 	}

	/*
	 *  DMAの有効確認
	 */
	if((sil_rew_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1)) & DFSDM_FLTCR1_JDMAEN) == 0)
		return E_OBJ;

	/*
	 *  DFSDMフィルターの状態チェック
	 */
	if(hdfsf->state == DFSDM_FILTER_STATE_READY || hdfsf->state == DFSDM_FILTER_STATE_REG){
		ER ercd = E_OK;
		/*
		 *  コールバック関数の設定
		 */
		hdfsf->hdmaInj->xfercallback = DFSDM_DMAInjectedConvCplt;
		hdfsf->hdmaInj->errorcallback = DFSDM_DMAError;
		hdfsf->hdmaInj->xferhalfcallback = (hdfsf->hdmaInj->Init.Mode == DMA_CIRCULAR) ? DFSDM_DMAInjectedHalfConvCplt : NULL;

		/*
		 *  DMAスタート
		 */
		if((ercd = dma_start(hdfsf->hdmaInj, (uint32_t)(hdfsf->base+TOFF_DFSDM_FLTJDATAR+2), (uint32_t)pData, Length)) != E_OK)
			hdfsf->state = DFSDM_FILTER_STATE_ERROR;
		else
			DFSDM_InjConvStart(hdfsf);	/* インジェクト変換スタート */
		return ercd;
	}
	else
		return E_OBJ;
}

/*
 *  DFSDMフィルターインジェクト変換停止
 *  parameter1 hdfsc    DFSDMフィルターハンドラへのポインタ
 *  return              正常終了時、E_OK
 */
ER
dfsdm_filterInjectedStop(DFSDM_Filter_Handle_t *hdfsf)
{
	ER ercd = E_OK;

	if(hdfsf == NULL)
		return E_PAR;

	/*
	 *  DFSDMフィルターの状態チェック
	 */
	if(hdfsf->state != DFSDM_FILTER_STATE_INJ && hdfsf->state != DFSDM_FILTER_STATE_REG_INJ)
		return E_OBJ;

	/*
	 *  DMA停止
	 */
	if((ercd = dma_end(hdfsf->hdmaInj)) != E_OK)
		hdfsf->state = DFSDM_FILTER_STATE_ERROR;
	else
		DFSDM_InjConvStop(hdfsf);	/* インジェクト変換停止 */
	return ercd;
}

/*
 *  DFSDMフィルターインジェクト変換値取得
 *  parameter1 hdfsc    DFSDMフィルターハンドラへのポインタ
 *  parameter2 Channel  DFSDMチャネル番号設定用ポインタ
 *  return              インジェクト変換値
 */
int32_t
dfsdm_filterGetInjectedValue(DFSDM_Filter_Handle_t *hdfsf, uint32_t *Channel)
{
	uint32_t reg = 0;

	if(hdfsf == NULL || Channel == NULL)
		return 0;

	/*
	 *  インジェクト変換値とチャネルを取得
	 */
	reg = sil_rew_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTRDATAR));

	*Channel = (reg & DFSDM_FLTJDATAR_JDATACH);
	return (int32_t)((reg & DFSDM_FLTJDATAR_JDATA) >> 8);
}

/*
 *  アナルグウォッチドックスタート
 *  parameter1 hdfsc    DFSDMフィルターハンドラへのポインタ
 *  paramter 2 awdParam アナルグウォッチドック設定構造体へのポインタ
 *  return              正常終了時、E_OK
 */
ER
dfsdm_filterAwdStart(DFSDM_Filter_Handle_t *hdfsf, DFSDM_Filter_AwdParamTypeDef *awdParam)
{
	if(hdfsf == NULL)
		return E_PAR;
	if(awdParam->DataSource != DFSDM_FILTER_AWD_FILTER_DATA &&
		awdParam->DataSource != DFSDM_FILTER_AWD_CHANNEL_DATA)
		return E_PAR;
	if(awdParam->Channel == 0 || awdParam->Channel > 0x000F00FF)
		return E_PAR;
	if(awdParam->HighThreshold < -8388608 && awdParam->HighThreshold > 8388607)
		return E_PAR;
	if(awdParam->LowThreshold < -8388608 && awdParam->LowThreshold > 8388607)
		return E_PAR;
	if(awdParam->HighBreakSignal > 0xF)
		return E_PAR;
	if(awdParam->LowBreakSignal > 0xF)
		return E_PAR;

	/*
	 *  DFSDMフィルター状態チェック
	 */
	if(hdfsf->state == DFSDM_FILTER_STATE_RESET || hdfsf->state == DFSDM_FILTER_STATE_ERROR)
		return E_OBJ;

	/*
	 *  アナルグウォッチドック・データソース設定
	 */
	sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_AWFSEL);
	sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), awdParam->DataSource);

	/*
	 *  スレッシュホールド、ブレークシグナル設定
	 */
	sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTAWHTR), (DFSDM_FLTAWHTR_AWHT | DFSDM_FLTAWHTR_BKAWH));
	sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTAWHTR), ((awdParam->HighThreshold << 8) |
                                        awdParam->HighBreakSignal));
	sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTAWLTR), (DFSDM_FLTAWLTR_AWLT | DFSDM_FLTAWLTR_BKAWL));
	sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTAWLTR), ((awdParam->LowThreshold << 8) |
                                        awdParam->LowBreakSignal));

	/*
	 *  チャネルと割込み設定
	 */
	sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR2), DFSDM_FLTCR2_AWDCH);
	sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR2), (((awdParam->Channel & DFSDM_LSB_MASK) << 16) |
                                        DFSDM_FLTCR2_AWDIE));
	return E_OK;
}

/*
 *  アナルグウォッチドック停止
 *  parameter1 hdfsc    DFSDMフィルターハンドラへのポインタ
 *  return              正常終了時、E_OK
 */
ER
dfsdm_filterAwdStop(DFSDM_Filter_Handle_t *hdfsf)
{
	if(hdfsf == NULL)
		return E_PAR;

	/*
	 *  DFSDMフィルター状態チェック
	 */
	if(hdfsf->state == DFSDM_FILTER_STATE_RESET || hdfsf->state == DFSDM_FILTER_STATE_ERROR)
		return E_OBJ;

	/*
	 *  チャネルと割込みのリセット
	 */
	sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR2), (DFSDM_FLTCR2_AWDCH | DFSDM_FLTCR2_AWDIE));

	/*
	 *  アナログウォッチドッグフラグクリア
	 */
	sil_wrw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTAWCFR), (DFSDM_FLTAWCFR_CLRAWHTF | DFSDM_FLTAWCFR_CLRAWLTF));

	/*
	 *  スレッシュホールドとブレークシグナルをリセット
	 */
	sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTAWHTR), (DFSDM_FLTAWHTR_AWHT | DFSDM_FLTAWHTR_BKAWH));
	sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTAWLTR), (DFSDM_FLTAWLTR_AWLT | DFSDM_FLTAWLTR_BKAWL));

	/*
	 *  データソースをリセット
	 */
	sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR1), DFSDM_FLTCR1_AWFSEL);
	return E_OK;
}

/*
 *  DFSDMフィルター・エクストリーム検出スタート
 *  parameter1 hdfsc    DFSDMフィルターハンドラへのポインタ
 *  parameter2 Channel  チャネル番号
 *  return              正常終了時、E_OK
 */
ER
dfsdm_filterExdStart(DFSDM_Filter_Handle_t *hdfsf, uint32_t Channel)
{
	if(hdfsf == NULL)
		return E_PAR;

	/*
	 *  DFSDMフィルター状態チェック
	 */
	if(hdfsf->state == DFSDM_FILTER_STATE_RESET || hdfsf->state == DFSDM_FILTER_STATE_ERROR)
		return E_OBJ;

	/*
	 *  エクストリーム検出チャネル設定
	 */
	sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR2), DFSDM_FLTCR2_EXCH);
	sil_orw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR2), ((Channel & DFSDM_LSB_MASK) << 8));
	return E_OK;
}

/*
 *  DFSDMフィルター・エクストリーム検出停止
 *  parameter1 hdfsc    DFSDMフィルターハンドラへのポインタ
 *  return              正常終了時、E_OK
 */
ER
dfsdm_filterExdStop(DFSDM_Filter_Handle_t *hdfsf)
{
	volatile uint32_t     reg;

	if(hdfsf == NULL)
		return E_PAR;

	/*
	 *  DFSDMフィルター状態チェック
	 */
	if(hdfsf->state == DFSDM_FILTER_STATE_RESET || hdfsf->state == DFSDM_FILTER_STATE_ERROR)
		return E_OBJ;

	/*
	 *  エクストリーム検出チャネルリセット
	 */
	sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR2), DFSDM_FLTCR2_EXCH);

    /*
	 *  エクストリーム検出値取得
	 */
	reg = sil_rew_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTEXMAX));
	reg = sil_rew_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTEXMIN));
    ((void)(reg));
	return E_OK;
}

/*
 *  DFSDMフィルター・エクストリーム検出最大値取得
 *  parameter1 hdfsc    DFSDMフィルターハンドラへのポインタ
 *  parameter2 Chnnel   チャネル保存領域のポインタ
 *  return              最大値
 */
int32_t
dfsdm_filterGetExdMaxValue(DFSDM_Filter_Handle_t *hdfsf, uint32_t *Channel)
{
	uint32_t reg = 0;

	if(hdfsf == NULL || Channel == NULL)
		return E_PAR;

	/*
	 *  チャネルと最大値取得
	 */
	reg = sil_rew_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTEXMAX));
	*Channel = (reg & DFSDM_FLTEXMAX_EXMAXCH);
	return (int32_t)((reg & DFSDM_FLTEXMAX_EXMAX) >> 8);
}

/*
 *  DFSDMフィルター・エクストリーム検出最小値取得
 *  parameter1 hdfsc    DFSDMフィルターハンドラへのポインタ
 *  parameter2 Chnnel   チャネル保存領域のポインタ
 *  return              最小値
 */
int32_t
dfsdm_filterGetExdMinValue(DFSDM_Filter_Handle_t *hdfsf, uint32_t *Channel)
{
	uint32_t reg = 0;

	if(hdfsf == NULL || Channel == NULL)
		return E_PAR;

	/*
	 *  チャネルと最小値取得
	 */
	reg = sil_rew_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTEXMIN));
	*Channel = (reg & DFSDM_FLTEXMIN_EXMINCH);
	return (int32_t)((reg & DFSDM_FLTEXMIN_EXMIN) >> 8);
}

/*
 *  変換時間取得
 *  parameter1 hdfsc    DFSDMフィルターハンドラへのポインタ
 *  return              時間(秒)
 */
uint32_t
dfsdm_filterGetConvTimeValue(DFSDM_Filter_Handle_t *hdfsf)
{
	uint32_t reg = 0;

	if(hdfsf == NULL)
		return E_PAR;

	/*
	 *  変換時間レジスタ値取得
	 */
	reg = sil_rew_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCNVTIMR));
	return (uint32_t)((reg & DFSDM_FLTCNVTIMR_CNVCNT) >> 4);
}


/*
 *  DFSDMフィルター割込みハンドラ
 */
void dfsdm_irqhandler(DFSDM_Filter_Handle_t *hdfsf)
{
	DFSDM_Channel_Handle_t *hdfsc;
	uint32_t isr = sil_rew_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTISR));
	uint32_t cr2 = sil_rew_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR2));

	/*
	 * レギュラー変換オーバーランエラー
	 */
	if(((isr & cr2) & DFSDM_FLTISR_ROVRF) != 0){
		/*
		 *  オーバーラン要因クリア
		 */
		sil_wrw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTICR), DFSDM_FLTICR_CLRROVRF);

		/*
		 *  エラー設定とコールバック
		 */
		hdfsf->errorcode = DFSDM_FILTER_ERROR_REGULAR_OVERRUN;
		if(hdfsf->errorcallback != NULL)
			hdfsf->errorcallback(hdfsf);
	}
	/*
	 *  インジェクト変換オーバーランエラー
	 */
	else if(((isr & cr2) & DFSDM_FLTISR_JOVRF) != 0){
		/*
		 *  オーバーラン要因クリア
		 */
		sil_wrw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTICR), DFSDM_FLTICR_CLRJOVRF);

		/*
		 *  エラー設定とコールバック
		 */
		hdfsf->errorcode = DFSDM_FILTER_ERROR_INJECTED_OVERRUN;
		if(hdfsf->errorcallback != NULL)
			hdfsf->errorcallback(hdfsf);
	}
	/*
	 *  レギュラー変換終了
	 */
	else if(((isr & cr2) & DFSDM_FLTISR_REOCF) != 0){
		/*
		 *  コールバック
		 */
		if(hdfsf->regconvcallback != NULL)
			hdfsf->regconvcallback(hdfsf);

		/*
		 *  非継続モード、かつ、ソフトウェアトリガの場合
		 */
		if((hdfsf->RegularContMode == DFSDM_CONTINUOUS_CONV_OFF) &&
			(hdfsf->Init.RegTrigger == DFSDM_FILTER_SW_TRIGGER)){
			/*
			 *  レギュラー変換割込み停止
			 */
			sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR2), DFSDM_FLTCR2_REOCIE);

			/*
			 *  DFSDMフィルター状態更新
			 */
			hdfsf->state = (hdfsf->state == DFSDM_FILTER_STATE_REG) ? DFSDM_FILTER_STATE_READY : DFSDM_FILTER_STATE_INJ;
		}
	}
	/*
	 *  インジェクト変換終了
	 */
	else if(((isr & cr2) & DFSDM_FLTISR_JEOCF) != 0){
		/*
		 *  コールバック
		 */
		if(hdfsf->injconvcallback != NULL)
			hdfsf->injconvcallback(hdfsf);

		/*
		 *  残りのインジェクト変換更新
		 */
		if(--hdfsf->InjConvRemaining == 0){
			/*
			 *  ソフトウェアトリガなら、インジェクト変換停止
			 */
			if(hdfsf->Init.InjTrigger == DFSDM_FILTER_SW_TRIGGER){
				sil_andw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTCR2), DFSDM_FLTCR2_JEOCIE);
				hdfsf->state = (hdfsf->state == DFSDM_FILTER_STATE_INJ) ? DFSDM_FILTER_STATE_READY : DFSDM_FILTER_STATE_REG;
			}
			/*
			 *  インジェクト変換モード更新
			 */
			hdfsf->InjConvRemaining = (hdfsf->Init.InjScanMode == DFSDM_ENABLE) ? hdfsf->InjectedChannelsNbr : 1;
		}
	}
	/*
	 *  アナログウォッチドッグ発生
	 */
	else if(((isr & cr2) &DFSDM_FLTISR_AWDF) != 0){
		uint32_t reg = sil_rew_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTAWSR));
		uint32_t threshold = 0;
		uint32_t channel = 0;

		/*
		 *  チャネル番号とスレッシュホールド取得
		 */
		threshold = ((reg & DFSDM_FLTAWSR_AWLTF) != 0) ? DFSDM_AWD_LOW_THRESHOLD : DFSDM_AWD_HIGH_THRESHOLD;
		if(threshold == DFSDM_AWD_HIGH_THRESHOLD){
			reg = reg >> 8;
		}
		while((reg & 1) == 0){
			channel++;
			reg = reg >> 1;
		}
		/*
		 *  アナルグウォッチドッグ・フラグクリア
		 */
		sil_wrw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTAWCFR), (threshold == DFSDM_AWD_HIGH_THRESHOLD) ? \
                                        (1 << (8 + channel)) : (1 << channel));

		/*
		 *  アナルグウォッチドッグ・コールバック
		 */
		if(hdfsf->awdconvcallback != NULL)
			hdfsf->awdconvcallback(hdfsf);
	}
	/*
	 *  クロックアブセンス発生
	 */
	else if(hdfsf->base == TADR_DFSDM1_FILTER0_BASE &&
		(isr & DFSDM_FLTISR_CKABF) != 0 && (cr2 & DFSDM_FLTCR2_CKABIE) != 0){
		uint32_t reg = (sil_rew_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTISR)) & DFSDM_FLTISR_CKABF) >> 16;
		uint32_t channel = 0;

		while(channel < DFSDM1_CHANNEL_NUMBER){
			/*
			 *  有効チャネルサーチ
			 */
			if(((reg & 1) != 0) && (Dfsdm1ChannelHandle[channel] != NULL)){
				/*
				 *  クロックアブセンス・イネーブルのチャネルをチェック
				 */
				if((sil_rew_mem((uint32_t *)(Dfsdm1ChannelHandle[channel]->base+TOFF_DFSDM_CHCFGR1)) & DFSDM_CHCFGR1_CKABEN) != 0){
					/*
					 *  クロックアブセンス・フラグクリア
					 */
					sil_wrw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTICR), (1 << (16 + channel)));

					/*
					 *  対応チャネルのコールバック
					 */
					hdfsc = Dfsdm1ChannelHandle[channel];
					if(hdfsc != NULL && hdfsc->ckabcallback != NULL)
						hdfsc->ckabcallback(hdfsc);
				}
			}
			channel++;
			reg = reg >> 1;
		}
	}
	/*
	 *  ショートサーキット・デテクション発生
	 */
	else if(hdfsf->base == TADR_DFSDM1_FILTER0_BASE &&
		(isr & DFSDM_FLTISR_SCDF) != 0 && (cr2 & DFSDM_FLTCR2_SCDIE) != 0){
		uint32_t reg = 0;
		uint32_t channel = 0;

		/*
		 *  チャネル番号取得
		 */
		reg = (sil_rew_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTISR)) & DFSDM_FLTISR_SCDF) >> 24;
		while((reg & 1) == 0){
			channel++;
			reg = reg >> 1;
		}

		/*
		 *  ショートサーキット・デテクションフラグクリア
		 */
		sil_wrw_mem((uint32_t *)(hdfsf->base+TOFF_DFSDM_FLTICR), (1 << (24 + channel)));

		/*
		 *  ショートサーキット・デテクション・コールバック
		 */
		hdfsc = Dfsdm1ChannelHandle[channel];
		if(hdfsc != NULL && hdfsc->scdcallback != NULL)
			hdfsc->scdcallback(hdfsc);
	}
}

