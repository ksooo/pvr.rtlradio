/*
    DABlin - capital DAB experience
    Copyright (C) 2015-2019 Stefan Pöschel

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pad_decoder.h"

// --- XPAD_CI -----------------------------------------------------------------
const size_t XPAD_CI::lens[] = {4, 6, 8, 12, 16, 24, 32, 48};

// --- CPADDecoder -----------------------------------------------------------------
void CPADDecoder::Reset()
{
  m_MOTAppType = -1;

  m_lastXPAD_CI.Reset();

  m_dlDecoder.Reset();
  m_DGLIDecoder.Reset();
  m_MOTDecoder.Reset();
  m_MOTManager.Reset();
}

void CPADDecoder::Process(const uint8_t* xpad_data,
                         size_t xpad_len,
                         bool exact_xpad_len,
                         const uint8_t* fpad_data)
{
  // undo reversed byte order + trim long MP2 frames
  size_t used_xpad_len = std::min(xpad_len, sizeof(m_xpad));
  for (size_t i = 0; i < used_xpad_len; i++)
    m_xpad[i] = xpad_data[xpad_len - 1 - i];

  xpad_cis_t xpad_cis;
  size_t xpad_cis_len = -1;

  int fpad_type = fpad_data[0] >> 6;
  int xpad_ind = (fpad_data[0] & 0x30) >> 4;
  bool ci_flag = fpad_data[1] & 0x02;

  XPAD_CI prev_xpad_ci = m_lastXPAD_CI;
  m_lastXPAD_CI.Reset();

  // build CI list
  if (fpad_type == 0b00)
  {
    if (ci_flag)
    {
      switch (xpad_ind)
      {
        case 0b01:
        { // short X-PAD
          if (xpad_len < 1)
            return;

          int type = m_xpad[0] & 0x1F;

          // skip end marker
          if (type != 0x00)
          {
            xpad_cis_len = 1;
            xpad_cis.emplace_back(3, type);
          }
          break;
        }
        case 0b10: // variable size X-PAD
          xpad_cis_len = 0;
          for (size_t i = 0; i < 4; i++)
          {
            if (xpad_len < i + 1)
              return;

            uint8_t ci_raw = m_xpad[i];
            xpad_cis_len++;

            // break on end marker
            if ((ci_raw & 0x1F) == 0x00)
              break;

            xpad_cis.emplace_back(ci_raw);
          }
          break;
      }
    }
    else
    {
      switch (xpad_ind)
      {
        case 0b01: // short X-PAD
        case 0b10: // variable size X-PAD
          // if there is a previous CI, append it
          if (prev_xpad_ci.type != -1)
          {
            xpad_cis_len = 0;
            xpad_cis.push_back(prev_xpad_ci);
          }
          break;
      }
    }
  }

  //	fprintf(stderr, "CPADDecoder: -----\n");
  if (xpad_cis.empty())
  {
    /* The CI list may be omitted if the (last) subfield of the X-PAD of the
		 * previous frame/AU is continued (see §7.4.2.1f in ETSI EN 300 401).
		 * However there are PAD encoders which wrongly assume that "previous"
		 * only takes frames/AUs containing X-PAD into account.
		 * This non-compliant encoding can generously be addressed by still
		 * keeping the necessary CI info.
		 */
    if (m_loose)
      m_lastXPAD_CI = prev_xpad_ci;
    return;
  }

  size_t announced_xpad_len = xpad_cis_len;
  for (const XPAD_CI& xpad_ci : xpad_cis)
    announced_xpad_len += xpad_ci.len;

  // abort, if the announced X-PAD len exceeds the available one
  if (announced_xpad_len > xpad_len)
    return;

  if (exact_xpad_len && !m_loose && announced_xpad_len < xpad_len)
  {
    /* If the announced X-PAD len falls below the available one (which can
		 * only happen with DAB+), a decoder shall discard the X-PAD (see §5.4.3
		 * in ETSI TS 102 563).
		 * This behaviour can be disabled in order to process the X-PAD anyhow.
		 */
    m_observer->PADLengthError(announced_xpad_len, xpad_len);
    return;
  }

  // process CIs
  size_t xpad_offset = xpad_cis_len;
  int xpad_ci_type_continued = -1;
  for (const XPAD_CI& xpad_ci : xpad_cis)
  {
    // len only valid for the *immediate* next data group after the DGLI!
    size_t m_dgliLength = m_DGLIDecoder.GetDGLILen();

    // handle Data Subfield
    switch (xpad_ci.type)
    {
      case 1: // Data Group Length Indicator
        m_DGLIDecoder.ProcessDataSubfield(ci_flag, m_xpad + xpad_offset, xpad_ci.len);

        xpad_ci_type_continued = 1;
        break;

      case 2: // Dynamic Label segment (start)
      case 3: // Dynamic Label segment (continuation)
        // if new m_label available, append it
        if (m_dlDecoder.ProcessDataSubfield(xpad_ci.type == 2, m_xpad + xpad_offset, xpad_ci.len))
          m_observer->PADChangeDynamicLabel(m_dlDecoder.GetLabel());

        xpad_ci_type_continued = 3;
        break;

      default:
        // MOT, X-PAD data group (start/continuation)
        if (m_MOTAppType != -1 &&
            (xpad_ci.type == m_MOTAppType || xpad_ci.type == m_MOTAppType + 1))
        {
          bool start = xpad_ci.type == m_MOTAppType;

          if (start)
            m_MOTDecoder.SetLen(m_dgliLength);

          // if new Data Group available, append it
          if (m_MOTDecoder.ProcessDataSubfield(start, m_xpad + xpad_offset, xpad_ci.len))
          {
            // if new slide available, show it
            if (m_MOTManager.HandleMOTDataGroup(m_MOTDecoder.GetMOTDataGroup()))
            {
              const MOT_FILE new_slide = m_MOTManager.GetFile();

              // check file type
              bool show_slide = true;
              if (new_slide.content_type != MOT_FILE::CONTENT_TYPE_IMAGE)
                show_slide = false;
              switch (new_slide.content_sub_type)
              {
                case MOT_FILE::CONTENT_SUB_TYPE_JFIF:
                case MOT_FILE::CONTENT_SUB_TYPE_PNG:
                  break;
                default:
                  show_slide = false;
              }

              if (show_slide)
                m_observer->PADChangeSlide(new_slide);
            }
          }

          xpad_ci_type_continued = m_MOTAppType + 1;
        }

        break;
    }
    //		fprintf(stderr, "CPADDecoder: Data Subfield: type: %2d, len: %2zu\n", it->type, it->len);

    xpad_offset += xpad_ci.len;
  }

  // set last CI
  m_lastXPAD_CI.len = xpad_offset;
  m_lastXPAD_CI.type = xpad_ci_type_continued;
}


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
  return crc_stored == crc_calced;
}


// --- CDGLIDecoder -----------------------------------------------------------------
void CDGLIDecoder::Reset()
{
  CDataGroup::Reset();

  m_dgliLength = 0;
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


// --- CDynamicLabelDecoder -----------------------------------------------------------------
void CDynamicLabelDecoder::Reset()
{
  CDataGroup::Reset();

  m_dl_sr.Reset();
  m_label.Reset();
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
  if (!m_dl_sr.AddSegment(dl_seg))
    return false;

  // append new m_label
  m_label.raw = m_dl_sr.label_raw;
  m_label.charset = m_dl_sr.dl_segs[0].prefix[1] >> 4;
  return true;
}


// --- DL_SEG_REASSEMBLER -----------------------------------------------------------------
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


// --- CMOTDecoder -----------------------------------------------------------------
void CMOTDecoder::Reset()
{
  CDataGroup::Reset();

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

  //	fprintf(stderr, "CMOTDecoder: m_motLength: %5zu\n", m_motLength);

  return true;
}

std::vector<uint8_t> CMOTDecoder::GetMOTDataGroup()
{
  std::vector<uint8_t> result(m_motLength);
  memcpy(&result[0], &m_dgRaw[0], m_motLength);
  return result;
}
