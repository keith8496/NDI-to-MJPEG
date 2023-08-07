#include <cstdio>
#include <chrono>
#include <cstring>
#include <Processing.NDI.Lib.h>

#ifdef _WIN32
#ifdef _WIN64
#pragma comment(lib, "Processing.NDI.Lib.x64.lib")
#else // _WIN64
#pragma comment(lib, "Processing.NDI.Lib.x86.lib")
#endif // _WIN64
#endif // _WIN32

int main(int argc, char* argv[])
{
	// Exit if missing arguments
    if (argc < 2) {
        printf("Error: Missing command line arguments.\n");
        return -1;
    }
    const char* searchSourceName = argv[1];

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

	// Run for one minute
	using namespace std::chrono;
	for (const auto start = high_resolution_clock::now(); high_resolution_clock::now() - start < minutes(1);) {
		// The descriptors
		NDIlib_video_frame_v2_t video_frame;
		NDIlib_audio_frame_v2_t audio_frame;

		switch (NDIlib_recv_capture_v2(pNDI_recv, &video_frame, &audio_frame, nullptr, 5000)) {
			// No data
			case NDIlib_frame_type_none:
				printf("No data received.\n");
				break;

				// Video data
			case NDIlib_frame_type_video:
				//printf("Video data received (%dx%d).\n", video_frame.xres, video_frame.yres);
				NDIlib_recv_free_video_v2(pNDI_recv, &video_frame);
				break;

				// Audio data
			case NDIlib_frame_type_audio:
				//printf("Audio data received (%d samples).\n", audio_frame.no_samples);
				NDIlib_recv_free_audio_v2(pNDI_recv, &audio_frame);
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