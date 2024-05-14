#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/spi/spi.h>

#define PICO_DISPLAY_CLASS_NAME "pico_display"
#define PICO_DISPLAY_DRIVER_NAME "pico_display_drv"

struct pico_display_private_data {
    struct spi_device *spi_device;
    struct cdev cdev;
};

static struct pico_display_drv_private_data {
    struct spi_driver spi_driver;
    struct class *pico_display_class;
    struct file_operations fops;
    dev_t pico_display_base_device_number;
    struct pico_display_private_data pico_display_private_data;
} pico_display_drv_private_data;

ssize_t pico_display_write (struct file *filp, const char __user *buf, size_t count, loff_t *off) {
    char c;
    if (copy_from_user(&c, buf, 1) != 0) {
        return -1;
    }

    struct spi_transfer transfer = {
        .tx_buf = &c,
        .len = 1,
    };

    if (spi_sync_transfer(pico_display_drv_private_data.pico_display_private_data.spi_device, &transfer, 1) < 0) {
        return -1;
    }

    return 1;
}

int pico_display_open (struct inode *inode, struct file *filp) {
    try_module_get(THIS_MODULE);

    return 0;
}

int pico_display_release (struct inode *inode, struct file *filp) {
    module_put(THIS_MODULE);

    return 0;
}

static int pico_display_drv_probe(struct spi_device *spi_device) {
    pr_info("%s called\n", __func__);

    spi_device->max_speed_hz = 1000000;
    spi_device->mode = 0;
    pico_display_drv_private_data.pico_display_private_data.spi_device = spi_device;

    cdev_init(&pico_display_drv_private_data.pico_display_private_data.cdev, &pico_display_drv_private_data.fops);
    if (cdev_add(&pico_display_drv_private_data.pico_display_private_data.cdev, pico_display_drv_private_data.pico_display_base_device_number, 1) < 0) {
        goto cleanup0;
    }
    if (IS_ERR(device_create(pico_display_drv_private_data.pico_display_class, NULL, pico_display_drv_private_data.pico_display_base_device_number, NULL, "pico_display0"))) {
        goto cleanup1;
    }

    return 0;

cleanup1:
    cdev_del(&pico_display_drv_private_data.pico_display_private_data.cdev);
cleanup0:
    return -1;
}

static void pico_display_drv_remove(struct spi_device *spi_device) {
    pr_info("%s called\n", __func__);

    struct pico_display_private_data *pico_display_private_data =
            container_of(&spi_device, struct pico_display_private_data, spi_device);

    device_destroy(pico_display_drv_private_data.pico_display_class,
            pico_display_private_data->cdev.dev);
    cdev_del(&pico_display_private_data->cdev);
}

static const struct of_device_id pico_display_of_match_table[] = {
    { .name = "pico_display", },
    {},
};

static int __init pico_display_drv_init(void) {
    pico_display_drv_private_data = (struct pico_display_drv_private_data) {
        .spi_driver = (struct spi_driver) {
            .probe = pico_display_drv_probe,
            .remove = pico_display_drv_remove,
            .driver = (struct device_driver) {
                .name = PICO_DISPLAY_DRIVER_NAME,
                .owner = THIS_MODULE,
                .of_match_table = pico_display_of_match_table,
            },
        },
        .fops = (struct file_operations) {
            .write = pico_display_write,
            .open = pico_display_open,
            .release = pico_display_release,
        },
    };
    pico_display_drv_private_data.pico_display_class = class_create(PICO_DISPLAY_CLASS_NAME);
    if (IS_ERR(pico_display_drv_private_data.pico_display_class)) {
        goto cleanup0;
    }

    if (alloc_chrdev_region(&pico_display_drv_private_data.pico_display_base_device_number,
            0, 1, PICO_DISPLAY_DRIVER_NAME) < 0) {
        goto cleanup1;
    }

    if (spi_register_driver(&pico_display_drv_private_data.spi_driver) < 0) {
        goto cleanup2;
    }

    return 0;

cleanup2:
    unregister_chrdev_region(pico_display_drv_private_data.pico_display_base_device_number, 1);
cleanup1:
    class_destroy(pico_display_drv_private_data.pico_display_class);
cleanup0:
    return -1;
}

static void __exit pico_display_drv_exit(void) {
    spi_unregister_driver(&pico_display_drv_private_data.spi_driver);
    unregister_chrdev_region(pico_display_drv_private_data.pico_display_base_device_number, 1);
    class_destroy(pico_display_drv_private_data.pico_display_class);
}

module_init(pico_display_drv_init);
module_exit(pico_display_drv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("José Marcelo Marques da Cruz");
