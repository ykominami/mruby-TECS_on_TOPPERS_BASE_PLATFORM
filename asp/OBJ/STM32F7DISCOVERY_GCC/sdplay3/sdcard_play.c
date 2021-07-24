/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2012 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
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
 *  $Id: sdcard_play.c 2416 2016-04-24 10:05:32Z ertl-hiro $
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "kernel_cfg.h"
#include "storagedevice.h"
#include "device.h"
#include "sdmmc.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "stdio.h"
#include "ff.h"
#include "fcntl.h"
#define _STM32F7XX_H_
#include "sdcard_play.h"
#include "topamelogo.h"


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


#define TIME_POSX   20
#define TIME_POSY   50
#define DATE_POSX   100
#define DATE_POSY   200
#define CZOOM       6
#define AUDIO_BOX   50
#define MUSIC_COUNT 10000

uint32_t heap_param[2] = {
	(SDRAM_ADDRESS+0x00100000),
	((7*1024)*1024)
};

static char root_dir[] = "0:";
static volatile int  disp_status = 0;
static volatile int  vol_status = 0;
static volatile int  mode = 0;
static volatile int  sdmode = 0;
static volatile int  admode = 0;
static volatile int  advolume = 50;
static TS_StateTypeDef  TS_State;
static struct dirent2 finfo;
static struct tm2 time;
static char cbuff[260];


static FILE_INFO  *pFinfo;
static MUSIC_INFO *pMinfo;
static uint8_t    *iaddr;



static void storagedev_notice(void *psdev, bool_t sw)
{
	int i;
	if(sw)
		vol_status |= 1<<((StorageDevice_t*)psdev)->_sdev_devno;
	else
		vol_status &= ~(1<<((StorageDevice_t*)psdev)->_sdev_devno);
	for(i = 0 ; i < 4 ; i++){
		if((vol_status & (1<<i)) != 0){
			root_dir[0] = '0' + i;
			break;
		}
	}
	syslog_4(LOG_NOTICE, "MEDIA[%08x][%04x][%s] on/off(%d) !", psdev, vol_status, root_dir, sw);
	mode = 1;
	sdmode = 2;
}

static uint32_t get_fat_time(void)
{
	int    reqtime = 0;
#ifndef SAMPLE_PROGRAM
	struct tm2 timedate;

	rtc_get_time(&timedate);
	reqtime  = ((timedate.tm_year-10)<<25)+(timedate.tm_mon<<21)+(timedate.tm_mday<<16);
	reqtime += (timedate.tm_hour<<11)+(timedate.tm_min<<5)+(timedate.tm_sec/2);
#else
	reqtime = 0x00210000;
#endif
	return reqtime;
}

/*
 *  ファイル拡張子チェック
 */
int
_checkext(const char* fpath, const char* ext)
{
	int i, j;
	char c;
	for(i = j = 0 ; i < MAX_PATH && fpath[i] != 0 ; i++){
		if(fpath[i] == '.'){
			j = i;
		}
	}
	if(j == 0)
		return 0;
	i = j + 1;
	for(j = 0 ; i < MAX_PATH && fpath[i] != 0 && ext[j] != 0 ; i++){
		c = fpath[i];
		if(c >= 'a' && c <= 'z')
			c -= 0x20;
		if(c != ext[j])
			return 0;
		j++;
	}
	return j;
}

/*
 *  拡大文字表示
 */
static void
DrawCharZ(uint16_t Xpos, uint16_t Ypos, sFONT *fonts, uint8_t ch, uint32_t textcolor, uint32_t backcolor, int times)
{
	uint32_t i = 0, j = 0;
	uint16_t height, width;
	uint8_t  offset;
	uint8_t  *pchar;
	const uint8_t *c;
	uint32_t line;

	height = fonts->Height;
	width  = fonts->Width;
	c = &fonts->table[(ch-' ') * height * ((width + 7) / 8)];
	offset =  8 *((width + 7)/8) -  width ;

	for(i = 0; i < height; i++){
		pchar = ((uint8_t *)c + (width + 7)/8 * i);

		switch(((width + 7)/8)){
		case 1:
			line =  pchar[0];
			break;
		case 2:
			line =  (pchar[0]<< 8) | pchar[1];
			break;
		case 3:
		default:
			line =  (pchar[0]<< 16) | (pchar[1]<< 8) | pchar[2];
			break;
		}

		for (j = 0; j < width; j++){
			if(line & (1 << (width- j + offset- 1)))
				BSP_LCD_SetTextColor(textcolor);
			else
				BSP_LCD_SetTextColor(backcolor);
			BSP_LCD_FillRect((Xpos + j*times), Ypos, times, times);
		}
		Ypos += times;
	}
}

/*
 *  バナー表示
 */
