/*
 * \brief  Driver that connects the option UART of USB modems
 *         to a terminal session
 * \author Sebastian Sumpf
 * \author Pirmin Duss
 * \date   2022-09-05
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */


/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <terminal_session/connection.h>

/* temporary includes */
#include <util/string.h>

/* local includes */
#include "option_serial.h"

namespace Uart {

	using namespace Genode;

	class Main;
};


class Uart::Main
{
	private:

		Env &_env;

		bool                   _usb_connected     { false };
		Attached_rom_dataspace _config            { _env, "config" };

		Usb_option             _driver            { _env };
		Signal_handler<Main>   _usb_handler       { _env.ep(), *this, &Main::_read_usb };
		Signal_handler<Main>   _usb_con_handler   { _env.ep(), *this, &Main::_usb_connect };
		Signal_handler<Main>   _terminal_handler  { _env.ep(), *this, &Main::_read_terminal };

		Constructible<Terminal::Connection>  _terminal  { };

		void _read_terminal()
		{
			static Command cmd { };
			while (_terminal->avail()) {

				char tmp[32] { };
				size_t size = _terminal->read(tmp, sizeof(cmd));
//				if (_usb_connected) {
					for (size_t s = 0; s < size; s++) {

						cmd = Command { cmd, Cstring { const_cast<const char *>(&tmp[s]), 1 } };
						if (cmd.complete()) {
Genode::log(__FUNCTION__, "::", __LINE__, ": data recieved from terminal");
							_driver.write(cmd);
							cmd = Command { };
						}
					}
//				}
			}
		}

		void _usb_connect()
		{
			if (!_terminal.constructed()) {
				_terminal.construct(_env);
				_terminal->read_avail_sigh(_terminal_handler);
			}

			/* read old stuff from terminal and throw it away */
//			while (_terminal->avail()) {
//				char buf[32] { };
//				_terminal->read(buf, sizeof(buf));
//				Genode::log("Discarding: ", Genode::Cstring { buf });
//			}
//			_usb_connected = true;
			_read_terminal();
		}

		void _read_usb()
		{
			while (_driver.char_avail()) {
				char c = _driver.get_char();
				_terminal->write(&c, 1);
			}
		}

	public:

		Main(Genode::Env &env) : _env(env)
		{
			_driver.read_avail_sigh(_usb_handler);
			_driver.connected_sigh(_usb_con_handler);
			//_usb_connect();
		}

};


void Component::construct(Genode::Env &env)
{
	static Uart::Main uart(env);
}
