#pragma once
#include "Config.hpp"

#if _USE_THREAD_POOL
#include <mutex>


namespace ESU_Parallel
{
	static std::mutex nauty_mtx;
}
#endif
