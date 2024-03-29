/*
 *  Shared Transport driver
 *	HCI-LL module responsible for TI proprietary HCI_LL protocol
 *  Copyright (C) 2009-2010 Texas Instruments
 *  Author: Pavan Savoy <pavan_savoy@ti.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define pr_fmt(fmt) "(stll) :" fmt
#include <linux/skbuff.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/ti_wilink_st.h>

#ifdef PWR_DEVICE_TAG
#undef PWR_DEVICE_TAG
#endif
#define PWR_DEVICE_TAG "WIFI"

/**********************************************************************/
/* internal functions */
static void send_ll_cmd(struct st_data_s *st_data,
	unsigned char cmd)
{

	pr_debug("%s: writing %x", __func__, cmd);
	st_int_write(st_data, &cmd, 1);
	return;
}

static void ll_device_want_to_sleep(struct st_data_s *st_data)
{

	struct kim_data_s	*kim_data = st_data->kim_data;
	struct platform_device *pdev = kim_data->kim_pdev;
	struct ti_st_plat_data	*pdata = pdev->dev.platform_data;

	pr_debug("%s", __func__);
	/* sanity check */
	if (st_data->ll_state != ST_LL_AWAKE)
		pr_err("ERR hcill: ST_LL_GO_TO_SLEEP_IND"
			  "in state %ld", st_data->ll_state);

/* communicate to platform about chip asleep */
	if (pdata->chip_awake)
		pdata->chip_asleep();

	send_ll_cmd(st_data, LL_SLEEP_ACK);
	/* update state */
	st_data->ll_state = ST_LL_ASLEEP;
}

static void ll_device_want_to_wakeup(struct st_data_s *st_data)
{

	struct kim_data_s	*kim_data = st_data->kim_data;
	struct ti_st_plat_data	*pdata = kim_data->kim_pdev->dev.platform_data;


	/* diff actions in diff states */
	switch (st_data->ll_state) {
	case ST_LL_ASLEEP:
/* communicate to platform about chip wakeup */
		if (pdata->chip_awake)
			pdata->chip_awake();

		send_ll_cmd(st_data, LL_WAKE_UP_ACK);	/* send wake_ack */
		break;
	case ST_LL_ASLEEP_TO_AWAKE:
		/* duplicate wake_ind */
		pr_err("duplicate wake_ind while waiting for Wake ack");
		break;
	case ST_LL_AWAKE:
		/* duplicate wake_ind */
		pr_err("duplicate wake_ind already AWAKE");
		break;
	case ST_LL_AWAKE_TO_ASLEEP:
		/* duplicate wake_ind */
		pr_err("duplicate wake_ind");
		break;
	}
	/* update state */
	st_data->ll_state = ST_LL_AWAKE;
}

/**********************************************************************/
/* functions invoked by ST Core */

/* called when ST Core wants to
 * enable ST LL */
void st_ll_enable(struct st_data_s *ll)
{

	struct kim_data_s      *kim_data = ll->kim_data;
	struct ti_st_plat_data *pdata = kim_data->kim_pdev->dev.platform_data;

	/* communicate to platform about chip enable */
	if (pdata->chip_enable)
		pdata->chip_enable();

	ll->ll_state = ST_LL_AWAKE;
}

/* called when ST Core /local module wants to
 * disable ST LL */
void st_ll_disable(struct st_data_s *ll)
{

	struct kim_data_s      *kim_data = ll->kim_data;
	struct ti_st_plat_data *pdata = kim_data->kim_pdev->dev.platform_data;
	/* communicate to platform about chip disable */
	if (pdata->chip_disable)
		pdata->chip_disable();

	ll->ll_state = ST_LL_INVALID;
}

/* called when ST Core wants to update the state */
void st_ll_wakeup(struct st_data_s *ll)
{
	struct kim_data_s       *kim_data = ll->kim_data;
	struct ti_st_plat_data  *pdata = kim_data->kim_pdev->dev.platform_data;

	if (likely(ll->ll_state != ST_LL_AWAKE)) {

		/* communicate to platform about chip wakeup */
		if (pdata->chip_awake)
			pdata->chip_awake();

		send_ll_cmd(ll, LL_WAKE_UP_IND);	/* WAKE_IND */
		ll->ll_state = ST_LL_ASLEEP_TO_AWAKE;
	} else {
		/* don't send the duplicate wake_indication */
		pr_err(" Chip already AWAKE ");
	}
}

/* called when ST Core wants the state */
unsigned long st_ll_getstate(struct st_data_s *ll)
{
	pr_debug(" returning state %ld", ll->ll_state);
	return ll->ll_state;
}

/* called from ST Core, when a PM related packet arrives */
unsigned long st_ll_sleep_state(struct st_data_s *st_data,
	unsigned char cmd)
{
	switch (cmd) {
	case LL_SLEEP_IND:	/* sleep ind */
		pr_debug("sleep indication recvd");
		ll_device_want_to_sleep(st_data);
		pr_mode_exit("Idle mode");
		pr_mode_enter("Sleep mode", 80, "uA");
		break;
	case LL_SLEEP_ACK:	/* sleep ack */
		pr_err("sleep ack rcvd: host shouldn't");
		break;
	case LL_WAKE_UP_IND:	/* wake ind */
		pr_debug("wake indication recvd");
		ll_device_want_to_wakeup(st_data);
		pr_mode_exit("Sleep mode");
		pr_mode_enter("Idle mode", 2850, "uA");
		break;
	case LL_WAKE_UP_ACK:	/* wake ack */
		pr_debug("wake ack rcvd");
		st_data->ll_state = ST_LL_AWAKE;
		break;
	default:
		pr_err(" unknown input/state ");
		return -EINVAL;
	}
	return 0;
}

/* Called from ST CORE to initialize ST LL */
long st_ll_init(struct st_data_s *ll)
{
	/* set state to invalid */
	ll->ll_state = ST_LL_INVALID;
	return 0;
}

/* Called from ST CORE to de-initialize ST LL */
long st_ll_deinit(struct st_data_s *ll)
{
	return 0;
}
