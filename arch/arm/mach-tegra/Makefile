GCOV_PROFILE := y

obj-y                                   += ahb.o
obj-y                                   += apbio.o
obj-y                                   += common.o
#obj-y                                   += board-info.o
obj-$(CONFIG_ARCH_TEGRA_3x_SOC)         += common-t3.o
obj-y                                   += io.o
obj-y                                   += irq.o
obj-y                                   += clock.o
obj-y                                   += clock-common.o
obj-y                                   += timer.o

plus_sec := $(call as-instr,.arch_extension sec,+sec)
AFLAGS_sleep.o :=-Wa,-march=armv7-a$(plus_sec)

obj-y					+= tegra3_clocks.o
obj-y                                   += timer-t3.o
obj-y                                   += pinmux.o
obj-y                                   += devices.o
obj-y                                   += delay.o
obj-y                                   += powergate.o
obj-y                                   += pm.o
obj-$(CONFIG_TEGRA_WDT_RECOVERY)        += wdt-recovery.o
obj-$(CONFIG_PM_SLEEP)                  += pm-irq.o
obj-y                                   += gic.o
obj-y                                   += sleep.o
obj-$(CONFIG_ARCH_TEGRA_3x_SOC)         += sleep-t3.o
obj-y                                   += fuse.o
obj-y                                   += kfuse.o
obj-y                                   += csi.o
obj-$(CONFIG_TEGRA_SILICON_PLATFORM)    += tegra_odm_fuses.o
obj-y                                   += i2c_error_recovery.o
obj-y                                   += mc.o
obj-$(CONFIG_USB_SUPPORT)               += usb_phy.o
obj-$(CONFIG_FIQ)                       += fiq.o
obj-$(CONFIG_TEGRA_FIQ_DEBUGGER)        += tegra_fiq_debugger.o
obj-$(CONFIG_TEGRA_PWM)                 += pwm.o
obj-$(CONFIG_TEGRA_ARB_SEMAPHORE)       += arb_sema.o

ifeq ($(CONFIG_TEGRA_SILICON_PLATFORM),y)
obj-y                                   += dvfs.o
obj-$(CONFIG_ARCH_TEGRA_3x_SOC)         += tegra3_dvfs.o
obj-$(CONFIG_ARCH_TEGRA_3x_SOC)         += latency_allowance.o
obj-$(CONFIG_TEGRA_EDP_LIMITS)          += edp.o
obj-$(CONFIG_ARCH_TEGRA_3x_SOC)         += tegra3_speedo.o
obj-$(CONFIG_ARCH_TEGRA_3x_SOC)         += tegra3_actmon.o
endif

obj-$(CONFIG_ARCH_TEGRA_3x_SOC)         += tegra3_emc.o
obj-$(CONFIG_ARCH_TEGRA_3x_SOC)         += wakeups-t3.o
obj-y                                   += wakeups.o
obj-$(CONFIG_ARCH_TEGRA_3x_SOC)         += pm-t3.o

obj-$(CONFIG_ARCH_TEGRA_3x_SOC)         += pinmux-t3-tables.o
# obj-$(CONFIG_ARCH_TEGRA_3x_SOC)         += board-dt-tegra30.o
obj-$(CONFIG_LOCAL_TIMERS)              += localtimer.o
obj-$(CONFIG_SMP)                       += platsmp.o
obj-$(CONFIG_HOTPLUG_CPU)               += hotplug.o
obj-y					+= flowctrl.o
obj-y                                   += headsmp.o
obj-y                                   += reset.o
obj-y                                   += power.o
obj-$(CONFIG_TEGRA_SYSTEM_DMA)          += dma.o
obj-$(CONFIG_CPU_FREQ)                  += cpu-tegra.o

ifeq ($(CONFIG_TEGRA_AUTO_HOTPLUG),y)
obj-$(CONFIG_ARCH_TEGRA_3x_SOC)         += cpu-tegra3.o
endif

obj-$(CONFIG_USB_SUPPORT)               += usb_phy.o

ifeq ($(CONFIG_CPU_IDLE),y)
obj-y                                   += cpuidle.o
ifeq ($(CONFIG_PM_SLEEP),y)
obj-$(CONFIG_ARCH_TEGRA_3x_SOC)         += cpuidle-t3.o
endif
endif

ifeq ($(CONFIG_TEGRA_THERMAL_THROTTLE),y)
obj-$(CONFIG_ARCH_TEGRA_3x_SOC)         += tegra3_throttle.o
endif

