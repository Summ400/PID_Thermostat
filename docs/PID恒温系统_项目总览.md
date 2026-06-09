# STM32+PID恒温系统 项目总览与阶段计划

> MCU: STM32F030K6T6 | 封装: LQFP-32 | 主频: 48MHz | Flash: 32KB | RAM: 4KB
> 原理图来源: SCH_Schematic1_2026-05-27.pdf（立创EDA导出）

---

## 一、系统功能框图（根据实际原理图）

```
                         ┌──────────────────────────────────────────────┐
                         │               STM32F030K6T6                  │
                         │                                              │
 K型热电偶 ──→ MAX6675   │──→ PA4(CS) PA5(SCK) PA6(MISO) [SPI读温度]  │
 (U4接线端子)            │                                              │
                         │                                              │
 EC11编码器 ─────────────│──→ PA11(A相) PA12(B相) PA8(C脚) PB0(按键) [设定温度]
 (H3排针)                │                                              │
                         │                                              │
 0.91" OLED ────────────│──→ PB6(SCL) PB7(SDA)          [显示界面]    │
 (H1排针)                │                                              │
                         │                                              │
 加热棒 ←── AOD4184A ←──│── PA7(PWM) ──→ R4(100Ω) ──→ Q1栅极  [TIM17_CH1]
 (P1接线端子)            │                     R2(100kΩ)下拉           │
                         │                                              │
 串口调试 ──────────────│──→ PA9(TX) PA10(RX)           [USART1调试]  │
 (H2排母)                │                                              │
                         │                                              │
 SWD烧录 ──────────────│──→ PA13(SWDIO) PA14(SWCLK)                  │
 (H4排针)                │                                              │
                         │                                              │
 复位按键 ──────────────│──→ NRST (SW1轻触按键, R1=10kΩ上拉)         │
                         │                                              │
 8MHz晶振 ──────────────│──→ PF0(OSC_IN) PF1(OSC_OUT)                │
 (X1, C1/C2=22pF)       └──────────────────────────────────────────────┘
```

---

## 二、实际引脚分配总表（从原理图提取）

| 外设 | 引脚 | CubeMX配置 | 说明 |
|------|------|-----------|------|
| **MAX6675_CS** | PA4 | GPIO_Output, Push-Pull, High | 片选，默认高电平 |
| **MAX6675_SCK** | PA5 | GPIO_Output, Push-Pull, High | SPI时钟线 |
| **MAX6675_MISO** | PA6 | GPIO_Input, NoPull | SPI数据线（从机输出） |
| **EC11_A** | PA11 | GPIO_EXTI, Rising+Falling | 编码器A相(软件EXTI解码) |
| **EC11_B** | PA12 | GPIO_EXTI, Rising+Falling | 编码器B相(软件EXTI解码) |
| **EC11_C** | PA8 | GPIO_Output OD, Low | C脚公共端(模拟GND) |
| **EC11_SW** | PB0 | GPIO_Input, Pull-Up | 编码器按键(TIM3轮询消抖) |
| **OLED_SCL** | PB6 | GPIO_Output, Open-Drain | I2C时钟（软件模拟） |
| **OLED_SDA** | PB7 | GPIO_Output, Open-Drain | I2C数据（软件模拟） |
| **PWM_加热器** | PA7 | TIM17_CH1, PWM Output | 加热器PWM → R4(100Ω) → Q1(AOD4184A) |
| **USART1_TX** | PA9 | USART1_TX | 串口调试发送 |
| **USART1_RX** | PA10 | USART1_RX | 串口调试接收 |
| **SWDIO** | PA13 | SYS_SWDIO | SWD调试口 |
| **SWCLK** | PA14 | SYS_SWCLK | SWD调试口 |
| **NRST** | NRST | SYS_NRST | 复位按键(SW1) |
| **BOOT0** | BOOT0 | 接GND | 从Flash启动 |
| **HSE** | PF0/PF1 | Crystal/Ceramic Resonator | 8MHz外部晶振(X1) |

---

## 三、原理图连接器对照

