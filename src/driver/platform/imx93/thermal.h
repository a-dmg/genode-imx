/*
 * \brief  Watchdog driver for i.MX9
 * \author Alice Domage <alice.domage@gapfruit.com>
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

#include <base/env.h>
#include <irq_session/connection.h>
#include <os/attached_mmio.h>
#include <base/log.h>

namespace Driver {
	using namespace Genode;
	struct Thermal;
}


struct Driver::Thermal: private Genode::Attached_mmio<0x1000>
{
	Env &_env;

//	enum { THERMAL_MMIO_BASE = 0x44482000, TEMPSENSOR_IRQ = 83, UTEMPSENSOR_IRQ = TEMPSENSOR_IRQ + 32 };

	Irq_connection              _irq { _env, 83 + 32 };
	Irq_connection              _irq_uart { _env, 19 + 32 };
	Io_signal_handler<Thermal> _irq_handler { _env.ep(), *this,
		&Thermal::_handle_irq };

	struct Tmr: Register<0x00, 32>
	{
		struct MODE: Bitfield<30, 2> { };
		struct ALPF: Bitfield<24, 2> { };
	};

	struct TIDR: Register<0x24, 32> {};

	struct TIISCR: Register<0x30, 32> {};
	struct TIASCR: Register<0x34, 32> {};
	struct TICSCR: Register<0x38, 32> {};

	struct TMHTITR: Register<0x50, 32> {

		struct EN: Bitfield<31, 1> { };
		struct TEMP: Bitfield<0, 8> { };
	};

	struct TIER: Register<0x20, 32> {};
	struct TCMCFG: Register<0xF00, 32> { };
	struct TMTMIT: Register<0xC, 32> { };

	struct TMSR: Register<0x50, 32> {

		struct SITE: Bitfield<0, 16> { };
	};

	void _service()
	{
	}

	void _handle_irq()
	{
		Genode::log("---------------------------INTERUPRT----------------------------");
		_irq.ack_irq();
	}


	Thermal(Env &env)
	: Attached_mmio<SIZE>(env, { (char*)0x44482000, SIZE }),
	  _env(env)
	{
		_irq.sigh(_irq_handler);
		_irq.ack_irq();
		_irq_uart.sigh(_irq_handler);
		_irq_uart.ack_irq();
		write<Tmr::MODE>(0x0);

		write<TIDR>(read<TIDR>() | 0xFF000000);

		write<TIISCR>(read<TIISCR>() & 0xFFFFFFF8);
		write<TIASCR>(read<TIASCR>() & 0xFFFFFFF8);
		write<TICSCR>(read<TICSCR>() & 0xFFFFFFF8);

		write<TIER>(read<TIER>() | 0xFF000000);

		write<TMHTITR::TEMP>(20);
		write<TMHTITR::EN>(1);

		write<TMTMIT>(0x7);

		write<Tmr::ALPF>(1);

		write<TMSR::SITE>(0x4);


		write<Tmr::MODE>(0x1);
		Genode::log("start thermal");

	}
};

