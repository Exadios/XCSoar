include_directories(${XCSOAR_SRC})
add_definitions(-DHAVE_POSIX) # For POSIX only
add_definitions(-DZLIB_CONST) # Make some of zlib const.
add_definitions(-DUSE_FREETYPE)

set(OUTPUT_DATA ${OUTPUT_BASE}/data)

# These are for the 'UNIX' target
set(GLX false)
set(OPENGL true)
set(FREETYPE true)
set(HAVE_POSIX true)
set(EGL false)
set(ENABLE_SDL false)
set(ENABLE_ALSA true)
set(VFB false)
set(USE_FB false)
set(USE_SDL true)
set(HAVE_WIN32 false)
set(HAVE_CE false)
set(USE_MEMORY_CANVAS ENABLE_SDL and not OPENGL)
set(GLES2 false)
set(GLSL ${GLES2})
set(HAVE_PCM_PLAYER true)

add_custom_target(lib DEPENDS Airspace
                              Async
                              Audio
                              Contest
                              Data
                              Datafield
                              Driver
                              Event
                              Form
                              Geo
                              Glide
                              Io
                              Jasper
                              Look
                              Lua
                              Math
                              Net
                              Os
                              Profile
                              Resource
                              Resources
                              Route
                              Screen
                              Shape
                              Task
                              Terrain
                              Thread
                              Util
                              Waypoint
                              Widget
                  DEPENDS
                          ) # Convenience target.
add_dependencies(lib generate)

# TODO this needs to be moved downtree.
if(false)
add_custom_command(OUTPUT ${OUTPUT_INCLUDE}/resource.h
                   COMMAND cat ${XCSOAR_SRC}/Resources.hpp | ${TOOLS_DIR}/MakeResourceDefines > ${OUTPUT_INCLUDE}/resource.h
                   DEPENDS ${XCSOAR_SRC}/Resources.hpp
                   DEPENDS ${TOOLS_DIR}/MakeResourceDefines
                   VERBATIM
                   )

set(TARGET_CPPFLAGS -I${OUTPUT_INCLUDE} "-DHAVE_POSIX -DHAVE_STDINT_H -DHAVE_UNISTD_H -DHAVE_VASPRINTF -DRADIANS -DEYE_CANDY -DUSE_WGS84 -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC")
set(OPENGL_CPPFLAGS "-DENABLE_OPENGL -DGL_GLEXT_PROTOTYPES")
set(SCREEN_CPPFLAGS "-DUSE_FREETYPE -DENABLE_OPENGL -DGL_GLEXT_PROTOTYPES -DUSE_GLX -DUSE_X11 -DUSE_POLL_EVENT")
add_custom_command(OUTPUT ${OUTPUT_BASE}/XCSoar.rc
                   COMMAND cat ${XCSOAR}/Data/XCSoar.rc | ${CMAKE_C_COMPILER} -E -o ${OUTPUT_BASE}/XCSoar.rc -I${OUTPUT_INCLUDE} ${TARGET_CPPFLAGS} ${SCREEN_CPPFLAGS} -
                   DEPENDS ${XCSOAR}/Data/XCSoar.rc
                   DEPENDS ${OUTPUT_INCLUDE}/resource.h
                   VERBATIM)
add_custom_command(OUTPUT ${OUTPUT_INCLUDE}/resource_data.h
                   COMMAND perl ${TOOLS_DIR}/GenerateResources.pl < ${OUTPUT_BASE}/XCSoar.rc > ${OUTPUT_INCLUDE}/resource_data.h
                   DEPENDS ${OUTPUT_BASE}/XCSoar.rc
                   DEPENDS ${TOOLS_DIR}/GenerateResources.pl
                   VERBATIM)
set_property(SOURCE ${OUTPUT_INCLUDE}/resource_data.h
             PROPERTY GENERATED true)
add_custom_command(OUTPUT ${OUTPUT_BASE}/build/lib/resources.a
                   COMMAND perl ${TOOLS_DIR}/LinkResources.pl ${OUTPUT_BASE}/XCSoar.rc ${OUTPUT_BASE}/build/lib/resources.a as ${ASFLAGS} ar ${ARFLAGS}
                   DEPENDS ${OUTPUT_BASE}/XCSoar.rc
                   DEPENDS ${TOOLS_DIR}/LinkResources.pl
                   VERBATIM)
add_custom_target(resources ALL
                  DEPENDS ${OUTPUT_INCLUDE}/resource_data.h)
endif(false)

add_definitions(-DHAVE_POSIX
                -DHAVE_STDINT_H
                -DHAVE_UNISTD_H
                -DHAVE_VASPRINTF
                -DRADIANS
                -DEYE_CANDY
                -DUSE_WGS84
                -DUSE_FREETYPE
                -DENABLE_OPENGL
                -DENABLE_ALSA
                -DGL_GLEXT_PROTOTYPES
                -DUSE_GLX
                -DUSE_X11
                -DUSE_POLL_EVENT
                )
set(ASFLAGS "-arch i386")
set(ARFLAGS "-rcs")

include_directories(SYSTEM /usr/include/SDL)

set(UTIL_DIR ${XCSOAR_SRC}/Util)
set(UTIL_SRCS 
              ${UTIL_DIR}/PrintException.cxx
              ${UTIL_DIR}/Base64.cxx
              ${UTIL_DIR}/CRC.cpp
              ${UTIL_DIR}/tstring.cpp
              ${UTIL_DIR}/UTF8.cpp
              ${UTIL_DIR}/ASCII.cxx
              ${UTIL_DIR}/TruncateString.cpp
              ${UTIL_DIR}/EscapeBackslash.cpp
              ${UTIL_DIR}/ExtractParameters.cpp
              ${UTIL_DIR}/ConvertString.cpp
              ${UTIL_DIR}/StaticString.cxx
              ${UTIL_DIR}/AllocatedString.cxx
              ${UTIL_DIR}/StringView.cxx
              ${UTIL_DIR}/StringCompare.cxx
              ${UTIL_DIR}/StringUtil.cpp)
include_directories(${XCSOAR_SRC} ${XCSOAR_SRC}/unix ${UTIL_DIR})
add_library(Util ${UTIL_SRCS})

set(AUDIO_DIR ${XCSOAR_SRC}/Audio)
set(AUDIO_SRCS
               ${AUDIO_DIR}/ToneSynthesiser.cpp
               ${AUDIO_DIR}/VarioSynthesiser.cpp
               ${AUDIO_DIR}/PCMPlayer.cpp
               ${AUDIO_DIR}/GlobalPCMResourcePlayer.cpp
               ${AUDIO_DIR}/GlobalPCMMixer.cpp
               ${AUDIO_DIR}/GlobalVolumeController.cpp
               ${AUDIO_DIR}/MixerPCMPlayer.cpp
               ${AUDIO_DIR}/PCMBufferDataSource.cpp
               ${AUDIO_DIR}/PCMMixerDataSource.cpp
               ${AUDIO_DIR}/PCMMixer.cpp
               ${AUDIO_DIR}/PCMResourcePlayer.cpp
               ${AUDIO_DIR}/VolumeController.cpp
               ${AUDIO_DIR}/ALSAEnv.cpp
               ${AUDIO_DIR}/ALSAPCMPlayer.cpp)
