/*
 * IrRemoteControl.c
 *
 * Created: 28/03/2014 03:00:06 م
 *  Author: master
 */ 

#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "atmega32_uart.h"
volatile unsigned int CurrentCaptureTime ;
volatile unsigned int PreviousCaptureTime ;
volatile unsigned int NewBitTime ;
volatile unsigned char NewBitTimeFlag;


#define POWER	16753245
#define MODE	16736925
#define MUTE	16769565
#define REWARD	16720605
#define FORWARD	16712445
#define PLAY	16761405
#define VOL_DE	16769055
#define VOL_IN	16754775
#define EQ		16748655
#define HUNDRED	16750695
#define	BACK	16756815
#define ZERO	16738455
#define ONE		16724175
#define TWO		16718055
#define THREE	16743045
#define FOUR	16716015
#define FIVE	16726215
#define SIX		16734885
#define SEVEN	16728765
#define EIGHT	16730805
#define NNE		16732845




ISR(TIMER1_CAPT_vect)
{
	CurrentCaptureTime=ICR1;
	NewBitTime=(CurrentCaptureTime-PreviousCaptureTime);
	PreviousCaptureTime=CurrentCaptureTime;
	NewBitTimeFlag=1;
	
}


void setUp(void)
{
	USART_INIT(16);			/* 57600 baud rate				*/
	TCCR1B|=(1<<CS12);		/* 265 prescale					*/
	TCCR1B|=(1<<ICNC1);		/* noice canceller				*/
	TCCR1B&=~(1<<ICES1);	/* negative edge select			*/
	DDRD&=~(1<<PD6);		/* Input capture pin (input)	*/
	TIMSK|=(1<<TICIE1);		/* input capture interrupt		*/
	sei();					/* set global interrupt			*/
	
}

void getCode(unsigned int* NewBitTimeDuration,unsigned long* IrCommand, unsigned char* NewIrCommandFlag)
{
	static unsigned char StartBit=32;
	static unsigned char CurrentBitPosition=0;
	
	if ((*NewBitTimeDuration)<1) /* negative value >> overflow */
	{
		NewBitTime+=65536;
		if ((*NewBitTimeDuration)<1) /* still negative >> garbage time */
		{
			IrCommand=0;
			CurrentBitPosition=0;
			StartBit=0;
			return;
		}
	}
	
	if (StartBit==1)
	{
		if ((*NewBitTimeDuration)>=60 && (*NewBitTimeDuration)<=80) /* Time of Zero bit */
		{
			*IrCommand&=~(1UL<<(--CurrentBitPosition));
		}
		else if ((*NewBitTimeDuration)>=130 && (*NewBitTimeDuration)<=150) /* Time of One bit */
		{
			*IrCommand|=(1UL<<(--CurrentBitPosition));
		}
		else /*error IR bit time >> ignore the whole command and recieve new one*/
		{
			
			*IrCommand=0;
			CurrentBitPosition=32;
			StartBit=0;
			return;
		}
		
		
	}
	else if((*NewBitTimeDuration)>=800 && (*NewBitTimeDuration)<=900)
	{
		StartBit=1;
		*IrCommand=0;
	}
	
	if(CurrentBitPosition==0)
	{
		*NewIrCommandFlag=1;
		CurrentBitPosition=32;
		StartBit=0;
	}
	
}

int main(void)
{
	setUp();
	USART_WRITE_STRING("Starting : ");
	unsigned long IrCMD=0;
	unsigned char NewIrCmdFlag=0;
	
	
	
    while(1)
    {
		
		if (NewBitTimeFlag)
		{
			NewBitTimeFlag=0;
			getCode(&NewBitTime,&IrCMD,&NewIrCmdFlag);
			if (NewIrCmdFlag==1)
			{
				NewIrCmdFlag=0;
				/*ultoa(IrCMD,IRCMD,10);
				USART_WRITE_STRING(IRCMD);
				USART_WRITE_BYTE(13);*/
				
				switch (IrCMD)
				{
				case POWER : 
				USART_WRITE_STRING("POWER");
					break;
				case MODE :
				USART_WRITE_STRING("MODE");
					break;
				case MUTE :
				USART_WRITE_STRING("MUTE");
					break;
				case PLAY :
				USART_WRITE_STRING("PLAY");
					break;	
				}
				IrCMD=0;
			}
		}
     
    }
}