/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2012 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
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
 *  $Id: lcdshield.c 2416 2016-02-12 12:05:32Z roi $
 */

/* 
 *  LCDシールド制御プログラムの本体
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <stdio.h>
#include <string.h>
#include <target_syssvc.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "kernel_cfg.h"
#include "device.h"
#include "spi.h"
#include "pinmode.h"
#include "adafruit_st7735.h"
#ifdef USE_MSC_FILE
#include "storagedevice.h"
#include "mscdiskio.h"
#endif
#include "lcdshield.h"
#include "adc.h"
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
#define ABS(X)  ((X) > 0 ? (X) : -(X))


/*
 *  使用する変数の定義
 */
static const Point testPolygon[7] = {
	{0, 0},
	{127, 159},
	{64, 0},
	{64, 159},
	{127, 0},
	{0, 159},
	{0, 0}
};

static const uint32_t pos_tbl[NUM_JSPOSITION-1][2] = {
	{  500, JS_DOWN  },
	{ 1000, JS_RIGHT },
	{ 1500, JS_ON    },
	{ 2500, JS_UP    },
	{ 3800, JS_LEFT  }
};
static const char *pos_str[NUM_JSPOSITION-1] = {
	"      ON      ",
	"      UP      ",
	"     DOWN     ",
	"     LEFT     ",
	"     RIGHT    "
};


LCD_Handler_t  LcdHandle;
LCD_DrawProp_t DrawProp;
uint8_t msc_state;

#define NUM_ADCBUF   1		/* 96:1ms */
volatile uint32_t uhADCxConvertedValue;

#ifndef USE_MSC_FILE
uint32_t sdbuffer[514/4];
#endif

/* Buffer used for transmission */
uint8_t aTxBuffer[BUFFERSIZE];

/* Buffer used for reception */
uint8_t aRxBuffer[BUFFERSIZE];

static uint32_t heap_area[2*1024];

uint32_t heap_param[2] = {
	(uint32_t)heap_area,
	(2*4*1024)
};

#ifdef USE_MSC_FILE
/*
 *  時刻の取出し
 */
static uint32_t get_fat_time(void)
{
	int    reqtime = 0;
	struct tm2 timedate;

	rtc_get_time(&timedate);
	reqtime  = ((timedate.tm_year-10)<<25)+(timedate.tm_mon<<21)+(timedate.tm_mday<<16);
	reqtime += (timedate.tm_hour<<11)+(timedate.tm_min<<5)+(timedate.tm_sec/2);
	return reqtime;
}
#endif

/*
 *  SW1割込み
 */
void sw_int(void)
{
	syslog_0(LOG_NOTICE, "## sw_int() ##");
}

/*
 *  GPIO設定関数
 */
static void
pinMode(uint8_t no, uint32_t mode)
{
	Arduino_PortControlBlock *pgcb = getGpioTable(DIGITAL_PIN, no);
	GPIO_Init_t GPIO_Init_Data;

	if(pgcb == NULL)
		return;
	pinClock(no);
	GPIO_Init_Data.mode  = mode;
	GPIO_Init_Data.pull  = GPIO_PULLMODE;
	GPIO_Init_Data.otype = GPIO_OTYPE;
	GPIO_Init_Data.speed = GPIO_SPEED_FAST;
	gpio_setup(pgcb->giobase, &GPIO_Init_Data, pgcb->giopin);
}

/*
 *  文字描画
 *  param1  x0: Line where to display the character shape
 *  param2  y0: Start column address
 *  param3  c: Pointer to the character data
 */
static void
DrawChar(uint16_t x0, uint16_t y0, const uint8_t *c)
{
	LCD_Handler_t *hlcd = DrawProp.hlcd;
	sFONT    *pFont = DrawProp.pFont;
	uint32_t i = 0, j = 0;
	uint16_t height, width;
	uint8_t  offset;
	uint8_t  *pchar;
	uint32_t line;

	height = pFont->Height;
	width  = pFont->Width;
	offset =  8 *((width + 7)/8) -  width ;
	lcd_setAddrWindow(hlcd, x0, y0, x0+width-1, y0+height-1);

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

		for(j = 0; j < width; j++){
			if(line & (1 << (width- j + offset- 1))){
				lcd_writedata2(hlcd, DrawProp.TextColor, 1);
			}
			else{
				lcd_writedata2(hlcd, DrawProp.BackColor, 1);
			}
		}
	}
}

