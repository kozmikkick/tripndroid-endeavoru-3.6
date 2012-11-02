/*
 * arch/arm/mach-tegra/gpio.c
 *
 * Copyright (c) 2010 Google, Inc
 *
 * Author:
 *	Erik Gilling <konkers@google.com>
 *
 * Copyright (c) 2011 NVIDIA Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/module.h>

#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/syscore_ops.h>

#include <asm/mach/irq.h>

#include <mach/iomap.h>
#include <mach/legacy_irq.h>
#include "../../arch/arm/mach-tegra/pm-irq.h"
#include <mach/pinmux.h>

#include "../../arch/arm/mach-tegra/gpio-names.h"
#include "../../arch/arm/mach-tegra/htc-gpio.h"
#include <mach/board_htc.h>

#define GPIO_BANK(x)		((x) >> 5)
#define GPIO_PORT(x)		(((x) >> 3) & 0x3)
#define GPIO_BIT(x)		((x) & 0x7)

#define GPIO_CNF(x)		(GPIO_REG(x) + 0x00)
#define GPIO_OE(x)		(GPIO_REG(x) + 0x10)
#define GPIO_OUT(x)		(GPIO_REG(x) + 0X20)
#define GPIO_IN(x)		(GPIO_REG(x) + 0x30)
#define GPIO_INT_STA(x)		(GPIO_REG(x) + 0x40)
#define GPIO_INT_ENB(x)		(GPIO_REG(x) + 0x50)
#define GPIO_INT_LVL(x)		(GPIO_REG(x) + 0x60)
#define GPIO_INT_CLR(x)		(GPIO_REG(x) + 0x70)

#if defined(CONFIG_ARCH_TEGRA_2x_SOC)
#define GPIO_REG(x)		(IO_TO_VIRT(TEGRA_GPIO_BASE) +	\
				 GPIO_BANK(x) * 0x80 +		\
				 GPIO_PORT(x) * 4)

#define GPIO_MSK_CNF(x)		(GPIO_REG(x) + 0x800)
#define GPIO_MSK_OE(x)		(GPIO_REG(x) + 0x810)
#define GPIO_MSK_OUT(x)		(GPIO_REG(x) + 0X820)
#define GPIO_MSK_INT_STA(x)	(GPIO_REG(x) + 0x840)
#define GPIO_MSK_INT_ENB(x)	(GPIO_REG(x) + 0x850)
#define GPIO_MSK_INT_LVL(x)	(GPIO_REG(x) + 0x860)
#else

#define GPIO_REG(x)		(GPIO_BANK(x) * 0x100 + GPIO_PORT(x) * 4)

#define GPIO_MSK_CNF(x)		(GPIO_REG(x) + 0x80)
#define GPIO_MSK_OE(x)		(GPIO_REG(x) + 0x90)
#define GPIO_MSK_OUT(x)		(GPIO_REG(x) + 0XA0)
#define GPIO_MSK_INT_STA(x)	(GPIO_REG(x) + 0xC0)
#define GPIO_MSK_INT_ENB(x)	(GPIO_REG(x) + 0xD0)
#define GPIO_MSK_INT_LVL(x)	(GPIO_REG(x) + 0xE0)
#endif

#define GPIO_INT_LVL_MASK		0x010101
#define GPIO_INT_LVL_EDGE_RISING	0x000101
#define GPIO_INT_LVL_EDGE_FALLING	0x000100
#define GPIO_INT_LVL_EDGE_BOTH		0x010100
#define GPIO_INT_LVL_LEVEL_HIGH		0x000001
#define GPIO_INT_LVL_LEVEL_LOW		0x000000
#define GPIO_DUMP_ENABLE_BIT 0x01

extern const char* enr_td_suspend_gpio_config_xc[TEGRA_NR_GPIOS];

extern const char* quo_suspend_gpio_config_xa[TEGRA_NR_GPIOS];
extern const char* quo_suspend_gpio_config_xb[TEGRA_NR_GPIOS];
extern const char* quo_suspend_gpio_config_xc[TEGRA_NR_GPIOS];
extern const char* quo_suspend_gpio_config_xd[TEGRA_NR_GPIOS];

static int gpio_dump_enable = 0;

struct tegra_gpio_bank {
	int bank;
	int irq;
	spinlock_t lvl_lock[4];
#ifdef CONFIG_PM_SLEEP
	u32 cnf[4];
	u32 out[4];
	u32 oe[4];
	u32 int_enb[4];
	u32 int_lvl[4];
	u32 wake_enb[4];
#endif
};

static void __iomem *regs;
static struct tegra_gpio_bank tegra_gpio_banks[8];

static inline void tegra_gpio_writel(u32 val, u32 reg)
{
	__raw_writel(val, regs + reg);
}

static inline u32 tegra_gpio_readl(u32 reg)
{
	return __raw_readl(regs + reg);
}

static int tegra_gpio_compose(int bank, int port, int bit)
{
	return (bank << 5) | ((port & 0x3) << 3) | (bit & 0x7);
}

void tegra_gpio_set_tristate(int gpio_nr, enum tegra_tristate ts)
{
	int pin_group  =  tegra_pinmux_get_pingroup(gpio_nr);
	tegra_pinmux_set_tristate(pin_group, ts);
}

static void tegra_gpio_mask_write(u32 reg, int gpio, int value)
{
	u32 val;

	val = 0x100 << GPIO_BIT(gpio);
	if (value)
		val |= 1 << GPIO_BIT(gpio);
	tegra_gpio_writel(val, reg);
}

int tegra_gpio_get_bank_int_nr(int gpio)
{
	int bank;
	int irq;
	if (gpio >= TEGRA_NR_GPIOS) {
		pr_warn("%s : Invalid gpio ID - %d\n", __func__, gpio);
		return -EINVAL;
	}
	bank = gpio >> 5;
	irq = tegra_gpio_banks[bank].irq;
	return irq;
}

void tegra_gpio_enable(int gpio)
{
	if (gpio >= TEGRA_NR_GPIOS) {
		pr_warn("%s : Invalid gpio ID - %d\n", __func__, gpio);
		return;
	}
	tegra_gpio_mask_write(GPIO_MSK_CNF(gpio), gpio, 1);
}
EXPORT_SYMBOL_GPL(tegra_gpio_enable);

void tegra_gpio_disable(int gpio)
{
	if (gpio >= TEGRA_NR_GPIOS) {
		pr_warn("%s : Invalid gpio ID - %d\n", __func__, gpio);
		return;
	}
	tegra_gpio_mask_write(GPIO_MSK_CNF(gpio), gpio, 0);
}
EXPORT_SYMBOL_GPL(tegra_gpio_disable);

void tegra_gpio_init_configure(unsigned gpio, bool is_input, int value)
{
	if (gpio >= TEGRA_NR_GPIOS) {
		pr_warn("%s : Invalid gpio ID - %d\n", __func__, gpio);
		return;
	}
	if (is_input) {
		tegra_gpio_mask_write(GPIO_MSK_OE(gpio), gpio, 0);
	} else {
		tegra_gpio_mask_write(GPIO_MSK_OUT(gpio), gpio, value);
		tegra_gpio_mask_write(GPIO_MSK_OE(gpio), gpio, 1);
	}
	tegra_gpio_mask_write(GPIO_MSK_CNF(gpio), gpio, 1);
}

static void tegra_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	tegra_gpio_mask_write(GPIO_MSK_OUT(offset), offset, value);
}

static int tegra_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	if ((tegra_gpio_readl(GPIO_OE(offset)) >> GPIO_BIT(offset)) & 0x1)
		return (tegra_gpio_readl(GPIO_OUT(offset)) >>
			GPIO_BIT(offset)) & 0x1;
	return (tegra_gpio_readl(GPIO_IN(offset)) >> GPIO_BIT(offset)) & 0x1;
}

static int tegra_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	tegra_gpio_mask_write(GPIO_MSK_OE(offset), offset, 0);
	return 0;
}

static int tegra_gpio_direction_output(struct gpio_chip *chip, unsigned offset,
					int value)
{
	tegra_gpio_set(chip, offset, value);
	tegra_gpio_mask_write(GPIO_MSK_OE(offset), offset, 1);
	return 0;
}

int tegra_gpio_to_int_pin(int gpio)
{
	if (gpio < TEGRA_NR_GPIOS)
		return tegra_gpio_banks[gpio >> 5].irq;

	return -EIO;
}

static int tegra_gpio_set_debounce(struct gpio_chip *chip, unsigned offset,
					unsigned debounce)
{
	return -ENOSYS;
}

static int tegra_gpio_to_irq(struct gpio_chip *chip, unsigned offset)
{
	return TEGRA_GPIO_TO_IRQ(offset);
}

static struct gpio_chip tegra_gpio_chip = {
	.label			= "tegra-gpio",
	.direction_input	= tegra_gpio_direction_input,
	.get			= tegra_gpio_get,
	.direction_output	= tegra_gpio_direction_output,
	.set			= tegra_gpio_set,
	.set_debounce		= tegra_gpio_set_debounce,
	.to_irq			= tegra_gpio_to_irq,
	.base			= 0,
	.ngpio			= TEGRA_NR_GPIOS,
};

static void tegra_gpio_irq_ack(struct irq_data *d)
{
	int gpio = d->irq - INT_GPIO_BASE;

	tegra_gpio_writel(1 << GPIO_BIT(gpio), GPIO_INT_CLR(gpio));
}

static void tegra_gpio_irq_mask(struct irq_data *d)
{
	int gpio = d->irq - INT_GPIO_BASE;

	tegra_gpio_mask_write(GPIO_MSK_INT_ENB(gpio), gpio, 0);
}

static void tegra_gpio_irq_unmask(struct irq_data *d)
{
	int gpio = d->irq - INT_GPIO_BASE;

	tegra_gpio_mask_write(GPIO_MSK_INT_ENB(gpio), gpio, 1);
}

static int tegra_gpio_irq_set_type(struct irq_data *d, unsigned int type)
{
	int gpio = d->irq - INT_GPIO_BASE;
	struct tegra_gpio_bank *bank = irq_data_get_irq_chip_data(d);
	int port = GPIO_PORT(gpio);
	int lvl_type;
	int val;
	unsigned long flags;

	switch (type & IRQ_TYPE_SENSE_MASK) {
	case IRQ_TYPE_EDGE_RISING:
		lvl_type = GPIO_INT_LVL_EDGE_RISING;
		break;

	case IRQ_TYPE_EDGE_FALLING:
		lvl_type = GPIO_INT_LVL_EDGE_FALLING;
		break;

	case IRQ_TYPE_EDGE_BOTH:
		lvl_type = GPIO_INT_LVL_EDGE_BOTH;
		break;

	case IRQ_TYPE_LEVEL_HIGH:
		lvl_type = GPIO_INT_LVL_LEVEL_HIGH;
		break;

	case IRQ_TYPE_LEVEL_LOW:
		lvl_type = GPIO_INT_LVL_LEVEL_LOW;
		break;

	default:
		return -EINVAL;
	}

	spin_lock_irqsave(&bank->lvl_lock[port], flags);

	val = tegra_gpio_readl(GPIO_INT_LVL(gpio));
	val &= ~(GPIO_INT_LVL_MASK << GPIO_BIT(gpio));
	val |= lvl_type << GPIO_BIT(gpio);
	tegra_gpio_writel(val, GPIO_INT_LVL(gpio));

	spin_unlock_irqrestore(&bank->lvl_lock[port], flags);

	if (type & (IRQ_TYPE_LEVEL_LOW | IRQ_TYPE_LEVEL_HIGH))
		__irq_set_handler_locked(d->irq, handle_level_irq);
	else if (type & (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING))
		__irq_set_handler_locked(d->irq, handle_edge_irq);

	tegra_pm_irq_set_wake_type(d->irq, type);

	return 0;
}

static void tegra_gpio_irq_handler(unsigned int irq, struct irq_desc *desc)
{
	struct tegra_gpio_bank *bank;
	int port;
	int pin;
	struct irq_chip *chip = irq_desc_get_chip(desc);

	chained_irq_enter(chip, desc);

	bank = irq_get_handler_data(irq);

	for (port = 0; port < 4; port++) {
		int gpio = tegra_gpio_compose(bank->bank, port, 0);
		unsigned long sta = tegra_gpio_readl(GPIO_INT_STA(gpio)) &
			tegra_gpio_readl(GPIO_INT_ENB(gpio));

		for_each_set_bit(pin, &sta, 8)
			generic_handle_irq(gpio_to_irq(gpio + pin));
	}

	chained_irq_exit(chip, desc);

}

#ifdef CONFIG_PM_SLEEP
static void tegra_gpio_resume(void)
{
	unsigned long flags;
	int b;
	int p;

	local_irq_save(flags);

	for (b = 0; b < ARRAY_SIZE(tegra_gpio_banks); b++) {
		struct tegra_gpio_bank *bank = &tegra_gpio_banks[b];

		for (p = 0; p < ARRAY_SIZE(bank->oe); p++) {
			unsigned int gpio = (b<<5) | (p<<3);
			tegra_gpio_writel(bank->cnf[p], GPIO_CNF(gpio));
			tegra_gpio_writel(bank->out[p], GPIO_OUT(gpio));
			tegra_gpio_writel(bank->oe[p], GPIO_OE(gpio));
			tegra_gpio_writel(bank->int_lvl[p], GPIO_INT_LVL(gpio));
			tegra_gpio_writel(bank->int_enb[p], GPIO_INT_ENB(gpio));
		}
	}
#if defined(CONFIG_MACH_ENDEAVORTD)
	int projectPhase = htc_get_pcbid_info();
	if (projectPhase == PROJECT_PHASE_XC){
		enr_xc_no_owner_gpio_resume();
	}
#endif
	local_irq_restore(flags);
}

static char GPIO_STATE[7][10] =
{
	"A",
	"O(H)",
	"O(L)",
	"I(NP)",
	"I(PD)",
	"I(PU)",
	"A(PU)"
};

static int gpio_config_state(int cnf, int oe, int out, int pupd)
{
	if ((cnf & 0x01) ==  1 && ( oe & 0x01) == 1) {
		if ((out & 0x01) == 1)
			return 1;
		else
			return 2;
	} else if ((cnf & 0x01) ==  1 && (oe & 0x01) == 0) {
		if (pupd == TEGRA_PUPD_NORMAL)
			return 3;
		else if (pupd == TEGRA_PUPD_PULL_DOWN)
			return 4;
		else if ( pupd == TEGRA_PUPD_PULL_UP)
			return 5;
	} else {
		if ( pupd == TEGRA_PUPD_PULL_UP)
			return 6;
	}
	return 0;
}

void gpio_dump(void)
{
	int b, p, i;
	struct tegra_gpio_bank *bank;
	const char** expected_gpio;
	unsigned int gpio;
	char port[2];
	int gpio_cnf;
	int gpio_out;
	int gpio_oe;
	enum tegra_pingroup ball;
	int reg;
	int tristate;
	int io_enable;
	char* reg_gpio_config;

	for (b = 0; b < ARRAY_SIZE(tegra_gpio_banks); b++) {
		bank = &tegra_gpio_banks[b];

		expected_gpio = enr_td_suspend_gpio_config_xc;

		for (p = 0; p < ARRAY_SIZE(bank->oe); p++) {
			gpio = (b<<5) | (p<<3);
			
			if((b*4+p) < 26)
				sprintf(port, "%c", (b*4+p)+'A');
			else
				sprintf(port, "%c%c", ((b*4+p)%26)+'A', ((b*4+p)%26)+'A');

			gpio_cnf = tegra_gpio_readl(GPIO_CNF(gpio));
			gpio_out = tegra_gpio_readl(GPIO_OUT(gpio));
			gpio_oe = tegra_gpio_readl(GPIO_OE(gpio));

			for (i = 0; i < 8; i++) {
				ball = gpio_to_pingroup[gpio+i];
				reg = tegra_pinmux_get_pullupdown(ball);
				tristate = tegra_pinmux_get_tristate(ball);
				io_enable = tegra_pinmux_get_io(ball);

				reg_gpio_config = GPIO_STATE[gpio_config_state(gpio_cnf, gpio_oe, gpio_out, reg %3)];
				printk("%s%d %s", port, i, reg_gpio_config);

				if(gpio+i < TEGRA_GPIO_INVALID){
					if (strcmp(reg_gpio_config, expected_gpio[gpio+i]) != 0){
						printk(" *%s %s", expected_gpio[gpio+i], tegra_soc_pingroups[ball].name);
					}
				}

				if(gpio_oe==1&&tristate==1)
					printk(" !tristate");
				if(reg_gpio_config[0]!='I' && reg == 1)
					printk(" pd");
				if(reg_gpio_config[0]!='I' && reg == 2)
					printk(" pu");

				if(reg_gpio_config[0]=='I' && io_enable==0)
					printk(" !inputDisable");

				printk("\n");
				gpio_cnf = gpio_cnf >> 1;
				gpio_out = gpio_out >> 1;
				gpio_oe = gpio_oe >> 1;
			}
		}

	}
}

static int tegra_gpio_suspend(void)
{
	unsigned long flags;
	int b;
	int p;
	unsigned int gpio;

	local_irq_save(flags);
	for (b = 0; b < ARRAY_SIZE(tegra_gpio_banks); b++) {
		struct tegra_gpio_bank *bank = &tegra_gpio_banks[b];

		for (p = 0; p < ARRAY_SIZE(bank->oe); p++) {
			gpio = (b<<5) | (p<<3);
			bank->cnf[p] = tegra_gpio_readl(GPIO_CNF(gpio));
			bank->out[p] = tegra_gpio_readl(GPIO_OUT(gpio));
			bank->oe[p] = tegra_gpio_readl(GPIO_OE(gpio));
			bank->int_enb[p] = tegra_gpio_readl(GPIO_INT_ENB(gpio));
			bank->int_lvl[p] = tegra_gpio_readl(GPIO_INT_LVL(gpio));
		}
	}


	local_irq_restore(flags);
   /*local_gpio_config_test(quo_suspend_gpio_config_xb);*/

	// TODO gpio_basic_during_suspend
	gpio_basic_during_suspend();

	if (gpio_dump_enable)
		gpio_dump();

	return 0;
}

