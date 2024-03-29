// Robot Template app
//
// Framework for creating applications that control the Kobuki robot

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_error.h"
#include "app_timer.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_spi.h"

#include "buckler.h"

#define DWM_CS   NRF_GPIO_PIN_MAP(0,18)
#define DWM_SCLK NRF_GPIO_PIN_MAP(0,17)
#define DWM_MOSI NRF_GPIO_PIN_MAP(0,16)
#define DWM_MISO NRF_GPIO_PIN_MAP(0,15)

nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);
nrf_drv_spi_config_t spi_config;

void init(void) {
  spi_config.sck_pin = DWM_SCLK;
  spi_config.mosi_pin = DWM_MOSI;
  spi_config.miso_pin = DWM_MISO;
  spi_config.ss_pin = DWM_CS;
  spi_config.irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY;
  spi_config.orc = 0;
  spi_config.frequency = NRF_DRV_SPI_FREQ_4M;
  spi_config.mode = NRF_DRV_SPI_MODE_0;
  spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;

  APP_ERROR_CHECK(nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL));
}


void spi_transfer(uint8_t* tx_buffer, uint8_t tx_length, uint8_t* rx_buffer) {
  APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_instance, tx_buffer, tx_length, rx_buffer, 2));
  //printf("First poll rx_buffer: %x %x\n", rx_buffer[0], rx_buffer[1]);

  while(rx_buffer[0] == 0x00 || rx_buffer[0] == 0xff) { // && rx_buffer[1] == 0x00) {
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_instance, NULL, 0, rx_buffer, 2));
    //printf("Polling rx_buffer: %x %x\n", rx_buffer[0], rx_buffer[1]);
  }

  //printf("Last polled rx_buffer: %x %x\n", rx_buffer[0], rx_buffer[1]);
  uint8_t rx_buffer_length = rx_buffer[0];
  printf("SIZE: %d\n", rx_buffer_length);
  APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_instance, NULL, 0, rx_buffer, rx_buffer_length));
  for(int k = 0; k < rx_buffer_length; k++) {
    //printf("0x%x ", rx_buffer[k]);
  }

  //printf("\n");
  //printf("rx_buffer: %x %x %x\n", rx_buffer[0], rx_buffer[1], rx_buffer[2]);
  while(rx_buffer[0] == 0x00 || rx_buffer[0] == 0xff) {
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_instance, NULL, 0, rx_buffer, rx_buffer_length));
    //printf("rx_buffer: ");
    for(int k = 0; k < rx_buffer_length; k++) {
      //printf("0x%x ", rx_buffer[k]);
    }
    //printf("\n");
  }
  //printf("\n\n");
}

void nrf_spi_reset(uint8_t* rx_buf) {
  uint8_t reset_byte[1];
  reset_byte[0] = 0xff;
  for(int i = 0; i < 3; i++) {
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_instance, reset_byte, 1, NULL, 0));
  }

  while(rx_buf[0] != 0xff && rx_buf[1] != 0xff) {
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi_instance, NULL, 0, rx_buf, 2));
    nrf_delay_ms(100);
    printf("rx_buf in reset: 0x%x 0x%x\n", rx_buf[0], rx_buf[1]);
  }

  uint8_t dummy_cmd[2] = {0};
  uint8_t dummy_rxbuf[3];
  spi_transfer(dummy_cmd, 2, dummy_rxbuf);
}

void to_little_endian(uint8_t* big_endian_int) {
  uint8_t little_endian_int[4];
  for(int i = 0; i < 4; i++) {
    little_endian_int[i] = big_endian_int[3-i];
  }  

  for(int k = 0; k < 4; k++) {
    big_endian_int[k] = little_endian_int[k];
  }
}

