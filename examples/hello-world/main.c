/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Hello World application
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include "periph/gpio.h"
#include "rtctimers-millis.h"
#include "periph/adc.h"
#include "shell_commands.h"
#include "shell.h"
#include "periph/pm.h"
#include "periph/pwm.h"
#include "thread.h"
#include "xtimer.h"
#include "opt3001.h"

static kernel_pid_t process_pid;
static msg_t process_msg;

#define MY_PROCESS_STACK_SIZE (1024)

#define ENABLE_DEBUG 	(1)
#include "debug.h"

uint32_t milliseconds_last_press = 0;

static opt3001_measure_t opt3001_data;
static opt3001_t opt3001;


static void *process_thread(void *arg)
	{
	(void)arg;
	msg_t message;
	gpio_set(GPIO_PIN(PORT_B, 0));
	while(1) 
		{
		msg_receive(&message);
		gpio_toggle (GPIO_PIN(PORT_B, 0));
	/*	if (gpio_read(GPIO_PIN(PORT_B, 0))==1)
			{
			DEBUG("РЕЗУЛЬТАТ %d\n", gpio_read(GPIO_PIN(PORT_B, 0)));
			printf("Освещенность %lu\n", opt3001_data.luminocity);
			}
		else
			{
			DEBUG("СЧИТЫВАНИЕ %d\n", gpio_read(GPIO_PIN(PORT_B, 1)));	
			}
	*/	opt3001_measure(&opt3001, &opt3001_data);
       		//printf("Luminocity is %lu\n", opt3001_data.luminocity);
       		int sample = adc_sample(3, ADC_RES_12BIT);
       		int vref = adc_sample(ADC_VREF_INDEX,ADC_RES_12BIT);
       		
       		sample = (sample*vref)/4096;
       		
       		printf("ADC value: %d mV, Vref: %d mV\n",sample, vref); 
		
		}
	return NULL;
	}

static void btn_led_toggle(void* arg)
	{
	(void)arg;

	if ((rtctimers_millis_now() - milliseconds_last_press) > 200) 
		{
		msg_send(&process_msg, process_pid);
		milliseconds_last_press = rtctimers_millis_now();
		}
	
	}
static const shell_command_t shell_commands[] = {
    { NULL, NULL, NULL }
};

int main(void)
	{
	pm_init();
	pm_prevent_sleep = 1;
	
	rtctimers_millis_init();
	
	
	opt3001.i2c = 1;
	opt3001_init(&opt3001);
	
	adc_init(3);
	adc_init (ADC_VREF_INDEX);
	
	pwm_init(PWM_DEV(0), PWM_LEFT, 1000, 1000);
	pwm_set(0, 0, 50);
	
	

	    puts("Hello World!");

	    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
	    printf("This board features a(n) %s MCU.\n", RIOT_MCU);
	    
	    opt3001_measure(&opt3001, &opt3001_data);
	    printf ("Luminocity is %lu/n", opt3001_data.luminocity);
	
	gpio_init(GPIO_PIN(PORT_B, 0), GPIO_OUT);
	gpio_init_int(GPIO_PIN(PORT_B, 1),GPIO_IN_PU, GPIO_FALLING, btn_led_toggle, NULL);


	char stack[MY_PROCESS_STACK_SIZE];
	process_pid = thread_create(stack, MY_PROCESS_STACK_SIZE,
			  	    THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
			  	    process_thread, NULL, "our process");
	char line_buf[SHELL_DEFAULT_BUFSIZE];
	shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    	
    	return 0;
    	}
