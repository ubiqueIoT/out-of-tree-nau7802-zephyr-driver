#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>

/* this is for getting the SENSOR_CHAN_FORCE*/
#include "../drivers/sensor/nau7802/nau7802.h"

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

LOG_MODULE_REGISTER(main_logging);

static struct bt_uuid_128 service_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0ULL));

static struct bt_uuid_128 char_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef1ULL));

static ssize_t read(struct bt_conn *conn,
					const struct bt_gatt_attr *attr,
					void *buf, uint16_t len, uint16_t offset);

static void ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value);

static uint32_t current_value = 0;

/* GATT service definition */
BT_GATT_SERVICE_DEFINE(custom_svc,
					   BT_GATT_PRIMARY_SERVICE(&service_uuid.uuid),

					   /* Add NOTIFY to properties */
					   BT_GATT_CHARACTERISTIC(&char_uuid.uuid,
											  BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
											  BT_GATT_PERM_READ,
											  read, NULL, &current_value),

					   /* CCC descriptor so the central can subscribe to notifications */
					   BT_GATT_CCC(ccc_cfg_changed, (BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)));

/* Advertising data: flags + our custom service UUID */
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

/* Read callback: return current value */
static ssize_t read(struct bt_conn *conn,
					const struct bt_gatt_attr *attr,
					void *buf, uint16_t len, uint16_t offset)
{
	return bt_gatt_attr_read(conn, attr, buf, len, offset,
							 attr->user_data,
							 sizeof(current_value));
}

static void ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ARG_UNUSED(attr);
	bool notify_enabled = (value == BT_GATT_CCC_NOTIFY);
	LOG_INF("Notify %s.\n", (notify_enabled ? "enabled" : "disabled"));
}

void notify_value(uint32_t value)
{
	int err = bt_gatt_notify(NULL, &custom_svc.attrs[2],
							 &value,
							 sizeof(value));
	if (err)
	{
		LOG_WRN("bt_gatt_notify failed (err %d), enable notifications\n", err);
	}
}

int main(void)
{
	const struct device *const nau7802 = DEVICE_DT_GET_ONE(nuvoton_nau7802);
	struct sensor_value sample;
	int err;

	if (!device_is_ready(nau7802))
	{
		LOG_ERR("sensor: device not ready.");
		return -1;
	}
	err = bt_enable(NULL);
	if (err)
	{
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return -1;
	}
	err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err)
	{
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return -1;
	}

	while (1)
	{
		if (sensor_sample_fetch(nau7802) < 0)
		{
			LOG_ERR("sensor: sample fetch error.");
		}
		if (sensor_channel_get(nau7802, SENSOR_CHAN_FORCE, &sample) < 0)
		{
			LOG_ERR("sensor: channel get error.");
		}
		else
		{
			current_value = sample.val1 < 0 ? -sample.val1 : sample.val1;
			notify_value(current_value);

			LOG_INF("Force: %d N", current_value);
		}
		k_sleep(K_MSEC(1000));
	}
	return 0;
}
