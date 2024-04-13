/*
 *  Copyright (C) 2024 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

/*
 * Adapted partly from welle.io
 * https://github.com/AlbrechtL/welle.io/
 *
 * src/backend/pad_decoder.cpp
 */

#include "pad_decoder_datagroup.h"

#include "dsp_dab/tools.h"

// --- CDataGroup -----------------------------------------------------------------
CDataGroup::CDataGroup(size_t dg_size_max)
{
  m_dgRaw.resize(dg_size_max);
  Reset();
}

void CDataGroup::Reset()
{
  m_dgSize = 0;
  m_dgSizeNeeded = GetInitialNeededSize();
}

bool CDataGroup::ProcessDataSubfield(bool start, const uint8_t* data, size_t len)
{
  if (start)
  {
    Reset();
  }
  else
  {
    // ignore Data Group continuation without previous start
    if (m_dgSize == 0)
      return false;
  }

  // abort, if needed size already reached (except needed size not yet set)
  if (m_dgSize >= m_dgSizeNeeded)
    return false;

  // abort, if maximum size already reached
  if (m_dgSize == m_dgRaw.size())
    return false;

  // append Data Subfield
  size_t copy_len = m_dgRaw.size() - m_dgSize;
  if (len < copy_len)
    copy_len = len;
  memcpy(&m_dgRaw[m_dgSize], data, copy_len);
  m_dgSize += copy_len;

  // abort, if needed size not yet reached
  if (m_dgSize < m_dgSizeNeeded)
    return false;

  return DecodeDataGroup();
}

bool CDataGroup::EnsureDataGroupSize(size_t desired_dg_size)
{
  m_dgSizeNeeded = desired_dg_size;
  return m_dgSize >= m_dgSizeNeeded;
}

bool CDataGroup::CheckCRC(size_t len)
{
  // ensure needed size reached
  if (m_dgSize < len + CalcCRC::CRCLen)
    return false;

  uint16_t crc_stored = m_dgRaw[len] << 8 | m_dgRaw[len + 1];
  uint16_t crc_calced = CalcCRC::CalcCRC_CRC16_CCITT.Calc(&m_dgRaw[0], len);
  //fprintf(stderr, "-----------------> %i %i %X %X - %i\n", m_dgSize, len, crc_stored, crc_calced, crc_stored == crc_calced);
  return crc_stored == crc_calced;
}
