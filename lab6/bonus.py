import pygame
import serial
import re
import time

def get_capacitor_code(value):
    standard_values = {
        "101": 100,
        "102": 1000,
        "103": 10000,
        "104": 100000,
        "105": 1000000,
        "221": 220,
        "222": 2200,
        "223": 22000,
        "224": 220000,
        "225": 2200000,
    }
    closest_code = min(standard_values, key=lambda k: abs(standard_values[k] - (value * 1e6)))
    return closest_code

ser = serial.Serial(
    port='COM3',  # Change this to your actual COM port
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE, 
    bytesize=serial.EIGHTBITS
)

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
    
    serial_port = serial.Serial('COM3', 9600, timeout=1)
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
    
    serial_port.close()
    pygame.quit()

if __name__ == "__main__":
    main()
