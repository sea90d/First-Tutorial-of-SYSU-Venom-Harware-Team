# 鍩轰簬 STM32G030 鐨勫婧愮幆澧冩俯搴︽祴閲忎笌 OLED 鍙鍖栫郴缁?
> 璇剧▼/鍩硅椤圭洰锛欼2C 娓╂箍搴﹂噰闆?+ ADC 鐑晱鐢甸樆娴嬫俯 + SSD1306 鏄剧ず + 涓插彛涓婁綅鏈鸿緭鍑? 
> MCU锛歋TM32G030F6Px锛圚AL 椹卞姩锛? 
> Author: `sea90d`

---

## 鎽樿

鏈枃璁捐骞跺疄鐜颁簡涓€涓祵鍏ュ紡鐜鍙傛暟閲囬泦绯荤粺锛岄噰鐢?**AHT20 鏁板瓧娓╂箍搴︿紶鎰熷櫒** 涓?**NTC 鐑晱鐢甸樆妯℃嫙娴嬫俯閫氶亾** 涓よ矾娓╁害淇℃伅杩涜铻嶅悎浼拌锛岃緭鍑虹幆澧冩俯搴︿笌婀垮害銆傜郴缁熷熀浜?STM32G030 骞冲彴锛屼娇鐢?I2C1 鎬荤嚎椹卞姩 AHT20 涓?SSD1306 OLED锛屼娇鐢?ADC1_IN0 閲囬泦鍒嗗帇鐢佃矾鐢靛帇骞堕€氳繃 Steinhart-Hart 鏂圭▼璁＄畻鐑晱鐢甸樆娓╁害锛屾渶缁堝湪 OLED 瀹炴椂鏄剧ず锛屽苟閫氳繃 USART1锛圥A9/PA10锛夊悜澶栧彂閫佷覆鍙ｆ暟鎹€? 
涓烘彁楂樺彲璋冭瘯鎬э紝绯荤粺鍦ㄥ惎鍔ㄩ樁娈靛姞鍏?I2C 鍦板潃鎵弿鏈哄埗锛岃嚜鍔ㄦ姤鍛婃€荤嚎鍦ㄧ嚎璁惧鍦板潃銆傞」鐩疄鐜颁簡浠庣‖浠舵帴鍙ｅ畾涔夈€侀┍鍔ㄧЩ妞嶃€佹暟鎹鐞嗗埌鍙鍖栬緭鍑虹殑瀹屾暣闂幆銆?
**鍏抽敭璇?*锛歋TM32G030锛汚HT20锛汼SD1306锛汵TC锛汼teinhart-Hart锛汭2C 鎵弿锛涘婧愭俯搴﹁瀺鍚?
---

## 1. 绯荤粺鎬讳綋璁捐

### 1.1 纭欢杩炴帴

| 澶栬 | MCU鎺ュ彛 | 寮曡剼 | 璇存槑 |
|---|---|---|---|
| AHT20 娓╂箍搴︿紶鎰熷櫒 | I2C1 | PB6(SCL), PB7(SDA) | 鍦板潃閫氬父涓?`0x38`锛?-bit锛?|
| SSD1306 OLED | I2C1 | PB6(SCL), PB7(SDA) | 鍦板潃閫氬父涓?`0x3C/0x3D`锛?-bit锛?|
| NTC 鐑晱鐢甸樆鍒嗗帇 | ADC1_IN0 | PA0 | 璇诲彇鍒嗗帇骞舵崲绠楁俯搴?|
| 涓插彛杈撳嚭 | USART1 | PA9(TX), PA10(RX) | 115200 bps |

### 1.2 纭欢鏋舵瀯鍥撅紙Figure 1锛?
```mermaid
graph LR
    MCU[STM32G030F6Px]
    I2C[I2C1 PB6/PB7]
    ADC[ADC1_IN0 PA0]
    UART[USART1 PA9/PA10]

    MCU --> I2C
    MCU --> ADC
    MCU --> UART

    I2C --> AHT20[AHT20 娓╂箍搴
    I2C --> OLED[SSD1306 OLED]
    ADC --> NTC[NTC 鍒嗗帇鐢佃矾]
    UART --> PC[涓婁綅鏈?涓插彛缁堢]
```

