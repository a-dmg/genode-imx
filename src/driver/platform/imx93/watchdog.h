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

/* Genode includes */
#include <base/env.h>
#include <irq_session/connection.h>
#include <os/attached_mmio.h>
#include <timer_session/connection.h>

/* Platform driver includes */
#include <device.h>
#include <device_owner.h>

namespace Driver {
	using namespace Genode;
	class Watchdog;
}


class Driver::Watchdog: public Driver::Device_owner
{
	private:

		using Io_mem  = Driver::Device::Io_mem;
		using Pci_bar = Driver::Device::Pci_bar;


		struct Ressources: Genode::Attached_mmio<0x10>
		{

			struct Control: Register<0x0, 32>
			{
				struct INT: Bitfield<6, 1> {};
				struct EN: Bitfield<7, 1> {};
				struct CMD32EN: Bitfield<13, 1> {};
			};

			struct Counter: Register<0x4, 32>
			{
				struct CNT8LOW:  Bitfield<0, 8>  { enum { UNLOCK = 0x0 }; };
				struct CNT8HIGH: Bitfield<8, 8>  { enum { UNLOCK = 0x0 }; };
				struct CNT16:    Bitfield<0, 16> { enum { UNLOCK = 0x0 }; };
				struct CNT32:    Bitfield<0, 32> { enum { UNLOCK = 0xD928C520, REFRESH = 0xB480A602 }; };
			};
			struct Timeout: Register<0x8, 32> {};
			struct Window: Register<0xC, 32> {};

			Genode::Env                         &_env;
			Irq_connection                       _irq;
			Io_signal_handler<Ressources>        _irq_handler;
			Timer::Connection                    _timer { _env };
			Timer::One_shot_timeout<Ressources>  _service_handler { _timer, *this, &Ressources::_handle_service};
			Genode::Duration                     _service_timeout;

			void _handle_service(Duration)
			{
				if (read<Control::CMD32EN>()) {
					write<Counter::CNT32>(Counter::CNT32::REFRESH);
				} else {
					Genode::error("Watchdog: CNT8 & CNT16 refresh not implemented");
				}
				/* reschedule */
				_service_handler.schedule(_service_timeout.trunc_to_plain_us());
			}

			void _handle_irq()
			{
				Genode::error("watchdog: service failed, device will restart");
				_irq.ack_irq();
			}

			Ressources(Env &env, Io_mem::Range io_mem, unsigned irq, unsigned timeout_ms)
			: Attached_mmio(env, { reinterpret_cast<char*>(io_mem.start), io_mem.size }),
			  _env { env },
			  _irq { env, irq },
			  _irq_handler { env.ep(), *this, &Ressources::_handle_irq },
			  _service_timeout { Genode::Milliseconds { timeout_ms } }
			{
				if (read<Control::EN>() == 1) {
					_service_handler.schedule(_service_timeout.trunc_to_plain_us());
				}

				if (read<Control::INT>() == 1) {
					_irq.sigh(_irq_handler);
					_irq.ack_irq();
				}
			}
		};

		Env                                &_env;
		bool                                _verbose;
		Constructible<Watchdog::Ressources> _device {};


	public:

		Watchdog(Env &env, bool verbose)
		:  _env(env), _verbose(verbose)
		{ }

		virtual ~Watchdog() {}

		void enable_device(Device const &device) override {
			bool missing_property { false };

			/* read IRQ configuration */
			unsigned irq { 0xFF };
			device.for_each_irq([&irq] (unsigned,
			                            unsigned number,
			                            Genode::Irq_session::Type,
			                            Genode::Irq_session::Polarity,
			                            Genode::Irq_session::Trigger,
			                            bool)
			{ irq = number; });
			if (irq == 0xFF) {
				Genode::error("Watchdog:", device.name()," missing 'irq'");
				missing_property = true;
			}

			/* read timeout-sec property configuration */
			unsigned timeout_sec { 0 };
			unsigned timeout_ms  { 0 };
			device.for_each_property([&] (const Device::Property::Name &name, const Device::Property::Name &value)
			{
				if (name == "timeout-sec") {
					if (Genode::parse({ value.string(), value.length() }, timeout_sec) == 0) {
						Genode::error("Watchdog: ", device.name(), " invalide 'timeout-sec' property ", value);
						missing_property = true;
					}
				}
			});
			if (timeout_sec == 0 && !missing_property) {
				Genode::error("Watchdog: ", device.name(), " missing 'timeout-sec' property");
				missing_property = true;
			}
			timeout_ms = timeout_sec * 1000 / 4;
			if (timeout_ms == 0) {
				Genode::error("Watchdog: ", device.name(),
				              " 'timeout-sec' property too-small, can not serve every 0s");
				missing_property = true;
			}

			/* read IOMEM configuration */
			Io_mem::Range io_mem {};
			device.for_each_io_mem([&] (unsigned, Io_mem::Range range, Pci_bar, bool) {
				io_mem = range;
			});
			if (io_mem.start == 0 && io_mem.size == 0) {
				Genode::error("Watchdog: ", device.name(), " missing 'io_mem'");
				missing_property = true;
			}

			/* return if there is a missing property */
			if (missing_property) return;

			/* contruct device */
			_device.construct(_env, io_mem, irq, timeout_ms);
			if (_verbose) {
				Genode::log("Watchdog: ", device.name(), "@", Hex { io_mem.start },
				            " servicing every ", timeout_ms, "ms (", timeout_sec, "s timeout)");
			}
		}

		/* when armed, watchdog can not be disabled */
		void disable_device(Device const &) override {}

		/* when armed, watchdog can not be updated */
		void update_devices_rom() override {}
};
