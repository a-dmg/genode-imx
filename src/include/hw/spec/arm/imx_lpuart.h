/*
 * \brief  Driver for Freescale's i.MX lpuart
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

#ifndef _INCLUDE__DRIVERS__LPUART__IMX_H_
#define _INCLUDE__DRIVERS__LPUART__IMX_H_

/* Genode includes */
#include <util/mmio.h>

namespace Hw { class Imx_lpuart; }


/**
 * Driver for i.MX LPUART
 */
class Hw::Imx_lpuart: Genode::Mmio<0x1000>
{
	/* Perform global function */
	struct Global: Register<0x8, 32> {
		struct Rst: Bitfield<1, 1>  { }; /* Software Reset */
	};

	/* Baud */
	struct Baud: Register<0x10, 32> {
		struct Sbr: Bitfield<0, 13> {}; /* Baud Rate Modulo Divisor */
		struct Osr: Bitfield<24, 5>  { enum { OSR_16 = 0b00000 }; }; /* Oversampling Ratio */
	};

	/* Status */
	struct Stat: Register<0x14, 32> {
		struct Tdre: Bitfield<23, 1>  { }; /* Transmit Data Register Empty Flag */
	};

	/* Control */
	struct Ctrl: Register<0x18, 32> {
		struct Pe:       Bitfield<1, 1>  { }; /* Parity Enable */
		struct M:        Bitfield<4, 1>  { }; /* 9-Bit or 8-Bit Mode Select */
		struct Re:       Bitfield<18, 1> { }; /* Receive Enable */
		struct Te:       Bitfield<19, 1> { }; /* Transmit Enable */
		struct Ilie:     Bitfield<20, 1> { }; /* Idle Line Interrupt Enable */
		struct Rie:      Bitfield<21, 1> { }; /* Receiver Interrupt Enable */
		struct Tcie:     Bitfield<22, 1> { }; /* Transmission Complete Interrupt Enable */
		struct Tie:      Bitfield<23, 1> { }; /* Transmit Interrupt Enable */
		struct Peie:     Bitfield<24, 1> { }; /* Parity Interrupt Enable */
		struct Feie:     Bitfield<25, 1> { }; /* Framing Error Interrupt Enable */
		struct Neie:     Bitfield<26, 1> { }; /* Noise Error Interrupt Enable */
		struct Orie:     Bitfield<27, 1> { }; /* Overrun Interrupt Enable */
	};

	/* Data */
	struct Data: Register<0x1C, 32> {
		struct Fifo: Bitfield<0, 8> { }; /* FIFO register */
	};

	/* Fifo control */
	struct Fifo: Register<0x28, 32> {
		struct Txfe:    Bitfield<7, 1>  { }; /* transmit FIFO enable */
		struct Txflush: Bitfield<15, 1> { }; /* Transmit FIFO Flush */
	};

	Genode::uint32_t _module_clock; /* known asynchronous module clock */
	Genode::uint32_t _bauds;        /* desired baud rate*/

	public:

		/**
		 * Constructor
		 *
		 * \param base  device MMIO base
		 */
		Imx_lpuart(Genode::addr_t base, Genode::uint32_t module_clock, Genode::uint32_t bauds)
		:
			Genode::Mmio<SIZE>({(char*)base, SIZE}),
			_module_clock { module_clock },
			_bauds { bauds }
		{
			init();
		}

		/**
		 * Perform a soft reset of the functional block, configure 8-bits mode,
		 * no parity, 1 stop bit, no start bit, 115200 bauds.
		 */
		void init()
		{
			/* perform software reset */
			write<Global::Rst>(1); /* assert reset, it will not deassert automatically */
			write<Global::Rst>(0); /* there is no minimum delay to deassert the reset signial */

			/* transmit & receive disabled */
			write<Ctrl>(Ctrl::Re::bits(0) | Ctrl::Te::bits(0));

			Genode::uint32_t ctrl = read<Ctrl>();

			/* keep interrupts disabled */
			Ctrl::Ilie::set(ctrl, 0);
			Ctrl::Rie::set(ctrl, 0);
			Ctrl::Tcie::set(ctrl, 0);
			Ctrl::Tie::set(ctrl, 0);
			Ctrl::Peie::set(ctrl, 0);
			Ctrl::Feie::set(ctrl, 0);
			Ctrl::Neie::set(ctrl, 0);
			Ctrl::Orie::set(ctrl, 0);

			/* 8-bits Mode */
			Ctrl::M::set(ctrl, 0);
			Ctrl::Pe::set(ctrl, 0);

			/*
			 * Baud rate generation, osr & sbr may be written only when
			 * both tx and rx are disabled.
			 *
			 * Use osr = 16, which keep the baud rate tolerance <3% for
			 * a usual value of 115200 bauds. See Reference Manual 62.3.3
			 */
			write<Baud::Osr>(Baud::Osr::OSR_16);
			Genode::uint32_t sbr = _module_clock / (16 * _bauds);
			write<Baud::Sbr>(sbr);

			write<Ctrl>(ctrl);

			/* enable transmit */
			write<Ctrl::Te>(1);

			/* flush FIFO */
			write<Fifo>(Fifo::Txfe::bits(1));
		}

		/**
		 * Print character 'c' to the UART's transmitter FIFO. Writing to FIFO is a blocking
		 * operation.
		 */
		void put_char(char c)
		{
			/*
			 * Block until the last character of the message has moved to the transmitter
			 * shifter.
			 * see Reference Manual 62.3.4.1
			 */
			while(read<Stat::Tdre>() == 0) {}
			/* write character to FIFO */
			write<Data::Fifo>(c);
		}
};

#endif /* _INCLUDE__DRIVERS__LPUART__IMX_H_ */
