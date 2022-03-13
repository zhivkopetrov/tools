// Corresponding header
#include "resource_builder/FileBuilder.h"

// System headers
#include <cerrno>
#include <cstring>
#include <iomanip>

// Other libraries headers
#include "resource_utils/defines/ResourceDefines.h"
#include "resource_utils/common/ResourceFileHeader.h"
#include "resource_utils/structs/CombinedStructs.h"
#include "utils/ErrorCode.h"
#include "utils/Log.h"

// Own components headers

namespace {
constexpr auto MB_PRECISION_AFTER_DECIMAL = 3;
constexpr auto TAB = "  "; // 2 spaces
constexpr auto DATA_TYPE = "uint64_t";
constexpr auto MAX_UINT64_T_HEX_LENGTH = 16;
}

FileBuilder::~FileBuilder() noexcept { closeCombinedStreams(); }

ErrorCode FileBuilder::openCombinedStreams(const std::string& resFileName,
                                           const std::string& fontFileName,
                                           const std::string& soundFileName) {
  // open fileStream for write
  _combinedResDestStream.open(resFileName.c_str(),
                              std::ofstream::out | std::ofstream::binary);

  if (!_combinedResDestStream) {
    LOGERR("Error, could not open ofstream for fileName: %s, reason: %s",
        resFileName.c_str(), strerror(errno));
    return ErrorCode::FAILURE;
  }

  _combinedResDestStream << ResourceFileHeader::getEngineResHeader()
                         << ResourceFileHeader::getEngineValueReservedSlot()
                         << "\n\n"
                         << ResourceFileHeader::getEngineResHeaderAddition()
                         << ResourceFileHeader::getEngineValueReservedSlot()
                         << "\n\n"
                         << ResourceFileHeader::getEngineFileSizeHeader()
                         << ResourceFileHeader::getEngineValueReservedSlot()
                         << "\n\n";

  // open fileStream for write
  _combinedFontDestStream.open(fontFileName.c_str(),
                               std::ofstream::out | std::ofstream::binary);

  if (!_combinedFontDestStream) {
    LOGERR("Error, could not open ofstream for fileName: %s, reason: %s",
           fontFileName.c_str(), strerror(errno));
    return ErrorCode::FAILURE;
  }

  _combinedFontDestStream
      << ResourceFileHeader::getEngineFontHeader()
      << ResourceFileHeader::getEngineValueReservedSlot() << "\n\n"
      << ResourceFileHeader::getEngineFileSizeHeader()
      << ResourceFileHeader::getEngineValueReservedSlot() << "\n\n";

  // open fileStream for write
  _combinedSoundDestStream.open(soundFileName.c_str(),
                                std::ofstream::out | std::ofstream::binary);

  if (!_combinedSoundDestStream) {
    LOGERR("Error, could not open ofstream for fileName: %s, reason: %s",
           soundFileName.c_str(), strerror(errno));
    return ErrorCode::FAILURE;
  }

  _combinedSoundDestStream
      << ResourceFileHeader::getEngineSoundHeader()
      << ResourceFileHeader::getEngineValueReservedSlot() << "\n\n"
      << ResourceFileHeader::getEngineSoundHeaderAddition()
      << ResourceFileHeader::getEngineValueReservedSlot() << "\n\n"
      << ResourceFileHeader::getEngineFileSizeHeader()
      << ResourceFileHeader::getEngineValueReservedSlot() << "\n\n";

  return ErrorCode::SUCCESS;
}

void FileBuilder::closeCombinedStreams() {
  // close the stream
  _combinedResDestStream.close();

  // reset stream flags since we will be reusing it for other files
  _combinedResDestStream.clear();

  // close the stream
  _combinedFontDestStream.close();

  // reset stream flags since we will be reusing it for other files
  _combinedFontDestStream.clear();

  // close the stream
  _combinedSoundDestStream.close();

  // reset stream flags since we will be reusing it for other files
  _combinedSoundDestStream.clear();
}

