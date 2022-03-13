#ifndef TOOLS_RESOURCE_BUILDER_INCLUDE_RESOURCEPARSER_H_
#define TOOLS_RESOURCE_BUILDER_INCLUDE_RESOURCEPARSER_H_

// System headers
#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_set>

// Other libraries headers
#include "resource_utils/structs/CombinedStructs.h"
#include "utils/ErrorCode.h"

// Own components headers
#include "resource_builder/FileBuilder.h"
#include "resource_builder/FileParser.h"
#include "resource_builder/SyntaxChecker.h"

// Forward Declarations

class ResourceParser {
public:
  ResourceParser();

  /** @brief used to initialize the ResourceParser by:
   *              > location the project folder on the hard drive;
   *              > setting engine resource file and font font names;
   *              > open streams for those files;
   *
   *  @returns ErrorCode - error code
   * */
  ErrorCode init();

  /** @brief used to parse project tree directory recursively, search
   *                                      for .rsrc files and parse them.
   *
   *  @param const std::string & - project to parse
   *
   *  @returns ErrorCode - error code
   * */
  ErrorCode parseResourceTree(const std::string &projectName);

private:
  /** @brief used to setup project tree directory for parsing.
   *
   *  @returns ErrorCode - error code
   * */
  ErrorCode setupResourceTree();

  ErrorCode processAllFiles();

  /** @brief used to determine whether the selected file is a .rsrc file
   *
   *  @param const std::string & - directory name
   *
   *  @returns bool              - is .rsrs file or not
   * */
  bool isResourceFile(const std::string &fileName) const;

  /** @brief used to open file stream from file name
   *
   *  @const std::string & - file name
   *
   *  @returns ErrorCode   - error code
   * */
  ErrorCode openSourceStream(const std::string &sourceFileName);

  /** @bried used to close file stream
   * */
  void closeSourceStream();

  /** @bried used start the parsing of an individual .rsrc file
   *                                                  when such is found.
   *
   *  @returns ErrorCode - error code
   * */
  ErrorCode buildResourceFile();

  /** @bried used fill internal resource data from the .rsrc file name
   *                              such as namespace, header guards etc...
   *
   *  @returns ErrorCode - error code
   * */
  ErrorCode buildResFileInternalData();

  /** @bried used fill internal resource data from the .rsrc file
   *                                                that is being parsed.
   *
   *  @returns ErrorCode - error code
   * */
  ErrorCode parseFileData();

  /** @bried used fill individual field of data
   *                       from a parsed string line from the .rsrc file.
   *
   *  @param const std::string & - parsed row data
   *  @param const int32_t       - event code
   *              (in order to know how to manipulate the parsed row data)
   *  @param CombinedData &      - populated structure
   *
   *  @returns ErrorCode         - error code
   * */
  ErrorCode setSingleRowData(const std::string &rowData,
                             const int32_t eventCode, CombinedData &outData);

  /** @bried used fill CombinedData description when
   *                                             "path" tag is processed.
   *
   *  @param const std::string & - relative file path
   *  @param CombinedData &      - populated structure
   *
   *  @returns ErrorCode         - error code
   * */
  ErrorCode fillPath(const std::string &relativeFilePath,
                     CombinedData &outData);

  /** @bried used fill CombinedData description when
   *                                      "description" tag is processed.
   *
   *  @param const std::string & - parsed row data
   *  @param CombinedData &      - populated structure
   *
   *  @returns ErrorCode         - error code
   * */
  ErrorCode fillDescription(const std::string &rowData, CombinedData &outData);

  /** @brief used fill CombinedData description when
   *                                         "position" tag is processed.
   *
   *  @param const std::string & - parsed row data
   *  @param CombinedData &      - populated structure
   *
   *  @returns ErrorCode         - error code
   * */
  ErrorCode setImagePosition(const std::string &rowData, CombinedData &outData);

  /** @brief used fill CombinedData description when
   *                                         "load" tag is processed.
   *
   *  @param const std::string & - parsed row data
   *  @param CombinedData &      - populated structure
   *
   *  @returns ErrorCode         - error code
   * */
  ErrorCode setTextureLoadType(const std::string &rowData,
                               CombinedData &outData);

  void resetInternals();

  void finishParseResourceTreeLogReport(const ErrorCode errorCode);

  /** Temporary variables used to remember certain
   *                                   file states (names, paths, etc).
   *  Since we are using recursive approach when parsing
   *  the project directory -> the motivation behind those local variables
   *  is not to pass the all around to dozen of functions and create
   *  unnecessary overhead.
   * */
  std::string _projectAbsFilePath;
  std::string _projectFolder;
  std::string _startDir;
  std::string _currAbsFilePath;
  std::string _currDestFile;
  std::string _currHeaderGuard;
  std::string _currNamespace;

  /** Counters for total widget, fonts and sounds count - used to later on
   *  call .reserve() on the containers that will be populated with
   *  the parsed data.
   * */
  uint64_t _staticWidgetsCounter;
  uint64_t _dynamicWidgetsCounter;
  uint64_t _fontsCounter;
  uint64_t _musicsCounter;
  uint64_t _chunksCounter;

  /** Counters for total widget, fonts + sound file sizes - used to
   *  later on perform a proper drawing of the loading screen progress bar
   * */
  int32_t _staticResFileTotalSize;
  int32_t _dynamicResFileTotalSize;
  int32_t _fontFileTotalSize;
  int32_t _soundFileTotalSize;

  /* Name of the stream that is currently being processed. */
  std::ifstream _sourceStream;

  /* Used to validate individual image/font/sound files */
  FileParser _fileParser;

  /* Used to auto-generate files from the parsed data */
  FileBuilder _fileBuilder;

  /* Used to catch syntax errors in the .rsrc files */
  SyntaxChecker _syntaxChecker;

  /* A vector that holds all parsed data from an individual .rsrc file */
  std::vector<CombinedData> _fileData;

  /* A sanity checker for finding duplicate files or file paths */
  std::unordered_set<std::string> _uniqueFiles;
};

#endif /* TOOLS_RESOURCE_BUILDER_INCLUDE_RESOURCEPARSER_H_ */