include_directories(${XCSOAR_SRC} ${AUDIO_DIR})
add_library(Audio ${AUDIO_SRCS})
set_target_properties(Audio
                      PROPERTIES COMPILE_FLAGS "-isystem /usr/include/alsa")

set(GEO_DIR ${XCSOAR_SRC}/Geo)
set(GEO_SRCS
             ${GEO_DIR}/Boost/RangeBox.cpp
             ${GEO_DIR}/ConvexHull/GrahamScan.cpp
             ${GEO_DIR}/ConvexHull/PolygonInterior.cpp
             ${GEO_DIR}/Memento/DistanceMemento.cpp
             ${GEO_DIR}/Memento/GeoVectorMemento.cpp
             ${GEO_DIR}/Flat/FlatProjection.cpp
             ${GEO_DIR}/Flat/TaskProjection.cpp
             ${GEO_DIR}/Flat/FlatBoundingBox.cpp
             ${GEO_DIR}/Flat/FlatGeoPoint.cpp
             ${GEO_DIR}/Flat/FlatRay.cpp
             ${GEO_DIR}/Flat/FlatPoint.cpp
             ${GEO_DIR}/Flat/FlatEllipse.cpp
             ${GEO_DIR}/Flat/FlatLine.cpp
             ${GEO_DIR}/Math.cpp
             ${GEO_DIR}/SimplifiedMath.cpp
             ${GEO_DIR}/Quadrilateral.cpp
             ${GEO_DIR}/GeoPoint.cpp
             ${GEO_DIR}/GeoVector.cpp
             ${GEO_DIR}/GeoBounds.cpp
             ${GEO_DIR}/GeoClip.cpp
             ${GEO_DIR}/Quadrilateral.cpp
             ${GEO_DIR}/SearchPoint.cpp
             ${GEO_DIR}/SearchPointVector.cpp
             ${GEO_DIR}/GeoEllipse.cpp ${GEO_DIR}/UTM.cpp)
include_directories(${XCSOAR_SRC} ${GEO_DIR})
add_library(Geo ${GEO_SRCS})

set(TERRAIN_DIR ${XCSOAR_SRC}/Terrain)
set(TERRAIN_SRCS 
                 ${TERRAIN_DIR}/RasterBuffer.cpp
                 ${TERRAIN_DIR}/RasterProjection.cpp
                 ${TERRAIN_DIR}/RasterMap.cpp
                 ${TERRAIN_DIR}/RasterTile.cpp
                 ${TERRAIN_DIR}/RasterTileCache.cpp
                 ${TERRAIN_DIR}/ZzipStream.cpp
                 ${TERRAIN_DIR}/Loader.cpp
                 ${TERRAIN_DIR}/WorldFile.cpp
                 ${TERRAIN_DIR}/Intersection.cpp
                 ${TERRAIN_DIR}/ScanLine.cpp
                 ${TERRAIN_DIR}/RasterTerrain.cpp
                 ${TERRAIN_DIR}/Thread.cpp
                 ${TERRAIN_DIR}/HeightMatrix.cpp
                 ${TERRAIN_DIR}/RasterRenderer.cpp
                 ${TERRAIN_DIR}/TerrainRenderer.cpp
                 ${TERRAIN_DIR}/TerrainSettings.cpp)
include_directories(${XCSOAR_SRC} ${TERRAIN_DIR} ${TERRAIN_DIR}/jasper)
add_library(Terrain ${TERRAIN_SRCS})
target_link_libraries(Terrain Jasper)

set(MATH_DIR ${XCSOAR_SRC}/Math)
set(MATH_SRCS 
              ${MATH_DIR}/Angle.cpp
              ${MATH_DIR}/ARange.cpp
              ${MATH_DIR}/FastMath.cpp
              ${MATH_DIR}/FastTrig.cpp
              ${MATH_DIR}/FastRotation.cpp
              ${MATH_DIR}/LeastSquares.cpp
              ${MATH_DIR}/DiffFilter.cpp
              ${MATH_DIR}/Filter.cpp
              ${MATH_DIR}/ZeroFinder.cpp
              ${MATH_DIR}/KalmanFilter1d.cpp
              ${MATH_DIR}/SelfTimingKalmanFilter1d.cpp
              ${MATH_DIR}/XYDataStore.cpp
              ${MATH_DIR}/ConvexFilter.cpp
              ${MATH_DIR}/Histogram.cpp)
include_directories(${XCSOAR_SRC} ${MATH_DIR} ${OUTPUT_INCLUDE})
add_library(Math ${MATH_SRCS})

set(NET_SRCS
             ${XCSOAR_SRC}/Net/State.cpp
             ${XCSOAR_SRC}/Net/IPv4Address.cxx
             ${XCSOAR_SRC}/Net/IPv6Address.cxx
             ${XCSOAR_SRC}/Net/StaticSocketAddress.cxx
             ${XCSOAR_SRC}/Net/AllocatedSocketAddress.cxx
             ${XCSOAR_SRC}/Net/SocketAddress.cxx
             ${XCSOAR_SRC}/Net/SocketDescriptor.cxx
             ${XCSOAR_SRC}/Net/HTTP/Multi.cpp
             ${XCSOAR_SRC}/Net/HTTP/Session.cpp
             ${XCSOAR_SRC}/Net/HTTP/Request.cpp
             ${XCSOAR_SRC}/Net/HTTP/FormData.cpp
             ${XCSOAR_SRC}/Net/HTTP/Init.cpp
             ${XCSOAR_SRC}/Net/HTTP/DownloadManager.cpp
             ${XCSOAR_SRC}/Net/HTTP/ToFile.cpp
             ${XCSOAR_SRC}/Net/HTTP/ToBuffer.cpp)
include_directories(${XCSOAR_SRC})
add_library(Net ${NET_SRCS})

set(IO_DIR ${XCSOAR_SRC}/IO)
set(IO_SRCS 
            ${IO_DIR}/BufferedReader.cxx
            ${IO_DIR}/FileReader.cxx
            ${IO_DIR}/BufferedOutputStream.cxx
            ${IO_DIR}/FileOutputStream.cxx
            ${IO_DIR}/GunzipReader.cxx
            ${IO_DIR}/ZlibError.cxx
            ${IO_DIR}/FileTransaction.cpp
            ${IO_DIR}/FileCache.cpp
            ${IO_DIR}/ZipArchive.cpp
            ${IO_DIR}/ZipReader.cpp
            ${IO_DIR}/ConvertLineReader.cpp
            ${IO_DIR}/FileLineReader.cpp
            ${IO_DIR}/KeyValueFileReader.cpp
            ${IO_DIR}/KeyValueFileWriter.cpp
            ${IO_DIR}/ZipLineReader.cpp
            ${IO_DIR}/CSVLine.cpp
            ${IO_DIR}/TextWriter.cpp)
include_directories(${XCSOAR_SRC} ${IO_DIR})
add_library(Io ${IO_SRCS})

