# nrf52 NVIC - example

此範例以 core-m4.h (nrf52832的core) 裡所定義的 nvic函式 和直接對register改值的方式實現interrupt (register的定義在nrf52.h)

## step
 
**1. 對要使用的interrupt初始化**
以下程式寫法請參考nrf52832 datasheet 中 register對應bit的定義
```
    NRF_GPIOTE->CONFIG[0] = (GPIOTE_CONFIG_POLARITY_HiToLo<<GPIOTE_CONFIG_POLARITY_Pos) //設定觸發條件是上升沿、下降沿或者任何變化  
                           |(BSP_BUTTON_0 << GPIOTE_CONFIG_PSEL_Pos) //設定相對應的中斷輸入pin腳
                           |(GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos) //設定interrupt是event或是task mode
                            
    NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Set << GPIOTE_INTENSET_IN0_Pos //enable相對應的INTENSET register
```  

 可在nrf52.h中找到gpiote定義結構如下
```
typedef struct {                                /*!< (@ 0x40006000) GPIOTE Structure                                           */
  __OM  uint32_t  TASKS_OUT[8];                 /*!< (@ 0x00000000) Description collection[0]: Task for writing to
                                                                    pin specified in CONFIG[0].PSEL. Action
                                                                    on pin is configured in CONFIG[0].POLARITY.                */
  __IM  uint32_t  RESERVED[4];
  __OM  uint32_t  TASKS_SET[8];                 /*!< (@ 0x00000030) Description collection[0]: Task for writing to
                                                                    pin specified in CONFIG[0].PSEL. Action
                                                                    on pin is to set it high.                                  */
  __IM  uint32_t  RESERVED1[4];
  __OM  uint32_t  TASKS_CLR[8];                 /*!< (@ 0x00000060) Description collection[0]: Task for writing to
                                                                    pin specified in CONFIG[0].PSEL. Action
                                                                    on pin is to set it low.                                   */
  __IM  uint32_t  RESERVED2[32];
  __IOM uint32_t  EVENTS_IN[8];                 /*!< (@ 0x00000100) Description collection[0]: Event generated from
                                                                    pin specified in CONFIG[0].PSEL                            */
  __IM  uint32_t  RESERVED3[23];
  __IOM uint32_t  EVENTS_PORT;                  /*!< (@ 0x0000017C) Event generated from multiple input GPIO pins
                                                                    with SENSE mechanism enabled                               */
  __IM  uint32_t  RESERVED4[97];
  __IOM uint32_t  INTENSET;                     /*!< (@ 0x00000304) Enable interrupt                                           */
  __IOM uint32_t  INTENCLR;                     /*!< (@ 0x00000308) Disable interrupt                                          */
  __IM  uint32_t  RESERVED5[129];
  __IOM uint32_t  CONFIG[8];                    /*!< (@ 0x00000510) Description collection[0]: Configuration for
                                                                    OUT[n], SET[n] and CLR[n] tasks and IN[n]
                                                                    event                                                      */
} NRF_GPIOTE_Type;                              /*!< Size = 1328 (0x530)  
```

 **2. 設定要使用的中斷的priority**
 ```
     NVIC_SetPriority(GPIOTE_IRQn,7)
 ```
   在nrf52.h中找到interrupt定義編號如下(註:cortex-m4相關的中斷編號皆<0)  
   ```
   
/* =========================================================================================================================== */
/* ================                                Interrupt Number Definition                                ================ */
/* =========================================================================================================================== */

typedef enum {
/* =======================================  ARM Cortex-M4 Specific Interrupt Numbers  ======================================== */
  Reset_IRQn                = -15,              /*!< -15  Reset Vector, invoked on Power up and warm reset                     */
  NonMaskableInt_IRQn       = -14,              /*!< -14  Non maskable Interrupt, cannot be stopped or preempted               */
  HardFault_IRQn            = -13,              /*!< -13  Hard Fault, all classes of Fault                                     */
  MemoryManagement_IRQn     = -12,              /*!< -12  Memory Management, MPU mismatch, including Access Violation
                                                     and No Match                                                              */
  BusFault_IRQn             = -11,              /*!< -11  Bus Fault, Pre-Fetch-, Memory Access Fault, other address/memory
                                                     related Fault                                                             */
  UsageFault_IRQn           = -10,              /*!< -10  Usage Fault, i.e. Undef Instruction, Illegal State Transition        */
  SVCall_IRQn               =  -5,              /*!< -5 System Service Call via SVC instruction                                */
  DebugMonitor_IRQn         =  -4,              /*!< -4 Debug Monitor                                                          */
  PendSV_IRQn               =  -2,              /*!< -2 Pendable request for system service                                    */
  SysTick_IRQn              =  -1,              /*!< -1 System Tick Timer                                                      */
/* ===========================================  nrf52 Specific Interrupt Numbers  ============================================ */
  POWER_CLOCK_IRQn          =   0,              /*!< 0  POWER_CLOCK                                                            */
  RADIO_IRQn                =   1,              /*!< 1  RADIO                                                                  */
  UARTE0_UART0_IRQn         =   2,              /*!< 2  UARTE0_UART0                                                           */
  SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQn=   3,  /*!< 3  SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0                                      */
  SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn=   4,  /*!< 4  SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1                                      */
  NFCT_IRQn                 =   5,              /*!< 5  NFCT                                                                   */
  GPIOTE_IRQn               =   6,              /*!< 6  GPIOTE                                                                 */
  SAADC_IRQn                =   7,              /*!< 7  SAADC                                                                  */
  TIMER0_IRQn               =   8,              /*!< 8  TIMER0                                                                 */
  TIMER1_IRQn               =   9,              /*!< 9  TIMER1                                                                 */
  TIMER2_IRQn               =  10,              /*!< 10 TIMER2                                                                 */
  RTC0_IRQn                 =  11,              /*!< 11 RTC0                                                                   */
  TEMP_IRQn                 =  12,              /*!< 12 TEMP                                                                   */
  RNG_IRQn                  =  13,              /*!< 13 RNG                                                                    */
  ECB_IRQn                  =  14,              /*!< 14 ECB                                                                    */
  CCM_AAR_IRQn              =  15,              /*!< 15 CCM_AAR                                                                */
  WDT_IRQn                  =  16,              /*!< 16 WDT                                                                    */
  RTC1_IRQn                 =  17,              /*!< 17 RTC1                                                                   */
  QDEC_IRQn                 =  18,              /*!< 18 QDEC                                                                   */
  COMP_LPCOMP_IRQn          =  19,              /*!< 19 COMP_LPCOMP                                                            */
  SWI0_EGU0_IRQn            =  20,              /*!< 20 SWI0_EGU0                                                              */
  SWI1_EGU1_IRQn            =  21,              /*!< 21 SWI1_EGU1                                                              */
  SWI2_EGU2_IRQn            =  22,              /*!< 22 SWI2_EGU2                                                              */
  SWI3_EGU3_IRQn            =  23,              /*!< 23 SWI3_EGU3                                                              */
  SWI4_EGU4_IRQn            =  24,              /*!< 24 SWI4_EGU4                                                              */
  SWI5_EGU5_IRQn            =  25,              /*!< 25 SWI5_EGU5                                                              */
  TIMER3_IRQn               =  26,              /*!< 26 TIMER3                                                                 */
  TIMER4_IRQn               =  27,              /*!< 27 TIMER4                                                                 */
  PWM0_IRQn                 =  28,              /*!< 28 PWM0                                                                   */
  PDM_IRQn                  =  29,              /*!< 29 PDM                                                                    */
  MWU_IRQn                  =  32,              /*!< 32 MWU                                                                    */
  PWM1_IRQn                 =  33,              /*!< 33 PWM1                                                                   */
  PWM2_IRQn                 =  34,              /*!< 34 PWM2                                                                   */
  SPIM2_SPIS2_SPI2_IRQn     =  35,              /*!< 35 SPIM2_SPIS2_SPI2                                                       */
  RTC2_IRQn                 =  36,              /*!< 36 RTC2                                                                   */
  I2S_IRQn                  =  37,              /*!< 37 I2S                                                                    */
  FPU_IRQn                  =  38               /*!< 38 FPU                                                                    */
} IRQn_Type;
   ```
   與中斷優先級所儲存的bits大小
   ```
   #define __NVIC_PRIO_BITS               3        /*!< Number of Bits used for Priority Levels                                   */
   ```
  上述表示nrf52832在中斷優先級有三個位元，因此中斷級數總共有8級(0~7)

 **3. enable要使用的interrupt**
 ```
    NVIC_EnableIRQ(GPIOTE_IRQn);
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
ISER[1]：中斷使能寄存器。要使能某個中斷，必須設置相應的ISER位為1，使該中斷被使能（這裡僅僅是使能，還要配合中斷分組、屏蔽、I/O口映射等設置才算是一個完整的中斷設置）。

ICER[1]：中斷除能寄存器，和ISER的作用恰好相反，用來清除某個中斷的使能的。專門設置一個ICER來清除中斷位，而不是向ISER寫0來清除，這是因為NVIC的這些寄存器都是寫1有效的，寫0無效的。

ISPR[1]：中斷掛起控制寄存器。通過置1可以將正在進行的中斷掛起，而執行同級或者更高級別的中斷。

ICPR[1]：中斷解掛控制寄存器。通過置1可以將掛起的中斷解掛。

IP[8]：中斷優先級控制寄存器組。這個寄存器組相當重要。中斷分組與這個寄存器密切相關。

NVIC_EnableIRQ() 改變register ISER的值  
而 NVIC_SetPriority() 便是改register IP的值

**4.中斷處理函數**  
進行完前三步設置之後，就可以編寫相應的中斷處理函數了。  
中斷處理函數命名是由上面配置的中斷決定的，例如上面配置的中斷是GPIOTE_IRQn，則中斷處理函數的名稱就是GPIOTE_IRQHandler；如果配置的中斷是TIMER1_IRQn，則中斷處理函數的名稱就是TIMER1_IRQHandler。
