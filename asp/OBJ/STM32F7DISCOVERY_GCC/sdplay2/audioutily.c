/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2015-2016 by TOPPERS PROJECT Educational Working Group.
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
 *  $Id: audioutil.c 2416 2016-03-05 12:23:35Z roi $
 */
/*
 *  AUDIO用ユーティリティ
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <kernel.h>
#include <t_stddef.h>
#include <t_syslog.h>
#include <target_syssvc.h>
#include "sdcard_play.h"
#include "mad.h"
#include "wm8994.h"
#include "kernel_cfg.h"

AUDIO_DrvTypeDef          *audio_drv = NULL;

static MUSIC_INFO Minfo;
static MUSIC_TRANS *pMTrans;

#define AUDIO_I2C_ADDRESS                ((uint16_t)0x34)
#define AUDIO_COUNT_MAX                  0x7FFFFFFF

/* To have 2 separate audio stream in Both headphone and speaker the 4 slot must be activated */
#define CODEC_AUDIOFRAME_SLOT_0123           SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1 | SAI_SLOTACTIVE_2 | SAI_SLOTACTIVE_3
/* To have an audio stream in headphone only SAI Slot 0 and Slot 2 must be activated */ 
#define CODEC_AUDIOFRAME_SLOT_02             SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_2
/* To have an audio stream in speaker only SAI Slot 1 and Slot 3 must be activated */ 
#define CODEC_AUDIOFRAME_SLOT_13             SAI_SLOTACTIVE_1 | SAI_SLOTACTIVE_3

#define AUDIODATA_SIZE                      ((uint16_t)2)   /* 16-bits audio data size */

#define DMA_MAX_SZE                              ((uint16_t)0xFFFF)
#define DMA_MAX(x)           (((x) <= DMA_MAX_SZE)? (x):DMA_MAX_SZE)

#define MPEG_BUFSZ           (64*1024)
#if 1	/* ROI DEBUG */
//#define MPEG_BUFSZ2          (64*1024)
#endif	/* ROI DEBUG */

static const uint16_t device_table[] = {
	OUTPUT_DEVICE_SPEAKER,
	OUTPUT_DEVICE_HEADPHONE,
	OUTPUT_DEVICE_BOTH,
	OUTPUT_DEVICE_AUTO,
	INPUT_DEVICE_DIGITAL_MICROPHONE_1,
	INPUT_DEVICE_DIGITAL_MICROPHONE_2,
	INPUT_DEVICE_INPUT_LINE_1,
	INPUT_DEVICE_INPUT_LINE_2
};

static const uint32_t option_table[] = {
	CODEC_PDWN_HW,
	CODEC_PDWN_SW
};


static uint32_t
audio_setup_buffer(MUSIC_INFO *minfo, MUSIC_TRANS *mtrans, int mode)
{
	uint32_t  *s, *d;
	uint32_t  len, len2, i = 0;

	d = (uint32_t *)(minfo->buffer+(mode-SET_BUFFER0)*WAV_BUFFER_SIZE);
	s = (uint32_t *)&minfo->maddr[pMTrans->mtail];
	if(pMTrans->msize < WAV_BUFFER_SIZE)
		len  = pMTrans->msize;
	else
		len = WAV_BUFFER_SIZE;
	syslog_2(LOG_NOTICE, "## audio_setup_buffer(1) rem[%08x] len[%08x] ##", mtrans->datasize - mtrans->transize, len);
	if(mtrans->transize >= mtrans->datasize){
		memset(d, 0, WAV_BUFFER_SIZE);
		return 0;
	}
	if((pMTrans->mtail + len) >= MUSIC_DATA_SIZE){
		len2 = MUSIC_DATA_SIZE - pMTrans->mtail;
		for(i = 0 ; i < len2 ; i += 4)
			*d++ = *s++;
		pMTrans->mtail = 0;
		s = (uint32_t *)minfo->maddr;
	}
	for(; i < len ; i += 4)
		*d++ = *s++;
	for(; i < WAV_BUFFER_SIZE; i += 4)
		*d++ = 0;
	pMTrans->mtail += len;
	pMTrans->msize -= len;
	return WAV_BUFFER_SIZE;
}

