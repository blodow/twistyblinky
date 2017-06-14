#pragma once

namespace uhttp {

int parseHeader(const char* buf, const char** body, int* length);

} // end uhttp namespace
