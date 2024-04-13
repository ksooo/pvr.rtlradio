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

#include "mot_entity.h"

#include <memory>

struct MOT_FILE;

// --- CMOTObject -----------------------------------------------------------------
class CMOTObject
{
public:
  CMOTObject();
  ~CMOTObject();

  void AddSeg(bool dg_type_header, int seg_number, bool last_seg, const uint8_t* data, size_t len);
  bool IsToBeShown();
  std::shared_ptr<MOT_FILE> GetFile() { return m_resultFile; }

private:
  bool ParseCheckHeader(const std::shared_ptr<MOT_FILE>& target_file);

  CMOTEntity m_header;
  CMOTEntity m_body;
  bool m_headerReceived{false};
  bool m_shown{false};

  std::shared_ptr<MOT_FILE> m_resultFile;
};