/*
 *  TRANSFAR END CALLBACK
 */
static void
audio_transfunc(AUDIO_Handle_t *haudio)
{
	if(pMTrans == NULL)
		return;
	pMTrans->transize += WAV_BUFFER_SIZE;
	audio_setup_buffer(pMTrans->minfo, pMTrans, SET_BUFFER1);
	iset_flg(AUDIO_FLG, SET_BUFFER1);
}

static void
audio_transhfunc(AUDIO_Handle_t *haudio)
{
	if(pMTrans == NULL)
		return;
	pMTrans->transize += WAV_BUFFER_SIZE;
	audio_setup_buffer(pMTrans->minfo, pMTrans, SET_BUFFER0);
	iset_flg(AUDIO_FLG, SET_BUFFER0);
}

void
audio_handle(AUDIO_Handle_t *haudio, uint32_t mode)
{
	syslog_2(LOG_NOTICE, "## audio int[%08x](%d) ##", haudio, mode);
}

/*
 *  AUDIO初期設定
 */
MUSIC_INFO *
AUDIO_Hard_Init(ID id)
{
	Minfo.haudio = audio_init(AUDIO1_PORTID);
	Minfo.haudio->audioerrorcallback = audio_handle;
	Minfo.maddr  = (uint8_t *)malloc(MUSIC_DATA_SIZE);
	Minfo.buffer = (uint8_t *)malloc(WAV_BUFFER_SIZE*2);
	AUDIO_OUT_Init(Minfo.haudio, AUDIO_OUT_DEVICE_HEADPHONE, 50, 22050, 32);
	memset(Minfo.buffer, 0, 0x4000);
	AUDIO_OUT_Play(Minfo.haudio, (uint16_t*)Minfo.buffer, 0x4000, 0);
	dly_tsk(1000);
	AUDIO_OUT_Stop(Minfo.haudio, POWERDOWN_HW);
	return &Minfo;
}

/*
 *  AUDIO OUT初期化
 */
ER
AUDIO_OUT_Init(AUDIO_Handle_t *haudio, AudioDevice Dev, uint8_t Volume, uint32_t AudioFreq, uint16_t framesize)
{
	ER ercd = E_OK;
	uint32_t deviceid = 0x00;

	syslog_3(LOG_NOTICE, "AUDIO_OUT_Init vol(%d) freq(%d) size(%d) !", Volume, AudioFreq, framesize);
	if(haudio == NULL)
		return E_PAR;
	/* PLL clock is set depending on the AudioFreq (44.1khz vs 48khz groups) */
	audio_clockconfig(haudio, AudioFreq, NULL);

	/* SAI data transfer preparation:
 	 Prepare the Media to be used for the audio transfer from memory to SAI peripheral */
	/* Configure SAI_Block_x 
	LSBFirst: Disabled 
	DataSize: 16 */
	haudio->OutInit.AudioFrequency = AudioFreq;
	haudio->OutInit.AudioMode = SAI_MODEMASTER_TX;
	haudio->OutInit.NoDivider = SAI_MASTERDIVIDER_ENABLE;
	haudio->OutInit.Protocol = SAI_FREE_PROTOCOL;
	haudio->OutInit.DataSize = SAI_DATASIZE_16;
	haudio->OutInit.FirstBit = SAI_FIRSTBIT_MSB;
	haudio->OutInit.ClockStrobing = SAI_CLOCKSTROBING_RISINGEDGE;
	haudio->OutInit.Synchro = SAI_ASYNCHRONOUS;
	haudio->OutInit.OutputDrive = SAI_OUTPUTDRIVE_ENABLE;
	haudio->OutInit.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;

	/* Configure SAI_Block_x Frame 
	Frame Length: 64
	Frame active Length: 32
	FS Definition: Start frame + Channel Side identification
	FS Polarity: FS active Low
	FS Offset: FS asserted one bit before the first bit of slot 0 */ 
	haudio->OutInit.FrameLength = framesize; 
	haudio->OutInit.ActiveFrameLength = framesize / 2;
	haudio->OutInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
	haudio->OutInit.FSPolarity = SAI_FS_ACTIVE_LOW;
	haudio->OutInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;

	/* Configure SAI Block_x Slot 
	Slot First Bit Offset: 0
	Slot Size  : 16
	Slot Number: 4
	Slot Active: All slot actives */
	haudio->OutInit.FirstBitOffset = 0;
	haudio->OutInit.SlotSize = SAI_SLOTSIZE_DATASIZE;
	haudio->OutInit.SlotNumber = 4; 
	haudio->OutInit.SlotActive = CODEC_AUDIOFRAME_SLOT_0123;

	audio_start(haudio, AUDIO_OUT_BLOCK);

	/* Enable SAI peripheral to generate MCLK */
	audio_enable(haudio, AUDIO_OUT_BLOCK);

	/* wm8994 codec initialization */
	if(audio_drv == NULL){
		deviceid = wm8994_drv.ReadID(AUDIO_I2C_ADDRESS);

		if((deviceid) == WM8994_ID){
			/* Reset the Codec Registers */
			wm8994_drv.Reset(AUDIO_I2C_ADDRESS);
			/* Initialize the audio driver structure */
			audio_drv = &wm8994_drv; 
		}
		else
			ercd = E_SYS;
	}

	if(ercd == E_OK){
		/* Initialize the codec internal registers */
		audio_drv->Init(AUDIO_I2C_ADDRESS, device_table[Dev], Volume, AudioFreq);
	}
	return ercd;
}