/*
 *  一文字描画
 *  param1  x0: Start column address
 *  param2  y0: Line where to display the character shape.
 *  param3  Ascii: Character ascii code
 *           This parameter must be a number between Min_Data = 0x20 and Max_Data = 0x7E 
 */
void
lcd_DisplayChar(uint16_t x0, uint16_t y0, uint8_t Ascii)
{
	sFONT    *pFont = DrawProp.pFont;
	DrawChar(x0, y0, &pFont->table[(Ascii-' ') * pFont->Height * ((pFont->Width + 7) / 8)]);
}

/*
 *  文字列描画
 *  param1  x0: X position (in pixel)
 *  param2  y0: Y position (in pixel)   
 *  param3  Text: Pointer to string to display on LCD
 *  param4  Mode: Display mode
 */
void
lcd_DisplayStringAt(uint16_t x0, uint16_t y0, char *Text, Text_AlignModeTypdef Mode)
{
	sFONT    *pFont = DrawProp.pFont;
	uint16_t ref_column = 1, i = 0;
	uint32_t size = 0, xsize = 0; 
	uint8_t  *ptr = (uint8_t *)Text;

	/* Get the text size */
	while (*ptr++) size ++ ;

	/* Characters number per line */
	xsize = (DrawProp.hlcd->_width/pFont->Width);

	switch (Mode){
	case CENTER_MODE:
		ref_column = x0 + ((xsize - size)* pFont->Width) / 2;
		break;
	case LEFT_MODE:
		ref_column = x0;
		break;
	case RIGHT_MODE:
		ref_column = - x0 + ((xsize - size)*pFont->Width);
		break;
	default:
		ref_column = x0;
		break;
	}

	/* Check that the Start column is located in the screen */
	if ((ref_column < 1) || (ref_column >= 0x8000)){
		ref_column = 1;
	}

	/* Send the string character by character on LCD */
	while ((*Text != 0) & (((DrawProp.hlcd->_width - (i*pFont->Width)) & 0xFFFF) >= pFont->Width)){
		/* Display one character on LCD */
		lcd_DisplayChar(ref_column, y0, *Text);
		/* Decrement the column position by 16 */
		ref_column += pFont->Width;
		/* Point on the next character */
		Text++;
		i++;
	}
}

/*
 *  グラフィック表示テスト
 */
void
grapics_test(LCD_Handler_t *hlcd)
{
	uint16_t x, y;
	int      i;

	DrawProp.BackColor = ST7735_BLACK;
	lcd_fillScreen(&DrawProp);
	dly_tsk(1000);
	lcd_drawPixel(hlcd, hlcd->_width/2, hlcd->_height/2, ST7735_GREEN);
	dly_tsk(1000);

	lcd_fillScreen(&DrawProp);
	for(y=0; y < hlcd->_height; y+=5) {
		lcd_drawFastHLine(hlcd, 0, y, hlcd->_width, ST7735_RED);
	}
	for (x=0; x < hlcd->_width; x+=5) {
    	lcd_drawFastVLine(hlcd, x, 0, hlcd->_height, ST7735_BLUE);
	}
	dly_tsk(1000);

	lcd_fillScreen(&DrawProp);
	DrawProp.TextColor = ST7735_GREEN;
	lcd_drawPolygon(&DrawProp, (Point *)testPolygon, sizeof(testPolygon)/sizeof(Point));
	dly_tsk(1000);

	lcd_fillScreen(&DrawProp);
	DrawProp.TextColor = ST7735_GREEN;
	for (x=0; x < hlcd->_width; x+=6) {
		lcd_drawRect(&DrawProp, hlcd->_width/2 -x/2, hlcd->_height/2 -x/2 , x, x);
	}
	for(i = 0 ; i < 3 ; i++){
		lcd_invertDisplay(hlcd, true);
		dly_tsk(500);
		lcd_invertDisplay(hlcd, false);
		dly_tsk(500);
	}
	lcd_fillScreen(&DrawProp);
	lcd_drawBitmap(hlcd, 33, 48, (uint8_t *)topamelogo);
	dly_tsk(1000);
}

/*
 *  USB HOST INFORMATION CALLBACK
 *  parameter1:  Host Handler
 *  parameter2:  Device Handler
 *  parameter3:  Host state ID.
 */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, TUSBH_Device_t *pdevice, uint8_t id)
{
	msc_state = id;
	if(pdevice->pClass != NULL){
		syslog_3(LOG_NOTICE, "## CLASS[%s](%d)(%d) ##", pdevice->pClass->Name, pdevice->type, id);
	}
}

