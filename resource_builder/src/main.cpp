// C system headers

// C++ system headers
#include <string>
#include <vector>

// Other libraries headers
#include "utils/ErrorCode.h"
#include "utils/Log.h"

// Own components headers
#include "resource_builder/ResourceParser.h"

int32_t main(const int32_t argc, const char *args[]) {
  if (1 >= argc) {
    LOGERR("Resource Builder tool expects a list a project folder names within "
        "the root project folder to parse");
    return FAILURE;
  }

  const std::vector<std::string> supportedProjects = [argc, args](){
    std::vector<std::string> projects(argc - 1);
    for (int32_t i = 1; i < argc; ++i) {
      projects[i - 1] = args[i];
    }
    return projects;
  }();

  ResourceParser parser;
  if (SUCCESS != parser.init()) {
    return FAILURE;
  }

  for (const auto & project : supportedProjects) {
    if (SUCCESS != parser.parseResourceTree(project)) {
      LOGERR("Error in parser.parseResourceTree() for project: %s",
          project.c_str());
      LOGC("Developer hint: Resolve your errors in the failed .rsrc "
           "files and rerun the resource_builder tool");
      return FAILURE;
    }
  }

  return SUCCESS;
}