/*
 *  AUDIO OUT PLAY
 */
ER
AUDIO_OUT_Play(AUDIO_Handle_t *haudio, uint16_t* pBuffer, uint32_t Size, uint32_t mode)
{
	if(haudio == NULL)
		return E_PAR;

	/* Call the audio Codec Play function */
	if(audio_drv->Play(AUDIO_I2C_ADDRESS, pBuffer, Size) != 0){
		return E_OBJ;
	}
	else{
		/* Update the Media layer and enable it for play */  
		haudio->transcallback = audio_transfunc;
		if(mode == 1)
			haudio->transhalfcallback = audio_transhfunc;
		else
			haudio->transhalfcallback = NULL;
		haudio->errorcallback = NULL;
		audio_transmit(haudio, (uint8_t*) pBuffer, DMA_MAX(Size / AUDIODATA_SIZE));
		return E_OK;
	}
}

/*
 *  AUDIO OUT STOP
 */
ER
AUDIO_OUT_Stop(AUDIO_Handle_t *haudio, CodecPowerDown Option)
{
	if(haudio == NULL)
		return E_PAR;
	/* Call the Media layer stop function */
	audio_dmastop(haudio, AUDIO_OUT_BLOCK);
	audio_disable(haudio, AUDIO_OUT_BLOCK);

	/* Call Audio Codec Stop function */
	if(audio_drv->Stop(AUDIO_I2C_ADDRESS, option_table[Option]) != 0){
		return E_OBJ;
	}
	else{
		if(Option == POWERDOWN_HW){
			/* Wait at least 100us */
			dly_tsk(1);
		}
		/* Return AUDIO_OK when all operations are correctly done */
		return E_OK;
	}
}


/*
 *  WAVデータ転送
 */
