// SPDX-License-Identifier: GPL-2.0
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>

/* ########################################################### */
/* #                    Private declarations                 # */
/* ########################################################### */
static int nunchuck_probe(struct i2c_client *client,
                          const struct i2c_device_id *id);
static void nunchuck_remove(struct i2c_client *client);

/* ########################################################### */
/* #                    Public API                           # */
/* ########################################################### */

static const struct of_device_id nunchuck_of_match[] = {
    {.compatible = "nintendo,nunchuck"},
    {},
};
MODULE_DEVICE_TABLE(of, nunchuck_of_match);

static struct i2c_driver nunchuck_i2c_driver = {
    .driver =
        {
            .name = "nunchuck_i2c",
            .of_match_table = nunchuck_of_match,
        },
    .probe = nunchuck_probe,
    .remove = nunchuck_remove,
};
module_i2c_driver(nunchuck_i2c_driver);

MODULE_LICENSE("GPL");

/* ########################################################### */
/* #                    Private API                           # */
/* ########################################################### */
int nunchuck_probe(struct i2c_client *client, const struct i2c_device_id *id) {
  pr_info("%s\n", __func__);

  return 0;
};

void nunchuck_remove(struct i2c_client *client) { pr_info("%s\n", __func__); };
