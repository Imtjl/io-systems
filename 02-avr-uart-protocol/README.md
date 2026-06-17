# 02 · AVR firmware + framed UART protocol

*On the metal.* Register-level firmware for an **ATmega328** (Arduino UNO) that
reads a **BMP280** over I²C and streams it to a host over a **custom framed UART
protocol** with CRC8. A Python client drives and verifies the link.

## What it demonstrates

- **Register-level peripherals, no `Serial`** — UART configured directly
  (`UBRR0`, `UCSR0A/B/C` → **19200 baud, 8E1**), `Timer1` in **CTC** mode for a
  1 Hz tick, and two ISRs (`TIMER1_COMPA_vect`, `USART_RX_vect`).
- **I²C sensor** — BMP280 read via `Wire` / `GyverBME280`.
- **A real link-layer protocol** — frames are `SYNC(0x5A) | length | payload |
  CRC8` (polynomial `0x07`), with state-machine framing in the RX ISR and
  rejection of bad sync / length / CRC.
- **Host counterpart** — `client.py` sends valid and deliberately broken frames
  (no sync / wrong length / wrong CRC), monitors the sensor stream, checks CRC,
  and unpacks the floats.

This is the seminar's protocol design (project 04) turned into working firmware.

## Run

Flash the sketch, then on the host:

```bash
python client.py --port /dev/ttyUSB0
```

**Requirements:** Arduino toolchain + `GyverBME280`; `pyserial` on the host.

## Files

| File | Role |
|------|------|
| `meteo_bmp280_i2c/meteo_bmp280_i2c.ino` | AVR firmware: UART/timer/ISRs, BMP280, framing |
| `client.py` | host: send/verify frames, monitor sensor stream |