ER
wav_transfar(MUSIC_TRANS *mtrans)
{
	static Riff_Header    rhead;
	static Data_Header    dhead;
	MUSIC_INFO *minfo = mtrans->minfo;
	FLGPTN   flgptn;
	ER       ercd = E_OK;
	int      len, no;

	pMTrans = mtrans;
	mtrans->mcount = 0;
	mtrans->mhead  = 0;
	mtrans->mtail  = 0;
	mtrans->transize = 0;
	no = fread(&rhead, sizeof(Riff_Header), 1, minfo->fileid);
	syslog_3(LOG_NOTICE, "## no(%d)[%08x][%08x] ##", no, rhead.chank_size, &rhead);
	if(rhead.chank_size > 16){
		fread(minfo->maddr, rhead.chank_size-16, 1, minfo->fileid);
	}
	no = fread(&dhead, sizeof(Data_Header), 1, minfo->fileid);
	while(dhead.id[0] != 'd' || dhead.id[1] != 'a' || dhead.id[2] != 't' || dhead.id[3] != 'a'){
		no = fread(minfo->maddr, dhead.data_size, 1, minfo->fileid);
		no = fread(&dhead, sizeof(Data_Header), 1, minfo->fileid);
		break;
	}
	syslog_4(LOG_NOTICE, "## file size[%08x] channels(%d) audiofreq(%d) bitswidth(%d) ##", rhead.file_size, rhead.channels, rhead.audiofreq, rhead.bitswidth);
	syslog_2(LOG_NOTICE, "## blocksize(%d) bytepersec(%d)  ##", rhead.blocksize, rhead.bytepersec);
	syslog_1(LOG_NOTICE, "## size[%08x] ##", dhead.data_size);
	mtrans->datasize   = dhead.data_size;
	mtrans->channels   = rhead.blocksize / 2;
	mtrans->samplerate = rhead.audiofreq / mtrans->channels;
	mtrans->func(mtrans);
	mtrans->mcount++;

	if(mtrans->datasize <= MUSIC_DATA_SIZE)
		minfo->transize = mtrans->datasize;
	else
		minfo->transize = MUSIC_DATA_SIZE;
	if(mtrans->datasize <= (WAV_BUFFER_SIZE*2)){
		minfo->buffsize = minfo->datasize;
		minfo->playmode = 0;
	}
	else{
		minfo->buffsize = (WAV_BUFFER_SIZE*2);
		minfo->playmode = 1;
	}
	no = fread(minfo->maddr, 1, minfo->transize, minfo->fileid);
	mtrans->mhead = no;
	mtrans->msize = no;
	audio_setup_buffer(minfo, mtrans, SET_BUFFER0);
	audio_setup_buffer(minfo, mtrans, SET_BUFFER1);
	mtrans->func(mtrans);
	mtrans->mcount++;

	while(mtrans->transize < mtrans->datasize){
		ercd = twai_flg(AUDIO_FLG, SET_BUFFER, TWF_ORW, &flgptn, 5000);
		if(ercd != E_OK){
			syslog_1(LOG_ERROR, "audio timeout(%d) !", ercd);
			break;
		}
		len = MUSIC_DATA_SIZE - mtrans->msize;
		no = minfo->datasize - minfo->transize;
		if(no <= 0)
			continue;
		if(len > no)
			len = no;
		if((mtrans->mhead + len) >= MUSIC_DATA_SIZE){
			no = MUSIC_DATA_SIZE - mtrans->mhead;
			if(no > 0)
				no = fread(&minfo->maddr[mtrans->mhead], 1, no, minfo->fileid);
			mtrans->mhead += no - MUSIC_DATA_SIZE;
			mtrans->msize += no;
			minfo->transize += no;
			len -= no;
		}
		no = fread(&minfo->maddr[mtrans->mhead], 1, len, minfo->fileid);
		mtrans->mhead += no;
		mtrans->msize += no;
		minfo->transize += len;
		mtrans->func(mtrans);
		mtrans->mcount++;
	}
	pMTrans = NULL;
	return ercd;
}

/*
 * This is the input callback. The purpose of this callback is to (re)fill
 * the stream buffer which is to be decoded. In this example, an entire file
 * has been mapped into memory, so we just call mad_stream_buffer() with the
 * address and length of the mapping. When this callback is called a second
 * time, we are finished decoding.
 */
static enum
mad_flow input(void *data,
		    struct mad_stream *stream)
{
	MUSIC_TRANS *mtrans = data;
	MUSIC_INFO  *minfo;
	int len, len2;

#ifndef MPEG_BUFSZ2
#if 1	/* ROI DEBUG */
	static ccount = 0;
#endif	/* ROI DEBUG */
	minfo = mtrans->minfo;
	if(minfo->transize >= minfo->datasize || mtrans->mcount >= AUDIO_COUNT_MAX)
		return MAD_FLOW_STOP;

	if(stream->next_frame) {
		BCOPY(stream->next_frame, minfo->start,
			minfo->length = &minfo->start[minfo->length] - stream->next_frame);
	}

