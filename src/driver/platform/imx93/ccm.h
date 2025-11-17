/*
 * \brief  Clock Controller Module for i.MX9
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
#include <common/clk/fracn_gpll.h>
#include <common/clk/composite_imx93.h>

namespace Driver {
	using namespace Genode;

	struct Ccm;
};

/* Clock Control Module */
struct Driver::Ccm
{
	using Root_clock         = Composite_imx93;
	using Root_parent        = Composite_imx93::Source_select;
//	using Root_clock_divider = ::Ccm::Root_clock_divider;
	using Gate               = ::Ccm::Gate;

	enum {
		CCM_MMIO_BASE        = 0x44450000,
		CCM_MMIO_SIZE        = 0x10000,
		CCM_ANALOG_MMIO_BASE = 0x44480000,
		CCM_ANALOG_MMIO_SIZE = 0x2000,
		ALWAYS_ON            = true,
	};

	Ccm(Genode::Env & env, Clocks & clocks, bool verbose);

	Genode::Env        &env;
	Clocks             &clocks;
	bool               verbose;
	Attached_mmio<0>   ccm_regs        { env, { reinterpret_cast<char *>(CCM_MMIO_BASE), CCM_MMIO_SIZE } };
	Attached_mmio<0>   ccm_analog_regs { env, { reinterpret_cast<char *>(CCM_ANALOG_MMIO_BASE), CCM_ANALOG_MMIO_SIZE } };

	Byte_range_ptr fracn_pll_range(unsigned pll) {
		return ccm_analog_regs.range_at(pll); }

	Byte_range_ptr root_range(unsigned nr) {
		return ccm_regs.range_at(nr); }

	Byte_range_ptr gate_range(unsigned off) {
		return ccm_regs.range_at(off); }

	Fixed_clock no_clk      { clocks, "no_clk",      {                 0 }};
	Fixed_clock k32_ref_clk { clocks, "32k_ref_clk", {             32768 }}; /* don't ask me why, ask NXP... */
	Fixed_clock m24_ref_clk { clocks, "24m_ref_clk", {  24 * 1000 * 1000 }}; /* 24  Mhz */
	Fixed_clock ext_1_clk   { clocks, "ext_1_clk",   { 133 * 1000 * 1000 }}; /* 133 Mhz*/

	Fixed_clock   system_pll_pfd0_clk      { clocks, "system_pll_pfd0_clk",      { 1 * 1000 * 1000 * 1000 } }; /* 1     Ghz*/
	Fixed_divider system_pll_pfd0_div2_clk { clocks, "system_pll_pfd0_div2_clk", system_pll_pfd0_clk, 2 };     /* 500   Mhz*/
	Fixed_clock   system_pll_pfd1_clk      { clocks, "system_pll_pfd1_clk",      {      800 * 1000 * 1000 } }; /* 800   Mhz*/
	Fixed_divider system_pll_pfd1_div2_clk { clocks, "system_pll_pfd1_div2_clk", system_pll_pfd1_clk, 2 };     /* 400   Mhz*/
	Fixed_clock   system_pll_pfd2_clk      { clocks, "system_pll_pfd2_clk",      {      625 * 1000 * 1000 } }; /* 625   Mhz */
	Fixed_divider system_pll_pfd2_div2_clk { clocks, "system_pll_pfd2_div2_clk", system_pll_pfd2_clk, 2 };     /* 312.5 Mhz */

	Fracn_gpll arm_pll_clk   { clocks,   "arm_pll_clk", m24_ref_clk, fracn_pll_range(0x1000), Fracn_gpll::FRACN_GPPLL_INTEGER, verbose, ALWAYS_ON };
	Fracn_gpll audio_pll_clk { clocks, "audio_pll_clk", m24_ref_clk, fracn_pll_range(0x1200), Fracn_gpll::FRACN_GPPLL_FRACN,   verbose };
	Fracn_gpll video_pll_clk { clocks, "video_pll_clk", m24_ref_clk, fracn_pll_range(0x1400), Fracn_gpll::FRACN_GPPLL_FRACN,   verbose };

