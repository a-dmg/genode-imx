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
{ }


void Board::Cpu::wake_up_all_cpus(void * ip)
{

	/*
	 * The cortex-a55 core is not multi-threaded, so the Affinity-0[7:0] bitset
	 * is always 0. This is a problem, because affinities are coded
	 * on 8 bits values and there are 4 of them defined in MPIDR_EL1 register,
	 * that provides a core identification mechanisme.
	 *
	 * On a cortex-a55 core, the MT[24] bit of the MPIDR_EL1 register
	 * is supposedly always 0, unless it is in a system with other
	 * cores that are multi-threaded.
	 *
	 * The cpu is then identified in the Affinity-1[11:8] bitset of
	 * MPIDR_EL1 register, that identify cores in a cluster and can
	 * vary from 0x0 to 0x7.
	 *
	 * This means that the cpu's id are not contigus on this platform,
	 * and identified with a 12 bits value, possibly offseted by 0x100,
	 * but not necessarly on other SoC that implement big.LITTLE for example.
	 * It is not the case of imx9 series SoC, it is safe to assume here that
	 * core's ids are non-contiguous and offsetted by 0x100.
	 *
	 * The bootstrap's and GIC's must be aware of that and eventually properly
	 * re-map the id's to a contigus 8bits id value.
	 *
	 * Please read section 3.2.90 of the Arm Cortex-A55 Core Technical Reference Manual.
	 */

	constexpr Genode::uint16_t AFFINITY1_OFFSET = 0x100;

	for (unsigned cpu_id = AFFINITY1_OFFSET; cpu_id < NR_OF_CPUS * AFFINITY1_OFFSET; cpu_id += AFFINITY1_OFFSET) {
		if (!Board::Psci::cpu_on(cpu_id, ip)) {
			Genode::error("(BOOT_CPU) Failed to boot secondary CPU");
		}
	}
}
