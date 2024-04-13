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
  m_MOTAppType = X_PADAppType_NotSet;

  m_lastXPAD_CI.Reset();

  m_dlDecoder.Reset();
  m_DGLIDecoder.Reset();
  m_MOTDecoder.Reset();
}

void CPADDecoder::SetMOTAppType(X_PADApplicationType type)
{
  m_MOTAppType = type;
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

  const auto fpad_type = static_cast<F_PADType>(fpad_data[0] >> 6);
  const auto xpad_ind = static_cast<X_PADInd>((fpad_data[0] & 0x30) >> 4);
  const auto byte_l_indicator = static_cast<ByteLIndicator>(fpad_data[0] & 0xF);
  const auto ci_flag = static_cast<ContentsIndicatorFlag>((fpad_data[1] & 0x02) >> 1);

  XPAD_CI prev_xpad_ci = m_lastXPAD_CI;
  m_lastXPAD_CI.Reset();

  // build CI list
  if (fpad_type == F_PADType::Type0)
  {
    if (ci_flag == ContentsIndicatorFlag::ContentsIndicatorsPresent)
    {
      switch (xpad_ind)
      {
        // short X-PAD
        case X_PADInd::shortData:
        {
          if (xpad_len < 1)
            return;

          const X_PADApplicationType type = static_cast<X_PADApplicationType>(m_xpad[0] & 0x1F);

          // skip end marker
          if (type != X_PADAppType_EndMarker)
          {
            xpad_cis_len = 1;
            xpad_cis.emplace_back(3, type);
          }
          break;
        }

        // variable size X-PAD
        case X_PADInd::variableSizeData:
        {
          xpad_cis_len = 0;
          for (size_t i = 0; i < 4; i++)
          {
            if (xpad_len < i + 1)
              return;

            const uint8_t ci_raw = m_xpad[i];
            xpad_cis_len++;

            // break on end marker
            if ((ci_raw & 0x1F) == X_PADAppType_EndMarker)
              break;

            xpad_cis.emplace_back(ci_raw);
          }
          break;
        }
      }
    }
    else
    {
      switch (xpad_ind)
      {
        case X_PADInd::shortData:
        case X_PADInd::variableSizeData:
          // if there is a previous CI, append it
          if (prev_xpad_ci.type != X_PADAppType_NotSet)
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
  X_PADApplicationType xpad_ci_type_continued = X_PADAppType_NotSet;
  for (const XPAD_CI& xpad_ci : xpad_cis)
  {
    // len only valid for the *immediate* next data group after the DGLI!
    size_t m_dgliLength = m_DGLIDecoder.GetDGLILen();

    // handle Data Subfield
    switch (xpad_ci.type)
    {
      case X_PADAppType_DataGroupLengthIndicator: // Data Group Length Indicator
      {
        const bool start = ci_flag == ContentsIndicatorFlag::ContentsIndicatorsPresent;
        m_DGLIDecoder.ProcessDataSubfield(start, m_xpad + xpad_offset, xpad_ci.len);

        xpad_ci_type_continued = X_PADAppType_DataGroupLengthIndicator;
        break;
      }

      case X_PADAppType_DynamicLabelSegment_StartOfX:
      case X_PADAppType_DynamicLabelSegment_ContinuationOfX:
      {
        const bool start = xpad_ci.type == X_PADAppType_DynamicLabelSegment_StartOfX;

        // if new m_label available, append it
        if (m_dlDecoder.ProcessDataSubfield(start, m_xpad + xpad_offset, xpad_ci.len))
          m_observer->PADChangeDynamicLabel(m_dlDecoder.GetLabel());

        xpad_ci_type_continued = X_PADAppType_DynamicLabelSegment_ContinuationOfX;
        break;
      }

      case X_PADAppType_MOT_StartOfX:
      case X_PADAppType_MOT_ContinuationOfX:
      case X_PADAppType_MOT_StartOfCAMessages:
      case X_PADAppType_MOT_ContinuationOfCAMessages:
      {
        // MOT, X-PAD data group (start/continuation)
        if (m_MOTAppType != X_PADAppType_NotSet &&
            (xpad_ci.type == m_MOTAppType || xpad_ci.type == m_MOTAppType + 1))
        {
          const bool start = xpad_ci.type == m_MOTAppType;

          if (start)
            m_MOTDecoder.SetLen(m_dgliLength);

          // if new Data Group available, append it
          if (m_MOTDecoder.ProcessDataSubfield(start, m_xpad + xpad_offset, xpad_ci.len))
          {
            // if new slide available, show it
            const auto new_slide = m_MOTDecoder.GetFile();
            if (!new_slide)
              break;

            // check file type
            bool show_slide = true;
            if (new_slide->content_main_type != MOTContentMainType::Image)
              show_slide = false;

            if (show_slide)
            {
              fprintf(stderr, "---------------------------> show_slide yes\n");
              m_observer->PADChangeSlide(new_slide);
            }
            else
            {
              fprintf(stderr, "---------------------------> show_slide false\n");
            }
          }

          xpad_ci_type_continued = static_cast<X_PADApplicationType>(m_MOTAppType + 1);
        }

        break;
      }

      default:
        fprintf(stderr, "---------------------------> Unknown X_PADAppType %i\n", xpad_ci.type);
        break;
    }
    //		fprintf(stderr, "CPADDecoder: Data Subfield: type: %2d, len: %2zu\n", it->type, it->len);

    xpad_offset += xpad_ci.len;
  }

  // set last CI
  m_lastXPAD_CI.len = xpad_offset;
  m_lastXPAD_CI.type = xpad_ci_type_continued;
}
