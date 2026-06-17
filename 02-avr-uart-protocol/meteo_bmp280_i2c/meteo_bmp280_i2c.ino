#include <Wire.h>
#include <GyverBME280.h>

#define SetBit(reg, bita) reg |= (1<<bita)
#define SYNC_BYTE 0x5A
#define BUFFER_SIZE 32

GyverBME280 bme;
volatile uint8_t rxBuffer[BUFFER_SIZE];  // Буфер для приема данных
volatile uint8_t rxIndex = 0;            // Индекс в буфере
volatile bool packetStarted = false;     // Флаг начала пакета
volatile uint8_t packetLength = 0;       // Длина пакета
volatile bool sendSensorData = false;    // Флаг отправки данных с датчика

// Функция расчета контрольной суммы CRC8
uint8_t calculateCRC8(uint8_t *data, uint8_t length) {
  uint8_t crc = 0;
  for (uint8_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ 0x07;
      } else {
        crc = crc << 1;
      }
    }
  }
  return crc;
}

// Функция отправки байта по UART
void UART_sendByte(uint8_t data) {
  // Ожидаем, пока буфер передачи не освободится
  while (!(UCSR0A & (1 << UDRE0)));
  // Отправляем байт
  UDR0 = data;
}

// Функция отправки пакета данных
void sendPacket(uint8_t* data, uint8_t length) {
  // Отправляем синхробайт
  UART_sendByte(SYNC_BYTE);
  
  // Отправляем длину данных
  UART_sendByte(length);
  
  // Отправляем сами данные
  for (uint8_t i = 0; i < length; i++) {
    UART_sendByte(data[i]);
  }
  
  // Рассчитываем и отправляем CRC
  uint8_t packet[length + 2];
  packet[0] = SYNC_BYTE;
  packet[1] = length;
  for (uint8_t i = 0; i < length; i++) {
    packet[i + 2] = data[i];
  }
  
  uint8_t crc = calculateCRC8(packet, length + 2);
  UART_sendByte(crc);
}

void setup() {
  // 1. Настраиваем UART (19200 бод, четная четность, 1 стоп-бит)
  uint16_t baudRate = 19200;
  uint16_t ubrr = 16000000 / 16 / baudRate - 1;

  UBRR0H = (unsigned char) (ubrr >> 8);
  UBRR0L = (unsigned char) ubrr;

  SetBit(UCSR0B, TXEN0);  // Включаем передатчик
  SetBit(UCSR0B, RXEN0);  // Включаем приемник
  SetBit(UCSR0B, RXCIE0); // Включаем прерывание по приему

  SetBit(UCSR0C, 1);      // UCSZ00 - 8-битный размер символа
  SetBit(UCSR0C, 2);      // UCSZ01 - 8-битный размер символа
  SetBit(UCSR0C, 5);      // UPM01 - включение четной четности
  
  // 2. Инициализируем датчик BMP280
  Wire.begin();
  if (!bme.begin(0x76)) {
    // Если не удалось инициализировать датчик, зажигаем светодиод
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);
    while (1); // Бесконечный цикл, микроконтроллер "застрял"
  }
  
  // 3. Настраиваем таймер для отправки данных раз в секунду
  // Очищаем регистры
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  
  // Устанавливаем значение сравнения для таймера
  // Для частоты 16 МГц, делитель 1024 и период 1 сек: 16000000/1024/1 = 15625
  OCR1A = 15624; // Корректировка на -1, так как считается от 0
  
  // Устанавливаем режим CTC (сброс при совпадении)
  TCCR1B |= (1 << WGM12);
  
  // Устанавливаем делитель 1024
  TCCR1B |= (1 << CS12) | (1 << CS10);
  
  // Разрешаем прерывание по совпадению
  TIMSK1 |= (1 << OCIE1A);
  
  // Разрешаем глобальные прерывания
  sei();
}

// Обработчик прерывания по таймеру (срабатывает раз в секунду)
ISR(TIMER1_COMPA_vect) {
  sendSensorData = true; // Устанавливаем флаг для отправки данных с датчика
}

// Обработчик прерывания по приему данных UART
ISR(USART_RX_vect) {
  uint8_t receivedByte = UDR0; // Считываем принятый байт
  
  // Если получен синхробайт, начинаем новый пакет
  if (receivedByte == SYNC_BYTE) {
    packetStarted = true;
    rxIndex = 0;
    rxBuffer[rxIndex++] = receivedByte;
    return;
  }
  
  // Если пакет уже начался, обрабатываем остальные байты
  if (packetStarted) {
    // Сохраняем байт в буфер
    rxBuffer[rxIndex++] = receivedByte;
    
    // Если это второй байт (длина пакета)
    if (rxIndex == 2) {
      packetLength = receivedByte;
      // Проверка на корректность длины
      if (packetLength > BUFFER_SIZE - 3) { // -3 учитывая синхробайт, длину и CRC
        packetStarted = false;
        return;
      }
    }
    
    // Если получили все данные (синхробайт + длина + данные + CRC)
    if (rxIndex == packetLength + 3) {
      // Проверка CRC
      uint8_t calculatedCRC = calculateCRC8(rxBuffer, rxIndex - 1);
      uint8_t receivedCRC = rxBuffer[rxIndex - 1];
      
      if (calculatedCRC == receivedCRC) {
        // Пакет корректный, отправляем его обратно
        uint8_t data[BUFFER_SIZE];
        for (uint8_t i = 0; i < packetLength; i++) {
          data[i] = rxBuffer[i + 2]; // Пропускаем синхробайт и длину
        }
        sendPacket(data, packetLength);
      }
      
      // Сбрасываем состояние пакета
      packetStarted = false;
    }
  }
}

void loop() {
  // Если установлен флаг отправки данных с датчика
  if (sendSensorData) {
    sendSensorData = false; // Сбрасываем флаг
    
    // Получаем данные с датчика
    float temperature = bme.readTemperature();
    float pressure = bme.readPressure();
    
    // Подготавливаем данные для отправки (8 байт = 2 float значения)
    uint8_t sensorData[8];
    // Копируем температуру (4 байта)
    memcpy(sensorData, &temperature, 4);
    // Копируем давление (4 байта)
    memcpy(sensorData + 4, &pressure, 4);
    
    // Отправляем пакет с данными
    sendPacket(sensorData, 8);
  }
}
