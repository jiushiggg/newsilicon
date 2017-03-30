################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include response-printf/subdir.mk
-include lcd-graphics/subdir.mk
-include hal-efr32/subdir.mk
-include command-interpreter/subdir.mk
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
all: dutycycle_2.axf

# Tool invocations
dutycycle_2.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m4 -mthumb -T "dutycycle_2.ld" -L"D:/SiliconLabs/SimplicityStudio/v4/developer/stacks/rail/v1.3.1.0//autogen/librail_release/" -Xlinker --gc-sections -Xlinker -Map="dutycycle_2.map" -mfpu=fpv4-sp-d16 -mfloat-abi=softfp --specs=nano.specs -u _printf_float -o dutycycle_2.axf "./CMSIS/efr32fg1p/startup_efr32fg1p.o" "./CMSIS/efr32fg1p/system_efr32fg1p.o" "./command-interpreter/command_interpreter.o" "./debug_ci.o" "./main.o" "./rail-callback-stubs.o" "./rail-callbacks.o" "./rail_config.o" "./hal-efr32/bsp_bcc.o" "./hal-efr32/bsp_stk.o" "./hal-efr32/bsp_stk_leds.o" "./hal-efr32/bsp_trace.o" "./hal-efr32/dmactrl.o" "./hal-efr32/em_assert.o" "./hal-efr32/em_cmu.o" "./hal-efr32/em_emu.o" "./hal-efr32/em_gpio.o" "./hal-efr32/em_int.o" "./hal-efr32/em_ldma.o" "./hal-efr32/em_leuart.o" "./hal-efr32/em_rmu.o" "./hal-efr32/em_rtc.o" "./hal-efr32/em_rtcc.o" "./hal-efr32/em_system.o" "./hal-efr32/em_timer.o" "./hal-efr32/em_usart.o" "./hal-efr32/gpiointerrupt.o" "./hal-efr32/hal_common.o" "./hal-efr32/hal_efr.o" "./hal-efr32/retargetio.o" "./hal-efr32/retargetserial.o" "./hal-efr32/rtcdriver.o" "./hal-efr32/udelay.o" "./hal-efr32/ustimer.o" "./lcd-graphics/bmp.o" "./lcd-graphics/display.o" "./lcd-graphics/displayls013b7dh03.o" "./lcd-graphics/displaypalemlib.o" "./lcd-graphics/dmd_display.o" "./lcd-graphics/glib.o" "./lcd-graphics/glib_bitmap.o" "./lcd-graphics/glib_circle.o" "./lcd-graphics/glib_font_narrow_6x8.o" "./lcd-graphics/glib_font_normal_8x8.o" "./lcd-graphics/glib_font_number_16x20.o" "./lcd-graphics/glib_line.o" "./lcd-graphics/glib_polygon.o" "./lcd-graphics/glib_rectangle.o" "./lcd-graphics/glib_string.o" "./lcd-graphics/graphics.o" "./response-printf/response_print.o" -lrail_efr32_gcc_release -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: dutycycle_2.hex'
	arm-none-eabi-objcopy -O ihex "dutycycle_2.axf" "dutycycle_2.hex"
	@echo ' '

	@echo 'Building bin file: dutycycle_2.bin'
	arm-none-eabi-objcopy -O binary "dutycycle_2.axf" "dutycycle_2.bin"
	@echo ' '

	@echo 'Building s37 file: dutycycle_2.s37'
	arm-none-eabi-objcopy -O srec "dutycycle_2.axf" "dutycycle_2.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "dutycycle_2.axf"
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) dutycycle_2.axf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