static int tegra_update_lp1_gpio_wake(struct irq_data *d, bool enable)
{
#ifdef CONFIG_PM_SLEEP
	struct tegra_gpio_bank *bank = irq_data_get_irq_chip_data(d);
	u8 mask;
	u8 port_index;
	u8 pin_index_in_bank;
	u8 pin_in_port;
	int gpio = d->irq - INT_GPIO_BASE;

	if (gpio < 0)
		return -EIO;
	pin_index_in_bank = (gpio & 0x1F);
	port_index = pin_index_in_bank >> 3;
	pin_in_port = (pin_index_in_bank & 0x7);
	mask = BIT(pin_in_port);
	if (enable)
		bank->wake_enb[port_index] |= mask;
	else
		bank->wake_enb[port_index] &= ~mask;
#endif

	return 0;
}

static int tegra_gpio_irq_set_wake(struct irq_data *d, unsigned int enable)
{
	struct tegra_gpio_bank *bank = irq_data_get_irq_chip_data(d);
	int ret = 0;

	/*
	 * update LP1 mask for gpio port/pin interrupt
	 * LP1 enable independent of LP0 wake support
	 */
	ret = tegra_update_lp1_gpio_wake(d, enable);
	if (ret) {
		pr_err("Failed gpio lp1 %s for irq=%d, error=%d\n",
		(enable ? "enable" : "disable"), d->irq, ret);
		goto fail;
	}

	/* LP1 enable for bank interrupt */
	ret = tegra_update_lp1_irq_wake(bank->irq, enable);
	if (ret)
		pr_err("Failed gpio lp1 %s for irq=%d, error=%d\n",
		(enable ? "enable" : "disable"), bank->irq, ret);

	/* LP0 */
	ret = tegra_pm_irq_set_wake(d->irq, enable);
	if (ret)
		pr_err("Failed gpio lp0 %s for irq=%d, error=%d\n",
		(enable ? "enable" : "disable"), d->irq, ret);

fail:
	return ret;
}
#else
#define tegra_gpio_irq_set_wake NULL
#define tegra_gpio_suspend NULL
#define tegra_gpio_resume NULL
#endif

