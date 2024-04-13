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

#include "mot_object.h"

#include "mot_file.h"
#include "utils/charsets.h"
#include "utils/log.h"

namespace
{

/*!
 * @brief List of all MOT parameters
 *
 * See ETSI EN 301 234 V2.1.1 (2006-05).
 */
enum class MOTSlideObjectType
{
  //! PermitOutdatedVersions
  //!
  //! When the MOT decoder notices a change to the data carousel (i.e. it gets a new
  //! MOT directory) then this parameter can be used to indicate if an outdated(old)
  //! version of an MOT object can be used until the current(new) version of this
  //! object is successfully reassembled.
  PermitOutdatedVersions = 0b000001,

  //! Creation time
  //!
  //! Authoring date of the object. The value of the parameter field is coded in the
  //! UTC format (see ETS 300 401[1]).
  CreationTime = 0b000010,

  //! Start validity
  //!
  //! The received object is valid after the time indicated. The value of the parameter
  //! field is coded in the UTC format(see ETS 300 401[1]).
  StartValidity = 0b000011,

  //! Expire time
  //!
  //! The received object is not valid anymore after the time indicated. The value of the
  //! parameter field is coded in the UTC format(see ETS 300 401[1]).If this parameter is
  //! not present the object is valid for an undefined period of time(up to the receiver).
  //! The object is not valid anymore after it expired and therefore it should not be
  //! presented anymore.
  ExpireTime = 0b000100,

  //! Trigger time (user application specific parameter)
  //!
  //! This parameter specifies the time for when the presentation takes place . The TriggerTime
  //! activates the object according to its ContentType.The value of the parameter field is
  //! coded in the UTC format (see ETS 300 401[1]).
  TriggerTime = 0b000101,

  //! Version number
  //!
  //! If several versions of an object are transferred, this parameter indicates its version number.
  //! The parameter value is coded as an unsigned binary number, starting at 0 and being incremented
  //! by 1 modulo 256 each time the version changes.If the VersionNumber differs, the content of
  //! the body was modified.
  VersionNumber = 0b000110,

  //! Repetition distance
  //!
  //! To support advanced caching of objects in the receiver, this parameter indicates a guaranteed
  //! maximum time until the next repetition of an object.The resolution in the time domain is
  //! 1 / 10 second to allow an exact synchronization, whereas the maximum time which can be
  //! indicated reaches up to 1 677 721 seconds (equal approx. 19 days, 10 hours and 2 minutes)
  //! for very slow repetition rates.
  RepetitionDistance = 0b000111,

  //! Group reference
  //!
  //! A number of objects forming a logical entity can be managed using the GroupReference, which
  //! allows to identify all members of the group by a single identifier.The 32 - bit GroupId can
  //! separate a large number of groups in parallel or during a long time period.Each group can
  //! comprise max. 65 535 elements.
  GroupReference = 0b001000,

  //! Expiration
  //!
  //! The parameter Expiration indicates how long an object can still be used by the MOT
  //! decoder after reception loss. The size of the DataField determines if an absolute or
  //! a relative expire time is specified.
  Expiration = 0b001001,

  //! Priority
  //!
  //! The parameter is used to indicate the storage priority, i.e. in case of a "disk full" state
  //! only the objects having a high priority should be stored.It indicates the relevance of the
  //! content of the particular object for the service, i.e.a home page of a HTML based service
  //! has a high priority and should therefore not be deleted first, whereas pictures
  //! (e.g.buttons etc.) are not as important as the home page and hence can be deleted first in
  //! case of a memory overflow. The possible values range from 0 = highest priority to
  //! 255 = lowest priority.
  Priority = 0b001010,

  //! Label
  //!
  //! The field of this parameter starts with a character set indicator (see ETS 300 401 [1]).
  //! The other 4 bits are Reserved for future additions(Rfa).Thereafter the label text follows.
  //! The total number of characters is fixed to 16. The field of this parameter is coded
  //! according to "Service label" (see ETS 300 401[1]), but without the starting Service Id
  //! (see figure 11).It should contain the label text to be displayed on 8 - or 16 - digit
  //! text displays.Labels are used to launch applications, they might be presented to the user.
  Label = 0b001011,