| 连接器 | 型号 | 引脚数 | 连接对象 | 引脚定义 |
|--------|------|--------|----------|----------|
| **H1** | PZ254V-11-04P | 4P排针 | OLED显示屏 | 3V3, SCL(PB6), SDA(PB7), GND |
| **H2** | 2.54-1*4P母 | 4P排母 | 串口调试 | U1_TX(PA9), U1_RX(PA10), GND, 3V3 |
| **H3** | PZ254V-11-05P | 5P排针 | EC11编码器 | EC11_A(PA11), EC11_B(PA12), EC11_C(PA8), EC11_SW(PB0), GND |
| **H4** | PZ254V-11-04P | 4P排针 | SWD烧录口 | SWDIO(PA13), SWCLK(PA14), NRST, 3V3 |
| **U4** | DB301V-5.0-2P | 2P接线端子 | K型热电偶 | T+, T- → MAX6675 |
| **P1** | WJ500V-5.08-2P | 2P接线端子 | 加热棒 | Q1漏极输出, GND |
| **DC1** | DC-005-25A | DC插座 | 电源输入 | VIN(外接电源) |

---

## 四、电源链路

```
DC插座(DC1) → 保险丝(U5, 10A) → VIN
                                    ↓
                              TPS54202(U2) DC-DC降压
                              ├─ EN脚: R3(22kΩ)上拉到VIN
                              ├─ BOOT脚: C12(56pF)
                              ├─ SW脚: L1(10µH) → 3V3输出
                              │              ├─ C10(22µF) → GND
                              │              └─ C11(22µF) → GND
                              └─ FB脚: 分压反馈
                              输入: C7(100µF)

3V3供电对象: STM32F030(U1), MAX6675(U3), OLED(H1), SWD(H4)
VIN供电对象: EC11编码器(H3), 加热棒驱动(Q1)
```

---

## 五、CubeMX配置清单

### 5.1 SYS
- Debug: `Serial Wire`
- Timebase Source: `SysTick`

### 5.2 RCC
- High Speed Clock (HSE): `Crystal/Ceramic Resonator`

### 5.3 时钟配置
```
HSE(8MHz) → PLL Source Mux: HSE
          → PLLMUL: ×12 (HSI进PLL前硬件自动÷2, 所以×12=48MHz)
          → System Clock Mux: PLLCLK
          → AHB Prescaler: 1   → HCLK = 48MHz
          → APB1 Prescaler: 1  → APB1 = 48MHz
```

### 5.4 USART1（串口调试）
- Mode: `Asynchronous`
- Baud Rate: `115200`
- Word Length: `8 Bits`
- Stop Bits: `1`
- TX: PA9, RX: PA10

### 5.5 EC11编码器（软件EXTI解码）
- EC11_A: PA11, GPIO_EXTI, Rising+Falling, NoPull
- EC11_B: PA12, GPIO_EXTI, Rising+Falling, NoPull
- EC11_C: PA8, GPIO_Output OD, 初始Low (模拟GND)
- EC11_SW: PB0, GPIO_Input, Pull-Up (TIM3轮询消抖)
- NVIC: 使能 EXTI4_15 interrupt, 优先级0
- 说明: PA11/PA12无定时器通道映射，无法用硬件编码器模式

### 5.6 TIM3（1ms系统定时器）
- Counter Period: `47999`
- Prescaler: `0`
- → 48MHz / 48000 = 1kHz = 1ms中断
- NVIC: 使能 `TIM3 global interrupt`, 优先级0
- 用途: 按键消抖(20ms)、长按检测(2s)

### 5.7 TIM17（PWM加热器）
- Channel1: `PWM Generation CH1`
- Counter Period (ARR): `65535`
- Prescaler (PSC): `0`
- Pulse (CCR): `0`
- → PWM频率 ≈ 732Hz (阶段4可改为1kHz)
- 引脚: PA7(CH1), AF5

### 5.8 GPIO

| 引脚 | 模式 | 标签 | 详细设置 |
|------|------|------|----------|
| PA4 | GPIO_Output | MAX6675_CS | Push-Pull, High Speed, 初始High |
| PA5 | GPIO_Output | MAX6675_SCK | Push-Pull, High Speed, 初始High |
| PA6 | GPIO_Input | MAX6675_MISO | NoPull |
| PB0 | GPIO_Input | EC11_SW | Pull-Up, TIM3轮询消抖(不用EXTI) |
| PA8 | GPIO_Output | EC11_C | Open-Drain, 初始Low, 模拟GND |
| PA11 | GPIO_EXTI | EC11_A | Rising+Falling, NoPull, EXTI4_15 |
| PA12 | GPIO_EXTI | EC11_B | Rising+Falling, NoPull, EXTI4_15 |
| PB6 | GPIO_Output | OLED_SCL | Open-Drain, High Speed, 初始High |
| PB7 | GPIO_Output | OLED_SDA | Open-Drain, High Speed, 初始High |

