# 04 · Designing a custom serial interface

*On the wire — by design.* A from-scratch interface spec built to a constraint:
**3 lines, synchronous, half-duplex**. Covers the physical, link, and transport
layers, with a worked throughput budget.

## What it demonstrates

- **Port / PHY design** — `CLK` (clock), `DATA` (bidirectional), `CMD` (R/W
  direction select), `GND`; bus topology; I²C-style addressing carried on the
  data line.
- **Link-layer framing** — a service frame (slave address + message length +
  ACK) followed by data frames (payload + redundancy code + ACK); reliability
  via the redundancy code and ACK/NACK.
- **Throughput budget** — derived effective bandwidth on a 1 Mbit/s line:
  **235.3 kbit/s** worst case (single-byte message, 34 bits to move 8 payload
  bits) up to **468.8 kbit/s** at the 256-byte maximum.
- **Transport layer & use cases** — power-on noise and packet-loss handling;
  applied to industrial temperature monitoring, smart-building lighting, and
  secure access control.

Project 02 is essentially this design realized in firmware (a SYNC/length/CRC
frame over UART).

## Note

Design deliverable: [`report.pdf`](report.pdf).