static struct syscore_ops tegra_gpio_syscore_ops = {
	.suspend = tegra_gpio_suspend,
	.resume = tegra_gpio_resume,
};

int tegra_gpio_resume_init(void)
{
	register_syscore_ops(&tegra_gpio_syscore_ops);

	return 0;
}

static struct irq_chip tegra_gpio_irq_chip = {
	.name		= "GPIO",
	.irq_ack	= tegra_gpio_irq_ack,
	.irq_mask	= tegra_gpio_irq_mask,
	.irq_unmask	= tegra_gpio_irq_unmask,
	.irq_set_type	= tegra_gpio_irq_set_type,
	.irq_set_wake	= tegra_gpio_irq_set_wake,
	.flags		= IRQCHIP_MASK_ON_SUSPEND,
};


/* This lock class tells lockdep that GPIO irqs are in a different
 * category than their parents, so it won't report false recursion.
 */
static struct lock_class_key gpio_lock_class;

static int __devinit tegra_gpio_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct tegra_gpio_bank *bank;
	int gpio;
	int i;
	int j;

	for (i = 0; i < ARRAY_SIZE(tegra_gpio_banks); i++) {
		res = platform_get_resource(pdev, IORESOURCE_IRQ, i);
		if (!res) {
			dev_err(&pdev->dev, "Missing IRQ resource\n");
			return -ENODEV;
		}

		bank = &tegra_gpio_banks[i];
		bank->bank = i;
		bank->irq = res->start;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "Missing MEM resource\n");
		return -ENODEV;
	}

	regs = devm_request_and_ioremap(&pdev->dev, res);
	if (!regs) {
		dev_err(&pdev->dev, "Couldn't ioremap regs\n");
		return -ENODEV;
	}

	for (i = 0; i < ARRAY_SIZE(tegra_gpio_banks); i++) {
		for (j = 0; j < 4; j++) {
			gpio = tegra_gpio_compose(i, j, 0);
			tegra_gpio_writel(0x00, GPIO_INT_ENB(gpio));
			tegra_gpio_writel(0x00, GPIO_INT_STA(gpio));
		}
	}

