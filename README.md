# RoboMini 电控初筛 · 初级任务：STM32 点灯

## 一、任务完成情况

- 使用 **STM32CubeMX 6.18.1** 完成 STM32F103C8T6 的引脚 / 时钟 / GPIO 配置
- 使用 **Keil MDK-ARM 5.43（Arm Compiler 6.24）** 编写代码、编译并下载到板子
- 板载 LED（PC13）以约 **1Hz** 频率闪烁，演示视频见压缩包内 `demo.mp4`
- 同时学习并总结了 GPIO 的工作模式与原理（见第六节）

## 二、开发环境

| 工具 | 版本 |
|---|---|
| STM32CubeMX | 6.18.1 |
| Keil MDK-ARM（μVision） | 5.43 |
| Arm Compiler | 6.24（armclang） |
| STM32Cube 固件包 | STM32Cube FW_F1 **V1.8.7** |
| Keil 器件支持包 | Keil.STM32F1xx_DFP 2.4.1 + ARM::CMSIS 6.3.0 |

## 三、硬件

- **STM32F103C8T6 最小系统板**（Blue Pill）：LQFP48，64KB Flash / 20KB RAM，板载 8MHz 晶振
- **ST-Link V2** 下载器，SWD 四线接线：

  | ST-Link | 开发板 |
  |---|---|
  | 3.3V | 3V3 |
  | GND | GND |
  | SWDIO | PA13 |
  | SWCLK | PA14 |

- 板载 LED 接在 **PC13**，采用**灌电流接法 → 低电平点亮**

## 四、CubeMX 关键配置

| 配置项 | 设置 |
|---|---|
| 芯片 | STM32F103C8Tx |
| **RCC** | High Speed Clock (HSE)：**Crystal/Ceramic Resonator**（8MHz 晶振） |
| **SYS** | Debug：**Serial Wire** ★（不配置会导致下载一次后无法再次连接） |
| **GPIO** | **PC13 → GPIO_Output**，User Label = `LED` |
| GPIO 参数 | Output Push Pull（推挽）、No pull-up/down、Low 速度 |
| **时钟树** | HSE 8MHz → PLL ×9 → SYSCLK / HCLK = **72MHz** |
| 工具链 | **MDK-ARM V5** |

## 五、点灯代码

文件：`Core/Src/main.c`（写在 CubeMX 的 `/* USER CODE BEGIN WHILE */` 区域内，重新生成代码不会被覆盖）

```c
while (1)
{
  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);   /* 翻转 PC13 电平 */
  HAL_Delay(500);                           /* 延时 500ms → 约 1Hz 闪烁 */
}
```

另外 `reference/main_register_version.c` 是同一功能的**寄存器直接操作版**（不依赖 HAL 库），作为学习对照。

## 六、GPIO 用法与原理（学习总结）

**1. 8 种工作模式**

| 模式 | 典型用途 |
|---|---|
| 输入：浮空 / 上拉 / 下拉 / 模拟 | 按键（上拉或下拉确定默认电平）、ADC 采集（模拟） |
| 输出：推挽 / 开漏 | 点灯用**推挽**；开漏只能拉低、需外部上拉，用于 I²C、电平转换、多设备共线 |
| 复用：推挽 / 开漏 | 引脚交给定时器 / USART / SPI 等外设时使用 |

**2. 灌电流 vs 拉电流**
本板 LED 接法是 `3.3V → 限流电阻 → LED → PC13`，所以 **PC13 输出低电平才点亮**（灌电流）。限流电阻按 (3.3V − 2.0V) ÷ 10mA ≈ 130Ω 估算，工程上取 330Ω~1kΩ 都安全。STM32 单个 IO 最大 20mA，整片约 150mA。

**3. 上拉 / 下拉电阻的作用**
输入引脚悬空时电平不确定，配置上拉或下拉可以给它一个确定的默认电平（按键常用），避免误触发。

**4. 关键寄存器**

| 寄存器 | 作用 |
|---|---|
| `MODER` | 引脚方向：输入 / 输出 / 复用 / 模拟 |
| `OTYPER` | 输出类型：推挽 / 开漏 |
| `OSPEEDR` | 输出速度（影响边沿斜率与 EMI） |
| `PUPDR` | 上拉 / 下拉 |
| `IDR` / `ODR` | 输入 / 输出数据寄存器 |
| **`BSRR`** | 置位 / 复位寄存器：**一次写即完成置位或复位，不需要"读-改-写"，是原子操作**，比直接操作 ODR 更安全 |

**5. 用到的 HAL 函数**
`HAL_GPIO_WritePin()`（写电平）、`HAL_GPIO_TogglePin()`（翻转）、`HAL_Delay()`（毫秒延时，基于 SysTick）。

## 七、编译与下载步骤

1. 用 Keil 打开 `led_blink_STM32F103C8/MDK-ARM/LED.uvprojx`
2. `Options for Target → Target` → ARM Compiler 选 **Use default compiler version 6（armclang）**
   （Keil 5.37 之后不再自带 Arm Compiler 5，而 CubeMX 生成的工程默认按 AC5 配置，必须切换）
3. `Options for Target → Asm` → Misc Controls 填 **`-masm=auto`**
   （CubeMX 生成的启动文件 `startup_stm32f103xb.s` 是 AC5 的 armasm 语法，AC6 需要该参数才能汇编）
4. `Options for Target → Debug` → 选择 **ST-Link Debugger**；`Settings → Flash Download` 勾选 **Reset and Run**
5. 按 **F7** 编译（0 Error）→ **F8** 下载

> 本工程已把上述 2、3 两项配置保存在 `LED.uvprojx` 里。

## 八、目录结构

```
led_blink_STM32F103C8/
├── LED.ioc                  CubeMX 工程文件（全部图形化配置都可复现）
├── .mxproject
├── Core/                    用户代码：main.c、gpio.c、中断服务等
├── Drivers/                 STM32F1 HAL 库 + CMSIS
└── MDK-ARM/
    ├── LED.uvprojx          Keil 工程（已配置 AC6 + -masm=auto）
    ├── LED.uvoptx
    ├── startup_stm32f103xb.s
    └── RTE/
reference/
└── main_register_version.c  寄存器版点灯实现（学习对照）
```

（完）