set(WAYPOINT_DIR ${XCSOAR_SRC}/Engine/Waypoint)
set(WAYPOINT_SRCS ${WAYPOINT_DIR}/WaypointVisitor.cpp
                  ${WAYPOINT_DIR}/Waypoints.cpp
                  ${WAYPOINT_DIR}/Waypoint.cpp)
include_directories(${XCSOAR_SRC} ${WAYPOINT_DIR})
add_library(Waypoint ${WAYPOINT_SRCS})
target_link_libraries(Waypoint Geo Math)

set(ROUTE_DIR ${XCSOAR_SRC}/Engine/Route)
set(ROUTE_SRCS ${ROUTE_DIR}/Config.cpp
               ${ROUTE_DIR}/RoutePlanner.cpp
               ${ROUTE_DIR}/AirspaceRoute.cpp
               ${ROUTE_DIR}/TerrainRoute.cpp
               ${ROUTE_DIR}/RouteLink.cpp
               ${ROUTE_DIR}/RoutePolar.cpp
               ${ROUTE_DIR}/RoutePolars.cpp
               ${ROUTE_DIR}/FlatTriangleFan.cpp
               ${ROUTE_DIR}/FlatTriangleFanTree.cpp
               ${ROUTE_DIR}/ReachFan.cpp)
include_directories(${XCSOAR_SRC} ${ROUTE_DIR} ${XCSOAR_SRC}/Engine)
add_library(Route ${ROUTE_SRCS})
target_link_libraries(Route Airspace)

set(GLIDE_DIR ${XCSOAR_SRC}/Engine/GlideSolvers)
set(GLIDE_SRCS ${GLIDE_DIR}/GlideSettings.cpp
               ${GLIDE_DIR}/GlideState.cpp
               ${GLIDE_DIR}/GlueGlideState.cpp
               ${GLIDE_DIR}/GlidePolar.cpp
               ${GLIDE_DIR}/PolarCoefficients.cpp
               ${GLIDE_DIR}/GlideResult.cpp
               ${GLIDE_DIR}/MacCready.cpp
               ${GLIDE_DIR}/InstantSpeed.cpp)

include_directories(${XCSOAR_SRC} ${GLIDE_DIR})
add_library(Glide ${GLIDE_SRCS})

set(CONTEST_DIR ${XCSOAR_SRC}/Engine/Contest)
set(CONTEST_SRCS ${CONTEST_DIR}/Settings.cpp
                 ${CONTEST_DIR}/ContestManager.cpp
                 ${CONTEST_DIR}/Solvers/Contests.cpp
                 ${CONTEST_DIR}/Solvers/AbstractContest.cpp
                 ${CONTEST_DIR}/Solvers/TraceManager.cpp
                 ${CONTEST_DIR}/Solvers/ContestDijkstra.cpp
                 ${CONTEST_DIR}/Solvers/DMStQuad.cpp
                 ${CONTEST_DIR}/Solvers/OLCLeague.cpp
                 ${CONTEST_DIR}/Solvers/OLCSprint.cpp
                 ${CONTEST_DIR}/Solvers/OLCClassic.cpp
                 ${CONTEST_DIR}/Solvers/OLCTriangle.cpp
                 ${CONTEST_DIR}/Solvers/OLCFAI.cpp
                 ${CONTEST_DIR}/Solvers/OLCPlus.cpp
                 ${CONTEST_DIR}/Solvers/DMStQuad.cpp
                 ${CONTEST_DIR}/Solvers/XContestFree.cpp
                 ${CONTEST_DIR}/Solvers/XContestTriangle.cpp
                 ${CONTEST_DIR}/Solvers/OLCSISAT.cpp
                 ${CONTEST_DIR}/Solvers/NetCoupe.cpp
                 ${CONTEST_DIR}/Solvers/Retrospective.cpp)
include_directories(${XCSOAR_SRC} ${CONTEST_DIR})
add_library(Contest ${CONTEST_SRCS})

set(TASK_DIR ${XCSOAR_SRC}/Engine/Task)
set(TASK_SRCS ${TASK_DIR}/Shapes/FAITriangleSettings.cpp
              ${TASK_DIR}/Shapes/FAITriangleRules.cpp
              ${TASK_DIR}/Shapes/FAITriangleArea.cpp
              ${TASK_DIR}/Shapes/FAITriangleTask.cpp
              ${TASK_DIR}/Shapes/FAITrianglePointValidator.cpp
              ${TASK_DIR}/TaskBehaviour.cpp
              ${TASK_DIR}/TaskManager.cpp
              ${TASK_DIR}/AbstractTask.cpp
              ${TASK_DIR}/Ordered/StartConstraints.cpp
              ${TASK_DIR}/Ordered/FinishConstraints.cpp
              ${TASK_DIR}/Ordered/Settings.cpp
              ${TASK_DIR}/Ordered/OrderedTask.cpp
              ${TASK_DIR}/Ordered/TaskAdvance.cpp
              ${TASK_DIR}/Ordered/SmartTaskAdvance.cpp
              ${TASK_DIR}/Ordered/Points/IntermediatePoint.cpp
              ${TASK_DIR}/Ordered/Points/OrderedTaskPoint.cpp
              ${TASK_DIR}/Ordered/Points/StartPoint.cpp
              ${TASK_DIR}/Ordered/Points/FinishPoint.cpp
              ${TASK_DIR}/Ordered/Points/ASTPoint.cpp
              ${TASK_DIR}/Ordered/Points/AATPoint.cpp
              ${TASK_DIR}/Ordered/AATIsoline.cpp
              ${TASK_DIR}/Ordered/AATIsolineSegment.cpp
              ${TASK_DIR}/Unordered/UnorderedTask.cpp
              ${TASK_DIR}/Unordered/UnorderedTaskPoint.cpp
              ${TASK_DIR}/Unordered/GotoTask.cpp
              ${TASK_DIR}/Unordered/AbortTask.cpp
              ${TASK_DIR}/Unordered/AlternateTask.cpp
              ${TASK_DIR}/Factory/AbstractTaskFactory.cpp
              ${TASK_DIR}/Factory/RTTaskFactory.cpp
              ${TASK_DIR}/Factory/FAITaskFactory.cpp
              ${TASK_DIR}/Factory/FAITriangleTaskFactory.cpp
              ${TASK_DIR}/Factory/FAIORTaskFactory.cpp
              ${TASK_DIR}/Factory/FAIGoalTaskFactory.cpp
              ${TASK_DIR}/Factory/AATTaskFactory.cpp
              ${TASK_DIR}/Factory/MatTaskFactory.cpp
              ${TASK_DIR}/Factory/MixedTaskFactory.cpp
              ${TASK_DIR}/Factory/TouringTaskFactory.cpp
              ${TASK_DIR}/Factory/Create.cpp
              ${TASK_DIR}/Points/TaskPoint.cpp
              ${TASK_DIR}/Points/SampledTaskPoint.cpp
              ${TASK_DIR}/Points/ScoredTaskPoint.cpp
              ${TASK_DIR}/Points/TaskLeg.cpp
              ${TASK_DIR}/ObservationZones/Boundary.cpp
              ${TASK_DIR}/ObservationZones/ObservationZoneClient.cpp
              ${TASK_DIR}/ObservationZones/ObservationZonePoint.cpp
              ${TASK_DIR}/ObservationZones/CylinderZone.cpp
              ${TASK_DIR}/ObservationZones/SectorZone.cpp
              ${TASK_DIR}/ObservationZones/LineSectorZone.cpp
              ${TASK_DIR}/ObservationZones/SymmetricSectorZone.cpp
              ${TASK_DIR}/ObservationZones/KeyholeZone.cpp
              ${TASK_DIR}/ObservationZones/AnnularSectorZone.cpp
              ${TASK_DIR}/PathSolvers/TaskDijkstra.cpp
              ${TASK_DIR}/PathSolvers/TaskDijkstraMin.cpp
              ${TASK_DIR}/PathSolvers/TaskDijkstraMax.cpp
              ${TASK_DIR}/PathSolvers/IsolineCrossingFinder.cpp
              ${TASK_DIR}/Solvers/TaskMacCready.cpp
              ${TASK_DIR}/Solvers/TaskMacCreadyTravelled.cpp
              ${TASK_DIR}/Solvers/TaskMacCreadyRemaining.cpp
              ${TASK_DIR}/Solvers/TaskMacCreadyTotal.cpp
              ${TASK_DIR}/Solvers/TaskBestMc.cpp
              ${TASK_DIR}/Solvers/TaskSolveTravelled.cpp
              ${TASK_DIR}/Solvers/TaskCruiseEfficiency.cpp
              ${TASK_DIR}/Solvers/TaskEffectiveMacCready.cpp
              ${TASK_DIR}/Solvers/TaskMinTarget.cpp
              ${TASK_DIR}/Solvers/TaskOptTarget.cpp
              ${TASK_DIR}/Solvers/TaskGlideRequired.cpp
              ${TASK_DIR}/Solvers/TaskSolution.cpp
              ${TASK_DIR}/Computer/ElementStatComputer.cpp
              ${TASK_DIR}/Computer/DistanceStatComputer.cpp
              ${TASK_DIR}/Computer/IncrementalSpeedComputer.cpp
              ${TASK_DIR}/Computer/TaskVarioComputer.cpp
              ${TASK_DIR}/Computer/TaskStatsComputer.cpp
              ${TASK_DIR}/Computer/WindowStatsComputer.cpp
              ${TASK_DIR}/Stats/CommonStats.cpp
              ${TASK_DIR}/Stats/ElementStat.cpp
              ${TASK_DIR}/Stats/TaskStats.cpp
              ${TASK_DIR}/Stats/StartStats.cpp)
