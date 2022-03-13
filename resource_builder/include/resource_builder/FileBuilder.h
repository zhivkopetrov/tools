#ifndef TOOLS_RESOURCE_BUILDER_INCLUDE_FILEBUILDER_H_
#define TOOLS_RESOURCE_BUILDER_INCLUDE_FILEBUILDER_H_

// System headers
#include <fstream>
#include <string>
#include <vector>

// Other libraries headers
#include "utils/ErrorCode.h"

// Own components header

// Forward declarations
struct CombinedData;

class FileBuilder {
 public:
  virtual ~FileBuilder() noexcept;

  /** @brief used to open combined resource file and font file streams
   *
   *  @param const std::string & - name of the engine packed resource file
   *  @param const std::string & - name of the engine packed font file
   *  @param const std::string & - name of the engine packed sound file
   *
   *  @returns ErrorCode         - error code
   * */
  ErrorCode openCombinedStreams(const std::string& resFileName,
                              const std::string& fontFileName,
                              const std::string& soundFileName);

  /** @brief used to open individual games resource header and cpp files
   *
   *  @returns ErrorCode - error code
   * */
  ErrorCode openDestStreams();

  /** @brief used to close individual games resource header and cpp files
   * */
  void closeDestStream();

  /** @brief used to set individual games namespace name for
   *                                resource header and cpp resource file
   *
   *  @param const std::string & - namespace value
   * */
  void setNamespace(const std::string& inputNamespace);

  /** @brief used to set individual games
   *                                    resource header and cpp file name
   *
   *  @param const std::string & - file name
   * */
  void setDestFileName(const std::string& destFileName);

  /** @brief used to set individual games header guards name for
   *                                resource header and cpp resource file
   *
   *  @param const std::string& - header guard value
   * */
  void setHeaderGuards(const std::string& guards);

  /** @brief used write all parsed data from a single .rsrc file to the:
   *              > engine resource file;
   *              > individual game resource header and cpp file;
   *
   *  @param const std::vector<CombinedData> & - the parsed data from the
   *                                                           .rsrc file
   * */
  void writeData(const std::vector<CombinedData>& data);

  /** @brief used write total widgets, fonts and sounds count for:
   *              > engine resource file;
   *              > engine font file;
   *              > engine sound file;
   *
   *  @param const uint64_t                   - static widgets count
   *  @param const uint64_t                   - dynamic widgets count
   *  @param const uint64_t                   - fonts count
   *  @param const uint64_t                   - musics count
   *  @param const uint64_t                   - sound chunk count
   *  @param const int32_t                    - total widgets file size
   *                                             (NOTE: static files only)
   *  @param const int32_t                    - total fonts file size
   *  @param const int32_t                    - total sounds file size
   *  @param const std::vector<std::string> & - all white-listed engine
   *                                                  common game folders
   * */
  void finishCombinedDestFiles(
      const uint64_t staticWidgetsCount, const uint64_t dynamicWidgetsCount,
      const uint64_t fontsCount, const uint64_t musicsCount,
      const uint64_t chunksCount, const int32_t totalWidgetFileSize,
      const int32_t totalFontsFileSize, const int32_t totalSoundsFileSize);

 private:
  /** @brief used to close combined resource and font files
   * */
  void closeCombinedStreams();

  /** @brief used write all parsed data from a single .rsrc file to the
   *                                              combined resource file.
   *
   *  @param const std::vector<CombinedData> & - the parsed data from the
   *                                                           .rsrc file
   * */
  void fillCombinedDestFile(const std::vector<CombinedData>& data);

  /** @brief used write all parsed data from a single .rsrc file to the
   *                        individual game resource header and cpp file.
   *
   *  @param const std::vector<CombinedData> & - the parsed data from the
   *                                                           .rsrc file
   * */
  void autoGenerateResFile(const std::vector<CombinedData>& data);

  /** @brief used write total widgets for combined resource file
   *
   *  @param const uint64_t - total static widgets count
   *  @param const uint64_t - total dynamic widgets count
   *  @param const int32_t  - total widgets file size
   * */
  void finishCombinedResFile(const uint64_t staticWidgetsCount,
                             const uint64_t dynamicWidgetsCount,
                             const int32_t totalWidgetFileSize);

  /** @brief used write total fonts for combined font file
   *
   *  @param const uint64_t - total fonts count
   *  @param const int32_t  - total fonts file size
   * */
  void finishCombinedFontFile(const uint64_t fontsCount,
                              const int32_t totalFontsFileSize);

  /** @brief used write total sound for combined sound file
   *
   *  @param const uint64_t - total musics count
   *  @param const uint64_t - total sound chunk count
   *  @param const int32_t  - total sounds file size
   * */
  void finishCombinedSoundFile(const uint64_t musicsCount,
                               const uint64_t chunksCount,
                               const int32_t totalSoundsFileSize);

  /* Destination stream for combined only resource file
   * this file contains information for every single resource listed
   * in the individual .rsrc files
   * */
  std::ofstream _combinedResDestStream;

  /* Destination stream for combined only font file
   * this file contains information for every single font listed
   * in the individual .rsrc files
   * */
  std::ofstream _combinedFontDestStream;

  /* Destination stream for combined only sound file
   * this file contains information for every single sound listed
   * in the individual .rsrc files
   * */
  std::ofstream _combinedSoundDestStream;

  /* Destination streams for every individual game resource file
   * that is being auto-generated
   *
   * NOTE: the static stream fills only the static resources (those that
   *       are loaded at engine startup and remain loaded until the end
   *       of the program).
   *
   *       the dynamic streams fills only the dynamic resources (those
   *       that are not loaded until the game is opened. Then they are
   *       loaded/unloaded on game open/close).
   * */
  std::ofstream _destStreamStatic;
  std::ofstream _destStreamDynamic;
  std::ofstream _destStreamDynamicValues;

  /* Absolute file name for individual game resource header and cpp file
   *                                              (without the extension)
   * */
  std::string _destFileNameStatic;
  std::string _destFileNameDynamic;
  std::string _destFileNameDynamicValues;

  /* Namespace value for for individual game resource header and cpp file
   * */
  std::string _namespaceStatic;
  std::string _namespaceDynamic;

  /* Header guard value for for individual game resource header
   *                                                         and cpp file
   * */
  std::string _headerGuardsStatic;
  std::string _headerGuardsDynamic;
};

#endif /* TOOLS_RESOURCE_BUILDER_INCLUDE_FILEBUILDER_H_ */
