# 03 · Decoding I²C & 1-Wire on a logic analyzer

*On the wire.* Capturing real bus traffic with a logic analyzer and decoding it
**by hand**, byte by byte, then turning the raw sensor registers into physical
values. This is the bus-level side of functional debugging.

## What it demonstrates

- **Reading raw waveforms** — recorded the timing diagrams of three exchanges on
  the signal lines and decoded the protocol manually.
- **I²C — BMP280:** read the calibration table from the trace
  (`dig_T1 = 28089`, `dig_T2 = 26581`, `dig_T3 = -1000`), read the raw
  temperature (`adc_T = 535648`), applied the Bosch compensation formula →
  **T = 27.24 °C**.
- **1-Wire — DHT-11:** decoded the single-line timing into **35.0 % RH / 27.2 °C**
  and verified the checksum (`35 + 0 + 27 + 2 = 64` ✓).
- **Engineering trade-off** (from the conclusion): 1-Wire is trivial to decode
  but niche; I²C needs calibration math and multi-step fixed-point work but is
  universal and multi-drop — simplicity vs. scalability.

## Note

This lab is analysis, not code — the deliverable is the decoded write-up in
[`report.pdf`](report.pdf).

> **TODO:** add a screenshot of the logic-analyzer timing diagram (BMP280 / I²C)
> here — it's the strongest visual proof that the bus was decoded from raw
> signals, and renders inline on GitHub.
