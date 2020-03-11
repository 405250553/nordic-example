# nrf52 NVIC - example

此範例以 core-m4.h (nrf52832的core) 裡所定義的 nvic函式 和直接對register改值的方式實現interrupt (register的定義在nrf52.h)

## step
 
 1. 對要使用的interrupt初始化
```
    NRF_GPIOTE->CONFIG[0] = (GPIOTE_CONFIG_POLARITY_HiToLo<<GPIOTE_CONFIG_POLARITY_Pos) //設定觸發條件是上升沿、下降沿或者任何變化  
                           |(BSP_BUTTON_0 << GPIOTE_CONFIG_PSEL_Pos) //設定相對應的中斷輸入pin腳
                           |(GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos) //設定interrupt是event或是task mode
                            
    NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Set << GPIOTE_INTENSET_IN0_Pos //enable相對應的INTENSET register
```  
 2. enable要使用的interrupt
 ```
     NVIC_SetPriority(GPIOTE_IRQn,7)
 ```
 
 3. 設定中斷的priority
