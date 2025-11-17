/*
 * \brief  Analog fraction PLL driver for i.MX9
 * \author Alice Domage
 * \date   2025-11-17
 */

/*
 * Copyright (C) 2025 Genode Labs GmbH
 * Copyright (C) 2025 gapfruit ag
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#pragma once

#include <common/ccm.h>
#include <os/attached_mmio.h>

namespace Driver {
	using namespace Genode;

	class Fracn_gpll;
};


class Driver::Fracn_gpll: public Driver::Clock, Mmio<0x100>
{
	public:

		enum Flag {
			FRACN_GPPLL_INTEGER = 0b0,
			FRACN_GPPLL_FRACN = 0b1,
		};


	private:

		Clock    & _parent;
		Flag const _flag;
		bool const _verbose;
		bool const _always_on;

	protected:
		void _enable()         override { Genode::warning("Fracn_gpll: ", __FUNCTION__, " is unimplemented."); }
		void _disable()        override { Genode::warning("Fracn_gpll: ", __FUNCTION__, " is unimplemented."); }

	public:

		Fracn_gpll(Clocks               &clocks,
				   Name                  name,
				   Clock                &parent,
				   Byte_range_ptr const &range,
				   Flag                  flag,
				   bool                  verbose,
				   bool const            always_on = false)
		: Driver::Clock { clocks, name },
		  Mmio          { range },
		  _parent       { parent },
		  _flag         { flag },
		  _verbose      { verbose },
		  _always_on    { always_on }
		{}

		void parent(Name /* name */) override { Genode::warning("Fracn_gpll: ", __FUNCTION__, " is unimplemented."); }
		void rate(Rate /* rate */)   override { Genode::warning("Fracn_gpll: ", __FUNCTION__, " is unimplemented.");}
		Rate rate()      const override { Genode::warning("Fracn_gpll: ", __FUNCTION__, " is unimplemented.");return {0}; }
};