#ifdef CONFIG_OF_GPIO
	tegra_gpio_chip.of_node = pdev->dev.of_node;
#endif

	gpiochip_add(&tegra_gpio_chip);

	for (i = INT_GPIO_BASE; i < (INT_GPIO_BASE + TEGRA_NR_GPIOS); i++) {
		bank = &tegra_gpio_banks[GPIO_BANK(irq_to_gpio(i))];

		irq_set_lockdep_class(i, &gpio_lock_class);
		irq_set_chip_data(i, bank);
		irq_set_chip_and_handler(i, &tegra_gpio_irq_chip,
					 handle_simple_irq);
		set_irq_flags(i, IRQF_VALID);
	}

	for (gpio = 0; gpio < TEGRA_NR_GPIOS; gpio++) {
		int irq = TEGRA_GPIO_TO_IRQ(gpio);
		/* No validity check; all Tegra GPIOs are valid IRQs */

		bank = &tegra_gpio_banks[GPIO_BANK(gpio)];

		irq_set_lockdep_class(irq, &gpio_lock_class);
		irq_set_chip_data(irq, bank);
		irq_set_chip_and_handler(irq, &tegra_gpio_irq_chip,
					 handle_simple_irq);
		set_irq_flags(irq, IRQF_VALID);
	}

	for (i = 0; i < ARRAY_SIZE(tegra_gpio_banks); i++) {
		bank = &tegra_gpio_banks[i];

		for (j = 0; j < 4; j++)
			spin_lock_init(&bank->lvl_lock[j]);

		irq_set_handler_data(bank->irq, bank);
		irq_set_chained_handler(bank->irq, tegra_gpio_irq_handler);

	}

	if (get_extra_kernel_flag() & GPIO_DUMP_ENABLE_BIT)
		gpio_dump_enable = 1;

	return 0;
}

