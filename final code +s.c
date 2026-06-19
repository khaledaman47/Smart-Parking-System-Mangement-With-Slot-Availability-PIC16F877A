/*
 * Smart Parking System - PIC16F877A
 * 10 Slots - 2 IR Sensors - 2 Servos - I2C LCD
 *
 * Crystal: 20MHz
 *
 * RB0 / Pin 33 = Entry IR sensor OUT  active LOW
 * RB1 / Pin 34 = Exit  IR sensor OUT  active LOW
 * RB2 / Pin 35 = Green LED
 * RB3 / Pin 36 = Yellow LED
 * RB4 / Pin 37 = Red LED
 * RC2 / Pin 17 = Buzzer
 * RC3 / Pin 18 = I2C LCD SCL
 * RC4 / Pin 23 = I2C LCD SDA
 *
 * RD0 / Pin 19 = Entry servo signal
 * RD2 / Pin 21 = Exit  servo signal
 *
 * RD1 / Pin 20 = Broken / not used
 */

#include <xc.h>

#define _XTAL_FREQ 20000000UL

#pragma config FOSC  = HS
#pragma config WDTE  = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP   = OFF
#pragma config CPD   = OFF
#pragma config WRT   = OFF
#pragma config CP    = OFF

/* ================= SETTINGS ================= */

#define TOTAL_SLOTS 10
#define LCD_ADDR    0x27      /* If LCD has light but no text, change to 0x3F */

/*
 * Servo positions:
 * 1500us = closed / center
 * 850us  = about 120 degree LEFT
 * 2150us = about 120 degree RIGHT
 */
#define ENTRY_CLOSE_US       1500u
#define ENTRY_OPEN_LEFT_US   850u

#define EXIT_CLOSE_US        1500u
#define EXIT_OPEN_RIGHT_US   2150u

#define SERVO_PERIOD_US      20000u

/*
 * 13us step:
 * 1500 -> 850 = 650us difference
 * 650 / 13 = 50 frames
 * 50 frames x 20ms = about 1 second open/close
 */
#define SERVO_STEP_US        13u

/*
 * 100 frames x 20ms = about 2 seconds hold open
 */
#define SERVO_HOLD_FRAMES    100u

#define GATE_IDLE            0u
#define GATE_OPENING         1u
#define GATE_HOLD_OPEN       2u
#define GATE_CLOSING         3u

/* LCD I2C backpack bits */
#define LCD_BL 0x08
#define LCD_EN 0x04
#define LCD_RS 0x01

/* ================= PIN NAMES ================= */

#define ENTRY_IR     PORTBbits.RB0
#define EXIT_IR      PORTBbits.RB1

#define LED_GREEN    PORTBbits.RB2
#define LED_YELLOW   PORTBbits.RB3
#define LED_RED      PORTBbits.RB4

#define BUZZER       PORTCbits.RC2
#define LCD_SCL      PORTCbits.RC3
#define LCD_SDA      PORTCbits.RC4

#define ENTRY_SERVO  PORTDbits.RD0
#define EXIT_SERVO   PORTDbits.RD2

/* ================= I2C LCD FUNCTIONS ================= */

void I2C_Start(void)
{
    TRISCbits.TRISC4 = 0;

    LCD_SDA = 1;
    __delay_us(5);
    LCD_SCL = 1;
    __delay_us(5);
    LCD_SDA = 0;
    __delay_us(5);
    LCD_SCL = 0;
    __delay_us(5);
}

void I2C_Stop(void)
{
    TRISCbits.TRISC4 = 0;

    LCD_SDA = 0;
    __delay_us(5);
    LCD_SCL = 1;
    __delay_us(5);
    LCD_SDA = 1;
    __delay_us(5);
}

void I2C_WriteBit(unsigned char b)
{
    if(b)
    {
        LCD_SDA = 1;
    }
    else
    {
        LCD_SDA = 0;
    }

    __delay_us(5);
    LCD_SCL = 1;
    __delay_us(5);
    LCD_SCL = 0;
    __delay_us(5);
}

void I2C_ReadAck(void)
{
    TRISCbits.TRISC4 = 1;
    __delay_us(5);
    LCD_SCL = 1;
    __delay_us(5);
    LCD_SCL = 0;
    __delay_us(5);
    TRISCbits.TRISC4 = 0;
}