include_directories(${XCSOAR_SRC} ${TASK_DIR})
add_library(Task ${TASK_SRCS})
target_link_libraries(Task Glide Time)

set(AIRSPACE_DIR ${XCSOAR_SRC}/Engine/Airspace)
set(ENGINE_DIR   ${XCSOAR_SRC}/Engine)
set(AIRSPACE_SRCS 
                  ${ENGINE_DIR}/Util/AircraftStateFilter.cpp
                  ${AIRSPACE_DIR}/AirspacesTerrain.cpp
                  ${AIRSPACE_DIR}/Airspace.cpp
                  ${AIRSPACE_DIR}/AirspaceAltitude.cpp
                  ${AIRSPACE_DIR}/AirspaceAircraftPerformance.cpp
                  ${AIRSPACE_DIR}/AbstractAirspace.cpp
                  ${AIRSPACE_DIR}/AirspaceCircle.cpp
                  ${AIRSPACE_DIR}/AirspacePolygon.cpp
                  ${AIRSPACE_DIR}/Airspaces.cpp
                  ${AIRSPACE_DIR}/AirspaceIntersectSort.cpp
                  ${AIRSPACE_DIR}/SoonestAirspace.cpp
                  ${AIRSPACE_DIR}/Predicate/AirspacePredicate.cpp
                  ${AIRSPACE_DIR}/Predicate/AirspacePredicateHeightRange.cpp
                  ${AIRSPACE_DIR}/Predicate/OutsideAirspacePredicate.cpp
                  ${AIRSPACE_DIR}/AirspaceIntersectionVisitor.cpp
                  ${AIRSPACE_DIR}/AirspaceWarningConfig.cpp
                  ${AIRSPACE_DIR}/AirspaceWarningManager.cpp
                  ${AIRSPACE_DIR}/AirspaceWarning.cpp
                  ${AIRSPACE_DIR}/AirspaceSorter.cpp)
include_directories(${XCSOAR_SRC} ${AIRSPACE_DIR})
add_library(Airspace ${AIRSPACE_SRCS})