	const Root_parent root_parents[Root_parent::MAX_SEL] = {
		{ m24_ref_clk, system_pll_pfd0_div2_clk, system_pll_pfd1_div2_clk, video_pll_clk },
		{ m24_ref_clk, system_pll_pfd0_div2_clk, system_pll_pfd1_div2_clk, system_pll_pfd2_div2_clk },
		{ m24_ref_clk, system_pll_pfd0_clk, system_pll_pfd1_clk, system_pll_pfd2_clk },
		{ m24_ref_clk, audio_pll_clk, video_pll_clk, ext_1_clk },
		{ m24_ref_clk, audio_pll_clk, video_pll_clk, system_pll_pfd0_clk },
		{ m24_ref_clk, system_pll_pfd0_clk, audio_pll_clk, ext_1_clk },
		{ m24_ref_clk, system_pll_pfd0_clk, system_pll_pfd1_clk, audio_pll_clk },
		{ m24_ref_clk, system_pll_pfd0_clk, system_pll_pfd1_clk, video_pll_clk },
		{ m24_ref_clk, audio_pll_clk, video_pll_clk, system_pll_pfd2_clk },
	};

	Root_clock a55_periph_clk_root { clocks, "a55_periph_root",
	                                 root_range(0x0000),
	                                 root_parents[Root_parent::FAST_SEL],
	                                 ALWAYS_ON };

	Root_clock a55_mtr_bus_clk_root { clocks, "a55_mtr_bus_root",
	                                  root_range(0x0080),
	                                  root_parents[Root_parent::LOW_SPEED_IO_SEL],
	                                  ALWAYS_ON };

	Root_clock a55_alt_clk_root { clocks, "a55_alt_root",
	                              root_range(0x0100),
	                              root_parents[Root_parent::FAST_SEL],
	                              ALWAYS_ON };

	Root_clock m33_clk_root { clocks, "m33_root",
	                          root_range(0x0180),
	                          root_parents[Root_parent::LOW_SPEED_IO_SEL],
	                          ALWAYS_ON };

	Root_clock bus_wakeup_clk_root { clocks, "bus_wakeup_root",
	                                 root_range(0x0280),
	                                 root_parents[Root_parent::LOW_SPEED_IO_SEL],
	                                 ALWAYS_ON };

	Root_clock bus_aon_clk_root { clocks, "bus_aon_root",
	                              root_range(0x0300),
	                              root_parents[Root_parent::LOW_SPEED_IO_SEL],
	                              ALWAYS_ON };

	Root_clock wakeup_axi_clk_root { clocks, "wakeup_axi_root",
	                                 root_range(0x0380),
	                                 root_parents[Root_parent::FAST_SEL],
	                                 ALWAYS_ON };

