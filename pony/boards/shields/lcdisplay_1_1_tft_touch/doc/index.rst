.. _lcdisplay_1_1_tft_touch:

LC-Display 1.1" TFT Touch Shield
#####################################################

Overview
********

The LC-Display 1.1" TFT Touch Shield has a resolution of 240x240
pixels and is based on the CG9A01 controller. This shield comes with
a capacitive CST816D controller touchscreen.

Requirements
************

Programming
***********

Set ``--shield lcdisplay_1_1_tft_touch`` when you invoke
``west build``. For example:

.. zephyr-app-commands::
   :zephyr-app: samples/subsys/display/lvgl
   :board: stm32f429ig_fire1
   :shield: lcdisplay_1_1_tft_touch
   :goals: build

References
**********
