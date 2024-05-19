#include "mbed.h"
#include <cstdint>
//////////////declaration
extern uint8_t screen_mode;
extern uint8_t press_data;
extern uint8_t error_flag;
extern uint8_t light_data;
extern uint8_t wash_time;
void buzzer_con(uint8_t state);
//////////////////////////////MAIN DEFINATION
BusOut leds(PC_0); // POOWER_LED RUN_LED
BusOut led_three(PB_3,PB_4);
PwmOut led3 (PB_5);
uint8_t power_ok=0;
uint8_t run_flag=0;
uint16_t seg_slow=0,key_slow=0,uart_slow=0; 
Timer timer_slow_down;
BufferedSerial serial(USBTX,USBRX);

/*
 * Function: led_pro
 * ----------------------------
 * Controls the LEDs and buzzer based on various conditions.
 *
 * Parameters: None
 *
 * Returns: None
 */
void led_pro()
{
    if(power_ok == 1){
        leds =  leds|0x01; 
    }
    else{
        leds =  leds&0xfe; 
    }

    if(power_ok == 1)
    {
        if(run_flag == 1){
            led3.pulsewidth_ms(wash_time/6);
        }
        else{
            led3.pulsewidth_ms(0);
        }

        if(run_flag == 0){
            led_three = led_three|0x02;
        }
        else{
            led_three =  led_three&0xfd;
        }

        if(error_flag == 1&&run_flag==1){
            led_three = led_three|0x01;
            buzzer_con(1);
        }
        else if(error_flag == 0&&run_flag!=2){
            led_three =  led_three&0xfe;
            buzzer_con(0);
        } 
    }
    else
    {
        led_three = 0x00;
    }
    
}
////////////////////////////SEG PART
//APB_1 BPB_14 CPB_11 DPA_11 EPA_12 FPB_15 GPB_2
BusOut SegDis(PB_1, PB_14, PB_11, PA_11, PA_12, PB_15, PB_2,PB_12);
DigitalOut decimal_point(PB_12); 
int hexDis[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x00,0x77,0x7c,0x39,0x5e,0x79,0x71,0x40,0X74,0X37};
// 0 1 2 3 4 5 6 7 8 9 x a b c d e f - h N
int screen_num[6]={0};
BusOut SegPos(PA_5,PA_6,PA_7,PB_6,PC_7,PA_9);
int dp_status = 0;
uint8_t pos=0;


/*
 * Function: SegDis_init
 * ----------------------------
 * Initializes the segment display by turning off all segments.
 *
 * Parameters: None
 *
 * Returns: None
 */
void SegDis_init(){
    SegDis.write(0x00); //turn off the display by setting all segments to '0'
    SegPos=0x00;
}

/*
 * Function: seg_display
 * ----------------------------
 * Updates the segment display based on the values stored in screen_num array and the current pos.
 *
 * Parameters: None
 *
 * Returns: None
 */
void seg_display()
{
        SegDis.write(0x00); 
        SegPos = ~(0x01<<pos);
        SegDis.write(screen_num[pos]); 
}


uint8_t wash_mode=1;
uint8_t wash_time=10;
uint8_t wash_temperature=35;
uint8_t time_light=0;
uint8_t run_mode = 0;
float temperature = 19.00;
uint8_t seg_flag = 0;
/*
 * Function: SegDis_show
 * ----------------------------
 * Updates the segment display based on various conditions such as power status, run mode, etc.
 *
 * Parameters: None
 *
 * Returns: None
 */
void SegDis_show(){
    if(seg_slow) return;
    seg_slow = 1;
    if(power_ok)
    {
        if(run_flag == 0)
        {
            if(screen_mode==0) {
                screen_num[0]=hexDis[11];
                screen_num[1]=hexDis[10];
                screen_num[2]=hexDis[10];
                screen_num[3]=hexDis[10];
                screen_num[4]=hexDis[wash_mode/10];
                screen_num[5]=hexDis[wash_mode%10];
            }
            else if(screen_mode==1) {
                screen_num[0]=hexDis[12];
                screen_num[1]=hexDis[10];
                screen_num[2]=hexDis[10];
                screen_num[3]=hexDis[10];
                screen_num[4]=hexDis[wash_time/10];
                screen_num[5]=hexDis[wash_time%10];
            }
            else if(screen_mode==2) {
                screen_num[0]=hexDis[13];
                screen_num[1]=hexDis[10];
                screen_num[2]=hexDis[10];
                screen_num[3]=hexDis[10];
                screen_num[4]=hexDis[wash_temperature/10];
                screen_num[5]=hexDis[wash_temperature%10];
            }
        }
        if(run_flag == 1)
        {
            if(run_mode == 0)
            {
                screen_num[0]=hexDis[wash_mode/10];
                screen_num[1]=hexDis[wash_mode%10];
                if(time_light == 0)
                {
                    screen_num[2]=hexDis[17];
                    screen_num[3]=hexDis[17];
                }
                else
                {
                    screen_num[2]=hexDis[10];
                    screen_num[3]=hexDis[10];
                }
                    screen_num[4]=hexDis[wash_time/10%10];
                    screen_num[5]=hexDis[wash_time%10];
            }
            else if(run_mode == 1)
            {
                screen_num[0]=hexDis[(uint8_t)temperature/10%10];
                screen_num[1]=hexDis[(uint8_t)temperature%10]|0x80;
                screen_num[2]=hexDis[(uint16_t)(temperature*10)%10];
                screen_num[3]=hexDis[17];
                screen_num[4]=hexDis[press_data/10];
                screen_num[5]=hexDis[press_data%10];
            }
        }
        else if(run_flag == 2)
        {
            screen_num[0]=hexDis[16];
            screen_num[1]=hexDis[1];
            screen_num[2]=hexDis[19];
            screen_num[3]=hexDis[5];
            screen_num[4]=hexDis[18];
            screen_num[5]=hexDis[10];
        }   
    } 
    else
    {
        SegDis.write(0x00);
        decimal_point.write(0);
        leds.write(0x00);
        screen_mode=0;
        run_flag = 0;
        wash_time = 10;
        if(time_light == 0)
        {
            screen_num[0]=hexDis[12];
            screen_num[1]=hexDis[15];
            screen_num[2]=hexDis[9];
            screen_num[3]=hexDis[1];
            screen_num[4]=hexDis[19];
            screen_num[5]=hexDis[10];  
        }
        else
        {
            screen_num[0]=hexDis[10];
            screen_num[1]=hexDis[12];
            screen_num[2]=hexDis[15];
            screen_num[3]=hexDis[9];
            screen_num[4]=hexDis[1];
            screen_num[5]=hexDis[19]; 
        }
    }
}


