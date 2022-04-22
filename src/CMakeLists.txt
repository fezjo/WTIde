file(GLOB_RECURSE SOURCE_FILES src *.cpp)
file(GLOB_RECURSE HEADER_FILES src *.h)

target_sources(
    ${TARGET_NAME}
    PRIVATE
        ${SOURCE_FILES}
        ${HEADER_FILES}
)