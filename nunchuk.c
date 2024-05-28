// SPDX-License-Identifier: GPL-2.0
/* ########################################################### */
/* #                    Imports                              # */
/* ########################################################### */
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/module.h>

/* ########################################################### */
/* #                    Private declarations                 # */
/* ########################################################### */
static int nunchuk_probe(struct i2c_client *client,
                         const struct i2c_device_id *id);
static void nunchuk_remove(struct i2c_client *client);
static int nunchuk_init(struct i2c_client *client);
int nunchuk_read_register(struct i2c_client *client, bool *is_z_pressed,
                          bool *is_c_pressed);
static int nunchuk_register_as_input_device(struct i2c_client *client,
                                            struct input_dev **input_dev);
static void nunchuk_poll(struct input_dev *input_dev);

/* ########################################################### */
/* #                    Public API                           # */
/* ########################################################### */
MODULE_LICENSE("GPL");

static const struct of_device_id nunchuk_of_match[] = {
    {.compatible = "nintendo,nunchuk"},
    {},
};
MODULE_DEVICE_TABLE(of, nunchuk_of_match);

static struct i2c_driver nunchuk_i2c_driver = {
    .driver =
        {
            .name = "nunchuk_i2c",
            .of_match_table = nunchuk_of_match,
        },
    .probe = nunchuk_probe,
    .remove = nunchuk_remove,
};
module_i2c_driver(nunchuk_i2c_driver);

/* ########################################################### */
/* #                    Private API                          # */
/* ########################################################### */
int nunchuk_probe(struct i2c_client *client, const struct i2c_device_id *id) {
  struct input_dev *input_dev_instance;
  int err;

  pr_info("%s\n", __func__);

  err = nunchuk_init(client);
  if (err) {
    return err;
  }

  err = nunchuk_register_as_input_device(client, &input_dev_instance);
  if (err) {
    return err;
  }

  return 0;
};

void nunchuk_remove(struct i2c_client *client) { pr_info("%s\n", __func__); };

int nunchuk_init(struct i2c_client *client) {
  char initialization_data[] = {0x55, 0x00};
  char initialization_regs[] = {0xf0, 0xfb};
  char buf[2];
  int bytes_written;

  pr_info("%s\n", __func__);

  for (int i = 0; i < sizeof(initialization_data) / sizeof(char); i++) {

    buf[0] = initialization_regs[i], buf[1] = initialization_data[i];

    bytes_written = i2c_master_send(client, buf, sizeof(buf) / sizeof(char));

    if (bytes_written < sizeof(buf) / sizeof(char)) {
      pr_err("Unable to send initialization data to nunchuk driver\n");
      return -1;
    }

    /* Wait for 1ms between initialization sequences. */
    /* This is nunchuk specification requirement. */
    udelay(1000);
  }

  return 0;
};

int nunchuk_register_as_input_device(struct i2c_client *client,
                                     struct input_dev **input_dev) {
  struct input_dev *input_dev_placeholder;
  int err;

  pr_info("%s\n", __func__);

  /* Allocate input device instance */
  input_dev_placeholder = devm_input_allocate_device(&client->dev);
  if (!input_dev) {
    pr_err("Unable to allocate nunchuk's input device\n");
    return -ENOMEM;
  }

  /* Input device configuration */
  ///* Driver metadata */
  input_dev_placeholder->name = "Wii Nunchuk";
  input_dev_placeholder->id.bustype = BUS_I2C;

  ///* User space key mapping */
  set_bit(EV_KEY, input_dev_placeholder->evbit);
  set_bit(BTN_C, input_dev_placeholder->keybit);
  set_bit(BTN_Z, input_dev_placeholder->keybit);

  ///* Link i2c_client to input_dev_placeholder */
  input_set_drvdata(input_dev_placeholder, client);

  ///* Set up polling */
  input_setup_polling(input_dev_placeholder, nunchuk_poll);
  input_set_poll_interval(input_dev_placeholder, 50);

  /* Register nunchuk as input device */
  err = input_register_device(input_dev_placeholder);
  if (err) {
    pr_err("Unable to register nunchuk as input device\n");
    return err;
  }

  *input_dev = input_dev_placeholder;

  return 0;
}

void nunchuk_poll(struct input_dev *input_dev) {
  struct i2c_client *client = input_get_drvdata(input_dev);
  bool is_z_pressed, is_c_pressed;
  int err;

  err = nunchuk_read_register(client, &is_z_pressed, &is_c_pressed);
  if (err) {
    pr_err("Unable to poll the nunchuk\n");
    return;
  }

  input_report_key(input_dev, BTN_Z, is_z_pressed);
  input_report_key(input_dev, BTN_C, is_c_pressed);
  input_sync(input_dev);

  return;
}

int nunchuk_read_register(struct i2c_client *client, bool *is_z_pressed,
                          bool *is_c_pressed) {
  char buf[6] = {0x00}; // We just reusing one buffer for all operations
  int bytes_amount;

  /* Sleep somwhere between 10ms and 20ms. */
  /* usleep_range is better for performance system than usleep. */
  /* TO-DO why is that? */
  usleep_range(10000, 20000);

  // Set nunchuk in registers reading mode
  bytes_amount = i2c_master_send(client, buf, 1);

  if (bytes_amount != 1) {
    pr_err("Unable to set nunchuk in registers reading mode\n");
    return -1;
  }

  // Read values
  bytes_amount = i2c_master_recv(client, buf, sizeof(buf) / sizeof(char));
  if (bytes_amount != sizeof(buf) / sizeof(char)) {
    pr_err("Unable to read nunchuk's registers\n");
    return -2;
  }

  /* For 6'th byte of read value: */
  /* • bit 0 == 0 means that Z is pressed. */
  /* • bit 0 == 1 means that Z is released. */
  /* • bit 1 == 0 means that C is pressed. */
  /* • bit 1 == 1 means that C is released. */
  *is_z_pressed = (buf[5] & BIT(0)) == 0;
  *is_c_pressed = (buf[5] & BIT(1)) == 0;

  /* pr_info("Z button (%s), C button (%s)\n", */
  /*         is_z_pressed ? "Pressed" : "Released", */
  /*         is_c_pressed ? "Pressed" : "Released"); */

  return 0;
}
