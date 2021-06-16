cmake_minimum_required(VERSION 3.6)
project(qfaktury VERSION 0.7.2 LANGUAGES CXX)

if(POLICY CMP0071)
  cmake_policy(SET CMP0071 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(ECM 5.43.0 REQUIRED CONFIG)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" "${CMAKE_ROOT}/Modules")
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 5.10 REQUIRED COMPONENTS Gui Core Widgets PrintSupport Xml XmlPatterns Network)

option(FEDORA "Check if this Fedora" ON)

find_package(Fedora)
find_package(QuaZip5)

# message(STATUS "${CMAKE_MODULE_PATH}")

if(FEDORA)
  add_definitions(-DFEDORA_FOUND)
  message("It's Fedora. It's possible to run beesu instead of gksu")
endif()

if(QUAZIP_FOUND)
  add_definitions(-DQUAZIP)
  message("QuaZip has been found. It's possible to create backup")
else()
  message("QuaZip hasn't been found in FindQuaZip5.cmake. It's not possible to create backup.")
endif()

if(NOT QUAZIP_FOUND)
    set(QUAZIP_INCLUDE_DIR "")
    set(ZLIB_INCLUDE_DIR "")
    set(QUAZIP_INCLUDE_DIRS "")
    set(QUAZIP_LIBRARIES "")
    find_path(QUAZIP_INCLUDE_DIR NAMES quazipdir.h JlCompress.h PATH_SUFFIXES quazip5)
    find_path(ZLIB_INCLUDE_DIR NAME zlib.h)
    set(QUAZIP_INCLUDE_DIRS ${QUAZIP_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR}) # QUAZIP_INCLUDE_DIRS contains includes for zlib and quazip in FindQuaZip5.cmake
    find_library(QUAZIP_LIBRARIES quazip5)
    if(NOT QUAZIP_INCLUDE_DIRS OR NOT QUAZIP_LIBRARIES)
        message("Could NOT find optional library quazip.")
    else()
        message("Found optional library quazip.")
    endif()
endif()


add_compile_options(
    -fvisibility=hidden
    -fvisibility-inlines-hidden

    -fstack-protector
)

# enable warnings
add_compile_options(
    -Werror

    -Wall
    -Wextra
    -Wpedantic

    -Wcast-align
    -Wcast-qual
    -Wctor-dtor-privacy
    -Wformat
    -Wlogical-op
    -Wnoexcept
    -Wold-style-cast
    -Wredundant-decls
    -Wstack-protector
    -Wstrict-null-sentinel

    # Code fixes required to enable following warnings
#    -Wconversion
#    -Wdouble-promotion
#    -Woverloaded-virtual
#    -Wzero-as-null-pointer-constant
)

# disable warnings
add_compile_options(
    -Wno-deprecated-declarations
)

# usefull switches for debugging
#add_compile_options(
#    -finstrument-functions
#    -fsanitize=address
#    -fno-omit-frame-pointer
#    -fsanitize=leak
#)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

set(SOURCES
    src/SimpleSmtp_SSL_QT5/smtp_attachements/emailwindow.cpp
    src/SimpleSmtp_SSL_QT5/smtp_attachements/smtp.cpp

    src/backup.cpp
    src/bill.cpp
    src/buyerdata.cpp
    src/buyers.cpp
    src/buyerslist.cpp
    src/changeamount.cpp
    src/convertamount.cpp
    src/correctgross.cpp
    src/correction.cpp
    src/csvexport.cpp
    src/custompaymdata.cpp
    src/custompayment.cpp
    src/deliverynote.cpp
    src/duplicate.cpp
    src/goods.cpp
    src/goodsgrosslist.cpp
    src/goodsissuednotes.cpp
    src/goodslist.cpp
    src/invoice.cpp
    src/invoicedata.cpp
    src/invoicegross.cpp
    src/invoicerr.cpp
    src/main.cpp
    src/mainwindow.cpp
    src/organizer.cpp
    src/owncalendar.cpp
    src/productdata.cpp
    src/runguard.cpp
    src/saftfile.cpp
    src/saftfileoutput.cpp
    src/send.cpp
    src/setting.cpp
    src/settings.cpp
    src/user.cpp
    src/validations.cpp
    src/warehouse.cpp
    src/warehousedata.cpp
    src/xmldatalayer.cpp

    cyg_profile_func.cpp
)

set(HEADER_FILES
    src/backup.h
    src/bill.h
    src/buyerdata.h
    src/buyers.h
    src/buyerslist.h
    src/changeamount.cpp
    src/changeamount.h
    src/config.h
    src/const.h
    src/convertamount.h
    src/correctgross.h
    src/correction.h
    src/csvexport.h
    src/custompaymdata.h
    src/custompayment.h
    src/datewidgetitem.h
    src/debug_message.h
    src/deliverynote.h
    src/detector.h
    src/documentdata.h
    src/duplicate.h
    src/fwd.hpp
    src/goods.h
    src/goodsgrosslist.h
    src/goodsissuednotes.h
    src/goodslist.h
    src/idatalayer.h
    src/invoice.h
    src/invoicedata.h
    src/invoicegross.h
    src/invoicerr.h
    src/mainwindow.h
    src/messagehandler.h
    src/organizer.h
    src/owncalendar.h
    src/productdata.h
    src/runguard.h
    src/saftfile.h
    src/saftfileoutput.h
    src/send.h
    src/setting.h
    src/settings.h
    src/user.h
    src/validations.h
    src/warehouse.cpp
    src/warehouse.h
    src/warehousedata.h
    src/xmldatalayer.h
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    src/SimpleSmtp_SSL_QT5/smtp_attachements
)

qt5_wrap_ui( UIS_HDRS
    ui/setting.ui
    ui/user.ui
    ui/mainwindow.ui
    ui/invoice.ui
    ui/goodslist.ui
    ui/goods.ui
    ui/custompaymentdialog.ui
    ui/changeamount.ui
    ui/buyers.ui
    ui/buyerslist.ui
    ui/organizer.ui
    ui/saftfile.ui
    ui/backup.ui
    ui/csvexport.ui
    src/SimpleSmtp_SSL_QT5/smtp_attachements/emailwindow.ui
)

QT5_ADD_RESOURCES(RESOURCES qfaktury.qrc)

add_executable(qfaktury ${SOURCES} ${HEADER_FILES} ${UIS_HDRS} ${RESOURCES})

target_link_libraries (qfaktury
    Qt5::Gui
    Qt5::Core
    Qt5::Widgets
    Qt5::PrintSupport
    Qt5::Xml
    Qt5::XmlPatterns
    Qt5::Network
    ${QUAZIP_LIBRARIES}
)

SET(QFAKTURY_PATH "/usr/share/qfaktury")

install(TARGETS qfaktury DESTINATION /usr/bin)
install(DIRECTORY icons DESTINATION ${QFAKTURY_PATH})
install(DIRECTORY schemas DESTINATION ${QFAKTURY_PATH})
install(DIRECTORY templates DESTINATION ${QFAKTURY_PATH})
install(DIRECTORY docs DESTINATION /usr/share/doc/qfaktury)
install(FILES QFaktury.desktop DESTINATION /usr/share/applications)
install(DIRECTORY src DESTINATION ${QFAKTURY_PATH})
install(DIRECTORY ui DESTINATION ${QFAKTURY_PATH})
