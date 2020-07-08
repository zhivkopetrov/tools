// Corresponding header
#include "ResourceParser.h"

// C system headers

// C++ system headers
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <functional>  //for std::hash

// Other libraries headers

// Own components headers
#include "resource_utils/defines/ResourceDefines.h"
#include "resource_utils/common/ResourceFileHeader.h"
#include "utils/data_type/StringUtils.h"
#include "utils/file_system/FileSystemUtils.h"
#include "utils/Log.h"

static std::hash<std::string> hashFunction;



namespace {
constexpr auto EXTERNAL_PATH_PREFIX = "external - ";
constexpr auto EXTERNAL_PATH_PREFIX_SIZE = 11;
constexpr auto MB_PRECISION_AFTER_DECIMAL = 3;
}

ResourceParser::ResourceParser() : _projectAbsFilePath("Not set") {
  resetInternals();
}

int32_t ResourceParser::init(const std::string & projectPath) {
  _projectAbsFilePath = projectPath.empty() ?
      FileSystemUtils::getRootDirectory() : projectPath;

  _fileParser.setAbsoluteProjectPath(_projectAbsFilePath);

  // Reserve enough memory for the whole parse process, no no unneeded
  // internal vector grow is invoked
  _fileData.reserve(200);

  return EXIT_SUCCESS;
}

int32_t ResourceParser::parseResourceTree() {
  int32_t err = EXIT_SUCCESS;
  _startDir = _projectAbsFilePath;
  _projectFolder = _projectAbsFilePath;
  if (_projectFolder.back() == '/') {
    _projectFolder.pop_back();
  }
  _projectFolder =
      FileSystemUtils::getFileNameFromAbsolutePath(_projectFolder);

  LOG("======================================");
  LOG("Starting recursive search on %s", _startDir.c_str());

  if (EXIT_SUCCESS != setupResourceTree()) {
    LOGERR("Error, setupResourceTree() failed");

    err = EXIT_FAILURE;
  }
  if (EXIT_SUCCESS == err) {
    if (EXIT_SUCCESS != processAllFiles()) {
      LOGERR("processAllFiles() failed");
      err = EXIT_FAILURE;
    }
  }

  if (EXIT_SUCCESS == err) {
    _fileBuilder.finishCombinedDestFiles(
        _staticWidgetsCounter, _dynamicWidgetsCounter, _fontsCounter,
        _musicsCounter, _chunksCounter, _staticResFileTotalSize,
        _fontFileTotalSize, _soundFileTotalSize);

    constexpr int32_t CONTAINERS_SIZE = 4;

    const int32_t ITEMS_SIZE[CONTAINERS_SIZE]{
        _staticResFileTotalSize, _dynamicResFileTotalSize, _fontFileTotalSize,
        _soundFileTotalSize};

    std::string itemsSizeStr[CONTAINERS_SIZE];

    for (int32_t i = 0; i < CONTAINERS_SIZE; ++i) {
      itemsSizeStr[i] =
          std::to_string(static_cast<double>(ITEMS_SIZE[i]) / 1024);
      const size_t DOT_POS = itemsSizeStr[i].find('.');

      itemsSizeStr[i] =
          (itemsSizeStr[i].substr(0, DOT_POS + 1 + MB_PRECISION_AFTER_DECIMAL));
      itemsSizeStr[i].append(" MB");
    }

    LOG_ON_SAME_LINE("\nRecursive search on %s ... ", _startDir.c_str());
    LOGG("[Done]");

    LOG_ON_SAME_LINE(
        "%s generation ... (%lu static files with size: %s "
        "and %lu dynamic files with size: %s) ",
        ResourceFileHeader::getResourceBinName().c_str(), _staticWidgetsCounter,
        itemsSizeStr[0].c_str(), _dynamicWidgetsCounter,
        itemsSizeStr[1].c_str());
    LOGG("[Done]");
    LOG_ON_SAME_LINE("%s generation ... (%lu static files with size: %s) ",
                     ResourceFileHeader::getFontBinName().c_str(), _fontsCounter,
                     itemsSizeStr[2].c_str());
    LOGG("[Done]");
    LOG_ON_SAME_LINE("%s generation ... (%lu static files with size: %s) ",
                     ResourceFileHeader::getSoundBinName().c_str(),
                     (_musicsCounter + _chunksCounter),
                     itemsSizeStr[3].c_str());
    LOGG("[Done]");
  } else {
    LOG_ON_SAME_LINE("\nRecursive search on %s ... ", _startDir.c_str());
    LOGR("[Failed]");
    LOG_ON_SAME_LINE("%s generation ... ",
                     ResourceFileHeader::getResourceBinName().c_str());
    LOGR("[Failed]");
    LOG_ON_SAME_LINE("%s generation ... ",
                     ResourceFileHeader::getFontBinName().c_str());
    LOGR("[Failed]");
    LOG_ON_SAME_LINE("%s generation ... ",
                     ResourceFileHeader::getSoundBinName().c_str());
    LOGR("[Failed]");
  }
  LOG("=======================================");

  // reset internal variables on both success and failure
  resetInternals();

  return err;
}

