################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
C:/projects/builds/c55_caf/config_17/git/c55_caf/c55x5_drivers/pal_sys/build/Release/pal_sys.obj: C:/projects/builds/c55_caf/config_17/git/c55_caf/c55x5_drivers/pal_sys/da225/src/pal_sys.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: Compiler'
	"C:/Program Files/Texas Instruments/C5500 Code Generation Tools 4.3.9/bin/cl55" -vcpu:3.3 -g -O2 --define="BIOS_BUILD" --include_path="C:/Program Files/Texas Instruments/C5500 Code Generation Tools 4.3.9/include" --include_path="C:/projects/builds/c55_caf/config_17/git/c55_caf/c55x5_drivers/pal_sys/build/pal_sys_bios_lib/Release" --include_path="C:/Program Files/Texas Instruments/bios_5_41_10_36/packages/ti/bios/include" --include_path="C:/Program Files/Texas Instruments/bios_5_41_10_36/packages/ti/rtdx/include/c5500" --include_path="C:/Program Files/Texas Instruments/xdais_7_10_00_06/packages/ti/xdais" --include_path="C:/projects/builds/c55_caf/config_17/git/c55_caf/c55x5_drivers/pal_sys/da225/src" --include_path="C:/projects/builds/c55_caf/config_17/git/c55_caf/c55x5_drivers/pal_sys/inc" --include_path="C:/projects/builds/c55_caf/config_17/git/c55_caf/c55x5_drivers/inc" --include_path="C:/projects/builds/c55_caf/config_17/git/c55_caf/c55x5_drivers/soc/da225/dsp/inc" --include_path="C:/projects/builds/c55_caf/config_17/git/c55_caf/c55x5_drivers/pal_os/inc" --diag_warning=225 --large_memory_model --obj_directory="C:/projects/builds/c55_caf/config_17/git/c55_caf/c55x5_drivers/pal_sys/build/Release" --preproc_with_compile --preproc_dependency="pal_sys.pp" $(GEN_OPTS_QUOTED) $(subst #,$(wildcard $(subst $(SPACE),\$(SPACE),$<)),"#")
	@echo 'Finished building: $<'
	@echo ' '


