/*
 * \brief  Reset_controller driver for imx8
 * \author Alice Domage <alice.domage@gapfruit.com>
 * \date   2023-06-07
 */

/*
 * Copyright (C) 2013-2023 Genode Labs GmbH
 * Copyright (C) 2023 gapfruit AG
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#pragma once

/* Genode includes */
#include <base/env.h>
#include <timer_session/connection.h>
#include <platform_session/connection.h>
#include <platform_session/device.h>


namespace Watchdog {

	using namespace Genode;

	class  Controller;
	struct Mmio;
}


struct Watchdog::Mmio : Platform::Device::Mmio
{
	using Control_register = Mmio::Register<0x00, 16>;
	struct Control: Control_register {
		struct WDZST : Control_register ::Bitfield<0, 1> { };
		struct WDBG  : Control_register ::Bitfield<1, 1> { };
		struct WDE   : Control_register ::Bitfield<2, 1> { };
		struct WDT   : Control_register ::Bitfield<3, 1> { };
		struct SRS   : Control_register ::Bitfield<4, 1> { };
		struct WDA   : Control_register ::Bitfield<5, 1> { };
		struct SRE   : Control_register ::Bitfield<6, 1> { };
		struct WDW   : Control_register ::Bitfield<7, 1> { };
		struct WT    : Control_register ::Bitfield<8, 8> {
			enum {
				TIMEOUT_500_MS     = 0x00,
				TIMEOUT_1_S        = 0x01,
				TIMEOUT_1_S_500_MS = 0x02,
				TIMEOUT_2_S        = 0x03,
				TIMEOUT_128_S      = 0xFF,
			};
		};
	};

	using Service_register = Mmio::Register<0x02, 16>;
	struct Service: Service_register {
		enum {
			WSR1 = 0x5555,
			WSR2 = 0xAAAA,
		};
	};

	using Reset_status_register = Mmio::Register<0x04, 16>;
	struct Reset_statue: Reset_status_register {
		struct SFTW : Reset_status_register ::Bitfield<0, 1> { };
		struct TOUT : Reset_status_register ::Bitfield<1, 1> { };
		struct POR  : Reset_status_register ::Bitfield<4, 1> { };
	};

	using Interrupt_control_register = Mmio::Register<0x02, 16>;
	struct Interrupt_control: Interrupt_control_register {
		struct WICT : Interrupt_control_register ::Bitfield<0, 8> {
			enum {
				TRIGGERED_PRIOR_0_S          = 0x0,
				TRIGGERED_PRIOR_500_MS       = 0x1,
				TRIGGERED_PRIOR_2_S          = 0x4,
				TRIGGERED_PRIOR_127_S_500_MS = 0xFF,
			};
		};
		struct WTIS : Interrupt_control_register ::Bitfield<14, 1> { };
		struct WIE  : Interrupt_control_register ::Bitfield<15, 1> { };
	};

	explicit Mmio(Platform::Device &device) : Platform::Device::Mmio { device } { }

};


class Watchdog::Controller
{
	private:

		Platform::Connection          _platform;
		Platform::Device              _device    { _platform };
		Platform::Device::Irq         _irq       { _device };
		Watchdog::Mmio                _mmio      { _device };
		Io_signal_handler<Controller> _irq_sigh;

		Timer::Connection                   _timer;
		Timer::Periodic_timeout<Controller> _periodic_timeout;

		void _irq_handler()
		{
			_mmio.write<Mmio::Service>(Mmio::Service::WSR1);
			_mmio.write<Mmio::Service>(Mmio::Service::WSR2);
			_mmio.write<Mmio::Interrupt_control::WTIS>(1);
			_irq.ack();
			log("watchdog IRQ triggered");
		}

	public:

		Controller(Env &env)
		:
			_platform         { env },
			_irq_sigh         { env.ep(), *this, &Controller::_irq_handler },
			_timer            { env },
			_periodic_timeout { _timer, *this, &Controller::timeout, Genode::Microseconds(1500) }
		{
			_irq.sigh(_irq_sigh);
			/* configure WDOG timeout period */
			_mmio.write<Mmio::Control::WT>(Mmio::Control::WT::TIMEOUT_2_S);
			log("watchdog enabled with a timeout period of 2s");
			/* Set the Software Reset Extention bit. */
			_mmio.write<Mmio::Control::SRE>(1);
			_mmio.write<Mmio::Control::WDT>(1);
			_mmio.write<Mmio::Interrupt_control::WIE>(1);
			_mmio.write<Mmio::Interrupt_control::WICT>(Mmio::Interrupt_control::WICT::TRIGGERED_PRIOR_500_MS);
			_mmio.write<Mmio::Control::WDE>(1);
			_irq_handler();
		}

		void timeout(Duration)
		{
			_mmio.write<Mmio::Service>(Mmio::Service::WSR1);
			_mmio.write<Mmio::Service>(Mmio::Service::WSR2);
		}

		void issue_por()
		{
			/*
			 * Assert the Software Reset Signal witch generate a reset signal
			 * on the WDOG_RESET_B_DEB signal witch triggers the System Reset Controller
			 * (SRC) to properly reset the SoC.
			 */
			_mmio.write<Mmio::Control::WDA>(0);
		}
};

