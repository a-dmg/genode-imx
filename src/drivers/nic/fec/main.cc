/*
 * \brief  i.MX FEC nic driver Linux port
 * \author Stefan Kalkowski
 * \author Alice Domage
 * \date   2023-01-30
 */

/*
 * Copyright (C) 2024 Genode Labs GmbH
 * Copyright (C) 2024 gapfruit ag
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

/* genode includes */
#include <base/component.h>
#include <base/env.h>
#include <base/attached_rom_dataspace.h>

/* lx_kit includes*/
#include <lx_kit/env.h>
#include <lx_emul/task.h>
#include <lx_emul/init.h>
#include <genode_c_api/uplink.h>

using namespace Genode;

extern task_struct *user_task_struct_ptr;

struct Main {


	Genode::Env &_env;
	Genode::Signal_handler<Main> _signal_handler { _env.ep(), *this, &Main::handle_signal };
	Genode::Attached_rom_dataspace _dtb_rom { _env, "nic_drv.dtb" };

	Main(Genode::Env &env)
	:
		_env { env }
	{
		log("--- i.MX Freescale nic driver started ---");
		Lx_kit::initialize(_env, _signal_handler);

		_env.exec_static_constructors();

		genode_uplink_init(genode_env_ptr(_env),
		                   genode_allocator_ptr(Lx_kit::env().heap),
		                   genode_signal_handler_ptr(_signal_handler));

		lx_emul_start_kernel(_dtb_rom.local_addr<void>());
	}

	void handle_signal()
	{
		if (user_task_struct_ptr)
			lx_emul_task_unblock(user_task_struct_ptr);

		Lx_kit::env().scheduler.execute();
		genode_uplink_notify_peers();
	}

};


void Component::construct(Genode::Env &env)
{
	static Main main { env };
}

