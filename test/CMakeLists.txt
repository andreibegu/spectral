configure_file(lit.site.cfg.py.in lit.site.cfg.py @ONLY)

# Because the current implementation of the compiler-driver uses the same 
# file-names when compiling the wrapper-functions, using more than one worker
# causes race-conditions. 
add_custom_target(check
        COMMAND lit --workers=1 "${CMAKE_CURRENT_BINARY_DIR}" -v
        DEPENDS pallasSpec2ir)