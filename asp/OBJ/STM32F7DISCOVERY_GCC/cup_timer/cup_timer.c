/*
 * カップラーメンタイマ
 */
#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "kernel_cfg.h"
#include "device.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "stdio.h"
#include "cup_timer.h"
#include "topamelogo.h"

/*
 *  サービスコールのエラーのログ出力
 */
void
svc_perror(const char *file, int_t line, const char *expr, ER ercd)
{
	if (ercd < 0) {
		t_perror(LOG_ERROR, file, line, expr, ercd);
	}
}

#define	SVC_PERROR(expr)	svc_perror(__FILE__, __LINE__, #expr, (expr))

/*
 * タイマータスク用のフラグのビット割り当て
 */ 
#define TIMER_START     0x01
#define TIMER_STOP      0x02
#define TIMER_UPDATE    0x04
#define TIMER_BASE_TIME 0x08
#define TIMER_ALL       0x0f

/*
 * ブリンクタスク用のフラグのビット割り当て
 */ 
#define BLINK_ACTIVE  0x01
#define BLINK_TIMEOUT 0x02
#define BLINK_OFF     0x04
#define BLINK_ALL     0x07

#define ITEM_YEAR     0
#define ITEM_MONTH    1
#define ITEM_DATE     2
#define ITEM_HOURS    3
#define ITEM_MINS     4
#define ITEM_SECS     5
#define NUM_ITEM      6

static const char monthday[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static const char *item_table[6] = {
	" year   ",
	" month  ",
	" date   ",
	" hours  ",
	"mintutes",
	"seconds "
};

volatile unsigned int BaseTime;
int    timer, start_time;


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define  CIRCLE_RADIUS        30
/* Private macro -------------------------------------------------------------*/
#define  CIRCLE_XPOS(i)       ((i * BSP_LCD_GetXSize()) / 5)
#define  CIRCLE_YPOS(i)       (BSP_LCD_GetYSize() - CIRCLE_RADIUS - 60 + 40)
/* Private variables ---------------------------------------------------------*/
#define  TIMER_Y              70
static TS_StateTypeDef  TS_State;

static void set_clock(struct tm2 *ptm, int item, int next);
static void set_time(char *buf, long time, char sep);

/**
  * @brief  Display CupRamenTimer Hint
  * @param  None
  * @retval None
  */
static void CupRamenTimer_SetHint(void)
{

	/* Set LCD Foreground Layer  */
	BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);
	BSP_LCD_SetFont(&LCD_DEFAULT_FONT);

  /* Clear the LCD */
  BSP_LCD_Clear(LCD_COLOR_WHITE);


  /* Set CupRamenTimer Demo description */
  BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
  BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), 80-30);
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
  BSP_LCD_SetFont(&Font24);
  BSP_LCD_DisplayStringAt(0, 0, (uint8_t *)"CUP Ramen Timer", CENTER_MODE);
  BSP_LCD_SetFont(&Font12);
  BSP_LCD_DisplayStringAt(0, 30, (uint8_t *)"TOPPERS PROJECT Educational Working Group", CENTER_MODE);

  /* Set the LCD Text Color */
  BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
  BSP_LCD_DrawRect(10, 90-30, BSP_LCD_GetXSize() - 20, BSP_LCD_GetYSize() - 100+30);
  BSP_LCD_DrawRect(11, 91-30, BSP_LCD_GetXSize() - 22, BSP_LCD_GetYSize() - 102+30);
  BSP_LCD_DrawBitmap(BSP_LCD_GetXSize() - 100, 100, (uint8_t *)topamelogo);
}


/**
  * @brief  Draw CupRamenTimer Background
  * @param  state : touch zone state
  * @retval None
  */
