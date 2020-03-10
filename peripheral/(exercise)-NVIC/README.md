# nrf52 NVIC - example

此範例以 core-m4.h(以nrf52832實作) 裡所定義的 nvic函式 和直接對register改值的方式實現interrupt

## step
 
 1. 對要用到的中斷初始化
 2. enable要使用的中斷
 3. 設定中斷的priority