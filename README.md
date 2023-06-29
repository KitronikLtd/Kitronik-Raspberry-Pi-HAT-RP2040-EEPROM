# RP2040 based EEPROM for Raspberry Pi HATs
Guide for how to use the RP2040 as an EEPROM for a Raspberry Pi HAT.

Use an RP2040 as the EEPROM on a Raspberry Pi HAT.

Shows how to create the EEPROM emulator program which runs on the RP2040.

At boot the RP2040 acts as the EEPROM storing information about the HAT. The RP2040 transfers this information to the Raspberry Pi, to tell it what functionality the HAT has. Once booted, the RP2040 can then act as a processor for the HAT and provide other functionality in main().

## 1. Clone GitHub repos
- Clone the following GitHub repos
```
git clone https://github.com/KitronikLtd/rp2040-based-eeprom.git
git clone https://github.com/raspberrypi/pico-sdk.git
git clone https://github.com/raspberrypi/hats.git
```

## 2. Update PICO_SDK_PATH in CMakeLists.txt
- Update line 5 in CMakeLists.txt to be the path of the pico-sdk
```
include(/home/user/pico-sdk/pico_sdk_init.cmake)
```

## 3. Create HAT EEPROM
- Follow the [HAT EEPROM guide](https://github.com/raspberrypi/hats/tree/master/eepromutils) to create the EEPROM settings and device tree

## 4. Add EEPROM bytes to main.c
- Create the EEPROM binary from inside the hats/eepromutils folder
```
dtc -I dts -O dtb -o eeprom_device_tree.dtbo eeprom_device_tree.dts
./eepmake eeprom_settings.txt eeprom.eep eeprom_device_tree.dtbo
```
- Convert the EEPROM binary to text form
```
xxd -i eeprom.eep eeprom.txt
```
- Copy the EEPROM binary text from hats/eepromutils/eeprom.txt to the eeprom array in rp2040-based-eeprom/main.c
- Update the eeprom_length variable in rp2040-based-eeprom/main.c
```
const unsigned char eeprom[] = {
};
const uint16_t eeprom_length = 0;
```

## 5. Build the HAT firmware
- Create the rp2040-based-eeprom/build folder to store the HAT firmware
```
mkdir build
```

- Build the HAT firmware from inside the rp2040-based-eeprom/build folder
```
cd build
cmake .. 
make
```

## 6. Flash the HAT firmware onto the RP2040
- You can now flash the HAT firmware onto the RP2040 using the following files in the rp2040-based-eeprom/build folder
```
rp2040-based-eeprom.bin
rp2040-based-eeprom.dis
rp2040-based-eeprom.elf
rp2040-based-eeprom.elf.map
rp2040-based-eeprom.hex
rp2040-based-eeprom.uf2
```
