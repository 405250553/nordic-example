# nrf52 NVIC - example

此範例以 core-m4.h (nrf52832的core) 裡所定義的 nvic函式 和直接對register改值的方式實現interrupt (register的定義在nrf52.h)

## step
 
 1. 對要使用的INTERRUPT初始化
```
NRF_GPIOTE->CONFIG[0] = (GPIOTE_CONFIG_POLARITY_HiToLo<<GPIOTE_CONFIG_POLARITY_Pos)  
                        |(BSP_BUTTON_0 << GPIOTE_CONFIG_PSEL_Pos)  
                        |(GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos)  
                            
NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Set << GPIOTE_INTENSET_IN0_Pos 
```  
NRF_GPIOTE->CONFIG //gpiote的channel，nrf52832有8個
NRF_GPIOTE->INTENSET //enable interrupt
 2. enable要使用的INTERRUPT
 3. 設定中斷的priority
