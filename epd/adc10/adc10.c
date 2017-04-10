#include "..\sys_init\datatype.h"
#include "adc10.h"
#include "..\global_variable\global_variable.h"


#define FULL_SCALE      ((UINT32)1023)
#define ADC_3_3V        ((UINT32)33)
#define ADC_3_2V        ((UINT32)32)
#define ADC_3_1V        ((UINT32)31)
#define ADC_3_0V        ((UINT32)30)
#define ADC_2_9V        ((UINT32)29)
#define ADC_2_8V        ((UINT32)28)
#define ADC_2_7V        ((UINT32)27)
#define ADC_2_6V        ((UINT32)26)


void adc_voltage(void)
{     
/*
  UINT32 i=0;
  
  ADC10CTL1 = INCH_11;                      // AVcc/2
  ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + REF2_5V + ADC10ON;
  
  ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
  while (ADC10CTL1 & ADC10BUSY);          // ADC10BUSY?
  
  i = ((UINT32)(ADC10MEM * AD_REF)) * 2;
  i += ((UINT32)512) ;
  i = i/FULL_SCALE;

  ADC10CTL0 &= (~ENC);
  ADC10CTL0 = 0;
  
  if(i >= ADC_3_3V)
    gelectric_quantity = 0x07;
  else if(i >= ADC_3_2V) 
    gelectric_quantity =  0x06;
  else if(i >= ADC_3_1V) 
    gelectric_quantity =  0x05; 
  else if(i >= ADC_3_0V) 
    gelectric_quantity =  0x04;
  else if(i >= ADC_2_9V) 
    gelectric_quantity =  0x03;
  else if(i >= ADC_2_8V) 
    gelectric_quantity =  0x02;
  else if(i >= ADC_2_7V) 
    gelectric_quantity =  0x01;
  else 
    gelectric_quantity =  0x00;
*/
}
