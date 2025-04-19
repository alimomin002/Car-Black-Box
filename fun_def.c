/*
 * File:   fun_def.c
 * Author: Ali Momin
 *
 * Created on 28 November, 2024, 5:02 PM
 */
#include "main.h"

unsigned char *gear[9] = {"ON", "GN", "G1", "G2", "G3", "G4", "G5", "GR", "C_"};  // Event Array
char menu[4][15] = {"View log    ", "Download log ", "Clear log    ", "set-time     "}; // Main menu array
unsigned int menu_pos = 0;  // Current menu position
int flag = 0;
unsigned char time[9];      // Time string
char event[3] = "ON";       // Current event
unsigned char speed = 0;
unsigned char control_flag;
extern unsigned char key;
unsigned short int gr = 0;      // Gear position
char log[13];
char pos = 0;
unsigned int event_count = 0;
unsigned char add = 0;          // EEPROM address
unsigned char clock_reg[3];
int main_flag = 0;
unsigned char read_arr[10][15]; // To read from EEPROM
unsigned char tim[9]; // for set time
unsigned int hour, min, sec;

// Reads time from the DS1307 RTC and formats it into the `time` array
void get_time(void) {

    clock_reg[0] = read_ds1307(HOUR_ADDR);
    clock_reg[1] = read_ds1307(MIN_ADDR);
    clock_reg[2] = read_ds1307(SEC_ADDR);

    if (clock_reg[0] & 0x40) {
        time[0] = '0' + ((clock_reg[0] >> 4) & 0x01);
        time[1] = '0' + (clock_reg[0] & 0x0F);
    } else {
        time[0] = '0' + ((clock_reg[0] >> 4) & 0x03);
        time[1] = '0' + (clock_reg[0] & 0x0F);
    }
    time[2] = ':';
    time[3] = '0' + ((clock_reg[1] >> 4) & 0x0F);
    time[4] = '0' + (clock_reg[1] & 0x0F);
    time[5] = ':';
    time[6] = '0' + ((clock_reg[2] >> 4) & 0x0F);
    time[7] = '0' + (clock_reg[2] & 0x0F);
    time[8] = '\0';
}

// Displays the dashboard information
void view_dashboard() {
    
    get_time();     // Read current time

    // Read speed from ADC
    speed = read_adc(CHANNEL4) / 10.33;

    // Handle key presses to change gear and log events
    if (key == MK_SW1) {
        strcpy(event, "C_");
        event_store();
        gr = 0;
    } else if (key == MK_SW2 && gr < 7) {
        gr++;
        strcpy(event, gear[gr]);
        event_store();
    } else if (key == MK_SW3 && gr > 1) {
        gr--;
        strcpy(event, gear[gr]);
        event_store();
    }

    // Display time, event, and speed on the CLCD
    clcd_print("  TIME     E  SP", LINE1(0));

    clcd_print(time, LINE2(0));
    clcd_print(event, LINE2(11));
    clcd_putch((speed / 10) + '0', LINE2(14));
    clcd_putch((speed % 10) + '0', LINE2(15));

    //for go to main menu
    if (key == MK_SW11) 
    {
        CLEAR_DISP_SCREEN;
        state = e_main_menu;

    }

}

// Stores a new event to the EEPROM
void event_store() 
{
    //store all data into one array.
    strncpy(log, time, 8);
    strncpy(&log[8], event, 2);
    log[10] = (speed / 10) + '0';
    log[11] = (speed % 10) + '0';
    log[12] = '\0';


    if (add >= 120) 
    {
        add = 0;
    }

    for (int i = 0; log[i] != '\0'; i++) 
    {
         // Write the log entry to EEPROM
        write_EEPROM(add, log[i]);
        add++;
    }
    
    event_count++;
    
  
    
}

