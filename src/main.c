/*header files for enabling stack,populating advertising data and start advertising*/

#include <zephyr/kernel.h>              /*zephyr/incude/zephyr*/
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <dk_buttons_and_leds.h>       /*nrf/include*/
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/addr.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include "my_lbs.h"


LOG_MODULE_REGISTER(Lesson4_Exercise2, LOG_LEVEL_INF);

#define COMPANY_ID_CODE            0x0059
#define DEVICE_NAME     CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME)-1)
#define USER_BUTTON DK_BTN1_MSK
#define RUN_STATUS_LED DK_LED1
#define RUN_LED_BLINK_INTERVAL 1000
#define CONNECTION_STATUS_LED   DK_LED2
#define USER_LED                DK_LED3
#define USER_BUTTON             DK_BTN1_MSK

struct bt_conn *my_conn = NULL;
static bool app_button_state;

static const struct bt_le_adv_param *adv_param =
	BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE|BT_LE_ADV_OPT_USE_IDENTITY,
	800,
	801,
	NULL);
    
static const struct bt_data ad[]={                              /*array of advertising packet*/
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL|BT_LE_AD_NO_BREDR)),              /*enable discovery mode flag*/
    BT_DATA(BT_DATA_NAME_COMPLETE,DEVICE_NAME, DEVICE_NAME_LEN),  /*Set advertising packet data*/
};  

static const struct bt_data sd[]={                              /*scan response packet*/
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_128_ENCODE(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd123)),
};


static void app_led_cb(bool led_state)
{
	dk_set_led(USER_LED, led_state);
}

static bool app_button_cb(void)
{
	return app_button_state;
}

static struct my_lbs_cb app_callbacks = {
	.led_cb = app_led_cb,
	.button_cb = app_button_cb,
};

static void button_changed(uint32_t button_state, uint32_t has_changed)
{   
	if (has_changed & USER_BUTTON) {
        uint32_t user_button_state = button_state & USER_BUTTON;
		app_button_state = user_button_state ? true : false;
        my_lbs_send_button_state_indicate(user_button_state);
	}
 
}

void on_connected(struct bt_conn *conn, uint8_t err)
{   
    //struct bt_conn_info info;

    if (err) {
        LOG_ERR("Connection error %d", err);
        return;
    }
    LOG_INF("Connected");
    dk_set_led_on(CONNECTION_STATUS_LED);
    
}

void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected. Reason %d", reason);
    dk_set_led_off(CONNECTION_STATUS_LED);
   
}

struct bt_conn_cb connection_callbacks = {
    .connected              = on_connected,
    .disconnected           = on_disconnected,
};

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
    bt_addr_le_t addr;

    err = dk_leds_init();
	if (err) {
		LOG_ERR("LEDs init failed (err %d)\n", err);
		return -1;
	}

    err = init_button();
    if (err) {
        LOG_ERR("Cannot init buttons (err: %d)", err);
        return -1;
    }
    
    err=bt_enable(NULL);
    if(err){
        LOG_ERR("Bluetooth init failed (err %d)\n", err);
        return -1;
    }
    LOG_INF(" Bluetooth init");

    bt_conn_cb_register(&connection_callbacks);

    err = my_lbs_init(&app_callbacks);
    if (err) {
	printk("Failed to init LBS (err:%d)\n", err);
	return -1;
    }

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