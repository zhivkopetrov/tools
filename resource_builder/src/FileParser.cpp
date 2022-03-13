// Corresponding header
#include "resource_builder/FileParser.h"

// System headers
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>

// Other libraries headers
#include "utils/data_type/EnumClassUtils.h"
#include "utils/drawing/Rectangle.h"
#include "utils/ErrorCode.h"
#include "utils/Log.h"

// Own components headers

FileParser::FileParser()
    : _absoluteProjectPath("Not set"),
      _relativeFolderPath("Not set"),
      _relativeFilePath("Not set"),
      _absoluteFilePath("Not set"),
      _pngHeader(),
      _spriteDes(nullptr),
      _imageWidth(0),
      _imageHeight(0),
      _fileSize(0),
      _currFileType(FileType::UNKNOWN),
      _isGraphicalFile(false) {
  _pngHeader = {0x89, 'P',  'N',  'G',  0x0D, 0x0A, 0x1A, 0x0A,
                0x00, 0x00, 0x00, 0x00, 'I',  'H',  'D',  'R'};

  _gifHeader = {'G', 'I', 'F'};

  _jpgHeader = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x00, 'J', 'F', 'I', 'F'};
}

FileParser::~FileParser() noexcept { closeFileAndReset(); }

void FileParser::setCompleteFilePathFromProject(
    const std::string& relativeFilePath) {
  _absoluteFilePath = _absoluteProjectPath;
  _absoluteFilePath.append(relativeFilePath);

  setFileTypeInternal();
}

void FileParser::setRelativeFilePath(const std::string& relativeFilePath) {
  _relativeFilePath = relativeFilePath;
  buildAbsoluteFilePath();
  setFileTypeInternal();
}

ErrorCode FileParser::openFile() {
  _fileStream.open(_absoluteFilePath.c_str(),
                   std::ifstream::in | std::ifstream::binary);

  if (!_fileStream) {
    LOGERR("Error, could not open ifstream for fileName: %s, reason: %s",
           _absoluteFilePath.c_str(), strerror(errno));
    return ErrorCode::FAILURE;
  }

  readFileSize();

  return ErrorCode::SUCCESS;
}

void FileParser::closeFileAndReset() {
  _spriteDes = nullptr;
  _isGraphicalFile = false;

  _imageWidth = 0;
  _imageHeight = 0;
  _fileSize = 0;

  // close stream and clear stream flags
  _fileStream.close();
  _fileStream.clear();
}

bool FileParser::isSupportedExtension() {
  bool success = false;

  switch (_currFileType) {
    case FileType::PNG:
      success = isValidPngFile();
      break;

    case FileType::JPG:
      success = isValidJpgFile();
      break;

    case FileType::GIF:
      success = isValidGifFile();
      break;

    case FileType::OTF:
      success = true;  // no validation on this stage for .otf extension
      break;

    case FileType::TTF:
      success = true;  // no validation on this stage for .ttf extension
      break;

    case FileType::WAV:
      success = true;  // no validation on this stage for .wav extension
      break;

    case FileType::OGG:
      success = true;  // no validation on this stage for .ogg extension
      break;

    default:
      LOGERR("Unknown file extension on file: %s", _absoluteFilePath.c_str());
      break;
  }

  if (!success) {
    LOGERR("Error, %s is not in any valid format", _absoluteFilePath.c_str());
  }

  return success;
}

void FileParser::buildAbsoluteFilePath() {
  _absoluteFilePath = _absoluteProjectPath;
  if (!_absoluteFilePath.empty() && _absoluteFilePath.back() != '/') {
    _absoluteFilePath.push_back('/');
  }
  _absoluteFilePath.append(_relativeFolderPath);
  if (!_relativeFolderPath.empty() && _relativeFolderPath.back() != '/') {
    _relativeFolderPath.push_back('/');
  }
  _absoluteFilePath.append(_relativeFilePath);
}