DigitalIn button1(PC_10); //power
DigitalIn button2(PC_11); //+
DigitalIn button3(PD_2); //-
DigitalIn button4(PC_1); //change
AnalogIn pot1(PC_2);
uint8_t key_value=0,key_old=0,key_up=0;//key_para
uint8_t key_time=0;
uint8_t screen_mode=1; //Select the parameters, duration, or mode to set
uint8_t long_press_flag=0;
uint8_t change_mode=0;
/*
 * Function: key_scan
 * ----------------------------
 * Scans the state of all buttons and returns a value based on which buttons are pressed.
 *
 * Returns: A value indicating the state of the buttons.
 */
uint8_t key_scan()
{
    uint8_t temp = 0;
    if(button1.read() == 1) temp += 1;
    if(button2.read() == 1) temp += 2;
    if(button3.read() == 1) temp += 3;
    if(button4.read() == 1) temp += 4;
    return temp;
}

/*
 * Function: key_read
 * ----------------------------
 * Reads the state of buttons and performs corresponding actions based on the button presses.
 *
 * Parameters: None
 *
 * Returns: None
 */
void key_read()
{
    if(key_slow) return;
    key_slow = 1;
    key_value =  key_scan();
    key_up = ~key_value&(key_old^key_value);
    key_old = key_value;

    //process
    if(key_up == 1) //power led
    {
       // leds = leds^0x01;
        power_ok = 1-power_ok;
        if(power_ok == 1)
            printf("Washing machine is turned on\n");
        else 
            printf("Washing machine is turned off\n");
    }
    if(power_ok)
    {
        if(run_flag==0)
        {
            if(change_mode ==0)
            {
                if(key_old == 5)
                    change_mode = 1;
                else if(key_old == 2)
                {
                    if(++key_time == 50)
                    {
                        if(screen_mode == 0)
                        {
                            if(++wash_mode == 4) wash_mode = 1;  
                        }
                        else if(screen_mode == 1)
                        {
                            if(++wash_time == 61) wash_time = 10;
                        }
                        else if(screen_mode == 2)
                        {
                            if(++wash_temperature == 51) wash_temperature = 10;
                        }
                        key_time=0;
                        long_press_flag = 1;
                    }
                }
                else if(key_up == 2)
                {
                    if(long_press_flag)//Prevent long press interference
                    {
                        long_press_flag = 0;
                        return;
                    }
                    if(screen_mode == 0)
                    {
                        if(++wash_mode == 4) wash_mode = 1;  
                    }
                    else if(screen_mode == 1)
                    {
                        if(++wash_time == 61) wash_time = 10;
                    }
                    else if(screen_mode == 2)
                    {
                        if(++wash_temperature == 51) wash_temperature = 10;
                    }
                }   
                if(key_old == 3)
                {
                    if(++key_time == 50)
                    {
                        if(screen_mode == 0)
                        {
                            if(--wash_mode == 0) wash_mode = 3;  
                        }
                        else if(screen_mode == 1)
                        {
                            if(--wash_time == 9) wash_time =60;
                        }
                        else if(screen_mode == 2)
                        {
                         if(--wash_temperature == 9) wash_temperature = 50;
                        }
                        key_time=0;
                        long_press_flag = 1;
                    }
                }
                else if(key_up == 3)
                {
                    if(long_press_flag)//Prevent long press interference
                    {
                        long_press_flag = 0;
                        return;
                    }
                    if(screen_mode == 0)
                    {
                        if(--wash_mode == 0) wash_mode =3;  
                    }
                    else if(screen_mode == 1)
                    {
                        if(--wash_time == 9) wash_time = 60;
                    }
                    else if(screen_mode == 2)
                    {
                        if(--wash_temperature == 9) wash_temperature = 50;
                    }
                } 
            }
            else if(change_mode == 1)
            {
                if(key_old==5)
                {
                    change_mode = 0;
                }
                if(screen_mode == 0)
                    wash_mode = (uint8_t)(pot1.read()*3);
                else if(screen_mode == 1)
                    wash_time = 10+(uint8_t)(pot1.read()*50);
                else if(screen_mode == 2)
                    wash_temperature = 10+(uint8_t)(pot1.read()*40);

                if(wash_mode == 0)
                    wash_mode = 1;
            }  
        }
        else if(run_flag == 1)
        {
            if(key_up == 2)
            {
                run_mode = 1-run_mode;
            }
        }
        else if(run_flag == 2)
        {
            if(key_up == 2)
            {
                run_flag = 0;
                buzzer_con(0);
            }
        }

        if(key_old == 4)
        {
            if(++key_time == 200)
            {
                printf("Washing machine begin running.\n");
                printf("SETING MODE: mode:%d,temp:%d,time:%d\n",run_mode,(uint8_t)temperature,wash_time);
                run_flag = 1; 
                key_time=0;
                long_press_flag = 1;
            }
        }
        else if(key_up == 4)
        {
            if(long_press_flag)
            {
                long_press_flag = 1;
                return;
            } 
            if(run_flag == 0)
            {
                if(++screen_mode == 3)  screen_mode = 0;  
            }
            else if(run_flag == 2)
            {
                run_flag = 0;
                buzzer_con(0);
            }
        }

        if(key_old == 0)
            key_time = 0;    
    }
}


