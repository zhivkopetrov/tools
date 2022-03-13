#ifndef TOOLS_RESOURCE_BUILDER_INCLUDE_SYNTAXCHECKER_H_
#define TOOLS_RESOURCE_BUILDER_INCLUDE_SYNTAXCHECKER_H_

// System headers
#include <cstdint>
#include <string>

// Other libraries headers
#include "resource_utils/defines/ResourceDefines.h"

// Own components header
#include "utils/ErrorCode.h"

// Forward declarations

/* Resource Builder Tool follows certain syntax in order to easily catch
 * type-o mistakes or even completely missing tag descriptions
 *
 * Supported syntax:
 *      > Images
 *          - tag         - name of the tag
 *          - type        - image
 *          - path        - path relative to the resource folder
 *          - description - empty for now
 *          - position    - coordinates relative to the attached monitor
 *          - load        - resource load type (on init or on demand)
 *
 *      Example:
 *      tag=[BACKGROUND]
 *      type=image
 *      path=p/static_sm_mg.png
 *      description=empty           //empty field is required
 *      position=0,0
 *      load=on_init
 *
 *
 *      > Sprite
 *          - tag         - name of the tag
 *          - type        - sprite
 *          - path        - path relative to the resource folder
 *          - description - 4 comma separated integers
 *                             o frame width
 *                             o frame height
 *                             o frame count
 *                             o horizontal offset
 *          - position    - coordinates relative to the attached monitor
 *          - load        - resource load type (on init or on demand)
 *
 *      Example:
 *      tag=[EXIT_BUTTON]
 *      type=sprite
 *      path=p/exit_button.png
 *      description=200,100,3,0
 *      position=10,1000
 *      load=on_demand
 *
 *      > Sprite_Manual
 *          NOTE: Sprite_Manul supports multiple description lines
 *
 *          - tag         - name of the tag
 *          - type        - sprite_manual
 *          - path        - path relative to the resource folder
 *          - description - 4 comma separated integers
 *                             o frame x coordinate within the image
 *                             o frame y coordinate within the image
 *                             o frame width within the image
 *                             o frame height within the image
 *          - position    - coordinates relative to the attached monitor
 *
 *      Example:
 *      tag=[EXIT_BUTTON]
 *      type=sprite
 *      path=p/exit_button.png
 *      description=0,0,100,0
 *      description=100,0,100,0
 *      description=200,0,150,0
 *      position=10,1000
 *      load=on_demand
 *
 *
 *      > Font
 *          - tag         - name of the tag
 *          - type        - font
 *          - path        - path relative to the resource folder
 *          - description - font size
 *
 *      Example:
 *      tag=[ORBITRON_LIGHT]
 *      type=font
 *      path=f/orbitron_light.otf
 *      description=20
 *
 *      > Sound
 *          - tag         - name of the tag
 *          - type        - sound
 *          - path        - path relative to the resource folder
 *          - description - music/chunk + initial sound level
 *                                     [low, very_low, medium, high, very_high]
 *
 *      Example:
 *      tag=[ENTER_GAME]
 *      type=sound
 *      path=s/enter_game.ogg
 *      description=chunk,medium
 *      or
 *      description=music,very_high
 * */
class SyntaxChecker {
 public:
  SyntaxChecker();
  ~SyntaxChecker() noexcept = default;

  /** @brief used to determine whether tag is valid
   *
   *  @param const std::string & - data to be checked
   *
   *  @returns bool - has valid tag or not
   * */
  bool hasValidTag(const std::string& line);

  /** @brief used to update order
   *              (in order to know which is the next tag to be expected)
   * */
  void updateOrder();

  /** @brief used to set field type to currently processed image
   *
   *  @param const FieldType - field type value
   * */
  void setFieldType(const ResourceDefines::FieldType fieldType) {
    _currFieldType = fieldType;
  }

  /** @brief used to set field type to currently processed image
   *
   *  @param const std::string & - field type value
   * */
  void setFieldTypeFromString(const std::string& dataType);

  /** @brief used to get image currently set field type
   *
   *  @returns FieldType - field type value
   * */
  ResourceDefines::FieldType getFieldType() const {
    return _currFieldType;
  }

  /** @brief used to reset image internal state
   * */
  void reset() {
    _currField = ResourceDefines::Field::TAG;
    _currFieldType = ResourceDefines::FieldType::UNKNOWN;
  }

  /** @brief used to determine whether all required data
   *                                      for currently set image is read.
   *       If yes -> return true and reset _currField to it's first value;
   *       If no  -> return false and continue reading more tags;
   *
   *  @returns bool - is chunk ready or not
   * */
  bool isChunkReady();

  /** @brief used to extract value of data behind it's tag and set
   *                                         the corresponding event code.
   *
   *  @param const std::string &   - input data (tag + tag information)
   *  @param std::string & outData - parsed tag information
   *  @param int32_t &             - event code (in order to check
   *                                                 which tag was parsed)
   *
   *  @returns ErrorCode           - error code
   * */
  ErrorCode extractRowData(const std::string& lineData, std::string& outData,
                           int32_t& outEventCode);

 private:
  /** Holds currently processed filed
   *              (in order to know which is the next tag to be expected)
   * */
  int32_t _currField;

  /** Holds currently processed field type
   *                 (in order to know how to manipulate tag information)
   * */
  ResourceDefines::FieldType _currFieldType;

  /**  Strings names in order check for syntax errors
   *                           in each individual provided tag run-time
   * */
  const std::string _TAG_STR;
  const std::string _PATH_STR;
  const std::string _TYPE_STR;
  const std::string _DESCR_STR;
  const std::string _POS_STR;
  const std::string _LOAD_STR;

  /**  Sizes of individual tag in order to check for syntax errors
   *                           in each individual provided tag run-time
   * */
  const uint64_t _TAG_STR_SIZE;
  const uint64_t _PATH_STR_SIZE;
  const uint64_t _TYPE_STR_SIZE;
  const uint64_t _DESCR_STR_SIZE;
  const uint64_t _POS_STR_SIZE;
  const uint64_t _LOAD_STR_SIZE;
};

#endif /* TOOLS_RESOURCE_BUILDER_INCLUDE_SYNTAXCHECKER_H_ */