ErrorCode FileBuilder::openDestStreams() {
  // open _destStreamStatic for write
  _destStreamStatic.open(_destFileNameStatic.c_str(),
                         std::ofstream::out | std::ofstream::binary);

  if (!_destStreamStatic) {
    LOGERR("Error, could not open ofstream for fileName: %s, reason: %s",
           _destFileNameStatic.c_str(), strerror(errno));
    return ErrorCode::FAILURE;
  }

  // open _destStreamDynamic for write
  _destStreamDynamic.open(_destFileNameDynamic.c_str(),
                          std::ofstream::out | std::ofstream::binary);

  if (!_destStreamDynamic) {
    LOGERR("Error, could not open ofstream for fileName: %s, reason: %s",
           _destFileNameDynamic.c_str(), strerror(errno));
    return ErrorCode::FAILURE;
  }

  // open _destStreamDynamicValues for write
  _destStreamDynamicValues.open(_destFileNameDynamicValues.c_str(),
                                std::ofstream::out | std::ofstream::binary);

  if (!_destStreamDynamicValues) {
    LOGERR("Error, could not open ofstream for fileName: %s, reason: %s",
           _destFileNameDynamicValues.c_str(), strerror(errno));
    return ErrorCode::FAILURE;
  }

  return ErrorCode::SUCCESS;
}

void FileBuilder::closeDestStream() {
  // close the header stream
  _destStreamStatic.close();
  _destStreamDynamic.close();
  _destStreamDynamicValues.close();

  // reset stream flags since we will be reusing it for other files
  _destStreamStatic.clear();
  _destStreamDynamic.clear();
  _destStreamDynamicValues.clear();
}

void FileBuilder::setNamespace(const std::string& inputNamespace) {
  _namespaceStatic = inputNamespace;

  _namespaceDynamic = inputNamespace;
  _namespaceDynamic.append("Dynamic");
}

void FileBuilder::setDestFileName(const std::string& destFileName) {
  _destFileNameStatic = destFileName;
  _destFileNameStatic.append(".h");

  _destFileNameDynamic = destFileName;
  _destFileNameDynamic.append("Dynamic.h");

  _destFileNameDynamicValues = destFileName;
  _destFileNameDynamicValues.append("DynamicValues.h");
}

void FileBuilder::setHeaderGuards(const std::string& guards) {
  _headerGuardsStatic = guards;

  _headerGuardsDynamic = guards;
  _headerGuardsDynamic.append("DYNAMIC");
}

void FileBuilder::writeData(const std::vector<CombinedData>& data) {
  fillCombinedDestFile(data);

  autoGenerateResFile(data);
}

void FileBuilder::finishCombinedDestFiles(
    const uint64_t staticWidgetsCount, const uint64_t dynamicWidgetsCount,
    const uint64_t fontsCount, const uint64_t musicsCount,
    const uint64_t chunksCount, const int32_t totalWidgetFileSize,
    const int32_t totalFontsFileSize, const int32_t totalSoundsFileSize) {
  finishCombinedResFile(staticWidgetsCount, dynamicWidgetsCount,
                        totalWidgetFileSize);

  finishCombinedFontFile(fontsCount, totalFontsFileSize);

  finishCombinedSoundFile(musicsCount, chunksCount, totalSoundsFileSize);

  closeCombinedStreams();
}

