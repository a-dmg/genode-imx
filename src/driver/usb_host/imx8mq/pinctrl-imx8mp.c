// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 NXP
 */

#include <linux/err.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/platform_device.h>

static const struct of_device_id imx8mp_pinctrl_of_match[] = {
	{ .compatible = "fsl,imx8mp-iomuxc", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, imx8mp_pinctrl_of_match);

static int imx8mp_pinctrl_probe(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver imx8mp_pinctrl_driver = {
	.driver = {
		.name = "imx8mp-pinctrl",
		.of_match_table = imx8mp_pinctrl_of_match,
		.suppress_bind_attrs = true,
	},
	.probe = imx8mp_pinctrl_probe,
};

static int __init imx8mp_pinctrl_init(void)
{
	return platform_driver_register(&imx8mp_pinctrl_driver);
}
arch_initcall(imx8mp_pinctrl_init);

MODULE_AUTHOR("Anson Huang <Anson.Huang@nxp.com>");
MODULE_DESCRIPTION("NXP i.MX8MP pinctrl driver");
MODULE_LICENSE("GPL v2");

