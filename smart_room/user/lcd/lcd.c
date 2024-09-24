#include"lcd.h"
// #include "button.h"
#include"esp_log.h"
#include"stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include"event_groups.h"
#include"string.h"
#include "driver/i2c.h"

#define I2C_MASTER_SCL_IO           GPIO_NUM_22      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           GPIO_NUM_21      /*!< GPIO number used for I2C master data  */
#define LCD_I2c_ADDR    0x27
#define I2C_PORT        0x00
#define I2C_MASTER_FREQ_HZ          100000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

#define	MODE_4_BIT		0x28
#define	CLR_SCR			0x01
#define	DISP_ON			0x0C
#define	CURSOR_ON		0x0E
#define	CURSOR_HOME		0x80

#define I2C_LCD_Delay_Ms(u16DelayMs) delay_ms(u16DelayMs)

typedef union
{
    /* data */
    struct
    {
        /* data */ 
        uint8_t RS      :1;
        uint8_t RW      :1;
        uint8_t En      :1;
        uint8_t BL      :1;
        uint8_t data    :4;  
          
    }bit;
    uint8_t byte;
}LCD_i2c_t;

typedef void (*Screen_func_t)(void);

void delay_ms(uint16_t ms)
{
    while(--ms)
    {
        for(uint8_t count = 0; count < 150; count ++)
        {
            asm("NOP");
        }
    }

}

static esp_err_t i2c_master_init(void);
static esp_err_t LCD_i2c_write_byte(uint8_t *data);
static esp_err_t LCD_i2c_register_read(uint8_t *data, size_t len);
void I2C_LCD_FlushVal(void);
void I2C_LCD_Init(void);
void I2C_LCD_WriteCmd(uint8_t u8Cmd);
void I2C_LCD_Write_4bit(uint8_t u8Data);
void LCD_WaitBusy(void);





LCD_i2c_t LCD_code;



void Display_Title(char *sting)
{
	uint8_t  len = strlen(sting);
	I2C_LCD_BackLight(1);
	I2C_LCD_Clear();
	LCD_Gotoxy((9 - (len / 2)), 1);
	I2C_LCD_Puts(sting);
	LCD_Gotoxy(2, 2);

}


void LCD_STR_Number(uint8_t *number, uint8_t length)
{
	for(uint8_t count = 0; count < length; count ++)
	{
		if(number[count] <= 9)
		{
			LCD_Write_Chr(number[count] + 48);
		}
		else if(number[count] > 9 && number[count] < 100)
		{
			LCD_Write_Chr(number[count]/10 + 48);
			LCD_Write_Chr(number[count]%10 + 48);
		}

		
	}
}

void I2C_LCD_FlushVal(void)
{
	LCD_i2c_write_byte(&LCD_code.byte);
}

void I2C_LCD_Init(void)
{
	ESP_ERROR_CHECK(i2c_master_init());
	LCD_code.byte = 0;
	// LCD_code.bit.BL = 1;
	I2C_LCD_Delay_Ms(50);

	I2C_LCD_FlushVal();
	
	LCD_code.bit.RS = 0;
	I2C_LCD_FlushVal();
	
	LCD_code.bit.RW = 0;
	I2C_LCD_FlushVal();
	
	I2C_LCD_Write_4bit(0x03);
	I2C_LCD_Delay_Ms(5000);
	
	I2C_LCD_Write_4bit(0x03);
	I2C_LCD_Delay_Ms(500);
	
	I2C_LCD_Write_4bit(0x03);
	I2C_LCD_Delay_Ms(500);
	
	I2C_LCD_Write_4bit(MODE_4_BIT >> 4);
	I2C_LCD_Delay_Ms(100);
	
	I2C_LCD_WriteCmd(MODE_4_BIT);
	I2C_LCD_WriteCmd(DISP_ON);
	// I2C_LCD_WriteCmd(CURSOR_ON);
	I2C_LCD_WriteCmd(CLR_SCR);

    // 	I2C_LCD_WriteCmd(0x02);	// Return home
	// I2C_LCD_WriteCmd(0x28);	// 4 bit mode
	// I2C_LCD_WriteCmd(0x0C);	// Display On , cursor off
	// I2C_LCD_WriteCmd(0x06);	// Increment Cursor (shift cursor to right)
	// I2C_LCD_WriteCmd(0x01); // clear display
    
}