void FileBuilder::fillCombinedDestFile(const std::vector<CombinedData>& data) {
  for (const auto& entry : data) {
    if ("font" == entry.type) {
      _combinedFontDestStream << std::hex << std::uppercase;
      _combinedFontDestStream << "0x" << std::setw(MAX_UINT64_T_HEX_LENGTH)
                              << std::setfill('0') << entry.header.hashValue
                              << '\n';

      _combinedFontDestStream << std::dec << std::nouppercase;

      _combinedFontDestStream << entry.header.path << '\n'
                              << entry.header.fileSize << '\n'
                              << entry.fontSize << "\n\n";
    } else if ("sound" == entry.type) {
      _combinedSoundDestStream << std::hex << std::uppercase;
      _combinedSoundDestStream << "0x" << std::setw(MAX_UINT64_T_HEX_LENGTH)
                               << std::setfill('0') << entry.header.hashValue
                               << '\n';

      _combinedSoundDestStream << std::dec << std::nouppercase;

      _combinedSoundDestStream << entry.header.path << '\n'
                               << entry.header.fileSize << '\n'
                               << entry.soundType << '\n'
                               << entry.soundLevel << "\n\n";
    } else  //"image"         == entry.type ||
            //"sprite"        == entry.type ||
            //"sprite_manual" == entry.type
    {
      _combinedResDestStream << std::hex << std::uppercase;
      _combinedResDestStream << "0x" << std::setw(MAX_UINT64_T_HEX_LENGTH)
                             << std::setfill('0') << entry.header.hashValue
                             << '\n';
      _combinedResDestStream << std::dec << std::nouppercase;

      _combinedResDestStream
          << entry.header.path << '\n'
          << entry.header.fileSize << '\n'
          << entry.textureLoadType << '\n'
          << entry.imageRect.x << ' ' << entry.imageRect.y << ' '
          << entry.imageRect.w << ' ' << entry.imageRect.h << '\n';

      _combinedResDestStream << entry.spriteData.size() << '\n';

      for (const auto& sprite : entry.spriteData) {
        _combinedResDestStream
            << sprite.x << ' ' << sprite.y << ' '
            << sprite.w << ' ' << sprite.h
            << '\n';
      }
      _combinedResDestStream << '\n';
    }
  }
}

void FileBuilder::autoGenerateResFile(const std::vector<CombinedData>& data) {
  // Write header file
  _destStreamStatic << ResourceFileHeader::getResourceFileHeader() << "#ifndef "
                    << _headerGuardsStatic << '\n'
                    << "#define " << _headerGuardsStatic << "\n\n"
                    << "#include "
                    << "<cstdint>"
                    << "\n\n"
                    << "namespace " << _namespaceStatic << "\n{\n"
                    << TAB << "enum "
                    << "ResourceTags : " << DATA_TYPE << "\n"
                    << TAB << "{\n";

  _destStreamDynamic << ResourceFileHeader::getResourceFileHeader()
                     << "#ifndef " << _headerGuardsDynamic << '\n'
                     << "#define " << _headerGuardsDynamic << "\n\n"
                     << "#include "
                     << "<cstdint>"
                     << "\n\n"
                     << "namespace " << _namespaceDynamic << "\n{\n"
                     << TAB << "enum "
                     << "ResourceTags : " << DATA_TYPE << "\n"
                     << TAB << "{\n";

  _destStreamDynamicValues
      << ResourceFileHeader::getEngineResDynamicValuesHeader();

  // create the output stringstream, because the same information might be
  // reused multiple times
  std::ostringstream hexHashValue;

  int32_t itemsSize = 0;  // in kBytes
  for (const auto &dataEntry : data) {
    itemsSize += dataEntry.header.fileSize;

    // set stream basefield manipulator to uppercase hex
    hexHashValue << std::hex << std::uppercase;

    // by standard setw is not sticky manipulator so we need to use it
    // on every stream write
    hexHashValue << std::setw(MAX_UINT64_T_HEX_LENGTH) << std::setfill('0')
                 << dataEntry.header.hashValue;

    if (ResourceDefines::TextureLoadType::ON_INIT ==
        dataEntry.textureLoadType)
    {
      _destStreamStatic << TAB << TAB << dataEntry.tagName << " = "
                        << "0x" << hexHashValue.str() << ",\n";
    } else // ResourceDefines::TextureLoadType::ON_DEMAND
    {
      _destStreamDynamic << TAB << TAB << dataEntry.tagName << " = "
                         << "0x" << hexHashValue.str() << ",\n";

      _destStreamDynamicValues << "0x" << hexHashValue.str() << '\n';
    }

    // reset stream basefield manipulator since,
    // by the standardit's a sticky option
    hexHashValue << std::dec << std::nouppercase;

    //'reset' the string stream by constructing it with an empty string
    // this way is more efficient, because we dont invoke the std::string
    // const char * constructor
    hexHashValue.str(std::string());

    // reset the stream flags so it's ready for the next iteration
    hexHashValue.clear();
  }

  // finish streams
  _destStreamStatic << TAB << "}; /* enum ResourceTags */\n"
                    << "} /* namespace " << _namespaceStatic << " */\n\n"
                    << "#endif /* " << _headerGuardsStatic << " */";

  _destStreamDynamic << TAB << "}; /* enum ResourceTags */\n"
                     << "} /* namespace " << _namespaceDynamic << " */\n\n"
                     << "#endif /* " << _headerGuardsDynamic << " */";

  _destStreamDynamicValues << "*/";

  const std::string ITEMS_SIZE_MB =
      std::to_string(static_cast<double>(itemsSize) / 1024);
  const size_t DOT_POS = ITEMS_SIZE_MB.find('.');

  std::string itemsSizeMbStr =
      (ITEMS_SIZE_MB.substr(0, DOT_POS + 1 + MB_PRECISION_AFTER_DECIMAL));
  itemsSizeMbStr.append(" MB");

  LOG_ON_SAME_LINE("(%zu static files with size: %s) ", data.size(),
                   itemsSizeMbStr.c_str());
}

