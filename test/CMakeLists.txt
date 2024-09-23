configure_file(lit.site.cfg.py.in lit.site.cfg.py @ONLY)

# Path where the venv for lit will be created 
set(LIT_VENV_PATH "${CMAKE_CURRENT_BINARY_DIR}/litEnv")

# In newer ubuntu versions, the lit-package cannot be directly installed into 
# the default python installation. This command creates a venv in the 
# build-directory, and installs lit in it.
add_custom_target(makeLitVenv
        COMMAND python3 -m venv "${LIT_VENV_PATH}"
        COMMAND "${LIT_VENV_PATH}/bin/pip3" install lit==18.1.8
)


# Because the current implementation of the compiler-driver uses the same 
# file-names when compiling the wrapper-functions, using more than one worker
# causes race-conditions. 
add_custom_target(checkV
        # Activate the venv        
        COMMAND . "${LIT_VENV_PATH}/bin/activate"
        # Run lit
        COMMAND lit --workers=1 "${CMAKE_CURRENT_BINARY_DIR}" -v
        # Deactivate lit
        COMMAND deactivate
        DEPENDS pallasSpec2ir
)

add_custom_target(check
        COMMAND lit --workers=1 "${CMAKE_CURRENT_BINARY_DIR}" -v
        DEPENDS pallasSpec2ir
)