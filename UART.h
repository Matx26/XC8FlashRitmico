/* 
 * File: UART.h
 * Author: Luna Matias  
 * Comments:
 * Revision history: 
 */

///////////////////////////////////////////////////////////////////////
//Archivo de cabecera para el uso del modulo UART de los pics /////////
///////////////////////////////////////////////////////////////////////
 
//funciones que tienen que ver con la transmision 
char UART_Init(const long int baudrate);      //func para iniciar el módulo UART
void UART_Write(char data);       //función para escribir un caracter
char UART_TX_Empty(void);   //función para controlar si la UART esta disponible para transmitir
void UART_Wrie_Text(char *text); //funcion para escribir un texto
//funciones que tienen que ver con la recebcion
char UART_Data_Ready(void);      //función que indica si hay un dato disponible para leer
char UART_Read(void); //    función para leer un caracter
void UART_Read_Text(char *Output, unsigned int length);  //funcion para leer una secuencia de datos


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//inicialización del módulo UART del  PIC 16f877a en XC8 ////
// como assincronico, para ello recibe la velocidad(baudrate/
// y calcula automaticomante                            /////
// el valor del registro SPBRG que define la velocidad (baud/
//rate)de transmision en funcion de la frecuencia del micro /
// (_XTAL_FREQ). Esta funcion retorna un 1 si la iniciliza-//
// cion salio bien y un 0 (cero) si salio mal /////////////// 
/////////////////////////////////////////////////////////////
char UART_Init(const long int baudrate)
{
    unsigned int x;
    x=(_XTAL_FREQ - baudrate*64)/(baudrate*64); //calculo el valor para el re-
                        //gistro SPBRG para baja velocidad de Baurate
    if(x > 255);         // si la velocidad es alta el valor de x sobrepasa  
    {               // a 255 y se rcalculo para alta velocidad de baurate
        x=(_XTAL_FREQ - baudrate*16)/(baudrate*16); //calculo el valor para el re-
                        //gistro SPBRG para alta velocidad de Baurate
        BRGH=1;     //indico que es alta la velocidad de baudrate
    }
    
    if(x<256)   //se corrobora que realmente el valor calculado de x no sobrepase
    {               // el reg de 8 bit de SPBRG
        SPBRG=x;   //se le asigna el valor calculo para el baudrate
        SYNC = 0,    //se define al UART en modo asincronico
        SPEN = 1,   //se habilita el puerto serial        
        TRISBbits.TRISB2 = 1; //segun lo deescrito en el datasheet
        TRISBbits.TRISB5 = 1; //segun lo descripto en el datasheet
        CREN = 1;   // habilito la recepcion continua
        TXEN = 1;   //Habilito la transmision
        return 1;   //si la funcion retorna un 1(un0) significa que todo salio bien        
    }
    return 0;   // retorna un cero si la inicilizacion a fallado
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
// Esta funcion transmite un byte pero para ello espera a ///
// que este disponible el modulo UART  para transmititr el //
// proximo byte
void UART_Write(char data)       //función para escribir un caracter
{
    while(!TRMT);   // espera hasta que el reg de envio este vacio o desocupado
    TXREG = data;   //cuando se livero envio el dato.
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
// Esta funcion espera a que este disponible el modulo UART//
// para transmitir, retorna un cero si esta libre para transmitir
char UART_TX_Empty(void)   //función para controlar si la UART esta disponible para transmitir
{
    return (char)(TRMT);    // retorno un 1 indica que esta ocupado transmitiendo y si
                    // retorna una o indica que esta libre
}

/*
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
// Esta funcion envia un texto que recibe como parametro////
void UART_Wrie_Text(char *text) //funcion para escribir un texto
{
    int i;
    for(i=0; text[i]!='/o'; i++)
    {
        UART_Write(text[i]);
    }

}
 */ 
  
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
// Esta funcion indica si esta disponible un dato en el    //
// registro de recepcion
char UART_Data_Ready(void)      //función que indica si hay un dato disponible para leer
{
    return (char)(RCIF);    //RCIF es la vandera que indica si hay una dato disponible
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
// Esta funcion espera a que este disponible un dato en la //
// recepcion y retorna el dato leido 
char UART_Read(void) //    función para leer un caracter
{
    while(!RCIF);   //espera a que un dato este disponible
    return RCREG;   //retorna el dato leido
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
// Esta funcion lee un cadena de caracteres y lo retorna   //
void UART_Read_Text(char *Output, unsigned int length)  //funcion para leer una secuencia de datos
{
    unsigned int i;
    for(i=0; i < length; i++)
    {
        Output[i] = UART_Read();
    }

}