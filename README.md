# Hand-Gesture-Controlled-Robotic-Car-With-Voice-Commands-Features
ESP8266 robot car controlled by hand gestures and voice commands using Python, MediaPipe, and OpenCV, with ultrasonic obstacle detection, adjustable speed, lights, and a buzzer alarm.
The ESP8266 controls the motors, lights, and buzzer through a PCA9685 module. Front and rear ultrasonic sensors provide obstacle detection during forward and reverse movement.

## Features

- Live hand gesture recognition using MediaPipe and OpenCV.
- Forward, reverse, left, right, and braking commands.
- Voice control for headlights, three speed levels, stopping, and an alarm.
- Front and rear obstacle stopping at a distance of **10 cm or less**.
- Automatic resumption when the obstacle clears and the corresponding movement gesture remains stored.
- Rear lights activated by the braking command.
- A two-second voice-triggered buzzer alarm.
- A mirrored camera preview with hand landmarks and recognition confidence.

## How It Works

1. The gesture script processes webcam frames locally using the MediaPipe Gesture Recognizer model.
2. When the recognized gesture changes, the script sends its label to the ESP8266.
3. The voice script uses Google's speech recognition service to convert microphone input into text and sends matching commands to the ESP8266.
4. The ESP8266 runs an HTTP server on port `80`, applies commands, and checks the ultrasonic sensors approximately every `100 ms`.
5. The PCA9685 provides the control signals for the motor driver, lights, and buzzer.

The laptop and ESP8266 must be on the same reachable local network. Internet access is required on the laptop for Google speech recognition; gesture inference runs locally once dependencies and the model are installed.

## Hardware

| Component | Purpose |
| --- | --- |
| ESP8266 NodeMCU | Wi-Fi communication and embedded control |
| PCA9685 PWM module | Motor driver and accessory control signals |
| Dual H-bridge motor driver | Drives the two DC motors; the firmware uses two direction inputs and one enable input per motor |
| Two DC motors and robot chassis | Movement and steering |
| Two ultrasonic distance sensors | Front and rear obstacle detection |
| Front LEDs | Voice-controlled headlights |
| Rear LEDs | Brake indication |
| Buzzer | Voice-triggered alarm |
| Suitable power supply | Powers the motors and electronics |
| Laptop with webcam and microphone | Gesture and voice recognition |

### ESP8266 Connections

Pin labels below use the NodeMCU `D` naming convention from the sketch.

| Signal | NodeMCU pin | GPIO |
| --- | --- | --- |
| PCA9685 SCL | D1 | GPIO5 |
| PCA9685 SDA | D2 | GPIO4 |
| Front ultrasonic TRIG | D5 | GPIO14 |
| Front ultrasonic ECHO | D6 | GPIO12 |
| Rear ultrasonic TRIG | D7 | GPIO13 |
| Rear ultrasonic ECHO | D0 | GPIO16 |

### PCA9685 Channel Assignment

The firmware uses I²C address `0x40` and a PWM frequency of `1000 Hz`.

| PCA9685 channel | Firmware name | Connection |
| --- | --- | --- |
| 0 | `inB1` | Motor B direction input 1 |
| 1 | `inB2` | Motor B direction input 2 |
| 2 | `enB` | Motor B enable / speed input |
| 3 | `inA1` | Motor A direction input 1 |
| 4 | `inA2` | Motor A direction input 2 |
| 5 | `enA` | Motor A enable / speed input |
| 6 | `FRONT_LEDS` | Front light control |
| 8 | `REAR_LEDS` | Rear light control |
| 9 | `buzzerPin` | Buzzer control |

Use a common ground between the controller, PCA9685, motor driver, and sensors. Drive motors through the motor driver, not directly from PCA9685 outputs. Use appropriate resistors and driver circuitry for the LEDs and buzzer. If using 5 V HC-SR04 sensors, reduce each ECHO signal to an ESP8266-compatible 3.3 V level.

## Repository Organization

The following layout is suggested for publishing the supplied code. The firmware and voice script were supplied together in a text file and should be saved as separate files using the names below. The gesture folder already contains `demo.py`, the model, requirements, and its upstream license.

| Path | Contents |
| --- | --- |
| `README.md` | Project documentation |
| `firmware/robot_car/robot_car.ino` | ESP8266 Arduino sketch |
| `voice_control.py` | Python voice recognition script |
| `mediapipe_gesture_recognition/demo.py` | Gesture recognition and HTTP commands |
| `mediapipe_gesture_recognition/gesture_recognizer.task` | Gesture recognition model included in the supplied archive |
| `mediapipe_gesture_recognition/requirements.txt` | Original gesture dependencies |
| `mediapipe_gesture_recognition/LICENSE.txt` | Upstream gesture demo license |

## Setup

### 1. Configure and Upload the Firmware

1. Open `firmware/robot_car/robot_car.ino` in Arduino IDE.
2. Install ESP8266 board support and select the appropriate NodeMCU board and serial port.
3. Install **Adafruit PWM Servo Driver Library** and its dependencies using Library Manager. `Wire`, `ESP8266WiFi`, and `ESP8266WebServer` are provided by the board packages.
4. Replace the sketch's Wi-Fi settings with your own credentials:

   ```cpp
   const char* ssid = "YOUR_WIFI_NAME";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```

5. Upload the sketch and open Serial Monitor at **115200 baud**.
6. Wait for `Connected!`, note the printed ESP IP address, and confirm `Server started` appears.

