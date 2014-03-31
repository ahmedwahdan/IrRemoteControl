/**************************************************
 	generic lcd library
	auther : Eng: Ahmed Wahdan 
	Email  : Ahmedibrahimwahdan@gmail.com
	20/10/2012
******************************************************************/
	#define LCD_PORT      PORTB
	#define LCD_RS        PB6
	#define LCD_RW        PB5
	#define LCD_EN        PB4
	#define LCD_D4        PB3
	#define LCD_D5        PB2
	#define LCD_D6        PB1
	#define LCD_D7        PB0

/*****************************************************************/

	#define  _LCD_CLEAR					0x01
	#define  _LCD_RETURN_HOME			0x80
	#define  _LCD_CURSOR_OFF			0x0C
	#define  _LCD_UNDERLINE_ON			0x0E
	#define  _LCD_BLINK_CURSOR_ON		0x0F
	#define  _LCD_TURN_ON				0x0C
	#define  _LCD_TURN_OFF				0x08
	#define  _LCD_SHIFT_LEFT			0x18
	#define  _LCD_SHIFT_RIGHT			0x1C
	#define  _LCD_CURSOR_SHIFT_LIFT		0x10
	#define  _LCD_CURSOR_SHIFT_RIGHT	0x14
	unsigned cursor_position=128;	

	







/*****************************************************************************/
	void LCD_READ()
		{
		LCD_PORT|=(1<<LCD_EN);       // set enable high to read
		_delay_us(100);
		LCD_PORT&=~(1<<LCD_EN);		// reset enable
		_delay_ms(5);     			// delay 5 ms 
		}

/****************************************************************************/
	void LCD_Write_4bit_mode(unsigned char value)
	{
	if (value&0x80)  LCD_PORT|=(1<<LCD_D7); else LCD_PORT&=~(1<<LCD_D7);
	if (value&0x40)  LCD_PORT|=(1<<LCD_D6); else LCD_PORT&=~(1<<LCD_D6);
	if (value&0x20)  LCD_PORT|=(1<<LCD_D5); else LCD_PORT&=~(1<<LCD_D5);
	if (value&0x10)  LCD_PORT|=(1<<LCD_D4); else LCD_PORT&=~(1<<LCD_D4);
	LCD_READ();
	if (value&0x08)  LCD_PORT|=(1<<LCD_D7); else LCD_PORT&=~(1<<LCD_D7);
	if (value&0x04)  LCD_PORT|=(1<<LCD_D6); else LCD_PORT&=~(1<<LCD_D6);
	if (value&0x02)  LCD_PORT|=(1<<LCD_D5); else LCD_PORT&=~(1<<LCD_D5);
	if (value&0x01)  LCD_PORT|=(1<<LCD_D4); else LCD_PORT&=~(1<<LCD_D4);
	LCD_READ();
	}
//////////////////////////////////////////////////////////
	void LCD_CMD(unsigned char command)
	{
	LCD_PORT &=~(1<<LCD_RS)&~(1<<LCD_RW); // write command ( not generic)
	LCD_Write_4bit_mode(command);
	}
/////////////////////////////////////////////////////////////
	void check_position()
	{
	if(cursor_position==143)
	{
		_lcd_goto_xy(2,1);
		cursor_position=192;
	}
	else if(cursor_position==207)
	{
		_lcd_goto_xy(1,1);
		cursor_position=128;
	}
	
	}

/////////////////////////////////////////////////////////////
void _lcd_goto_xy(unsigned char row,unsigned char position)
{
	if (row==1)
	{
		LCD_CMD(127+position);
		cursor_position=(127+position);
	}
	
	else if(row==2)
	{
		LCD_CMD(191+position);
		cursor_position=(191+position);
	}
	
}
///////////////////////////////////////////////////////
void LCD_Write_Character( unsigned char  character)
{
	LCD_PORT &=~(1<<LCD_RW); 
	LCD_PORT |=(1<<LCD_RS); // write data ( not generic)
	LCD_Write_4bit_mode(character);
	cursor_position++;
	//check_position();
	
}
//////////////////////////////////////////////
void LCD_Write_String( unsigned char  *pointer)
	{
		while(*pointer)
		{
			LCD_Write_Character(*pointer);
			pointer++;
		}		
	}
	
////////////////////////////////////////////////

void LCD_Write_float( float  float_value)
{
	long int vlaue = 0;
	unsigned int x,y;
	char buffer_1[5]={0};
	char buffer_2[5]={0};
	vlaue=(float_value)*100;
	y=vlaue % 100;   // kasr
	x=vlaue / 100;   // sa7e7
	
	itoa(x,buffer_1,10);
	itoa(y,buffer_2,10);
	LCD_Write_String(buffer_1);
	LCD_Write_Character('.');
	if (y<10)
	{
		LCD_Write_Character('0');
	}
	LCD_Write_String(buffer_2);
	
}

////////////////////////////////////////////////////////	
void lcd_ini(void)
{
	_delay_ms(15);	
	LCD_PORT &=~(1<<LCD_RS);					//Write intructions
												// 15 ms power on delay
	LCD_PORT|=(1<<LCD_D4)|(1<<LCD_D5);			//intial    0x30
	LCD_READ();									// reading
	LCD_READ();									// reading
	LCD_READ();									// reading
	
	LCD_PORT &=~(1<<LCD_D4);
	
	LCD_READ();									// reading
		
	LCD_Write_4bit_mode(0x28);					// Function Set: 4-bit, 2 Line, 5x7 Dots
	
	LCD_Write_4bit_mode(0x0c);					 //display on ,cursor off	
	
	LCD_Write_4bit_mode(0x06);					//entry mode (advanced cursor)
	
	LCD_Write_4bit_mode(0x01);					//clear display , cursor home	
}


////////////////////////////////////////////////////////////

