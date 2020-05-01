// C system headers

// C++ system headers
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "ResourceParser.h"
#include "utils/Log.h"

int32_t main(int32_t argc, char* args[]) {
  const std::vector<std::string> DEFAULT_PROJECTS{"engine", "displaymodule"};

  std::vector<std::string> allSupportedProjects;
  allSupportedProjects.reserve(DEFAULT_PROJECTS.size());

  if (1 < argc) {
    // start from the second argument, because the first is the program name
    for (int32_t i = 1; i < argc; ++i) {
      allSupportedProjects.emplace_back(args[i]);
    }

    std::vector<std::string>::iterator it = allSupportedProjects.begin();
    for (; it != allSupportedProjects.end();) {
      bool isMatchFound = false;
      for (const std::string& projectName : DEFAULT_PROJECTS) {
        if (projectName == *it) {
          isMatchFound = true;
          break;  // break the inner loop
        }
      }

      if (isMatchFound) {
        ++it;  // normal update
      } else {
        it = allSupportedProjects.erase(it);  // erase and update

        LOGERR("Error, unknown project \"%s\" will be skipped", (*it).c_str());
      }
    }
  } else  // no additional parameters
  {
    allSupportedProjects = DEFAULT_PROJECTS;
  }

  ResourceParser parser;
  if (EXIT_SUCCESS != parser.init("project/")) {
    LOGERR("Error in parser.init()");

   return EXIT_FAILURE;
  }

  for (const auto & project : allSupportedProjects) {
    if (EXIT_SUCCESS != parser.parseResourceTree(project)) {
      LOGERR("Error in parser.parseResourceTree()");

      LOGC(
          "Developer hint: Resolve your errors in the failed .rsrc "
          "files and rerun the resourceBuilder tool");

      break;
    }

    LOG();
  }

  return EXIT_SUCCESS;
}
