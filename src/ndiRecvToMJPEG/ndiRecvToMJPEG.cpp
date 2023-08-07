#include <cstdio>
#include <chrono>
#include <cstring>
#include <Processing.NDI.Lib.h>
#include <nadjieb/mjpeg_streamer.hpp>
#include <opencv2/opencv.hpp>

#ifdef _WIN32
#ifdef _WIN64
#pragma comment(lib, "Processing.NDI.Lib.x64.lib")
#else // _WIN64
#pragma comment(lib, "Processing.NDI.Lib.x86.lib")
#endif // _WIN64
#endif // _WIN32

// for convenience
using MJPEGStreamer = nadjieb::MJPEGStreamer;


int main(int argc, char* argv[])
{
	//
    // Check Arguments
    //
    if (argc < 2) {
        printf("Error: Missing command line arguments.\n");
        return -1;
    }
    
    const char* searchSourceName = argv[1];

    
    //
    // Setup Streamer
    //
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 90};
    MJPEGStreamer streamer;

    // By default "/shutdown" is the target to graceful shutdown the streamer
    // if you want to change the target to graceful shutdown:
    //      streamer.setShutdownTarget("/stop");

    // By default std::thread::hardware_concurrency() workers is used for streaming
    // if you want to use 4 workers instead:
    //      streamer.start(8080, 4);
    streamer.start(8080);


    //
    // Setup NDI
    //

    // Not required, but "correct" (see the SDK documentation).
	if (!NDIlib_initialize())
		return 0;

	// We are going to create an NDI finder that locates sources on the network.
	NDIlib_find_instance_t pNDI_find = NDIlib_find_create_v2();
	if (!pNDI_find)
		return 0;

	// Search for one minute
	int source_no_found = -1;
    const NDIlib_source_t* p_sources = NULL;
	using namespace std::chrono;
	for (const auto start = high_resolution_clock::now(); high_resolution_clock::now() - start < minutes(1);) {
		// Wait up till 5 seconds to check for new sources to be added or removed
    	if (!NDIlib_find_wait_for_sources(pNDI_find, 5000 /* milliseconds */)) {
			printf("Searching for source...\n");
			continue;
		}

		// Get the updated list of sources
		uint32_t no_sources = 0;
	    p_sources = NDIlib_find_get_current_sources(pNDI_find, &no_sources);

		// Find our source
		for (uint32_t i = 0; i < no_sources; i++) {
			const char* ndiSourceName = p_sources[i].p_ndi_name;
            if ( strcmp(searchSourceName, ndiSourceName) == 0 ) {
                printf("Found %s\n", ndiSourceName);
                source_no_found = i;
            }
        }

        // Exit loop if source was found
        if (source_no_found > -1) break;

	}

    // Error if source not found
    if (source_no_found == -1) {
        printf("Error: Source was not found.\n");
        return -2;
    }

	// We now have a source, so we create a receiver to look at it.
	NDIlib_recv_instance_t pNDI_recv = NDIlib_recv_create_v3();
	if (!pNDI_recv)
		return 0;

	// Connect to our sources
    NDIlib_recv_connect(pNDI_recv, &p_sources[source_no_found]);

	// Destroy the NDI finder. We needed to have access to the pointers to p_sources[0]
	NDIlib_find_destroy(pNDI_find);

	//// Run for one minute
	//using namespace std::chrono;
    //for (const auto start = high_resolution_clock::now(); high_resolution_clock::now() - start < minutes(1);) {
    // Visit /shutdown or another defined target to stop the loop and graceful shutdown
    while (streamer.isRunning()) {
		NDIlib_video_frame_v2_t video_frame;

		switch (NDIlib_recv_capture_v3(pNDI_recv, &video_frame, nullptr, nullptr, 5000)) {
			// No data
			case NDIlib_frame_type_none:
				printf("No data received.\n");
				break;

			// Video data
			case NDIlib_frame_type_video:
				//printf("Video data received (%dx%d).\n", video_frame.xres, video_frame.yres);
                
                // http://localhost:8080/bgr
                std::vector<uchar> buff_bgr;
                cv::imencode(".jpg", frame, buff_bgr, params);
                streamer.publish("/bgr", std::string(buff_bgr.begin(), buff_bgr.end()));

                cv::Mat hsv;
                cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

                // http://localhost:8080/hsv
                std::vector<uchar> buff_hsv;
                cv::imencode(".jpg", hsv, buff_hsv, params);
                streamer.publish("/hsv", std::string(buff_hsv.begin(), buff_hsv.end()));
                
                NDIlib_recv_free_video_v2(pNDI_recv, &video_frame);
				break;
		}
	}

	// Destroy the receiver
	NDIlib_recv_destroy(pNDI_recv);

	// Finished
	NDIlib_destroy();

	// Success. We are done
	return 0;
}