/*
 * File:   main.c
 * Author: Ali Momin
 *
 * Created on 28 November, 2024, 4:51 PM
 */
#include <xc.h>
#include "main.h"

State_t state;

unsigned char key;
extern int flag;

void init_config()
{
    state = e_dashboard;
    init_matrix_keypad(); // Initialize keypad
	init_clcd();            // Initialize CLCD
    init_adc();             // Initialize ADC
    init_i2c();             // Initialize I2C
	init_ds1307();          // Initialize DS1307 (RTC)
}

void main(void) 
{
    init_config();

    while(1)
    {
        // Detect key press
        key = read_switches(STATE_CHANGE);
      
        switch (state)
        {
            case e_dashboard:
                // Display dashboard
                view_dashboard();
                break;
            
            case e_main_menu:
                // Display dashboard
                display_main_menu();
                break;
            
            case e_view_log:
                // Display dashboard
                view_log();
                break;
                 
            case e_download_log:
                // Download system logs
                download_log();
                break;
                
            case e_clear_log:
                // Clear system logs
               clear_log();
                break;
                
                      
            case e_set_time:
                // Set system time
                set_time();
                break;
                
        }
        
    }
    
}