/*
 *  メインタスク
 */
void
main_task(intptr_t exinf)
{
#ifdef USE_MSC_FILE
	StorageDevice_t *psdev;
#else
	TUSBH_ERCODE  status = TUSBH_E_OK;
	char pdrv = 0;
#endif
	SPI_Init_t spi_initd;
	USB_OTG_Init_t USB_Data_Init;
	LCD_Handler_t   *hlcd;
	SPI_Handle_t  *hspi;
	USB_OTG_Handle_t *husb;
	USBH_HandleTypeDef *husbh;
    FLGPTN  flgptn;
	ER_UINT	ercd;
	int sens = 1000;
	int i;

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

	syslog_0(LOG_NOTICE, "## START ##");
	/*
	 *  CS PORT(D10-PB6)
	 */
	pinMode(CS_PORT, GPIO_MODE_OUTPUT);
	/*
	 *  RS PORT(D8-PA9)
	 */
	pinMode(RS_PORT, GPIO_MODE_OUTPUT);
	/*
	 *  RST PORT(D9-PC7)
	 */
	pinMode(RST_PORT, GPIO_MODE_OUTPUT);

	spi_initd.Prescaler     = SPI_BAUDRATEPRESCALER_64;
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
	spi_initd.semid = SPI1DMA_SEM;
	spi_initd.semlock = 0;
	if((hspi = spi_init(SPI1_PORTID, &spi_initd)) == NULL){
		syslog_0(LOG_ERROR, "spi_start initialize error !");
	}

	/* Init USB OTG Hard */
	/* Set the LL Driver parameters */
    USB_Data_Init.usb_otg_mode = USB_OTG_MODE_HOST;
	USB_Data_Init.host_channels = 11;	/* HOST */
	USB_Data_Init.dma_enable = 0;
	USB_Data_Init.low_power_enable = 0;
	USB_Data_Init.phy_itface = USB_PHY_EMBEDDED; 
	USB_Data_Init.sof_enable = 0;
	USB_Data_Init.speed = USB_SPEED_FULL;
	USB_Data_Init.vbus_sensing_enable = 0;	/* HOST */
	USB_Data_Init.lpm_enable = 0;
	USB_Data_Init.use_external_vbus = 0;
	husb = usbo_init(USB1_PORTID, &USB_Data_Init);

	/*
	 *  USB HOSTミドルウェア設定
	 */
	husbh = (USBH_HandleTypeDef *)malloc(sizeof(USBH_HandleTypeDef));
	husbh->pSysData = husb;
	tusbhInit(husbh, USBH_UserProcess, 0);
#ifdef USE_MSC_FILE
	psdev = SDMGetStorageDevice(MSC_DEVNO);
	psdev->_sdev_local[1] = husbh;
	SDeviceHead._get_datetime = get_fat_time;
#endif

	/*
	 *  USB HOSTスタート
	 */
	tusbhStart(husbh);

	i = 0;
	while(msc_state != HOST_USER_CLASS_ACTIVE && i < 15){
		syslog_2(LOG_NOTICE, " WAIT MSC READY(%d)(%d) !", msc_state, i);
		dly_tsk(1000);
		i++;
	}
	if(msc_state != HOST_USER_CLASS_ACTIVE)
		syslog_0(LOG_NOTICE, "MSC INITIAL ERROR !");

#ifndef USE_MSC_FILE
	if(msc_state == HOST_USER_CLASS_ACTIVE && tusbhMscUnitIsReady(husbh, pdrv)){
		unsigned char *p;
		int j, bpb;
		int SecPerCls, Rsv, NumFat, Fsize, RootDirC, RootSec, tsect;
		p = (unsigned char *)sdbuffer;
		for(i = 0 ; i < 1 ; i++){
			status = tusbhMscRead(husbh, pdrv, i, (uint8_t *)sdbuffer, 1);
			syslog_3(LOG_NOTICE, "## status(%d) i(%d) sdbuffer[%08x] ##", status, i, sdbuffer);
			dly_tsk(300);
			if(i == 0){
				if(p[0x36] == 'F' && p[0x37] == 'A' && p[0x38] == 'T')
					bpb = 0;
				else{
					bpb = p[0x1c6];
					bpb |= p[0x1c7]<<8;
					bpb |= p[0x1c8]<<16;
					bpb |= p[0x1c9]<<24;
				}
			}
			printf("\nsec(%d) ", i);
			for(j = 0 ; j < 512 ; j++){
				if((j % 16) == 0)
					printf("\n%03x ", j);
				printf("%02x ", p[j]);
			}
			printf("\n");
		}
		syslog_1(LOG_NOTICE, "## bpb(%08x) ##", bpb);
		if(bpb != 0){
			status = tusbhMscRead(husbh, pdrv, bpb, (uint8_t *)sdbuffer, 1);
			syslog_3(LOG_NOTICE, "## status(%d) i(%d) sdbuffer[%08x] ##", status, bpb, sdbuffer);
			dly_tsk(300);
			printf("\nsec(%d) ", bpb);
			for(j = 0 ; j < 512 ; j++){
				if((j % 16) == 0)
					printf("\n%03x ", j);
				printf("%02x ", p[j]);
			}
			printf("\n");
		}
		SecPerCls = p[13];
		Rsv = p[14] + (p[15] << 8);
		NumFat = p[16];
		RootDirC = p[17] + (p[18]<<8);
		Fsize = p[22] + (p[23] << 8);
		if(Fsize == 0){
			Fsize = p[36] + (p[37] << 8);
			RootSec = p[44] + (p[45] << 8) + (p[46] << 16) + (p[47] << 24);
		}
		else
			RootSec = bpb + Rsv + (NumFat * Fsize);
		tsect = p[32] + (p[33] << 8) + (p[34] << 16) + (p[35] << 24);
		if(tsect == 0)
			tsect = p[19] + (p[20] << 8) + (p[21] << 16) + (p[22] << 24);
		syslog_4(LOG_NOTICE, "## SecPerCls(%d) Rsv(%d) NumFat(%d) RootDirC(%d) ##", SecPerCls, Rsv, NumFat, RootDirC);
		syslog_3(LOG_NOTICE, "## Fsize(%d) RootSec(%d) totalsec(%d) ##", Fsize, RootSec, tsect);

		for(i = 0 ; i < 2 ; i++){
			status = tusbhMscRead(husbh, pdrv, (i+RootSec), (uint8_t *)sdbuffer, 1);
			syslog_3(LOG_NOTICE, "## status(%d) i(%d) sdbuffer[%08x] ##", status, (i+RootSec), sdbuffer);
			dly_tsk(300);
			p = (unsigned char *)sdbuffer;
			printf("\nsec(%d) ", (i+RootSec));
			for(j = 0 ; j < 512 ; j++){
				if((j % 16) == 0)
					printf("\n%03x ", j);
				printf("%02x ", p[j]);
			}
			printf("\n");
		}
	}
#endif
	hlcd = &LcdHandle;
	hlcd->hspi = hspi;
	hlcd->spi_lock= 0;
	hlcd->cs_pin  = CS_PORT;
	hlcd->rs_pin  = RS_PORT;
	hlcd->rst_pin = RST_PORT;
	hlcd->cs2_pin = CS2_PORT;
	DrawProp.hlcd = hlcd;
	lcd_initR(hlcd, INITR_BLACKTAB);
	DrawProp.BackColor = ST7735_BLACK;
	DrawProp.TextColor = ST7735_WHITE;
	grapics_test(hlcd);
#if 1	/* LOCAL DEFINE */
	DrawProp.TextColor = ST7735_BLACK;
	DrawProp.BackColor = ST7735_WHITE;
	lcd_fillScreen(&DrawProp);
	lcd_fillRect(hlcd, 0, 0, hlcd->_width, hlcd->_height/3, ST7735_BLUE);
	DrawProp.TextColor = ST7735_WHITE;
	DrawProp.BackColor = ST7735_BLUE;
	DrawProp.pFont = &Font20;
	lcd_DisplayStringAt(0, 20, "TBP TEST", CENTER_MODE);

	DrawProp.TextColor = ST7735_BLACK;
	DrawProp.BackColor = ST7735_WHITE;
	DrawProp.pFont = &Font8;
	lcd_DisplayStringAt(0, 80, "Helle World !", CENTER_MODE);
	DrawProp.pFont = &Font12;
	lcd_DisplayStringAt(0, 100, "Helle World !", CENTER_MODE);
	DrawProp.pFont = &Font16;
	lcd_DisplayStringAt(0, 120, "Helle World !", CENTER_MODE);

	DrawProp.TextColor = ST7735_RED;
	for(i = 20 ; i < 40 ; i++){
		lcd_DrawCircle(&DrawProp, hlcd->_width/2, hlcd->_height/2+30, i);
		if(i == 25)
			i += 10;
	}
#endif	/* LOCAL DEFINE */
	syslog_0(LOG_NOTICE, "LCD TEST END");
	while(sens > 0){
		wai_flg(SW_FLG, JS_MASK, TWF_ORW, &flgptn);
		syslog_1(LOG_NOTICE, "## JS EVENT[%04x] ##", flgptn);
		for(i = 0 ; i < NUM_JSPOSITION ; i++){
			if(flgptn & (1<<i))
				break;
		}
		if(i == NUM_JSPOSITION)
			i = 0;
#if 1	/* LOCAL DEFINE */
		DrawProp.pFont = &Font12;
		if(i == 0 || i == NUM_JSPOSITION)
			lcd_DisplayStringAt(0, 100, "Helle World !", CENTER_MODE);
		else
			lcd_DisplayStringAt(0, 100, (char *)pos_str[i-1], CENTER_MODE);
#else	/* LOCAL DEFINE */
		syslog_1(LOG_NOTICE, "## event(%d) ##", i);
#endif	/* LOCAL DEFINE */
		sens--;
	}
#ifdef USE_MSC_FILE
	syslog_0(LOG_NOTICE, "## STOP ##");
	slp_tsk();
#endif
	spi_deinit(hspi);

	syslog(LOG_NOTICE, "Sample program ends.");
//	SVC_PERROR(ext_ker());
}