Use a 2.4 GHz Wi-Fi network supported by the ESP8266. Replace personal Wi-Fi credentials with placeholders before publishing the sketch.

### 2. Prepare Python

Use **Python 3.10**, matching the supplied gesture project. Run these commands from the repository root.

Create a virtual environment:

```bash
python -m venv .venv
```

Activate it on Windows PowerShell:

```powershell
.\.venv\Scripts\Activate.ps1
```

Or on Windows Command Prompt:

```bat
.venv\Scripts\activate.bat
```

Or on Linux / macOS:

```bash
source .venv/bin/activate
```

Install the dependencies:

```bash
python -m pip install --upgrade pip
python -m pip install -r mediapipe_gesture_recognition/requirements.txt
python -m pip install opencv-contrib-python==4.11.0.86 SpeechRecognition requests PyAudio
```

The supplied gesture requirements pin `mediapipe==0.10.21` and `numpy==1.26.4`. The OpenCV version above matches the environment included in the archive. PyAudio provides microphone access and may require platform-specific PortAudio dependencies if installation fails.

Keep `gesture_recognizer.task` beside `demo.py`; the script locates it relative to its own directory.

### 3. Set the ESP8266 IP Address

Update `ESP_IP` in **both** `voice_control.py` and `mediapipe_gesture_recognition/demo.py` with the IP printed in Serial Monitor:

```python
ESP_IP = "YOUR_ESP_IP"
```

Verify communication by opening the following address in a browser, replacing `YOUR_ESP_IP`:

```text
http://YOUR_ESP_IP/test
```

Expected response:

```text
ESP8266 received the test signal
```

## Running the Project

Power the car and wait for the ESP8266 to connect. Open two terminals at the repository root and activate the Python environment in each.

Start gesture recognition in the first terminal:

```bash
python mediapipe_gesture_recognition/demo.py
```

Start voice recognition in the second terminal:

```bash
python voice_control.py
```

Show one hand clearly to the webcam and speak commands into the microphone. Press **q** in the camera window to close gesture recognition. Press **Ctrl+C** in the voice terminal to stop voice recognition.

**Stop the car before closing either program.** Closing the camera or voice process does not send a stop request.

## Controls

### Hand Gestures

| Gesture | Label sent to ESP8266 | Action |
| --- | --- | --- |
| Open palm | `Open_Palm` | Move forward if the front sensor is clear |
| Index finger pointing up | `Pointing_Up` | Reverse if the rear sensor is clear |
| Thumbs up | `Thumb_Up` | Turn right |
| Thumbs down | `Thumb_Down` | Turn left |
| Closed fist | `Closed_Fist` | Brake and activate rear lights |
| Recognizer category `None` | `None` | Disable motor outputs |

Losing sight of the hand is different from recognizing the category `None`: when no hand is detected, the Python script sends no command. Other gesture labels have no movement handler in the firmware.

### Voice Commands

| Spoken phrase | Action |
| --- | --- |
| “lights on” | Turn on front lights |
| “lights off” | Turn off front lights |
| “low speed” | Set PWM value to `1400` |
| “medium speed” | Set PWM value to `2600` |
| “high speed” | Set PWM value to `4095` |
| “stop” | Brake, activate rear lights, and clear the stored movement gesture |
| “alarm” | Activate the buzzer for two seconds |

High speed is the startup setting. These values are PWM commands, not measured vehicle speeds. A speed request updates the stored value; the motor PWM is applied the next time a movement function runs. After a voice stop, change the recognized gesture to issue a new movement command, because the gesture script only transmits changes.

## HTTP Interface

All routes below are used with HTTP GET requests.

| Endpoint | Purpose |
| --- | --- |
| `/test` | Check connectivity |
| `/gesture?value=Open_Palm` | Send a gesture label |
| `/lights/on` | Turn front lights on |
| `/lights/off` | Turn front lights off |
| `/low/speed` | Select low speed |
| `/medium/speed` | Select medium speed |
| `/high/speed` | Select high speed |
| `/stop` | Brake and clear the stored gesture |
| `/alarm` | Trigger the two-second alarm |

## Obstacle Detection and Current Limitations

- The front sensor blocks forward movement and the rear sensor blocks reverse movement at **10 cm or less**. Turning commands do not check obstacle distance.
- When an obstacle clears, movement resumes automatically if the stored gesture still requests forward or reverse movement.
- Obstacle stopping disables motor outputs; it does not call the braking function or activate the rear brake lights.
- A missing ultrasonic echo is represented as `9999` and treated as clear space in the current firmware.
- There is no command timeout. Loss of hand tracking, camera exit, or a disconnected laptop does not automatically stop the motors.
- Voice stop is a normal software command, not a latched emergency stop; a subsequent movement gesture can move the car again.
- The HTTP server has no authentication. Use it on a trusted local network.
- The voice script does not catch HTTP connection errors or set request timeouts. Check connectivity and restart the script if a request fails.

For initial testing, raise the wheels off the ground and verify motor directions, braking, sensors, and controls before driving.


## Acknowledgments

The gesture recognition component adapts the [MediaPipe Gesture Recognizer demo by jrmusan](https://github.com/jrmusan/mediapipe_gesture_recognition). The included upstream demo is distributed under the Apache License 2.0; retain its `LICENSE.txt` and applicable notices when publishing adapted files.

Computer vision uses MediaPipe and OpenCV. Voice recognition uses the Python SpeechRecognition library with its Google recognizer, and the firmware uses the Adafruit PWM Servo Driver library.
