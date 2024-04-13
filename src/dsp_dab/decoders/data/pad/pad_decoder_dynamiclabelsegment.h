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
 * src/backend/pad_decoder.h
 */

#pragma once

#include "pad_decoder_datagroup.h"
#include "utils/charsets.h"

#include <array>
#include <memory>
#include <unordered_map>

enum class DLPlusCategory : uint8_t
{
  Dummy = 0,
  Item,
  Info,
  Programme,
  Interactivity,
  rfu, // Reserved for future use
  PrivateClasses,
  Descriptor
};

enum class DLPlusContentType : uint8_t
{
  DUMMY = 0,
  ITEM_TITLE = 1,
  ITEM_ALBUM = 2,
  ITEM_TRACKNUMBER = 3,
  ITEM_ARTIST = 4,
  ITEM_COMPOSITION = 5,
  ITEM_MOVEMENT = 6,
  ITEM_CONDUCTOR = 7,
  ITEM_COMPOSER = 8,
  ITEM_BAND = 9,
  ITEM_COMMENT = 10,
  ITEM_GENRE = 11,
  INFO_NEWS = 12,
  INFO_NEWS_LOCAL = 13,
  INFO_STOCKMARKET = 14,
  INFO_SPORT = 15,
  INFO_LOTTERY = 16,
  INFO_HOROSCOPE = 17,
  INFO_DAILY_DIVERSION = 18,
  INFO_HEALTH = 19,
  INFO_EVENT = 20,
  INFO_SCENE = 21,
  INFO_CINEMA = 22,
  INFO_TV = 23,
  INFO_DATE_TIME = 24,
  INFO_WEATHER = 25,
  INFO_TRAFFIC = 26,
  INFO_ALARM = 27,
  INFO_ADVERTISEMENT = 28,
  INFO_URL = 29,
  INFO_OTHER = 30,
  STATIONNAME_SHORT = 31,
  STATIONNAME_LONG = 32,
  PROGRAMME_NOW = 33,
  PROGRAMME_NEXT = 34,
  PROGRAMME_PART = 35,
  PROGRAMME_HOST = 36,
  PROGRAMME_EDITORIAL_STAFF = 37,
  PROGRAMME_FREQUENCY = 38,
  PROGRAMME_HOMEPAGE = 39,
  PROGRAMME_SUBCHANNEL = 40,
  PHONE_HOTLINE = 41,
  PHONE_STUDIO = 42,
  PHONE_OTHER = 43,
  SMS_STUDIO = 44,
  SMS_OTHER = 45,
  EMAIL_HOTLINE = 46,
  EMAIL_STUDIO = 47,
  EMAIL_OTHER = 48,
  MMS_OTHER = 49,
  CHAT = 50,
  CHAT_CENTER = 51,
  VOTE_QUESTION = 52,
  VOTE_CENTRE = 53,
  DESCRIPTOR_PLACE = 59,
  DESCRIPTOR_APPOINTMENT = 60,
  DESCRIPTOR_IDENTIFIER = 61,
  DESCRIPTOR_PURCHASE = 62,
  DESCRIPTOR_GET_DATA = 63
};

// --- CDynamicLabel -----------------------------------------------------------------
class CDynamicLabel
{
public:
  CDynamicLabel() = default;

  void Reset()
  {
    raw.clear();
    charset = charsets::CharacterSet::Undefined;
  }

  std::vector<uint8_t> raw;
  charsets::CharacterSet charset{charsets::CharacterSet::Undefined}; // -1?????

  std::string m_dynamicLabel;
  std::unordered_map<uint8_t, std::string> m_playItems;
  std::unordered_map<uint8_t, std::string> m_infoItems;
  std::unordered_map<uint8_t, std::string> m_programmeItems;
  std::unordered_map<uint8_t, std::string> m_interactivityItems;
  std::unordered_map<uint8_t, std::string> m_descriptorItems;

  struct DLPlusContentTypeInfo
  {
    uint16_t code;
    DLPlusCategory category;
    const char* id3v2;
    const char* name;
  };

  static std::array<DLPlusContentTypeInfo, 64> DLPlusContentTypes;
};

struct DL_SEG_REASSEMBLER;

// --- CDynamicLabelDecoder -----------------------------------------------------------------
class CDynamicLabelDecoder : public CDataGroup
{
public:
  CDynamicLabelDecoder();
  ~CDynamicLabelDecoder();

  void Reset();

  CDynamicLabel GetLabel() { return m_label; }

private:
  size_t GetInitialNeededSize() override;
  bool DecodeDataGroup() override;

  bool ProcessDynLabelCommand_ClearDisplay();
  bool ProcessDynLabelCommand_DLPlusCommand(uint8_t firstLast);
  bool CheckDataPacket(size_t field_len);

  std::unique_ptr<DL_SEG_REASSEMBLER> m_dl_sr;
  CDynamicLabel m_label;
  bool m_programmeItemToogle{false};
};
