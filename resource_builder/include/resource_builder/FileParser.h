#ifndef TOOLS_RESOURCE_BUILDER_INCLUDE_FILEPARSER_H_
#define TOOLS_RESOURCE_BUILDER_INCLUDE_FILEPARSER_H_

// System headers
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

// Other libraries headers
#include "resource_utils/defines/ResourceDefines.h"
#include "utils/ErrorCode.h"

// Own components headers

// Forward Declaration
struct Rectangle;

class FileParser {
 public:
  FileParser();
  ~FileParser() noexcept;

  /** In order to easily construct absolute file path
   *                                           the following is needed:
   *       > absolute project path;
   *       > relative folder path;
   *       > relative file path;
   * */

  /** @brief used to set absolute project path
   *
   *  @param const std::string & - project path
   * */
   void setAbsoluteProjectPath(const std::string& projectPath) {
    _absoluteProjectPath = projectPath;
  }

  /** @brief used to set relative folder path
   *
   *  @param const std::string & - relative folder path
   * */
   void setRelativeFolderPath(const std::string& folderPath) {
    _relativeFolderPath = folderPath;
  }

  /** @brief used to set relative image path
   *         NOTE: the function automatically builds up
   *               absolute image path since we have
   *                                       all the required components:
   *                                            > absolute project path;
   *                                            > relative folder path;
   *                                            > relative image path;
   *
   *  @param const std::string & - relative image path
   * */
  void setRelativeFilePath(const std::string& relativeFilePath);

  /** @brief used to set relative image path
   *         NOTE: the function automatically builds up
   *               absolute image path since we have
   *                   all the required components:
   *                          > absolute project path;
   *                          > relative image path from project folder;
   *
   *         NOTE 2: relative folder path is not used in this case;
   *
   *  @param const std::string & - relative image path
   * */
  void setCompleteFilePathFromProject(const std::string& relativeFilePath);

  /** @brief used to reset relative folder path
   * */
   void resetRelativeFolderPath() { _relativeFolderPath = "Not set"; }

   /** @brief used to obtain the value of the relative image path
    *
    *  @returns std::string - the relative image path
    * */
    std::string getRelativeFilePath() const { return _relativeFilePath; }

  /** @brief used to obtain the value of the absolute image path
   *
   *  @returns std::string - the absolute image path
   * */
   std::string getAbsoluteFilePath() const { return _absoluteFilePath; }

  /** @brief used to acquire access to the sprite description vector
   *                                  for the currently processed image.
   *
   *  @param std::vector<int32_t> & - reference to sprite description
   * */
   void setSpriteDescription(std::vector<int32_t>& description) {
    _spriteDes = &description;
  }

  /** @brief used to acquire width and height from the
   *                                          currently processed image.
   *         NOTE: width and height data are read directly
   *                                          from the image header.
   *
   *  @param int32_t & - image width
   *  @param int32_t & - image height
   * */
   void getImageDimension(int32_t& outWidth, int32_t& outHeight) {
    outWidth = _imageWidth;
    outHeight = _imageHeight;
  }

  /** @brief used to open file descriptor
   *
   *  @returns ErrorCode - error code
   * */
   ErrorCode openFile();

  /** @brief used to close and reset the file description and
   *                                            reset internal variables.
   * */
  void closeFileAndReset();

  /** @brief used to determine whether file falls into
   *                                       familiar file extensions list.
   *         Currently supported extensions:
   *          - Graphical:
   *              > .jpg;
   *              > .png;
   *              > .gif
   *
   *          - Fonts:
   *              > .otf;
   *
   *          - Sounds:
   *              > .wav;
   *              > .ogg;
   * */
  bool isSupportedExtension();

  /** @brief used to determine whether correct sprite description was
   *                         provided in the .rsrc file "description" tag.
   *         Currently supported sprite layouts:
   *              > SpriteLayout::HORIZONTAL;
   *              > SpriteLayout::VECTICAL;
   *              > SpriteLayout::MIXED;
   *
   *  @param SpriteLayout & - out SpriteLayout.
   *                              On success the proper layout is set.
   *                              On failure SpriteLayout::UNKNOWN is set.
   * */
  bool isValidSpriteDescription(ResourceDefines::SpriteLayout& outLayout);

  /** @brief used to determine whether correct sprite manual description
   *                     was provided in the .rsrc file "description" tag.
   * */
  bool isValidSpriteManualDescription();

