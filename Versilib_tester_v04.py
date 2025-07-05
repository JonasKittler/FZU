# VerSiLib tester v04
# Raspberry Pi Pico - MicroPython verze testovaciho generatoru pulzu
# velmi jednoduchy test BEZ pouziti PIO
# generovani pulzu pro krokove motory (X, Y) a SPDMA signal
# rozsah je dan pevne menit lze jen sire pulzu jinak je vse v kodu
# kontrola blika 3 ledkami jako by se hybal Y, X + SPDMA
# 1.7.2025 MK

from machine import Pin
import utime
import random

# ===========================
# Inicializace
# ===========================

# Piny pro vystup
pin_x = Pin(2, Pin.OUT)
pin_y = Pin(3, Pin.OUT)
pin_spdma = Pin(0, Pin.OUT)
pulz_h = 1000 # sirka pulzu v us
pulz_l = 2000# sirka mezi pulzy v us

# nech to běžet
while True:
    print("********** Zacatek **********")

    for y in range(10): # 
        print(" y = %d" % y)
        pin_y.value(1)
        utime.sleep_us(pulz_h)
        pin_y.value(0)
        utime.sleep_us(pulz_l)
        
    print(" pocatecni x")
    pin_x.value(1)
    utime.sleep_us(pulz_h)
    pin_x.value(0)
    utime.sleep_us(pulz_l)
    for x in range(20):
        # fixni cas pro prvni 2 pulzy spdma
        pin_spdma.value(1)
        utime.sleep_us(pulz_h)
        pin_spdma.value(0)
        utime.sleep_us(pulz_l)
        pin_spdma.value(1)
        utime.sleep_us(pulz_h)
        pin_spdma.value(0)
        utime.sleep_us(pulz_l)
        # stanoveni nove sirky pulzu spdma ktera se memni podle toho  o jaky se jedna krok X
        spdma_pulz_h = pulz_h * x 
        print(" spdma_pulz_h = %i" % spdma_pulz_h)
        for spdma in range(10):
            print(" spdma = %d" % spdma)
            pin_spdma.value(1)
            utime.sleep_us(spdma_pulz_h)
            pin_spdma.value(0)
            utime.sleep_us(pulz_l)
        print(" x = %d" % x)
        pin_x.value(1)
        utime.sleep_us(pulz_h)
        pin_x.value(0)
        utime.sleep_us(pulz_l)
    utime.sleep(10)        