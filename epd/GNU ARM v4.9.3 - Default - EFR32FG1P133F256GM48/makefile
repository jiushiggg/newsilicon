################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include sys_init/subdir.mk
-include response-printf/subdir.mk
-include reed/subdir.mk
-include protocol/subdir.mk
-include osd2_0/subdir.mk
-include osd/subdir.mk
-include led/subdir.mk
-include lcd-graphics/subdir.mk
-include info_flash/subdir.mk
-include hal-efr32/subdir.mk
-include global_variable/subdir.mk
-include file_sys/subdir.mk
-include ext_flash/subdir.mk
-include epd_screen/subdir.mk
-include driver/subdir.mk
-include delay/subdir.mk
-include crc16/subdir.mk
-include command-interpreter/subdir.mk
-include adc10/subdir.mk
-include CMSIS/efr32fg1p/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: epd.axf

# Tool invocations
epd.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m4 -mthumb -T "epd.ld" -L"D:/SiliconLabs/SimplicityStudio/v4/developer/stacks/rail/v1.3.1.0//autogen/librail_release/" -Xlinker --gc-sections -Xlinker -Map="epd.map" -mfpu=fpv4-sp-d16 -mfloat-abi=softfp --specs=nano.specs -u _printf_float -o epd.axf "./CMSIS/efr32fg1p/startup_efr32fg1p.o" "./CMSIS/efr32fg1p/system_efr32fg1p.o" "./adc10/adc10.o" "./command-interpreter/command_interpreter.o" "./crc16/crc16.o" "./debug_ci.o" "./main.o" "./rail-callback-stubs.o" "./rail-callbacks.o" "./rail_config.o" "./delay/delay.o" "./driver/mx25flash_spi.o" "./epd_screen/epd_io.o" "./epd_screen/hlt154.o" "./epd_screen/hlt213.o" "./epd_screen/hlt213_l.o" "./epd_screen/hlt213_red.o" "./epd_screen/hlt420.o" "./epd_screen/hlt_29.o" "./epd_screen/hlt_29_red.o" "./epd_screen/ltep_weifen213.o" "./epd_screen/tep_weifei_29.o" "./epd_screen/tep_weifei_29_red.o" "./epd_screen/tep_weifei_420_bw.o" "./epd_screen/tep_weifei_420_red.o" "./epd_screen/tep_weifen_154.o" "./epd_screen/tep_weifen_154_red.o" "./epd_screen/tep_weifen_213.o" "./epd_screen/tep_weifeng_213_red.o" "./epd_screen/weifei_jx_538.o" "./epd_screen/weifei_jx_75.o" "./epd_screen/weifei_jx_75_red.o" "./epd_screen/weifen42_red.o" "./epd_screen/weifen42_red_ic_double.o" "./epd_screen/weifeng_213_yellow.o" "./epd_screen/weifeng_jx_154_yellow.o" "./epd_screen/weifeng_jx_29_yellow.o" "./ext_flash/extern_flash.o" "./file_sys/mini_fs.o" "./global_variable/global_variable.o" "./hal-efr32/bsp_bcc.o" "./hal-efr32/bsp_stk.o" "./hal-efr32/bsp_stk_leds.o" "./hal-efr32/bsp_trace.o" "./hal-efr32/dmactrl.o" "./hal-efr32/em_assert.o" "./hal-efr32/em_cmu.o" "./hal-efr32/em_emu.o" "./hal-efr32/em_gpio.o" "./hal-efr32/em_int.o" "./hal-efr32/em_ldma.o" "./hal-efr32/em_leuart.o" "./hal-efr32/em_rmu.o" "./hal-efr32/em_rtc.o" "./hal-efr32/em_rtcc.o" "./hal-efr32/em_system.o" "./hal-efr32/em_timer.o" "./hal-efr32/em_usart.o" "./hal-efr32/gpiointerrupt.o" "./hal-efr32/hal_common.o" "./hal-efr32/hal_efr.o" "./hal-efr32/retargetio.o" "./hal-efr32/retargetserial.o" "./hal-efr32/rtcdriver.o" "./hal-efr32/udelay.o" "./hal-efr32/ustimer.o" "./info_flash/f_io_rom_flash.o" "./info_flash/save_sys_status.o" "./lcd-graphics/bmp.o" "./lcd-graphics/display.o" "./lcd-graphics/displayls013b7dh03.o" "./lcd-graphics/displaypalemlib.o" "./lcd-graphics/dmd_display.o" "./lcd-graphics/glib.o" "./lcd-graphics/glib_bitmap.o" "./lcd-graphics/glib_circle.o" "./lcd-graphics/glib_font_narrow_6x8.o" "./lcd-graphics/glib_font_normal_8x8.o" "./lcd-graphics/glib_font_number_16x20.o" "./lcd-graphics/glib_line.o" "./lcd-graphics/glib_polygon.o" "./lcd-graphics/glib_rectangle.o" "./lcd-graphics/glib_string.o" "./lcd-graphics/graphics.o" "./led/led.o" "./osd/bit_display.o" "./osd/osd_cmd.o" "./osd/osd_flash.o" "./osd2_0/osd2_0_cmd.o" "./osd2_0/split_dis_layer.o" "./protocol/three_event.o" "./protocol/three_protocol.o" "./reed/reed.o" "./response-printf/response_print.o" "./sys_init/sys_config.o" "./sys_init/sys_init.o" "./sys_init/sys_isr.o" -lrail_efr32_gcc_release -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: epd.hex'
	arm-none-eabi-objcopy -O ihex "epd.axf" "epd.hex"
	@echo ' '

	@echo 'Building bin file: epd.bin'
	arm-none-eabi-objcopy -O binary "epd.axf" "epd.bin"
	@echo ' '

	@echo 'Building s37 file: epd.s37'
	arm-none-eabi-objcopy -O srec "epd.axf" "epd.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "epd.axf"
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) epd.axf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