  //! Content name
  //!
  //! The DataField of this parameter starts with a one byte field, comprising a 4-bit
  //! character set indicator(see table 3) and a 4 - bit Rfa field.The following character
  //! field contains a unique name or identifier for the object.The total number of characters
  //! is determined by the DataFieldLength indicator minus one byte.
  //!
  //! Hierarchical structures should use a slash "/" to separate different levels.No system
  //! specific restrictions shall be applied. Slashes forward inside the ContentName separate
  //! levels and slashes are not permitted for any other meaning than this.
  ContentName = 0b001100,

  //! Unique Body Version
  //!
  //! This parameter is used to uniquely identify a version of an MOT body (identified by its ContentName).
  UniqueBodyVersion = 0b001101,

  //! Content Description
  //!
  //! The field of the parameter starts with a 4-bit character set indicator (see ETS 300 401 [1]).
  //! The other 4 bits are Reserved for future additions(Rfa).Afterwards the text describing the
  //! content of the object follows. This description shall be presented on receivers with limited
  //! display capabilities(i.e.text - only).The total number of characters is determined by the
  //! DataFieldLength Indicator, decreased by the starting character set indicator(one byte).
  ContentDescription = 0b001111,

  //! Mime type
  //!
  //! In HTTP, the type of an object is indicated using the Multi-purpose Internet Mail
  //! Extensions (MIME) [4] mechanism. MIME strings categorize object types according to first
  //! a general type followed by a specific format, e.g. "text/html", "image/jpeg" and
  //! "application/octet-stream".
  MimeType = 0b010000,

  //! Compression type
  //!
  //! The CompressionType parameter is used to indicate that an object has been compressed
  //! and which compression algorithm has been applied to the data.The DataField of this
  //! parameter carries a one byte identifier(CompressionId) for the compressed data format. If
  //! new compressed data formats are to be used, a new CompressionId shall be obtained from
  //! and registered with the WorldDAB Information and Registration Centre.
  //!
  //! The registered compressed data formats shall be defined in TS 101 756[7], table 18.
  //!
  //! Even an MOT decoder that does not support any compression shall check this parameter to
  //! determine if an MOT body is compressed.
  CompressionType = 0b010001,

  //! Additional Header (user application specific parameter)
  AdditionalHeader = 0b100000,

  //! ProfileSubset
  //!
  //! The data carousel for a user application carries objects to support more than one
  //! user application profile.
  ProfileSubset = 0b100001,

  //! CAInfo
  //!
  //! The CAInfo parameter is used to indicate the scrambling status of individual objects
  //! within the data carousel where a service potentially contains both scrambled and
  //! unscrambled objects.The syntax of the CAInfo parameter is defined in TS 102 367[8].
  CAInfo = 0b100011,

  //! CAReplacementObject
  //!
  //! If an object within the data carousel is scrambled and the receiver is unable to
  //! unscramble the object, it is desirable for  the receiver to be able to present
  //! information about how the user may subscribe to the service in order to decrypt the
  //! scrambled objects.
  CAReplacementObject = 0b100100,

  //! CAReplacementObject
  //!
  //! @note Application specific parameter.
  Category_SlideID = 0b100101, // 0x25

  //! CategoryTitle
  //!
  //! @note Application specific parameter.
  CategoryTitle = 0b100110, // 0x26

  //! ClickThroughURL
  //!
  //! @note Application specific parameter.
  ClickThroughURL = 0b100111, // 0x27

  //! AlternativeLocationURL
  //!
  //! @note Application specific parameter.
  AlternativeLocationURL = 0b101000, // 0x28

  //! Alert
  //!
  //! @note Application specific parameter.
  Alert = 0b101001, // 0x29

  //! Application specific
  //!
  //! This parameter field contains private parameters exclusively used by the application
  //! itself and therefor no specification is required.
  ApplicationSpecific = 0b111111
};

/*!
   * @brief PLI (Parameter Length Indicator)
   *
   * This 2-bit field describes the total length of the associated parameter.
   *
   * See ETSI EN 301 234 V2.1.1 (2006-05) clause 6.2.
   */
enum class MOTParLengthInd
{
  //! Total parameter length = 1 byte, no DataField available.
  fixed1ByteSize = 0b00,