---

## 2. 杞欢鏋舵瀯涓庡伐浣滄祦

### 2.1 杞欢娴佺▼鍥撅紙Figure 2锛?
```mermaid
flowchart TD
    A[绯荤粺涓婄數] --> B[HAL_Init + 鏃堕挓閰嶇疆]
    B --> C[GPIO/ADC/I2C/UART 鍒濆鍖朷
    C --> D[I2C1 鎵弿鍦板潃骞朵覆鍙ｆ墦鍗癩
    D --> E[OLED 鍒濆鍖朷
    E --> F[AHT20 鍒濆鍖朷
    F --> G{涓诲惊鐜?1s}
    G --> H[AHT20 瑙﹀彂娴嬮噺骞惰鍙朷
    G --> I[ADC 澶氭閲囨牱 + NTC娓╁害璁＄畻]
    H --> J[铻嶅悎娓╁害璁＄畻]
    I --> J
    J --> K[OLED 鍒锋柊鏄剧ず]
    J --> L[USART1 鎵撳嵃鏁版嵁]
    K --> G
    L --> G
```

### 2.2 浠ｇ爜妯″潡鏄犲皠

| 妯″潡 | 涓昏鍑芥暟 | 鏂囦欢 |
|---|---|---|
| I2C 鍦板潃鎵弿 | `I2C1_ScanDevices()` | `Core/Src/main.c` |
| AHT20 椹卞姩 | `AHT20_Init()` `AHT20_Read()` | `Core/Src/main.c` |
| ADC/NTC 娴嬫俯 | `Read_ADC_Voltage()` `Calculate_NTC_Temperature()` `Read_NTC_Temperature()` | `Core/Src/main.c` |
| 娓╁害铻嶅悎涓庤緭鍑?| `OLED_ShowEnvData()` `UART_PrintEnvData()` | `Core/Src/main.c` |
| OLED 椹卞姩绉绘 | `OLED_Send()`锛堢粦瀹?`hi2c1`锛?| `Core/Src/oled.c` |
| 涓插彛寮曡剼涓庢椂閽?| USART1 MSP 閰嶇疆 | `Core/Src/stm32g0xx_hal_msp.c` |

---

## 3. 鍏抽敭鏂规硶涓庢暟瀛︽ā鍨?
### 3.1 AHT20 鏁版嵁鎹㈢畻

璁惧師濮?20-bit 婀垮害鍊间负 $S_{RH}$锛屾俯搴﹀€间负 $S_T$锛屽垯锛?
$$
RH(\%) = \frac{S_{RH}}{2^{20}} \times 100
$$

$$
T_{AHT20}(^\circ C)=\frac{S_{T}}{2^{20}} \times 200 - 50
$$

### 3.2 NTC 娓╁害璁＄畻锛圫teinhart-Hart锛?
鐢卞垎鍘嬪叧绯伙紙涓婃媺鐢甸樆 $R_{ref}=10k\Omega$锛夛細

$$
R_{NTC} = \frac{(V_{ref}-V_{adc})\cdot R_{ref}}{V_{adc}}
$$

闅忓悗浣跨敤 Steinhart-Hart 褰㈠紡锛?
$$
\frac{1}{T(K)} = A + B\ln\left(\frac{R_{NTC}}{R_0}\right) + C\left[\ln\left(\frac{R_{NTC}}{R_0}\right)\right]^3
$$

鏈」鐩父鏁帮細
- $A=0.001129148$
- $B=0.000234125$
- $C=0.0000000876741$
- $R_0=10k\Omega$

鏈€鍚庯細

$$
T_{NTC}(^\circ C)=T(K)-273.15
$$

