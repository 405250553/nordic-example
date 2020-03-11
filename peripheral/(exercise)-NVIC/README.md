# nrf52 NVIC - example

此範例以 core-m4.h (nrf52832的core) 裡所定義的 nvic函式 和直接對register改值的方式實現interrupt (register的定義在nrf52.h)

## step
 
**1. 對要使用的interrupt初始化**
```
    NRF_GPIOTE->CONFIG[0] = (GPIOTE_CONFIG_POLARITY_HiToLo<<GPIOTE_CONFIG_POLARITY_Pos) //設定觸發條件是上升沿、下降沿或者任何變化  
                           |(BSP_BUTTON_0 << GPIOTE_CONFIG_PSEL_Pos) //設定相對應的中斷輸入pin腳
                           |(GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos) //設定interrupt是event或是task mode
                            
    NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Set << GPIOTE_INTENSET_IN0_Pos //enable相對應的INTENSET register
```  
 **2. 設定中斷的priorityenable要使用的interrupt**
 ```
     NVIC_SetPriority(GPIOTE_IRQn,7)
 ```
 
   在core-m4.h裡定義的NVIC結構如下
 
```
typedef struct
{
  __IOM uint32_t ISER[8U];               /*!< Offset: 0x000 (R/W)  Interrupt Set Enable Register */
        uint32_t RESERVED0[24U];
  __IOM uint32_t ICER[8U];               /*!< Offset: 0x080 (R/W)  Interrupt Clear Enable Register */
        uint32_t RSERVED1[24U];
  __IOM uint32_t ISPR[8U];               /*!< Offset: 0x100 (R/W)  Interrupt Set Pending Register */
        uint32_t RESERVED2[24U];
  __IOM uint32_t ICPR[8U];               /*!< Offset: 0x180 (R/W)  Interrupt Clear Pending Register */
        uint32_t RESERVED3[24U];
  __IOM uint32_t IABR[8U];               /*!< Offset: 0x200 (R/W)  Interrupt Active bit Register */
        uint32_t RESERVED4[56U];
  __IOM uint8_t  IP[240U];               /*!< Offset: 0x300 (R/W)  Interrupt Priority Register (8Bit wide) */
        uint32_t RESERVED5[644U];
  __OM  uint32_t STIR;                   /*!< Offset: 0xE00 ( /W)  Software Trigger Interrupt Register */
}  NVIC_Type;
```
 **3. enable要使用的interrupt**
