import pygame
import serial
import re
import time


standard_values = {
    "101": 100,
    "102": 1000, # 1nF
    "103": 10000, # 10 nF
    "104": 100000, # 0.1 uF
    "105": 1000000, # 1 uF
    "221": 220,
    "222": 2200,
    "223": 22000,
    "224": 220000,
    "225": 2200000,
}

correction_factors = {
    "102": -0.03,   # Measured 1.03 nF → Adjust to 1.0 nF
    "103": -0.1,    # Measured 10.1 nF → Adjust to 10.0 nF
    "104": 0,       # Measured correctly
    "105": 50,      # Measured 0.95 µF → Adjust to 1.0 µF
}

def get_capacitor_code(value):
    """Determine the capacitor code from a measured capacitance (uF)."""
    value_nF = value * 1e3  # Convert uF to nF

    # Apply correction factor if applicable
    for code, correction in correction_factors.items():
        corrected_value = standard_values[code] / 1000 + correction
        if abs(corrected_value - value_nF) <= 0.1 * corrected_value:  # 10% tolerance
            return code

    # If no direct match, find the closest
    closest_code = min(standard_values, key=lambda k: abs(standard_values[k] / 1000 - value_nF))
    return closest_code

#ser = serial.Serial(
#    port='COM3',  # Change this to your actual COM port
 #   baudrate=115200,
 #   parity=serial.PARITY_NONE,
  #  stopbits=serial.STOPBITS_ONE, 
   # bytesize=serial.EIGHTBITS
#)

def read_serial_data(serial_port):
    try:
        line = ser.readline().decode('utf-8').strip()
        match = re.search(r"Cap=([0-9.]+)uf", line)
        if match:
            return float(match.group(1))
    except Exception as e:
        print("Serial error:", e)
    return None

def main():
    pygame.init()
    screen = pygame.display.set_mode((400, 200))
    pygame.display.set_caption("Capacitance Reader")
    font = pygame.font.Font(None, 36)
    
    ser = serial.Serial('COM3', 115200, timeout=1)
    time.sleep(2)  # Give some time for the serial connection
    
    running = True
    while running:
        screen.fill((30, 30, 30))
        
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
        
        capacitance = read_serial_data(serial_port)
        if capacitance is not None:
            cap_code = get_capacitor_code(capacitance)
            text = font.render(f"Capacitance: {capacitance:.4f} uF", True, (255, 255, 255))
            code_text = font.render(f"Capacitor Code: {cap_code}", True, (255, 255, 0))
            screen.blit(text, (50, 60))
            screen.blit(code_text, (50, 100))
        
        pygame.display.flip()
        time.sleep(0.5)  # Reduce CPU usage
    
    ser.close()
    pygame.quit()

if __name__ == "__main__":
    main()
