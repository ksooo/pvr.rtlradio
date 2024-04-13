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

#include "dsp_dab/tools.h"

#include <kodi/tools/StringUtils.h>

typedef enum DynamicLabelFirstLast : uint8_t
{
  DynLabelFirstLast_IntermediateSegment = 0b00,
  DynLabelFirstLast_LastSegment = 0b01,
  DynLabelFirstLast_FirstSegment = 0b10,
  DynLabelFirstLast_OneAndOnlySegment = 0b11
} DynamicLabelFirstLast;

typedef enum DynamicLabelCommand : uint8_t
{
  DynLabelCommand_ClearDisplay = 0b01,
  DynLabelCommand_DLPlusCommand = 0b10,
} DynamicLabelCommand;

enum class CommandId : uint8_t
{
  DLPlusTags = 0x0
};

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

/*
 * @note ID3V2 id's starting with "TK" are means as "text kodi" and only between Kodi and add-on used!
 */
std::array<CDynamicLabel::DLPlusContentTypeInfo, 64> CDynamicLabel::DLPlusContentTypes = {{
    {0, DLPlusCategory::Dummy, "", "Dummy"},
    {1, DLPlusCategory::Item, "TIT2", "ITEM.TITLE"},
    {2, DLPlusCategory::Item, "TALB", "ITEM.ALBUM"},
    {3, DLPlusCategory::Item, "TRCK", "ITEM.TRACKNUMBER"},
    {4, DLPlusCategory::Item, "TPE1", "ITEM.ARTIST"},
    {5, DLPlusCategory::Item, "TIT1", "ITEM.COMPOSITION"},
    {6, DLPlusCategory::Item, "TIT3", "ITEM.MOVEMENT"},
    {7, DLPlusCategory::Item, "TPE3", "ITEM.CONDUCTOR"},
    {8, DLPlusCategory::Item, "TCOM", "ITEM.COMPOSER"},
    {9, DLPlusCategory::Item, "TPE2", "ITEM.BAND"},
    {10, DLPlusCategory::Item, "COMM", "ITEM.COMMENT"},
    {11, DLPlusCategory::Item, "TCON", "ITEM.GENRE"},
    {12, DLPlusCategory::Info, "", "INFO.NEWS"},
    {13, DLPlusCategory::Info, "", "INFO.NEWS.LOCAL"},
    {14, DLPlusCategory::Info, "", "INFO.STOCKMARKET"},
    {15, DLPlusCategory::Info, "", "INFO.SPORT"},
    {16, DLPlusCategory::Info, "", "INFO.LOTTERY"},
    {17, DLPlusCategory::Info, "", "INFO.HOROSCOPE"},
    {18, DLPlusCategory::Info, "", "INFO.DAILY_DIVERSION"},
    {19, DLPlusCategory::Info, "", "INFO.HEALTH"},
    {20, DLPlusCategory::Info, "", "INFO.EVENT"},
    {21, DLPlusCategory::Info, "", "INFO.SCENE"},
    {22, DLPlusCategory::Info, "", "INFO.CINEMA"},
    {23, DLPlusCategory::Info, "", "INFO.TV"},
    {24, DLPlusCategory::Info, "", "INFO.DATE_TIME"},
    {25, DLPlusCategory::Info, "", "INFO.WEATHER"},
    {26, DLPlusCategory::Info, "", "INFO.TRAFFIC"},
    {27, DLPlusCategory::Info, "", "INFO.ALARM"},
    {28, DLPlusCategory::Info, "", "INFO.ADVERTISEMENT"},
    {29, DLPlusCategory::Info, "", "INFO.URL URL"},
    {30, DLPlusCategory::Info, "", "INFO.OTHER"},
    {31, DLPlusCategory::Programme, "", "STATIONNAME.SHORT"},
    {32, DLPlusCategory::Programme, "", "STATIONNAME.LONG"},
    {33, DLPlusCategory::Programme, "TKNO", "PROGRAMME.NOW"}, /* own id3v2 id */
    {34, DLPlusCategory::Programme, "TKNE", "PROGRAMME.NEXT"}, /* own id3v2 id */
    {35, DLPlusCategory::Programme, "", "PROGRAMME.PART"},
    {36, DLPlusCategory::Programme, "", "PROGRAMME.HOST"},
    {37, DLPlusCategory::Programme, "", "PROGRAMME.EDITORIAL_STAFF"},
    {38, DLPlusCategory::Programme, "", "PROGRAMME.FREQUENCY"},
    {39, DLPlusCategory::Programme, "WORS", "PROGRAMME.HOMEPAGE"},
    {40, DLPlusCategory::Programme, "", "PROGRAMME.SUBCHANNEL"},
    {41, DLPlusCategory::Interactivity, "", "PHONE.HOTLINE"},
    {42, DLPlusCategory::Interactivity, "", "PHONE.STUDIO"},
    {43, DLPlusCategory::Interactivity, "", "PHONE.OTHER"},
    {44, DLPlusCategory::Interactivity, "", "SMS.STUDIO"},
    {45, DLPlusCategory::Interactivity, "", "SMS.OTHER"},
    {46, DLPlusCategory::Interactivity, "", "EMAIL.HOTLINE"},
    {47, DLPlusCategory::Interactivity, "", "EMAIL.STUDIO"},
    {48, DLPlusCategory::Interactivity, "", "EMAIL.OTHER"},
    {49, DLPlusCategory::Interactivity, "", "MMS.OTHER"},
    {50, DLPlusCategory::Interactivity, "", "CHAT"},
    {51, DLPlusCategory::Interactivity, "", "CHAT.CENTER"},
    {52, DLPlusCategory::Interactivity, "", "VOTE.QUESTION"},
    {53, DLPlusCategory::Interactivity, "", "VOTE.CENTRE"},
    {54, DLPlusCategory::rfu, "", ""},
    {55, DLPlusCategory::rfu, "", ""},
    {56, DLPlusCategory::PrivateClasses, "", ""},
    {57, DLPlusCategory::PrivateClasses, "", ""},
    {58, DLPlusCategory::PrivateClasses, "", ""},
    {59, DLPlusCategory::Descriptor, "", "DESCRIPTOR.PLACE"},
    {60, DLPlusCategory::Descriptor, "", "DESCRIPTOR.APPOINTMENT"},
    {61, DLPlusCategory::Descriptor, "TSRC", "DESCRIPTOR.IDENTIFIER"},
    {62, DLPlusCategory::Descriptor, "WPAY", "DESCRIPTOR.PURCHASE"},
    {63, DLPlusCategory::Descriptor, "", "DESCRIPTOR.GET_DATA"},
}};


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
  /*
   * See ETSI EN 300 401 V2.1.1 (2017-01) clause 7.4.5.2
   */
  const uint16_t dataGroupStart = (m_dgRaw[0] << 8) | m_dgRaw[1];

  // First 4 bits about dynamic label data group
  const bool toggleBit = dataGroupStart & (1 << 15);
  const uint8_t firstLast = (dataGroupStart >> 13) & 0b11;
  const bool cFlag = dataGroupStart & (1 << 12);

  // handle command/segment
  if (cFlag)
  {
    const uint8_t command = (dataGroupStart >> 8) & 0xF;
    switch (command)
    {
      case DynLabelCommand_ClearDisplay: // remove label
        return ProcessDynLabelCommand_ClearDisplay();

      case DynLabelCommand_DLPlusCommand:
        return ProcessDynLabelCommand_DLPlusCommand(firstLast);

      default:
        // ignore command
        CDataGroup::Reset();
        return false;
    }
  }

  const size_t field_len = (m_dgRaw[0] & 0x0F) + 1;

  // abort on invalid data
  if (!CheckDataPacket(field_len))
    return false;

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
  m_label.charset = static_cast<charsets::CharacterSet>((dataGroupStart >> 4) & 0xF);

  m_label.m_dynamicLabel = charsets::toUtf8(&m_label.raw[0], m_label.charset, m_label.raw.size());
  return true;
}

