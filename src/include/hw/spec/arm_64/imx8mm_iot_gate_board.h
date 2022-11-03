/*
 * \brief  Board definitions for i.MX8 Quad EVK
 * \author Stefan Kalkowski
 * \author Pirmin Duss
 * \date   2020-10-01
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__INCLUDE__HW__SPEC__ARM_64__IMX8MM_IOT_GATE__BOARD_H_
#define _SRC__INCLUDE__HW__SPEC__ARM_64__IMX8MM_IOT_GATE__BOARD_H_

#include <drivers/uart/imx.h>
#include <hw/spec/arm/boot_info.h>

namespace Hw::Imx8mm_iot_gate_board {
	using Serial = Genode::Imx_uart;

	enum {
		RAM_BASE   = 0x40000000,
		RAM_SIZE   = 0x80000000,       /* 2 GiB */

		UART_BASE  = 0x30880000,       /* UART3 */
		UART_SIZE  = 0x1000,
		UART_CLOCK = 250000000,
	};

	static constexpr Genode::size_t NR_OF_CPUS = 4;

	namespace Cpu_mmio {
		enum {
			IRQ_CONTROLLER_DISTR_BASE  = 0x38800000,
			IRQ_CONTROLLER_DISTR_SIZE  = 0x10000,
			IRQ_CONTROLLER_VT_CPU_BASE = 0x31020000,
			IRQ_CONTROLLER_VT_CPU_SIZE = 0x2000,
			IRQ_CONTROLLER_REDIST_BASE = 0x38880000,
			IRQ_CONTROLLER_REDIST_SIZE = 0xc0000,
		};
	};
};

#endif /* _SRC__INCLUDE__HW__SPEC__ARM_64__IMX8MM_IOT_GATE__BOARD_H_ */
