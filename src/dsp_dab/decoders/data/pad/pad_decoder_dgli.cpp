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

#include "pad_decoder_dgli.h"

#include "dsp_dab/tools.h"

// --- CDGLIDecoder -----------------------------------------------------------------
CDGLIDecoder::CDGLIDecoder() : CDataGroup(2 + CalcCRC::CRCLen)
{
}

void CDGLIDecoder::Reset()
{
  CDataGroup::Reset();

  m_dgliLength = 0;
}

size_t CDGLIDecoder::GetInitialNeededSize()
{
  // DG len + CRC
  return 2 + CalcCRC::CRCLen;
}

bool CDGLIDecoder::DecodeDataGroup()
{
  // abort on invalid CRC
  if (!CheckCRC(2))
  {
    CDataGroup::Reset();
    return false;
  }

  m_dgliLength = (m_dgRaw[0] & 0x3F) << 8 | m_dgRaw[1];

  CDataGroup::Reset();

  //	fprintf(stderr, "CDGLIDecoder: m_dgliLength: %5zu\n", m_dgliLength);

  return true;
}

size_t CDGLIDecoder::GetDGLILen()
{
  size_t result = m_dgliLength;
  m_dgliLength = 0;
  return result;
}
