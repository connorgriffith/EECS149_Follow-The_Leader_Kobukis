// 
//
// Use this to test nrf_dwm functions

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf_dwm.h"

#define DWM_CS   NRF_GPIO_PIN_MAP(0,18)
#define DWM_SCLK NRF_GPIO_PIN_MAP(0,17)
#define DWM_MOSI NRF_GPIO_PIN_MAP(0,16)
#define DWM_MISO NRF_GPIO_PIN_MAP(0,15)

nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);
nrf_drv_spi_config_t spi_config;

int main(void) {
  ret_code_t error_code = NRF_SUCCESS;

  // initialize RTT library
  error_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(error_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  printf("Log initialized!\n");

  spi_init(&spi_instance, &spi_config);
  nrf_spi_reset(&spi_instance);
  reboot_node(&spi_instance);
  //get_loc_single(&spi_instance);

  /*
  uint64_t id;
  get_node_id(&spi_instance, &id);
  uint8_t* id_buf = (uint8_t*) &id;
  printf("Node ID: ");
  for(int i = 0; i < 8; i++) {
    printf("0x%x ", id_buf[i]);
  }
  printf("\n");
  */

  float distance = get_dist_m(&spi_instance);
  printf("Calculated Distance: %f\n\n", distance); 
  while (1) {
      nrf_delay_ms(1);
  }
}

