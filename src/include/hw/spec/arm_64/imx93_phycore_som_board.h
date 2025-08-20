/*
 * \brief  Board definitions for i.MX93 phycore som
 * \author Alice Domage
 * \date   2025-02-02
 */

/*
 * Copyright (C) 2025 Genode Labs GmbH
 * Copyright (C) 2025 gapfruit ag
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__INCLUDE__HW__SPEC__ARM_64__IMX93_PHYCORE_SOM__BOARD_H_
#define _SRC__INCLUDE__HW__SPEC__ARM_64__IMX93_PHYCORE_SOM__BOARD_H_

#include <hw/spec/arm/boot_info.h>
#include <hw/spec/arm/imx_lpuart.h>

namespace Hw::Imx93_phycore_som_board {
	using Serial = Hw::Imx_lpuart;

	enum {
		RAM_BASE   = 0x80000000,
		RAM_SIZE   = 0x80000000, /* 2 GiB */

		UART_BASE  = 0x44380000, /* lpuart1 */
		UART_SIZE  = 0x1000,
		UART_CLOCK = 24000000,   /* NOTE: uboot=> clk dump */
		BAUD_RATE  = 115200,     /* ATTENTION: other values may require to extend the imx_lpuart driver */
	};

	static constexpr Genode::size_t NR_OF_CPUS = 2;

	namespace Cpu_mmio {
		enum {
			IRQ_CONTROLLER_DISTR_BASE  = 0x48000000,
			IRQ_CONTROLLER_DISTR_SIZE  = 0x10000,
			IRQ_CONTROLLER_REDIST_BASE = 0x48040000,
			IRQ_CONTROLLER_REDIST_SIZE = 0xc0000,
		};
	};
};

#endif /* _SRC__INCLUDE__HW__SPEC__ARM_64__IMX93_PHYCORE_SOM__BOARD_H_ */
