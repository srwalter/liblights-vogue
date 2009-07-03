
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include "hardware.h"
#include "lights.h"

enum WhichLight {
    BATTERY,
    NOTIFICATION,
    ATTENTION,
    BACKLIGHT,
    BUTTONS,
};

struct vogue_light_t {
    struct light_device_t light;
    enum WhichLight which;
};

static int vogue_lights_close (struct hw_device_t *device)
{
    return 0;
}

static void write_sys (char *file, int val)
{
    int fd = open(file, O_WRONLY);
    char buf[16];
    int rc;

    snprintf(buf, 16, "%d\n", val);

    if (fd < 0)
        goto out;

    do {
        rc = write(fd, buf, strlen(buf));
    } while (rc < 0 && errno == EINTR);

out:
    close (fd);
}

static int vogue_set_light (struct light_device_t *dev,
                            struct light_state_t const* state)
{
    struct vogue_light_t *light = (struct vogue_light_t *)dev;
    int flash = 0;

    system("echo set light >> /sdcard/lights");

    switch (light->which) {
        case BATTERY:
            system("echo battery >> /sdcard/lights");
            if (state->flashMode != LIGHT_FLASH_NONE)
                flash = 6;

            if (state->color & 0xff0000)
                write_sys("/sys/class/leds/red/brightness", 1 | flash);
            else
                write_sys("/sys/class/leds/red/brightness", 0);

            if (state->color & 0xff00)
                write_sys("/sys/class/leds/green/brightness", 1 | flash);
            else
                write_sys("/sys/class/leds/green/brightness", 0);
            break;

        case NOTIFICATION:
            system("echo note >> /sdcard/lights");
            if ((state->color & 0xffffff) == 0)
                write_sys("/sys/class/leds/blue/brightness", 0);
            else
                write_sys("/sys/class/leds/blue/brightness", 1);
            break;

        case ATTENTION:
            break;

        case BUTTONS:
            system("echo buttons >> /sdcard/lights");
            if ((state->color & 0xffffff) == 0)
                write_sys("/sys/class/leds/button-backlight/brightness", 0);
            else
                write_sys("/sys/class/leds/button-backlight/brightness", 1);
            break;

        case BACKLIGHT: {
            unsigned char bright;
            int color = state->color;

            bright = ((77*((color>>16)&0x00ff)) + (150*((color>>8)&0x00ff)) + (29*(color&0x00ff))) >> 8;
            system("echo backlight >> /sdcard/lights");
            write_sys("/sys/class/leds/lcd-backlight/brightness", bright);
            break;
        }
    }

    return 0;
}

static
struct hw_device_t *alloc_vogue_light (const struct hw_module_t *module,
                                       enum WhichLight which)
{
    struct vogue_light_t *light = malloc(sizeof(struct vogue_light_t));

    system("echo alloc >> /sdcard/lights");
    light->light.common.tag = HARDWARE_DEVICE_TAG;
    light->light.common.module = (struct hw_module_t *)module;
    light->light.common.close = vogue_lights_close;
    light->light.set_light = vogue_set_light;
    light->which = which;

    return (struct hw_device_t*)light;
}

static int vogue_lights_open (const struct hw_module_t *module,
                              const char *id,
                              struct hw_device_t **pdev)
{
    struct hw_device_t *dev;
    system("touch /sdcard/lights");

    if (!strcmp("battery", id)) {
        dev = alloc_vogue_light (module, BATTERY);
    } else if (!strcmp("notifications", id)) {
        dev = alloc_vogue_light (module, NOTIFICATION);
    } else if (!strcmp("attention", id)) {
        dev = alloc_vogue_light (module, ATTENTION);
    } else if (!strcmp("backlight", id)) {
        dev = alloc_vogue_light (module, BACKLIGHT);
    } else if (!strcmp("buttons", id)) {
        dev = alloc_vogue_light (module, BUTTONS);
    } else {
        system("echo open fail >> /sdcard/lights");
        *pdev = NULL;
        return -1;
    }

    *pdev = dev;
    return 0;
}

static struct hw_module_methods_t vogue_lights_methods = {
    .open       = vogue_lights_open,
};

struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag        = HARDWARE_MODULE_TAG,
    .id         = "lights",
    .name       = "lights.vogue.so",
    .methods    = &vogue_lights_methods,
};
