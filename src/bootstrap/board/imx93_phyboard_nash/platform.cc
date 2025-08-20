/*
 * \brief   Platform implementations specific for base-hw and i.MX93 PHYTECH SOM
 * \author  Alice Domage
 * \date    2025-08-20
 */

/*
 * Copyright (C) 2025 Genode Labs GmbH
 * Copyright (C) 2025 gapfruit ag
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <platform.h>


/**
 * Leave out the first page (being 0x0) from bootstraps RAM allocator,
 * some code does not feel happy with addresses being zero
 */
Bootstrap::Platform::Board::Board()
:
	early_ram_regions(Memory_region { ::Board::RAM_BASE, ::Board::RAM_SIZE }),
	late_ram_regions(Memory_region { }),
	core_mmio(Memory_region { ::Board::UART_BASE, ::Board::UART_SIZE },
	          Memory_region { ::Board::Cpu_mmio::IRQ_CONTROLLER_DISTR_BASE,
	                          ::Board::Cpu_mmio::IRQ_CONTROLLER_DISTR_SIZE },
	          Memory_region { ::Board::Cpu_mmio::IRQ_CONTROLLER_REDIST_BASE,
	                          ::Board::Cpu_mmio::IRQ_CONTROLLER_REDIST_SIZE })
{
	::Board::Pic pic { };
}


void Board::Cpu::wake_up_all_cpus(void * ip)
{
	for (unsigned cpu_id = 0x100; cpu_id < NR_OF_CPUS * 0x100; cpu_id += 0x100) {
		if (!Board::Psci::cpu_on(cpu_id, ip)) {
			Genode::error("(BOOT_CPU) Failed to boot secondary CPU");
		}
	}
}
