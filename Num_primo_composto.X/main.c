/*
 * File:   main.c
 * Author: Pedro
 *
 * Created on 23 de Agosto de 2021, 22:55
 */

#include "config.h"
#include <xc.h>
#include "delay.h"
#include "library_shelf.h"


char value;

void num_primo (char x)
{
    char cont, num_p, i;
   
    cont = 0;  num_p = x;  i = 0;
  
    while( i <= num_p )
    {
        if(num_p % i == 0) cont++; 
        if(cont == 2) value = 1;
        else value = 0;  
        
        i++;
    }  
}

int return_nump (void)
{
    return(value);
}

void main(void)
{
    int valor = 11;
    
    print.init();
    
    while(1)
    {
        num_primo(valor);
        

        if(return_nump()) 
        {
               print.string("Nmr primo:    "); 
               print.number(0,13,valor,3);
        }
        else   print.string("Nmr composto:");   
               print.number(0,13,valor,3);
    }    
  }

