# Smart-Parking-System-Mangement-With-Slot-Availability-PIC16F877A
# Smart Parking Management System Using PIC16F877A

## Project Information

This project was developed as part of the Microprocessor Systems course.

The Smart Parking System automatically manages vehicle entry and exit while monitoring available parking spaces in real time.

The system uses PIC16F877A, IR sensors, servo motors, an LCD display, LEDs, and a buzzer to provide an automated parking management solution.

---

## Features

- Automatic vehicle counting
- Real-time parking availability display
- Automatic entrance gate control
- Automatic exit gate control
- Parking full detection
- Audio notification using buzzer
- Status indication LEDs
- Edge-triggered IR sensor detection
- Software-generated servo PWM control
- Software-generated I2C communication

---

## Hardware Components

- PIC16F877A Microcontroller
- 20MHz Crystal Oscillator
- 16x2 LCD Display with I2C Backpack
- 2 IR Obstacle Sensors
- 2 Servo Motors
- Buzzer
- Green, Yellow, and Red LEDs
- 10K Potentiometer
- Resistors
- Breadboard and Jumper Wires

---

## System Capacity

Total Parking Spaces: 10

The parking count is automatically updated whenever a vehicle enters or exits.

---

## Pin Configuration

| Device | PIC Pin |
|----------|----------|
| Entry IR Sensor | RB0 |
| Exit IR Sensor | RB1 |
| Green LED | RB2 |
| Yellow LED | RB3 |
| Red LED | RB4 |
| Buzzer | RC2 |
| LCD SCL | RC3 |
| LCD SDA | RC4 |
| Entry Servo | RD0 |
| Exit Servo | RD2 |

---

## System Operation

### Vehicle Entry

1. Vehicle detected by Entry IR Sensor.
2. System checks available parking spaces.
3. If space is available:
   - Parking count decreases.
   - LCD updates.
   - Buzzer beeps.
   - Entrance gate opens.
   - Gate closes automatically after a short delay.
4. If parking is full:
   - Entry is denied.
   - Warning buzzer is activated.
   - Gate remains closed.

### Vehicle Exit

1. Vehicle detected by Exit IR Sensor.
2. Parking count increases.
3. LCD updates.
4. Buzzer confirmation sound is activated.
5. Exit gate opens.
6. Gate closes automatically after a short delay.

---

## Software

- MPLAB X IDE
- XC8 Compiler
- Proteus Simulation

---

## Academic Information

**Course:** Microprocessor Systems

**Microcontroller:** PIC16F877A

**Programming Language:** Embedded C

**Compiler:** XC8

**Simulation Software:** Proteus

---

## Group Members

| Name | Student ID |
|--------|------------|
| Khaled Abdalfattah | 62220019 |
| Khaled Aman | 62220081 |
| Waleed Al-Ogaidi | 62210072 |

---

## Future Improvements

- RFID Access Control
- Mobile Application Integration
- Cloud Monitoring
- Individual Parking Slot Detection
- Online Reservation System

---

## Project Images

Circuit diagrams, simulation screenshots, and hardware photos will be added in future updates.

---

## License

Academic Project – Developed for educational purposes.