static void
SD_Banner(void)
{
	BSP_LCD_SetFont(&LCD_DEFAULT_FONT);
	BSP_LCD_Clear(LCD_COLOR_BLACK);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_FillRect(60, 40, BSP_LCD_GetXSize()-120, BSP_LCD_GetYSize()-80);
	BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_SetFont(&Font24);
#if _DRIVES > 1
	BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()/2-60, (uint8_t *)"SD-CARD/MSC PLAYER", CENTER_MODE);
#else
	BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()/2-60, (uint8_t *)"SD-CARD PLAYER", CENTER_MODE);
#endif
	BSP_LCD_SetFont(&Font12);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
#if _DRIVES > 1
	BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()/2-20, (uint8_t *)"TOPPERS BASE PLATFORM V1.3 for stm32f746", CENTER_MODE);
#else
	BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()/2-20, (uint8_t *)"TOPPERS BASE PLATFORM V1.0 for stm32f746", CENTER_MODE);
#endif
	BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()/2, (uint8_t *)"TOPPERS PROJECT Educational Working Group", CENTER_MODE);
	BSP_LCD_DrawBitmap(BSP_LCD_GetXSize()/2+40, BSP_LCD_GetYSize()/2+20, (uint8_t *)topamelogo);
}

/*
 *  アイコン表示
 */
static void
SD_SetHint(FILE_INFO *pfinfo, int sno)
{
	static uint8_t ssec = 61;
	StorageDevice_t *psdev;
	int x, y, h;
	int mx, my;

	if((disp_status & (JPEG_APL<<MENU_SHIFT)) != 0)
		return;
	rtc_get_time(&time);
	if(ssec == time.tm_sec)
		return;
	ssec = time.tm_sec;
	h = BSP_LCD_GetYSize() / 4;
	mx = BSP_LCD_GetXSize() - h + 1;
	my = 1;
	BSP_LCD_SetFont(&Font12);
	if((disp_status & CLOCK_APL) != 0){
		BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
		BSP_LCD_FillRect(mx, my, h-2, h-2);
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
		x = mx + 4;
		y = my + 2;
		BSP_LCD_DisplayStringAt(x+12, y, (uint8_t *)"clock", LEFT_MODE);
		if((disp_status & (CLOCK_APL<<MENU_SHIFT)) == 0){
			y += 20;
			sprintf(cbuff, "%02d/%02d/%02d", time.tm_year+1970-2000, time.tm_mon, time.tm_mday);
			BSP_LCD_DisplayStringAt(x, y, (uint8_t *)cbuff, LEFT_MODE);
			y += 15;
			sprintf(cbuff, "%02d:%02d:%02d", time.tm_hour, time.tm_min, time.tm_sec);
			BSP_LCD_DisplayStringAt(x, y, (uint8_t *)cbuff, LEFT_MODE);
		}
		else if(sno >= 0){
			BSP_LCD_DisplayStringAt(x+(h/2)-7, y+20, (uint8_t *)"up", LEFT_MODE);
			BSP_LCD_DisplayStringAt(x+(h/2)-14, y+40, (uint8_t *)"down", LEFT_MODE);
		}
	}
	my += h;
	if((disp_status & MEDIA_APL) != 0){
		BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
		BSP_LCD_FillRect(mx, my, h-2, h-2);
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		BSP_LCD_SetBackColor(LCD_COLOR_DARKBLUE);
		x = mx + 4;
		y = my + 2;
		BSP_LCD_DisplayStringAt(x+12, y, (uint8_t *)"media", LEFT_MODE);
		psdev = SDMGetStorageDevice(pfinfo->devno);
		if(pfinfo->num_file == 0 || psdev == NULL)
			BSP_LCD_DisplayStringAt(x, y+40, (uint8_t *)"no media", LEFT_MODE);
		else if((psdev->_sdev_attribute & SDEV_DEVERROR) != 0)
			BSP_LCD_DisplayStringAt(x, y+40, (uint8_t *)"error", LEFT_MODE);
		else{
			y += 20;
			cbuff[0] = '[';
			cbuff[1] = pfinfo->devno + '0';
			cbuff[2] = ':';
			cbuff[3] = '/';
			cbuff[4] = ']';
			cbuff[5] = 0;
			BSP_LCD_DisplayStringAt(x, y, (uint8_t *)cbuff, LEFT_MODE);
			sprintf(cbuff, "%d file", pfinfo->num_file);
			y += 15;
			BSP_LCD_DisplayStringAt(x, y, (uint8_t *)cbuff, LEFT_MODE);
		}
	}
	my += h;
	if((disp_status & AUDIO_APL) != 0){
		BSP_LCD_SetTextColor(LCD_COLOR_RED);
		BSP_LCD_FillRect(mx, my, h-2, h-2);
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		BSP_LCD_SetBackColor(LCD_COLOR_RED);
		x = mx + 4;
		y = my + 2;
		BSP_LCD_DisplayStringAt(x+12, y, (uint8_t *)"audio", LEFT_MODE);
		if(disp_status & (AUDIO_APL<<ACTSHIFT)){
			if((pMinfo->status & AUDIO_PHASE) != 0)
				BSP_LCD_DisplayStringAt(x+8, y+20, (uint8_t *)" stop ", LEFT_MODE);
			else
				BSP_LCD_DisplayStringAt(x+8, y+20, (uint8_t *)"playing", LEFT_MODE);
		}
	}
	my += h;
	if((disp_status & JPEG_APL) != 0){
		BSP_LCD_SetTextColor(LCD_COLOR_DARKGREEN);
		BSP_LCD_FillRect(mx, my, h-2, h-2);
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		BSP_LCD_SetBackColor(LCD_COLOR_DARKGREEN);
		x = mx + 4;
		y = my + 2;
		BSP_LCD_DisplayStringAt(x+12, y, (uint8_t *)"jpeg", LEFT_MODE);
	}
}

