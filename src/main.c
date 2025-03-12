/*header files for enabling stack,populating advertising data and start advertising*/

#include <zephyr/kernel.h>              /*zephyr/incude/zephyr*/
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <dk_buttons_and_leds.h>       /*nrf/include*/
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/addr.h>
#include <zephyr/bluetooth/conn.h>

LOG_MODULE_REGISTER(Lesson3_Exercise1, LOG_LEVEL_INF);

#define COMPANY_ID_CODE            0x0059
#define DEVICE_NAME     CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME)-1)
#define USER_BUTTON DK_BTN1_MSK
#define RUN_STATUS_LED DK_LED1
#define RUN_LED_BLINK_INTERVAL 1000
#define CONNECTION_STATUS_LED   DK_LED2

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

void on_connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("Connection error %d", err);
        return;
    }
    LOG_INF("Connected");
  //  my_conn = bt_conn_ref(conn);
    dk_set_led(CONNECTION_STATUS_LED, 1);

}

void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected. Reason %d", reason);
   // bt_conn_unref(my_conn);
    dk_set_led(CONNECTION_STATUS_LED, 1);
   
}
int main(void)
{
    struct bt_conn_cb connection_callbacks = {
        .connected              = on_connected,
        .disconnected           = on_disconnected,
    };
    int blink_status=0;
    int err;
    bt_addr_le_t addr;

    err = dk_leds_init();
	if (err) {
		LOG_ERR("LEDs init failed (err %d)\n", err);
		return -1;
	}
    
    err = bt_addr_le_from_str("FF:EE:DD:CC:BB:AA", "random", &addr);
    if (err) {
        printk("Invalid BT address (err %d)\n", err);
    }

    err = bt_id_create(&addr, NULL);
    if (err < 0) {
        printk("Creating new ID failed (err %d)\n", err);
    }

    err=bt_enable(NULL);
    if(err){
        LOG_ERR("Bluetooth init failed (err %d)\n", err);
        return -1;
    }
    LOG_INF(" Bluetooth init");

    bt_conn_cb_register(&connection_callbacks);

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