/*
 * \brief  Platform driver for i.MX 9 SoC
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


/* genode includes */
#include <base/component.h>
#include <base/attached_rom_dataspace.h>

/* platform driver common includes */
#include <common.h>
#include <common/src.h>

/* local includes */
#include "ccm.h"
#include "gpc.h"
#include "iomuxc.h"
#include "watchdog.h"
#include "thermal.h"

namespace Driver { struct Main; };

struct Driver::Main
{
	Env                  & _env;
	Attached_rom_dataspace _config_rom     { _env, "config"        };
/*	Attached_rom_dataspace _system_rom     { _env, "system"        }; */
	Common                 _common         { _env, _config_rom     };
	Signal_handler<Main>   _config_handler { _env.ep(), *this,
	                                         &Main::_handle_config };
/*	Signal_handler<Main>   _system_handler { _env.ep(), *this,
	                                         &Main::_system_update }; */

	bool _verbose { _config_rom.node().attribute_value("verbose", false) };

/*	Iomuxc   _iomuxc   { _env, _common.platform_info() }; */
	Watchdog   _watchdog { _env, _verbose };
	Thermal    _thermal { _env };

	Ccm _ccm { _env, _common.devices().clocks(), _verbose };
/*	Gpc _gpc { _env, _common.devices().powers() }; */
/*	Src _src { _env, _common.devices().resets() }; */

	void _handle_config();
	void _system_update();

	void _handle_watchdog()
	{
		auto wdg_device = _config_rom.node().attribute_value("watchdog", Driver::Device::Name {});

		if (wdg_device != "") {
			_common.devices().for_each([&] (Device &device) {
				if (device.name() == wdg_device) {
					device.acquire(_watchdog);
				}
			});
		}
	}

	Main(Genode::Env & e)
	: _env(e)
	{

		_handle_watchdog();
		_config_rom.sigh(_config_handler);
/*		_system_rom.sigh(_system_handler); */
		_handle_config();
		_system_update();
		_common.announce_service();
	}
};


void Driver::Main::_handle_config()
{
	_config_rom.update();
	_common.handle_config(_config_rom.node());
}


void Driver::Main::_system_update()
{
//	_system_rom.update();
//
//	if (!_system_rom.valid())
//		return;
//
//	auto const state = _system_rom.node().attribute_value("state", String<16>());
//
//	if (state == "reset") {
//		if (_verbose) warning("Will reset...");
//		_watchdog.reset();
//	}
}


void Component::construct(Genode::Env &env) { static Driver::Main main(env); }