void LCD_Gotoxy (int col, int row)
{
	int pos_Addr;
	if(row == 1) 
	{
		pos_Addr = 0x80 + row - 2 + col;
	}
	else
	{
		pos_Addr = 0x80 | (0x40 + col - 1);
	}
	I2C_LCD_WriteCmd(pos_Addr);
}

void I2C_LCD_WriteCmd(uint8_t u8Cmd)
{
	
	LCD_WaitBusy();



	LCD_code.bit.RS = 0;
	I2C_LCD_FlushVal();
	
	LCD_code.bit.RW = 0;
	I2C_LCD_FlushVal();
	
	I2C_LCD_Write_4bit(u8Cmd >> 4);
	I2C_LCD_Write_4bit(u8Cmd);
}

void I2C_LCD_Write_4bit(uint8_t u8Data)
{
	//4 bit can ghi chinh la 4 5 6 7
	//dau tien gan LCD_E=1
	//ghi du lieu
	//sau do gan LCD_E=0

    // LCD_code.byte = 0;

    LCD_code.bit.data = u8Data & 0x0F;

    // printf("lcd cmd %d \n", LCD_code.byte);
    LCD_code.bit.En = 1;
    I2C_LCD_FlushVal();

    LCD_code.bit.En = 0;
    I2C_LCD_FlushVal();
	
}

void LCD_WaitBusy(void)
{
	uint8_t temp = 0x80;
	
	//dau tien ghi tat ca 4 bit thap bang 1
    LCD_code.bit.data = 0x0F;
	I2C_LCD_FlushVal();

	LCD_code.bit.RS = 0;
	I2C_LCD_FlushVal();
	
	LCD_code.bit.RW = 1;
	I2C_LCD_FlushVal();
	
	do {
		LCD_code.bit.En = 1;
		I2C_LCD_FlushVal();
		LCD_i2c_register_read(&temp, 1);
		LCD_code.bit.En = 0;
		I2C_LCD_FlushVal();
		LCD_code.bit.En = 1;
		I2C_LCD_FlushVal();
		LCD_code.bit.En = 0;
		I2C_LCD_FlushVal();
	} while (temp & 0x08);
}



void LCD_Write_Chr(char chr)
{
	
	LCD_WaitBusy();

    // printf(" no busy \n");

	LCD_code.bit.RS = 1;
	I2C_LCD_FlushVal();
	LCD_code.bit.RW = 0;
	I2C_LCD_FlushVal();
	I2C_LCD_Write_4bit(chr >> 4);
	I2C_LCD_Write_4bit(chr);
	
}

void I2C_LCD_Puts(char *sz)
{
	
	while (1) {
		if (*sz) {
			LCD_Write_Chr(*sz++);
		} else {
			break;
		}
	}
}

void I2C_LCD_Clear(void)
{
	
	I2C_LCD_WriteCmd(CLR_SCR);
}

void I2C_LCD_NewLine(void)
{
	
	I2C_LCD_WriteCmd(0xc0);
}

void I2C_LCD_BackLight(uint8_t u8BackLight)
{
	
	if(u8BackLight) {
		LCD_code.bit.BL = 1;
	} else {
		LCD_code.bit.BL = 0;
	}
	I2C_LCD_FlushVal();
}


static esp_err_t LCD_i2c_write_byte(uint8_t *data)
{
    int ret = 0;
    // uint8_t write_buf = data;

    ret = i2c_master_write_to_device(I2C_PORT, LCD_I2c_ADDR, data, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    // printf("i2c return %d \n", ret);
    return ret;
}

static esp_err_t LCD_i2c_register_read(uint8_t *data, size_t len)
{
    return i2c_master_read_from_device(I2C_PORT, LCD_I2c_ADDR, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_PORT;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}