static void CupRamenTimer_DrawBackground (uint8_t state)
{

  switch (state)
  {

	case 16:
    case 0:
      BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
      BSP_LCD_FillCircle(CIRCLE_XPOS(1), CIRCLE_YPOS(1), CIRCLE_RADIUS);

      BSP_LCD_SetTextColor(LCD_COLOR_RED);
      BSP_LCD_FillCircle(CIRCLE_XPOS(2), CIRCLE_YPOS(2), CIRCLE_RADIUS);

      BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
      BSP_LCD_FillEllipse(CIRCLE_XPOS(3), CIRCLE_YPOS(3), CIRCLE_RADIUS, CIRCLE_RADIUS);

      BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
      BSP_LCD_FillEllipse(CIRCLE_XPOS(4), CIRCLE_YPOS(3), CIRCLE_RADIUS, CIRCLE_RADIUS);
      if(state == 16)
		dly_tsk(200);

      BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
      BSP_LCD_FillCircle(CIRCLE_XPOS(1), CIRCLE_YPOS(1), CIRCLE_RADIUS - 2);
      BSP_LCD_FillCircle(CIRCLE_XPOS(2), CIRCLE_YPOS(2), CIRCLE_RADIUS - 2);
      BSP_LCD_FillEllipse(CIRCLE_XPOS(3), CIRCLE_YPOS(3), CIRCLE_RADIUS - 2, CIRCLE_RADIUS - 2);
      BSP_LCD_FillEllipse(CIRCLE_XPOS(4), CIRCLE_YPOS(3), CIRCLE_RADIUS - 2, CIRCLE_RADIUS - 2);
      if(state == 16)
		dly_tsk(200);
      break;

    case 1:
      BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
      BSP_LCD_FillCircle(CIRCLE_XPOS(1), CIRCLE_YPOS(1), CIRCLE_RADIUS);
      BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
      BSP_LCD_FillCircle(CIRCLE_XPOS(1), CIRCLE_YPOS(1), CIRCLE_RADIUS - 2);
      break;

    case 2:
      BSP_LCD_SetTextColor(LCD_COLOR_RED);
      BSP_LCD_FillCircle(CIRCLE_XPOS(2), CIRCLE_YPOS(2), CIRCLE_RADIUS);
      BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
      BSP_LCD_FillCircle(CIRCLE_XPOS(2), CIRCLE_YPOS(2), CIRCLE_RADIUS - 2);
      break;

    case 4:
      BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
      BSP_LCD_FillCircle(CIRCLE_XPOS(3), CIRCLE_YPOS(3), CIRCLE_RADIUS);
      BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
      BSP_LCD_FillCircle(CIRCLE_XPOS(3), CIRCLE_YPOS(3), CIRCLE_RADIUS - 2);
      break;

    case 8:
      BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
      BSP_LCD_FillCircle(CIRCLE_XPOS(4), CIRCLE_YPOS(4), CIRCLE_RADIUS);
      BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
      BSP_LCD_FillCircle(CIRCLE_XPOS(4), CIRCLE_YPOS(4), CIRCLE_RADIUS - 2);
      break;

  }
}

/**
  * @brief  CupRamenTimer Demo
  * @param  None
  * @retval None
  */
uint8_t CupRamenTimer_demo (void)
{
  uint8_t  status = 0;
  uint8_t  state = 0;

  CupRamenTimer_SetHint();

  status = BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());

  if (status != TS_OK)
  {
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 95, (uint8_t *)"ERROR", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 80, (uint8_t *)"CupRamenTimer cannot be initialized", CENTER_MODE);
  }
  else
  {
    CupRamenTimer_DrawBackground(state);
  }
  return status;
}

/*
 * SWコールバック
 */ 
void
sw_handle(void)
{
	if(timer > 0) {
      SVC_PERROR(iset_flg(FLG_TIMER, TIMER_STOP));
	}
	else {
      SVC_PERROR(iset_flg(FLG_TIMER, TIMER_START));
	}
}

/*
 * タイマータスク
 */ 
void
timer_task(intptr_t exinf)
{
    FLGPTN flgptn;

    BaseTime = 0;
	sta_cyc(BASE_TIME_HANDLER);
    for (;;) {
        timer = 0;
        do {
	        set_led_state(LED03, OFF);
            wai_flg(FLG_TIMER, TIMER_ALL, TWF_ORW, &flgptn);
            if ((flgptn & TIMER_START) != 0) {
				start_time = BaseTime;
                timer = start_time + INIT_TIME*TO_SEC;
                set_flg(FLG_BLINK, BLINK_ACTIVE);
                syslog_0(LOG_NOTICE, "Timer start!");
            }
            if ((flgptn & TIMER_STOP) != 0) {
                set_flg(FLG_BLINK, BLINK_OFF);
                syslog_0(LOG_NOTICE, "Timer stop!");
            }
        } while (timer == 0);

        while (timer > 0) {
	        set_led_state(LED03, ON);
            wai_flg(FLG_TIMER, TIMER_ALL, TWF_ORW, &flgptn);
            if ((flgptn & TIMER_STOP) != 0) {
                set_flg(FLG_BLINK, BLINK_OFF);
                timer = 0;
                syslog_0(LOG_NOTICE, "Timer stop!");
            }
            else {
                if ((flgptn & TIMER_UPDATE) != 0) {
                    timer = timer + EXTRA_UNIT*TO_SEC;
                    syslog_2(LOG_NOTICE, "Extend %d sec. Remaining  %d sec",
                             EXTRA_UNIT, (timer-BaseTime)/TO_SEC);
                }
                if ((flgptn & TIMER_BASE_TIME) != 0) {
                    if (BaseTime > timer) {
                        set_flg(FLG_BLINK, BLINK_TIMEOUT);
                        syslog_0(LOG_NOTICE, "Timer timeout");
						timer = 0;
                    }
                }
            }
        }
    }
}