AnalogIn press(PA_1);
AnalogIn temp(PC_3);
AnalogIn light(PB_0);
uint8_t press_data=0;
uint8_t para_press = 50;
uint8_t error_flag = 0;
uint8_t light_data = 0;
/*
 * Function: data_check
 * ----------------------------
 * Reads data from sensors and checks for error conditions based on predefined thresholds.
 *
 * Parameters: None
 *
 * Returns: None
 */
void data_check()
{
    if(uart_slow) return;
    uart_slow = 1;
    press_data = (uint8_t)(press.read()*100.00);
    temperature = (temp.read()*330.0000);
    light_data = (uint8_t)(light.read()*100.00);
    if(press_data>para_press)
    {
        printf("Error state!!!!\n");
        error_flag = 1;
    }
    else if(temperature>wash_temperature)
    {
        error_flag = 1;
        printf("Error state!!!!\n");
    }
    else 
        error_flag = 0;

    if(run_flag == 1)
    printf("data:%d,%.2f,%d,%d\n",wash_time,temperature,press_data,light_data);
}



/*
 * Function: buzzer_con
 * ----------------------------
 * Controls the buzzer state based on the input state parameter.
 *
 * Parameters:
 *     state: An integer representing the state of the buzzer. 1 for ON, 0 for OFF.
 *
 * Returns: None
 */
PwmOut buzzer(PA_15);
void buzzer_con(uint8_t state)
{
    if(state)
    {
        buzzer.pulsewidth_us(buzzer.read_period_us()/2);
    }
    else
    {
        buzzer.pulsewidth_us(0); 
    }
}

///////////////////////////////////
//ms_interrupt
uint16_t ms_flag=0;
uint16_t time_s=0;
/*
 * Function: ms_interrupt
 * ----------------------------
 * Interrupt service routine for handling millisecond interrupts.
 * Controls various timing-related operations.
 *
 * Parameters: None
 *
 * Returns: None
 */
void ms_interrupt()
{
    if(timer_slow_down.elapsed_time() >= std::chrono::milliseconds(1) ){
        timer_slow_down.reset();
        if(++seg_slow==6) seg_slow = 0;
        if(++key_slow==5)  key_slow = 0;
        if(++uart_slow == 500) uart_slow =0;
        if(++pos == 6) pos=0;
        seg_display();
        if(++ms_flag == 500)
        {
            time_light = 1-time_light;
            ms_flag = 0;
        }
        if(++time_s == 1001)
        {
            if(run_flag==1)
            {
                if(--wash_time == 0)
                {
                    wash_time = 10;
                    run_flag = 2;
                    printf("Washing machine is finished.\n");
                    buzzer_con(1);
                }
            }
            time_s = 0;
        }
    }  
}


/*
 * Function: init_all
 * ----------------------------
 * Initializes all components and peripherals used in the system.
 *
 * Parameters: None
 *
 * Returns: None
 */
void init_all()
{
    SegDis_init();
    led3.period_ms(10);
    serial.set_baud(115200);
    timer_slow_down.start();
    buzzer.period_us((float) 1000000.0f/ (float) 330); 
    led_three = 0x00; //led init
}



int main()
{
    init_all();
    while (true) {
        ms_interrupt();
        key_read();
        SegDis_show();
        data_check();
        led_pro();
    }
}
