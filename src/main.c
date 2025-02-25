/*header files for enabling stack,populating advertising data and start advertising*/

#include <zephyr/kernel.h>              /*zephyr/incude/zephyr*/
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <dk_buttons_and_leds.h>       /*nrf/include*/

LOG_MODULE_REGISTER(Lesson2_Exercise1, LOG_LEVEL_INF);

#define DEVICE_NAME     CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME)-1)

#define RUN_STATUS_LED DK_LED1
#define RUN_LED_BLINK_INTERVAL 1000

static const struct bt_data ad[]={                              /*array of advertising packet*/
  BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),              /*Set advertising flags*/
  BT_DATA(BT_DATA_NAME_COMPLETE,DEVICE_NAME, DEVICE_NAME_LEN)  /*Set advertising packet data*/
};  

static unsigned char url_data[] ={0x17,'/','/','a','c','a','d','e','m','y','.',
    'n','o','r','d','i','c','s','e','m','i','.',
    'c','o','m'};

static const struct bt_data sd[]={                              /*scan response packet*/
    BT_DATA(BT_DATA_URI, url_data,sizeof(url_data))
};


int main(void){
    int blink_status=0;
    int err;

    err = dk_leds_init();
    if(err){
        LOG_ERR("leds init failed  (err %d)\n", err);
        return -1;
    }

    err=bt_enable(NULL);
    if(err){
        LOG_ERR("Bluetooth init failed (err %d)\n", err);
        return -1;
    }
    LOG_INF(" Bluetooth init");

    err = bt_le_adv_start(BT_LE_ADV_NCONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
	LOG_ERR("Advertising failed to start (err %d)\n", err);
	return -1;
    }
    LOG_INF(" Advertisitment start");

    for(;;){
        dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
        k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
       
    }
}