/*
 *  LED4点滅タスク
 */
void
blink_task(intptr_t exinf)
{
	static unsigned int save_time = 0xffffffff;
	static struct tm2 time;
	unsigned int current_time;
	unsigned int ttime;
    int status;
    FLGPTN flgptn;
	ER result;
	uint16_t x, y;
	uint8_t  state = 0;
	uint8_t  item  = 0;
	char     text[30];
	int      next = 0;
	uint8_t  radius;
	uint8_t  radius_previous = 0;

	BSP_LCD_Init();
	BSP_LCD_LayerDefaultInit(1, SDRAM_ADDRESS);

	status = CupRamenTimer_demo();
	if(status != TS_OK){
		syslog_1(LOG_ERROR, "Touch screen ERROR(%d) !", status);
		dly_tsk(20000000);
	}
	rtc_get_time(&time);
	if(time.tm_year == 30){
		time.tm_year = 16 + 30;
		dly_tsk(5);
		rtc_set_time(&time);
	}

	while (1){
		/* Check in polling mode in touch screen the touch status and coordinates */
		/* if touch occurred                                                      */
		current_time = BaseTime/TO_SEC;
		if(current_time != save_time){
		    SVC_PERROR(set_flg(FLG_TIMER, TIMER_BASE_TIME));
			BSP_LCD_SetFont(&Font24);
			BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
			BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
			if(timer == 0){
				rtc_get_time(&time);
				if(next != 0){
					set_clock(&time, item, next);
					next = 0;
				}
				BSP_LCD_DisplayStringAt(0, TIMER_Y, (uint8_t *)" READY  ", CENTER_MODE);
				ttime = (time.tm_year - 30) * 3600 + time.tm_mon * 60 + time.tm_mday;
				set_time(text, ttime, '/');
				BSP_LCD_DisplayStringAt(0, TIMER_Y+30, (uint8_t *)text, CENTER_MODE);
				ttime = time.tm_sec + time.tm_min * 60 + time.tm_hour * 60 * 60;
				set_time(text, ttime, ':');
				BSP_LCD_DisplayStringAt(0, TIMER_Y+60, (uint8_t *)text, CENTER_MODE);
				BSP_LCD_SetFont(&Font12);
				BSP_LCD_DisplayStringAt(CIRCLE_XPOS(1)-18, CIRCLE_YPOS(1)-CIRCLE_RADIUS-15, (uint8_t *)"start", LEFT_MODE);
				BSP_LCD_DisplayStringAt(CIRCLE_XPOS(2)-20, CIRCLE_YPOS(2)-CIRCLE_RADIUS-15, (uint8_t *)item_table[item], LEFT_MODE);
				BSP_LCD_DisplayStringAt(CIRCLE_XPOS(3)-16, CIRCLE_YPOS(2)-CIRCLE_RADIUS-15, (uint8_t *)" up ", LEFT_MODE);
				BSP_LCD_DisplayStringAt(CIRCLE_XPOS(4)-16, CIRCLE_YPOS(2)-CIRCLE_RADIUS-15, (uint8_t *)"down", LEFT_MODE);
			}
			else{
				BSP_LCD_DisplayStringAt(0, TIMER_Y, (uint8_t *)"RUNNING ", CENTER_MODE);
				set_time(text, (BaseTime - start_time)/TO_SEC, ':');
				BSP_LCD_DisplayStringAt(0, TIMER_Y+30, (uint8_t *)text, CENTER_MODE);
				set_time(text, (timer - start_time)/TO_SEC, ':');
				BSP_LCD_DisplayStringAt(0, TIMER_Y+60, (uint8_t *)text, CENTER_MODE);
				BSP_LCD_SetFont(&Font12);
				BSP_LCD_DisplayStringAt(CIRCLE_XPOS(1)-18, CIRCLE_YPOS(1)-CIRCLE_RADIUS-15, (uint8_t *)"stop ", LEFT_MODE);
				BSP_LCD_DisplayStringAt(CIRCLE_XPOS(2)-20, CIRCLE_YPOS(2)-CIRCLE_RADIUS-15, (uint8_t *)"        ", LEFT_MODE);
				BSP_LCD_DisplayStringAt(CIRCLE_XPOS(3)-16, CIRCLE_YPOS(2)-CIRCLE_RADIUS-15, (uint8_t *)" up ", LEFT_MODE);
				BSP_LCD_DisplayStringAt(CIRCLE_XPOS(4)-16, CIRCLE_YPOS(2)-CIRCLE_RADIUS-15, (uint8_t *)"    ", LEFT_MODE);
			}
			save_time = current_time;
		}
		BSP_TS_GetState(&TS_State);
		if(TS_State.touchDetected){
			/* Get X and Y position of the touch post calibrated */
			x = TS_State.touchX[0];
			y = TS_State.touchY[0];

			syslog_1(LOG_NOTICE, "Nb touch detected = %d", TS_State.touchDetected);
			/* Display 1st touch detected coordinates */
			syslog_2(LOG_NOTICE, "1[%d,%d]    ", x, y);
			if (TS_State.touchDetected >= 2){  /* Display 2nd touch detected coordinates if applicable */
				syslog_2(LOG_NOTICE, "2[%d,%d]    ", TS_State.touchX[1], TS_State.touchY[1]);
			}
			if (TS_State.touchDetected >= 3){  /* Display 3rd touch detected coordinates if applicable */
				syslog_2(LOG_NOTICE, "3[%d,%d]    ", TS_State.touchX[2], TS_State.touchY[2]);
			}
			if (TS_State.touchDetected >= 4){  /* Display 4th touch detected coordinates if applicable */
				syslog_2(LOG_NOTICE, "4[%d,%d]    ", TS_State.touchX[3], TS_State.touchY[3]);
			}
			if (TS_State.touchDetected >= 5){  /* Display 5th touch detected coordinates if applicable */
				syslog_2(LOG_NOTICE, "5[%d,%d]    ", TS_State.touchX[4], TS_State.touchY[4]);
			}

			/* Calculate circle radius to fill according to finger pressure applied on screen (weight) */
			radius = TS_State.touchWeight[0]/3;
			if (radius > CIRCLE_RADIUS){
				radius = CIRCLE_RADIUS;
			}
			else if (radius < 1){
				radius = 1;
			}

			if ((y > (CIRCLE_YPOS(1) - CIRCLE_RADIUS)) &&
				(y < (CIRCLE_YPOS(1) + CIRCLE_RADIUS))){

				if ((x > (CIRCLE_XPOS(1) - CIRCLE_RADIUS)) &&
					(x < (CIRCLE_XPOS(1) + CIRCLE_RADIUS))){
					if ((radius != radius_previous) || (state != 1)){
						if (state != 1){ /* Erase previous filled circle */
							CupRamenTimer_DrawBackground(state);
						}
						BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
						BSP_LCD_FillCircle(CIRCLE_XPOS(1), CIRCLE_YPOS(1), radius);
						radius_previous = radius;
						state = 1;
					}
					if(timer == 0)
						SVC_PERROR(set_flg(FLG_TIMER, TIMER_START));
					else
						SVC_PERROR(set_flg(FLG_TIMER, TIMER_STOP));
				}
				if ((x > (CIRCLE_XPOS(2) - CIRCLE_RADIUS)) &&
					(x < (CIRCLE_XPOS(2) + CIRCLE_RADIUS))){
					if ((radius != radius_previous) || (state != 2)){
						if (state != 2){ /* Erase previous filled circle */
							CupRamenTimer_DrawBackground(state);
						}
						BSP_LCD_SetTextColor(LCD_COLOR_RED);
						BSP_LCD_FillCircle(CIRCLE_XPOS(2), CIRCLE_YPOS(2), radius);
						radius_previous = radius;
						state = 2;
					}
					if(timer == 0){
						item++;
						if(item >= NUM_ITEM)
							item = 0;
					}
				}

				if ((x > (CIRCLE_XPOS(3) - CIRCLE_RADIUS)) &&
					(x < (CIRCLE_XPOS(3) + CIRCLE_RADIUS))){
					if ((radius != radius_previous) || (state != 4)){
						if (state != 4){ /* Erase previous filled circle */
							CupRamenTimer_DrawBackground(state);
						}
						BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
						BSP_LCD_FillCircle(CIRCLE_XPOS(3), CIRCLE_YPOS(3), radius);
						radius_previous = radius;
						state = 4;
					}
					if(timer == 0)
						next++;
					else
						SVC_PERROR(set_flg(FLG_TIMER, TIMER_UPDATE));
				}

				if ((x > (CIRCLE_XPOS(4) - CIRCLE_RADIUS)) &&
					(x < (CIRCLE_XPOS(4) + CIRCLE_RADIUS))){
					if ((radius != radius_previous) || (state != 8)){
						if (state != 8){ /* Erase previous filled circle */
							CupRamenTimer_DrawBackground(state);
						}
						BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
						BSP_LCD_FillCircle(CIRCLE_XPOS(4), CIRCLE_YPOS(3), radius);
						radius_previous = radius;
						state = 8;
					}
					if(timer == 0)
						next--;
				}
			}
			dly_tsk(200);
		} /* of if(TS_State.touchDetected) */
		result = twai_flg(FLG_BLINK, BLINK_ALL, TWF_ORW, &flgptn, 50);
		if(result == E_OK){
            if ((flgptn & BLINK_TIMEOUT) != 0) {
				state = 0;
				CupRamenTimer_DrawBackground(16);
				CupRamenTimer_DrawBackground(16);
				CupRamenTimer_DrawBackground(16);
			}
		}
	}
}