void I2C_WriteByte(unsigned char data)
{
    unsigned char i;
    unsigned char mask = 0x80;

    for(i = 0; i < 8; i++)
    {
        I2C_WriteBit((unsigned char)(data & mask));
        mask = (unsigned char)(mask >> 1);
    }

    I2C_ReadAck();
}

/* ================= LCD FUNCTIONS ================= */

void LCD_SendNibble(unsigned char nibble, unsigned char ctrl)
{
    unsigned char d;

    d = (unsigned char)((nibble << 4) | ctrl | LCD_BL);

    I2C_Start();
    I2C_WriteByte((unsigned char)((LCD_ADDR << 1) | 0x00));
    I2C_WriteByte((unsigned char)(d | LCD_EN));
    I2C_Stop();

    __delay_us(2);

    I2C_Start();
    I2C_WriteByte((unsigned char)((LCD_ADDR << 1) | 0x00));
    I2C_WriteByte((unsigned char)(d & (unsigned char)(~LCD_EN)));
    I2C_Stop();

    __delay_us(50);
}

void LCD_SendByte(unsigned char byte, unsigned char ctrl)
{
    LCD_SendNibble((unsigned char)((byte >> 4) & 0x0F), ctrl);
    LCD_SendNibble((unsigned char)(byte & 0x0F), ctrl);
}

void LCD_Cmd(unsigned char cmd)
{
    LCD_SendByte(cmd, 0x00);

    if(cmd <= 0x03)
    {
        __delay_ms(2);
    }
}

void LCD_Char(char c)
{
    LCD_SendByte((unsigned char)c, LCD_RS);
}

void LCD_String(const char *s)
{
    while(*s != '\0')
    {
        LCD_Char(*s);
        s++;
    }
}

void LCD_SetCursor(unsigned char row, unsigned char col)
{
    if(row == 0)
    {
        LCD_Cmd((unsigned char)(0x80 + col));
    }
    else
    {
        LCD_Cmd((unsigned char)(0xC0 + col));
    }
}

void LCD_Init(void)
{
    __delay_ms(50);

    LCD_SendNibble(0x03, 0);
    __delay_ms(5);

    LCD_SendNibble(0x03, 0);
    __delay_ms(5);

    LCD_SendNibble(0x03, 0);
    __delay_us(200);

    LCD_SendNibble(0x02, 0);
    __delay_us(200);

    LCD_Cmd(0x28);
    LCD_Cmd(0x0C);
    LCD_Cmd(0x06);
    LCD_Cmd(0x01);
    __delay_ms(2);
}

void LCD_PrintNum(unsigned char n)
{
    if(n >= 10u)
    {
        LCD_Char((char)('0' + (n / 10u)));
    }

    LCD_Char((char)('0' + (n % 10u)));
}

/* ================= DISPLAY FUNCTIONS ================= */

void UpdateLEDs(unsigned char free_slots)
{
    LED_GREEN = 0;
    LED_YELLOW = 0;
    LED_RED = 0;

    if(free_slots == 0u)
    {
        LED_RED = 1;
    }
    else if(free_slots == 1u)
    {
        LED_YELLOW = 1;
    }
    else
    {
        LED_GREEN = 1;
    }
}

void UpdateLCD(unsigned char free_slots)
{
    LCD_Cmd(0x01);
    __delay_ms(2);

    LCD_SetCursor(0, 0);
    LCD_String("Smart Parking");

    LCD_SetCursor(1, 0);

    if(free_slots == 0u)
    {
        LCD_String("FULL! No Slots");
    }
    else
    {
        LCD_String("Free: ");
        LCD_PrintNum(free_slots);
        LCD_Char('/');
        LCD_PrintNum(TOTAL_SLOTS);
    }
}

/* ================= BUZZER ================= */

unsigned int buzzer_frames = 0u;

void Buzzer_Start(unsigned int frames)
{
    buzzer_frames = frames;
}

void Buzzer_Update(void)
{
    if(buzzer_frames > 0u)
    {
        BUZZER = 1;
        buzzer_frames--;
    }
    else
    {
        BUZZER = 0;
    }
}

/* ================= SERVO FUNCTIONS ================= */

void Delay_us_Custom(unsigned int us)
{
    unsigned int i;

    for(i = 0u; i < us; i += 10u)
    {
        __delay_us(10);
    }
}

/*
 * Send one 20ms servo frame for both servos.
 * If pulse = 0, that servo signal stays LOW/rest.
 */