	syslog_1(LOG_NOTICE, "## input:read [%08x] ##", minfo->start + minfo->length);
	len = fread(minfo->start + minfo->length, 1, MPEG_BUFSZ - minfo->length, minfo->fileid);
	syslog_3(LOG_NOTICE, "input:read len[%08x] mtrans->length[%08x][%08x]!", len, minfo->length, minfo->transize);

	if(len == 0){
		if((MPEG_BUFSZ - minfo->length) < MAD_BUFFER_GUARD)
			syslog_2(LOG_ERROR, "input:buffer->length[%08x] GUARD[%08x] !", minfo->length, MAD_BUFFER_GUARD);

		len2 = len;
		while (len2 < MAD_BUFFER_GUARD && (minfo->length+len) < MPEG_BUFSZ)
			minfo->start[minfo->length + len2++] = 0;
	}

	mad_stream_buffer(stream, minfo->start, minfo->length += len);
#if 1	/* ROI DEBUG */
	if(ccount == 1){
		syslog_2(LOG_NOTICE, "## STOP minfo->start[%08x] minfo->length[%08x] ##", minfo->start, minfo->length);
		slp_tsk();
	}
	ccount++;
#endif	/* ROI DEBUG */
	minfo->transize += len;
#else
	static int count=0;
	minfo = mtrans->minfo;
	if(minfo->transize >= minfo->datasize || mtrans->mcount >= AUDIO_COUNT_MAX)
		return MAD_FLOW_STOP;

	len = MPEG_BUFSZ2;
	if(stream->next_frame) {
		len -= &minfo->start[minfo->length] - stream->next_frame;
		BCOPY(stream->next_frame, minfo->start, count);
		minfo->length = MPEG_BUFSZ2;
		count -= MPEG_BUFSZ2;
		if(count < (MPEG_BUFSZ2*2)){
			syslog_0(LOG_NOTICE, "## STOP ##");
			slp_tsk();
		}
	}

	if(count == 0){
		len = fread(minfo->start + minfo->length, 1, MPEG_BUFSZ, minfo->fileid);
		syslog_3(LOG_NOTICE, "input:read len[%08x] mtrans->length[%08x][%08x]!", len, minfo->length, minfo->transize);
		minfo->length = MPEG_BUFSZ2;
		count = MPEG_BUFSZ - minfo->length;
	}
	mad_stream_buffer(stream, minfo->start, minfo->length);
	minfo->transize += len;
#endif
	syslog_2(LOG_NOTICE, "input:trans[%08x] size[%08x]", minfo->transize, minfo->length);

	return MAD_FLOW_CONTINUE;
}

/*
 * The following utility routine performs simple rounding, clipping, and
 * scaling of MAD's high-resolution samples down to 16 bits. It does not
 * perform any dithering or noise shaping, which would be recommended to
 * obtain any exceptional audio quality. It is therefore not recommended to
 * use this routine if high-quality output is desired.
 */
