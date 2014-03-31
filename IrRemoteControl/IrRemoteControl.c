/*
 * IrRemoteControl.c
 *
 * Created: 28/03/2014 03:00:06 م
 *  Author: master
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "atmega32_uart.h"
#include "lcd_library.h"


#define POWER	 255//93
#define MODE	16736925
#define MUTE	16769565
#define REWARD	16720605
#define FORWARD	16712445
#define PLAY	61
#define VOL_DE	16769055
#define VOL_IN	16754775
#define EQ		16748655
#define HUNDRED	16750695
#define	BACK	79
#define ZERO	151
#define ONE		127//207
#define TWO		191//231
#define THREE	159//133
#define FOUR	239
#define FIVE	199
#define SIX		165
#define SEVEN	189
#define EIGHT	181
#define NINE	173
#define NULL	0

#define LOCKED				1
#define UNLOCKED			2
#define READY				3
#define CORRECT				4
#define INCORRECT			5
#define EMPTYPASSWORD		6
#define NEWPASSWORD			7
#define AWAITINGPASSWORD	8
#define RECIEVINGPASSWORD	9
#define BACKONEDIGIT		10
#define NOCOMMANDSELECTED	11
#define COMMANDSELECTED		12
#define ON					13
#define OFF					14
#define ALARM				15
#define LASERPIN			PC0
#define SERINEPIN			PC1
#define IDLETIME			30
#define SYSTEMPASSWORD		"123"
#define PASSMAXLENGTH		10
#define DEVICEADDRESS		127//255



volatile uint8_t SysStatus=LOCKED;
uint8_t ALARMSYSSTATUS=OFF;
volatile uint8_t IDLETIMECOUNTER;
volatile uint8_t IDLETIMECOUNTERFLAG;
volatile uint8_t LcdUpdateFlag=LOCKED;

		

volatile unsigned int CurrentCaptureTime ;
volatile unsigned int PreviousCaptureTime ;
volatile unsigned int NewBitTime ;
volatile unsigned char NewBitTimeFlag;

unsigned char data[10];




ISR(INT0_vect)
{
	
//	GICR&=~(1<<INT0);				/*External interrupt INT0 disable	*/
	//fire the serine
	//USART_WRITE_STRING("Alarm");
	//ALARMSYSSTATUS=ALARM;
	
}



ISR(TIMER1_CAPT_vect)
{
	CurrentCaptureTime=ICR1;
	NewBitTime=(CurrentCaptureTime-PreviousCaptureTime);
	PreviousCaptureTime=CurrentCaptureTime;
	NewBitTimeFlag=1;
	
}

ISR(TIMER1_OVF_vect)
{
	if (IDLETIMECOUNTERFLAG==1)
	{
		IDLETIMECOUNTER++;
		if (IDLETIMECOUNTER==IDLETIME)
		{
			SysStatus=LOCKED;
			IDLETIMECOUNTER=0;
			IDLETIMECOUNTERFLAG=0;
			LcdUpdateFlag=LOCKED;
			//USART_WRITE_STRING("IDLE >> LOCKED");
		}
	}
	
}



void setUp(void)
{
	USART_INIT(16);					/* 57600 baud rate					*/
	DDRB=0xff;
	lcd_ini();						/* Initial LCD						*/
	TCCR1B|=(1<<CS12);				/* 265 prescale						*/
	TCCR1B|=(1<<ICNC1);				/* noice canceller					*/
	TCCR1B&=~(1<<ICES1);			/* negative edge select				*/
	DDRD&=~(1<<PD6);				/* Input capture pin (input)		*/
	DDRC|=(1<<LASERPIN);			/* Laser pin out					*/
	DDRC|=(1<<SERINEPIN);			/*Serine pin out					*/
	TIMSK|=(1<<TOIE1);				/* Timer overflow interrupt			*/
	TIMSK|=(1<<TICIE1);				/* input capture interrupt			*/
	//GICR|=(1<<INT0);				/*External interrupt INT0 enable	*/
	MCUCR|=(1<<ISC00)|(1<<ISC01);	/*Interrupt executed on rising edge	*/
	sei();							/* set global interrupt				*/
	
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
			*IrCommandBuffer=0;
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
		*Iraddress=(char)((*IrCommandBuffer)>>16);
		if (*Iraddress==DEVICEADDRESS)
		{
			*IrCommand=(char)(*IrCommandBuffer);
			*NewIrCommandFlag=1;
		}
		CurrentBitPosition=32;
		StartBit=0;
		
	}
	
}

void GetPassword(uint8_t* IrAddress,uint8_t* IrCommand,uint8_t* PasswordFlag,uint8_t* Password,uint8_t* LcdFlag)
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
			*LcdFlag=NEWPASSWORD;
			PasswordIndex=0;
			USART_WRITE_STRING(Password);
			
		}
		else if (PasswordIndex<PASSMAXLENGTH)
		{
			
			*LcdFlag=RECIEVINGPASSWORD;
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
				case PLAY:
				Password[PasswordIndex++]='p';
				USART_WRITE_BYTE('p');
				break;
				case BACK:
				Password[--PasswordIndex]='0';
				*LcdFlag=BACKONEDIGIT;
				break;
				
				
			}
			
			
			
			
		}
	
	}	
}

void CheckPassword(uint8_t* Password,uint8_t* PasswordFlag,uint8_t* LcdFlag)
{
	USART_WRITE_STRING("check");
	if (*Password==NULL)
	{
		*PasswordFlag=EMPTYPASSWORD;
		*LcdFlag=EMPTYPASSWORD;
		
	}
	else if(!strcmp(Password,SYSTEMPASSWORD))
	{
		*PasswordFlag=CORRECT;
		*LcdFlag=CORRECT;
		
	}
	else
	{
		*PasswordFlag=INCORRECT;
		*LcdFlag=INCORRECT;
		
	}
	
}


