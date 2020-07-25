// C system headers

// C++ system headers
#include <string>

#include "ResourceParser.h"
#include "utils/ErrorCode.h"
#include "utils/Log.h"

int32_t main(int32_t argc, char* args[]) {

  std::string projectPath;
  if (2 <= argc) {
    projectPath = args[1];
  }

  ResourceParser parser;
  if (SUCCESS != parser.init(projectPath)) {
    LOGERR("Error in parser.init() for projectPath: %s", projectPath.c_str());
    return FAILURE;
  }

  if (SUCCESS != parser.parseResourceTree()) {
    LOGERR("Error in parser.parseResourceTree()");
    LOGC("Developer hint: Resolve your errors in the failed .rsrc "
         "files and rerun the resource_builder tool");
    return FAILURE;
  }

  return SUCCESS;
}