### 3.3 澶氭簮娓╁害铻嶅悎

褰撳墠瀹炵幇閲囩敤绛夋潈骞冲潎锛?
$$
T_{avg}=\frac{T_{AHT20}+T_{NTC}}{2}
$$

褰?AHT20 璇绘暟寮傚父鏃讹紝閫€鍖栦负 NTC 鍗曡矾杈撳嚭銆?
---

## 4. 宸ョ▼瀹炵幇姝ラ锛堜笌鏈」鐩紑鍙戣繃绋嬩竴鑷达級

1. 鍩轰簬 CubeMX 鐢熸垚 STM32G030 HAL 宸ョ▼楠ㄦ灦锛圓DC/I2C/UART/GPIO锛夈€?2. 灏?SSD1306 椹卞姩锛坄oled.c/.h`銆乣font.c/.h`锛夎縼绉诲埌 `Core` 鐩綍骞跺姞鍏?Keil 宸ョ▼銆?3. 淇 OLED 搴曞眰鍙戦€佹帴鍙ｏ細灏?I2C 鍙ユ焺缁戝畾鍒?`hi2c1`銆?4. 缂栧啓 AHT20 鍒濆鍖栦笌娴嬮噺娴佺▼锛堣蒋澶嶄綅銆佺姸鎬佹鏌ャ€乥usy 杞锛夈€?5. 缂栧啓 ADC 閲囨牱 + NTC 娓╁害鎹㈢畻鍑芥暟锛屽苟閲囩敤澶氭閲囨牱骞冲潎鎶戝埗鍣０銆?6. 瀹炵幇 OLED 鍥涜鏄剧ず锛歚T_AHT20`銆乣T_NTC`銆乣T_AVG`銆乣HUM`銆?7. 瀹炵幇 USART1 鏂囨湰杈撳嚭锛屼究浜庝笂浣嶆満璁板綍涓庤皟璇曘€?8. 澧炲姞 `I2C1_ScanDevices()` 鍚姩鎵弿锛岃嚜鍔ㄥ畾浣嶄粠鏈哄湴鍧€銆?9. 淇涓插彛閰嶇疆涓烘澘绾у疄闄呰繛鎺ワ細`USART1 PA9/PA10`銆?
---

## 5. 鍚姩鎵弿涓庡吀鍨嬭緭鍑?
### 5.1 I2C 鎵弿杈撳嚭锛堜覆鍙ｏ級

```text
[I2C1] Scan start...
[I2C1] Found: 7-bit 0x38, 8-bit W 0x70
[I2C1] Found: 7-bit 0x3C, 8-bit W 0x78
[I2C1] Scan done, total 2 device(s).
```

> 娉細SSD1306 甯歌 7-bit 鍦板潃涓?`0x3C`锛堝搴?8-bit 鍐欏湴鍧€ `0x78`锛夛紝涔熷彲鑳戒负 `0x3D`锛坄0x7A`锛夈€?
### 5.2 涓诲惊鐜緭鍑猴紙涓插彛锛?
```text
AHT20 T=26.41 C, H=52.83 %RH | NTC T=26.09 C | AVG T=26.25 C
```

---

## 6. 宸ョ▼鐩綍锛堟牳蹇冿級

```text
tutorial/
鈹溾攢 Core/
鈹? 鈹溾攢 Inc/
鈹? 鈹? 鈹溾攢 main.h
鈹? 鈹? 鈹溾攢 oled.h
鈹? 鈹? 鈹斺攢 font.h
鈹? 鈹斺攢 Src/
鈹?    鈹溾攢 main.c
鈹?    鈹溾攢 oled.c
鈹?    鈹溾攢 font.c
鈹?    鈹斺攢 stm32g0xx_hal_msp.c
鈹溾攢 Drivers/
鈹斺攢 MDK-ARM/
   鈹斺攢 Desktop.uvprojx
