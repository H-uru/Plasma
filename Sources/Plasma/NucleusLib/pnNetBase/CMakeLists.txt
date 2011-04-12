include_directories("../../CoreLib")
include_directories("../../NucleusLib")

set(pnNetBase_HEADERS
    Intern.h
    Pch.h
    pnNbConst.h
    pnNetBase.h
)

set(pnNetBase_PRIVATE
    Private/pnNbAllIncludes.h
    Private/pnNbEchoMsgs.h
    Private/pnNbError.h
    Private/pnNbError.cpp
    Private/pnNbProtocol.h
    Private/pnNbProtocol.cpp
    Private/pnNbSrvs.h
    Private/pnNbSrvs.cpp
    Private/pnNbKeys.h
    Private/pnNbKeys.cpp
)

add_library(pnNetBase STATIC ${pnNetBase_HEADERS} ${pnNetBase_PRIVATE})

source_group("Header Files" FILES ${pnNetBase_HEADERS})
source_group("Private" FILES ${pnNetBase_PRIVATE})