  /** @brief used to fill sprite description
   *                                 (every sprite Rectangle  dimensions)
   *                from the provided in the .rsrc file "description" tag.
   *
   *  @param const SpriteLayout &     - currently set SpriteLayout.
   *  @param std::vector<Rectangle> & - fully populated sprite description
   *
   *  @returns ErrorCode              - error code
   * */
  ErrorCode fillSpriteData(const ResourceDefines::SpriteLayout& layout,
                           std::vector<Rectangle>& outData);

  /** @brief used to determine whether the processed file is graphical one
   *                         e.g. with extensions .jpg, .png, .gif or not
   *
   *  @return bool - is graphical file or not
   * */
   bool isGraphicalFile() const { return _isGraphicalFile; }

  /** @brief used to acquire the file size in kBytes
   *
   *         NOTE: since we are returning file size in kBytes we choose
   *               int32_t over int64_t for faster calculations.
   *
   *  @return int32_t - file size in kBytes
   * */
   int32_t getFileSizeInKiloBytes() const {
    return static_cast<int32_t>(_fileSize / 1024);
  }

 private:
  /** @brief used to build up absolute file path from:
   *                                            > absolute project path;
   *                                            > relative folder path;
   *                                            > relative file path;
   *
   *         The function also sets _currFileType
   *                                      to the currently used extension.
   * */
  void buildAbsoluteFilePath();

  /** @brief used to set currently used file extension.
   * */
  void setFileTypeInternal();

  /** @brief used determine whether the provided *.png file is a valid.
   *         This is done by parsing the image header and comparing it
   *         to the standard .PNG header.
   * */
  bool isValidPngFile();

  /** @brief used determine whether the provided *.gif file is a valid.
   *         This is done by parsing the image header and comparing it
   *         to the standard .GIF header.
   * */
  bool isValidGifFile();

  /** @brief used determine whether the provided *.gif file is a valid.
   *         This is done by parsing the image header and comparing it
   *         to the standard .GIF header.
   * */
  bool isValidJpgFile();

  /** @brief used to fill sprite description as horizontal layout
   *
   *  @param std::vector<Rectangle>& outData - fully populated sprite
   *                                                          description
   * */
  void setHorizontalSpriteLayout(std::vector<Rectangle>& outData);

  /** @brief used to fill sprite description as vertical layout
   *
   *  @param std::vector<Rectangle> & outData - fully populated sprite
   *                                                          description
   * */
  void setVerticalSpriteLayout(std::vector<Rectangle>& outData);

  /** @brief used to fill sprite description as mixed layout
   *
   *  @param std::vector<Rectangle> & outData - fully populated sprite
   *                                                          description
   * */
  void setMixedSpriteLayout(std::vector<Rectangle>& outData);

  /** @brief used to read the file size
   * */
  void readFileSize();

  std::string _absoluteProjectPath;
  std::string _relativeFolderPath;
  std::string _relativeFilePath;
  std::string _absoluteFilePath;

  std::ifstream _fileStream;

  /* A vector that holds the standard .PNG header bytes. It is used
   * to validate other .PNG files */
  std::vector<uint8_t> _pngHeader;

  /* A vector that holds the standard .PNG header bytes. It is used
   * to validate other .GIF files */
  std::vector<uint8_t> _gifHeader;

  /* A vector that holds the standard .JPG header bytes. It is used
   * to validate other .JPG files */
  std::vector<uint8_t> _jpgHeader;

  /** A pointer to fully populated sprite data description that is parsed
   *  from the individual .rsrc files. In order not to copy the data
   *  when it's not needed -> a pointer is used.
   * */
  std::vector<int32_t>* _spriteDes;

  /* Image dimensions parsed from the image header */
  int32_t _imageWidth;
  int32_t _imageHeight;

  /* Holds the current file size in bytes */
  int64_t _fileSize;

  enum class FileType : uint8_t {
    // Graphical
    PNG = 0,
    JPG,
    GIF,

    // Fonts
    OTF,
    TTF,

    // Sounds
    WAV,
    OGG,

    UNKNOWN = 255
  };

  FileType _currFileType;

  /* Used to determine whether the processed file is graphical one -
   * e.g. with extensions .jpg, .png, .gif or not
   * */
  bool _isGraphicalFile;
};

#endif /* TOOLS_RESOURCE_BUILDER_INCLUDE_FILEPARSER_H_ */
