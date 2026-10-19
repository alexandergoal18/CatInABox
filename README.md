# Cat in a Box

An Arduino-powered **cat-in-a-box** project: flip a switch and a hidden paw pops out of a box, then retreats and closes the lid. The cat gets more mischievous with repeated interactions, using ultrasonic sensors to detect someone nearby and tease them with a sneak peek.

## Features

- **Two servo motors** control the lid and the cat's paw.
- **Switch activation** triggers the open → paw extend → retract → close sequence.
- **Random delays** on the first three activations make the cat appear reluctant.
- **Two ultrasonic sensors** detect objects within 20 cm and trigger a partial lid opening and paw peek after the initial activations.
- **Angry lid slams** appear periodically as the interaction count grows.
- **Smooth servo movement** and adjustable angles let you tune the mechanism.

## Hardware

- Arduino-compatible board with the listed pins available
- 2 servo motors (lid and paw)
- 2 ultrasonic distance sensors with trigger/echo connections (e.g., HC-SR04)
- 1 switch or push button
- Suitable servo power supply and connecting wires

## Pin Connections

| Component | Arduino pin |
| --- | --- |
| Lid servo signal | 10 |
| Paw servo signal | 11 |
| Ultrasonic sensor 1 TRIG / ECHO | 6 / 7 |
| Ultrasonic sensor 2 TRIG / ECHO | 8 / 9 |
| Switch | 16 |

The switch uses `INPUT_PULLUP`, so connect it between pin **16** and **GND** (active LOW). Connect all component grounds together. Verify that your board supports pin 16, and provide enough power for both servos.

## Getting Started

1. Connect the hardware according to the table above.
2. Open `cat.ino` in the Arduino IDE.
3. Install or enable the Arduino **Servo** library (included with many Arduino IDE board packages).
4. Select your board and port, then upload the sketch.
5. Activate the switch to watch the cat respond. Optionally, open Serial Monitor at **115200 baud** to see the startup message.

## How It Behaves

1. **First 3 activations:** the cat waits a random 300–1500 ms before reacting.
2. **From activation 3 onward:** the ultrasonic sensors can cause the lid to crack open and the paw to peek out while someone is within 20 cm.
3. **Later interactions:** the sketch periodically enters an “angry” mode that slams the lid before the normal paw action; the number of slams increases over time.
4. **After 7 activations:** the peek angle changes so the paw extends farther during anticipation.

## Customize

Edit the settings near the top of `cat.ino`:

| Setting | Default | Purpose |
| --- | --- | --- |
| `DIST_THRESHOLD_CM` | `20` | Presence detection distance |
| `NUM_FAKE_OFF_PRESSES` | `3` | Number of initial delayed activations |
| `FAKE_DELAY_MIN_MS` / `FAKE_DELAY_MAX_MS` | `300` / `1500` | Random delay range |
| `LID_CLOSED`, `LID_PREOPEN`, `LID_OPEN` | `33`, `45`, `55` | Lid positions in degrees |
| `PAW_HIDDEN`, `PAW_PEEK`, `PAW_HIT` | `33`, `70`, `112` | Paw positions in degrees |
| `SERVO_STEP_DELAY_MS` | `1` | Delay between servo movement steps |

**Note:** Adjust servo positions carefully for your specific mechanism to avoid forcing a servo against a physical stop. The sketch currently uses the physical switch for triggering; the serial-key trigger function exists but is not enabled in `loop()`.

## Project File

- [`cat.ino`](cat.ino) — complete Arduino sketch.
