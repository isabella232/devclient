set(MACOS_APP "${CMAKE_BINARY_DIR}/devclient.app")
install(
    CODE
    "include(BundleUtilities)
    macro(fixup_bundle_item resolved_embedded_item exepath dirs)
        execute_process(COMMAND chmod +w \${resolved_embedded_item})
        _fixup_bundle_item(\"\${resolved_embedded_item}\" \"\${exepath}\" \"\${dirs}\")
    endmacro()
    fixup_bundle(${MACOS_APP} \"\" \"\")")