#include <event2/event.h>

int main() {
    auto base = event_base_new();
    event_base_free(base);
}
