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
ISER[1]：中斷使能寄存器。由前邊可以看到，nRF51的寄存器編號為0-25，在這裡ISER[1]為32位寄存器，總共可以表示32個中斷，bit0-bit25分別對應中斷0-25（其實後來發現在NVIC_EnableIRQ()函數中是將1左移IRQn位之後又與0x1F，也就是空出了低五位，所以在這裡應該是對應的高地址，而把低位給空出來的）。要使能某個中斷，必須設置相應的ISER位為1，使該中斷被使能（這裡僅僅是使能，還要配合中斷分組、屏蔽、I/O口映射等設置才算是一個完整的中斷設置）。

ICER[1]：中斷除能寄存器，和ISER的作用恰好相反，用來清除某個中斷的使能的。專門設置一個ICER來清除中斷位，而不是向ISER寫0來清除，這是因為NVIC的這些寄存器都是寫1有效的，寫0無效的。

ISPR[1]：中斷掛起控制寄存器。通過置1可以將正在進行的中斷掛起，而執行同級或者更高級別的中斷。

ICPR[1]：中斷解掛控制寄存器。通過置1可以將掛起的中斷解掛。

IP[8]：中斷優先級控制寄存器組。這個寄存器組相當重要。中斷分組與這個寄存器密切相關。


 **3. enable要使用的interrupt**
