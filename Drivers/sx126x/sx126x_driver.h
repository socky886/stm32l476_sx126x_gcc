#ifndef __SX126X_DRIVER__H__
#define __SX126X_DRIVER__H__

void sx126x_init(void);
void sx126x_test(void);
void sx126x_tx_cw(void);
void sx126x_tx_packet(void);
void sx126x_rx_packet(void);
unsigned int crc16(unsigned char *data, unsigned char length);
static void sx126x_io_init(void);
static void sx126x_io_test(void);
#endif