/*
 *  クロックメニュー表示
 */
static void
SD_DispTime(int sno, int upmode)
{
	int x = TIME_POSX;
	int y = TIME_POSY;
	int h = BSP_LCD_GetYSize() / 4;
	char *p;
	uint32_t textcolor = LCD_COLOR_BLACK;
	static uint8_t ssec = 61;

	if((disp_status & (JPEG_APL<<MENU_SHIFT)) != 0)
		return;
	rtc_get_time(&time);
	if(ssec == time.tm_sec && sno < 0)
		return;
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_FillRect(1, 1, BSP_LCD_GetXSize()-h-2, BSP_LCD_GetYSize()-2);
	BSP_LCD_SetFont(&Font24);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);

	if(sno == 3){
		textcolor = LCD_COLOR_BLUE;
		if(upmode > 0 && time.tm_hour < 59)
			time.tm_hour++;
		else if(upmode < 0 && time.tm_hour > 0)
			time.tm_hour--;
	}
	sprintf(cbuff, "%02d", time.tm_hour);
	p = cbuff;
	while(*p > 0){
		DrawCharZ(x, y, &Font24, *p++, textcolor, LCD_COLOR_WHITE, CZOOM);
		x += (Font24.Width-2) * CZOOM;
	}
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_FillRect(x, y+40, CZOOM*2, CZOOM*2);
	BSP_LCD_FillRect(x, y+80, CZOOM*2, CZOOM*2);
	x += 10;
	textcolor = LCD_COLOR_BLACK;
	if(sno == 4){
		textcolor = LCD_COLOR_BLUE;
		if(upmode > 0 && time.tm_min < 59)
			time.tm_min++;
		else if(upmode < 0 && time.tm_min > 0)
			time.tm_min--;
	}
	sprintf(cbuff, "%02d", time.tm_min);
	p = cbuff;
	while(*p > 0){
		DrawCharZ(x, y, &Font24, *p++, textcolor, LCD_COLOR_WHITE, CZOOM);
		x += (Font24.Width-2) * CZOOM;
	}
	x = BSP_LCD_GetXSize() - h - 50;
	y = TIME_POSY-30;
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	if(sno == 5){
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
		if(upmode > 0 && time.tm_sec < 59)
			time.tm_sec++;
		else if(upmode < 0 && time.tm_sec > 0)
			time.tm_sec--;
	}
	sprintf(cbuff, "%02d", time.tm_sec);
	BSP_LCD_DisplayStringAt(x, y, (uint8_t *)cbuff, LEFT_MODE);

	x = DATE_POSX;
	y = DATE_POSY;
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	if(sno == 0){
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
		if(upmode > 0 && time.tm_year < 130)
			time.tm_year++;
		else if(upmode < 0 && time.tm_year > 30)
			time.tm_year--;
	}
	sprintf(cbuff, "%04d", time.tm_year+1970);
	BSP_LCD_DisplayStringAt(x, y, (uint8_t *)cbuff, LEFT_MODE);
	x += 17*4;
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	cbuff[0] = '/';
	cbuff[1] = 0;
	BSP_LCD_DisplayStringAt(x, y, (uint8_t *)cbuff, LEFT_MODE);
	x += 17;
	if(sno == 1){
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
		if(upmode > 0 && time.tm_mon < 12)
			time.tm_mon++;
		else if(upmode < 0 && time.tm_mon > 1)
			time.tm_mon--;
	}
	sprintf(cbuff, "%02d", time.tm_mon);
	BSP_LCD_DisplayStringAt(x, y, (uint8_t *)cbuff, LEFT_MODE);
	x += 17*2;
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	cbuff[0] = '/';
	cbuff[1] = 0;
	BSP_LCD_DisplayStringAt(x, y, (uint8_t *)cbuff, LEFT_MODE);
	x += 17;
	if(sno == 2){
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
		if(upmode > 0 && time.tm_mday < 31)
			time.tm_mday++;
		else if(upmode < 0 && time.tm_mday > 1)
			time.tm_mday--;
	}
	sprintf(cbuff, "%02d", time.tm_mday);
	BSP_LCD_DisplayStringAt(x, y, (uint8_t *)cbuff, LEFT_MODE);

	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	ssec = time.tm_sec;
	if(sno >= 0 && upmode != 0){
		rtc_set_time(&time);
	}
}

