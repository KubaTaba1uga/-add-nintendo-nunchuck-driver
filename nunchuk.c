// SPDX-License-Identifier: GPL-2.0
/* ########################################################### */
/* #                    Imports                              # */
/* ########################################################### */
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>

/* ########################################################### */
/* #                    Private declarations                 # */
/* ########################################################### */
static int nunchuk_probe(struct i2c_client *client,
                         const struct i2c_device_id *id);
static void nunchuk_remove(struct i2c_client *client);
static int nunchuk_init(struct i2c_client *client);

/* ########################################################### */
/* #                    Public API                           # */
/* ########################################################### */

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

MODULE_LICENSE("GPL");

/* ########################################################### */
/* #                    Private API                          # */
/* ########################################################### */
int nunchuk_probe(struct i2c_client *client, const struct i2c_device_id *id) {
  pr_info("%s\n", __func__);

  int err;

  err = nunchuk_init(client);

  return err;
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
