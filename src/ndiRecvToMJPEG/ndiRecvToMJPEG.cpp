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

int main(int argc, char* argv[]) {

	if (argc < 2) {
        printf("Error: Missing NDI Source Name.\n");
        return -1;
    }
    
	// Not required, but "correct" (see the SDK documentation).
	if (!NDIlib_initialize())
		return 0;

	// We are going to create an NDI finder that locates sources on the network.
	uint32_t source_no_found;
    NDIlib_find_instance_t pNDI_find = NDIlib_find_create_v2();
	if (!pNDI_find)
		return 0;

	// Run for one minute
	using namespace std::chrono;
	for (const auto start = high_resolution_clock::now(); high_resolution_clock::now() - start < minutes(1);) {
		// Wait up till 5 seconds to check for new sources to be added or removed
		if (!NDIlib_find_wait_for_sources(pNDI_find, 5000 /* milliseconds */)) {
			printf("Searching for source...\n");
			continue;
		}

		// Get the updated list of sources
		uint32_t no_sources = 0;
		const NDIlib_source_t* p_sources = NDIlib_find_get_current_sources(pNDI_find, &no_sources);

		// Find our source
		for (uint32_t i = 0; i < no_sources; i++) {
			if (strcmp(p_sources[i].p_ndi_name,argv[1])==0) {
                printf("Found %s\n", i + 1, p_sources[i].p_ndi_name);
                source_no_found = i;
                break;
            }
        }
    }

    if (!source_no_found) {
        printf("Error: Source not found!\n");
        return -2;
    }

	// Destroy the NDI finder
	NDIlib_find_destroy(pNDI_find);

	// Finished
	NDIlib_destroy();

	// Success. We are done
	return 0;
}