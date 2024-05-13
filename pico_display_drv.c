#include <linux/module.h>
#include <linux/spi/spi.h>

static struct pico_display_drv_private_data {
    struct spi_driver spi_driver;
} pico_display_drv_private_data;

static int pico_display_drv_probe(struct spi_device *spi_device) {
    pr_info("%s called\n", __func__);

    return 0;
}

static void pico_display_drv_remove(struct spi_device *spi_device) {
    pr_info("%s called\n", __func__);
}

static const struct of_device_id pico_display_of_match_table[] = {
    { .name = "pico_display", },
    {},
};

static int __init pico_display_drv_init(void) {
    pico_display_drv_private_data.spi_driver = (struct spi_driver) {
        .probe = pico_display_drv_probe,
        .remove = pico_display_drv_remove,
        .driver = (struct device_driver) {
            .name = "pico_display_drv",
            .owner = THIS_MODULE,
            .of_match_table = pico_display_of_match_table,
        },
    };

    spi_register_driver(&pico_display_drv_private_data.spi_driver);

    return 0;
}

static void __exit pico_display_drv_exit(void) {
    spi_unregister_driver(&pico_display_drv_private_data.spi_driver);
}

module_init(pico_display_drv_init);
module_exit(pico_display_drv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("José Marcelo Marques da Cruz");