void FileParser::setFileTypeInternal() {
  const uint8_t MAX_EXTENSION_SIZE = 3;
  const std::string extension = _absoluteFilePath.substr(
      _absoluteFilePath.size() - MAX_EXTENSION_SIZE, MAX_EXTENSION_SIZE);
  if (MAX_EXTENSION_SIZE < extension.size()) {
    _currFileType = FileType::UNKNOWN;
    _isGraphicalFile = false;
    return;
  }

  if ("png" == extension) {
    _currFileType = FileType::PNG;
    _isGraphicalFile = true;
  } else if ("jpg" == extension) {
    _currFileType = FileType::JPG;
    _isGraphicalFile = true;
  } else if ("gif" == extension) {
    _currFileType = FileType::GIF;
    _isGraphicalFile = true;
  } else if ("otf" == extension) {
    _currFileType = FileType::OTF;
    _isGraphicalFile = true;
  } else if ("ttf" == extension) {
    _currFileType = FileType::TTF;
    _isGraphicalFile = true;
  } else if ("wav" == extension) {
    _currFileType = FileType::WAV;
    _isGraphicalFile = false;
  } else if ("ogg" == extension) {
    _currFileType = FileType::OGG;
    _isGraphicalFile = false;
  } else {
    _currFileType = FileType::UNKNOWN;
    _isGraphicalFile = false;
  }
}

bool FileParser::isValidPngFile() {
  bool success = true;

  const uint8_t ONE_BYTE_SIZE = sizeof(uint64_t);
  uint8_t* singleByte = nullptr;
  uint64_t header8B = 0;

  if (!_fileStream)  // sanity check
  {
    LOGERR("Internal error, ifstream for %s not opened",
           _absoluteFilePath.c_str());
    success = false;
  }

  if (true == success) {
    // reading PNG dimensions requires the first 24 bytes of the file
    const int64_t PNG_HEADER_SIZE = 24;

    // Check if file has enough bytes for header
    if (_fileSize < PNG_HEADER_SIZE) {
      LOGERR(
          "Warning, file: %s is too small: %ld and has incomplete png "
          "header",
          _absoluteFilePath.c_str(), _fileSize);
      success = false;
    }
  }

  if (true == success) {
    // return file pointer to beginning
    _fileStream.seekg(0, std::ifstream::beg);

    _fileStream.read(reinterpret_cast<char*>(&header8B), sizeof(header8B));

    // point singleByte to header8B's first byte
    singleByte = reinterpret_cast<uint8_t*>(&header8B);

    for (uint8_t i = 0; i < ONE_BYTE_SIZE; ++i) {
      // check first 8 bytes of the pngHeader
      if (_pngHeader[i] != *singleByte) {
        success = false;
        break;
      }

      // increment 1 byte
      ++singleByte;
    }
  }

  if (true == success) {
    const uint8_t ONE_AND_A_HALF_BYTES_SIZE =
        ONE_BYTE_SIZE + (ONE_BYTE_SIZE / 2);
    const uint8_t TWO_BYTE_SIZE = 2 * ONE_BYTE_SIZE;

    // now check second 8 bytes of the header
    _fileStream.read(reinterpret_cast<char*>(&header8B), sizeof(header8B));
    singleByte = reinterpret_cast<uint8_t*>(&header8B);

    // we are only interested in the second half of the those 8 bytes
    // this means singleByte is starting from 5th bit
    singleByte += sizeof(uint32_t);
    for (uint8_t i = ONE_AND_A_HALF_BYTES_SIZE; i < TWO_BYTE_SIZE; ++i) {
      if (_pngHeader[i] != *singleByte) {
        success = false;
        break;
      }
      ++singleByte;
    }
  }

  if (true == success) {
    _fileStream.read(reinterpret_cast<char*>(&_imageWidth),
                     sizeof(_imageWidth));
    _fileStream.read(reinterpret_cast<char*>(&_imageHeight),
                     sizeof(_imageHeight));

    // width and height are guaranteed by the standard to be in big endianess
    _imageWidth = ntohl(_imageWidth);
    _imageHeight = ntohl(_imageHeight);
  }

  return success;
}

