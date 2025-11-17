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

#include <ccm.h>

/*******************
 ** CCM interface **
 *******************/

Driver::Ccm::Ccm(Genode::Env &env,
                 Clocks      &clocks,
                 bool        verbose)
: env { env }, clocks { clocks }, verbose { verbose }
{
	/*
	 * Now we can safely disable clocks. If there are still root-clocks
	 * depending on it, ref-counting will protect us from disabling it
	 */
	audio_pll_clk.disable();
	video_pll_clk.disable();
}
