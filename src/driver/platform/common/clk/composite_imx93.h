/*
 * \brief  Composite clock for i.MX93 & i.MX91
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

namespace Driver {
	using namespace Genode;

	class Composite_clock_imx9;
};


class Composite_imx93: public Driver::Clock, Genode::Mmio<0x24>
{
	private:

		struct Control: Register<0x0, 32>
		{
			struct Div:    Bitfield<0,8>  {};
			struct Mux:    Bitfield<8,2> {
				enum Source {
					PARENT_0 = 0x0,
					PARENT_1 = 0x1,
					PARENT_2 = 0x2,
					PARENT_3 = 0x3,
					MAX      = 0x4,
					INVALIDE = 0xFF,
				};
			};
			struct Enable: Bitfield<24,1> { enum { OFF = 0x1, ON = 0x0 }; };
		};

		struct Status: Register<0x20, 32>
		{
			struct Div:    Bitfield<0,8>  {};
			struct Mux:    Bitfield<8,2> { };
			struct Enable: Bitfield<24,1> { enum { OFF = 0x1, ON = 0x0 }; };
		};

	public:

		struct Source_select
		{
			enum {
				LOW_SPEED_IO_SEL = 0x0,
				NON_IO_SEL,
				FAST_SEL,
				AUDIO_SEL,
				VIDEO_SEL,
				TPM_SEL,
				CKO1_SEL,
				CKO2_SEL,
				MISC_SEL,
				MAX_SEL,
			};

			struct Clock_ref {
				Driver::Clock &ref;
				Clock_ref(Driver::Clock & c) : ref(c) {}
			};

			Clock_ref _ref_clks[Control::Mux::Source::MAX];

			Source_select(Clock &ref_0, Clock &ref_1, Clock &ref_2, Clock &ref_3)
			:  _ref_clks { ref_0 , ref_1 , ref_2 , ref_3 }
			{}

			Control::Mux::Source mux_setting(Name name) const {
				for (unsigned i = 0; i < Control::Mux::Source::MAX; i++) {
					if (_ref_clks[i].ref.name == name) {
						return static_cast<Control::Mux::Source>(i);
					}
				}
				return Control::Mux::Source::INVALIDE;
			}
		};


	private:

		const Source_select &_source_select;
		const bool           _always_on;
		const bool           _verbose;


	protected:

		void _enable() override {
			auto mux = static_cast<Control::Mux::Source>(read<Status::Mux>());
			_source_select._ref_clks[mux].ref.enable();
			write<Control::Enable>(Control::Enable::ON);
			/* wait for setting to apply */
			while (read<Status::Enable>() != Control::Enable::ON) {}
			if (_verbose) Genode::log(name, " clock enabled");
		}

		void _disable() override {
			if (_always_on) return;
			auto mux = static_cast<Control::Mux::Source>(read<Status::Mux>());
			write<Control::Enable>(Control::Enable::OFF);
			_source_select._ref_clks[mux].ref.disable();
			/* wait for setting to apply */
			while (read<Status::Enable>() != Control::Enable::OFF) {}
			if (_verbose) Genode::log(name, " clock disabled");
		}


	public:

		Composite_imx93(Driver::Clocks       &clocks,
						Name                    name,
						Genode::Byte_range_ptr const &range,
						const Source_select          &refs,
						bool const              always_on = false,
						bool const              verbose = true)
		: Driver::Clock {  clocks, name }, Mmio { range },
		  _source_select { refs },
		  _always_on { always_on },
		  _verbose { verbose }
		{}

		void parent(Name parent) override {
			if (Genode::uint8_t mux = _source_select.mux_setting(parent) != Control::Mux::Source::INVALIDE) {
				/**
				 * enable parent before setting it,
				 * otherwise the system stalls
				 */
				_source_select._ref_clks[mux].ref.enable();
				/* configure parent */
				write<Control::Mux>(mux);
				/* wait for setting to apply */
				while (read<Status::Mux>() != mux) {Genode::log(__FUNCTION__);}
				if (_verbose) Genode::log(name, " set parent '", parent, "'");
			} else {
				Genode::error(name, " can not mux to '", parent,"', no such clock's parent.");
			}
		}

		void rate(Rate /* rate */)   override { Genode::warning("Fracn_gpll: ", __FUNCTION__, " is unimplemented.");}

		Rate rate()      const override { Genode::warning("Fracn_gpll: ", __FUNCTION__, " is unimplemented.");return {0}; }

};