void ExecuteIrCmd(uint8_t* IrCommand,uint8_t* LcdFlag)
{
	static uint8_t CommandStatus=NOCOMMANDSELECTED;
	static uint8_t Command=0;
	if (CommandStatus==NOCOMMANDSELECTED)
	{
		if (*IrCommand==PLAY)
		{
			CommandStatus=COMMANDSELECTED;
			Command=*IrCommand;
			*LcdFlag=COMMANDSELECTED;
		}
		
	}
	else
	{
		if (*IrCommand==POWER)
		{
			CommandStatus=NOCOMMANDSELECTED;
			*LcdFlag=NOCOMMANDSELECTED;
			if (ALARMSYSSTATUS==OFF)
			{
				ALARMSYSSTATUS=ON;
				PORTC=0xff;
			}
			else
			{
				ALARMSYSSTATUS=OFF;
				PORTC=0x00;
			}
			
		}
		else if (*IrCommand==BACK)
		{
			CommandStatus=NOCOMMANDSELECTED;
			*LcdFlag=BACK;
		}
		
	}
	
	
}



void LcdUpdate(uint8_t* LcdUpdateFlag)
{
	
	
	switch(*LcdUpdateFlag)
	{
		case LOCKED:
		//lcd backlight off (case battery )
		LCD_CMD(_LCD_CLEAR);
		LCD_Write_String("Welcome ..");
		break;
		case READY:
		LCD_CMD(_LCD_CLEAR);
		LCD_Write_String("Enter password :");
		_lcd_goto_xy(2,1);
		break;
		case RECIEVINGPASSWORD :
		LCD_Write_Character('*');
		break;
		case BACKONEDIGIT:
		LCD_CMD(_LCD_CURSOR_SHIFT_LIFT);
		LCD_Write_Character(' ');
		LCD_CMD(_LCD_CURSOR_SHIFT_LIFT);
		break;
		case EMPTYPASSWORD :
		LCD_CMD(_LCD_CLEAR);
		LCD_Write_String("Empty password");
		_lcd_goto_xy(2,1);
		break;
		case INCORRECT:
		LCD_CMD(_LCD_CLEAR);
		LCD_Write_String("Incorrect password");
		_lcd_goto_xy(2,1);
		break;
		case CORRECT :
		LCD_CMD(_LCD_CLEAR);
		LCD_Write_String("Select option :");
		break;
		case UNLOCKED:
		LCD_CMD(_LCD_CLEAR);
		LCD_Write_String("Select an option :");
		break;
		case COMMANDSELECTED:
		if (ALARMSYSSTATUS==OFF)
		{
			LCD_CMD(_LCD_CLEAR);
			LCD_Write_String("Switch on  ?");
			_lcd_goto_xy(2,1);
			LCD_Write_String("yes");
			_lcd_goto_xy(2,9);
			LCD_Write_String("no");
		}
		else
		{
			LCD_CMD(_LCD_CLEAR);
			LCD_Write_String("Switch off  ?");
			_lcd_goto_xy(2,1);
			LCD_Write_String("yes");
			_lcd_goto_xy(2,9);
			LCD_Write_String("no");
		}
		break;
		case NOCOMMANDSELECTED:
		if (ALARMSYSSTATUS==ON)
		{
			LCD_CMD(_LCD_CLEAR);
			LCD_Write_String("System started");
		}
		else
		{
			LCD_CMD(_LCD_CLEAR);
			LCD_Write_String("System stopped");
		}
		break;
		case BACK:
		LCD_CMD(_LCD_CLEAR);
		LCD_Write_String("Select option :");
		break;
		
	}
	*LcdUpdateFlag=0;
}


int main(void)
{
	setUp();
	DDRC=0xff;
	//USART_WRITE_STRING("Starting : ");
	
	unsigned long Buffer=0;
	uint8_t IrCmd=0;
	uint8_t IrAddress=0;
	uint8_t NewIrCmdFlag=0;
	uint8_t ReceivedPassword[10]={0};
	uint8_t ReceivedPasswordFlag=AWAITINGPASSWORD;	
	LcdUpdate(&LcdUpdateFlag);
	
	
	
    while(1)
    {
		
		if (NewBitTimeFlag)
		{
			NewBitTimeFlag=0;
			getCode(&NewBitTime,&Buffer,&IrAddress,&IrCmd,&NewIrCmdFlag);
			if (NewIrCmdFlag==1)
			{
				NewIrCmdFlag=0;
				IDLETIMECOUNTERFLAG=1;
				IDLETIMECOUNTER=0;
				if (SysStatus==LOCKED)
				{
					if (IrCmd==POWER)
					{
						SysStatus=READY;
						LcdUpdateFlag=READY;
					}
				} 
				else if (SysStatus==READY)
				{
					GetPassword(&IrAddress,&IrCmd,&ReceivedPasswordFlag,ReceivedPassword,&LcdUpdateFlag);
					
					
					if (ReceivedPasswordFlag==NEWPASSWORD)
					{
						CheckPassword(&ReceivedPassword,&ReceivedPasswordFlag,&LcdUpdateFlag);
						if (ReceivedPasswordFlag==CORRECT) 
						{
							SysStatus=UNLOCKED;
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
					
					ExecuteIrCmd(&IrCmd,&LcdUpdateFlag);
					
				}
				
				
				IrCmd=0;
			}
			
		}
		
		if (LcdUpdateFlag)
		{
			LcdUpdate(&LcdUpdateFlag);
		}
     
    }
}