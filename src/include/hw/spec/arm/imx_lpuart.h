/*
 * \brief  Driver for Freescale's i.MX lpuart
 * \author Alice Domage
 * \date   2025-08-20
 */

/*
 * Copyright (C) 2025 Gapfruit
 * Copyright (C) 2025 Genode Labs GmbH
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
	/* Provide module status */
	struct Stat: Register<0x18, 32> {
		struct Tdre: Bitfield<23, 1>  { }; /* Transmit data register empty */
	};

	/* Lpuart features control */
	struct Ctrl: Register<0x18, 32> {
		struct Pt:       Bitfield<0, 1>  { }; /* */
		struct Pe:       Bitfield<1, 1>  { }; /* */
		struct Ilt:      Bitfield<2, 1>  { }; /* */
		struct Wake:     Bitfield<3, 1>  { }; /* */
		struct M:        Bitfield<4, 1>  { }; /* */
		struct Rsrc:     Bitfield<5, 1>  { }; /* */
		struct Doze_en:  Bitfield<6, 1>  { }; /* */
		struct Loops:    Bitfield<7, 1>  { }; /* */
		struct Idle_cfg: Bitfield<8, 3>  { }; /* */
		struct M7:       Bitfield<11, 1> { }; /* */
		struct MA2ie:    Bitfield<14, 1> { }; /* */
		struct MA1ie:    Bitfield<15, 1> { }; /* */
		struct Re:       Bitfield<18, 1> { }; /* */
		struct Te:       Bitfield<19, 1> { }; /* */

		static access_t disable()
		{
			return Re::bits(0) | Te::bits(0);
		}

		static access_t enable()
		{
			/*The settings are always 8 data bits,
			 * no parity, 1 stop bit, no start bits.
			 */
			return Te::bits(1) | M::bits(0) | Pe::bits(0);
		}
	};

	struct Data: Register<0x1C, 32> {
		struct Fifo: Bitfield<0, 8> { }; /* FIFO read or transmit */
	};

	struct Match: Register<0x20, 32> { };
	struct Modir: Register<0x24, 32> { };

	/* Fifo control */
	struct Fifo: Register<0x24, 32> {
		struct Tx_fifo_size: Bitfield<4, 3>  { }; /* transmit FIFO buffer depth */
		struct Tx_fe:        Bitfield<7, 1>  { }; /* transmit FIFO enable */
		struct Tx_flush:     Bitfield<15, 1> { }; /* transmit FIFO flush */
	};

	/* Programable threshold for notification and tx/rx operations */
	struct Water: Register<0x24, 32> {
		struct Tx_water: Bitfield<0, 4> { }; /* Transmit watermark */
	};

	/**
	 * Transmit character 'c' without care about its type
	 */
	void _put_char(char c)
	{
		while(read<Stat::Tdre>() == 0) {}
		write<Data::Fifo>(c);
	}

	public:

		/**
		 * Constructor
		 *
		 * \param base  device MMIO base
		 */
		Imx_lpuart(Genode::addr_t base, Genode::uint32_t, Genode::uint32_t)
		:
			Genode::Mmio<SIZE>({(char*)base, SIZE})
		{
			init();
		}

		void init()
		{
			write<Ctrl>(Ctrl::disable());
			write<Modir>(0);

			/* set the TX water to half of FIFO size */
			Genode::uint32_t tx_fifo_size = read<Fifo::Tx_fifo_size>();
			if (tx_fifo_size > 1)
				tx_fifo_size = tx_fifo_size >> 1;
			write<Water::Tx_water>(tx_fifo_size);

			write<Fifo>(Fifo::Tx_flush::bits(1) | Fifo::Tx_fe::bits(1));
			write<Match>(0);
			write<Ctrl>(Ctrl::enable());
		}

		/**
		 * Print character 'c' through the UART
		 */
		void put_char(char c)
		{
			/* transmit character */
			_put_char(c);
		}
};

#endif /* _INCLUDE__DRIVERS__LPUART__IMX_H_ */
