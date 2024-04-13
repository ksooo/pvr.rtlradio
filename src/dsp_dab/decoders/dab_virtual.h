/*
 *  Copyright (C) 2024 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

/*
 * Adapted from welle.io
 * https://github.com/AlbrechtL/welle.io/
 *
 * src/backend/dab_virtual.h
 */

#pragma once

#include "dsp_dab/dab-constants.h"

#include <cstdint>

#define CUSize  (4 * 16)

class DabVirtual
{
  public:
    virtual ~DabVirtual() = default;
    virtual int32_t process(const softbit_t *v, int16_t cnt) = 0;
};


