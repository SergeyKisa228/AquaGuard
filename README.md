# AquaGuard 💧
A wireless water-leak detector based on STM32F401 and ESP32.   It monitors a water sensor, drives an RGB indicator, and sends instant alerts (`WARNING` / `GOOD`) to a smartphone via Bluetooth Classic.

<img width="720" height="1280" alt="proj" src="https://github.com/user-attachments/assets/b4ee22e5-d0b0-42be-9967-224877475422" />

# Description 🙂

The system continuously reads the analogue output of a T1592 water sensor using the STM32’s ADC.  
When water is detected, the RGB LED blinks red, and the STM32 transmits `"WARNING\n"` over UART.  
When the surface is dry, the LED stays green, and `"GOOD\n"` is sent.  
All messages are forwarded by an ESP32 to a mobile phone via Bluetooth Classic (SPP).  
A push button toggles the whole monitoring on/off, and the phone receives `"SYSTEM ON"` / `"SYSTEM OFF"` accordingly.  
Status reports are repeated periodically (WARNING every 5 s, GOOD every 20 s) while the system is active.

# Hardware 🔧

- **STM32F401RCT6** (main controller, ADC + UART + timer)
- **ESP32‑WROOM** (Bluetooth bridge)
- **T1592 water sensor** (3‑pin analogue module: `+`, `–`, `S`)
- **RGB LED** 3× 220 Ω resistors
- **Push button** (for system on/off)

# Schematic ❗
<img width="632" height="588" alt="Scematic" src="https://github.com/user-attachments/assets/93acac6f-acad-4c37-83aa-99985e5414cd" />


# ESP32
- ESP32 UART2-RX GPIO16 PA9 (USART1_TX)
- ESP32 UART2-TX GPIO17 PA10 (USART1_RX) 
- ESP32 GND - GND

# How it works 💬

**STM32 firmware (CubeIDE)**  
- System clock: 84 MHz from 25 MHz HSE.  
- ADC1 (PA0) continuously samples the water sensor. A threshold determines the `water_present` flag.  
- TIM2 generates an interrupt every 500 ms to blink the red LED when water is present.  
- USART1 (115200 baud) transmits messages to ESP32.  
- EXTI on PB10 (button) toggles `system_active`. A software debounce with a 150 ms lock‑out ensures clean switching.  
- Global flags `send_system_status` and `periodic_counter` handle the first‑time message and periodic repetitions without flooding the channel.

**ESP32 firmware (Arduino)**  
- UART2 (GPIO16/17) receives lines from STM32.  
- A software filter accepts only predefined strings (`SYSTEM ON`, `SYSTEM OFF`, `WARNING`, `GOOD`) – any noise produced during STM32 startup is ignored.  
- Data is relayed to a smartphone via Classic Bluetooth SPP (`BluetoothSerial` library). A PIN code (`11111`) secures the connection.  
- Optionally, the on‑board LED (GPIO2) can be toggled by commands `'1'` / `'0'` from the phone.

## Автор
👾 **SergeyKisa228** 👾