---

## 六、项目文件结构

```
PID恒温系统/
├── Core/                    # CubeMX生成（不要手动修改）
│   ├── Inc/
│   │   ├── main.h
│   │   ├── gpio.h
│   │   ├── tim.h
│   │   ├── usart.h
│   │   └── stm32f0xx_it.h
│   └── Src/
│       ├── main.c
│       ├── gpio.c
│       ├── tim.c
│       ├── usart.c
│       ├── stm32f0xx_it.c
│       └── system_stm32f0xx.c
├── Drivers/                 # HAL库（CubeMX生成）
├── MDK-ARM/                 # Keil工程文件
├── 用户代码/                # 手写代码（全部放这里）
│   ├── 温度读取.c           # 阶段1: MAX6675温度读取
│   ├── 温度读取.h
│   ├── 旋转编码器.c         # 阶段2: EC11编码器
│   ├── 旋转编码器.h
│   ├── OLED显示.c           # 阶段3: OLED显示
│   ├── OLED显示.h
│   ├── 字库数据.h           # 字库数据
│   ├── PWM加热.c            # 阶段4: PWM加热控制
│   ├── PWM加热.h
│   ├── PID算法.c            # 阶段5: PID算法
│   ├── PID算法.h
│   ├── 主逻辑.c             # 阶段6: 主逻辑整合
│   └── 主逻辑.h
└── PID恒温系统.ioc          # CubeMX工程文件
```

---

## 七、阶段计划

### 阶段0: CubeMX工程创建 ✋

**目标**: 生成基础工程，确保编译通过，串口打印验证

**操作步骤**:
1. 打开STM32CubeMX，选择 **STM32F030K6T6**
2. 按上面"五、CubeMX配置清单"逐项配置
3. Project Manager → Toolchain: `MDK-ARM V5`
4. Code Generator → ✅ 勾选 `Generate peripheral initialization as a pair of .c/.h files`
5. ✅ 勾选 `Keep User Code when re-generating`
6. 生成代码
7. 在Keil中打开工程，编译确认 **0 Error**
8. 在 `main.c` 中添加串口重定向（见下方代码）
9. 在while(1)中打印 "Hello PID Thermostat!" 验证烧录

**串口重定向代码**（添加到main.c的 USER CODE BEGIN 0 区域）:
```c
#include <stdio.h>

int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
```

**验收标准**: 工程编译0 Error，串口助手能收到打印信息

---

### 阶段1: MAX6675温度读取 ✋

**目标**: 通过软件SPI读取MAX6675，串口打印温度值

**你需要写的文件**: `温度读取.c` / `温度读取.h`

**核心知识点**:
- MAX6675是只读SPI设备，CS拉低后发送16个SCK时钟即可读出数据
- 16位数据格式: D15=Dummy, D14~D3=12位温度值, D2=热电偶断线标志, D1~D0=Dummy
- 分辨率: 0.25°C/LSB，温度 = (data >> 3) × 0.25
- 转换时间约0.17秒，不要读太快

**引脚连接**（从原理图）:
```
MAX6675 CS   → PA4 (GPIO_Output)
MAX6675 SCK  → PA5 (GPIO_Output)
MAX6675 MISO → PA6 (GPIO_Input)
MAX6675 VCC  → 3V3
MAX6675 GND  → GND
MAX6675 T+/T- → U4接线端子 → K型热电偶
```

**你需要实现的函数**:
```c
void    温度初始化(void);          // CS/SCK输出高, MISO输入
float   读取温度(void);            // 读16位数据，返回摄氏度
uint8_t 热电偶是否连接(void);      // 检查热电偶是否断线(D2位)
```

**验证方法**:
- 在main.c中每500ms调用 `读取温度()`
- 用 `printf` 通过串口打印温度值
- 用手摸热电偶，观察温度变化

**验收标准**: 串口能稳定输出室温（约25°C），手摸后温度上升

---

### 阶段2: EC11旋转编码器 ✅ 已完成

**状态**: 已完成，详见 `docs/阶段2_编码器_当前状态.md`

**实际实现文件**: user_code/encoder.c / encoder.h

**实际方案**:
- 旋转检测: PA11/PA12 EXTI双边沿触发 + 电平比较判断方向
- 按键检测: PB0在TIM3(1ms)中断中轮询, 4状态消抖状态机
- 公共端: PA8配置为开漏输出低电平, 模拟GND

