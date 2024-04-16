/*
 * \brief   Platform implementations specific for base-hw and i.MX8Q EVK
 * \author  Alice Domage
 * \date    2024-02-27
 */

/*
 * Copyright (C) 2024 Genode Labs GmbH
 * Copyright (C) 2024 gapfruit ag
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

	static volatile unsigned long iomux_values[][2] {

		/* IOMUXC pin control for ENET */
		{ 0x30330054, 0x0  }, /* ENET_MDC set ALT 0 - ENET1_MDC */
		{ 0x30330058, 0x0  }, /* ENET_MDIO set ALT 0 - ENET1_MDIO */
		{ 0x3033057C, 0x0  }, /* ENET1_MDIO_SELECT_INPUT - ENET_MDIO_ALT 0*/
		{ 0x3033005C, 0x0  }, /* ENET_TD3 set ALT 0 - ENET1_RGMMI_TD3 */
		{ 0x30330060, 0x0  }, /* ENET_TD2 set ALT 0 - ENET1_RGMII_TD2 */
		{ 0x30330064, 0x0  }, /* ENET_TD1 set ALT 0 - ENET1_TD1 */
		{ 0x30330068, 0x0  }, /* ENET_TD0 set ALT 0 - ENET1_RGMII_TD1 */
		{ 0x3033006C, 0x0  }, /* ENET_TX_CTL set ALT 0 - ENET1_TX_CTL */
		{ 0x30330070, 0x0  }, /* ENET_TXC set ALT 0 - ENET1_RGMII_TXC */
		{ 0x30330074, 0x0  }, /* ENET_RX_CTL set ALT 0 - ENET1_RX_CTL */
		{ 0x30330078, 0x0  }, /* ENET_RXC set ALT 0 - ENET1_RGMII_RXC */
		{ 0x3033007C, 0x0  }, /* ENET_RD0 set ALT 0 - ENET1_RGMII_RD0 */
		{ 0x30330080, 0x0  }, /* ENET_RD1 set ALT 0 - ENET1_RGMII_RD1 */
		{ 0x30330084, 0x0  }, /* ENET_RD2 set ALT 0 - ENET1_RGMII_RD2 */
		{ 0x30330088, 0x0  }, /* ENET_RD3 set ALT 0 - ENET1_RGMII_RD3 */

		/* IOMUXC pad control for ENET */
		{ 0x303302B4, 0x2  }, /* ENET_MDC */
		{ 0x303302B8, 0x2  }, /* ENET_MDIO */
		{ 0x303302BC, 0x16 }, /* ENET_TD3 */
		{ 0x303302C0, 0x16 }, /* ENET_TD2 */
		{ 0x303302C4, 0x16 }, /* ENET_TD1 */
		{ 0x303302C8, 0x16 }, /* ENET_TD0 */
		{ 0x303302CC, 0x16 }, /* ENET_TX_CTL */
		{ 0x303302D0, 0x16 }, /* ENET_TXC */
		{ 0x303302D4, 0x90 }, /* ENET_RX_CTL */
		{ 0x303302D8, 0x90 }, /* ENET_RXC */
		{ 0x303302DC, 0x90 }, /* ENET_RD0 */
		{ 0x303302E0, 0x90 }, /* ENET_RD1 */
		{ 0x303302E4, 0x90 }, /* ENET_RD2 */
		{ 0x303302E8, 0x90 }, /* ENET_RD3 */

		/* Fec PHY */
		{ 0x303301A8, 0x5 }, /* SAI2_TXFS MUX GPIO4_IO24*/
		{ 0x30330408, 0x10 }, /* SAI2 PAD */

	};

	unsigned num_values = sizeof(iomux_values) / (2*sizeof(unsigned long));
	for (unsigned i = 0; i < num_values; i++)
		*((volatile Genode::uint32_t*)iomux_values[i][0]) = (Genode::uint32_t)iomux_values[i][1];

}


void Board::Cpu::wake_up_all_cpus(void * ip)
{
	enum Function_id { CPU_ON = 0xC4000003 };

	unsigned long result = 0;
	for (unsigned i = 1; i < NR_OF_CPUS; i++) {
		asm volatile("mov x0, %1  \n"
		             "mov x1, %2  \n"
		             "mov x2, %3  \n"
		             "mov x3, %2  \n"
		             "smc #0      \n"
		             "mov %0, x0  \n"
		             : "=r" (result) : "r" (CPU_ON), "r" (i), "r" (ip)
		                      : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
		                        "x8", "x9", "x10", "x11", "x12", "x13", "x14");
	}
}
