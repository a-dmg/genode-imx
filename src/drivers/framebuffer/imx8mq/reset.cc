/*
 * \brief  Linux kernel / Lx_emul reset pin control
 * \author Stefan Kalkowski
 * \date   2021-05-25
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <base/log.h>
#include <lx_kit/env.h>

struct device;
struct reset_control;

extern "C" struct reset_control *
__devm_reset_control_get(struct device * dev, const char * id, int index,
                         bool shared, bool optional, bool acquired);

extern "C" int reset_control_deassert(struct reset_control * rstc);


extern "C" struct reset_control * __devm_reset_control_get(struct device *,
                                                           const char * id,
                                                           int, bool, bool, bool)
{
	return (struct reset_control*) id;
}


extern "C" int reset_control_deassert(struct reset_control * rstc)
{
	Genode::String<64> type("reset-pin,", (const char*)rstc);
	Lx_kit::env().devices.for_each([&] (Lx_kit::Device & d) {
		if (type == d.compatible()) { d.enable(); } });
	return 0;
}