bool FileParser::isValidGifFile() {
  bool success = true;

  if (!_fileStream)  // sanity check
  {
    LOGERR("Internal error, ifstream for %s not opened",
           _absoluteFilePath.c_str());
    success = false;
  }

  if (true == success) {
    // reading GIF dimensions requires the first 10 bytes of the file
    const int64_t GIF_HEADER_SIZE = 10;

    // Check if file has enough bytes for header
    if (_fileSize < GIF_HEADER_SIZE) {
      LOGERR(
          "Warning, file: %s is too small: %ld and has incomplete gif "
          "header",
          _absoluteFilePath.c_str(), _fileSize);
      success = false;
    }
  }

  if (true == success) {
    // return file pointer to beginning
    _fileStream.seekg(0, std::ifstream::beg);

    // read half of the header -> 4 bytes
    uint32_t header4B = 0;
    _fileStream.read(reinterpret_cast<char*>(&header4B), sizeof(header4B));

    // point singleByte to header4B's first byte
    uint8_t* singleByte = reinterpret_cast<uint8_t*>(&header4B);

    const size_t SIZE = _gifHeader.size();

    for (size_t i = 0; i < SIZE; ++i) {
      // check first 8 bytes of the pngHeader
      if (_gifHeader[i] != *singleByte) {
        success = false;
        break;
      }

      // increment 1 byte
      ++singleByte;
    }
  }

  if (true == success) {
    uint16_t unused2B = 0;
    // read 2 empty bytes
    _fileStream.read(reinterpret_cast<char*>(&unused2B), sizeof(unused2B));

    uint16_t gifWidth = 0;
    uint16_t gifHeight = 0;
    _fileStream.read(reinterpret_cast<char*>(&gifWidth), sizeof(gifWidth));
    _fileStream.read(reinterpret_cast<char*>(&gifHeight), sizeof(gifHeight));

    _imageWidth = static_cast<int32_t>(gifWidth);
    _imageHeight = static_cast<int32_t>(gifHeight);
  }

  return success;
}

bool FileParser::isValidJpgFile() {
  bool success = true;
  uint8_t* singleByte = nullptr;
  uint32_t header4B = 0;

  if (!_fileStream)  // sanity check
  {
    LOGERR("Internal error, ifstream for %s not opened",
           _absoluteFilePath.c_str());
    success = false;
  }

  if (true == success) {
    // reading JPG dimensions requires the first 24 bytes of the file
    const int64_t JPG_HEADER_SIZE = 24;

    // Check if file has enough bytes for header
    if (_fileSize < JPG_HEADER_SIZE) {
      LOGERR(
          "Warning, file: %s is too small: %ld and has incomplete jpg"
          " header",
          _absoluteFilePath.c_str(), _fileSize);
      success = false;
    }
  }

  if (true == success) {
    // return file pointer to beginning
    _fileStream.seekg(0, std::ifstream::beg);

    _fileStream.read(reinterpret_cast<char*>(&header4B), sizeof(header4B));

    // point singleByte to header4B's first byte
    singleByte = reinterpret_cast<uint8_t*>(&header4B);

    const size_t HEADER_4B_SIZE = sizeof(header4B);

    for (size_t i = 0; i < HEADER_4B_SIZE; ++i) {
      // check first 8 bytes of the pngHeader
      if (_jpgHeader[i] != *singleByte) {
        success = false;
        break;
      }

      // increment 1 byte
      ++singleByte;
    }
  }

  if (true == success) {
    uint16_t unused2B = 0;
    // read 2 empty bytes
    _fileStream.read(reinterpret_cast<char*>(&unused2B), sizeof(unused2B));

    _fileStream.read(reinterpret_cast<char*>(&header4B), sizeof(header4B));

    // point singleByte to header8B's first byte
    singleByte = reinterpret_cast<uint8_t*>(&header4B);

    const size_t SIZE = _jpgHeader.size();
    const size_t START = sizeof(header4B) + sizeof(unused2B);

    for (size_t i = START; i < SIZE; ++i) {
      // check first 8 bytes of the pngHeader
      if (_jpgHeader[i] != *singleByte) {
        success = false;
        break;
      }

      // increment 1 byte
      ++singleByte;
    }
  }

  if (true == success) {
    // reading JPEG dimensions requires scanning through jpeg chunks
    // we need to read the first 12 bytes of each chunk.
    constexpr uint8_t CHUNK_SIZE = 12;
    int64_t pos = 0;
    uint8_t buf[CHUNK_SIZE] = {0};

    do {
      /** Since .jpg is a compressed format reads are not consequent ->
       * move file pointer to the new position
       * */
      _fileStream.seekg(pos, std::ifstream::beg);

      // read next consequent chunk
      _fileStream.read(reinterpret_cast<char*>(buf), CHUNK_SIZE);

      // internal jpg validation
      if (buf[3] == 0xC0 || buf[3] == 0xC1 || buf[3] == 0xC2 ||
          buf[3] == 0xC3 || buf[3] == 0xC9 || buf[3] == 0xCA ||
          buf[3] == 0xCB) {
        break;
      } else {
        // calculate new pos at every iteration
        pos += (2 + (buf[4] << 8) + buf[5]);

        // final chunk found -> stop the search
        if (pos + CHUNK_SIZE > _fileSize) {
          break;
        }
      }
    } while (0xFF == buf[2]);

    _imageHeight = (buf[7] << 8) + buf[8];
    _imageWidth = (buf[9] << 8) + buf[10];
  }

  return success;
}