void Servo_Frame(unsigned int entry_pulse_us, unsigned int exit_pulse_us)
{
    unsigned int max_pulse;
    unsigned int rest_time;

    ENTRY_SERVO = 0;
    EXIT_SERVO = 0;

    if(entry_pulse_us == 0u && exit_pulse_us == 0u)
    {
        __delay_ms(20);
        return;
    }

    if(entry_pulse_us > SERVO_PERIOD_US)
    {
        entry_pulse_us = SERVO_PERIOD_US;
    }

    if(exit_pulse_us > SERVO_PERIOD_US)
    {
        exit_pulse_us = SERVO_PERIOD_US;
    }

    if(entry_pulse_us > 0u)
    {
        ENTRY_SERVO = 1;
    }

    if(exit_pulse_us > 0u)
    {
        EXIT_SERVO = 1;
    }

    if(entry_pulse_us == 0u)
    {
        Delay_us_Custom(exit_pulse_us);
        EXIT_SERVO = 0;
        max_pulse = exit_pulse_us;
    }
    else if(exit_pulse_us == 0u)
    {
        Delay_us_Custom(entry_pulse_us);
        ENTRY_SERVO = 0;
        max_pulse = entry_pulse_us;
    }
    else if(entry_pulse_us < exit_pulse_us)
    {
        Delay_us_Custom(entry_pulse_us);
        ENTRY_SERVO = 0;

        Delay_us_Custom((unsigned int)(exit_pulse_us - entry_pulse_us));
        EXIT_SERVO = 0;

        max_pulse = exit_pulse_us;
    }
    else if(exit_pulse_us < entry_pulse_us)
    {
        Delay_us_Custom(exit_pulse_us);
        EXIT_SERVO = 0;

        Delay_us_Custom((unsigned int)(entry_pulse_us - exit_pulse_us));
        ENTRY_SERVO = 0;

        max_pulse = entry_pulse_us;
    }
    else
    {
        Delay_us_Custom(entry_pulse_us);
        ENTRY_SERVO = 0;
        EXIT_SERVO = 0;

        max_pulse = entry_pulse_us;
    }

    rest_time = (unsigned int)(SERVO_PERIOD_US - max_pulse);
    Delay_us_Custom(rest_time);
}

/* ================= GATE CONTROL ================= */

void Gate_Trigger(unsigned char *state,
                  unsigned int *pulse_us,
                  unsigned int *hold_counter,
                  unsigned int close_us)
{
    if(*state == GATE_IDLE)
    {
        *pulse_us = close_us;
        *hold_counter = 0u;
        *state = GATE_OPENING;
    }
    else if(*state == GATE_HOLD_OPEN)
    {
        *hold_counter = SERVO_HOLD_FRAMES;
    }
    else if(*state == GATE_CLOSING)
    {
        *state = GATE_OPENING;
    }
}

unsigned int Gate_Update(unsigned char *state,
                         unsigned int *pulse_us,
                         unsigned int *hold_counter,
                         unsigned int close_us,
                         unsigned int open_us)
{
    unsigned int output_pulse = 0u;

    if(*state == GATE_IDLE)
    {
        output_pulse = 0u;
    }
    else if(*state == GATE_OPENING)
    {
        output_pulse = *pulse_us;

        if(*pulse_us < open_us)
        {
            *pulse_us = (unsigned int)(*pulse_us + SERVO_STEP_US);

            if(*pulse_us > open_us)
            {
                *pulse_us = open_us;
            }
        }
        else if(*pulse_us > open_us)
        {
            *pulse_us = (unsigned int)(*pulse_us - SERVO_STEP_US);

            if(*pulse_us < open_us)
            {
                *pulse_us = open_us;
            }
        }
        else
        {
            *state = GATE_HOLD_OPEN;
            *hold_counter = SERVO_HOLD_FRAMES;
        }
    }
    else if(*state == GATE_HOLD_OPEN)
    {
        output_pulse = open_us;

        if(*hold_counter > 0u)
        {
            *hold_counter = (unsigned int)(*hold_counter - 1u);
        }
        else
        {
            *pulse_us = open_us;
            *state = GATE_CLOSING;
        }
    }
    else if(*state == GATE_CLOSING)
    {
        output_pulse = *pulse_us;

        if(*pulse_us < close_us)
        {
            *pulse_us = (unsigned int)(*pulse_us + SERVO_STEP_US);

            if(*pulse_us > close_us)
            {
                *pulse_us = close_us;
            }
        }
        else if(*pulse_us > close_us)
        {
            *pulse_us = (unsigned int)(*pulse_us - SERVO_STEP_US);

            if(*pulse_us < close_us)
            {
                *pulse_us = close_us;
            }
        }
        else
        {
            output_pulse = close_us;
            *state = GATE_IDLE;
        }
    }
    else
    {
        *state = GATE_IDLE;
        output_pulse = 0u;
    }

    return output_pulse;
}

