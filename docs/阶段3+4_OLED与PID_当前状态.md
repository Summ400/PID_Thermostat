# 阶段3+4 OLED显示与PID控制 — 当前状态

> 最后更新：2026-06-05
> 状态：✅ 代码完成，MOSFET未焊接无法实测加热
> 项目路径：D:\STM32_Project\PID_Thermostat\

---

## 一、项目背景

- MCU：STM32F030K6T6 (LQFP-32, 48MHz, 32KB Flash, 4KB RAM)
- 项目：PID恒温系统（热电偶+加热棒+编码器设定+OLED显示）
- 命名规则：模块前缀 temp_ / enc_ / oled_ / heater_ / pid_ / app_，文件名英文
- ARMCC编译器（不支持中文文件名/函数名）
- MicroLIB必须启用，否则printf不工作
- GPIO必须通过CubeMX配置，手动写GPIO初始化无效

---

## 二、已完成阶段

| 阶段 | 内容 | 状态 |
|------|------|------|
| 阶段0 | CubeMX工程 + USART1串口 + printf重定向 | ✅ |
| 阶段1 | MAX6675温度读取（soft_spi + temp_reader） | ✅ |
| 阶段2 | EC11编码器旋转+按键检测（encoder） | ✅ |
| 阶段3 | SSD1306 OLED显示（soft_i2c + oled_font + oled_display） | ✅ |
| 阶段4 | PID算法 + 加热器控制（pid_control + heater_ctrl） | ✅ 代码完成 |

---

## 三、阶段3：SSD1306 OLED显示

### 3.1 硬件

- 型号：0.91寸 SSD1306 OLED，128×32分辨率
- 接口：软件模拟I2C，地址0x78
- 引脚：PB6(SCL) / PB7(SDA)，开漏输出

### 3.2 文件清单

| 文件 | 职责 |
|------|------|
| `soft_i2c.c/h` | 软件I2C底层驱动（start/stop/write_byte） |
| `oled_font.c/h` | 8×16 ASCII字库（空格~z，共95个字符） |
| `oled_display.c/h` | OLED显示驱动（init/clear/show_string/show_number） |

### 3.3 关键API

```c
void oled_init(void);                                    /* 初始化OLED */
void oled_clear(void);                                   /* 清屏 */
void oled_show_string(uint8_t x, uint8_t line, const char *str);  /* 显示字符串，line=0或1 */
void oled_show_number(uint8_t x, uint8_t line, float num, uint8_t decimals);  /* 显示浮点数 */
```

### 3.4 显示布局

```
行0: Temp: 27.5 C        ← 当前温度
行1: Targ: 30.0 C        ← 目标温度
```

- "Temp:"、"Targ:"、"C" 在初始化时画一次，不刷新
- 数字每500ms刷新一次，避免闪烁

### 3.5 设计决策

1. **为什么用8×16字体而不是6×8？**
   - 0.91寸屏幕很小，6×8字体看不清
   - 8×16字体每行8字符，2行共16字符，够显示温度信息

2. **为什么用软件I2C而不是硬件I2C？**
   - STM32F030K6T6的硬件I2C有已知BUG（ST Errata）
   - 软件I2C简单可靠，OLED通信速率要求不高

3. **为什么不用oled_clear()刷新？**
   - clear()会全屏擦除再重绘，肉眼可见闪烁
   - 只刷新数字位置，静态标签只画一次

---

## 四、阶段4：PID控制 + PWM加热

### 4.1 硬件

- MOSFET：AOD4184A（P沟道，逻辑电平驱动）
- PWM：TIM17_CH1 → PA7 → R4(100Ω) → Q1栅极
- ARR=65535，PWM频率约732Hz
- **当前状态：AOD4184A未焊接，PWM输出无法实测**

### 4.2 文件清单

| 文件 | 职责 |
|------|------|
| `pid_control.c/h` | 增量式PID算法 |
| `heater_ctrl.c/h` | PWM占空比控制 + 安全保护 |

### 4.3 关键API

```c
/* PID */
void    pid_init(pid_t *pid, float kp, float ki, float kd, int32_t out_max);
void    pid_setpoint(pid_t *pid, float sp);
int32_t pid_compute(pid_t *pid, float measurement);
void    pid_reset(pid_t *pid);

/* 加热器 */
void     heater_init(void);
void     heater_set_duty(uint16_t duty);       /* 0=全关, 65535=全开 */
void     heater_off(void);                     /* 紧急关断 */
uint8_t  heater_is_safe(float temp, uint8_t tc_ok);  /* 安全检查 */
```

