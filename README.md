# SterorCamera

2017-09-06: run error:  libopencv*.so connect to libQt*.so.4 ???
2017-09-22: open ttyS0 in root
            cannot receive data from ttyS0,

2017-9-26:  command end with '\n' instead of '\r'
            call QSerialPort::realLine() in readData()
            add ~TrackerImagePool() in TrackerImagePool.cpp
            getMarkerCenters3D() need to be complete in TrackerImageProcessor.cpp
            slot function handleCameraError() when TrackerImageCapture happend error