```

---

## 7. 缂栬瘧涓庝笅杞?
### 7.1 Keil MDK

1. 鎵撳紑 `MDK-ARM/Desktop.uvprojx`
2. 閫夋嫨鐩爣 `Desktop`
3. Build
4. 涓嬭浇鍒版澘鍗″苟鎵撳紑涓插彛缁堢锛?15200, 8N1锛?
### 7.2 涓插彛鍙傛暟

- 娉㈢壒鐜囷細`115200`
- 鏁版嵁浣嶏細`8`
- 鍋滄浣嶏細`1`
- 鏍￠獙浣嶏細`None`

---

## 8. 璇樊鏉ユ簮涓庤璁?
1. **浼犳劅鍣ㄥ浐鏈夎宸?*锛欰HT20 涓?NTC 鍧囨湁鏍囧畾璇樊涓庢俯婕傘€?2. **鐢垫簮涓?ADC 鍙傝€冩尝鍔?*锛歚Vref` 鍋囪涓?3.3V锛屽疄闄呮尝鍔ㄤ細寮曞叆 NTC 娓╁害鍋忓樊銆?3. **鍒嗗帇鐢甸樆绮惧害**锛歚10k惟` 鐢甸樆瀹瑰樊鐩存帴褰卞搷 `R_NTC` 浼拌绮惧害銆?4. **鑷儹鏁堝簲涓庡竷灞€**锛歂TC 璐磋繎鍙戠儹鍣ㄤ欢浼氫骇鐢熺郴缁熷亸宸€?5. **铻嶅悎绛栫暐绠€鍗?*锛氱瓑鏉冨钩鍧囨湭鍒╃敤鍔ㄦ€佺疆淇″害锛屽悗缁彲浣跨敤鍔犳潈铻嶅悎鎴栧崱灏旀浖婊ゆ尝銆?
---

## 9. 鍙鐜版€у缓璁?
涓哄鐜板疄楠岀粨鏋滐紝寤鸿鍥哄畾浠ヤ笅鏉′欢锛?
- 鐜娓╁害绋冲畾锛岄伩鍏嶅己瀵规祦涓庣洿鍚广€?- 鏉垮崱 3.3V 渚涚數绋冲畾銆?- NTC 鍙傝€冪數闃讳娇鐢?1% 鎴栨洿楂樼簿搴﹀櫒浠躲€?- OLED 涓?AHT20 鍏辩嚎 I2C 鎷夌數闃诲尮閰嶏紙甯歌 4.7k惟锛夈€?- 姣忔鏀瑰姩鍚庤褰曚覆鍙ｆ棩蹇楀苟淇濈暀鐗堟湰鍙凤紙Git commit hash锛夈€?
---

## 10. 鍚庣画鏀硅繘鏂瑰悜

- 铻嶅悎绠楁硶鍗囩骇锛氬熀浜庢柟宸及璁＄殑鑷€傚簲鍔犳潈銆?- 鏁版嵁璁板綍锛氬紩鍏ョ幆褰㈢紦瀛樻垨澶栭儴瀛樺偍鐢ㄤ簬闀挎湡缁熻銆?- 鏍″噯鏈哄埗锛氬鍔犱袱鐐规爣瀹氾紙浣庢俯/甯告俯锛夎ˉ鍋垮弬鏁般€?- UI 鏀硅繘锛歄LED 澧炲姞瓒嬪娍鏇茬嚎鍜岀姸鎬佸浘鏍囥€?
---

## 鍙傝€冭祫鏂?
1. Aosong, *AHT20 Datasheet*  
2. Solomon Systech, *SSD1306 Datasheet*  
3. STMicroelectronics, *STM32G0 HAL User Manual*  
4. Steinhart, J. S., & Hart, S. R. (1968). Calibration curves for thermistors.

---

## 璁稿彲

鏈粨搴撶敤浜庡涔犱笌瀹為獙锛岄粯璁ら伒寰師鍘?HAL 涓庣涓夋柟椹卞姩鍚勮嚜璁稿彲鏉℃銆?