void FileBuilder::finishCombinedResFile(const uint64_t staticWidgetsCount,
                                        const uint64_t dynamicWidgetsCount,
                                        const int32_t totalWidgetFileSize) {
  const uint64_t widgetHeaderSize =
      ResourceFileHeader::getEngineResHeader().size();
  const uint64_t widgetAdditionSize =
      ResourceFileHeader::getEngineResHeaderAddition().size();
  const uint64_t engineValueHeaderSize =
      ResourceFileHeader::getEngineValueReservedSlot().size();
  const uint64_t fileSizeHeaderSize =
      ResourceFileHeader::getEngineFileSizeHeader().size();

  std::string counterStr;
  std::string blankStr;

  // move file pointer to proper position
  _combinedResDestStream.seekp(widgetHeaderSize, std::ofstream::beg);

  counterStr = std::to_string(staticWidgetsCount);

  blankStr = std::string(engineValueHeaderSize - counterStr.size(),  // size
                         ' ');                                       // content

  // write total static widget count to file
  _combinedResDestStream << counterStr << blankStr;

  // calculate widget addition offset
  const uint64_t widgetAdditionOffset =
      widgetHeaderSize + engineValueHeaderSize +
      2 +  //+2. because of the the two newline characters
      widgetAdditionSize;

  // move file pointer to proper position
  _combinedResDestStream.seekp(widgetAdditionOffset, std::ofstream::beg);

  counterStr = std::to_string(dynamicWidgetsCount);

  blankStr = std::string(engineValueHeaderSize - counterStr.size(),  // size
                         ' ');                                       // content
  // write total sound chunks count to file
  _combinedResDestStream << counterStr << blankStr;

  // calculate file size offset
  const uint64_t fileSizeOffset =
      widgetAdditionOffset + engineValueHeaderSize +
      2 +  //+2. because of the the two newline characters
      fileSizeHeaderSize;

  // move file pointer to proper position
  _combinedResDestStream.seekp(fileSizeOffset, std::ofstream::beg);

  counterStr = std::to_string(totalWidgetFileSize);

  blankStr = std::string(engineValueHeaderSize - counterStr.size(),  // size
                         ' ');                                       // content

  // write total widget file size to file
  _combinedResDestStream << counterStr << blankStr;
}

