/*
 *  Copyright (C) 2024 Team Kodi (https://kodi.tv)
 *  Copyright (c) 2020-2022 Michael G. Brehm
 *  Copyright (C) 2015 Jan van Katwijk (J.vanKatwijk@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

/*
 * Adapted from welle.io
 * https://github.com/AlbrechtL/welle.io/
 *
 * src/backend/charsets.h
 */

#pragma once

#include <cstdint>
#include <string>

namespace charsets
{

/**
 * @brief Codes assigned to character sets, as defined
 * in ETSI TS 101 756 v1.6.1, section 5.2.
 */
enum class CharacterSet : uint8_t
{
  EbuLatin = 0x00, // Complete EBU Latin based repertoire - see annex C
  UnicodeUcs2 = 0x06,
  UnicodeUtf8 = 0x0F,
  Undefined,
};

/**
 * @brief Converts the string from the given charset to a UTF-8
 * encoded string.
 *
 * @param[in] buffer Data of string to convert
 * @param[in] charset With @ref CharacterSet selected character type
 * @param[in] num_bytes [opt] Size of @ref buffer, can be set as 0 too
 *                      @note If num_bytes is nonzero, the @ref buffer must be zero
 *                      terminated.
 * @return The to UTF8 converted character string
 */
std::string toUtf8(const void* buffer, CharacterSet charset, size_t num_bytes = 0);

} // namespace charsets