# Device drivers
set(DEVICE_DIR ${XCSOAR_SRC}/Device)
set(DRIVER_DIR ${DEVICE_DIR}/Driver)
set(DRIVER_SRCS 
                ${DRIVER_DIR}/Volkslogger/Register.cpp
                ${DRIVER_DIR}/Volkslogger/Parser.cpp
                ${DRIVER_DIR}/Volkslogger/Protocol.cpp
                ${DRIVER_DIR}/Volkslogger/Declare.cpp
                ${DRIVER_DIR}/Volkslogger/Database.cpp
                ${DRIVER_DIR}/Volkslogger/Util.cpp
                ${DRIVER_DIR}/Volkslogger/dbbconv.cpp
                ${DRIVER_DIR}/Volkslogger/grecord.cpp
                ${DRIVER_DIR}/Volkslogger/vlapi2.cpp
                ${DRIVER_DIR}/Volkslogger/vlapihlp.cpp
                ${DRIVER_DIR}/Volkslogger/vlconv.cpp
                ${DRIVER_DIR}/Volkslogger/Logger.cpp
                ${DRIVER_DIR}/CAI302/Protocol.cpp
                ${DRIVER_DIR}/CAI302/PocketNav.cpp
                ${DRIVER_DIR}/CAI302/Mode.cpp
                ${DRIVER_DIR}/CAI302/Parser.cpp
                ${DRIVER_DIR}/CAI302/Settings.cpp
                ${DRIVER_DIR}/CAI302/Declare.cpp
                ${DRIVER_DIR}/CAI302/Logger.cpp
                ${DRIVER_DIR}/CAI302/Manage.cpp
                ${DRIVER_DIR}/CAI302/Register.cpp
                ${DRIVER_DIR}/IMI/Protocol/MessageParser.cpp
                ${DRIVER_DIR}/IMI/Protocol/Communication.cpp
                ${DRIVER_DIR}/IMI/Protocol/Checksum.cpp
                ${DRIVER_DIR}/IMI/Protocol/Conversion.cpp
                ${DRIVER_DIR}/IMI/Protocol/IGC.cpp
                ${DRIVER_DIR}/IMI/Protocol/Protocol.cpp
                ${DRIVER_DIR}/IMI/Declare.cpp
                ${DRIVER_DIR}/IMI/Internal.cpp
                ${DRIVER_DIR}/IMI/Logger.cpp
                ${DRIVER_DIR}/IMI/Register.cpp
                ${DRIVER_DIR}/LX/NanoLogger.cpp
                ${DRIVER_DIR}/LX/NanoDeclare.cpp
                ${DRIVER_DIR}/LX/Protocol.cpp
                ${DRIVER_DIR}/LX/Mode.cpp
                ${DRIVER_DIR}/LX/Parser.cpp
                ${DRIVER_DIR}/LX/Settings.cpp
                ${DRIVER_DIR}/LX/Declare.cpp
                ${DRIVER_DIR}/LX/Logger.cpp
                ${DRIVER_DIR}/LX/Convert.cpp
                ${DRIVER_DIR}/LX/LXN.cpp
                ${DRIVER_DIR}/LX/Register.cpp
                ${DRIVER_DIR}/FLARM/Device.cpp
                ${DRIVER_DIR}/FLARM/Register.cpp
                ${DRIVER_DIR}/FLARM/Mode.cpp
                ${DRIVER_DIR}/FLARM/Parser.cpp
                ${DRIVER_DIR}/FLARM/StaticParser.cpp
                ${DRIVER_DIR}/FLARM/Settings.cpp
                ${DRIVER_DIR}/FLARM/Declare.cpp 
                ${DRIVER_DIR}/FLARM/Logger.cpp
                ${DRIVER_DIR}/FLARM/CRC16.cpp
                ${DRIVER_DIR}/FLARM/BinaryProtocol.cpp
                ${DRIVER_DIR}/FLARM/TextProtocol.cpp
                ${DRIVER_DIR}/Flytec/Register.cpp
                ${DRIVER_DIR}/Flytec/Parser.cpp
                ${DRIVER_DIR}/Flytec/Logger.cpp
                ${DRIVER_DIR}/Vega/Misc.cpp
                ${DRIVER_DIR}/Vega/Parser.cpp
                ${DRIVER_DIR}/Vega/Settings.cpp
                ${DRIVER_DIR}/Vega/Volatile.cpp
                ${DRIVER_DIR}/Vega/Register.cpp
                ${DRIVER_DIR}/BlueFly/Misc.cpp
                ${DRIVER_DIR}/BlueFly/Parser.cpp
                ${DRIVER_DIR}/BlueFly/Settings.cpp
                ${DRIVER_DIR}/BlueFly/Register.cpp
                ${DRIVER_DIR}/XCTracer/Parser.cpp
                ${DRIVER_DIR}/XCTracer/Register.cpp
                ${DRIVER_DIR}/ThermalExpress/Driver.cpp
                ${DEVICE_DIR}/Driver.cpp
                ${DEVICE_DIR}/Register.cpp
                ${DRIVER_DIR}/AltairPro.cpp
                ${DRIVER_DIR}/BorgeltB50.cpp
                ${DRIVER_DIR}/CaiGpsNav.cpp
                ${DRIVER_DIR}/CaiLNav.cpp
                ${DRIVER_DIR}/Condor.cpp
                ${DRIVER_DIR}/CProbe.cpp
                ${DRIVER_DIR}/EW.cpp
                ${DRIVER_DIR}/EWMicroRecorder.cpp
                ${DRIVER_DIR}/Eye.cpp
                ${DRIVER_DIR}/FlymasterF1.cpp
                ${DRIVER_DIR}/FlyNet.cpp
                ${DRIVER_DIR}/Generic.cpp
                ${DRIVER_DIR}/LevilAHRS_G.cpp
                ${DRIVER_DIR}/Leonardo.cpp
                ${DRIVER_DIR}/NmeaOut.cpp
                ${DRIVER_DIR}/OpenVario.cpp
                ${DRIVER_DIR}/PosiGraph.cpp
                ${DRIVER_DIR}/XCOM760.cpp
                ${DRIVER_DIR}/ILEC.cpp
                ${DRIVER_DIR}/Westerboer.cpp
                ${DRIVER_DIR}/Zander.cpp
                ${DRIVER_DIR}/Vaulter.cpp
                ${DRIVER_DIR}/KRT2.cpp
                ${DRIVER_DIR}/ATR833.cpp)
include_directories(${XCSOAR_SRC} ${DRIVER_DIR})
add_library(Driver ${DRIVER_SRCS})
target_link_libraries(Driver Port)

set(PORT_DIR ${XCSOAR_SRC}/Device/Port)
set(PORT_SRCS 
              ${PORT_DIR}/Port.cpp
              ${PORT_DIR}/BufferedPort.cpp
              ${PORT_DIR}/UDPPort.cpp
              ${PORT_DIR}/TCPPort.cpp
              ${PORT_DIR}/K6BtPort.cpp
              ${PORT_DIR}/DumpPort.cpp
              ${PORT_DIR}/NullPort.cpp
              ${PORT_DIR}/TTYEnumerator.cpp
              ${PORT_DIR}/TTYPort.cpp
              ${PORT_DIR}/TCPClientPort.cpp)
if(HAVE_POSIX)
set(PORT_SRCS ${PORT_SRCS}
              ${PORT_DIR}/TTYEnumerator.cpp
              ${PORT_DIR}/TTYPort.cpp)
endif(HAVE_POSIX)
add_library(Port ${PORT_SRCS})
target_link_libraries(Port Os)

set(SHAPE_DIR ${XCSOAR_SRC}/Topography/shapelib)
set(SHAPE_SRCS 
               ${SHAPE_DIR}/mapalloc.c
               ${SHAPE_DIR}/mapstring.c
               ${SHAPE_DIR}/mapbits.c
               ${SHAPE_DIR}/mapprimitive.c
               ${SHAPE_DIR}/mapsearch.c
               ${SHAPE_DIR}/mapshape.c
               ${SHAPE_DIR}/maptree.c
               ${SHAPE_DIR}/mapxbase.c)
include_directories(${XCSOAR_SRC} ${SHAPE_DIR})
add_library(Shape ${SHAPE_SRCS})

set(THREAD_DIR ${XCSOAR_SRC}/Thread)
set(THREAD_SRCS 
                ${THREAD_DIR}/Thread.cpp
                ${THREAD_DIR}/SuspensibleThread.cpp
                ${THREAD_DIR}/RecursivelySuspensibleThread.cpp
                ${THREAD_DIR}/WorkerThread.cpp
                ${THREAD_DIR}/StandbyThread.cpp
                ${THREAD_DIR}/Debug.cpp)
include_directories(${XCSOAR_SRC} ${THREAD_DIR})
add_library(Thread ${THREAD_SRCS})
target_link_libraries(Thread pthread)

set(OS_DIR ${XCSOAR_SRC}/OS)
set(OS_SRCS	
            ${OS_DIR}/Clock.cpp
            ${OS_DIR}/FileDescriptor.cxx
            ${OS_DIR}/FileMapping.cpp
            ${OS_DIR}/FileUtil.cpp
            ${OS_DIR}/RunFile.cpp
            ${OS_DIR}/Path.cpp
            ${OS_DIR}/PathName.cpp
            ${OS_DIR}/Process.cpp
            ${OS_DIR}/SystemLoad.cpp
            ${OS_DIR}/EventPipe.cpp)
include_directories(${XCSOAR_SRC} ${OS_DIR})
add_library(Os ${OS_SRCS})