/*
 *  ADC転送終了コールバック関数
 */
static void HAL_ADC_ConvCpltCallback2(ADC_Handle_t* hadc)
{
	isig_sem(ADCDMA_SEM);
}

/*
 *  ジョイスティック監視タスク
 */
void stick_task(intptr_t exinf)
{
	ADC_Init_t     aInit;
	ADC_ChannelConf_t sConfig;
	ADC_Handle_t *hadc;
	uint32_t event = 0;
	int i;

	/*
	 *  ADC3初期化
	 */
	aInit.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
	aInit.Resolution = ADC_RESOLUTION_12B;
	aInit.ScanConvMode = ADC_SCANMODE_DISABLE;
//	aInit.ContinuousConvMode = ADC_CONTINUOUS_ENABLE;
	aInit.ContinuousConvMode = ADC_CONTINUOUS_DISABLE;
	aInit.DiscontinuousConvMode = ADC_DISCONTINUOUS_DISABLE;
	aInit.NumDiscConversion = 0;
	aInit.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	aInit.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;
	aInit.DataAlign = ADC_DATAALIGN_RIGHT;
	aInit.NumConversion = 1;
	aInit.DMAContinuousRequests = ADC_DMACONTINUOUS_ENABLE;
	aInit.EOCSelection = ADC_EOC_SEQ_DISABLE;

	if((hadc = adc_init(ADC3_PORTID, &aInit)) == NULL){
		/* Initialization Error */
		syslog_0(LOG_NOTICE, "## adc_init ERROR ##");
	}
	hadc->xfercallback = HAL_ADC_ConvCpltCallback2;

	/*
	 *  ADCチャネル設定
	 */
	sConfig.Channel = ADCx_CHANNEL;
	sConfig.Rank = 1;
//  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	sConfig.GpioBase = ADCx_CHANNEL_GPIO_PORT;
	sConfig.GpioPin  = ADCx_CHANNEL_PINNO;
	if(adc_setupchannel(hadc, &sConfig) != E_OK){
		/* Channel Configuration Error */
		syslog_0(LOG_NOTICE, "## adc_setupchannel ERROR ##");
	}

	for(;;){
		uhADCxConvertedValue = 0;
		/*
		 *  ADC DMAスタート
		 */
		if(adc_start_dma(hadc, (uint32_t*)&uhADCxConvertedValue, 1) != E_OK){
			/* Start Conversation Error */
			syslog_0(LOG_NOTICE, "## adc_start_dma ERROR ##");
		}
		while(hadc->status == ADC_STATUS_BUSY){
 			twai_sem(ADCDMA_SEM, 10);
		}
		adc_end_dma(hadc);
		for(i = 0 ; i < (NUM_JSPOSITION-1) ; i++){
			if(uhADCxConvertedValue < pos_tbl[i][0]){
				event = pos_tbl[i][1];
				set_flg(SW_FLG, event);
				break;
			}
		}
		if(i == 5 && event != JS_OFF){
			event = JS_OFF;
			set_flg(SW_FLG, JS_OFF);
		}
  		dly_tsk(50);
	}
	syslog_2(LOG_NOTICE, "## STOP[%08x] (%d) ##", &uhADCxConvertedValue, hadc->status);
	slp_tsk();
}