// Displays the main menu
void display_main_menu() {
    static unsigned char arrow = 0, i = 0;

    //Arrow Handling
    if (arrow == 0) {
        clcd_print("->", LINE1(0));
        clcd_print("  ", LINE2(0));
    } else {
        clcd_print("  ", LINE1(0));
        clcd_print("->", LINE2(0));
    }
    clcd_print(menu[i], LINE1(3));
    clcd_print(menu[i + 1], LINE2(3));

    if (key == MK_SW12) {
        if (menu_pos < 3) {
            menu_pos++;
        }
        if (arrow == 0) {
            arrow = 1;

        }
        else if (i < 2) {
            i++;
        }
    }
    if (key == MK_SW11) {
        if (menu_pos > 0) {
            menu_pos--;
        }
        if (arrow == 1) {
            arrow = 0;
        } else if (i > 0) {
            i--;

        }
    }
    // Select the menu item or exit to dashboard
    // Set state based on the selected menu position
    if (key == MK_SW2) {
        CLEAR_DISP_SCREEN;
        state = e_dashboard;
        arrow = 0;
        i = 0;
    }
    if (key == MK_SW1 && menu_pos == 0) {
        CLEAR_DISP_SCREEN;
        state = e_view_log;
    } else if (key == MK_SW1 && menu_pos == 1) {
        CLEAR_DISP_SCREEN;
        state = e_download_log;
    } else if (key == MK_SW1 && menu_pos == 2) {
        CLEAR_DISP_SCREEN;
        state = e_clear_log;
    } else if (key == MK_SW1 && menu_pos == 3) {
        CLEAR_DISP_SCREEN;
        state = e_set_time;
    }
}

void view_log() {
    static unsigned int start_ind = 0, once = 1,once1 = 1;
    unsigned int dummy = event_count;
    
    // Handle empty logs
    if(event_count == 0)
    {
        if (once1) {
        CLEAR_DISP_SCREEN;
        once1 = 0;
        }
        clcd_print("No Log Found", LINE1(0));
    }
    else
    {

    clcd_print("SL  TIME   EV SP", LINE1(0));
    // if(flag)

    if (once) {
        CLEAR_DISP_SCREEN;
        event_reader(); // Read log data for EEPROM
        once = 0;
    }
    
    clcd_putch(start_ind + 48, LINE2(0));
    clcd_print(read_arr[start_ind], LINE2(2));


    // Limit to the last 10 entries
    if(event_count > 10)
    {
        dummy = 10;
    }
    if (key == MK_SW12 && start_ind < dummy - 1) {
        start_ind++;
    }

    if (start_ind > 0 && key == MK_SW11) {
        start_ind--;
    }
    }

    // Return to the main menu
    if (key == MK_SW2) 
    {
        CLEAR_DISP_SCREEN;
        state = e_main_menu;
        
    }


}

void event_reader() {
    unsigned char add1 = 0;
    int end=event_count;
    
    // Read the last 10 events or all if less than 10
    if (event_count > 10 )
    {
        add1 =( event_count % 10 ) * 12 ; // Calculate starting address
        end = 10;
    }
    
// Read events from EEPROM
    for (int i = 0; i < end ; i++) {
        for (int j = 0; j < 15; j++) {
            if (j == 8 || j == 11) {
                read_arr[i][j] = ' ';
            } else if (j == 14) {
                read_arr[i][j] = '\0';
            } else {
                read_arr[i][j] = read_EEPROM(add1++);
                if(add1 >= 120 )
                {
                    add1 = 0;
                }
            }
        }
    }
}