set(WIDGET_DIR ${XCSOAR_SRC}/Widget)
set(WIDGET_SRCS 
                ${WIDGET_DIR}/Widget.cpp
                ${WIDGET_DIR}/ActionWidget.cpp
                ${WIDGET_DIR}/CallbackWidget.cpp
                ${WIDGET_DIR}/WindowWidget.cpp
                ${WIDGET_DIR}/CreateWindowWidget.cpp
                ${WIDGET_DIR}/ListWidget.cpp
                ${WIDGET_DIR}/TextListWidget.cpp
                ${WIDGET_DIR}/ContainerWidget.cpp
                ${WIDGET_DIR}/SolidWidget.cpp
                ${WIDGET_DIR}/PanelWidget.cpp
                ${WIDGET_DIR}/TabWidget.cpp
                ${WIDGET_DIR}/TextWidget.cpp
                ${WIDGET_DIR}/LargeTextWidget.cpp
                ${WIDGET_DIR}/OverlappedWidget.cpp
                ${WIDGET_DIR}/TwoWidgets.cpp
                ${WIDGET_DIR}/RowFormWidget.cpp
                ${WIDGET_DIR}/EditRowFormWidget.cpp
                ${WIDGET_DIR}/ProfileRowFormWidget.cpp
                ${WIDGET_DIR}/UnitRowFormWidget.cpp
                ${WIDGET_DIR}/ManagedWidget.cpp
                ${WIDGET_DIR}/PagerWidget.cpp
                ${WIDGET_DIR}/ArrowPagerWidget.cpp
                ${WIDGET_DIR}/OffsetButtonsWidget.cpp
                ${WIDGET_DIR}/ButtonPanelWidget.cpp
                ${WIDGET_DIR}/ButtonWidget.cpp
                ${WIDGET_DIR}/QuestionWidget.cpp
                ${WIDGET_DIR}/KeyboardWidget.cpp
                ${WIDGET_DIR}/ViewImageWidget.cpp
                ${WIDGET_DIR}/DockWindow.cpp)
include_directories(${XCSOAR_SRC} ${WIDGET_DIR})
add_library(Widget ${WIDGET_SRCS})
target_link_libraries(Widget Form Datafield)

include_directories(${XCSOAR_SRC} ${XCSOAR_SRC}/Profile)
add_library(Profile ${XCSOAR_SRC}/Profile/ProfileMap.cpp
                    ${XCSOAR_SRC}/Profile/File.cpp
                    ${XCSOAR_SRC}/Profile/Current.cpp
                    ${XCSOAR_SRC}/Profile/Map.cpp
                    ${XCSOAR_SRC}/Profile/StringValue.cpp
                    ${XCSOAR_SRC}/Profile/NumericValue.cpp
                    ${XCSOAR_SRC}/Profile/PathValue.cpp
                    ${XCSOAR_SRC}/Profile/GeoValue.cpp
                    ${XCSOAR_SRC}/Profile/ProfileKeys.cpp
                    ${XCSOAR_SRC}/Profile/ProfileMap.cpp)
target_link_libraries(Profile Io)

set(JASPER_DIR ${XCSOAR_SRC}/Terrain/jasper)
set(JASPER_SRCS 
                ${JASPER_DIR}/base/jas_malloc.c
                ${JASPER_DIR}/base/jas_seq.c
                ${JASPER_DIR}/base/jas_stream.c
                ${JASPER_DIR}/base/jas_string.c
                ${JASPER_DIR}/base/jas_tvp.c
                ${JASPER_DIR}/jp2/jp2_cod.c
                ${JASPER_DIR}/jpc/jpc_bs.c
                ${JASPER_DIR}/jpc/jpc_cs.c
                ${JASPER_DIR}/jpc/jpc_dec.c
                ${JASPER_DIR}/jpc/jpc_math.c
                ${JASPER_DIR}/jpc/jpc_mqdec.c
                ${JASPER_DIR}/jpc/jpc_mqcod.c
                ${JASPER_DIR}/jpc/jpc_qmfb.c
                ${JASPER_DIR}/jpc/jpc_rtc.cpp
                ${JASPER_DIR}/jpc/jpc_t1dec.c
                ${JASPER_DIR}/jpc/jpc_t1cod.c
                ${JASPER_DIR}/jpc/jpc_t2dec.c
                ${JASPER_DIR}/jpc/jpc_t2cod.c
                ${JASPER_DIR}/jpc/jpc_tagtree.c
                ${JASPER_DIR}/jpc/jpc_tsfb.c)
include_directories(${XCSOAR_SRC} ${JASPER_DIR})
add_library(Jasper ${JASPER_SRCS})
set_target_properties(Jasper
                      PROPERTIES COMPILE_FLAGS "-Wno-unused-but-set-parameter -Wno-unused-but-set-variable -Wno-type-limits -Wno-shift-negative-value -Wno-sign-compare -Wno-tautological-compare")
target_link_libraries(Jasper Zzip)

set(ZZIP_DIR ${XCSOAR_SRC}/zzip)
set(ZZIP_SRCS ${ZZIP_DIR}/fetch.c
              ${ZZIP_DIR}/file.c
              ${ZZIP_DIR}/plugin.c
              ${ZZIP_DIR}/zip.c
              ${ZZIP_DIR}/stat.c)
add_library(Zzip ${ZZIP_SRCS})
target_link_libraries(Zzip z)

set(FORM_DIR ${XCSOAR_SRC}/Form)
set(FORM_SRCS	
              ${XCSOAR_SRC}/UIUtil/KineticManager.cpp
              ${XCSOAR_SRC}/Renderer/TextRenderer.cpp
              ${XCSOAR_SRC}/Renderer/TabRenderer.cpp
              ${XCSOAR_SRC}/Renderer/ButtonRenderer.cpp
              ${XCSOAR_SRC}/Renderer/TextButtonRenderer.cpp
              ${XCSOAR_SRC}/Renderer/SymbolRenderer.cpp
              ${XCSOAR_SRC}/Renderer/SymbolButtonRenderer.cpp
              ${XCSOAR_SRC}/Renderer/BitmapButtonRenderer.cpp
              ${XCSOAR_SRC}/Renderer/ColorButtonRenderer.cpp
              ${FORM_DIR}/Control.cpp
              ${FORM_DIR}/Panel.cpp
              ${FORM_DIR}/Form.cpp
              ${FORM_DIR}/Button.cpp
              ${FORM_DIR}/ButtonPanel.cpp
              ${FORM_DIR}/CheckBox.cpp
              ${FORM_DIR}/Frame.cpp
              ${FORM_DIR}/Draw.cpp
              ${FORM_DIR}/List.cpp
              ${FORM_DIR}/ScrollBar.cpp
              ${FORM_DIR}/Edit.cpp
              ${FORM_DIR}/DigitEntry.cpp
              ${FORM_DIR}/TabDisplay.cpp
              ${FORM_DIR}/TabMenuDisplay.cpp
              ${FORM_DIR}/CharacterButton.cpp
              ${FORM_DIR}/GridView.cpp
              ${FORM_DIR}/HLine.cpp)
include_directories(${XCSOAR_SRC} ${FORM_DIR})
add_library(Form ${FORM_SRCS})
target_link_libraries(Form Screen)

