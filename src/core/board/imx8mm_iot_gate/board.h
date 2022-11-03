/*
 * \brief  Board driver for core
 * \author Pirmin Duss
 * \date   2020-10-01
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _CORE__SPEC__IMX8MM_IOT_GATE__BOARD_H_
#define _CORE__SPEC__IMX8MM_IOT_GATE__BOARD_H_

#include <hw/spec/arm_64/imx8mm_iot_gate_board.h>
#include <spec/arm/generic_timer.h>
#include <spec/arm/virtualization/gicv3.h>
#include <spec/arm_v8/cpu.h>

/* base-hw includes */
#include <spec/arm_64/cpu/vcpu_state_virtualization.h>

/* base-hw Core includes */
#include <spec/arm/virtualization/board.h>

namespace Board {
	using namespace Hw::Imx8mm_iot_gate_board;

	enum {
		TIMER_IRQ           = 14 + 16,
		VT_TIMER_IRQ        = 11 + 16,
		VT_MAINTAINANCE_IRQ = 9  + 16,
		VCPU_MAX            = 16
	};
};

#endif /* _CORE__SPEC__IMX8MM_IOT_GATE__BOARD_H_ */
