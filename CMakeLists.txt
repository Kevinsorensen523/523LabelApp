cmake_minimum_required(VERSION 3.16)

project(523LabelApp)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(523LabelApp
    main.cpp
    raw_printer.cpp
)

target_link_libraries(523LabelApp PRIVATE Qt6::Widgets)

# Windows: kirim ZPL lewat Print Spooler (OpenPrinter/WritePrinter)
if(WIN32)
    target_link_libraries(523LabelApp PRIVATE winspool)
    set_target_properties(523LabelApp PROPERTIES WIN32_EXECUTABLE ON)
endif()

# Mac: build sebagai .app bundle supaya bisa dibuka dari Finder
if(APPLE)
    set_target_properties(523LabelApp PROPERTIES MACOSX_BUNDLE ON)
endif()
