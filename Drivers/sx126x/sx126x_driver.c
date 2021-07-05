#include "sx126x_driver.h"
#include <stdio.h>
#include <string.h>
#include "board.h"
#include "gpio.h"
#include "delay.h"
#include "timer.h"
#include "radio.h"
#include "sx126x.h"
#include "board-config.h"


#define USE_MODEM_FSK
#define RF_FREQUENCY                                433000000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm

#if defined( USE_MODEM_LORA )

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#elif defined( USE_MODEM_FSK )

#define FSK_FDEV                                    25000     // Hz
#define FSK_DATARATE                                50000     // bps
#define FSK_BANDWIDTH                               50000     // Hz
#define FSK_AFC_BANDWIDTH                           83333     // Hz
#define FSK_PREAMBLE_LENGTH                         5         // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false

#else
    #error "Please define a modem in the compiler options."
#endif

typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
}States_t;

#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 64 // Define the payload size here

const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];

States_t State = LOWPOWER;

int8_t RssiValue = 0;
int8_t SnrValue = 0;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * LED GPIO pins objects
 */
extern Gpio_t Led1;
extern Gpio_t Led2;

extern SX126x_t SX126x;
uint8_t xorbuf[]={0xff,0xe1 ,0x1d ,0x9a ,0xed ,0x85 ,0x33 ,0x24 ,0xea ,0x7a ,0xd2 ,0x39 ,0x70 ,0x97 ,0x57 ,0x0a ,0x54 ,0x7d ,0x2d ,0xd8 ,0x6d ,0x0d ,0xba ,0x8f ,0x67 ,0x59 ,0xc7 ,0xa2 ,0xbf ,0x34 ,0xca ,0x18 ,0x30 ,0x53 ,0x93 ,0xdf ,0x92 ,0xec ,0xa7 ,0x15 ,0x8a ,0xdc ,0xf4 ,0x86 ,0x55 ,0x4e ,0x18 ,0x21 ,0x40 ,0xc4 ,0xc4 ,0xd5 ,0xc6 ,0x91 ,0x8a ,0xcd ,0xe7 };

/**
 * @brief 
 * 
 */