/*
 *  メディア表示
 */
static void
SD_DispFile(FILE_INFO *pfinfo, int rno)
{
	struct dirent2 *finfo;
	int    no, year, count, len;
	int    x = 5, y = 8;
	int    h = BSP_LCD_GetYSize() / 4;

	if((disp_status & (JPEG_APL<<MENU_SHIFT)) != 0)
		return;
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_FillRect(1, 1, BSP_LCD_GetXSize()-h-2, BSP_LCD_GetYSize()-2);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_SetFont(&Font12);
	if(pfinfo->num_file == 0){
		BSP_LCD_DisplayStringAt(x, y, (uint8_t *)"no file !", LEFT_MODE);
		return;
	}
	for(count = 0 ; count < pfinfo->num_file && y < (BSP_LCD_GetYSize()-30) ; count++){
		if(count == rno){
			BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
			BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
		}
		else{
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
			BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
		}
		memset(cbuff, ' ', sizeof(cbuff));
		len = (BSP_LCD_GetXSize()-h-4) / 7;
		cbuff[len] = 0;
		finfo = &pfinfo->finfo[count];
		sprintf(cbuff, "%02d %s ", count+1, finfo->d_name);
		no = strlen(finfo->d_name);
		for(no += 3 ; no < len ; no++){
			if(cbuff[no] < ' ')
				cbuff[no] = ' ';
		}
		no = len - 22;
		year = (finfo->d_date>>9);
		if(year >= 20)
			year -= 20;
		else
			year += 80;
		sprintf(&cbuff[no], "%02d/%02d/%02d ", year, (finfo->d_date>>5) & 15, finfo->d_date & 31);
		no += 9;
		if(finfo->d_type & AM_DIR)
			strcpy(&cbuff[no], "<DIR>  ");
		else if(finfo->d_type & AM_VOL)
			strcpy(&cbuff[no], "<VOL>  ");
		else{
			sprintf(&cbuff[no], "%d", (int)finfo->d_fsize);
		}
		BSP_LCD_DisplayStringAt(x, y, (uint8_t *)cbuff, LEFT_MODE);
		y += 15;
	}
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	sprintf(cbuff, "%d free blocks %d bytes in a block", (int)pfinfo->fsize, (int)pfinfo->tsize);
	BSP_LCD_DisplayStringAt(x, y, (uint8_t *)cbuff, LEFT_MODE);
}

/*
 *  オーディオ表示
 */
static void
SD_DispAudio(void)
{
	int  h = BSP_LCD_GetYSize() / 4;
	int  y = BSP_LCD_GetYSize() / 2;
	int  x, w, i;

	if((disp_status & (JPEG_APL<<MENU_SHIFT)) != 0)
		return;
	if(admode == 0)
		return;
	w = BSP_LCD_GetXSize() - h - 2;
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_FillRect(1, 1, BSP_LCD_GetXSize()-h-2, BSP_LCD_GetYSize()-2);
	BSP_LCD_SetTextColor(LCD_COLOR_RED);
	BSP_LCD_FillRect(40, 40, BSP_LCD_GetXSize()-h-80, BSP_LCD_GetYSize()-80);
	if((disp_status & (AUDIO_APL<<ACTSHIFT)) != 0){
		x = w / 2 - 120;
		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		BSP_LCD_FillRect(x, y, AUDIO_BOX, AUDIO_BOX);
		BSP_LCD_SetTextColor(LCD_COLOR_RED);
		BSP_LCD_FillRect(x+2, y+2, AUDIO_BOX-4, AUDIO_BOX-4);
		x = w / 2 - 20;
		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		if((pMinfo->status & AUDIO_PHASE) != 0){
			BSP_LCD_DrawVLine(x, y, AUDIO_BOX);
			BSP_LCD_DrawVLine(x+1, y, AUDIO_BOX);
			BSP_LCD_DrawLine(x, y, x+36, y+AUDIO_BOX/2+1);
			BSP_LCD_DrawLine(x, y+1, x+36, y+AUDIO_BOX/2);
			BSP_LCD_DrawLine(x, y+AUDIO_BOX-1, x+36, y+AUDIO_BOX/2);
			BSP_LCD_DrawLine(x, y+AUDIO_BOX-1, x+36, y+AUDIO_BOX/2+1);
		}
		else{
			BSP_LCD_FillRect(x, y, 16, AUDIO_BOX);
			BSP_LCD_SetTextColor(LCD_COLOR_RED);
			BSP_LCD_FillRect(x+2, y+2, 12, AUDIO_BOX-4);
			x = w / 2;
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
			BSP_LCD_FillRect(x, y, 16, AUDIO_BOX);
			BSP_LCD_SetTextColor(LCD_COLOR_RED);
			BSP_LCD_FillRect(x+2, y+2, 12, AUDIO_BOX-4);
		}
	}
	x = w / 2 + 40;
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_FillCircle(x, y+AUDIO_BOX/2, AUDIO_BOX/2-10);
	x += 20;
	for(i = 0 ; i < 100 ; i += 10, x += 7){
		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		BSP_LCD_FillRect(x, y, 6, AUDIO_BOX);
		if(advolume > i){
			BSP_LCD_SetTextColor(LCD_COLOR_RED);
			BSP_LCD_FillRect(x+1, y+1, 5, AUDIO_BOX-2);
		}
	}
	x += 17;
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_FillCircle(x, y+AUDIO_BOX/2, AUDIO_BOX/2-10);
	BSP_LCD_SetTextColor(LCD_COLOR_RED);
	BSP_LCD_FillCircle(x, y+AUDIO_BOX/2, AUDIO_BOX/4);
	admode = 0;
}

