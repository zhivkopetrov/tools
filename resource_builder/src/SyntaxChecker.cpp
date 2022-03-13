// Corresponding header
#include "resource_builder/SyntaxChecker.h"

// System headers
#include <cctype>

// Other libraries headers
#include "utils/ErrorCode.h"
#include "utils/Log.h"

// Own components headers

SyntaxChecker::SyntaxChecker()
    : _TAG_STR("tag"),
      _PATH_STR("path"),
      _TYPE_STR("type"),
      _DESCR_STR("description"),
      _POS_STR("position"),
      _LOAD_STR("load"),
      _TAG_STR_SIZE(_TAG_STR.size()),
      _PATH_STR_SIZE(_PATH_STR.size()),
      _TYPE_STR_SIZE(_TYPE_STR.size()),
      _DESCR_STR_SIZE(_DESCR_STR.size()),
      _POS_STR_SIZE(_POS_STR.size()),
      _LOAD_STR_SIZE(_LOAD_STR.size()) {
  _currField = ResourceDefines::Field::TAG;
  _currFieldType = ResourceDefines::FieldType::UNKNOWN;
}

void SyntaxChecker::updateOrder() {
  if (ResourceDefines::FieldType::FONT == _currFieldType ||
      ResourceDefines::FieldType::SOUND == _currFieldType) {
    // fonts and sounds do not have positions
    if (ResourceDefines::Field::DESCRIPTION == _currField) {
      _currField = ResourceDefines::Field::END_FIELD;
    } else {
      ++_currField;
    }
  } else {
    ++_currField;  // normal update
  }
}

bool SyntaxChecker::hasValidTag(const std::string& line) {
  bool success = true;

  if (ResourceDefines::FieldType::SPRITE_MANUAL == _currFieldType) {
    if (ResourceDefines::Field::POSITION == _currField) {
      if (line.size() < _DESCR_STR_SIZE) {
        LOGERR("Internal error. Expected tag: '%s', Received tag '%s'",
               _LOAD_STR.c_str(), line.c_str());

        success = false;
      } else {
        const std::string chunk = line.substr(0, _DESCR_STR_SIZE);
        if (chunk == _DESCR_STR) {
          // manual sprites have multiple descriptions
          _currField = ResourceDefines::Field::DESCRIPTION;
        }
      }
    }
  }

  switch (_currField) {
    case ResourceDefines::Field::TAG: {
      if (line.size() < _TAG_STR_SIZE) {
        LOGERR("Internal error. Expected tag: '%s', Received tag '%s'",
               _LOAD_STR.c_str(), line.c_str());

        success = false;
      } else {
        const std::string chunk = line.substr(0, _TAG_STR_SIZE);
        if (chunk != _TAG_STR) {
          LOGERR(
              "Internal error. Expected tag: '%s', Received tag"
              "'%s'",
              _TAG_STR.c_str(), chunk.c_str());

          success = false;
        }
      }
    } break;

    case ResourceDefines::Field::TYPE: {
      if (line.size() < _TYPE_STR_SIZE) {
        LOGERR("Internal error. Expected tag: '%s', Received tag '%s'",
               _LOAD_STR.c_str(), line.c_str());

        success = false;
      } else {
        const std::string chunk = line.substr(0, _TYPE_STR_SIZE);
        if (chunk != _TYPE_STR) {
          LOGERR(
              "Internal error. Expected tag: '%s', Received tag "
              "'%s'",
              _TYPE_STR.c_str(), chunk.c_str());

          success = false;
        }
      }
    } break;

    case ResourceDefines::Field::PATH: {
      if (line.size() < _PATH_STR_SIZE) {
        LOGERR("Internal error. Expected tag: '%s', Received tag '%s'",
               _LOAD_STR.c_str(), line.c_str());

        success = false;
      } else {
        const std::string chunk = line.substr(0, _PATH_STR_SIZE);
        if (chunk != _PATH_STR) {
          LOGERR(
              "Internal error. Expected tag: '%s', Received tag "
              "'%s'",
              _PATH_STR.c_str(), chunk.c_str());
          success = false;
        }
      }
    } break;

    case ResourceDefines::Field::DESCRIPTION: {
      if (line.size() < _DESCR_STR_SIZE) {
        LOGERR("Internal error. Expected tag: '%s', Received tag '%s'",
               _LOAD_STR.c_str(), line.c_str());

        success = false;
      } else {
        const std::string chunk = line.substr(0, _DESCR_STR_SIZE);
        if (chunk != _DESCR_STR) {
          LOGERR(
              "Internal error. Expected tag: '%s', Received tag "
              "'%s'",
              _DESCR_STR.c_str(), chunk.c_str());

          success = false;
        }
      }
    } break;

    case ResourceDefines::Field::POSITION: {
      if (line.size() < _POS_STR_SIZE) {
        LOGERR("Internal error. Expected tag: '%s', Received tag '%s'",
               _LOAD_STR.c_str(), line.c_str());

        success = false;
      } else {
        const std::string chunk = line.substr(0, _POS_STR_SIZE);
        if (chunk != _POS_STR) {
          LOGERR(
              "Internal error. Expected tag: '%s', Received tag"
              "'%s' ",
              _POS_STR.c_str(), chunk.c_str());

          success = false;
        }
      }
    } break;

    case ResourceDefines::Field::LOAD: {
      if (line.size() < _LOAD_STR_SIZE) {
        LOGERR("Internal error. Expected tag: '%s', Received tag '%s'",
               _LOAD_STR.c_str(), line.c_str());

        success = false;
      } else {
        const std::string chunk = line.substr(0, _LOAD_STR_SIZE);
        if (chunk != _LOAD_STR) {
          LOGERR(
              "Internal error. Expected tag: '%s', Received tag "
              "'%s' ",
              _LOAD_STR.c_str(), chunk.c_str());

          success = false;
        }
      }
    } break;

    default:
      LOGERR("Error, invalid enum value %d", _currField);
      break;
  }

  return success;
}

