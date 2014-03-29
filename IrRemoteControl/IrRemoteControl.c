/*
 * IrRemoteControl.c
 *
 * Created: 28/03/2014 03:00:06 م
 *  Author: master
 */ 

#define POWER	93
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
#define ZERO	151
#define ONE		207
#define TWO		231
#define THREE	133
#define FOUR	239
#define FIVE	199
#define SIX		165
#define SEVEN	189
#define EIGHT	181
#define NINE	173
#define NULL	0

#define LOCKED				0
#define UNLOCKED			1
#define READY				2
#define CORRECT				3
#define INCORRECT			4
#define EMPTYPASSWORD		5
#define NEWPASSWORD			6
#define PASSMAXLENGTH		10
#define DEVICEADDRESS		255
#define SYSTEMPASSWORD		"01234"
		
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "atmega32_uart.h"
volatile unsigned int CurrentCaptureTime ;
volatile unsigned int PreviousCaptureTime ;
volatile unsigned int NewBitTime ;
volatile unsigned char NewBitTimeFlag;
unsigned char SysStatus=LOCKED;
unsigned char data[10];







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

void getCode(uint16_t* NewBitTimeDuration,unsigned long* IrCommandBuffer,uint8_t* Iraddress,uint8_t* IrCommand, uint8_t* NewIrCommandFlag)
{
	static unsigned char StartBit=32;
	static unsigned char CurrentBitPosition=0;
	
	if ((*NewBitTimeDuration)<1) /* negative value >> overflow */
	{
		NewBitTime+=65536;
		if ((*NewBitTimeDuration)<1) /* still negative >> garbage time */
		{
			IrCommandBuffer=0;
			CurrentBitPosition=0;
			StartBit=0;
			return;
		}
	}
	
	if (StartBit==1)
	{
		if ((*NewBitTimeDuration)>=60 && (*NewBitTimeDuration)<=80) /* Time of Zero bit */
		{
			*IrCommandBuffer&=~(1UL<<(--CurrentBitPosition));
		}
		else if ((*NewBitTimeDuration)>=130 && (*NewBitTimeDuration)<=150) /* Time of One bit */
		{
			*IrCommandBuffer|=(1UL<<(--CurrentBitPosition));
		}
		else /*error IR bit time >> ignore the whole command and recieve new one*/
		{
			
			*IrCommandBuffer=0;
			CurrentBitPosition=32;
			StartBit=0;
			return;
		}
		
		
	}
	else if((*NewBitTimeDuration)>=800 && (*NewBitTimeDuration)<=900)
	{
		StartBit=1;
		*IrCommandBuffer=0;
	}
	
	if(CurrentBitPosition==0)
	{
		*NewIrCommandFlag=1;
		*IrCommand=(char)(*IrCommandBuffer);
		*Iraddress=(char)((*IrCommandBuffer)>>16);
		CurrentBitPosition=32;
		StartBit=0;
	}
	
}

void GetPassword(uint8_t* IrAddress,uint8_t* IrCommand,uint8_t* PasswordFlag,uint8_t* Password)
{
	static uint8_t PasswordIndex=0;
	
	if (*IrAddress==DEVICEADDRESS)
	{
		if (*IrCommand==POWER)
		{
			if (PasswordIndex<PASSMAXLENGTH)
			{
				Password[PasswordIndex]=0;
			}
			else
			{
				Password[PASSMAXLENGTH]=0;
			}
			
			*PasswordFlag=NEWPASSWORD;
			PasswordIndex=0;
			USART_WRITE_STRING(Password);
			
		}
		else if (PasswordIndex<PASSMAXLENGTH)
		{
			switch(*IrCommand)
			{
				case ZERO:
				Password[PasswordIndex++]='0';
				USART_WRITE_BYTE('0');
				break;
				case ONE:
				Password[PasswordIndex++]='1';
				USART_WRITE_BYTE('1');
				break;
				case TWO:
				Password[PasswordIndex++]='2';
				USART_WRITE_BYTE('2');
				break;
				case THREE:
				Password[PasswordIndex++]='3';
				USART_WRITE_BYTE('3');
				break;
				case FOUR:
				Password[PasswordIndex++]='4';
				USART_WRITE_BYTE('4');
				break;
				case FIVE:
				Password[PasswordIndex++]='5';
				USART_WRITE_BYTE('5');
				break;
				case SIX:
				Password[PasswordIndex++]='6';
				USART_WRITE_BYTE('6');
				break;
				case SEVEN:
				Password[PasswordIndex++]='7';
				USART_WRITE_BYTE('7');
				break;
				case EIGHT:
				Password[PasswordIndex++]='8';
				USART_WRITE_BYTE('8');
				break;
				case NINE:
				Password[PasswordIndex++]='9';
				USART_WRITE_BYTE('9');
				break;
				
				
			}
			
			
			//Password[PasswordIndex++]=*IrCommand;
			
		}
	
	}	
}

void CheckPassword(uint8_t* Password,uint8_t* PasswordFlag)
{
	USART_WRITE_STRING("check");
	if (*Password==NULL)
	{
		*PasswordFlag=EMPTYPASSWORD;
		USART_WRITE_STRING("emptyyy");
	}
	else if(!strcmp(Password,SYSTEMPASSWORD))
	{
		*PasswordFlag=CORRECT;
		USART_WRITE_STRING("correcttt");
	}
	else
	{
		*PasswordFlag=INCORRECT;
		USART_WRITE_STRING("incorrecttt");
	}
	
}

int main(void)
{
	setUp();
	USART_WRITE_STRING("Starting : ");
	unsigned long Buffer=0;
	uint8_t IrCmd=0;
	uint8_t IrAddress=0;
	uint8_t NewIrCmdFlag=0;
	uint8_t ReceivedPassword[10]={0};
	uint8_t ReceivedPasswordFlag=0;	
	
	
	
	
    while(1)
    {
		
		if (NewBitTimeFlag)
		{
			NewBitTimeFlag=0;
			getCode(&NewBitTime,&Buffer,&IrAddress,&IrCmd,&NewIrCmdFlag);
			if (NewIrCmdFlag==1)
			{
				NewIrCmdFlag=0;
				if (SysStatus==LOCKED)
				{
					if (IrCmd==POWER)
					{
						SysStatus=READY;
						USART_WRITE_STRING("ready");
					}
				} 
				else if (SysStatus==READY)
				{
					GetPassword(&IrAddress,&IrCmd,&ReceivedPasswordFlag,ReceivedPassword);
					if (ReceivedPasswordFlag==NEWPASSWORD)
					{
						CheckPassword(&ReceivedPassword,&ReceivedPasswordFlag);
						if (ReceivedPasswordFlag==CORRECT) 
						{
							SysStatus=UNLOCKED;
							USART_WRITE_STRING("correct");
						}
						else if (ReceivedPasswordFlag==INCORRECT)
						{
							USART_WRITE_STRING("incorrect");
							// error message
						}
						else if (ReceivedPasswordFlag==EMPTYPASSWORD)
						{
							USART_WRITE_STRING("empty");
							// empty password message
						}
					}
					
				
				}
				
				else if (SysStatus==UNLOCKED)
				{
					USART_WRITE_STRING("Unlocked");
				}
				
				
				IrCmd=0;
			}
		}
     
    }
}