bool FileParser::isValidSpriteDescription(
    ResourceDefines::SpriteLayout& outLayout) {
  bool success = true;
  int32_t totalWidth = 0;
  int32_t totalHeight = 0;

  //_spriteDescription has format:
  // singleSpriteX, singleSpriteY, numberOfSprites, horizontalOffset

  if (nullptr == _spriteDes) {
    outLayout = ResourceDefines::SpriteLayout::UNKNOWN;
    LOGERR("Error, _spriteDes is not set!");
    return false;
  }

  if (4 != _spriteDes->size())  // sanity check
  {
    outLayout = ResourceDefines::SpriteLayout::UNKNOWN;
    LOGERR("Internal error. _spriteDes.->size() is not == 4!");
    return false;
  }

  std::vector<int32_t>& spriteDesRef = *_spriteDes;

  // First check if it's a valid horizontal sprite layout
  //___________
  //|1|2|3|4|5|
  //-----------

  totalWidth = (spriteDesRef[ResourceDefines::SPRITE_NUMBER_IDX] *
                (spriteDesRef[ResourceDefines::WIDTH_IDX] +
                 spriteDesRef[ResourceDefines::OFFSET_IDX])) -
               spriteDesRef[ResourceDefines::OFFSET_IDX];

  if (_imageWidth < totalWidth) {
    outLayout = ResourceDefines::SpriteLayout::UNKNOWN;
    success = false;
  } else {
    outLayout = ResourceDefines::SpriteLayout::HORIZONTAL;
    success = true;
  }

  // if first check succeeded we do not need to check other sprite layouts
  if (false == success) {
    // Second check if it's a valid vertical sprite layout
    //___
    //|1|
    //|2|
    //|3|
    //|4|
    //|5|

    totalHeight = spriteDesRef[ResourceDefines::HEIGHT_IDX] *
                  spriteDesRef[ResourceDefines::SPRITE_NUMBER_IDX];

    if (_imageHeight < totalHeight) {
      outLayout = ResourceDefines::SpriteLayout::UNKNOWN;
      success = false;
    } else {
      outLayout = ResourceDefines::SpriteLayout::VERTICAL;
      success = true;
    }
  }

  if (false == success) {
    // Third check if it's a valid mixed layout
    //_______
    //|1|2|3|
    //|4|5| |
    //-------

    // this is single sprite width + offset
    //(the numbers 1-5 in the above picture)
    // e.g. "worst" case scenario
    // example: spriteData: 200, 100, 3, 20
    // CHUNK = 200 + 20 = 220
    const int32_t CHUNK = spriteDesRef[ResourceDefines::WIDTH_IDX] +
                          spriteDesRef[ResourceDefines::OFFSET_IDX];

    // example: CHUNKS 1, 2 and 3 in the above picture
    const int32_t CHUNKS_PER_ROW = _imageWidth / CHUNK;

    // example: ROWS_PER_IMAGE = 2 in the above picture
    const int32_t ROWS_PER_IMAGE =
        _imageHeight / spriteDesRef[ResourceDefines::HEIGHT_IDX];

    if (CHUNKS_PER_ROW * ROWS_PER_IMAGE <
        spriteDesRef[ResourceDefines::SPRITE_NUMBER_IDX]) {
      outLayout = ResourceDefines::SpriteLayout::UNKNOWN;
      success = false;
    } else {
      outLayout = ResourceDefines::SpriteLayout::MIXED;
      success = true;
    }
  }

  return success;
}

