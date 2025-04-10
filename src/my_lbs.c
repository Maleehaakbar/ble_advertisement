/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief LED Button Service (LBS) sample
 */

 #include <zephyr/types.h>
 #include <stddef.h>
 #include <string.h>
 #include <errno.h>
 #include <zephyr/sys/printk.h>
 #include <zephyr/sys/byteorder.h>
 #include <zephyr/kernel.h>
 
 #include <zephyr/bluetooth/bluetooth.h>
 #include <zephyr/bluetooth/hci.h>
 #include <zephyr/bluetooth/conn.h>
 #include <zephyr/bluetooth/uuid.h>
 #include <zephyr/bluetooth/gatt.h>
 #include <zephyr/logging/log.h>
 #include "my_lbs.h"

 
LOG_MODULE_DECLARE(Lesson4_Exercise2);
 
static bool button_state;
static struct my_lbs_cb lbs_cb;
static struct bt_gatt_indicate_params ind_params;
static bool indicate_enabled;


static void mylbsbc_ccc_cfg_changed(const struct bt_gatt_attr *attr,
    uint16_t value)
{
    indicate_enabled = (value == BT_GATT_CCC_INDICATE);
}

// This function is called when a remote device has acknowledged the indication at its host layer
static void indicate_cb(struct bt_conn *conn, struct bt_gatt_indicate_params *params, uint8_t err)
{
	LOG_DBG("Indication %s\n", err != 0U ? "fail" : "success");
}

 /* STEP 6 - Implement the write callback function of the LED characteristic */
static ssize_t write_led(struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    const void *buf,
    uint16_t len, uint16_t offset, uint8_t flags)
{
    LOG_INF("Attribute write, handle: %u, conn: %p", attr->handle,
    (void *)conn);

    if (len != 1U) {
    LOG_DBG("Write led: Incorrect data length");
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    if (offset != 0) {
    LOG_DBG("Write led: Incorrect data offset");
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    if (lbs_cb.led_cb) {
    //Read the received value 
    uint8_t val = *((uint8_t *)buf);

    if (val == 0x00 || val == 0x01) {
   //Call the application callback function to update the LED state
   lbs_cb.led_cb(val ? true : false);
    } else {
    LOG_DBG("Write led: Incorrect value");
    return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
    }
}

return len;
}
 /* STEP 5 - Implement the read callback function of the Button characteristic */
static ssize_t read_button(struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf,
    uint16_t len,
    uint16_t offset)
{
//get a pointer to button_state which is passed in the BT_GATT_CHARACTERISTIC() and stored in attr->user_data
    const char *value = attr->user_data;

    LOG_INF("Attribute read, handle: %u, conn: %p", attr->handle,
    (void *)conn);

    if (lbs_cb.button_cb) {
// Call the application callback function to update the get the current value of the button
    button_state = lbs_cb.button_cb();
    return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
           sizeof(*value));
    }

    return 0;
}
 
 /* LED Button Service Declaration */
 /* Create and add the MY LBS service to the Bluetooth LE stack */

BT_GATT_SERVICE_DEFINE(my_lbs_svc,
BT_GATT_PRIMARY_SERVICE(BT_UUID_LBS),
/*Create and add the Button characteristic */
BT_GATT_CHARACTERISTIC(BT_UUID_LBS_BUTTON,
    BT_GATT_CHRC_READ|BT_GATT_CHRC_INDICATE,
    BT_GATT_PERM_READ, read_button, NULL,
    &button_state),
BT_GATT_CCC(mylbsbc_ccc_cfg_changed,
	BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
/*Create and add the LED characteristic. */
BT_GATT_CHARACTERISTIC(BT_UUID_LBS_LED,
    BT_GATT_CHRC_WRITE,
    BT_GATT_PERM_WRITE,
    NULL, write_led, NULL),
    
    );
 

 /* A function to register application callbacks for the LED and Button characteristics  */

int my_lbs_send_button_state_indicate(bool button_state)
{
    if (!indicate_enabled) {
        return -EACCES;
    }
 
     /* STEP 5.2 - Populate the indication */
    ind_params.attr = &my_lbs_svc.attrs[2];
    ind_params.func = indicate_cb;
    ind_params.destroy = NULL;
    ind_params.data = &button_state;
    ind_params.len = sizeof(button_state);
    return bt_gatt_indicate(NULL, &ind_params);
}

int my_lbs_init(struct my_lbs_cb *callbacks)
{
    if (callbacks) {
        lbs_cb.led_cb = callbacks->led_cb;
        lbs_cb.button_cb = callbacks->button_cb;
        LOG_INF("Register application callbacks");
    }
 
    return 0;
}