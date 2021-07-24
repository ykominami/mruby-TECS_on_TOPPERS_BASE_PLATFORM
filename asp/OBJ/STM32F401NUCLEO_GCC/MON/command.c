#include <kernel.h>
#include <t_syslog.h>
#include <stdio.h>
#include <t_stdlib.h>
#include <stdlib.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "kernel_cfg.h"
#include "monitor.h"
#include "device.h"

/*
 *  デバイスコマンド番号
 */

static int_t led_func(int argc, char **argv);
static int_t dip_func(int argc, char **argv);
static int_t cup_func(int argc, char **argv);

/*
 *  デバイスコマンドテーブル
 */
static const COMMAND_INFO device_command_info[] = {
	{"LED",		led_func},
	{"DIP",		dip_func},
	{"CUP",		cup_func}
};

#define NUM_DEVICE_CMD  (sizeof(device_command_info)/sizeof(COMMAND_INFO))

static const char device_name[] = "DEVICE";
static const char device_help[] =
"  Device  LED (no) (on-1,off-0)  led   control\n"
"          DIP (no) (on-1,off-0)  dipsw control\n"
"          CUP command            cup   control\n";

static const uint16_t led_pattern[4] = {
	LED01, LED02, LED03, LED04
};

static COMMAND_LINK device_command_link = {
	NULL,
	NUM_DEVICE_CMD,
	device_name,
	NULL,
	device_help,
	&device_command_info[0]
};

static uint8_t cup_command;

static int a2i(char *str)
{
	int num = 0;

	while(*str >= '0' && *str <= '9'){
		num = num * 10 + *str++ - '0';
	}
	return num;
}

/*
 *  DEVICEコマンド設定関数
 */
void device_info_init(intptr_t exinf)
{
	setup_command(&device_command_link);
}

/*
 *  LED設定コマンド関数
 */
static int_t led_func(int argc, char **argv)
{
	int_t    arg1=0, arg2;
	uint16_t reg;

	if(argc < 3)
		return -1;
	arg1 = atoi(argv[1]);
	arg2 = atoi(argv[2]);
	if(arg1 >= 1 && arg1 <= 4){
		reg = led_in();
		if(arg2 != 0)
			reg |= led_pattern[arg1-1];
		else
			reg &= ~led_pattern[arg1-1];
		led_out(reg);
	}
	return 0;
}

/*
 *  DIPSW設定コマンド関数
 */
static int_t dip_func(int argc, char **argv)
{
	int_t   arg1=0, arg2=0, i;

	if(argc > 1)
		arg1 = atoi(argv[1]);
	if(argc > 2)
		arg2 = atoi(argv[2]);
	if(arg1 >= 1 && arg1 <= 8){
		if(arg2 != 0)
			dipsw_value |= (0x1<<(arg1-1));
		else
			dipsw_value &= ~(0x1<<(arg1-1));
	}
	printf("dipsw\n1   2   3   4   5   6   7   8\n");
	for(i = 0 ; i < 8 ; i++){
		if(dipsw_value & (1<<i))
			printf("ON  ");
		else
			printf("OFF ");
	}
	printf("\n");
	return 0;
}

/*
 *  カップラーメンタイマ補助コマンド関数
 */
static int_t cup_func(int argc, char **argv)
{
	if(argc > 1)
		cup_command = *argv[1];
	return 0;
}

/*
 * CUPラーメンコマンド
 */
int8_t
get_cup_command(void)
{
	uint8_t cmd = cup_command;
	cup_command = 0;
	return cmd;
}