static inline
signed int scale(mad_fixed_t sample)
{
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

/*
 * This is the output callback function. It is called after each frame of
 * MPEG audio data has been completely decoded. The purpose of this callback
 * is to output (or play) the decoded PCM audio.
 */
static enum
mad_flow output(void *data,
		     struct mad_header const *header,
		     struct mad_pcm *pcm)
{
	MUSIC_TRANS *mtrans = data;
	MUSIC_INFO  *minfo;
	FLGPTN   flgptn;
	ER       ercd = E_OK;
	unsigned int nsamples;
	mad_fixed_t const *left_ch, *right_ch;

	minfo = mtrans->minfo;
	if(mtrans->mcount == 0){
		mtrans->channels   = pcm->channels;
		mtrans->samplerate = pcm->samplerate / mtrans->channels;
		mtrans->func(mtrans);
		mtrans->mcount++;
	}
	if(mtrans->mcount >= AUDIO_COUNT_MAX)
		return MAD_FLOW_CONTINUE;

	/* pcm->samplerate contains the sampling frequency */

	nsamples  = pcm->length;
	left_ch   = pcm->samples[0];
	right_ch  = pcm->samples[1];

	while (nsamples--) {
		signed int sample;

		/* output sample(s) in 16-bit signed little-endian PCM */

		while((volatile int)mtrans->msize >= (MUSIC_DATA_SIZE-3)){
			if(mtrans->mcount == 1){
				mtrans->func(mtrans);
				mtrans->mcount++;
			}
			ercd = twai_flg(AUDIO_FLG, SET_BUFFER, TWF_ORW, &flgptn, 5000);
			if(ercd != E_OK){
				syslog_1(LOG_ERROR, "output timeout(%d) !", ercd);
				minfo->transize = minfo->datasize;
				mtrans->transize = AUDIO_COUNT_MAX;
				mtrans->mcount = AUDIO_COUNT_MAX;
				return MAD_FLOW_CONTINUE;
			}
		}
		sample = scale(*left_ch++);
		loc_cpu();
		minfo->maddr[mtrans->mhead++] = (sample >> 0) & 0xff;
		minfo->maddr[mtrans->mhead++] = (sample >> 8) & 0xff;
		if(mtrans->mhead >= MUSIC_DATA_SIZE)
			mtrans->mhead -= MUSIC_DATA_SIZE;
		mtrans->msize += 2;
		mtrans->datasize += 2;
		if (mtrans->channels == 2) {
			sample = scale(*right_ch++);
			minfo->maddr[mtrans->mhead++] = (sample >> 0) & 0xff;
			minfo->maddr[mtrans->mhead++] = (sample >> 8) & 0xff;
			if(mtrans->mhead >= MUSIC_DATA_SIZE)
				mtrans->mhead -= MUSIC_DATA_SIZE;
			mtrans->msize += 2;
			mtrans->datasize += 2;
		}
		unl_cpu();
	}

	return MAD_FLOW_CONTINUE;
}

/*
 * This is the error callback function. It is called whenever a decoding
 * error occurs. The error is indicated by stream->error; the list of
 * possible MAD_ERROR_* errors can be found in the mad.h (or stream.h)
 * header file.
 */
static enum
mad_flow error(void *data,
		    struct mad_stream *stream,
		    struct mad_frame *frame)
{
	MUSIC_TRANS *mtrans = data;
	MUSIC_INFO  *minfo;

	minfo = mtrans->minfo;
	syslog_3(LOG_ERROR, "decoding error 0x%04x (%s) at byte offset %u",
		stream->error, mad_stream_errorstr(stream),
		stream->this_frame - minfo->start);

	/* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

	return MAD_FLOW_CONTINUE;
}

/*
 *  MP3 DECODE
 */
ER
mp3_decode(MUSIC_TRANS *mtrans)
{
	MUSIC_INFO  *minfo;
	struct mad_decoder decoder;
	FLGPTN   flgptn;
	int result;

	/* initialize our private message structure */
	pMTrans = mtrans;
	minfo = mtrans->minfo;
	minfo->start  = malloc(MPEG_BUFSZ);
	minfo->length = 0;
	minfo->buffsize = (WAV_BUFFER_SIZE*2);
	minfo->playmode = 1;
	minfo->transize = 0;
	syslog_1(LOG_NOTICE, "## fsize[%08x] ##", minfo->datasize);

	mtrans->mcount = 0;
	mtrans->mhead  = 0;
	mtrans->mtail  = 0;
	mtrans->msize  = 0;
	mtrans->datasize = 0;
	mtrans->transize = 0;

	/* configure input, output, and error functions */

	mad_decoder_init(&decoder, mtrans,
		   input, 0 /* header */, 0 /* filter */, output,
		   error, 0 /* message */);

	/* start decoding */

	result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

	/* release the decoder */

	mad_decoder_finish(&decoder);
	syslog_2(LOG_NOTICE, "## data[%08x] trans[%08x] ##", minfo->datasize, minfo->transize);
	while(mtrans->transize < mtrans->datasize){
		twai_flg(AUDIO_FLG, SET_BUFFER, TWF_ORW, &flgptn, 1000);
	}
	free(minfo->start);
	pMTrans = NULL;

	if(result == 0)
		return E_OK;
	else
		return E_OBJ;
}

