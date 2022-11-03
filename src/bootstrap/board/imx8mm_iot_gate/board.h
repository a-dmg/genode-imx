/*
 * \brief  Board driver for bootstrap
 * \author Pirmin Duss
 * \date   2019-06-12
 */

#ifndef _BOOTSTRAP__SPEC__IMX8MM_IOT_GATE__BOARD_H_
#define _BOOTSTRAP__SPEC__IMX8MM_IOT_GATE__BOARD_H_

#include <hw/spec/arm_64/imx8mm_iot_gate_board.h>
#include <hw/spec/arm_64/cpu.h>
#include <hw/spec/arm/gicv3.h>
#include <hw/spec/arm/lpae.h>

namespace Board {
	using namespace Hw::Imx8mm_iot_gate_board;

	struct Cpu : Hw::Arm_64_cpu
	{
		static void wake_up_all_cpus(void*);
	};

	using Hw::Pic;
};

#endif /* _BOOTSTRAP__SPEC__IMX8MM_IOT_GATE__BOARD_H_ */
