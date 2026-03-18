# 基于 STM32G030 的多源温度采集与 OLED 显示系统

> 平台：`STM32G030F6Px`
> 框架：`STM32 HAL`
> 作者：`sea90d`

## 简介

项目通过两路温度源进行采集和显示：

- `AHT20` 通过 `I2C1` 采集温湿度
- `NTC` 通过 `ADC1_IN0 (PA0)` 采集分压并换算温度
- `SSD1306 OLED` 实时显示
- `USART1` 输出调试日志

当前代码中曾加入 `PA1` 呼吸灯逻辑，用于根据温度调整呼吸频率；这部分现已在代码中注释停用，保留为后续扩展。

## 硬件连接

| 模块 | 接口 | 引脚 | 说明 |
|---|---|---|---|
| AHT20 | I2C1 | PB6 / PB7 | 温湿度采集 |
| SSD1306 OLED | I2C1 | PB6 / PB7 | 屏幕显示 |
| NTC 分压 | ADC1_IN0 | PA0 | 温度采样 |
| 串口调试 | USART1 | PA9 / PA10 | 115200 8N1 |
| LED 预留 | GPIO | PA1 | 控制 NMOS 栅极，当前已停用 |

## 软件流程

1. 初始化 `GPIO / ADC / I2C / UART`
2. 扫描 `I2C` 设备
3. 初始化 `OLED`
4. 初始化 `AHT20`
5. 周期读取 `AHT20` 与 `NTC`
6. 计算融合温度
7. 刷新 `OLED` 并输出串口日志

## NTC 温度模型

当前 NTC 采用 `B` 值模型，不再使用 Steinhart-Hart 系数。

已知标定点：

- `R25 = 10kΩ`
- `R50 = 3.588kΩ`
- `T25 = 25°C`
- `T50 = 50°C`

先计算 `B` 值：

```math
B = \frac{T_a \times T_b}{T_b - T_a} \times \ln\left(\frac{R_a}{R_b}\right)
```

其中温度使用开尔文：

```math
T_a = 25 + 273.15,\quad T_b = 50 + 273.15
```

代入后：

```math
B \approx 3950.196K
```

再根据当前电阻值反推温度：

```math
\frac{1}{T} = \frac{1}{T_{25}} + \frac{\ln(R / R_{25})}{B}
```

最后换算为摄氏度：

```math
T(^\circ C) = T(K) - 273.15
```

## 分压关系

代码当前按以下接法计算：

- `10k` 上拉到 `3.3V`
- `NTC` 下拉到 `GND`
- `ADC` 采集中点电压

对应电阻公式：

```math
R_{NTC} = \frac{V_{adc} \times R_{ref}}{V_{ref} - V_{adc}}
```

如果你的硬件接法和这里不同，需要同步修改公式。

## 串口输出示例

```text
[I2C1] Scan start...
[I2C1] Found: 7-bit 0x38, 8-bit W 0x70
[I2C1] Found: 7-bit 0x3C, 8-bit W 0x78
[I2C1] Scan done, total 2 device(s).

AHT20 T=26.41 C, H=52.83 %RH | NTC ADC=1842, V=1.484 V, R=8180 ohm, T=30.11 C | AVG T=28.26 C
```

## 构建

1. 打开 `MDK-ARM/Desktop.uvprojx`
2. 选择目标 `Desktop`
3. 编译并下载
4. 打开串口观察输出

## 代码位置

| 功能 | 文件 |
|---|---|
| AHT20 读取 | `Core/Src/main.c` |
| NTC 采样与温度计算 | `Core/Src/main.c` |
| OLED 显示 | `Core/Src/main.c` / `Core/Src/oled.c` |
| GPIO / ADC / I2C / UART 初始化 | `Core/Src/main.c` / `Core/Src/stm32g0xx_hal_msp.c` |

## 当前状态

- `AHT20` 采集正常
- `NTC` 已切换为 `B=3950` 标定模型
- `PA1` 呼吸灯逻辑已保留但默认注释停用
