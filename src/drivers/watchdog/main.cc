/*
 * \brief  Reset controller for imx8
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


/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <base/component.h>

/* Local includes */
#include "watchdog.h"


namespace Watchdog {

	using namespace Genode;

	class Main;
}

class Watchdog::Main
{
	private:

		Watchdog::Controller           _controller;
		Signal_handler<Watchdog::Main> _system_report_handler;
		Attached_rom_dataspace         _system_report;

		void state_changed()
		{
			_system_report.update();

			if (!_system_report.valid())
				return;

			try {
				if (_system_report.xml().attribute_value("state", String<32>()) == "reset") {
					_controller.issue_por();
				}
			} catch (...) {
				error("Invalide system report");
				error(_system_report.xml());
			}
		}


	public:

		Main(Genode::Env &env)
		:
			_controller            { env },
			_system_report_handler { env.ep(), *this, &Watchdog::Main::state_changed },
			_system_report         { env, "system" }
		{
			_system_report.sigh(_system_report_handler);
		}
};

void Component::construct(Genode::Env &env) { static Watchdog::Main main(env); }

