# Read vcpkg.json
file(READ "${CMAKE_SOURCE_DIR}/vcpkg.json" VCPKG_CONTENTS)
string(JSON VERSION GET ${VCPKG_CONTENTS} version)
string(JSON DESCRIPTION GET ${VCPKG_CONTENTS} description)

# Check current version in vcpkg overlay if it exists
set(VCPKG_OVERLAY_PATH "${CMAKE_SOURCE_DIR}/vcpkg/veritable-lasagna")
set(VCPKG_OVERLAY_JSON "${VCPKG_OVERLAY_PATH}/vcpkg.json")
set(NEEDS_VCPKG_UPDATE TRUE)

if(EXISTS "${VCPKG_OVERLAY_JSON}")
    file(READ "${VCPKG_OVERLAY_JSON}" VCPKG_OVERLAY_CONTENTS)
    string(JSON OVERLAY_VERSION GET ${VCPKG_OVERLAY_CONTENTS} version)
    if("${VERSION}" STREQUAL "${OVERLAY_VERSION}")
        set(NEEDS_VCPKG_UPDATE FALSE)
    endif()
endif()

# Update vcpkg overlay if needed
if(NEEDS_VCPKG_UPDATE)
    file(MAKE_DIRECTORY "${VCPKG_OVERLAY_PATH}")
    file(COPY "${CMAKE_SOURCE_DIR}/vcpkg.json" DESTINATION "${VCPKG_OVERLAY_PATH}")
    message(STATUS "Updated vcpkg overlay with new version ${VERSION}")
endif()

# Read existing README
file(READ "${CMAKE_SOURCE_DIR}/README.md" README_CONTENTS)

# Update version in title
string(REGEX REPLACE
        "(# Veritable Lasagna \\(v)[0-9\\.]+\\)"
        "\\1${VERSION})"
        README_NEW "${README_CONTENTS}")

# Update version in back to top link
string(REGEX REPLACE
        "(\\[Back to Top\\]\\(#veritable-lasagna-v)[0-9\\.]+\\)"
        "\\1${VERSION})"
        README_NEW "${README_NEW}")

# Update description under the title if it exists
string(REGEX REPLACE
        "(> )[^<\n]+"
        "\\1${DESCRIPTION}"
        README_NEW "${README_NEW}")

# Write back only if content changed
if(NOT "${README_CONTENTS}" STREQUAL "${README_NEW}")
    file(WRITE "${CMAKE_SOURCE_DIR}/README.md" "${README_NEW}")
    message(STATUS "README.md updated with new metadata")
endif()