  //! Total parameter length = 2 bytes, length of DataField is 1 byte.
  fixed2BytesSize = 0b01,

  //! Total parameter length = 5 bytes; length of DataField is 4 bytes.
  fixed5BytesSize = 0b10,

  //! Total parameter length depends on the DataFieldLength indicator (the maximum
  //! parameter length is 32 770 bytes).
  variableSize = 0b11
};

} // namespace

// --- CMOTObject -----------------------------------------------------------------
CMOTObject::CMOTObject()
{
  m_resultFile = std::make_shared<MOT_FILE>();
}

CMOTObject::~CMOTObject() = default;

void CMOTObject::AddSeg(
    bool dg_type_header, int seg_number, bool last_seg, const uint8_t* data, size_t len)
{
  (dg_type_header ? m_header : m_body).AddSeg(seg_number, last_seg, data, len);
}

bool CMOTObject::ParseCheckHeader(const std::shared_ptr<MOT_FILE>& file)
{
  if (!file)
    return false;

  std::vector<uint8_t> data = m_header.GetData();

  // parse/check header core
  if (data.size() < 7)
    return false;

  /*!
   * Check "(data[5] & 0x01) << 8;"
   *
   * Normally have the content subtype 9 bits, but as bit 9 unused do we use the 8 bits only.
   * Here we do a small check to confirm for future no use about and for the case it comes
   * to implement support for it.
   *
   * Correct code:
   * ```cpp
   * int content_sub_type = ((data[5] & 0x01) << 8) | data[6];
   * ```
   *
   * Ref. ETSI EN 301 234 and ETSI TS 101 756 clause 6.1.
   */
  if (data[5] & 0x01)
    utils::LOG(utils::LOGLevel::WARNING, "Inside this DAB+ stream becomes about MOT for content "
                                         "subtype 9 bits used (we only read first 8)!");

  const size_t body_size = (data[0] << 20) | (data[1] << 12) | (data[2] << 4) | (data[3] >> 4);
  const size_t header_size = ((data[3] & 0x0F) << 9) | (data[4] << 1) | (data[5] >> 7);
  const MOTContentMainType content_main_type =
      static_cast<MOTContentMainType>((data[5] & 0x7F) >> 1);
  const MOTContentType content_type = static_cast<MOTContentType>(
      ((data[5] & 0x7F) << 7) | data[6]); //!< @warning See text above about bit 9!

  fprintf(stderr,
          "body_size: %5zu, header_size: %3zu, content_main_type: 0x%02X, content_type: 0x%04X\n",
          body_size, header_size, content_main_type, content_type);

  if (header_size != m_header.GetSize())
    return false;

  const bool header_update = content_type == MOTContentType::TransportHeaderUpdate;

  // abort, if neither none nor both conditions (header received/update) apply
  if (m_headerReceived != header_update)
    return false;

  if (!header_update)
  {
    // store core info
    file->body_size = body_size;
    file->content_main_type = content_main_type;
    file->content_full_type = content_type;
  }

  const std::string old_content_name = file->content_name;
  std::string new_content_name;

  // parse/check header extension
  for (size_t offset = 7; offset < data.size();)
  {
    const MOTParLengthInd pli = static_cast<MOTParLengthInd>(data[offset] >> 6);
    const MOTSlideObjectType param_id = static_cast<MOTSlideObjectType>(data[offset] & 0x3F);
    ++offset;

    // get parameter len
    size_t data_len;
    switch (pli)
    {
      case MOTParLengthInd::fixed1ByteSize:
        data_len = 0;
        break;
      case MOTParLengthInd::fixed2BytesSize:
        data_len = 1;
        break;
      case MOTParLengthInd::fixed5BytesSize:
        data_len = 4;
        break;
      case MOTParLengthInd::variableSize:
        if (offset >= data.size())
          return false;
        bool ext = data[offset] & 0x80;
        data_len = data[offset] & 0x7F;
        ++offset;

        if (ext)
        {
          if (offset >= data.size())
            return false;
          data_len = (data_len << 8) + data[offset];
          ++offset;
        }
        break;
    }

    if (offset + data_len - 1 >= data.size())
      return false;

    // process parameter
    switch (param_id)
    {
      case MOTSlideObjectType::ExpireTime:
      {
        file->expire_time = data[offset]; // TODO not tested
        fprintf(stderr, "ExpireTime:             %i\n", file->expire_time);
        break;
      }

      case MOTSlideObjectType::TriggerTime:
      {
        if (data_len < 4)
          return false;
        // TODO: not only distinguish between Now or not
        file->trigger_time_now = !(data[offset] & 0x80);
        fprintf(stderr, "TriggerTime:            %i %s\n", file->trigger_time_now,
                file->trigger_time_now ? "Now" : "(not Now)");
        break;
      }

      case MOTSlideObjectType::ContentName:
      {
        if (data_len == 0)
          return false;
        //file->content_name = CharsetTools::ConvertTextToUTF8(&data[offset + 1], data_len - 1, data[offset] >> 4, true, &file->content_name_charset);
        file->content_name =
            charsets::toUtf8((const char*)&data[offset + 1],
                             (charsets::CharacterSet)(data[offset] >> 4), data_len - 1);
        new_content_name = file->content_name;
        fprintf(stderr, "ContentName:            '%s'\n", file->content_name.c_str());
        break;
      }

      case MOTSlideObjectType::UniqueBodyVersion:
      {
        if (data_len != 4)
          return false;

        uint32_t version = (data[offset] << 24) | (data[offset + 1] << 16) |
                           (data[offset + 2] << 8) | data[offset + 3];
        fprintf(stderr, "UniqueBodyVersion:      '0x%X'\n", version);
        break;
      }

      case MOTSlideObjectType::Category_SlideID:
      {
        file->category = data[offset];
        file->slide_id = data[offset + 1];
        fprintf(stderr, "Category/SlideID:       '%i' / '%i'\n", file->category, file->slide_id);
        break;
      }

      case MOTSlideObjectType::CategoryTitle:
      {
        file->category_title = std::string((char*)&data[offset], data_len); // already UTF-8
        break;
      }

      case MOTSlideObjectType::ClickThroughURL:
      {
        file->click_through_url = std::string((char*)&data[offset], data_len); // already UTF-8
        fprintf(stderr, "ClickThroughURL:        '%s'\n", file->click_through_url.c_str());
        break;
      }

      case MOTSlideObjectType::AlternativeLocationURL:
      {
        file->alternative_location_url =
            std::string((char*)&data[offset], data_len); // already UTF-8
        fprintf(stderr, "AlternativeLocationURL: '%s'\n", file->alternative_location_url.c_str());
        break;
      }

      case MOTSlideObjectType::Alert:
      {
        fprintf(stderr, "Alert: \n");
        break;
      }

      default:
        fprintf(stderr, "Unsupported param_id: '%X' (data_len = %zi)\n", param_id, data_len);
    }
    offset += data_len;
  }

  if (!header_update)
  {
    // ensure actual header is processed only once
    m_headerReceived = true;
  }
  else
  {
    // ensure matching content name
    if (new_content_name != old_content_name)
      return false;
  }

  return true;
}

bool CMOTObject::IsToBeShown()
{
  // abort, if already shown
  if (m_shown)
    return false;

  // try to process finished header
  if (m_header.IsFinished())
  {
    // parse/check MOT header
    bool result = ParseCheckHeader(m_resultFile);
    m_header.Reset(); // allow for header updates
    if (!result)
      return false;
  }

  // abort, if incomplete/not yet triggered
  if (!m_headerReceived)
    return false;
  if (!m_body.IsFinished() || m_resultFile->body_size != m_body.GetSize())
    return false;
  if (!m_resultFile->trigger_time_now)
    return false;

  // add body data
  m_resultFile->data = m_body.GetData();

  m_shown = true;
  return true;
}
