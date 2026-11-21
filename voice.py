import speech_recognition as sr
import requests

ESP_IP = "192.168.1.7"


recognizer = sr.Recognizer()
while True:
    with sr.Microphone() as source:
        print("Speak...")
        recognizer.adjust_for_ambient_noise(source, duration=0.5)
        audio = recognizer.listen(source)

    try:
        voice_command = recognizer.recognize_google(audio)

        voice_command = voice_command.lower()

        print("You said:", voice_command)

    except sr.UnknownValueError:
        voice_command = ""
        print("Could not understand")

    except sr.RequestError:
        voice_command = ""
        print("Speech recognition service error")

    if "lights on" in voice_command:
        print("LIGHT_ON")
        requests.get(f"http://{ESP_IP}/lights/on")

    elif "lights off" in voice_command:
        print("LIGHT_OFF")
        requests.get(f"http://{ESP_IP}/lights/off")
    
    elif "high speed" in voice_command:
        print("FAST")
        requests.get(f"http://{ESP_IP}/high/speed")

    elif "medium speed" in voice_command:
        print("MEDIUM")
        requests.get(f"http://{ESP_IP}/medium/speed")

    elif "low speed" in voice_command:
        print("LOW")
        requests.get(f"http://{ESP_IP}/low/speed")

    elif "stop" in voice_command:
            print("stop")
            requests.get(f"http://{ESP_IP}/stop")

    elif "alarm" in voice_command:
                print("alarm")
                requests.get(f"http://{ESP_IP}/alarm")              



             