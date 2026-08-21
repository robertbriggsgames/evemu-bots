#
# evemu-bots — in-process playerbots module for EVEmu Crucible.
# Added to the eve-server target by the core modules/ loader.
#
IF(NOT TARGET eve-server)
  MESSAGE(FATAL_ERROR "evemu-bots must be built as an EVEmu module (clone into modules/evemu-bots)")
ENDIF()

SET(EVEMU_BOTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

SET(EVEMU_BOTS_SOURCES
  "${EVEMU_BOTS_DIR}/src/host/ModuleExport.cpp"
  "${EVEMU_BOTS_DIR}/src/host/DummyTCP.cpp"
  "${EVEMU_BOTS_DIR}/src/host/CommandQueue.cpp"
  "${EVEMU_BOTS_DIR}/src/host/BotDB.cpp"
  "${EVEMU_BOTS_DIR}/src/host/BotFactory.cpp"
  "${EVEMU_BOTS_DIR}/src/host/BotMgr.cpp"
  "${EVEMU_BOTS_DIR}/src/api/Json.cpp"
  "${EVEMU_BOTS_DIR}/src/api/Metrics.cpp"
  "${EVEMU_BOTS_DIR}/src/api/HttpServer.cpp"
  "${EVEMU_BOTS_DIR}/src/api/ControlApi.cpp"
  "${EVEMU_BOTS_DIR}/src/population/BotConfig.cpp"
  "${EVEMU_BOTS_DIR}/src/population/PopulationPlanner.cpp"
  "${EVEMU_BOTS_DIR}/src/ai/Activities.cpp"
  "${EVEMU_BOTS_DIR}/src/org/OrgDirector.cpp"
)

TARGET_SOURCES(eve-server PRIVATE ${EVEMU_BOTS_SOURCES})
TARGET_INCLUDE_DIRECTORIES(eve-server PRIVATE
  "${EVEMU_BOTS_DIR}/src"
)

INSTALL(FILES "${EVEMU_BOTS_DIR}/conf/playerbots.conf.dist" DESTINATION "etc")
INSTALL(FILES "${EVEMU_BOTS_DIR}/sql/001_bot_meta.sql" DESTINATION "etc/sql")

MESSAGE(STATUS "EVEmu module enabled: evemu-bots")
