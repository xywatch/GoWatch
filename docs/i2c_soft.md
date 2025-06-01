如果是 I2C_Start() 后首次发送地址（即 设备地址 + 方向）：
写操作：(0x51 << 1) | 0 = 0xA2
（向 PCF8563 写入数据时发送 0xA2）

读操作：(0x51 << 1) | 1 = 0xA3
（从 PCF8563 读取数据时发送 0xA3）

（2）如果是发送寄存器地址（如 0x00、0x02 等）：
直接发送 寄存器地址本身（无需左移或处理方向位），例如：
I2C_SendByte(0x02); // 发送秒寄存器地址

3. 在 i2c_soft.c 中的实际应用
根据 i2c_soft.c 的实现：

Single_Write 函数：
内部先发送 设备写地址 0xA2，再发送寄存器地址。

```
I2C_Start();
I2C_SendByte(0xA2);      // 设备地址 + 写
I2C_SendByte(REG_Address); // 寄存器地址
Single_Read 函数：
内部先发送 设备写地址 0xA2（用于指定寄存器），再发送 设备读地址 0xA3。
```

```
I2C_Start();
I2C_SendByte(0xA2);      // 设备地址 + 写
I2C_SendByte(REG_Address); // 寄存器地址
I2C_Start();
I2C_SendByte(0xA3);      // 设备地址 + 读
```