/*
 *  SD-card Playerメイン表示
 */
static void
sdcard_viewer(StorageDevice_t *psdev, FILE_INFO *pfinfo)
{
	uint16_t x, y, fno, exec = 0;
	uint16_t h = BSP_LCD_GetYSize() / 4;
	int upmode = 0;
	int sno = -1;

	sdmode = 1;
	do{
		SD_SetHint(pfinfo, sno);
		if((disp_status & (CLOCK_APL<<MENU_SHIFT)) != 0)
			SD_DispTime(sno, upmode);
		else if((disp_status & (MEDIA_APL<<MENU_SHIFT)) != 0 && sdmode == 1){
			SD_DispFile(pfinfo, -1);
			sdmode = 0;
			if((psdev->_sdev_attribute & SDEV_DEVERROR) == 0
				&& (psdev->_sdev_attribute & SDEV_EMPLOY) != 0){
				exec = 1;
			}
		}
		else if((disp_status & (AUDIO_APL<<MENU_SHIFT)) != 0)
			SD_DispAudio();
		upmode = 0;
		BSP_TS_GetState(&TS_State);
		if(TS_State.touchDetected){
			/* Get X and Y position of the touch post calibrated */
			x = TS_State.touchX[0];
			y = TS_State.touchY[0];
			syslog_3(LOG_NOTICE, "## detect(%d) x(%d) y(%d) ##", TS_State.touchDetected, x, y);
			if(sno >= 0){
				if(x < (BSP_LCD_GetXSize()-h) || y > h)
					sno = -1;
				else if(y < ((h/3)*2))
					upmode = 1;
				else
					upmode = -1;
			}
			else if(x > (BSP_LCD_GetXSize()-h)){
				if(y < h){
					disp_status &= ~((MEDIA_APL | AUDIO_APL)<<MENU_SHIFT);
					disp_status |= CLOCK_APL<<MENU_SHIFT;
				}
				else if(y < (h*2)){
					disp_status &= ~((CLOCK_APL | AUDIO_APL)<<MENU_SHIFT);
					disp_status |= MEDIA_APL<<MENU_SHIFT;
					int devno = root_dir[0] - '0';
					if(devno == 0 && (vol_status & 2) != 0){
						sdmode = 2;
						mode = 1;
						root_dir[0] = '1';
					}
					else if(devno == 1 && (vol_status & 1) != 0){
						sdmode = 2;
						mode = 1;
						root_dir[0] = '0';
					}
					else
						sdmode = 1;
				}
				else if(y < (h*3)){
					disp_status &= ~((CLOCK_APL | MEDIA_APL)<<MENU_SHIFT);
					disp_status |= AUDIO_APL<<MENU_SHIFT;
					admode = 1;
				}
				else if(exec != 0)
					mode = 2;
			}
			else if((disp_status & (MEDIA_APL<<MENU_SHIFT)) != 0 && y >= 8 && exec != 0){
				fno = (y - 8) / 15;
				if(fno < pfinfo->num_file){
					if((pfinfo->finfo[fno].d_dummy & FILE_EXE) != 0){
						SD_DispFile(pfinfo, fno);
						dly_tsk(1000);
						mode = 3+fno;
					}
				}
			}
			else if((disp_status & (AUDIO_APL<<MENU_SHIFT)) != 0 && y > (h*2)){
				if(x < ((BSP_LCD_GetXSize()-h)/2+20)){
					if((disp_status & (AUDIO_APL<<ACTSHIFT)) != 0){
						if(x < ((BSP_LCD_GetXSize()-h)/2-30))
							pMinfo->status |= AUDIO_STOP;
						else if(x < ((BSP_LCD_GetXSize()-h)/2+60)){
							if((pMinfo->status & AUDIO_PHASE) != 0){
								BSP_AUDIO_OUT_Resume(pMinfo->haudio);
								pMinfo->status &= ~AUDIO_PHASE;
							}
							else{
								BSP_AUDIO_OUT_Pause(pMinfo->haudio);
								pMinfo->status |= AUDIO_PHASE;
							}
						}
					}
				}
				else if(x < ((BSP_LCD_GetXSize()-h)/2+80)){
					if(advolume >= 10)
						advolume -= 10;
				}
				else{
					if(advolume <= 90)
						advolume += 10;
				}
				admode = 1;
			}
			else if((disp_status & (CLOCK_APL<<MENU_SHIFT)) != 0
					&& x < (BSP_LCD_GetXSize()-h) && y < (DATE_POSY+30)){
				if(y > DATE_POSY){
					if(x < (DATE_POSX+17*4))
						sno = 0;
					else if(x < (DATE_POSX+17*7))
						sno = 1;
					else if(x < (DATE_POSX+17*10))
						sno = 2;
				}
				else if(y > 80){
					if(x < ((BSP_LCD_GetXSize()-h)/2))
						sno = 3;
					else
						sno = 4;
				}
				else if(x >= (BSP_LCD_GetXSize()-h-100)){
					sno = 5;
				}
			}
		}
		dly_tsk(200);
	}while(mode == 0);
}

