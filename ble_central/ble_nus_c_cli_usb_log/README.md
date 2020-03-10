# nrf52840 dongle ble_nus central + log - example

此 project 利用 https://github.com/jimmywong2003/nrf52840-dongle-example 寫好的模組(nrf_log_backend_usb) ，結合nus central來實現dongle接收資料

## Requirement

* NRF52840 DK (PCA10056) or NRF52840 Dongle (PCA10059)
* SDK 15.3
* Softdevice S140 v6.1.1
* Segger Embedded Studio
* Terminal on the PC (ex: putty)

## compile 

 * 需將資料夾 sdk_mod 移動到 project 資料夾所在的目錄下 ，否則nrf_log模組會讀取不到