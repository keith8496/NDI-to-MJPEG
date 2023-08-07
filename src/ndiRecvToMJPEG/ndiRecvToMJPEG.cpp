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

    // Not required, but "correct" (see the SDK documentation).
	if (!NDIlib_initialize())
		return 0;

	// We are going to create an NDI finder that locates sources on the network.
	NDIlib_find_instance_t pNDI_find = NDIlib_find_create_v2();
	if (!pNDI_find)
		return 0;

	// Run for one minute
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
			const char* searchSourceName = argv[1];
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

	// Destroy the NDI finder
	NDIlib_find_destroy(pNDI_find);

	// Finished
	NDIlib_destroy();

	// Success. We are done
	return 0;
}