int32_t ResourceParser::setupResourceTree() {
  // files above are located in the build directory
  std::string projectAbsBuildFilePath = _projectAbsFilePath;
  projectAbsBuildFilePath.append("build/");

  std::string resourcesFolder = projectAbsBuildFilePath;
  resourcesFolder.append(_projectFolder).append("/").append(
      ResourceFileHeader::getResourcesBinFolderName());

  if (!FileSystemUtils::isDirectoryPresent(resourcesFolder)) {
    if (EXIT_SUCCESS !=
        FileSystemUtils::createDirectoryRecursive(resourcesFolder)) {
      LOGERR("createDirectoryRecursive() failed for '%s'",
          resourcesFolder.c_str());
      return EXIT_FAILURE;
    }
  }

  resourcesFolder.append("/");
  const std::string resFile =
      resourcesFolder + ResourceFileHeader::getResourceBinName();
  const std::string fontFile =
      resourcesFolder + ResourceFileHeader::getFontBinName();
  const std::string soundFile =
      resourcesFolder + ResourceFileHeader::getSoundBinName();

  if (EXIT_SUCCESS !=
      _fileBuilder.openCombinedStreams(resFile, fontFile, soundFile)) {
    LOGERR("Error in _fileBuilder.openCombinedStreams()");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int32_t ResourceParser::processAllFiles() {
  const std::vector<std::string> blackListFolders { "build" };
  std::vector<std::string> files;

  const int32_t res = FileSystemUtils::getAllFilesInDirectoryRecursively(
      _startDir, blackListFolders, files);
  if (EXIT_FAILURE == res) {
    LOGERR("getAllFilesInDirectoryRecursively() failed");
    return EXIT_FAILURE;
  }

  for (const auto& fileName : files) {
    // Skip file if it's not resource file
    if (!isResourceFile(fileName)) {
      continue;
    }
    _currAbsFilePath = fileName;

    if (EXIT_SUCCESS != buildResourceFile()) {
      LOGERR("Error in buildResourceFile() for %s.", _currAbsFilePath.c_str());
      LOGR("Cancelling parsing for next files");
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}

bool ResourceParser::isResourceFile(const std::string& fileName) const {
  bool result = false;

  // check wheter file has .rsrc extension
  const size_t pos = fileName.rfind(".rsrc");

  // file is found
  if (std::string::npos != pos) {
    result = true;
  }

  return result;
}

int32_t ResourceParser::openSourceStream(const std::string& sourceFileName) {
  int32_t err = EXIT_SUCCESS;

  // open fileStream for read
  _sourceStream.open(sourceFileName.c_str(),
                     std::ifstream::in | std::ifstream::binary);

  if (!_sourceStream) {
    LOGERR("Error, could not open ifstream for fileName: %s, reason: %s",
           sourceFileName.c_str(), strerror(errno));

    err = EXIT_FAILURE;
  }

  return err;
}

void ResourceParser::closeSourceStream() {
  // close the stream
  _sourceStream.close();

  // reset stream flags since we will be reusing it for other files
  _sourceStream.clear();
}

int32_t ResourceParser::buildResourceFile() {
  LOG_ON_SAME_LINE("Parsing %s ... ", _currAbsFilePath.c_str());

  int32_t err = buildResFileInternalData();

  if (EXIT_SUCCESS != err) {
    LOGERR(
        "Error in buildResourceFile(), Resource file from %s "
        "could not be created",
        _currAbsFilePath.c_str());
  }

  if (EXIT_SUCCESS == err) {
    err = openSourceStream(_currAbsFilePath);

    if (EXIT_SUCCESS != err) {
      LOGERR(
          "Error in openSourceStream(), Resource file from %s "
          "could not be created",
          _currAbsFilePath.c_str());
    }
  }

  if (EXIT_SUCCESS == err) {
    err = parseFileData();

    if (EXIT_SUCCESS != err) {
      LOGERR("Error in parseFileData() for %s", _currAbsFilePath.c_str());
    }
  }

  if (EXIT_SUCCESS == err) {
    err = _fileBuilder.openDestStreams();

    if (EXIT_SUCCESS != err) {
      LOGERR("Error in openDestStream() for %s", _currDestFile.c_str());
    }
  }

  if (EXIT_SUCCESS == err) {
    // whole .rsrc file is parsed -> write its data
    _fileBuilder.writeData(_fileData);

    LOGG("[Done]");
  } else {
    LOG_ON_SAME_LINE("Parsing of %s ... ", _currAbsFilePath.c_str());
    LOGR("[Failed]");
  }

  // close streams on both success or failure
  closeSourceStream();
  _fileBuilder.closeDestStream();
  _fileParser.resetRelativeFolderPath();

  return err;
}

int32_t ResourceParser::buildResFileInternalData() {
  int32_t err = EXIT_SUCCESS;

  uint64_t prjPathStartIdx = 0;
  uint64_t prjPathEndIdx = 0;

  // locate dot index so we can substring the instance name
  const uint64_t dotPos = _currAbsFilePath.find(".");

  // dotPos not found
  if (std::string::npos == dotPos) {
    LOGERR("Internal error. Resource file from %s could not be created",
        _currAbsFilePath.c_str());
    return EXIT_FAILURE;
  }
  const std::string absFileName = _currAbsFilePath.substr(0, dotPos);
  //filename with no extension
  const std::string fileName =
      FileSystemUtils::getFileNameFromAbsolutePath(absFileName);

  prjPathStartIdx = _currAbsFilePath.find(_projectFolder);
  if (prjPathStartIdx == std::string::npos) {
    LOGERR("Internal error. Resource file from %s could not be created",
        _currAbsFilePath.c_str());
    return EXIT_FAILURE;
  }

  prjPathStartIdx += _projectFolder.size() + 1;

  prjPathEndIdx = _currAbsFilePath.find(fileName);

  if (std::string::npos == prjPathEndIdx) {
    LOGERR("Internal error. Resource file from %s could not be created",
        _currAbsFilePath.c_str());
    return EXIT_FAILURE;
  }

  // get project path
  const std::string PROJECT_PATH =
      _currAbsFilePath.substr(prjPathStartIdx,  // start index
                              prjPathEndIdx - prjPathStartIdx);  // size

  // remember relative folder path before we append *game*_resources_h_
  _fileParser.setRelativeFolderPath(PROJECT_PATH);

  _currHeaderGuard = PROJECT_PATH;
  _currHeaderGuard.append(absFileName);
  _currHeaderGuard.append("RESOURCES_H_");

  const uint64_t headerGuardSize = _currHeaderGuard.size();
  for (uint64_t i = 0; i < headerGuardSize; ++i) {
    if (isalpha(_currHeaderGuard[i])) {
      _currHeaderGuard[i] =
          static_cast<char>(std::toupper(_currHeaderGuard[i]));
    } else if (_currHeaderGuard[i] == '/') {
      _currHeaderGuard[i] = '_';
    }
  }

  _currDestFile = absFileName;
  _currDestFile.append("Resources");

  _currNamespace = fileName;
  _currNamespace.append("Resources");

  _fileBuilder.setNamespace(_currNamespace);
  _fileBuilder.setDestFileName(_currDestFile);
  _fileBuilder.setHeaderGuards(_currHeaderGuard);

  return err;
}

int32_t ResourceParser::parseFileData() {
  _syntaxChecker.reset();

  std::string lineData = "";
  std::string rowData = "";
  int32_t eventCode = 0;
  int32_t parsedRowNumber = 0;

  CombinedData combinedData;

  // clear resources from previous parsed file
  _fileData.clear();

  while (std::getline(_sourceStream, lineData)) {
    ++parsedRowNumber;

    if (lineData.size() == 0) { // it is empty line -> skip it
      continue;
    } else if (lineData[0] == '#') { // it is comment line -> skip it
      continue;
    } else if (_syntaxChecker.hasValidTag(lineData)) {
      if (EXIT_SUCCESS !=
          _syntaxChecker.extractRowData(lineData, &rowData, &eventCode)) {
        LOGERR("Error in extractRowData()");
        return EXIT_FAILURE;
      }

      if (EXIT_SUCCESS != setSingleRowData(rowData, eventCode, &combinedData)) {
        LOGERR("Error in setSingleRowData()");
        return EXIT_FAILURE;
      }
    } else {
      LOGERR(
          "Internal error occurred on line: %d. Canceling parsing for %s",
          parsedRowNumber, _currAbsFilePath.c_str());
      return EXIT_FAILURE;
    }

    _syntaxChecker.updateOrder();

    if (_syntaxChecker.isChunkReady()) {
      // accumulate only TextureLoadType::ON_INIT widgets!
      if (_fileParser.isGraphicalFile() &&
          (ResourceDefines::TextureLoadType::ON_INIT ==
           combinedData.textureLoadType)) {
        ++_staticWidgetsCounter;
        _staticResFileTotalSize += combinedData.header.fileSize;
      } else {
        ++_dynamicWidgetsCounter;
        _dynamicResFileTotalSize += combinedData.header.fileSize;
      }

      _fileData.emplace_back(combinedData);
      combinedData.reset();
      _fileParser.closeFileAndReset();
    }
  }

  if (_fileData.empty()) {
    LOGERR("Configuration not complete for %s", _currAbsFilePath.c_str());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int32_t ResourceParser::setSingleRowData(const std::string& rowData,
                                         const int32_t eventCode,
                                         CombinedData* outData) {
  int32_t err = EXIT_SUCCESS;

  switch (eventCode) {
    case ResourceDefines::Field::TAG:
      // get rid of the "[ ]" brackets
      outData->tagName = rowData.substr(1, rowData.size() - 2);
      break;

    case ResourceDefines::Field::TYPE:
      outData->type = rowData;
      _syntaxChecker.setFieldTypeFromString(rowData);
      break;

    case ResourceDefines::Field::PATH:
      err = fillPath(rowData, outData);
      if (EXIT_SUCCESS != err) {
        LOGERR("Error in fillPath()");
      }
      break;

    case ResourceDefines::Field::DESCRIPTION:
      err = fillDescription(rowData, outData);
      if (EXIT_SUCCESS != err) {
        LOGERR("Error in fillDescription()");
      }
      break;

    case ResourceDefines::Field::POSITION:
      err = setImagePosition(rowData, outData);
      if (EXIT_SUCCESS != err) {
        LOGERR("Error in setImagePosition()");
      }
      break;

    case ResourceDefines::Field::LOAD:
      err = setTextureLoadType(rowData, outData);
      if (EXIT_SUCCESS != err) {
        LOGERR("Error in setImagePosition()");
      }
      break;

    default:
      LOGERR("Error, invalid enum value %d", eventCode);

      err = EXIT_FAILURE;
      break;
  }

  return err;
}

int32_t ResourceParser::fillPath(const std::string& rowData,
                                 CombinedData* outData) {
  int32_t err = EXIT_SUCCESS;

  if (std::string::npos == rowData.find(EXTERNAL_PATH_PREFIX)) {
    // use local folder hierarchy
    // Example: p/images/reel.png
    _fileParser.setRelativeFilePath(rowData);
  } else {
    // use non-local file placement (file outside the local folder)
    // Example: commonresources/p/attendantmenu/add_button.png
    _fileParser.setCompleteFilePathFromProject(rowData.substr(
        EXTERNAL_PATH_PREFIX_SIZE, rowData.size() - EXTERNAL_PATH_PREFIX_SIZE));
  }

  if (EXIT_SUCCESS != _fileParser.openFile()) {
    LOGERR("Error in _fileParser.openFile()");

    err = EXIT_FAILURE;
  } else  // EXIT_SUCCESS == err
  {
    outData->header.fileSize = _fileParser.getFileSizeInKiloBytes();

    if (_fileParser.isSupportedExtension()) {
      outData->header.path = _fileParser.getAbsoluteFilePath();

      // calculate hash value from resource string location
      outData->header.hashValue = hashFunction(outData->header.path);

      if (_fileParser.isGraphicalFile()) {
        _fileParser.getImageDimension(&outData->imageRect.w,
                                      &outData->imageRect.h);
      }
    }
  }

  return err;
}

int32_t ResourceParser::fillDescription(const std::string& rowData,
                                        CombinedData* outData) {
  int32_t err = EXIT_SUCCESS;

  switch (_syntaxChecker.getFieldType()) {
    case ResourceDefines::FieldType::IMAGE:
      outData->spriteData.emplace_back(0,                      // x
                                       0,                      // y
                                       outData->imageRect.w,   // w
                                       outData->imageRect.h);  // h
      break;

    case ResourceDefines::FieldType::SPRITE: {
      // reserve 4 slots for description parameters
      std::vector<int32_t> spriteDescription;
      const uint32_t SPRITE_DATA_SIZE = 4;

      if (EXIT_SUCCESS !=
          StringUtils::extractIntsFromString(rowData, " ,", &spriteDescription,
                                             SPRITE_DATA_SIZE)) {
        LOGERR(
            "Error in extractIntsFromString() "
            "for data: [%s], delimiters: [ ,], maxNumbers: %d",
            rowData.c_str(), SPRITE_DATA_SIZE);

        err = EXIT_FAILURE;
      }

      if (EXIT_SUCCESS == err) {
        _fileParser.setSpriteDescription(spriteDescription);
        ResourceDefines::SpriteLayout spriteLayout =
            ResourceDefines::SpriteLayout::UNKNOWN;

        if (_fileParser.isValidSpriteDescription(&spriteLayout)) {
          if (EXIT_SUCCESS !=
              _fileParser.fillSpriteData(spriteLayout, &outData->spriteData)) {
            LOGERR("Error in _fileParser.fillSpriteData()");

            err = EXIT_FAILURE;
          }
        } else {
          LOGERR(
              "Error wrong description for .rsrc file: %s, "
              "with tag: %s",
              _currAbsFilePath.c_str(), outData->tagName.c_str());

          err = EXIT_FAILURE;
        }
      }
    } break;

    case ResourceDefines::FieldType::SPRITE_MANUAL: {
      // reserve 4 slots for description parameters
      std::vector<int32_t> spriteDescription;
      const uint32_t SPRITE_DATA_SIZE = 4;

      if (EXIT_SUCCESS !=
          StringUtils::extractIntsFromString(rowData, " ,", &spriteDescription,
                                             SPRITE_DATA_SIZE)) {
        LOGERR(
            "Error in extractIntsFromString() "
            "for data: [%s], delimiters: [ ,], maxNumbers: %d",
            rowData.c_str(), SPRITE_DATA_SIZE);

        err = EXIT_FAILURE;
      }

      if (EXIT_SUCCESS == err) {
        _fileParser.setSpriteDescription(spriteDescription);

        if (_fileParser.isValidSpriteManualDescription()) {
          outData->spriteData.emplace_back(spriteDescription[0],   // x
                                           spriteDescription[1],   // y
                                           spriteDescription[2],   // w
                                           spriteDescription[3]);  // h
        } else {
          LOGERR(
              "Error wrong description for .rsrc file: %s, "
              "with tag: %s",
              _currAbsFilePath.c_str(), outData->tagName.c_str());

          err = EXIT_FAILURE;
        }
      }
    } break;

    case ResourceDefines::FieldType::FONT:
      outData->fontSize = StringUtils::safeStoi(rowData);

      ++_fontsCounter;
      _fontFileTotalSize += outData->header.fileSize;
      break;

    case ResourceDefines::FieldType::SOUND: {
      std::vector<std::string> tokens;
      const uint32_t MAX_TOKEN_SIZE = 2;
      StringUtils::tokenize(rowData, ", ", &tokens, MAX_TOKEN_SIZE);

      if (MAX_TOKEN_SIZE != tokens.size()) {
        LOGERR(
            "Error wrong description for .rsrc file: %s, "
            "with tag: %s",
            _currAbsFilePath.c_str(), outData->tagName.c_str());

        err = EXIT_FAILURE;
      }

      if (EXIT_SUCCESS == err) {
        if ("chunk" == tokens[0]) {
          ++_chunksCounter;
          outData->soundType = tokens[0];
        } else if ("music" == tokens[0]) {
          ++_musicsCounter;
          outData->soundType = tokens[0];
        } else {
          LOGERR(
              "Error wrong description for .rsrc file: %s, with "
              "tag: %s. First argument must be 'music' or 'chunk'",
              _currAbsFilePath.c_str(), outData->tagName.c_str());

          err = EXIT_FAILURE;
        }
      }

      if (EXIT_SUCCESS == err) {
        if ("low" == tokens[1] || "medium" == tokens[1] ||
            "high" == tokens[1] || "very_high" == tokens[1]) {
          outData->soundLevel = tokens[1];
          _soundFileTotalSize += outData->header.fileSize;
        } else {
          LOGERR(
              "Error wrong description for .rsrc file: %s, with "
              "tag: %s. Second argument 'sound level' must be "
              "'low', 'medium', 'high' or 'very_high'",
              _currAbsFilePath.c_str(), outData->tagName.c_str());

          err = EXIT_FAILURE;
        }
      }
    } break;

    default:
      LOGERR("Internal error, unknown CombinedData.type : %s",
             outData->type.c_str());

      err = EXIT_FAILURE;
      break;
  }

  return err;
}

int32_t ResourceParser::setImagePosition(const std::string& rowData,
                                         CombinedData* outData) {
  int32_t err = EXIT_SUCCESS;

  std::vector<int32_t> data;
  const uint32_t DATA_SIZE = 2;

  if (EXIT_SUCCESS ==
      StringUtils::extractIntsFromString(rowData, " ,", &data, DATA_SIZE)) {
    outData->imageRect.x = data[0];
    outData->imageRect.y = data[1];
  } else {
    LOGERR("Error in extractIntsFromString() for data: %s, maxNumbers: %d",
           rowData.c_str(), DATA_SIZE);

    err = EXIT_FAILURE;
  }

  return err;
}

int32_t ResourceParser::setTextureLoadType(const std::string& rowData,
                                           CombinedData* outData) {
  int32_t err = EXIT_SUCCESS;

  if ("on_init" == rowData) {
    outData->textureLoadType = ResourceDefines::TextureLoadType::ON_INIT;
  } else if ("on_demand" == rowData) {
    outData->textureLoadType = ResourceDefines::TextureLoadType::ON_DEMAND;
  } else {
    LOGERR(
        "Error wrong description for .rsrc file: %s, with tag: "
        "%s. Second argument must be 'on_init' or 'on_demand'",
        _currAbsFilePath.c_str(), outData->tagName.c_str());

    err = EXIT_FAILURE;
  }

  return err;
}

void ResourceParser::resetInternals() {
  _startDir = "Not set";
  _currAbsFilePath = "Not set";
  _currDestFile = "Not set";
  _currHeaderGuard = "Not set";
  _currNamespace = "Not set";
  _staticWidgetsCounter = 0;
  _dynamicWidgetsCounter = 0;
  _fontsCounter = 0;
  _musicsCounter = 0;
  _chunksCounter = 0;
  _staticResFileTotalSize = 0;
  _dynamicResFileTotalSize = 0;
  _fontFileTotalSize = 0;
  _soundFileTotalSize = 0;

  _syntaxChecker.reset();
  _fileData.clear();
}
