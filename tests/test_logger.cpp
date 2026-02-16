#include "core/logger/logger.hpp"
#include <cstdlib>

int main() {
    // Test 1: Info log
    LOG_INFO("System initialized");
    
    // Test 2: Warning with printf-style formatting
    LOG_WARN("Low memory warning: %d%% free", 15);
    
    // Test 3: Error with string formatting
    LOG_ERR("File not found: %s", "config.json");
    
    // Test 4: Debug with pointer formatting
    LOG_DEBUG("Pointer address: %p", (void*)0x1234);
    
    return EXIT_SUCCESS;
}
