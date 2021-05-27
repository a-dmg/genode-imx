/**
 * \brief  Lx_emul support for I/O memory
 * \author Stefan Kalkowski
 * \date   2021-04-17
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void * lx_emul_io_mem_map(unsigned long phys_addr, unsigned long size);

#ifdef __cplusplus
}
#endif
