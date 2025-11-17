/*
 * \brief  IOMUX controller for i.MX8MP
 * \author Alice Domage
 * \date   2025-11-17
 */

/*
 * Copyright (C) 2025 Genode Labs GmbH
 * Copyright (C) 2025 gapfruit ag
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#pragma once

#include <os/attached_mmio.h>
#include <base/node.h>

namespace Driver {
	using namespace Genode;

	struct Iomuxc;
};


/* The following values are taken from device-tree sources of the
 * vendor from their Linux kernel forks, therefore they have this
 * specific form of arrays:
 */
enum Indices {
	MUX_REG, CONF_REG, INPUT_REG, MUX_MODE, INPUT_VAL, CONF_VAL, MAX };

static Genode::uint32_t imx93_phyboard_nash_pinctrl_setting [][MAX] { };


struct Driver::Iomuxc : Genode::Attached_mmio<0x10000>
{
	enum {
		PINCTRL_MMIO_BASE = 0x30330000,
		PINCTRL_MMIO_SIZE = 0x10000,
	};

	struct Mux : Mmio<32>
	{
		struct Reg : Register<0, 32>
		{
			struct Mode : Bitfield<0,3> {};
		};

		Mux(Byte_range_ptr const &range, Reg::access_t mux_mode)
		:
			Mmio<SIZE>(range)
		{
			write<Reg::Mode>(mux_mode);
		}
	};

	struct Misc : Mmio<32>
	{
		struct Reg : Register<0, 32> { };

		Misc(Byte_range_ptr const &range, Reg::access_t val)
		:
			Mmio<SIZE>(range)
		{
			write<Reg>(val);
		}
	};

	void _settings(auto pinctrl_setting, unsigned count)
	{
		for (unsigned i = 0; i < count; i++) {
			Mux mux(range_at(pinctrl_setting[i][MUX_REG]),
			        pinctrl_setting[i][MUX_MODE]);
			Misc conf(range_at(pinctrl_setting[i][CONF_REG]),
			        pinctrl_setting[i][CONF_VAL]);

			/* set input register only if it is set != 0 */
			if (pinctrl_setting[i][INPUT_REG])
				Misc input(range_at(pinctrl_setting[i][INPUT_REG]),
				           pinctrl_setting[i][INPUT_VAL]);
		}
	}

	Iomuxc(Env &env, Node const &info)
	:
		Attached_mmio<SIZE>(env, { (char*)PINCTRL_MMIO_BASE, SIZE })
	{
		using Board_name = String<64>;

		Board_name board;
		info.with_optional_sub_node("board", [&] (Node const &node) {
			board = node.attribute_value("name", Board_name()); });

		if (board == "imx93_phyboard_pollux") {
			_settings(imx93_phyboard_nash_pinctrl_setting,
			          sizeof(imx93_phyboard_nash_pinctrl_setting) / (MAX*sizeof(uint32_t)));
			return;
		}
	}
};