obj-$(CONFIG_ARCH_TEGRA_3x_SOC)         += tegra3_thermal.o
obj-$(CONFIG_TEGRA_IOVMM)               += iovmm.o
obj-$(CONFIG_TEGRA_IOVMM_SMMU)          += iovmm-smmu.o
obj-$(CONFIG_DEBUG_ICEDCC)              += sysfs-dcc.o
obj-$(CONFIG_TEGRA_CLUSTER_CONTROL)     += sysfs-cluster.o
obj-$(CONFIG_SENSORS_TEGRA_TSENSOR)     += tegra3_tsensor.o
obj-$(CONFIG_TEGRA_DYNAMIC_PWRDET)      += powerdetect.o
obj-$(CONFIG_TEGRA_USB_MODEM_POWER)     += tegra_usb_modem_power.o
obj-$(CONFIG_TEGRA_PCI)                 += pcie.o

obj-$(CONFIG_MACH_HARMONY)              += board-harmony.o
obj-$(CONFIG_MACH_HARMONY)              += board-harmony-pinmux.o
obj-$(CONFIG_MACH_HARMONY)              += board-harmony-pcie.o
obj-$(CONFIG_MACH_HARMONY)              += board-harmony-power.o

obj-$(CONFIG_MACH_PAZ00)		+= board-paz00.o
obj-$(CONFIG_MACH_PAZ00)		+= board-paz00-pinmux.o

obj-$(CONFIG_MACH_SEABOARD)             += board-seaboard.o
obj-$(CONFIG_MACH_SEABOARD)             += board-seaboard-pinmux.o

obj-$(CONFIG_MACH_TEGRA_DT)             += board-dt-tegra20.o
obj-$(CONFIG_MACH_TEGRA_DT)             += board-harmony-pinmux.o
obj-$(CONFIG_MACH_TEGRA_DT)             += board-seaboard-pinmux.o
obj-$(CONFIG_MACH_TEGRA_DT)             += board-paz00-pinmux.o
obj-$(CONFIG_MACH_TEGRA_DT)             += board-trimslice-pinmux.o

obj-$(CONFIG_MACH_TRIMSLICE)            += board-trimslice.o
obj-$(CONFIG_MACH_TRIMSLICE)            += board-trimslice-pinmux.o

obj-${CONFIG_MACH_ENDEAVORU}		+= board-endeavoru.o
obj-${CONFIG_MACH_ENDEAVORU}		+= board-endeavoru-audio.o
obj-${CONFIG_MACH_ENDEAVORU}		+= board-endeavoru-panel.o
obj-${CONFIG_MACH_ENDEAVORU}		+= board-endeavoru-pinmux.o
obj-${CONFIG_MACH_ENDEAVORU}		+= board-endeavoru-sdhci.o
obj-${CONFIG_MACH_ENDEAVORU}		+= board-endeavoru-memory.o
obj-${CONFIG_MACH_ENDEAVORU}		+= board-endeavoru-power.o
obj-${CONFIG_MACH_ENDEAVORU}		+= board-endeavoru-baseband.o
#obj-${CONFIG_MACH_ENDEAVORU}		+= board-endeavoru-kbc.o
obj-${CONFIG_MACH_ENDEAVORU}		+= board-endeavoru-sensors.o
obj-${CONFIG_MACH_ENDEAVORU}		+= tegra_flashlight.o
obj-${CONFIG_MACH_ENDEAVORU}		+= devices_htc.o
obj-${CONFIG_MACH_ENDEAVORU}		+= htc-gpio.o
obj-${CONFIG_MACH_ENDEAVORU}		+= tegra_partitions.o
obj-${CONFIG_MACH_ENDEAVORU}		+= htc_audio_power.o
obj-${CONFIG_MACH_ENDEAVORU}		+= htc_simhotswap.o
obj-${CONFIG_MACH_ENDEAVORU}		+= nvdumper.o

obj-${CONFIG_HTC_BATT_CORE}             += htc_battery_core.o
obj-$(CONFIG_BATTERY_TPS80032)          += htc_battery_tps80032.o

obj-$(CONFIG_HTC_HEADSET_MGR)		+= htc_headset_mgr.o
obj-$(CONFIG_HTC_HEADSET_GPIO)		+= htc_headset_gpio.o
obj-$(CONFIG_HTC_HEADSET_PMIC)		+= htc_headset_pmic.o

obj-${CONFIG_TEGRA_BB_XMM_POWER}        += baseband-xmm-power.o
obj-${CONFIG_TEGRA_BB_XMM_POWER2}       += baseband-xmm-power2.o                                 