void set_time() {
    // char dummy;
    strcpy(tim, time); //copy current time in one array
    
    static int delay;
    static int field_flag = 0;
    static flag = 0;
    //unsigned short int d_hour,d_min,d_sec;

    clcd_print("HH:MM:SS", LINE1(0)); // Display format
    clcd_putch(':', LINE2(2));
    clcd_putch(':', LINE2(5));
    if (flag == 0) {
        // Display current time on CLCD
        clcd_putch(tim[0], LINE2(0));
        clcd_putch(tim[1], LINE2(1));
        clcd_putch(tim[3], LINE2(3));
        clcd_putch(tim[4], LINE2(4));
        clcd_putch(tim[6], LINE2(6));
        clcd_putch(tim[7], LINE2(7));

        hour = (tim[0] - 48)*10;
        hour = hour + (tim[1] - 48);
        min = (tim[3] - 48)*10;
        min = min + (tim[4] - 48);
        sec = (tim[6] - 48)*10;
        sec = sec + (tim[7] - 48);
        flag = 1;
    }
    // Handle hour, minute, and second settings
    // Blinking effect and increment logic for each field
    if (key == MK_SW11) {

        clcd_putch(48 + hour / 10, LINE2(0));
        clcd_putch(48 + hour % 10, LINE2(1));
        clcd_putch(48 + min / 10, LINE2(3));
        clcd_putch(48 + min % 10, LINE2(4));
        clcd_putch(48 + sec / 10, LINE2(6));
        clcd_putch(48 + sec % 10, LINE2(7));
        field_flag++;
    }
    if (field_flag == 4)
        field_flag = 1;

    if (field_flag == 1) {
        if (delay++ < 1000) {
            clcd_putch(48 + hour / 10, LINE2(0));
            clcd_putch(48 + hour % 10, LINE2(1));

        } else if (delay > 1000 && delay < 2000) {
            clcd_putch(' ', LINE2(0));
            clcd_putch(' ', LINE2(1));
        } else
            delay = 0;

        if (key == MK_SW12)
            hour++;

        if (hour > 23)
            hour = 0;
        if (hour < 0)
            hour = 23;
    }

    if (field_flag == 2) {
        if (delay++ < 1000) //for showing blinking _
        {
            clcd_putch(48 + min / 10, LINE2(3));
            clcd_putch(48 + min % 10, LINE2(4));
        } else if (delay > 1000 && delay < 2000) {
            clcd_putch(' ', LINE2(3));
            clcd_putch(' ', LINE2(4));
        } else
            delay = 0;

        if (key == MK_SW12)
            min++;

        if (min > 59)
            min = 0;
        if (min < 0)
            min = 59;
    }


    if (field_flag == 3) {
        if (delay++ < 1000) //for showing blinking _
        {
            clcd_putch(48 + sec / 10, LINE2(6));
            clcd_putch(48 + sec % 10, LINE2(7));
        } else if (delay > 1000 && delay < 2000) {
            clcd_putch(' ', LINE2(6));
            clcd_putch(' ', LINE2(7));
        } else
            delay = 0;

        if (key == MK_SW12)
            sec++;

        if (sec > 59)
            sec = 0;
        if (sec < 0)
            sec = 59;
    }

    if (key == MK_SW1) {
        CLEAR_DISP_SCREEN;
        write_ds1307(HOUR_ADDR, ((hour / 10 << 4) | (hour % 10)));
        write_ds1307(MIN_ADDR, ((min / 10 << 4) | (min % 10)));
        write_ds1307(SEC_ADDR, ((sec / 10 << 4) | (sec % 10)));
        clcd_print("SET TIME SUCCESS", LINE1(0));
        
        field_flag = 0;
        __delay_us(900000);
        // main_key=0;
        CLEAR_DISP_SCREEN;
        state = e_main_menu;
    } else if (key == MK_SW2) {
        //  main_key=0;
        CLEAR_DISP_SCREEN;
        state = e_main_menu;
    }

}

void download_log() {
    
    int once = 1;
    
    // Handle empty logs
    if(event_count == 0)
    {
        if(once)
        {
            CLEAR_DISP_SCREEN;
            once = 0;
        }
   
      puts("No Log Found");
    }
    
    static unsigned char onces = 1;
     if(onces)
     {
         event_reader(); // Read log data
         init_uart();   //// Initialize UART for data transmission
         onces=0;
     } 
    clcd_print("Download Log...", LINE1(0));
    for(unsigned long int i = 500000;i--; );
    CLEAR_DISP_SCREEN;
    puts("  TIME   EV SP");
    puts("\n\r");
    
    int stop = event_count;
    
    if (event_count > 10 ) stop=10;
        
   for(int i = 0; i < stop ;i++)
   {
        puts(read_arr[i]);
       puts("\n\r");
   } 
    __delay_us(9000);
    
     CLEAR_DISP_SCREEN;
    state = e_main_menu;
}

void clear_log() {
 // Clear all log data
    add = 0;
    flag = 0;
    event_count = 0;
    
    clcd_print("Cleared log...", LINE1(0));

    for (long int i = 900000; i--;);

    CLEAR_DISP_SCREEN;
    state = e_main_menu;
}