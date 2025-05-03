#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/slab.h>

// TODO:
// init:
// setup virt keyboard: done
// setup virt mouse: done
// setup virt controller handler: done
// handler to "Wireless Controller": done
// disconnect function: done
// assign buttons to NSEW -> alt, return, tab, win: done
// r1 r2 -> leftclick rightclick: done
// l1 l2 -> shift, ctrl: done
// thumbl, thumbr -> c, v for copypaste: done
// joystick right -> mouse movements: not working
// dpad -> arrowkeys: done
// gyro -> mouse movements: done


MODULE_LICENSE("GPL");
MODULE_AUTHOR("annnniiii (f20220211@goa.bits-pilani.ac.in)");
MODULE_DESCRIPTION("Playstation4 controller remaped to mouse movements and keyboard.");

// Virtual keyboard and mouse devices
static struct input_dev *virtual_kbd;
static struct input_dev *virtual_mouse;
static struct input_handler ps4_input_handler;   

// Sensitivity of the gyro mapping to mouse movement (inverse)
#define SENSITIVITY_X 5000
#define SENSITIVITY_Y 5000

static void ps4_event(struct input_handle *handle, unsigned int type, unsigned int code, int value) {
    if (type == EV_KEY) {
        switch (code) {
            case BTN_SOUTH: // X
                input_report_key(virtual_kbd, KEY_ENTER, value);
                input_sync(virtual_kbd);
                break;
            case BTN_EAST: // O
                input_report_key(virtual_kbd, KEY_TAB, value);
                input_sync(virtual_kbd);
                break;
            case BTN_WEST: // Square
                input_report_key(virtual_kbd, KEY_LEFTMETA, value);
                input_sync(virtual_kbd);
                break;
            case BTN_NORTH: // Triangle
                input_report_key(virtual_kbd, KEY_LEFTALT, value);
                input_sync(virtual_kbd);
                break;
            case BTN_TR: // R1
                input_report_key(virtual_mouse, BTN_LEFT, value);
                input_sync(virtual_mouse);
                break;
            case BTN_TR2: // R2
                input_report_key(virtual_mouse, BTN_RIGHT, value);
                input_sync(virtual_mouse);
                break;
            case BTN_TL: // L1
                input_report_key(virtual_kbd, KEY_LEFTSHIFT, value);
                input_sync(virtual_kbd);
                break;
            case BTN_TL2: // L2
                input_report_key(virtual_kbd, KEY_LEFTCTRL, value);
                input_sync(virtual_kbd);
                break;
            case BTN_THUMBL:  // left joystick
                input_report_key(virtual_kbd, KEY_C, value);
                input_sync(virtual_kbd);
                break;
            case BTN_THUMBR:  // right joystick
                input_report_key(virtual_kbd, KEY_V, value);
                input_sync(virtual_kbd);
                break;
            default:
                break;
        }
    } else if (type == EV_ABS) {
        switch (code) {

            // ps4 controller reports dpad buttons as analog.

            case ABS_HAT0X: // Left/Right on DPad
                if (value == 1) { // Right
                    input_report_key(virtual_kbd, KEY_RIGHT, 1);
                    input_sync(virtual_kbd);
                } else if (value == -1) { // Left
                    input_report_key(virtual_kbd, KEY_LEFT, 1);
                    input_sync(virtual_kbd);
                } else { // Neutral
                    input_report_key(virtual_kbd, KEY_RIGHT, 0);
                    input_report_key(virtual_kbd, KEY_LEFT, 0);
                    input_sync(virtual_kbd);
                }
                break;
            case ABS_HAT0Y: // Up/Down on DPad
                if (value == 1) { // Down
                    input_report_key(virtual_kbd, KEY_DOWN, 1);
                    input_sync(virtual_kbd);
                } else if (value == -1) { // Up
                    input_report_key(virtual_kbd, KEY_UP, 1);
                    input_sync(virtual_kbd);
                } else { // Neutral
                    input_report_key(virtual_kbd, KEY_DOWN, 0);
                    input_report_key(virtual_kbd, KEY_UP, 0);
                    input_sync(virtual_kbd);
                }
                break;

            // for some reason ps4 gyro x is actually mouse y and vice versa also, values are reverse 

            case ABS_RX:  // Gyro X axis 
                value = (int)(value/SENSITIVITY_X);
                input_report_rel(virtual_mouse, REL_Y, -value);
                input_sync(virtual_mouse);
                break;
            case ABS_RY:  // Gyro Y axis
                value = (int)(value/SENSITIVITY_Y);
                input_report_rel(virtual_mouse, REL_X, -value);
                input_sync(virtual_mouse);
                break;
            default:
                break;
        }
    }
}

