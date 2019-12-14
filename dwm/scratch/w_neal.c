#include "nrf_drv_spi.h"
#include "nrf_drv_gpiote.h"
//#include "dwm_api.h"
//#include "dwm1001_tlv.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include <stdlib.h>


#define SPI_BITS                 8
#define SPI_SPEED                4000000
#define SPI_DELAY                0

#define SPI_CS      NRF_GPIO_PIN_MAP(0,18)
#define SPI_SCLK    NRF_GPIO_PIN_MAP(0,17)
#define SPI_MISO    NRF_GPIO_PIN_MAP(0,15)
#define SPI_MOSI    NRF_GPIO_PIN_MAP(0,16)

/*
#define SPI_SCLK BUCKLER_LCD_SCLK
#define SPI_MISO BUCKLER_LCD_MISO
#define SPI_MOSI BUCKLER_LCD_MOSI
*/

#define TLV_MAX_SIZE 255
#define TLV_TYPE_CFG_TN_SET 5

static const nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);
static nrf_drv_spi_config_t spi_config;

int init(void) {
  spi_config.sck_pin    = SPI_SCLK;
  spi_config.miso_pin   = SPI_MISO;
  spi_config.mosi_pin   = SPI_MOSI;
  spi_config.ss_pin     = SPI_CS;
  spi_config.frequency  = NRF_DRV_SPI_FREQ_4M;
  spi_config.mode       = NRF_DRV_SPI_MODE_1;
  spi_config.bit_order  = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST; 
  spi_config.irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY;
  spi_config.orc = 0;
  return nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
}

void shift_bytes(uint8_t* buffer, uint8_t length) {
  for (int i = 0; i < length; i++) {
    buffer[i] = buffer[i] << 1;
  }
}

void nrf_spi_reset(void) {
  uint8_t reset_byte[1];
  reset_byte[0] = 0xff;
  for (int i = 0; i < 3; i++) {
    nrf_drv_spi_transfer(&spi_instance, reset_byte, 1, NULL, 0);
    printf("Sent %d reset byte(s)...\n", i);
  }

  /*
  while (reset_response[0] == 0x00 && reset_response[1] == 0x00) {
    nrf_drv_spi_transfer(&spi_instance, NULL, 0, reset_response, 2);
    nrf_delay_ms(100); 
  }
*/
  return;
}

/* Poll for 2
   Check
   Get NOT 255
   Store those 2 (received_bytes)
   call transfer(..,..,.., rx_buffer, received_bytes[1])  
*/
uint8_t* spi_transfer(uint8_t* tx_buffer, uint8_t* rx_buffer, uint8_t tx_length) {
  nrf_spi_reset();

  uint8_t resp[2] = {0};
  nrf_drv_spi_transfer(&spi_instance, tx_buffer, tx_length, NULL, 0);
  while (resp[0] == 0x00 || resp[0] == 0xff) { 
    nrf_drv_spi_transfer(&spi_instance, NULL , 0 , resp, 2);
    nrf_delay_ms(10);
    printf("Polling 2 bytes...\n");
  }
  
  uint8_t rx_buffer_length = resp[0];

  printf("Size:\t%x\n", rx_buffer_length);
  printf("Num:\t%d\n\n", resp[1]);

  nrf_drv_spi_transfer(&spi_instance, NULL, 0, rx_buffer, rx_buffer_length);
  nrf_delay_ms(10);
  
  while(rx_buffer[0] == 0x00 || rx_buffer[0] == 0xff) {
    for(int k = 0; k < rx_buffer_length; k++) {
      printf("RX_buffer[%d]:\t%x\n", k, rx_buffer[k]);
    }

    printf("Waiting for response...\n");
    nrf_drv_spi_transfer(&spi_instance, NULL, 0, rx_buffer, rx_buffer_length);
    nrf_delay_ms(10);
  }

  for(int i = 0; i < tx_length; i++) {
    printf("TX_buffer[%d]:\t%x\n", i, tx_buffer[i]);
  }

  printf("\n");

  for(int k = 0; k < rx_buffer_length; k++) {
    printf("RX_buffer[%d]:\t%x\n", k, rx_buffer[k]);
  }
  
  return rx_buffer;
}

