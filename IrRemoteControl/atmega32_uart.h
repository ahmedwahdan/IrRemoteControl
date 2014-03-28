#define  USART_CONTROL_STATUS_REGISTER_A             UCSRA
#define  USART_CONTROL_STATUS_REGISTER_B             UCSRB
#define  USART_CONTROL_STATUS_REGISTER_C             UCSRC
#define  USART_DATA_REGISTER						 UDR
#define  USART_TRANSMITTER_ENABLE					 RXEN   // UCSRB
#define  USART_RECIEVER_ENABLE						 TXEN   // UCSRB
#define  USART_TX_COMPLETE_INTERRUPT			   	 TXCIE  // UCSRB
#define  USART_RX_COMPLETE_INTERRUPT				 RXCIE  // UCSRB
#define  USART_DATA_REGISTER_EMPTY_INTERRUPT         UDRIE  // UCSRB
#define  USART_CHARACTER_SIZE_BIT0					 UCSZ0  // UCSRC
#define  USART_CHARACTER_SIZE_BIT1					 UCSZ1  // UCSRC
#define  USART_CHARACTER_SIZE_BIT2					 UCSZ2  // UCSRB
#define  USART_REGISTER_SELECT						 URSEL  // UCSRC
#define  USART_MODE_SELECT							 UMSEL  // UCSRC
#define  USART_PARITY_MODE_BIT0						 UPM0   // UCSRC
#define  USART_PARITY_MODE_BIT1						 UPM1   // UCSRC
#define  USART_STOP_BIT								 USBS   // UCSRC
#define  USART_BAUD_RATE_REGISTER_L					 UBRRL  // UBBRL
#define  USART_BAUD_RATE_REGISTER_H					 UBRRH  // UBBRH
#define  USART_DOUBLE_TRANSMOSSION_SPEED             U2X    // UCSRA
#define  USART_DATA_REGITER_EMPTY					 UDRE	// UCSRA


////////////////////////////////////////////////////////////////////////

void USART_INIT(unsigned char BAUD)
{
	USART_CONTROL_STATUS_REGISTER_B |=(1<<USART_TRANSMITTER_ENABLE)|(1<<USART_RECIEVER_ENABLE)|(1<<USART_RX_COMPLETE_INTERRUPT)|(1<<USART_RX_COMPLETE_INTERRUPT);
	// Enable TX,RX,TX complete interrupt, Rx complete interrupt
	USART_BAUD_RATE_REGISTER_L =BAUD; 
	USART_BAUD_RATE_REGISTER_H =(BAUD>>8);
	
	USART_CONTROL_STATUS_REGISTER_C |=(1<<USART_REGISTER_SELECT)|(1<<USART_CHARACTER_SIZE_BIT0)|(1<<USART_CHARACTER_SIZE_BIT1);
	// Set 8 bit character size, no parity, 1 stop bit
	
}

//////////////////////////////////////////////////////////////////////
void USART_WRITE_BYTE(unsigned char BYTE)
{
	while(!(USART_CONTROL_STATUS_REGISTER_A&(1<<UDRE)));
	UDR=BYTE;
}
///////////////////////////////////////////////////////////////////////
void USART_WRITE_STRING(unsigned char *pointer)
{
	while(*pointer)
	{
		USART_WRITE_BYTE(*pointer);
		pointer++;
	}
}

unsigned char USART_DATA_AVAILABLE()
{
	unsigned char check=0;
   if(USART_CONTROL_STATUS_REGISTER_A & (1<<RXC))
   check=1;
   else
   check=0;

   return check;
}

unsigned char USART_READ_BYTE()
{
	if (USART_DATA_AVAILABLE())
	{
		return UDR;
	}

   
}

void USART_READ_STRING(unsigned char string_long,unsigned char *string_address)
{
	unsigned char i=1;
	for (i=1;i<string_long;i++)
	{
		*string_address=USART_READ_BYTE();
		string_address++;
		
	}
	*string_address=0;
	
}

void USART_WRITE_Float( float float_value)
{
	long int vlaue = 0;
	unsigned int x,y;
	vlaue=(float_value)*100;
	y=vlaue % 100;   // kasr
	x=vlaue / 100;   // sa7e7
	
	USART_WRITE_BYTE((char)x);
	USART_WRITE_BYTE((char)(x>>8));
	USART_WRITE_BYTE((char)y);
	USART_WRITE_BYTE((char)(y>>8));
	
}

float USART_READ_FLoat()
{
	char buffer1[5]={0};
	char buffer2[5]={0};	
	char i=0;
	int x[2]={0};
	float vlaue;
	for (i=0;i<2;i++)
	{
		while(!(USART_DATA_AVAILABLE()))
		{
			
		}
		x[i]=USART_READ_BYTE();
		
	}
	
	
	vlaue=((float)(x[0]))+(((float)x[1])/100);
	
	return vlaue;
}

