# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/alumno/esp/v5.2/esp-idf/components/bootloader/subproject"
  "C:/Repositorios/GonzalezViduzzi-1C-2024/firmware/projects/Proyecto_integrador/build/bootloader"
  "C:/Repositorios/GonzalezViduzzi-1C-2024/firmware/projects/Proyecto_integrador/build/bootloader-prefix"
  "C:/Repositorios/GonzalezViduzzi-1C-2024/firmware/projects/Proyecto_integrador/build/bootloader-prefix/tmp"
  "C:/Repositorios/GonzalezViduzzi-1C-2024/firmware/projects/Proyecto_integrador/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Repositorios/GonzalezViduzzi-1C-2024/firmware/projects/Proyecto_integrador/build/bootloader-prefix/src"
  "C:/Repositorios/GonzalezViduzzi-1C-2024/firmware/projects/Proyecto_integrador/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Repositorios/GonzalezViduzzi-1C-2024/firmware/projects/Proyecto_integrador/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Repositorios/GonzalezViduzzi-1C-2024/firmware/projects/Proyecto_integrador/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()