int main(void) { 
  init();  
   
  //uint8_t reset_request[2];
  //reset_request[0] = 0x14;
  //reset_request[1] = 0x00;
  //// shift_bytes(reset_request, 2);  

  //uint8_t reset_response[3];
  //spi_transfer(reset_request, reset_response, 2);  

  uint8_t cfg_tx_buffer[4];
  cfg_tx_buffer[0] = 0x28;
  cfg_tx_buffer[1] = 0x02;
  cfg_tx_buffer[2] = 0x0D;
  cfg_tx_buffer[3] = 0x01; 
  // shift_bytes(cfg_tx_buffer, 4);

  uint8_t cfg_rx_buffer[3];
  spi_transfer(cfg_tx_buffer, cfg_rx_buffer, 4);
 /* 
  uint8_t reset_request[2];
  reset_request[0] = 0x14;
  reset_request[1] = 0x00;
  // shift_bytes(reset_request, 2);  

  uint8_t reset_response[3];
  spi_transfer(reset_request, reset_response, 2);
   */ 
  // uint8_t cfg_tx_buffer[4];
  // cfg_tx_buffer[0] = TLV_TYPE_CFG_TN_SET;
  // cfg_tx_buffer[1] = 2;
  // cfg_tx_buffer[2] = 0x62;
  // cfg_tx_buffer[2] = 0x62;
  // cfg_tx_buffer[3] = 0; 
  // shift_bytes(cfg_tx_buffer, 4);

  // uint8_t cfg_rx_buffer[TLV_MAX_SIZE];
  // cfg_rx_buffer[0] = 0x00;
  // spi_transfer(cfg_rx_buffer, cfg_rx_buffer, 4);
  // uint8_t data[2];
  // data[0] = 0x14;
  // data[1] = 0x00;
  // shift_bytes(data, 2);
  // ret_code_t err_code = nrf_drv_spi_transfer(&spi_instance, data, 2, NULL, 0);
  // printf("First transfer passed\n");
  // APP_ERROR_CHECK(err_code);
  // if (err_code != NRF_SUCCESS) {
  //   return false;  
  // }
  // uint8_t size_num[2];
  // err_code = nrf_drv_spi_transfer(&spi_instance, NULL, 0, size_num, 2);
  // printf("Second transfer passed.\n");
  // while (size_num[0] == 0x00) {
  //   APP_ERROR_CHECK(err_code);
  //   if (err_code != NRF_SUCCESS) {
  //     return false;
  //   }
  //   nrf_delay_ms(10);
  //   err_code = nrf_drv_spi_transfer(&spi_instance, NULL, 0, size_num, 2);
  // }
  // printf("%x %x\n", size_num[0], size_num[1]);
  // uint8_t* readData = (uint8_t *)malloc(sizeof(uint8_t)*size_num[0]);
  // err_code = nrf_drv_spi_transfer(&spi_instance, NULL, 0, readData, size_num[0]);
  // APP_ERROR_CHECK(err_code);
  // if (err_code != NRF_SUCCESS) {
  //   return false;
  // }

/*
  uint8_t cfg_tx_buffer[4];
  cfg_tx_buffer[0] = TLV_TYPE_CFG_TN_SET;
  cfg_tx_buffer[1] = 2;
  cfg_tx_buffer[2] = 0x62;
  cfg_tx_buffer[3] = 0; 
  //shift_bytes(cfg_tx_buffer, 4);

  uint8_t cfg_rx_buffer[3];
  //cfg_rx_buffer[0] = 0x00;
  spi_transfer(cfg_tx_buffer, cfg_rx_buffer, 4);


  uint8_t reset_request[2];
  reset_request[0] = 0x13;
  reset_request[1] = 0;
  //shift_bytes(reset_request, 2);  

  uint8_t reset_response[3];
  //reset_response[0] = 0x00;
  spi_transfer(reset_request, reset_response, 2);
 
  uint8_t cfg_tx_buffer[4];
  cfg_tx_buffer[0] = TLV_TYPE_CFG_TN_SET;
  cfg_tx_buffer[1] = 2;
  cfg_tx_buffer[2] = 0x62;
  cfg_tx_buffer[2] = 0x62;
  cfg_tx_buffer[3] = 0; 
  shift_bytes(cfg_tx_buffer, 4);

  uint8_t cfg_rx_buffer[TLV_MAX_SIZE];
  cfg_rx_buffer[0] = 0x00;
  spi_transfer(cfg_rx_buffer, cfg_rx_buffer, 4);
*/
}
