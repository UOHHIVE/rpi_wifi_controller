import RPi.GPIO as GPIO
import time
GPIO.setmode(GPIO.BCM)
GPIO.setup(17, GPIO.OUT)

while True:
    GPIO.output(17,GPIO.HIGH)
    time.sleep(4)
    GPIO.output(17,GPIO.LOW)
    time.sleep(1)

# import gpiod
# import time

# PIN_NO = 17
# chip = gpiod.Chip('gpiochip4')
# led_line = chip.get_line(PIN_NO)
# led_line.request(consumer="myLed", type=gpiod.LINE_REQ_DIR_OUT)

# while True:
#     led_line.set_value(1)
#     time.sleep(4)
#     led_line.set_value(0)
#     time.sleep(1)
        