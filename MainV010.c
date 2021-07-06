/*
 * File:   MainV010.c
 * Author: Matias
 * EL proyecto intentara conectar hacer un flash audioritmico. La salida digital
 * que activa el flash estara gobernado por la señal analogica leida proviente de
 * un microfono conectado a un preamplificador
 * 
 * V0.1.0 = Esta version leera una entrada analogica por el puerto AN0 (RA0) y 
 *          mostrara el valor leido en voltage en un LCD 16*2 
 *
 * Created on 17 de octubre de 2019, 19:44
 */

#define _XTAL_FREQ 8000000
//definicion de los puertos para conectar el LCD 16*2
#define EN RA7
#define RS RA6
#define D4 RA1
#define D5 RA2
#define D6 RA3
#define D7 RA4


#include <xc.h>
#include "ConfigBits.h"
#include "lcd_16x4.h"
#include <stdio.h>
#include "UART.h"

void interrupt Atention();
void strobo(unsigned char x);


void main(void) 
{ 
    unsigned char mode = 0;
    char respuesta = 0;
    
//congiguramos el oscilador interno a 8 Mhz
    OSCCONbits.IRCF = 0b111;   //internal RC internal oscilator frequency is 8Mhz
    OSCCONbits.SCS = 0b00;  //oscilator mode is defie by configurtration bits FOSC
                            //FOSC esta configurado como oscilador interno y ra6 y ra7 como digital port
    
    
//definimos los puertos como digitales o analogicos
    ANSELbits.ANS0 = 1; //AN0 (RA0) como entrada analogica
    ANSELbits.ANS1 = 0; //AN1 (RA1) como entrada analogica
    ANSELbits.ANS2 = 0; //AN2 (RA2) como entrada analogica
    ANSELbits.ANS3 = 0; //AN3 (RA3) como entrada analogica
    ANSELbits.ANS4 = 0; //AN4 (RA4) como entrada analogica
    ANSELbits.ANS5 = 0; //AN5 (RB6) como entrada analogica
    ANSELbits.ANS6 = 0; //AN6 (RB7) como entrada analogica

//definimos los puertos como digitales como entradas o salidas    
    TRISAbits.TRISA0 = 1;   //como entrada para la entrada analogica que proviene del microfono
    TRISAbits.TRISA1 = 0;   //controla el port D4 del lcd 
    TRISAbits.TRISA2 = 0;   //controla el port D5 del lcd
    TRISAbits.TRISA3 = 0;   //controla el port D6 del lcd
    TRISAbits.TRISA4 = 0;   //controla el port D7 del lcd
    TRISAbits.TRISA6 = 0;   //controla el port RS del lcd
    TRISAbits.TRISA7 = 0;   //controla el port EN del lcd
    
    TRISBbits.TRISB0 = 0;   //salida que prende el flash
    TRISBbits.TRISB1 = 0;   //salida que indica que que el programa esta funcionando
    TRISBbits.TRISB2 = 1;   //PicRX para comunicar con el modulo bluetoth
    TRISBbits.TRISB5 = 0;   //PicTX para comunicar con el modulo bluetoth
    
    
    Lcd_Init();
    Lcd_Clear(); 
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String((char*)("Hola Mundo"));
    __delay_ms(500);
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String((char*)("Soy Matias"));
    __delay_ms(500);
    
    UART_Init(9600);    //inicializo la UART a 9600 bits/seg
    
    
// Configuracion del modulo A/D, AN0, Tda = 2us
    ADCON1bits.ADFM = 1; //right justifed, six most significant bits of ADRESH are read as '0'
    ADCON1bits.ADCS2 = 1; //A/D clock is divide by 2
    ADCON1bits.VCFG = 0b00; //+Vref = Vdd, -Vref = Gnd
    
    ADCON0bits.ADCS = 0b01; //A/d convertion clock is (Fosc/16) => que se cumple con
                            //la condicion de que TDA sea mayor que 1.6us, en este caso es 2uS
    ADCON0bits.CHS = 0b000; //select channel is AN0 (RA0)
    ADCON0bits.ADON = 1;    //modulo A/D energizado, listo para funcionar
    
    //configuro la interrupcion
    PIR1bits.ADIF = 0;  //borramos la bandera antes de empezar
    PIE1bits.ADIE = 1;  //habilitamos la interrupcion por conversion A/D
    INTCONbits.PEIE = 1;    //Habilito las interrpciones perifericas
    INTCONbits.GIE = 1;     //habilito las interrpciones globalmente
    __delay_us(50);     //retardo de 20us para la carga del capacitor
    
    //ADCON0bits.GO_DONE = 1; //inicializo la coversion a/d
    
   
    while(1)
    {
        switch(mode)
        {   //modo audioritmico
            case 0: ADCON0bits.GO_DONE = 1; //inicializo la coversion a/d
                break;
                
            //modo muy rapido (delay = 100ms)
            case 1: strobo(1);                
                break;
                
            //modo rapido (delay = 500ms)
            case 2: strobo(5);
                break;
            
            //modo medio (delay = 1000ms)    
            case 3: strobo(10);
                break;
            
            //modo lento (delay = 1500ms)
            case 4: strobo(15);                
                break;
                
            //modo muy lento (delay = 2000ms)    
            case 5: strobo(20);
                break;
        
        }
        //pregunto si hay un dato disponible, para combiar el modo de funcionamiento
        if(UART_Data_Ready())  
        {
            mode = UART_Read(); //el dato leido define el modo de funcionammiento
        }
    }   


    return;
}


void interrupt Atention()
{
    int cuentas = 0; //variable que almacenara el valor del conversor A/D
    char s[10]; //para usar en el lcd
    
    if(PIE1bits.ADIE == 1 && PIR1bits.ADIF == 1)
    {
        cuentas = ADRESH; //como la justificacion es a la izquierda asise optiene el resultado
        cuentas = cuentas << 8;
        cuentas = cuentas + ADRESL;
        if(cuentas >= 512) //si el valar es mayor a 512 activo la salida RB0 
        {
            PORTBbits.RB0 = 1;  
        }
        __delay_ms(10);
        PORTBbits.RB0 = 0;
        
        //sprintf(s,"Cuentas=%04d",cuentas);        //le doy formato a cuentas                       
        //Lcd_Set_Cursor(1,1);        // position cursor x=a,y=1
        //Lcd_Write_String(s);        //escribo el lcd
            
        //__delay_us(50);     //retardo de 20us para la carga del capacitor
        //aca no se espera el tiempo de adquisicion porque el la intrucciones anteriores (lcd) ya generan un ratardo suficiente
        //ADCON0bits.GO_DONE = 1; //inicializo la coversion a/d
        PIR1bits.ADIF = 0; //borro la bandera para proximo ingreso
    }

}



void strobo(unsigned char x)
{
    unsigned char i = 0;
    //ADCON0bits.GO_DONE = 0; //inicializo la coversion a/d
    
    PORTBbits.RB0 = 1;  //prendemos la salida del flash 
    __delay_ms(10); 
    PORTBbits.RB0 = 0;  //prendemos la salida del flash 
    
    for(i = 0; i < x; i++) //Retardo proporcion al valor de x (x por 100ms)
    {
        __delay_ms(100);    
    }
    
   
    
}