ErrorCode SyntaxChecker::extractRowData(const std::string& lineData,
                                        std::string& outData,
                                        int32_t& outEventCode) {
  uint64_t dataStartIndex = 0;

  const uint64_t LINE_DATA_SIZE = lineData.size();
  const uint64_t DELIMITER_POS = lineData.find("=");

  if (std::string::npos == DELIMITER_POS) {
    LOGERR("Error, '=' sign could not be found");
    return ErrorCode::FAILURE;
  }

  if (DELIMITER_POS == LINE_DATA_SIZE) {
    LOGERR("Error, no data for current tag");
    return ErrorCode::FAILURE;
  }

  for (uint64_t i = DELIMITER_POS + 1; i < LINE_DATA_SIZE; ++i) {
    if (!isblank(lineData[i])) {
      dataStartIndex = i;
      break;
    }
  }

  // initial position did not change
  if (dataStartIndex == 0) {
    // empty information leading to crash e.g. "tag =   "
    LOGERR("Error, no data for current tag");
    return ErrorCode::FAILURE;
  }

  outData = lineData.substr(dataStartIndex,                    // start pos
                            LINE_DATA_SIZE - dataStartIndex);  // size

  outEventCode = _currField;

  return ErrorCode::SUCCESS;
}

void SyntaxChecker::setFieldTypeFromString(const std::string& dataType) {
  if ("image" == dataType) {
    _currFieldType = ResourceDefines::FieldType::IMAGE;
  } else if ("sprite" == dataType) {
    _currFieldType = ResourceDefines::FieldType::SPRITE;
  } else if ("sprite_manual" == dataType) {
    _currFieldType = ResourceDefines::FieldType::SPRITE_MANUAL;
  } else if ("font" == dataType) {
    _currFieldType = ResourceDefines::FieldType::FONT;
  } else if ("sound" == dataType) {
    _currFieldType = ResourceDefines::FieldType::SOUND;
  } else {
    _currFieldType = ResourceDefines::FieldType::UNKNOWN;

    LOGERR("Internal error, _currFieldType = FieldType::UNKNOWN");
  }
}

bool SyntaxChecker::isChunkReady() {
  bool result = false;

  // if total count is reached -> reset
  if (ResourceDefines::Field::END_FIELD == _currField) {
    result = true;
    _currField = ResourceDefines::Field::TAG;
  }

  return result;
}
