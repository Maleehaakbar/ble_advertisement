/*header files for enabling stack,populating advertising data and start advertising*/

#include <zephyr/kernel.h>              /*zephyr/incude/zephyr*/
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <dk_buttons_and_leds.h>       /*nrf/include*/

LOG_MODULE_REGISTER(Lesson2_Exercise1, LOG_LEVEL_INF);

#define COMPANY_ID_CODE            0x0059
#define DEVICE_NAME     CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME)-1)
#define USER_BUTTON DK_BTN1_MSK
#define RUN_STATUS_LED DK_LED1
#define RUN_LED_BLINK_INTERVAL 1000

typedef struct adv_mfg_data {
	uint16_t company_code;	    /* Company Identifier Code. */
	uint16_t number_press;      /* Number of times Button 1 is pressed*/
} adv_mfg_data_type;

static const struct bt_le_adv_param *adv_param =
	BT_LE_ADV_PARAM(BT_LE_ADV_OPT_NONE,
	800,
	801,
	NULL);

static adv_mfg_data_type adv_mfg_data = {COMPANY_ID_CODE,0x00};

static const struct bt_data ad[]={                              /*array of advertising packet*/
  BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),              /*Set advertising flags*/
  BT_DATA(BT_DATA_NAME_COMPLETE,DEVICE_NAME, DEVICE_NAME_LEN),  /*Set advertising packet data*/
  BT_DATA(BT_DATA_MANUFACTURER_DATA,(unsigned char *)&adv_mfg_data, sizeof(adv_mfg_data)),
};  

static unsigned char url_data[] ={0x17,'/','/','a','c','a','d','e','m','y','.',
    'n','o','r','d','i','c','s','e','m','i','.',
    'c','o','m'};

static const struct bt_data sd[]={                              /*scan response packet*/
    BT_DATA(BT_DATA_URI, url_data,sizeof(url_data))
};

static void button_changed(uint32_t button_state, uint32_t has_changed)
{
	if (has_changed & button_state & USER_BUTTON) {
	adv_mfg_data.number_press += 1;
	bt_le_adv_update_data(ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));
	}
}

static int init_button(void)
{
	int err;

	err = dk_buttons_init(button_changed);
	if (err) {
		printk("Cannot init buttons (err: %d)\n", err);
	}

	return err;
}

int main(void)
{
    int blink_status=0;
    int err;

    err = dk_leds_init();
    if(err){
        LOG_ERR("leds init failed  (err %d)\n", err);
        return -1;
    }

    err = init_button();
    if (err) {
	printk("Button init failed (err %d)\n", err);
	return -1;
    }

    err=bt_enable(NULL);
    if(err){
        LOG_ERR("Bluetooth init failed (err %d)\n", err);
        return -1;
    }
    LOG_INF(" Bluetooth init");

    err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
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