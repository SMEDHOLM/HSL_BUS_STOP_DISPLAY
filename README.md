# HSL_BUS_STOP_DISPLAY
This code is for ESP32-2432S028 (microcontroller with display). It turns this microcontroller into HSL's BUS STOP display.
ESP32 connect to the given WIFI and makes HTTP POST request to digitransi.com with persona API key and json response body. As response it gets JSON body that gets displayed.

IMPORTANT:
To make code work download all included libraries in ARDUINO IDE and in TFT_eSPI library replace User_Setup.h with file from this repository.
![HSL_BUS_STOP](https://github.com/user-attachments/assets/77807437-f574-4a06-8685-04d995e38209)