**已实现API**:
- `enc_get_delta()` — 读取旋转增量(正=顺时针, 负=逆时针)
- `enc_button_pressed()` — 短按检测
- `enc_button_long_press()` — 长按检测(>2秒)

---

### 阶段3: OLED显示 ✋

**目标**: 0.91寸OLED显示当前温度和目标温度

**你需要写的文件**: `OLED显示.c` / `OLED显示.h` / `字库数据.h`

**核心知识点**:
- SSD1306驱动IC，I2C地址0x78（写模式）
- 软件I2C：GPIO模拟SCL/SDA时序
- 显示RAM 128×64，分8页（Page0~Page7），每页128字节
- 写命令: I2C起始 → 0x78 → 0x00 → 命令字节
- 写数据: I2C起始 → 0x78 → 0x40 → 数据字节
- 初始化序列约25条命令（参考SSD1306数据手册）

**引脚连接**（从原理图，H1排针）:
```
OLED SCL → PB6 (GPIO_Output, Open-Drain)
OLED SDA → PB7 (GPIO_Output, Open-Drain)
OLED VCC → 3V3 (H1第1脚)
OLED GND → GND (H1第4脚)
```

**你需要实现的函数**:
```c
void    OLED初始化(void);              // 初始化序列
void    OLED清屏(void);                // 清屏
void    OLED定位(uint8_t x, uint8_t y); // 定位光标
void    OLED显示字符(uint8_t x, uint8_t y, char ch);   // 显示单字符
void    OLED显示字符串(uint8_t x, uint8_t y, char *str); // 显示字符串
void    OLED显示数字(uint8_t x, uint8_t y, float num, uint8_t 小数位数); // 显示浮点数
```

**显示界面设计**:
```
┌────────────────────┐
│  PID恒温控制系统    │  ← 标题
│                    │
│  当前:  25.50 C     │  ← 当前温度
│  目标:  50.00 C     │  ← 目标温度
│  占空比: 30%        │  ← PWM占空比
│                    │
│  [状态: 加热中]     │  ← 状态
└────────────────────┘
```

**验证方法**:
- OLED显示固定字符串验证驱动
- 接入MAX6675后显示实时温度
- 接入编码器后可调节目标温度

**验收标准**: OLED稳定显示，无花屏，温度/目标温度正确更新

---

### 阶段4: PWM加热控制 ✋

**目标**: PWM驱动MOSFET控制加热器功率

**你需要写的文件**: `PWM加热.c` / `PWM加热.h`

**核心知识点**:
- TIM17_CH1输出PWM，频率约732Hz（ARR=65535, PSC=0）
- 占空比 = CCR / (ARR+1) × 100%
- `__HAL_TIM_SET_COMPARE(&htim17, TIM_CHANNEL_1, value)` 设置占空比
- value范围: 0(全关) ~ 1000(全开)
- AOD4184A是N-MOSFET，PWM高电平导通

**引脚连接**（从原理图）:
```
PA7(PWM) → R4(100Ω, 栅极限流) → Q1(AOD4184A)栅极
Q1源极 → GND
Q1漏极 → P1接线端子(接加热棒)
R2(100kΩ) → Q1栅极到GND(下拉, 防误触发)
P1接线端子另一端 → VIN(电源正极)
```

**你需要实现的函数**:
```c
void    加热器初始化(void);                  // 启动PWM输出
void    设置占空比(uint16_t 千分比);          // 设置占空比(0~1000, 千分比)
uint16_t 获取占空比(void);                    // 获取当前占空比
void    关闭加热器(void);                     // 关闭PWM输出(安全关断)
```

**验证方法**:
- 先不接加热器，用示波器/逻辑分析仪看PA7波形
- 设置不同占空比，验证波形变化
- 接入加热器后，低占空比(10%)测试，手感受热

**⚠️ 安全提醒**:
- 首次测试必须低占空比(≤10%)
- 确保加热器散热良好，避免过热起火
- 建议加温度上限保护（软件层面，超过120°C强制关PWM）

**验收标准**: PWM波形正确，占空比可调，加热器能发热

---

### 阶段5: PID算法实现 ✋

**目标**: 实现增量式PID，温度自动稳定在目标值

**你需要写的文件**: `PID算法.c` / `PID算法.h`

**核心知识点**:
- **位置式PID**: 输出 = Kp×e + Ki×Σe + Kd×(e-e_上次)
- **增量式PID**: Δ输出 = Kp×(e-e1) + Ki×e + Kd×(e-2e1+e2)
- 本项目推荐增量式PID，输出增量不会突变，更安全
- PID计算周期: 建议200ms~500ms（加热系统响应慢）
- 积分限幅: 防止积分饱和（加热器功率有上限）

