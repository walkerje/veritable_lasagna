# Create git hooks directory if it doesn't exist
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/.git/hooks")

# Copy hooks
configure_file(
        "${CMAKE_SOURCE_DIR}/cmake/hooks/pre-commit"
        "${CMAKE_SOURCE_DIR}/.git/hooks/pre-commit"
        COPYONLY
)

# Make hooks executable
file(CHMOD "${CMAKE_SOURCE_DIR}/.git/hooks/pre-commit"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
