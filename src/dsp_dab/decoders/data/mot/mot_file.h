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
  * src/backend/mot_manager.cpp
  */

#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

/*!
 * @brief Values for the MOT header parameters content type.
 *
 * See ETSI TS 101 756 V2.4.1 (2020-08) clause 6.1.
 */
enum class MOTContentMainType : int16_t
{
  //! @brief Data not set.
  //!
  //! @note Not relates to ETSI TS 101 756, value internally used within add-on
  NotSet = -1,

  //! @brief General data content
  GeneralData = 0x00,

  //! @brief Text data content
  Text = 0x01,

  //! @brief Image data content
  Image = 0x02,

  //! @brief Audio data content
  Audio = 0x03,

  //! @brief Video data content
  Video = 0x04,

  //! @brief Transport data content
  Transport = 0x05,

  //! @brief System data content
  System = 0x06,

  //! @brief Application data content
  //!
  //! Defined by user application.
  Application = 0x07,

  //! @brief Proprietary table data content
  //!
  //! Defined by proprietary application.
  Proprietary = 0x3f
};

/*!
 * @brief Values for the MOT header parameters content typee and content subtype
 *
 * In values here are the subtype (bits 0-16) together with his main type (bits 17-32)
 * see @ref MOTContentMainType) defined.
 *
 * See ETSI TS 101 756 V2.4.1 (2020-08) clause 6.1.
 */
enum class MOTContentType : int32_t
{
  //! @brief Masks
  //@{
  NotSet = -1,
  BaseTypeMask = 0x3f00,
  SubTypeMask = 0x00ff,
  //@}

  //! @brief General Data: 0x00xx
  //@{
  GeneralDataObjectTransfer = 0x0000,
  GeneralDataMIMEHTTP = 0x0001,
  //@}

  //! @brief Text formats: 0x01xx
  //@{
  TextASCII = 0x0100,
  TextLatin1 = 0x0101,
  TextHTML = 0x0102,
  TextPDF = 0x0103,
  //@}

  //! @brief Image formats: 0x02xx
  //@{
  ImageGIF = 0x0200,
  ImageJFIF = 0x0201,
  ImageBMP = 0x0202,
  ImagePNG = 0x0203,
  //@}

  //! @brief Audio formats: 0x03xx
  //@{
  AudioMPEG1Layer1 = 0x0300,
  AudioMPEG1Layer2 = 0x0301,
  AudioMPEG1Layer3 = 0x0302,
  AudioMPEG2Layer1 = 0x0303,
  AudioMPEG2Layer2 = 0x0304,
  AudioMPEG2Layer3 = 0x0305,
  AudioPCM = 0x0306,
  AudioAIFF = 0x0307,
  AudioATRAC = 0x0308,
  AudioUndefined = 0x0309,
  AudioMPEG4 = 0x030a,
  //@}

  //! @brief Video formats: 0x04xx
  //@{
  VideoMPEG1 = 0x0400,
  VideoMPEG2 = 0x0401,
  VideoMPEG4 = 0x0402,
  VideoH263 = 0x0403,
  //@}

  //! @brief MOT transport: 0x05xx
  //@{
  TransportHeaderUpdate = 0x0500,
  TransportHeaderOnly = 0x0501,
  //@}

  //! @brief System: 0x06xx
  //@{
  SystemMHEG = 0x0600,
  SystemJava = 0x0601,
  //@}

  //! @brief Application Specific: 0x07xx
  //@{
  Application = 0x0700,
  //@}

  //! @brief Proprietary: 0x3fxx
  //@{
  Proprietary = 0x3f00
  //@}
};

/*!
 * @brief Return the base type from the @ref MOTContentType
 */
inline MOTContentMainType getContentBaseType(MOTContentType ct)
{
  return static_cast<MOTContentMainType>(
      (static_cast<int32_t>(ct) & static_cast<int32_t>(MOTContentType::BaseTypeMask)) >> 8);
}

/*!
 * Return the sub type from the @ref MOTContentType
 *
 * As 8 bit returned, normally have 9 bits.
 */
inline uint8_t getContentSubType(MOTContentType ct)
{
  return static_cast<uint8_t>(static_cast<int32_t>(ct) &
                              static_cast<int32_t>(MOTContentType::SubTypeMask));
}

// --- MOT_FILE -----------------------------------------------------------------
struct MOT_FILE
{
  std::vector<uint8_t> data;

  // from header core
  size_t body_size = -1;
  MOTContentMainType content_main_type{MOTContentMainType::NotSet};
  MOTContentType content_full_type{MOTContentType::NotSet};

  // from header extension
  std::string content_name;
  std::string content_name_charset;
  std::string click_through_url;
  std::string alternative_location_url;
  bool trigger_time_now{false};
  uint32_t expire_time{0};
  uint8_t category{0};
  uint8_t slide_id{0};
  std::string category_title;
};