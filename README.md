# PS4 Controller to Virtual Keyboard & Mouse – Linux Kernel Module

This project implements a Linux kernel module that remaps a PlayStation 4 (DualShock 4) controller to simulate keyboard and mouse events. Rather than controlling games, the controller can function as a full alternative input device for general computer use.

---

## 🧠 Overview

The kernel module leverages the `input.h` library to:

- Create virtual keyboard and mouse devices in kernel space.
- Remap controller events into standard keyboard and mouse actions.
- Enable the PS4 controller to act as a hybrid input device.

---

## ⚙️ Implementation Details

1. **Initialization**
   - Virtual input devices (keyboard and mouse) are created using `input_dev` structs from `input.h`.
   
2. **Connection**
   - The controller is identified by the name “Wireless Controller” using the `ps4_connect()` function.
   
3. **Event Remapping**
   - The `ps4_event()` function listens to and remaps input events.

### 🔄 Mappings

#### EV_ABS (Axis Events)
- Gyroscope X/Y → Mouse X/Y movement
- D-Pad → Arrow keys

#### EV_KEY (Button Events)
| Controller Button | Remapped To       |
|-------------------|-------------------|
| Cross             | Return            |
| Circle            | Tab               |
| Triangle          | Left Alt          |
| Square            | Left Meta (Win)   |
| R1, R2            | Left click, Right click |
| L1, L2            | Left Shift, Left Ctrl |
| ThumbL, ThumbR    | C, V              |

---

## How the kernel module works

![implementation](https://github.com/user-attachments/assets/a5bc0650-9ca2-4dfb-b9a6-1ea0283ea25a)

Also note that the original inputs from the controller are also passed as is and not masked.


## 📚 References

- [Linux input-event-codes.h](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h)
- [Input event code documentation](https://docs.kernel.org/input/event-codes.html)
- [Input subsystem programming](https://docs.kernel.org/input/input-programming.html)

---

