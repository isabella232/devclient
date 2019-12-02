set(MACOS_APP "${CMAKE_BINARY_DIR}/devclient.app")
set(MACOS_APP_CPY "${CMAKE_BINARY_DIR}/devclient-standalone.app")
install(
    CODE
    "include(BundleUtilities)
    macro(fixup_bundle_item resolved_embedded_item exepath dirs)
        message(\"Making writable: \${resolved_embedded_item}\")
        message(\"exepath: \${exepath}\")
        message(\"dirs: \${dirs}\")
        execute_process(COMMAND chmod +w \${resolved_embedded_item})
        _fixup_bundle_item(\"\${resolved_embedded_item}\" \"\${exepath}\" \"\${dirs}\")
    endmacro()
    copy_and_fixup_bundle(${MACOS_APP} ${MACOS_APP_CPY} \"\" \"\")")