/*
 *  ファイル情報の取得
 */
static int_t
volume_dir(char *arg, FILE_INFO *pfinfo)
{
	StorageDevice_t *psdev;
	struct statfs2  status;
	char   *path, *spath, name[16], *p;
	void   *dir;

	name[0] = 0;
	spath = path = arg;
	pfinfo->devno = SDMGetDeviceNo((const char **)&path);
	pfinfo->num_file = 0;
	pfinfo->num_jpeg = 0;
	pfinfo->tsize = 0;
	pfinfo->fsize = 0;
	psdev = SDMGetStorageDevice(pfinfo->devno);
	if(psdev == 0){
		return -1;
	}
	if((psdev->_sdev_attribute & SDEV_DEVERROR) != 0
			|| (psdev->_sdev_attribute & SDEV_EMPLOY) == 0)
		return -1;
	if(psdev->pdevff == 0 || psdev->pdevff->_sdevff_opendir == 0){
		return -1;
	}
	dir = psdev->pdevff->_sdevff_opendir(spath);
	if(dir != NULL){
		if(*spath == 0){
			name[0] = pfinfo->devno + '0';
			name[1] = ':';
			name[2] = '/';
			name[3] = 0;
			spath = name;
		}
		while(psdev->pdevff->_sdevff_readdir(dir, &finfo) != 0 && finfo.d_name[0] != 0 && pfinfo->num_file < NUM_FILEMAX){
			BCOPY(&finfo, &pfinfo->finfo[pfinfo->num_file], sizeof(struct dirent2));
			pfinfo->finfo[pfinfo->num_file].d_dummy = FILE_ACT;
			p = pfinfo->finfo[pfinfo->num_file].d_name;
			if(_checkext(p, "JPG") != 0){
				pfinfo->finfo[pfinfo->num_file].d_dummy |= FILE_JPEG;
				pfinfo->num_jpeg++;
			}
			else if(_checkext(p, "WAV") != 0)
				pfinfo->finfo[pfinfo->num_file].d_dummy |= FILE_WAV;
			else if(_checkext(p, "MP3") != 0)
				pfinfo->finfo[pfinfo->num_file].d_dummy |= FILE_MP3;
			pfinfo->num_file++;
		}
		psdev->pdevff->_sdevff_closedir(dir);
		if(psdev->pdevff->_sdevff_statfs != 0
			 && psdev->pdevff->_sdevff_statfs(spath, &status) == 0){
			pfinfo->tsize = status.f_bsize;
			pfinfo->fsize = status.f_bfree;
		}
	}
	else{
		syslog_0(LOG_ERROR, "disk error !");
	}
	return 0;
}

