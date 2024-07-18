// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 NXP.
 */

#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/types.h>

static int imx8mp_clocks_probe(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id imx8mp_clk_of_match[] = {
	{ .compatible = "fsl,imx8mp-ccm" },
	{ /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, imx8mp_clk_of_match);

static struct platform_driver imx8mp_clk_driver = {
	.probe = imx8mp_clocks_probe,
	.driver = {
		.name = "imx8mp-ccm",
		/*
		 * Disable bind attributes: clocks are not removed and
		 * reloading the driver will crash or break devices.
		 */
		.suppress_bind_attrs = true,
		.of_match_table = imx8mp_clk_of_match,
	},
};

module_platform_driver(imx8mp_clk_driver);

MODULE_AUTHOR("Anson Huang <Anson.Huang@nxp.com>");
MODULE_DESCRIPTION("NXP i.MX8MP clock driver");
MODULE_LICENSE("GPL v2");
