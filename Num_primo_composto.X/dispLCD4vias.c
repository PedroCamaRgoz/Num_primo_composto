/*
 * File:   dispLCD4vias.c
 * Author: 20187263
 *
 * Created on 28 de Abril de 2021, 14:29
 */

//* -------------------------------------------------
// *          MAPA DE ENTRADAS E SAIDAS
// * -------------------------------------------------
// *  Pinos   |n�     |Conex�o
// * ---------|-------|-------------------------------
// *  RD2     |21     | LCD_RS
// *  RD3     |22     | LCD_EN
// *  RD4     |27     | LCD_D4
// *  RD5     |28     | LCD_D5
// *  RD6     |29     | LCD_D6
// *  RD7     |30     | LCD_D7
// * -------------------------------------------------
// */

#include <xc.h>
#include "config.h"
//#include "keyboard.h"
#include "dispLCD4vias.h"

//***************** Interface com PORTs/Pinos
#define LCD_BUS         LCDbits.BUS
#define LCD_EN          LCDbits.EN
#define LCD_RS          LCDbits.RS
#define LCD_B0          LCDbits.B0
#define LCD_B1          LCDbits.B1
#define LCD_ROWS        2
#define LCD_COLS        16
#define LCD_ADDRS       0x008
#define LCD_ADDRS_B     0x006
//#define LCD_ADDRS_AUT( X )   volatile LCDbits_t LCDbits __at( X )

typedef union 
{
    struct 
    {
        unsigned char BUS :4;
        unsigned char RS  :1;
        unsigned char EN  :1;
        unsigned char B0  :1;
        unsigned char B1  :1;
    };
} LCDbits_t;

volatile LCDbits_t LCDbits __at(LCD_ADDRS);
// (LCD_ADDRS);

// Escreve um comando no display (Instruction Register)
void dispLCD_instReg( unsigned char i )
{
    LCD_RS = 0;
    LCD_BUS = (i>>4);

    LCD_EN = 0;
    if( i == LCD_CLEAR_DISPLAY || i == LCD_RETURN_HOME )
        __delay_ms( 2 );
    else
        __delay_us( 40 );
    LCD_EN = 1;
    
    
    if( (i & 0xF0) == (LCD_FUNCTION_SET | LCD_FS_DATA_LENGTH_4) )
    {
        LCD_RS = 0;
        LCD_BUS = i>>4;
        LCD_EN = 0;
        __delay_us( 40 );
        LCD_EN = 1;
    }
    
    LCD_RS = 0;
    LCD_BUS = i & 0x0F;
    LCD_EN = 0;
    if( i == LCD_CLEAR_DISPLAY || i == LCD_RETURN_HOME )
        __delay_ms( 2 );
    else 
        __delay_us( 40 );
    LCD_EN = 1;
}

// Escreve um dado no display (Data Register))
void dispLCD_dataReg( unsigned char d )
{
    LCD_RS = 1;
    LCD_BUS = d >> 4;
    LCD_EN = 0;
    __delay_us( 40 );
    LCD_EN = 1;

    LCD_RS = 1;
    LCD_BUS = d & 0x0F;
    LCD_EN = 0;
    __delay_us( 40 );
    LCD_EN = 1;
}

// Posicionar o cursor na coordenada: linha e coluna 
void dispLCD_lincol( unsigned char lin, unsigned char col)
{
    dispLCD_instReg( LCD_SET_DDRAM_ADDRS( (LCD_ADDR_LINE_1 * lin) + (col + LCD_ADDR_LINE_0) ) );
}

// Inicializa os pinos conectados ao display
void LCD_init( void )
{
    TRISDbits.TRISD0 = 0;   //BUS D4
    TRISDbits.TRISD1 = 0;   //BUS D5
    TRISDbits.TRISD2 = 0;   //BUS D6
    TRISDbits.TRISD3 = 0;   //BUS D7
    
    TRISDbits.TRISD4 = 0;   // RS
    TRISDbits.TRISD5 = 0;   // EN
    TRISDbits.TRISD6 = 1;   // B0
    TRISDbits.TRISD7 = 1;   // B1
    
    ANSELH = 0;
    TRISB  = 0;
    PORTB  = 0;

    LCD_EN = 1;
    dispLCD_instReg( LCD_FUNCTION_SET|LCD_FS_DATA_LENGTH_4|LCD_FS_LINE_NUMBER_2);
    dispLCD_instReg( LCD_DISPLAY_CONTROL|LCD_DC_DISPLAY_ON|LCD_DC_CURSOR_OFF|LCD_DC_BLINK_OFF );
    dispLCD_instReg( LCD_CLEAR_DISPLAY );
    dispLCD_instReg( LCD_RETURN_HOME );
    
   // keyboard_init();
}

void LCD_str( const char * str )
{
    dispLCD_lincol( 0, 0 );

    while( *str )
    {
        if( *str == '\n' )
        {
            dispLCD_lincol(1,0);
        }
        dispLCD_dataReg( *str );
        ++str;
    }
}


// Escreve uma string no display
void LCD_Texto( unsigned char lin, unsigned char col, const char *str )
{
    char pos = col;
    dispLCD_lincol( lin, col );

    while( *str )
    {
        dispLCD_dataReg( *str );
        ++str;
        ++pos;
    }
}

// Escreve um n�mero inteiro no display
void LCD_num(  unsigned char lin, unsigned char col, int num, unsigned char tam )
{
    int div;
    unsigned char size;
    char sinal;                                 // 0:+ 1:-
   
    sinal = num < 0;                
    if( sinal ) num = (~num) + 1;              // s� entra se aqui se sinal for = 1, num menor que 0.
     
    
    dispLCD_lincol(lin, col);    
   
    div=10000;  size = 5;
    
    while( div >= 1 )
    {
        if( num/div == 0 )
            --size;
        else
            break;                          // sai do while
        div/=10;                            // apos o if ele divide "div" por 10 para diminuir as casas numericas.
    }

    
    while( tam > (size+sinal) && tam > 1 )   // tira os espa�os em branco
    {
      dispLCD_dataReg(' ');
      --tam;
    }  

    if( sinal )
        dispLCD_dataReg('-');
 
    do
    {
        dispLCD_dataReg( (num / div) + '0' );   // 0 = 30 hex;
        num = num % div;
        div/=10;
    }
    while( div >= 1 );                          // dps de passar por "while( tam > (size+sinal) && tam > 1 )" adiciona 0 
                                                // se o numero precisar.
}

// Apaga todos os caracteres no display
void dispLCD_clr( void )
{
    dispLCD_instReg(LCD_CLEAR_DISPLAY);
}

unsigned char lcd_b0 (void)
{
    return(!LCD_B0);
}

unsigned char lcd_b1 (void)
{
    return(!LCD_B1);
}