static void
music_callback(MUSIC_TRANS *pmtrans)
{
	MUSIC_INFO *pminfo = pmtrans->minfo;
	ER         ercd;

	if(pmtrans->mcount == 0){
		ercd = AUDIO_OUT_Init(pminfo->haudio, AUDIO_OUT_DEVICE_HEADPHONE, advolume, pmtrans->samplerate, pmtrans->channels * 32);
		syslog_1(LOG_NOTICE, "## BSP_AUDIO_OUT_Init (%d) ##", ercd);
	}
	else if(pmtrans->mcount == 1){
		ercd = AUDIO_OUT_Play(pminfo->haudio, (uint16_t*)pminfo->buffer, pminfo->buffsize, pminfo->playmode);
		syslog_3(LOG_NOTICE, "## BSP_AUDIO_OUT_Play(2) (%d) buffer[%08x] size[%08x] ##", ercd, pminfo->buffer, pminfo->buffsize);
	}
}

void sw_handle(void)
{
	syslog_0(LOG_NOTICE, "## sw int ##");
}

#if _DRIVES > 1
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
#endif

/*
 *  メインタスク
 */
void main_task(intptr_t exinf)
{
	struct stat buf;
	StorageDevice_t *psdev;
#if _DRIVES > 1
	USB_OTG_Init_t USB_Data_Init;
	USB_OTG_Handle_t *phusb;
	USBH_HandleTypeDef *phusbh;
	TUSBH_ERCODE  result;
#endif
	ER ercd;
	int width, height, size, fno, no;
	int x, y, lcount, vcount, i;
	uint8_t status = 0;

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

	syslog_0(LOG_NOTICE, "### START LCD ###");
	BSP_LCD_Init();
	BSP_LCD_LayerDefaultInit(1, SDRAM_ADDRESS);
 	/* Set LCD Foreground Layer  */
	BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);
	SD_Banner();
	syslog_0(LOG_NOTICE, "### END LCD ###");

	psdev = SDMGetStorageDevice(0);
	psdev->_sdev_notice = storagedev_notice;
	SDeviceHead._get_datetime = get_fat_time;
	width  = BSP_LCD_GetXSize();
	height = BSP_LCD_GetYSize();
	pFinfo = (FILE_INFO *)malloc(sizeof(FILE_INFO));
	iaddr  = (uint8_t *)malloc(PPM_OUTSIZE+BMP_HEAD_SIZE);
	syslog_2(LOG_NOTICE, "## DISP finfo[%08x] iaddr[%08x] ##", pFinfo, iaddr);
	status = BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
	if(status != TS_OK){
		syslog_1(LOG_ERROR, "TS ERROR(%d) !", status);
 	}
	rtc_get_time(&time);
	if(time.tm_year == 30){
		time.tm_year = 17 + 30;
		dly_tsk(5);
		rtc_set_time(&time);
	}
#if _DRIVES > 1
    USB_Data_Init.usb_otg_mode = USB_OTG_MODE;
	USB_Data_Init.host_channels = 11;	/* HOST */
	USB_Data_Init.dev_endpoints = 4;		/* DEV */
	USB_Data_Init.use_dedicated_ep1 = 0;	/* DEV */
	USB_Data_Init.dma_enable = 1;
	USB_Data_Init.low_power_enable = 0;
	USB_Data_Init.phy_itface = USB_PHY_ULPI; 
	USB_Data_Init.sof_enable = 0;
	USB_Data_Init.speed = USB_SPEED_HIGH;
	USB_Data_Init.vbus_sensing_enable = 0;	/* HOST/DEV */
	USB_Data_Init.use_external_vbus = 1;
	phusb = usbo_init(USB2_PORTID, &USB_Data_Init);

	/*
	 *  USB HOSTミドルウェア設定
	 */
	phusbh = (USBH_HandleTypeDef *)malloc(sizeof(USBH_HandleTypeDef));
	phusbh->pSysData = phusb;
	tusbhInit(phusbh, USBH_UserProcess, 0);
	psdev = SDMGetStorageDevice(MSC_DEVNO);
	psdev->_sdev_local[1] = phusbh;
	psdev->_sdev_notice = storagedev_notice;

	/*
	 *  USB HOSTスタート
	 */
	result = tusbhStart(phusbh);
	syslog_1(LOG_NOTICE, "USB HOST START result(%d) !", result);