**你需要实现的函数**:
```c
void    PID初始化(float kp, float ki, float kd);  // 初始化PID参数
void    设置目标温度(float 目标);                   // 设置目标温度
float   获取目标温度(void);                         // 获取目标温度
int16_t PID计算(float 当前温度);                    // 执行一次PID计算，返回PWM增量
void    PID重置(void);                              // 重置PID状态
void    设置输出限幅(int16_t 最小值, int16_t 最大值); // 输出限幅
```

**PID参数整定方法（先理论后实测）**:

```
第一步: 只用P控制
  Ki=0, Kd=0, Kp从小到大调
  → 目标: 温度能上升但低于目标值（稳态误差）
  → 预估Kp起始值: 10~50

第二步: 加入I控制
  Ki从小到大调
  → 目标: 消除稳态误差，温度逐渐逼近目标值
  → 预估Ki起始值: 0.01~0.5

第三步: 加入D控制
  Kd从小到大调
  → 目标: 减小超调，加快稳定
  → 预估Kd起始值: 1~20

第四步: 微调
  观察温度曲线，反复调整三个参数
```

**验证方法**:
- 设定目标温度50°C，观察温度上升曲线
- 记录: 超调量、稳定时间、稳态误差
- 调整参数直到: 超调<5°C，稳定时间<60秒，稳态误差<1°C

**验收标准**: 温度能稳定在目标值±1°C以内

---

### 阶段6: 系统整合与优化 ✋

**目标**: 所有模块联调，完善功能细节

**你需要写的文件**: `主逻辑.c` / `主逻辑.h`（整合到main.c也可以）

**整合内容**:

1. **主循环结构**:
```c
while(1) {
    编码器处理();          // 编码器处理（调节目标温度）
    OLED刷新显示();        // OLED刷新显示
    // PID计算在定时器中断中按周期执行
}
```

2. **1ms定时器中断**:
```c
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if(htim->Instance == TIM3) {
        编码器定时处理();         // 编码器按键消抖
        pid计时器++;              // PID周期计时
        if(pid计时器 >= 200)      // 200ms执行一次PID
        {
            pid计时器 = 0;
            pid标志 = 1;          // 置标志位，主循环中计算
        }
    }
}
```

3. **安全保护**:
- 温度上限保护: 当前温度 > 120°C → 强制关PWM
- 热电偶断线检测: MAX6675 D2位 → 报警并关PWM
- 目标温度范围限制: 编码器设定范围 0~200°C

4. **OLED界面完善**:
- 显示加热状态: 加热中 / 已稳定 / 降温中 / 故障
- 编码器调节时目标温度闪烁提示
- 断线报警显示

**验收标准**: 系统完整运行，温度稳定，所有保护功能正常

---

## 八、阶段依赖关系

```
阶段0: CubeMX工程 ──────────────────────────────────────┐
                                                         │
阶段1: MAX6675温度 ────────────────────────────┐         │
                                               │         │
阶段2: EC11编码器 ─────────────────────┐       │         │
                                       │       │         │
阶段3: OLED显示 ──────────────┐        │       │         │
                              │        │       │         │
阶段4: PWM加热 ───────────────┤        │       │         │
                              │        │       │         │
阶段5: PID算法 ←──────────────┴────────┴───────┘         │
                              │                           │
阶段6: 系统整合 ←─────────────┴───────────────────────────┘
```

> 阶段1~4可以**任意顺序**完成，互不依赖
> 阶段5需要阶段1+4完成（需要温度读数和PWM输出）
> 阶段6需要全部完成

---

## 九、串口调试配置

你的板子有H2排母预留了USART1，可以直接用串口调试：

**硬件连接**（H2排母）:
```
H2 Pin1 → PA9  (U1_TX, 单片机发送)
H2 Pin2 → PA10 (U1_RX, 单片机接收)
H2 Pin3 → GND
H2 Pin4 → 3V3
```

用USB转TTL模块连接：
- TTL的RX → H2 Pin1 (PA9/TX)
- TTL的TX → H2 Pin2 (PA10/RX)
- TTL的GND → H2 Pin3 (GND)

**串口重定向代码**（添加到main.c）:
```c
#include <stdio.h>

int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
```

串口助手设置: 波特率 115200, 8N1