static struct of_device_id tegra_gpio_of_match[] __devinitdata = {
	{ .compatible = "nvidia,tegra20-gpio", },
	{ },
};

static struct platform_driver tegra_gpio_driver = {
	.driver		= {
		.name	= "tegra-gpio",
		.owner	= THIS_MODULE,
		.of_match_table = tegra_gpio_of_match,
	},
	.probe		= tegra_gpio_probe,
};

static int __init tegra_gpio_init(void)
{
	return platform_driver_register(&tegra_gpio_driver);
}
postcore_initcall(tegra_gpio_init);

void __init tegra_gpio_config(struct tegra_gpio_table *table, int num)
{
	int i;
	int gpio;

	for (i = 0; i < num; i++) {
		gpio = table[i].gpio;

		if (table[i].enable)
			tegra_gpio_enable(gpio);
		else
			tegra_gpio_disable(gpio);
	}
}

#ifdef	CONFIG_DEBUG_FS

#include <linux/debugfs.h>
#include <linux/seq_file.h>

static int dbg_gpio_show(struct seq_file *s, void *unused)
{
	int i;
	int j;
	int gpio;

	seq_printf(s, "Bank:Port CNF OE OUT IN INT_STA INT_ENB INT_LVL\n");
	for (i = 0; i < ARRAY_SIZE(tegra_gpio_banks); i++) {
		for (j = 0; j < 4; j++) {
			gpio = tegra_gpio_compose(i, j, 0);
			seq_printf(s,
				"%d:%d %02x %02x %02x %02x %02x %02x %06x\n",
				i, j,
				tegra_gpio_readl(GPIO_CNF(gpio)),
				tegra_gpio_readl(GPIO_OE(gpio)),
				tegra_gpio_readl(GPIO_OUT(gpio)),
				tegra_gpio_readl(GPIO_IN(gpio)),
				tegra_gpio_readl(GPIO_INT_STA(gpio)),
				tegra_gpio_readl(GPIO_INT_ENB(gpio)),
				tegra_gpio_readl(GPIO_INT_LVL(gpio)));
		}
	}
	return 0;
}

static int dbg_gpio_open(struct inode *inode, struct file *file)
{
	return single_open(file, dbg_gpio_show, &inode->i_private);
}

static const struct file_operations debug_fops = {
	.open		= dbg_gpio_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init tegra_gpio_debuginit(void)
{
	(void) debugfs_create_file("tegra_gpio", S_IRUGO,
					NULL, NULL, &debug_fops);
	return 0;
}
late_initcall(tegra_gpio_debuginit);
#endif