bool FileParser::isValidSpriteManualDescription() {
  bool success = true;

  std::vector<int32_t>& spriteDesRef = *_spriteDes;

  if (0 > spriteDesRef[ResourceDefines::IMAGE_X_IDX]) {
    LOGERR("Error, negative value: %d provided for _spriteDes.x",
           spriteDesRef[ResourceDefines::IMAGE_X_IDX]);

    success = false;
  }

  if (0 > spriteDesRef[ResourceDefines::IMAGE_Y_IDX]) {
    LOGERR("Error, negative value: %d provided for _spriteDes.y",
           spriteDesRef[ResourceDefines::IMAGE_Y_IDX]);

    success = false;
  }

  if (0 > spriteDesRef[ResourceDefines::SPRITE_WIDTH_IDX]) {
    LOGERR("Error, negative value: %d provided for _spriteDes.w",
           spriteDesRef[ResourceDefines::SPRITE_WIDTH_IDX]);

    success = false;
  }

  if (0 > spriteDesRef[ResourceDefines::SPRITE_HEIGHT_IDX]) {
    LOGERR("Error, negative value: %d provided for _spriteDes.h",
           spriteDesRef[ResourceDefines::SPRITE_HEIGHT_IDX]);

    success = false;
  }

  if ((spriteDesRef[ResourceDefines::IMAGE_X_IDX] +
       spriteDesRef[ResourceDefines::SPRITE_WIDTH_IDX]) > _imageWidth) {
    LOGERR(
        "Error, out of bound sprite description provided: (x + w = %d), "
        "where _imageWidth: %d",
        (spriteDesRef[ResourceDefines::IMAGE_X_IDX] +
         spriteDesRef[ResourceDefines::SPRITE_WIDTH_IDX]),
        _imageWidth);

    success = false;
  }

  if ((spriteDesRef[ResourceDefines::IMAGE_Y_IDX] +
       spriteDesRef[ResourceDefines::SPRITE_HEIGHT_IDX]) > _imageHeight) {
    LOGERR(
        "Error, out of bound sprite description provided: (y + h = %d), "
        "where _imageHeight: %d",
        (spriteDesRef[ResourceDefines::IMAGE_Y_IDX] +
         spriteDesRef[ResourceDefines::SPRITE_HEIGHT_IDX]),
        _imageHeight);

    success = false;
  }

  return success;
}

