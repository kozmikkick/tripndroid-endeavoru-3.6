# common clock types
obj-$(CONFIG_HAVE_CLK)		+= clk-devres.o
obj-$(CONFIG_CLKDEV_LOOKUP)	+= clkdev.o
obj-$(CONFIG_COMMON_CLK)	+= clk.o clk-fixed-rate.o clk-gate.o \
				   clk-mux.o clk-divider.o clk-fixed-factor.o
# SoCs specific
obj-$(CONFIG_ARCH_NOMADIK)	+= clk-nomadik.o
obj-$(CONFIG_ARCH_HIGHBANK)	+= clk-highbank.o
obj-$(CONFIG_ARCH_MXS)		+= mxs/
obj-$(CONFIG_ARCH_SOCFPGA)	+= socfpga/
obj-$(CONFIG_PLAT_SPEAR)	+= spear/
obj-$(CONFIG_ARCH_U300)		+= clk-u300.o
obj-$(CONFIG_COMMON_CLK_VERSATILE) += versatile/

# Chip specific
obj-$(CONFIG_COMMON_CLK_WM831X) += clk-wm831x.o