void sx126x_init(void)
{
    // GpioInit(&Led1, LED_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0);
    // GpioInit(&Led2, LED_2, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0);
    

     SpiInit( &SX126x.Spi, SPI_1, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
    SX126xIoInit();
    return ;
    Radio.Init(0);

    Radio.SetChannel(RF_FREQUENCY);

#if defined(USE_MODEM_LORA)

    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

    Radio.SetMaxPayloadLength(MODEM_LORA, BUFFER_SIZE);

#elif defined(USE_MODEM_FSK)

    Radio.SetTxConfig(MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                      FSK_DATARATE, 0,
                      FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, 0, 3000);

    Radio.SetRxConfig(MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                      0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                      0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, true,
                      0, 0, false, true);

    Radio.SetMaxPayloadLength(MODEM_FSK, BUFFER_SIZE);

#else
#error "Please define a frequency band in the compiler options."
#endif
}

/**
 * @brief 
 * 
 */
void sx126x_test(void)
{
    //write register
    char xx[20];
    int a;
    printf("register write and read test\n");
    SX126xWriteRegister(0x06C0, 0x12);
    SX126xWriteRegister(0x06C1, 0x34);
    SX126xWriteRegister(0x06C2, 0x56);
    SX126xWriteRegister(0x06C3, 0x78);

    a=SX126xReadRegister(0x06C0);
    sprintf(xx,"the sync 0 is 0x%02x\n",a);
    printf(xx);

    a=SX126xReadRegister(0x06C1);
    sprintf(xx,"the sync 1 is 0x%02x\n",a);
    printf(xx);

    a=SX126xReadRegister(0x06C2);
    sprintf(xx,"the sync 2 is 0x%02x\n",a);
    printf(xx);

    a=SX126xReadRegister(0x06C3);
    sprintf(xx,"the sync 3 is 0x%02x\n",a);
    printf(xx);
}

/**
 * @brief sx126x tx contiunous wave
 * 
 */
void sx126x_tx_cw(void)
{
    Radio.SetTxContinuousWave(RF_FREQUENCY,TX_OUTPUT_POWER,0);
}

/**
 * @brief sx126x tx packet
 * 
 */
void sx126x_tx_packet(void)
{
    uint16_t irqRegs;
    int i;
    // Buffer[0]=11^0xff;
    // Buffer[0]=10;
    // for (i = 1; i <= 10; i++)
    // {
    //     // Buffer[i]='A'+i;
    //     // Buffer[i]=0x10+i;
    //     // Buffer[i]=Buffer[i]^xorbuf[i];

    // }
    for ( i = 0; i < 7; i++)
    {
        Buffer[i]=0x12+0x10*(i);
        // printf("the buffer[%d] is 0x%02x\n",i,Buffer[i]);
    }
    
    BufferSize=7;
    Radio.Standby();
    
    DelayMs(2);
    irqRegs=SX126xGetIrqStatus();
    SX126xClearIrqStatus(irqRegs);
    Radio.Send( Buffer, BufferSize );
    i=0;
    while (1)
    {
        irqRegs=SX126xGetIrqStatus();
        // printf("the Irq status is 0x%04x\n",irqRegs);
        if((irqRegs&IRQ_TX_DONE) ==IRQ_TX_DONE)
        {
            printf("transmit finished\n");
            GpioToggle(&Led1);
            break;
        }
        else
        {
            // printf("--------\n");
            i++;
            HAL_Delay(10);
            if(i>=200)
            {
                printf("transmit timeout\n");
                break;
            }
        }
    }

}

/**
 * @brief sx126x rx packet
 * 
 */
void sx126x_rx_packet(void)
{
    int i;
    uint16_t irqRegs = SX126xGetIrqStatus();
    uint8_t rxbuf[255];
    
    uint8_t size=0;
    PacketStatus_t radiopkstatus;
    SX126xClearIrqStatus(irqRegs);
    // Radio.Rx(0);
    Radio.RxBoosted(0);
    i=0;
    while (1)
    {
        irqRegs = SX126xGetIrqStatus();

        // printf("the Irq status is 0x%04x\n",irqRegs);
        if((irqRegs&IRQ_RX_DONE)==IRQ_RX_DONE)
        {
            printf("receive packet successfully\n");
            GpioToggle(&Led2);

            SX126xGetPayload( rxbuf, &size , 255 );
            printf("the receive length is %d\n",size);
            // size=20;
            for (i = 0; i < size; i++)
            {
                // rxbuf[i]=rxbuf[i]^xorbuf[i];
                printf("0x%02x ,",rxbuf[i]);
                // printf("%c ",rxbuf[i]);
            }
            // printf("\nthe crc is :");
            // printf("0x%02x ,",rxbuf[11]);
            // printf("0x%02x ,",rxbuf[12]);
            printf("\n");
            SX126xGetPacketStatus(&radiopkstatus);
            printf("the rssi is %d\n", radiopkstatus.Params.Gfsk.RssiAvg);

            break;
        }
        else
        {
            i++;
            HAL_Delay(1);
            if(i>=100)
            {
                printf("receive timeout\n");
                break;
            }
        }
    }
    Radio.Standby();
    HAL_Delay(10);

}

/**
 * @brief crc16 for IBM
 * which can get the same result as si4463
 * the important factor is not reverse,start from the MSB
 * @param data
 * @param length 
 * @return unsigned int 
 */
unsigned int crc16(unsigned char *data, unsigned char length)
{
  int j;
  // unsigned int reg_crc = 0xFFFF;
  unsigned int reg_crc = 0x0000;
  // unsigned int reg_crc = 0x4230;
  while (length--)
  {

    reg_crc ^= (*data++<<8);
    for (j = 0; j < 8; j++)
    {
      if (reg_crc & 0x8000)
      {
        // reg_crc = (reg_crc >> 1) ^ 0xA001;
        reg_crc = (reg_crc << 1) ^ 0x8005;
      }
      else
      {
        reg_crc = reg_crc << 1;
      }
    }
  }
  reg_crc&=0xffff;
  return reg_crc;
}
