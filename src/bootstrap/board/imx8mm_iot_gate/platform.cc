/*
 * \brief   Platform implementations specific for base-hw and i.MX8Q EVK
 * \author  Stefan Kalkowski
 * \author  Pirmin Duss
 * \author  Alice Domage
 * \date    2020-10-01
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
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

		/* IOMUXC pin control for usdhc 2 */
		{ 0x30330038, 0x0 }, /* GPIO1_IO4 set ALT 0  - GPIO1_IO4 */
		{ 0x303300EC, 0x0 }, /* SD2_REET_B set ALT 5 - GPIO2_IO19 */
		{ 0x303300D0, 0x5 }, /* SD2_CD_B set ALT 5   - GPIO2_IO12 */
		{ 0x303300D4, 0x0 }, /* SD2_CLK set ALT 0    - USDHC_2_CLK */
		{ 0x303300D8, 0x0 }, /* SD2_CMD set ALT 0    - USDHC_2_CMD */
		{ 0x303300DC, 0x0 }, /* SD2_DATA0 set ALT 0  - USDHC_2_DATA0 */
		{ 0x303300E0, 0x0 }, /* SD2_DATA1 set ALT 0  - USDHC_2_DATA1 */
		{ 0x303300E4, 0x0 }, /* SD2_DATA2 set ALT 0  - USDHC_2_DATA2 */
		{ 0x303300E8, 0x0 }, /* SD2_DATA3 set ALT 0  - USDHC_2_DATA3 */
		{ 0x303300F0, 0x5 }, /* SD2_WP set ALT 5     - GPIO2_IO20 */

		/* IOMUXC pad control for usdhc 2 */
		{ 0x303302A0, 0x1d0 }, /* GPIO1_IO4 set PE, HYS, PUE */
		{ 0x30330338, 0x81  }, /* SD2_CD_B set PUE, DSE:X1 */
		{ 0x3033033C, 0x190 }, /* SD2_CLK set PE, HYS, FSL:FAST */
		{ 0x30330340, 0x1d0 }, /* SD2_CMD set PE, HYS, PUE */
		{ 0x30330344, 0x1d0 }, /* SD2_DATA0 set PE, HYS, PUE */
		{ 0x30330348, 0x1d0 }, /* SD2_DATA1 set PE, HYS, PUE */
		{ 0x3033034C, 0x1d0 }, /* SD2_DATA2 set PE, HYS, PUE */
		{ 0x30330350, 0x1d0 }, /* SD2_DATA3 set PE, HYS, PUE */
		{ 0x30330354, 0x41  }, /* SD2_RESET_B set PUE, DSE:X1 */
		{ 0x30330358, 0x116 }, /* SD2_WP */

		/* IOMUXC pad control for USB */
		{ 0x30330050, 0x1 }, /* IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO10  - USB1_OTG_ID */
		{ 0x30330054, 0x1 }, /* IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO11  - USB2_OTG_ID */
		{ 0x30330058, 0x1 }, /* IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO12  - USB1_OTG_PWR */
		{ 0x3033005C, 0x1 }, /* IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO13  - USB1_OTG_OC */
		{ 0x30330060, 0x1 }, /* IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO14  - USB2_OTG_PWR */
		{ 0x30330064, 0x1 }, /* IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO15  - USB2_OTG_OC */

		/* IOMUXC pin control for usdhc 3 */
		{ 0x303300FC, 0x2  }, /* NAND_CE1_B set ALT 2  - USDHC3_STROBE */
		{ 0x30330100, 0x2  }, /* NAND_CE2_B set ALT 2  - USDHC3_DATA5 */
		{ 0x30330104, 0x2  }, /* NAND_CE3_B set ALT 2  - USDHC3_DATA6 */
		{ 0x30330108, 0x2  }, /* NAND_CLE set ALT 2    - USDHC3_DATA7 */
		{ 0x3033011C, 0x2  }, /* NAND_DATA04 set ALT 2 - USDHC3_DATA0 */
		{ 0x30330120, 0x2  }, /* NAND_DATA05 set ALT 2 - USDHC3_DATA1 */
		{ 0x30330124, 0x2  }, /* NAND_DATA06 set ALT 2 - USDHC3_DATA2 */
		{ 0x30330128, 0x2  }, /* NAND_DATA07 set ALT 2 - USDHC3_DATA3 */
		{ 0x30330130, 0x2  }, /* NAND_RE_B set ALT 2   - USDHC3_DATA4 */
		{ 0x30330138, 0x12 }, /* NAND_WE_B set ALT 2   - USDHC3_CLK */
		{ 0x3033013C, 0x2  }, /* NAND_WP_B set ALT 2   - USDHC3_CMD */

		/* IOMUXC pad control for usdhc 3 */
		{ 0x30330364, 0x190      }, /* NAND_CE1_B set FSL:FAST, HYS, PE */
		{ 0x30330368, 0x1d0      }, /* NAND_CE2_B set HYS, PUE, ODE, PE */
		{ 0x3033036C, 0x1d0      }, /* NAND_CE3_B set HYS, PUE, ODE, PE */
		{ 0x30330370, 0x1d0      }, /* NAND_CLE set HYS, PUE, ODE, PE */
		{ 0x30330384, 0x1d0      }, /* NAND_DATA04 set HYS, PUE, ODE, PE */
		{ 0x30330388, 0x1d0      }, /* NAND_DATA05 set HYS, PUE, ODE, PE */
		{ 0x3033038C, 0x1d0      }, /* NAND_DATA06 set HYS, PUE, ODE, PE */
		{ 0x30330390, 0x1d0      }, /* NAND_DATA07 set HYS, PUE, ODE, PE */
		{ 0x30330398, 0x1d0      }, /* NAND_RE_B set HYS, PUE, ODE, PE */
		{ 0x303303A0, 0x40000190 }, /* NAND_WE_B set FSL:FAST, HYS, PE */
		{ 0x303303A4, 0x1d0      }, /* NAND_WP_B set HYS, PUE, ODE, PE */

		/* GPIO5 pin control for TPM Infineon SLB 9670 */
		{ 0x30330240, 0x05 }, /* GPIO5_IO25 set ALT2 - TPM SS */
		{ 0x3033024C, 0x05 }, /* GPIO5_IO28 set ALT2 - TPM RST */
		{ 0x30330250, 0x05 }, /* GPIO5_IO29 set ALT2 - TPM PIRQ */

		/* ECSPI3 pin control for TPM Infineon SLB 9670 */
		{ 0x30330234, 0x01 }, /* ECSPI3_SCLK set ALT 1 - TPM SCLK */
		{ 0x30330238, 0x01 }, /* ECSPI3_MOSI set ALT 1 - TPM MOSI */
		{ 0x3033023C, 0x01 }, /* ECSPI3_MISO set ALT 1 - TPM MISO */

		/* IOMUXC pin control for ENET */
		{ 0x30330068, 0x0  }, /* ENET_MDC set ALT 0 - ENET1_MDC */
		{ 0x3033006C, 0x0  }, /* ENET_MDIO set ALT 0 - ENET1_MDIO */
		{ 0x303304C0, 0x1  }, /* ENET1_MDIO_SELECT_INPUT - ENET_MDIO_ALT 0*/
		{ 0x30330070, 0x0  }, /* ENET_TD3 set ALT 0 - ENET1_RGMMI_TD3 */
		{ 0x30330074, 0x0  }, /* ENET_TD2 set ALT 0 - ENET1_RGMII_TD2 */
		{ 0x30330078, 0x0  }, /* ENET_TD1 set ALT 0 - ENET1_TD1 */
		{ 0x3033007C, 0x0  }, /* ENET_TD0 set ALT 0 - ENET1_RGMII_TD1 */
		{ 0x30330080, 0x0  }, /* ENET_TX_CTL set ALT 0 - ENET1_TX_CTL */
		{ 0x30330084, 0x0  }, /* ENET_TXC set ALT 0 - ENET1_RGMII_TXC */
		{ 0x30330088, 0x0  }, /* ENET_RX_CTL set ALT 0 - ENET1_RX_CTL */
		{ 0x3033008C, 0x0  }, /* ENET_RXC set ALT 0 - ENET1_RGMII_RXC */
		{ 0x30330090, 0x0  }, /* ENET_RD0 set ALT 0 - ENET1_RGMII_RD0 */
		{ 0x30330094, 0x0  }, /* ENET_RD1 set ALT 0 - ENET1_RGMII_RD1 */
		{ 0x30330098, 0x0  }, /* ENET_RD2 set ALT 0 - ENET1_RGMII_RD2 */
		{ 0x3033009C, 0x0  }, /* ENET_RD3 set ALT 0 - ENET1_RGMII_RD3 */

		/* IOMUXC pad control for ENET */
		{ 0x303302D0, 0x3  }, /* ENET_MDC */
		{ 0x303302D4, 0x3  }, /* ENET_MDIO */
		{ 0x303302D8, 0x1f }, /* ENET_TD3 */
		{ 0x303302DC, 0x1f }, /* ENET_TD2 */
		{ 0x303302E0, 0x1f }, /* ENET_TD1 */
		{ 0x303302E4, 0x1f }, /* ENET_TD0 */
		{ 0x303302E8, 0x1f }, /* ENET_TX_CTL */
		{ 0x303302EC, 0x1f }, /* ENET_TXC */
		{ 0x303302F0, 0x91 }, /* ENET_RX_CTL */
		{ 0x303302F4, 0x91 }, /* ENET_RXC */
		{ 0x303302F8, 0x91 }, /* ENET_RD0 */
		{ 0x303302FC, 0x91 }, /* ENET_RD1 */
		{ 0x30330300, 0x91 }, /* ENET_RD2 */
		{ 0x30330304, 0x91 }, /* ENET_RD3 */

		/* USB net control GPIOs */
		{ 0x30330154, 0x005 },  /* SAI5_RXD3_GPIO3_IO24 set ALT 5 - HUB_RSTn */
		{ 0x303301CC, 0x005 },  /* SAI3_RXFS_GPIO4_IO28 set ALT 5 - USB_PS_EN */
		{ 0x303303BC, 0x140 },  /* SAI5_RXD3_GPIO3_IO24 enable pull up resistor */
		{ 0x30330434, 0x140 },  /* SAI5_RXD3_GPIO3_IO24 enable pull up resistor */

		/* I2c2 pin/pad control for RTC AM1805 */
		{ 0x3033021C, 0x10  }, /* Enable SION I2c2_scl */
		{ 0x30330220, 0x10  }, /* Enable SION I2c2_sda */
		{ 0x30330484, 0x1C3 }, /* I2c2_scl pullup resistor */
		{ 0x30330488, 0x1C3 }, /* I2c2_sda pullup resistor */
	};

	unsigned num_values = sizeof(iomux_values) / (2*sizeof(unsigned long));
	for (unsigned i = 0; i < num_values; i++)
		*((volatile Genode::uint32_t*)iomux_values[i][0]) = (Genode::uint32_t)iomux_values[i][1];

//
// PLLs and clocks are handler differently on the iMX8m mini than on the iMX8mq.
//
// Normally most of the clocks are already initialized by uboot / ATF (ARM Trusted Firmware).
//
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
