if(NOT EXISTS "/home/niko/Ontwikkeling/opa/install_manifest.txt")
    message(FATAL_ERROR "Cannot find install manifest: /home/niko/Ontwikkeling/opa/install_manifest.txt")
endif()

file(READ "/home/niko/Ontwikkeling/opa/install_manifest.txt" files)
string(REGEX REPLACE "\n" ";" files "${files}")
foreach(file ${files})
    message(STATUS "Uninstalling $ENV{DESTDIR}${file}")
    if(IS_SYMLINK "$ENV{DESTDIR}${file}" OR EXISTS "$ENV{DESTDIR}${file}")
        exec_program(
            "/nix/store/xjg2fzw513iig1cghd4mvcq5fh2cyv4y-cmake-3.24.0/bin/cmake" ARGS "-E remove \"$ENV{DESTDIR}${file}\""
            OUTPUT_VARIABLE rm_out
            RETURN_VALUE rm_retval
            )
        if(NOT "${rm_retval}" STREQUAL 0)
            message(FATAL_ERROR "Problem when removing $ENV{DESTDIR}${file}")
        endif()
    else()
        message(STATUS "File $ENV{DESTDIR}${file} does not exist.")
    endif()
endforeach()