	Root_clock swo_trace_root { clocks, "swo_trace_root",
	                            root_range(0x0400),
	                            root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock m33_systick_root { clocks, "m33_systick_root",
	                              root_range(0x0480),
	                              root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock flexio1_root { clocks, "flexio1_root",
	                          root_range(0x0500),
	                          root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock flexio2_root { clocks, "flexio2_root",
	                          root_range(0x0580),
	                          root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lptmr1_root { clocks, "lptmr1_root",
	                         root_range(0x0700),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lptmr2_root { clocks, "lptmr2_root",
	                         root_range(0x0780),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock tpm2_root { clocks, "tpm2_root",
	                       root_range(0x0880),
	                       root_parents[Root_parent::TPM_SEL] };

	Root_clock tpm4_root { clocks, "tpm4_root",
	                       root_range(0x0980),
	                       root_parents[Root_parent::TPM_SEL] };

	Root_clock tpm5_root { clocks, "tpm5_root",
	                       root_range(0x0a00),
	                       root_parents[Root_parent::TPM_SEL] };

	Root_clock tpm6_root { clocks, "tpm6_root",
	                       root_range(0x0a80),
	                       root_parents[Root_parent::TPM_SEL] };

	Root_clock flexspi1_root { clocks, "flexspi1_root",
	                           root_range(0x0b00),
	                           root_parents[Root_parent::FAST_SEL] };

	Root_clock can1_root { clocks, "can1_root",
	                       root_range(0x0b80),
	                       root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock can2_root { clocks, "can2_root",
	                       root_range(0x0c00),
	                       root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpuart1_root { clocks, "lpuart1_root",
	                          root_range(0x0c80),
	                          root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpuart2_root { clocks, "lpuart2_root",
	                          root_range(0x0d00),
	                          root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpuart3_root { clocks, "lpuart3_root",
	                          root_range(0x0d80),
	                          root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpuart4_root { clocks, "lpuart4_root",
	                          root_range(0x0e00),
	                          root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpuart5_root { clocks, "lpuart5_root",
	                          root_range(0x0e80),
	                          root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpuart6_root { clocks, "lpuart6_root",
	                          root_range(0x0f00),
	                          root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpuart7_root { clocks, "lpuart7_root",
	                          root_range(0x0f80),
	                          root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpuart8_root { clocks, "lpuart8_root",
	                          root_range(0x1000),
	                          root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpi2c1_root { clocks, "lpi2c1_root",
	                         root_range(0x1080),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpi2c2_root { clocks, "lpi2c2_root",
	                         root_range(0x1100),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpi2c3_root { clocks, "lpi2c3_root",
	                         root_range(0x1180),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpi2c4_root { clocks, "lpi2c4_root",
	                         root_range(0x1200),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpi2c5_root { clocks, "lpi2c5_root",
	                         root_range(0x1280),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpi2c6_root { clocks, "lpi2c6_root",
	                         root_range(0x1300),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpi2c7_root { clocks, "lpi2c7_root",
	                         root_range(0x1380),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpi2c8_root { clocks, "lpi2c8_root",
	                         root_range(0x1400),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpspi1_root { clocks, "lpspi1_root",
	                         root_range(0x1480),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpspi2_root { clocks, "lpspi2_root",
	                         root_range(0x1500),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpspi3_root { clocks, "lpspi3_root",
	                         root_range(0x1580),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpspi4_root { clocks, "lpspi4_root",
	                         root_range(0x1600),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpspi5_root { clocks, "lpspi5_root",
	                         root_range(0x1680),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpspi6_root { clocks, "lpspi6_root",
	                         root_range(0x1700),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpspi7_root { clocks, "lpspi7_root",
	                         root_range(0x1780),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock lpspi8_root { clocks, "lpspi8_root",
	                         root_range(0x1800),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock i3c1_root { clocks, "i3c1_root",
	                       root_range(0x1880),
	                       root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock i3c2_root { clocks, "i3c2_root",
	                       root_range(0x1900),
	                       root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock usdhc1_root { clocks, "usdhc1_root",
	                         root_range(0x1980),
	                         root_parents[Root_parent::FAST_SEL] };

	Root_clock usdhc2_root { clocks, "usdhc2_root",
	                         root_range(0x1a00),
	                         root_parents[Root_parent::FAST_SEL] };

	Root_clock usdhc3_root { clocks, "usdhc3_root",
	                         root_range(0x1a80),
	                         root_parents[Root_parent::FAST_SEL] };

	Root_clock sai1_root { clocks, "sai1_root",
	                       root_range(0x1b00),
	                       root_parents[Root_parent::AUDIO_SEL] };

	Root_clock sai2_root { clocks, "sai2_root",
	                       root_range(0x1b80),
	                       root_parents[Root_parent::AUDIO_SEL] };

	Root_clock sai3_root { clocks, "sai3_root",
	                       root_range(0x1c00),
	                       root_parents[Root_parent::AUDIO_SEL] };

	Root_clock ccm_cko1_root { clocks, "ccm_cko1_root",
	                           root_range(0x1c80),
	                           root_parents[Root_parent::CKO1_SEL] };

	Root_clock ccm_cko2_root { clocks, "ccm_cko2_root",
	                           root_range(0x1d00),
	                           root_parents[Root_parent::CKO2_SEL] };

	Root_clock ccm_cko3_root { clocks, "ccm_cko3_root",
	                           root_range(0x1d80),
	                           root_parents[Root_parent::CKO1_SEL] };

	Root_clock ccm_cko4_root { clocks, "ccm_cko4_root",
	                           root_range(0x1e00),
	                           root_parents[Root_parent::CKO2_SEL] };


	Root_clock hsio_clk_root { clocks, "hsio_root",
	                           root_range(0x1e80),
	                           root_parents[Root_parent::LOW_SPEED_IO_SEL],
	                           ALWAYS_ON };

	Root_clock hsio_usb_test_60m_root { clocks, "hsio_usb_test_60m_root",
	                                    root_range(0x1f00),
	                                    root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock hsio_acscan_80m_root { clocks, "hsio_acscan_80m_root",
	                                  root_range(0x1f80),
	                                  root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock hsio_acscan_480m_root { clocks, "hsio_acscan_480m_root",
	                                   root_range(0x2000),
	                                   root_parents[Root_parent::MISC_SEL] };

	Root_clock nic_axi_clk_root { clocks, "nic_axi_root",
	                              root_range(0x2080),
	                              root_parents[Root_parent::FAST_SEL],
	                              ALWAYS_ON };

	Root_clock ml_apb_root { clocks, "ml_apb_root",
	                         root_range(0x2180),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock ml_root { clocks, "ml_root",
	                     root_range(0x2200),
	                     root_parents[Root_parent::FAST_SEL] };

	Root_clock media_axi_root { clocks, "media_axi_root",
	                            root_range(0x2280),
	                            root_parents[Root_parent::FAST_SEL] };

	Root_clock media_apb_root { clocks, "media_apb_root",
	                            root_range(0x2300),
	                            root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock media_ldb_root { clocks, "media_ldb_root",
	                            root_range(0x2380),
	                            root_parents[Root_parent::VIDEO_SEL] };

	Root_clock media_disp_pix_root { clocks, "media_disp_pix_root",
	                                 root_range(0x2400),
	                                 root_parents[Root_parent::VIDEO_SEL] };

	Root_clock cam_pix_root { clocks, "cam_pix_root",
	                          root_range(0x2480),
	                          root_parents[Root_parent::VIDEO_SEL] };

	Root_clock mipi_test_byte_root { clocks, "mipi_test_byte_root",
	                                 root_range(0x2500),
	                                 root_parents[Root_parent::VIDEO_SEL] };

	Root_clock mipi_phy_cfg_root { clocks, "mipi_phy_cfg_root",
	                               root_range(0x2580),
	                               root_parents[Root_parent::VIDEO_SEL] };

	Root_clock adc_root { clocks, "adc_root",
	                      root_range(0x2700),
	                      root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock pdm_root { clocks, "pdm_root",
	                      root_range(0x2780),
	                      root_parents[Root_parent::AUDIO_SEL] };

	Root_clock tstmr1_root { clocks, "tstmr1_root",
	                         root_range(0x2800),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock tstmr2_root { clocks, "tstmr2_root",
	                         root_range(0x2880),
	                         root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock mqs1_root { clocks, "mqs1_root",
	                       root_range(0x2900),
	                       root_parents[Root_parent::AUDIO_SEL] };

	Root_clock mqs2_root { clocks, "mqs2_root",
	                       root_range(0x2980),
	                       root_parents[Root_parent::AUDIO_SEL] };

	Root_clock audio_xcvr_root { clocks, "audio_xcvr_root",
	                             root_range(0x2a00),
	                             root_parents[Root_parent::NON_IO_SEL] };

	Root_clock spdif_root { clocks, "spdif_root",
	                        root_range(0x2a80),
	                        root_parents[Root_parent::AUDIO_SEL] };

	Root_clock enet_root { clocks, "enet_root",
	                       root_range(0x2b00),
	                       root_parents[Root_parent::NON_IO_SEL] };

	Root_clock enet_timer1_root { clocks, "enet_timer1_root",
	                              root_range(0x2b80),
	                              root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock enet_timer2_root { clocks, "enet_timer2_root",
	                              root_range(0x2c00),
	                              root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock enet_ref_root { clocks, "enet_ref_root",
	                           root_range(0x2c80),
	                           root_parents[Root_parent::NON_IO_SEL] };

	Root_clock enet_ref_phy_root { clocks, "enet_ref_phy_root",
	                               root_range(0x2d00),
	                               root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock i3c1_slow_root { clocks, "i3c1_slow_root",
	                            root_range(0x2d80),
	                            root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock i3c2_slow_root { clocks, "i3c2_slow_root",
	                            root_range(0x2e00),
	                            root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock usb_phy_root { clocks, "usb_phy_root",
	                          root_range(0x2e80),
	                          root_parents[Root_parent::LOW_SPEED_IO_SEL] };

	Root_clock pal_came_scan_root { clocks, "pal_came_scan_root",
	                                root_range(0x2f00),
	                                root_parents[Root_parent::MISC_SEL] };

// TODO:
//	{ A55_GATE,		"a55_alt",	"a55_alt_root",		0x8000, },
//	/* M33 critical clk for system run */
//	{ CM33_GATE,		"cm33",		"m33_root",		0x8040, CLK_IS_CRITICAL },
//	{ ADC1_GATE,		"adc1",		"adc_root",		0x82c0, },
//	{ WDOG1_GATE,		"wdog1",	"osc_24m",		0x8300, },
//	{ WDOG2_GATE,		"wdog2",	"osc_24m",		0x8340, },
//	{ WDOG3_GATE,		"wdog3",	"osc_24m",		0x8380, },
//	{ WDOG4_GATE,		"wdog4",	"osc_24m",		0x83c0, },
//	{ WDOG5_GATE,		"wdog5",	"osc_24m",		0x8400, },
//	{ SEMA1_GATE,		"sema1",	"bus_aon_root",		0x8440, },
//	{ SEMA2_GATE,		"sema2",	"bus_wakeup_root",	0x8480, },
//	{ MU1_A_GATE,		"mu1_a",	"bus_aon_root",		0x84c0, CLK_IGNORE_UNUSED },
//	{ MU2_A_GATE,		"mu2_a",	"bus_wakeup_root",	0x84c0, CLK_IGNORE_UNUSED },
//	{ MU1_B_GATE,		"mu1_b",	"bus_aon_root",		0x8500, 0, &share_count_mub },
//	{ MU2_B_GATE,		"mu2_b",	"bus_wakeup_root",	0x8500, 0, &share_count_mub },
//	{ EDMA1_GATE,		"edma1",	"m33_root",		0x8540, },
//	{ EDMA2_GATE,		"edma2",	"wakeup_axi_root",	0x8580, },
//	{ FLEXSPI1_GATE,	"flexspi1",	"flexspi1_root",	0x8640, },
//	{ GPIO1_GATE,		"gpio1",	"m33_root",		0x8880, },
//	{ GPIO2_GATE,		"gpio2",	"bus_wakeup_root",	0x88c0, },
//	{ GPIO3_GATE,		"gpio3",	"bus_wakeup_root",	0x8900, },
//	{ GPIO4_GATE,		"gpio4",	"bus_wakeup_root",	0x8940, },
//	{ FLEXIO1_GATE,	"flexio1",	"flexio1_root",		0x8980, },
//	{ FLEXIO2_GATE,	"flexio2",	"flexio2_root",		0x89c0, },
//	{ LPIT1_GATE,		"lpit1",	"bus_aon_root",		0x8a00, },
//	{ LPIT2_GATE,		"lpit2",	"bus_wakeup_root",	0x8a40, },
//	{ LPTMR1_GATE,	"lptmr1",	"lptmr1_root",		0x8a80, },
//	{ LPTMR2_GATE,	"lptmr2",	"lptmr2_root",		0x8ac0, },
//	{ TPM1_GATE,		"tpm1",		"bus_aon_root",		0x8b00, },
//	{ TPM2_GATE,		"tpm2",		"tpm2_root",		0x8b40, },
//	{ TPM3_GATE,		"tpm3",		"bus_wakeup_root",	0x8b80, },
//	{ TPM4_GATE,		"tpm4",		"tpm4_root",		0x8bc0, },
//	{ TPM5_GATE,		"tpm5",		"tpm5_root",		0x8c00, },
//	{ TPM6_GATE,		"tpm6",		"tpm6_root",		0x8c40, },
//	{ CAN1_GATE,		"can1",		"can1_root",		0x8c80, },
//	{ CAN2_GATE,		"can2",		"can2_root",		0x8cc0, },
//	{ LPUART1_GATE,	"lpuart1",	"lpuart1_root",		0x8d00, },
//	{ LPUART2_GATE,	"lpuart2",	"lpuart2_root",		0x8d40, },
//	{ LPUART3_GATE,	"lpuart3",	"lpuart3_root",		0x8d80, },
//	{ LPUART4_GATE,	"lpuart4",	"lpuart4_root",		0x8dc0, },
//	{ LPUART5_GATE,	"lpuart5",	"lpuart5_root",		0x8e00, },
//	{ LPUART6_GATE,	"lpuart6",	"lpuart6_root",		0x8e40, },
//	{ LPUART7_GATE,	"lpuart7",	"lpuart7_root",		0x8e80, },
//	{ LPUART8_GATE,	"lpuart8",	"lpuart8_root",		0x8ec0, },
//	{ LPI2C1_GATE,	"lpi2c1",	"lpi2c1_root",		0x8f00, },
//	{ LPI2C2_GATE,	"lpi2c2",	"lpi2c2_root",		0x8f40, },
//	{ LPI2C3_GATE,	"lpi2c3",	"lpi2c3_root",		0x8f80, },
//	{ LPI2C4_GATE,	"lpi2c4",	"lpi2c4_root",		0x8fc0, },
//	{ LPI2C5_GATE,	"lpi2c5",	"lpi2c5_root",		0x9000, },
//	{ LPI2C6_GATE,	"lpi2c6",	"lpi2c6_root",		0x9040, },
//	{ LPI2C7_GATE,	"lpi2c7",	"lpi2c7_root",		0x9080, },
//	{ LPI2C8_GATE,	"lpi2c8",	"lpi2c8_root",		0x90c0, },
//	{ LPSPI1_GATE,	"lpspi1",	"lpspi1_root",		0x9100, },
//	{ LPSPI2_GATE,	"lpspi2",	"lpspi2_root",		0x9140, },
//	{ LPSPI3_GATE,	"lpspi3",	"lpspi3_root",		0x9180, },
//	{ LPSPI4_GATE,	"lpspi4",	"lpspi4_root",		0x91c0, },
//	{ LPSPI5_GATE,	"lpspi5",	"lpspi5_root",		0x9200, },
//	{ LPSPI6_GATE,	"lpspi6",	"lpspi6_root",		0x9240, },
//	{ LPSPI7_GATE,	"lpspi7",	"lpspi7_root",		0x9280, },
//	{ LPSPI8_GATE,	"lpspi8",	"lpspi8_root",		0x92c0, },
//	{ I3C1_GATE,		"i3c1",		"i3c1_root",		0x9300, },
//	{ I3C2_GATE,		"i3c2",		"i3c2_root",		0x9340, },
//	{ USDHC1_GATE,	"usdhc1",	"usdhc1_root",		0x9380, },
//	{ USDHC2_GATE,	"usdhc2",	"usdhc2_root",		0x93c0, },
//	{ USDHC3_GATE,	"usdhc3",	"usdhc3_root",		0x9400, },
//	{ SAI1_GATE,          "sai1",         "sai1_root",            0x9440, 0, &share_count_sai1},
//	{ SAI1_IPG,		"sai1_ipg_clk", "bus_aon_root",		0x9440, 0, &share_count_sai1},
//	{ SAI2_GATE,          "sai2",         "sai2_root",            0x9480, 0, &share_count_sai2},
//	{ SAI2_IPG,		"sai2_ipg_clk", "bus_wakeup_root",	0x9480, 0, &share_count_sai2},
//	{ SAI3_GATE,          "sai3",         "sai3_root",            0x94c0, 0, &share_count_sai3},
//	{ SAI3_IPG,		"sai3_ipg_clk", "bus_wakeup_root",	0x94c0, 0, &share_count_sai3},
//	{ MIPI_CSI_GATE,	"mipi_csi",	"media_apb_root",	0x9580, },
//	{ MIPI_DSI_GATE,	"mipi_dsi",	"media_apb_root",	0x95c0, },
//	{ LVDS_GATE,		"lvds",		"media_ldb_root",	0x9600, },
//	{ LCDIF_GATE,		"lcdif",	"media_apb_root",	0x9640, },
//	{ PXP_GATE,		"pxp",		"media_apb_root",	0x9680, },
//	{ ISI_GATE,		"isi",		"media_apb_root",	0x96c0, },
//	{ NIC_MEDIA_GATE,	"nic_media",	"media_axi_root",	0x9700, },
//	{ USB_CONTROLLER_GATE, "usb_controller", "hsio_root",		0x9a00, },
//	{ USB_TEST_60M_GATE,	"usb_test_60m",	"hsio_usb_test_60m_root", 0x9a40, },
//	{ HSIO_TROUT_24M_GATE, "hsio_trout_24m", "osc_24m",		0x9a80, },
//	{ PDM_GATE,		"pdm",		"pdm_root",		0x9ac0, 0, &share_count_pdm},
//	{ PDM_IPG,		"pdm_ipg_clk",	"bus_aon_root",		0x9ac0, 0, &share_count_pdm},
//	{ MQS1_GATE,		"mqs1",		"sai1_root",		0x9b00, },
//	{ MQS2_GATE,		"mqs2",		"sai3_root",		0x9b40, },
//	{ AUD_XCVR_GATE,	"aud_xcvr",	"audio_xcvr_root",	0x9b80, },
//	{ SPDIF_GATE,		"spdif",	"spdif_root",		0x9c00, },
//	{ HSIO_32K_GATE,	"hsio_32k",	"osc_32k",		0x9dc0, },
//	{ ENET1_GATE,		"enet1",	"wakeup_axi_root",	0x9e00, },
//	{ ENET_QOS_GATE,	"enet_qos",	"wakeup_axi_root",	0x9e40, },
//	/* Critical because clk accessed during CPU idle */
//	{ SYS_CNT_GATE,	"sys_cnt",	"osc_24m",		0x9e80, CLK_IS_CRITICAL},
//	{ TSTMR1_GATE,	"tstmr1",	"bus_aon_root",		0x9ec0, },
//	{ TSTMR2_GATE,	"tstmr2",	"bus_wakeup_root",	0x9f00, },
//	{ TMC_GATE,		"tmc",		"osc_24m",		0x9f40, },
//	{ PMRO_GATE,		"pmro",		"osc_24m",		0x9f80, }


};
