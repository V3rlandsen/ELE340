#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Forenklet STM32 PID-logger
Kun kommandoer: k, s, p<verdi>, i<verdi>, d<verdi>
Ingen hex-parsing – kun tekstbasert kontroll!
Perfekt til debugging og tuning av PID live!
"""

import serial
import threading
import queue
import time

# =============================================
# Konfigurasjon
# =============================================
PORT = 'COM5'          # Endre til din port
BAUD = 115200
TIMEOUT = 1

# =============================================
# Globale variabler
# =============================================
ser = None
log_thread = None
stop_event = threading.Event()
data_queue = queue.Queue()

# =============================================
# Tråd som leser alt fra STM32 og printer det live
# =============================================
def serial_reader():
    print("Logger startet – trykk Ctrl+C for å avslutte")
    while not stop_event.is_set():
        try:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    print(line)
                    data_queue.put(line)  # Hvis du vil lagre senere
        except:
            break
    print("Logger avsluttet")

# =============================================
# Send kommando til STM32
# =============================================
def send_command(cmd):
    ser.write((cmd + '\n').encode('utf-8'))
    print(f">>> Sendt: {cmd}")

# =============================================
# Hovedprogram
# =============================================
def main():
    global ser, log_thread

    print("STM32 PID Live Logger")
    print("Kommandoer:")
    print("  k       → start logging")
    print("  s       → stopp logging")
    print("  p1500   → Kp = 1.500")
    print("  i80     → Ki = 0.080")
    print("  d5      → Kd = 0.005")
    print("  a250    → settpunkt = 250")
    print()

    try:
        ser = serial.Serial(PORT, BAUD, timeout=TIMEOUT)
        print(f"Tilkoblet {ser.name}")
        time.sleep(2)  # Vent på STM32 reset

        # Start lesetråd
        log_thread = threading.Thread(target=serial_reader, daemon=True)
        log_thread.start()

        print("\nKlar! Skriv kommando og trykk Enter:\n")

        while True:
            cmd = input("").strip()

            if not cmd:
                continue

            # Start / stopp logging
            if cmd == 'k':
                send_command('k')
            elif cmd == 's':
                send_command('s')
            elif cmd == 'q':  # Avslutt programmet
                break

            # PID-kommandoer: pXXXX, iXXXX, dXXXX
            elif cmd[0] in 'pida' and len(cmd) > 1 and cmd[1:].isdigit():
                letter = cmd[0]
                value_str = cmd[1:]

                # Send som "p1234" → STM32 tolker det som Kp = 1.234
                send_command(cmd)
                print(cmd)

            else:
                print("Ugyldig kommando! Bruk: k, s, p1000, i50, d10")

    except serial.SerialException:
        print(f"Klarte ikke koble til {PORT} – er kortet koblet til?")
    except KeyboardInterrupt:
        print("\nAvslutter...")
    finally:
        stop_event.set()
        if ser and ser.is_open:
            ser.close()
        print("Ferdig!")

if __name__ == "__main__":
    main()