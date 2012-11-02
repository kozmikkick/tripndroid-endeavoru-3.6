/*
 *
 * Copyright (c) 2012, TripNDroid Mobile Engineering.
 *
 * HTC Header containing various defines/functions used on Endeavoru
 * that where added by HTC and used tree-wide in the kernel. This
 * file needs to be removed and the original stuff needs to be
 * implemented. If possible removing of HTC stuff is the way to go.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __HTC_HEADER_H
#define __HTC_HEADER_H

#define BIT0                            0x00000001
#define BIT1                            0x00000002
#define BIT2                            0x00000004
#define BIT3                            0x00000008
#define BIT4                            0x00000010
#define BIT5                            0x00000020
#define BIT6                            0x00000040
#define BIT7                            0x00000080
#define BIT8                            0x00000100
#define BIT9                            0x00000200
#define BIT10                           0x00000400
#define BIT11                           0x00000800
#define BIT12                           0x00001000
#define BIT13                           0x00002000
#define BIT14                           0x00004000
#define BIT15                           0x00008000
#define BIT16                           0x00010000
#define BIT17                           0x00020000
#define BIT18                           0x00040000
#define BIT19                           0x00080000
#define BIT20                           0x00100000
#define BIT21                           0x00200000
#define BIT22                           0x00400000
#define BIT23                           0x00800000
#define BIT24                           0x01000000
#define BIT25                           0x02000000
#define BIT26                           0x04000000
#define BIT27                           0x08000000
#define BIT28                           0x10000000
#define BIT29                           0x20000000
#define BIT30                           0x40000000
#define BIT31                           0x80000000

#ifdef CONFIG_SPIN_LOCK_PRINT
#define PM_SP_FORMATION "[SP] "
#define sp_pr_emerg(fmt, ...)    printk( KERN_EMERG   pr_fmt( PM_SP_FORMATION fmt), ##__VA_ARGS__)
#define sp_pr_alert(fmt, ...)    printk( KERN_ALERT   pr_fmt( PM_SP_FORMATION fmt), ##__VA_ARGS__)
#define sp_pr_crit(fmt, ...)     printk( KERN_CRIT    pr_fmt( PM_SP_FORMATION fmt), ##__VA_ARGS__)
#define sp_pr_err(fmt, ...)      printk( KERN_ERR     pr_fmt( PM_SP_FORMATION fmt), ##__VA_ARGS__)
#define sp_pr_warning(fmt, ...)  printk( KERN_WARNING pr_fmt( PM_SP_FORMATION fmt), ##__VA_ARGS__)
#define sp_pr_warn               pm_pr_warning
#define sp_pr_notice(fmt, ...)   printk( KERN_NOTICE  pr_fmt( PM_SP_FORMATION fmt), ##__VA_ARGS__)
#define sp_pr_info(fmt, ...)     printk( KERN_INFO    pr_fmt( PM_SP_FORMATION fmt), ##__VA_ARGS__)
#define sp_pr_debug(fmt, ...)     printk( KERN_INFO    pr_fmt( PM_SP_FORMATION fmt), ##__VA_ARGS__)
#else
#define sp_pr_emerg(fmt, ...) do {;} while(0)
#define sp_pr_alert(fmt, ...) do {;} while(0)
#define sp_pr_crit(fmt, ...) do {;} while(0)
#define sp_pr_err(fmt, ...) do {;} while(0) 
#define sp_pr_warning(fmt, ...) do {;} while(0)
#define sp_pr_warn               pm_pr_warning
#define sp_pr_notice(fmt, ...) do {;} while(0)
#define sp_pr_info(fmt, ...) do {;} while(0)
#define sp_pr_debug(fmt, ...) do {;} while(0)
#endif


#ifdef CONFIG_PM_R_DEBUG

#define __PM_R_DEBUG_2_INFO
#define PM_R_FORMATION "[R] "

#define pm_pr_emerg(fmt, ...)    printk( KERN_EMERG   pr_fmt( PM_R_FORMATION fmt), ##__VA_ARGS__)
#define pm_pr_alert(fmt, ...)    printk( KERN_ALERT   pr_fmt( PM_R_FORMATION fmt), ##__VA_ARGS__)
#define pm_pr_crit(fmt, ...)     printk( KERN_CRIT    pr_fmt( PM_R_FORMATION fmt), ##__VA_ARGS__)
#define pm_pr_err(fmt, ...)      printk( KERN_ERR     pr_fmt( PM_R_FORMATION fmt), ##__VA_ARGS__)
#define pm_pr_warning(fmt, ...)  printk( KERN_WARNING pr_fmt( PM_R_FORMATION fmt), ##__VA_ARGS__)
#define pm_pr_warn               pm_pr_warning
#define pm_pr_notice(fmt, ...)   printk( KERN_NOTICE  pr_fmt( PM_R_FORMATION fmt), ##__VA_ARGS__)
#define pm_pr_info(fmt, ...)     printk( KERN_INFO    pr_fmt( PM_R_FORMATION fmt), ##__VA_ARGS__)
#ifdef __PM_R_DEBUG_2_INFO
#define pm_pr_debug(fmt, ...)    printk( KERN_INFO    pr_fmt( PM_R_FORMATION fmt), ##__VA_ARGS__)
#else 
#define pm_pr_debug(fmt, ...)    printk( KERN_CONT    pr_fmt( PM_R_FORMATION fmt), ##__VA_ARGS__)
#endif

#define pmr_pr_info(fmt, ...)    printk(KERN_INFO     pr_fmt(                fmt), ##__VA_ARGS__)

#else

#define pm_pr_emerg(fmt, ...) do {;} while(0)
#define pm_pr_alert(fmt, ...) do {;} while(0)
#define pm_pr_crit(fmt, ...) do {;} while(0)
#define pm_pr_err(fmt, ...) do {;} while(0) 
#define pm_pr_warning(fmt, ...) do {;} while(0)
#define pm_pr_warn               pm_pr_warning
#define pm_pr_notice(fmt, ...) do {;} while(0)
#define pm_pr_info(fmt, ...) do {;} while(0)
#define pm_pr_debug(fmt, ...) do {;} while(0)
#define pm_pr_debug(fmt, ...) do {;} while(0)
#define pmr_pr_info(fmt, ...) do {;} while(0)

#endif

#define DBG_EHCI_URB BIT12

#define DBG_USBCHR_L4 BIT11
#define DBG_USBCHR_L3 BIT10
#define DBG_USBCHR_L2 BIT9
#define DBG_USBCHR_L1 BIT8

#define DBG_RAWIP_L4 BIT7
#define DBG_RAWIP_L3 BIT6
#define DBG_RAWIP_L2 BIT5
#define DBG_RAWIP_L1 BIT4

#define DBG_ACM_REFCNT BIT3
#define DBG_ACM1_RW BIT1
#define DBG_ACM0_RW BIT0

/*********************************************************************
  Various defines
***********************************************************************/
#define SII9234_I2C_RETRY_COUNT 2	/* How many retry's for mhl i2c */

#define ABS_MT_POSITION 	0x2a    /* Group a set of X and Y */
#define ABS_MT_AMPLITUDE        0x2b    /* Group a set of Z and W */
#define KEY_APP_SWITCH		249	/* key for list app*/

#endif // __HTC_HEADER_H