int main(void) {
  // init and nrf_spi_reset work
  ret_code_t error_code = NRF_SUCCESS;
  init();

  uint8_t reset_buf[3];
  reset_buf[0] = 0;
  reset_buf[1] = 0;
  reset_buf[2] = 0;

  nrf_spi_reset(reset_buf);
  nrf_delay_ms(10);

/*
  // Factory reset
  printf("Factory reset\n");
  uint8_t factory_rst_tx[2];
  factory_rst_tx[0] = 0x13;
  factory_rst_tx[1] = 0x00;

  uint8_t factory_rst_rx[3];
  spi_transfer(factory_rst_tx, 2, factory_rst_rx);
*/

/*
  // Tag configuration
  printf("Sending tag configuration...\n");

  uint8_t tag_cfg_tx[4];
  tag_cfg_tx[0] = 0x05;
  tag_cfg_tx[1] = 0x02;
  tag_cfg_tx[2] = 0x62;
  tag_cfg_tx[3] = 0x01;

  printf("Configure buffer: 0x%x 0x%x 0x%x 0x%x\n", tag_cfg_tx[0], tag_cfg_tx[1], tag_cfg_tx[2], tag_cfg_tx[3]);

  uint8_t tag_cfg_rx[3];

  nrf_delay_ms(50);
  spi_transfer(tx_buf, 4, rx_buf);
*/

/*
  // Get node ID
  printf("Getting node ID\n");
  uint8_t get_id_tx[2];
  get_id_tx[0] = 0x30;
  get_id_tx[1] = 0x00;

  uint8_t get_id_rx[13];
  nrf_delay_ms(50);
  spi_transfer(get_id_tx, 2, get_id_rx);
  printf("node id: 0x%llx \n", get_id_rx+5);
*/

/*
  // Module reboot (dwm_reset)
  printf("Rebooting module\n");
  uint8_t reboot_tx[2];
  reboot_tx[0] = 0x14;
  reboot_tx[1] = 0x00;

  uint8_t reboot_rx[3];
  nrf_delay_ms(50);
  spi_transfer(reboot_tx, 2, reboot_rx);
*/

/*
  //Get tag location once and parse entire response
  printf("Getting tag location\n");
  uint8_t get_loc_tx[2];
  get_loc_tx[0] = 0x0C;
  get_loc_tx[1] = 0x00;

  uint8_t get_loc_rx[255];
  nrf_delay_ms(50);
  spi_transfer(get_loc_tx, 2, get_loc_rx); 
 
  nrf_delay_ms(50);
  printf("\n\nfinished transfer...parsing and printing:\n\n");
  printf("\tReturn value response:\t0x%x 0x%x 0x%x\n\n", get_loc_rx[0], get_loc_rx[1], get_loc_rx[2]);


  to_little_endian((uint8_t*) (get_loc_rx + 5));
  to_little_endian((uint8_t*) (get_loc_rx + 9));
  to_little_endian((uint8_t*) (get_loc_rx + 13));
  int32_t* x_pos = (int32_t*) (get_loc_rx + 5);
  int32_t* y_pos = (int32_t*) (get_loc_rx + 9);
  int32_t* z_pos = (int32_t*) (get_loc_rx + 13);
  printf("\tTag position:\n\t\tType: 0x%x,\tLength: %d\n", get_loc_rx[3], get_loc_rx[4]);
  printf("\t\tx: %ld, y: %ld, z: %ld\n", *x_pos, *y_pos, *z_pos);
  printf("\t\tTag position quality factor: %d\n\n", get_loc_rx[17]);

  to_little_endian((uint8_t*) (get_loc_rx + 23));
  int32_t* distance = (int32_t*) (get_loc_rx + 26);
  to_little_endian((uint8_t*) (get_loc_rx + 28));
  to_little_endian((uint8_t*) (get_loc_rx + 32));
  to_little_endian((uint8_t*) (get_loc_rx + 36));
  x_pos = (int32_t*) (get_loc_rx + 28);
  y_pos = (int32_t*) (get_loc_rx + 32);
  z_pos = (int32_t*) (get_loc_rx + 36);
  printf("\tAnchor ranging:\n\t\tType: 0x%x,\tLength: %d\n", get_loc_rx[13], get_loc_rx[19]);
  printf("\t\tNumber of anchors: %d\n", get_loc_rx[20]);
  printf("\t\tAnchor #1 address: %x%x\n", get_loc_rx[21], get_loc_rx[22]);
  printf("\t\tDistance to anchor #1: %ld\n", *distance);
  printf("\t\tDistance quality factor: %d\n", get_loc_rx[27]);
  printf("\t\tAnchor position:\n");
  printf("\t\t\tx: %ld, y: %ld, z: %ld\n", *x_pos, *y_pos, *z_pos);
  printf("\t\tAnchor position quality factor: %d\n\n", get_loc_rx[40]);
*/

/*
  //Continuously get location
  uint8_t get_loc_tx[2];
  get_loc_tx[0] = 0x0C;
  get_loc_tx[1] = 0x00;

  uint8_t get_loc_rx[255];

  while(1) {
    nrf_delay_ms(50);
    spi_transfer(get_loc_tx, 2, get_loc_rx); 
    
    to_little_endian((uint8_t*) (get_loc_rx + 23));
    int32_t* distance = (int32_t*) (get_loc_rx + 26);
    printf("%ld\n", *distance);
  }
*/

/*
  // The 3 transfer calls below toggle pin 13
  uint8_t tx_buf[4];
  tx_buf[0] = 0x28;
  tx_buf[1] = 0x02;
  tx_buf[2] = 0x0D;
  tx_buf[3] = 0x01;

  uint8_t rx_buf[3];
  rx_buf[0] = 0;
  rx_buf[1] = 0;
  rx_buf[2] = 0;

  printf("Sending gpio_cfg_output command...\n");
  spi_transfer(tx_buf, 4, rx_buf);      // This is our call to our transfer function
  nrf_delay_ms(50);
  tx_buf[3] = 0x00;
  spi_transfer(tx_buf, 4, rx_buf);
  nrf_delay_ms(50);
  tx_buf[3] = 0x01;
  spi_transfer(tx_buf, 4, rx_buf);
*/

    // loop forever, running state machine
    while (1) {
        nrf_delay_ms(1);
    }
}

