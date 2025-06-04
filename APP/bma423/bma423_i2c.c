#include "bma423_i2c.h"
#include "i2c_soft.h"
#include "delay.h"

// 7位设备地址左移1位，变成8位
// 最低位用于表示读/写操作（0=写，1=读）

uint16_t _readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len)
{
    uint16_t i;
    
    // Start I2C communication
    if (!I2C_Start()) {
        return 1;  // Error: Bus busy
    }
    
    // Send device address (write mode)
    I2C_SendByte(address << 1);  // Shift address left by 1 bit
    if (!I2C_WaitAck()) {
        I2C_Stop();
        return 2;  // Error: No ACK from device
    }
    
    // Send register address
    I2C_SendByte(reg);
    if (!I2C_WaitAck()) {
        I2C_Stop();
        return 3;  // Error: No ACK from device
    }
    
    // Restart I2C communication for reading
    if (!I2C_Start()) {
        return 4;  // Error: Bus busy
    }
    
    // Send device address (read mode)
    I2C_SendByte((address << 1) | 0x01);  // Shift address left and set read bit
    if (!I2C_WaitAck()) {
        I2C_Stop();
        return 5;  // Error: No ACK from device
    }
    
    // Read data
    for (i = 0; i < len; i++) {
        data[i] = I2C_ReceiveByte();
        if (i < len - 1) {
            I2C_Ack();  // Send ACK for all bytes except the last one
        } else {
            I2C_NoAck();  // Send NACK for the last byte
        }
    }
    
    I2C_Stop();
    return 0;  // Success
}

uint16_t _writeRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len)
{
    uint16_t i;
    
    // Start I2C communication
    if (!I2C_Start()) {
        return 1;  // Error: Bus busy
    }
    
    // Send device address (write mode)
    I2C_SendByte(address << 1);  // Shift address left by 1 bit
    if (!I2C_WaitAck()) {
        I2C_Stop();
        return 2;  // Error: No ACK from device
    }
    
    // Send register address
    I2C_SendByte(reg);
    if (!I2C_WaitAck()) {
        I2C_Stop();
        return 3;  // Error: No ACK from device
    }
    
    // Send data
    for (i = 0; i < len; i++) {
        I2C_SendByte(data[i]);
        if (!I2C_WaitAck()) {
            I2C_Stop();
            return 4;  // Error: No ACK from device
        }
    }
    
    I2C_Stop();
    return 0;  // Success
} 