### 4.4 PID算法说明

采用**增量式PID**，每次只计算输出变化量：

```
Δu = Kp × (e(k) - e(k-1))           ← 比例项：误差变化趋势
   + Ki × e(k)                       ← 积分项：消除稳态误差
   + Kd × (e(k) - 2e(k-1) + e(k-2)) ← 微分项：抑制超调

新输出 = 旧输出 + Δu
```

增量式 vs 位置式：
- 增量式输出平滑变化，不会突变，对加热系统更安全
- 自动抗积分饱和
- 切换模式时不会跳变

### 4.5 当前PID参数（预设值，待实测调试）

| 参数 | 值 | 说明 |
|------|----|------|
| Kp | 100.0 | 比例系数 |
| Ki | 0.5 | 积分系数 |
| Kd | 20.0 | 微分系数 |

### 4.6 安全保护

| 保护条件 | 动作 |
|----------|------|
| 热电偶断线（MAX6675 D2=1） | 强制关闭PWM |
| 温度≥120°C | 强制关闭PWM |
| 正常状态 | PID自动控制 |

### 4.7 编码器调节目标温度

- 顺时针转一格 → 目标温度 +1°C
- 逆时针转一格 → 目标温度 -1°C
- 范围：0°C ~ 120°C
- 计算方式：`target_temp += delta * 0.25`（编码器每格4个脉冲，×0.25=1°C）

### 4.8 温度读取优化

`temp_read_all()` 一次SPI读取同时获取温度和热电偶状态，避免连续两次读取导致数据跳动（MAX6675转换周期170ms）。

---

## 五、完整文件清单

```
user_code/
├── soft_spi.c/h          # 软件SPI（MAX6675用）
├── soft_i2c.c/h          # 软件I2C（OLED用）
├── temp_reader.c/h       # MAX6675温度读取
├── encoder.c/h           # EC11编码器旋转+按键
├── oled_font.c/h         # 8×16字库
├── oled_display.c/h      # OLED显示驱动
├── pid_control.c/h       # 增量式PID算法
└── heater_ctrl.c/h       # PWM加热器控制+安全保护
```

---

## 六、main.c 当前逻辑

```
初始化:
  enc_init() → TIM3启动 → TIM17 PWM启动 → temp_init()
  i2c_init() → oled_init() → 画静态标签
  heater_init() → pid_init() → pid_setpoint(30°C)

主循环(500ms):
  1. temp_read_all() → 获取温度+热电偶状态
  2. enc_get_delta() → 编码器调节目标温度
  3. heater_is_safe()?
     是 → pid_compute() → heater_set_duty()
     否 → heater_off()
  4. 串口打印温度/目标/PID输出
  5. OLED刷新数字
```

---

## 七、踩坑记录

### 7.1 MAX6675连续读取导致温度跳动

`temp_read()` 和 `temp_thermocouple_ok()` 各做一次SPI读取，MAX6675转换周期170ms，短时间内连读两次，第二次可能读到无效数据。

**解决**：合并为 `temp_read_all()`，一次读取同时获取温度和热电偶状态。

### 7.2 int16_t装不下65535

PID输出范围0~65535（TIM17 ARR值），但 `int16_t` 最大32767，导致编译警告和运行时溢出。

**解决**：PID结构体中 `output`/`out_min`/`out_max` 改为 `int32_t`。

### 7.3 OLED全屏刷新闪烁

`oled_clear()` + 重绘所有内容会导致肉眼可见闪烁。

**解决**：初始化时画静态标签（"Temp:"、"Targ:"、"C"），循环中只刷新数字位置。

---

## 八、待完成事项

| 事项 | 说明 | 优先级 |
|------|------|--------|
| 焊接AOD4184A MOSFET | 接上加热棒后才能实际验证PID | 高 |
| 实测调试PID参数 | Kp/Ki/Kd当前是预设值，需根据实际加热曲线调整 | 高 |
| 阶段5 系统集成 | 菜单/状态机/更多OLED信息显示 | 中 |

---

## 九、已知问题

1. HSE外部晶振不起振（负载电容22pF偏小，应换30pF），当前用HSI
2. TIM14在CubeMX中没有Internal Clock选项（可能是CubeMX对F030的显示问题），用TIM3代替
3. TIM17的PWM频率约732Hz（ARR=65535），焊接后可改为1kHz
4. PID参数未实测验证，当前Kp=100/Ki=0.5/Kd=20是估算值
