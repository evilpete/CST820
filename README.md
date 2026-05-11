
# CST820

An Arduino library for the CST820 capacitive touch screen 

includes "Down", "Up", "Contact" event type (still trying ti get gestures working).

interface mostly the same as [fbiego/CST816S](https://github.com/fbiego/CST816S)
(on CYD hardware it seems interrupts are broken or not supported)

- **`bool available();`**  
  Return to booliean data is avalible,
  loads current values into public class attribute '[data](https://github.com/evilpete/CST820/blob/626291d09e740522f69bfe11b67e9b275f16c029/CST820.h#L26)'

- **`void disable_auto_sleep();`**  
  Disables auto sleep, keeping the device active indefinitely.

- **`void enable_auto_sleep();`**  
  Re-enables auto sleep with the current timeout (default is 2 seconds).

- **`void enable_double_click();`**
  to enable double-tap

- **`void disable_double_click();`**
  to disable double-tap