// fetch controller

static int ps4_connect(struct input_handler *handler, struct input_dev *dev,
                       const struct input_device_id *id) {
    struct input_handle *handle;

    if (!dev || !strstr(dev->name, "Wireless Controller"))
        return -ENODEV;

    handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
    if (!handle)
        return -ENOMEM;

    handle->dev = dev;
    handle->handler = handler;
    handle->name = "ps4-remapper";

    input_register_handle(handle);
    input_open_device(handle);

    pr_info("Connected to PS4 Controller\n");
    return 0;
}

static void ps4_disconnect(struct input_handle *handle) {
    input_close_device(handle);
    input_unregister_handle(handle);
    kfree(handle);
    pr_info("PS4 Controller disconnected\n");
}

static const struct input_device_id ps4_ids[] = {
    { .driver_info = 1 },
    {},
};

static struct input_handler ps4_input_handler = {
    .event = ps4_event,
    .connect = ps4_connect,
    .disconnect = ps4_disconnect,
    .name = "ps4-remapper-handler",
    .id_table = ps4_ids,
};

static int __init ps4_init(void) {
    int ret;

    virtual_kbd = input_allocate_device();
    if (!virtual_kbd)
        return -ENOMEM;

    virtual_mouse = input_allocate_device();
    if (!virtual_mouse) {
        input_free_device(virtual_kbd);
        return -ENOMEM;
    }

    // Setup virtual keyboard
    virtual_kbd->name = "Virtual PS4 Keyboard";
    virtual_kbd->evbit[0] = BIT_MASK(EV_KEY);
    virtual_kbd->keybit[BIT_WORD(KEY_ENTER)] |= BIT_MASK(KEY_ENTER);
    virtual_kbd->keybit[BIT_WORD(KEY_TAB)] |= BIT_MASK(KEY_TAB);
    virtual_kbd->keybit[BIT_WORD(KEY_SPACE)] |= BIT_MASK(KEY_SPACE);
    virtual_kbd->keybit[BIT_WORD(KEY_LEFTSHIFT)] |= BIT_MASK(KEY_LEFTSHIFT);
    virtual_kbd->keybit[BIT_WORD(KEY_LEFTCTRL)] |= BIT_MASK(KEY_LEFTCTRL);
    virtual_kbd->keybit[BIT_WORD(KEY_LEFTALT)] |= BIT_MASK(KEY_LEFTALT);
    virtual_kbd->keybit[BIT_WORD(KEY_UP)] |= BIT_MASK(KEY_UP);
    virtual_kbd->keybit[BIT_WORD(KEY_DOWN)] |= BIT_MASK(KEY_DOWN);
    virtual_kbd->keybit[BIT_WORD(KEY_LEFT)] |= BIT_MASK(KEY_LEFT);
    virtual_kbd->keybit[BIT_WORD(KEY_RIGHT)] |= BIT_MASK(KEY_RIGHT);
    virtual_mouse->keybit[BIT_WORD(BTN_LEFT)] |= BIT_MASK(BTN_LEFT);
    virtual_kbd->keybit[BIT_WORD(KEY_C)] |= BIT_MASK(KEY_C);
    virtual_kbd->keybit[BIT_WORD(KEY_V)] |= BIT_MASK(KEY_V);
    virtual_kbd->keybit[BIT_WORD(KEY_LEFTMETA)] |= BIT_MASK(KEY_LEFTMETA);


    // Setup virtual mouse
    virtual_mouse->name = "Virtual PS4 Mouse";
    virtual_mouse->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL);
    virtual_mouse->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y);
    virtual_mouse->keybit[BIT_WORD(BTN_LEFT)] |= BIT_MASK(BTN_LEFT);
    virtual_mouse->keybit[BIT_WORD(BTN_RIGHT)] |= BIT_MASK(BTN_RIGHT);

    ret = input_register_device(virtual_kbd);
    if (ret) {
        input_free_device(virtual_kbd);
        input_free_device(virtual_mouse);
        return ret;
    }

    ret = input_register_device(virtual_mouse);
    if (ret) {
        input_unregister_device(virtual_kbd);
        input_free_device(virtual_mouse);
        return ret;
    }

    return input_register_handler(&ps4_input_handler);
}

static void __exit ps4_exit(void) {
    input_unregister_handler(&ps4_input_handler);
    input_unregister_device(virtual_kbd);
    input_unregister_device(virtual_mouse);
}

module_init(ps4_init);
module_exit(ps4_exit);
