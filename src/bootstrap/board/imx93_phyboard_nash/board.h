/*
 * \brief  Board driver for bootstrap
 * \author Alice Domage
 * \date   2025-08-20
 */

/*
 * Copyright (C) 2025 Genode Labs GmbH
 * Copyright (C) 2025 gapfruit ag
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _BOOTSTRAP__SPEC__IMX93_PHYBOARD_NASH__BOARD_H_
#define _BOOTSTRAP__SPEC__IMX93_PHYBOARD_NASH__BOARD_H_

#include <hw/spec/arm_64/imx93_phycore_som_board.h>
#include <hw/spec/arm_64/cpu.h>
#include <hw/spec/arm/gicv3.h>
#include <hw/spec/arm/lpae.h>
#include <hw/spec/arm_64/psci_call.h>

namespace Board {
	using namespace Hw::Imx93_phycore_som_board;

	using Psci = Hw::Psci<Hw::Psci_smc_functor>;

	struct Cpu : Hw::Arm_64_cpu
	{
		static void wake_up_all_cpus(void*);
	};

	using Hw::Global_interrupt_controller;
	using Hw::Local_interrupt_controller;
};

#endif /* _BOOTSTRAP__SPEC__IMX93_PHYBOARD_NASH__BOARD_H_ */