#endif

	pMinfo = AUDIO_Hard_Init(AUDIO2_PORTID);
	act_tsk(AUDIO_TASK);
	if(sdmode == 2){
		volume_dir(root_dir, pFinfo);
		sdmode = 1;
	}
	BSP_LCD_Clear(LCD_COLOR_BLACK);
	if(pFinfo->num_file == 0)
		disp_status = ALL_APL | (CLOCK_APL<<MENU_SHIFT);
	else
		disp_status = ALL_APL | (MEDIA_APL<<MENU_SHIFT);
	while(mode < (3+NUM_FILEMAX)){
		if(sdmode == 2){
			volume_dir(root_dir, pFinfo);
			sdmode = 1;
		}
		mode = 0;
		psdev = SDMGetStorageDevice(root_dir[0]-'0');
		sdcard_viewer(psdev, pFinfo);
		dly_tsk(200);
		if((psdev->_sdev_attribute & SDEV_DEVERROR) != 0)
			continue;
		if((psdev->_sdev_attribute & SDEV_EMPLOY) == 0)
			continue;
		if(mode < 2)
			continue;

		if(mode == 2){
			lcount = pFinfo->num_jpeg;
			vcount = 100;
			fno = 0;
		}
		else{
			lcount = MUSIC_COUNT;
			vcount = 200;
			fno = mode - 3;
		}
		while(lcount > 0){
			i = fno;
			fno++;
			if((pFinfo->finfo[i].d_dummy & (FILE_WAV|FILE_MP3)) != 0 && lcount == MUSIC_COUNT){
				lcount = 0;
				if((disp_status & (AUDIO_APL<<ACTSHIFT)) != 0)
					continue;
				strcpy(cbuff, root_dir);
				no = strlen(cbuff);
				strcpy(&cbuff[no], pFinfo->finfo[i].d_name);
				if(stat(cbuff, &buf) == 0 &&
					(pMinfo->fileid = fopen(cbuff, "rb")) != NULL) {
					pMinfo->datasize = buf.st_size;
					syslog_1(LOG_NOTICE, "## music data size[%08x] ##", pMinfo->datasize);
					if((pFinfo->finfo[i].d_dummy & FILE_WAV) != 0)
						set_flg(AUDIO_FLG, SET_WAV);
					else
						set_flg(AUDIO_FLG, SET_MP3);
				}
				continue;
			}
			else if((pFinfo->finfo[i].d_dummy & FILE_JPEG) == 0)
				continue;
			syslog_1(LOG_NOTICE, "## lcount(%d) ##", lcount);
			lcount--;
			disp_status |= (JPEG_APL<<MENU_SHIFT);
			strcpy(cbuff, root_dir);
			no = strlen(cbuff);
			strcpy(&cbuff[no], pFinfo->finfo[i].d_name);
			size = jpeg2ppm(cbuff, (uint8_t *)iaddr + BMP_HEAD_SIZE - P6_HEAD_SIZE, PPM_OUTSIZE);
			if(size == 0){
				syslog_0(LOG_ERROR, "convert jpeg to pmm error !");
				free(iaddr);
				disp_status &= ~(JPEG_APL<<MENU_SHIFT);
				continue;
			}

			syslog_1(LOG_NOTICE, "jpeg size[%08x]", size);
			size = ppm2rgb(iaddr, &width, &height);
			syslog_3(LOG_NOTICE, "%d %d [%08x]", width, height, size);
			BSP_LCD_Clear(LCD_COLOR_BLACK);
			x = (BSP_LCD_GetXSize() - width)/2;
			if(x < 0)
				x = 0;
			y = (BSP_LCD_GetYSize() - height)/2;
			if(y < 0)
				y = 0;
			BSP_LCD_DrawBitmap(x, y, (uint8_t *)iaddr);
			if(height > BSP_LCD_GetYSize()){
				iaddr[22] = BSP_LCD_GetYSize() & 0xff;
				iaddr[23] = BSP_LCD_GetYSize() >> 8;
			}
			for(i = 0 ; i < vcount ; i++){
				BSP_TS_GetState(&TS_State);
				if(TS_State.touchDetected){
					lcount = 0;
					break;
				}
				dly_tsk(100);
			}
			disp_status &= ~(JPEG_APL<<MENU_SHIFT);
			BSP_LCD_Clear(LCD_COLOR_BLACK);
			if(lcount == 0)
				dly_tsk(600);
			else
				dly_tsk(200);
		}
		syslog_0(LOG_NOTICE, "## end ##");
	}

	free(iaddr);
	syslog(LOG_NOTICE, "Sample program ends.");
	SVC_PERROR(ext_ker());
	assert(0);
}

/*
 *  AUDIOタスク
 */
void audio_task(intptr_t exinf)
{
	MUSIC_TRANS mtrans;
	FLGPTN   flgptn;

	for(;;){
		wai_flg(AUDIO_FLG, (SET_WAV | SET_MP3), TWF_ORW, &flgptn);
		disp_status |= (AUDIO_APL<<ACTSHIFT);
		mtrans.minfo = pMinfo;
		mtrans.func  = music_callback;
		if((flgptn & SET_WAV) != 0)
			wav_transfar(&mtrans);
		else
			mp3_decode(&mtrans);
		AUDIO_OUT_Stop(pMinfo->haudio, POWERDOWN_HW);
		fclose(pMinfo->fileid);
		disp_status &= ~(AUDIO_APL<<ACTSHIFT);
		admode = 1;
		syslog_0(LOG_NOTICE, "## STOP ##");
	}
	slp_tsk();
}