/* ================= MAIN ================= */

void main(void)
{
    unsigned char free_slots = TOTAL_SLOTS;

    unsigned char entry_now;
    unsigned char exit_now;

    unsigned char entry_prev = 1u;
    unsigned char exit_prev = 1u;

    unsigned char entry_gate_state = GATE_IDLE;
    unsigned char exit_gate_state = GATE_IDLE;

    unsigned int entry_servo_pulse = ENTRY_CLOSE_US;
    unsigned int exit_servo_pulse = EXIT_CLOSE_US;

    unsigned int entry_hold_counter = 0u;
    unsigned int exit_hold_counter = 0u;

    unsigned int entry_frame_pulse = 0u;
    unsigned int exit_frame_pulse = 0u;

    ADCON1 = 0x07;

    CCP1CON = 0x00;
    CCP2CON = 0x00;

    TRISA = 0xFF;

    /*
     * RB0 = Entry IR input
     * RB1 = Exit IR input
     * RB2 = Green LED output
     * RB3 = Yellow LED output
     * RB4 = Red LED output
     * RB5-RB7 = Inputs
     */
    TRISB = 0xE3;

    /*
     * RC2 = Buzzer output
     * RC3 = LCD SCL output
     * RC4 = LCD SDA output
     */
    TRISC = 0xE3;

    /*
     * RD0 = Entry servo output
     * RD1 = Broken, keep input
     * RD2 = Exit servo output
     * RD3-RD7 = Inputs
     */
    TRISD = 0xFA;

    TRISE = 0x07;

    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;
    PORTD = 0x00;

    OPTION_REGbits.nRBPU = 0;

    ENTRY_SERVO = 0;
    EXIT_SERVO = 0;
    BUZZER = 0;

    LCD_Init();
    UpdateLCD(free_slots);
    UpdateLEDs(free_slots);

    Buzzer_Start(5u);

    while(1)
    {
        entry_now = (unsigned char)ENTRY_IR;
        exit_now  = (unsigned char)EXIT_IR;

        /*
         * IR sensors are active LOW:
         * 1 = no car
         * 0 = car detected
         *
         * Count only on new detection:
         * HIGH -> LOW
         */

        if(entry_prev == 1u && entry_now == 0u)
        {
            if(free_slots > 0u)
            {
                free_slots--;

                UpdateLCD(free_slots);
                UpdateLEDs(free_slots);

                Buzzer_Start(5u);

                Gate_Trigger(&entry_gate_state,
                             &entry_servo_pulse,
                             &entry_hold_counter,
                             ENTRY_CLOSE_US);
            }
            else
            {
                Buzzer_Start(20u);
            }
        }

        if(exit_prev == 1u && exit_now == 0u)
        {
            if(free_slots < TOTAL_SLOTS)
            {
                free_slots++;
            }

            UpdateLCD(free_slots);
            UpdateLEDs(free_slots);

            Buzzer_Start(8u);

            Gate_Trigger(&exit_gate_state,
                         &exit_servo_pulse,
                         &exit_hold_counter,
                         EXIT_CLOSE_US);
        }

        entry_prev = entry_now;
        exit_prev = exit_now;

        entry_frame_pulse = Gate_Update(&entry_gate_state,
                                        &entry_servo_pulse,
                                        &entry_hold_counter,
                                        ENTRY_CLOSE_US,
                                        ENTRY_OPEN_LEFT_US);

        exit_frame_pulse = Gate_Update(&exit_gate_state,
                                       &exit_servo_pulse,
                                       &exit_hold_counter,
                                       EXIT_CLOSE_US,
                                       EXIT_OPEN_RIGHT_US);

        Buzzer_Update();

        Servo_Frame(entry_frame_pulse, exit_frame_pulse);
    }
}