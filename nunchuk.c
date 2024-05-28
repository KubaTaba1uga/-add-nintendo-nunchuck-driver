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
static int nunchuk_read_register(struct i2c_client *client);

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

  input_dev_instance = devm_input_allocate_device(&client->dev);
  if (!input_dev_instance)
    return -ENOMEM;

  err = nunchuk_init(client);
  if (err) {
    return err;
  }

  err = input_register_device(input_dev_instance);
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

int nunchuk_read_register(struct i2c_client *client) {
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

  /* For 6 byte of read value: */
  /* • bit 0 == 0 means that Z is pressed. */
  /* • bit 0 == 1 means that Z is released. */
  /* • bit 1 == 0 means that C is pressed. */
  /* • bit 1 == 1 means that C is released. */
  pr_info("Z button (%s), C button (%s)\n",
          (buf[5] & BIT(0)) == 0 ? "Pressed" : "Released",
          (buf[5] & BIT(1)) == 0 ? "Pressed" : "Released");

  return 0;
}
