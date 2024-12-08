#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <mpg123.h>
#include <lame/lame.h>

namespace converter {

int getBitrate(const std::string& filename);

int getMP3Bitrate(mpg123_handle* mh);

std::string changeBitrateDirectly(const std::string& inputMP3Data, int compress_degree);

} // namespace converter