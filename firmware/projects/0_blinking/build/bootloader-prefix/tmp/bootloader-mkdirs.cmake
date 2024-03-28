# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/dgonz/esp/v5.1.2/esp-idf/components/bootloader/subproject"
  "C:/Repositorios/EProgramable_FIUNER/firmware/projects/0_blinking/build/bootloader"
  "C:/Repositorios/EProgramable_FIUNER/firmware/projects/0_blinking/build/bootloader-prefix"
  "C:/Repositorios/EProgramable_FIUNER/firmware/projects/0_blinking/build/bootloader-prefix/tmp"
  "C:/Repositorios/EProgramable_FIUNER/firmware/projects/0_blinking/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Repositorios/EProgramable_FIUNER/firmware/projects/0_blinking/build/bootloader-prefix/src"
  "C:/Repositorios/EProgramable_FIUNER/firmware/projects/0_blinking/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Repositorios/EProgramable_FIUNER/firmware/projects/0_blinking/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Repositorios/EProgramable_FIUNER/firmware/projects/0_blinking/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
