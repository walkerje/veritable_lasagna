# Create git hooks directory if it doesn't exist
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/.git/hooks")

# Copy hooks
configure_file(
        "${CMAKE_SOURCE_DIR}/cmake/hooks/pre-push"
        "${CMAKE_SOURCE_DIR}/.git/hooks/pre-push"
        COPYONLY
)

# Make hooks executable
file(CHMOD "${CMAKE_SOURCE_DIR}/.git/hooks/pre-push"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
