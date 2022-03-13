// System headers
#include <string>
#include <vector>
#include <cstdlib>

// Other libraries headers
#include "utils/ErrorCode.h"
#include "utils/Log.h"

// Own components headers
#include "resource_builder/ResourceParser.h"

int32_t main(const int32_t argc, const char *args[]) {
  if (1 >= argc) {
    LOGERR("Resource Builder tool expects a list a project folder names within "
        "the root project folder to parse");
    return EXIT_FAILURE;
  }

  const std::vector<std::string> supportedProjects = [argc, args](){
    std::vector<std::string> projects(argc - 1);
    for (int32_t i = 1; i < argc; ++i) {
      projects[i - 1] = args[i];
    }
    return projects;
  }();

  ResourceParser parser;
  if (ErrorCode::SUCCESS != parser.init()) {
    return EXIT_FAILURE;
  }

  for (const auto & project : supportedProjects) {
    if (ErrorCode::SUCCESS != parser.parseResourceTree(project)) {
      LOGERR("Error in parser.parseResourceTree() for project: %s",
          project.c_str());
      LOGC("Developer hint: Resolve your errors in the failed .rsrc "
           "files and rerun the resource_builder tool");
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
