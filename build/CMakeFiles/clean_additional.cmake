# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "apps/obsbot-linux/CMakeFiles/obsbot-linux_autogen.dir/AutogenUsed.txt"
  "apps/obsbot-linux/CMakeFiles/obsbot-linux_autogen.dir/ParseCache.txt"
  "apps/obsbot-linux/obsbot-linux_autogen"
  "core/CMakeFiles/obsbot-core_autogen.dir/AutogenUsed.txt"
  "core/CMakeFiles/obsbot-core_autogen.dir/ParseCache.txt"
  "core/obsbot-core_autogen"
  )
endif()