set(DATAFIELD_DIR ${XCSOAR_SRC}/Form/DataField)
set(DATAFIELD_SRCS 	
                   ${DATAFIELD_DIR}/Base.cpp
                   ${DATAFIELD_DIR}/Boolean.cpp
                   ${DATAFIELD_DIR}/ComboList.cpp
                   ${DATAFIELD_DIR}/Enum.cpp
                   ${DATAFIELD_DIR}/File.cpp
                   ${DATAFIELD_DIR}/Number.cpp
                   ${DATAFIELD_DIR}/Float.cpp
                   ${DATAFIELD_DIR}/Angle.cpp
                   ${DATAFIELD_DIR}/GeoPoint.cpp
                   ${DATAFIELD_DIR}/RoughTime.cpp
                   ${DATAFIELD_DIR}/Time.cpp
                   ${DATAFIELD_DIR}/Integer.cpp
                   ${DATAFIELD_DIR}/String.cpp
                   ${DATAFIELD_DIR}/Prefix.cpp
                   ${DATAFIELD_DIR}/Password.cpp)
include_directories(${XCSOAR_SRC} ${DATAFIELD_DIR})
add_library(Datafield ${DATAFIELD_SRCS})

set(TIME_DIR ${XCSOAR_SRC}/Time)
set(TIME_SRCS ${TIME_DIR}/DeltaTime.cpp
              ${TIME_DIR}/WrapClock.cpp
              ${TIME_DIR}/LocalTime.cpp
              ${TIME_DIR}/BrokenTime.cpp
              ${TIME_DIR}/BrokenDate.cpp
              ${TIME_DIR}/BrokenDateTime.cpp)
add_library(Time ${TIME_SRCS})

set(SCREEN_DIR ${XCSOAR_SRC}/Screen)
set(SCREEN_SRCS 
                ${SCREEN_DIR}/Debug.cpp
                ${SCREEN_DIR}/ProgressBar.cpp
                ${SCREEN_DIR}/Util.cpp
                ${SCREEN_DIR}/Icon.cpp
                ${SCREEN_DIR}/Canvas.cpp
                ${SCREEN_DIR}/Color.cpp
                ${SCREEN_DIR}/BufferCanvas.cpp
                ${SCREEN_DIR}/Window.cpp
                ${SCREEN_DIR}/SolidContainerWindow.cpp
                ${SCREEN_DIR}/BufferWindow.cpp
                ${SCREEN_DIR}/DoubleBufferWindow.cpp
                ${SCREEN_DIR}/SingleWindow.cpp
                ${SCREEN_DIR}/FreeType/Font.cpp
                ${SCREEN_DIR}/FreeType/Init.cpp
                ${SCREEN_DIR}/X11/TopWindow.cpp
                ${SCREEN_DIR}/Custom/Cache.cpp
                ${SCREEN_DIR}/OpenGL/Init.cpp
                ${SCREEN_DIR}/OpenGL/Dynamic.cpp
                ${SCREEN_DIR}/OpenGL/Rotate.cpp
                ${SCREEN_DIR}/OpenGL/Geo.cpp
                ${SCREEN_DIR}/OpenGL/Globals.cpp
                ${SCREEN_DIR}/OpenGL/Extension.cpp
                ${SCREEN_DIR}/OpenGL/FBO.cpp
                ${SCREEN_DIR}/OpenGL/VertexArray.cpp
                ${SCREEN_DIR}/OpenGL/ConstantAlpha.cpp
                ${SCREEN_DIR}/OpenGL/Bitmap.cpp
                ${SCREEN_DIR}/OpenGL/RawBitmap.cpp
                ${SCREEN_DIR}/OpenGL/Canvas.cpp
                ${SCREEN_DIR}/OpenGL/BufferCanvas.cpp
                ${SCREEN_DIR}/OpenGL/TopCanvas.cpp
                ${SCREEN_DIR}/OpenGL/SubCanvas.cpp
                ${SCREEN_DIR}/OpenGL/Texture.cpp
                ${SCREEN_DIR}/OpenGL/UncompressedImage.cpp
                ${SCREEN_DIR}/OpenGL/Buffer.cpp
                ${SCREEN_DIR}/OpenGL/Shapes.cpp
                ${SCREEN_DIR}/OpenGL/Surface.cpp
                ${SCREEN_DIR}/OpenGL/Triangulate.cpp
                ${SCREEN_DIR}/Custom/Files.cpp
                ${SCREEN_DIR}/Custom/Bitmap.cpp
                ${SCREEN_DIR}/Custom/ResourceBitmap.cpp
                ${SCREEN_DIR}/GLX/Init.cpp
                ${SCREEN_DIR}/GLX/TopCanvas.cpp
                ${SCREEN_DIR}/FB/Window.cpp
                ${SCREEN_DIR}/FB/TopWindow.cpp
                ${SCREEN_DIR}/FB/SingleWindow.cpp
                ${SCREEN_DIR}/Custom/GeoBitmap.cpp
                ${SCREEN_DIR}/Custom/Pen.cpp
                ${SCREEN_DIR}/Custom/Timer.cpp
                ${SCREEN_DIR}/Custom/LargeTextWindow.cpp
                ${SCREEN_DIR}/Custom/Window.cpp
                ${SCREEN_DIR}/Custom/WList.cpp
                ${SCREEN_DIR}/Custom/ContainerWindow.cpp
                ${SCREEN_DIR}/Custom/TopWindow.cpp
                ${SCREEN_DIR}/Custom/SingleWindow.cpp
                ${SCREEN_DIR}/Custom/MoreCanvas.cpp 
                ${SCREEN_DIR}/Custom/LibPNG.cpp
                ${SCREEN_DIR}/Custom/LibJPEG.cpp
                ${SCREEN_DIR}/Custom/LibTiff.cpp
                ${SCREEN_DIR}/Custom/LibPNG.cpp
                ${SCREEN_DIR}/Custom/LibJPEG.cpp
                ${SCREEN_DIR}/Custom/LibTiff.cpp)
include_directories(${XCSOAR_SRC}
                    ${SCREEN_DIR}
                    ${SCREEN_DIR}/FreeType
                    ${SCREEN_DIR}/OpenGL)
add_library(Screen ${SCREEN_SRCS})
# TODO Find this package.
set_target_properties(Screen
                      PROPERTIES COMPILE_FLAGS "-isystem /usr/include/freetype2")
target_link_libraries(Screen Resource GL pthread freetype)
add_dependencies(Screen resources)

set(LUA_DIR ${XCSOAR_SRC}/Lua)
set(LUA_SRCS 
             ${LUA_DIR}/Ptr.cpp 
             ${LUA_DIR}/Error.cpp 
             ${LUA_DIR}/Catch.cpp 
             ${LUA_DIR}/Persistent.cpp 
             ${LUA_DIR}/Background.cpp 
             ${LUA_DIR}/Associate.cpp 
             ${LUA_DIR}/RunFile.cpp 
             ${LUA_DIR}/StartFile.cpp 
             ${LUA_DIR}/Log.cpp 
             ${LUA_DIR}/Timer.cpp 
             ${LUA_DIR}/Geo.cpp 
             ${LUA_DIR}/Map.cpp 
             ${LUA_DIR}/Blackboard.cpp 
             ${LUA_DIR}/Airspace.cpp 
             ${LUA_DIR}/Dialogs.cpp 
             ${LUA_DIR}/Legacy.cpp 
             ${LUA_DIR}/Full.cpp 
             ${LUA_DIR}/Basic.cpp 
             ${LUA_DIR}/Task.cpp 
             ${LUA_DIR}/Settings.cpp 
             ${LUA_DIR}/Wind.cpp 
             ${LUA_DIR}/Logger.cpp 
             ${LUA_DIR}/Tracking.cpp 
             ${LUA_DIR}/Replay.cpp 
             ${LUA_DIR}/InputEvent.cpp
   )