ErrorCode FileParser::fillSpriteData(
    const ResourceDefines::SpriteLayout& layout,
    std::vector<Rectangle>& outData) {
  switch (layout) {
    case ResourceDefines::SpriteLayout::HORIZONTAL:
      setHorizontalSpriteLayout(outData);
      break;

    case ResourceDefines::SpriteLayout::VERTICAL:
      setVerticalSpriteLayout(outData);
      break;

    case ResourceDefines::SpriteLayout::MIXED:
      setMixedSpriteLayout(outData);
      break;

    default:
      LOGERR("Internal error, Invalid enum class value: %hhu",
             getEnumValue(layout));
      return ErrorCode::FAILURE;
  }

  return ErrorCode::SUCCESS;
}

void FileParser::setHorizontalSpriteLayout(std::vector<Rectangle>& outData) {
  std::vector<int32_t>& spriteDesRef = *_spriteDes;

  const int32_t SPRITE_COUNT = spriteDesRef[ResourceDefines::SPRITE_NUMBER_IDX];
  for (int32_t i = 0; i < SPRITE_COUNT; ++i) {
    outData.emplace_back(
        i * (spriteDesRef[ResourceDefines::WIDTH_IDX] +
             spriteDesRef[ResourceDefines::OFFSET_IDX]),  // x
        0,                                                // y
        spriteDesRef[ResourceDefines::WIDTH_IDX],         // w
        spriteDesRef[ResourceDefines::HEIGHT_IDX]);       // h
  }
}

void FileParser::setVerticalSpriteLayout(std::vector<Rectangle>& outData) {
  std::vector<int32_t>& spriteDesRef = *_spriteDes;

  const int32_t SPRITE_COUNT = spriteDesRef[ResourceDefines::SPRITE_NUMBER_IDX];
  for (int32_t i = 0; i < SPRITE_COUNT; ++i) {
    outData.emplace_back(0,                                              // x
                         i * spriteDesRef[ResourceDefines::HEIGHT_IDX],  // y
                         spriteDesRef[ResourceDefines::WIDTH_IDX],       // w
                         spriteDesRef[ResourceDefines::HEIGHT_IDX]);     // h
  }
}

void FileParser::setMixedSpriteLayout(std::vector<Rectangle>& outData) {
  std::vector<int32_t>& spriteDesRef = *_spriteDes;

  const int32_t CHUNK = spriteDesRef[ResourceDefines::WIDTH_IDX] +
                        spriteDesRef[ResourceDefines::OFFSET_IDX];

  // maximum described chunks in the .rsrc file
  const int32_t MAX_CHUNKS = spriteDesRef[ResourceDefines::SPRITE_NUMBER_IDX];

  // example: CHUNKS 1, 2 and 3 in the above picture
  const int32_t CHUNKS_PER_ROW = _imageWidth / CHUNK;

  // example: ROWS_PER_IMAGE = 2 in the above picture
  const int32_t ROWS_PER_IMAGE =
      _imageHeight / spriteDesRef[ResourceDefines::HEIGHT_IDX];

  int32_t currentChunk = 1;

  for (int32_t i = 0; i < ROWS_PER_IMAGE; ++i) {
    for (int32_t j = 0; j < CHUNKS_PER_ROW; ++j) {
      /** If developer has entered less than the maximum possible sprites
       * for example if the resource has some "empty" sprites at it's end
       * */
      if (MAX_CHUNKS < currentChunk) {
        /** Maximum entered chunks reached ->
         *                     do not process the remaining empty ones
         * */
        return;
      }

      ++currentChunk;

      outData.emplace_back(
          j * (spriteDesRef[ResourceDefines::WIDTH_IDX] +
               spriteDesRef[ResourceDefines::OFFSET_IDX]),  // x
          i * spriteDesRef[ResourceDefines::HEIGHT_IDX],    // y
          spriteDesRef[ResourceDefines::WIDTH_IDX],         // w
          spriteDesRef[ResourceDefines::HEIGHT_IDX]);       // h
    }
  }
}

void FileParser::readFileSize() {
  _fileStream.seekg(0, std::ifstream::beg);
  const std::streampos startPos = _fileStream.tellg();

  // Acquire file size
  _fileStream.seekg(0, std::ifstream::end);
  _fileSize = _fileStream.tellg() - startPos;
}
