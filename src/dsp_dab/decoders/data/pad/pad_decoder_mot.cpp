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

#include "pad_decoder_mot.h"

#include "dsp_dab/tools.h"

// --- CMOTDecoder -----------------------------------------------------------------
CMOTDecoder::CMOTDecoder() : CDataGroup(16384)
{
  // = 2^14
  Reset();
}

void CMOTDecoder::Reset()
{
  CDataGroup::Reset();
  m_motManager.Reset();

  m_motLength = 0;
}

bool CMOTDecoder::DecodeDataGroup()
{
  // ignore too short lens
  if (m_motLength < CalcCRC::CRCLen)
    return false;

  // only DGs with CRC are supported here!

  // abort on invalid CRC
  if (!CheckCRC(m_motLength - CalcCRC::CRCLen))
  {
    CDataGroup::Reset();
    return false;
  }

  CDataGroup::Reset();

  return m_motManager.HandleMOTDataGroup(m_dgRaw.data(), m_motLength);
}

std::vector<uint8_t> CMOTDecoder::GetMOTDataGroup()
{
  std::vector<uint8_t> result(m_motLength);
  memcpy(&result[0], &m_dgRaw[0], m_motLength);
  return result;
}