add_library(Lua ${LUA_SRCS})
target_link_libraries(Lua Look)
set_target_properties(Lua
                      PROPERTIES COMPILE_FLAGS "-isystem /usr/include/lua5.2 -DUSE_FREETYPE -DENABLE_OPENGL -DGL_GLEXT_PROTOTYPES -DUSE_POLL_EVENT")

set(LOOK_DIR ${XCSOAR_SRC}/Look)
set(LOOK_SRCS 
              ${LOOK_DIR}/AutoFont.cpp 
              ${LOOK_DIR}/Look.cpp 
              ${LOOK_DIR}/DialogLook.cpp 
              ${LOOK_DIR}/ButtonLook.cpp 
              ${LOOK_DIR}/CheckBoxLook.cpp 
              ${LOOK_DIR}/TerminalLook.cpp 
              ${LOOK_DIR}/VarioLook.cpp 
              ${LOOK_DIR}/ChartLook.cpp 
              ${LOOK_DIR}/MapLook.cpp 
              ${LOOK_DIR}/OverlayLook.cpp 
              ${LOOK_DIR}/TopographyLook.cpp 
              ${LOOK_DIR}/WindArrowLook.cpp 
              ${LOOK_DIR}/ThermalBandLook.cpp 
              ${LOOK_DIR}/TraceHistoryLook.cpp 
              ${LOOK_DIR}/AirspaceLook.cpp 
              ${LOOK_DIR}/TrailLook.cpp 
              ${LOOK_DIR}/CrossSectionLook.cpp 
              ${LOOK_DIR}/GestureLook.cpp 
              ${LOOK_DIR}/HorizonLook.cpp 
              ${LOOK_DIR}/TaskLook.cpp 
              ${LOOK_DIR}/TrafficLook.cpp 
              ${LOOK_DIR}/InfoBoxLook.cpp 
              ${LOOK_DIR}/WaypointLook.cpp 
              ${LOOK_DIR}/AircraftLook.cpp 
              ${LOOK_DIR}/NOAALook.cpp 
              ${LOOK_DIR}/FinalGlideBarLook.cpp 
              ${LOOK_DIR}/FlarmTrafficLook.cpp 
              ${LOOK_DIR}/VarioBarLook.cpp 
              ${LOOK_DIR}/IconLook.cpp 
              ${LOOK_DIR}/ThermalAssistantLook.cpp 
              ${LOOK_DIR}/WaveLook.cpp 
              ${LOOK_DIR}/ClimbPercentLook.cpp
   )
add_library(Look ${LOOK_SRCS})
target_link_libraries(Look Screen)
set_target_properties(Look
                      PROPERTIES COMPILE_FLAGS "-DUSE_FREETYPE -DENABLE_OPENGL -DGL_GLEXT_PROTOTYPES -DUSE_POLL_EVENT")

set(EVENT_DIR ${XCSOAR_SRC}/Event)
set(EVENT_SRCS 	${EVENT_DIR}/Globals.cpp
                ${EVENT_DIR}/Idle.cpp
                ${EVENT_DIR}/DelayedNotify.cpp
                ${EVENT_DIR}/Notify.cpp
                ${EVENT_DIR}/Poll/Timer.cpp
                ${EVENT_DIR}/Poll/Loop.cpp
                ${EVENT_DIR}/Poll/Queue.cpp
                ${EVENT_DIR}/Poll/X11Queue.cpp)
include_directories(${XCSOAR_SRC} ${EVENT_DIR})
add_library(Event ${EVENT_SRCS})
target_link_libraries(Event SDL)

include_directories(${XCSOAR_SRC})
add_library(Resource ${XCSOAR_SRC}/ResourceLoader.cpp)

set(ASYNC_DIR ${XCSOAR_SRC}/IO/Async)
set(ASYNC_SRCS 	${ASYNC_DIR}/AsioThread.cpp 
	              ${ASYNC_DIR}/GlobalAsioThread.cpp
                ${ASYNC_DIR}/SignalListener.cpp)
include_directories(${XCSOAR_SRC} ${ASYNC_DIR})
add_library(Async ${ASYNC_SRCS})

set(RESOURCE_SRCS ${OUTPUT_BASE}/resources.c)
add_library(Resources ${RESOURCE_SRCS})

set(MO_DIR ${OUTPUT_BASE}/po)
add_custom_target(make_output_po
                  COMMAND [ -d ${MO_DIR} ] || mkdir ${MO_DIR}
                 )
set(MO_SRCS
            ca
            he
            nl
            sr
            cs
            hr
            pl
            sv
            da
            hu
            pt
            tr
            de
            it
            pt_BR
            uk
            el
            ja
            ro
            vi
            es
            ko
            ru
            zh_CN
            fi
            lt
            sk
            fr
            nb
            sl
   )
foreach(mo IN LISTS MO_SRCS)
  set(fqs ${MO_DIR}/${mo}.mo)
  set(fqd ${OUTPUT_BASE}/include/${mo}.mo.c)
  add_custom_command(OUTPUT ${fqd}
                     COMMAND msgfmt --check -o ${fqs} ${XCSOAR}/po/${mo}.po
                     COMMAND perl -I${XCSOAR}/tools ${TOOLS_DIR}/BinToC.pl ${fqs} ${fqd}
                     DEPENDS make_output_po
                    )
  set(DATA_SRCS ${DATA_SRCS} ${fqd})
endforeach(mo)
set(DATA_RESOURCES 
                   COPYING
                   AUTHORS
   )
foreach(dr IN LISTS DATA_RESOURCES)
  set(fqs ${OUTPUT_DATA}/${dr}.gz)
  set(fqd ${OUTPUT_DATA}/${dr}.gz.c)
  add_custom_command(OUTPUT ${fqd}
                     COMMAND perl -I${XCSOAR}/tools ${TOOLS_DIR}/BinToC.pl ${fqs} ${fqd}
                    )
  set(DATA_SRCS ${DATA_SRCS} ${fqd})
endforeach(dr)
add_custom_command(OUTPUT ${OUTPUT_DATA}/egm96s.dem.c
                   COMMAND perl -I${XCSOAR}/tools ${TOOLS_DIR}/BinToC.pl ${XCSOAR}/Data/other/egm96s.dem ${OUTPUT_DATA}/egm96s.dem.c 
                  )
add_library(Data ${DATA_SRCS} ${OUTPUT_DATA}/egm96s.dem.c)
add_custom_target(lib_debug
                  COMMAND echo "PWD: `pwd`"
                 )
