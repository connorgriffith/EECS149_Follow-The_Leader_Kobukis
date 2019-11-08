// BLE RX app
//
// Receives BLE advertisements with data

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "app_util.h"
#include "simple_ble.h"



#include "app_error.h"
#include "app_timer.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_spi.h"

#include "buckler.h"
#include "display.h"
#include "kobukiActuator.h"
#include "kobukiSensorPoll.h"
#include "kobukiSensorTypes.h"
#include "kobukiUtilities.h"
#include "mpu9250.h"


// BLE configuration
// This is mostly irrelevant since we are scanning only
static simple_ble_config_t ble_config = {
        // BLE address is c0:98:e5:49:00:00
        .platform_id       = 0x49,    // used as 4th octet in device BLE address
        .device_id         = 0x0005,  // Last two octets of device address
        .adv_name          = "EE149", // irrelevant in this example
        .adv_interval      = MSEC_TO_UNITS(100, UNIT_0_625_MS), // send a packet once per second (minimum is 20 ms)
        .min_conn_interval = MSEC_TO_UNITS(50, UNIT_1_25_MS), // irrelevant if advertising only
        .max_conn_interval = MSEC_TO_UNITS(80, UNIT_1_25_MS), // irrelevant if advertising only
};
simple_ble_app_t* simple_ble_app;
NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);

const uint8_t LEADER_address[6] = {0xAA, 0xAA, 0x49, 0xE5, 0x98, 0xC0};
// TODO: implement BLE advertisement callback
void ble_evt_adv_report(ble_evt_t const* p_ble_evt) {
  uint8_t* address = p_ble_evt->evt.gap_evt.params.adv_report.peer_addr.addr;
  if (address[0] == 0xAA && address[1] == 0xAA && address[5] == 0xC0 && address[3] == 0xE5) {
    //printf("%x:%x:%x:%x:%x:%x\n", address.addr[0], address.addr[1], address.addr[2], address.addr[3], address.addr[4], address.addr[5]);
      //ble_data_t data = p_ble_evt->evt.gap_evt.params.adv_report.data;
      // uint8_t* data_addr = data.p_data;
      // uint16_t data_len = data.len;
      for (int i = 0; i < 6; i++)
      {
        if (LEADER_address[i] != address[i]) {
          return;
        }
      }

     uint8_t* data = p_ble_evt->evt.gap_evt.params.adv_report.data.p_data;
     //printf("Data: %d\n", *data);
     uint32_t length;
     int8_t* dataPtr;
     bool flag = false;

     for (int i = 0; i < 31;)
     {
       length = data[i];
       if (data[i + 1] == 0xFF) {
          dataPtr = &data[i + 4];
          flag = true;
          break;
       } 
       i = i + (length + 1);
     }

     if (!flag)
     {
       return;
     } else {
      printf("Length: %d\n", length);
      printf("First int: %d\n", *dataPtr);
      char buf[16];
      snprintf(buf, 16, "%d", *dataPtr);
      //printf("%s\n", dataPtr);
      display_write(buf, DISPLAY_LINE_0);
      //printf("%.*s\n", (length -3), dataPtr);
     }
    }
}

int main(void) {

  // initialize display
  nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);
  nrf_drv_spi_config_t spi_config = {
    .sck_pin = BUCKLER_LCD_SCLK,
    .mosi_pin = BUCKLER_LCD_MOSI,
    .miso_pin = BUCKLER_LCD_MISO,
    .ss_pin = BUCKLER_LCD_CS,
    .irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
    .orc = 0,
    .frequency = NRF_DRV_SPI_FREQ_4M,
    .mode = NRF_DRV_SPI_MODE_2,
    .bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST
  };
  ret_code_t error_code = nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
  APP_ERROR_CHECK(error_code);
  display_init(&spi_instance);
  printf("Display initialized!\n");

    // initialize i2c master (two wire interface)
  nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
  i2c_config.scl = BUCKLER_SENSORS_SCL;
  i2c_config.sda = BUCKLER_SENSORS_SDA;
  i2c_config.frequency = NRF_TWIM_FREQ_100K;
  error_code = nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);
  APP_ERROR_CHECK(error_code);
  mpu9250_init(&twi_mngr_instance);
  printf("IMU initialized!\n");

  // initialize Kobuki
  kobukiInit();
  printf("Kobuki initialized!\n");

  // Setup BLE
  // Note: simple BLE is our own library. You can find it in `nrf5x-base/lib/simple_ble/`
  simple_ble_app = simple_ble_init(&ble_config);
  advertising_stop();

  // TODO: Start scanning
  //BLE Event above will now be called whenever there is an event to decode
  scanning_start();

  while(1) {
    // Sleep while SoftDevice handles BLE
    power_manage();
  }
}