/*
 *  LED1点滅周期ハンドラ
 */ 
void
led1_blink_handler(void)
{
    static unsigned char sta_led = OFF;

    if (sta_led == ON) {
        sta_led = OFF;
    } else {
        sta_led = ON;
    }
    
    set_led_state(LED01, sta_led);
}


/*
 * スイッチ状態スキャン周期ハンドラ
 */
void
sw_scan_handler(void)
{
    unsigned char sw;
    FLGPTN   flgptn = 0;

    set_led_state(LED02, OFF);
	sw = get_cup_command();
    if (sw == 'S' || sw == 's') {
        flgptn |= TIMER_START;
    }
    else if (sw == 'E' || sw == 'e') {
        flgptn |= TIMER_STOP;
    }
    else if (sw == 'U' || sw == 'u') {
        flgptn |= TIMER_UPDATE;
    }

    if (flgptn != 0x00)
      SVC_PERROR(iset_flg(FLG_TIMER, flgptn));
}

/*
 * 基本時間生成周期ハンドラ
 */
void
base_time_handler(void)
{
    BaseTime++;
}

static void set_clock(struct tm2 *ptm, int item, int next)
{
	int maxday = monthday[ptm->tm_mon - 1];

	if(((ptm->tm_year - 30) % 4) == 0 && ptm->tm_mon == 2)
		maxday++;
	switch(item){
	case ITEM_YEAR:
		ptm->tm_year += next;
		if(ptm->tm_year < 30)
			ptm->tm_year = 99+30;
		else if(ptm->tm_year > (99+30))
			ptm->tm_year = 30;
		break;
	case ITEM_MONTH:
		ptm->tm_mon += next;
		if(ptm->tm_mon < 1)
			ptm->tm_mon = 12;
		else if(ptm->tm_mon > 12)
			ptm->tm_mon = 1;
		break;
	case ITEM_DATE:
		ptm->tm_mday += next;
		if(ptm->tm_mday < 1)
			ptm->tm_mday = maxday;
		else if(ptm->tm_mday > maxday)
			ptm->tm_mday = 1;
		break;
	case ITEM_HOURS:
		ptm->tm_hour += next;
		if(ptm->tm_hour < 0)
			ptm->tm_hour = 23;
		else if(ptm->tm_hour > 23)
			ptm->tm_hour = 0;
		break;
	case ITEM_MINS:
		ptm->tm_min += next;
		if(ptm->tm_min < 0)
			ptm->tm_min = 59;
		else if(ptm->tm_min > 59)
			ptm->tm_min = 0;
		break;
	case ITEM_SECS:
		ptm->tm_sec += next;
		if(ptm->tm_sec < 0)
			ptm->tm_sec = 59;
		else if(ptm->tm_sec > 59)
			ptm->tm_sec = 0;
		break;
	default:
		break;
	}
	mktime(ptm);
	rtc_set_time(ptm);
}

static void set_value(char *buf, int value)
{
    buf[1] = (value % 10) + '0';
    buf[0] = (value / 10) + '0';
}

static void set_time(char *buf, long time, char sep)
{
    int wk = time;
    set_value(&buf[6], wk % 60);
    wk = wk / 60;
    buf[5] = sep;
    set_value(&buf[3], wk % 60);
    wk = wk / 60;
    buf[2] = sep;
    set_value(&buf[0], wk);
}