bool CDynamicLabelDecoder::CheckDataPacket(size_t field_len)
{
  size_t real_len = 2 + field_len;

  if (!EnsureDataGroupSize(real_len + CalcCRC::CRCLen))
    return false;

  // abort on invalid CRC
  if (!CheckCRC(real_len))
  {
    CDataGroup::Reset();
    return false;
  }

  return true;
}

bool CDynamicLabelDecoder::ProcessDynLabelCommand_ClearDisplay()
{
  if (!CheckDataPacket(0))
    return false;

  m_label.Reset();
  return true;
}

bool CDynamicLabelDecoder::ProcessDynLabelCommand_DLPlusCommand(uint8_t firstLast)
{
  const bool linkMessage = m_dgRaw[1] & (1 << 7);
  const uint8_t segNo = (firstLast & DynLabelFirstLast_FirstSegment) ? 0 : (m_dgRaw[1] >> 4) & 0x7;
  const CommandId CId = static_cast<CommandId>(m_dgRaw[2] >> 4);

  bool updated = false;

  if (CId == CommandId::DLPlusTags)
  {
    const bool itemToggleBit = m_dgRaw[2] & 0b1000;
    const bool itemRunningBit = m_dgRaw[2] & 0b0100;
    const uint8_t numberOfTags = (m_dgRaw[2] & 0b0011) + 1;

    if (!CheckDataPacket(numberOfTags * 3 + 1))
      return false;

    if (!itemRunningBit && itemToggleBit != m_programmeItemToogle)
    {
      m_label.m_infoItems.clear();
      m_label.m_programmeItems.clear();
      m_label.m_interactivityItems.clear();
      m_label.m_descriptorItems.clear();
      updated = true;
    }
    m_programmeItemToogle = itemToggleBit;
    if (!itemRunningBit && !m_label.m_playItems.empty())
    {
      m_label.m_playItems.clear();
      updated = true;
    }

#if 0
    fprintf(
      stderr,
      "CDynamicLabelDecoder: dlPlus itemToggleBit: %s, itemRunningBit: %s, numberOfTags: %i\n",
      itemToggleBit ? "yes" : "no ", itemRunningBit ? "yes" : "no ", numberOfTags);
#endif

    for (uint8_t i = 0; i < numberOfTags * 3; i += 3)
    {
      const uint8_t contentType = m_dgRaw[3 + i + 0] & 0x7F;
      const uint8_t startMarker = m_dgRaw[3 + i + 1] & 0x7F;
      const uint8_t lengthMarker = (m_dgRaw[3 + i + 2] & 0x7F);

      if (startMarker + lengthMarker + 1 > m_label.raw.size())
        continue;

      std::string text;
      if (lengthMarker > 0)
      {
        text = charsets::toUtf8(&m_label.raw[0] + startMarker, m_label.charset, lengthMarker + 1);
        kodi::tools::StringUtils::Trim(text);
      }

      switch (CDynamicLabel::DLPlusContentTypes[contentType].category)
      {
        case DLPlusCategory::Item:
          if (static_cast<DLPlusContentType>(contentType) == DLPlusContentType::ITEM_TITLE &&
              text.empty() && !m_label.m_playItems.empty())
          {
            m_label.m_playItems.clear();
            updated = true;
          }
          else if (!text.empty())
          {
            if (m_label.m_playItems.find(contentType) == m_label.m_playItems.end() ||
                m_label.m_playItems[contentType] != text)
            {
              m_label.m_playItems[contentType] = text;
              updated = true;
            }
          }
          else if (m_label.m_playItems.find(contentType) != m_label.m_playItems.end())
          {
            m_label.m_playItems.erase(contentType);
            updated = true;
          }

          break;

        case DLPlusCategory::Info:
          if (!text.empty())
          {
            if (m_label.m_infoItems.find(contentType) == m_label.m_infoItems.end() ||
                m_label.m_infoItems[contentType] != text)
            {
              m_label.m_infoItems[contentType] = text;
              updated = true;
            }
          }
          else if (m_label.m_infoItems.find(contentType) != m_label.m_infoItems.end())
          {
            m_label.m_infoItems.erase(contentType);
            updated = true;
          }

          break;

        case DLPlusCategory::Programme:
          if (!text.empty())
          {
            if (m_label.m_programmeItems.find(contentType) == m_label.m_programmeItems.end() ||
                m_label.m_programmeItems[contentType] != text)
            {
              m_label.m_programmeItems[contentType] = text;
              updated = true;
            }
          }
          else if (!itemRunningBit &&
                   m_label.m_programmeItems.find(contentType) != m_label.m_programmeItems.end())
          {
            m_label.m_programmeItems.erase(contentType);
            updated = true;
          }

          break;

        case DLPlusCategory::Interactivity:
          if (!text.empty())
          {
            if (m_label.m_interactivityItems.find(contentType) ==
                    m_label.m_interactivityItems.end() ||
                m_label.m_interactivityItems[contentType] != text)
            {
              m_label.m_interactivityItems[contentType] = text;
              updated = true;
            }
          }
          else if (m_label.m_interactivityItems.find(contentType) !=
                   m_label.m_interactivityItems.end())
          {
            m_label.m_interactivityItems.erase(contentType);
            updated = true;
          }

          break;

        case DLPlusCategory::Descriptor:
          if (!text.empty())
          {
            if (m_label.m_descriptorItems.find(contentType) == m_label.m_descriptorItems.end() ||
                m_label.m_descriptorItems[contentType] != text)
            {
              m_label.m_descriptorItems[contentType] = text;
              updated = true;
            }
          }
          else if (m_label.m_descriptorItems.find(contentType) != m_label.m_descriptorItems.end())
          {
            m_label.m_descriptorItems.erase(contentType);
            updated = true;
          }

          break;

        default:
          break;
      }
    }
  }
#if 1
  else
  {
    fprintf(stderr, "CDynamicLabelDecoder: Unsupported DLPlus CId 0x%X\n", CId);
  }
#endif

  CDataGroup::Reset();

  return updated;
}
