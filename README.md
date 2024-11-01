# Saccharum-Medical-Insoles-Arduino-Codes
This project is a smart insole designed to help individuals with diabetes prevent foot ulcers and improve blood circulation. Initially designed as a diabetic sock, the final version uses a flexible insole with embedded sensors and a TENS (Transcutaneous Electrical Nerve Stimulation) system. Key features include:

Temperature Monitoring: Six NTC sensors placed at high-risk pressure points (heel and metatarsal areas) monitor temperature variations to detect early signs of ulcers. A Wemos D1 Mini microcontroller with Wi-Fi and Deep Sleep functionality manages the sensors for power efficiency, while a multiplexer connects them.

TENS Stimulation for Circulation: An independent TENS device controlled by an ESP32 microcontroller sends electrical pulses to stimulate blood flow, preventing vasculopathy. Signals are amplified using an LT1637HS op-amp, and a PWM board ensures precise signal control.

Smart Data Processing: The system leverages statistical analysis, using a 95% confidence interval and Z-scores based on a moving average of readings to identify outliers. A space-inspired cluster test compares sensor data, minimizing false alerts during physical activity.

Technologies Used: Arduino, C++, Analog Multiplexer, Op-Amp, ESP32, Wemos D1 Mini, PWM, NTC Sensors, Conductive Thread

This repository includes the full code, schematics, and documentation for replicating or extending the project.






