# GoWatch
<img src="./images/preview.jpg">

本项目有3个分支, 大致功能都一样. v1使用mpu6050, v2/v3使用BMA423. 详细区别如下:

| 分支 | MCU芯片 | RTC | 运动传感器 | 气压温度传感器 |
| ---- | ---- | ---- | ---- | ---- |
| [v1](https://github.com/xywatch/GoWatch/tree/v1) | STM32F103CBT6 | [DS3231MZ+TRL](https://item.taobao.com/item.htm?id=784371508620) | [MPU6050](https://detail.tmall.com/item.htm?id=598555471828&skuId=5780956225523) | [BME280](https://item.taobao.com/item.htm?id=641115619117) |
| [v2](https://github.com/xywatch/GoWatch/tree/v2) | STM32F103CBT6 | [PCF8563BS](https://item.taobao.com/item.htm?id=756706038432&spm=tbpc.boughtlist.suborder_itemtitle.1.42092e8d5rsiuO) | [BMA423](https://item.taobao.com/item.htm?_u=nfl2f5gc82f&id=585190973333&sku_properties=-1%3A-1&spm=a1z09.2.0.0.54c72e8dzYZERC) | 无 |
| [v3](https://github.com/xywatch/GoWatch/tree/v3) | STM32L151CBT6 | [PCF8563BS](https://item.taobao.com/item.htm?id=756706038432&spm=tbpc.boughtlist.suborder_itemtitle.1.42092e8d5rsiuO) | [BMA423](https://item.taobao.com/item.htm?_u=nfl2f5gc82f&id=585190973333&sku_properties=-1%3A-1&spm=a1z09.2.0.0.54c72e8dzYZERC) | 无 |

v1使用了MPU6050导致功耗比较大, 200毫安电池只能使用一天. v2/v3可使用3-4天.

其它相同的元件:

|      | 型号      | 购买链接      |
|------------|------------|------------|
| TYPE-C   | TYPE-C 母座 16P/沉板1.6     |    [淘宝](https://item.taobao.com/item.htm?id=847085611169&spm=tbpc.boughtlist.suborder_itemtitle.1.42092e8d5rsiuO)   |
| 无源蜂鸣器 | MLT-5020 3v | [淘宝](https://item.taobao.com/item.htm?id=904963885727&spm=tbpc.boughtlist.suborder_itemtitle.1.42092e8d5rsiuO) |
| 0.96寸OLED显示屏 | 蓝光 | [淘宝](https://item.taobao.com/item.htm?id=771751339592&skuId=5457594059803&spm=tbpc.boughtlist.suborder_itemtitle.1.42092e8d5rsiuO) |
| 1.27mm间距排母 | 1x7P（塑高2.0mm）| [淘宝](https://item.taobao.com/item.htm?_u=nfl2f5gee9e&id=533845762209&skuId=3898422332914&spm=a1z09.2.0.0.54c72e8dzYZERC) |
| 三向开关 | LY-K6-01 | [淘宝](https://item.taobao.com/item.htm?_u=nfl2f5g573c&id=607224934874&spm=a1z09.2.0.0.54c72e8dzYZERC) |
| 轻触开关 | 2x3x1.5mm | [淘宝](https://item.taobao.com/item.htm?_u=nfl2f5g09ac&id=852427078276&spm=a1z09.2.0.0.54c72e8dzYZERC) |
|PCB连接器螺丝| M1.4x3 | [淘宝](https://detail.tmall.com/item.htm?_u=nfl2f5gd49f&id=671590959819&skuId=5199969587667&spm=a1z09.2.0.0.54c72e8dzYZERC) |
|PCB连接器铜柱| M1.4x2.5x3 | [淘宝](https://item.taobao.com/item.htm?_u=nfl2f5g9475&id=809151694062&spm=a1z09.2.0.0.54c72e8dzYZERC) |
|3.7V电池| 502025-200毫安 (如需要更薄, 可以买302025电池) | [淘宝](https://item.taobao.com/item.htm?id=784361648217&spm=tbpc.boughtlist.suborder_itemtitle.1.42092e8d5rsiuO)|
|表带|表带宽15mm|[淘宝](https://item.taobao.com/item.htm?id=872619805915&spm=tbpc.boughtlist.suborder_itemtitle.1.42092e8d5rsiuO)|

## 如何烧录
<img src="./images/dowload.png">

## 如何调试串口
<img src="./images/serial.png">
