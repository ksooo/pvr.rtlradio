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

#include "pad_decoder_dynamiclabelsegment.h"

#include "tools.h"

// --- DL_SEG -----------------------------------------------------------------
struct DL_SEG
{
  uint8_t prefix[2];
  std::vector<uint8_t> chars;

  bool Toggle() const { return prefix[0] & 0x80; }
  bool First() const { return prefix[0] & 0x40; }
  bool Last() const { return prefix[0] & 0x20; }
  int SegNum() const { return First() ? 0 : ((prefix[1] & 0x70) >> 4); }
};

typedef std::map<int, DL_SEG> dl_segs_t;


// --- DL_SEG_REASSEMBLER -----------------------------------------------------------------
struct DL_SEG_REASSEMBLER
{
  bool AddSegment(DL_SEG& dl_seg);
  bool CheckForCompleteLabel();
  void Reset();

  dl_segs_t dl_segs;
  std::vector<uint8_t> label_raw;
};

void DL_SEG_REASSEMBLER::Reset()
{
  dl_segs.clear();
  label_raw.clear();
}

bool DL_SEG_REASSEMBLER::AddSegment(DL_SEG& dl_seg)
{
  dl_segs_t::const_iterator it;

  // if there are already segments with other toggle value in cache, first clear it
  it = dl_segs.cbegin();
  if (it != dl_segs.cend() && it->second.Toggle() != dl_seg.Toggle())
    dl_segs.clear();

  // if the segment is already there, abort
  it = dl_segs.find(dl_seg.SegNum());
  if (it != dl_segs.cend())
    return false;

  // add segment
  dl_segs[dl_seg.SegNum()] = dl_seg;

  // check for complete m_label
  return CheckForCompleteLabel();
}

bool DL_SEG_REASSEMBLER::CheckForCompleteLabel()
{
  dl_segs_t::const_iterator it;

  // check if all segments are in cache
  int segs = 0;
  for (int i = 0; i < 8; i++)
  {
    it = dl_segs.find(i);
    if (it == dl_segs.cend())
      return false;

    segs++;

    if (it->second.Last())
      break;

    if (i == 7)
      return false;
  }

  // append complete m_label
  label_raw.clear();
  for (int i = 0; i < segs; i++)
    label_raw.insert(label_raw.end(), dl_segs[i].chars.begin(), dl_segs[i].chars.end());

  //	std::string m_label((const char*) &label_raw[0], label_raw.size());
  //	fprintf(stderr, "DL_SEG_REASSEMBLER: new m_label: '%s'\n", m_label.c_str());
  return true;
}


// --- CDynamicLabelDecoder -----------------------------------------------------------------

CDynamicLabelDecoder::CDynamicLabelDecoder() : CDataGroup(2 + 16 + CalcCRC::CRCLen)
{
  m_dl_sr = std::make_unique<DL_SEG_REASSEMBLER>();
  Reset();
}

CDynamicLabelDecoder::~CDynamicLabelDecoder() = default;

void CDynamicLabelDecoder::Reset()
{
  CDataGroup::Reset();

  m_dl_sr->Reset();
  m_label.Reset();
}

size_t CDynamicLabelDecoder::GetInitialNeededSize()
{
  // at least prefix + CRC
  return 2 + CalcCRC::CRCLen;
}

bool CDynamicLabelDecoder::DecodeDataGroup()
{
  bool command = m_dgRaw[0] & 0x10;

  size_t field_len = 0;
  bool cmd_remove_label = false;

  // handle command/segment
  if (command)
  {
    switch (m_dgRaw[0] & 0x0F)
    {
      case 0x01: // remove label
        cmd_remove_label = true;
        break;
      default:
        // ignore command
        CDataGroup::Reset();
        return false;
    }
  }
  else
  {
    field_len = (m_dgRaw[0] & 0x0F) + 1;
  }

  size_t real_len = 2 + field_len;

  if (!EnsureDataGroupSize(real_len + CalcCRC::CRCLen))
    return false;

  // abort on invalid CRC
  if (!CheckCRC(real_len))
  {
    CDataGroup::Reset();
    return false;
  }

  // on Remove Label command, display empty label
  if (cmd_remove_label)
  {
    m_label.Reset();
    return true;
  }

  // create new segment
  DL_SEG dl_seg;
  memcpy(dl_seg.prefix, &m_dgRaw[0], 2);
  dl_seg.chars.insert(dl_seg.chars.begin(), m_dgRaw.begin() + 2, m_dgRaw.begin() + 2 + field_len);

  CDataGroup::Reset();

  //	fprintf(stderr, "CDynamicLabelDecoder: segnum %d, toggle: %s, chars_len: %2d%s\n", dl_seg.SegNum(), dl_seg.Toggle() ? "Y" : "N", dl_seg.chars.size(), dl_seg.Last() ? " [LAST]" : "");

  // try to add segment
  if (!m_dl_sr->AddSegment(dl_seg))
    return false;

  // append new m_label
  m_label.raw = m_dl_sr->label_raw;
  m_label.charset = m_dl_sr->dl_segs[0].prefix[1] >> 4;
  return true;
}
