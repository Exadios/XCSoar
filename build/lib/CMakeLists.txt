include_directories(${XCSOAR_SRC})
add_definitions(-DHAVE_POSIX) # For POSIX only
add_definitions(-DZLIB_CONST) # Make some of zlib const.

add_custom_target(lib DEPENDS Airspace
                              Async
                              Contest
                              Datafield
                              Driver
                              Event
                              Form
                              Geo
                              Glide
                              Io
                              Jasper
                              Math
                              Os
                              Profile
#                              Resource
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
add_definitions(-DHAVE_POSIX
                -DENABLE_OPENGL
                -DGL_GLEXT_PROTOTYPES
                -DUSE_GLX
                -DUSE_X11
                -DUSE_POLL_EVENT
                -DENABLE_SDL
                -DUSE_FREETYPE
                -DHAVE_STDINT_H
                -DHAVE_UNISTD_H
                -DHAVE_VASPRINTF
                -DRADIANS
                -DEYE_CANDY)
include_directories(SYSTEM /usr/include/SDL)

set(UTIL_DIR ${XCSOAR_SRC}/Util)
set(UTIL_SRCS ${UTIL_DIR}/CRC.cpp
              ${UTIL_DIR}/tstring.cpp
              ${UTIL_DIR}/UTF8.cpp
              ${UTIL_DIR}/EscapeBackslash.cpp
              ${UTIL_DIR}/ConvertString.cpp
              ${UTIL_DIR}/StaticString.cpp
              ${UTIL_DIR}/StringUtil.cpp)
include_directories(${XCSOAR_SRC} ${XCSOAR_SRC}/unix ${UTIL_DIR})
add_library(Util ${UTIL_SRCS})

set(GEO_DIR ${XCSOAR_SRC}/Geo)
set(GEO_SRCS ${GEO_DIR}/ConvexHull/GrahamScan.cpp
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
	           ${GEO_DIR}/GeoPoint.cpp
	           ${GEO_DIR}/GeoVector.cpp
	           ${GEO_DIR}/GeoBounds.cpp
	           ${GEO_DIR}/GeoClip.cpp
	           ${GEO_DIR}/SearchPoint.cpp
	           ${GEO_DIR}/SearchPointVector.cpp
	           ${GEO_DIR}/GeoEllipse.cpp
	           ${GEO_DIR}/UTM.cpp)
include_directories(${XCSOAR_SRC} ${GEO_DIR})
add_library(Geo ${GEO_SRCS})

set(TERRAIN_DIR ${XCSOAR_SRC}/Terrain)
set(TERRAIN_SRCS ${TERRAIN_DIR}/RasterBuffer.cpp
                 ${TERRAIN_DIR}/RasterProjection.cpp
                 ${TERRAIN_DIR}/RasterMap.cpp
                 ${TERRAIN_DIR}/RasterTile.cpp
                 ${TERRAIN_DIR}/RasterTileCache.cpp
                 ${TERRAIN_DIR}/Intersection.cpp
                 ${TERRAIN_DIR}/ScanLine.cpp
                 ${TERRAIN_DIR}/RasterTerrain.cpp
                 ${TERRAIN_DIR}/RasterWeatherStore.cpp
                 ${TERRAIN_DIR}/RasterWeatherCache.cpp
                 ${TERRAIN_DIR}/HeightMatrix.cpp
                 ${TERRAIN_DIR}/RasterRenderer.cpp
                 ${TERRAIN_DIR}/TerrainRenderer.cpp
                 ${TERRAIN_DIR}/WeatherTerrainRenderer.cpp
                 ${TERRAIN_DIR}/TerrainSettings.cpp)
include_directories(${XCSOAR_SRC} ${TERRAIN_DIR} ${TERRAIN_DIR}/jasper)
add_library(Terrain ${TERRAIN_SRCS})

set(MATH_DIR ${XCSOAR_SRC}/Math)
set(MATH_SRCS ${MATH_DIR}/Angle.cpp
              ${MATH_DIR}/ARange.cpp
              ${MATH_DIR}/FastMath.cpp
              ${MATH_DIR}/FastTrig.cpp
              ${MATH_DIR}/FastRotation.cpp
              ${MATH_DIR}/fixed.cpp
              ${MATH_DIR}/LeastSquares.cpp
              ${MATH_DIR}/DiffFilter.cpp
              ${MATH_DIR}/Filter.cpp
              ${MATH_DIR}/ZeroFinder.cpp
              ${MATH_DIR}/KalmanFilter1d.cpp
              ${MATH_DIR}/SelfTimingKalmanFilter1d.cpp)
include_directories(${XCSOAR_SRC} ${MATH_DIR} ${OUTPUT_INCLUDE})
add_library(Math ${MATH_SRCS})

set(IO_DIR ${XCSOAR_SRC}/IO)
set(IO_SRCS ${IO_DIR}/FileTransaction.cpp
            ${IO_DIR}/FileCache.cpp
            ${IO_DIR}/FileSource.cpp
            ${IO_DIR}/ZipSource.cpp
            ${IO_DIR}/InflateSource.cpp
            ${IO_DIR}/LineSplitter.cpp
            ${IO_DIR}/ConvertLineReader.cpp
            ${IO_DIR}/FileLineReader.cpp
            ${IO_DIR}/KeyValueFileReader.cpp
            ${IO_DIR}/KeyValueFileWriter.cpp
            ${IO_DIR}/ZipLineReader.cpp
            ${IO_DIR}/InflateLineReader.cpp
            ${IO_DIR}/TextFile.cpp
            ${IO_DIR}/CSVLine.cpp
            ${IO_DIR}/BatchTextWriter.cpp
            ${IO_DIR}/BinaryWriter.cpp
            ${IO_DIR}/TextWriter.cpp)
include_directories(${XCSOAR_SRC} ${IO_DIR})
add_library(Io ${IO_SRCS})

set(WAYPOINT_DIR ${XCSOAR_SRC}/Engine/Waypoint)
set(WAYPOINT_SRCS ${WAYPOINT_DIR}/WaypointVisitor.cpp
                  ${WAYPOINT_DIR}/Waypoints.cpp
                  ${WAYPOINT_DIR}/Waypoint.cpp)
include_directories(${XCSOAR_SRC} ${WAYPOINT_DIR})
add_library(Waypoint ${WAYPOINT_SRCS})

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

set(GLIDE_DIR ${XCSOAR_SRC}/Engine/GlideSolvers)
set(GLIDE_SRCS ${GLIDE_DIR}/GlideSettings.cpp
               ${GLIDE_DIR}/GlideState.cpp
               ${GLIDE_DIR}/GlueGlideState.cpp
               ${GLIDE_DIR}/GlidePolar.cpp
               ${GLIDE_DIR}/PolarCoefficients.cpp
               ${GLIDE_DIR}/GlideResult.cpp
               ${GLIDE_DIR}/MacCready.cpp)
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

set(AIRSPACE_DIR ${XCSOAR_SRC}/Engine/Airspace)
set(ENGINE_DIR   ${XCSOAR_SRC}/Engine)
set(AIRSPACE_SRCS ${ENGINE_DIR}/Util/AircraftStateFilter.cpp
                  ${AIRSPACE_DIR}/AirspacesTerrain.cpp
                  ${AIRSPACE_DIR}/Airspace.cpp
                  ${AIRSPACE_DIR}/AirspaceAltitude.cpp
                  ${AIRSPACE_DIR}/AirspaceAircraftPerformance.cpp
                  ${AIRSPACE_DIR}/AbstractAirspace.cpp
                  ${AIRSPACE_DIR}/AirspaceCircle.cpp
                  ${AIRSPACE_DIR}/AirspacePolygon.cpp
                  ${AIRSPACE_DIR}/Airspaces.cpp
                  ${AIRSPACE_DIR}/AirspaceIntersectSort.cpp
                  ${AIRSPACE_DIR}/Predicate/AirspacePredicate.cpp
                  ${AIRSPACE_DIR}/Predicate/AirspacePredicateAircraftInside.cpp
                  ${AIRSPACE_DIR}/Predicate/AirspacePredicateHeightRange.cpp
                  ${AIRSPACE_DIR}/Predicate/OutsideAirspacePredicate.cpp
                  ${AIRSPACE_DIR}/AirspaceVisitor.cpp
                  ${AIRSPACE_DIR}/AirspaceIntersectionVisitor.cpp
                  ${AIRSPACE_DIR}/AirspaceWarningConfig.cpp
                  ${AIRSPACE_DIR}/AirspaceWarningManager.cpp
                  ${AIRSPACE_DIR}/AirspaceWarning.cpp
                  ${AIRSPACE_DIR}/AirspaceSorter.cpp)
include_directories(${XCSOAR_SRC} ${AIRSPACE_DIR})
add_library(Airspace ${AIRSPACE_SRCS})

# Device drivers
set(DRIVER_DIR ${XCSOAR_SRC}/Device/Driver)

set(VOLKSLOGGER_SRCS ${DRIVER_DIR}/Volkslogger/Register.cpp
                     ${DRIVER_DIR}/Volkslogger/Parser.cpp
                     ${DRIVER_DIR}/Volkslogger/Protocol.cpp
                     ${DRIVER_DIR}/Volkslogger/Declare.cpp
                     ${DRIVER_DIR}/Volkslogger/Database.cpp
                     ${DRIVER_DIR}/Volkslogger/Util.cpp
                     ${DRIVER_DIR}/Volkslogger/dbbconv.cpp
                     ${DRIVER_DIR}/Volkslogger/grecord.cpp
                     ${DRIVER_DIR}/Volkslogger/vlapi2.cpp
                     ${DRIVER_DIR}/Volkslogger/vlapihlp.cpp
                     ${DRIVER_DIR}/Volkslogger/vlutils.cpp
                     ${DRIVER_DIR}/Volkslogger/vlconv.cpp
                     ${DRIVER_DIR}/Volkslogger/Logger.cpp)
set(CAI302_SRCS ${DRIVER_DIR}/CAI302/Protocol.cpp
                ${DRIVER_DIR}/CAI302/PocketNav.cpp
                ${DRIVER_DIR}/CAI302/Mode.cpp
                ${DRIVER_DIR}/CAI302/Parser.cpp
                ${DRIVER_DIR}/CAI302/Settings.cpp
                ${DRIVER_DIR}/CAI302/Declare.cpp
                ${DRIVER_DIR}/CAI302/Logger.cpp
                ${DRIVER_DIR}/CAI302/Manage.cpp
                ${DRIVER_DIR}/CAI302/Register.cpp)
set(IMI_SRCS ${DRIVER_DIR}/IMI/Protocol/MessageParser.cpp
             ${DRIVER_DIR}/IMI/Protocol/Communication.cpp
             ${DRIVER_DIR}/IMI/Protocol/Checksum.cpp
             ${DRIVER_DIR}/IMI/Protocol/Conversion.cpp
             ${DRIVER_DIR}/IMI/Protocol/IGC.cpp
             ${DRIVER_DIR}/IMI/Protocol/Protocol.cpp
             ${DRIVER_DIR}/IMI/Declare.cpp
             ${DRIVER_DIR}/IMI/Internal.cpp
             ${DRIVER_DIR}/IMI/Logger.cpp
             ${DRIVER_DIR}/IMI/Register.cpp)
set(LX_SRCS ${DRIVER_DIR}/LX/NanoLogger.cpp
            ${DRIVER_DIR}/LX/NanoDeclare.cpp
            ${DRIVER_DIR}/LX/Protocol.cpp
            ${DRIVER_DIR}/LX/Mode.cpp
            ${DRIVER_DIR}/LX/Parser.cpp
            ${DRIVER_DIR}/LX/Settings.cpp
            ${DRIVER_DIR}/LX/Declare.cpp
            ${DRIVER_DIR}/LX/Logger.cpp
            ${DRIVER_DIR}/LX/Convert.cpp
            ${DRIVER_DIR}/LX/LXN.cpp
            ${DRIVER_DIR}/LX/Register.cpp)
set(FLARM_SRCS ${DRIVER_DIR}/FLARM/Device.cpp
               ${DRIVER_DIR}/FLARM/Register.cpp
               ${DRIVER_DIR}/FLARM/Mode.cpp
               ${DRIVER_DIR}/FLARM/Parser.cpp
               ${DRIVER_DIR}/FLARM/StaticParser.cpp
               ${DRIVER_DIR}/FLARM/Settings.cpp
               ${DRIVER_DIR}/FLARM/Declare.cpp
               ${DRIVER_DIR}/FLARM/Logger.cpp
               ${DRIVER_DIR}/FLARM/CRC16.cpp
               ${DRIVER_DIR}/FLARM/BinaryProtocol.cpp
               ${DRIVER_DIR}/FLARM/TextProtocol.cpp)
set(FLYTEC_SRCS ${DRIVER_DIR}/Flytec/Register.cpp
                ${DRIVER_DIR}/Flytec/Parser.cpp
                ${DRIVER_DIR}/Flytec/Logger.cpp)
set(VEGA_SRCS ${DRIVER_DIR}/Vega/Misc.cpp
              ${DRIVER_DIR}/Vega/Parser.cpp
              ${DRIVER_DIR}/Vega/Settings.cpp
              ${DRIVER_DIR}/Vega/Volatile.cpp
              ${DRIVER_DIR}/Vega/Register.cpp)
set(OTHER_SRCS ${DRIVER_DIR}/AltairPro.cpp
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
               ${DRIVER_DIR}/GTAltimeter.cpp
               ${DRIVER_DIR}/NmeaOut.cpp
               ${DRIVER_DIR}/OpenVario.cpp
               ${DRIVER_DIR}/PosiGraph.cpp
               ${DRIVER_DIR}/XCOM760.cpp
               ${DRIVER_DIR}/ILEC.cpp
               ${DRIVER_DIR}/Westerboer.cpp
               ${DRIVER_DIR}/Zander.cpp
               ${DRIVER_DIR}/Vaulter.cpp)
set(DRIVER_SRCS ${VOLKSLOGGER_SRCS}
                ${CAI302_SRCS}
                ${IMI_SRCS}
                ${LX_SRCS}
                ${FLARM_SRCS}
                ${FLYTEC_SRCS}
                ${VEGA_SRCS}
                ${BLUEFLY_SRCS}
                ${OTHER_SRCS})
include_directories(${XCSOAR_SRC} ${DRIVER_DIR})
add_library(Driver ${DRIVER_SRCS})

set(SHAPE_DIR ${XCSOAR_SRC}/Topography/shapelib)
set(SHAPE_SRCS ${SHAPE_DIR}/mapstring.c
               ${SHAPE_DIR}/mapbits.c
               ${SHAPE_DIR}/mapfile.c
               ${SHAPE_DIR}/mapprimitive.c
               ${SHAPE_DIR}/mapsearch.c
               ${SHAPE_DIR}/mapshape.c
               ${SHAPE_DIR}/maptree.c
               ${SHAPE_DIR}/mapxbase.c)
include_directories(${XCSOAR_SRC} ${SHAPE_DIR})
add_library(Shape ${SHAPE_SRCS})

set(THREAD_DIR ${XCSOAR_SRC}/Thread)
set(THREAD_SRCS ${XCSOAR_SRC}/Poco/RWLock.cpp
                ${THREAD_DIR}/Thread.cpp
                ${THREAD_DIR}/SuspensibleThread.cpp
                ${THREAD_DIR}/RecursivelySuspensibleThread.cpp
                ${THREAD_DIR}/WorkerThread.cpp
                ${THREAD_DIR}/StandbyThread.cpp
                ${THREAD_DIR}/Mutex.cpp
                ${THREAD_DIR}/Debug.cpp)
include_directories(${XCSOAR_SRC} ${THREAD_DIR})
add_library(Thread ${THREAD_SRCS})

set(OS_DIR ${XCSOAR_SRC}/OS)
set(OS_SRCS	${OS_DIR}/Clock.cpp
            ${OS_DIR}/SocketAddress.cpp
            ${OS_DIR}/SocketDescriptor.cpp
            ${OS_DIR}/FileDescriptor.cpp
            ${OS_DIR}/FileMapping.cpp
            ${OS_DIR}/FileUtil.cpp
            ${OS_DIR}/PathName.cpp
            ${OS_DIR}/Process.cpp
            ${OS_DIR}/SystemLoad.cpp
            ${OS_DIR}/Poll.cpp
            ${OS_DIR}/EventPipe.cpp)
include_directories(${XCSOAR_SRC} ${OS_DIR})
add_library(Os ${OS_SRCS})

set(WIDGET_DIR ${XCSOAR_SRC}/Widget)
set(WIDGET_SRCS ${WIDGET_DIR}/Widget.cpp
                ${WIDGET_DIR}/ActionWidget.cpp
                ${WIDGET_DIR}/CallbackWidget.cpp
                ${WIDGET_DIR}/WindowWidget.cpp
                ${WIDGET_DIR}/CreateWindowWidget.cpp
                ${WIDGET_DIR}/ListWidget.cpp
                ${WIDGET_DIR}/ContainerWidget.cpp
                ${WIDGET_DIR}/SolidWidget.cpp
                ${WIDGET_DIR}/PanelWidget.cpp
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
                ${WIDGET_DIR}/XMLWidget.cpp
                ${WIDGET_DIR}/ButtonPanelWidget.cpp
                ${WIDGET_DIR}/QuestionWidget.cpp
                ${WIDGET_DIR}/KeyboardWidget.cpp
                ${WIDGET_DIR}/DockWindow.cpp)
include_directories(${XCSOAR_SRC} ${WIDGET_DIR})
add_library(Widget ${WIDGET_SRCS})

include_directories(${XCSOAR_SRC} ${XCSOAR_SRC}/Profile)
add_library(Profile ${XCSOAR_SRC}/Profile/ProfileMap.cpp)

set(JASPER_DIR ${XCSOAR_SRC}/Terrain/jasper)
set(JASPER_SRCS ${JASPER_DIR}/base/jas_debug.c
                ${JASPER_DIR}/base/jas_malloc.c
                ${JASPER_DIR}/base/jas_seq.c
                ${JASPER_DIR}/base/jas_stream.c
                ${JASPER_DIR}/base/jas_string.c
                ${JASPER_DIR}/base/jas_tvp.c
                ${JASPER_DIR}/jp2/jp2_cod.c
                ${JASPER_DIR}/jp2/jp2_dec.c
                ${JASPER_DIR}/jpc/jpc_bs.c
                ${JASPER_DIR}/jpc/jpc_cs.c
                ${JASPER_DIR}/jpc/jpc_dec.c
                ${JASPER_DIR}/jpc/jpc_math.c
                ${JASPER_DIR}/jpc/jpc_mct.c
                ${JASPER_DIR}/jpc/jpc_mqdec.c
                ${JASPER_DIR}/jpc/jpc_mqcod.c
                ${JASPER_DIR}/jpc/jpc_qmfb.c
                ${JASPER_DIR}/jpc/jpc_rtc.cpp
                ${JASPER_DIR}/jpc/jpc_t1dec.c
                ${JASPER_DIR}/jpc/jpc_t1cod.c
                ${JASPER_DIR}/jpc/jpc_t2dec.c
                ${JASPER_DIR}/jpc/jpc_t2cod.c
                ${JASPER_DIR}/jpc/jpc_tagtree.c
                ${JASPER_DIR}/jpc/jpc_tsfb.c
                ${JASPER_DIR}/jpc/jpc_util.c)
include_directories(${XCSOAR_SRC} ${JASPER_DIR})
add_library(Jasper ${JASPER_SRCS})
set_target_properties(Jasper
                      PROPERTIES COMPILE_FLAGS "-Wno-unused-but-set-parameter -Wno-unused-but-set-variable")

set(FORM_DIR ${XCSOAR_SRC}/Form)
set(FORM_SRCS	${XCSOAR_SRC}/UIUtil/KineticManager.cpp
              ${XCSOAR_SRC}/Renderer/ButtonRenderer.cpp
              ${XCSOAR_SRC}/Renderer/SymbolRenderer.cpp
              ${FORM_DIR}/Internal.cpp
              ${FORM_DIR}/Control.cpp
              ${FORM_DIR}/Panel.cpp
              ${FORM_DIR}/SubForm.cpp
              ${FORM_DIR}/Form.cpp
              ${FORM_DIR}/Button.cpp
              ${FORM_DIR}/ButtonPanel.cpp
              ${FORM_DIR}/CheckBox.cpp
              ${FORM_DIR}/SymbolButton.cpp
              ${FORM_DIR}/Frame.cpp
              ${FORM_DIR}/Draw.cpp
              ${FORM_DIR}/List.cpp
              ${FORM_DIR}/ScrollBar.cpp
              ${FORM_DIR}/Edit.cpp
              ${FORM_DIR}/DigitEntry.cpp
              ${FORM_DIR}/TabBar.cpp
              ${FORM_DIR}/TabDisplay.cpp
              ${FORM_DIR}/TabMenuDisplay.cpp
              ${FORM_DIR}/CharacterButton.cpp
              ${FORM_DIR}/UnitUtil.cpp
              ${FORM_DIR}/GridView.cpp
              ${FORM_DIR}/CustomButton.cpp
              ${FORM_DIR}/HLine.cpp
              ${FORM_DIR}/Util.cpp)
include_directories(${XCSOAR_SRC} ${FORM_DIR})
add_library(Form ${FORM_SRCS})

set(DATAFIELD_DIR ${XCSOAR_SRC}/Form/DataField)
set(DATAFIELD_SRCS 	${DATAFIELD_DIR}/Base.cpp
                    ${DATAFIELD_DIR}/Boolean.cpp
                    ${DATAFIELD_DIR}/ComboList.cpp
                    ${DATAFIELD_DIR}/Enum.cpp
                    ${DATAFIELD_DIR}/FileReader.cpp
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

set(SCREEN_DIR ${XCSOAR_SRC}/Screen)
set(SCREEN_SRCS ${SCREEN_DIR}/Debug.cpp
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
                ${SCREEN_DIR}/SingleWindow.cpp)

set(FREETYPE_SRCS ${SCREEN_DIR}/FreeType/Font.cpp
                  ${SCREEN_DIR}/FreeType/Init.cpp)

set(OPENGL_SRCS ${SCREEN_DIR}/Custom/Cache.cpp
                ${SCREEN_DIR}/OpenGL/Init.cpp
                ${SCREEN_DIR}/OpenGL/Dynamic.cpp
                ${SCREEN_DIR}/OpenGL/Rotate.cpp
                ${SCREEN_DIR}/OpenGL/Geo.cpp
                ${SCREEN_DIR}/OpenGL/Globals.cpp
                ${SCREEN_DIR}/OpenGL/Extension.cpp
                ${SCREEN_DIR}/OpenGL/FBO.cpp
                ${SCREEN_DIR}/OpenGL/VertexArray.cpp
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
                ${SCREEN_DIR}/OpenGL/Triangulate.cpp)
include_directories(${XCSOAR_SRC}
                    ${SCREEN_DIR}
                    ${SCREEN_DIR}/FreeType
                    ${SCREEN_DIR}/OpenGL)
add_library(Screen ${SCREEN_SRCS} ${FREETYPE_SRCS} ${OPENGL_SRCS})
# TODO Find this package.
set_target_properties(Screen
                      PROPERTIES COMPILE_FLAGS "-isystem /usr/include/freetype2")

set(EVENT_DIR ${XCSOAR_SRC}/Event)
set(EVENT_SRCS 	${EVENT_DIR}/Shared/Timer.cpp
                ${EVENT_DIR}/Shared/TimerQueue.cpp
                ${EVENT_DIR}/Globals.cpp
                ${EVENT_DIR}/Idle.cpp
                ${EVENT_DIR}/DelayedNotify.cpp
                ${EVENT_DIR}/Notify.cpp)
include_directories(${XCSOAR_SRC} ${EVENT_DIR})
add_library(Event ${EVENT_SRCS})

#include_directories(${XCSOAR_SRC})
#add_library(Resource ${XCSOAR_SRC}/ResourceLoader.cpp)

set(ASYNC_DIR ${XCSOAR_SRC}/IO/Async)
set(ASYNC_SRCS 	${ASYNC_DIR}/DiscardFileEventHandler.cpp
                ${ASYNC_DIR}/IOLoop.cpp
                ${ASYNC_DIR}/IOThread.cpp
                ${ASYNC_DIR}/GlobalIOThread.cpp)
include_directories(${XCSOAR_SRC} ${ASYNC_DIR})
add_library(Async ${ASYNC_SRCS})