void FileBuilder::finishCombinedFontFile(const uint64_t fontsCount,
                                         const int32_t totalFontsFileSize) {
  const uint64_t fontHeaderSize =
      ResourceFileHeader::getEngineFontHeader().size();
  const uint64_t engineValueHeaderSize =
      ResourceFileHeader::getEngineValueReservedSlot().size();
  const uint64_t fileSizeHeaderSize =
      ResourceFileHeader::getEngineFileSizeHeader().size();

  std::string counterStr;
  std::string blankStr;

  // move file pointer to proper position
  _combinedFontDestStream.seekp(fontHeaderSize, std::ofstream::beg);

  counterStr = std::to_string(fontsCount);

  blankStr = std::string(engineValueHeaderSize - counterStr.size(),  // size
                         ' ');                                       // content
  // write total fonts count to file
  _combinedFontDestStream << counterStr << blankStr;

  // calculate file size offset
  const uint64_t fileSizeOffset =
      fontHeaderSize + engineValueHeaderSize +
      2 +  //+2. because of the the two newline characters
      fileSizeHeaderSize;

  // move file pointer to proper position
  _combinedFontDestStream.seekp(fileSizeOffset, std::ofstream::beg);

  counterStr = std::to_string(totalFontsFileSize);

  blankStr = std::string(engineValueHeaderSize - counterStr.size(),  // size
                         ' ');                                       // content

  // write total fonts file size to file
  _combinedFontDestStream << counterStr << blankStr;
}

void FileBuilder::finishCombinedSoundFile(const uint64_t musicsCount,
                                          const uint64_t chunksCount,
                                          const int32_t totalSoundsFileSize) {
  const uint64_t soundHeaderSize =
      ResourceFileHeader::getEngineSoundHeader().size();
  const uint64_t engineValueHeaderSize =
      ResourceFileHeader::getEngineValueReservedSlot().size();
  const uint64_t soundAdditionSize =
      ResourceFileHeader::getEngineSoundHeaderAddition().size();
  const uint64_t fileSizeHeaderSize =
      ResourceFileHeader::getEngineFileSizeHeader().size();

  std::string counterStr;
  std::string blankStr;

  // move file pointer to proper position
  _combinedSoundDestStream.seekp(soundHeaderSize, std::ofstream::beg);

  counterStr = std::to_string(musicsCount);

  blankStr = std::string(engineValueHeaderSize - counterStr.size(),  // size
                         ' ');                                       // content
  // write total musics count to file
  _combinedSoundDestStream << counterStr << blankStr;

  // calculate sound addition offset
  const uint64_t soundAdditionOffset =
      soundHeaderSize + engineValueHeaderSize +
      2 +  //+2. because of the the two newline characters
      soundAdditionSize;

  // move file pointer to proper position
  _combinedSoundDestStream.seekp(soundAdditionOffset, std::ofstream::beg);

  counterStr = std::to_string(chunksCount);

  blankStr = std::string(engineValueHeaderSize - counterStr.size(),  // size
                         ' ');                                       // content
  // write total sound chunks count to file
  _combinedSoundDestStream << counterStr << blankStr;

  // calculate file size offset
  const uint64_t fileSizeOffset =
      soundAdditionOffset + engineValueHeaderSize +
      2 +  //+2. because of the the two newline characters
      fileSizeHeaderSize;

  // move file pointer to proper position
  _combinedSoundDestStream.seekp(fileSizeOffset, std::ofstream::beg);

  counterStr = std::to_string(totalSoundsFileSize);

  blankStr = std::string(engineValueHeaderSize - counterStr.size(),  // size
                         ' ');                                       // content

  // write total fonts file size to file
  _combinedSoundDestStream << counterStr << blankStr;
}
